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

#include "game/data/gamesettings.h"
#include "game/data/map/map.h"
#include "game/data/model.h"
#include "game/data/player/clans.h"
#include "game/data/player/player.h"
#include "game/startup/gamepreparation.h"
#include "utility/listhelpers.h"
#include "utility/log.h"
#include "utility/mathtools.h"
#include "utility/ranges.h"

namespace
{
	// TODO: Hard coded alien units
	const sID alienAssaultId {0, 2};
	const sID alienPlaneId {0, 3};
	const sID alienShipId {0, 4};
	const sID alienTankId {0, 5};

	//--------------------------------------------------------------------------
	int getResourceDensityFactor (eGameSettingsResourceDensity density)
	{
		switch (density)
		{
			case eGameSettingsResourceDensity::Sparse:
				return 0;
			case eGameSettingsResourceDensity::Normal:
				return 1;
			case eGameSettingsResourceDensity::Dense:
				return 2;
			case eGameSettingsResourceDensity::TooMuch:
				return 3;
		}
		assert (false);
		return 0;
	}

	//--------------------------------------------------------------------------
	int getResourceAmountFactor (eGameSettingsResourceAmount amount)
	{
		switch (amount)
		{
			case eGameSettingsResourceAmount::Limited:
				return 0;
			case eGameSettingsResourceAmount::Normal:
				return 1;
			case eGameSettingsResourceAmount::High:
				return 2;
			case eGameSettingsResourceAmount::TooMuch:
				return 3;
		}
		assert (false);
		return 0;
	}

	//--------------------------------------------------------------------------
	cPosition getRandomAlienPosition (cModel& model)
	{
		// alien factory is big,
		// surrounded by alien units
		return {
			static_cast<int> (2 + model.randomGenerator.get (model.getMap()->getSize().x() - 3)),
			static_cast<int> (2 + model.randomGenerator.get (model.getMap()->getSize().y() - 3))
		};
	}

	//--------------------------------------------------------------------------
	std::vector<cPosition> toPositions (const std::vector<std::shared_ptr<cPlayer>>& players)
	{
		return ranges::Transform (players, [](const auto& player) { return player->getLandingPos(); });
	}

	//--------------------------------------------------------------------------
	std::size_t minSquaredDistance (const std::vector<cPosition>& positions, const cPosition& ref)
	{
		auto res = (positions[0] - ref).l2NormSquared();

		for (const auto& position : positions)
		{
			res = std::min (res, (position - ref).l2NormSquared());
		}
		return res;
	}

	//--------------------------------------------------------------------------
	std::vector<cPosition> computeAlienLandingPosition (cModel& model)
	{
		const sID alienFactoryId = model.getUnitsData()->getAlienFactoryID();
		const std::size_t threshold = 5 * 5; // square distance between alien factory and existing buildings
		const std::size_t expectedAlienPositionCount = 2 * model.getPlayerList().size();
		auto& map = *model.getMap()->staticMap;
		const auto mapSize = map.getSize();
		const cStaticUnitData& alienFactoryData = model.getUnitsData()->getStaticUnitData (alienFactoryId);
		std::vector<cPosition> alienPositions;

		std::size_t maxTryCount = 3 * expectedAlienPositionCount; // ensure the loop is finite
		while (alienPositions.size() != expectedAlienPositionCount && --maxTryCount)
		{
			cPosition candidate = getRandomAlienPosition (model);

			if (!map.possiblePlace (alienFactoryData, candidate)) continue;
			if (!alienPositions.empty() && minSquaredDistance (alienPositions, candidate) < threshold) continue;
			if (minSquaredDistance (toPositions (model.getPlayerList()), candidate) < threshold) continue;

			alienPositions.push_back (candidate);
		}
		return alienPositions;
	}

