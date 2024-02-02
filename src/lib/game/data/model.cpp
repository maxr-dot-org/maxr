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

#include "game/logic/casualtiestracker.h"
#include "game/logic/fxeffects.h"
#include "game/logic/jobs/destroyjob.h"
#include "game/logic/movejob.h"
#include "game/logic/pathcalculator.h"
#include "game/logic/turncounter.h"
#include "game/logic/turntimeclock.h"
#include "map/map.h"
#include "map/mapview.h"
#include "player/player.h"
#include "player/playerbasicdata.h"
#include "units/building.h"
#include "units/vehicle.h"
#include "utility/crc.h"
#include "utility/listhelpers.h"
#include "utility/ranges.h"

#include <cassert>
#include <set>

namespace
{
	//--------------------------------------------------------------------------
	auto byPlayerId (int playerId)
	{
		return [=] (const std::shared_ptr<cPlayer>& player) { return player->getId() == playerId; };
	}
} // namespace

//------------------------------------------------------------------------------
cModel::cModel() :
	gameSettings (std::make_shared<cGameSettings>()),
	unitsData (std::make_shared<cUnitsData>()),
	turnCounter (std::make_shared<cTurnCounter> (1)),
	casualtiesTracker (std::make_shared<cCasualtiesTracker>())
{
	turnTimeClock = std::make_shared<cTurnTimeClock> (*this);
}

//------------------------------------------------------------------------------
cModel::~cModel()
{
}

//------------------------------------------------------------------------------
void cModel::initGameId()
{
	while (gameId == 0)
	{
		gameId = randomGenerator.get();
	}
}

//------------------------------------------------------------------------------
void cModel::advanceGameTime()
{
	gameTime++;
	gameTimeChanged();

	runMoveJobs();
	runAttackJobs();
	effectsList.run();
	handleTurnEnd();
	helperJobs.run (*this);
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
	//crc = calcCheckSum (gameTime, crc);
	crc = calcCheckSum (randomGenerator, crc);
	crc = calcCheckSum (gameId, crc);
	crc = calcCheckSum (*gameSettings, crc);
	crc = calcCheckSum (*map, crc);
	crc = calcCheckSum (playerList, crc);
	crc = calcCheckSum (neutralBuildings, crc);
	crc = calcCheckSum (neutralVehicles, crc);
	crc = calcCheckSum (nextUnitId, crc);
	crc = calcCheckSum (*unitsData, crc);
	crc = calcCheckSum (moveJobs, crc);
	crc = calcCheckSum (attackJobs, crc);
	crc = calcCheckSum (*turnCounter, crc);
	crc = calcCheckSum (turnEndState, crc);
	crc = calcCheckSum (activeTurnPlayer->getId(), crc);
	crc = calcCheckSum (turnEndDeadline, crc);
	crc = calcCheckSum (turnLimitDeadline, crc);
	crc = calcCheckSum (*turnTimeClock, crc);
	crc = calcCheckSum (helperJobs, crc);
	crc = calcCheckSum (*casualtiesTracker, crc);

	return crc;
}
//------------------------------------------------------------------------------
void cModel::setGameSettings (const cGameSettings& gameSettings_)
{
	*gameSettings = gameSettings_;

	if (gameSettings->turnLimitActive)
	{
		turnLimitDeadline = turnTimeClock->startNewDeadlineFromNow (gameSettings->turnLimit);
	}
}
//------------------------------------------------------------------------------
void cModel::setMap (std::shared_ptr<cStaticMap> map_)
{
	map = std::make_shared<cMap> (map_);
	for (auto player : playerList)
		player->initMaps (map->getSize());
}
//------------------------------------------------------------------------------
cPlayer* cModel::getPlayer (int playerNr)
{
	auto it = ranges::find_if (playerList, byPlayerId (playerNr));
	return it == playerList.end() ? nullptr : it->get();
}
//------------------------------------------------------------------------------
const cPlayer* cModel::getPlayer (int playerNr) const
{
	auto it = ranges::find_if (playerList, byPlayerId (playerNr));
	return it == playerList.end() ? nullptr : it->get();
}
//------------------------------------------------------------------------------
const cPlayer* cModel::getPlayer (std::string playerName) const
{
	// first try to find player by number
	const int playerNr = atoi (playerName.c_str());
	if (playerNr != 0 || playerName == "0")
	{
		return getPlayer (playerNr);
	}

	// try to find player by name
	auto it = ranges::find_if (playerList, [&] (const auto& player) { return player->getName() == playerName; });
	return it == playerList.end() ? nullptr : it->get();
}

