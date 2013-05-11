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
#include "hud.h"
#include "events.h"
#include "network.h"
#include "serverevents.h"
#include "clientevents.h"
#include "netmessage.h"
#include "attackJobs.h"
#include "movejobs.h"
#include "upgradecalculator.h"
#include "menus.h"
#include "settings.h"
#include "ringbuffer.h"
#include "buildings.h"
#include "vehicles.h"
#include "player.h"
#include "savegame.h"
#include "gametimer.h"
#include "jobs.h"
#if DEDICATED_SERVER_APPLICATION
#include "dedicatedserver.h"
#endif
#include "casualtiestracker.h"

#ifdef _MSC_VER
#include <windows.h>
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)
#endif


//-------------------------------------------------------------------------------------
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
		RaiseException (MS_VC_EXCEPTION, 0, sizeof (info) / sizeof (ULONG_PTR), (ULONG_PTR*) &info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif

	cServer* server = reinterpret_cast<cServer*> (arg);
	server->run();
	return 0;
}

//-------------------------------------------------------------------------------------
cServer::cServer (cTCP* network_, cMap* const map, cList<cPlayer*>* const PlayerList, eGameTypes const gameType, bool const bPlayTurns, int turnLimit, int scoreLimit)
	: network(network_)
	, gameTimer()
	, lastEvent (0)
	, lastTurnEnd (0)
	, executingRemainingMovements (false)
	, casualtiesTracker (0)
{
	assert (! (turnLimit && scoreLimit));

	this->turnLimit = turnLimit;
	this->scoreLimit = scoreLimit;
	Map = map;
	this->PlayerList = PlayerList;
	this->gameType = gameType;
	this->bPlayTurns = bPlayTurns;
	bHotSeat = false;
	bExit = false;
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
		ServerThread = SDL_CreateThread (CallbackRunServerThread, this);


	gameTimer.maxEventQueueSize = MAX_SERVER_EVENT_COUNTER;
	gameTimer.start ();
}

void cServer::stop ()
{
	bExit = true;
	gameTimer.stop ();

	if (!DEDICATED_SERVER)
	{
		if (ServerThread)
		{
			SDL_WaitThread (ServerThread, NULL);
			ServerThread = NULL;
		}
	}
}
//-------------------------------------------------------------------------------------
cServer::~cServer()
{

	stop ();

	delete casualtiesTracker;
	casualtiesTracker = 0;

	//disconect clients
	if (network)
	{
		for (unsigned int i = 0; i < PlayerList->Size(); i++)
		{
			network->close ( (*PlayerList) [i]->iSocketNum);
		}
	}

	while (eventQueue.size())
	{
		delete eventQueue.read();
	}
	delete lastEvent;
	lastEvent = 0;

	for (size_t i = 0; i != AJobs.Size(); ++i)
	{
		delete AJobs[i];
	}

	for (size_t i = 0; i != PlayerList->Size(); ++i)
	{
		delete (*PlayerList) [i];
	}
	PlayerList->Clear();
	while (neutralBuildings)
	{
		cBuilding* nextBuilding = static_cast<cBuilding*> (neutralBuildings->next);
		delete neutralBuildings;
		neutralBuildings = nextBuilding;
	}
}

//-------------------------------------------------------------------------------------
void cServer::setDeadline (int iDeadline)
{
	iTurnDeadline = iDeadline;
}

//-------------------------------------------------------------------------------------
cNetMessage* cServer::pollEvent()
{
	delete lastEvent;
	lastEvent = NULL;

	cNetMessage* event;
	if (eventQueue.size() <= 0)
	{
		return NULL;
	}

	event = eventQueue.read();
	lastEvent = event;

	return event;
}

//-------------------------------------------------------------------------------------
int cServer::pushEvent (cNetMessage* message)
{
	eventQueue.write (message);
	return 0;
}

//-------------------------------------------------------------------------------------
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
		if (EventHandler != 0)
			EventHandler->pushEvent (message);
		return;
	}

	cPlayer* Player = NULL;
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		if ( (Player = (*PlayerList) [i])->Nr == iPlayerNum) break;
	}

	if (Player == NULL)
	{
		//player not found
		Log.write ("Server: Can't send message. Player " + iToStr (iPlayerNum) + " not found.", cLog::eLOG_TYPE_NET_WARNING);
		delete message;
		return;
	}
	// Socketnumber MAX_CLIENTS for lokal client
	if (Player->iSocketNum == MAX_CLIENTS)
	{
		if (EventHandler != 0)
			EventHandler->pushEvent (message);
	}
	// on all other sockets the netMessage will be send over TCP/IP
	else
	{
		if (network) network->sendTo (Player->iSocketNum, message->iLength, message->serialize());
		delete message;
	}
}

//-------------------------------------------------------------------------------------
void cServer::run()
{
	while (!bExit)
	{
		cNetMessage* event = pollEvent();

		if (event)
		{
			HandleNetMessage (event);
			checkPlayerUnits();
		}

		// don't do anything if games hasn't been started yet!
		static unsigned int lastTime = 0;
		lastTime = gameTimer.gameTime;
		if (bStarted)
		{
			gameTimer.run (*this);
		}

		if (!event && lastTime == gameTimer.gameTime) //nothing to do
		{
			SDL_Delay (10);
		}
	}
}