	//--------------------------------------------------------------------------
	void landAlien (cModel& model, const cPosition& landingPos)
	{
		const sID alienFactoryId = model.getUnitsData()->getAlienFactoryID();
		model.addBuilding (landingPos, alienFactoryId, nullptr);
		const auto& map = *model.getMap()->staticMap;

		cPosition offsets[] = {cPosition (0, -1), cPosition (1, -1), cPosition (0, 2), cPosition (1, 2)};

		for (const auto& offset : offsets)
		{
			const auto position = landingPos + offset;

			if (map.isWater (position))
			{
				model.addVehicle (position, alienShipId, nullptr);
			}
			else if (!map.isBlocked (position))
			{
				sID alienUnitId = alienTankId;
				switch (model.randomGenerator.get (6))
				{
					case 0: alienUnitId = alienPlaneId; break;
					case 1:
					case 2: alienUnitId = alienAssaultId; break;
					default: alienUnitId = alienTankId; break;
				}
				model.addVehicle (position, alienUnitId, nullptr);
			}
		}
	}

	//--------------------------------------------------------------------------
	void addAliens (cModel& model)
	{
		std::vector<cPosition> alienPositions = computeAlienLandingPosition (model);

		for (const auto& position : alienPositions)
		{
			landAlien (model, position);
		}
	}
}

