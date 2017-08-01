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

#include <cassert>
#include <set>

#include "game/logic/server.h"

#include "game/logic/attackjob.h"
#include "game/data/units/building.h"
#include "game/data/units/landingunit.h"
#include "game/logic/casualtiestracker.h"
#include "game/logic/client.h"
#include "game/logic/clientevents.h"
#include "utility/listhelpers.h"
#include "game/logic/gametimer.h"
#include "game/logic/jobs.h"
#include "utility/log.h"
//#include "menuevents.h"
#include "netmessage.h"
#include "network.h"
#include "game/data/player/player.h"
#include "game/logic/serverevents.h"
#include "settings.h"
#include "game/logic/upgradecalculator.h"
#include "game/data/units/vehicle.h"
#include "game/data/gamesettings.h"
#include "ui/graphical/game/gameguistate.h"
#include "game/data/report/savedreport.h"
#include "game/data/report/savedreportsimple.h"
#include "game/data/report/savedreportchat.h"
#include "game/data/report/special/savedreportlostconnection.h"
#include "game/data/report/special/savedreportturnstart.h"
#include "game/logic/turncounter.h"
#include "game/logic/turntimeclock.h"
#include "utility/random.h"
#include "debug.h"

#if DEDICATED_SERVER_APPLICATION
# include "dedicatedserver.h"
#endif
#include "movejob.h"

//------------------------------------------------------------------------------
int CallbackRunServerThread (void* arg)
{
	CR_ENABLE_CRASH_RPT_CURRENT_THREAD();

	cServer* server = reinterpret_cast<cServer*> (arg);
	server->run();
	return 0;
}

//------------------------------------------------------------------------------
cServer::cServer(std::shared_ptr<cTCP> network_) :
	network(std::move(network_)),
	gameTimer(std::make_shared<cGameTimerServer>()),
	serverThread(nullptr),
	turnClock(std::make_unique<cTurnCounter>(1)),
	gameSettings(std::make_unique<cGameSettings>()),
	casualtiesTracker(new cCasualtiesTracker()),
	serverState(SERVER_STATE_INITGAME)
{
	bExit = false;
	openMapDefeat = true;
	activeTurnPlayer = nullptr;
	iNextUnitID = 1;
	savingID = 0;
	savingIndex = -1;

	if (!DEDICATED_SERVER)
	{
		//if (network) network->setMessageReceiver(this);
	}

	gameTimer->maxEventQueueSize = MAX_SERVER_EVENT_COUNTER;
	gameTimer->start();

	// connect to casualties tracker to send updates about casualties changes
	signalConnectionManager.connect(casualtiesTracker->casualtyChanged, [this](const sID unitId, int playerNr)
	{
		// TODO: send updated entry only!
		//sendCasualtiesReport(*this, nullptr);
	});

}
	

//------------------------------------------------------------------------------
cServer::~cServer()
{
	stop();

	// disconnect clients
	/*if (network)
	{
		for (size_t i = 0; i != playerList.size(); ++i)
		{
			network->close (playerList[i]->getSocketNum());
		}
		network->setMessageReceiver (nullptr);
	}
	*/
	for (size_t i = 0; i != AJobs.size(); ++i)
	{
		delete AJobs[i];
	}
}

//------------------------------------------------------------------------------
void cServer::setMap (std::shared_ptr<cStaticMap> staticMap)
{
	Map = std::make_unique<cMap> (staticMap);

	for (size_t i = 0; i < playerList.size(); ++i)
	{
		playerList[i]->initMaps (*Map);
	}
}

//------------------------------------------------------------------------------
void cServer::addPlayer (std::unique_ptr<cPlayer> player)
{
	if (Map != nullptr) player->initMaps (*Map);

	playerList.push_back (std::move (player));
}

//------------------------------------------------------------------------------
void cServer::setGameSettings (const cGameSettings& gameSettings_)
{
	*gameSettings = gameSettings_;
}

//------------------------------------------------------------------------------
bool cServer::isTurnBasedGame() const
{
	return gameSettings->getGameType() == eGameSettingsGameType::Turns;
}

//------------------------------------------------------------------------------
eGameTypes cServer::getGameType() const
{
	if (network) return GAME_TYPE_TCPIP;
	if (gameSettings->getGameType() == eGameSettingsGameType::HotSeat) return GAME_TYPE_HOTSEAT;
	return GAME_TYPE_SINGLE;
}

//------------------------------------------------------------------------------
void cServer::start()
{
	if (serverThread) return;

	if (DEDICATED_SERVER) return;

	serverThread = SDL_CreateThread (CallbackRunServerThread, "server", this);
}

//------------------------------------------------------------------------------
void cServer::stop()
{
	bExit = true;
	gameTimer->stop();

	if (!DEDICATED_SERVER)
	{
		if (serverThread)
		{
			SDL_WaitThread (serverThread, nullptr);
			serverThread = nullptr;
		}
	}
}

//------------------------------------------------------------------------------
void cServer::pushEvent (std::unique_ptr<cNetMessage> message)
{
	eventQueue.push (std::move (message));
}
//------------------------------------------------------------------------------
std::unique_ptr<cNetMessage> cServer::popEvent()
{
	std::unique_ptr<cNetMessage> message;
	eventQueue.try_pop(message);
	return message;
}


//------------------------------------------------------------------------------
void cServer::sendNetMessage (std::unique_ptr<cNetMessage> message, const cPlayer* player)
{
	/*
	const auto playerNumber = player != nullptr ? player->getId() : -1;
	const auto playerName = player != nullptr ? player->getName() : "all players";

	message->iPlayerNr = playerNumber;
	message->serialize();
	if (message->iType != NET_GAME_TIME_SERVER)
	{
		Log.write ("Server: --> " + playerName + " (" + iToStr (playerNumber) + ") "
				   + message->getTypeAsString()
				   + ", gameTime:" + iToStr(model.getGameTime())
				   + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);
	}

	if (player == nullptr)
	{
		if (network)
			network->send (message->iLength, message->data);
		for (size_t i = 0; i != localClients.size(); ++i)
			localClients[i]->pushEvent (std::make_unique<cNetMessage> (*message));
		return;
	}

	if (player->isLocal())
	{
		for (size_t i = 0; i != localClients.size(); ++i)
		{
			if (localClients[i]->getActivePlayer().getId() == player->getId())
			{
				localClients[i]->pushEvent (std::move (message));
				break;
			}
		}
	}
	// on all other sockets the netMessage will be send over TCP/IP
	else
	{
		if (network) network->sendTo (player->getSocketNum(), message->iLength, message->serialize());
	}
	*/
}

//------------------------------------------------------------------------------
void cServer::run()
{
	while (!bExit)
	{
		std::unique_ptr<cNetMessage> message;
		while (eventQueue.try_pop (message))
		{
			handleNetMessage (*message);
//			if (message->iType != NET_GAME_TIME_CLIENT)
//				checkPlayerUnits();
		}

		// don't do anything if games hasn't been started yet!
		unsigned int lastTime = model.getGameTime();
		if (serverState == SERVER_STATE_INGAME)
		{
			//gameTimer->run (*this);
		}

		// nothing done
		if (!message && lastTime == model.getGameTime())
		{
			SDL_Delay (10);
		}
	}
}

