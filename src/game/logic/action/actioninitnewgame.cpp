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
	//TODO: clan
	//TODO: upgrades
	//TODO: copy credits
	//TODO: delete all units
	//TODO: Fehlerprüfung der empfangenen Nachricht
	cPlayer& player = *model.getPlayer(playerNr);
	player.setLandingPos(landingPosition);
	makeLanding(landingPosition, player, landingUnits, model);
	model.getMap()->placeRessources(model);
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

			//prevent, that units are placed far away from the starting position
			if (landingRadius > sqrt(units.size()) && landingRadius > 3) return false;

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
void cActionInitNewGame::makeLanding(const cPosition& position, cPlayer& player, const std::vector<sLandingUnit>& landingUnits, cModel& model) const
{
	cMap& map = *model.getMap();
	cPosition landingPosition = position;

	// Find place for mine if bridgehead is fixed
	if (model.getGameSettings()->getBridgeheadType() == eGameSettingsBridgeheadType::Definite)
	{
		if (findPositionForStartMine(landingPosition, model.getUnitsData(), map.staticMap))
		{
			// place buildings:
			model.addBuilding(landingPosition + cPosition(-1, 0), model.getUnitsData()->specialIDSmallGen, &player, true);
			model.addBuilding(landingPosition + cPosition(0, -1), model.getUnitsData()->specialIDMine, &player, true);
		}
		else
		{
			Log.write("couldn't place player start mine: " + player.getName(), cLog::eLOG_TYPE_NET_ERROR);
		}
	}

	for (size_t i = 0; i != landingUnits.size(); ++i)
	{
		const sLandingUnit& landing = landingUnits[i];
		cVehicle* vehicle = nullptr;
		int radius = 1;

		while (!vehicle)
		{
			vehicle = landVehicle(landingPosition, radius, landing.unitID, player, model);
			radius += 1;
		}
		if (landing.cargo && vehicle)
		{
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
	int radius = 1;
	while (true)
	{
		for (int offY = -radius; offY < radius; ++offY)
		{
			for (int offX = -radius; offX < radius; ++offX)
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
