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

#ifdef _MSC_VER

# define NOMINMAX // do not use min, max as macro
# include <windows.h>
const DWORD MS_VC_EXCEPTION = 0x406D1388;

# pragma pack (push, 8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
# pragma pack (pop)
#endif


#include "server.h"

#include "attackJobs.h"
#include "buildings.h"
#include "casualtiestracker.h"
#include "client.h"
#include "clientevents.h"
#include "clist.h"
#include "events.h"
#include "gametimer.h"
#include "hud.h"
#include "jobs.h"
#include "log.h"
#include "menus.h"
#include "movejobs.h"
#include "netmessage.h"
#include "network.h"
#include "player.h"
#include "ringbuffer.h"
#include "savegame.h"
#include "serverevents.h"
#include "settings.h"
#include "upgradecalculator.h"
#include "vehicles.h"

#if DEDICATED_SERVER_APPLICATION
# include "dedicatedserver.h"
#endif

//------------------------------------------------------------------------------
int CallbackRunServerThread (void* arg)
{
#if defined _MSC_VER && defined DEBUG //set a readable thread name for debugging
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = "Server Thread";
	info.dwThreadID = -1;
	info.dwFlags = 0;

	__try
	{
		RaiseException (MS_VC_EXCEPTION, 0, sizeof (info) / sizeof (ULONG_PTR), reinterpret_cast<ULONG_PTR*> (&info));
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif

	cServer* server = reinterpret_cast<cServer*> (arg);
	server->run();
	return 0;
}

//------------------------------------------------------------------------------
cServer::cServer (cTCP* network_)
	: network (network_)
	, localClient (NULL)
	, gameTimer()
	, lastTurnEnd (0)
	, executingRemainingMovements (false)
	, casualtiesTracker (0)
{
	this->turnLimit = 0;
	this->scoreLimit = 0;
	this->bPlayTurns = false;
	Map = NULL;
	this->PlayerList = NULL;
	bExit = false;
	openMapDefeat = true;
	bStarted = false;
	neutralBuildings = NULL;
	iActiveTurnPlayerNr = 0;
	iTurn = 1;
	iDeadlineStartTime = 0;
	iTurnDeadline = 90; // just temporary set to 90 seconds
	iNextUnitID = 1;
	iWantPlayerEndNum = -1;
	savingID = 0;
	savingIndex = -1;

	casualtiesTracker = new cCasualtiesTracker();

	if (!DEDICATED_SERVER)
	{
		if (network) network->setMessageReceiver (this);
		serverThread = SDL_CreateThread (CallbackRunServerThread, this);
	}

	gameTimer.maxEventQueueSize = MAX_SERVER_EVENT_COUNTER;
	gameTimer.start();
}
//------------------------------------------------------------------------------
cServer::~cServer()
{
	stop();

	delete casualtiesTracker;
	casualtiesTracker = 0;

	// disconect clients
	if (network)
	{
		for (size_t i = 0; i < PlayerList->size(); ++i)
		{
			network->close ( (*PlayerList) [i]->getSocketNum());
		}
	}

	while (eventQueue.size())
	{
		delete eventQueue.read();
	}

	for (size_t i = 0; i != AJobs.size(); ++i)
	{
		delete AJobs[i];
	}

	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		delete (*PlayerList) [i];
	}
	PlayerList->clear();
	while (neutralBuildings)
	{
		cBuilding* nextBuilding = neutralBuildings->next;
		delete neutralBuildings;
		neutralBuildings = nextBuilding;
	}
}

//------------------------------------------------------------------------------
void cServer::setMap (cMap& map_)
{
	Map = &map_;
}

//------------------------------------------------------------------------------
void cServer::setPlayers (std::vector<cPlayer*>* playerList_)
{
	PlayerList = playerList_;
}

//------------------------------------------------------------------------------
void cServer::setGameSettings (const sSettings& gameSettings)
{
	this->bPlayTurns = gameSettings.gameType == SETTINGS_GAMETYPE_TURNS;
	this->turnLimit = 0;
	this->scoreLimit = 0;
	switch (gameSettings.victoryType)
	{
		case SETTINGS_VICTORY_TURNS: turnLimit = gameSettings.duration; break;
		case SETTINGS_VICTORY_POINTS: scoreLimit = gameSettings.duration; break;
		case SETTINGS_VICTORY_ANNIHILATION: break;
		default: assert (0);
	}

	assert (turnLimit == 0 || scoreLimit == 0);
}

//------------------------------------------------------------------------------
eGameTypes cServer::getGameType() const
{
	if (network) return GAME_TYPE_TCPIP;
	if (PlayerList->size() > 1 && bPlayTurns) return GAME_TYPE_HOTSEAT;
	return GAME_TYPE_SINGLE;
}