//------------------------------------------------------------------------------
void cModel::setPlayerList (const std::vector<cPlayerBasicData>& splayers)
{
	assert (playerList.empty());

	for (const auto& playerInfo : splayers)
	{
		auto player = std::make_shared<cPlayer> (playerInfo, *unitsData);
		if (map) player->initMaps (map->getSize());
		playerList.push_back (player);
	}
	activeTurnPlayer = playerList[0].get();
}

//------------------------------------------------------------------------------
const cPlayer* cModel::getActiveTurnPlayer() const
{
	return activeTurnPlayer;
}

//------------------------------------------------------------------------------
cBuilding& cModel::addBuilding (const cPosition& position, const sID& id, cPlayer* player)
{
	const auto& staticUnitData = unitsData->getStaticUnitData (id);
	const auto& dynamicUnitData = player ? *player->getLastUnitData (id) : unitsData->getDynamicUnitData (id);
	auto addedBuilding = std::make_shared<cBuilding> (&staticUnitData, &dynamicUnitData, player, nextUnitId++);

	addedBuilding->setPosition (position);
	map->addBuilding (*addedBuilding, position);
	if (player)
	{
		player->addUnit (addedBuilding);
		player->base.addBuilding (*addedBuilding, *map);
		player->addToScan (*addedBuilding);
		if (addedBuilding->isSentryActive())
		{
			player->addToSentryMap (*addedBuilding);
		}
	}
	else
	{
		neutralBuildings.insert (addedBuilding);
	}
	addedBuilding->initMineResourceProd (*map);

	// if this is a top building, delete connectors, mines and roads
	if (addedBuilding->getStaticUnitData().surfacePosition == eSurfacePosition::Ground)
	{
		for (const auto& pos : addedBuilding->getPositions())
		{
			const auto& buildings = map->getField (pos).getBuildings();
			for (size_t i = 0; i != buildings.size(); ++i)
			{
				if (buildings[i]->getStaticData().canBeOverbuild == eOverbuildType::YesNRemove)
				{
					deleteUnit (buildings[i]);
					--i;
				}
			}
		}
	}

	if (addedBuilding->getStaticData().canMineMaxRes > 0)
	{
		addedBuilding->startWork();
	}
	addedBuilding->detectOtherUnits (*map);

	return *addedBuilding;
}

//------------------------------------------------------------------------------
cVehicle& cModel::addVehicle (const cPosition& position, const sID& id, cPlayer* player)
{
	const auto& staticUnitData = unitsData->getStaticUnitData (id);
	const auto& dynamicUnitData = player ? *player->getLastUnitData (id) : unitsData->getDynamicUnitData (id);
	auto addedVehicle = std::make_shared<cVehicle> (staticUnitData, dynamicUnitData, player, nextUnitId++);
	addedVehicle->setPosition (position);

	map->addVehicle (*addedVehicle, position);
	if (player)
	{
		player->addUnit (addedVehicle);
		player->addToScan (*addedVehicle);

		// scan with surveyor:
		if (addedVehicle->getStaticData().canSurvey)
		{
			addedVehicle->doSurvey (*getMap());
		}
		addedVehicle->detectOtherUnits (*map);
	}
	else
	{
		neutralVehicles.insert (addedVehicle);
	}
	return *addedVehicle;
}

//------------------------------------------------------------------------------
void cModel::destroyUnit (cUnit& unit)
{
	addJob (std::make_unique<cDestroyJob> (unit, *this));
}