void cServer::doGameActions()
{
	checkDeadline ();
	handleMoveJobs ();
	runJobs ();
	handleWantEnd ();
	checkPlayerUnits ();
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_TCT_ACCEPT (cNetMessage& message)
{
	assert (message.iType == TCP_ACCEPT);

	sendRequestIdentification (*network, message.popInt16());
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_TCT_CLOSE_OR_GAME_EV_WANT_DISCONNECT (cNetMessage& message)
{
	assert (message.iType == TCP_CLOSE || message.iType == GAME_EV_WANT_DISCONNECT);

	// Socket should be closed
	int iSocketNumber = -1;
	if (message.iType == TCP_CLOSE)
	{
		iSocketNumber = message.popInt16();
	}
	else // a client disconnected. Usefull for play with DEDICATED_SERVER
	{
		for (unsigned int i = 0; i < PlayerList->Size(); i++)
		{
			if (message.iPlayerNr == (*PlayerList) [i]->Nr)
			{
				iSocketNumber = (*PlayerList) [i]->iSocketNum;
				break;
			}
		}
	}
	if (iSocketNumber == -1)
		return;
	network->close (iSocketNumber);

	cPlayer* Player = NULL;
	// resort socket numbers of the players
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		if ( (*PlayerList) [i]->iSocketNum == iSocketNumber)
		{
			Player = (*PlayerList) [i];
		}
		else if ( (*PlayerList) [i]->iSocketNum > iSocketNumber && (*PlayerList) [i]->iSocketNum != MAX_CLIENTS)
		{
			(*PlayerList) [i]->iSocketNum--;
		}
	}

	if (Player)
	{
		Player->iSocketNum = -1;

		// freeze clients
		if (DEDICATED_SERVER == false)   // the dedicated server doesn't force to wait for reconnect, because it's expected client behaviour
			enableFreezeMode (FREEZE_WAIT_FOR_RECONNECT);
		sendChatMessageToClient (*this, "Text~Multiplayer~Lost_Connection", SERVER_INFO_MESSAGE, -1, Player->name);

		DisconnectedPlayerList.Add (Player);

		memset (Player->ScanMap, 0, Map->size * Map->size);
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_CHAT_CLIENT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_CHAT_CLIENT);

	sendChatMessageToClient (*this, message.popString(), USER_MESSAGE);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_TO_END_TURN (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_TO_END_TURN);

	if (bPlayTurns)
	{
		cPlayer* Player = getPlayerFromNumber (message.iPlayerNr);
		if ( (*PlayerList) [iActiveTurnPlayerNr] != Player) return;
	}
	handleEnd (message.iPlayerNr);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_START_WORK (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_START_WORK);

	int iID = message.popInt32();

	cBuilding* building = getBuildingFromID (iID);
	if (building == NULL || building->owner->Nr != message.iPlayerNr) return;

	//handle message
	building->ServerStartWork (*this);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_STOP_WORK (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_STOP_WORK);

	int iID = message.popInt32();

	cBuilding* building = getBuildingFromID (iID);
	if (building == NULL || building->owner->Nr != message.iPlayerNr) return;

	//handle message
	building->ServerStopWork (*this, false);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_MOVE_JOB_CLIENT (cNetMessage& message)
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
	sendMoveJobServer (*this, MoveJob, vehicle.owner->Nr);
	for (unsigned int i = 0; i < vehicle.seenByPlayerList.Size(); i++)
	{
		sendMoveJobServer (*this, MoveJob, vehicle.seenByPlayerList[i]->Nr);
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_STOP_MOVE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_STOP_MOVE);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle == NULL || Vehicle->ServerMoveJob == NULL) return;

	Vehicle->ServerMoveJob->stop();
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_MOVEJOB_RESUME (cNetMessage& message)
{
	assert (message.iType == GAME_EV_MOVEJOB_RESUME);

	int id = message.popInt32();
	if (id == 0)
	{
		cPlayer* player = getPlayerFromNumber (message.iPlayerNr);
		if (!player) return;
		cVehicle* vehicle = player->VehicleList;

		while (vehicle)
		{
			if (vehicle->ServerMoveJob && !vehicle->moving)
				vehicle->ServerMoveJob->resume();

			vehicle = static_cast<cVehicle*> (vehicle->next);
		}
	}
	else
	{
		cVehicle* vehicle = getVehicleFromID (id);
		if (!vehicle || vehicle->owner->Nr != message.iPlayerNr) return;

		if (vehicle->ServerMoveJob)
			vehicle->ServerMoveJob->resume();
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_ATTACK (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_ATTACK);
	//identify agressor
	bool bIsVehicle = message.popBool();
	cVehicle* attackingVehicle = NULL;
	cBuilding* attackingBuilding = NULL;
	if (bIsVehicle)
	{
		int ID = message.popInt32();
		attackingVehicle = getVehicleFromID (ID);
		if (attackingVehicle == NULL)
		{
			Log.write (" Server: vehicle with ID " + iToStr (ID) + " not found", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		if (attackingVehicle->owner->Nr != message.iPlayerNr)
		{
			Log.write (" Server: Message was not send by vehicle owner!", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		if (attackingVehicle->isBeeingAttacked) return;
	}
	else
	{
		int offset = message.popInt32();
		if (offset < 0 || offset > Map->size * Map->size)
		{
			Log.write (" Server: Invalid agressor offset", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		attackingBuilding = Map->fields[offset].getTopBuilding();
		if (attackingBuilding == NULL)
		{
			Log.write (" Server: No Building at agressor offset", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		if (attackingBuilding->owner->Nr != message.iPlayerNr)
		{
			Log.write (" Server: Message was not send by building owner!", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		if (attackingBuilding->isBeeingAttacked) return;
	}

	//find target offset
	int targetOffset = message.popInt32();
	if (targetOffset < 0 || targetOffset > Map->size * Map->size)
	{
		Log.write (" Server: Invalid target offset!", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}

	int targetID = message.popInt32();
	if (targetID != 0)
	{
		cVehicle* targetVehicle = getVehicleFromID (targetID);
		if (targetVehicle == NULL)
		{
			Log.write (" Server: vehicle with ID " + iToStr (targetID) + " not found!", cLog::eLOG_TYPE_NET_WARNING);
			return;
		}
		int oldOffset = targetOffset;
		//the target offset doesn't need to match the vehicle position, when it is big
		if (!targetVehicle->data.isBig)
		{
			targetOffset = targetVehicle->PosX + targetVehicle->PosY * Map->size;
		}
		Log.write (" Server: attacking vehicle " + targetVehicle->getDisplayName() + ", " + iToStr (targetVehicle->iID), cLog::eLOG_TYPE_NET_DEBUG);
		if (oldOffset != targetOffset) Log.write (" Server: target offset changed from " + iToStr (oldOffset) + " to " + iToStr (targetOffset), cLog::eLOG_TYPE_NET_DEBUG);
	}

	//check if attack is possible
	cUnit* attackingUnit = bIsVehicle ? (cUnit*) attackingVehicle : (cUnit*) attackingBuilding;
	if (attackingUnit->canAttackObjectAt (targetOffset % Map->size, targetOffset / Map->size, Map, true) == false)
	{
		Log.write (" Server: The server decided, that the attack is not possible", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}
	AJobs.Add (new cServerAttackJob (*this, attackingUnit, targetOffset, false));
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_ATTACKJOB_FINISHED (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ATTACKJOB_FINISHED);

	int ID = message.popInt16();
	cServerAttackJob* aJob = NULL;

	unsigned int i = 0;
	for (; i < AJobs.Size(); i++)
	{
		if (AJobs[i]->iID == ID)
		{
			aJob = AJobs[i];
			break;
		}
	}
	if (aJob == NULL)   //attack job not found
	{
		Log.write (" Server: ServerAttackJob not found", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	aJob->clientFinished (message.iPlayerNr);
	if (aJob->executingClients.Size() == 0)
	{
		AJobs.Delete (i);
		delete aJob;
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_MINELAYERSTATUS (cNetMessage& message)
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
		sendUnitData (*this, Vehicle, Vehicle->owner->Nr);
		for (unsigned int i = 0; i < Vehicle->seenByPlayerList.Size(); i++)
		{
			sendUnitData (*this, Vehicle, Vehicle->seenByPlayerList[i]->Nr);
		}
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_BUILD (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_BUILD);
	cVehicle* Vehicle;
	int iBuildSpeed, iBuildOff, iPathOff;
	int iTurboBuildRounds[3];
	int iTurboBuildCosts[3];

	Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle == NULL) return;
	if (Vehicle->IsBuilding || Vehicle->BuildPath) return;

	sID BuildingTyp;
	BuildingTyp.iFirstPart = message.popInt16();
	BuildingTyp.iSecondPart = message.popInt16();
	if (BuildingTyp.getUnitDataOriginalVersion() == NULL)
	{
		Log.write (" Server: invalid unit: " + iToStr (BuildingTyp.iFirstPart) + "."  + iToStr (BuildingTyp.iSecondPart), cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	const sUnitData& Data = *BuildingTyp.getUnitDataOriginalVersion();
	iBuildSpeed = message.popInt16();
	if (iBuildSpeed > 2 || iBuildSpeed < 0) return;
	iBuildOff = message.popInt32();

	Vehicle->calcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, BuildingTyp.getUnitDataCurrentVersion (Vehicle->owner)->buildCosts);

	if (iTurboBuildCosts[iBuildSpeed] > Vehicle->data.storageResCur || iTurboBuildRounds[iBuildSpeed] <= 0)
	{
		// TODO: differ between diffrent aborting types ( buildposition blocked, not enough material, ... )
		sendBuildAnswer (*this, false, Vehicle);
		return;
	}

	if (iBuildOff < 0 || iBuildOff >= Map->size * Map->size) return;
	int buildX = iBuildOff % Map->size;
	int buildY = iBuildOff / Map->size;
	int oldPosX = Vehicle->PosX;
	int oldPosY = Vehicle->PosY;

	if (Vehicle->data.canBuild.compare (Data.buildAs) != 0) return;

	if (Data.isBig)
	{
		sideStepStealthUnit (buildX    , buildY    , Vehicle, iBuildOff);
		sideStepStealthUnit (buildX + 1, buildY    , Vehicle, iBuildOff);
		sideStepStealthUnit (buildX    , buildY + 1, Vehicle, iBuildOff);
		sideStepStealthUnit (buildX + 1, buildY + 1, Vehicle, iBuildOff);

		if (! (Map->possiblePlaceBuilding (Data, buildX,     buildY    , Vehicle) &&
			   Map->possiblePlaceBuilding (Data, buildX + 1, buildY    , Vehicle) &&
			   Map->possiblePlaceBuilding (Data, buildX,     buildY + 1, Vehicle) &&
			   Map->possiblePlaceBuilding (Data, buildX + 1, buildY + 1, Vehicle)))
		{
			sendBuildAnswer (*this, false, Vehicle);
			return;
		}
		Vehicle->BuildBigSavedPos = Vehicle->PosX + Vehicle->PosY * Map->size;

		// set vehicle to build position
		Map->moveVehicleBig (Vehicle, buildX, buildY);
		Vehicle->owner->DoScan();
	}
	else
	{
		if (iBuildOff != Vehicle->PosX + Vehicle->PosY * Map->size) return;

		if (!Map->possiblePlaceBuilding (Data, iBuildOff, Vehicle))
		{
			sendBuildAnswer (*this, false, Vehicle);
			return;
		}
	}

	Vehicle->BuildingTyp = BuildingTyp;
	bool bBuildPath = message.popBool();
	iPathOff = message.popInt32();
	if (iPathOff < 0 || iPathOff >= Map->size * Map->size) return;
	Vehicle->BandX = iPathOff % Map->size;
	Vehicle->BandY = iPathOff / Map->size;

	Vehicle->BuildCosts = iTurboBuildCosts[iBuildSpeed];
	Vehicle->BuildRounds = iTurboBuildRounds[iBuildSpeed];
	Vehicle->BuildCostsStart = Vehicle->BuildCosts;
	Vehicle->BuildRoundsStart = Vehicle->BuildRounds;

	Vehicle->IsBuilding = true;
	Vehicle->BuildPath = bBuildPath;

	sendBuildAnswer (*this, true, Vehicle);
	addJob (new cStartBuildJob (Vehicle, oldPosX, oldPosY, Data.isBig));

	if (Vehicle->ServerMoveJob) Vehicle->ServerMoveJob->release();
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_END_BUILDING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_END_BUILDING);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle == NULL) return;

	int iEscapeX = message.popInt16();
	int iEscapeY = message.popInt16();

	if (!Vehicle->IsBuilding || Vehicle->BuildRounds > 0) return;
	if (!Map->possiblePlace (Vehicle, iEscapeX, iEscapeY))
	{
		sideStepStealthUnit (iEscapeX, iEscapeY, Vehicle);
	}

	if (!Map->possiblePlace (Vehicle, iEscapeX, iEscapeY)) return;

	addUnit (Vehicle->PosX, Vehicle->PosY, Vehicle->BuildingTyp.getBuilding(), Vehicle->owner);

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
		Map->moveVehicle (Vehicle, x, y);

		// refresh SeenByPlayerLists
		checkPlayerUnits();
	}

	/*// send new vehicle status and position		//done implicitly by addMoveJob()
	sendUnitData ( Vehicle, Vehicle->owner->Nr );
	for ( unsigned int i = 0; i < Vehicle->seenByPlayerList.Size(); i++ )
	{
		sendUnitData(Vehicle, Vehicle->seenByPlayerList[i]->Nr );
	}*/

	// drive away from the building lot
	addMoveJob (Vehicle->PosX, Vehicle->PosY, iEscapeX, iEscapeY, Vehicle);
	Vehicle->ServerMoveJob->checkMove();	//begin the movment immediately, so no other unit can block the destination field
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_STOP_BUILDING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_STOP_BUILDING);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle == NULL) return;
	if (!Vehicle->IsBuilding) return;
	stopVehicleBuilding (Vehicle);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_TRANSFER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_TRANSFER);

	cVehicle* SrcVehicle = NULL, *DestVehicle = NULL;
	cBuilding* SrcBuilding = NULL, *DestBuilding = NULL;

	if (message.popBool()) SrcVehicle = getVehicleFromID (message.popInt16());
	else SrcBuilding = getBuildingFromID (message.popInt16());

	if (message.popBool()) DestVehicle = getVehicleFromID (message.popInt16());
	else DestBuilding = getBuildingFromID (message.popInt16());

	if ( (!SrcBuilding && !SrcVehicle) || (!DestBuilding && !DestVehicle)) return;

	int iTranfer = message.popInt16();
	int iType = message.popInt16();

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
			sendUnitData (*this, DestBuilding, DestBuilding->owner->Nr);
			sendUnitData (*this, SrcBuilding, SrcBuilding->owner->Nr);
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
			sendSubbaseValues (*this, SrcBuilding->SubBase, SrcBuilding->owner->Nr);
			DestVehicle->data.storageResCur += iTranfer;
			sendUnitData (*this, DestVehicle, DestVehicle->owner->Nr);
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
			sendSubbaseValues (*this, DestBuilding->SubBase, DestBuilding->owner->Nr);
		}
		else
		{
			if (DestVehicle->IsBuilding || DestVehicle->IsClearing) return;
			if (DestVehicle->data.storeResType != iType) return;
			if (DestVehicle->data.storageResCur + iTranfer > DestVehicle->data.storageResMax || DestVehicle->data.storageResCur + iTranfer < 0) return;
			DestVehicle->data.storageResCur += iTranfer;
			sendUnitData (*this, DestVehicle, DestVehicle->owner->Nr);
		}
		SrcVehicle->data.storageResCur -= iTranfer;
		sendUnitData (*this, SrcVehicle, SrcVehicle->owner->Nr);
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_BUILDLIST (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_BUILDLIST);

	int iTurboBuildRounds[3], iTurboBuildCosts[3];
	bool bLand = false, bWater = false;
	int iX, iY;

	cBuilding* Building = getBuildingFromID (message.popInt16());
	if (Building == NULL) return;

	// check whether the building has water and land fields around it
	iX = Building->PosX - 2;
	iY = Building->PosY - 1;
	for (int i = 0; i < 12; i++)
	{
		if (i == 4 ||  i == 6 || i == 8)
		{
			iX -= 3;
			iY += 1;
		}
		else
		{
			if (i == 5 || i == 7) iX += 3;
			else iX++;
		}
		if (iX < 0 || iX >= Map->size || iY < 0 || iY >= Map->size) continue;

		int iOff = iX + iY * Map->size;

		cBuildingIterator bi = Map->fields[iOff].getBuildings();
		while (bi && (bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE || bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)) bi++;

		if (!Map->isWater (iX, iY) || (bi && bi->data.surfacePosition == sUnitData::SURFACE_POS_BASE)) bLand = true;
		else if (Map->isWater (iX, iY) && bi && bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)
		{
			bLand = true;
			bWater = true;
			break;
		}
		else if (Map->isWater (iX, iY)) bWater = true;

	}

	// reset building status
	if (Building->IsWorking)
	{
		Building->ServerStopWork (*this, false);
	}

	int iBuildSpeed = message.popInt16();
	if (iBuildSpeed == 0) Building->MetalPerRound =  1 * Building->data.needsMetal;
	if (iBuildSpeed == 1) Building->MetalPerRound =  4 * Building->data.needsMetal;
	if (iBuildSpeed == 2) Building->MetalPerRound = 12 * Building->data.needsMetal;

	cList<sBuildList*>* NewBuildList = new cList<sBuildList*>;

	int iCount = message.popInt16();
	for (int i = 0; i < iCount; i++)
	{
		sID Type;
		Type.iFirstPart = message.popInt16();
		Type.iSecondPart = message.popInt16();

		// if the first unit hasn't changed copy it to the new buildlist
		if (Building->BuildList->Size() > 0 && i == 0 && Type == (*Building->BuildList) [0]->type)
		{
			//recalculate costs, because build speed could have beed changed
			Building->CalcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, Type.getUnitDataCurrentVersion (Building->owner)->buildCosts, (*Building->BuildList) [0]->metall_remaining);
			sBuildList* BuildListItem = new sBuildList;
			BuildListItem->metall_remaining = iTurboBuildCosts[iBuildSpeed];
			BuildListItem->type = Type;
			NewBuildList->Add (BuildListItem);
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

		NewBuildList->Add (BuildListItem);
	}

	// delete all buildings from the list
	for (size_t i = 0; i != Building->BuildList->Size(); ++i)
	{
		delete (*Building->BuildList) [i];
	}
	delete Building->BuildList;
	Building->BuildList = NewBuildList;

	if (Building->BuildList->Size() > 0)
	{
		if ( (*Building->BuildList) [0]->metall_remaining == -1)
		{
			Building->CalcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, (*Building->BuildList) [0]->type.getUnitDataCurrentVersion (Building->owner)->buildCosts);
			(*Building->BuildList) [0]->metall_remaining = iTurboBuildCosts[iBuildSpeed];
		}

		Building->RepeatBuild = message.popBool();
		Building->BuildSpeed = iBuildSpeed;
		if ( (*Building->BuildList) [0]->metall_remaining > 0)
		{
			Building->ServerStartWork (*this);
		}
	}
	sendBuildList (*this, Building);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_EXIT_FIN_VEH (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_EXIT_FIN_VEH);

	sBuildList* BuildingListItem;

	cBuilding* Building = getBuildingFromID (message.popInt16());
	if (Building == NULL) return;

	int iX = message.popInt16();
	if (iX < 0 || iX > Map->size) return;
	int iY = message.popInt16();
	if (iY < 0 || iY > Map->size) return;

	if (Building->BuildList->Size() <= 0) return;
	BuildingListItem = (*Building->BuildList) [0];
	if (BuildingListItem->metall_remaining > 0) return;

	if (!Building->isNextTo (iX, iY)) return;

	if (!Map->possiblePlaceVehicle (BuildingListItem->type.getVehicle()->data, iX, iY, Building->owner))
	{
		sideStepStealthUnit (iX, iY, BuildingListItem->type.getVehicle()->data, Building->owner);
	}
	if (!Map->possiblePlaceVehicle (BuildingListItem->type.getVehicle()->data, iX, iY, Building->owner)) return;

	addUnit (iX, iY, BuildingListItem->type.getVehicle(), Building->owner, false);

	//start new buildjob
	Building->BuildList->Delete (0);
	if (Building->RepeatBuild)
	{
		BuildingListItem->metall_remaining = -1;
		Building->BuildList->Add (BuildingListItem);
	}
	else
	{
		delete BuildingListItem;
	}

	if (Building->BuildList->Size() > 0)
	{
		BuildingListItem = (*Building->BuildList) [0];
		if (BuildingListItem->metall_remaining == -1)
		{
			int iTurboBuildCosts[3];
			int iTurboBuildRounds[3];
			Building->CalcTurboBuild (iTurboBuildRounds, iTurboBuildCosts, BuildingListItem->type.getUnitDataCurrentVersion (Building->owner)->buildCosts);
			BuildingListItem->metall_remaining = iTurboBuildCosts[Building->BuildSpeed];
		}

		Building->ServerStartWork (*this);
	}
	sendBuildList (*this, Building);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_CHANGE_RESOURCES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_CHANGE_RESOURCES);

	sSubBase* SubBase;

	cBuilding* Building = getBuildingFromID (message.popInt16());
	if (Building == NULL) return;

	unsigned int iMetalProd = message.popInt16();
	unsigned int iOilProd = message.popInt16();
	unsigned int iGoldProd = message.popInt16();

	SubBase = Building->SubBase;

	SubBase->setMetalProd (0);
	SubBase->setOilProd (0);
	SubBase->setGoldProd (0);

	//no need to verify the values. They will be reduced automatically, if nessesary
	SubBase->setMetalProd (iMetalProd);
	SubBase->setGoldProd (iGoldProd);
	SubBase->setOilProd (iOilProd);

	sendSubbaseValues (*this, SubBase, Building->owner->Nr);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_CHANGE_MANUAL_FIRE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_CHANGE_MANUAL_FIRE);

	if (message.popBool())	// vehicle
	{
		cVehicle* Vehicle = getVehicleFromID (message.popInt16());
		if (Vehicle == 0)
			return;
		Vehicle->manualFireActive = !Vehicle->manualFireActive;
		if (Vehicle->manualFireActive && Vehicle->sentryActive)
		{
			Vehicle->owner->deleteSentry (Vehicle);
		}

		sendUnitData (*this, Vehicle, Vehicle->owner->Nr);
		for (unsigned int i = 0; i < Vehicle->seenByPlayerList.Size(); i++)
			sendUnitData (*this, Vehicle, Vehicle->seenByPlayerList[i]->Nr);
	}
	else	// building
	{
		cBuilding* Building = getBuildingFromID (message.popInt16());
		if (Building == 0)
			return;
		Building->manualFireActive = !Building->manualFireActive;
		if (Building->manualFireActive && Building->sentryActive)
		{
			Building->owner->deleteSentry (Building);
		}

		sendUnitData (*this, Building, Building->owner->Nr);
		for (unsigned int i = 0; i < Building->seenByPlayerList.Size(); i++)
			sendUnitData (*this, Building, Building->seenByPlayerList[i]->Nr);
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_CHANGE_SENTRY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_CHANGE_SENTRY);

	if (message.popBool())	// vehicle
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

		sendUnitData (*this, vehicle, vehicle->owner->Nr);
		for (unsigned int i = 0; i < vehicle->seenByPlayerList.Size(); i++)
		{
			sendUnitData (*this, vehicle, vehicle->seenByPlayerList[i]->Nr);
		}
	}
	else	// building
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

		sendUnitData (*this, building, building->owner->Nr);
		for (unsigned int i = 0; i < building->seenByPlayerList.Size(); i++)
		{
			sendUnitData (*this, building, building->seenByPlayerList[i]->Nr);
		}
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_MARK_LOG (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_MARK_LOG);

	cNetMessage* message2 = new cNetMessage (GAME_EV_MARK_LOG);
	message2->pushString (message.popString());
	sendNetMessage (message2);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_SUPPLY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_SUPPLY);

	cVehicle* SrcVehicle = NULL, *DestVehicle = NULL;
	cBuilding* SrcBuilding = NULL, *DestBuilding = NULL;
	int iType, iValue;

	// get the units
	iType = message.popChar();
	if (iType > SUPPLY_TYPE_REPAIR) return;   // unknown type
	if (message.popBool()) SrcVehicle = getVehicleFromID (message.popInt16());
	else SrcBuilding = getBuildingFromID (message.popInt16());
	if (message.popBool()) DestVehicle = getVehicleFromID (message.popInt16());
	else DestBuilding = getBuildingFromID (message.popInt16());

	if ( (!SrcVehicle && !SrcBuilding) || (!DestVehicle && !DestBuilding)) return;

	// check whether the supply is ok and reduce cargo of sourceunit
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
			if (iValue > DestData->hitpointsMax) iValue = DestData->hitpointsMax;
		}
		sendUnitData (*this, SrcVehicle, SrcVehicle->owner->Nr);	// the changed values aren't interesting for enemy players, so only send the new data to the owner
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
			if (iValue > DestVehicle->data.hitpointsMax) iValue = DestVehicle->data.hitpointsMax;
		}
		sendUnitData (*this, SrcBuilding, SrcBuilding->owner->Nr);	// the changed values aren't interesting for enemy players, so only send the new data to the owner
	}

	// repair or reload the destination unit
	if (DestVehicle)
	{
		if (iType == SUPPLY_TYPE_REARM) DestVehicle->data.ammoCur = DestVehicle->data.ammoMax;
		else DestVehicle->data.hitpointsCur = iValue;

		sendSupply (*this, DestVehicle->iID, true, iValue, iType, DestVehicle->owner->Nr);

		//send unitdata to the players who are not the owner
		for (unsigned int i = 0; i < DestVehicle->seenByPlayerList.Size(); i++)
			sendUnitData (*this, DestVehicle, DestVehicle->seenByPlayerList[i]->Nr);
	}
	else
	{
		if (iType == SUPPLY_TYPE_REARM) DestBuilding->data.ammoCur = DestBuilding->data.ammoMax;
		else DestBuilding->data.hitpointsCur = iValue;

		sendSupply (*this, DestBuilding->iID, false, iValue, iType, DestBuilding->owner->Nr);

		//send unitdata to the players who are not the owner
		for (unsigned int i = 0; i < DestBuilding->seenByPlayerList.Size(); i++)
			sendUnitData (*this, DestBuilding, DestBuilding->seenByPlayerList[i]->Nr);
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_VEHICLE_UPGRADE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_VEHICLE_UPGRADE);

	bool upgradeAll = message.popBool();
	unsigned int storageSlot = 0;
	if (!upgradeAll)
		storageSlot = message.popInt16();
	cBuilding* storingBuilding = getBuildingFromID (message.popInt32());
	if (storingBuilding == 0)
		return;

	int totalCosts = 0;
	int availableMetal = storingBuilding->SubBase->Metal;

	cList<cVehicle*> upgradedVehicles;
	for (unsigned int i = 0; i < storingBuilding->storedUnits.Size(); i++)
	{
		if (upgradeAll || i == storageSlot)
		{
			cVehicle* vehicle = storingBuilding->storedUnits[i];
			sUnitData& upgradedVersion = storingBuilding->owner->VehicleData[vehicle->typ->nr];

			if (vehicle->data.version >= upgradedVersion.version)
				continue; // already uptodate
			cUpgradeCalculator& uc = cUpgradeCalculator::instance();
			int upgradeCost = uc.getMaterialCostForUpgrading (upgradedVersion.buildCosts);

			if (availableMetal >= totalCosts + upgradeCost)
			{
				upgradedVehicles.Add (vehicle);
				totalCosts += upgradeCost;
			}
		}
	}

	if (upgradedVehicles.Size() > 0)
	{
		if (totalCosts > 0)
			storingBuilding->SubBase->addMetal (*this, -totalCosts);
		for (unsigned int i = 0; i < upgradedVehicles.Size(); i++)
			upgradedVehicles[i]->upgradeToCurrentVersion();
		sendUpgradeVehicles (*this, upgradedVehicles, totalCosts, storingBuilding->iID, storingBuilding->owner->Nr);
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_START_CLEAR (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_START_CLEAR);

	int id = message.popInt16();
	cVehicle* Vehicle = getVehicleFromID (id);
	if (Vehicle == NULL)
	{
		Log.write ("Server: Can not find vehicle with id " + iToStr (id) + " for clearing", LOG_TYPE_NET_WARNING);
		return;
	}

	int off = Vehicle->PosX + Vehicle->PosY * Map->size;
	cBuilding* building = (*Map) [off].getRubble();

	if (!building)
	{
		sendClearAnswer (*this, 2, Vehicle, 0, -1, Vehicle->owner->Nr);
		return;
	}

	int rubbleoffset = -1;
	if (building->data.isBig)
	{
		rubbleoffset = building->PosX + building->PosY * Map->size;

		sideStepStealthUnit (building->PosX    , building->PosY    , Vehicle, rubbleoffset);
		sideStepStealthUnit (building->PosX + 1, building->PosY    , Vehicle, rubbleoffset);
		sideStepStealthUnit (building->PosX    , building->PosY + 1, Vehicle, rubbleoffset);
		sideStepStealthUnit (building->PosX + 1, building->PosY + 1, Vehicle, rubbleoffset);

		if ( (!Map->possiblePlace (Vehicle, building->PosX    , building->PosY) && rubbleoffset != off) ||
			 (!Map->possiblePlace (Vehicle, building->PosX + 1, building->PosY) && rubbleoffset + 1 != off) ||
			 (!Map->possiblePlace (Vehicle, building->PosX    , building->PosY + 1) && rubbleoffset + Map->size != off) ||
			 (!Map->possiblePlace (Vehicle, building->PosX + 1, building->PosY + 1) && rubbleoffset + Map->size + 1 != off))
		{
			sendClearAnswer (*this, 1, Vehicle, 0, -1, Vehicle->owner->Nr);
			return;
		}

		Vehicle->BuildBigSavedPos = off;
		Map->moveVehicleBig (Vehicle, building->PosX, building->PosY);
	}

	Vehicle->IsClearing = true;
	Vehicle->ClearingRounds = building->data.isBig ? 4 : 1;
	Vehicle->owner->DoScan ();
	addJob (new cStartBuildJob (Vehicle, off % Map->size, off / Map->size, building->data.isBig));

	sendClearAnswer (*this, 0, Vehicle, Vehicle->ClearingRounds, rubbleoffset, Vehicle->owner->Nr);
	for (unsigned int i = 0; i < Vehicle->seenByPlayerList.Size(); i++)
	{
		sendClearAnswer (*this, 0, Vehicle, 0, rubbleoffset, Vehicle->seenByPlayerList[i]->Nr);
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_STOP_CLEAR (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_STOP_CLEAR);

	int id = message.popInt16();
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
			Map->moveVehicle (Vehicle, Vehicle->BuildBigSavedPos % Map->size, Vehicle->BuildBigSavedPos / Map->size);
			Vehicle->owner->DoScan ();
			sendStopClear (*this, Vehicle, Vehicle->BuildBigSavedPos, Vehicle->owner->Nr);
			for (unsigned int i = 0; i < Vehicle->seenByPlayerList.Size(); i++)
			{
				sendStopClear (*this, Vehicle, Vehicle->BuildBigSavedPos, Vehicle->seenByPlayerList[i]->Nr);
			}
		}
		else
		{
			sendStopClear (*this, Vehicle, -1, Vehicle->owner->Nr);
			for (unsigned int i = 0; i < Vehicle->seenByPlayerList.Size(); i++)
			{
				sendStopClear (*this, Vehicle, -1, Vehicle->seenByPlayerList[i]->Nr);
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_ABORT_WAITING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ABORT_WAITING);

	if (DisconnectedPlayerList.Size() < 1) return;
	// only server player can abort the waiting
	cPlayer* LocalPlayer = getPlayerFromNumber (message.iPlayerNr);
	if (LocalPlayer->iSocketNum != MAX_CLIENTS) return;

	// delete disconnected players
	for (unsigned int i = 0; i < DisconnectedPlayerList.Size(); i++)
	{
		deletePlayer (DisconnectedPlayerList[i]);
		DisconnectedPlayerList.Delete (i);
	}
	disableFreezeMode (FREEZE_WAIT_FOR_RECONNECT);
	if (bPlayTurns) sendWaitFor (*this, iActiveTurnPlayerNr);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_IDENTIFICATION (cNetMessage& message)
{
	assert (message.iType == GAME_EV_IDENTIFICATION);

	std::string playerName = message.popString();
	int socketNumber = message.popInt16();
	unsigned int i;
	for (i = 0; i < DisconnectedPlayerList.Size(); i++)
	{
		if (!playerName.compare (DisconnectedPlayerList[i]->name))
		{
			DisconnectedPlayerList[i]->iSocketNum = socketNumber;
			sendReconnectAnswer (*network, true, socketNumber, DisconnectedPlayerList[i]);
			break;
		}
	}
	if (i == DisconnectedPlayerList.Size()) sendReconnectAnswer (*network, false, socketNumber, NULL);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_RECON_SUCESS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_RECON_SUCESS);

	cPlayer* Player = NULL;
	int playerNum = message.popInt16();
	// remove the player from the disconnected list
	for (unsigned int i = 0; i < DisconnectedPlayerList.Size(); i++)
	{
		if (DisconnectedPlayerList[i]->Nr == playerNum)
		{
			Player = DisconnectedPlayerList[i];
			DisconnectedPlayerList.Delete (i);
			break;
		}
	}
	resyncPlayer (Player);

	disableFreezeMode (FREEZE_WAIT_FOR_RECONNECT);
	if (bPlayTurns) sendWaitFor (*this, iActiveTurnPlayerNr);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_LOAD (cNetMessage& message)
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
			//vehicle is removed from enemy clients by cServer::checkPlayerUnits()
			sendStoreVehicle (*this, StoringVehicle->iID, true, StoredVehicle->iID, StoringVehicle->owner->Nr);
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
			//vehicle is removed from enemy clients by cServer::checkPlayerUnits()
			sendStoreVehicle (*this, StoringBuilding->iID, false, StoredVehicle->iID, StoringBuilding->owner->Nr);
		}
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_EXIT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_EXIT);

	cVehicle* StoredVehicle = getVehicleFromID (message.popInt16());
	if (!StoredVehicle) return;

	if (message.popBool())
	{
		cVehicle* StoringVehicle = getVehicleFromID (message.popInt16());
		if (!StoringVehicle) return;

		int x = message.popInt16();
		int y = message.popInt16();
		if (!StoringVehicle->isNextTo (x, y)) return;

		//sidestep stealth units if nessesary
		sideStepStealthUnit (x, y, StoredVehicle);

		if (StoringVehicle->canExitTo (x, y, Map, StoredVehicle->typ))
		{
			StoringVehicle->exitVehicleTo (StoredVehicle, x + y * Map->size, Map);
			//vehicle is added to enemy clients by cServer::checkPlayerUnits()
			sendActivateVehicle (*this, StoringVehicle->iID, true, StoredVehicle->iID, x, y, StoringVehicle->owner->Nr);
			if (StoredVehicle->data.canSurvey)
			{
				sendVehicleResources (*this, StoredVehicle, Map);
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

		int x = message.popInt16();
		int y = message.popInt16();
		if (!StoringBuilding->isNextTo (x, y)) return;

		//sidestep stealth units if nessesary
		sideStepStealthUnit (x, y, StoredVehicle);

		if (StoringBuilding->canExitTo (x, y, Map, StoredVehicle->typ))
		{
			StoringBuilding->exitVehicleTo (StoredVehicle, x + y * Map->size, Map);
			//vehicle is added to enemy clients by cServer::checkPlayerUnits()
			sendActivateVehicle (*this, StoringBuilding->iID, false, StoredVehicle->iID, x, y, StoringBuilding->owner->Nr);
			if (StoredVehicle->data.canSurvey)
			{
				sendVehicleResources (*this, StoredVehicle, Map);
				StoredVehicle->doSurvey (*this);
			}
			StoredVehicle->InSentryRange (*this);
		}
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_REQUEST_RESYNC (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REQUEST_RESYNC);

	cPlayer* player = getPlayerFromNumber (message.popChar());
	if (player) resyncPlayer (player, true);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_BUY_UPGRADES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_BUY_UPGRADES);

	int iPlayerNr = message.popInt16();
	cPlayer* player = getPlayerFromNumber (iPlayerNr);
	if (player == 0)
		return;

	bool updateCredits = false;

	int iCount = message.popInt16(); // get the number of upgrades in this message
	for (int i = 0; i < iCount; i++)
	{
		sID ID;
		ID.iFirstPart = message.popInt16();
		ID.iSecondPart = message.popInt16();

		int newDamage = message.popInt16();
		int newMaxShots = message.popInt16();
		int newRange = message.popInt16();
		int newMaxAmmo = message.popInt16();
		int newArmor = message.popInt16();
		int newMaxHitPoints = message.popInt16();
		int newScan = message.popInt16();
		int newMaxSpeed = 0;
		if (ID.iFirstPart == 0) newMaxSpeed = message.popInt16();

		sUnitData* upgradedUnit = ID.getUnitDataCurrentVersion (player);
		if (upgradedUnit == 0)
			continue; // skip this upgrade, because there is no such unitData

		int costs = getUpgradeCosts (ID, player, ID.iFirstPart == 0, newDamage, newMaxShots, newRange, newMaxAmmo, newArmor, newMaxHitPoints, newScan, newMaxSpeed);
		if (costs <= player->Credits)
		{
			// update the unitData of the player and send an ack-msg for this upgrade to the player
			upgradedUnit->damage = newDamage;
			upgradedUnit->shotsMax = newMaxShots;
			upgradedUnit->range = newRange;
			upgradedUnit->ammoMax = newMaxAmmo;
			upgradedUnit->armor = newArmor;
			upgradedUnit->hitpointsMax = newMaxHitPoints;
			upgradedUnit->scan = newScan;
			if (ID.iFirstPart == 0) upgradedUnit->speedMax = newMaxSpeed;
			upgradedUnit->version++;

			player->Credits -= costs;
			updateCredits = true;

			sendUnitUpgrades (*this, upgradedUnit, iPlayerNr);
		}
	}
	if (updateCredits)
		sendCredits (*this, player->Credits, iPlayerNr);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_BUILDING_UPGRADE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_BUILDING_UPGRADE);

	unsigned int unitID = message.popInt32();
	bool upgradeAll = message.popBool();

	cBuilding* building = getBuildingFromID (unitID);
	cPlayer* player = ( (building != 0) ? building->owner : 0);
	if (player == 0)
		return;

	int availableMetal = building->SubBase->Metal;

	sUnitData& upgradedVersion = player->BuildingData[building->typ->nr];
	if (building->data.version >= upgradedVersion.version)
		return; // already uptodate
	cUpgradeCalculator& uc = cUpgradeCalculator::instance();
	int upgradeCostPerBuilding = uc.getMaterialCostForUpgrading (upgradedVersion.buildCosts);
	int totalCosts = 0;
	cList<cBuilding*> upgradedBuildings;

	// in any case update the selected building
	if (availableMetal >= totalCosts + upgradeCostPerBuilding)
	{
		upgradedBuildings.Add (building);
		totalCosts += upgradeCostPerBuilding;
	}

	if (upgradeAll)
	{
		sSubBase* subBase = building->SubBase;
		for (unsigned int subBaseBuildIdx = 0; subBaseBuildIdx < subBase->buildings.Size(); subBaseBuildIdx++)
		{
			cBuilding* otherBuilding = subBase->buildings[subBaseBuildIdx];
			if (otherBuilding == building)
				continue;
			if (otherBuilding->typ->nr != building->typ->nr)
				continue;
			if (otherBuilding->data.version >= upgradedVersion.version)
				continue;
			upgradedBuildings.Add (otherBuilding);
			totalCosts += upgradeCostPerBuilding;
			if (availableMetal < totalCosts + upgradeCostPerBuilding)
				break; // no more raw material left...
		}
	}

	if (totalCosts > 0)
		building->SubBase->addMetal (*this, -totalCosts);
	if (upgradedBuildings.Size() > 0)
	{
		bool scanNecessary = false;
		bool refreshSentry = false;
		for (unsigned int i = 0; i < upgradedBuildings.Size(); i++)
		{
			if (!scanNecessary && upgradedBuildings[i]->data.scan < upgradedVersion.scan)
				scanNecessary = true; // Scan range was upgraded. So trigger a scan.
			if (upgradedBuildings[i]->sentryActive && upgradedBuildings[i]->data.range < upgradedVersion.range)
				refreshSentry = true;

			upgradedBuildings[i]->upgradeToCurrentVersion();
		}
		sendUpgradeBuildings (*this, upgradedBuildings, totalCosts, player->Nr);
		if (scanNecessary)
			player->DoScan();
		if (refreshSentry)
		{
			player->refreshSentryAir();
			player->refreshSentryGround();
		}
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_RESEARCH_CHANGE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_RESEARCH_CHANGE);

	int iPlayerNr = message.popInt16();
	cPlayer* player = getPlayerFromNumber (iPlayerNr);
	if (player == 0)
		return;
	int newUsedResearch = 0;
	int newResearchSettings[cResearch::kNrResearchAreas];
	for (int area = cResearch::kNrResearchAreas - 1; area >= 0; area--)
	{
		newResearchSettings[area] = message.popInt16();
		newUsedResearch += newResearchSettings[area];
	}
	if (newUsedResearch > player->ResearchCount)
		return; // can't turn on research centers automatically!

	cList<cBuilding*> researchCentersToStop; // needed, if newUsedResearch < player->ResearchCount
	cList<cBuilding*> researchCentersToChangeArea;
	cList<int> newAreasForResearchCenters;

	bool error = false;
	cBuilding* curBuilding = player->BuildingList;
	for (int newArea = 0; newArea < cResearch::kNrResearchAreas; newArea++)
	{
		int centersToAssign = newResearchSettings[newArea];
		while (centersToAssign > 0 && curBuilding != 0)
		{
			if (curBuilding->data.canResearch && curBuilding->IsWorking)
			{
				researchCentersToChangeArea.Add (curBuilding);
				newAreasForResearchCenters.Add (newArea);
				centersToAssign--;
			}
			curBuilding = static_cast<cBuilding*> (curBuilding->next);
		}
		if (curBuilding == 0 && centersToAssign > 0)
		{
			error = true; // not enough active research centers!
			break;
		}
	}
	while (curBuilding != 0)   // shut down unused research centers
	{
		if (curBuilding->data.canResearch && curBuilding->IsWorking)
			researchCentersToStop.Add (curBuilding);
		curBuilding = static_cast<cBuilding*> (curBuilding->next);
	}
	if (error)
		return;

	for (unsigned int i = 0; i < researchCentersToStop.Size(); i++)
		researchCentersToStop[i]->ServerStopWork (*this, false);

	for (unsigned int i = 0; i < researchCentersToChangeArea.Size(); i++)
		researchCentersToChangeArea[i]->researchArea = newAreasForResearchCenters[i];
	player->refreshResearchCentersWorkingOnArea();

	sendResearchSettings (*this, researchCentersToChangeArea, newAreasForResearchCenters, iPlayerNr);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_AUTOMOVE_STATUS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_AUTOMOVE_STATUS);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle) Vehicle->hasAutoMoveJob = message.popBool();
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_COM_ACTION (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_COM_ACTION);

	cVehicle* srcVehicle = getVehicleFromID (message.popInt16());
	if (!srcVehicle) return;

	cVehicle* destVehicle = NULL;
	cBuilding* destBuilding = NULL;
	if (message.popBool()) destVehicle = getVehicleFromID (message.popInt16());
	else destBuilding = getBuildingFromID (message.popInt16());

	bool steal = message.popBool();
	// check whether the commando action is possible
	if (! ( (destVehicle && srcVehicle->canDoCommandoAction (destVehicle->PosX, destVehicle->PosY, Map, steal)) ||
			(destBuilding && srcVehicle->canDoCommandoAction (destBuilding->PosX, destBuilding->PosY, Map, steal)) ||
			(destBuilding && destBuilding->data.isBig && srcVehicle->canDoCommandoAction (destBuilding->PosX, destBuilding->PosY + 1, Map, steal)) ||
			(destBuilding && destBuilding->data.isBig && srcVehicle->canDoCommandoAction (destBuilding->PosX + 1, destBuilding->PosY, Map, steal)) ||
			(destBuilding && destBuilding->data.isBig && srcVehicle->canDoCommandoAction (destBuilding->PosX + 1, destBuilding->PosY + 1, Map, steal)))) return;

	// check whether the action is successfull or not
	int chance = srcVehicle->calcCommandoChance (destVehicle ? (cUnit*) destVehicle : (cUnit*) destBuilding, steal);
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
			// only on disabling units the infiltartor gets exp. As higher his level is as slower he rises onto the next one.
			// every 5 rankings he needs one succesfull disabling more, to get to the next ranking
			srcVehicle->CommandoRank += (float) 1 / ( (int) ( ( (int) srcVehicle->CommandoRank + 5) / 5));

			int strength = srcVehicle->calcCommandoTurns (destVehicle ? (cUnit*) destVehicle : (cUnit*) destBuilding);
			if (destVehicle)
			{
				// stop the vehicle and make it disabled
				destVehicle->turnsDisabled = strength;

				//save speed and number of shots before disabling
				destVehicle->lastSpeed = destVehicle->data.speedCur;
				destVehicle->lastShots = destVehicle->data.shotsCur;

				destVehicle->data.speedCur = 0;
				destVehicle->data.shotsCur = 0;


				if (destVehicle->IsBuilding) stopVehicleBuilding (destVehicle);
				if (destVehicle->ServerMoveJob) destVehicle->ServerMoveJob->release();
				sendUnitData (*this, destVehicle, destVehicle->owner->Nr);
				for (unsigned int i = 0; i < destVehicle->seenByPlayerList.Size(); i++)
				{
					sendUnitData (*this, destVehicle, destVehicle->seenByPlayerList[i]->Nr);
				}
				destVehicle->owner->DoScan();
				checkPlayerUnits();
			}
			else if (destBuilding)
			{
				// stop the vehicle and make it disabled
				destBuilding->turnsDisabled = strength;

				//save number of shots before disabling
				destBuilding->lastShots = destBuilding->data.shotsCur;

				destBuilding->data.shotsCur = 0;
				destBuilding->wasWorking = destBuilding->IsWorking;
				destBuilding->ServerStopWork (*this, true);
				sendDoStopWork (*this, destBuilding);
				sendUnitData (*this, destBuilding, destBuilding->owner->Nr);
				for (unsigned int i = 0; i < destBuilding->seenByPlayerList.Size(); i++)
				{
					sendUnitData (*this, destBuilding, destBuilding->seenByPlayerList[i]->Nr);
				}
				destBuilding->owner->DoScan();
				checkPlayerUnits();
			}
		}
		success = true;
	}
	// disabled units fail to detect infiltrator even if he screws up
	else if ( (destBuilding && destBuilding->turnsDisabled == 0) || (destVehicle && destVehicle->turnsDisabled == 0))
	{
		// detect the infiltrator on failed action and let enemy units fire on him
		//TODO: uncover the infiltrator for all players, or only for the owner of the target unit? --eiko
		for (unsigned int i = 0; i < PlayerList->Size(); i++)
		{
			cPlayer* player = (*PlayerList) [i];
			if (player == srcVehicle->owner) continue;
			if (!player->ScanMap[srcVehicle->PosX + srcVehicle->PosY * Map->size]) continue;

			srcVehicle->setDetectedByPlayer (*this, player);
		}
		checkPlayerUnits();
		srcVehicle->InSentryRange (*this);
	}
	srcVehicle->data.shotsCur--;
	sendUnitData (*this, srcVehicle, srcVehicle->owner->Nr);
	sendCommandoAnswer (*this, success, steal, srcVehicle, srcVehicle->owner->Nr);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_SAVE_HUD_INFO (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SAVE_HUD_INFO);

	int msgSaveingID = message.popInt16();
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

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_SAVE_REPORT_INFO (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SAVE_REPORT_INFO);

	int msgSaveingID = message.popInt16();
	if (msgSaveingID != savingID) return;
	cPlayer* player = getPlayerFromNumber (message.popInt16());
	if (player == NULL) return;

	sSavedReportMessage savedReport;
	savedReport.message = message.popString();
	savedReport.type = (sSavedReportMessage::eReportTypes) message.popInt16();
	savedReport.xPos = message.popInt16();
	savedReport.yPos = message.popInt16();
	savedReport.unitID.iFirstPart = message.popInt16();
	savedReport.unitID.iSecondPart = message.popInt16();
	savedReport.colorNr = message.popInt16();

	player->savedReportsList.Add (savedReport);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_FIN_SEND_SAVE_INFO (cNetMessage& message)
{
	assert (message.iType == GAME_EV_FIN_SEND_SAVE_INFO);

	int msgSaveingID = message.popInt16();
	if (msgSaveingID != savingID) return;
	cPlayer* player = getPlayerFromNumber (message.popInt16());
	if (player == NULL) return;

	cSavegame savegame (savingIndex);
	savegame.writeAdditionalInfo (*player->savedHud, player->savedReportsList, player);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_REQUEST_CASUALTIES_REPORT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REQUEST_CASUALTIES_REPORT);

	sendCasualtiesReport (*this, message.iPlayerNr);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_SELFDESTROY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_SELFDESTROY);

	cBuilding* building = getBuildingFromID (message.popInt16());
	if (!building || building->owner->Nr != message.iPlayerNr) return;

	sendSelfDestroy (*this, building);
	destroyUnit (building);
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_WANT_CHANGE_UNIT_NAME (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WANT_CHANGE_UNIT_NAME);

	int unitID = message.popInt16();
	cUnit* unit = getUnitFromID (unitID);

	if (unit != 0)
	{
		unit->changeName (message.popString());
		for (unsigned int i = 0; i < unit->seenByPlayerList.Size(); i++)
			sendUnitData (*this, unit, i);
		sendUnitData (*this, unit, unit->owner->Nr);
	}
}

