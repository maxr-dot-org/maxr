/***************************************************************************
*      Mechanized Assault and Exploration Reloaded Projectfile            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include "actioninitnewgame.h"
#include "game/data/model.h"
#include "game/data/gamesettings.h"
#include "game/data/map/map.h"
#include "utility/log.h"
#include "game/data/player/player.h"
#include "utility/listhelpers.h"
#include "game/data/player/clans.h"
#include "utility/string/toString.h"

// TODO: remove
std::vector<std::pair<sID, int>> createInitialLandingUnitsList(int clan, const cGameSettings& gameSettings, const cUnitsData& unitsData); // defined in windowsingleplayer.cpp


//------------------------------------------------------------------------------
cActionInitNewGame::cActionInitNewGame() : 
	cAction(eActiontype::ACTION_INIT_NEW_GAME), 
	clan(-1)
{};

//------------------------------------------------------------------------------
cActionInitNewGame::cActionInitNewGame(cBinaryArchiveOut& archive)
	: cAction(eActiontype::ACTION_INIT_NEW_GAME)
{
	serializeThis(archive);
}

//------------------------------------------------------------------------------
void cActionInitNewGame::execute(cModel& model) const
{
	//Note: this funktion handels incoming data from network. Make every possible sanity check!
	
	cPlayer& player = *model.getPlayer(playerNr);
	const cUnitsData& unitsdata = *model.getUnitsData();

	player.removeAllUnits();
	Log.write(" GameId: " + toString(model.getGameId()), cLog::eLOG_TYPE_NET_DEBUG);

	// init clan
	if (model.getGameSettings()->getClansEnabled())
	{
		if (clan < 0 || static_cast<size_t>(clan) >= unitsdata.getNrOfClans())
		{
			Log.write(" Landing failed. Invalid clan number.", cLog::eLOG_TYPE_NET_ERROR);
			return;
		}
		player.setClan(clan, unitsdata);
	}
	else
	{
		player.setClan(-1, unitsdata);
	}

	// init landing position
	if (!model.getMap()->isValidPosition(landingPosition))
	{
		Log.write(" Received invalid landing position", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	cPosition updatedLandingPosition = landingPosition;
	if (model.getGameSettings()->getBridgeheadType() == eGameSettingsBridgeheadType::Definite)
	{
		// Find place for mine if bridgehead is fixed
		if (!findPositionForStartMine(updatedLandingPosition, model.getUnitsData(), model.getMap()->staticMap))
		{
			Log.write("couldn't place player start mine: " + player.getName(), cLog::eLOG_TYPE_NET_ERROR);
			return;
		}
	}
	player.setLandingPos(updatedLandingPosition);

	// place new ressources
	model.getMap()->placeRessources(model);

	// apply upgrades
	int credits = model.getGameSettings()->getStartCredits();
	for (const auto& upgrade : unitUpgrades)
	{
		const sID& unitId = upgrade.first;
		const cUnitUpgrade& upgradeValues = upgrade.second;

		if (!unitsdata.isValidId(unitId))
		{
			Log.write(" Apply upgrades failed. Unknown sID: " + unitId.getText(), cLog::eLOG_TYPE_NET_ERROR);
			return;
		}
		int costs = upgradeValues.calcTotalCosts(unitsdata.getDynamicUnitData(unitId, player.getClan()), *player.getUnitDataCurrentVersion(unitId), player.getResearchState());
		if (costs <= 0)
		{
			Log.write(" Apply upgrades failed. Couldn't calculate costs.", cLog::eLOG_TYPE_NET_ERROR);
			return;
		}
		credits -= costs;
		if (credits <= 0)
		{
			Log.write(" Apply upgrade failed. Used more than the available credits.", cLog::eLOG_TYPE_NET_ERROR);
			return;
		}
		upgradeValues.updateUnitData(*player.getUnitDataCurrentVersion(unitId));
	}

	// land vehicles
	auto initialLandingUnits = createInitialLandingUnitsList(clan, *model.getGameSettings(), unitsdata);
	for (const auto& landing : landingUnits)
	{
		if (!unitsdata.isValidId(landing.unitID))
		{
			Log.write(" Landing failed. Unknown sID: " + landing.unitID.getText(), cLog::eLOG_TYPE_NET_ERROR);
			return;
		}

		auto it = std::find_if(initialLandingUnits.begin(), initialLandingUnits.end(), [landing](std::pair<sID, int> unit){ return unit.first == landing.unitID; });
		if (it != initialLandingUnits.end())
		{
			// landing unit is one of the initial landing units, that the player gets for free
			// so don't pay for the unit and the default cargo
			credits -= (landing.cargo - it->second) / 5;
			initialLandingUnits.erase(it);
		}
		else
		{
			credits -= landing.cargo / 5;
			credits -= unitsdata.getDynamicUnitData(landing.unitID, clan).getBuildCost();
		}
	}
	if (credits < 0)
	{
		Log.write(" Landing failed. Used more than the available credits", cLog::eLOG_TYPE_ERROR);
		return;
	}
	makeLanding(player, landingUnits, model);

	//transfer remaining credits to player
	player.setCredits(credits);
}

bool cActionInitNewGame::isValidLandingPosition(cPosition position, std::shared_ptr<cStaticMap> map, bool fixedBridgeHead, const std::vector<sLandingUnit>& units, std::shared_ptr<const cUnitsData> unitsData)
{
	std::vector<cPosition> blockedPositions;
	if (fixedBridgeHead)
	{
		bool found = findPositionForStartMine(position, unitsData, map);
		if (!found)
			return false;

		//small generator
		blockedPositions.push_back(position + cPosition(-1, 0));
		//mine
		blockedPositions.push_back(position + cPosition(0, -1));
		blockedPositions.push_back(position + cPosition(1, -1));
		blockedPositions.push_back(position + cPosition(1,  0));
		blockedPositions.push_back(position + cPosition(0,  0));
	}

	for (const auto& unit : units)
	{
		const cStaticUnitData& unitData = unitsData->getStaticUnitData(unit.unitID);
		bool placed = false;
		int landingRadius = 1;

		while (!placed)
		{
			landingRadius += 1;

			// prevent, that units are placed far away from the starting position 
			if (landingRadius > 1.5 * sqrt(units.size()) && landingRadius > 3) return false;

			for (int offY = -landingRadius; (offY < landingRadius) && !placed; ++offY)
			{
				for (int offX = -landingRadius; (offX < landingRadius) && !placed; ++offX)
				{
					const cPosition place = position + cPosition(offX, offY);
					if (map->possiblePlace(unitData, place) && !Contains(blockedPositions, place))
					{
						blockedPositions.push_back(place);
						placed = true;
					}
				}
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------------
void cActionInitNewGame::makeLanding(cPlayer& player, const std::vector<sLandingUnit>& landingUnits, cModel& model) const
{
	cMap& map = *model.getMap();
	cPosition landingPosition = player.getLandingPos();

	if (model.getGameSettings()->getBridgeheadType() == eGameSettingsBridgeheadType::Definite)
	{
		// place buildings:
		model.addBuilding(landingPosition + cPosition(-1, 0), model.getUnitsData()->getSpecialIDSmallGen(), &player, true);
		model.addBuilding(landingPosition + cPosition(0, -1), model.getUnitsData()->getSpecialIDMine(), &player, true);
	}

	for (size_t i = 0; i != landingUnits.size(); ++i)
	{
		const sLandingUnit& landing = landingUnits[i];
		cVehicle* vehicle = nullptr;
		int radius = 1;

		if (!model.getUnitsData()->isValidId(landing.unitID))
		{
			Log.write(" Landing of unit failed. Unknown sID: " + landing.unitID.getText(), cLog::eLOG_TYPE_NET_ERROR);
			continue;
		}

		while (!vehicle)
		{
			vehicle = landVehicle(landingPosition, radius, landing.unitID, player, model);
			radius += 1;
		}
		if (landing.cargo && vehicle)
		{
			if (vehicle->getStaticUnitData().storeResType != eResourceType::Gold)
				vehicle->setStoredResources(landing.cargo);
		}
	}
}
//------------------------------------------------------------------------------
cVehicle* cActionInitNewGame::landVehicle(const cPosition& landingPosition, int radius, const sID& id, cPlayer& player, cModel& model) const
{
	for (int offY = -radius; offY < radius; ++offY)
	{
		for (int offX = -radius; offX < radius; ++offX)
		{
			if (!model.getMap()->possiblePlaceVehicle(model.getUnitsData()->getStaticUnitData(id), landingPosition + cPosition(offX, offY), &player)) continue;

			return &model.addVehicle(landingPosition + cPosition(offX, offY), id, &player, true);
		}
	}
	return nullptr;
}

bool cActionInitNewGame::findPositionForStartMine(cPosition& position, std::shared_ptr<const cUnitsData> unitsData, std::shared_ptr<const cStaticMap> map)
{
	int radius = 0;
	while (true)
	{
		for (int offY = -radius; offY <= radius; ++offY)
		{
			for (int offX = -radius; offX <= radius; ++offX)
			{
				const cPosition place = position + cPosition(offX, offY);
				if (map->possiblePlace(unitsData->getSmallGeneratorData(), place + cPosition(-1, 0)) &&
					map->possiblePlace(unitsData->getMineData(), place + cPosition(0, -1)))
				{
					position = place;
					return true;
				}
			}
		}
		radius += 1;

		if (radius > 2)
		{
			return false;
		}
	}

}