//------------------------------------------------------------------------------
void cModel::addRubble (const cPosition& position, int value, bool big)
{
	value = std::max (1, value);

	if (map->isWaterOrCoast (position))
	{
		if (big)
		{
			addRubble (position + cPosition (1, 0), value / 4, false);
			addRubble (position + cPosition (0, 1), value / 4, false);
			addRubble (position + cPosition (1, 1), value / 4, false);
		}
		return;
	}

	if (big && map->isWaterOrCoast (position + cPosition (1, 0)))
	{
		addRubble (position, value / 4, false);
		addRubble (position + cPosition (0, 1), value / 4, false);
		addRubble (position + cPosition (1, 1), value / 4, false);
		return;
	}

	if (big && map->isWaterOrCoast (position + cPosition (0, 1)))
	{
		addRubble (position, value / 4, false);
		addRubble (position + cPosition (1, 0), value / 4, false);
		addRubble (position + cPosition (1, 1), value / 4, false);
		return;
	}

	if (big && map->isWaterOrCoast (position + cPosition (1, 1)))
	{
		addRubble (position, value / 4, false);
		addRubble (position + cPosition (1, 0), value / 4, false);
		addRubble (position + cPosition (0, 1), value / 4, false);
		return;
	}

	std::shared_ptr<cBuilding> rubble;
	if (big)
	{
		rubble = std::make_shared<cBuilding> (&unitsData->getRubbleBigData(), nullptr, nullptr, nextUnitId);
	}
	else
	{
		rubble = std::make_shared<cBuilding> (&unitsData->getRubbleSmallData(), nullptr, nullptr, nextUnitId);
	}

	nextUnitId++;

	rubble->setPosition (position);
	rubble->setRubbleValue (value, randomGenerator);

	map->addBuilding (*rubble, position);

	neutralBuildings.insert (std::move (rubble));
}

//------------------------------------------------------------------------------
void cModel::deleteUnit (cUnit* unit)
{
	if (unit == nullptr)
		return;

	NetLog.debug (" cModel: delete unit, id: " + std::to_string (unit->getId()) + " @" + std::to_string (getGameTime()));

	if (unit->isABuilding() && static_cast<cBuilding*> (unit)->isRubble())
	{
		deleteRubble (*static_cast<cBuilding*> (unit));
		return;
	}
	cPlayer* owner = unit->getOwner();

	casualtiesTracker->logCasualty (*unit);

	std::shared_ptr<cUnit> owningPtr; // keep owning ptr to make sure that unit instance will outlive the following method.
	if (owner)
	{
		if (cBuilding* building = dynamic_cast<cBuilding*> (unit))
		{
			owningPtr = owner->removeUnit (*building);
		}
		else if (cVehicle* vehicle = dynamic_cast<cVehicle*> (unit))
		{
			owningPtr = owner->removeUnit (*vehicle);
		}
		unit->forEachStoredUnits ([owner] (const auto& storedVehicle) { owner->removeUnit (storedVehicle); });
	}
	helperJobs.onRemoveUnit (*unit);

	// detach from move job
	if (cVehicle* vehicle = dynamic_cast<cVehicle*> (unit))
	{
		if (vehicle->getMoveJob())
		{
			assert (vehicle->getMoveJob()->getVehicleId() == vehicle->getId());
			vehicle->getMoveJob()->removeVehicle();
		}
	}
	if (unit->isAttacking())
	{
		for (auto& attackJob : attackJobs)
		{
			attackJob->onRemoveUnit (*unit);
		}
	}

	// lose eco points
	if (unit->isABuilding() && static_cast<cBuilding*> (unit)->points != 0 && owner)
	{
		owner->changeScore (-static_cast<cBuilding*> (unit)->points);
	}

	if (auto* building = dynamic_cast<cBuilding*> (unit))
		map->deleteBuilding (*building);
	else if (auto* vehicle = dynamic_cast<cVehicle*> (unit))
		map->deleteVehicle (*vehicle);

	if (unit->isABuilding() && static_cast<cBuilding*> (unit)->subBase != nullptr && owner)
		owner->base.deleteBuilding (static_cast<cBuilding&> (*unit), *map);

	if (owner != nullptr)
	{
		owner->removeFromSentryMap (*unit);
		owner->removeFromScan (*unit);
	}
}
//------------------------------------------------------------------------------
void cModel::deleteRubble (cBuilding& rubble)
{
	assert (rubble.isRubble());

	map->deleteBuilding (rubble);

	auto iter = neutralBuildings.find (rubble);
	assert (iter != neutralBuildings.end());

	if (iter != neutralBuildings.end())
	{
		neutralBuildings.erase (iter);
	}
}