//-------------------------------------------------------------------------------------
void cServer::HandleNetMessage_GAME_EV_END_MOVE_ACTION (cNetMessage& message)
{
	assert (message.iType == GAME_EV_END_MOVE_ACTION);

	cVehicle* vehicle = getVehicleFromID (message.popInt32());
	if (!vehicle || !vehicle->ServerMoveJob) return;

	int destID = message.popInt32();
	eEndMoveActionType type = (eEndMoveActionType) message.popChar();
	vehicle->ServerMoveJob->addEndAction (destID, type);
}

//-------------------------------------------------------------------------------------
int cServer::HandleNetMessage (cNetMessage* message)
{
	if (message->iType != NET_GAME_TIME_CLIENT)
		Log.write ("Server: --> " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);

	switch (message->iType)
	{
		case TCP_ACCEPT: HandleNetMessage_TCT_ACCEPT (*message); break;
		case TCP_CLOSE: // Follow
		case GAME_EV_WANT_DISCONNECT:
			HandleNetMessage_TCT_CLOSE_OR_GAME_EV_WANT_DISCONNECT (*message);
			break;
		case GAME_EV_CHAT_CLIENT: HandleNetMessage_GAME_EV_CHAT_CLIENT (*message); break;
		case GAME_EV_WANT_TO_END_TURN: HandleNetMessage_GAME_EV_WANT_TO_END_TURN (*message); break;
		case GAME_EV_WANT_START_WORK: HandleNetMessage_GAME_EV_WANT_START_WORK (*message); break;
		case GAME_EV_WANT_STOP_WORK: HandleNetMessage_GAME_EV_WANT_STOP_WORK (*message); break;
		case GAME_EV_MOVE_JOB_CLIENT: HandleNetMessage_GAME_EV_MOVE_JOB_CLIENT (*message); break;
		case GAME_EV_WANT_STOP_MOVE: HandleNetMessage_GAME_EV_WANT_STOP_MOVE (*message); break;
		case GAME_EV_MOVEJOB_RESUME: HandleNetMessage_GAME_EV_MOVEJOB_RESUME (*message); break;
		case GAME_EV_WANT_ATTACK: HandleNetMessage_GAME_EV_WANT_ATTACK (*message); break;
		case GAME_EV_ATTACKJOB_FINISHED: HandleNetMessage_GAME_EV_ATTACKJOB_FINISHED (*message); break;
		case GAME_EV_MINELAYERSTATUS: HandleNetMessage_GAME_EV_MINELAYERSTATUS (*message); break;
		case GAME_EV_WANT_BUILD: HandleNetMessage_GAME_EV_WANT_BUILD (*message); break;
		case GAME_EV_END_BUILDING: HandleNetMessage_GAME_EV_END_BUILDING (*message); break;
		case GAME_EV_WANT_STOP_BUILDING: HandleNetMessage_GAME_EV_WANT_STOP_BUILDING (*message); break;
		case GAME_EV_WANT_TRANSFER: HandleNetMessage_GAME_EV_WANT_TRANSFER (*message); break;
		case GAME_EV_WANT_BUILDLIST: HandleNetMessage_GAME_EV_WANT_BUILDLIST (*message); break;
		case GAME_EV_WANT_EXIT_FIN_VEH: HandleNetMessage_GAME_EV_WANT_EXIT_FIN_VEH (*message); break;
		case GAME_EV_CHANGE_RESOURCES: HandleNetMessage_GAME_EV_CHANGE_RESOURCES (*message); break;
		case GAME_EV_WANT_CHANGE_MANUAL_FIRE: HandleNetMessage_GAME_EV_WANT_CHANGE_MANUAL_FIRE (*message); break;
		case GAME_EV_WANT_CHANGE_SENTRY: HandleNetMessage_GAME_EV_WANT_CHANGE_SENTRY (*message); break;
		case GAME_EV_WANT_MARK_LOG: HandleNetMessage_GAME_EV_WANT_MARK_LOG (*message); break;
		case GAME_EV_WANT_SUPPLY: HandleNetMessage_GAME_EV_WANT_SUPPLY (*message); break;
		case GAME_EV_WANT_VEHICLE_UPGRADE: HandleNetMessage_GAME_EV_WANT_VEHICLE_UPGRADE (*message); break;
		case GAME_EV_WANT_START_CLEAR: HandleNetMessage_GAME_EV_WANT_START_CLEAR (*message); break;
		case GAME_EV_WANT_STOP_CLEAR: HandleNetMessage_GAME_EV_WANT_STOP_CLEAR (*message); break;
		case GAME_EV_ABORT_WAITING: HandleNetMessage_GAME_EV_ABORT_WAITING (*message); break;
		case GAME_EV_IDENTIFICATION: HandleNetMessage_GAME_EV_IDENTIFICATION (*message); break;
		case GAME_EV_RECON_SUCESS: HandleNetMessage_GAME_EV_RECON_SUCESS (*message); break;
		case GAME_EV_WANT_LOAD: HandleNetMessage_GAME_EV_WANT_LOAD (*message); break;
		case GAME_EV_WANT_EXIT: HandleNetMessage_GAME_EV_WANT_EXIT (*message); break;
		case GAME_EV_REQUEST_RESYNC: HandleNetMessage_GAME_EV_REQUEST_RESYNC (*message); break;
		case GAME_EV_WANT_BUY_UPGRADES: HandleNetMessage_GAME_EV_WANT_BUY_UPGRADES (*message); break;
		case GAME_EV_WANT_BUILDING_UPGRADE: HandleNetMessage_GAME_EV_WANT_BUILDING_UPGRADE (*message); break;
		case GAME_EV_WANT_RESEARCH_CHANGE: HandleNetMessage_GAME_EV_WANT_RESEARCH_CHANGE (*message); break;
		case GAME_EV_AUTOMOVE_STATUS: HandleNetMessage_GAME_EV_AUTOMOVE_STATUS (*message); break;
		case GAME_EV_WANT_COM_ACTION: HandleNetMessage_GAME_EV_WANT_COM_ACTION (*message); break;
		case GAME_EV_SAVE_HUD_INFO: HandleNetMessage_GAME_EV_SAVE_HUD_INFO (*message); break;
		case GAME_EV_SAVE_REPORT_INFO: HandleNetMessage_GAME_EV_SAVE_REPORT_INFO (*message); break;
		case GAME_EV_FIN_SEND_SAVE_INFO: HandleNetMessage_GAME_EV_FIN_SEND_SAVE_INFO (*message); break;
		case GAME_EV_REQUEST_CASUALTIES_REPORT: HandleNetMessage_GAME_EV_REQUEST_CASUALTIES_REPORT (*message); break;
		case GAME_EV_WANT_SELFDESTROY: HandleNetMessage_GAME_EV_WANT_SELFDESTROY (*message); break;
		case GAME_EV_WANT_CHANGE_UNIT_NAME: HandleNetMessage_GAME_EV_WANT_CHANGE_UNIT_NAME (*message); break;
		case GAME_EV_END_MOVE_ACTION: HandleNetMessage_GAME_EV_END_MOVE_ACTION (*message); break;
		case NET_GAME_TIME_CLIENT: gameTimer.handleSyncMessage (*message); break;
		default:
			Log.write ("Server: Can not handle message, type " + message->getTypeAsString(), cLog::eLOG_TYPE_NET_ERROR);
	}

	CHECK_MEMORY;
	return 0;
}

