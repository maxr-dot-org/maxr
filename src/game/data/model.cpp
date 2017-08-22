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
#include "game/logic/movejob.h"
#include "utility/crc.h"
#include "game/logic/pathcalculator.h"
#include "utility/listhelpers.h"
#include "game/logic/turncounter.h"
#include "game/logic/turntimeclock.h"
#include "game/logic/fxeffects.h"

//------------------------------------------------------------------------------
cModel::cModel() :
	nextUnitId(0),
	gameSettings(std::make_shared<cGameSettings>()),
	unitsData(std::make_shared<cUnitsData>()),
	turnCounter (std::make_shared<cTurnCounter> (1)),
	gameTime(0),
	gameId(0),
	turnEndState(TURN_ACTIVE),
	activeTurnPlayer(nullptr),
	turnEndDeadline(0),
	turnLimitDeadline(0)
{
	turnTimeClock = std::make_shared<cTurnTimeClock>(*this);

	gameSettings->turnEndDeadlineChanged.connect([this]()
	{
		if (turnEndDeadline)
		{
			turnTimeClock->changeDeadline(turnEndDeadline, gameSettings->getTurnEndDeadline());
		}
	});

	gameSettings->turnEndDeadlineActiveChanged.connect([this]()
	{
		if (!gameSettings->isTurnEndDeadlineActive() && turnEndDeadline)
		{
			turnTimeClock->removeDeadline(turnEndDeadline);
			turnEndDeadline = 0;
		}
	});

	gameSettings->turnLimitChanged.connect([this]()
	{
		if (turnLimitDeadline)
		{
			turnTimeClock->changeDeadline(turnLimitDeadline, gameSettings->getTurnLimit());
		}
	});

	gameSettings->turnLimitActiveChanged.connect([this]()
	{
		if (!gameSettings->isTurnLimitActive() && turnLimitDeadline)
		{
			turnTimeClock->removeDeadline(turnLimitDeadline);
			turnLimitDeadline = 0;
		}
		else if (gameSettings->isTurnLimitActive() && !turnLimitDeadline)
		{
			turnLimitDeadline = turnTimeClock->startNewDeadlineFrom(turnTimeClock->getStartGameTime(), gameSettings->getTurnLimit());
		}
	});
};

cModel::~cModel()
{
	if (map != nullptr)
	{
		map->reset();
	}
}

//------------------------------------------------------------------------------
void cModel::initGameId()
{
	if (gameId == 0)
	{
		while ((gameId = randomGenerator.get()) == 0);
	}
}

//------------------------------------------------------------------------------
void cModel::advanceGameTime()
{
	gameTime++;
	gameTimeChanged();

	runMoveJobs();
	effectsList.run();
	handleTurnEnd();
}

//------------------------------------------------------------------------------
unsigned int cModel::getGameTime() const
{
	return gameTime;
}

//------------------------------------------------------------------------------
uint32_t cModel::getChecksum() const
{
	uint32_t crc = 0;
	//crc = calcCheckSum(gameTime, crc);
	crc = calcCheckSum(gameId, crc);
	crc = calcCheckSum(*gameSettings, crc);
	crc = calcCheckSum(*map, crc);
	for (const auto& player : playerList)
		crc = calcCheckSum(*player, crc);
	for (const auto& building : neutralBuildings)
		crc = calcCheckSum(*building, crc);
	crc = calcCheckSum(nextUnitId, crc);
	crc = calcCheckSum(*unitsData, crc);
	for (const auto& movejob : moveJobs)
		crc = calcCheckSum(*movejob, crc);
	crc = calcCheckSum(*turnCounter, crc);
	crc = calcCheckSum(turnEndState, crc);
	crc = calcCheckSum(activeTurnPlayer->getId(), crc);
	crc = calcCheckSum(turnEndDeadline, crc);
	crc = calcCheckSum(turnLimitDeadline, crc);
	crc = calcCheckSum(*turnTimeClock, crc);

	return crc;
}
//------------------------------------------------------------------------------
void cModel::setGameSettings(const cGameSettings& gameSettings_)
{
	*gameSettings = gameSettings_;

	if (gameSettings->isTurnLimitActive())
	{
		turnLimitDeadline = turnTimeClock->startNewDeadlineFromNow(gameSettings->getTurnLimit());
	}
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
		if (player->getId() == playerNr)
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
		if (player->getId() == playerNr)
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
		auto player = std::make_shared<cPlayer>(playerInfo, *unitsData);
		if (map) player->initMaps(*map);
		playerList.push_back(player);

	}
	activeTurnPlayer = playerList[0].get();;
}