//------------------------------------------------------------------------------
cMoveJob* cModel::addMoveJob (cVehicle& vehicle, const cPosition& destination)
{
	cMapView mapView (map, nullptr);
	cPathCalculator pc (vehicle, mapView, destination, false);
	auto path = pc.calcPath();
	if (path.empty())
	{
		return nullptr;
	}

	return addMoveJob (vehicle, path);
}

//------------------------------------------------------------------------------
cMoveJob* cModel::addMoveJob (cVehicle& vehicle, const std::forward_list<cPosition>& path)
{
	cMoveJob* currentMoveJob = vehicle.getMoveJob();
	if (currentMoveJob)
	{
		if (currentMoveJob->isActive())
		{
			// cannot add movejob while the unit is already moving
			return nullptr;
		}
		else
		{
			// a waiting movejob can be replaced by new one
			currentMoveJob->stop (vehicle);
			currentMoveJob->removeVehicle();
		}
	}
	auto moveJob = std::make_unique<cMoveJob> (path, vehicle, *this);
	vehicle.setMoveJob (moveJob.get());

	moveJobs.push_back (std::move (moveJob));

	return moveJobs.back().get();
}

//------------------------------------------------------------------------------
std::vector<const cPlayer*> cModel::resumeMoveJobs (const cPlayer* player /*= nullptr*/)
{
	std::vector<const cPlayer*> players;
	for (const auto& moveJob : moveJobs)
	{
		cVehicle* vehicle = moveJob->getVehicleId() ? getVehicleFromID (*moveJob->getVehicleId()) : nullptr;

		if (!vehicle || (player && vehicle->getOwner() != player))
		{
			continue;
		}
		if (moveJob->isWaiting() && vehicle->data.getSpeed() > 0)
		{
			moveJob->resume();
			players.push_back (vehicle->getOwner());
		}
	}
	RemoveDuplicates (players);

	return players;
}

//------------------------------------------------------------------------------
void cModel::addAttackJob (cUnit& aggressor, const cPosition& targetPosition)
{
	attackJobs.push_back (std::make_unique<cAttackJob> (aggressor, targetPosition, *this));
}

//------------------------------------------------------------------------------
void cModel::handlePlayerStartTurn (cPlayer& player)
{
	if (gameSettings->gameType == eGameSettingsGameType::HotSeat && player.getId() == activeTurnPlayer->getId())
	{
		turnTimeClock->restartFromNow();

		if (gameSettings->turnLimitActive)
		{
			turnLimitDeadline = turnTimeClock->startNewDeadlineFromNow (gameSettings->turnLimit);
		}
	}
}

//------------------------------------------------------------------------------
void cModel::handlePlayerFinishedTurn (cPlayer& player)
{
	player.setHasFinishedTurn (true);

	if (gameSettings->gameType == eGameSettingsGameType::Simultaneous && gameSettings->turnEndDeadlineActive && !turnEndDeadline)
	{
		turnEndDeadline = turnTimeClock->startNewDeadlineFromNow (gameSettings->turnEndDeadline);
	}

	playerFinishedTurn (player);
}

//------------------------------------------------------------------------------
void cModel::addFx (std::shared_ptr<cFx> fx)
{
	effectsList.push_back (fx);
	addedEffect (fx);
}