//-------------------------------------------------------------------------------------
int cServer::getUpgradeCosts (sID& ID, cPlayer* player, bool bVehicle,
							  int newDamage, int newMaxShots, int newRange,
							  int newMaxAmmo, int newArmor, int newMaxHitPoints,
							  int newScan, int newMaxSpeed)
{
	sUnitData* currentVersion = ID.getUnitDataCurrentVersion (player);
	sUnitData* startVersion = ID.getUnitDataOriginalVersion (player);
	if (currentVersion == 0 || startVersion == 0)
		return 1000000; // error (unbelievably high cost...)

	int cost = 0;
	cUpgradeCalculator& uc = cUpgradeCalculator::instance();
	if (newDamage > currentVersion->damage)
	{
		int costForUpgrade = uc.getCostForUpgrade (startVersion->damage, currentVersion->damage, newDamage, cUpgradeCalculator::kAttack, player->researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newMaxShots > currentVersion->shotsMax)
	{
		int costForUpgrade = uc.getCostForUpgrade (startVersion->shotsMax, currentVersion->shotsMax, newMaxShots, cUpgradeCalculator::kShots, player->researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newRange > currentVersion->range)
	{
		int costForUpgrade = uc.getCostForUpgrade (startVersion->range, currentVersion->range, newRange, cUpgradeCalculator::kRange, player->researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newMaxAmmo > currentVersion->ammoMax)
	{
		int costForUpgrade = uc.getCostForUpgrade (startVersion->ammoMax, currentVersion->ammoMax, newMaxAmmo, cUpgradeCalculator::kAmmo, player->researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newArmor > currentVersion->armor)
	{
		int costForUpgrade = uc.getCostForUpgrade (startVersion->armor, currentVersion->armor, newArmor, cUpgradeCalculator::kArmor, player->researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newMaxHitPoints > currentVersion->hitpointsMax)
	{
		int costForUpgrade = uc.getCostForUpgrade (startVersion->hitpointsMax, currentVersion->hitpointsMax, newMaxHitPoints, cUpgradeCalculator::kHitpoints, player->researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (newScan > currentVersion->scan)
	{
		int costForUpgrade = uc.getCostForUpgrade (startVersion->scan, currentVersion->scan, newScan, cUpgradeCalculator::kScan, player->researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	if (bVehicle && newMaxSpeed > currentVersion->speedMax)
	{
		int costForUpgrade = uc.getCostForUpgrade (startVersion->speedMax / 4, currentVersion->speedMax / 4, newMaxSpeed / 4, cUpgradeCalculator::kSpeed, player->researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}

	return cost;
}

//-------------------------------------------------------------------------------------
cVehicle* cServer::landVehicle (int iX, int iY, int iWidth, int iHeight, sVehicle* Vehicle, cPlayer* Player)
{
	cVehicle* VehcilePtr = NULL;
	for (int i = -iHeight / 2; i < iHeight / 2; i++)
	{
		for (int k = -iWidth / 2; k < iWidth / 2; k++)
		{

			if (!Map->possiblePlaceVehicle (Vehicle->data, iX + k, iY + i, Player)) continue;

			addUnit (iX + k, iY + i, Vehicle, Player, true);
			VehcilePtr = (*Map) [iX + k + (iY + i) * Map->size].getVehicles();
			return VehcilePtr;
		}
	}
	return VehcilePtr;
}

//-------------------------------------------------------------------------------------
void cServer::makeLanding (int iX, int iY, cPlayer* Player, cList<sLandingUnit>* List, bool bFixed)
{
	cVehicle* Vehicle;
	int iWidth, iHeight;

	// Find place for mine if bridgehead is fixed
	if (bFixed)
	{
		if (Map->possiblePlaceBuilding (*specialIDSmallGen.getUnitDataOriginalVersion(), iX - 1    , iY - 1 + 1) &&
			Map->possiblePlaceBuilding (*specialIDMine.getUnitDataOriginalVersion(), iX - 1 + 1, iY - 1) &&
			Map->possiblePlaceBuilding (*specialIDMine.getUnitDataOriginalVersion(), iX - 1 + 2, iY - 1) &&
			Map->possiblePlaceBuilding (*specialIDMine.getUnitDataOriginalVersion(), iX - 1 + 2, iY - 1 + 1) &&
			Map->possiblePlaceBuilding (*specialIDMine.getUnitDataOriginalVersion(), iX - 1 + 1, iY - 1 + 1))
		{
			// place buildings:
			addUnit (iX - 1,     iY - 1 + 1, &UnitsData.getBuilding (specialIDSmallGen.getBuilding()->nr, Player->getClan()), Player, true);
			addUnit (iX - 1 + 1, iY - 1,     &UnitsData.getBuilding (specialIDMine.getBuilding()->nr, Player->getClan()), Player, true);
		}
		else
		{
			Log.write ("couldn't place player start mine: " + Player->name, cLog::eLOG_TYPE_ERROR);
		}
	}

	iWidth = 2;
	iHeight = 2;
	for (unsigned int i = 0; i < List->Size(); i++)
	{
		sLandingUnit& Landing = (*List) [i];
		Vehicle = landVehicle (iX, iY, iWidth, iHeight, Landing.unitID.getVehicle(), Player);
		while (!Vehicle)
		{
			iWidth += 2;
			iHeight += 2;
			Vehicle = landVehicle (iX, iY, iWidth, iHeight, Landing.unitID.getVehicle(), Player);
		}
		if (Landing.cargo && Vehicle)
		{
			Vehicle->data.storageResCur = Landing.cargo;
			sendUnitData (*this, Vehicle, Vehicle->owner->Nr);
		}
	}
}

//-------------------------------------------------------------------------------------
void cServer::correctLandingPos (int& iX, int& iY)
{
	int iWidth = 2;
	int iHeight = 2;
	int margin = 1;
	while (true)
	{
		for (int i = -iHeight / 2; i < iHeight / 2; i++)
		{
			for (int k = -iWidth / 2; k < iWidth / 2; k++)
			{
				if (Map->possiblePlaceBuildingWithMargin (*specialIDSmallGen.getUnitDataOriginalVersion(), iX + k    , iY + i + 1, margin) &&
					Map->possiblePlaceBuildingWithMargin (*specialIDMine.getUnitDataOriginalVersion(), iX + k + 1, iY + i    , margin) &&
					Map->possiblePlaceBuildingWithMargin (*specialIDMine.getUnitDataOriginalVersion(), iX + k + 2, iY + i    , margin) &&
					Map->possiblePlaceBuildingWithMargin (*specialIDMine.getUnitDataOriginalVersion(), iX + k + 2, iY + i + 1, margin) &&
					Map->possiblePlaceBuildingWithMargin (*specialIDMine.getUnitDataOriginalVersion(), iX + k + 1, iY + i + 1, margin))
				{
					iX += k + 1;
					iY += i + 1;
					return;
				}
			}
		}
		iWidth += 2;
		iHeight += 2;
	}
}

//-------------------------------------------------------------------------------------
cVehicle* cServer::addUnit (int iPosX, int iPosY, const sVehicle* Vehicle, cPlayer* Player, bool bInit, bool bAddToMap, unsigned int ID)
{
	cVehicle* AddedVehicle;
	// generate the vehicle:
	AddedVehicle = Player->AddVehicle (iPosX, iPosY, Vehicle, ID ? ID : iNextUnitID);
	iNextUnitID++;

	// place the vehicle:
	if (bAddToMap) Map->addVehicle (AddedVehicle, iPosX, iPosY);

	// scan with surveyor:
	if (AddedVehicle->data.canSurvey)
	{
		sendVehicleResources (*this, AddedVehicle, Map);
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

	sendAddUnit (*this, iPosX, iPosY, AddedVehicle->iID, true, Vehicle->data.ID, Player->Nr, bInit);

	//detection must be done, after the vehicle has been sent to clients
	AddedVehicle->makeDetection (*this);
	return AddedVehicle;
}

//-------------------------------------------------------------------------------------
cBuilding* cServer::addUnit (int iPosX, int iPosY, const sBuilding* Building, cPlayer* Player, bool bInit, unsigned int ID)
{
	cBuilding* AddedBuilding;
	// generate the building:
	AddedBuilding = Player->addBuilding (iPosX, iPosY, Building, ID ? ID : iNextUnitID);
	if (AddedBuilding->data.canMineMaxRes > 0) AddedBuilding->CheckRessourceProd(*this);
	if (AddedBuilding->sentryActive) Player->addSentry (AddedBuilding);

	iNextUnitID++;

	int iOff = iPosX + Map->size * iPosY;

	cBuilding* buildingToBeDeleted =  Map->fields[iOff].getTopBuilding();

	Map->addBuilding (AddedBuilding, iPosX, iPosY);

	sendAddUnit (*this, iPosX, iPosY, AddedBuilding->iID, false, Building->data.ID, Player->Nr, bInit);

	// integrate the building to the base:
	Player->base.addBuilding (AddedBuilding, true);

	//if this is a top building, delete connectors, mines and roads
	if (AddedBuilding->data.surfacePosition == sUnitData::SURFACE_POS_GROUND)
	{
		if (AddedBuilding->data.isBig)
		{
			cBuildingIterator building = Map->fields[iOff].getBuildings();
			while (building)
			{
				if (building->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit (building);
					building--;
				}
				building++;
			}
			iOff++;
			building = Map->fields[iOff].getBuildings();
			while (building)
			{
				if (building->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit (building);
					building--;
				}
				building++;
			}
			iOff += Map->size;
			building = Map->fields[iOff].getBuildings();
			while (building)
			{
				if (building->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit (building);
					building--;
				}
				building++;
			}
			iOff--;
			building = Map->fields[iOff].getBuildings();
			while (building)
			{
				if (building->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit (building);
					building--;
				}
				building++;
			}
		}
		else
		{
			deleteUnit (buildingToBeDeleted);
			cBuildingIterator building = Map->fields[iOff].getBuildings();
			while (building)
			{
				if (building->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_YESNREMOVE)
				{
					deleteUnit (building);
					building--;
				}
				building++;
			}
		}
	}

	if (AddedBuilding->data.canMineMaxRes > 0)
	{
		sendProduceValues (*this, AddedBuilding);
		AddedBuilding->ServerStartWork (*this);
	}
	AddedBuilding->makeDetection (*this);
	return AddedBuilding;
}

//-------------------------------------------------------------------------------------
void cServer::deleteUnit (cUnit* unit, bool notifyClient)
{
	if (unit == 0)
		return;

	if (unit->isBuilding() && unit->owner == 0)
	{
		deleteRubble (static_cast<cBuilding*> (unit));
		return;
	}

	if (unit->owner && casualtiesTracker && ( (unit->isBuilding() && unit->data.buildCosts <= 2) == false))
		casualtiesTracker->logCasualty (unit->data.ID, unit->owner->Nr);

	if (unit->prev)
	{
		unit->prev->next = unit->next;
		if (unit->next)
			unit->next->prev = unit->prev;
	}
	else
	{
		if (unit->isVehicle())
			unit->owner->VehicleList = static_cast<cVehicle*> (unit->next);
		else
			unit->owner->BuildingList = static_cast<cBuilding*> (unit->next);
		if (unit->next)
			unit->next->prev = 0;
	}

	//detach from attack job
	if (unit->attacking)
	{
		for (unsigned int i = 0; i < AJobs.Size(); i++)
		{
			if (AJobs[i]->unit == unit)
				AJobs[i]->unit = 0;
		}
	}

	//detach from move job
	if (!unit->isBuilding())
	{
		cVehicle* vehicle = static_cast<cVehicle*> (unit);
		if (vehicle->ServerMoveJob)
		{
			vehicle->ServerMoveJob->Vehicle = NULL;
		}
	}

	//remove from sentry list
	unit->owner->deleteSentry (unit);

	// lose eco points
	if (unit->isBuilding() && static_cast<cBuilding*> (unit)->points != 0)
	{
		unit->owner->setScore (unit->owner->getScore (iTurn) - static_cast<cBuilding*> (unit)->points, iTurn);
		sendScore (*this, unit->owner, iTurn);
	}

	if (unit->isBuilding())
		Map->deleteBuilding (static_cast<cBuilding*> (unit));
	else
		Map->deleteVehicle (static_cast<cVehicle*> (unit));

	if (notifyClient)
		sendDeleteUnit (*this, unit, -1);

	if (unit->isBuilding() && static_cast<cBuilding*> (unit)->SubBase != 0)
		unit->owner->base.deleteBuilding (static_cast<cBuilding*> (unit), true);

	cPlayer* owner = unit->owner;
	delete unit;

	if (owner != 0)
		owner->DoScan();
}

//-------------------------------------------------------------------------------------
void cServer::checkPlayerUnits()
{
	cPlayer* UnitPlayer;	// The player whos unit is it
	cPlayer* MapPlayer;		// The player who is scaning for new units

	for (unsigned int iUnitPlayerNum = 0; iUnitPlayerNum < PlayerList->Size(); iUnitPlayerNum++)
	{
		UnitPlayer = (*PlayerList) [iUnitPlayerNum];
		cVehicle* NextVehicle = UnitPlayer->VehicleList;
		while (NextVehicle != NULL)
		{
			for (unsigned int iMapPlayerNum = 0; iMapPlayerNum < PlayerList->Size(); iMapPlayerNum++)
			{
				if (iMapPlayerNum == iUnitPlayerNum) continue;
				MapPlayer = (*PlayerList) [iMapPlayerNum];
				int iOff = NextVehicle->PosX + NextVehicle->PosY * Map->size;

				bool stealthUnit = NextVehicle->data.isStealthOn != TERRAIN_NONE;
				if (MapPlayer->ScanMap[iOff] == 1 && (!stealthUnit || NextVehicle->isDetectedByPlayer (MapPlayer) || (MapPlayer->isDefeated && openMapDefeat)) && !NextVehicle->Loaded)
				{
					unsigned int i;
					for (i = 0; i < NextVehicle->seenByPlayerList.Size(); i++)
					{
						if (NextVehicle->seenByPlayerList[i] == MapPlayer) break;
					}
					if (i == NextVehicle->seenByPlayerList.Size())
					{
						NextVehicle->seenByPlayerList.Add (MapPlayer);
						sendAddEnemyUnit (*this, NextVehicle, MapPlayer->Nr);
						sendUnitData (*this, NextVehicle, MapPlayer->Nr);
						if (NextVehicle->ServerMoveJob)
						{
							sendMoveJobServer (*this, NextVehicle->ServerMoveJob, MapPlayer->Nr);
							if (ActiveMJobs.Contains(NextVehicle->ServerMoveJob) && !NextVehicle->ServerMoveJob->bFinished && !NextVehicle->ServerMoveJob->bEndForNow)
							{
								Log.write(" Server: sending extra MJOB_OK for unit ID " + iToStr(NextVehicle->iID) + " to client " + iToStr(MapPlayer->Nr), cLog::eLOG_TYPE_NET_DEBUG);
								cNetMessage* message = new cNetMessage (GAME_EV_NEXT_MOVE);
								message->pushChar (MJOB_OK);
								message->pushInt16 (NextVehicle->iID);
								sendNetMessage (message, MapPlayer->Nr);
							}
						}
					}
				}
				else
				{
					unsigned int i;
					for (i = 0; i < NextVehicle->seenByPlayerList.Size(); i++)
					{
						if (NextVehicle->seenByPlayerList[i] == MapPlayer)
						{
							NextVehicle->seenByPlayerList.Delete (i);
							sendDeleteUnit (*this, NextVehicle, MapPlayer->Nr);
							break;
						}
					}
				}
			}
			NextVehicle = static_cast<cVehicle*> (NextVehicle->next);
		}
		cBuilding* NextBuilding = UnitPlayer->BuildingList;
		while (NextBuilding != NULL)
		{
			for (unsigned int iMapPlayerNum = 0; iMapPlayerNum < PlayerList->Size(); iMapPlayerNum++)
			{
				if (iMapPlayerNum == iUnitPlayerNum) continue;
				MapPlayer = (*PlayerList) [iMapPlayerNum];
				int iOff = NextBuilding->PosX + NextBuilding->PosY * Map->size;
				bool stealthUnit = NextBuilding->data.isStealthOn != TERRAIN_NONE;

				if (MapPlayer->ScanMap[iOff] == 1  && (!stealthUnit || NextBuilding->isDetectedByPlayer (MapPlayer) || (MapPlayer->isDefeated && openMapDefeat)))
				{
					unsigned int i;
					for (i = 0; i < NextBuilding->seenByPlayerList.Size(); i++)
					{
						if (NextBuilding->seenByPlayerList[i] == MapPlayer) break;
					}
					if (i == NextBuilding->seenByPlayerList.Size())
					{
						NextBuilding->seenByPlayerList.Add (MapPlayer);
						sendAddEnemyUnit (*this, NextBuilding, MapPlayer->Nr);
						sendUnitData (*this, NextBuilding, MapPlayer->Nr);
					}
				}
				else
				{
					unsigned int i;
					for (i = 0; i < NextBuilding->seenByPlayerList.Size(); i++)
					{
						if (NextBuilding->seenByPlayerList[i] == MapPlayer)
						{
							NextBuilding->seenByPlayerList.Delete (i);
							sendDeleteUnit (*this, NextBuilding, MapPlayer->Nr);

							break;
						}
					}
				}
			}
			NextBuilding = static_cast<cBuilding*> (NextBuilding->next);
		}
	}

	//check the neutral objects
	cBuilding* building = neutralBuildings;
	while (building != NULL)
	{
		for (unsigned int iMapPlayerNum = 0; iMapPlayerNum < PlayerList->Size(); iMapPlayerNum++)
		{
			MapPlayer = (*PlayerList) [iMapPlayerNum];
			int iOff = building->PosX + building->PosY * Map->size;

			if (MapPlayer->ScanMap[iOff] == 1)
			{
				unsigned int i;
				for (i = 0; i < building->seenByPlayerList.Size(); i++)
				{
					if (building->seenByPlayerList[i] == MapPlayer) break;
				}
				if (i == building->seenByPlayerList.Size())
				{
					building->seenByPlayerList.Add (MapPlayer);
					sendAddRubble (*this, building, MapPlayer->Nr);
				}
			}
			else
			{
				unsigned int i;
				for (i = 0; i < building->seenByPlayerList.Size(); i++)
				{
					if (building->seenByPlayerList[i] == MapPlayer)
					{
						building->seenByPlayerList.Delete (i);
						sendDeleteUnit (*this, building, MapPlayer->Nr);
						break;
					}
				}
			}
		}
		building = static_cast<cBuilding*> (building->next);
	}
}

//-------------------------------------------------------------------------------------
bool cServer::isPlayerDisconnected (cPlayer* player) const
{
	if (player->iSocketNum == MAX_CLIENTS)
		return false;

	if (network)
		return !network->isConnected (player->iSocketNum);

	return true;
}

//-------------------------------------------------------------------------------------
void cServer::markAllPlayersAsDisconnected()
{
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		cPlayer* player = (*PlayerList) [i];
		if (DisconnectedPlayerList.Contains (player) == false)
			DisconnectedPlayerList.Add (player);
		memset (player->ScanMap, 0, Map->size * Map->size);
	}
}

//-------------------------------------------------------------------------------------
cPlayer* cServer::getPlayerFromNumber (int iNum)
{
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		cPlayer* const p = (*PlayerList) [i];
		if (p->Nr == iNum) return p;
	}
	return NULL;
}

//-------------------------------------------------------------------------------------
cPlayer* cServer::getPlayerFromString (const std::string& playerID)
{
	//first try to find player by number
	int playerNr = atoi (playerID.c_str());
	if (playerNr != 0 || playerID[0] == '0')
	{
		return getPlayerFromNumber (playerNr);
	}

	//try to find plyer by name
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		if ( (*PlayerList) [i]->name.compare (playerID) == 0) return (*PlayerList) [i];
	}

	return NULL;

}

//-------------------------------------------------------------------------------------
void cServer::handleEnd (int iPlayerNum)
{
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
		bool bWaitForPlayer = (gameType == GAME_TYPE_TCPIP && bPlayTurns);
		if (checkEndActions (iPlayerNum))
		{
			iWantPlayerEndNum = iPlayerNum;
			return;
		}
		iActiveTurnPlayerNr++;
		if (iActiveTurnPlayerNr >= (int) PlayerList->Size())
		{
			iActiveTurnPlayerNr = 0;
			makeTurnEnd();
			iTurn++;

			if (gameType == GAME_TYPE_HOTSEAT)
			{
				sendMakeTurnEnd (*this, true, bWaitForPlayer, (*PlayerList) [iActiveTurnPlayerNr]->Nr, iPlayerNum);
			}
			else
			{
				for (unsigned int i = 0; i < PlayerList->Size(); i++)
				{
					sendMakeTurnEnd (*this, true, bWaitForPlayer, (*PlayerList) [iActiveTurnPlayerNr]->Nr, (*PlayerList) [i]->Nr);
				}
			}
		}
		else
		{
			if (gameType == GAME_TYPE_HOTSEAT)
			{
				sendMakeTurnEnd (*this, false, bWaitForPlayer, (*PlayerList) [iActiveTurnPlayerNr]->Nr, iPlayerNum);
				// TODO: in hotseat: maybe send information to client about the next player
			}
			else
			{
				for (unsigned int i = 0; i < PlayerList->Size(); i++)
				{
					sendMakeTurnEnd (*this, false, bWaitForPlayer, (*PlayerList) [iActiveTurnPlayerNr]->Nr, i);
				}
			}
		}
		// send report to next player
		sendTurnReport (*this, (*PlayerList) [iActiveTurnPlayerNr]);
	}
	else // it's a simultanous TCP/IP multiplayer game
	{
		// defeated player are ignored when they hit the end button
		if (getPlayerFromNumber (iPlayerNum)->isDefeated) return;

		// check whether this player has already finished his turn
		for (unsigned int i = 0; i < PlayerEndList.Size(); i++)
		{
			if (PlayerEndList[i]->Nr == iPlayerNum) return;
		}
		PlayerEndList.Add (getPlayerFromNumber (iPlayerNum));
		bool firstTimeEnded = PlayerEndList.Size() == 1;

		// make sure that all defeated players are added to the endlist
		for (unsigned int i = 0; i < PlayerList->Size(); i++)
		{
			if ( (*PlayerList) [i]->isDefeated)
			{
				bool isAdded = false;
				for (unsigned int j = 0; j < PlayerEndList.Size(); j++) if (PlayerEndList[j] == (*PlayerList) [i]) isAdded = true;
				if (!isAdded) PlayerEndList.Add ( (*PlayerList) [i]);
			}
		}

		if (iWantPlayerEndNum == -1)
		{
			// When playing with dedicated server where a player is not connected, play without a deadline,
			// but wait till all players pressed "End".
			if (firstTimeEnded && (DEDICATED_SERVER == false || DisconnectedPlayerList.Size() == 0))
			{
				sendTurnFinished (*this, iPlayerNum, iTurnDeadline);
				iDeadlineStartTime = SDL_GetTicks();
			}
			else
			{
				sendTurnFinished (*this, iPlayerNum, -1);
			}
		}

		if (PlayerEndList.Size() >= PlayerList->Size())
		{
			iDeadlineStartTime = 0;
			if (checkEndActions (-1))
			{
				iWantPlayerEndNum = iPlayerNum;
				return;
			}

			PlayerEndList.Clear();

			iTurn++;
			makeTurnEnd();

		}
	}
}

//-------------------------------------------------------------------------------------
void cServer::handleWantEnd()
{
	if (!gameTimer.timer50ms) return;

	//wait until all clients have reported a gametime that is after the turn end.
	//that means they have finished prosessing all the turn end messages, and we can start the new turn sumultaiously on all clients
	if (freezeModes.waitForTurnEnd && !executingRemainingMovements)
	{
		for (unsigned int i = 0; i < PlayerList->Size(); i++)
		{
			cPlayer* player = (*PlayerList) [i];
			if (!isPlayerDisconnected (player) && gameTimer.getReceivedTime (i) <= lastTurnEnd)
				return;
		}

		// send reports to all players
		for (unsigned int i = 0; i < PlayerList->Size(); i++)
		{
			sendMakeTurnEnd (*this, true, false, -1, i);
		}
		for (unsigned int i = 0; i < PlayerList->Size(); i++)
		{
			sendTurnReport (*this, (*PlayerList) [i]);
		}

		//begin the new turn
		disableFreezeMode (FREEZE_WAIT_FOR_TURNEND);
	}



	if (iWantPlayerEndNum != -1 && iWantPlayerEndNum != -2)
	{
		for (unsigned int i = 0; i < PlayerEndList.Size(); i++)
		{
			if (iWantPlayerEndNum == PlayerEndList[i]->Nr) PlayerEndList.Delete (i);
		}
		handleEnd (iWantPlayerEndNum);
	}
}

//-------------------------------------------------------------------------------------
bool cServer::checkEndActions (int iPlayer)
{
	enableFreezeMode (FREEZE_WAIT_FOR_TURNEND);

	std::string sMessage;
	if (ActiveMJobs.Size() > 0)
	{
		sMessage = "Text~Comp~Turn_Wait";
	}
	else
	{
		for (unsigned int i = 0; i < PlayerList->Size(); i++)
		{
			cVehicle* NextVehicle;
			NextVehicle = (*PlayerList) [i]->VehicleList;
			while (NextVehicle != NULL)
			{
				if (NextVehicle->ServerMoveJob && NextVehicle->data.speedCur > 0 && !NextVehicle->moving)
				{
					// restart movejob
					NextVehicle->ServerMoveJob->resume();
					sMessage = "Text~Comp~Turn_Automove";
				}
				NextVehicle = static_cast<cVehicle*> (NextVehicle->next);
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
				for (unsigned int i = 0; i < PlayerList->Size(); i++)
				{
					sendChatMessageToClient (*this, sMessage, SERVER_INFO_MESSAGE, (*PlayerList) [i]->Nr);
				}
			}
		}
		executingRemainingMovements = true;
		return true;
	}
	executingRemainingMovements = false;
	return false;
}

//-------------------------------------------------------------------------------------
void cServer::makeTurnEnd()
{
	enableFreezeMode (FREEZE_WAIT_FOR_TURNEND);
	lastTurnEnd = gameTimer.gameTime;

	// reload all buildings
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		cBuilding* Building;
		cPlayer* Player;
		Player = (*PlayerList) [i];

		Building = Player->BuildingList;
		while (Building)
		{
			bool forceSendUnitData = false;
			if (Building->turnsDisabled > 0)
			{
				Building->turnsDisabled--;
				if (Building->turnsDisabled == 0 && Building->wasWorking)
				{
					Building->ServerStartWork (*this);
					Building->wasWorking = false;
				}
				forceSendUnitData = true;
			}
			if ( (Building->data.canAttack && Building->refreshData()) || forceSendUnitData)
			{
				for (unsigned int k = 0; k < Building->seenByPlayerList.Size(); k++)
				{
					sendUnitData (*this, Building, Building->seenByPlayerList[k]->Nr);
				}
				sendUnitData (*this, Building, Building->owner->Nr);
			}
			Building = static_cast<cBuilding*> (Building->next);
		}
	}

	// reload all vehicles
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		cVehicle* Vehicle;
		cPlayer* Player;
		Player = (*PlayerList) [i];

		Vehicle = Player->VehicleList;
		while (Vehicle)
		{
			bool forceSendUnitData = false;
			if (Vehicle->turnsDisabled > 0)
			{
				Vehicle->turnsDisabled--;
				forceSendUnitData = true;
			}
			if (Vehicle->refreshData() || forceSendUnitData)
			{
				for (unsigned int k = 0; k < Vehicle->seenByPlayerList.Size(); k++)
				{
					sendUnitData (*this, Vehicle, Vehicle->seenByPlayerList[k]->Nr);
				}
				sendUnitData (*this, Vehicle, Vehicle->owner->Nr);
			}
			if (Vehicle->ServerMoveJob) Vehicle->ServerMoveJob->bEndForNow = false;
			Vehicle = static_cast<cVehicle*> (Vehicle->next);
		}
	}

	//hide stealth units
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		cPlayer* player = (*PlayerList) [i];
		player->DoScan();							//make sure the detection maps are up to date
		cVehicle* vehicle = player->VehicleList;
		while (vehicle)
		{
			vehicle->clearDetectedInThisTurnPlayerList();
			vehicle->makeDetection (*this);
			vehicle = static_cast<cVehicle*> (vehicle->next);
		}
	}

	// produce resources
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		(*PlayerList) [i]->base.handleTurnend (*this);
	}

	// do research:
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
		(*PlayerList) [i]->doResearch (*this);

	// eco-spheres:
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		(*PlayerList) [i]->accumulateScore (*this);
	}

	// Gun'em down:
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		cVehicle* Vehicle;
		cPlayer* Player;
		Player = (*PlayerList) [i];

		Vehicle = Player->VehicleList;
		while (Vehicle)
		{
			Vehicle->InSentryRange (*this);
			Vehicle = static_cast<cVehicle*> (Vehicle->next);
		}
	}

	if (DEDICATED_SERVER == false)
	{
		//FIXME: saving of running attack jobs does not work correctly yet.
		// make autosave
		if (cSettings::getInstance().shouldAutosave())
		{
			cSavegame Savegame (10);	// autosaves are always in slot 10
			Savegame.save (*this, lngPack.i18n ("Text~Settings~Autosave") + " " + lngPack.i18n ("Text~Comp~Turn") + " " + iToStr (iTurn));
			makeAdditionalSaveRequest (10);
		}
	}
#if DEDICATED_SERVER_APPLICATION
	else
	{
		cDedicatedServer::instance().doAutoSave(*this);
	}
#endif

	checkDefeats();

	iWantPlayerEndNum = -1;
}

//-------------------------------------------------------------------------------------
void cServer::checkDefeats()
{
	std::set<cPlayer*> winners, losers;
	int best_score = 0;

	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		cPlayer* Player = (*PlayerList) [i];
		if (!Player->isDefeated)
		{
			int score = Player->getScore (iTurn);
			if (
				(scoreLimit && score >= scoreLimit) ||
				(turnLimit && iTurn >= turnLimit)
			)
			{
				if (score >= best_score)
				{
					if (score > best_score)
					{
						winners.clear();
						best_score = score;
					}
					winners.insert (Player);
				}
			}

			cBuilding* Building = Player->BuildingList;
			cVehicle* Vehicle = Player->VehicleList;
			while (Vehicle)
			{
				if (Vehicle->data.canAttack || !Vehicle->data.canBuild.empty()) break;
				Vehicle = static_cast<cVehicle*> (Vehicle->next);
			}
			if (Vehicle != NULL) continue;
			while (Building)
			{
				if (Building->data.canAttack || !Building->data.canBuild.empty()) break;
				Building = static_cast<cBuilding*> (Building->next);
			}
			if (Building != NULL) continue;

			losers.insert (Player);
		}
	}

	// If some players have won, anyone who hasn't won has lost.
	if (!winners.empty())
		for (unsigned int i = 0; i < PlayerList->Size(); i++)
		{
			cPlayer* Player = (*PlayerList) [i];

			if (winners.find (Player) == winners.end())
				losers.insert (Player);
		}

	// Defeat all players who have lost.
	for (std::set<cPlayer*>::iterator i = losers.begin(); i != losers.end(); ++i)
	{
		cPlayer* Player = *i;

		Player->isDefeated = true;
		sendDefeated (*this, Player);

		if (openMapDefeat && Player->iSocketNum != -1)
		{
			memset (Player->ScanMap, 1, Map->size * Map->size);
			checkPlayerUnits();
			sendNoFog (*this, Player->Nr);
		}
	}

	/*
		Handle the case where there is more than one winner. Original MAX calls
		a draw and displays the results screen. For now we will have sudden
		death, i.e. first player to get ahead in score wins.
	*/
	if (winners.size() > 1)
	{
		for (unsigned int i = 0; i < PlayerList->Size(); i++)
			sendChatMessageToClient (*this, "Text~Comp~SuddenDeath", SERVER_INFO_MESSAGE, i);
	}
}