void cServer::doGameActions()
{
	cAttackJob::runAttackJobs (AJobs);
	runJobs();
	checkPlayerUnits();
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_ATTACK (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_ATTACK);
	int targetID = message.popInt32();
	auto targetPosition = message.popPosition();
	int aggressorID = message.popInt32();

	cUnit* aggressor = getUnitFromID (aggressorID);
	cUnit* target = getUnitFromID (targetID);

	//validate aggressor
	if (aggressor == nullptr)
	{
		Log.write (" Server: vehicle with ID " + iToStr (aggressorID) + " not found", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}
	if (aggressor->getOwner()->getId() != message.iPlayerNr)
	{
		Log.write (" Server: Message was not send by vehicle owner!", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	if (aggressor->isBeeingAttacked()) return;

	//validate target
	if (!Map->isValidPosition (targetPosition))
	{
		Log.write (" Server: Invalid target coordinates", cLog::eLOG_TYPE_NET_ERROR);
	}
	if (target && !target->isABuilding() && !target->getIsBig())
	{
		if (targetPosition != target->getPosition())
		{
			Log.write (" Server: target coords changed from (" + iToStr (targetPosition.x()) + "," + iToStr (targetPosition.y()) + ") to (" + iToStr (target->getPosition().x()) + "," + iToStr (target->getPosition().y()) + ") to match current unit position", cLog::eLOG_TYPE_NET_DEBUG);
		}
		targetPosition = target->getPosition();

		Log.write (" Server: attacking unit " + target->getDisplayName() + ", " + iToStr (target->iID), cLog::eLOG_TYPE_NET_DEBUG);
	}



	// check if attack is possible
	if (aggressor->canAttackObjectAt (targetPosition, *Map, true) == false)
	{
		Log.write (" Server: The server decided, that the attack is not possible", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}
	addAttackJob (aggressor, targetPosition);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_MINELAYERSTATUS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_MINELAYERSTATUS);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (!Vehicle) return;

	Vehicle->setClearMines (message.popBool());
	Vehicle->setLayMines (message.popBool());

	if (Vehicle->isUnitClearingMines() && Vehicle->isUnitLayingMines())
	{
		Vehicle->setClearMines (false);
		Vehicle->setLayMines (false);
		return;
	}

	bool result = false;
	if (Vehicle->isUnitClearingMines()) result = Vehicle->clearMine (*this);
	if (Vehicle->isUnitLayingMines()) result = Vehicle->layMine (*this);

	if (result)
	{
		//sendUnitData (*this, *Vehicle);
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_BUILD (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_BUILD);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle == nullptr) return;
	if (Vehicle->isUnitBuildingABuilding() || Vehicle->BuildPath) return;

	const sID BuildingTyp = message.popID();
	if (!model.getUnitsData()->isValidId(BuildingTyp))
	{
		Log.write (" Server: invalid unit: " + iToStr (BuildingTyp.firstPart) + "." + iToStr (BuildingTyp.secondPart), cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	const cStaticUnitData& Data = model.getUnitsData()->getStaticUnitData(BuildingTyp);
	const int iBuildSpeed = message.popInt16();
	if (iBuildSpeed > 2 || iBuildSpeed < 0) return;
	const auto buildPosition = message.popPosition();

	std::array<int, 3> iTurboBuildRounds;
	std::array<int, 3> iTurboBuildCosts;
	Vehicle->calcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, Vehicle->getOwner()->getUnitDataCurrentVersion (BuildingTyp)->getBuildCost());

	if (iTurboBuildCosts[iBuildSpeed] > Vehicle->getStoredResources() ||
		iTurboBuildRounds[iBuildSpeed] <= 0)
	{
		// TODO: differ between different aborting types
		// (buildposition blocked, not enough material, ...)
		sendBuildAnswer (*this, false, *Vehicle);
		return;
	}

	if (Map->isValidPosition (buildPosition) == false) return;
	const auto oldPosition = Vehicle->getPosition();

	if (Vehicle->getStaticUnitData().canBuild != Data.buildAs) return;

	if (Data.isBig)
	{
		sideStepStealthUnit (buildPosition,                   *Vehicle, buildPosition);
		sideStepStealthUnit (buildPosition + cPosition (1, 0), *Vehicle, buildPosition);
		sideStepStealthUnit (buildPosition + cPosition (0, 1), *Vehicle, buildPosition);
		sideStepStealthUnit (buildPosition + cPosition (1, 1), *Vehicle, buildPosition);

		if (! (Map->possiblePlaceBuilding (Data, buildPosition,                   Vehicle) &&
			   Map->possiblePlaceBuilding (Data, buildPosition + cPosition (1, 0), Vehicle) &&
			   Map->possiblePlaceBuilding (Data, buildPosition + cPosition (0, 1), Vehicle) &&
			   Map->possiblePlaceBuilding (Data, buildPosition + cPosition (1, 1), Vehicle)))
		{
			sendBuildAnswer (*this, false, *Vehicle);
			return;
		}
		Vehicle->buildBigSavedPosition = Vehicle->getPosition();

		// set vehicle to build position
		Map->moveVehicleBig (*Vehicle, buildPosition);
		Vehicle->getOwner()->doScan();
	}
	else
	{
		if (buildPosition != Vehicle->getPosition()) return;

		if (!Map->possiblePlaceBuilding (Data, buildPosition, Vehicle))
		{
			sendBuildAnswer (*this, false, *Vehicle);
			return;
		}
	}

	Vehicle->setBuildingType (BuildingTyp);
	const bool bBuildPath = message.popBool();
	const cPosition pathPosition = message.popPosition();
	if (Map->isValidPosition (pathPosition) == false) return;
	Vehicle->bandPosition = pathPosition;

	Vehicle->setBuildCosts (iTurboBuildCosts[iBuildSpeed]);
	Vehicle->setBuildTurns (iTurboBuildRounds[iBuildSpeed]);
	Vehicle->setBuildCostsStart (Vehicle->getBuildCosts());
	Vehicle->setBuildTurnsStart (Vehicle->getBuildTurns());

	Vehicle->setBuildingABuilding (true);
	Vehicle->BuildPath = bBuildPath;

	sendBuildAnswer (*this, true, *Vehicle);
	//addJob (new cStartBuildJob (*Vehicle, oldPosition, Data.isBig));

	if (Vehicle->getMoveJob()) Vehicle->getMoveJob()->stop();
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_END_BUILDING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_END_BUILDING);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle == nullptr) return;

	const cPosition escapePosition (message.popPosition());

	if (!Vehicle->isUnitBuildingABuilding() || Vehicle->getBuildTurns() > 0) return;
	if (!Map->possiblePlace (*Vehicle, escapePosition))
	{
		sideStepStealthUnit (escapePosition, *Vehicle);
	}

	if (!Map->possiblePlace (*Vehicle, escapePosition)) return;

	//addBuilding (Vehicle->getPosition(), Vehicle->getBuildingType(), Vehicle->getOwner());

	// end building
	Vehicle->setBuildingABuilding (false);
	Vehicle->BuildPath = false;

	// set the vehicle to the border
	if (model.getUnitsData()->getStaticUnitData(Vehicle->getBuildingType()).isBig)
	{
		int x = Vehicle->getPosition().x();
		int y = Vehicle->getPosition().y();
		if (escapePosition.x() > Vehicle->getPosition().x()) x++;
		if (escapePosition.y() > Vehicle->getPosition().y()) y++;
		Map->moveVehicle (*Vehicle, cPosition (x, y));

		// refresh SeenByPlayerLists
		checkPlayerUnits();
	}

	// drive away from the building lot
	//addMoveJob (Vehicle->getPosition(), escapePosition, Vehicle);
	// begin the movement immediately,
	// so no other unit can block the destination field
	//Vehicle->ServerMoveJob->checkMove();
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_STOP_BUILDING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_STOP_BUILDING);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle == nullptr) return;
	if (!Vehicle->isUnitBuildingABuilding()) return;
	stopVehicleBuilding (*Vehicle);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_BUILDLIST (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_BUILDLIST);

	cBuilding* Building = getBuildingFromID (message.popInt16());
	if (Building == nullptr) return;

	// check whether the building has water and land fields around it
	int iX = Building->getPosition().x() - 2;
	int iY = Building->getPosition().y() - 1;
	bool bLand = false;
	bool bWater = false;
	for (int i = 0; i < 12; ++i)
	{
		if (i == 4 || i == 6 || i == 8)
		{
			iX -= 3;
			iY += 1;
		}
		else
		{
			if (i == 5 || i == 7) iX += 3;
			else iX++;
		}
		if (Map->isValidPosition (cPosition (iX, iY)) == false) continue;

		const auto& buildings = Map->getField (cPosition (iX, iY)).getBuildings();
		auto b_it = buildings.begin();
		auto b_end = buildings.end();
		while (b_it != b_end && ((*b_it)->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE || (*b_it)->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE_BASE)) ++b_it;

		if (!Map->isWaterOrCoast(cPosition(iX, iY)) || (b_it != b_end && (*b_it)->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_BASE)) bLand = true;
		else if (Map->isWaterOrCoast(cPosition(iX, iY)) && b_it != b_end && (*b_it)->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE_SEA)
		{
			bLand = true;
			bWater = true;
			break;
		}
		else if (Map->isWaterOrCoast (cPosition (iX, iY))) bWater = true;
	}

	// reset building status
	if (Building->isUnitWorking())
	{
		Building->stopWork (false);
	}

	const int iBuildSpeed = message.popInt16();
	if (iBuildSpeed == 0) Building->setMetalPerRound ( 1 * Building->getStaticUnitData().needsMetal);
	if (iBuildSpeed == 1) Building->setMetalPerRound ( 4 * Building->getStaticUnitData().needsMetal);
	if (iBuildSpeed == 2) Building->setMetalPerRound (12 * Building->getStaticUnitData().needsMetal);
	std::vector<cBuildListItem> NewBuildList;

	const int iCount = message.popInt16();
	for (int i = 0; i < iCount; i++)
	{
		const sID Type = message.popID();

		// if the first unit hasn't changed copy it to the new buildlist
		if (!Building->isBuildListEmpty() && i == 0 && Type == Building->getBuildListItem (0).getType())
		{
			// recalculate costs, because build speed could have been changed
			std::array<int, 3> iTurboBuildRounds;
			std::array<int, 3> iTurboBuildCosts;
			Building->calcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, Building->getOwner()->getUnitDataCurrentVersion (Type)->getBuildCost(), Building->getBuildListItem (0).getRemainingMetal());
			NewBuildList.push_back (cBuildListItem (Type, iTurboBuildCosts[iBuildSpeed]));
			continue;
		}

		// check whether this building can build this unit
		if (model.getUnitsData()->getStaticUnitData(Type).factorSea > 0 && model.getUnitsData()->getStaticUnitData(Type).factorGround == 0 && !bWater)
			continue;
		else if (model.getUnitsData()->getStaticUnitData(Type).factorGround > 0 && model.getUnitsData()->getStaticUnitData(Type).factorSea == 0 && !bLand)
			continue;

		if (Building->getStaticUnitData().canBuild != model.getUnitsData()->getStaticUnitData(Type).buildAs)
			continue;

		cBuildListItem BuildListItem (Type, -1);

		NewBuildList.push_back (BuildListItem);
	}

	Building->setBuildList (std::move (NewBuildList));

	if (!Building->isBuildListEmpty())
	{
		if (Building->getBuildListItem (0).getRemainingMetal() == -1)
		{
			std::array<int, 3> iTurboBuildRounds;
			std::array<int, 3> iTurboBuildCosts;
			Building->calcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, Building->getOwner()->getUnitDataCurrentVersion (Building->getBuildListItem (0).getType())->getBuildCost());
			Building->getBuildListItem (0).setRemainingMetal (iTurboBuildCosts[iBuildSpeed]);
		}

		Building->setRepeatBuild(message.popBool());
		Building->setBuildSpeed(iBuildSpeed);
		if (Building->getBuildListItem (0).getRemainingMetal() > 0)
		{
			Building->startWork ();
		}
	}
	sendBuildList (*this, *Building);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_EXIT_FIN_VEH (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_EXIT_FIN_VEH);

	cBuilding* Building = getBuildingFromID (message.popInt16());
	if (Building == nullptr) return;

	const auto position = message.popPosition();
	if (Map->isValidPosition (position) == false) return;
	if (Building->isBuildListEmpty()) return;

	cBuildListItem& BuildingListItem = Building->getBuildListItem (0);
	if (BuildingListItem.getRemainingMetal() > 0) return;

	if (!Building->isNextTo (position)) return;

	const cStaticUnitData& unitData = model.getUnitsData()->getStaticUnitData(BuildingListItem.getType());
	if (!Map->possiblePlaceVehicle (unitData, position, Building->getOwner()))
	{
		sideStepStealthUnit (position, unitData, Building->getOwner());
	}
	if (!Map->possiblePlaceVehicle (unitData, position, Building->getOwner())) return;

	//addVehicle (position, BuildingListItem.getType(), Building->getOwner(), false);

	// start new buildjob
	if (Building->getRepeatBuild())
	{
		BuildingListItem.setRemainingMetal (-1);
		Building->addBuildListItem (BuildingListItem);
	}
	Building->removeBuildListItem (0);

	if (!Building->isBuildListEmpty())
	{
		cBuildListItem& BuildingListItem = Building->getBuildListItem (0);
		if (BuildingListItem.getRemainingMetal() == -1)
		{
			std::array<int, 3> iTurboBuildRounds;
			std::array<int, 3> iTurboBuildCosts;
			Building->calcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, Building->getOwner()->getUnitDataCurrentVersion (BuildingListItem.getType())->getBuildCost());
			BuildingListItem.setRemainingMetal (iTurboBuildCosts[Building->getBuildSpeed()]);
		}
		Building->startWork ();
	}
	sendBuildList (*this, *Building);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_CHANGE_RESOURCES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_CHANGE_RESOURCES);

	cBuilding* Building = getBuildingFromID (message.popInt16());
	if (Building == nullptr) return;

	const unsigned int iMetalProd = message.popInt16();
	const unsigned int iOilProd = message.popInt16();
	const unsigned int iGoldProd = message.popInt16();

	cSubBase& subBase = *Building->subBase;

	subBase.setMetalProd (0);
	subBase.setOilProd (0);
	subBase.setGoldProd (0);

	// no need to verify the values.
	// They will be reduced automatically, if necessary
	subBase.setMetalProd (iMetalProd);
	subBase.setGoldProd (iGoldProd);
	subBase.setOilProd (iOilProd);

	//sendSubbaseValues (*this, subBase, *Building->getOwner());
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_CHANGE_MANUAL_FIRE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_CHANGE_MANUAL_FIRE);

	if (message.popBool()) // vehicle
	{
		cVehicle* Vehicle = getVehicleFromID (message.popInt16());
		if (Vehicle == 0)
			return;
		Vehicle->setManualFireActive (!Vehicle->isManualFireActive());
		if (Vehicle->isManualFireActive() && Vehicle->isSentryActive())
		{
			Vehicle->getOwner()->deleteSentry (*Vehicle);
		}

		//sendUnitData (*this, *Vehicle);
	}
	else // building
	{
		cBuilding* Building = getBuildingFromID (message.popInt16());
		if (Building == 0)
			return;
		Building->setManualFireActive (!Building->isManualFireActive());
		if (Building->isManualFireActive() && Building->isSentryActive())
		{
			Building->getOwner()->deleteSentry (*Building);
		}

		//sendUnitData (*this, *Building);
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_CHANGE_SENTRY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_CHANGE_SENTRY);

	if (message.popBool()) // vehicle
	{
		cVehicle* vehicle = getVehicleFromID (message.popInt16());
		if (vehicle == nullptr) return;

		if (vehicle->isSentryActive())
		{
			vehicle->getOwner()->deleteSentry (*vehicle);
		}
		else
		{
			vehicle->getOwner()->addSentry (*vehicle);
			vehicle->setManualFireActive (false);
		}

		//sendUnitData (*this, *vehicle);
	}
	else // building
	{
		cBuilding* building = getBuildingFromID (message.popInt16());
		if (building == nullptr) return;

		if (building->isSentryActive())
		{
			building->getOwner()->deleteSentry (*building);
		}
		else
		{
			building->getOwner()->addSentry (*building);
			building->setManualFireActive (false);
		}

		//sendUnitData (*this, *building);
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_MARK_LOG (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_MARK_LOG);

	auto message2 = std::make_unique<cNetMessage> (GAME_EV_MARK_LOG);
	message2->pushString (message.popString());
	sendNetMessage (std::move (message2));
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_SUPPLY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_SUPPLY);

	cVehicle* SrcVehicle = nullptr;
	cVehicle* DestVehicle = nullptr;
	cBuilding* SrcBuilding = nullptr;
	cBuilding* DestBuilding = nullptr;

	// get the units
	const int iType = message.popChar();
	if (iType > SUPPLY_TYPE_REPAIR) return;   // unknown type
	if (message.popBool()) SrcVehicle = getVehicleFromID (message.popInt16());
	else SrcBuilding = getBuildingFromID (message.popInt16());
	if (message.popBool()) DestVehicle = getVehicleFromID (message.popInt16());
	else DestBuilding = getBuildingFromID (message.popInt16());

	if ((!SrcVehicle && !SrcBuilding) || (!DestVehicle && !DestBuilding)) return;

	// check whether the supply is ok and reduce cargo of sourceunit
	int iValue;
	if (SrcVehicle)
	{
		if (DestVehicle && !SrcVehicle->canSupply (DestVehicle, iType)) return;
		if (DestBuilding && !SrcVehicle->canSupply (DestBuilding, iType)) return;

		// do the supply
		if (iType == SUPPLY_TYPE_REARM)
		{
			SrcVehicle->setStoredResources (SrcVehicle->getStoredResources() - 1);
			iValue = DestVehicle ? DestVehicle->data.getAmmoMax() : DestBuilding->data.getAmmoMax();
		}
		else
		{
			cDynamicUnitData* DestData = DestVehicle ? &DestVehicle->data : &DestBuilding->data;
			// reduce cargo for repair and calculate maximal repair value
			iValue = DestData->getHitpoints();
			while (SrcVehicle->getStoredResources() > 0 && iValue < DestData->getHitpointsMax())
			{
				iValue += Round (((float)DestData->getHitpointsMax() / DestData->getBuildCost()) * 4);
				SrcVehicle->setStoredResources (SrcVehicle->getStoredResources() - 1);
			}
			iValue = std::min (DestData->getHitpointsMax(), iValue);
		}
		// the changed values aren't interesting for enemy players,
		// so only send the new data to the owner
		//sendUnitData (*this, *SrcVehicle, *SrcVehicle->getOwner());
	}
	else
	{
		// buildings can only supply vehicles
		if (!DestVehicle) return;

		// do the supply
		if (iType == SUPPLY_TYPE_REARM)
		{
			if (SrcBuilding->subBase->getMetalStored() < 1) return;
			SrcBuilding->subBase->addMetal (-1);
			iValue = DestVehicle->data.getAmmoMax();
		}
		else
		{
			// reduce cargo for repair and calculate maximal repair value
			iValue = DestVehicle->data.getHitpoints();
			while (SrcBuilding->subBase->getMetalStored() > 0 && iValue < DestVehicle->data.getHitpointsMax())
			{
				iValue += Round (((float) DestVehicle->data.getHitpointsMax() / DestVehicle->data.getBuildCost()) * 4);
				SrcBuilding->subBase->addMetal (-1);
			}
			iValue = std::min (DestVehicle->data.getHitpointsMax(), iValue);
		}
		// the changed values aren't interesting for enemy players,
		// so only send the new data to the owner
		//sendUnitData (*this, *SrcBuilding, *SrcBuilding->getOwner());
	}

	// repair or reload the destination unit
	if (DestVehicle)
	{
		if (iType == SUPPLY_TYPE_REARM) DestVehicle->data.setAmmo (DestVehicle->data.getAmmoMax());
		else DestVehicle->data.setHitpoints (iValue);

		sendSupply (*this, DestVehicle->iID, true, iValue, iType, *DestVehicle->getOwner());

		// send unitdata to the players who are not the owner
		//for (size_t i = 0; i != DestVehicle->seenByPlayerList.size(); ++i)
			//sendUnitData (*this, *DestVehicle, *DestVehicle->seenByPlayerList[i]);
	}
	else
	{
		if (iType == SUPPLY_TYPE_REARM) DestBuilding->data.setAmmo (DestBuilding->data.getAmmoMax());
		else DestBuilding->data.setHitpoints (iValue);

		sendSupply (*this, DestBuilding->iID, false, iValue, iType, *DestBuilding->getOwner());

		// send unitdata to the players who are not the owner
		//for (size_t i = 0; i != DestBuilding->seenByPlayerList.size(); ++i)
			//sendUnitData (*this, *DestBuilding, *DestBuilding->seenByPlayerList[i]);
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_VEHICLE_UPGRADE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_VEHICLE_UPGRADE);

	const bool upgradeAll = message.popBool();
	unsigned int storageSlot = 0;
	if (!upgradeAll)
		storageSlot = message.popInt16();
	cBuilding* storingBuilding = getBuildingFromID (message.popInt32());
	if (storingBuilding == 0)
		return;

	int totalCosts = 0;
	const int availableMetal = storingBuilding->subBase->getMetalStored();

	std::vector<cVehicle*> upgradedVehicles;
	for (size_t i = 0; i != storingBuilding->storedUnits.size(); ++i)
	{
		if (upgradeAll || i == storageSlot)
		{
			cVehicle* vehicle = storingBuilding->storedUnits[i];
			const cDynamicUnitData& upgradedVersion = *storingBuilding->getOwner()->getUnitDataCurrentVersion (vehicle->data.getId());

			if (vehicle->data.getVersion() >= upgradedVersion.getVersion())
				continue; // already up to date
			cUpgradeCalculator& uc = cUpgradeCalculator::instance();
			const int upgradeCost = uc.getMaterialCostForUpgrading (upgradedVersion.getBuildCost());

			if (availableMetal >= totalCosts + upgradeCost)
			{
				upgradedVehicles.push_back (vehicle);
				totalCosts += upgradeCost;
			}
		}
	}

	if (upgradedVehicles.empty() == false)
	{
		if (totalCosts > 0)
			storingBuilding->subBase->addMetal (-totalCosts);
		for (size_t i = 0; i != upgradedVehicles.size(); ++i)
			upgradedVehicles[i]->upgradeToCurrentVersion();
		sendUpgradeVehicles (*this, upgradedVehicles, totalCosts, storingBuilding->iID, *storingBuilding->getOwner());
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_START_CLEAR (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_START_CLEAR);

	const int id = message.popInt16();
	cVehicle* Vehicle = getVehicleFromID (id);
	if (Vehicle == nullptr)
	{
		Log.write ("Server: Can not find vehicle with id " + iToStr (id) + " for clearing", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}

	cBuilding* building = Map->getField (Vehicle->getPosition()).getRubble();

	if (!building)
	{
		sendClearAnswer (*this, 2, *Vehicle, 0, cPosition (-1, -1), Vehicle->getOwner());
		return;
	}

	cPosition rubblePosition (-1, -1);
	if (building->getIsBig())
	{
		rubblePosition = building->getPosition();

		sideStepStealthUnit (building->getPosition()                  , *Vehicle, rubblePosition);
		sideStepStealthUnit (building->getPosition() + cPosition (1, 0), *Vehicle, rubblePosition);
		sideStepStealthUnit (building->getPosition() + cPosition (0, 1), *Vehicle, rubblePosition);
		sideStepStealthUnit (building->getPosition() + cPosition (1, 1), *Vehicle, rubblePosition);

		if ((!Map->possiblePlace (*Vehicle, building->getPosition()) && rubblePosition                   != Vehicle->getPosition()) ||
			(!Map->possiblePlace (*Vehicle, building->getPosition() + cPosition (1, 0)) && rubblePosition + cPosition (1, 0) != Vehicle->getPosition()) ||
			(!Map->possiblePlace (*Vehicle, building->getPosition() + cPosition (0, 1)) && rubblePosition + cPosition (0, 1) != Vehicle->getPosition()) ||
			(!Map->possiblePlace (*Vehicle, building->getPosition() + cPosition (1, 1)) && rubblePosition + cPosition (1, 1) != Vehicle->getPosition()))
		{
			sendClearAnswer (*this, 1, *Vehicle, 0, cPosition (-1, -1), Vehicle->getOwner());
			return;
		}

		Vehicle->buildBigSavedPosition = Vehicle->getPosition();
		Map->moveVehicleBig (*Vehicle, building->getPosition());
	}

	Vehicle->setClearing (true);
	Vehicle->setClearingTurns (building->getIsBig() ? 4 : 1);
	Vehicle->getOwner()->doScan();
	//addJob (new cStartBuildJob (*Vehicle, Vehicle->getPosition(), building->data.isBig));

	sendClearAnswer (*this, 0, *Vehicle, Vehicle->getClearingTurns(), rubblePosition, Vehicle->getOwner());
	for (size_t i = 0; i != Vehicle->seenByPlayerList.size(); ++i)
	{
		sendClearAnswer (*this, 0, *Vehicle, 0, rubblePosition, Vehicle->seenByPlayerList[i]);
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_STOP_CLEAR (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_STOP_CLEAR);

	const int id = message.popInt16();
	cVehicle* Vehicle = getVehicleFromID (id);
	if (Vehicle == nullptr)
	{
		Log.write ("Server: Can not find vehicle with id " + iToStr (id) + " for stop clearing", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}

	if (Vehicle->isUnitClearing())
	{
		Vehicle->setClearing (false);
		Vehicle->setClearingTurns (0);

		if (Vehicle->getIsBig())
		{
			Map->moveVehicle (*Vehicle, Vehicle->buildBigSavedPosition);
			Vehicle->getOwner()->doScan();
			sendStopClear (*this, *Vehicle, Vehicle->buildBigSavedPosition, *Vehicle->getOwner());
			for (size_t i = 0; i != Vehicle->seenByPlayerList.size(); ++i)
			{
				sendStopClear (*this, *Vehicle, Vehicle->buildBigSavedPosition, *Vehicle->seenByPlayerList[i]);
			}
		}
		else
		{
			sendStopClear (*this, *Vehicle, cPosition (-1, -1), *Vehicle->getOwner());
			for (size_t i = 0; i != Vehicle->seenByPlayerList.size(); ++i)
			{
				sendStopClear (*this, *Vehicle, cPosition (-1, -1), *Vehicle->seenByPlayerList[i]);
			}
		}
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_ABORT_WAITING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ABORT_WAITING);

	if (DisconnectedPlayerList.empty()) return;
	// only server player can abort the waiting
	auto& LocalPlayer = getPlayerFromNumber (message.iPlayerNr);
	//if (LocalPlayer.isLocal() == false) return;

	// delete disconnected players
	for (size_t i = 0; i != DisconnectedPlayerList.size(); ++i)
	{
		deletePlayer (*DisconnectedPlayerList[i]);
	}
	DisconnectedPlayerList.clear();
//	disableFreezeMode (FREEZE_WAIT_FOR_RECONNECT);
	//if (isTurnBasedGame()) sendWaitFor (*this, *activeTurnPlayer, nullptr);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_LOAD (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_LOAD);

	cVehicle* StoredVehicle = getVehicleFromID (message.popInt16());
	if (!StoredVehicle) return;

	if (message.popBool())
	{
		cVehicle* StoringVehicle = getVehicleFromID (message.popInt16());
		if (!StoringVehicle) return;

		if (StoringVehicle->canLoad (StoredVehicle))
		{
			StoringVehicle->storeVehicle (*StoredVehicle, *Map);
			if (StoredVehicle->getMoveJob()) StoredVehicle->getMoveJob()->stop();
			// vehicle is removed from enemy clients by cServer::checkPlayerUnits()
			sendStoreVehicle (*this, StoringVehicle->iID, true, StoredVehicle->iID, *StoringVehicle->getOwner());
		}
	}
	else
	{
		cBuilding* StoringBuilding = getBuildingFromID (message.popInt16());
		if (!StoringBuilding) return;

		if (StoringBuilding->canLoad (StoredVehicle))
		{
			StoringBuilding->storeVehicle (*StoredVehicle, *Map);
			if (StoredVehicle->getMoveJob()) StoredVehicle->getMoveJob()->stop();
			// vehicle is removed from enemy clients by cServer::checkPlayerUnits()
			sendStoreVehicle (*this, StoringBuilding->iID, false, StoredVehicle->iID, *StoringBuilding->getOwner());
		}
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_EXIT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_EXIT);

	cVehicle* StoredVehicle = getVehicleFromID (message.popInt16());
	if (!StoredVehicle) return;

	if (message.popBool())
	{
		cVehicle* StoringVehicle = getVehicleFromID (message.popInt16());
		if (!StoringVehicle) return;

		const auto position = message.popPosition();

		if (!StoringVehicle->isNextTo (position)) return;

		// sidestep stealth units if necessary
		sideStepStealthUnit (position, *StoredVehicle);

		if (StoringVehicle->canExitTo (position, *Map, StoredVehicle->getStaticUnitData()))
		{
			StoringVehicle->exitVehicleTo (*StoredVehicle, position, *Map);
			// vehicle is added to enemy clients by cServer::checkPlayerUnits()
			sendActivateVehicle (*this, StoringVehicle->iID, true, StoredVehicle->iID, position, *StoringVehicle->getOwner());
			if (StoredVehicle->getStaticUnitData().canSurvey)
			{
				sendVehicleResources (*this, *StoredVehicle);
				StoredVehicle->doSurvey ();
			}

			if (StoredVehicle->canLand (*Map))
			{
				StoredVehicle->setFlightHeight (0);
			}
			else
			{
				StoredVehicle->setFlightHeight (64);
			}
			StoredVehicle->InSentryRange (*this);
		}
	}
	else
	{
		cBuilding* StoringBuilding = getBuildingFromID (message.popInt16());
		if (!StoringBuilding) return;

		const auto position = message.popPosition();

		if (!StoringBuilding->isNextTo (position)) return;

		// sidestep stealth units if necessary
		sideStepStealthUnit (position, *StoredVehicle);

		if (StoringBuilding->canExitTo(position, *Map, StoredVehicle->getStaticUnitData()))
		{
			StoringBuilding->exitVehicleTo (*StoredVehicle, position, *Map);
			// vehicle is added to enemy clients by cServer::checkPlayerUnits()
			sendActivateVehicle (*this, StoringBuilding->iID, false, StoredVehicle->iID, position, *StoringBuilding->getOwner());
			if (StoredVehicle->getStaticUnitData().canSurvey)
			{
				sendVehicleResources (*this, *StoredVehicle);
				StoredVehicle->doSurvey ();
			}
			StoredVehicle->InSentryRange (*this);
		}
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_BUY_UPGRADES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_BUY_UPGRADES);

	const int iPlayerNr = message.popInt16();
	auto& player = getPlayerFromNumber (iPlayerNr);

	bool updateCredits = false;

	// get the number of upgrades in this message
	const int iCount = message.popInt16();
	for (int i = 0; i < iCount; ++i)
	{
		const sID ID = message.popID();
		const int newDamage = message.popInt16();
		const int newMaxShots = message.popInt16();
		const int newRange = message.popInt16();
		const int newMaxAmmo = message.popInt16();
		const int newArmor = message.popInt16();
		const int newMaxHitPoints = message.popInt16();
		const int newScan = message.popInt16();
		int newMaxSpeed = 0;
		if (ID.isAVehicle()) newMaxSpeed = message.popInt16();

		cDynamicUnitData* upgradedUnit = player.getUnitDataCurrentVersion (ID);
		if (upgradedUnit == 0)
			continue; // skip this upgrade, because there is no such unitData

		const int costs = getUpgradeCosts (ID, player, newDamage, newMaxShots, newRange, newMaxAmmo, newArmor, newMaxHitPoints, newScan, newMaxSpeed);
		if (costs <= player.getCredits())
		{
			// update the unitData of the player and send an ack-msg
			// for this upgrade to the player
			upgradedUnit->setDamage (newDamage);
			upgradedUnit->setShotsMax (newMaxShots);
			upgradedUnit->setRange (newRange);
			upgradedUnit->setAmmoMax (newMaxAmmo);
			upgradedUnit->setArmor (newArmor);
			upgradedUnit->setHitpointsMax (newMaxHitPoints);
			upgradedUnit->setScan (newScan);
			if (ID.isAVehicle()) upgradedUnit->setSpeedMax (newMaxSpeed);
			upgradedUnit->setVersion (upgradedUnit->getVersion() + 1);

			player.setCredits (player.getCredits() - costs);
			updateCredits = true;

			sendUnitUpgrades (*this, *upgradedUnit, player);
		}
	}
	if (updateCredits)
		sendCredits (*this, player.getCredits(), player);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_BUILDING_UPGRADE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_BUILDING_UPGRADE);

	const unsigned int unitID = message.popInt32();
	const bool upgradeAll = message.popBool();

	cBuilding* building = getBuildingFromID (unitID);
	cPlayer* player = (building != 0) ? building->getOwner() : 0;
	if (player == 0)
		return;

	const int availableMetal = building->subBase->getMetalStored();

	const cDynamicUnitData& upgradedVersion = *player->getUnitDataCurrentVersion (building->data.getId());
	if (building->data.getVersion() >= upgradedVersion.getVersion())
		return; // already up to date
	cUpgradeCalculator& uc = cUpgradeCalculator::instance();
	const int upgradeCostPerBuilding = uc.getMaterialCostForUpgrading (upgradedVersion.getBuildCost());
	int totalCosts = 0;
	std::vector<cBuilding*> upgradedBuildings;

	// in any case update the selected building
	if (availableMetal >= totalCosts + upgradeCostPerBuilding)
	{
		upgradedBuildings.push_back (building);
		totalCosts += upgradeCostPerBuilding;
	}

	if (upgradeAll)
	{
		cSubBase* subBase = building->subBase;
		for (unsigned int subBaseBuildIdx = 0; subBaseBuildIdx < subBase->getBuildings().size(); subBaseBuildIdx++)
		{
			cBuilding* otherBuilding = subBase->getBuildings()[subBaseBuildIdx];
			if (otherBuilding == building)
				continue;
			if (otherBuilding->data.getId() != building->data.getId())
				continue;
			if (otherBuilding->data.getVersion() >= upgradedVersion.getVersion())
				continue;
			upgradedBuildings.push_back (otherBuilding);
			totalCosts += upgradeCostPerBuilding;
			if (availableMetal < totalCosts + upgradeCostPerBuilding)
				break; // no more raw material left...
		}
	}

	if (totalCosts > 0)
		building->subBase->addMetal (-totalCosts);
	if (upgradedBuildings.empty() == false)
	{
		bool scanNecessary = false;
		bool refreshSentry = false;
		for (size_t i = 0; i != upgradedBuildings.size(); ++i)
		{
			// Scan range was upgraded. So trigger a scan.
			if (!scanNecessary && upgradedBuildings[i]->data.getScan() < upgradedVersion.getScan())
				scanNecessary = true;
			if (upgradedBuildings[i]->isSentryActive() && upgradedBuildings[i]->data.getRange() < upgradedVersion.getRange())
				refreshSentry = true;

			upgradedBuildings[i]->upgradeToCurrentVersion();
		}
		sendUpgradeBuildings (*this, upgradedBuildings, totalCosts, *player);
		if (scanNecessary)
			player->doScan();
		if (refreshSentry)
		{
			player->refreshSentryAir();
			player->refreshSentryGround();
		}
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_RESEARCH_CHANGE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_RESEARCH_CHANGE);

	const int iPlayerNr = message.popInt16();
	auto& player = getPlayerFromNumber (iPlayerNr);
	int newUsedResearch = 0;
	int newResearchSettings[cResearch::kNrResearchAreas];
	for (int area = cResearch::kNrResearchAreas - 1; area >= 0; --area)
	{
		newResearchSettings[area] = message.popInt16();
		newUsedResearch += newResearchSettings[area];
	}
	if (newUsedResearch > player.getResearchCentersWorkingTotal())
		return; // can't turn on research centers automatically!

	// needed, if newUsedResearch < player->ResearchCount
	std::vector<cBuilding*> researchCentersToStop;
	std::vector<cBuilding*> researchCentersToChangeArea;
	std::vector<cResearch::ResearchArea> newAreasForResearchCenters;

	bool error = false;
	const auto buildings = player.getBuildings();
	auto currentBuildingIter = buildings.begin();
	for (int newArea = 0; newArea != cResearch::kNrResearchAreas; ++newArea)
	{
		int centersToAssign = newResearchSettings[newArea];
		for (; currentBuildingIter != buildings.end() && centersToAssign > 0; ++currentBuildingIter)
		{
			auto& building = *currentBuildingIter;
			if (building->getStaticUnitData().canResearch && building->isUnitWorking())
			{
				researchCentersToChangeArea.push_back (building.get());
				newAreasForResearchCenters.push_back ((cResearch::ResearchArea)newArea);
				--centersToAssign;
			}
		}
		if (centersToAssign > 0)
		{
			error = true; // not enough active research centers!
			break;
		}
	}
	// shut down unused research centers
	for (; currentBuildingIter != buildings.end(); ++currentBuildingIter)
	{
		auto& building = *currentBuildingIter;
		if (building->getStaticUnitData().canResearch && building->isUnitWorking())
			researchCentersToStop.push_back (building.get());
	}
	if (error)
		return;

	for (size_t i = 0; i != researchCentersToStop.size(); ++i)
		researchCentersToStop[i]->stopWork (false);

	for (size_t i = 0; i != researchCentersToChangeArea.size(); ++i)
		researchCentersToChangeArea[i]->setResearchArea (newAreasForResearchCenters[i]);
	player.refreshResearchCentersWorkingOnArea();

	sendResearchSettings (*this, researchCentersToChangeArea, newAreasForResearchCenters, player);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_AUTOMOVE_STATUS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_AUTOMOVE_STATUS);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle) Vehicle->hasAutoMoveJob = message.popBool();
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_COM_ACTION (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_COM_ACTION);

	cVehicle* srcVehicle = getVehicleFromID (message.popInt16());
	if (!srcVehicle) return;

	cVehicle* destVehicle = nullptr;
	cBuilding* destBuilding = nullptr;
	if (message.popBool()) destVehicle = getVehicleFromID (message.popInt16());
	else destBuilding = getBuildingFromID (message.popInt16());
	cUnit* destUnit = destVehicle;
	if (destUnit == nullptr) destUnit = destBuilding;
	const bool steal = message.popBool();
	// check whether the commando action is possible
	if (! ((destUnit && srcVehicle->canDoCommandoAction (destUnit->getPosition(), *Map, steal)) ||
		   (destBuilding && destBuilding->getIsBig() && srcVehicle->canDoCommandoAction (destUnit->getPosition() + cPosition (0, 1), *Map, steal)) ||
		   (destBuilding && destBuilding->getIsBig() && srcVehicle->canDoCommandoAction (destUnit->getPosition() + cPosition (1, 0), *Map, steal)) ||
		   (destBuilding && destBuilding->getIsBig() && srcVehicle->canDoCommandoAction (destUnit->getPosition() + cPosition (1, 1), *Map, steal)))) return;

	// check whether the action is successful or not
	const int chance = srcVehicle->calcCommandoChance (destUnit, steal);
	bool success = false;
	if (random (100) < chance)
	{
		if (steal)
		{
			if (destVehicle)
			{
				// change the owner
				if (destVehicle->isUnitBuildingABuilding()) stopVehicleBuilding (*destVehicle);
				if (destVehicle->getMoveJob()) destVehicle->getMoveJob()->stop();
				changeUnitOwner (*destVehicle, *srcVehicle->getOwner());
			}
		}
		else
		{
			// only on disabling units the infiltrator gets exp.
			// As higher his level is as slower he rises onto the next one.
			// every 5 rankings he needs one successful disabling more,
			// to get to the next ranking
			srcVehicle->setCommandoRank (srcVehicle->getCommandoRank() +  1.f / (((int) srcVehicle->getCommandoRank() + 5) / 5));

			const int strength = srcVehicle->calcCommandoTurns (destUnit);
			// stop the unit and make it disabled
			destUnit->setDisabledTurns (strength);
			if (destVehicle)
			{
				if (destVehicle->isUnitBuildingABuilding()) stopVehicleBuilding (*destVehicle);
				if (destVehicle->getMoveJob()) destVehicle->getMoveJob()->stop();
			}
			else if (destBuilding)
			{
				destBuilding->wasWorking = destBuilding->isUnitWorking();
				destBuilding->stopWork (true);
				//sendDoStopWork (*this, *destBuilding);
			}
			//sendUnitData (*this, *destUnit);
			destUnit->getOwner()->doScan();
			checkPlayerUnits();
		}
		success = true;
	}
	// disabled units fail to detect infiltrator even if he screws up
	else if (destUnit->isDisabled() == false)
	{
		// detect the infiltrator on failed action
		// and let enemy units fire on him
		// TODO: uncover the infiltrator for all players,
		// or only for the owner of the target unit? --eiko
		for (size_t i = 0; i != playerList.size(); ++i)
		{
			auto& player = *playerList[i];
			if (&player == srcVehicle->getOwner()) continue;
			if (!player.canSeeAnyAreaUnder (*srcVehicle)) continue;

			srcVehicle->setDetectedByPlayer (*this, &player);
		}
		checkPlayerUnits();
		srcVehicle->InSentryRange (*this);
	}
	srcVehicle->data.setShots (srcVehicle->data.getShots() - 1);
	//sendUnitData (*this, *srcVehicle);
	sendCommandoAnswer (*this, success, steal, *srcVehicle, *srcVehicle->getOwner());
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_REQUEST_CASUALTIES_REPORT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REQUEST_CASUALTIES_REPORT);

	auto& player = getPlayerFromNumber (message.iPlayerNr);

	//sendCasualtiesReport (*this, &player);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_SELFDESTROY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_SELFDESTROY);

	cBuilding* building = getBuildingFromID (message.popInt16());
	if (!building || building->getOwner()->getId() != message.iPlayerNr) return;

	if (building->isBeeingAttacked()) return;

	sendSelfDestroy (*this, *building);
	destroyUnit (*building);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_CHANGE_UNIT_NAME (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_CHANGE_UNIT_NAME);

	const int unitID = message.popInt16();
	cUnit* unit = getUnitFromID (unitID);

	if (unit == 0) return;

	unit->changeName (message.popString());
	//sendUnitData (*this, *unit);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_END_MOVE_ACTION (cNetMessage& message)
{
	assert (message.iType == GAME_EV_END_MOVE_ACTION);

	cVehicle* vehicle = getVehicleFromID (message.popInt32());
	//if (!vehicle || !vehicle->ServerMoveJob) return;

	const int destID = message.popInt32();
	eEndMoveActionType type = (eEndMoveActionType) message.popChar();
	//vehicle->ServerMoveJob->addEndAction (destID, type);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_KICK_PLAYER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_KICK_PLAYER);

	auto& player = getPlayerFromNumber (message.popInt32());

	kickPlayer (player);
}