//------------------------------------------------------------------------------
const cPlayer* cModel::getActiveTurnPlayer() const
{
	return activeTurnPlayer;
}

//------------------------------------------------------------------------------
cVehicle& cModel::addVehicle(const cPosition& position, const sID& id, cPlayer* player, bool init, bool addToMap)
{
	// generate the vehicle:
	cVehicle& addedVehicle = player->addNewVehicle(position, unitsData->getStaticUnitData(id), nextUnitId);
	nextUnitId++;

	// place the vehicle:
	if (addToMap) map->addVehicle(addedVehicle, position);

	// scan with surveyor:
	if (addedVehicle.getStaticUnitData().canSurvey)
	{
		addedVehicle.doSurvey();
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
	cBuilding& addedBuilding = player->addNewBuilding(position, unitsData->getStaticUnitData(id), nextUnitId);
	nextUnitId++;

	addedBuilding.initMineRessourceProd(*map);
	
	cBuilding* buildingToBeDeleted = map->getField(position).getTopBuilding();

	map->addBuilding(addedBuilding, position);

	// integrate the building to the base:
	player->base.addBuilding(&addedBuilding, *map);

	// if this is a top building, delete connectors, mines and roads
	if (addedBuilding.getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_GROUND)
	{
		if (addedBuilding.getIsBig())
		{
			auto bigPosition = position;
			auto buildings = &map->getField(bigPosition).getBuildings();

			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ((*buildings)[i]->getStaticUnitData().canBeOverbuild == cStaticUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit((*buildings)[i]);
					--i;
				}
			}
			bigPosition.x()++;
			buildings = &map->getField(bigPosition).getBuildings();
			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ((*buildings)[i]->getStaticUnitData().canBeOverbuild == cStaticUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit((*buildings)[i]);
					--i;
				}
			}
			bigPosition.y()++;
			buildings = &map->getField(bigPosition).getBuildings();
			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ((*buildings)[i]->getStaticUnitData().canBeOverbuild == cStaticUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit((*buildings)[i]);
					--i;
				}
			}
			bigPosition.x()--;
			buildings = &map->getField(bigPosition).getBuildings();
			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ((*buildings)[i]->getStaticUnitData().canBeOverbuild == cStaticUnitData::OVERBUILD_TYPE_YESNREMOVE)
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
				if (buildings[i]->getStaticUnitData().canBeOverbuild == cStaticUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit(buildings[i]);
					--i;
				}
			}
		}
	}

	if (addedBuilding.getStaticUnitData().canMineMaxRes > 0)
	{
		addedBuilding.startWork();
	}
	//TODO: detection
	return addedBuilding;
}

//------------------------------------------------------------------------------
void cModel::destroyUnit(cUnit& unit)
{
	bool isVehicle = false;
	const auto position = unit.getPosition();
	auto& field = map->getField(position);

	//delete planes immediately
	if (unit.isAVehicle())
	{
		isVehicle = true;
		auto& vehicle = static_cast<cVehicle&> (unit);
		if (vehicle.getStaticUnitData().factorAir > 0 && vehicle.getFlightHeight() > 0)
		{
			deleteUnit(&vehicle);
			return;
		}
	}

	//check, if there is a big unit on the field
	bool bigUnit = false;
	auto topBuilding = field.getTopBuilding();
	if ((topBuilding && topBuilding->getIsBig()) || unit.getIsBig())
		bigUnit = true;

	if (unit.isAVehicle())
	{
		addDestroyFx(static_cast<cVehicle&> (unit));
	}
	else
	{
		addDestroyFx(static_cast<cBuilding&> (unit));
	}

	//delete unit
	int rubbleValue = 0;
	if (!unit.getStaticUnitData().isHuman)
	{
		rubbleValue += unit.data.getBuildCost();
		// stored material is always added completely to the rubble
		if (unit.getStaticUnitData().storeResType == eResourceType::Metal)
			rubbleValue += unit.getStoredResources() * 2;
	}
	deleteUnit(&unit);

	//delete all buildings (except connectors, when a vehicle is destroyed)
	rubbleValue += deleteBuildings(field, !isVehicle);
	if (bigUnit)
	{
		rubbleValue += deleteBuildings(map->getField(position + cPosition(1, 0)), !isVehicle);
		rubbleValue += deleteBuildings(map->getField(position + cPosition(0, 1)), !isVehicle);
		rubbleValue += deleteBuildings(map->getField(position + cPosition(1, 1)), !isVehicle);
	}

	//add rubble
	auto rubble = field.getRubble();
	if (rubble)
	{
		rubble->setRubbleValue(rubble->getRubbleValue() + rubbleValue / 2, randomGenerator);
	}
	else
	{
		if (rubbleValue > 2)
			addRubble(position, rubbleValue / 2, bigUnit);
	}
}

