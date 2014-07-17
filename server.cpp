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

#include "server.h"

#include "attackJobs.h"
#include "buildings.h"
#include "casualtiestracker.h"
#include "client.h"
#include "clientevents.h"
#include "clist.h"
#include "gametimer.h"
#include "hud.h"
#include "jobs.h"
#include "log.h"
#include "menuevents.h"
#include "menus.h"
#include "movejobs.h"
#include "netmessage.h"
#include "network.h"
#include "game/data/player/player.h"
#include "savegame.h"
#include "serverevents.h"
#include "settings.h"
#include "upgradecalculator.h"
#include "vehicles.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "game/data/report/savedreport.h"
#include "game/data/report/savedreportsimple.h"
#include "game/data/report/savedreportchat.h"
#include "game/data/report/special/savedreportlostconnection.h"
#include "game/data/report/special/savedreportturnstart.h"
#include "game/logic/turnclock.h"
#include "game/logic/turntimeclock.h"
#include "utility/random.h"

#if DEDICATED_SERVER_APPLICATION
# include "dedicatedserver.h"
#endif

//------------------------------------------------------------------------------
int CallbackRunServerThread (void* arg)
{
	cServer* server = reinterpret_cast<cServer*> (arg);
	server->run();
	return 0;
}

//------------------------------------------------------------------------------
cServer::cServer (std::shared_ptr<cTCP> network_) :
	network (std::move(network_)),
	gameTimer(std::make_shared<cGameTimerServer>()),
	serverThread (nullptr),
	turnClock (std::make_unique<cTurnClock> (1)),
	turnTimeClock (std::make_unique<cTurnTimeClock> (gameTimer)),
	lastTurnEnd (0),
	executingRemainingMovements (false),
	gameSettings (std::make_unique<cGameSettings> ()),
	casualtiesTracker (new cCasualtiesTracker ()),
	serverState (SERVER_STATE_INITGAME)
{
	bExit = false;
	openMapDefeat = true;
	activeTurnPlayer = nullptr;
	iNextUnitID = 1;
	iWantPlayerEndNum = -1;
	savingID = 0;
	savingIndex = -1;

	if (!DEDICATED_SERVER)
	{
		if (network) network->setMessageReceiver (this);
	}

	gameTimer->maxEventQueueSize = MAX_SERVER_EVENT_COUNTER;
	gameTimer->start ();

	// connect to casualties tracker to send updates about casualties changes
	signalConnectionManager.connect (casualtiesTracker->casualtyChanged, [this](const sID unitId, int playerNr)
	{
		// TODO: send updated entry only!
		sendCasualtiesReport (*this, nullptr);
	});

	signalConnectionManager.connect (gameSettings->turnEndDeadlineChanged, [this]()
	{
		for (size_t i = 0; i < playerList.size (); ++i)
		{
			sendGameSettings (*this, *playerList[i]);
		}
		if (turnEndDeadline)
		{
			turnEndDeadline->changeDeadline (gameSettings->getTurnEndDeadline());
		}
	});

	signalConnectionManager.connect (gameSettings->turnEndDeadlineActiveChanged, [this]()
	{
		for (size_t i = 0; i < playerList.size (); ++i)
		{
			sendGameSettings (*this, *playerList[i]);
		}
		if (!gameSettings->isTurnEndDeadlineActive() && turnEndDeadline)
		{
			turnTimeClock->removeDeadline (turnEndDeadline);
			turnEndDeadline = nullptr;
		}
		else if (gameSettings->isTurnEndDeadlineActive () && !turnEndDeadline && !PlayerEndList.empty ())
		{
			turnEndDeadline = turnTimeClock->startNewDeadlineFromNow (gameSettings->getTurnEndDeadline ());
			sendTurnEndDeadlineStartTime (*this, turnEndDeadline->getStartGameTime ());
		}
	});

	signalConnectionManager.connect (gameSettings->turnLimitChanged, [this]()
	{
		for (size_t i = 0; i < playerList.size (); ++i)
		{
			sendGameSettings (*this, *playerList[i]);
		}
		if (turnLimitDeadline)
		{
			turnLimitDeadline->changeDeadline (gameSettings->getTurnLimit ());
		}
	});

	signalConnectionManager.connect (gameSettings->turnLimitActiveChanged, [this]()
	{
		for (size_t i = 0; i < playerList.size (); ++i)
		{
			sendGameSettings (*this, *playerList[i]);
		}
		if (!gameSettings->isTurnLimitActive () && turnLimitDeadline)
		{
			turnTimeClock->removeDeadline (turnLimitDeadline);
			turnLimitDeadline = nullptr;
		}
		else if (gameSettings->isTurnLimitActive () && !turnLimitDeadline)
		{
			turnLimitDeadline = turnTimeClock->startNewDeadlineFrom (turnTimeClock->getStartGameTime(), gameSettings->getTurnLimit ());
		}
	});
}
//------------------------------------------------------------------------------
cServer::~cServer()
{
	stop();

	// disconnect clients
	if (network)
	{
		for (size_t i = 0; i != playerList.size(); ++i)
		{
			network->close (playerList[i]->getSocketNum());
		}
		network->setMessageReceiver (nullptr);
	}

	for (size_t i = 0; i != AJobs.size(); ++i)
	{
		delete AJobs[i];
	}
}

//------------------------------------------------------------------------------
void cServer::setMap (std::shared_ptr<cStaticMap> staticMap)
{
	Map = new cMap (staticMap);

	for (size_t i = 0; i < playerList.size (); ++i)
	{
		playerList[i]->initMaps (*Map);
	}
}

//------------------------------------------------------------------------------
void cServer::addPlayer (std::unique_ptr<cPlayer> player)
{
	if(Map != nullptr) player->initMaps (*Map);

	playerList.push_back (std::move(player));
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
	if (gameSettings->getGameType () == eGameSettingsGameType::HotSeat) return GAME_TYPE_HOTSEAT;
	return GAME_TYPE_SINGLE;
}

//------------------------------------------------------------------------------
void cServer::start ()
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
			SDL_WaitThread (serverThread, NULL);
			serverThread = NULL;
		}
	}
}

//------------------------------------------------------------------------------
void cServer::setTurnEndDeadline (const std::chrono::seconds& deadline)
{
	gameSettings->setTurnEndDeadline (deadline);
}

//------------------------------------------------------------------------------
void cServer::setTurnEndDeadlineActive (bool value)
{
	gameSettings->setTurnEndDeadlineActive (value);
}

//------------------------------------------------------------------------------
void cServer::setTurnLimit (const std::chrono::seconds& deadline)
{
	gameSettings->setTurnLimit (deadline);
}

//------------------------------------------------------------------------------
void cServer::setTurnLimitActive (bool value)
{
	gameSettings->setTurnLimitActive (value);
}

//------------------------------------------------------------------------------
void cServer::pushEvent (std::unique_ptr<cNetMessage> message)
{
	eventQueue.push (std::move(message));
}

//------------------------------------------------------------------------------
void cServer::sendNetMessage (AutoPtr<cNetMessage>& message, const cPlayer* player)
{
	const auto playerNumber = player != nullptr ? player->getNr () : -1;
    const auto playerName = player != nullptr ? player->getName () : "all players";

	message->iPlayerNr = playerNumber;
	message->serialize();
	if (message->iType != NET_GAME_TIME_SERVER)
	{
        Log.write ("Server: --> " + playerName + " (" + iToStr(playerNumber) + ") "
				   + message->getTypeAsString ()
				   + ", gameTime:" + iToStr (this->gameTimer->gameTime)
				   + ", Hexdump: " + message->getHexDump (), cLog::eLOG_TYPE_NET_DEBUG);
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
			if (localClients[i]->getActivePlayer().getNr() == player->getNr())
			{
				localClients[i]->pushEvent (std::unique_ptr<cNetMessage>(message.Release()));
				break;
			}
		}
	}
	// on all other sockets the netMessage will be send over TCP/IP
	else
	{
		if (network) network->sendTo (player->getSocketNum(), message->iLength, message->serialize());
	}
}

//------------------------------------------------------------------------------
void cServer::run()
{
	while (!bExit)
	{
		std::unique_ptr<cNetMessage> message;
		if (eventQueue.try_pop (message))
		{
			handleNetMessage (*message);
			checkPlayerUnits();
		}

		// don't do anything if games hasn't been started yet!
		unsigned int lastTime = gameTimer->gameTime;
		if (serverState == SERVER_STATE_INGAME)
		{
			gameTimer->run (*this);
		}

		// nothing done
		if (!message && lastTime == gameTimer->gameTime)
		{
			SDL_Delay (10);
		}
	}
}