//-------------------------------------------------------------------------------------
void cServer::addReport (sID Type, bool bVehicle, int iPlayerNum)
{
	sTurnstartReport* Report;
	cPlayer* Player = getPlayerFromNumber (iPlayerNum);
	if (bVehicle)
	{
		for (unsigned int i = 0; i < Player->ReportVehicles.Size(); i++)
		{
			Report = Player->ReportVehicles[i];
			if (Report->Type == Type)
			{
				Report->iAnz++;
				return;
			}
		}
		Report = new sTurnstartReport;
		Report->Type = Type;
		Report->iAnz = 1;
		Player->ReportVehicles.Add (Report);
	}
	else
	{
		for (unsigned int i = 0; i < Player->ReportBuildings.Size(); i++)
		{
			Report = Player->ReportBuildings[i];
			if (Report->Type == Type)
			{
				Report->iAnz++;
				return;
			}
		}
		Report = new sTurnstartReport;
		Report->Type = Type;
		Report->iAnz = 1;
		Player->ReportBuildings.Add (Report);
	}
}

//-------------------------------------------------------------------------------------
void cServer::checkDeadline()
{
	if (!gameTimer.timer50ms) return;
	Uint32 currentTicks = SDL_GetTicks();
	if (iTurnDeadline >= 0 && iDeadlineStartTime > 0)
	{
		// stop time when waiting for reconnection
		if (DisconnectedPlayerList.Size() > 0)
			iDeadlineStartTime = currentTicks;

		if (currentTicks - iDeadlineStartTime > (unsigned int) iTurnDeadline * 1000)
		{
			if (checkEndActions (-1))
			{
				iWantPlayerEndNum = -2;
				return;
			}

			PlayerEndList.Clear();

			iTurn++;
			iDeadlineStartTime = 0;
			makeTurnEnd();
		}
	}
}