//------------------------------------------------------------------------------
void cModel::addDestroyFx(const cVehicle& vehicle)
{
	if (vehicle.getIsBig())
	{
		addFx(std::make_shared<cFxExploBig>(vehicle.getPosition() * 64 + 64, map->isWaterOrCoast(vehicle.getPosition())));
	}
	else if (vehicle.getStaticUnitData().factorAir > 0 && vehicle.getFlightHeight() != 0)
	{
		addFx(std::make_shared<cFxExploAir>(vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32));
	}
	else if (map->isWaterOrCoast(vehicle.getPosition()))
	{
		addFx(std::make_shared<cFxExploWater>(vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32));
	}
	else
	{
		addFx(std::make_shared<cFxExploSmall>(vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32));
	}

	if (vehicle.uiData->hasCorpse)
	{
		// add corpse
		addFx(std::make_shared<cFxCorpse>(vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32));
	}
}

//------------------------------------------------------------------------------
void cModel::addDestroyFx(const cBuilding& building)
{
	const cBuilding* topBuilding = map->getField(building.getPosition()).getBuilding();
	if (topBuilding && topBuilding->getIsBig())
	{
		addFx(std::make_shared<cFxExploBig>(topBuilding->getPosition() * 64 + 64, map->isWaterOrCoast(topBuilding->getPosition())));
	}
	else
	{
		addFx(std::make_shared<cFxExploSmall>(building.getPosition() * 64 + 32));
	}
}

//------------------------------------------------------------------------------
int cModel::deleteBuildings(cMapField& field, bool deleteConnector)
{
	auto buildings = field.getBuildings();
	int rubble = 0;

	for (auto b_it = buildings.begin(); b_it != buildings.end(); ++b_it)
	{
		if ((*b_it)->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE && deleteConnector == false) continue;
		if ((*b_it)->isRubble()) continue;

		if ((*b_it)->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE) //no rubble for connectors
			rubble += (*b_it)->data.getBuildCost();
		if ((*b_it)->getStaticUnitData().storeResType == eResourceType::Metal)
			rubble += (*b_it)->getStoredResources() * 2; // stored material is always added completely to the rubble

		deleteUnit(*b_it);
	}

	return rubble;
}

//------------------------------------------------------------------------------
void cModel::addRubble(const cPosition& position, int value, bool big)
{
	value = std::max(1, value);

	if (map->isWaterOrCoast(position))
	{
		if (big)
		{
			addRubble(position + cPosition(1, 0), value / 4, false);
			addRubble(position + cPosition(0, 1), value / 4, false);
			addRubble(position + cPosition(1, 1), value / 4, false);
		}
		return;
	}

	if (big && map->isWaterOrCoast(position + cPosition(1, 0)))
	{
		addRubble(position, value / 4, false);
		addRubble(position + cPosition(0, 1), value / 4, false);
		addRubble(position + cPosition(1, 1), value / 4, false);
		return;
	}

	if (big && map->isWaterOrCoast(position + cPosition(0, 1)))
	{
		addRubble(position, value / 4, false);
		addRubble(position + cPosition(1, 0), value / 4, false);
		addRubble(position + cPosition(1, 1), value / 4, false);
		return;
	}

	if (big && map->isWaterOrCoast(position + cPosition(1, 1)))
	{
		addRubble(position, value / 4, false);
		addRubble(position + cPosition(1, 0), value / 4, false);
		addRubble(position + cPosition(0, 1), value / 4, false);
		return;
	}

	std::shared_ptr<cBuilding> rubble;
	if (big)
	{
		rubble = std::make_shared<cBuilding>(&unitsData->getRubbleBigData(), nullptr, nullptr, nextUnitId);
	}
	else
	{
		rubble = std::make_shared<cBuilding>(&unitsData->getRubbleSmallData(), nullptr, nullptr, nextUnitId);
	}

	nextUnitId++;

	rubble->setPosition(position);

	rubble->setRubbleValue(value, randomGenerator);

	map->addBuilding(*rubble, position);


	neutralBuildings.insert(std::move(rubble));
}