//------------------------------------------------------------------------------
int cServer::handleNetMessage (cNetMessage& message)
{
	//if (message.iType != NET_GAME_TIME_CLIENT)
	{
		Log.write ("Server: <-- Player " + iToStr (message.iPlayerNr) + " "
				   + message.getTypeAsString()
				   + ", gameTime:" + iToStr(model.getGameTime())
				   + ", Hexdump: " + message.getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);
	}

	switch (message.iType)
	{
		case GAME_EV_WANT_ATTACK: handleNetMessage_GAME_EV_WANT_ATTACK (message); break;
		case GAME_EV_MINELAYERSTATUS: handleNetMessage_GAME_EV_MINELAYERSTATUS (message); break;
		case GAME_EV_WANT_BUILD: handleNetMessage_GAME_EV_WANT_BUILD (message); break;
		case GAME_EV_END_BUILDING: handleNetMessage_GAME_EV_END_BUILDING (message); break;
		case GAME_EV_WANT_STOP_BUILDING: handleNetMessage_GAME_EV_WANT_STOP_BUILDING (message); break;
		case GAME_EV_WANT_BUILDLIST: handleNetMessage_GAME_EV_WANT_BUILDLIST (message); break;
		case GAME_EV_WANT_EXIT_FIN_VEH: handleNetMessage_GAME_EV_WANT_EXIT_FIN_VEH (message); break;
		case GAME_EV_CHANGE_RESOURCES : handleNetMessage_GAME_EV_CHANGE_RESOURCES (message); break;
		case GAME_EV_WANT_CHANGE_MANUAL_FIRE: handleNetMessage_GAME_EV_WANT_CHANGE_MANUAL_FIRE (message); break;
		case GAME_EV_WANT_CHANGE_SENTRY: handleNetMessage_GAME_EV_WANT_CHANGE_SENTRY (message); break;
		case GAME_EV_WANT_MARK_LOG: handleNetMessage_GAME_EV_WANT_MARK_LOG (message); break;
		case GAME_EV_WANT_SUPPLY: handleNetMessage_GAME_EV_WANT_SUPPLY (message); break;
		case GAME_EV_WANT_VEHICLE_UPGRADE: handleNetMessage_GAME_EV_WANT_VEHICLE_UPGRADE (message); break;
		case GAME_EV_WANT_START_CLEAR: handleNetMessage_GAME_EV_WANT_START_CLEAR (message); break;
		case GAME_EV_WANT_STOP_CLEAR: handleNetMessage_GAME_EV_WANT_STOP_CLEAR (message); break;
		case GAME_EV_ABORT_WAITING: handleNetMessage_GAME_EV_ABORT_WAITING (message); break;
		case GAME_EV_WANT_LOAD: handleNetMessage_GAME_EV_WANT_LOAD (message); break;
		case GAME_EV_WANT_EXIT: handleNetMessage_GAME_EV_WANT_EXIT (message); break;
		case GAME_EV_WANT_BUY_UPGRADES: handleNetMessage_GAME_EV_WANT_BUY_UPGRADES (message); break;
		case GAME_EV_WANT_BUILDING_UPGRADE: handleNetMessage_GAME_EV_WANT_BUILDING_UPGRADE (message); break;
		case GAME_EV_WANT_RESEARCH_CHANGE: handleNetMessage_GAME_EV_WANT_RESEARCH_CHANGE (message); break;
		case GAME_EV_AUTOMOVE_STATUS: handleNetMessage_GAME_EV_AUTOMOVE_STATUS (message); break;
		case GAME_EV_WANT_COM_ACTION: handleNetMessage_GAME_EV_WANT_COM_ACTION (message); break;
		case GAME_EV_REQUEST_CASUALTIES_REPORT: handleNetMessage_GAME_EV_REQUEST_CASUALTIES_REPORT (message); break;
		case GAME_EV_WANT_SELFDESTROY: handleNetMessage_GAME_EV_WANT_SELFDESTROY (message); break;
		case GAME_EV_WANT_CHANGE_UNIT_NAME: handleNetMessage_GAME_EV_WANT_CHANGE_UNIT_NAME (message); break;
		case GAME_EV_END_MOVE_ACTION: handleNetMessage_GAME_EV_END_MOVE_ACTION (message); break;
		case GAME_EV_WANT_KICK_PLAYER: handleNetMessage_GAME_EV_WANT_KICK_PLAYER (message); break;
		default:
			Log.write ("Server: Can not handle message, type " + message.getTypeAsString(), cLog::eLOG_TYPE_NET_ERROR);
			break;
	}

	CHECK_MEMORY;
	return 0;
}