//------------------------------------------------------------------------------
void cServer::stop()
{
	bExit = true;
	gameTimer.stop();

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
void cServer::setDeadline (int iDeadline)
{
	iTurnDeadline = iDeadline;
}

//------------------------------------------------------------------------------
cNetMessage* cServer::pollEvent()
{
	if (eventQueue.size() <= 0)
	{
		return NULL;
	}
	return eventQueue.read();
}

//------------------------------------------------------------------------------
void cServer::pushEvent (cNetMessage* message)
{
	eventQueue.write (message);
}

//------------------------------------------------------------------------------
void cServer::sendNetMessage (cNetMessage* message, int iPlayerNum)
{
	message->iPlayerNr = iPlayerNum;
	message->serialize();
	if (message->iType != NET_GAME_TIME_SERVER)
		Log.write ("Server: <-- " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);

	if (iPlayerNum == -1)
	{
		if (network)
			network->send (message->iLength, message->data);
		if (localClient != NULL)
			localClient->pushEvent (message);
		return;
	}

	cPlayer* Player = getPlayerFromNumber (iPlayerNum);

	if (Player == NULL)
	{
		// player not found
		Log.write ("Server: Can't send message. Player " + iToStr (iPlayerNum) + " not found.", cLog::eLOG_TYPE_NET_WARNING);
		delete message;
		return;
	}
	if (Player->isLocal())
	{
		if (localClient != NULL)
			localClient->pushEvent (message);
	}
	// on all other sockets the netMessage will be send over TCP/IP
	else
	{
		if (network) network->sendTo (Player->getSocketNum(), message->iLength, message->serialize());
		delete message;
	}
}

//------------------------------------------------------------------------------
void cServer::run()
{
	while (!bExit)
	{
		AutoPtr<cNetMessage> event (pollEvent());

		if (event)
		{
			handleNetMessage (event);
			checkPlayerUnits();
		}

		// don't do anything if games hasn't been started yet!
		unsigned int lastTime = gameTimer.gameTime;
		if (bStarted)
		{
			gameTimer.run (*this);
		}

		// nothing done
		if (!event && lastTime == gameTimer.gameTime)
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
		const cPlayer* player = getPlayerFromNumber (message.iPlayerNr);
		if (player != NULL) iSocketNumber = player->getSocketNum();
	}
	if (iSocketNumber == -1)
		return;
	network->close (iSocketNumber);

	cPlayer* Player = NULL;
	// resort socket numbers of the players
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		if ( (*PlayerList) [i]->getSocketNum() == iSocketNumber)
		{
			Player = (*PlayerList) [i];
			break;
		}
	}
	for (size_t i = 0; i != PlayerList->size(); ++i)
		(*PlayerList) [i]->onSocketIndexDisconnected (iSocketNumber);

	if (Player)
	{
		// freeze clients
		// the dedicated server doesn't force to wait for reconnect,
		// because it's expected client behaviour
		if (DEDICATED_SERVER == false)
			enableFreezeMode (FREEZE_WAIT_FOR_RECONNECT);
		sendChatMessageToClient (*this, "Text~Multiplayer~Lost_Connection", SERVER_INFO_MESSAGE, -1, Player->getName());

		DisconnectedPlayerList.push_back (Player);

		Player->revealMap();
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_CHAT_CLIENT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_CHAT_CLIENT);

	sendChatMessageToClient (*this, message.popString(), USER_MESSAGE);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_TO_END_TURN (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_TO_END_TURN);

	if (bPlayTurns)
	{
		const cPlayer* Player = getPlayerFromNumber (message.iPlayerNr);
		if ( (*PlayerList) [iActiveTurnPlayerNr] != Player) return;
	}
	handleEnd (message.iPlayerNr);
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

	cServerMoveJob* MoveJob = cServerMoveJob::generateFromMessage (*this, &message);
	if (!MoveJob)
	{
		return;
	}

	addActiveMoveJob (MoveJob);
	Log.write (" Server: Added received movejob", cLog::eLOG_TYPE_NET_DEBUG);
	// send the movejob to all players who can see this unit
	const cVehicle& vehicle = *MoveJob->Vehicle;
	sendMoveJobServer (*this, *MoveJob, vehicle.owner->getNr());
	for (size_t i = 0; i != vehicle.seenByPlayerList.size(); ++i)
	{
		sendMoveJobServer (*this, *MoveJob, vehicle.seenByPlayerList[i]->getNr());
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
		cPlayer* player = getPlayerFromNumber (message.iPlayerNr);
		if (!player) return;

		for (cVehicle* vehicle = player->VehicleList; vehicle; vehicle = vehicle->next)
		{
			if (vehicle->ServerMoveJob && !vehicle->moving)
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
	// identify agressor
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
		if (attackingUnit->isBeeingAttacked) return;
	}
	else
	{
		const int offset = message.popInt32();
		if (Map->isValidOffset (offset) == false)
		{
			Log.write (" Server: Invalid agressor offset", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		attackingUnit = Map->fields[offset].getTopBuilding();
		if (attackingUnit == NULL)
		{
			Log.write (" Server: No Building at agressor offset", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		if (attackingUnit->owner->getNr() != message.iPlayerNr)
		{
			Log.write (" Server: Message was not send by building owner!", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		if (attackingUnit->isBeeingAttacked) return;
	}

	// find target offset
	int targetOffset = message.popInt32();
	if (Map->isValidOffset (targetOffset) == false)
	{
		Log.write (" Server: Invalid target offset!", cLog::eLOG_TYPE_NET_WARNING);
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
		const int oldOffset = targetOffset;
		// the target offset doesn't need to match the vehicle position,
		// when it is big
		if (!targetVehicle->data.isBig)
		{
			targetOffset = Map->getOffset (targetVehicle->PosX, targetVehicle->PosY);
		}
		Log.write (" Server: attacking vehicle " + targetVehicle->getDisplayName() + ", " + iToStr (targetVehicle->iID), cLog::eLOG_TYPE_NET_DEBUG);
		if (oldOffset != targetOffset) Log.write (" Server: target offset changed from " + iToStr (oldOffset) + " to " + iToStr (targetOffset), cLog::eLOG_TYPE_NET_DEBUG);
	}

	// check if attack is possible
	if (attackingUnit->canAttackObjectAt (targetOffset % Map->getSize(), targetOffset / Map->getSize(), Map, true) == false)
	{
		Log.write (" Server: The server decided, that the attack is not possible", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}
	AJobs.push_back (new cServerAttackJob (*this, attackingUnit, targetOffset, false));
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

	Vehicle->ClearMines = message.popBool();
	Vehicle->LayMines = message.popBool();

	if (Vehicle->ClearMines && Vehicle->LayMines)
	{
		Vehicle->ClearMines = false;
		Vehicle->LayMines = false;
		return;
	}

	bool result = false;
	if (Vehicle->ClearMines) result = Vehicle->clearMine (*this);
	if (Vehicle->LayMines) result = Vehicle->layMine (*this);

	if (result)
	{
		sendUnitData (*this, *Vehicle, Vehicle->owner->getNr());
		for (size_t i = 0; i != Vehicle->seenByPlayerList.size(); ++i)
		{
			sendUnitData (*this, *Vehicle, Vehicle->seenByPlayerList[i]->getNr());
		}
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_BUILD (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_BUILD);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle == NULL) return;
	if (Vehicle->IsBuilding || Vehicle->BuildPath) return;

	const sID BuildingTyp = message.popID();
	if (BuildingTyp.getUnitDataOriginalVersion() == NULL)
	{
		Log.write (" Server: invalid unit: " + iToStr (BuildingTyp.iFirstPart) + "." + iToStr (BuildingTyp.iSecondPart), cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	const sUnitData& Data = *BuildingTyp.getUnitDataOriginalVersion();
	const int iBuildSpeed = message.popInt16();
	if (iBuildSpeed > 2 || iBuildSpeed < 0) return;
	const int iBuildOff = message.popInt32();

	int iTurboBuildRounds[3];
	int iTurboBuildCosts[3];
	Vehicle->calcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, Vehicle->owner->getUnitDataCurrentVersion (BuildingTyp)->buildCosts);

	if (iTurboBuildCosts[iBuildSpeed] > Vehicle->data.storageResCur ||
		iTurboBuildRounds[iBuildSpeed] <= 0)
	{
		// TODO: differ between different aborting types
		// (buildposition blocked, not enough material, ...)
		sendBuildAnswer (*this, false, *Vehicle);
		return;
	}

	if (Map->isValidOffset (iBuildOff) == false) return;
	const int buildX = iBuildOff % Map->getSize();
	const int buildY = iBuildOff / Map->getSize();
	const int oldPosX = Vehicle->PosX;
	const int oldPosY = Vehicle->PosY;

	if (Vehicle->data.canBuild.compare (Data.buildAs) != 0) return;

	if (Data.isBig)
	{
		sideStepStealthUnit (buildX    , buildY    , *Vehicle, iBuildOff);
		sideStepStealthUnit (buildX + 1, buildY    , *Vehicle, iBuildOff);
		sideStepStealthUnit (buildX    , buildY + 1, *Vehicle, iBuildOff);
		sideStepStealthUnit (buildX + 1, buildY + 1, *Vehicle, iBuildOff);

		if (! (Map->possiblePlaceBuilding (Data, buildX,     buildY    , Vehicle) &&
			   Map->possiblePlaceBuilding (Data, buildX + 1, buildY    , Vehicle) &&
			   Map->possiblePlaceBuilding (Data, buildX,     buildY + 1, Vehicle) &&
			   Map->possiblePlaceBuilding (Data, buildX + 1, buildY + 1, Vehicle)))
		{
			sendBuildAnswer (*this, false, *Vehicle);
			return;
		}
		Vehicle->BuildBigSavedPos = Map->getOffset (Vehicle->PosX, Vehicle->PosY);

		// set vehicle to build position
		Map->moveVehicleBig (*Vehicle, buildX, buildY);
		Vehicle->owner->doScan();
	}
	else
	{
		if (iBuildOff != Map->getOffset (Vehicle->PosX, Vehicle->PosY)) return;

		if (!Map->possiblePlaceBuilding (Data, iBuildOff, Vehicle))
		{
			sendBuildAnswer (*this, false, *Vehicle);
			return;
		}
	}

	Vehicle->BuildingTyp = BuildingTyp;
	const bool bBuildPath = message.popBool();
	const int iPathOff = message.popInt32();
	if (Map->isValidOffset (iPathOff) == false) return;
	Vehicle->BandX = iPathOff % Map->getSize();
	Vehicle->BandY = iPathOff / Map->getSize();

	Vehicle->BuildCosts = iTurboBuildCosts[iBuildSpeed];
	Vehicle->BuildRounds = iTurboBuildRounds[iBuildSpeed];
	Vehicle->BuildCostsStart = Vehicle->BuildCosts;
	Vehicle->BuildRoundsStart = Vehicle->BuildRounds;

	Vehicle->IsBuilding = true;
	Vehicle->BuildPath = bBuildPath;

	sendBuildAnswer (*this, true, *Vehicle);
	addJob (new cStartBuildJob (*Vehicle, oldPosX, oldPosY, Data.isBig));

	if (Vehicle->ServerMoveJob) Vehicle->ServerMoveJob->release();
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_END_BUILDING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_END_BUILDING);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle == NULL) return;

	const int iEscapeX = message.popInt16();
	const int iEscapeY = message.popInt16();

	if (!Vehicle->IsBuilding || Vehicle->BuildRounds > 0) return;
	if (!Map->possiblePlace (*Vehicle, iEscapeX, iEscapeY))
	{
		sideStepStealthUnit (iEscapeX, iEscapeY, *Vehicle);
	}

	if (!Map->possiblePlace (*Vehicle, iEscapeX, iEscapeY)) return;

	addBuilding (Vehicle->PosX, Vehicle->PosY, Vehicle->BuildingTyp, Vehicle->owner);

	// end building
	Vehicle->IsBuilding = false;
	Vehicle->BuildPath = false;

	// set the vehicle to the border
	if (Vehicle->BuildingTyp.getUnitDataOriginalVersion()->isBig)
	{
		int x = Vehicle->PosX;
		int y = Vehicle->PosY;
		if (iEscapeX > Vehicle->PosX) x++;
		if (iEscapeY > Vehicle->PosY) y++;
		Map->moveVehicle (*Vehicle, x, y);

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
	addMoveJob (Vehicle->PosX, Vehicle->PosY, iEscapeX, iEscapeY, Vehicle);
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
	if (!Vehicle->IsBuilding) return;
	stopVehicleBuilding (Vehicle);
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

	if ( (!SrcBuilding && !SrcVehicle) || (!DestBuilding && !DestVehicle)) return;

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
			if (DestBuilding->data.storageResCur + iTranfer > DestBuilding->data.storageResMax || DestBuilding->data.storageResCur + iTranfer < 0) return;
			if (SrcBuilding->data.storageResCur - iTranfer > SrcBuilding->data.storageResMax || SrcBuilding->data.storageResCur - iTranfer < 0) return;

			DestBuilding->data.storageResCur += iTranfer;
			SrcBuilding->data.storageResCur -= iTranfer;
			sendUnitData (*this, *DestBuilding, DestBuilding->owner->getNr());
			sendUnitData (*this, *SrcBuilding, SrcBuilding->owner->getNr());
		}
		else
		{
			if (DestVehicle->IsBuilding || DestVehicle->IsClearing) return;
			if (DestVehicle->data.storeResType != iType) return;
			if (DestVehicle->data.storageResCur + iTranfer > DestVehicle->data.storageResMax || DestVehicle->data.storageResCur + iTranfer < 0) return;
			switch (iType)
			{
				case sUnitData::STORE_RES_METAL:
				{
					if (SrcBuilding->SubBase->Metal - iTranfer > SrcBuilding->SubBase->MaxMetal || SrcBuilding->SubBase->Metal - iTranfer < 0) bBreakSwitch = true;
					if (!bBreakSwitch) SrcBuilding->SubBase->addMetal (*this, -iTranfer);
				}
				break;
				case sUnitData::STORE_RES_OIL:
				{
					if (SrcBuilding->SubBase->Oil - iTranfer > SrcBuilding->SubBase->MaxOil || SrcBuilding->SubBase->Oil - iTranfer < 0) bBreakSwitch = true;
					if (!bBreakSwitch) SrcBuilding->SubBase->addOil (*this, -iTranfer);
				}
				break;
				case sUnitData::STORE_RES_GOLD:
				{
					if (SrcBuilding->SubBase->Gold - iTranfer > SrcBuilding->SubBase->MaxGold || SrcBuilding->SubBase->Gold - iTranfer < 0) bBreakSwitch = true;
					if (!bBreakSwitch) SrcBuilding->SubBase->addGold (*this, -iTranfer);
				}
				break;
			}
			if (bBreakSwitch) return;
			sendSubbaseValues (*this, *SrcBuilding->SubBase, SrcBuilding->owner->getNr());
			DestVehicle->data.storageResCur += iTranfer;
			sendUnitData (*this, *DestVehicle, DestVehicle->owner->getNr());
		}
	}
	else
	{
		if (SrcVehicle->data.storeResType != iType) return;
		if (SrcVehicle->data.storageResCur - iTranfer > SrcVehicle->data.storageResMax || SrcVehicle->data.storageResCur - iTranfer < 0) return;
		if (DestBuilding)
		{
			bool bBreakSwitch = false;
			switch (iType)
			{
				case sUnitData::STORE_RES_METAL:
				{
					if (DestBuilding->SubBase->Metal + iTranfer > DestBuilding->SubBase->MaxMetal || DestBuilding->SubBase->Metal + iTranfer < 0) bBreakSwitch = true;
					if (!bBreakSwitch) DestBuilding->SubBase->addMetal (*this, iTranfer);
				}
				break;
				case sUnitData::STORE_RES_OIL:
				{
					if (DestBuilding->SubBase->Oil + iTranfer > DestBuilding->SubBase->MaxOil || DestBuilding->SubBase->Oil + iTranfer < 0) bBreakSwitch = true;
					if (!bBreakSwitch) DestBuilding->SubBase->addOil (*this, iTranfer);
				}
				break;
				case sUnitData::STORE_RES_GOLD:
				{
					if (DestBuilding->SubBase->Gold + iTranfer > DestBuilding->SubBase->MaxGold || DestBuilding->SubBase->Gold + iTranfer < 0) bBreakSwitch = true;
					if (!bBreakSwitch) DestBuilding->SubBase->addGold (*this, iTranfer);
				}
				break;
			}
			if (bBreakSwitch) return;
			sendSubbaseValues (*this, *DestBuilding->SubBase, DestBuilding->owner->getNr());
		}
		else
		{
			if (DestVehicle->IsBuilding || DestVehicle->IsClearing) return;
			if (DestVehicle->data.storeResType != iType) return;
			if (DestVehicle->data.storageResCur + iTranfer > DestVehicle->data.storageResMax || DestVehicle->data.storageResCur + iTranfer < 0) return;
			DestVehicle->data.storageResCur += iTranfer;
			sendUnitData (*this, *DestVehicle, DestVehicle->owner->getNr());
		}
		SrcVehicle->data.storageResCur -= iTranfer;
		sendUnitData (*this, *SrcVehicle, SrcVehicle->owner->getNr());
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_BUILDLIST (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_BUILDLIST);

	cBuilding* Building = getBuildingFromID (message.popInt16());
	if (Building == NULL) return;

	// check whether the building has water and land fields around it
	int iX = Building->PosX - 2;
	int iY = Building->PosY - 1;
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
		if (Map->isValidPos (iX, iY) == false) continue;

		int iOff = Map->getOffset (iX, iY);

		std::vector<cBuilding*>& buildings = Map->fields[iOff].getBuildings();
		std::vector<cBuilding*>::iterator b_it = buildings.begin();
		std::vector<cBuilding*>::iterator b_end = buildings.end();
		while (b_it != b_end && ( (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE || (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)) ++b_it;

		if (!Map->isWaterOrCoast (iX, iY) || (b_it != b_end && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_BASE)) bLand = true;
		else if (Map->isWaterOrCoast (iX, iY) && b_it != b_end && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)
		{
			bLand = true;
			bWater = true;
			break;
		}
		else if (Map->isWaterOrCoast (iX, iY)) bWater = true;
	}

	// reset building status
	if (Building->IsWorking)
	{
		Building->ServerStopWork (*this, false);
	}

	const int iBuildSpeed = message.popInt16();
	if (iBuildSpeed == 0) Building->MetalPerRound =  1 * Building->data.needsMetal;
	if (iBuildSpeed == 1) Building->MetalPerRound =  4 * Building->data.needsMetal;
	if (iBuildSpeed == 2) Building->MetalPerRound = 12 * Building->data.needsMetal;

	std::vector<sBuildList*>* NewBuildList = new std::vector<sBuildList*>;

	const int iCount = message.popInt16();
	for (int i = 0; i < iCount; i++)
	{
		const sID Type = message.popID();

		// if the first unit hasn't changed copy it to the new buildlist
		if (Building->BuildList->size() > 0 && i == 0 && Type == (*Building->BuildList) [0]->type)
		{
			// recalculate costs, because build speed could have beed changed
			int iTurboBuildRounds[3];
			int iTurboBuildCosts[3];
			Building->CalcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, Building->owner->getUnitDataCurrentVersion (Type)->buildCosts, (*Building->BuildList) [0]->metall_remaining);
			sBuildList* BuildListItem = new sBuildList;
			BuildListItem->metall_remaining = iTurboBuildCosts[iBuildSpeed];
			BuildListItem->type = Type;
			NewBuildList->push_back (BuildListItem);
			continue;
		}

		// check whether this building can build this unit
		if (Type.getUnitDataOriginalVersion()->factorSea > 0 && Type.getUnitDataOriginalVersion()->factorGround == 0 && !bWater)
			continue;
		else if (Type.getUnitDataOriginalVersion()->factorGround > 0 && Type.getUnitDataOriginalVersion()->factorSea == 0 && !bLand)
			continue;

		if (Building->data.canBuild.compare (Type.getUnitDataOriginalVersion()->buildAs) != 0)
			continue;

		sBuildList* BuildListItem = new sBuildList;
		BuildListItem->metall_remaining = -1;
		BuildListItem->type = Type;

		NewBuildList->push_back (BuildListItem);
	}

	// delete all buildings from the list
	for (size_t i = 0; i != Building->BuildList->size(); ++i)
	{
		delete (*Building->BuildList) [i];
	}
	delete Building->BuildList;
	Building->BuildList = NewBuildList;

	if (Building->BuildList->empty() == false)
	{
		if ( (*Building->BuildList) [0]->metall_remaining == -1)
		{
			int iTurboBuildRounds[3];
			int iTurboBuildCosts[3];
			Building->CalcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, Building->owner->getUnitDataCurrentVersion ( (*Building->BuildList) [0]->type)->buildCosts);
			(*Building->BuildList) [0]->metall_remaining = iTurboBuildCosts[iBuildSpeed];
		}

		Building->RepeatBuild = message.popBool();
		Building->BuildSpeed = iBuildSpeed;
		if ( (*Building->BuildList) [0]->metall_remaining > 0)
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

	const int iX = message.popInt16();
	const int iY = message.popInt16();
	if (Map->isValidPos (iX, iY) == false) return;
	if (Building->BuildList->empty()) return;

	sBuildList* BuildingListItem = (*Building->BuildList) [0];
	if (BuildingListItem->metall_remaining > 0) return;

	if (!Building->isNextTo (iX, iY)) return;

	const sUnitData& unitData = *BuildingListItem->type.getUnitDataOriginalVersion();
	if (!Map->possiblePlaceVehicle (unitData, iX, iY, Building->owner))
	{
		sideStepStealthUnit (iX, iY, unitData, Building->owner);
	}
	if (!Map->possiblePlaceVehicle (unitData, iX, iY, Building->owner)) return;

	addVehicle (iX, iY, BuildingListItem->type, Building->owner, false);

	// start new buildjob
	Building->BuildList->erase (Building->BuildList->begin());
	if (Building->RepeatBuild)
	{
		BuildingListItem->metall_remaining = -1;
		Building->BuildList->push_back (BuildingListItem);
	}
	else
	{
		delete BuildingListItem;
	}

	if (Building->BuildList->empty() == false)
	{
		BuildingListItem = (*Building->BuildList) [0];
		if (BuildingListItem->metall_remaining == -1)
		{
			int iTurboBuildCosts[3];
			int iTurboBuildRounds[3];
			Building->CalcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, Building->owner->getUnitDataCurrentVersion (BuildingListItem->type)->buildCosts);
			BuildingListItem->metall_remaining = iTurboBuildCosts[Building->BuildSpeed];
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

	sendSubbaseValues (*this, subBase, Building->owner->getNr());
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
		Vehicle->manualFireActive = !Vehicle->manualFireActive;
		if (Vehicle->manualFireActive && Vehicle->sentryActive)
		{
			Vehicle->owner->deleteSentry (Vehicle);
		}

		sendUnitData (*this, *Vehicle, Vehicle->owner->getNr());
		for (size_t i = 0; i != Vehicle->seenByPlayerList.size(); ++i)
			sendUnitData (*this, *Vehicle, Vehicle->seenByPlayerList[i]->getNr());
	}
	else // building
	{
		cBuilding* Building = getBuildingFromID (message.popInt16());
		if (Building == 0)
			return;
		Building->manualFireActive = !Building->manualFireActive;
		if (Building->manualFireActive && Building->sentryActive)
		{
			Building->owner->deleteSentry (Building);
		}

		sendUnitData (*this, *Building, Building->owner->getNr());
		for (size_t i = 0; i != Building->seenByPlayerList.size(); ++i)
			sendUnitData (*this, *Building, Building->seenByPlayerList[i]->getNr());
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

		if (vehicle->sentryActive)
		{
			vehicle->owner->deleteSentry (vehicle);
		}
		else
		{
			vehicle->owner->addSentry (vehicle);
			vehicle->manualFireActive = false;
		}

		sendUnitData (*this, *vehicle, vehicle->owner->getNr());
		for (size_t i = 0; i != vehicle->seenByPlayerList.size(); ++i)
		{
			sendUnitData (*this, *vehicle, vehicle->seenByPlayerList[i]->getNr());
		}
	}
	else // building
	{
		cBuilding* building = getBuildingFromID (message.popInt16());
		if (building == NULL) return;

		if (building->sentryActive)
		{
			building->owner->deleteSentry (building);
		}
		else
		{
			building->owner->addSentry (building);
			building->manualFireActive = false;
		}

		sendUnitData (*this, *building, building->owner->getNr());
		for (size_t i = 0; i != building->seenByPlayerList.size(); ++i)
		{
			sendUnitData (*this, *building, building->seenByPlayerList[i]->getNr());
		}
	}
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_MARK_LOG (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_MARK_LOG);

	cNetMessage* message2 = new cNetMessage (GAME_EV_MARK_LOG);
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

	if ( (!SrcVehicle && !SrcBuilding) || (!DestVehicle && !DestBuilding)) return;

	// check whether the supply is ok and reduce cargo of sourceunit
	int iValue;
	if (SrcVehicle)
	{
		if (DestVehicle && !SrcVehicle->canSupply (DestVehicle, iType)) return;
		if (DestBuilding && !SrcVehicle->canSupply (DestBuilding, iType)) return;

		// do the supply
		if (iType == SUPPLY_TYPE_REARM)
		{
			SrcVehicle->data.storageResCur--;
			iValue = DestVehicle ? DestVehicle->data.ammoMax : DestBuilding->data.ammoMax;
		}
		else
		{
			sUnitData* DestData = DestVehicle ? &DestVehicle->data : &DestBuilding->data;
			// reduce cargo for repair and calculate maximal repair value
			iValue = DestData->hitpointsCur;
			while (SrcVehicle->data.storageResCur > 0 && iValue < DestData->hitpointsMax)
			{
				iValue += Round ( ( (float) DestData->hitpointsMax / DestData->buildCosts) * 4);
				SrcVehicle->data.storageResCur--;
			}
			iValue = std::min (DestData->hitpointsMax, iValue);
		}
		// the changed values aren't interesting for enemy players,
		// so only send the new data to the owner
		sendUnitData (*this, *SrcVehicle, SrcVehicle->owner->getNr());
	}
	else
	{
		// buildings can only supply vehicles
		if (!DestVehicle) return;

		// do the supply
		if (iType == SUPPLY_TYPE_REARM)
		{
			if (SrcBuilding->SubBase->Metal < 1) return;
			SrcBuilding->SubBase->addMetal (*this, -1);
			iValue = DestVehicle->data.ammoMax;
		}
		else
		{
			// reduce cargo for repair and calculate maximal repair value
			iValue = DestVehicle->data.hitpointsCur;
			while (SrcBuilding->SubBase->Metal > 0 && iValue < DestVehicle->data.hitpointsMax)
			{
				iValue += Round ( ( (float) DestVehicle->data.hitpointsMax / DestVehicle->data.buildCosts) * 4);
				SrcBuilding->SubBase->addMetal (*this, -1);
			}
			iValue = std::min (DestVehicle->data.hitpointsMax, iValue);
		}
		// the changed values aren't interesting for enemy players,
		// so only send the new data to the owner
		sendUnitData (*this, *SrcBuilding, SrcBuilding->owner->getNr());
	}

	// repair or reload the destination unit
	if (DestVehicle)
	{
		if (iType == SUPPLY_TYPE_REARM) DestVehicle->data.ammoCur = DestVehicle->data.ammoMax;
		else DestVehicle->data.hitpointsCur = iValue;

		sendSupply (*this, DestVehicle->iID, true, iValue, iType, DestVehicle->owner->getNr());

		// send unitdata to the players who are not the owner
		for (size_t i = 0; i != DestVehicle->seenByPlayerList.size(); ++i)
			sendUnitData (*this, *DestVehicle, DestVehicle->seenByPlayerList[i]->getNr());
	}
	else
	{
		if (iType == SUPPLY_TYPE_REARM) DestBuilding->data.ammoCur = DestBuilding->data.ammoMax;
		else DestBuilding->data.hitpointsCur = iValue;

		sendSupply (*this, DestBuilding->iID, false, iValue, iType, DestBuilding->owner->getNr());

		// send unitdata to the players who are not the owner
		for (size_t i = 0; i != DestBuilding->seenByPlayerList.size(); ++i)
			sendUnitData (*this, *DestBuilding, DestBuilding->seenByPlayerList[i]->getNr());
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
	const int availableMetal = storingBuilding->SubBase->Metal;

	std::vector<cVehicle*> upgradedVehicles;
	for (size_t i = 0; i != storingBuilding->storedUnits.size(); ++i)
	{
		if (upgradeAll || i == storageSlot)
		{
			cVehicle* vehicle = storingBuilding->storedUnits[i];
			const sUnitData& upgradedVersion = *storingBuilding->owner->getUnitDataCurrentVersion (vehicle->data.ID);

			if (vehicle->data.version >= upgradedVersion.version)
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
		sendUpgradeVehicles (*this, upgradedVehicles, totalCosts, storingBuilding->iID, storingBuilding->owner->getNr());
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

	const int off = Map->getOffset (Vehicle->PosX, Vehicle->PosY);
	cBuilding* building = (*Map) [off].getRubble();

	if (!building)
	{
		sendClearAnswer (*this, 2, *Vehicle, 0, -1, Vehicle->owner->getNr());
		return;
	}

	int rubbleoffset = -1;
	if (building->data.isBig)
	{
		rubbleoffset = Map->getOffset (building->PosX, building->PosY);

		sideStepStealthUnit (building->PosX    , building->PosY    , *Vehicle, rubbleoffset);
		sideStepStealthUnit (building->PosX + 1, building->PosY    , *Vehicle, rubbleoffset);
		sideStepStealthUnit (building->PosX    , building->PosY + 1, *Vehicle, rubbleoffset);
		sideStepStealthUnit (building->PosX + 1, building->PosY + 1, *Vehicle, rubbleoffset);

		if ( (!Map->possiblePlace (*Vehicle, building->PosX    , building->PosY) && rubbleoffset != off) ||
			 (!Map->possiblePlace (*Vehicle, building->PosX + 1, building->PosY) && rubbleoffset + 1 != off) ||
			 (!Map->possiblePlace (*Vehicle, building->PosX    , building->PosY + 1) && rubbleoffset + Map->getSize() != off) ||
			 (!Map->possiblePlace (*Vehicle, building->PosX + 1, building->PosY + 1) && rubbleoffset + Map->getSize() + 1 != off))
		{
			sendClearAnswer (*this, 1, *Vehicle, 0, -1, Vehicle->owner->getNr());
			return;
		}

		Vehicle->BuildBigSavedPos = off;
		Map->moveVehicleBig (*Vehicle, building->PosX, building->PosY);
	}

	Vehicle->IsClearing = true;
	Vehicle->ClearingRounds = building->data.isBig ? 4 : 1;
	Vehicle->owner->doScan();
	addJob (new cStartBuildJob (*Vehicle, off % Map->getSize(), off / Map->getSize(), building->data.isBig));

	sendClearAnswer (*this, 0, *Vehicle, Vehicle->ClearingRounds, rubbleoffset, Vehicle->owner->getNr());
	for (size_t i = 0; i != Vehicle->seenByPlayerList.size(); ++i)
	{
		sendClearAnswer (*this, 0, *Vehicle, 0, rubbleoffset, Vehicle->seenByPlayerList[i]->getNr());
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

	if (Vehicle->IsClearing)
	{
		Vehicle->IsClearing = false;
		Vehicle->ClearingRounds = 0;

		if (Vehicle->data.isBig)
		{
			Map->moveVehicle (*Vehicle, Vehicle->BuildBigSavedPos % Map->getSize(), Vehicle->BuildBigSavedPos / Map->getSize());
			Vehicle->owner->doScan();
			sendStopClear (*this, *Vehicle, Vehicle->BuildBigSavedPos, Vehicle->owner->getNr());
			for (size_t i = 0; i != Vehicle->seenByPlayerList.size(); ++i)
			{
				sendStopClear (*this, *Vehicle, Vehicle->BuildBigSavedPos, Vehicle->seenByPlayerList[i]->getNr());
			}
		}
		else
		{
			sendStopClear (*this, *Vehicle, -1, Vehicle->owner->getNr());
			for (size_t i = 0; i != Vehicle->seenByPlayerList.size(); ++i)
			{
				sendStopClear (*this, *Vehicle, -1, Vehicle->seenByPlayerList[i]->getNr());
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
	cPlayer* LocalPlayer = getPlayerFromNumber (message.iPlayerNr);
	if (LocalPlayer->isLocal() == false) return;

	// delete disconnected players
	for (size_t i = 0; i != DisconnectedPlayerList.size(); ++i)
	{
		deletePlayer (DisconnectedPlayerList[i]);
	}
	DisconnectedPlayerList.clear();
	disableFreezeMode (FREEZE_WAIT_FOR_RECONNECT);
	if (bPlayTurns) sendWaitFor (*this, iActiveTurnPlayerNr);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_IDENTIFICATION (cNetMessage& message)
{
	assert (message.iType == GAME_EV_IDENTIFICATION);

	const std::string playerName = message.popString();
	const int socketNumber = message.popInt16();

	for (size_t i = 0; i != DisconnectedPlayerList.size(); ++i)
	{
		if (!playerName.compare (DisconnectedPlayerList[i]->getName()))
		{
			DisconnectedPlayerList[i]->setSocketIndex (socketNumber);
			sendReconnectAnswer (*this, socketNumber, *DisconnectedPlayerList[i]);
			return;
		}
	}
	sendReconnectAnswer (*network, socketNumber);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_RECON_SUCESS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_RECON_SUCESS);

	cPlayer* Player = NULL;
	const int playerNum = message.popInt16();
	// remove the player from the disconnected list
	for (size_t i = 0; i != DisconnectedPlayerList.size(); ++i)
	{
		if (DisconnectedPlayerList[i]->getNr() == playerNum)
		{
			Player = DisconnectedPlayerList[i];
			DisconnectedPlayerList.erase (DisconnectedPlayerList.begin() + i);
			break;
		}
	}
	resyncPlayer (Player);

	disableFreezeMode (FREEZE_WAIT_FOR_RECONNECT);
	if (bPlayTurns) sendWaitFor (*this, iActiveTurnPlayerNr);
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
			StoringVehicle->storeVehicle (StoredVehicle, Map);
			if (StoredVehicle->ServerMoveJob) StoredVehicle->ServerMoveJob->release();
			// vehicle is removed from enemy clients by cServer::checkPlayerUnits()
			sendStoreVehicle (*this, StoringVehicle->iID, true, StoredVehicle->iID, StoringVehicle->owner->getNr());
		}
	}
	else
	{
		cBuilding* StoringBuilding = getBuildingFromID (message.popInt16());
		if (!StoringBuilding) return;

		if (StoringBuilding->canLoad (StoredVehicle))
		{
			StoringBuilding->storeVehicle (StoredVehicle, Map);
			if (StoredVehicle->ServerMoveJob) StoredVehicle->ServerMoveJob->release();
			// vehicle is removed from enemy clients by cServer::checkPlayerUnits()
			sendStoreVehicle (*this, StoringBuilding->iID, false, StoredVehicle->iID, StoringBuilding->owner->getNr());
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

		const int x = message.popInt16();
		const int y = message.popInt16();
		if (!StoringVehicle->isNextTo (x, y)) return;

		// sidestep stealth units if necessary
		sideStepStealthUnit (x, y, *StoredVehicle);

		if (StoringVehicle->canExitTo (x, y, *Map, StoredVehicle->data))
		{
			StoringVehicle->exitVehicleTo (StoredVehicle, Map->getOffset (x, y), Map);
			// vehicle is added to enemy clients by cServer::checkPlayerUnits()
			sendActivateVehicle (*this, StoringVehicle->iID, true, StoredVehicle->iID, x, y, StoringVehicle->owner->getNr());
			if (StoredVehicle->data.canSurvey)
			{
				sendVehicleResources (*this, *StoredVehicle);
				StoredVehicle->doSurvey (*this);
			}

			if (StoredVehicle->canLand (*Map))
			{
				StoredVehicle->FlightHigh = 0;
			}
			else
			{
				StoredVehicle->FlightHigh = 64;
			}
			StoredVehicle->InSentryRange (*this);
		}
	}
	else
	{
		cBuilding* StoringBuilding = getBuildingFromID (message.popInt16());
		if (!StoringBuilding) return;

		const int x = message.popInt16();
		const int y = message.popInt16();
		if (!StoringBuilding->isNextTo (x, y)) return;

		// sidestep stealth units if necessary
		sideStepStealthUnit (x, y, *StoredVehicle);

		if (StoringBuilding->canExitTo (x, y, *Map, StoredVehicle->data))
		{
			StoringBuilding->exitVehicleTo (StoredVehicle, Map->getOffset (x, y), Map);
			// vehicle is added to enemy clients by cServer::checkPlayerUnits()
			sendActivateVehicle (*this, StoringBuilding->iID, false, StoredVehicle->iID, x, y, StoringBuilding->owner->getNr());
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

	cPlayer* player = getPlayerFromNumber (message.popChar());
	if (player) resyncPlayer (player, true);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_BUY_UPGRADES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_BUY_UPGRADES);

	const int iPlayerNr = message.popInt16();
	cPlayer* player = getPlayerFromNumber (iPlayerNr);
	if (player == 0)
		return;

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

		sUnitData* upgradedUnit = player->getUnitDataCurrentVersion (ID);
		if (upgradedUnit == 0)
			continue; // skip this upgrade, because there is no such unitData

		const int costs = getUpgradeCosts (ID, *player, newDamage, newMaxShots, newRange, newMaxAmmo, newArmor, newMaxHitPoints, newScan, newMaxSpeed);
		if (costs <= player->Credits)
		{
			// update the unitData of the player and send an ack-msg
			// for this upgrade to the player
			upgradedUnit->damage = newDamage;
			upgradedUnit->shotsMax = newMaxShots;
			upgradedUnit->range = newRange;
			upgradedUnit->ammoMax = newMaxAmmo;
			upgradedUnit->armor = newArmor;
			upgradedUnit->hitpointsMax = newMaxHitPoints;
			upgradedUnit->scan = newScan;
			if (ID.isAVehicle()) upgradedUnit->speedMax = newMaxSpeed;
			upgradedUnit->version++;

			player->Credits -= costs;
			updateCredits = true;

			sendUnitUpgrades (*this, *upgradedUnit, iPlayerNr);
		}
	}
	if (updateCredits)
		sendCredits (*this, player->Credits, iPlayerNr);
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

	const int availableMetal = building->SubBase->Metal;

	const sUnitData& upgradedVersion = *player->getUnitDataCurrentVersion (building->data.ID);
	if (building->data.version >= upgradedVersion.version)
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
			if (otherBuilding->data.version >= upgradedVersion.version)
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
			if (!scanNecessary && upgradedBuildings[i]->data.scan < upgradedVersion.scan)
				scanNecessary = true;
			if (upgradedBuildings[i]->sentryActive && upgradedBuildings[i]->data.range < upgradedVersion.range)
				refreshSentry = true;

			upgradedBuildings[i]->upgradeToCurrentVersion();
		}
		sendUpgradeBuildings (*this, upgradedBuildings, totalCosts, player->getNr());
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
	cPlayer* player = getPlayerFromNumber (iPlayerNr);
	if (player == 0)
		return;
	int newUsedResearch = 0;
	int newResearchSettings[cResearch::kNrResearchAreas];
	for (int area = cResearch::kNrResearchAreas - 1; area >= 0; --area)
	{
		newResearchSettings[area] = message.popInt16();
		newUsedResearch += newResearchSettings[area];
	}
	if (newUsedResearch > player->ResearchCount)
		return; // can't turn on research centers automatically!

	// needed, if newUsedResearch < player->ResearchCount
	std::vector<cBuilding*> researchCentersToStop;
	std::vector<cBuilding*> researchCentersToChangeArea;
	std::vector<int> newAreasForResearchCenters;

	bool error = false;
	cBuilding* curBuilding = player->BuildingList;
	for (int newArea = 0; newArea != cResearch::kNrResearchAreas; ++newArea)
	{
		int centersToAssign = newResearchSettings[newArea];
		while (centersToAssign > 0 && curBuilding != 0)
		{
			if (curBuilding->data.canResearch && curBuilding->IsWorking)
			{
				researchCentersToChangeArea.push_back (curBuilding);
				newAreasForResearchCenters.push_back (newArea);
				--centersToAssign;
			}
			curBuilding = curBuilding->next;
		}
		if (curBuilding == 0 && centersToAssign > 0)
		{
			error = true; // not enough active research centers!
			break;
		}
	}
	// shut down unused research centers
	for (; curBuilding != 0; curBuilding = curBuilding->next)
	{
		if (curBuilding->data.canResearch && curBuilding->IsWorking)
			researchCentersToStop.push_back (curBuilding);
	}
	if (error)
		return;

	for (size_t i = 0; i != researchCentersToStop.size(); ++i)
		researchCentersToStop[i]->ServerStopWork (*this, false);

	for (size_t i = 0; i != researchCentersToChangeArea.size(); ++i)
		researchCentersToChangeArea[i]->researchArea = newAreasForResearchCenters[i];
	player->refreshResearchCentersWorkingOnArea();

	sendResearchSettings (*this, researchCentersToChangeArea, newAreasForResearchCenters, iPlayerNr);
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
	if (! ( (destUnit && srcVehicle->canDoCommandoAction (destUnit->PosX, destUnit->PosY, Map, steal)) ||
			(destBuilding && destBuilding->data.isBig && srcVehicle->canDoCommandoAction (destBuilding->PosX, destBuilding->PosY + 1, Map, steal)) ||
			(destBuilding && destBuilding->data.isBig && srcVehicle->canDoCommandoAction (destBuilding->PosX + 1, destBuilding->PosY, Map, steal)) ||
			(destBuilding && destBuilding->data.isBig && srcVehicle->canDoCommandoAction (destBuilding->PosX + 1, destBuilding->PosY + 1, Map, steal)))) return;

	// check whether the action is successfull or not
	const int chance = srcVehicle->calcCommandoChance (destUnit, steal);
	bool success = false;
	if (random (100) < chance)
	{
		if (steal)
		{
			if (destVehicle)
			{
				// change the owner
				if (destVehicle->IsBuilding) stopVehicleBuilding (destVehicle);
				if (destVehicle->ServerMoveJob) destVehicle->ServerMoveJob->release();
				changeUnitOwner (destVehicle, srcVehicle->owner);
			}
		}
		else
		{
			// only on disabling units the infiltartor gets exp.
			// As higher his level is as slower he rises onto the next one.
			// every 5 rankings he needs one succesfull disabling more,
			// to get to the next ranking
			srcVehicle->CommandoRank += 1.f / ( ( (int) srcVehicle->CommandoRank + 5) / 5);

			const int strength = srcVehicle->calcCommandoTurns (destUnit);
			// stop the unit and make it disabled
			destUnit->turnsDisabled = strength;
			if (destVehicle)
			{
				// save speed and number of shots before disabling
				destVehicle->lastSpeed = destVehicle->data.speedCur;
				destVehicle->lastShots = destVehicle->data.shotsCur;

				destVehicle->data.speedCur = 0;
				destVehicle->data.shotsCur = 0;

				if (destVehicle->IsBuilding) stopVehicleBuilding (destVehicle);
				if (destVehicle->ServerMoveJob) destVehicle->ServerMoveJob->release();
			}
			else if (destBuilding)
			{
				// save number of shots before disabling
				destBuilding->lastShots = destBuilding->data.shotsCur;

				destBuilding->data.shotsCur = 0;
				destBuilding->wasWorking = destBuilding->IsWorking;
				destBuilding->ServerStopWork (*this, true);
				sendDoStopWork (*this, *destBuilding);
			}
			sendUnitData (*this, *destUnit, destUnit->owner->getNr());
			for (size_t i = 0; i != destUnit->seenByPlayerList.size(); ++i)
			{
				sendUnitData (*this, *destUnit, destUnit->seenByPlayerList[i]->getNr());
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
		for (size_t i = 0; i != PlayerList->size(); ++i)
		{
			cPlayer* player = (*PlayerList) [i];
			if (player == srcVehicle->owner) continue;
			if (!player->canSeeAnyAreaUnder (*srcVehicle)) continue;

			srcVehicle->setDetectedByPlayer (*this, player);
		}
		checkPlayerUnits();
		srcVehicle->InSentryRange (*this);
	}
	srcVehicle->data.shotsCur--;
	sendUnitData (*this, *srcVehicle, srcVehicle->owner->getNr());
	sendCommandoAnswer (*this, success, steal, *srcVehicle, srcVehicle->owner->getNr());
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_SAVE_HUD_INFO (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SAVE_HUD_INFO);

	const int msgSaveingID = message.popInt16();
	if (msgSaveingID != savingID) return;
	cPlayer* player = getPlayerFromNumber (message.popInt16());
	if (player == NULL) return;

	player->savedHud->selUnitID = message.popInt16();
	player->savedHud->offX = message.popInt16();
	player->savedHud->offY = message.popInt16();
	player->savedHud->zoom = message.popFloat();
	player->savedHud->colorsChecked = message.popBool();
	player->savedHud->gridChecked = message.popBool();
	player->savedHud->ammoChecked = message.popBool();
	player->savedHud->fogChecked = message.popBool();
	player->savedHud->twoXChecked = message.popBool();
	player->savedHud->rangeChecked = message.popBool();
	player->savedHud->scanChecked = message.popBool();
	player->savedHud->statusChecked = message.popBool();
	player->savedHud->surveyChecked = message.popBool();
	player->savedHud->lockChecked = message.popBool();
	player->savedHud->hitsChecked = message.popBool();
	player->savedHud->tntChecked = message.popBool();
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_SAVE_REPORT_INFO (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SAVE_REPORT_INFO);

	const int msgSaveingID = message.popInt16();
	if (msgSaveingID != savingID) return;
	cPlayer* player = getPlayerFromNumber (message.popInt16());
	if (player == NULL) return;

	sSavedReportMessage savedReport;
	savedReport.message = message.popString();
	savedReport.type = (sSavedReportMessage::eReportTypes) message.popInt16();
	savedReport.xPos = message.popInt16();
	savedReport.yPos = message.popInt16();
	savedReport.unitID = message.popID();
	savedReport.colorNr = message.popInt16();

	player->savedReportsList.push_back (savedReport);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_FIN_SEND_SAVE_INFO (cNetMessage& message)
{
	assert (message.iType == GAME_EV_FIN_SEND_SAVE_INFO);

	const int msgSaveingID = message.popInt16();
	if (msgSaveingID != savingID) return;
	cPlayer* player = getPlayerFromNumber (message.popInt16());
	if (player == NULL) return;

	cSavegame savegame (savingIndex);
	savegame.writeAdditionalInfo (*player->savedHud, player->savedReportsList, player);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_REQUEST_CASUALTIES_REPORT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REQUEST_CASUALTIES_REPORT);

	sendCasualtiesReport (*this, message.iPlayerNr);
}

//------------------------------------------------------------------------------
void cServer::handleNetMessage_GAME_EV_WANT_SELFDESTROY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_SELFDESTROY);

	cBuilding* building = getBuildingFromID (message.popInt16());
	if (!building || building->owner->getNr() != message.iPlayerNr) return;

	sendSelfDestroy (*this, *building);
	destroyUnit (building);
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
		sendUnitData (*this, *unit, i);
	sendUnitData (*this, *unit, unit->owner->getNr());
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
int cServer::handleNetMessage (cNetMessage* message)
{
	if (message->iType != NET_GAME_TIME_CLIENT)
		Log.write ("Server: --> " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);

	switch (message->iType)
	{
		case TCP_ACCEPT: handleNetMessage_TCP_ACCEPT (*message); break;
		case TCP_CLOSE: // Follow
		case GAME_EV_WANT_DISCONNECT:
			handleNetMessage_TCP_CLOSE_OR_GAME_EV_WANT_DISCONNECT (*message);
			break;
		case GAME_EV_CHAT_CLIENT: handleNetMessage_GAME_EV_CHAT_CLIENT (*message); break;
		case GAME_EV_WANT_TO_END_TURN: handleNetMessage_GAME_EV_WANT_TO_END_TURN (*message); break;
		case GAME_EV_WANT_START_WORK: handleNetMessage_GAME_EV_WANT_START_WORK (*message); break;
		case GAME_EV_WANT_STOP_WORK: handleNetMessage_GAME_EV_WANT_STOP_WORK (*message); break;
		case GAME_EV_MOVE_JOB_CLIENT: handleNetMessage_GAME_EV_MOVE_JOB_CLIENT (*message); break;
		case GAME_EV_WANT_STOP_MOVE: handleNetMessage_GAME_EV_WANT_STOP_MOVE (*message); break;
		case GAME_EV_MOVEJOB_RESUME: handleNetMessage_GAME_EV_MOVEJOB_RESUME (*message); break;
		case GAME_EV_WANT_ATTACK: handleNetMessage_GAME_EV_WANT_ATTACK (*message); break;
		case GAME_EV_ATTACKJOB_FINISHED: handleNetMessage_GAME_EV_ATTACKJOB_FINISHED (*message); break;
		case GAME_EV_MINELAYERSTATUS: handleNetMessage_GAME_EV_MINELAYERSTATUS (*message); break;
		case GAME_EV_WANT_BUILD: handleNetMessage_GAME_EV_WANT_BUILD (*message); break;
		case GAME_EV_END_BUILDING: handleNetMessage_GAME_EV_END_BUILDING (*message); break;
		case GAME_EV_WANT_STOP_BUILDING: handleNetMessage_GAME_EV_WANT_STOP_BUILDING (*message); break;
		case GAME_EV_WANT_TRANSFER: handleNetMessage_GAME_EV_WANT_TRANSFER (*message); break;
		case GAME_EV_WANT_BUILDLIST: handleNetMessage_GAME_EV_WANT_BUILDLIST (*message); break;
		case GAME_EV_WANT_EXIT_FIN_VEH: handleNetMessage_GAME_EV_WANT_EXIT_FIN_VEH (*message); break;
		case GAME_EV_CHANGE_RESOURCES: handleNetMessage_GAME_EV_CHANGE_RESOURCES (*message); break;
		case GAME_EV_WANT_CHANGE_MANUAL_FIRE: handleNetMessage_GAME_EV_WANT_CHANGE_MANUAL_FIRE (*message); break;
		case GAME_EV_WANT_CHANGE_SENTRY: handleNetMessage_GAME_EV_WANT_CHANGE_SENTRY (*message); break;
		case GAME_EV_WANT_MARK_LOG: handleNetMessage_GAME_EV_WANT_MARK_LOG (*message); break;
		case GAME_EV_WANT_SUPPLY: handleNetMessage_GAME_EV_WANT_SUPPLY (*message); break;
		case GAME_EV_WANT_VEHICLE_UPGRADE: handleNetMessage_GAME_EV_WANT_VEHICLE_UPGRADE (*message); break;
		case GAME_EV_WANT_START_CLEAR: handleNetMessage_GAME_EV_WANT_START_CLEAR (*message); break;
		case GAME_EV_WANT_STOP_CLEAR: handleNetMessage_GAME_EV_WANT_STOP_CLEAR (*message); break;
		case GAME_EV_ABORT_WAITING: handleNetMessage_GAME_EV_ABORT_WAITING (*message); break;
		case GAME_EV_IDENTIFICATION: handleNetMessage_GAME_EV_IDENTIFICATION (*message); break;
		case GAME_EV_RECON_SUCESS: handleNetMessage_GAME_EV_RECON_SUCESS (*message); break;
		case GAME_EV_WANT_LOAD: handleNetMessage_GAME_EV_WANT_LOAD (*message); break;
		case GAME_EV_WANT_EXIT: handleNetMessage_GAME_EV_WANT_EXIT (*message); break;
		case GAME_EV_REQUEST_RESYNC: handleNetMessage_GAME_EV_REQUEST_RESYNC (*message); break;
		case GAME_EV_WANT_BUY_UPGRADES: handleNetMessage_GAME_EV_WANT_BUY_UPGRADES (*message); break;
		case GAME_EV_WANT_BUILDING_UPGRADE: handleNetMessage_GAME_EV_WANT_BUILDING_UPGRADE (*message); break;
		case GAME_EV_WANT_RESEARCH_CHANGE: handleNetMessage_GAME_EV_WANT_RESEARCH_CHANGE (*message); break;
		case GAME_EV_AUTOMOVE_STATUS: handleNetMessage_GAME_EV_AUTOMOVE_STATUS (*message); break;
		case GAME_EV_WANT_COM_ACTION: handleNetMessage_GAME_EV_WANT_COM_ACTION (*message); break;
		case GAME_EV_SAVE_HUD_INFO: handleNetMessage_GAME_EV_SAVE_HUD_INFO (*message); break;
		case GAME_EV_SAVE_REPORT_INFO: handleNetMessage_GAME_EV_SAVE_REPORT_INFO (*message); break;
		case GAME_EV_FIN_SEND_SAVE_INFO: handleNetMessage_GAME_EV_FIN_SEND_SAVE_INFO (*message); break;
		case GAME_EV_REQUEST_CASUALTIES_REPORT: handleNetMessage_GAME_EV_REQUEST_CASUALTIES_REPORT (*message); break;
		case GAME_EV_WANT_SELFDESTROY: handleNetMessage_GAME_EV_WANT_SELFDESTROY (*message); break;
		case GAME_EV_WANT_CHANGE_UNIT_NAME: handleNetMessage_GAME_EV_WANT_CHANGE_UNIT_NAME (*message); break;
		case GAME_EV_END_MOVE_ACTION: handleNetMessage_GAME_EV_END_MOVE_ACTION (*message); break;
		case NET_GAME_TIME_CLIENT: gameTimer.handleSyncMessage (*message); break;
		default:
			Log.write ("Server: Can not handle message, type " + message->getTypeAsString(), cLog::eLOG_TYPE_NET_ERROR);
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
	if (newDamage > currentVersion->damage)
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->damage, currentVersion->damage, newDamage, cUpgradeCalculator::kAttack, player.researchLevel);
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
	if (newRange > currentVersion->range)
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->range, currentVersion->range, newRange, cUpgradeCalculator::kRange, player.researchLevel);
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
	if (newArmor > currentVersion->armor)
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->armor, currentVersion->armor, newArmor, cUpgradeCalculator::kArmor, player.researchLevel);
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
	if (newScan > currentVersion->scan)
	{
		const int costForUpgrade = uc.getCostForUpgrade (startVersion->scan, currentVersion->scan, newScan, cUpgradeCalculator::kScan, player.researchLevel);
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
void cServer::placeInitialResources (std::vector<sClientLandData>& landData, const sSettings& settings)
{
	for (size_t i = 0; i != landData.size(); ++i)
	{
		correctLandingPos (landData[i].iLandX, landData[i].iLandY);
		Map->placeRessourcesAddPlayer (landData[i].iLandX, landData[i].iLandY, settings.resFrequency);
	}
	Map->placeRessources (settings.metal, settings.oil, settings.gold);
}

//------------------------------------------------------------------------------
cVehicle* cServer::landVehicle (int iX, int iY, int iWidth, int iHeight, const sUnitData& unitData, cPlayer* player)
{
	for (int offY = -iHeight / 2; offY < iHeight / 2; ++offY)
	{
		for (int offX = -iWidth / 2; offX < iWidth / 2; ++offX)
		{
			if (!Map->possiblePlaceVehicle (unitData, iX + offX, iY + offY, player)) continue;

			return addVehicle (iX + offX, iY + offY, unitData.ID, player, true);
		}
	}
	return NULL;
}

//------------------------------------------------------------------------------
void cServer::makeLanding (const std::vector<sClientLandData>& landPos,
							const std::vector<std::vector<sLandingUnit>*>& landingUnits,
							const sSettings& settings)
{
	const bool fixed = settings.bridgeHead == SETTING_BRIDGEHEAD_DEFINITE;
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		const int x = landPos[i].iLandX;
		const int y = landPos[i].iLandY;
		cPlayer* player = (*PlayerList) [i];

		makeLanding (x, y, player, *landingUnits[i], fixed);
	}
}

//------------------------------------------------------------------------------
void cServer::makeLanding (int iX, int iY, cPlayer* Player, const std::vector<sLandingUnit>& landingUnits, bool bFixed)
{
	// Find place for mine if bridgehead is fixed
	if (bFixed)
	{
		if (Map->possiblePlaceBuilding (*UnitsData.specialIDSmallGen.getUnitDataOriginalVersion(), iX - 1, iY - 1 + 1) &&
			Map->possiblePlaceBuilding (*UnitsData.specialIDMine.getUnitDataOriginalVersion(), iX - 1 + 1, iY - 1) &&
			Map->possiblePlaceBuilding (*UnitsData.specialIDMine.getUnitDataOriginalVersion(), iX - 1 + 2, iY - 1) &&
			Map->possiblePlaceBuilding (*UnitsData.specialIDMine.getUnitDataOriginalVersion(), iX - 1 + 2, iY - 1 + 1) &&
			Map->possiblePlaceBuilding (*UnitsData.specialIDMine.getUnitDataOriginalVersion(), iX - 1 + 1, iY - 1 + 1))
		{
			// place buildings:
			addBuilding (iX - 1,     iY - 1 + 1, UnitsData.specialIDSmallGen, Player, true);
			addBuilding (iX - 1 + 1, iY - 1,     UnitsData.specialIDMine, Player, true);
		}
		else
		{
			Log.write ("couldn't place player start mine: " + Player->getName(), cLog::eLOG_TYPE_ERROR);
		}
	}

	int iWidth = 2;
	int iHeight = 2;
	for (size_t i = 0; i != landingUnits.size(); ++i)
	{
		const sLandingUnit& Landing = landingUnits[i];
		cVehicle* Vehicle = landVehicle (iX, iY, iWidth, iHeight, *Landing.unitID.getUnitDataOriginalVersion (Player), Player);
		while (!Vehicle)
		{
			iWidth += 2;
			iHeight += 2;
			Vehicle = landVehicle (iX, iY, iWidth, iHeight, *Landing.unitID.getUnitDataOriginalVersion (Player), Player);
		}
		if (Landing.cargo && Vehicle)
		{
			Vehicle->data.storageResCur = Landing.cargo;
			sendUnitData (*this, *Vehicle, Vehicle->owner->getNr());
		}
	}
}

//------------------------------------------------------------------------------
void cServer::correctLandingPos (int& iX, int& iY)
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
				if (Map->possiblePlaceBuildingWithMargin (*UnitsData.specialIDSmallGen.getUnitDataOriginalVersion(), iX + offX, iY + offY + 1, margin) &&
					Map->possiblePlaceBuildingWithMargin (*UnitsData.specialIDMine.getUnitDataOriginalVersion(), iX + offX + 1, iY + offY    , margin) &&
					Map->possiblePlaceBuildingWithMargin (*UnitsData.specialIDMine.getUnitDataOriginalVersion(), iX + offX + 2, iY + offY    , margin) &&
					Map->possiblePlaceBuildingWithMargin (*UnitsData.specialIDMine.getUnitDataOriginalVersion(), iX + offX + 2, iY + offY + 1, margin) &&
					Map->possiblePlaceBuildingWithMargin (*UnitsData.specialIDMine.getUnitDataOriginalVersion(), iX + offX + 1, iY + offY + 1, margin))
				{
					iX += offX + 1;
					iY += offY + 1;
					return;
				}
			}
		}
		iWidth += 2;
		iHeight += 2;
	}
}

//------------------------------------------------------------------------------
cVehicle* cServer::addVehicle (int iPosX, int iPosY, const sID& id, cPlayer* Player, bool bInit, bool bAddToMap, unsigned int uid)
{
	// generate the vehicle:
	cVehicle* AddedVehicle = Player->addVehicle (iPosX, iPosY, id, uid ? uid : iNextUnitID);
	iNextUnitID++;

	// place the vehicle:
	if (bAddToMap) Map->addVehicle (*AddedVehicle, iPosX, iPosY);

	// scan with surveyor:
	if (AddedVehicle->data.canSurvey)
	{
		sendVehicleResources (*this, *AddedVehicle);
		AddedVehicle->doSurvey (*this);
	}
	if (!bInit) AddedVehicle->InSentryRange (*this);

	if (AddedVehicle->canLand (*Map))
	{
		AddedVehicle->FlightHigh = 0;
	}
	else
	{
		AddedVehicle->FlightHigh = 64;
	}

	sendAddUnit (*this, iPosX, iPosY, AddedVehicle->iID, true, id, Player->getNr(), bInit);

	// detection must be done, after the vehicle has been sent to clients
	AddedVehicle->makeDetection (*this);
	return AddedVehicle;
}

//------------------------------------------------------------------------------
cBuilding* cServer::addBuilding (int iPosX, int iPosY, const sID& id, cPlayer* Player, bool bInit, unsigned int uid)
{
	// generate the building:
	cBuilding* AddedBuilding = Player->addBuilding (iPosX, iPosY, id, uid ? uid : iNextUnitID);
	if (AddedBuilding->data.canMineMaxRes > 0) AddedBuilding->CheckRessourceProd (*this);
	if (AddedBuilding->sentryActive) Player->addSentry (AddedBuilding);

	iNextUnitID++;

	int iOff = Map->getOffset (iPosX, iPosY);
	cBuilding* buildingToBeDeleted = Map->fields[iOff].getTopBuilding();

	Map->addBuilding (*AddedBuilding, iPosX, iPosY);
	sendAddUnit (*this, iPosX, iPosY, AddedBuilding->iID, false, id, Player->getNr(), bInit);

	// integrate the building to the base:
	Player->base.addBuilding (AddedBuilding, this);

	// if this is a top building, delete connectors, mines and roads
	if (AddedBuilding->data.surfacePosition == sUnitData::SURFACE_POS_GROUND)
	{
		if (AddedBuilding->data.isBig)
		{
			std::vector<cBuilding*>* buildings = &Map->fields[iOff].getBuildings();

			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ( (*buildings) [i]->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit ( (*buildings) [i]);
					--i;
				}
			}
			iOff++;
			buildings = &Map->fields[iOff].getBuildings();
			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ( (*buildings) [i]->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit ( (*buildings) [i]);
					--i;
				}
			}
			iOff += Map->getSize();
			buildings = &Map->fields[iOff].getBuildings();
			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ( (*buildings) [i]->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit ( (*buildings) [i]);
					--i;
				}
			}
			iOff--;
			buildings = &Map->fields[iOff].getBuildings();
			for (size_t i = 0; i != buildings->size(); ++i)
			{
				if ( (*buildings) [i]->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit ( (*buildings) [i]);
					--i;
				}
			}
		}
		else
		{
			deleteUnit (buildingToBeDeleted);

			std::vector<cBuilding*>& buildings = Map->fields[iOff].getBuildings();
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

	if (AddedBuilding->data.canMineMaxRes > 0)
	{
		sendProduceValues (*this, *AddedBuilding);
		AddedBuilding->ServerStartWork (*this);
	}
	AddedBuilding->makeDetection (*this);
	return AddedBuilding;
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

	if (unit->owner && casualtiesTracker && ( (unit->isABuilding() && unit->data.buildCosts <= 2) == false))
		casualtiesTracker->logCasualty (unit->data.ID, unit->owner->getNr());

	if (unit->isABuilding())
	{
		cBuilding* building = static_cast<cBuilding*> (unit);
		remove_from_intrusivelist (building->owner->BuildingList, *building);
	}
	else
	{
		cVehicle* vehicle = static_cast<cVehicle*> (unit);
		remove_from_intrusivelist (vehicle->owner->VehicleList, *vehicle);
	}

	// detach from attack job
	if (unit->attacking)
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
	unit->owner->deleteSentry (unit);

	// lose eco points
	if (unit->isABuilding() && static_cast<cBuilding*> (unit)->points != 0)
	{
		unit->owner->setScore (unit->owner->getScore (iTurn) - static_cast<cBuilding*> (unit)->points, iTurn);
		sendScore (*this, *unit->owner, iTurn);
	}

	if (unit->isABuilding())
		Map->deleteBuilding (*static_cast<cBuilding*> (unit));
	else
		Map->deleteVehicle (*static_cast<cVehicle*> (unit));

	if (notifyClient)
		sendDeleteUnit (*this, *unit, -1);

	if (unit->isABuilding() && static_cast<cBuilding*> (unit)->SubBase != 0)
		unit->owner->base.deleteBuilding (static_cast<cBuilding*> (unit), this);

	cPlayer* owner = unit->owner;
	delete unit;

	if (owner != 0)
		owner->doScan();
}

//------------------------------------------------------------------------------
void cServer::checkPlayerUnits (cVehicle& vehicle, cPlayer& MapPlayer)
{
	if (&MapPlayer == vehicle.owner) return;

	std::vector<cPlayer*>& seenByPlayers = vehicle.seenByPlayerList;
	const bool stealthUnit = vehicle.data.isStealthOn != TERRAIN_NONE;

	if (MapPlayer.canSeeAnyAreaUnder (vehicle) && !vehicle.Loaded &&
		(!stealthUnit || vehicle.isDetectedByPlayer (&MapPlayer) || (MapPlayer.isDefeated && openMapDefeat)))
	{
		if (Contains (seenByPlayers, &MapPlayer) == false)
		{
			seenByPlayers.push_back (&MapPlayer);
			sendAddEnemyUnit (*this, vehicle, MapPlayer.getNr());
			sendUnitData (*this, vehicle, MapPlayer.getNr());
			if (vehicle.ServerMoveJob)
			{
				sendMoveJobServer (*this, *vehicle.ServerMoveJob, MapPlayer.getNr());
				if (Contains (ActiveMJobs, vehicle.ServerMoveJob) && !vehicle.ServerMoveJob->bFinished && !vehicle.ServerMoveJob->bEndForNow && vehicle.moving)
				{
					Log.write (" Server: sending extra MJOB_OK for unit ID " + iToStr (vehicle.iID) + " to client " + iToStr (MapPlayer.getNr()), cLog::eLOG_TYPE_NET_DEBUG);
					cNetMessage* message = new cNetMessage (GAME_EV_NEXT_MOVE);
					message->pushChar (MJOB_OK);
					message->pushInt16 (vehicle.iID);
					sendNetMessage (message, MapPlayer.getNr());
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
			sendDeleteUnit (*this, vehicle, MapPlayer.getNr());
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
			sendAddEnemyUnit (*this, building, MapPlayer.getNr());
			sendUnitData (*this, building, MapPlayer.getNr());
		}
	}
	else
	{
		std::vector<cPlayer*>::iterator it = std::find (seenByPlayers.begin(), seenByPlayers.end(), &MapPlayer);

		if (it != seenByPlayers.end())
		{
			seenByPlayers.erase (it);
			sendDeleteUnit (*this, building, MapPlayer.getNr());
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
			sendAddRubble (*this, building, MapPlayer.getNr());
		}
	}
	else
	{
		std::vector<cPlayer*>::iterator it = std::find (seenByPlayers.begin(), seenByPlayers.end(), &MapPlayer);

		if (it != seenByPlayers.end())
		{
			seenByPlayers.erase (it);
			sendDeleteUnit (*this, building, MapPlayer.getNr());
		}
	}
}

//------------------------------------------------------------------------------
void cServer::checkPlayerUnits()
{
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		// The player whos unit is it
		cPlayer* UnitPlayer = (*PlayerList) [i];
		for (cVehicle* NextVehicle = UnitPlayer->VehicleList;
			 NextVehicle != NULL;
			 NextVehicle = NextVehicle->next)
		{
			for (size_t j = 0; j != PlayerList->size(); ++j)
			{
				checkPlayerUnits (*NextVehicle, * (*PlayerList) [j]);
			}
		}
		for (cBuilding* NextBuilding = UnitPlayer->BuildingList;
			 NextBuilding != NULL;
			 NextBuilding = NextBuilding->next)
		{
			for (size_t j = 0; j != PlayerList->size(); ++j)
			{
				checkPlayerUnits (*NextBuilding, * (*PlayerList) [j]);
			}
		}
	}

	//check the neutral objects
	for (cBuilding* building = neutralBuildings; building != NULL; building = building->next)
	{
		for (size_t i = 0; i != PlayerList->size(); ++i)
		{
			checkPlayerRubbles (*building, * (*PlayerList) [i]);
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
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		(*PlayerList) [i]->onSocketIndexDisconnected (socketIndex);
	}
	deletePlayer (player);
}

//------------------------------------------------------------------------------
void cServer::markAllPlayersAsDisconnected()
{
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		cPlayer* player = (*PlayerList) [i];
		if (Contains (DisconnectedPlayerList, player) == false)
			DisconnectedPlayerList.push_back (player);
		player->revealMap();
	}
}

//------------------------------------------------------------------------------
cPlayer* cServer::getPlayerFromNumber (int iNum)
{
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		cPlayer* p = (*PlayerList) [i];
		if (p->getNr() == iNum) return p;
	}
	return NULL;
}

//------------------------------------------------------------------------------
cPlayer* cServer::getPlayerFromString (const std::string& playerID)
{
	// first try to find player by number
	const int playerNr = atoi (playerID.c_str());
	if (playerNr != 0 || playerID[0] == '0')
	{
		return getPlayerFromNumber (playerNr);
	}

	// try to find player by name
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		if ( (*PlayerList) [i]->getName().compare (playerID) == 0) return (*PlayerList) [i];
	}
	return NULL;
}

//------------------------------------------------------------------------------
void cServer::handleEnd (int iPlayerNum)
{
	const eGameTypes gameType = getGameType();

	if (gameType == GAME_TYPE_SINGLE)
	{
		sendTurnFinished (*this, iPlayerNum, -1);
		if (checkEndActions (iPlayerNum))
		{
			iWantPlayerEndNum = iPlayerNum;
			return;
		}
		iTurn++;
		makeTurnEnd();
	}
	else if (gameType == GAME_TYPE_HOTSEAT || bPlayTurns)
	{
		const bool bWaitForPlayer = (gameType == GAME_TYPE_TCPIP && bPlayTurns);
		if (checkEndActions (iPlayerNum))
		{
			iWantPlayerEndNum = iPlayerNum;
			return;
		}
		iActiveTurnPlayerNr++;
		if (iActiveTurnPlayerNr >= (int) PlayerList->size())
		{
			iActiveTurnPlayerNr = 0;
			makeTurnEnd();
			iTurn++;

			if (gameType == GAME_TYPE_HOTSEAT)
			{
				sendMakeTurnEnd (*this, true, bWaitForPlayer, (*PlayerList) [iActiveTurnPlayerNr]->getNr(), iPlayerNum);
			}
			else
			{
				for (size_t i = 0; i != PlayerList->size(); ++i)
				{
					sendMakeTurnEnd (*this, true, bWaitForPlayer, (*PlayerList) [iActiveTurnPlayerNr]->getNr(), (*PlayerList) [i]->getNr());
				}
			}
		}
		else
		{
			if (gameType == GAME_TYPE_HOTSEAT)
			{
				sendMakeTurnEnd (*this, false, bWaitForPlayer, (*PlayerList) [iActiveTurnPlayerNr]->getNr(), iPlayerNum);
				// TODO: in hotseat:
				// maybe send information to client about the next player
			}
			else
			{
				for (size_t i = 0; i != PlayerList->size(); ++i)
				{
					sendMakeTurnEnd (*this, false, bWaitForPlayer, (*PlayerList) [iActiveTurnPlayerNr]->getNr(), i);
				}
			}
		}
		// send report to next player
		sendTurnReport (*this, * (*PlayerList) [iActiveTurnPlayerNr]);
	}
	else // it's a simultaneous TCP/IP multiplayer game
	{
		// defeated player are ignored when they hit the end button
		if (getPlayerFromNumber (iPlayerNum)->isDefeated) return;

		// check whether this player has already finished his turn
		for (size_t i = 0; i != PlayerEndList.size(); ++i)
		{
			if (PlayerEndList[i]->getNr() == iPlayerNum) return;
		}
		PlayerEndList.push_back (getPlayerFromNumber (iPlayerNum));
		const bool firstTimeEnded = PlayerEndList.size() == 1;

		// make sure that all defeated players are added to the endlist
		for (size_t i = 0; i != PlayerList->size(); ++i)
		{
			cPlayer* player = (*PlayerList) [i];
			if (player->isDefeated)
			{
				if (Contains (PlayerEndList, player) == false)
					PlayerEndList.push_back (player);
			}
		}

		if (iWantPlayerEndNum == -1)
		{
			// When playing with dedicated server
			// where a player is not connected, play without a deadline,
			// but wait till all players pressed "End".
			if (firstTimeEnded && (DEDICATED_SERVER == false || DisconnectedPlayerList.size() == 0))
			{
				sendTurnFinished (*this, iPlayerNum, 100 * iTurnDeadline);
				iDeadlineStartTime = gameTimer.gameTime;
			}
			else
			{
				sendTurnFinished (*this, iPlayerNum, -1);
			}
		}

		if (PlayerEndList.size() >= PlayerList->size())
		{
			iDeadlineStartTime = 0;
			if (checkEndActions (-1))
			{
				iWantPlayerEndNum = iPlayerNum;
				return;
			}

			PlayerEndList.clear();

			iTurn++;
			makeTurnEnd();
		}
	}
}

//------------------------------------------------------------------------------
void cServer::handleWantEnd()
{
	if (!gameTimer.timer50ms) return;

	// wait until all clients have reported a gametime
	// that is after the turn end.
	// that means they have finished processing all the turn end messages,
	// and we can start the new turn simultaneously on all clients
	if (freezeModes.waitForTurnEnd && !executingRemainingMovements)
	{
		for (size_t i = 0; i != PlayerList->size(); ++i)
		{
			cPlayer& player = *(*PlayerList) [i];
			if (!isPlayerDisconnected (player) && gameTimer.getReceivedTime (i) <= lastTurnEnd)
				return;
		}

		// send reports to all players
		for (size_t i = 0; i != PlayerList->size(); ++i)
		{
			sendMakeTurnEnd (*this, true, false, -1, i);
		}
		for (size_t i = 0; i != PlayerList->size(); ++i)
		{
			sendTurnReport (*this, * (*PlayerList) [i]);
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
		handleEnd (iWantPlayerEndNum);
	}
}

//------------------------------------------------------------------------------
bool cServer::checkEndActions (int iPlayer)
{
	enableFreezeMode (FREEZE_WAIT_FOR_TURNEND);

	std::string sMessage;
	if (ActiveMJobs.empty() == false)
	{
		sMessage = "Text~Comp~Turn_Wait";
	}
	else
	{
		for (size_t i = 0; i != PlayerList->size(); ++i)
		{
			for (cVehicle* NextVehicle = (*PlayerList) [i]->VehicleList;
				 NextVehicle != NULL;
				 NextVehicle = NextVehicle->next)
			{
				if (NextVehicle->ServerMoveJob && NextVehicle->data.speedCur > 0 && !NextVehicle->moving)
				{
					// restart movejob
					NextVehicle->ServerMoveJob->resume();
					sMessage = "Text~Comp~Turn_Automove";
				}
			}
		}
	}
	if (!sMessage.empty())
	{
		if (iPlayer != -1)
		{
			if (iWantPlayerEndNum == -1)
			{
				sendChatMessageToClient (*this, sMessage, SERVER_INFO_MESSAGE, iPlayer);
			}
		}
		else
		{
			if (iWantPlayerEndNum == -1)
			{
				for (size_t i = 0; i != PlayerList->size(); ++i)
				{
					sendChatMessageToClient (*this, sMessage, SERVER_INFO_MESSAGE, (*PlayerList) [i]->getNr());
				}
			}
		}
		executingRemainingMovements = true;
		return true;
	}
	executingRemainingMovements = false;
	return false;
}

//------------------------------------------------------------------------------
void cServer::makeTurnEnd()
{
	enableFreezeMode (FREEZE_WAIT_FOR_TURNEND);
	lastTurnEnd = gameTimer.gameTime;

	// reload all buildings
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		cPlayer& player = *(*PlayerList) [i];
		for (cBuilding* Building = player.BuildingList;
			 Building;
			 Building = Building->next)
		{
			bool forceSendUnitData = false;
			if (Building->isDisabled())
			{
				Building->turnsDisabled--;
				if (Building->isDisabled() == false && Building->wasWorking)
				{
					Building->ServerStartWork (*this);
					Building->wasWorking = false;
				}
				forceSendUnitData = true;
			}
			if ( (Building->data.canAttack && Building->refreshData()) || forceSendUnitData)
			{
				for (size_t k = 0; k != Building->seenByPlayerList.size(); ++k)
				{
					sendUnitData (*this, *Building, Building->seenByPlayerList[k]->getNr());
				}
				sendUnitData (*this, *Building, Building->owner->getNr());
			}
		}
	}

	// reload all vehicles
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		cPlayer& player = *(*PlayerList) [i];
		for (cVehicle* Vehicle = player.VehicleList;
			 Vehicle;
			 Vehicle = Vehicle->next)
		{
			bool isModified = false;
			if (Vehicle->isDisabled())
			{
				Vehicle->turnsDisabled--;
				isModified = true;
			}
			isModified |= Vehicle->refreshData();
			isModified |= Vehicle->refreshData_Build (*this);
			isModified |= Vehicle->refreshData_Clear (*this);

			if (isModified)
			{
				for (size_t k = 0; k != Vehicle->seenByPlayerList.size(); ++k)
				{
					sendUnitData (*this, *Vehicle, Vehicle->seenByPlayerList[k]->getNr());
				}
				sendUnitData (*this, *Vehicle, Vehicle->owner->getNr());
			}
			if (Vehicle->ServerMoveJob) Vehicle->ServerMoveJob->bEndForNow = false;
		}
	}

	// hide stealth units
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		cPlayer& player = *(*PlayerList) [i];
		player.doScan(); // make sure the detection maps are up to date

		for (cVehicle* vehicle = player.VehicleList; vehicle; vehicle = vehicle->next)
		{
			vehicle->clearDetectedInThisTurnPlayerList();
			vehicle->makeDetection (*this);
		}
	}

	// produce resources
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		(*PlayerList) [i]->base.handleTurnend (*this);
	}

	// do research:
	for (size_t i = 0; i != PlayerList->size(); ++i)
		(*PlayerList) [i]->doResearch (*this);

	// eco-spheres:
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		(*PlayerList) [i]->accumulateScore (*this);
	}

	// Gun'em down:
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		cPlayer& player = *(*PlayerList) [i];

		for (cVehicle* vehicle = player.VehicleList; vehicle; vehicle = vehicle->next)
		{
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
			Savegame.save (*this, lngPack.i18n ("Text~Settings~Autosave") + " " + lngPack.i18n ("Text~Comp~Turn") + " " + iToStr (iTurn));
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
void cServer::checkDefeats()
{
	std::set<cPlayer*> winners;
	std::set<cPlayer*> losers;
	int best_score = 0;

	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		cPlayer& player = *(*PlayerList) [i];
		if (player.isDefeated)
		{
			continue;
		}
		const int score = player.getScore (iTurn);
		if ( (scoreLimit && score >= scoreLimit) ||
			 (turnLimit && iTurn >= turnLimit))
		{
			if (score >= best_score)
			{
				if (score > best_score)
				{
					winners.clear();
					best_score = score;
				}
				winners.insert (&player);
			}
		}

		cVehicle* vehicle = player.VehicleList;
		for (; vehicle; vehicle = vehicle->next)
		{
			if (vehicle->data.canAttack || !vehicle->data.canBuild.empty()) break;
		}
		if (vehicle != NULL) continue;
		cBuilding* building = player.BuildingList;
		for (; building; building = building->next)
		{
			if (building->data.canAttack || !building->data.canBuild.empty()) break;
		}
		if (building != NULL) continue;

		losers.insert (&player);
	}

	// If some players have won, anyone who hasn't won has lost.
	if (!winners.empty())
		for (size_t i = 0; i != PlayerList->size(); ++i)
		{
			cPlayer& player = *(*PlayerList) [i];

			if (winners.find (&player) == winners.end())
				losers.insert (&player);
		}

	// Defeat all players who have lost.
	for (std::set<cPlayer*>::iterator i = losers.begin(); i != losers.end(); ++i)
	{
		cPlayer& player = **i;

		player.isDefeated = true;
		sendDefeated (*this, player);

		if (openMapDefeat && player.getSocketNum() != -1)
		{
			player.revealMap();
			checkPlayerUnits();
			sendNoFog (*this, player.getNr());
		}
	}

	// Handle the case where there is more than one winner.
	// Original MAX calls a draw and displays the results screen.
	// For now we will have sudden death,
	// i.e. first player to get ahead in score wins.
	if (winners.size() > 1)
	{
		for (size_t i = 0; i != PlayerList->size(); ++i)
			sendChatMessageToClient (*this, "Text~Comp~SuddenDeath", SERVER_INFO_MESSAGE, i);
	}
}

//------------------------------------------------------------------------------
void cServer::addReport (sID id, int iPlayerNum)
{
	cPlayer* player = getPlayerFromNumber (iPlayerNum);
	if (id.isAVehicle())
	{
		for (size_t i = 0; i != player->ReportVehicles.size(); ++i)
		{
			sTurnstartReport& report = *player->ReportVehicles[i];
			if (report.Type == id)
			{
				report.iAnz++;
				return;
			}
		}
		sTurnstartReport* report = new sTurnstartReport;
		report->Type = id;
		report->iAnz = 1;
		player->ReportVehicles.push_back (report);
	}
	else
	{
		for (size_t i = 0; i != player->ReportBuildings.size(); ++i)
		{
			sTurnstartReport* report = player->ReportBuildings[i];
			if (report->Type == id)
			{
				report->iAnz++;
				return;
			}
		}
		sTurnstartReport* report = new sTurnstartReport;
		report->Type = id;
		report->iAnz = 1;
		player->ReportBuildings.push_back (report);
	}
}

//------------------------------------------------------------------------------
void cServer::checkDeadline()
{
	if (!gameTimer.timer50ms) return;
	if (iTurnDeadline < 0 || iDeadlineStartTime <= 0) return;

	if (gameTimer.gameTime <= iDeadlineStartTime + iTurnDeadline * 100) return;

	if (checkEndActions (-1))
	{
		iWantPlayerEndNum = -2;
		return;
	}

	PlayerEndList.clear();

	iTurn++;
	iDeadlineStartTime = 0;
	makeTurnEnd();
}

//------------------------------------------------------------------------------
void cServer::addActiveMoveJob (cServerMoveJob* MoveJob)
{
	ActiveMJobs.push_back (MoveJob);
}

//------------------------------------------------------------------------------
void cServer::handleMoveJobs()
{
	for (int i = ActiveMJobs.size() - 1; i >= 0; i--)
	{
		cServerMoveJob* MoveJob = ActiveMJobs[i];
		cVehicle* Vehicle = MoveJob->Vehicle;

		//suspend movejobs of attacked vehicles
		if (Vehicle && Vehicle->isBeeingAttacked)
			continue;

		// stop the job
		if (MoveJob->bEndForNow && Vehicle)
		{
			Log.write (" Server: Movejob has end for now and will be stoped (delete from active ones)", cLog::eLOG_TYPE_NET_DEBUG);
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
				Vehicle->moving = false;
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
				if (Vehicle->data.storageResCur >= Vehicle->BuildCostsStart && Map->possiblePlaceBuilding (*Vehicle->BuildingTyp.getUnitDataOriginalVersion(), Vehicle->PosX, Vehicle->PosY, Vehicle))
				{
					addJob (new cStartBuildJob (*Vehicle, Vehicle->PosX, Vehicle->PosY, Vehicle->data.isBig));
					Vehicle->IsBuilding = true;
					Vehicle->BuildCosts = Vehicle->BuildCostsStart;
					Vehicle->BuildRounds = Vehicle->BuildRoundsStart;
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

		if (!Vehicle->moving)
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
				Log.write (" Server: Movejob has end for now and will be stoped (delete from active ones)", cLog::eLOG_TYPE_NET_DEBUG);
				sendNextMove (*this, *Vehicle, MJOB_STOP, MoveJob->iSavedSpeed);
				ActiveMJobs.erase (ActiveMJobs.begin() + i);
				continue;
			}
		}

		if (MoveJob->iNextDir != Vehicle->dir)
		{
			// rotate vehicle
			if (gameTimer.timer100ms)
			{
				Vehicle->rotateTo (MoveJob->iNextDir);
			}
		}
		else
		{
			// move the vehicle
			if (gameTimer.timer10ms)
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
cVehicle* cServer::getVehicleFromID (unsigned int iID) const
{
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		for (cVehicle* vehicle = (*PlayerList) [i]->VehicleList;
			 vehicle != 0;
			 vehicle = vehicle->next)
		{
			if (vehicle->iID == iID)
				return vehicle;
		}
	}
	return 0;
}

//------------------------------------------------------------------------------
cBuilding* cServer::getBuildingFromID (unsigned int iID) const
{
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		for (cBuilding* building = (*PlayerList) [i]->BuildingList;
			 building != 0;
			 building = building->next)
		{
			if (building->iID == iID)
				return building;
		}
	}
	return 0;
}

//------------------------------------------------------------------------------
void cServer::destroyUnit (cVehicle* vehicle)
{
	const int offset = Map->getOffset (vehicle->PosX, vehicle->PosY);
	int value = 0;
	int oldRubbleValue = 0;
	bool bigRubble = false;
	int rubblePosX = vehicle->PosX;
	int rubblePosY = vehicle->PosY;

	if (vehicle->data.factorAir > 0 && vehicle->FlightHigh != 0)
	{
		deleteUnit (vehicle);
		return;
	}

	//delete all buildings on the field, except connectors
	std::vector<cBuilding*>* buildings = & (*Map) [offset].getBuildings();
	std::vector<cBuilding*>::iterator b_it = buildings->begin();
	if (b_it != buildings->end() && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;

	while (b_it != buildings->end())
	{
		// this seems to be rubble
		if ( (*b_it)->owner == 0 && (*b_it)->RubbleValue > 0)
		{
			oldRubbleValue += (*b_it)->RubbleValue;
			if ( (*b_it)->data.isBig)
			{
				rubblePosX = (*b_it)->PosX;
				rubblePosY = (*b_it)->PosY;
				bigRubble = true;
			}
		}
		else // normal unit
			value += (*b_it)->data.buildCosts;
		deleteUnit (*b_it);
		b_it = buildings->begin();
		if (b_it != buildings->end() && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;
	}

	if (vehicle->data.isBig)
	{
		bigRubble = true;
		buildings = & (*Map) [offset + 1].getBuildings();
		b_it = buildings->begin();
		if (b_it != buildings->end() && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;
		while (b_it != buildings->end())
		{
			value += (*b_it)->data.buildCosts;
			deleteUnit (*b_it);
			b_it = buildings->begin();
			if (b_it != buildings->end() && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;
		}

		buildings = & (*Map) [offset + Map->getSize()].getBuildings();
		b_it = buildings->begin();
		if (b_it != buildings->end() && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;
		while (b_it != buildings->end())
		{
			value += (*b_it)->data.buildCosts;
			deleteUnit (*b_it);
			b_it = buildings->begin();
			if (b_it != buildings->end() && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;
		}

		buildings = & (*Map) [offset + 1 + Map->getSize()].getBuildings();
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

	if (!vehicle->data.hasCorpse)
	{
		value += vehicle->data.buildCosts;
		// stored material is always added completely to the rubble
		if (vehicle->data.storeResType == sUnitData::STORE_RES_METAL)
			value += vehicle->data.storageResCur * 2;
	}

	if (value > 0 || oldRubbleValue > 0)
		addRubble (rubblePosX, rubblePosY, value / 2 + oldRubbleValue, bigRubble);

	deleteUnit (vehicle);
}

//------------------------------------------------------------------------------
int cServer::deleteBuildings (std::vector<cBuilding*>& buildings)
{
	int rubble = 0;
	while (buildings.empty() == false)
	{
		cBuilding* building = buildings[0];
		if (building->owner)
		{
			rubble += building->data.buildCosts;
			if (building->data.storeResType == sUnitData::STORE_RES_METAL)
				rubble += building->data.storageResCur * 2; // stored material is always added completely to the rubble
		}
		else
			rubble += building->RubbleValue * 2;
		deleteUnit (building);
	}
	return rubble;
}

//------------------------------------------------------------------------------
void cServer::destroyUnit (cBuilding* b)
{
	int offset = Map->getOffset (b->PosX, b->PosY);
	int rubble = 0;
	bool big = false;

	cBuilding* topBuilding = Map->fields[offset].getTopBuilding();
	if (topBuilding && topBuilding->data.isBig)
	{
		big = true;
		offset = Map->getOffset (topBuilding->PosX, topBuilding->PosY);

		std::vector<cBuilding*>* buildings = &Map->fields[offset + 1].getBuildings();
		rubble += deleteBuildings (*buildings);

		buildings = &Map->fields[offset + Map->getSize()].getBuildings();
		rubble += deleteBuildings (*buildings);

		buildings = &Map->fields[offset + Map->getSize() + 1].getBuildings();
		rubble += deleteBuildings (*buildings);
	}

	sUnitData::eSurfacePosition surfacePosition = b->data.surfacePosition;

	std::vector<cBuilding*>* buildings = &Map->fields[offset].getBuildings();
	rubble += deleteBuildings (*buildings);

	if (surfacePosition != sUnitData::SURFACE_POS_ABOVE && rubble > 2)
		addRubble (offset % Map->getSize(), offset / Map->getSize(), rubble / 2, big);
}

//------------------------------------------------------------------------------
void cServer::addRubble (int x, int y, int value, bool big)
{
	value = std::max (1, value);

	if (Map->isWaterOrCoast (x, y))
	{
		if (big)
		{
			addRubble (x + 1, y    , value / 4, false);
			addRubble (x    , y + 1, value / 4, false);
			addRubble (x + 1, y + 1, value / 4, false);
		}
		return;
	}

	if (big && Map->isWaterOrCoast (x + 1, y))
	{
		addRubble (x    , y    , value / 4, false);
		addRubble (x    , y + 1, value / 4, false);
		addRubble (x + 1, y + 1, value / 4, false);
		return;
	}

	if (big && Map->isWaterOrCoast (x, y + 1))
	{
		addRubble (x    , y    , value / 4, false);
		addRubble (x + 1, y    , value / 4, false);
		addRubble (x + 1, y + 1, value / 4, false);
		return;
	}

	if (big && Map->isWaterOrCoast (x + 1, y + 1))
	{
		addRubble (x    , y    , value / 4, false);
		addRubble (x + 1, y    , value / 4, false);
		addRubble (x    , y + 1, value / 4, false);
		return;
	}

	cBuilding* rubble = new cBuilding (NULL, NULL, iNextUnitID);
	push_front_into_intrusivelist (neutralBuildings, *rubble);

	iNextUnitID++;

	rubble->PosX = x;
	rubble->PosY = y;

	rubble->data.isBig = big;
	rubble->RubbleValue = value;

	Map->addBuilding (*rubble, x, y);

	if (big)
	{
		rubble->RubbleTyp = random (2);
	}
	else
	{
		rubble->RubbleTyp = random (5);
	}
}

//------------------------------------------------------------------------------
void cServer::deleteRubble (cBuilding* rubble)
{
	Map->deleteBuilding (*rubble);

	remove_from_intrusivelist (neutralBuildings, *rubble);
	sendDeleteUnit (*this, *rubble, -1);

	delete rubble;
}

//------------------------------------------------------------------------------
void cServer::deletePlayer (cPlayer* Player)
{
	//remove units
	for (cVehicle* Vehicle = Player->VehicleList; Vehicle;)
	{
		cVehicle* nextVehicle = Vehicle->next;
		if (!Vehicle->Loaded) deleteUnit (Vehicle);
		Vehicle = nextVehicle;
	}
	while (Player->BuildingList)
	{
		deleteUnit (Player->BuildingList);
	}

	// remove the player of all detected by player lists
	for (unsigned int playerNum = 0; playerNum < PlayerList->size(); playerNum++)
	{
		cPlayer* UnitPlayer = (*PlayerList) [playerNum];
		if (UnitPlayer == Player) continue;

		for (cVehicle* Vehicle = UnitPlayer->VehicleList; Vehicle; Vehicle = Vehicle->next)
		{
			if (Vehicle->data.isStealthOn != TERRAIN_NONE && Vehicle->isDetectedByPlayer (Player)) Vehicle->resetDetectedByPlayer (*this, Player);
		}
	}
	// delete the player
	sendDeletePlayer (*this, *Player);
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		if (Player == (*PlayerList) [i])
		{
			PlayerList->erase (PlayerList->begin() + i);
			delete Player;
			break;
		}
	}
}

//------------------------------------------------------------------------------
void cServer::resyncPlayer (cPlayer* Player, bool firstDelete)
{
	Log.write (" Server:  ============================= begin resync  ==========================", cLog::eLOG_TYPE_NET_DEBUG);
	if (firstDelete)
	{
		for (size_t i = 0; i != PlayerList->size(); ++i)
		{
			cPlayer* UnitPlayer = (*PlayerList) [i];
			if (UnitPlayer == Player) continue;

			for (cVehicle* Vehicle = UnitPlayer->VehicleList; Vehicle; Vehicle = Vehicle->next)
			{
				Remove (Vehicle->seenByPlayerList, Player);
			}

			for (cBuilding* Building = UnitPlayer->BuildingList; Building; Building = Building->next)
			{
				Remove (Building->seenByPlayerList, Player);
			}
		}

		for (cBuilding* Building = neutralBuildings; Building; Building = Building->next)
		{
			Remove (Building->seenByPlayerList, Player);
		}
		sendDeleteEverything (*this, Player->getNr());
	}

	sendGameTime (*this, *Player, gameTimer.gameTime);

	//if (settings->clans == SETTING_CLANS_ON)
	{
		sendClansToClients (*this, *PlayerList);
	}
	sendTurn (*this, iTurn, lastTurnEnd, *Player);
	if (iDeadlineStartTime > 0) sendTurnFinished (*this, -1, 100 * iTurnDeadline - (gameTimer.gameTime - iDeadlineStartTime), Player);
	sendResources (*this, *Player);

	// send all units to the client
	for (cVehicle* Vehicle = Player->VehicleList; Vehicle; Vehicle = Vehicle->next)
	{
		if (!Vehicle->Loaded) resyncVehicle (*Vehicle, *Player);
	}

	for (cBuilding* Building = Player->BuildingList; Building; Building = Building->next)
	{
		sendAddUnit (*this, Building->PosX, Building->PosY, Building->iID, false, Building->data.ID, Player->getNr(), true);
		for (size_t i = 0; i != Building->storedUnits.size(); ++i)
		{
			cVehicle& storedVehicle = *Building->storedUnits[i];
			resyncVehicle (storedVehicle, *Player);
			sendStoreVehicle (*this, Building->iID, false, storedVehicle.iID, Player->getNr());
		}
		sendUnitData (*this, *Building, Player->getNr());
		if (Building->data.canMineMaxRes > 0) sendProduceValues (*this, *Building);
		if (Building->BuildList && Building->BuildList->size() > 0) sendBuildList (*this, *Building);
	}
	// send all subbases
	for (size_t i = 0; i != Player->base.SubBases.size(); ++i)
	{
		sendSubbaseValues (*this, *Player->base.SubBases[i], Player->getNr());
	}
	// refresh enemy units
	Player->doScan();
	checkPlayerUnits();
	// send upgrades
	for (size_t i = 0; i != UnitsData.getNrVehicles(); ++i)
	{
		// if only costs were researched, the version is not incremented
		if (Player->VehicleData[i].version > 0
			|| Player->VehicleData[i].buildCosts != UnitsData.getVehicle (i, Player->getClan()).buildCosts)
			sendUnitUpgrades (*this, Player->VehicleData[i], Player->getNr());
	}
	for (size_t i = 0; i != UnitsData.getNrBuildings(); ++i)
	{
		// if only costs were researched, the version is not incremented
		if (Player->BuildingData[i].version > 0
			|| Player->BuildingData[i].buildCosts != UnitsData.getBuilding (i, Player->getClan()).buildCosts)
			sendUnitUpgrades (*this, Player->BuildingData[i], Player->getNr());
	}
	// send credits
	sendCredits (*this, Player->Credits, Player->getNr());
	// send research
	sendResearchLevel (*this, Player->researchLevel, Player->getNr());
	sendRefreshResearchCount (*this, Player->getNr());

	// send all players' score histories & eco-counts
	for (size_t i = 0; i != PlayerList->size(); ++i)
	{
		cPlayer& subj = *(*PlayerList) [i];
		for (int t = 1; t <= iTurn; ++t)
			sendScore (*this, subj, t, Player);
		sendNumEcos (*this, subj, Player);
	}

	sendVictoryConditions (*this, *Player);

	// send attackJobs
	for (size_t i = 0; i != AJobs.size(); ++i)
	{
		cServerAttackJob& ajob = *AJobs[i];
		for (size_t ajobClient = 0; ajobClient != ajob.executingClients.size(); ++ajobClient)
		{
			if (ajob.executingClients[ajobClient] == Player)
			{
				ajob.sendFireCommand (Player);
			}
		}
	}

	Log.write (" Server:  ============================= end resync  ==========================", cLog::eLOG_TYPE_NET_DEBUG);
}

//------------------------------------------------------------------------------
void cServer::resyncVehicle (const cVehicle& Vehicle, const cPlayer& Player)
{
	sendAddUnit (*this, Vehicle.PosX, Vehicle.PosY, Vehicle.iID, true, Vehicle.data.ID, Player.getNr(), true, !Vehicle.Loaded);
	if (Vehicle.ServerMoveJob) sendMoveJobServer (*this, *Vehicle.ServerMoveJob, Player.getNr());
	for (size_t i = 0; i != Vehicle.storedUnits.size(); ++i)
	{
		const cVehicle& storedVehicle = *Vehicle.storedUnits[i];
		resyncVehicle (storedVehicle, Player);
		sendStoreVehicle (*this, Vehicle.iID, true, storedVehicle.iID, Player.getNr());
	}
	sendUnitData (*this, Vehicle, Player.getNr());
	sendSpecificUnitData (*this, Vehicle);
	if (Vehicle.hasAutoMoveJob) sendSetAutomoving (*this, Vehicle);
	if (Vehicle.detectedByPlayerList.empty() == false) sendDetectionState (*this, Vehicle);
}

//------------------------------------------------------------------------------
bool cServer::addMoveJob (int srcX, int srcY, int destX, int destY, cVehicle* vehicle)
{
	cServerMoveJob* MoveJob = new cServerMoveJob (*this, srcX, srcY, destX, destY, vehicle);
	if (!MoveJob->calcPath())
	{
		delete MoveJob;
		vehicle->ServerMoveJob = NULL;
		return false;
	}

	sendMoveJobServer (*this, *MoveJob, vehicle->owner->getNr());
	for (size_t i = 0; i != vehicle->seenByPlayerList.size(); ++i)
	{
		sendMoveJobServer (*this, *MoveJob, vehicle->seenByPlayerList[i]->getNr());
	}

	addActiveMoveJob (MoveJob);
	return true;
}

//------------------------------------------------------------------------------
void cServer::changeUnitOwner (cVehicle* vehicle, cPlayer* newOwner)
{
	if (vehicle->owner && casualtiesTracker)
		casualtiesTracker->logCasualty (vehicle->data.ID, vehicle->owner->getNr());

	// delete vehicle in the list of the old player
	cPlayer* oldOwner = vehicle->owner;

	remove_from_intrusivelist (oldOwner->VehicleList, *vehicle);
	// add the vehicle to the list of the new player
	vehicle->owner = newOwner;
	newOwner->addUnitToList (vehicle);

	//the vehicle is fully operational for the new owner
	if (vehicle->isDisabled())
	{
		vehicle->data.speedCur = vehicle->lastSpeed;
		vehicle->data.shotsCur = vehicle->lastShots;
	}
	vehicle->turnsDisabled = 0;

	// delete the unit on the clients and add it with new owner again
	sendDeleteUnit (*this, *vehicle, oldOwner->getNr());
	for (size_t i = 0; i != vehicle->seenByPlayerList.size(); ++i)
	{
		sendDeleteUnit (*this, *vehicle, vehicle->seenByPlayerList[i]->getNr());
	}
	vehicle->seenByPlayerList.clear();
	vehicle->detectedByPlayerList.clear();
	sendAddUnit (*this, vehicle->PosX, vehicle->PosY, vehicle->iID, true, vehicle->data.ID, vehicle->owner->getNr(), false);
	sendUnitData (*this, *vehicle, vehicle->owner->getNr());
	sendSpecificUnitData (*this, *vehicle);

	oldOwner->doScan();
	newOwner->doScan();
	checkPlayerUnits();

	// let the unit work for his new owner
	if (vehicle->data.canSurvey)
	{
		sendVehicleResources (*this, *vehicle);
		vehicle->doSurvey (*this);
	}
	vehicle->makeDetection (*this);
}

//------------------------------------------------------------------------------
void cServer::stopVehicleBuilding (cVehicle* vehicle)
{
	if (!vehicle->IsBuilding) return;

	int iPos = Map->getOffset (vehicle->PosX, vehicle->PosY);

	vehicle->IsBuilding = false;
	vehicle->BuildPath = false;

	if (vehicle->BuildingTyp.getUnitDataOriginalVersion()->isBig)
	{
		Map->moveVehicle (*vehicle, vehicle->BuildBigSavedPos % Map->getSize(), vehicle->BuildBigSavedPos / Map->getSize());
		iPos = vehicle->BuildBigSavedPos;
		vehicle->owner->doScan();
	}
	sendStopBuild (*this, vehicle->iID, iPos, vehicle->owner->getNr());
	for (size_t i = 0; i != vehicle->seenByPlayerList.size(); ++i)
	{
		sendStopBuild (*this, vehicle->iID, iPos, vehicle->seenByPlayerList[i]->getNr());
	}
}

void cServer::sideStepStealthUnit (int PosX, int PosY, const cVehicle& vehicle, int bigOffset)
{
	sideStepStealthUnit (PosX, PosY, vehicle.data, vehicle.owner, bigOffset);
}

void cServer::sideStepStealthUnit (int PosX, int PosY, const sUnitData& vehicleData, cPlayer* vehicleOwner, int bigOffset)
{
	// TODO: make sure, the stealth vehicle takes the direct diagonal move.
	// Also when two straight moves would be shorter.

	if (vehicleData.factorAir > 0) return;

	// first look for an undetected stealth unit
	cVehicle* stealthVehicle = Map->fields[Map->getOffset (PosX, PosY)].getVehicle();
	if (!stealthVehicle) return;
	if (stealthVehicle->owner == vehicleOwner) return;
	if (stealthVehicle->data.isStealthOn == TERRAIN_NONE) return;
	if (stealthVehicle->isDetectedByPlayer (vehicleOwner)) return;

	// make sure a running movement is finished,
	// before starting the side step move
	if (stealthVehicle->moving) stealthVehicle->ServerMoveJob->doEndMoveVehicle();

	// found a stealth unit. Try to find a place where the unit can move
	bool placeFound = false;
	int minCosts = 99;
	int bestX, bestY;
	const int minx = std::max (PosX - 1, 0);
	const int maxx = std::min (PosX + 1, Map->getSize() - 1);
	const int miny = std::max (PosY - 1, 0);
	const int maxy = std::min (PosY + 1, Map->getSize() - 1);
	for (int x = minx; x <= maxx; ++x)
	{
		for (int y = miny; y <= maxy; ++y)
		{
			if (x == PosX && y == PosY) continue;

			// when a bigOffet was passed,
			// for example a contructor needs space for a big building
			// so not all directions are allowed for the side stepping
			if (bigOffset != -1)
			{
				int off = Map->getOffset (x, y);
				if (off == bigOffset ||
					off == bigOffset + 1 ||
					off == bigOffset + Map->getSize() ||
					off == bigOffset + Map->getSize() + 1) continue;
			}

			// check whether this field is a possible destination
			if (!Map->possiblePlace (*stealthVehicle, x, y)) continue;

			// check costs of the move
			cPathCalculator pathCalculator (0, 0, 0, 0, *Map, *stealthVehicle);
			int costs = pathCalculator.calcNextCost (PosX, PosY, x, y);
			if (costs > stealthVehicle->data.speedCur) continue;

			// check whether the vehicle would be detected
			// on the destination field
			bool detectOnDest = false;
			if (stealthVehicle->data.isStealthOn & TERRAIN_GROUND)
			{
				for (size_t i = 0; i != PlayerList->size(); ++i)
				{
					if ( (*PlayerList) [i] == stealthVehicle->owner) continue;
					if ( (*PlayerList) [i]->hasLandDetection (Map->getOffset (x, y))) detectOnDest = true;
				}
				if (Map->isWater (x, y)) detectOnDest = true;
			}
			if (stealthVehicle->data.isStealthOn & TERRAIN_SEA)
			{
				for (size_t i = 0; i != PlayerList->size(); ++i)
				{
					if ( (*PlayerList) [i] == stealthVehicle->owner) continue;
					if ( (*PlayerList) [i]->hasSeaDetection (Map->getOffset (x, y))) detectOnDest = true;
				}
				if (!Map->isWater (x, y)) detectOnDest = true;

				if (stealthVehicle->data.factorGround > 0 && stealthVehicle->data.factorSea > 0)
				{
					cBuilding* b = Map->fields[Map->getOffset (x, y)].getBaseBuilding();
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
				bestX = x;
				bestY = y;
				placeFound = true;
			}
		}
	}

	if (placeFound)
	{
		addMoveJob (PosX, PosY, bestX, bestY, stealthVehicle);
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

int cServer::getTurn() const
{
	return iTurn;
}

void cServer::addJob (cJob* job)
{
	helperJobs.addJob (*job);
}

void cServer::runJobs()
{
	helperJobs.run (gameTimer);
}

void cServer::enableFreezeMode (eFreezeMode mode, int playerNumber)
{
	freezeModes.enable (mode, playerNumber);
	switch (mode)
	{
		case FREEZE_PAUSE: gameTimer.stop(); break;
		case FREEZE_WAIT_FOR_RECONNECT: gameTimer.stop(); break;
		case FREEZE_WAIT_FOR_TURNEND: break;
		case FREEZE_WAIT_FOR_PLAYER:
			//gameTimer.stop(); //done in cGameTimer::nextTickAllowed();
			break;
		default:
			Log.write (" Server: Tried to enable unsupportet freeze mode: " + iToStr (mode), cLog::eLOG_TYPE_NET_ERROR);
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
			Log.write (" Server: Tried to disable unsupportet freeze mode: " + iToStr (mode), cLog::eLOG_TYPE_NET_ERROR);
	}

	if (!freezeModes.pause && !freezeModes.waitForReconnect)
	{
		gameTimer.start();
	}
}