//-------------------------------------------------------------------------------------
void cServer::addActiveMoveJob (cServerMoveJob* MoveJob)
{
	ActiveMJobs.Add (MoveJob);
}

//-------------------------------------------------------------------------------------
void cServer::handleMoveJobs()
{
	for (int i = ActiveMJobs.Size() - 1; i >= 0; i--)
	{
		cServerMoveJob* MoveJob;
		cVehicle* Vehicle;

		MoveJob = ActiveMJobs[i];
		Vehicle = MoveJob->Vehicle;

		//suspend movejobs of attacked vehicles
		if (Vehicle && Vehicle->isBeeingAttacked)
			continue;

		// stop the job
		if (MoveJob->bEndForNow && Vehicle)
		{
			Log.write (" Server: Movejob has end for now and will be stoped (delete from active ones)", cLog::eLOG_TYPE_NET_DEBUG);
			sendNextMove (*this, Vehicle, MJOB_STOP, MoveJob->iSavedSpeed);
			ActiveMJobs.Delete (i);
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

				sendNextMove (*this, Vehicle, MJOB_FINISHED);
			}
			else Log.write (" Server: Delete movejob with nonactive vehicle (released one)", cLog::eLOG_TYPE_NET_DEBUG);

			//execute endMoveAction
			if (Vehicle && MoveJob->endAction) MoveJob->endAction->execute (*this);

			delete MoveJob;
			ActiveMJobs.Delete (i);


			//continue path building
			if (Vehicle && Vehicle->BuildPath)
			{
				if (Vehicle->data.storageResCur >= Vehicle->BuildCostsStart && Map->possiblePlaceBuilding (*Vehicle->BuildingTyp.getUnitDataOriginalVersion(), Vehicle->PosX, Vehicle->PosY , Vehicle))
				{
					addJob (new cStartBuildJob (Vehicle, Vehicle->PosX, Vehicle->PosY, Vehicle->data.isBig));
					Vehicle->IsBuilding = true;
					Vehicle->BuildCosts = Vehicle->BuildCostsStart;
					Vehicle->BuildRounds = Vehicle->BuildRoundsStart;
					sendBuildAnswer (*this, true, Vehicle);
				}
				else
				{
					Vehicle->BuildPath = false;
					sendBuildAnswer (*this, false, Vehicle);
				}
			}

			continue;
		}

		if (Vehicle == NULL) continue;


		if (!Vehicle->moving)
		{
			if (!MoveJob->checkMove() && !MoveJob->bFinished)
			{
				ActiveMJobs.Delete (i);
				delete MoveJob;
				Vehicle->ServerMoveJob = NULL;
				Log.write (" Server: Movejob deleted and informed the clients to stop this movejob", LOG_TYPE_NET_DEBUG);
				continue;
			}
			if (MoveJob->bEndForNow)
			{
				Log.write (" Server: Movejob has end for now and will be stoped (delete from active ones)", cLog::eLOG_TYPE_NET_DEBUG);
				sendNextMove (*this, Vehicle, MJOB_STOP, MoveJob->iSavedSpeed);
				ActiveMJobs.Delete (i);
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
			//move the vehicle
			if (gameTimer.timer10ms)
			{
				MoveJob->moveVehicle();
			}
		}
	}
}