//------------------------------------------------------------------------------
int cServer::getUpgradeCosts (const sID& ID, cPlayer& player,
							  int newDamage, int newMaxShots, int newRange,
							  int newMaxAmmo, int newArmor, int newMaxHitPoints,
							  int newScan, int newMaxSpeed)
{
	const bool bVehicle = ID.isAVehicle();
	const cDynamicUnitData* currentVersion = player.getUnitDataCurrentVersion (ID);
	const cDynamicUnitData* startVersion = &model.getUnitsData()->getDynamicUnitData(ID, player.getClan());
	if (currentVersion == 0 || startVersion == 0)
		return 1000000; // error (unbelievably high cost...)

	int cost = 0;
	const cUpgradeCalculator& uc = cUpgradeCalculator::instance();
	if (newDamage > currentVersion->getDamage())
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->getDamage(), currentVersion->getDamage(), newDamage, cUpgradeCalculator::kAttack, player.getResearchState());
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newMaxShots > currentVersion->getShotsMax())
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->getShotsMax(), currentVersion->getShotsMax(), newMaxShots, cUpgradeCalculator::kShots, player.getResearchState());
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newRange > currentVersion->getRange())
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->getRange(), currentVersion->getRange(), newRange, cUpgradeCalculator::kRange, player.getResearchState());
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newMaxAmmo > currentVersion->getAmmoMax())
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->getAmmoMax(), currentVersion->getAmmoMax(), newMaxAmmo, cUpgradeCalculator::kAmmo, player.getResearchState());
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newArmor > currentVersion->getArmor())
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->getArmor(), currentVersion->getArmor(), newArmor, cUpgradeCalculator::kArmor, player.getResearchState());
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newMaxHitPoints > currentVersion->getHitpointsMax())
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->getHitpointsMax(), currentVersion->getHitpointsMax(), newMaxHitPoints, cUpgradeCalculator::kHitpoints, player.getResearchState());
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newScan > currentVersion->getScan())
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->getScan(), currentVersion->getScan(), newScan, cUpgradeCalculator::kScan, player.getResearchState());
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (bVehicle && newMaxSpeed > currentVersion->getSpeedMax())
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->getSpeedMax() / 4, currentVersion->getSpeedMax() / 4, newMaxSpeed / 4, cUpgradeCalculator::kSpeed, player.getResearchState());
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}

	return cost;
}