void cServer::doGameActions()
{
	checkDeadline();
	handleMoveJobs();
	runJobs();
	handleWantEnd();
	checkPlayerUnits();
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_TCP_ACCEPT (cNetMessage& message)
{
	assert (message.iType == TCP_ACCEPT);

	sendRequestIdentification (*network, message.popInt16());
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_MU_MSG_CLAN (cNetMessage& message)
{
	assert (message.iType == MU_MSG_CLAN);
	assert (serverState == SERVER_STATE_INITGAME);

	unsigned int playerNr = message.popInt16 ();
	int clanNr = message.popInt16 ();  // -1 = no clan

	auto& player = getPlayerFromNumber (playerNr);
	player.setClan (clanNr);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_MU_MSG_LANDING_VEHICLES (cNetMessage& message)
{
	assert (message.iType == MU_MSG_LANDING_VEHICLES);
	assert (serverState == SERVER_STATE_INITGAME);

	unsigned int playerNr = message.popInt16();
	auto& player = getPlayerFromNumber (playerNr);

	auto& landingUnits = playerLandingUnits[&player];

	int iCount = message.popInt16();
	for (int i = 0; i < iCount; i++)
	{
		sLandingUnit unit;
		unit.cargo = message.popInt16();
		unit.unitID = message.popID();
		landingUnits.push_back (unit);
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_MU_MSG_UPGRADES (cNetMessage& message)
{
	assert (message.iType == MU_MSG_UPGRADES);
	assert (serverState == SERVER_STATE_INITGAME);

	// TODO: check that it is possible (gold...)
	const int playerNr = message.popInt16();
	auto& player = getPlayerFromNumber (playerNr);

	const int count = message.popInt16();
	for (int i = 0; i < count; i++)
	{
		const sID ID = message.popID();
		sUnitData& unitData = *player.getUnitDataCurrentVersion (ID);

		unitData.setDamage(message.popInt16());
		unitData.shotsMax = message.popInt16();
		unitData.setRange(message.popInt16());
		unitData.ammoMax = message.popInt16();
		unitData.setArmor(message.popInt16());
		unitData.hitpointsMax = message.popInt16();
		unitData.setScan(message.popInt16());
		if (ID.isAVehicle()) unitData.speedMax = message.popInt16();
		unitData.setVersion(unitData.getVersion()+1);
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_MU_MSG_LANDING_COORDS (cNetMessage& message)
{
	assert (message.iType == MU_MSG_LANDING_COORDS);
	assert (serverState == SERVER_STATE_INITGAME);

	int playerNr = message.popChar();
	Log.write ("Server: received landing coords from Player " + iToStr (playerNr), cLog::eLOG_TYPE_NET_DEBUG);

	auto& player = getPlayerFromNumber (playerNr);

	playerLandingPositions[&player] = message.popPosition ();
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_MU_MSG_READY_TO_START (cNetMessage& message)
{
	assert (message.iType == MU_MSG_READY_TO_START);
	assert (serverState == SERVER_STATE_INITGAME);

	int playerNr = message.popChar ();
	Log.write ("Server: received ready to start from Player " + iToStr (playerNr), cLog::eLOG_TYPE_NET_DEBUG);

	auto& player = getPlayerFromNumber (playerNr);

	readyToStartPlayers.insert (&player);

	if (readyToStartPlayers.size () < playerList.size ()) return;

	startNewGame ();
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_TCP_CLOSE_OR_GAME_EV_WANT_DISCONNECT (cNetMessage& message)
{
	assert (message.iType == TCP_CLOSE || message.iType == GAME_EV_WANT_DISCONNECT);

	// Socket should be closed
	int iSocketNumber = -1;
	if (message.iType == TCP_CLOSE)
	{
		iSocketNumber = message.popInt16();
	}
	else // a client disconnected. Useful for play with DEDICATED_SERVER
	{
		const auto& player = getPlayerFromNumber (message.iPlayerNr);
		iSocketNumber = player.getSocketNum();
	}
	if (iSocketNumber == -1)
		return;
	network->close (iSocketNumber);

	cPlayer* Player = NULL;
	// resort socket numbers of the players
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		if (playerList[i]->getSocketNum () == iSocketNumber)
		{
			Player = playerList[i].get();
			break;
		}
	}
	for (size_t i = 0; i != playerList.size (); ++i)
		playerList[i]->onSocketIndexDisconnected (iSocketNumber);

	if (Player)
	{
		// freeze clients
		// the dedicated server doesn't force to wait for reconnect,
		// because it's expected client behaviour
		if (DEDICATED_SERVER == false)
			enableFreezeMode (FREEZE_WAIT_FOR_RECONNECT);
		sendSavedReport (*this, cSavedReportLostConnection (*Player), nullptr);

		DisconnectedPlayerList.push_back (Player);

		Player->revealMap();
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_CHAT_CLIENT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_CHAT_CLIENT);

    const auto& player = getPlayerFromNumber(message.popChar ());

	sendSavedReport (*this, cSavedReportChat (player, message.popString()), nullptr);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_TO_END_TURN (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_TO_END_TURN);

	auto& player = getPlayerFromNumber (message.iPlayerNr);

	if (isTurnBasedGame())
	{
		if (activeTurnPlayer->getNr() != message.iPlayerNr) return;
	}
	handleEnd (player);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_START_WORK (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_START_WORK);

	const int iID = message.popInt32();
	cBuilding* building = getBuildingFromID (iID);

	if (building == NULL || building->owner->getNr() != message.iPlayerNr) return;

	building->ServerStartWork (*this);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_STOP_WORK (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_STOP_WORK);

	const int iID = message.popInt32();
	cBuilding* building = getBuildingFromID (iID);

	if (building == NULL || building->owner->getNr() != message.iPlayerNr) return;

	building->ServerStopWork (*this, false);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_MOVE_JOB_CLIENT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_MOVE_JOB_CLIENT);

	cServerMoveJob* MoveJob = cServerMoveJob::generateFromMessage (*this, message);
	if (!MoveJob)
	{
		return;
	}

	addActiveMoveJob (*MoveJob);
	Log.write (" Server: Added received movejob", cLog::eLOG_TYPE_NET_DEBUG);
	// send the movejob to all players who can see this unit
	const cVehicle& vehicle = *MoveJob->Vehicle;
	sendMoveJobServer (*this, *MoveJob, *vehicle.owner);
	for (size_t i = 0; i != vehicle.seenByPlayerList.size(); ++i)
	{
		sendMoveJobServer (*this, *MoveJob, *vehicle.seenByPlayerList[i]);
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_STOP_MOVE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_STOP_MOVE);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle == NULL || Vehicle->ServerMoveJob == NULL) return;

	Vehicle->ServerMoveJob->stop();
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_MOVEJOB_RESUME (cNetMessage& message)
{
	assert (message.iType == GAME_EV_MOVEJOB_RESUME);

	const int vehicleId = message.popInt32();
	if (vehicleId == 0) // All vehicles
	{
		auto& player = getPlayerFromNumber (message.iPlayerNr);

		const auto& vehicles = player.getVehicles ();
		for (auto i = vehicles.begin (); i != vehicles.end (); ++i)
		{
			const auto& vehicle = *i;
			if (vehicle->ServerMoveJob && !vehicle->isUnitMoving ())
				vehicle->ServerMoveJob->resume();
		}
	}
	else
	{
		cVehicle* vehicle = getVehicleFromID (vehicleId);
		if (!vehicle || vehicle->owner->getNr() != message.iPlayerNr) return;

		if (vehicle->ServerMoveJob)
			vehicle->ServerMoveJob->resume();
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_ATTACK (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_ATTACK);
	// identify aggressor
	const bool bIsVehicle = message.popBool();
	cUnit* attackingUnit = NULL;
	if (bIsVehicle)
	{
		const int ID = message.popInt32();
		attackingUnit = getVehicleFromID (ID);
		if (attackingUnit == NULL)
		{
			Log.write (" Server: vehicle with ID " + iToStr (ID) + " not found", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		if (attackingUnit->owner->getNr() != message.iPlayerNr)
		{
			Log.write (" Server: Message was not send by vehicle owner!", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		if (attackingUnit->isBeeingAttacked ()) return;
	}
	else
	{
		const auto position = message.popPosition ();
		if (Map->isValidPosition (position) == false)
		{
			Log.write (" Server: Invalid aggressor offset", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		attackingUnit = Map->getField (position).getTopBuilding ();
		if (attackingUnit == NULL)
		{
			Log.write (" Server: No Building at aggressor offset", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		if (attackingUnit->owner->getNr() != message.iPlayerNr)
		{
			Log.write (" Server: Message was not send by building owner!", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		if (attackingUnit->isBeeingAttacked ()) return;
	}

	// find target offset
	auto targetPosition = message.popPosition ();
	if (Map->isValidPosition (targetPosition) == false)
	{
		Log.write (" Server: Invalid target position!", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}

	const int targetID = message.popInt32();
	if (targetID != 0)
	{
		cVehicle* targetVehicle = getVehicleFromID (targetID);
		if (targetVehicle == NULL)
		{
			Log.write (" Server: vehicle with ID " + iToStr (targetID) + " not found!", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		const auto oldPosition = targetPosition;
		// the target offset doesn't need to match the vehicle position,
		// when it is big
		if (!targetVehicle->data.isBig)
		{
			targetPosition = targetVehicle->getPosition();
		}
		Log.write (" Server: attacking vehicle " + targetVehicle->getDisplayName() + ", " + iToStr (targetVehicle->iID), cLog::eLOG_TYPE_NET_DEBUG);
		if(oldPosition != targetPosition) Log.write(" Server: target offset changed from " + iToStr(oldPosition.x()) + "x" + iToStr(oldPosition.y()) + " to " + iToStr(targetPosition.x()) + "x" + iToStr(targetPosition.y()), cLog::eLOG_TYPE_NET_DEBUG);
	}

	// check if attack is possible
	if (attackingUnit->canAttackObjectAt (targetPosition, *Map, true) == false)
	{
		Log.write (" Server: The server decided, that the attack is not possible", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}
	AJobs.push_back(new cServerAttackJob(*this, attackingUnit, targetPosition, false));
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_ATTACKJOB_FINISHED (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ATTACKJOB_FINISHED);

	const int ID = message.popInt16();
	cServerAttackJob* aJob = NULL;

	unsigned int i = 0;
	for (; i < AJobs.size(); i++)
	{
		if (AJobs[i]->iID == ID)
		{
			aJob = AJobs[i];
			break;
		}
	}
	if (aJob == NULL) // attack job not found
	{
		Log.write (" Server: ServerAttackJob not found", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	aJob->clientFinished (message.iPlayerNr);
	if (aJob->executingClients.empty())
	{
		AJobs.erase (AJobs.begin() + i);
		delete aJob;
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_MINELAYERSTATUS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_MINELAYERSTATUS);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (!Vehicle) return;

	Vehicle->setClearMines(message.popBool());
	Vehicle->setLayMines(message.popBool());

	if (Vehicle->isUnitClearingMines () && Vehicle->isUnitLayingMines ())
	{
		Vehicle->setClearMines(false);
		Vehicle->setLayMines(false);
		return;
	}

	bool result = false;
	if (Vehicle->isUnitClearingMines ()) result = Vehicle->clearMine (*this);
	if (Vehicle->isUnitLayingMines ()) result = Vehicle->layMine (*this);

	if (result)
	{
		sendUnitData (*this, *Vehicle, *Vehicle->owner);
		for (size_t i = 0; i != Vehicle->seenByPlayerList.size(); ++i)
		{
			sendUnitData (*this, *Vehicle, *Vehicle->seenByPlayerList[i]);
		}
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_BUILD (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_BUILD);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle == NULL) return;
	if (Vehicle->isUnitBuildingABuilding () || Vehicle->BuildPath) return;

	const sID BuildingTyp = message.popID();
	if (BuildingTyp.getUnitDataOriginalVersion() == NULL)
	{
		Log.write (" Server: invalid unit: " + iToStr (BuildingTyp.iFirstPart) + "." + iToStr (BuildingTyp.iSecondPart), cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	const sUnitData& Data = *BuildingTyp.getUnitDataOriginalVersion();
	const int iBuildSpeed = message.popInt16();
	if (iBuildSpeed > 2 || iBuildSpeed < 0) return;
	const auto buildPosition = message.popPosition ();

	std::array<int, 3> iTurboBuildRounds;
	std::array<int, 3> iTurboBuildCosts;
	Vehicle->calcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, Vehicle->owner->getUnitDataCurrentVersion (BuildingTyp)->buildCosts);

	if (iTurboBuildCosts[iBuildSpeed] > Vehicle->data.getStoredResources () ||
		iTurboBuildRounds[iBuildSpeed] <= 0)
	{
		// TODO: differ between different aborting types
		// (buildposition blocked, not enough material, ...)
		sendBuildAnswer (*this, false, *Vehicle);
		return;
	}

	if (Map->isValidPosition (buildPosition) == false) return;
	const auto oldPosition = Vehicle->getPosition();

	if (Vehicle->data.canBuild != Data.buildAs) return;

	if (Data.isBig)
	{
		sideStepStealthUnit (buildPosition,                   *Vehicle, buildPosition);
		sideStepStealthUnit (buildPosition + cPosition(1, 0), *Vehicle, buildPosition);
		sideStepStealthUnit (buildPosition + cPosition(0, 1), *Vehicle, buildPosition);
		sideStepStealthUnit (buildPosition + cPosition(1, 1), *Vehicle, buildPosition);

		if (! (Map->possiblePlaceBuilding (Data, buildPosition,                   Vehicle) &&
			   Map->possiblePlaceBuilding (Data, buildPosition + cPosition(1, 0), Vehicle) &&
			   Map->possiblePlaceBuilding (Data, buildPosition + cPosition(0, 1), Vehicle) &&
			   Map->possiblePlaceBuilding (Data, buildPosition + cPosition(1, 1), Vehicle)))
		{
			sendBuildAnswer (*this, false, *Vehicle);
			return;
		}
		Vehicle->buildBigSavedPosition = Vehicle->getPosition();

		// set vehicle to build position
		Map->moveVehicleBig (*Vehicle, buildPosition);
		Vehicle->owner->doScan();
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

	Vehicle->setBuildingType(BuildingTyp);
	const bool bBuildPath = message.popBool();
	const cPosition pathPosition = message.popPosition ();
	if (Map->isValidPosition (pathPosition) == false) return;
	Vehicle->bandPosition = pathPosition;

	Vehicle->setBuildCosts (iTurboBuildCosts[iBuildSpeed]);
	Vehicle->setBuildTurns (iTurboBuildRounds[iBuildSpeed]);
	Vehicle->setBuildCostsStart(Vehicle->getBuildCosts ());
	Vehicle->setBuildTurnsStart(Vehicle->getBuildTurns ());

	Vehicle->setBuildingABuilding(true);
	Vehicle->BuildPath = bBuildPath;

	sendBuildAnswer (*this, true, *Vehicle);
	addJob (new cStartBuildJob (*Vehicle, oldPosition, Data.isBig));

	if (Vehicle->ServerMoveJob) Vehicle->ServerMoveJob->release();
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_END_BUILDING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_END_BUILDING);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle == NULL) return;

    const cPosition escapePosition (message.popPosition());

	if (!Vehicle->isUnitBuildingABuilding () || Vehicle->getBuildTurns () > 0) return;
	if (!Map->possiblePlace (*Vehicle, escapePosition))
	{
		sideStepStealthUnit(escapePosition, *Vehicle);
	}

	if (!Map->possiblePlace (*Vehicle, escapePosition)) return;

	addBuilding (Vehicle->getPosition(), Vehicle->getBuildingType (), Vehicle->owner);

	// end building
	Vehicle->setBuildingABuilding(false);
	Vehicle->BuildPath = false;

	// set the vehicle to the border
	if (Vehicle->getBuildingType ().getUnitDataOriginalVersion ()->isBig)
	{
		int x = Vehicle->getPosition().x();
		int y = Vehicle->getPosition().y();
        if (escapePosition.x() > Vehicle->getPosition ().x ()) x++;
        if (escapePosition.y() > Vehicle->getPosition ().y ()) y++;
		Map->moveVehicle (*Vehicle, cPosition(x, y));

		// refresh SeenByPlayerLists
		checkPlayerUnits();
	}

#if 0
	// send new vehicle status and position //done implicitly by addMoveJob()
	sendUnitData (Vehicle, Vehicle->owner->Nr);
	for (size_t i = 0; i < Vehicle->seenByPlayerList.size(); ++i)
	{
		sendUnitData (Vehicle, Vehicle->seenByPlayerList[i]->Nr);
	}
#endif

	// drive away from the building lot
	addMoveJob (Vehicle->getPosition(), escapePosition, Vehicle);
	// begin the movement immediately,
	// so no other unit can block the destination field
	Vehicle->ServerMoveJob->checkMove();
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_STOP_BUILDING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_STOP_BUILDING);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle == NULL) return;
	if (!Vehicle->isUnitBuildingABuilding ()) return;
	stopVehicleBuilding (*Vehicle);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_TRANSFER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_TRANSFER);

	cVehicle* SrcVehicle = NULL;
	cVehicle* DestVehicle = NULL;
	cBuilding* SrcBuilding = NULL;
	cBuilding* DestBuilding = NULL;

	if (message.popBool()) SrcVehicle = getVehicleFromID (message.popInt16());
	else SrcBuilding = getBuildingFromID (message.popInt16());

	if (message.popBool()) DestVehicle = getVehicleFromID (message.popInt16());
	else DestBuilding = getBuildingFromID (message.popInt16());

	if ((!SrcBuilding && !SrcVehicle) || (!DestBuilding && !DestVehicle)) return;

	const int iTranfer = message.popInt16();
	const int iType = message.popInt16();

	if (SrcBuilding)
	{
		bool bBreakSwitch = false;
		if (DestBuilding)
		{
			if (SrcBuilding->SubBase != DestBuilding->SubBase) return;
			if (SrcBuilding->owner != DestBuilding->owner) return;
			if (SrcBuilding->data.storeResType != iType) return;
			if (SrcBuilding->data.storeResType != DestBuilding->data.storeResType) return;
			if (DestBuilding->data.getStoredResources () + iTranfer > DestBuilding->data.storageResMax || DestBuilding->data.getStoredResources () + iTranfer < 0) return;
			if (SrcBuilding->data.getStoredResources () - iTranfer > SrcBuilding->data.storageResMax || SrcBuilding->data.getStoredResources () - iTranfer < 0) return;

			DestBuilding->data.setStoredResources (DestBuilding->data.getStoredResources () + iTranfer);
			SrcBuilding->data.setStoredResources (SrcBuilding->data.getStoredResources () - iTranfer);
			sendUnitData (*this, *DestBuilding, *DestBuilding->owner);
			sendUnitData (*this, *SrcBuilding, *SrcBuilding->owner);
		}
		else
		{
			if (DestVehicle->isUnitBuildingABuilding () || DestVehicle->isUnitClearing ()) return;
			if (DestVehicle->data.storeResType != iType) return;
			if (DestVehicle->data.getStoredResources () + iTranfer > DestVehicle->data.storageResMax || DestVehicle->data.getStoredResources () + iTranfer < 0) return;
			switch (iType)
			{
				case sUnitData::STORE_RES_METAL:
				{
					if (SrcBuilding->SubBase->getMetal () - iTranfer > SrcBuilding->SubBase->MaxMetal || SrcBuilding->SubBase->getMetal () - iTranfer < 0) bBreakSwitch = true;
					if (!bBreakSwitch) SrcBuilding->SubBase->addMetal (*this, -iTranfer);
				}
				break;
				case sUnitData::STORE_RES_OIL:
				{
					if (SrcBuilding->SubBase->getOil () - iTranfer > SrcBuilding->SubBase->MaxOil || SrcBuilding->SubBase->getOil () - iTranfer < 0) bBreakSwitch = true;
					if (!bBreakSwitch) SrcBuilding->SubBase->addOil (*this, -iTranfer);
				}
				break;
				case sUnitData::STORE_RES_GOLD:
				{
					if (SrcBuilding->SubBase->getGold () - iTranfer > SrcBuilding->SubBase->MaxGold || SrcBuilding->SubBase->getGold () - iTranfer < 0) bBreakSwitch = true;
					if (!bBreakSwitch) SrcBuilding->SubBase->addGold (*this, -iTranfer);
				}
				break;
			}
			if (bBreakSwitch) return;
			sendSubbaseValues (*this, *SrcBuilding->SubBase, *SrcBuilding->owner);
			DestVehicle->data.setStoredResources (DestVehicle->data.getStoredResources () + iTranfer);
			sendUnitData (*this, *DestVehicle, *DestVehicle->owner);
		}
	}
	else
	{
		if (SrcVehicle->data.storeResType != iType) return;
		if (SrcVehicle->data.getStoredResources () - iTranfer > SrcVehicle->data.storageResMax || SrcVehicle->data.getStoredResources () - iTranfer < 0) return;
		if (DestBuilding)
		{
			bool bBreakSwitch = false;
			switch (iType)
			{
				case sUnitData::STORE_RES_METAL:
				{
					if (DestBuilding->SubBase->getMetal () + iTranfer > DestBuilding->SubBase->MaxMetal || DestBuilding->SubBase->getMetal () + iTranfer < 0) bBreakSwitch = true;
					if (!bBreakSwitch) DestBuilding->SubBase->addMetal (*this, iTranfer);
				}
				break;
				case sUnitData::STORE_RES_OIL:
				{
					if (DestBuilding->SubBase->getOil () + iTranfer > DestBuilding->SubBase->MaxOil || DestBuilding->SubBase->getOil () + iTranfer < 0) bBreakSwitch = true;
					if (!bBreakSwitch) DestBuilding->SubBase->addOil (*this, iTranfer);
				}
				break;
				case sUnitData::STORE_RES_GOLD:
				{
					if (DestBuilding->SubBase->getGold () + iTranfer > DestBuilding->SubBase->MaxGold || DestBuilding->SubBase->getGold () + iTranfer < 0) bBreakSwitch = true;
					if (!bBreakSwitch) DestBuilding->SubBase->addGold (*this, iTranfer);
				}
				break;
			}
			if (bBreakSwitch) return;
			sendSubbaseValues (*this, *DestBuilding->SubBase, *DestBuilding->owner);
		}
		else
		{
			if (DestVehicle->isUnitBuildingABuilding () || DestVehicle->isUnitClearing ()) return;
			if (DestVehicle->data.storeResType != iType) return;
			if (DestVehicle->data.getStoredResources () + iTranfer > DestVehicle->data.storageResMax || DestVehicle->data.getStoredResources () + iTranfer < 0) return;
			DestVehicle->data.setStoredResources(DestVehicle->data.getStoredResources() + iTranfer);
			sendUnitData (*this, *DestVehicle, *DestVehicle->owner);
		}
		SrcVehicle->data.setStoredResources (SrcVehicle->data.getStoredResources () - iTranfer);
		sendUnitData (*this, *SrcVehicle, *SrcVehicle->owner);
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_BUILDLIST (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_BUILDLIST);

	cBuilding* Building = getBuildingFromID (message.popInt16());
	if (Building == NULL) return;

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
		if (Map->isValidPosition (cPosition(iX, iY)) == false) continue;

		const auto& buildings = Map->getField (cPosition (iX, iY)).getBuildings ();
		auto b_it = buildings.begin();
		auto b_end = buildings.end ();
		while (b_it != b_end && ((*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE || (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)) ++b_it;

		if (!Map->isWaterOrCoast (cPosition(iX, iY)) || (b_it != b_end && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_BASE)) bLand = true;
		else if (Map->isWaterOrCoast (cPosition(iX, iY)) && b_it != b_end && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)
		{
			bLand = true;
			bWater = true;
			break;
		}
		else if (Map->isWaterOrCoast (cPosition(iX, iY))) bWater = true;
	}

	// reset building status
	if (Building->isUnitWorking ())
	{
		Building->ServerStopWork (*this, false);
	}

	const int iBuildSpeed = message.popInt16();
	if (iBuildSpeed == 0) Building->MetalPerRound =  1 * Building->data.needsMetal;
	if (iBuildSpeed == 1) Building->MetalPerRound =  4 * Building->data.needsMetal;
	if (iBuildSpeed == 2) Building->MetalPerRound = 12 * Building->data.needsMetal;

	std::vector<cBuildListItem> NewBuildList;

	const int iCount = message.popInt16();
	for (int i = 0; i < iCount; i++)
	{
		const sID Type = message.popID();

		// if the first unit hasn't changed copy it to the new buildlist
		if (!Building->isBuildListEmpty () && i == 0 && Type == Building->getBuildListItem (0).getType ())
		{
			// recalculate costs, because build speed could have been changed
			std::array<int, 3> iTurboBuildRounds;
			std::array<int, 3> iTurboBuildCosts;
			Building->calcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, Building->owner->getUnitDataCurrentVersion (Type)->buildCosts, Building->getBuildListItem (0).getRemainingMetal ());
			NewBuildList.push_back (cBuildListItem (Type, iTurboBuildCosts[iBuildSpeed]));
			continue;
		}

		// check whether this building can build this unit
		if (Type.getUnitDataOriginalVersion()->factorSea > 0 && Type.getUnitDataOriginalVersion()->factorGround == 0 && !bWater)
			continue;
		else if (Type.getUnitDataOriginalVersion()->factorGround > 0 && Type.getUnitDataOriginalVersion()->factorSea == 0 && !bLand)
			continue;

		if (Building->data.canBuild != Type.getUnitDataOriginalVersion()->buildAs)
			continue;

		cBuildListItem BuildListItem(Type, -1);

		NewBuildList.push_back (BuildListItem);
	}

	Building->setBuildList (std::move (NewBuildList));

	if (!Building->isBuildListEmpty())
	{
		if (Building->getBuildListItem(0).getRemainingMetal () == -1)
		{
			std::array<int, 3> iTurboBuildRounds;
			std::array<int, 3> iTurboBuildCosts;
			Building->calcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, Building->owner->getUnitDataCurrentVersion (Building->getBuildListItem (0).getType ())->buildCosts);
			Building->getBuildListItem (0).setRemainingMetal (iTurboBuildCosts[iBuildSpeed]);
		}

		Building->RepeatBuild = message.popBool();
		Building->BuildSpeed = iBuildSpeed;
		if (Building->getBuildListItem (0).getRemainingMetal () > 0)
		{
			Building->ServerStartWork (*this);
		}
	}
	sendBuildList (*this, *Building);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_EXIT_FIN_VEH (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_EXIT_FIN_VEH);

	cBuilding* Building = getBuildingFromID (message.popInt16());
	if (Building == NULL) return;

	const auto position = message.popPosition();
	if (Map->isValidPosition (position) == false) return;
	if (Building->isBuildListEmpty()) return;

	cBuildListItem& BuildingListItem = Building->getBuildListItem (0);
	if (BuildingListItem.getRemainingMetal () > 0) return;

	if (!Building->isNextTo (position)) return;

	const sUnitData& unitData = *BuildingListItem.getType ().getUnitDataOriginalVersion ();
	if (!Map->possiblePlaceVehicle (unitData, position, Building->owner))
	{
		sideStepStealthUnit (position, unitData, Building->owner);
	}
	if (!Map->possiblePlaceVehicle (unitData, position, Building->owner)) return;

	addVehicle (position, BuildingListItem.getType (), Building->owner, false);

	// start new buildjob
	if (Building->RepeatBuild)
	{
		BuildingListItem.setRemainingMetal (-1);
		Building->addBuildListItem (BuildingListItem);
	}
	Building->removeBuildListItem (0);

	if (!Building->isBuildListEmpty())
	{
		cBuildListItem& BuildingListItem = Building->getBuildListItem(0);
		if (BuildingListItem.getRemainingMetal () == -1)
		{
			std::array<int, 3> iTurboBuildRounds;
			std::array<int, 3> iTurboBuildCosts;
			Building->calcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, Building->owner->getUnitDataCurrentVersion (BuildingListItem.getType ())->buildCosts);
			BuildingListItem.setRemainingMetal (iTurboBuildCosts[Building->BuildSpeed]);
		}
		Building->ServerStartWork (*this);
	}
	sendBuildList (*this, *Building);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_CHANGE_RESOURCES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_CHANGE_RESOURCES);

	cBuilding* Building = getBuildingFromID (message.popInt16());
	if (Building == NULL) return;

	const unsigned int iMetalProd = message.popInt16();
	const unsigned int iOilProd = message.popInt16();
	const unsigned int iGoldProd = message.popInt16();

	sSubBase& subBase = *Building->SubBase;

	subBase.setMetalProd (0);
	subBase.setOilProd (0);
	subBase.setGoldProd (0);

	// no need to verify the values.
	// They will be reduced automatically, if necessary
	subBase.setMetalProd (iMetalProd);
	subBase.setGoldProd (iGoldProd);
	subBase.setOilProd (iOilProd);

	sendSubbaseValues (*this, subBase, *Building->owner);
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
		Vehicle->setManualFireActive(!Vehicle->isManualFireActive());
		if (Vehicle->isManualFireActive() && Vehicle->isSentryActive())
		{
			Vehicle->owner->deleteSentry (*Vehicle);
		}

		sendUnitData (*this, *Vehicle, *Vehicle->owner);
		for (size_t i = 0; i != Vehicle->seenByPlayerList.size(); ++i)
			sendUnitData (*this, *Vehicle, *Vehicle->seenByPlayerList[i]);
	}
	else // building
	{
		cBuilding* Building = getBuildingFromID (message.popInt16());
		if (Building == 0)
			return;
		Building->setManualFireActive (!Building->isManualFireActive ());
		if (Building->isManualFireActive () && Building->isSentryActive ())
		{
			Building->owner->deleteSentry (*Building);
		}

		sendUnitData (*this, *Building, *Building->owner);
		for (size_t i = 0; i != Building->seenByPlayerList.size(); ++i)
			sendUnitData (*this, *Building, *Building->seenByPlayerList[i]);
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_CHANGE_SENTRY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_CHANGE_SENTRY);

	if (message.popBool()) // vehicle
	{
		cVehicle* vehicle = getVehicleFromID (message.popInt16());
		if (vehicle == NULL) return;

		if (vehicle->isSentryActive())
		{
			vehicle->owner->deleteSentry (*vehicle);
		}
		else
		{
			vehicle->owner->addSentry (*vehicle);
			vehicle->setManualFireActive(false);
		}

		sendUnitData (*this, *vehicle, *vehicle->owner);
		for (size_t i = 0; i != vehicle->seenByPlayerList.size(); ++i)
		{
			sendUnitData (*this, *vehicle, *vehicle->seenByPlayerList[i]);
		}
	}
	else // building
	{
		cBuilding* building = getBuildingFromID (message.popInt16());
		if (building == NULL) return;

		if (building->isSentryActive())
		{
			building->owner->deleteSentry (*building);
		}
		else
		{
			building->owner->addSentry (*building);
			building->setManualFireActive (false);
		}

		sendUnitData (*this, *building, *building->owner);
		for (size_t i = 0; i != building->seenByPlayerList.size(); ++i)
		{
			sendUnitData (*this, *building, *building->seenByPlayerList[i]);
		}
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_MARK_LOG (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_MARK_LOG);

	AutoPtr<cNetMessage> message2 (new cNetMessage (GAME_EV_MARK_LOG));
	message2->pushString (message.popString());
	sendNetMessage (message2);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_SUPPLY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_SUPPLY);

	cVehicle* SrcVehicle = NULL;
	cVehicle* DestVehicle = NULL;
	cBuilding* SrcBuilding = NULL;
	cBuilding* DestBuilding = NULL;

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
			SrcVehicle->data.setStoredResources (SrcVehicle->data.getStoredResources()-1);
			iValue = DestVehicle ? DestVehicle->data.ammoMax : DestBuilding->data.ammoMax;
		}
		else
		{
			sUnitData* DestData = DestVehicle ? &DestVehicle->data : &DestBuilding->data;
			// reduce cargo for repair and calculate maximal repair value
			iValue = DestData->getHitpoints ();
			while (SrcVehicle->data.getStoredResources () > 0 && iValue < DestData->hitpointsMax)
			{
				iValue += Round (((float)DestData->hitpointsMax / DestData->buildCosts) * 4);
				SrcVehicle->data.setStoredResources (SrcVehicle->data.getStoredResources ()-1);
			}
			iValue = std::min (DestData->hitpointsMax, iValue);
		}
		// the changed values aren't interesting for enemy players,
		// so only send the new data to the owner
		sendUnitData (*this, *SrcVehicle, *SrcVehicle->owner);
	}
	else
	{
		// buildings can only supply vehicles
		if (!DestVehicle) return;

		// do the supply
		if (iType == SUPPLY_TYPE_REARM)
		{
			if (SrcBuilding->SubBase->getMetal () < 1) return;
			SrcBuilding->SubBase->addMetal (*this, -1);
			iValue = DestVehicle->data.ammoMax;
		}
		else
		{
			// reduce cargo for repair and calculate maximal repair value
			iValue = DestVehicle->data.getHitpoints ();
			while (SrcBuilding->SubBase->getMetal () > 0 && iValue < DestVehicle->data.hitpointsMax)
			{
				iValue += Round (((float) DestVehicle->data.hitpointsMax / DestVehicle->data.buildCosts) * 4);
				SrcBuilding->SubBase->addMetal (*this, -1);
			}
			iValue = std::min (DestVehicle->data.hitpointsMax, iValue);
		}
		// the changed values aren't interesting for enemy players,
		// so only send the new data to the owner
		sendUnitData (*this, *SrcBuilding, *SrcBuilding->owner);
	}

	// repair or reload the destination unit
	if (DestVehicle)
	{
		if (iType == SUPPLY_TYPE_REARM) DestVehicle->data.setAmmo(DestVehicle->data.ammoMax);
		else DestVehicle->data.setHitpoints(iValue);

		sendSupply (*this, DestVehicle->iID, true, iValue, iType, *DestVehicle->owner);

		// send unitdata to the players who are not the owner
		for (size_t i = 0; i != DestVehicle->seenByPlayerList.size(); ++i)
			sendUnitData (*this, *DestVehicle, *DestVehicle->seenByPlayerList[i]);
	}
	else
	{
		if (iType == SUPPLY_TYPE_REARM) DestBuilding->data.setAmmo(DestBuilding->data.ammoMax);
		else DestBuilding->data.setHitpoints(iValue);

		sendSupply (*this, DestBuilding->iID, false, iValue, iType, *DestBuilding->owner);

		// send unitdata to the players who are not the owner
		for (size_t i = 0; i != DestBuilding->seenByPlayerList.size(); ++i)
			sendUnitData (*this, *DestBuilding, *DestBuilding->seenByPlayerList[i]);
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
	const int availableMetal = storingBuilding->SubBase->getMetal ();

	std::vector<cVehicle*> upgradedVehicles;
	for (size_t i = 0; i != storingBuilding->storedUnits.size(); ++i)
	{
		if (upgradeAll || i == storageSlot)
		{
			cVehicle* vehicle = storingBuilding->storedUnits[i];
			const sUnitData& upgradedVersion = *storingBuilding->owner->getUnitDataCurrentVersion (vehicle->data.ID);

			if (vehicle->data.getVersion () >= upgradedVersion.getVersion ())
				continue; // already up to date
			cUpgradeCalculator& uc = cUpgradeCalculator::instance();
			const int upgradeCost = uc.getMaterialCostForUpgrading (upgradedVersion.buildCosts);

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
			storingBuilding->SubBase->addMetal (*this, -totalCosts);
		for (size_t i = 0; i != upgradedVehicles.size(); ++i)
			upgradedVehicles[i]->upgradeToCurrentVersion();
		sendUpgradeVehicles (*this, upgradedVehicles, totalCosts, storingBuilding->iID, *storingBuilding->owner);
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_START_CLEAR (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_START_CLEAR);

	const int id = message.popInt16();
	cVehicle* Vehicle = getVehicleFromID (id);
	if (Vehicle == NULL)
	{
		Log.write ("Server: Can not find vehicle with id " + iToStr (id) + " for clearing", LOG_TYPE_NET_WARNING);
		return;
	}

	cBuilding* building = Map->getField(Vehicle->getPosition()).getRubble();

	if (!building)
	{
		sendClearAnswer (*this, 2, *Vehicle, 0, cPosition(-1, -1), Vehicle->owner);
		return;
	}

	cPosition rubblePosition(-1, -1);
	if (building->data.isBig)
	{
		rubblePosition = building->getPosition();

		sideStepStealthUnit (building->getPosition()                  , *Vehicle, rubblePosition);
		sideStepStealthUnit (building->getPosition() + cPosition(1, 0), *Vehicle, rubblePosition);
		sideStepStealthUnit (building->getPosition() + cPosition(0, 1), *Vehicle, rubblePosition);
		sideStepStealthUnit (building->getPosition() + cPosition(1, 1), *Vehicle, rubblePosition);

		if ((!Map->possiblePlace (*Vehicle, building->getPosition()                  ) && rubblePosition                   != Vehicle->getPosition()) ||
			(!Map->possiblePlace (*Vehicle, building->getPosition() + cPosition(1, 0)) && rubblePosition + cPosition(1, 0) != Vehicle->getPosition()) ||
			(!Map->possiblePlace (*Vehicle, building->getPosition() + cPosition(0, 1)) && rubblePosition + cPosition(0, 1) != Vehicle->getPosition()) ||
			(!Map->possiblePlace (*Vehicle, building->getPosition() + cPosition(1, 1)) && rubblePosition + cPosition(1, 1) != Vehicle->getPosition()))
		{
			sendClearAnswer (*this, 1, *Vehicle, 0, cPosition (-1, -1), Vehicle->owner);
			return;
		}

		Vehicle->buildBigSavedPosition = Vehicle->getPosition();
		Map->moveVehicleBig (*Vehicle, building->getPosition());
	}

	Vehicle->setClearing(true);
	Vehicle->setClearingTurns(building->data.isBig ? 4 : 1);
	Vehicle->owner->doScan();
	addJob (new cStartBuildJob (*Vehicle, Vehicle->getPosition(), building->data.isBig));

	sendClearAnswer (*this, 0, *Vehicle, Vehicle->getClearingTurns (), rubblePosition, Vehicle->owner);
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
	if (Vehicle == NULL)
	{
		Log.write ("Server: Can not find vehicle with id " + iToStr (id) + " for stop clearing", LOG_TYPE_NET_WARNING);
		return;
	}

	if (Vehicle->isUnitClearing ())
	{
		Vehicle->setClearing(false);
		Vehicle->setClearingTurns(0);

		if (Vehicle->data.isBig)
		{
			Map->moveVehicle (*Vehicle, Vehicle->buildBigSavedPosition);
			Vehicle->owner->doScan();
			sendStopClear (*this, *Vehicle, Vehicle->buildBigSavedPosition, *Vehicle->owner);
			for (size_t i = 0; i != Vehicle->seenByPlayerList.size(); ++i)
			{
				sendStopClear (*this, *Vehicle, Vehicle->buildBigSavedPosition, *Vehicle->seenByPlayerList[i]);
			}
		}
		else
		{
			sendStopClear (*this, *Vehicle, cPosition(-1, -1), *Vehicle->owner);
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
	if (LocalPlayer.isLocal() == false) return;

	// delete disconnected players
	for (size_t i = 0; i != DisconnectedPlayerList.size(); ++i)
	{
		deletePlayer (DisconnectedPlayerList[i]);
	}
	DisconnectedPlayerList.clear();
	disableFreezeMode (FREEZE_WAIT_FOR_RECONNECT);
	if (isTurnBasedGame ()) sendWaitFor (*this, activeTurnPlayer->getNr(), nullptr);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_IDENTIFICATION (cNetMessage& message)
{
	assert (message.iType == GAME_EV_IDENTIFICATION);

	const std::string playerName = message.popString();
	const int socketNumber = message.popInt16();

	for (size_t i = 0; i != DisconnectedPlayerList.size(); ++i)
	{
		if (playerName == DisconnectedPlayerList[i]->getName())
		{
			DisconnectedPlayerList[i]->setSocketIndex (socketNumber);
			sendReconnectAnswer (*this, socketNumber, *DisconnectedPlayerList[i]);
			return;
		}
	}
	sendReconnectAnswer (*this, socketNumber);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_RECON_SUCCESS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_RECON_SUCCESS);

	cPlayer* player = nullptr;
	const int playerNum = message.popInt16();
	// remove the player from the disconnected list
	for (size_t i = 0; i != DisconnectedPlayerList.size(); ++i)
	{
		if (DisconnectedPlayerList[i]->getNr() == playerNum)
		{
			player = DisconnectedPlayerList[i];
			DisconnectedPlayerList.erase (DisconnectedPlayerList.begin() + i);
			break;
		}
	}
	if (player == nullptr) return;
	resyncPlayer (*player);

	disableFreezeMode (FREEZE_WAIT_FOR_RECONNECT);
	if (isTurnBasedGame ()) sendWaitFor (*this, activeTurnPlayer->getNr(), nullptr);
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
			if (StoredVehicle->ServerMoveJob) StoredVehicle->ServerMoveJob->release();
			// vehicle is removed from enemy clients by cServer::checkPlayerUnits()
			sendStoreVehicle (*this, StoringVehicle->iID, true, StoredVehicle->iID, *StoringVehicle->owner);
		}
	}
	else
	{
		cBuilding* StoringBuilding = getBuildingFromID (message.popInt16());
		if (!StoringBuilding) return;

		if (StoringBuilding->canLoad (StoredVehicle))
		{
			StoringBuilding->storeVehicle (*StoredVehicle, *Map);
			if (StoredVehicle->ServerMoveJob) StoredVehicle->ServerMoveJob->release();
			// vehicle is removed from enemy clients by cServer::checkPlayerUnits()
			sendStoreVehicle (*this, StoringBuilding->iID, false, StoredVehicle->iID, *StoringBuilding->owner);
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

		if (StoringVehicle->canExitTo (position, *Map, StoredVehicle->data))
		{
			StoringVehicle->exitVehicleTo (*StoredVehicle, position, *Map);
			// vehicle is added to enemy clients by cServer::checkPlayerUnits()
			sendActivateVehicle (*this, StoringVehicle->iID, true, StoredVehicle->iID, position, *StoringVehicle->owner);
			if (StoredVehicle->data.canSurvey)
			{
				sendVehicleResources (*this, *StoredVehicle);
				StoredVehicle->doSurvey (*this);
			}

			if (StoredVehicle->canLand (*Map))
			{
				StoredVehicle->setFlightHeight(0);
			}
			else
			{
				StoredVehicle->setFlightHeight(64);
			}
			StoredVehicle->InSentryRange (*this);
		}
	}
	else
	{
		cBuilding* StoringBuilding = getBuildingFromID (message.popInt16());
		if (!StoringBuilding) return;

		const auto position = message.popPosition ();

		if (!StoringBuilding->isNextTo (position)) return;

		// sidestep stealth units if necessary
		sideStepStealthUnit (position, *StoredVehicle);

		if (StoringBuilding->canExitTo (position, *Map, StoredVehicle->data))
		{
			StoringBuilding->exitVehicleTo (*StoredVehicle, position, *Map);
			// vehicle is added to enemy clients by cServer::checkPlayerUnits()
			sendActivateVehicle (*this, StoringBuilding->iID, false, StoredVehicle->iID, position, *StoringBuilding->owner);
			if (StoredVehicle->data.canSurvey)
			{
				sendVehicleResources (*this, *StoredVehicle);
				StoredVehicle->doSurvey (*this);
			}
			StoredVehicle->InSentryRange (*this);
		}
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_REQUEST_RESYNC (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REQUEST_RESYNC);

	auto& player = getPlayerFromNumber (message.popChar());
	resyncPlayer (player, true);
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

		sUnitData* upgradedUnit = player.getUnitDataCurrentVersion (ID);
		if (upgradedUnit == 0)
			continue; // skip this upgrade, because there is no such unitData

		const int costs = getUpgradeCosts (ID, player, newDamage, newMaxShots, newRange, newMaxAmmo, newArmor, newMaxHitPoints, newScan, newMaxSpeed);
		if (costs <= player.Credits)
		{
			// update the unitData of the player and send an ack-msg
			// for this upgrade to the player
			upgradedUnit->setDamage(newDamage);
			upgradedUnit->shotsMax = newMaxShots;
			upgradedUnit->setRange(newRange);
			upgradedUnit->ammoMax = newMaxAmmo;
			upgradedUnit->setArmor(newArmor);
			upgradedUnit->hitpointsMax = newMaxHitPoints;
			upgradedUnit->setScan(newScan);
			if (ID.isAVehicle()) upgradedUnit->speedMax = newMaxSpeed;
			upgradedUnit->setVersion(upgradedUnit->getVersion() + 1);

			player.Credits -= costs;
			updateCredits = true;

			sendUnitUpgrades (*this, *upgradedUnit, player);
		}
	}
	if (updateCredits)
		sendCredits (*this, player.Credits, player);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_BUILDING_UPGRADE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_BUILDING_UPGRADE);

	const unsigned int unitID = message.popInt32();
	const bool upgradeAll = message.popBool();

	cBuilding* building = getBuildingFromID (unitID);
	cPlayer* player = (building != 0) ? building->owner : 0;
	if (player == 0)
		return;

	const int availableMetal = building->SubBase->getMetal ();

	const sUnitData& upgradedVersion = *player->getUnitDataCurrentVersion (building->data.ID);
	if (building->data.getVersion () >= upgradedVersion.getVersion ())
		return; // already up to date
	cUpgradeCalculator& uc = cUpgradeCalculator::instance();
	const int upgradeCostPerBuilding = uc.getMaterialCostForUpgrading (upgradedVersion.buildCosts);
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
		sSubBase* subBase = building->SubBase;
		for (unsigned int subBaseBuildIdx = 0; subBaseBuildIdx < subBase->buildings.size(); subBaseBuildIdx++)
		{
			cBuilding* otherBuilding = subBase->buildings[subBaseBuildIdx];
			if (otherBuilding == building)
				continue;
			if (otherBuilding->data.ID != building->data.ID)
				continue;
			if (otherBuilding->data.getVersion () >= upgradedVersion.getVersion ())
				continue;
			upgradedBuildings.push_back (otherBuilding);
			totalCosts += upgradeCostPerBuilding;
			if (availableMetal < totalCosts + upgradeCostPerBuilding)
				break; // no more raw material left...
		}
	}

	if (totalCosts > 0)
		building->SubBase->addMetal (*this, -totalCosts);
	if (upgradedBuildings.empty() == false)
	{
		bool scanNecessary = false;
		bool refreshSentry = false;
		for (size_t i = 0; i != upgradedBuildings.size(); ++i)
		{
			// Scan range was upgraded. So trigger a scan.
			if (!scanNecessary && upgradedBuildings[i]->data.getScan () < upgradedVersion.getScan ())
				scanNecessary = true;
			if (upgradedBuildings[i]->isSentryActive () && upgradedBuildings[i]->data.getRange () < upgradedVersion.getRange ())
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
	if (newUsedResearch > player.workingResearchCenterCount)
		return; // can't turn on research centers automatically!

	// needed, if newUsedResearch < player->ResearchCount
	std::vector<cBuilding*> researchCentersToStop;
	std::vector<cBuilding*> researchCentersToChangeArea;
	std::vector<int> newAreasForResearchCenters;

	bool error = false;
	const auto buildings = player.getBuildings ();
	auto currentBuildingIter = buildings.begin ();
	for (int newArea = 0; newArea != cResearch::kNrResearchAreas; ++newArea)
	{
		int centersToAssign = newResearchSettings[newArea];
		for (; currentBuildingIter != buildings.end () && centersToAssign > 0; ++currentBuildingIter)
		{
			auto& building = *currentBuildingIter;
			if (building->data.canResearch && building->isUnitWorking ())
			{
				researchCentersToChangeArea.push_back (building.get());
				newAreasForResearchCenters.push_back (newArea);
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
	for (; currentBuildingIter != buildings.end (); ++currentBuildingIter)
	{
		auto& building = *currentBuildingIter;
		if (building->data.canResearch && building->isUnitWorking ())
			researchCentersToStop.push_back (building.get());
	}
	if (error)
		return;

	for (size_t i = 0; i != researchCentersToStop.size(); ++i)
		researchCentersToStop[i]->ServerStopWork (*this, false);

	for (size_t i = 0; i != researchCentersToChangeArea.size(); ++i)
		researchCentersToChangeArea[i]->researchArea = newAreasForResearchCenters[i];
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

	cVehicle* destVehicle = NULL;
	cBuilding* destBuilding = NULL;
	if (message.popBool()) destVehicle = getVehicleFromID (message.popInt16());
	else destBuilding = getBuildingFromID (message.popInt16());
	cUnit* destUnit = destVehicle;
	if (destUnit == NULL) destUnit = destBuilding;
	const bool steal = message.popBool();
	// check whether the commando action is possible
	if (! ((destUnit && srcVehicle->canDoCommandoAction (destUnit->getPosition(), *Map, steal)) ||
		   (destBuilding && destBuilding->data.isBig && srcVehicle->canDoCommandoAction (destUnit->getPosition() + cPosition(0, 1), *Map, steal)) ||
		   (destBuilding && destBuilding->data.isBig && srcVehicle->canDoCommandoAction (destUnit->getPosition() + cPosition(1, 0), *Map, steal)) ||
		   (destBuilding && destBuilding->data.isBig && srcVehicle->canDoCommandoAction (destUnit->getPosition() + cPosition(1, 1), *Map, steal)))) return;

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
				if (destVehicle->isUnitBuildingABuilding ()) stopVehicleBuilding (*destVehicle);
				if (destVehicle->ServerMoveJob) destVehicle->ServerMoveJob->release();
				changeUnitOwner (*destVehicle, *srcVehicle->owner);
			}
		}
		else
		{
			// only on disabling units the infiltrator gets exp.
			// As higher his level is as slower he rises onto the next one.
			// every 5 rankings he needs one successful disabling more,
			// to get to the next ranking
			srcVehicle->setCommandoRank(srcVehicle->getCommandoRank() +  1.f / (((int) srcVehicle->getCommandoRank() + 5) / 5));

			const int strength = srcVehicle->calcCommandoTurns (destUnit);
			// stop the unit and make it disabled
			destUnit->setDisabledTurns(strength);
			if (destVehicle)
			{
				// save speed and number of shots before disabling
				destVehicle->lastSpeed = destVehicle->data.speedCur;
				destVehicle->lastShots = destVehicle->data.getShots ();

				destVehicle->data.speedCur = 0;
				destVehicle->data.setShots(0);

				if (destVehicle->isUnitBuildingABuilding ()) stopVehicleBuilding (*destVehicle);
				if (destVehicle->ServerMoveJob) destVehicle->ServerMoveJob->release();
			}
			else if (destBuilding)
			{
				// save number of shots before disabling
				destBuilding->lastShots = destBuilding->data.getShots ();

				destBuilding->data.setShots(0);
				destBuilding->wasWorking = destBuilding->isUnitWorking ();
				destBuilding->ServerStopWork (*this, true);
				sendDoStopWork (*this, *destBuilding);
			}
			sendUnitData (*this, *destUnit, *destUnit->owner);
			for (size_t i = 0; i != destUnit->seenByPlayerList.size(); ++i)
			{
				sendUnitData (*this, *destUnit, *destUnit->seenByPlayerList[i]);
			}
			destUnit->owner->doScan();
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
			if (&player == srcVehicle->owner) continue;
			if (!player.canSeeAnyAreaUnder (*srcVehicle)) continue;

			srcVehicle->setDetectedByPlayer (*this, &player);
		}
		checkPlayerUnits();
		srcVehicle->InSentryRange (*this);
	}
	srcVehicle->data.setShots(srcVehicle->data.getShots()-1);
	sendUnitData (*this, *srcVehicle, *srcVehicle->owner);
	sendCommandoAnswer (*this, success, steal, *srcVehicle, *srcVehicle->owner);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_SAVE_HUD_INFO (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SAVE_HUD_INFO);

	const int msgSaveingID = message.popInt16();
	if (msgSaveingID != savingID) return;
	cPlayer& player = getPlayerFromNumber (message.popInt16());

	player.savedHud->selUnitID = message.popInt16();
	player.savedHud->offX = message.popInt16();
	player.savedHud->offY = message.popInt16();
	player.savedHud->zoom = message.popFloat();
	player.savedHud->colorsChecked = message.popBool();
	player.savedHud->gridChecked = message.popBool();
	player.savedHud->ammoChecked = message.popBool();
	player.savedHud->fogChecked = message.popBool();
	player.savedHud->twoXChecked = message.popBool();
	player.savedHud->rangeChecked = message.popBool();
	player.savedHud->scanChecked = message.popBool();
	player.savedHud->statusChecked = message.popBool();
	player.savedHud->surveyChecked = message.popBool();
	player.savedHud->lockChecked = message.popBool();
	player.savedHud->hitsChecked = message.popBool();
	player.savedHud->tntChecked = message.popBool();
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_SAVE_REPORT_INFO (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SAVE_REPORT_INFO);

	const int msgSaveingID = message.popInt16();
	if (msgSaveingID != savingID) return;
	auto& player = getPlayerFromNumber (message.popInt16());

	player.savedReportsList.push_back (cSavedReport::createFrom (message));
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_FIN_SEND_SAVE_INFO (cNetMessage& message)
{
	assert (message.iType == GAME_EV_FIN_SEND_SAVE_INFO);

	const int msgSaveingID = message.popInt16();
	if (msgSaveingID != savingID) return;
	auto& player = getPlayerFromNumber (message.popInt16());

	cSavegame savegame (savingIndex);
	savegame.writeAdditionalInfo (*player.savedHud, player.savedReportsList, &player);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_REQUEST_CASUALTIES_REPORT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REQUEST_CASUALTIES_REPORT);

	auto& player = getPlayerFromNumber (message.iPlayerNr);

	sendCasualtiesReport (*this, &player);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_SELFDESTROY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_SELFDESTROY);

	cBuilding* building = getBuildingFromID (message.popInt16());
	if (!building || building->owner->getNr() != message.iPlayerNr) return;

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
	for (size_t i = 0; i != unit->seenByPlayerList.size(); ++i)
		sendUnitData (*this, *unit, *unit->seenByPlayerList[i]);
	sendUnitData (*this, *unit, *unit->owner);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_END_MOVE_ACTION (cNetMessage& message)
{
	assert (message.iType == GAME_EV_END_MOVE_ACTION);

	cVehicle* vehicle = getVehicleFromID (message.popInt32());
	if (!vehicle || !vehicle->ServerMoveJob) return;

	const int destID = message.popInt32();
	eEndMoveActionType type = (eEndMoveActionType) message.popChar();
	vehicle->ServerMoveJob->addEndAction (destID, type);
}

//------------------------------------------------------------------------------
int cServer::handleNetMessage (cNetMessage& message)
{
	if (message.iType != NET_GAME_TIME_CLIENT)
	{
		Log.write ("Server: <-- Player " + iToStr (message.iPlayerNr) + " "
				   + message.getTypeAsString ()
				   + ", gameTime:" + iToStr (this->gameTimer->gameTime)
				   + ", Hexdump: " + message.getHexDump (), cLog::eLOG_TYPE_NET_DEBUG);
	}

	switch (message.iType)
	{
		case TCP_ACCEPT: handleNetMessage_TCP_ACCEPT (message); break;
		case MU_MSG_CLAN: handleNetMessage_MU_MSG_CLAN (message); break;
		case MU_MSG_LANDING_VEHICLES: handleNetMessage_MU_MSG_LANDING_VEHICLES (message); break;
		case MU_MSG_UPGRADES: handleNetMessage_MU_MSG_UPGRADES (message); break;
		case MU_MSG_LANDING_COORDS: handleNetMessage_MU_MSG_LANDING_COORDS (message); break;
		case MU_MSG_READY_TO_START: handleNetMessage_MU_MSG_READY_TO_START (message); break;

		case TCP_CLOSE: // Follow
		case GAME_EV_WANT_DISCONNECT:
			handleNetMessage_TCP_CLOSE_OR_GAME_EV_WANT_DISCONNECT (message);
			break;
		case GAME_EV_CHAT_CLIENT: handleNetMessage_GAME_EV_CHAT_CLIENT (message); break;
		case GAME_EV_WANT_TO_END_TURN: handleNetMessage_GAME_EV_WANT_TO_END_TURN (message); break;
		case GAME_EV_WANT_START_WORK: handleNetMessage_GAME_EV_WANT_START_WORK (message); break;
		case GAME_EV_WANT_STOP_WORK: handleNetMessage_GAME_EV_WANT_STOP_WORK (message); break;
		case GAME_EV_MOVE_JOB_CLIENT: handleNetMessage_GAME_EV_MOVE_JOB_CLIENT (message); break;
		case GAME_EV_WANT_STOP_MOVE: handleNetMessage_GAME_EV_WANT_STOP_MOVE (message); break;
		case GAME_EV_MOVEJOB_RESUME: handleNetMessage_GAME_EV_MOVEJOB_RESUME (message); break;
		case GAME_EV_WANT_ATTACK: handleNetMessage_GAME_EV_WANT_ATTACK (message); break;
		case GAME_EV_ATTACKJOB_FINISHED: handleNetMessage_GAME_EV_ATTACKJOB_FINISHED (message); break;
		case GAME_EV_MINELAYERSTATUS: handleNetMessage_GAME_EV_MINELAYERSTATUS (message); break;
		case GAME_EV_WANT_BUILD: handleNetMessage_GAME_EV_WANT_BUILD (message); break;
		case GAME_EV_END_BUILDING: handleNetMessage_GAME_EV_END_BUILDING (message); break;
		case GAME_EV_WANT_STOP_BUILDING: handleNetMessage_GAME_EV_WANT_STOP_BUILDING (message); break;
		case GAME_EV_WANT_TRANSFER: handleNetMessage_GAME_EV_WANT_TRANSFER (message); break;
		case GAME_EV_WANT_BUILDLIST: handleNetMessage_GAME_EV_WANT_BUILDLIST (message); break;
		case GAME_EV_WANT_EXIT_FIN_VEH: handleNetMessage_GAME_EV_WANT_EXIT_FIN_VEH (message); break;
		case GAME_EV_CHANGE_RESOURCES: handleNetMessage_GAME_EV_CHANGE_RESOURCES (message); break;
		case GAME_EV_WANT_CHANGE_MANUAL_FIRE: handleNetMessage_GAME_EV_WANT_CHANGE_MANUAL_FIRE (message); break;
		case GAME_EV_WANT_CHANGE_SENTRY: handleNetMessage_GAME_EV_WANT_CHANGE_SENTRY (message); break;
		case GAME_EV_WANT_MARK_LOG: handleNetMessage_GAME_EV_WANT_MARK_LOG (message); break;
		case GAME_EV_WANT_SUPPLY: handleNetMessage_GAME_EV_WANT_SUPPLY (message); break;
		case GAME_EV_WANT_VEHICLE_UPGRADE: handleNetMessage_GAME_EV_WANT_VEHICLE_UPGRADE (message); break;
		case GAME_EV_WANT_START_CLEAR: handleNetMessage_GAME_EV_WANT_START_CLEAR (message); break;
		case GAME_EV_WANT_STOP_CLEAR: handleNetMessage_GAME_EV_WANT_STOP_CLEAR (message); break;
		case GAME_EV_ABORT_WAITING: handleNetMessage_GAME_EV_ABORT_WAITING (message); break;
		case GAME_EV_IDENTIFICATION: handleNetMessage_GAME_EV_IDENTIFICATION (message); break;
		case GAME_EV_RECON_SUCCESS: handleNetMessage_GAME_EV_RECON_SUCCESS (message); break;
		case GAME_EV_WANT_LOAD: handleNetMessage_GAME_EV_WANT_LOAD (message); break;
		case GAME_EV_WANT_EXIT: handleNetMessage_GAME_EV_WANT_EXIT (message); break;
		case GAME_EV_REQUEST_RESYNC: handleNetMessage_GAME_EV_REQUEST_RESYNC (message); break;
		case GAME_EV_WANT_BUY_UPGRADES: handleNetMessage_GAME_EV_WANT_BUY_UPGRADES (message); break;
		case GAME_EV_WANT_BUILDING_UPGRADE: handleNetMessage_GAME_EV_WANT_BUILDING_UPGRADE (message); break;
		case GAME_EV_WANT_RESEARCH_CHANGE: handleNetMessage_GAME_EV_WANT_RESEARCH_CHANGE (message); break;
		case GAME_EV_AUTOMOVE_STATUS: handleNetMessage_GAME_EV_AUTOMOVE_STATUS (message); break;
		case GAME_EV_WANT_COM_ACTION: handleNetMessage_GAME_EV_WANT_COM_ACTION (message); break;
		case GAME_EV_SAVE_HUD_INFO: handleNetMessage_GAME_EV_SAVE_HUD_INFO (message); break;
		case GAME_EV_SAVE_REPORT_INFO: handleNetMessage_GAME_EV_SAVE_REPORT_INFO (message); break;
		case GAME_EV_FIN_SEND_SAVE_INFO: handleNetMessage_GAME_EV_FIN_SEND_SAVE_INFO (message); break;
		case GAME_EV_REQUEST_CASUALTIES_REPORT: handleNetMessage_GAME_EV_REQUEST_CASUALTIES_REPORT (message); break;
		case GAME_EV_WANT_SELFDESTROY: handleNetMessage_GAME_EV_WANT_SELFDESTROY (message); break;
		case GAME_EV_WANT_CHANGE_UNIT_NAME: handleNetMessage_GAME_EV_WANT_CHANGE_UNIT_NAME (message); break;
		case GAME_EV_END_MOVE_ACTION: handleNetMessage_GAME_EV_END_MOVE_ACTION (message); break;
		case NET_GAME_TIME_CLIENT: gameTimer->handleSyncMessage (message); break;
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
	const sUnitData* currentVersion = player.getUnitDataCurrentVersion (ID);
	const sUnitData* startVersion = ID.getUnitDataOriginalVersion (&player);
	if (currentVersion == 0 || startVersion == 0)
		return 1000000; // error (unbelievably high cost...)

	int cost = 0;
	const cUpgradeCalculator& uc = cUpgradeCalculator::instance();
	if (newDamage > currentVersion->getDamage ())
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->getDamage (), currentVersion->getDamage (), newDamage, cUpgradeCalculator::kAttack, player.researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newMaxShots > currentVersion->shotsMax)
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->shotsMax, currentVersion->shotsMax, newMaxShots, cUpgradeCalculator::kShots, player.researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newRange > currentVersion->getRange ())
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->getRange (), currentVersion->getRange (), newRange, cUpgradeCalculator::kRange, player.researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newMaxAmmo > currentVersion->ammoMax)
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->ammoMax, currentVersion->ammoMax, newMaxAmmo, cUpgradeCalculator::kAmmo, player.researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newArmor > currentVersion->getArmor ())
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->getArmor (), currentVersion->getArmor (), newArmor, cUpgradeCalculator::kArmor, player.researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newMaxHitPoints > currentVersion->hitpointsMax)
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->hitpointsMax, currentVersion->hitpointsMax, newMaxHitPoints, cUpgradeCalculator::kHitpoints, player.researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newScan > currentVersion->getScan ())
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->getScan (), currentVersion->getScan (), newScan, cUpgradeCalculator::kScan, player.researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (bVehicle && newMaxSpeed > currentVersion->speedMax)
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->speedMax / 4, currentVersion->speedMax / 4, newMaxSpeed / 4, cUpgradeCalculator::kSpeed, player.researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}

	return cost;
}

//------------------------------------------------------------------------------
void cServer::placeInitialResources ()
{
	for (auto i = playerLandingPositions.begin (); i != playerLandingPositions.end (); ++i)
	{
		auto& landingPosition = i->second;
		correctLandingPos (landingPosition);
		Map->placeRessourcesAddPlayer (landingPosition, gameSettings->getResourceDensity ());
	}
	Map->placeRessources (gameSettings->getMetalAmount(), gameSettings->getOilAmount(), gameSettings->getGoldAmount());
}

//------------------------------------------------------------------------------
cVehicle* cServer::landVehicle (const cPosition& landingPosition, int iWidth, int iHeight, const sUnitData& unitData, cPlayer& player)
{
	for (int offY = -iHeight / 2; offY < iHeight / 2; ++offY)
	{
		for (int offX = -iWidth / 2; offX < iWidth / 2; ++offX)
		{
			if (!Map->possiblePlaceVehicle (unitData, landingPosition + cPosition (offX, offY), &player)) continue;

			return &addVehicle (landingPosition + cPosition (offX, offY), unitData.ID, &player, true);
		}
	}
	return NULL;
}

//------------------------------------------------------------------------------
//void cServer::makeLanding (const std::vector<cPosition>& landPos,
//						   const std::vector<std::vector<sLandingUnit>*>& landingUnits)
//{
//	const bool fixed = gameSetting->getBridgeheadType () == eGameSettingsBridgeheadType::Definite;
//	for (size_t i = 0; i != playerList.size(); ++i)
//	{
//		const int x = landPos[i].x ();
//		const int y = landPos[i].y ();
//		auto& player = *playerList[i];
//
//		makeLanding (x, y, &player, *landingUnits[i], fixed);
//	}
//}

//------------------------------------------------------------------------------
void cServer::makeLanding()
{
	const bool fixed = gameSettings->getBridgeheadType () == eGameSettingsBridgeheadType::Definite;
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		auto& player = *playerList[i];

		makeLanding (playerLandingPositions.at(&player), player, playerLandingUnits.at(&player), fixed);
	}
}

//------------------------------------------------------------------------------
void cServer::makeLanding (const cPosition& landingPosition, cPlayer& player, const std::vector<sLandingUnit>& landingUnits, bool isFixed)
{
	// Find place for mine if bridgehead is fixed
	if (isFixed)
	{
		if (Map->possiblePlaceBuilding (*UnitsData.specialIDSmallGen.getUnitDataOriginalVersion (), landingPosition + cPosition (-1, 0)) &&
			Map->possiblePlaceBuilding (*UnitsData.specialIDMine.getUnitDataOriginalVersion (), landingPosition + cPosition (0, -1)) &&
			Map->possiblePlaceBuilding (*UnitsData.specialIDMine.getUnitDataOriginalVersion (), landingPosition + cPosition (1, -1)) &&
			Map->possiblePlaceBuilding (*UnitsData.specialIDMine.getUnitDataOriginalVersion (), landingPosition + cPosition (1, 0)) &&
			Map->possiblePlaceBuilding (*UnitsData.specialIDMine.getUnitDataOriginalVersion (), landingPosition + cPosition (0, 0)))
		{
			// place buildings:
			addBuilding (landingPosition + cPosition (-1, 0), UnitsData.specialIDSmallGen, &player, true);
			addBuilding (landingPosition + cPosition (0, -1), UnitsData.specialIDMine, &player, true);
		}
		else
		{
			Log.write ("couldn't place player start mine: " + player.getName(), cLog::eLOG_TYPE_ERROR);
		}
	}

	int iWidth = 2;
	int iHeight = 2;
	for (size_t i = 0; i != landingUnits.size(); ++i)
	{
		const sLandingUnit& Landing = landingUnits[i];
		cVehicle* Vehicle = landVehicle (landingPosition, iWidth, iHeight, *Landing.unitID.getUnitDataOriginalVersion (&player), player);
		while (!Vehicle)
		{
			iWidth += 2;
			iHeight += 2;
			Vehicle = landVehicle (landingPosition, iWidth, iHeight, *Landing.unitID.getUnitDataOriginalVersion (&player), player);
		}
		if (Landing.cargo && Vehicle)
		{
			Vehicle->data.setStoredResources(Landing.cargo);
			sendUnitData (*this, *Vehicle, *Vehicle->owner);
		}
	}
}

//------------------------------------------------------------------------------
void cServer::correctLandingPos (cPosition& landingPosition)
{
	int iWidth = 2;
	int iHeight = 2;
	const int margin = 1;
	while (true)
	{
		for (int offY = -iHeight / 2; offY < iHeight / 2; ++offY)
		{
			for (int offX = -iWidth / 2; offX < iWidth / 2; ++offX)
			{
				if (Map->possiblePlaceBuildingWithMargin (*UnitsData.specialIDSmallGen.getUnitDataOriginalVersion (), landingPosition + cPosition (offX, offY + 1), margin) &&
					Map->possiblePlaceBuildingWithMargin (*UnitsData.specialIDMine.getUnitDataOriginalVersion (), landingPosition + cPosition (offX + 1, offY), margin) &&
					Map->possiblePlaceBuildingWithMargin (*UnitsData.specialIDMine.getUnitDataOriginalVersion (), landingPosition + cPosition (offX + 2, offY), margin) &&
					Map->possiblePlaceBuildingWithMargin (*UnitsData.specialIDMine.getUnitDataOriginalVersion (), landingPosition + cPosition (offX + 2, offY + 1), margin) &&
					Map->possiblePlaceBuildingWithMargin (*UnitsData.specialIDMine.getUnitDataOriginalVersion (), landingPosition + cPosition (offX + 1, offY + 1), margin))
				{
					landingPosition.x() += offX + 1;
					landingPosition.y() += offY + 1;
					return;
				}
			}
		}
		iWidth += 2;
		iHeight += 2;
	}
}

//------------------------------------------------------------------------------
void cServer::startNewGame()
{
	assert (serverState == SERVER_STATE_INITGAME);
	// send victory conditions to clients
	for (size_t i = 0; i != playerList.size(); ++i)
		sendGameSettings (*this, *playerList[i]);
	// send clan info to clients
	if (gameSettings->getClansEnabled())
		sendClansToClients (*this, playerList);

	// place resources
	placeInitialResources ();
	// make the landing
	makeLanding();
	serverState = SERVER_STATE_INGAME;
}

//------------------------------------------------------------------------------
cVehicle& cServer::addVehicle (const cPosition& position, const sID& id, cPlayer* Player, bool bInit, bool bAddToMap, unsigned int uid)
{
	// generate the vehicle:
	cVehicle& addedVehicle = Player->addNewVehicle(position, id, uid ? uid : iNextUnitID);
	iNextUnitID++;

	// place the vehicle:
	if (bAddToMap) Map->addVehicle (addedVehicle, position);

	// scan with surveyor:
	if (addedVehicle.data.canSurvey)
	{
		sendVehicleResources (*this, addedVehicle);
		addedVehicle.doSurvey (*this);
	}
	if (!bInit) addedVehicle.InSentryRange (*this);

	if (addedVehicle.canLand (*Map))
	{
		addedVehicle.setFlightHeight(0);
	}
	else
	{
		addedVehicle.setFlightHeight(64);
	}

	sendAddUnit(*this, position, addedVehicle.iID, true, id, *Player, bInit);

	// detection must be done, after the vehicle has been sent to clients
	addedVehicle.makeDetection (*this);
	return addedVehicle;
}

//------------------------------------------------------------------------------
cBuilding& cServer::addBuilding (const cPosition& position, const sID& id, cPlayer* Player, bool bInit, unsigned int uid)
{
	// generate the building:
	cBuilding& addedBuilding = Player->addNewBuilding (position, id, uid ? uid : iNextUnitID);
	if (addedBuilding.data.canMineMaxRes > 0) addedBuilding.CheckRessourceProd (*this);
	if (addedBuilding.isSentryActive ()) Player->addSentry (addedBuilding);

	iNextUnitID++;

	cBuilding* buildingToBeDeleted = Map->getField(position).getTopBuilding();

	Map->addBuilding (addedBuilding, position);
	sendAddUnit(*this, position, addedBuilding.iID, false, id, *Player, bInit);

	// integrate the building to the base:
	Player->base.addBuilding (&addedBuilding, this);

	// if this is a top building, delete connectors, mines and roads
	if (addedBuilding.data.surfacePosition == sUnitData::SURFACE_POS_GROUND)
	{
		if (addedBuilding.data.isBig)
		{
			auto bigPosition = position;
			auto buildings = &Map->getField(bigPosition).getBuildings();

			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ((*buildings) [i]->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit ((*buildings) [i]);
					--i;
				}
			}
			bigPosition.x()++;
			buildings = &Map->getField(bigPosition).getBuildings();
			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ((*buildings) [i]->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit ((*buildings) [i]);
					--i;
				}
			}
			bigPosition.y()++;
			buildings = &Map->getField(bigPosition).getBuildings();
			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ((*buildings) [i]->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit ((*buildings) [i]);
					--i;
				}
			}
			bigPosition.x()--;
			buildings = &Map->getField(bigPosition).getBuildings();
			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ((*buildings) [i]->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit ((*buildings) [i]);
					--i;
				}
			}
		}
		else
		{
			deleteUnit (buildingToBeDeleted);

			const auto& buildings = Map->getField(position).getBuildings();
			for (size_t i = 0; i != buildings.size(); ++i)
			{
				if (buildings[i]->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit (buildings[i]);
					--i;
				}
			}
		}
	}

	if (addedBuilding.data.canMineMaxRes > 0)
	{
		sendProduceValues (*this, addedBuilding);
		addedBuilding.ServerStartWork (*this);
	}
	addedBuilding.makeDetection (*this);
	return addedBuilding;
}

//------------------------------------------------------------------------------
void cServer::deleteUnit (cUnit* unit, bool notifyClient)
{
	if (unit == 0)
		return;

	if (unit->isABuilding() && unit->owner == 0)
	{
		deleteRubble (static_cast<cBuilding*> (unit));
		return;
	}

	cPlayer* owner = unit->owner;

	if (unit->owner && casualtiesTracker != NULL && ((unit->isABuilding() && unit->data.buildCosts <= 2) == false))
		casualtiesTracker->logCasualty (unit->data.ID, unit->owner->getNr());

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

	// detach from attack job
	if (unit->isAttacking ())
	{
		for (size_t i = 0; i != AJobs.size(); ++i)
		{
			if (AJobs[i]->unit == unit)
				AJobs[i]->unit = 0;
		}
	}

	helperJobs.onRemoveUnit (unit);

	// detach from move job
	if (unit->isAVehicle())
	{
		cVehicle* vehicle = static_cast<cVehicle*> (unit);
		if (vehicle->ServerMoveJob)
		{
			vehicle->ServerMoveJob->Vehicle = NULL;
		}
	}

	// remove from sentry list
	unit->owner->deleteSentry (*unit);

	// lose eco points
	if (unit->isABuilding() && static_cast<cBuilding*> (unit)->points != 0)
	{
		unit->owner->setScore (unit->owner->getScore (turnClock->getTurn ()) - static_cast<cBuilding*> (unit)->points, turnClock->getTurn ());
		sendScore (*this, *unit->owner, turnClock->getTurn ());
		sendNumEcos (*this, *unit->owner);
	}

	if (unit->isABuilding())
		Map->deleteBuilding (*static_cast<cBuilding*> (unit));
	else
		Map->deleteVehicle (*static_cast<cVehicle*> (unit));

	if (notifyClient)
		sendDeleteUnit (*this, *unit, nullptr);

	if (unit->isABuilding() && static_cast<cBuilding*> (unit)->SubBase != 0)
		unit->owner->base.deleteBuilding (static_cast<cBuilding*> (unit), this);

	if (owner != 0)
	{
		owner->doScan ();
	}
}

//------------------------------------------------------------------------------
void cServer::checkPlayerUnits (cVehicle& vehicle, cPlayer& MapPlayer)
{
	if (&MapPlayer == vehicle.owner) return;

	std::vector<cPlayer*>& seenByPlayers = vehicle.seenByPlayerList;
	const bool stealthUnit = vehicle.data.isStealthOn != TERRAIN_NONE;

	if (MapPlayer.canSeeAnyAreaUnder (vehicle) && !vehicle.isUnitLoaded () &&
		(!stealthUnit || vehicle.isDetectedByPlayer (&MapPlayer) || (MapPlayer.isDefeated && openMapDefeat)))
	{
		if (Contains (seenByPlayers, &MapPlayer) == false)
		{
			seenByPlayers.push_back (&MapPlayer);
			sendAddEnemyUnit (*this, vehicle, MapPlayer);
			sendUnitData (*this, vehicle, MapPlayer);
			if (vehicle.ServerMoveJob)
			{
				sendMoveJobServer (*this, *vehicle.ServerMoveJob, MapPlayer);
				if (Contains (ActiveMJobs, vehicle.ServerMoveJob) && !vehicle.ServerMoveJob->bFinished && !vehicle.ServerMoveJob->bEndForNow && vehicle.isUnitMoving ())
				{
					Log.write (" Server: sending extra MJOB_OK for unit ID " + iToStr (vehicle.iID) + " to client " + iToStr (MapPlayer.getNr()), cLog::eLOG_TYPE_NET_DEBUG);
					AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_NEXT_MOVE));
					message->pushChar (MJOB_OK);
					message->pushInt16 (vehicle.iID);
					sendNetMessage (message, &MapPlayer);
				}
			}
		}
	}
	else
	{
		std::vector<cPlayer*>::iterator it = std::find (seenByPlayers.begin(), seenByPlayers.end(), &MapPlayer);

		if (it != seenByPlayers.end())
		{
			seenByPlayers.erase (it);
			sendDeleteUnit (*this, vehicle, &MapPlayer);
		}
	}
}

//------------------------------------------------------------------------------
void cServer::checkPlayerUnits (cBuilding& building, cPlayer& MapPlayer)
{
	if (&MapPlayer == building.owner) return;
	std::vector<cPlayer*>& seenByPlayers = building.seenByPlayerList;
	const bool stealthUnit = building.data.isStealthOn != TERRAIN_NONE;

	if (MapPlayer.canSeeAnyAreaUnder (building) &&
		(!stealthUnit || building.isDetectedByPlayer (&MapPlayer) || (MapPlayer.isDefeated && openMapDefeat)))
	{
		if (Contains (seenByPlayers, &MapPlayer) == false)
		{
			seenByPlayers.push_back (&MapPlayer);
			sendAddEnemyUnit (*this, building, MapPlayer);
			sendUnitData (*this, building, MapPlayer);
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
		const auto& vehicles = unitPlayer.getVehicles ();
		for (auto i = vehicles.begin (); i != vehicles.end (); ++i)
		{
			const auto& vehicle = *i;
			for (size_t j = 0; j != playerList.size (); ++j)
			{
				checkPlayerUnits (*vehicle, *playerList[j]);
			}
		}
		const auto& buildings = unitPlayer.getBuildings ();
		for (auto i = buildings.begin (); i != buildings.end (); ++i)
		{
			const auto& building = *i;
			for (size_t j = 0; j != playerList.size (); ++j)
			{
				checkPlayerUnits (*building, *playerList[j]);
			}
		}
	}

	//check the neutral objects
	for (auto i = neutralBuildings.begin (); i != neutralBuildings.end (); ++i)
	{
		const auto& building = *i;
		for (size_t i = 0; i != playerList.size (); ++i)
		{
			checkPlayerRubbles (*building, *playerList[i]);
		}
	}
}

//------------------------------------------------------------------------------
bool cServer::isPlayerDisconnected (const cPlayer& player) const
{
	if (player.isLocal()) return false;

	if (network)
		return !network->isConnected (player.getSocketNum());

	return true;
}

//------------------------------------------------------------------------------
void cServer::kickPlayer (cPlayer* player)
{
	// close the socket
	const int socketIndex = player->getSocketNum();
	if (network) network->close (socketIndex);
	for (size_t i = 0; i != playerList.size (); ++i)
	{
		playerList[i]->onSocketIndexDisconnected (socketIndex);
	}
	deletePlayer (player);
}

//------------------------------------------------------------------------------
void cServer::markAllPlayersAsDisconnected()
{
	for (size_t i = 0; i != playerList.size (); ++i)
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
	auto iter = std::find_if (playerList.begin (), playerList.end (), [playerNumber](const std::unique_ptr<cPlayer>& player){ return player->getNr () == playerNumber; });
	if (iter == playerList.end ()) throw std::runtime_error ("Could not find player with number '" + iToStr(playerNumber) + "'");

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
	auto iter = std::find_if (playerList.begin (), playerList.end (), [&playerID](const std::unique_ptr<cPlayer>& player){ return player->getName () == playerID; });
	return iter == playerList.end () ? nullptr : iter->get ();
}

//------------------------------------------------------------------------------
void cServer::handleEnd (const cPlayer& player)
{
	const eGameTypes gameType = getGameType();

	if (gameType == GAME_TYPE_SINGLE)
	{
		sendTurnFinished (*this, player);
		if (checkEndActions (&player))
		{
			iWantPlayerEndNum = player.getNr();
			return;
		}
		makeTurnEnd();
	}
	else if (gameType == GAME_TYPE_HOTSEAT || isTurnBasedGame())
	{
		const bool bWaitForPlayer = (gameType == GAME_TYPE_TCPIP && isTurnBasedGame());
		if (checkEndActions (&player))
		{
			iWantPlayerEndNum = player.getNr();
			return;
		}

		// select next player
		auto nextPlayerIter = std::find_if (playerList.begin (), playerList.end (), [this](const std::unique_ptr<cPlayer>& player){return player.get() == activeTurnPlayer; });
		assert (nextPlayerIter != playerList.end ());
		++nextPlayerIter;

		if (nextPlayerIter == playerList.end ())
		{
			activeTurnPlayer = playerList.front ().get ();

			makeTurnEnd();

			if (gameType == GAME_TYPE_HOTSEAT)
			{
				sendMakeTurnEnd (*this, true, bWaitForPlayer, activeTurnPlayer->getNr (), player);
			}
			else
			{
				for (size_t i = 0; i != playerList.size (); ++i)
				{
					sendMakeTurnEnd (*this, true, bWaitForPlayer, activeTurnPlayer->getNr (), *playerList[i]);
				}
			}
		}
		else
		{
			activeTurnPlayer = nextPlayerIter->get ();

			if (gameType == GAME_TYPE_HOTSEAT)
			{
				sendMakeTurnEnd (*this, false, bWaitForPlayer, activeTurnPlayer->getNr (), player);
				// TODO: in hotseat:
				// maybe send information to client about the next player
			}
			else
			{
				for (size_t i = 0; i != playerList.size (); ++i)
				{
					sendMakeTurnEnd (*this, false, bWaitForPlayer, activeTurnPlayer->getNr (), *playerList[i]);
				}
			}
		}
		// send report to next player
		sendSavedReport (*this, cSavedReportTurnStart (*activeTurnPlayer, turnClock->getTurn()), activeTurnPlayer);
		activeTurnPlayer->resetTurnReportData ();
	}
	else // it's a simultaneous TCP/IP multiplayer game
	{
		// defeated player are ignored when they hit the end button
		if (player.isDefeated) return;

		// check whether this player has already finished his turn
		for (size_t i = 0; i != PlayerEndList.size(); ++i)
		{
			if (PlayerEndList[i]->getNr() == player.getNr()) return;
		}
		PlayerEndList.push_back (&player);
		const bool firstTimeEnded = PlayerEndList.size() == 1;

		// make sure that all defeated players are added to the endlist
		for (size_t i = 0; i != playerList.size(); ++i)
		{
			auto& currentPlayer = *playerList[i];
			if (currentPlayer.isDefeated)
			{
				if (Contains (PlayerEndList, &currentPlayer) == false)
					PlayerEndList.push_back (&currentPlayer);
			}
		}

		if (iWantPlayerEndNum == -1)
		{
			// When playing with dedicated server
			// where a player is not connected, play without a deadline,
			// but wait till all players pressed "End".
			if (firstTimeEnded && (DEDICATED_SERVER == false || DisconnectedPlayerList.empty()))
			{
				sendTurnFinished (*this, player);
				if (gameSettings->isTurnEndDeadlineActive ())
				{
					turnEndDeadline = turnTimeClock->startNewDeadlineFromNow (gameSettings->getTurnEndDeadline ());
					sendTurnEndDeadlineStartTime (*this, turnEndDeadline->getStartGameTime());
				}
			}
			else
			{
				sendTurnFinished (*this, player);
			}
		}

		if (PlayerEndList.size() >= playerList.size())
		{
			if (checkEndActions (nullptr))
			{
				iWantPlayerEndNum = player.getNr ();
				return;
			}

			PlayerEndList.clear();

			makeTurnEnd();
		}
	}
}

//------------------------------------------------------------------------------
void cServer::handleWantEnd()
{
	if (!gameTimer->timer50ms) return;

	// wait until all clients have reported a gametime
	// that is after the turn end.
	// that means they have finished processing all the turn end messages,
	// and we can start the new turn simultaneously on all clients
	if (freezeModes.waitForTurnEnd && !executingRemainingMovements)
	{
		for (size_t i = 0; i != playerList.size(); ++i)
		{
			cPlayer& player = *playerList[i];
			if (!isPlayerDisconnected (player) && gameTimer->getReceivedTime (i) <= lastTurnEnd)
				return;
		}

		// send reports to all players
		for (size_t i = 0; i != playerList.size (); ++i)
		{
			sendMakeTurnEnd (*this, true, false, -1, *playerList[i]);
		}
		for (size_t i = 0; i != playerList.size (); ++i)
		{
			sendSavedReport (*this, cSavedReportTurnStart (*playerList[i], turnClock->getTurn ()), playerList[i].get ());
			playerList[i]->resetTurnReportData ();
		}

		// begin the new turn
		disableFreezeMode (FREEZE_WAIT_FOR_TURNEND);
	}

	if (iWantPlayerEndNum != -1 && iWantPlayerEndNum != -2)
	{
		for (size_t i = 0; i != PlayerEndList.size(); ++i)
		{
			if (iWantPlayerEndNum == PlayerEndList[i]->getNr())
			{
				PlayerEndList.erase (PlayerEndList.begin() + i);
				break;
			}
		}
		handleEnd (getPlayerFromNumber(iWantPlayerEndNum));
	}
}

//------------------------------------------------------------------------------
bool cServer::checkEndActions (const cPlayer* player)
{
	enableFreezeMode (FREEZE_WAIT_FOR_TURNEND);

	executingRemainingMovements = false;
	bool remainingAutomove = false;
	if (!ActiveMJobs.empty())
	{
		executingRemainingMovements = true;
	}
	else
	{
		for (size_t i = 0; i != playerList.size (); ++i)
		{
			cPlayer& player = *playerList[i];
			const auto& vehicles = player.getVehicles ();
			for (auto i = vehicles.begin (); i != vehicles.end (); ++i)
			{
				const auto& vehicle = *i;
				if (vehicle->ServerMoveJob && vehicle->data.speedCur > 0 && !vehicle->isUnitMoving ())
				{
					// restart movejob
					vehicle->ServerMoveJob->resume ();
					executingRemainingMovements = true;
					remainingAutomove = true;
				}
			}
		}
	}
	if (executingRemainingMovements)
	{
		if (player != nullptr)
		{
			if (iWantPlayerEndNum == -1)
			{
				if (remainingAutomove) sendSavedReport (*this, cSavedReportSimple (eSavedReportType::TurnAutoMove), player);
				else sendSavedReport (*this, cSavedReportSimple (eSavedReportType::TurnWait), player);
				
			}
		}
		else
		{
			if (iWantPlayerEndNum == -1)
			{
				for (size_t i = 0; i != playerList.size(); ++i)
				{
					if (remainingAutomove) sendSavedReport (*this, cSavedReportSimple (eSavedReportType::TurnAutoMove), playerList[i].get ());
					else sendSavedReport (*this, cSavedReportSimple (eSavedReportType::TurnWait), playerList[i].get ());
				}
			}
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void cServer::makeTurnEnd()
{
	turnClock->increaseTurn ();
	startTurnTimers ();

	enableFreezeMode (FREEZE_WAIT_FOR_TURNEND);
	lastTurnEnd = gameTimer->gameTime;

	// reload all buildings
	for (size_t i = 0; i != playerList.size (); ++i)
	{
		cPlayer& player = *playerList[i];
		const auto& buildings = player.getBuildings ();
		for (auto i = buildings.begin (); i != buildings.end (); ++i)
		{
			const auto& building = *i;

			bool forceSendUnitData = false;
			if (building->isDisabled ())
			{
				building->setDisabledTurns (building->getDisabledTurns () - 1);
				if (building->isDisabled () == false && building->wasWorking)
				{
					building->ServerStartWork (*this);
					building->wasWorking = false;
				}
				forceSendUnitData = true;
			}
			if ((building->data.canAttack && building->refreshData ()) || forceSendUnitData)
			{
				for (size_t k = 0; k != building->seenByPlayerList.size (); ++k)
				{
					sendUnitData (*this, *building, *building->seenByPlayerList[k]);
				}
				sendUnitData (*this, *building, *building->owner);
			}
		}
	}

	// reload all vehicles
	for (size_t i = 0; i != playerList.size (); ++i)
	{
		cPlayer& player = *playerList[i];
		const auto& vehicles = player.getVehicles ();
		for (auto i = vehicles.begin (); i != vehicles.end (); ++i)
		{
			const auto& vehicle = *i;

			bool isModified = false;
			if (vehicle->isDisabled ())
			{
				vehicle->setDisabledTurns (vehicle->getDisabledTurns () - 1);
				isModified = true;
			}
			isModified |= vehicle->refreshData ();
			isModified |= vehicle->refreshData_Build (*this);
			isModified |= vehicle->refreshData_Clear (*this);

			if (isModified)
			{
				for (size_t k = 0; k != vehicle->seenByPlayerList.size (); ++k)
				{
					sendUnitData (*this, *vehicle, *vehicle->seenByPlayerList[k]);
				}
				sendUnitData (*this, *vehicle, *vehicle->owner);
			}
			if (vehicle->ServerMoveJob) vehicle->ServerMoveJob->bEndForNow = false;
		}
	}

	// hide stealth units
	for (size_t i = 0; i != playerList.size (); ++i)
	{
		cPlayer& player = *playerList[i];
		player.doScan(); // make sure the detection maps are up to date

		const auto& vehicles = player.getVehicles ();
		for (auto i = vehicles.begin (); i != vehicles.end (); ++i)
		{
			const auto& vehicle = *i;
			vehicle->clearDetectedInThisTurnPlayerList();
			vehicle->makeDetection (*this);
		}
	}

	// produce resources
	for (size_t i = 0; i != playerList.size (); ++i)
	{
		playerList[i]->base.handleTurnend (*this);
	}

	// do research:
	for (size_t i = 0; i != playerList.size (); ++i)
		playerList[i]->doResearch (*this);

	// eco-spheres:
	for (size_t i = 0; i != playerList.size (); ++i)
	{
		playerList[i]->accumulateScore (*this);
	}

	// Gun'em down:
	for (size_t i = 0; i != playerList.size (); ++i)
	{
		cPlayer& player = *playerList[i];

		const auto& vehicles = player.getVehicles ();
		for (auto i = vehicles.begin (); i != vehicles.end (); ++i)
		{
			const auto& vehicle = *i;
			vehicle->InSentryRange (*this);
		}
	}

	if (DEDICATED_SERVER == false)
	{
		// FIXME: saving of running attack jobs does not work correctly yet.
		// make autosave
		if (cSettings::getInstance().shouldAutosave())
		{
			cSavegame Savegame (10); // autosaves are always in slot 10
			Savegame.save (*this, lngPack.i18n ("Text~Comp~Turn_5") + " " + iToStr (turnClock->getTurn ()) + " - " + lngPack.i18n ("Text~Settings~Autosave"));
			makeAdditionalSaveRequest (10);
		}
	}
#if DEDICATED_SERVER_APPLICATION
	else
	{
		cDedicatedServer::instance().doAutoSave (*this);
	}
#endif

	checkDefeats();

	iWantPlayerEndNum = -1;
}

//------------------------------------------------------------------------------
void cServer::startTurnTimers ()
{
	turnTimeClock->restartFromNow ();
	sendTurnStartTime (*this, turnTimeClock->getStartGameTime ());
	turnTimeClock->clearAllDeadlines ();
	if (gameSettings->isTurnLimitActive ())
	{
		turnLimitDeadline = turnTimeClock->startNewDeadlineFrom (turnTimeClock->getStartGameTime (), gameSettings->getTurnLimit ());
	}
}

//------------------------------------------------------------------------------
bool cServer::isVictoryConditionMet() const
{
	switch (gameSettings->getVictoryCondition())
	{
		case eGameSettingsVictoryCondition::Turns:
			return turnClock->getTurn () >= static_cast<int>(gameSettings->getVictoryTurns ());
		case eGameSettingsVictoryCondition::Points:
		{
			for (size_t i = 0; i != playerList.size (); ++i)
			{
				const cPlayer& player = *playerList[i];
				if (player.isDefeated) continue;
				if (player.getScore (turnClock->getTurn ()) >= static_cast<int>(gameSettings->getVictoryPoints ())) return true;
			}
			return false;
		}
		case eGameSettingsVictoryCondition::Death:
		{
			int nbActivePlayer = 0;
			for (size_t i = 0; i != playerList.size (); ++i)
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
	for (size_t i = 0; i != playerList.size (); ++i)
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

	for (size_t i = 0; i != playerList.size (); ++i)
	{
		cPlayer& player = *playerList[i];
		if (player.isDefeated) continue;
		const int score = player.getScore (turnClock->getTurn ());

		if (score < best_score) continue;

		if (score > best_score)
		{
			winners.clear();
			best_score = score;
		}
		winners.insert (&player);
	}

	// anyone who hasn't won has lost.
	for (size_t i = 0; i != playerList.size (); ++i)
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
void cServer::checkDeadline()
{
	if (!gameTimer->timer50ms) return;

	if (!turnTimeClock->hasReachedAnyDeadline ()) return;

	if (checkEndActions (nullptr))
	{
		iWantPlayerEndNum = -2;
		return;
	}

	PlayerEndList.clear();

	makeTurnEnd();
}

//------------------------------------------------------------------------------
void cServer::addActiveMoveJob (cServerMoveJob& MoveJob)
{
	ActiveMJobs.push_back (&MoveJob);
}

//------------------------------------------------------------------------------
void cServer::handleMoveJobs()
{
	for (int i = ActiveMJobs.size() - 1; i >= 0; i--)
	{
		cServerMoveJob* MoveJob = ActiveMJobs[i];
		cVehicle* Vehicle = MoveJob->Vehicle;

		//suspend movejobs of attacked vehicles
		if (Vehicle && Vehicle->isBeeingAttacked ())
			continue;

		// stop the job
		if (MoveJob->bEndForNow && Vehicle)
		{
			Log.write (" Server: Movejob has end for now and will be stopped (delete from active ones)", cLog::eLOG_TYPE_NET_DEBUG);
			sendNextMove (*this, *Vehicle, MJOB_STOP, MoveJob->iSavedSpeed);
			ActiveMJobs.erase (ActiveMJobs.begin() + i);
			continue;
		}
		else if (MoveJob->bFinished || MoveJob->Vehicle == NULL)
		{
			if (Vehicle && Vehicle->ServerMoveJob == MoveJob)
			{
				Log.write (" Server: Movejob is finished and will be deleted now", cLog::eLOG_TYPE_NET_DEBUG);
				Vehicle->ServerMoveJob = NULL;
				Vehicle->setMoving (false);
				Vehicle->MoveJobActive = false;

				sendNextMove (*this, *Vehicle, MJOB_FINISHED);
			}
			else Log.write (" Server: Delete movejob with nonactive vehicle (released one)", cLog::eLOG_TYPE_NET_DEBUG);

			// execute endMoveAction
			if (Vehicle && MoveJob->endAction) MoveJob->endAction->execute (*this);

			delete MoveJob;
			ActiveMJobs.erase (ActiveMJobs.begin() + i);


			// continue path building
			if (Vehicle && Vehicle->BuildPath)
			{
				if (Vehicle->data.getStoredResources () >= Vehicle->getBuildCostsStart () && Map->possiblePlaceBuilding (*Vehicle->getBuildingType ().getUnitDataOriginalVersion (), Vehicle->getPosition (), Vehicle))
				{
					addJob (new cStartBuildJob (*Vehicle, Vehicle->getPosition(), Vehicle->data.isBig));
					Vehicle->setBuildingABuilding(true);
					Vehicle->setBuildCosts(Vehicle->getBuildCostsStart());
					Vehicle->setBuildTurns(Vehicle->getBuildTurnsStart ());
					sendBuildAnswer (*this, true, *Vehicle);
				}
				else
				{
					Vehicle->BuildPath = false;
					sendBuildAnswer (*this, false, *Vehicle);
				}
			}

			continue;
		}

		if (Vehicle == NULL) continue;

		if (!Vehicle->isUnitMoving ())
		{
			if (!MoveJob->checkMove() && !MoveJob->bFinished)
			{
				ActiveMJobs.erase (ActiveMJobs.begin() + i);
				delete MoveJob;
				Vehicle->ServerMoveJob = NULL;
				Log.write (" Server: Movejob deleted and informed the clients to stop this movejob", LOG_TYPE_NET_DEBUG);
				continue;
			}
			if (MoveJob->bEndForNow)
			{
				Log.write (" Server: Movejob has end for now and will be stopped (delete from active ones)", cLog::eLOG_TYPE_NET_DEBUG);
				sendNextMove (*this, *Vehicle, MJOB_STOP, MoveJob->iSavedSpeed);
				ActiveMJobs.erase (ActiveMJobs.begin() + i);
				continue;
			}
		}

		if (MoveJob->iNextDir != Vehicle->dir)
		{
			// rotate vehicle
			if (gameTimer->timer100ms)
			{
				Vehicle->rotateTo (MoveJob->iNextDir);
			}
		}
		else
		{
			// move the vehicle
			if (gameTimer->timer10ms)
			{
				MoveJob->moveVehicle();
			}
		}
	}
}

//------------------------------------------------------------------------------
cUnit* cServer::getUnitFromID (unsigned int iID) const
{
	cUnit* result = getVehicleFromID (iID);
	if (result == NULL)
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
	for (size_t i = 0; i != playerList.size (); ++i)
	{
		auto unit = playerList[i]->getBuildingFromId (id);
		if (unit) return unit;
	}
	return 0;
}

//------------------------------------------------------------------------------
void cServer::destroyUnit (cVehicle& vehicle)
{
	int value = 0;
	int oldRubbleValue = 0;
	bool bigRubble = false;
	auto rubblePosition = vehicle.getPosition();

	if (vehicle.data.factorAir > 0 && vehicle.getFlightHeight () != 0)
	{
		deleteUnit (&vehicle);
		return;
	}

	//delete all buildings on the field, except connectors
	auto buildings = &Map->getField(vehicle.getPosition()).getBuildings();
	auto b_it = buildings->begin();
	if (b_it != buildings->end() && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;

	while (b_it != buildings->end())
	{
		// this seems to be rubble
		if ((*b_it)->owner == 0 && (*b_it)->RubbleValue > 0)
		{
			oldRubbleValue += (*b_it)->RubbleValue;
			if ((*b_it)->data.isBig)
			{
				rubblePosition = (*b_it)->getPosition();
				bigRubble = true;
			}
		}
		else // normal unit
			value += (*b_it)->data.buildCosts;
		deleteUnit (*b_it);
		b_it = buildings->begin();
		if (b_it != buildings->end() && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;
	}

	if (vehicle.data.isBig)
	{
		bigRubble = true;
		buildings = &Map->getField(vehicle.getPosition() + cPosition(1, 0)).getBuildings();
		b_it = buildings->begin();
		if (b_it != buildings->end() && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;
		while (b_it != buildings->end())
		{
			value += (*b_it)->data.buildCosts;
			deleteUnit (*b_it);
			b_it = buildings->begin();
			if (b_it != buildings->end() && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;
		}

		buildings = &Map->getField(vehicle.getPosition() + cPosition(0, 1)).getBuildings();
		b_it = buildings->begin();
		if (b_it != buildings->end() && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;
		while (b_it != buildings->end())
		{
			value += (*b_it)->data.buildCosts;
			deleteUnit (*b_it);
			b_it = buildings->begin();
			if (b_it != buildings->end() && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;
		}

		buildings = &Map->getField(vehicle.getPosition() + cPosition(1, 1)).getBuildings();
		b_it = buildings->begin();
		if (b_it != buildings->end() && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;
		while (b_it != buildings->end())
		{
			value += (*b_it)->data.buildCosts;
			deleteUnit (*b_it);
			b_it = buildings->begin();
			if (b_it != buildings->end() && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;
		}
	}

	if (!vehicle.data.hasCorpse)
	{
		value += vehicle.data.buildCosts;
		// stored material is always added completely to the rubble
		if (vehicle.data.storeResType == sUnitData::STORE_RES_METAL)
			value += vehicle.data.getStoredResources () * 2;
	}

	if (value > 0 || oldRubbleValue > 0)
		addRubble (rubblePosition, value / 2 + oldRubbleValue, bigRubble);

	deleteUnit (&vehicle);
}

//------------------------------------------------------------------------------
int cServer::deleteBuildings (const std::vector<cBuilding*>& buildings)
{
	int rubble = 0;
	while (buildings.empty() == false)
	{
		cBuilding* building = buildings[0];
		if (building->owner)
		{
			rubble += building->data.buildCosts;
			if (building->data.storeResType == sUnitData::STORE_RES_METAL)
				rubble += building->data.getStoredResources () * 2; // stored material is always added completely to the rubble
		}
		else
			rubble += building->RubbleValue * 2;
		deleteUnit (building);
	}
	return rubble;
}

//------------------------------------------------------------------------------
void cServer::destroyUnit (cBuilding& b)
{
	auto position = b.getPosition();
	int rubble = 0;
	bool big = false;

	cBuilding* topBuilding = Map->getField(position).getTopBuilding();
	if (topBuilding && topBuilding->data.isBig)
	{
		big = true;
		position = topBuilding->getPosition();

		auto buildings = &Map->getField(position + cPosition(0, 1)).getBuildings();
		rubble += deleteBuildings (*buildings);

		buildings = &Map->getField(position + cPosition(1, 0)).getBuildings();
		rubble += deleteBuildings (*buildings);

		buildings = &Map->getField(position + cPosition(1, 1)).getBuildings();
		rubble += deleteBuildings (*buildings);
	}

	sUnitData::eSurfacePosition surfacePosition = b.data.surfacePosition;

	auto buildings = &Map->getField(position).getBuildings();
	rubble += deleteBuildings (*buildings);

	if (surfacePosition != sUnitData::SURFACE_POS_ABOVE && rubble > 2)
		addRubble (position, rubble / 2, big);
}

//------------------------------------------------------------------------------
void cServer::addRubble (const cPosition& position, int value, bool big)
{
	value = std::max (1, value);

	if (Map->isWaterOrCoast (position))
	{
		if (big)
		{
			addRubble (position + cPosition(1, 0), value / 4, false);
			addRubble (position + cPosition(0, 1), value / 4, false);
			addRubble (position + cPosition(1, 1), value / 4, false);
		}
		return;
	}

	if(big && Map->isWaterOrCoast(position + cPosition(1, 0)))
	{
		addRubble (position,                   value / 4, false);
		addRubble (position + cPosition(0, 1), value / 4, false);
		addRubble (position + cPosition(1, 1), value / 4, false);
		return;
	}

	if (big && Map->isWaterOrCoast (position + cPosition(0, 1)))
	{
		addRubble (position,                   value / 4, false);
		addRubble (position + cPosition(1, 0), value / 4, false);
		addRubble (position + cPosition(1, 1), value / 4, false);
		return;
	}

	if (big && Map->isWaterOrCoast (position + cPosition(1, 1)))
	{
		addRubble (position,                   value / 4, false);
		addRubble (position + cPosition(1, 0), value / 4, false);
		addRubble (position + cPosition(0, 1), value / 4, false);
		return;
	}

    auto rubble = std::make_shared<cBuilding> (nullptr, nullptr, iNextUnitID);

	iNextUnitID++;

	rubble->setPosition(position);

	rubble->data.isBig = big;
	rubble->RubbleValue = value;

	Map->addBuilding (*rubble, position);

	if (big)
	{
		rubble->RubbleTyp = random (2);
	}
	else
	{
		rubble->RubbleTyp = random (5);
	}
	neutralBuildings.insert (std::move(rubble));
}

//------------------------------------------------------------------------------
void cServer::deleteRubble (cBuilding* rubble)
{
	Map->deleteBuilding (*rubble);

	auto iter = neutralBuildings.find (*rubble);
	assert (iter != neutralBuildings.end ());
	if (iter != neutralBuildings.end ()) neutralBuildings.erase (iter);

	sendDeleteUnit (*this, *rubble, nullptr);
}

//------------------------------------------------------------------------------
void cServer::deletePlayer (cPlayer* player)
{
	//remove units
	const auto& vehicles = player->getVehicles ();
    while (!vehicles.empty())
	{
        deleteUnit (vehicles.begin()->get());
	}
    const auto& buildings = player->getVehicles ();
    while (!buildings.empty ())
    {
        deleteUnit (buildings.begin ()->get ());
    }

	// remove the player of all detected by player lists
	for (size_t i = 0; i != playerList.size (); ++i)
	{
		cPlayer* unitPlayer = playerList[i].get();
		if (unitPlayer == player) continue;

		const auto& vehicles = unitPlayer->getVehicles ();
		for (auto i = vehicles.begin (); i != vehicles.end (); ++i)
		{
			const auto& vehicle = *i;
			if (vehicle->data.isStealthOn != TERRAIN_NONE && vehicle->isDetectedByPlayer (player)) vehicle->resetDetectedByPlayer (*this, player);
		}
	}
	// delete the player
	sendDeletePlayer (*this, *player, nullptr);
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		if (player == playerList[i].get ())
		{
			playerList.erase (playerList.begin () + i);
			break;
		}
	}
}

//------------------------------------------------------------------------------
void cServer::resyncPlayer (cPlayer& player, bool firstDelete)
{
	Log.write (" Server:  ============================= begin resync  ==========================", cLog::eLOG_TYPE_NET_DEBUG);
	if (firstDelete)
	{
		for (size_t i = 0; i != playerList.size(); ++i)
		{
			cPlayer* unitPlayer = playerList[i].get ();
			if (unitPlayer == &player) continue;

			const auto& vehicles = unitPlayer->getVehicles ();
			for (auto i = vehicles.begin (); i != vehicles.end (); ++i)
			{
				const auto& vehicle = *i;
				Remove (vehicle->seenByPlayerList, &player);
			}

			const auto& buildings = unitPlayer->getBuildings ();
			for (auto i = buildings.begin (); i != buildings.end (); ++i)
			{
				const auto& building = *i;
				Remove (building->seenByPlayerList, &player);
			}
		}

		for (auto i = neutralBuildings.begin (); i != neutralBuildings.end (); ++i)
		{
			const auto& building = *i;
			Remove (building->seenByPlayerList, &player);
		}
		sendDeleteEverything (*this, player);
	}

	sendGameTime (*this, player, gameTimer->gameTime);

	//if (settings->clans == SETTING_CLANS_ON)
	{
		sendClansToClients (*this, playerList);
	}
	sendTurn (*this, turnClock->getTurn (), player);
	sendResources (*this, player);

	// send all units to the client
	const auto& vehicles = player.getVehicles ();
	for (auto i = vehicles.begin (); i != vehicles.end (); ++i)
	{
		const auto& vehicle = *i;
		if (!vehicle->isUnitLoaded ()) resyncVehicle (*vehicle, player);
	}

	const auto& buildings = player.getBuildings ();
	for (auto i = buildings.begin (); i != buildings.end (); ++i)
	{
		const auto& building = *i;
		sendAddUnit (*this, building->getPosition (), building->iID, false, building->data.ID, player, true);
		for (size_t i = 0; i != building->storedUnits.size (); ++i)
		{
			cVehicle& storedVehicle = *building->storedUnits[i];
			resyncVehicle (storedVehicle, player);
			sendStoreVehicle (*this, building->iID, false, storedVehicle.iID, player);
		}
		sendUnitData (*this, *building, player);
		if (building->data.canMineMaxRes > 0) sendProduceValues (*this, *building);
		if (!building->isBuildListEmpty()) sendBuildList (*this, *building);
	}
	// send all subbases
	for (size_t i = 0; i != player.base.SubBases.size (); ++i)
	{
		sendSubbaseValues (*this, *player.base.SubBases[i], player);
	}
	// refresh enemy units
	player.doScan ();
	checkPlayerUnits();
	// send upgrades
	for (size_t i = 0; i != UnitsData.getNrVehicles(); ++i)
	{
		// if only costs were researched, the version is not incremented
		if (player.VehicleData[i].getVersion () > 0
			|| player.VehicleData[i].buildCosts != UnitsData.getVehicle (i, player.getClan ()).buildCosts)
			sendUnitUpgrades (*this, player.VehicleData[i], player);
	}
	for (size_t i = 0; i != UnitsData.getNrBuildings(); ++i)
	{
		// if only costs were researched, the version is not incremented
		if (player.BuildingData[i].getVersion () > 0
			|| player.BuildingData[i].buildCosts != UnitsData.getBuilding (i, player.getClan ()).buildCosts)
			sendUnitUpgrades (*this, player.BuildingData[i], player);
	}
	// send credits
	sendCredits (*this, player.Credits, player);
	// send research
	sendResearchLevel (*this, player.researchLevel, player);
	sendRefreshResearchCount (*this, player);

	// send all players' score histories & eco-counts
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		cPlayer& subj = *playerList[i];
		for (int t = 1; t <= turnClock->getTurn (); ++t)
			sendScore (*this, subj, t, &player);
		sendNumEcos (*this, subj, &player);
	}

	sendGameSettings (*this, player);

	// send attackJobs
	for (size_t i = 0; i != AJobs.size(); ++i)
	{
		cServerAttackJob& ajob = *AJobs[i];
		for (size_t ajobClient = 0; ajobClient != ajob.executingClients.size(); ++ajobClient)
		{
			if (ajob.executingClients[ajobClient] == &player)
			{
				ajob.sendFireCommand (&player);
			}
		}
	}

	// send Hud setting
	sendHudSettings (*this, player);

	Log.write (" Server:  ============================= end resync  ==========================", cLog::eLOG_TYPE_NET_DEBUG);
}

//------------------------------------------------------------------------------
void cServer::resyncVehicle (const cVehicle& Vehicle, const cPlayer& Player)
{
	sendAddUnit (*this, Vehicle.getPosition(), Vehicle.iID, true, Vehicle.data.ID, Player, true, !Vehicle.isUnitLoaded ());
	if (Vehicle.ServerMoveJob) sendMoveJobServer (*this, *Vehicle.ServerMoveJob, Player);
	for (size_t i = 0; i != Vehicle.storedUnits.size(); ++i)
	{
		const cVehicle& storedVehicle = *Vehicle.storedUnits[i];
		resyncVehicle (storedVehicle, Player);
		sendStoreVehicle (*this, Vehicle.iID, true, storedVehicle.iID, Player);
	}
	sendUnitData (*this, Vehicle, Player);
	sendSpecificUnitData (*this, Vehicle);
	if (Vehicle.hasAutoMoveJob) sendSetAutomoving (*this, Vehicle);
	if (Vehicle.detectedByPlayerList.empty() == false) sendDetectionState (*this, Vehicle);
}

//------------------------------------------------------------------------------
bool cServer::addMoveJob (const cPosition& source, const cPosition& destination, cVehicle* vehicle)
{
	cServerMoveJob* MoveJob = new cServerMoveJob (*this, source, destination, vehicle);
	if (!MoveJob->calcPath())
	{
		delete MoveJob;
		vehicle->ServerMoveJob = NULL;
		return false;
	}

	sendMoveJobServer (*this, *MoveJob, *vehicle->owner);
	for (size_t i = 0; i != vehicle->seenByPlayerList.size(); ++i)
	{
		sendMoveJobServer (*this, *MoveJob, *vehicle->seenByPlayerList[i]);
	}

	addActiveMoveJob (*MoveJob);
	return true;
}

//------------------------------------------------------------------------------
void cServer::changeUnitOwner (cVehicle& vehicle, cPlayer& newOwner)
{
	if (vehicle.owner && casualtiesTracker != NULL)
		casualtiesTracker->logCasualty (vehicle.data.ID, vehicle.owner->getNr());

	// delete vehicle in the list of the old player
	cPlayer* oldOwner = vehicle.owner;

	auto owningVehiclePtr = oldOwner->removeUnit (vehicle);
	// add the vehicle to the list of the new player
	vehicle.owner = &newOwner;
	newOwner.addUnit (owningVehiclePtr);

	//the vehicle is fully operational for the new owner
	if (vehicle.isDisabled())
	{
		vehicle.data.speedCur = vehicle.lastSpeed;
		vehicle.data.setShots(vehicle.lastShots);
	}
	vehicle.setDisabledTurns (0);

	// delete the unit on the clients and add it with new owner again
	sendDeleteUnit (*this, vehicle, oldOwner);
	for (size_t i = 0; i != vehicle.seenByPlayerList.size(); ++i)
	{
		sendDeleteUnit (*this, vehicle, vehicle.seenByPlayerList[i]);
	}
	vehicle.seenByPlayerList.clear();
	vehicle.detectedByPlayerList.clear();
	sendAddUnit (*this, vehicle.getPosition(), vehicle.iID, true, vehicle.data.ID, *vehicle.owner, false);
	sendUnitData (*this, vehicle, *vehicle.owner);
	sendSpecificUnitData (*this, vehicle);

	oldOwner->doScan();
	newOwner.doScan();
	checkPlayerUnits();

	// let the unit work for his new owner
	if (vehicle.data.canSurvey)
	{
		sendVehicleResources (*this, vehicle);
		vehicle.doSurvey (*this);
	}
	vehicle.makeDetection (*this);
}

//------------------------------------------------------------------------------
void cServer::stopVehicleBuilding (cVehicle& vehicle)
{
	if (!vehicle.isUnitBuildingABuilding ()) return;

	vehicle.setBuildingABuilding(false);
	vehicle.BuildPath = false;

	auto position = vehicle.getPosition ();

	if (vehicle.getBuildingType ().getUnitDataOriginalVersion ()->isBig)
	{
		Map->moveVehicle (vehicle, vehicle.buildBigSavedPosition);
		position = vehicle.buildBigSavedPosition;
		vehicle.owner->doScan();
	}
	sendStopBuild (*this, vehicle.iID, position, *vehicle.owner);
	for (size_t i = 0; i != vehicle.seenByPlayerList.size(); ++i)
	{
		sendStopBuild (*this, vehicle.iID, position, *vehicle.seenByPlayerList[i]);
	}
}

void cServer::sideStepStealthUnit (const cPosition& position, const cVehicle& vehicle, const cPosition& bigOffset)
{
	sideStepStealthUnit(position, vehicle.data, vehicle.owner, bigOffset);
}

void cServer::sideStepStealthUnit(const cPosition& position, const sUnitData& vehicleData, cPlayer* vehicleOwner, const cPosition& bigOffset)
{
	// TODO: make sure, the stealth vehicle takes the direct diagonal move.
	// Also when two straight moves would be shorter.

	if (vehicleData.factorAir > 0) return;

	// first look for an undetected stealth unit
	cVehicle* stealthVehicle = Map->getField(position).getVehicle();
	if (!stealthVehicle) return;
	if (stealthVehicle->owner == vehicleOwner) return;
	if (stealthVehicle->data.isStealthOn == TERRAIN_NONE) return;
	if (stealthVehicle->isDetectedByPlayer (vehicleOwner)) return;

	// make sure a running movement is finished,
	// before starting the side step move
	if (stealthVehicle->isUnitMoving ()) stealthVehicle->ServerMoveJob->doEndMoveVehicle ();

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
			const cPosition currentPosition(x, y);
			if(currentPosition == position) continue;

			// when a bigOffet was passed,
			// for example a constructor needs space for a big building
			// so not all directions are allowed for the side stepping
			if (bigOffset != -1)
			{
				if (currentPosition == bigOffset ||
					currentPosition == bigOffset + cPosition(1, 0) ||
					currentPosition == bigOffset + cPosition(0, 1) ||
					currentPosition == bigOffset + cPosition(1, 1)) continue;
			}

			// check whether this field is a possible destination
			if (!Map->possiblePlace (*stealthVehicle, currentPosition)) continue;

			// check costs of the move
			cPathCalculator pathCalculator (cPosition(0, 0), cPosition(0, 0), *Map, *stealthVehicle);
			int costs = pathCalculator.calcNextCost (position, currentPosition);
			if (costs > stealthVehicle->data.speedCur) continue;

			// check whether the vehicle would be detected
			// on the destination field
			bool detectOnDest = false;
			if (stealthVehicle->data.isStealthOn & TERRAIN_GROUND)
			{
				for (size_t i = 0; i != playerList.size (); ++i)
				{
					if (playerList[i].get() == stealthVehicle->owner) continue;
					if (playerList[i]->hasLandDetection (currentPosition)) detectOnDest = true;
				}
				if (Map->isWater (currentPosition)) detectOnDest = true;
			}
			if (stealthVehicle->data.isStealthOn & TERRAIN_SEA)
			{
				for (size_t i = 0; i != playerList.size (); ++i)
				{
					if (playerList[i].get() == stealthVehicle->owner) continue;
					if (playerList[i]->hasSeaDetection (currentPosition)) detectOnDest = true;
				}
				if (!Map->isWater (currentPosition)) detectOnDest = true;

				if (stealthVehicle->data.factorGround > 0 && stealthVehicle->data.factorSea > 0)
				{
					cBuilding* b = Map->getField(currentPosition).getBaseBuilding();
					if (b && (b->data.surfacePosition == sUnitData::SURFACE_POS_BASE || b->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA || b->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)) detectOnDest = true;
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
		addMoveJob (position, bestPosition, stealthVehicle);
		// begin the movement immediately,
		// so no other unit can block the destination field
		stealthVehicle->ServerMoveJob->checkMove();
		return;
	}

	// sidestepping failed. Uncover the vehicle.
	stealthVehicle->setDetectedByPlayer (*this, vehicleOwner);
	checkPlayerUnits();
}

void cServer::makeAdditionalSaveRequest (int saveNum)
{
	savingID++;
	savingIndex = saveNum;
	sendRequestSaveInfo (*this, savingID);
}

void cServer::addJob (cJob* job)
{
	helperJobs.addJob (*job);
}

void cServer::runJobs()
{
	helperJobs.run (*gameTimer);
}

void cServer::enableFreezeMode (eFreezeMode mode, int playerNumber)
{
	freezeModes.enable (mode, playerNumber);
	switch (mode)
	{
		case FREEZE_PAUSE: gameTimer->stop(); break;
		case FREEZE_WAIT_FOR_RECONNECT: gameTimer->stop(); break;
		case FREEZE_WAIT_FOR_TURNEND: break;
		case FREEZE_WAIT_FOR_PLAYER:
			//gameTimer->stop(); //done in cGameTimer::nextTickAllowed();
			break;
		default:
			Log.write (" Server: Tried to enable unsupported freeze mode: " + iToStr (mode), cLog::eLOG_TYPE_NET_ERROR);
			break;
	}

	sendFreeze (*this, mode, freezeModes.getPlayerNumber());
}

void cServer::disableFreezeMode (eFreezeMode mode)
{
	freezeModes.disable (mode);
	switch (mode)
	{
		case FREEZE_PAUSE:
		case FREEZE_WAIT_FOR_RECONNECT:
		case FREEZE_WAIT_FOR_TURNEND:
		case FREEZE_WAIT_FOR_PLAYER:
			sendUnfreeze (*this, mode);
			break;
		default:
			Log.write (" Server: Tried to disable unsupported freeze mode: " + iToStr (mode), cLog::eLOG_TYPE_NET_ERROR);
			break;
	}

	if (!freezeModes.pause && !freezeModes.waitForReconnect)
	{
		gameTimer->start();
	}
}