//------------------------------------------------------------------------------
void cMap::placeInitialResources (cModel& model)
{
	const auto& playerList = model.getPlayerList();
	const auto& gameSettings = *model.getGameSettings();

	Resources.fill (sResources());

	std::vector<eResourceType> resSpotTypes (playerList.size(), eResourceType::Metal);
	std::vector<T_2<int>> resSpots;
	for (const auto& player : playerList)
	{
		const auto& position = player->getLandingPos();
		resSpots.push_back (T_2<int> ((position.x() & ~1) + 1, position.y() & ~1));
	}

	const eGameSettingsResourceDensity density = gameSettings.resourceDensity;
	std::map<eResourceType, eGameSettingsResourceAmount> frequencies;

	frequencies[eResourceType::Metal] = gameSettings.metalAmount;
	frequencies[eResourceType::Oil] = gameSettings.oilAmount;
	frequencies[eResourceType::Gold] = gameSettings.goldAmount;

	const std::size_t resSpotCount = (std::size_t) (getSize().x() * getSize().y() * 0.003f * (1.5f + getResourceDensityFactor (density)));
	const std::size_t playerCount = playerList.size();
	// create remaining resource positions
	while (resSpots.size() < resSpotCount)
	{
		T_2<int> pos;

		pos.x = 2 + model.randomGenerator.get (getSize().x() - 4);
		pos.y = 2 + model.randomGenerator.get (getSize().y() - 4);
		resSpots.push_back (pos);
	}
	resSpotTypes.resize (resSpotCount);
	// Resourcen gleichmässiger verteilen
	for (std::size_t j = 0; j < 3; j++)
	{
		for (std::size_t i = playerCount; i < resSpotCount; i++)
		{
			T_2<float> d;
			for (std::size_t j = 0; j < resSpotCount; j++)
			{
				if (i == j) continue;

				int diffx1 = resSpots[i].x - resSpots[j].x;
				int diffx2 = diffx1 + (getSize().x() - 4);
				int diffx3 = diffx1 - (getSize().x() - 4);
				int diffy1 = resSpots[i].y - resSpots[j].y;
				int diffy2 = diffy1 + (getSize().y() - 4);
				int diffy3 = diffy1 - (getSize().y() - 4);
				if (abs (diffx2) < abs (diffx1)) diffx1 = diffx2;
				if (abs (diffx3) < abs (diffx1)) diffx1 = diffx3;
				if (abs (diffy2) < abs (diffy1)) diffy1 = diffy2;
				if (abs (diffy3) < abs (diffy1)) diffy1 = diffy3;
				T_2<float> diff (diffx1, diffy1);
				if (diff == T_2<float>::Zero)
				{
					diff.x += 1;
				}
				const float dist = diff.dist();
				d += diff * (10.f / (dist * dist));

			}
			resSpots[i] += T_2<int> (Round (d.x), Round (d.y));
			if (resSpots[i].x < 2) resSpots[i].x += getSize().x() - 4;
			if (resSpots[i].y < 2) resSpots[i].y += getSize().y() - 4;
			if (resSpots[i].x > getSize().x() - 3) resSpots[i].x -= getSize().x() - 4;
			if (resSpots[i].y > getSize().y() - 3) resSpots[i].y -= getSize().y() - 4;
		}
	}
	// Resourcen Typ bestimmen
	for (std::size_t i = playerCount; i < resSpotCount; i++)
	{
		std::map<eResourceType, double> amount;
		for (std::size_t j = 0; j < i; j++)
		{
			const float maxDist = 40.f;
			float dist = sqrtf (resSpots[i].distSqr (resSpots[j]));
			if (dist < maxDist) amount[resSpotTypes[j]] += 1 - sqrtf (dist / maxDist);
		}

		amount[eResourceType::Metal] /= 1.0f;
		amount[eResourceType::Oil] /= 0.8f;
		amount[eResourceType::Gold] /= 0.4f;

		eResourceType type = eResourceType::Metal;
		if (amount[eResourceType::Oil] < amount[type]) type = eResourceType::Oil;
		if (amount[eResourceType::Gold] < amount[type]) type = eResourceType::Gold;

		resSpots[i].x &= ~1;
		resSpots[i].y &= ~1;
		resSpots[i].x += static_cast<int> (type) % 2;
		resSpots[i].y += (static_cast<int> (type) / 2) % 2;

		resSpotTypes[i] = static_cast<eResourceType> (((resSpots[i].y % 2) * 2) + (resSpots[i].x % 2));
	}
	// Resourcen platzieren
	for (std::size_t i = 0; i < resSpotCount; i++)
	{
		T_2<int> pos = resSpots[i];
		T_2<int> p;
		bool hasGold = model.randomGenerator.get (100) < 40;
		const int minx = std::max (pos.x - 1, 0);
		const int maxx = std::min (pos.x + 1, getSize().x() - 1);
		const int miny = std::max (pos.y - 1, 0);
		const int maxy = std::min (pos.y + 1, getSize().y() - 1);
		for (p.y = miny; p.y <= maxy; ++p.y)
		{
			for (p.x = minx; p.x <= maxx; ++p.x)
			{
				T_2<int> absPos = p;
				eResourceType type = static_cast<eResourceType> ((absPos.y % 2) * 2 + (absPos.x % 2));

				int index = getOffset (cPosition (absPos.x, absPos.y));
				if (type != eResourceType::None &&
					((hasGold && i >= playerCount) || resSpotTypes[i] == eResourceType::Gold || type != eResourceType::Gold) &&
					!isBlocked (cPosition (absPos.x, absPos.y)))
				{
					sResources res;
					res.typ = type;
					if (i >= playerCount)
					{
						res.value = 1 + model.randomGenerator.get (2 + getResourceAmountFactor (frequencies[type]) * 2);
						if (p == pos) res.value += 3 + model.randomGenerator.get (4 + getResourceAmountFactor (frequencies[type]) * 2);
					}
					else
					{
						res.value = 1 + 4 + getResourceAmountFactor (frequencies[type]);
						if (p == pos) res.value += 3 + 2 + getResourceAmountFactor (frequencies[type]);
					}
					res.value = std::min<unsigned char> (16, res.value);
					Resources.set (index, res);
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
cActionInitNewGame::cActionInitNewGame (sInitPlayerData initPlayerData) :
	initPlayerData (std::move (initPlayerData))
{}

//------------------------------------------------------------------------------
cActionInitNewGame::cActionInitNewGame (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionInitNewGame::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	model.initGameId();

	cPlayer& player = *model.getPlayer (playerNr);
	const cUnitsData& unitsdata = *model.getUnitsData();

	player.removeAllUnits();
	Log.write (" GameId: " + std::to_string (model.getGameId()), cLog::eLOG_TYPE_NET_DEBUG);

	// init clan
	if (model.getGameSettings()->clansEnabled)
	{
		if (initPlayerData.clan < 0 || static_cast<size_t> (initPlayerData.clan) >= unitsdata.getNrOfClans())
		{
			Log.write (" Landing failed. Invalid clan number.", cLog::eLOG_TYPE_NET_ERROR);
			return;
		}
		player.setClan (initPlayerData.clan, unitsdata);
	}
	else
	{
		player.setClan (-1, unitsdata);
	}

	// init landing position
	if (!model.getMap()->isValidPosition (initPlayerData.landingPosition))
	{
		Log.write (" Received invalid landing position", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	cPosition updatedLandingPosition = initPlayerData.landingPosition;
	if (model.getGameSettings()->bridgeheadType == eGameSettingsBridgeheadType::Definite)
	{
		// Find place for mine if bridgehead is fixed
		if (!findPositionForStartMine (updatedLandingPosition, *model.getUnitsData(), *model.getMap()->staticMap))
		{
			Log.write ("couldn't place player start mine: " + player.getName(), cLog::eLOG_TYPE_NET_ERROR);
			return;
		}
	}
	player.setLandingPos (updatedLandingPosition);

	const bool allPlayerReady = ranges::find_if (model.getPlayerList(), [](const auto& player){ return player->getLandingPos() == cPosition {-1, -1}; }) == model.getPlayerList().end();

	if (allPlayerReady)
	{
		model.getMap()->placeInitialResources (model);
		if (model.getGameSettings()->alienEnabled) addAliens (model);
	}

	// apply upgrades
	int credits = model.getGameSettings()->startCredits;
	for (const auto& upgrade : initPlayerData.unitUpgrades)
	{
		const sID& unitId = upgrade.first;
		const cUnitUpgrade& upgradeValues = upgrade.second;

		if (!unitsdata.isValidId (unitId))
		{
			Log.write (" Apply upgrades failed. Unknown sID: " + unitId.getText(), cLog::eLOG_TYPE_NET_ERROR);
			return;
		}
		int costs = upgradeValues.calcTotalCosts (unitsdata.getDynamicUnitData (unitId, player.getClan()), *player.getUnitDataCurrentVersion (unitId), player.getResearchState());
		if (costs <= 0)
		{
			Log.write (" Apply upgrades failed. Couldn't calculate costs.", cLog::eLOG_TYPE_NET_ERROR);
			return;
		}
		credits -= costs;
		if (credits <= 0)
		{
			Log.write (" Apply upgrade failed. Used more than the available credits.", cLog::eLOG_TYPE_NET_ERROR);
			return;
		}
		upgradeValues.updateUnitData (*player.getUnitDataCurrentVersion (unitId));
	}

	// land vehicles
	auto initialLandingUnits = computeInitialLandingUnits (initPlayerData.clan, *model.getGameSettings(), unitsdata);
	for (const auto& landing : initPlayerData.landingUnits)
	{
		if (!unitsdata.isValidId (landing.unitID))
		{
			Log.write (" Landing failed. Unknown sID: " + landing.unitID.getText(), cLog::eLOG_TYPE_NET_ERROR);
			return;
		}

		auto it = ranges::find_if (initialLandingUnits, [landing](std::pair<sID, int> unit){ return unit.first == landing.unitID; });
		if (it != initialLandingUnits.end())
		{
			// landing unit is one of the initial landing units, that the player gets for free
			// so don't pay for the unit and the default cargo
			credits -= (landing.cargo - it->second) / 5;
			initialLandingUnits.erase (it);
		}
		else
		{
			credits -= landing.cargo / 5;
			credits -= unitsdata.getDynamicUnitData (landing.unitID, initPlayerData.clan).getBuildCost();
		}
	}
	if (credits < 0)
	{
		Log.write (" Landing failed. Used more than the available credits", cLog::eLOG_TYPE_ERROR);
		return;
	}
	makeLanding (player, initPlayerData.landingUnits, model);

	//transfer remaining credits to player
	player.setCredits (credits);
}

//------------------------------------------------------------------------------
bool cActionInitNewGame::isValidLandingPosition (cPosition position, const cStaticMap& map, bool fixedBridgeHead, const std::vector<sLandingUnit>& units, const cUnitsData& unitsData)
{
	std::vector<cPosition> blockedPositions;
	if (fixedBridgeHead)
	{
		bool found = findPositionForStartMine (position, unitsData, map);
		if (!found)
			return false;

		//small generator
		blockedPositions.push_back (position + cPosition (-1, 0));
		//mine
		blockedPositions.push_back (position + cPosition (0, -1));
		blockedPositions.push_back (position + cPosition (1, -1));
		blockedPositions.push_back (position + cPosition (1,  0));
		blockedPositions.push_back (position + cPosition (0,  0));
	}

	for (const auto& unit : units)
	{
		const cStaticUnitData& unitData = unitsData.getStaticUnitData (unit.unitID);
		bool placed = false;
		int landingRadius = 1;

		while (!placed)
		{
			landingRadius += 1;

			// prevent, that units are placed far away from the starting position
			if (landingRadius > 1.5 * sqrt (units.size()) && landingRadius > 3) return false;

			for (int offY = -landingRadius; (offY < landingRadius) && !placed; ++offY)
			{
				for (int offX = -landingRadius; (offX < landingRadius) && !placed; ++offX)
				{
					const cPosition place = position + cPosition (offX, offY);
					if (map.possiblePlace (unitData, place) && !Contains (blockedPositions, place))
					{
						blockedPositions.push_back (place);
						placed = true;
					}
				}
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------------
void cActionInitNewGame::makeLanding (cPlayer& player, const std::vector<sLandingUnit>& landingUnits, cModel& model) const
{
	cPosition landingPosition = player.getLandingPos();

	if (model.getGameSettings()->bridgeheadType == eGameSettingsBridgeheadType::Definite)
	{
		// place buildings:
		model.addBuilding (landingPosition + cPosition (-1, 0), model.getUnitsData()->getSmallGeneratorID(), &player);
		model.addBuilding (landingPosition + cPosition (0, -1), model.getUnitsData()->getMineID(), &player);
	}

	for (size_t i = 0; i != landingUnits.size(); ++i)
	{
		const sLandingUnit& landing = landingUnits[i];
		cVehicle* vehicle = nullptr;
		int radius = 1;

		if (!model.getUnitsData()->isValidId (landing.unitID))
		{
			Log.write (" Landing of unit failed. Unknown sID: " + landing.unitID.getText(), cLog::eLOG_TYPE_NET_ERROR);
			continue;
		}

		while (!vehicle)
		{
			vehicle = landVehicle (landingPosition, radius, landing.unitID, player, model);
			radius += 1;
		}
		if (landing.cargo && vehicle)
		{
			if (vehicle->getStaticUnitData().storeResType != eResourceType::Gold)
				vehicle->setStoredResources (landing.cargo);
		}
	}
}
//------------------------------------------------------------------------------
cVehicle* cActionInitNewGame::landVehicle (const cPosition& landingPosition, int radius, const sID& id, cPlayer& player, cModel& model) const
{
	for (int offY = -radius; offY < radius; ++offY)
	{
		for (int offX = -radius; offX < radius; ++offX)
		{
			if (!model.getMap()->possiblePlaceVehicle (model.getUnitsData()->getStaticUnitData (id), landingPosition + cPosition (offX, offY), &player)) continue;

			return &model.addVehicle (landingPosition + cPosition (offX, offY), id, &player);
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
bool cActionInitNewGame::findPositionForStartMine (cPosition& position, const cUnitsData& unitsData, const cStaticMap& map)
{
	const auto& mine = unitsData.getStaticUnitData (unitsData.getMineID());
	const auto& smallGenerator = unitsData.getStaticUnitData (unitsData.getSmallGeneratorID());

	for (int radius = 0; radius < 3; ++radius)
	{
		for (int offY = -radius; offY <= radius; ++offY)
		{
			for (int offX = -radius; offX <= radius; ++offX)
			{
				const cPosition place = position + cPosition (offX, offY);
				if (map.possiblePlace (smallGenerator, place + cPosition (-1, 0)) &&
					map.possiblePlace (mine, place + cPosition (0, -1)))
				{
					position = place;
					return true;
				}
			}
		}
	}
	return false;
}