//------------------------------------------------------------------------------
void cServer::deleteUnit (cUnit* unit, bool notifyClient)
{
	if (unit == 0)
		return;

	if (unit->isABuilding() && unit->getOwner() == 0)
	{
		deleteRubble (static_cast<cBuilding*> (unit));
		return;
	}

	cPlayer* owner = unit->getOwner();

	if (unit->getOwner() && casualtiesTracker != nullptr && ((unit->isABuilding() && unit->data.getBuildCost() <= 2) == false))
		casualtiesTracker->logCasualty (unit->data.getId(), unit->getOwner()->getId());

	std::shared_ptr<cUnit> owningPtr; // keep owning ptr to make sure that unit instance will outlive this method.
	if (unit->isABuilding())
	{
		cBuilding* building = static_cast<cBuilding*> (unit);
		owningPtr = owner->removeUnit (*building);
	}
	else
	{
		cVehicle* vehicle = static_cast<cVehicle*> (unit);
		owningPtr = owner->removeUnit (*vehicle);
	}

	helperJobs.onRemoveUnit (unit);

	// detach from move job
	if (unit->isAVehicle())
	{
		/*cVehicle* vehicle = static_cast<cVehicle*> (unit);
		if (vehicle->ServerMoveJob)
		{
			vehicle->ServerMoveJob->Vehicle = nullptr;
		}*/
	}

	// remove from sentry list
	unit->getOwner()->deleteSentry (*unit);

	// lose eco points
	if (unit->isABuilding() && static_cast<cBuilding*> (unit)->points != 0)
	{
		unit->getOwner()->setScore (unit->getOwner()->getScore (turnClock->getTurn()) - static_cast<cBuilding*> (unit)->points, turnClock->getTurn());
		sendScore (*this, *unit->getOwner(), turnClock->getTurn());
		sendNumEcos (*this, *unit->getOwner());
	}

	if (unit->isABuilding())
		Map->deleteBuilding (*static_cast<cBuilding*> (unit));
	else
		Map->deleteVehicle (*static_cast<cVehicle*> (unit));

	if (notifyClient)
		sendDeleteUnit (*this, *unit, nullptr);

	if (unit->isABuilding() && static_cast<cBuilding*> (unit)->subBase != 0)
		unit->getOwner()->base.deleteBuilding (static_cast<cBuilding*> (unit), *Map);

	if (owner != 0)
	{
		owner->doScan();
	}
}