//-------------------------------------------------------------------------------------
cUnit* cServer::getUnitFromID (unsigned int iID) const
{
	cUnit* result = getVehicleFromID (iID);
	if (result == 0)
		result = getBuildingFromID (iID);
	return result;
}

//-------------------------------------------------------------------------------------
cVehicle* cServer::getVehicleFromID (unsigned int iID) const
{
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		cVehicle* vehicle = (*PlayerList) [i]->VehicleList;
		while (vehicle != 0)
		{
			if (vehicle->iID == iID)
				return vehicle;
			vehicle = static_cast<cVehicle*> (vehicle->next);
		}
	}
	return 0;
}

//-------------------------------------------------------------------------------------
cBuilding* cServer::getBuildingFromID (unsigned int iID) const
{
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		cBuilding* building = (*PlayerList) [i]->BuildingList;
		while (building != 0)
		{
			if (building->iID == iID)
				return building;
			building = static_cast<cBuilding*> (building->next);
		}
	}
	return 0;
}

//-------------------------------------------------------------------------------------
void cServer::destroyUnit (cVehicle* vehicle)
{
	int offset = vehicle->PosX + vehicle->PosY * Map->size;
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
	cBuildingIterator bi = (*Map) [offset].getBuildings();
	if (bi && bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) bi++;

	while (!bi.end)
	{
		if (bi->owner == 0 && bi->RubbleValue > 0)   // this seems to be rubble
		{
			oldRubbleValue += bi->RubbleValue;
			if (bi->data.isBig)
			{
				rubblePosX = bi->PosX;
				rubblePosY = bi->PosY;
				bigRubble = true;
			}
		}
		else // normal unit
			value += bi->data.buildCosts;
		deleteUnit (bi);
		bi = (*Map) [offset].getBuildings();
		if (bi && bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) bi++;
	}

	if (vehicle->data.isBig)
	{
		bigRubble = true;
		bi = (*Map) [offset + 1].getBuildings();
		if (bi && bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) bi++;
		while (!bi.end)
		{
			value += bi->data.buildCosts;
			deleteUnit (bi);
			bi = (*Map) [offset].getBuildings();
			if (bi && bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) bi++;
		}

		bi = (*Map) [offset + Map->size].getBuildings();
		if (bi && bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) bi++;
		while (!bi.end)
		{
			value += bi->data.buildCosts;
			deleteUnit (bi);
			bi = (*Map) [offset].getBuildings();
			if (bi && bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) bi++;
		}

		bi = (*Map) [offset + 1 + Map->size].getBuildings();
		if (bi && bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) bi++;
		while (!bi.end)
		{
			value += bi->data.buildCosts;
			deleteUnit (bi);
			bi = (*Map) [offset].getBuildings();
			if (bi && bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) bi++;
		}
	}

	if (!vehicle->data.hasCorpse)
	{
		value += vehicle->data.buildCosts;
		if (vehicle->data.storeResType == sUnitData::STORE_RES_METAL)
			value += vehicle->data.storageResCur * 2; // stored material is always added completely to the rubble
	}

	if (value > 0 || oldRubbleValue > 0)
		addRubble (rubblePosX, rubblePosY, value / 2 + oldRubbleValue, bigRubble);

	deleteUnit (vehicle);
}

//-------------------------------------------------------------------------------------
int cServer::deleteBuildings (cBuildingIterator building)
{
	int rubble = 0;
	while (building.size() > 0)
	{
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

//-------------------------------------------------------------------------------------
void cServer::destroyUnit (cBuilding* b)
{
	int offset = b->PosX + b->PosY * Map->size;
	int rubble = 0;
	bool big = false;

	cBuilding* topBuilding = Map->fields[offset].getTopBuilding();
	if (topBuilding && topBuilding->data.isBig)
	{
		big = true;
		offset = topBuilding->PosX + topBuilding->PosY * Map->size;

		cBuildingIterator building = Map->fields[offset + 1].getBuildings();
		rubble += deleteBuildings (building);

		building = Map->fields[offset + Map->size].getBuildings();
		rubble += deleteBuildings (building);

		building = Map->fields[offset + Map->size + 1].getBuildings();
		rubble += deleteBuildings (building);
	}

	sUnitData::eSurfacePosition surfacePosition = b->data.surfacePosition;

	cBuildingIterator building = Map->fields[offset].getBuildings();
	rubble += deleteBuildings (building);

	if (surfacePosition != sUnitData::SURFACE_POS_ABOVE && rubble > 2)
		addRubble (offset % Map->size, offset / Map->size, rubble / 2, big);
}

//-------------------------------------------------------------------------------------
void cServer::addRubble (int x, int y, int value, bool big)
{
	if (value <= 0) value = 1;

	if (Map->isWater (x, y))
	{
		if (big)
		{
			addRubble (x + 1, y    , value / 4, false);
			addRubble (x    , y + 1, value / 4, false);
			addRubble (x + 1, y + 1, value / 4, false);
		}
		return;
	}

	if (big &&
		Map->isWater (x + 1, y))
	{
		addRubble (x    , y    , value / 4, false);
		addRubble (x    , y + 1, value / 4, false);
		addRubble (x + 1, y + 1, value / 4, false);
		return;
	}

	if (big &&
		Map->isWater (x, y + 1))
	{
		addRubble (x    , y    , value / 4, false);
		addRubble (x + 1, y    , value / 4, false);
		addRubble (x + 1, y + 1, value / 4, false);
		return;
	}

	if (big &&
		Map->isWater (x + 1, y + 1))
	{
		addRubble (x    , y    , value / 4, false);
		addRubble (x + 1, y    , value / 4, false);
		addRubble (x    , y + 1, value / 4, false);
		return;
	}

	cBuilding* rubble = new cBuilding (NULL, NULL, iNextUnitID);
	rubble->next = neutralBuildings;
	if (neutralBuildings) neutralBuildings->prev = rubble;
	neutralBuildings = rubble;
	rubble->prev = NULL;

	iNextUnitID++;

	rubble->PosX = x;
	rubble->PosY = y;

	rubble->data.isBig = big;
	rubble->RubbleValue = value;

	Map->addBuilding (rubble, x, y);

	if (big)
	{
		rubble->RubbleTyp = random (2);
	}
	else
	{
		rubble->RubbleTyp = random (5);
	}
}

//-------------------------------------------------------------------------------------
void cServer::deleteRubble (cBuilding* rubble)
{
	Map->deleteBuilding (rubble);

	if (!rubble->prev)
	{
		neutralBuildings = static_cast<cBuilding*> (rubble->next);
		if (rubble->next) rubble->next->prev = NULL;
	}
	else
	{
		rubble->prev->next = rubble->next;
		if (rubble->next)
			rubble->next->prev = rubble->prev;
	}
	sendDeleteUnit (*this, rubble, -1);

	delete rubble;
}

//-------------------------------------------------------------------------------------
void cServer::deletePlayer (cPlayer* Player)
{
	//remove units
	cVehicle* Vehicle = Player->VehicleList;
	while (Vehicle)
	{
		cVehicle* nextVehicle = static_cast<cVehicle*> (Vehicle->next);
		if (!Vehicle->Loaded) deleteUnit (Vehicle);
		Vehicle = nextVehicle;
	}
	while (Player->BuildingList)
	{
		deleteUnit (Player->BuildingList);
	}

	// remove the player of all detected by player lists
	for (unsigned int playerNum = 0; playerNum < PlayerList->Size(); playerNum++)
	{
		cPlayer* UnitPlayer = (*PlayerList) [playerNum];
		if (UnitPlayer == Player) continue;
		Vehicle = UnitPlayer->VehicleList;
		while (Vehicle)
		{
			if (Vehicle->data.isStealthOn != TERRAIN_NONE && Vehicle->isDetectedByPlayer (Player)) Vehicle->resetDetectedByPlayer (*this, Player);
			Vehicle = static_cast<cVehicle*> (Vehicle->next);
		}
	}
	// delete the player
	sendDeletePlayer (*this, Player);
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		if (Player == (*PlayerList) [i])
		{
			PlayerList->Delete (i);
			delete Player;
		}
	}
}

//-------------------------------------------------------------------------------------
void cServer::resyncPlayer (cPlayer* Player, bool firstDelete)
{

	Log.write (" Server:  ============================= begin resync  ==========================", cLog::eLOG_TYPE_NET_DEBUG);
	cVehicle* Vehicle;
	cBuilding* Building;
	if (firstDelete)
	{
		cPlayer* UnitPlayer;
		for (unsigned int i = 0; i < PlayerList->Size(); i++)
		{
			UnitPlayer = (*PlayerList) [i];
			if (UnitPlayer == Player) continue;
			Vehicle = UnitPlayer->VehicleList;
			while (Vehicle)
			{
				Vehicle->seenByPlayerList.Remove (Player);
				Vehicle = static_cast<cVehicle*> (Vehicle->next);
			}
			Building = UnitPlayer->BuildingList;
			while (Building)
			{
				Building->seenByPlayerList.Remove (Player);
				Building = static_cast<cBuilding*> (Building->next);
			}
		}
		Building = neutralBuildings;
		while (Building)
		{
			Building->seenByPlayerList.Remove (Player);
			Building = static_cast<cBuilding*> (Building->next);
		}
		sendDeleteEverything (*this, Player->Nr);
	}

	sendGameTime (*this, Player, gameTimer.gameTime);

	//if (settings->clans == SETTING_CLANS_ON)
	{
		sendClansToClients (*this, PlayerList);
	}
	sendTurn (*this, iTurn, Player);
	if (iDeadlineStartTime > 0) sendTurnFinished (*this, -1, iTurnDeadline - Round ( (SDL_GetTicks() - iDeadlineStartTime) / 1000), Player);
	sendResources (*this, Player);

	// send all units to the client
	Vehicle = Player->VehicleList;
	while (Vehicle)
	{
		if (!Vehicle->Loaded) resyncVehicle (Vehicle, Player);
		Vehicle = static_cast<cVehicle*> (Vehicle->next);
	}

	Building = Player->BuildingList;
	while (Building)
	{
		sendAddUnit (*this, Building->PosX, Building->PosY, Building->iID, false, Building->data.ID, Player->Nr, true);
		for (unsigned int i = 0; i < Building->storedUnits.Size(); i++)
		{
			cVehicle* StoredVehicle = Building->storedUnits[i];
			resyncVehicle (StoredVehicle, Player);
			sendStoreVehicle (*this, Building->iID, false, StoredVehicle->iID, Player->Nr);
		}
		sendUnitData (*this, Building, Player->Nr);
		if (Building->data.canMineMaxRes > 0) sendProduceValues (*this, Building);
		if (Building->BuildList && Building->BuildList->Size() > 0) sendBuildList (*this, Building);
		Building = static_cast<cBuilding*> (Building->next);
	}
	// send all subbases
	for (unsigned int i = 0; i < Player->base.SubBases.Size(); i++)
	{
		sendSubbaseValues (*this, Player->base.SubBases[i], Player->Nr);
	}
	// refresh enemy units
	Player->DoScan();
	checkPlayerUnits();
	// send upgrades
	for (unsigned int i = 0; i < UnitsData.getNrVehicles(); i++)
	{
		if (Player->VehicleData[i].version > 0
			|| Player->VehicleData[i].buildCosts != UnitsData.getVehicle (i, Player->getClan()).data.buildCosts)    // if only costs were researched, the version is not incremented
			sendUnitUpgrades (*this, &Player->VehicleData[i], Player->Nr);
	}
	for (unsigned int i = 0; i < UnitsData.getNrBuildings(); i++)
	{
		if (Player->BuildingData[i].version > 0
			|| Player->BuildingData[i].buildCosts != UnitsData.getBuilding (i, Player->getClan()).data.buildCosts)    // if only costs were researched, the version is not incremented
			sendUnitUpgrades (*this, &Player->BuildingData[i], Player->Nr);
	}
	// send credits
	sendCredits (*this, Player->Credits, Player->Nr);
	// send research
	sendResearchLevel (*this, &Player->researchLevel, Player->Nr);
	sendRefreshResearchCount (*this, Player->Nr);

	// send all players' score histories & eco-counts
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		cPlayer* subj = (*PlayerList) [i];
		for (int t = 1; t <= iTurn; t++)
			sendScore (*this, subj, t, Player);
		sendNumEcos (*this, subj, Player);
	}

	sendVictoryConditions (*this, turnLimit, scoreLimit, Player);

	// send attackJobs
	for (unsigned int i = 0; i < AJobs.Size(); i++)
	{
		cServerAttackJob* ajob = AJobs[i];
		for (int unsigned ajobClient = 0; ajobClient < ajob->executingClients.Size(); ajobClient++)
		{
			if (ajob->executingClients[ajobClient] == Player)
			{
				ajob->sendFireCommand (Player);
			}
		}
	}

	Log.write (" Server:  ============================= end resync  ==========================", cLog::eLOG_TYPE_NET_DEBUG);
}