//------------------------------------------------------------------------------
void cModel::addJob (std::unique_ptr<cJob> job)
{
	helperJobs.addJob (*this, std::move (job));
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
cUnit* cModel::getUnitFromID (unsigned int id) const
{
	cUnit* result = getVehicleFromID (id);
	if (result == nullptr)
		result = getBuildingFromID (id);
	return result;
}

//------------------------------------------------------------------------------
cVehicle* cModel::getVehicleFromID (unsigned int id) const
{
	for (const auto& player : playerList)
	{
		auto unit = player->getVehicleFromId (id);
		if (unit) return unit;
	}
	auto iter = neutralVehicles.find (id);
	return iter == neutralVehicles.end() ? nullptr : iter->get();
	return nullptr;
}

//------------------------------------------------------------------------------
cBuilding* cModel::getBuildingFromID (unsigned int id) const
{
	for (const auto& player : playerList)
	{
		auto unit = player->getBuildingFromId (id);
		if (unit) return unit;
	}

	auto iter = neutralBuildings.find (id);
	return iter == neutralBuildings.end() ? nullptr : iter->get();
}

//------------------------------------------------------------------------------
void cModel::refreshMapPointer()
{
	map->reset();
	for (const auto& player : playerList)
	{
		for (const auto& vehicle : player->getVehicles())
		{
			if (!vehicle->isUnitLoaded())
				map->addVehicle (*vehicle, vehicle->getPosition());
		}
		for (const auto& building : player->getBuildings())
		{
			map->addBuilding (*building, building->getPosition());
		}
	}
	for (const auto& building : neutralBuildings)
	{
		map->addBuilding (*building, building->getPosition());
	}
	for (const auto& vehicle : neutralVehicles)
	{
		map->addVehicle (*vehicle, vehicle->getPosition());
	}
}

//------------------------------------------------------------------------------
void cModel::runMoveJobs()
{
	for (auto& moveJob : moveJobs)
	{
		moveJob->run (*this); //this can add new items to 'moveJobs'
		if (moveJob->isFinished())
		{
			cVehicle* vehicle = moveJob->getVehicleId() ? getVehicleFromID (*moveJob->getVehicleId()) : nullptr;
			if (vehicle != nullptr && vehicle->getMoveJob() == moveJob.get())
			{
				vehicle->setMoveJob (nullptr);
			}
			moveJob.reset();
		}
	}
	Remove (moveJobs, nullptr);
}

//------------------------------------------------------------------------------
void cModel::runAttackJobs()
{
	for (auto* attackJob : ExtractPtrs (attackJobs))
	{
		attackJob->run (*this); //this can add new items to 'attackjobs'
	}
	EraseIf (attackJobs, [] (const auto& job) { return job->finished(); });
}

//------------------------------------------------------------------------------
void cModel::handleTurnEnd()
{
	switch (turnEndState)
	{
		case eTurnEndState::TurnActive:
		{
			bool turnFinished = true;
			if (gameSettings->gameType == eGameSettingsGameType::Simultaneous)
			{
				turnFinished = ranges::all_of (playerList, [] (const auto& player) { return player->isDefeated || player->getHasFinishedTurn(); });
			}
			else
			{
				if (!activeTurnPlayer->getHasFinishedTurn())
				{
					turnFinished = false;
				}
			}
			if (turnFinished || turnTimeClock->hasReachedAnyDeadline())
			{
				turnEnded();

				const auto resumedMJobOwners = resumeMoveJobs (gameSettings->gameType == eGameSettingsGameType::Simultaneous ? nullptr : activeTurnPlayer);
				for (const auto& player : resumedMJobOwners)
				{
					player->turnEndMovementsStarted();
				}

				turnEndState = eTurnEndState::ExecuteRemainingMovements;
			}
		}
		break;
		case eTurnEndState::ExecuteRemainingMovements:
		{
			const bool activeMoveJob = ranges::any_of (moveJobs, [] (const auto& moveJob) { return moveJob->isActive(); });
			if (!activeMoveJob)
			{
				turnEndState = eTurnEndState::ExecuteTurnStart;
			}
		}
		break;
		case eTurnEndState::ExecuteTurnStart:
		{
			sNewTurnReport newTurnReport;
			if (gameSettings->gameType == eGameSettingsGameType::Simultaneous)
			{
				turnCounter->increaseTurn();

				for (auto& player : playerList)
				{
					newTurnReport.reports.emplace (player->getId(), player->makeTurnStart (*this));
				}

				// check game end conditions, after turn start, so generated points from this turn are also counted
				checkDefeats();
			}
			else
			{
				// select next player
				auto nextPlayerIter = ranges::find_if (playerList, [this] (const std::shared_ptr<cPlayer>& player) { return player.get() == activeTurnPlayer; });
				assert (nextPlayerIter != playerList.end());
				++nextPlayerIter;
				//TODO: skip defeated player?
				bool hasChangedTurn = false;
				if (nextPlayerIter == playerList.end())
				{
					activeTurnPlayer = playerList.front().get();
					turnCounter->increaseTurn();
					hasChangedTurn = true;
				}
				else
				{
					activeTurnPlayer = nextPlayerIter->get();
				}

				if (turnCounter->getTurn() > 1)
				{
					// don't execute turn start action in turn 1, because model is already completely initialized for turn 1
					newTurnReport.reports.emplace (activeTurnPlayer->getId(), activeTurnPlayer->makeTurnStart (*this));
				}

				if (hasChangedTurn)
				{
					// check game end conditions, after turn start, so generated points from this turn are also counted
					// and only check when first player starts the turn. So all players have played the same amount of turns.
					checkDefeats();
				}
			}

			turnTimeClock->clearAllDeadlines();
			turnEndDeadline = 0;
			turnLimitDeadline = 0;

			if (gameSettings->gameType != eGameSettingsGameType::HotSeat)
			{
				turnTimeClock->restartFromNow();

				if (gameSettings->turnLimitActive)
				{
					turnLimitDeadline = turnTimeClock->startNewDeadlineFromNow (gameSettings->turnLimit);
				}
			}

			turnEndState = eTurnEndState::TurnActive;
			newTurnStarted (newTurnReport);
		}
		break;
	}
}

//------------------------------------------------------------------------------
void cModel::setUnitsData (std::shared_ptr<cUnitsData> unitsData_)
{
	unitsData = unitsData_;
}

//------------------------------------------------------------------------------
void cModel::sideStepStealthUnit (const cPosition& position, const cVehicle& vehicle, const cPosition& bigOffset)
{
	sideStepStealthUnit (position, vehicle.getStaticUnitData(), vehicle.getOwner(), bigOffset);
}

//------------------------------------------------------------------------------
void cModel::sideStepStealthUnit (const cPosition& position, const cStaticUnitData& vehicleData, cPlayer* vehicleOwner, const cPosition& bigOffset)
{
	if (map->possiblePlaceVehicle (vehicleData, position, nullptr))
	{
		return;
	}

	if (vehicleData.factorAir > 0) return;

	// first look for an undetected stealth unit
	cVehicle* stealthVehicle = map->getField (position).getVehicle();
	if (!stealthVehicle) return;
	if (stealthVehicle->getOwner() == vehicleOwner) return;
	if (stealthVehicle->getStaticUnitData().isStealthOn == eTerrainFlag::None) return;
	if (stealthVehicle->isDetectedByPlayer (vehicleOwner)) return;

	if (stealthVehicle->isUnitMoving())
	{
		stealthVehicle->setDetectedByPlayer (vehicleOwner);
		return;
	}

	// found a stealth unit. Try to find a place where the unit can move
	bool placeFound = false;
	int minCosts = 99;
	cPosition bestPosition;
	const int minx = std::max (position.x() - 1, 0);
	const int maxx = std::min (position.x() + 1, map->getSize().x() - 1);
	const int miny = std::max (position.y() - 1, 0);
	const int maxy = std::min (position.y() + 1, map->getSize().y() - 1);
	for (int x = minx; x <= maxx; ++x)
	{
		for (int y = miny; y <= maxy; ++y)
		{
			const cPosition currentPosition (x, y);
			if (currentPosition == position) continue;

			// when a bigOffet was passed,
			// for example a constructor needs space for a big building
			// so not all directions are allowed for the side stepping
			if (bigOffset != -1)
			{
				if (currentPosition == bigOffset || currentPosition == bigOffset + cPosition (1, 0) || currentPosition == bigOffset + cPosition (0, 1) || currentPosition == bigOffset + cPosition (1, 1)) continue;
			}

			// check whether this field is a possible destination
			if (!map->possiblePlace (*stealthVehicle, currentPosition, false)) continue;

			// check costs of the move
			int costs = cPathCalculator::calcNextCost (position, currentPosition, stealthVehicle, map.get());
			if (costs > stealthVehicle->data.getSpeed()) continue;

			// check whether the vehicle would be detected
			// on the destination field
			bool detectOnDest = false;
			if (stealthVehicle->getStaticUnitData().isStealthOn & eTerrainFlag::Ground)
			{
				if (ranges::any_of (playerList, [&] (auto& player) {
						return player.get() != stealthVehicle->getOwner() && player->hasLandDetection (currentPosition);
					}))
				{
					detectOnDest = true;
				}
				if (map->isWater (currentPosition)) detectOnDest = true;
			}
			if (stealthVehicle->getStaticUnitData().isStealthOn & eTerrainFlag::Sea)
			{
				if (ranges::any_of (playerList, [&] (auto& player) {
						return player.get() != stealthVehicle->getOwner() && player->hasSeaDetection (currentPosition);
					}))
				{
					detectOnDest = true;
				}
				if (!map->isWater (currentPosition)) detectOnDest = true;

				if (stealthVehicle->getStaticUnitData().factorGround > 0)
				{
					if (map->getField (currentPosition).hasBridgeOrPlattform())
					{
						detectOnDest = true;
					}
				}
			}
			if (detectOnDest) continue;

			// take the move with the lowest costs.
			// Decide randomly, when costs are equal
			if (costs < minCosts || (costs == minCosts && randomGenerator.get (2)))
			{
				// this is a good candidate for a destination
				minCosts = costs;
				bestPosition = currentPosition;
				placeFound = true;
			}
		}
	}

	if (placeFound)
	{
		std::forward_list<cPosition> path;
		path.push_front (bestPosition);
		auto moveJob = addMoveJob (*stealthVehicle, path);
		moveJob->resume();
		return;
	}

	// sidestepping failed. Uncover the vehicle.
	stealthVehicle->setDetectedByPlayer (vehicleOwner);
}

//------------------------------------------------------------------------------
bool cModel::isVictoryConditionMet() const
{
	// if there is only one active player left, the game is over
	// but only, if there have been other players.
	const int activePlayers = ranges::count_if (playerList, [] (const auto& player) { return !player->isDefeated; });
	if (activePlayers == 1 && playerList.size() > 1) return true;

	switch (gameSettings->victoryConditionType)
	{
		case eGameSettingsVictoryCondition::Turns:
		{
			return turnCounter->getTurn() > static_cast<int> (gameSettings->victoryTurns);
		}
		case eGameSettingsVictoryCondition::Points:
		{
			return ranges::any_of (playerList, [this] (const auto& player) { return !player->isDefeated && player->getScore() >= static_cast<int> (gameSettings->victoryPoints); });
		}
		case eGameSettingsVictoryCondition::Death:
			// The victory condition for this mode is already checked.
			return false;
	}

	return false;
}

//------------------------------------------------------------------------------
void cModel::defeatLoserPlayers()
{
	for (const auto& player : playerList)
	{
		if (player->isDefeated) continue;
		if (player->mayHaveOffensiveUnit()) continue;

		player->isDefeated = true;
		playerHasLost (*player);
	}
}

//------------------------------------------------------------------------------
void cModel::checkDefeats()
{
	defeatLoserPlayers();
	if (!isVictoryConditionMet()) return;

	std::set<cPlayer*> winners;
	int best_score = -1;

	// find player(s) with highest score
	for (const auto& player : playerList)
	{
		if (player->isDefeated) continue;
		const int score = player->getScore();

		if (score < best_score) continue;

		if (score > best_score)
		{
			winners.clear();
			best_score = score;
		}
		winners.insert (player.get());
	}

	// Handle the case where there is more than one winner.
	// Original MAX calls a draw and displays the results screen.
	// For now we will have sudden death,
	// i.e. first player to get ahead in score wins.
	if (winners.size() > 1)
	{
		suddenDeathMode();
	}
	else
	{
		assert (winners.size() == 1);
		for (const auto& player : playerList)
		{
			if (player->isDefeated) continue;
			if (winners.count (player.get()) != 0)
			{
				playerHasWon (*player);
			}
			else
			{
				player->isDefeated = true;
				playerHasLost (*player);
			}
		}
	}
}