//------------------------------------------------------------------------------
void cServer::checkPlayerUnits (cVehicle& vehicle, cPlayer& MapPlayer)
{
	if (&MapPlayer == vehicle.getOwner()) return;

	std::vector<cPlayer*>& seenByPlayers = vehicle.seenByPlayerList;
	const bool stealthUnit = vehicle.getStaticUnitData().isStealthOn != TERRAIN_NONE;

// 	if (MapPlayer.canSeeAnyAreaUnder (vehicle) && !vehicle.isUnitLoaded() &&
// 		(!stealthUnit || vehicle.isDetectedByPlayer (&MapPlayer) || (MapPlayer.isDefeated && openMapDefeat)))
// 	{
// 		if (Contains (seenByPlayers, &MapPlayer) == false)
// 		{
// 			seenByPlayers.push_back (&MapPlayer);
// 			sendAddEnemyUnit (*this, vehicle, MapPlayer);
// 			sendUnitData (*this, vehicle, MapPlayer);
// 			if (vehicle.ServerMoveJob)
// 			{
// 				sendMoveJobServer (*this, *vehicle.ServerMoveJob, MapPlayer);
// 				if (Contains (ActiveMJobs, vehicle.ServerMoveJob) && !vehicle.ServerMoveJob->bFinished && !vehicle.ServerMoveJob->bEndForNow && vehicle.isUnitMoving())
// 				{
// 					Log.write (" Server: sending extra MJOB_OK for unit ID " + iToStr (vehicle.iID) + " to client " + iToStr (MapPlayer.getId()), cLog::eLOG_TYPE_NET_DEBUG);
// 					auto message = std::make_unique<cNetMessage> (GAME_EV_NEXT_MOVE);
// 					message->pushChar (MJOB_OK);
// 					message->pushInt16 (vehicle.iID);
// 					sendNetMessage (std::move (message), &MapPlayer);
// 				}
// 			}
// 		}
// 	}
// 	else
// 	{
// 		std::vector<cPlayer*>::iterator it = std::find (seenByPlayers.begin(), seenByPlayers.end(), &MapPlayer);
// 
// 		if (it != seenByPlayers.end())
// 		{
// 			seenByPlayers.erase (it);
// 			sendDeleteUnit (*this, vehicle, &MapPlayer);
// 		}
// 	}
}

//------------------------------------------------------------------------------
void cServer::checkPlayerUnits (cBuilding& building, cPlayer& MapPlayer)
{
	if (&MapPlayer == building.getOwner()) return;
	std::vector<cPlayer*>& seenByPlayers = building.seenByPlayerList;
	const bool stealthUnit = building.getStaticUnitData().isStealthOn != TERRAIN_NONE;

	if (MapPlayer.canSeeAnyAreaUnder (building) &&
		(!stealthUnit || building.isDetectedByPlayer (&MapPlayer) || (MapPlayer.isDefeated && openMapDefeat)))
	{
		if (Contains (seenByPlayers, &MapPlayer) == false)
		{
			seenByPlayers.push_back (&MapPlayer);
			//sendAddEnemyUnit (*this, building, MapPlayer);
			//sendUnitData (*this, building, MapPlayer);
		}
	}
	else
	{
		std::vector<cPlayer*>::iterator it = std::find (seenByPlayers.begin(), seenByPlayers.end(), &MapPlayer);

		if (it != seenByPlayers.end())
		{
			seenByPlayers.erase (it);
			sendDeleteUnit (*this, building, &MapPlayer);
		}
	}
}