//-------------------------------------------------------------------------------------
void cServer::resyncVehicle (cVehicle* Vehicle, cPlayer* Player)
{
	sendAddUnit (*this, Vehicle->PosX, Vehicle->PosY, Vehicle->iID, true, Vehicle->data.ID, Player->Nr, true, !Vehicle->Loaded);
	if (Vehicle->ServerMoveJob) sendMoveJobServer (*this, Vehicle->ServerMoveJob, Player->Nr);
	for (unsigned int i = 0; i < Vehicle->storedUnits.Size(); i++)
	{
		cVehicle* StoredVehicle = Vehicle->storedUnits[i];
		resyncVehicle (StoredVehicle, Player);
		sendStoreVehicle (*this, Vehicle->iID, true, StoredVehicle->iID, Player->Nr);
	}
	sendUnitData (*this, Vehicle, Player->Nr);
	sendSpecificUnitData (*this, Vehicle);
	if (Vehicle->hasAutoMoveJob) sendSetAutomoving (*this, Vehicle);
	if (Vehicle->detectedByPlayerList.Size() > 0) sendDetectionState (*this, Vehicle);
}

//--------------------------------------------------------------------------
bool cServer::addMoveJob (int srcX, int srcY, int destX, int destY, cVehicle* vehicle)
{
	cServerMoveJob* MoveJob = new cServerMoveJob (*this, srcX, srcY, destX, destY, vehicle);
	if (!MoveJob->calcPath())
	{
		delete MoveJob;
		vehicle->ServerMoveJob = NULL;
		return false;
	}

	sendMoveJobServer (*this, MoveJob, vehicle->owner->Nr);
	for (unsigned int i = 0; i < vehicle->seenByPlayerList.Size(); i++)
	{
		sendMoveJobServer (*this, MoveJob, vehicle->seenByPlayerList[i]->Nr);
	}

	addActiveMoveJob (MoveJob);
	return true;
}

//--------------------------------------------------------------------------
void cServer::changeUnitOwner (cVehicle* vehicle, cPlayer* newOwner)
{
	if (vehicle->owner && casualtiesTracker)
		casualtiesTracker->logCasualty (vehicle->data.ID, vehicle->owner->Nr);

	// delete vehicle in the list of the old player
	cPlayer* oldOwner = vehicle->owner;
	cVehicle* vehicleList = oldOwner->VehicleList;
	while (vehicleList)
	{
		if (vehicleList == vehicle)
		{
			if (vehicleList->prev)
			{
				vehicleList->prev->next = vehicleList->next;
				if (vehicleList->next) vehicleList->next->prev = vehicleList->prev;
			}
			else if (vehicleList->next)
			{
				oldOwner->VehicleList = static_cast<cVehicle*> (vehicleList->next);
				vehicleList->next->prev = NULL;
			}
			else oldOwner->VehicleList = NULL;
			break;
		}
		vehicleList = static_cast<cVehicle*> (vehicleList->next);
	}
	// add the vehicle to the list of the new player
	vehicle->owner = newOwner;
	newOwner->addUnitToList (vehicle);

	//the vehicle is fully operational for the new owner
	if (vehicle->turnsDisabled > 0)
	{
		vehicle->data.speedCur = vehicle->lastSpeed;
		vehicle->data.shotsCur = vehicle->lastShots;
	}
	vehicle->turnsDisabled = 0;


	// delete the unit on the clients and add it with new owner again
	sendDeleteUnit (*this, vehicle, oldOwner->Nr);
	for (size_t i = 0; i != vehicle->seenByPlayerList.Size(); ++i)
	{
		sendDeleteUnit (*this, vehicle, vehicle->seenByPlayerList[i]->Nr);
	}
	vehicle->seenByPlayerList.Clear();
	vehicle->detectedByPlayerList.Clear();
	sendAddUnit (*this, vehicle->PosX, vehicle->PosY, vehicle->iID, true, vehicle->data.ID, vehicle->owner->Nr, false);
	sendUnitData (*this, vehicle, vehicle->owner->Nr);
	sendSpecificUnitData (*this, vehicle);

	oldOwner->DoScan();
	newOwner->DoScan();
	checkPlayerUnits();

	// let the unit work for his new owner
	if (vehicle->data.canSurvey)
	{
		sendVehicleResources (*this, vehicle, Map);
		vehicle->doSurvey (*this);
	}
	vehicle->makeDetection (*this);
}

//--------------------------------------------------------------------------
void cServer::stopVehicleBuilding (cVehicle* vehicle)
{
	if (!vehicle->IsBuilding) return;

	int iPos = vehicle->PosX + vehicle->PosY * Map->size;

	vehicle->IsBuilding = false;
	vehicle->BuildPath = false;

	if (vehicle->BuildingTyp.getUnitDataOriginalVersion()->isBig)
	{
		Map->moveVehicle (vehicle, vehicle->BuildBigSavedPos % Map->size, vehicle->BuildBigSavedPos / Map->size);
		iPos = vehicle->BuildBigSavedPos;
		vehicle->owner->DoScan();
	}
	sendStopBuild (*this, vehicle->iID, iPos, vehicle->owner->Nr);
	for (unsigned int i = 0; i < vehicle->seenByPlayerList.Size(); i++)
	{
		sendStopBuild (*this, vehicle->iID, iPos, vehicle->seenByPlayerList[i]->Nr);
	}
}

void cServer::sideStepStealthUnit (int PosX, int PosY, cVehicle* vehicle, int bigOffset)
{
	sideStepStealthUnit (PosX, PosY, vehicle->data, vehicle->owner, bigOffset);
}

void cServer::sideStepStealthUnit (int PosX, int PosY, sUnitData& vehicleData, cPlayer* vehicleOwner, int bigOffset)
{
	//TODO: make sure, the stealth vehicle takes the direct diagonal move. Also when two straight moves would be shorter.

	if (vehicleData.factorAir > 0) return;

	//first look for an undetected stealth unit
	cVehicle* stealthVehicle = Map->fields[PosX + PosY * Map->size].getVehicles();
	if (!stealthVehicle) return;
	if (stealthVehicle->owner == vehicleOwner) return;
	if (stealthVehicle->data.isStealthOn == TERRAIN_NONE) return;
	if (stealthVehicle->isDetectedByPlayer (vehicleOwner)) return;

	//make sure a running movement is finished, before starting the side step move
	if (stealthVehicle->moving) stealthVehicle->ServerMoveJob->doEndMoveVehicle();

	//found a stealth unit. Try to find a place where the unit can move
	bool placeFound = false;
	int minCosts = 99;
	int bestX, bestY;
	for (int x = PosX - 1; x <= PosX + 1; x++)
	{
		if (x < 0 || x >= Map->size) continue;
		for (int y = PosY - 1; y <= PosY + 1; y++)
		{
			if (y < 0 || y >= Map->size) continue;
			if (x == PosX && y == PosY) continue;

			//when a bigOffet was passed, for example a contructor needs space for a big building
			//so not all directions are allowed for the side stepping
			if (bigOffset != -1)
			{
				int off = x + y * Map->size;
				if (off == bigOffset ||
					off == bigOffset + 1 ||
					off == bigOffset + Map->size ||
					off == bigOffset + Map->size + 1) continue;
			}

			//check whether this field is a possible destination
			if (!Map->possiblePlace (stealthVehicle, x, y)) continue;

			//check costs of the move
			cPathCalculator pathCalculator (0, 0, 0, 0, Map, stealthVehicle);
			int costs = pathCalculator.calcNextCost (PosX, PosY, x, y);
			if (costs > stealthVehicle->data.speedCur) continue;

			//check whether the vehicle would be detected on the destination field
			bool detectOnDest = false;
			if (stealthVehicle->data.isStealthOn & TERRAIN_GROUND)
			{
				for (unsigned int i = 0; i < PlayerList->Size(); i++)
				{
					if ( (*PlayerList) [i] == stealthVehicle->owner) continue;
					if ( (*PlayerList) [i]->DetectLandMap[x + y * Map->size]) detectOnDest = true;
				}
				if (Map->isWater (x, y, true)) detectOnDest = true;
			}
			if (stealthVehicle->data.isStealthOn & TERRAIN_SEA)
			{
				for (unsigned int i = 0; i < PlayerList->Size(); i++)
				{
					if ( (*PlayerList) [i] == stealthVehicle->owner) continue;
					if ( (*PlayerList) [i]->DetectSeaMap[x + y * Map->size]) detectOnDest = true;
				}
				if (!Map->isWater (x, y, true)) detectOnDest = true;

				if (stealthVehicle->data.factorGround > 0 && stealthVehicle->data.factorSea > 0)
				{
					cBuilding* b = Map->fields[x + y * Map->size].getBaseBuilding();
					if (b && (b->data.surfacePosition == sUnitData::SURFACE_POS_BASE || b->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA || b->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)) detectOnDest = true;
				}
			}
			if (detectOnDest) continue;

			//take the move with the lowest costs. Decide randomly, when costs are equal
			if (costs < minCosts || (costs == minCosts && random (2)))
			{
				//this is a good candidate for a destination
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
		stealthVehicle->ServerMoveJob->checkMove(); //begin the movment immediately, so no other unit can block the destination field
		return;
	}

	//sidestepping failed. Uncover the vehicle.
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
	//only one job per unit
	releaseJob (job->unit);

	helperJobs.Add (job);
	job->unit->job = job;
}

void cServer::runJobs ()
{
	for (unsigned int i = 0; i < helperJobs.Size(); i++)
	{
		if (!helperJobs[i]->finished)
		{
			helperJobs[i]->run (gameTimer);
		}
		if (helperJobs[i]->finished)
		{
			if (helperJobs[i]->unit)
				helperJobs[i]->unit->job = NULL;
			delete helperJobs[i];
			helperJobs.Delete (i);
			i--;
		}
	}
}

void cServer::releaseJob (cUnit* unit)
{
	if (unit->job)
	{
		unit->job->unit = NULL;
		unit->job->finished = true;
	}
}

void cServer::enableFreezeMode (eFreezeMode mode, int playerNumber)
{
	switch (mode)
	{
		case FREEZE_PAUSE:
			freezeModes.pause = true;
			gameTimer.stop ();
			break;
		case FREEZE_WAIT_FOR_RECONNECT:
			freezeModes.waitForReconnect = true;
			gameTimer.stop ();
			break;
		case FREEZE_WAIT_FOR_TURNEND:
			freezeModes.waitForTurnEnd = true;
			break;
		case FREEZE_WAIT_FOR_PLAYER:
			freezeModes.waitForPlayer = true;
			//gameTimer.stop ();	//done in cGameTimer::nextTickAllowed();
			freezeModes.playerNumber = playerNumber;
			break;
		default:
			Log.write (" Server: Tried to enable unsupportet freeze mode: " + iToStr (mode), cLog::eLOG_TYPE_NET_ERROR);
	}

	sendFreeze (*this, mode, freezeModes.playerNumber);
}

void cServer::disableFreezeMode (eFreezeMode mode)
{
	switch (mode)
	{
		case FREEZE_PAUSE:
			freezeModes.pause = false;
			sendUnfreeze (*this, mode);
			break;
		case FREEZE_WAIT_FOR_RECONNECT:
			freezeModes.waitForReconnect = false;
			sendUnfreeze (*this, mode);
			break;
		case FREEZE_WAIT_FOR_TURNEND:
			freezeModes.waitForTurnEnd = false;
			sendUnfreeze (*this, mode);
			break;
		case FREEZE_WAIT_FOR_PLAYER:
			freezeModes.waitForPlayer = false;
			sendUnfreeze (*this, mode);
			break;
		default:
			Log.write (" Server: Tried to disable unsupportet freeze mode: " + iToStr (mode), cLog::eLOG_TYPE_NET_ERROR);
	}

	if (! (freezeModes.pause || freezeModes.waitForReconnect))
	{
		gameTimer.start ();
	}
}