//------------------------------------------------------------------------------
void cModel::deleteUnit(cUnit* unit)
{
	if (unit == 0)
		return;

	if (unit->isABuilding() && static_cast<cBuilding*>(unit)->isRubble())
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
		if (vehicle->getMoveJob())
		{
			vehicle->getMoveJob()->removeVehicle(vehicle);
		}
	}

	// remove from sentry list
	owner->deleteSentry(*unit);

	// lose eco points
	if (unit->isABuilding() && static_cast<cBuilding*> (unit)->points != 0)
	{
		//unit->getOwner()->setScore(unit->getOwner()->getScore(turnClock->getTurn()) - static_cast<cBuilding*> (unit)->points, turnClock->getTurn());
		owner->countEcoSpheres();
	}

	if (unit->isABuilding())
		map->deleteBuilding(*static_cast<cBuilding*> (unit));
	else
		map->deleteVehicle(*static_cast<cVehicle*> (unit));

	if (unit->isABuilding() && static_cast<cBuilding*> (unit)->subBase != nullptr)
		owner->base.deleteBuilding(static_cast<cBuilding*> (unit), *map);

	if (owner != nullptr)
	{
		owner->doScan();
	}
}
//------------------------------------------------------------------------------
void cModel::deleteRubble(cBuilding* rubble)
{
	assert(rubble->isRubble());

	map->deleteBuilding(*rubble);

	auto iter = neutralBuildings.find(*rubble);
	assert(iter != neutralBuildings.end());

	if (iter != neutralBuildings.end())
	{
		neutralBuildings.erase(iter);
	}
}

//------------------------------------------------------------------------------
void cModel::addMoveJob(cVehicle& vehicle, const std::forward_list<cPosition>& path)
{
	cMoveJob* currentMoveJob = vehicle.getMoveJob();
	if (currentMoveJob)
	{
		if (currentMoveJob->isActive())
		{
			// cannot add movejob while the unit is already moving
			return;
		}
		else
		{
			// a waiting movejob can be replaced by new one
			currentMoveJob->stop();
		}
	}

	cMoveJob* moveJob = new cMoveJob(path, vehicle, *this);
	vehicle.setMoveJob(moveJob);

	moveJobs.push_back(moveJob);
}

//------------------------------------------------------------------------------
std::vector<const cPlayer*> cModel::resumeMoveJobs(const cPlayer* player /*= nullptr*/)
{
	std::vector<const cPlayer*> players;
	for (const auto& moveJob : moveJobs)
	{
		if ((player && moveJob->getVehicle()->getOwner() != player) || !moveJob->getVehicle())
		{
			continue;
		}
		if (moveJob->isWaiting() && moveJob->getVehicle() && moveJob->getVehicle()->data.getSpeed() > 0)
		{
			moveJob->resume();
			players.push_back(moveJob->getVehicle()->getOwner());
		}
	}
	RemoveDuplicates(players);

	return players;
}

//------------------------------------------------------------------------------
void cModel::handlePlayerFinishedTurn(cPlayer& player)
{
	player.setHasFinishedTurn(true);

	if (gameSettings->getGameType() != eGameSettingsGameType::Turns && gameSettings->isTurnEndDeadlineActive() && !turnEndDeadline)
	{
		turnEndDeadline = turnTimeClock->startNewDeadlineFromNow(gameSettings->getTurnEndDeadline());
	}

	playerFinishedTurn(player);
}

//------------------------------------------------------------------------------
void cModel::addFx(std::shared_ptr<cFx> fx)
{
	effectsList.push_back(fx);
	addedEffect(fx);
}

//------------------------------------------------------------------------------
std::shared_ptr<const cTurnTimeClock> cModel::getTurnTimeClock() const
{
	return turnTimeClock;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cTurnCounter> cModel::getTurnCounter() const
{
	return turnCounter;
}

//------------------------------------------------------------------------------
cUnit* cModel::getUnitFromID(unsigned int id) const
{
	cUnit* result = getVehicleFromID(id);
	if (result == nullptr)
		result = getBuildingFromID(id);
	return result;
}

//------------------------------------------------------------------------------
cVehicle* cModel::getVehicleFromID(unsigned int id) const
{
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		auto unit = playerList[i]->getVehicleFromId(id);
		if (unit) return unit;
	}
	return 0;
}