//------------------------------------------------------------------------------
void cServer::checkPlayerRubbles (cBuilding& building, cPlayer& MapPlayer)
{
	std::vector<cPlayer*>& seenByPlayers = building.seenByPlayerList;

	if (MapPlayer.canSeeAnyAreaUnder (building))
	{
		if (Contains (seenByPlayers, &MapPlayer) == false)
		{
			seenByPlayers.push_back (&MapPlayer);
			sendAddRubble (*this, building, MapPlayer);
		}
	}
	else
	{
		std::vector<cPlayer*>::iterator it = std::find (seenByPlayers.begin(), seenByPlayers.end(), &MapPlayer);

		if (it != seenByPlayers.end())
		{
			seenByPlayers.erase (it);
			sendDeleteUnit (*this, building, &MapPlayer);
		}
	}
}

//------------------------------------------------------------------------------
void cServer::checkPlayerUnits()
{
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		// The player who's unit is it
		cPlayer& unitPlayer = *playerList[i];
		const auto& vehicles = unitPlayer.getVehicles();
		for (auto i = vehicles.begin(); i != vehicles.end(); ++i)
		{
			const auto& vehicle = *i;
			for (size_t j = 0; j != playerList.size(); ++j)
			{
				checkPlayerUnits (*vehicle, *playerList[j]);
			}
		}
		const auto& buildings = unitPlayer.getBuildings();
		for (auto i = buildings.begin(); i != buildings.end(); ++i)
		{
			const auto& building = *i;
			for (size_t j = 0; j != playerList.size(); ++j)
			{
				checkPlayerUnits (*building, *playerList[j]);
			}
		}
	}

	//check the neutral objects
	for (auto i = neutralBuildings.begin(); i != neutralBuildings.end(); ++i)
	{
		const auto& building = *i;
		for (size_t i = 0; i != playerList.size(); ++i)
		{
			checkPlayerRubbles (*building, *playerList[i]);
		}
	}
}

//------------------------------------------------------------------------------
bool cServer::isPlayerDisconnected (const cPlayer& player) const
{
	/*if (player.isLocal()) return false;

	if (network)
		return !network->isConnected (player.getSocketNum());
*/
	return true;
}

//------------------------------------------------------------------------------
void cServer::kickPlayer (cPlayer& player)
{
	// close the socket
	/*const int socketIndex = player.getSocketNum();
	if (network) network->close (socketIndex);
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		playerList[i]->onSocketIndexDisconnected (socketIndex);
	}
	deletePlayer (player);
	*/
}

//------------------------------------------------------------------------------
void cServer::markAllPlayersAsDisconnected()
{
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		cPlayer* player = playerList[i].get();
		if (Contains (DisconnectedPlayerList, player) == false)
			DisconnectedPlayerList.push_back (player);
		player->revealMap();
	}
}

//------------------------------------------------------------------------------
cPlayer& cServer::getPlayerFromNumber (int playerNumber)
{
	const auto& constMe = *this;
	return const_cast<cPlayer&> (constMe.getPlayerFromNumber (playerNumber));
}

//------------------------------------------------------------------------------
const cPlayer& cServer::getPlayerFromNumber (int playerNumber) const
{
	auto iter = std::find_if (playerList.begin(), playerList.end(), [playerNumber] (const std::unique_ptr<cPlayer>& player) { return player->getId() == playerNumber; });
	if (iter == playerList.end()) throw std::runtime_error ("Could not find player with number '" + iToStr (playerNumber) + "'");

	auto& player = *iter;

	assert (player != nullptr);

	return *player;
}

//------------------------------------------------------------------------------
cPlayer* cServer::getPlayerFromString (const std::string& playerID)
{
	// first try to find player by number
	const int playerNr = atoi (playerID.c_str());
	if (playerNr != 0 || playerID[0] == '0')
	{
		return &getPlayerFromNumber (playerNr);
	}

	// try to find player by name
	auto iter = std::find_if (playerList.begin(), playerList.end(), [&playerID] (const std::unique_ptr<cPlayer>& player) { return player->getName() == playerID; });
	return iter == playerList.end() ? nullptr : iter->get();
}

//------------------------------------------------------------------------------
void cServer::setActiveTurnPlayer (const cPlayer& player)
{
	for (size_t i = 0; i < playerList.size(); ++i)
	{
		if (playerList[i]->getId() == player.getId())
		{
			activeTurnPlayer = playerList[i].get();
		}
	}
}

//------------------------------------------------------------------------------
cPlayer* cServer::getActiveTurnPlayer()
{
	return activeTurnPlayer;
}

//------------------------------------------------------------------------------
bool cServer::isVictoryConditionMet() const
{
	switch (gameSettings->getVictoryCondition())
	{
		case eGameSettingsVictoryCondition::Turns:
			return turnClock->getTurn() >= static_cast<int> (gameSettings->getVictoryTurns());
		case eGameSettingsVictoryCondition::Points:
		{
			for (size_t i = 0; i != playerList.size(); ++i)
			{
				const cPlayer& player = *playerList[i];
				if (player.isDefeated) continue;
				if (player.getScore (turnClock->getTurn()) >= static_cast<int> (gameSettings->getVictoryPoints())) return true;
			}
			return false;
		}
		case eGameSettingsVictoryCondition::Death:
		{
			int nbActivePlayer = 0;
			for (size_t i = 0; i != playerList.size(); ++i)
			{
				const cPlayer& player = *playerList[i];
				if (player.isDefeated) continue;
				++nbActivePlayer;
				if (nbActivePlayer >= 2) return false;
			}
			return nbActivePlayer == 1;
		}
	}
	assert (0);
	return false;
}

//------------------------------------------------------------------------------
void cServer::defeatLoserPlayers()
{
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		cPlayer& player = *playerList[i];
		if (player.isDefeated) continue;
		if (player.mayHaveOffensiveUnit()) continue;

		player.isDefeated = true;
		sendDefeated (*this, player);
		if (openMapDefeat == false)

			player.revealMap();
		checkPlayerUnits();
		sendNoFog (*this, player);
	}
}

//------------------------------------------------------------------------------
void cServer::checkDefeats()
{
	defeatLoserPlayers();
	if (isVictoryConditionMet() == false) return;

	std::set<cPlayer*> winners;
	int best_score = -1;

	for (size_t i = 0; i != playerList.size(); ++i)
	{
		cPlayer& player = *playerList[i];
		if (player.isDefeated) continue;
		const int score = player.getScore (turnClock->getTurn());

		if (score < best_score) continue;

		if (score > best_score)
		{
			winners.clear();
			best_score = score;
		}
		winners.insert (&player);
	}

	// anyone who hasn't won has lost.
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		cPlayer& player = *playerList[i];

		if (player.isDefeated) continue;
		if (winners.find (&player) != winners.end()) continue;

		player.isDefeated = true;
		sendDefeated (*this, player);

		if (openMapDefeat)
		{
			player.revealMap();
			checkPlayerUnits();
			sendNoFog (*this, player);
		}
	}

	// Handle the case where there is more than one winner.
	// Original MAX calls a draw and displays the results screen.
	// For now we will have sudden death,
	// i.e. first player to get ahead in score wins.
	if (winners.size() > 1)
	{
		// TODO: reimplement
		//for (size_t i = 0; i != playerList.size (); ++i)
		//	sendSavedReport (*this, cSavedReportTranslated ("Text~Comp~SuddenDeath"), playerList[i].get ());
	}
	// TODO: send win message to the winner.
}

//------------------------------------------------------------------------------
cUnit* cServer::getUnitFromID (unsigned int iID) const
{
	cUnit* result = getVehicleFromID (iID);
	if (result == nullptr)
		result = getBuildingFromID (iID);
	return result;
}

//------------------------------------------------------------------------------
cVehicle* cServer::getVehicleFromID (unsigned int id) const
{
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		auto unit = playerList[i]->getVehicleFromId (id);
		if (unit) return unit;
	}
	return 0;
}

//------------------------------------------------------------------------------
cBuilding* cServer::getBuildingFromID (unsigned int id) const
{
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		auto unit = playerList[i]->getBuildingFromId (id);
		if (unit) return unit;
	}
	return 0;
}

//------------------------------------------------------------------------------
void cServer::destroyUnit (cUnit& unit)
{
	bool isVehicle = false;
	const auto position = unit.getPosition();
	auto& field = Map->getField (position);

	//delete planes immediately
	if (unit.isAVehicle())
	{
		isVehicle = true;
		auto& vehicle = static_cast<cVehicle&> (unit);
		if (vehicle.getStaticUnitData().factorAir > 0 && vehicle.getFlightHeight() > 0)
		{
			deleteUnit (&vehicle);
			return;
		}
	}

	//check, if there is a big unit on the field
	bool bigUnit = false;
	auto topBuilding = field.getTopBuilding();
	if ((topBuilding && topBuilding->getIsBig()) || unit.getIsBig())
		bigUnit = true;

	//delete unit
	int rubbleValue = 0;
	if (!unit.getStaticUnitData().isHuman)
	{
		rubbleValue += unit.data.getBuildCost();
		// stored material is always added completely to the rubble
		if (unit.getStaticUnitData().storeResType == eResourceType::Metal)
			rubbleValue += unit.getStoredResources() * 2;
	}
	deleteUnit (&unit);

	//delete all buildings (except connectors, when a vehicle is destroyed)
	rubbleValue += deleteBuildings (field, !isVehicle);
	if (bigUnit)
	{
		rubbleValue += deleteBuildings (Map->getField (position + cPosition (1, 0)), !isVehicle);
		rubbleValue += deleteBuildings (Map->getField (position + cPosition (0, 1)), !isVehicle);
		rubbleValue += deleteBuildings (Map->getField (position + cPosition (1, 1)), !isVehicle);
	}

	//add rubble
	auto rubble = field.getRubble();
	if (rubble)
	{
/*		rubble->RubbleValue += rubbleValue / 2;
		if (rubble->getIsBig())
			rubble->RubbleTyp = random (2);
		else
			rubble->RubbleTyp = random (5); */
	}
	else
	{
		if (rubbleValue > 2)
			addRubble (position, rubbleValue / 2, bigUnit);
	}
}

//------------------------------------------------------------------------------
int cServer::deleteBuildings (cMapField& field, bool deleteConnector)
{
	auto buildings = field.getBuildings();
	int rubble = 0;

	for (auto b_it = buildings.begin(); b_it != buildings.end(); ++b_it)
	{
		if ((*b_it)->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE && deleteConnector == false) continue;
		if ((*b_it)->getOwner() == nullptr) continue;

		if ((*b_it)->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE) //no rubble for connectors
			rubble += (*b_it)->data.getBuildCost();
		if ((*b_it)->getStaticUnitData().storeResType == eResourceType::Metal)
			rubble += (*b_it)->getStoredResources() * 2; // stored material is always added completely to the rubble

		deleteUnit (*b_it);
	}

	return rubble;
}

