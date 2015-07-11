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

#include "model.h"
#include "player/player.h"
#include "map/map.h"
#include "units/vehicle.h"
#include "units/building.h"
#include "game/logic/movejobs.h"
#include "utility/files.h"

//------------------------------------------------------------------------------
cModel::cModel() :
	nextUnitId(0),
	gameSettings(std::make_shared<cGameSettings>())
{
	/*signalConnectionManager.connect(model.getGameSettings()->turnEndDeadlineChanged, [this]()
	{
		if (turnEndDeadline)
		{
			turnEndDeadline->changeDeadline(model.getGameSettings()->getTurnEndDeadline());
		}
	});

	signalConnectionManager.connect(model.getGameSettings()->turnEndDeadlineActiveChanged, [this]()
	{
		if (!model.getGameSettings()->isTurnEndDeadlineActive() && turnEndDeadline)
		{
			turnTimeClock->removeDeadline(turnEndDeadline);
			turnEndDeadline = nullptr;
		}
	});

	signalConnectionManager.connect(model.getGameSettings()->turnLimitChanged, [this]()
	{
		if (turnLimitDeadline)
		{
			turnLimitDeadline->changeDeadline(model.getGameSettings()->getTurnLimit());
		}
	});

	signalConnectionManager.connect(model.getGameSettings()->turnLimitActiveChanged, [this]()
	{
		if (!model.getGameSettings()->isTurnLimitActive() && turnLimitDeadline)
		{
			turnTimeClock->removeDeadline(turnLimitDeadline);
			turnLimitDeadline = nullptr;
		}
		else if (model.getGameSettings()->isTurnLimitActive() && !turnLimitDeadline)
		{
			turnLimitDeadline = turnTimeClock->startNewDeadlineFrom(turnTimeClock->getStartGameTime(), model.getGameSettings()->getTurnLimit());
		}
	}); */
};
//------------------------------------------------------------------------------
void cModel::runJobs(const cGameTimer& timer)
{
	//TODO
}
//------------------------------------------------------------------------------
uint32_t cModel::calcChecksum() const
{
	uint32_t crc = 0;
	//for (auto : map->ress)
	//crc = calcChecksum(map->resourcesToString)

	return crc;
}
//------------------------------------------------------------------------------
void cModel::setGameSettings(const cGameSettings& gameSettings_)
{
	*gameSettings = gameSettings_;
}
//------------------------------------------------------------------------------
void cModel::setMap(std::shared_ptr<cStaticMap> map_)
{
	map = std::make_shared<cMap>(map_);
	for (auto player : playerList)
		player->initMaps(*map);
}
//------------------------------------------------------------------------------
cPlayer* cModel::getPlayer(int playerNr)
{
	for (auto player : playerList)
	{
		if (player->getNr() == playerNr)
			return player.get();
	}
	return nullptr;
}
//------------------------------------------------------------------------------
const cPlayer* cModel::getPlayer(int playerNr) const
{
	//TODO: remove code duplication
	for (auto player : playerList)
	{
		if (player->getNr() == playerNr)
			return player.get();
	}
	return nullptr;
}
//------------------------------------------------------------------------------
const cPlayer* cModel::getPlayer(std::string player) const
{
	// first try to find player by number
	const int playerNr = atoi(player.c_str());
	if (playerNr != 0 || player[0] == '0')
	{
		return getPlayer(playerNr);
	}

	// try to find player by name
	for (auto p : playerList)
	{
		if (p->getName() == player)
			return p.get();
	}
	return nullptr;
}
//------------------------------------------------------------------------------
void cModel::setPlayerList(const std::vector<cPlayerBasicData>& splayers)
{
	assert(playerList.size() == 0);

	for (auto playerInfo : splayers)
	{
		auto player = std::make_shared<cPlayer>(playerInfo);
		if (map) player->initMaps(*map);
		playerList.push_back(player);

	}
}
//------------------------------------------------------------------------------
cVehicle& cModel::addVehicle(const cPosition& position, const sID& id, cPlayer* player, bool init, bool addToMap)
{
	// generate the vehicle:
	cVehicle& addedVehicle = player->addNewVehicle(position, id, nextUnitId);
	nextUnitId++;

	// place the vehicle:
	if (addToMap) map->addVehicle(addedVehicle, position);

	// scan with surveyor:
	if (addedVehicle.data.canSurvey)
	{
		addedVehicle.doSurvey(*map);
	}

	if (addedVehicle.canLand(*map))
	{
		addedVehicle.setFlightHeight(0);
	}
	else
	{
		addedVehicle.setFlightHeight(64);
	}

	//TODO: detection
	return addedVehicle;
}
//------------------------------------------------------------------------------
cBuilding& cModel::addBuilding(const cPosition& position, const sID& id, cPlayer* player, bool init)
{
	// generate the building:
	cBuilding& addedBuilding = player->addNewBuilding(position, id, nextUnitId);
	nextUnitId++;

	if (addedBuilding.data.canMineMaxRes > 0) addedBuilding.checkRessourceProd(*map);
	
	cBuilding* buildingToBeDeleted = map->getField(position).getTopBuilding();

	map->addBuilding(addedBuilding, position);

	// integrate the building to the base:
	player->base.addBuilding(&addedBuilding);

	// if this is a top building, delete connectors, mines and roads
	if (addedBuilding.data.surfacePosition == sUnitData::SURFACE_POS_GROUND)
	{
		if (addedBuilding.data.isBig)
		{
			auto bigPosition = position;
			auto buildings = &map->getField(bigPosition).getBuildings();

			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ((*buildings)[i]->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit((*buildings)[i]);
					--i;
				}
			}
			bigPosition.x()++;
			buildings = &map->getField(bigPosition).getBuildings();
			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ((*buildings)[i]->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit((*buildings)[i]);
					--i;
				}
			}
			bigPosition.y()++;
			buildings = &map->getField(bigPosition).getBuildings();
			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ((*buildings)[i]->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit((*buildings)[i]);
					--i;
				}
			}
			bigPosition.x()--;
			buildings = &map->getField(bigPosition).getBuildings();
			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ((*buildings)[i]->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit((*buildings)[i]);
					--i;
				}
			}
		}
		else
		{
			deleteUnit(buildingToBeDeleted);

			const auto& buildings = map->getField(position).getBuildings();
			for (size_t i = 0; i != buildings.size(); ++i)
			{
				if (buildings[i]->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit(buildings[i]);
					--i;
				}
			}
		}
	}

	if (addedBuilding.data.canMineMaxRes > 0)
	{
		//TODO: start work
	}
	//TODO: detection
	return addedBuilding;
}
//------------------------------------------------------------------------------
void cModel::deleteUnit(cUnit* unit)
{
	if (unit == 0)
		return;

	if (unit->isABuilding() && unit->getOwner() == 0)
	{
		deleteRubble(static_cast<cBuilding*> (unit));
		return;
	}

	cPlayer* owner = unit->getOwner();

	//TODO
	//if (unit->getOwner() && casualtiesTracker != nullptr && ((unit->isABuilding() && unit->data.buildCosts <= 2) == false))
	//	casualtiesTracker->logCasualty(unit->data.ID, unit->getOwner()->getNr());

	std::shared_ptr<cUnit> owningPtr; // keep owning ptr to make sure that unit instance will outlive the following method.
	if (unit->isABuilding())
	{
		cBuilding* building = static_cast<cBuilding*> (unit);
		owningPtr = owner->removeUnit(*building);
	}
	else
	{
		cVehicle* vehicle = static_cast<cVehicle*> (unit);
		owningPtr = owner->removeUnit(*vehicle);
	}

	//helperJobs.onRemoveUnit(unit);

	// detach from move job
	if (unit->isAVehicle())
	{
		cVehicle* vehicle = static_cast<cVehicle*> (unit);
		if (vehicle->ServerMoveJob)
		{
			vehicle->ServerMoveJob->Vehicle = nullptr;
		}
	}

	// remove from sentry list
	unit->getOwner()->deleteSentry(*unit);

	// lose eco points
	if (unit->isABuilding() && static_cast<cBuilding*> (unit)->points != 0)
	{
		//unit->getOwner()->setScore(unit->getOwner()->getScore(turnClock->getTurn()) - static_cast<cBuilding*> (unit)->points, turnClock->getTurn());
		unit->getOwner()->countEcoSpheres();
	}

	if (unit->isABuilding())
		map->deleteBuilding(*static_cast<cBuilding*> (unit));
	else
		map->deleteVehicle(*static_cast<cVehicle*> (unit));

	if (unit->isABuilding() && static_cast<cBuilding*> (unit)->SubBase != nullptr)
		unit->getOwner()->base.deleteBuilding(static_cast<cBuilding*> (unit));

	if (owner != nullptr)
	{
		owner->doScan();
	}
}
//------------------------------------------------------------------------------
void cModel::deleteRubble(cBuilding* rubble)
{
	map->deleteBuilding(*rubble);

	auto iter = neutralBuildings.find(*rubble);
	assert(iter != neutralBuildings.end());

	if (iter != neutralBuildings.end())
	{
		neutralBuildings.erase(iter);
	}
}