//------------------------------------------------------------------------------
cBuilding* cModel::getBuildingFromID(unsigned int id) const
{
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		auto unit = playerList[i]->getBuildingFromId(id);
		if (unit) return unit;
	}

	auto iter = neutralBuildings.find(id);
	return iter == neutralBuildings.end() ? nullptr : iter->get();
}

//------------------------------------------------------------------------------
void cModel::refreshMapPointer()
{
	for (const auto& player : playerList)
	{
		for (const auto& vehicle : player->getVehicles())
		{
			if (!vehicle->isUnitLoaded())
				map->addVehicle(*vehicle, vehicle->getPosition());
		}
		for (const auto& building : player->getBuildings())
		{
			map->addBuilding(*building, building->getPosition());
		}
	}
	for (const auto& building : neutralBuildings)
	{
		map->addBuilding(*building, building->getPosition());
	}
}

//------------------------------------------------------------------------------
void cModel::runMoveJobs()
{
	for (auto& moveJob : moveJobs)
	{
		moveJob->run(*this);
		if (moveJob->isFinished())
		{
			cVehicle* vehicle = moveJob->getVehicle();
			if (vehicle != nullptr && vehicle->getMoveJob() == moveJob)
			{
				vehicle->setMoveJob(nullptr);
			}
			delete moveJob;
			moveJob = nullptr;
		}
	}
	Remove(moveJobs, nullptr);
}

//------------------------------------------------------------------------------
void cModel::handleTurnEnd()
{
	switch (turnEndState)
	{
	case TURN_ACTIVE:
		{
			bool turnFinished = true;
			if (gameSettings->getGameType() == eGameSettingsGameType::Turns)
			{
				if (!activeTurnPlayer->getHasFinishedTurn())
				{
					turnFinished = false;
				}
			}
			else
			{
				for (const auto& player : playerList)
				{
					if (!player->getHasFinishedTurn() && !player->isDefeated)
					{
						turnFinished = false;
					}
				}
			}
			if (turnFinished || turnTimeClock->hasReachedAnyDeadline())
			{
				turnEnded();

				const auto player = gameSettings->getGameType() == eGameSettingsGameType::Turns ? activeTurnPlayer : nullptr;
				const auto resumedMJobOwners = resumeMoveJobs(player);
				for (const auto& player : resumedMJobOwners)
				{
					player->turnEndMovementsStarted();
				}

				turnEndState = EXECUTE_REMAINING_MOVEMENTS;
			}
		}
		break;
	case EXECUTE_REMAINING_MOVEMENTS:
		{
			bool activeMoveJob = false;
			for (const auto& moveJob : moveJobs)
			{
				if (moveJob->isActive())
				{
					activeMoveJob = true;
				}
			}
			if (!activeMoveJob)
			{
				turnEndState = EXECUTE_TURN_START;
			}
		}
		break;
	case EXECUTE_TURN_START:
		{
			if (gameSettings->getGameType() == eGameSettingsGameType::Turns)
			{
				// select next player
				//TODO: skip defeated player?
				auto nextPlayerIter = std::find_if(playerList.begin(), playerList.end(), [this](const std::shared_ptr<cPlayer>& player) {return player.get() == activeTurnPlayer; });
				assert(nextPlayerIter != playerList.end());
				++nextPlayerIter;
				if (nextPlayerIter == playerList.end())
				{
					activeTurnPlayer = playerList.front().get();
				}
				else
				{
					activeTurnPlayer = nextPlayerIter->get();
				}

				if (activeTurnPlayer == playerList.front().get())
				{
					//TODO: checkDefeats();
					turnCounter->increaseTurn();
				}

				if (turnCounter->getTurn() > 1)
				{
					// don't execute turn start action in turn 1, because model is already completely initialized for turn 1
					activeTurnPlayer->makeTurnStart();
				}
			}
			else
			{
				turnCounter->increaseTurn();

				for (auto& player : playerList)
				{
					player->makeTurnStart();
				}

			}

			turnTimeClock->restartFromNow();
			turnTimeClock->clearAllDeadlines();
			turnEndDeadline = 0;
			turnLimitDeadline = 0;
			if (gameSettings->isTurnLimitActive())
			{
				turnLimitDeadline = turnTimeClock->startNewDeadlineFromNow(gameSettings->getTurnLimit());
			}

			turnEndState = TURN_ACTIVE;
			newTurnStarted();
		}
		break;
	}
}

//------------------------------------------------------------------------------
void cModel::setUnitsData(std::shared_ptr<cUnitsData> unitsData_)
{
	unitsData = unitsData_;
}