//------------------------------------------------------------------------------
void cServer::addRubble (const cPosition& position, int value, bool big)
{
	value = std::max (1, value);

	if (Map->isWaterOrCoast (position))
	{
		if (big)
		{
			addRubble (position + cPosition (1, 0), value / 4, false);
			addRubble (position + cPosition (0, 1), value / 4, false);
			addRubble (position + cPosition (1, 1), value / 4, false);
		}
		return;
	}

	if (big && Map->isWaterOrCoast (position + cPosition (1, 0)))
	{
		addRubble (position,                   value / 4, false);
		addRubble (position + cPosition (0, 1), value / 4, false);
		addRubble (position + cPosition (1, 1), value / 4, false);
		return;
	}

	if (big && Map->isWaterOrCoast (position + cPosition (0, 1)))
	{
		addRubble (position,                   value / 4, false);
		addRubble (position + cPosition (1, 0), value / 4, false);
		addRubble (position + cPosition (1, 1), value / 4, false);
		return;
	}

	if (big && Map->isWaterOrCoast (position + cPosition (1, 1)))
	{
		addRubble (position,                   value / 4, false);
		addRubble (position + cPosition (1, 0), value / 4, false);
		addRubble (position + cPosition (0, 1), value / 4, false);
		return;
	}

	auto rubble = std::make_shared<cBuilding> (nullptr, nullptr, nullptr, iNextUnitID);

	iNextUnitID++;

	rubble->setPosition (position);

	rubble->setIsBig(big);
	//rubble->RubbleValue = value;

	Map->addBuilding (*rubble, position);

	/*if (big)
	{
		rubble->RubbleTyp = random (2);
	}
	else
	{
		rubble->RubbleTyp = random (5);
	}*/
	neutralBuildings.insert (std::move (rubble));
}

//------------------------------------------------------------------------------
void cServer::deleteRubble (cBuilding* rubble)
{
	Map->deleteBuilding (*rubble);

	auto iter = neutralBuildings.find (*rubble);
	assert (iter != neutralBuildings.end());

	if (iter != neutralBuildings.end())
	{
		auto owningUnitPtr = *iter;

		neutralBuildings.erase (iter);

		sendDeleteUnit (*this, *owningUnitPtr, nullptr);
	}
}

//------------------------------------------------------------------------------
void cServer::deletePlayer (cPlayer& player)
{
	//remove units
	const auto& vehicles = player.getVehicles();
	while (!vehicles.empty())
	{
		deleteUnit (vehicles.begin()->get());
	}
	const auto& buildings = player.getVehicles();
	while (!buildings.empty())
	{
		deleteUnit (buildings.begin()->get());
	}

	// remove the player of all detected by player lists
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		cPlayer* unitPlayer = playerList[i].get();
		if (unitPlayer == &player) continue;

		const auto& vehicles = unitPlayer->getVehicles();
		for (auto i = vehicles.begin(); i != vehicles.end(); ++i)
		{
			const auto& vehicle = *i;
			if (vehicle->getStaticUnitData().isStealthOn != TERRAIN_NONE && vehicle->isDetectedByPlayer (&player)) vehicle->resetDetectedByPlayer (*this, &player);
		}
	}
	// delete the player
	sendDeletePlayer (*this, player, nullptr);
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		if (&player == playerList[i].get())
		{
			playerList.erase (playerList.begin() + i);
			break;
		}
	}
}

//------------------------------------------------------------------------------
void cServer::changeUnitOwner (cVehicle& vehicle, cPlayer& newOwner)
{
	if (vehicle.getOwner() && casualtiesTracker != nullptr)
		casualtiesTracker->logCasualty (vehicle.data.getId(), vehicle.getOwner()->getId());

	// delete vehicle in the list of the old player
	cPlayer* oldOwner = vehicle.getOwner();

	auto owningVehiclePtr = oldOwner->removeUnit (vehicle);
	// add the vehicle to the list of the new player
	vehicle.setOwner (&newOwner);
	newOwner.addUnit (owningVehiclePtr);

	//the vehicle is fully operational for the new owner
	vehicle.setDisabledTurns (0);

	// delete the unit on the clients and add it with new owner again
	sendDeleteUnit (*this, vehicle, oldOwner);
	for (size_t i = 0; i != vehicle.seenByPlayerList.size(); ++i)
	{
		sendDeleteUnit (*this, vehicle, vehicle.seenByPlayerList[i]);
	}
	vehicle.seenByPlayerList.clear();
	vehicle.detectedByPlayerList.clear();
	//sendAddUnit (*this, vehicle.getPosition(), vehicle.iID, true, vehicle.data.getId(), *vehicle.getOwner(), false);
	//sendUnitData (*this, vehicle, *vehicle.getOwner());
	sendSpecificUnitData (*this, vehicle);

	oldOwner->doScan();
	newOwner.doScan();
	checkPlayerUnits();

	// let the unit work for his new owner
	if (vehicle.getStaticUnitData().canSurvey)
	{
		sendVehicleResources (*this, vehicle);
		vehicle.doSurvey ();
	}
	vehicle.makeDetection (*this);
}

//------------------------------------------------------------------------------
void cServer::stopVehicleBuilding (cVehicle& vehicle)
{
	if (!vehicle.isUnitBuildingABuilding()) return;

	vehicle.setBuildingABuilding (false);
	vehicle.BuildPath = false;

	auto position = vehicle.getPosition();

	if (vehicle.getIsBig())
	{
		Map->moveVehicle (vehicle, vehicle.buildBigSavedPosition);
		position = vehicle.buildBigSavedPosition;
		vehicle.getOwner()->doScan();
	}
	sendStopBuild (*this, vehicle.iID, position, *vehicle.getOwner());
	for (size_t i = 0; i != vehicle.seenByPlayerList.size(); ++i)
	{
		sendStopBuild (*this, vehicle.iID, position, *vehicle.seenByPlayerList[i]);
	}
}

void cServer::sideStepStealthUnit (const cPosition& position, const cVehicle& vehicle, const cPosition& bigOffset)
{
	sideStepStealthUnit (position, vehicle.getStaticUnitData(), vehicle.getOwner(), bigOffset);
}

void cServer::sideStepStealthUnit (const cPosition& position, const cStaticUnitData& vehicleData, cPlayer* vehicleOwner, const cPosition& bigOffset)
{
	// TODO: make sure, the stealth vehicle takes the direct diagonal move.
	// Also when two straight moves would be shorter.

	if (vehicleData.factorAir > 0) return;

	// first look for an undetected stealth unit
	cVehicle* stealthVehicle = Map->getField (position).getVehicle();
	if (!stealthVehicle) return;
	if (stealthVehicle->getOwner() == vehicleOwner) return;
	if (stealthVehicle->getStaticUnitData().isStealthOn == TERRAIN_NONE) return;
	if (stealthVehicle->isDetectedByPlayer (vehicleOwner)) return;

	if (stealthVehicle->isUnitMoving())
	{
		stealthVehicle->setDetectedByPlayer(*this, vehicleOwner);
		return;
	}

	// found a stealth unit. Try to find a place where the unit can move
	bool placeFound = false;
	int minCosts = 99;
	cPosition bestPosition;
	const int minx = std::max (position.x() - 1, 0);
	const int maxx = std::min (position.x() + 1, Map->getSize().x() - 1);
	const int miny = std::max (position.y() - 1, 0);
	const int maxy = std::min (position.y() + 1, Map->getSize().y() - 1);
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
				if (currentPosition == bigOffset ||
					currentPosition == bigOffset + cPosition (1, 0) ||
					currentPosition == bigOffset + cPosition (0, 1) ||
					currentPosition == bigOffset + cPosition (1, 1)) continue;
			}

			// check whether this field is a possible destination
			if (!Map->possiblePlace (*stealthVehicle, currentPosition)) continue;

			// check costs of the move
			int costs = cPathCalculator::calcNextCost (position, currentPosition, stealthVehicle, Map.get());
			if (costs > stealthVehicle->data.getSpeed()) continue;

			// check whether the vehicle would be detected
			// on the destination field
			bool detectOnDest = false;
			if (stealthVehicle->getStaticUnitData().isStealthOn & TERRAIN_GROUND)
			{
				for (size_t i = 0; i != playerList.size(); ++i)
				{
					if (playerList[i].get() == stealthVehicle->getOwner()) continue;
					if (playerList[i]->hasLandDetection (currentPosition)) detectOnDest = true;
				}
				if (Map->isWater (currentPosition)) detectOnDest = true;
			}
			if (stealthVehicle->getStaticUnitData().isStealthOn & TERRAIN_SEA)
			{
				for (size_t i = 0; i != playerList.size(); ++i)
				{
					if (playerList[i].get() == stealthVehicle->getOwner()) continue;
					if (playerList[i]->hasSeaDetection (currentPosition)) detectOnDest = true;
				}
				if (!Map->isWater (currentPosition)) detectOnDest = true;

				if (stealthVehicle->getStaticUnitData().factorGround > 0 && stealthVehicle->getStaticUnitData().factorSea > 0)
				{
					cBuilding* b = Map->getField (currentPosition).getBaseBuilding();
					if (b && (b->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_BASE || b->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE_SEA || b->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE_BASE)) detectOnDest = true;
				}
			}
			if (detectOnDest) continue;

			// take the move with the lowest costs.
			// Decide randomly, when costs are equal
			if (costs < minCosts || (costs == minCosts && random (2)))
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
		//addMoveJob (position, bestPosition, stealthVehicle);
		// begin the movement immediately,
		// so no other unit can block the destination field
		return;
	}

	// sidestepping failed. Uncover the vehicle.
	stealthVehicle->setDetectedByPlayer (*this, vehicleOwner);
	checkPlayerUnits();
}

void cServer::addJob (cJob* job)
{
	helperJobs.addJob (*job);
}
void cServer::runJobs()
{
	helperJobs.run (*gameTimer);
}

//------------------------------------------------------------------------------
void cServer::addAttackJob (cUnit* aggressor, const cPosition& targetPosition)
{
	cAttackJob* attackJob = new cAttackJob (this, aggressor, targetPosition);
	AJobs.push_back (attackJob);
}


const cGameGuiState& cServer::getPlayerGameGuiState (const cPlayer& player)
{
	return playerGameGuiStates[player.getId()];
}
