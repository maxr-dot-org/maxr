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
#include "server.h"
#include "events.h"
#include "network.h"
#include "serverevents.h"
#include "menu.h"
#include "netmessage.h"
#include "mjobs.h"
#include "attackJobs.h"
#include "movejobs.h"

Uint32 ServerTimerCallback(Uint32 interval, void *arg)
{
	((cServer *)arg)->Timer();
	return interval;
}

int CallbackRunServerThread( void *arg )
{
	cServer *Server = (cServer *) arg;
	Server->run();
	return 0;
}

cServer::cServer(cMap* const map, cList<cPlayer*>* const PlayerList, int const iGameType, bool const bPlayTurns)
{
	bDebugCheckPos = false;
	Map = map;
	this->PlayerList = PlayerList;
	this->iGameType = iGameType;
	this->bPlayTurns = bPlayTurns;
	bExit = false;
	bStarted = false;
	neutralBuildings = NULL;
	iActiveTurnPlayerNr = 0;
	iTurn = 1;
	iDeadlineStartTime = 0;
	iTurnDeadline = 45; // just temporary set to 45 seconds
	iNextUnitID = 1;
	iTimerTime = 0;
	iWantPlayerEndNum = -1;
	TimerID = SDL_AddTimer ( 50, ServerTimerCallback, this );

	ServerThread = SDL_CreateThread( CallbackRunServerThread, this );
}

cServer::~cServer()
{
	bExit = true;
	SDL_WaitThread ( ServerThread, NULL );
	SDL_RemoveTimer ( TimerID );

	while ( EventQueue.Size() )
	{
		delete EventQueue[0];
		EventQueue.Delete (0);
	}

	while ( PlayerList->Size() )
	{
		delete (*PlayerList)[0];
		PlayerList->Delete ( 0 );
	}

	while ( AJobs.Size() )
	{
		delete AJobs[0];
		AJobs.Delete(0);
	}
	while ( neutralBuildings )
	{
		cBuilding* nextBuilding = neutralBuildings->next;
		delete neutralBuildings;
		neutralBuildings = nextBuilding;
	}
}


SDL_Event* cServer::pollEvent()
{
	static SDL_Event* lastEvent = NULL;
	if ( lastEvent != NULL )
	{
		free (lastEvent->user.data1);
		delete lastEvent;
		lastEvent = NULL;
	}

	SDL_Event* event;
	if ( EventQueue.Size() <= 0 )
	{
		return NULL;
	}

	cMutex::Lock l(QueueMutex);
	event = EventQueue[0];
	lastEvent = event;
	EventQueue.Delete( 0 );
	return event;
}

int cServer::pushEvent( SDL_Event *event )
{
	cMutex::Lock l(QueueMutex);
	EventQueue.Add ( event );
	return 0;
}

void cServer::sendNetMessage( cNetMessage* message, int iPlayerNum )
{
	message->iPlayerNr = iPlayerNum;

	if (message->iType != DEBUG_CHECK_VEHICLE_POSITIONS)  //do not pollute log file with debug events
		cLog::write("Server: <-- " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );

	if ( iPlayerNum == -1 )
	{
		EventHandler->pushEvent( message->getGameEvent() );
		if ( network ) network->send( message->iLength, message->data );
		delete message;
		//if ( message->iType != DEBUG_CHECK_VEHICLE_POSITIONS && bDebugCheckPos ) sendCheckVehiclePositions();
		return;
	}

	cPlayer *Player = NULL;
	for ( int i = 0; i < PlayerList->Size(); i++ )
	{
		if ((Player = (*PlayerList)[i])->Nr == iPlayerNum) break;
	}

	if ( Player == NULL )
	{
		//player not found
		cLog::write("Server: Can't send message. Player " + iToStr(iPlayerNum) + " not found.", cLog::eLOG_TYPE_NET_WARNING);
		delete message;
		return;
	}
	// Socketnumber MAX_CLIENTS for lokal client
	if ( Player->iSocketNum == MAX_CLIENTS )
	{
		EventHandler->pushEvent ( message->getGameEvent() );
	}
	// on all other sockets the netMessage will be send over TCP/IP
	else
	{
		if ( network ) network->sendTo( Player->iSocketNum, message->iLength, message->serialize() );
	}
	//if ( message->iType != DEBUG_CHECK_VEHICLE_POSITIONS ) sendCheckVehiclePositions(Player);
	delete message;
}

void cServer::run()
{
	while ( !bExit )
	{
		SDL_Event* event = pollEvent();

		if ( event )
		{
			switch ( event->type )
			{
			case NETWORK_EVENT:
				switch ( event->user.code )
				{
				case TCP_ACCEPTEVENT:
					break;
				case TCP_RECEIVEEVENT:
					// new Data received
					{
						cNetMessage message( (char*) event->user.data1 );
						HandleNetMessage( &message );
						CHECK_MEMORY;
					}
					break;
				case TCP_CLOSEEVENT:
					{
						// Socket should be closed
						network->close ( ((Sint16 *)event->user.data1)[0] );
						// Lost Connection

						cNetMessage message(GAME_EV_LOST_CONNECTION );
						message.pushInt16( ((Sint16*)event->user.data1)[0] );
						pushEvent( message.getGameEvent() );
					}
					break;
				}
				break;
			case GAME_EVENT:
				{
					cNetMessage message( (char*) event->user.data1 );
					HandleNetMessage( &message );
					CHECK_MEMORY;
				}
				break;

			default:
				break;
			}
		}

		// don't do anything if games hasn't been started yet!
		if ( !bStarted ) { SDL_Delay( 10 ); continue; }

		checkPlayerUnits();
		CHECK_MEMORY;
		checkDeadline();
		CHECK_MEMORY;
		handleMoveJobs ();
		CHECK_MEMORY;
		handleTimer();
		CHECK_MEMORY;
		handleWantEnd();
		CHECK_MEMORY;
		SDL_Delay( 10 );
	}
}

int cServer::HandleNetMessage( cNetMessage *message )
{
	cLog::write("Server: --> " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );

	switch ( message->iType )
	{
	case GAME_EV_LOST_CONNECTION:
		{
			int iSocketNum = message->popInt16();
			// This is just temporary so doesn't need to be translated
			string sMessage = "Lost connection to ";
			// get the name of player to who the connection has been lost
			for ( int i = 0; i < PlayerList->Size(); i++ )
			{
				cPlayer const* const p = (*PlayerList)[i];
				if (p->iSocketNum == iSocketNum)
				{
					sMessage += p->name;
					break;
				}
			}
			// get the lokal player number
			int iPlayerNum;
			for ( int i = 0; i < PlayerList->Size(); i++ )
			{
				cPlayer const* const p = (*PlayerList)[i];
				if (p->iSocketNum == MAX_CLIENTS)
				{
					iPlayerNum = p->Nr;
					break;
				}
			}
			// send a message to the lokal client
			sendChatMessageToClient( sMessage.c_str(), USER_MESSAGE, iPlayerNum );
		}
		break;
	case GAME_EV_CHAT_CLIENT:
		sendChatMessageToClient( message->popString(), USER_MESSAGE );
		break;
	case GAME_EV_WANT_TO_END_TURN:
		{
			if ( bPlayTurns )
			{
				cPlayer *Player = getPlayerFromNumber ( message->iPlayerNr );
				if ( (*PlayerList)[iActiveTurnPlayerNr] != Player ) break;
			}

			handleEnd ( message->iPlayerNr );
		}
		break;
	case GAME_EV_WANT_START_WORK:
		{
			int PosY = message->popInt16();
			int PosX = message->popInt16();

			//check for invalid messages
			if ( PosY < 0 || PosY > Map->size ) break;
			if ( PosX < 0 || PosX > Map->size ) break;
			cBuilding* building = Map->GO[PosX + PosY*Map->size].top;
			if ( building == NULL || building->owner->Nr != message->iPlayerNr ) break;

			//handle message
			building->ServerStartWork();
			break;
		}
	case GAME_EV_WANT_STOP_WORK:
		{
			int PosY = message->popInt16();
			int PosX = message->popInt16();

			//check for invalid messages
			if ( PosY < 0 || PosY > Map->size ) break;
			if ( PosX < 0 || PosX > Map->size ) break;
			cBuilding* building = Map->GO[PosX + PosY*Map->size].top;
			if ( building == NULL || building->owner->Nr != message->iPlayerNr ) break;

			//handle message
			building->ServerStopWork(false);
			break;
		}
	case GAME_EV_MOVE_JOB_CLIENT:
		{
			int iVehicleID = message->popInt16();
			int iSrcOff = message->popInt32();
			int iDestOff = message->popInt32();
			bool bPlane = message->popBool();

			//FIXME: I think there are some memleaks. is the client movejob alway deleted? --Eiko
			cVehicle *Vehicle = getVehicleFromID ( iVehicleID );
			if ( Vehicle == NULL )
			{
				cLog::write(" Server: Can't find vehicle with id " + iToStr ( iVehicleID ) + " for movejob from " +  iToStr (iSrcOff%Map->size) + "x" + iToStr (iSrcOff/Map->size) + " to " + iToStr (iDestOff%Map->size) + "x" + iToStr (iDestOff/Map->size), cLog::eLOG_TYPE_NET_WARNING);
				break;
			}
			if ( Vehicle->PosX+Vehicle->PosY*Map->size != iSrcOff )
			{
				cLog::write(" Server: Vehicle with id " + iToStr ( iVehicleID ) + " is at wrong position (" + iToStr (Vehicle->PosX) + "x" + iToStr(Vehicle->PosY) + ") for movejob from " +  iToStr (iSrcOff%Map->size) + "x" + iToStr (iSrcOff/Map->size) + " to " + iToStr (iDestOff%Map->size) + "x" + iToStr (iDestOff/Map->size), cLog::eLOG_TYPE_NET_WARNING);
				break;
			}
			if ( Vehicle->bIsBeeingAttacked )
			{
				cLog::write(" Server: cannot move a vehicle currently under attack", cLog::eLOG_TYPE_NET_DEBUG );
				break;
			}
			if ( Vehicle->Attacking )
			{
				cLog::write(" Server: cannot move a vehicle currently attacking", cLog::eLOG_TYPE_NET_DEBUG );
				break;
			}
			if ( Vehicle->IsBuilding )
			{
				cLog::write(" Server: cannot move a vehicle currently building", cLog::eLOG_TYPE_NET_DEBUG );
				break;
			}
			if ( Vehicle->IsClearing )
			{
				cLog::write(" Server: cannot move a vehicle currently building", cLog::eLOG_TYPE_NET_DEBUG );
				break;
			}
			
			cServerMoveJob *MoveJob = new cServerMoveJob ( iSrcOff, iDestOff, bPlane, Vehicle );
			if ( !MoveJob->generateFromMessage ( message ) )
			{
				delete MoveJob;
				break;
			}

			addActiveMoveJob ( MoveJob );
			cLog::write(" Server: Added received movejob", cLog::eLOG_TYPE_NET_DEBUG);
			// send the movejob to all other player who can see this unit
			for ( int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
			{
				sendMoveJobServer( MoveJob, *Vehicle->SeenByPlayerList[i]);
			}
		}
		break;
	case GAME_EV_WANT_STOP_MOVE:
		{
			cVehicle *Vehicle = getVehicleFromID ( message->popInt16() );
			if ( Vehicle == NULL ) break;
			if ( Vehicle->ServerMoveJob == NULL ) break;
			
			Vehicle->ServerMoveJob->release();
		}
		break;
	case GAME_EV_WANT_ATTACK:
		{
			//identify agressor
			bool bIsVehicle = message->popBool();
			cVehicle* attackingVehicle = NULL;
			cBuilding* attackingBuilding = NULL;
			if ( bIsVehicle )
			{
				int ID = message->popInt32();
				attackingVehicle = getVehicleFromID( ID );
				if ( attackingVehicle == NULL ) 
				{
					cLog::write(" Server: vehicle with ID " + iToStr(ID) + " not found", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				if ( attackingVehicle->owner->Nr != message->iPlayerNr )
				{
					cLog::write(" Server: Message was not send by vehicle owner!", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				if ( attackingVehicle->bIsBeeingAttacked ) break;
			}
			else
			{
				int offset = message->popInt32();
				if ( offset < 0 || offset > Map->size * Map->size ) 
				{
					cLog::write(" Server: Invalid agressor offset", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				attackingBuilding = Map->GO[offset].top;
				if ( attackingBuilding == NULL )
				{
					cLog::write(" Server: No Building at agressor offset", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				if ( attackingBuilding->owner->Nr != message->iPlayerNr )
				{
					cLog::write(" Server: Message was not send by building owner!", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				if ( attackingBuilding->bIsBeeingAttacked ) break;
			}

			//find target offset
			int targetOffset = message->popInt32();
			if ( targetOffset < 0 || targetOffset > Map->size * Map->size )
			{
				cLog::write(" Server: Invalid target offset!", cLog::eLOG_TYPE_NET_WARNING);
				break;
			}

			int targetID = message->popInt32();
			if ( targetID != 0 )
			{
				cVehicle* targetVehicle = getVehicleFromID( targetID );
				if ( targetVehicle == NULL )
				{
					cLog::write(" Server: vehicle with ID " + iToStr(targetID) + " not found!", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				int oldOffset = targetOffset;
				targetOffset = targetVehicle->PosX + targetVehicle->PosY * Map->size;
				cLog::write( " Server: attacking vehicle " + targetVehicle->name + ", " + iToStr(targetVehicle->iID), cLog::eLOG_TYPE_NET_DEBUG );
				if ( oldOffset != targetOffset ) cLog::write(" Server: target offset changed from " + iToStr( oldOffset ) + " to " + iToStr( targetOffset ), cLog::eLOG_TYPE_NET_DEBUG );
			}

			//check if attack is possible
			if ( bIsVehicle )
			{
				//FIXME: CanAttackObjekt is accessing the client map!
				if ( !attackingVehicle->CanAttackObject( targetOffset, true ) )
				{
					cLog::write(" Server: The server decided, that the attack is not possible", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				AJobs.Add( new cServerAttackJob( attackingVehicle, targetOffset ));
			}
			else
			{
				//FIXME: CanAttackObjekt is accessing the client map!
				if ( !attackingBuilding->CanAttackObject( targetOffset, true ) )
				{
					cLog::write(" Server: The server decided, that the attack is not possible", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				AJobs.Add( new cServerAttackJob( attackingBuilding, targetOffset ));
			}

		}
		break;
	case GAME_EV_ATTACKJOB_FINISHED:
		{
			int ID = message->popInt16();
			cServerAttackJob* aJob = NULL;

			int i = 0;
			for ( ; i < AJobs.Size(); i++ )
			{
				if (AJobs[i]->iID == ID)
				{
					aJob = AJobs[i];
					break;
				}
			}
			if ( aJob == NULL ) //attack job not found
			{
				cLog::write(" Server: ServerAttackJob not found",cLog::eLOG_TYPE_NET_ERROR);
				break;
			}
			aJob->clientFinished( message->iPlayerNr );
			if ( aJob->executingClients.Size() == 0 )
			{
				AJobs.Delete(i);
				delete aJob;
			}
		}
		break;
	case GAME_EV_MINELAYERSTATUS:
		{
			cVehicle *Vehicle = getVehicleFromID( message->popInt16() );
			if ( Vehicle )
			{
				bool bWasClearing = Vehicle->ClearMines;
				bool bWasLaying = Vehicle->LayMines;
				Vehicle->ClearMines = message->popBool();
				Vehicle->LayMines = message->popBool();

				if ( !bWasClearing && Vehicle->ClearMines ) Vehicle->clearMine();
				if ( !bWasLaying && Vehicle->LayMines ) Vehicle->layMine();
			}
		}
		break;
	case GAME_EV_WANT_BUILD:
		{
			cVehicle *Vehicle;
			sUnitData *Data;
			int iBuildingType, iBuildSpeed, iBuildOff, iPathOff;
			int iTurboBuildRounds[3];
			int iTurboBuildCosts[3];

			Vehicle = getVehicleFromID ( message->popInt16() );
			if ( Vehicle == NULL ) break;

			iBuildingType = message->popInt16();
			Data = &UnitsData.building[iBuildingType].data;
			iBuildSpeed = message->popInt16();
			iBuildOff = message->popInt32();

			if ( Data->is_big )
			{
				if ( Vehicle->data.can_build != BUILD_BIG ) break;

				if ( checkBlockedBuildField ( iBuildOff, Vehicle, Data ) ||
					checkBlockedBuildField ( iBuildOff+1, Vehicle, Data ) ||
					checkBlockedBuildField ( iBuildOff+Map->size, Vehicle, Data ) ||
					checkBlockedBuildField ( iBuildOff+Map->size+1, Vehicle, Data ) )
				{
					sendBuildAnswer ( false, Vehicle->iID, 0, 0, 0, 0, Vehicle->owner->Nr );
					break;
				}
				Vehicle->BuildBigSavedPos = Vehicle->PosX+Vehicle->PosY*Map->size;

				// set vehicle to build position
				Map->moveVehicleBig( Vehicle, iBuildOff );
			}
			else
			{
				if ( iBuildOff != Vehicle->PosX+Vehicle->PosY*Map->size ) break;

				if ( checkBlockedBuildField ( iBuildOff, Vehicle, Data ) )
				{
					sendBuildAnswer ( false, Vehicle->iID, 0, 0, 0, 0, Vehicle->owner->Nr );
					break;
				}
			}

			Vehicle->BuildingTyp = iBuildingType;
			Vehicle->BuildPath = message->popBool();
			iPathOff = message->popInt32();
			Vehicle->BandX = iPathOff%Map->size;
			Vehicle->BandY = iPathOff/Map->size;

			if ( iBuildSpeed == -1 && Vehicle->BuildPath )
			{
				Vehicle->BuildCosts = Vehicle->BuildCostsStart;
				Vehicle->BuildRounds = Vehicle->BuildRoundsStart;
			}
			else
			{
				if ( iBuildSpeed > 2 || iBuildSpeed < 0 ) break;
				Vehicle->calcTurboBuild( iTurboBuildRounds, iTurboBuildCosts, Vehicle->owner->BuildingData[iBuildingType].iBuilt_Costs, Vehicle->owner->BuildingData[iBuildingType].iBuilt_Costs_Max );

				Vehicle->BuildCosts = iTurboBuildCosts[iBuildSpeed];
				Vehicle->BuildRounds = iTurboBuildRounds[iBuildSpeed];
				Vehicle->BuildCostsStart = Vehicle->BuildCosts;
				Vehicle->BuildRoundsStart = Vehicle->BuildRounds;
			}

			Vehicle->IsBuilding = true;

			if ( Vehicle->BuildCosts > Vehicle->data.cargo )
			{
				sendBuildAnswer ( false, Vehicle->iID, 0, 0, 0, 0, Vehicle->owner->Nr );
				break;
			}

			for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
			{
				sendBuildAnswer(true, Vehicle->iID, iBuildOff, iBuildingType, Vehicle->BuildRounds, Vehicle->BuildCosts, *Vehicle->SeenByPlayerList[i]);
			}
			sendBuildAnswer ( true, Vehicle->iID, iBuildOff, iBuildingType, Vehicle->BuildRounds, Vehicle->BuildCosts, Vehicle->owner->Nr );

			if ( Vehicle->ServerMoveJob ) Vehicle->ServerMoveJob->release();
		}
		break;
	case GAME_EV_END_BUILDING:
		{
			cVehicle *Vehicle = getVehicleFromID ( message->popInt16() );
			if ( Vehicle == NULL ) break;

			int iEscapeX = message->popInt16();
			int iEscapeY = message->popInt16();

			if ( !Vehicle->IsBuilding || Vehicle->BuildRounds > 0 ) break;

			// end building
			Vehicle->IsBuilding = false;
			Vehicle->BuildPath = false;

			addUnit( Vehicle->PosX, Vehicle->PosY, &UnitsData.building[Vehicle->BuildingTyp], Vehicle->owner );

			// set the vehicle to the border
			if ( Vehicle->data.can_build == BUILD_BIG )
			{
				int x = Vehicle->PosX;
				int y = Vehicle->PosY;
				if ( iEscapeX > Vehicle->PosX ) x++;
				if ( iEscapeY > Vehicle->PosY ) y++;
				Map->moveVehicle( Vehicle, x, y );

				// refresh SeenByPlayerLists
				checkPlayerUnits();
			}

			// send new vehicle status and position
			sendUnitData ( Vehicle, Vehicle->owner->Nr );
			for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
			{
				sendUnitData(Vehicle, *Vehicle->SeenByPlayerList[i]);
			}

			// drive away from the building lot
			cServerMoveJob *MoveJob = new cServerMoveJob( Vehicle->PosX+Vehicle->PosY*Map->size, iEscapeX+iEscapeY*Map->size, false, Vehicle );
			if ( MoveJob->calcPath() )
			{
				sendMoveJobServer ( MoveJob, Vehicle->owner->Nr );
				for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendMoveJobServer( MoveJob, *Vehicle->SeenByPlayerList[i]);
				}
				addActiveMoveJob ( MoveJob );
			}
			else
			{
				delete MoveJob;
				Vehicle->ServerMoveJob = NULL;
			}
		}
		break;
	case GAME_EV_WANT_CONTINUE_PATH:
		{
			cVehicle *Vehicle = getVehicleFromID ( message->popInt16() );
			if ( Vehicle == NULL ) break;

			int iNextX = message->popInt16();
			int iNextY = message->popInt16();

			if ( !Vehicle->IsBuilding || Vehicle->BuildRounds > 0 ) break;

			// check whether the exit field is free
			cServerMoveJob *MoveJob = new cServerMoveJob( Vehicle->PosX+Vehicle->PosY*Map->size, iNextX+iNextY*Map->size, false, Vehicle );
			if ( Vehicle->checkPathBuild ( iNextX+iNextY*Map->size, Vehicle->BuildingTyp, Map ) && MoveJob->calcPath() )
			{
				addUnit( Vehicle->PosX, Vehicle->PosY, &UnitsData.building[Vehicle->BuildingTyp], Vehicle->owner );
				Vehicle->IsBuilding = false;
				Vehicle->BuildPath = false;

				sendUnitData ( Vehicle, Vehicle->owner->Nr );
				sendContinuePathAnswer ( true, Vehicle->iID, Vehicle->owner->Nr );
				sendMoveJobServer ( MoveJob, Vehicle->owner->Nr );
				for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendUnitData(Vehicle, *Vehicle->SeenByPlayerList[i]);
					sendContinuePathAnswer ( true, Vehicle->iID, *Vehicle->SeenByPlayerList[i]);
					sendMoveJobServer( MoveJob, *Vehicle->SeenByPlayerList[i]);
				}
				addActiveMoveJob ( MoveJob );
			}
			else
			{
				delete MoveJob;
				Vehicle->ServerMoveJob = NULL;

				if ( UnitsData.building[Vehicle->BuildingTyp].data.is_base || UnitsData.building[Vehicle->BuildingTyp].data.is_connector )
				{
					addUnit( Vehicle->PosX, Vehicle->PosY, &UnitsData.building[Vehicle->BuildingTyp], Vehicle->owner );
					Vehicle->IsBuilding = false;
					Vehicle->BuildPath = false;
					sendUnitData ( Vehicle, Vehicle->owner->Nr );
					for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
					{
						sendUnitData(Vehicle, *Vehicle->SeenByPlayerList[i]);
					}
				}
				sendContinuePathAnswer ( false, Vehicle->iID, Vehicle->owner->Nr );
				for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendContinuePathAnswer ( false, Vehicle->iID, Vehicle->owner->Nr );
				}
			}
		}
		break;
	case GAME_EV_WANT_STOP_BUILDING:
		{
			cVehicle *Vehicle = getVehicleFromID ( message->popInt16() );
			if ( Vehicle == NULL ) break;
			if ( !Vehicle->IsBuilding ) break;
			int iPos = Vehicle->PosX+Vehicle->PosY*Map->size;

			Vehicle->IsBuilding = false;
			Vehicle->BuildPath = false;

			if ( Vehicle->data.can_build == BUILD_BIG )
			{
				Map->moveVehicle( Vehicle, Vehicle->BuildBigSavedPos );
				iPos = Vehicle->BuildBigSavedPos;
			}
			sendStopBuild ( Vehicle->iID, iPos, Vehicle->owner->Nr );
			for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
			{
				sendStopBuild ( Vehicle->iID, iPos, *Vehicle->SeenByPlayerList[i] );
			}
		}
		break;
	case GAME_EV_WANT_TRANSFER:
		{
			cVehicle *SrcVehicle = NULL, *DestVehicle = NULL;
			cBuilding *SrcBuilding = NULL, *DestBuilding = NULL;

			if ( message->popBool() ) SrcVehicle = getVehicleFromID ( message->popInt16() );
			else SrcBuilding = getBuildingFromID ( message->popInt16() );

			if ( message->popBool() ) DestVehicle = getVehicleFromID ( message->popInt16() );
			else DestBuilding = getBuildingFromID ( message->popInt16() );

			if ( ( !SrcBuilding && !SrcVehicle ) || ( !DestBuilding && !DestVehicle ) ) break;
			if ( SrcBuilding && DestBuilding ) break;

			int iTranfer = message->popInt16();
			int iType = message->popInt16();

			if ( SrcBuilding )
			{
				bool bBreakSwitch = false;
				if ( DestVehicle->IsBuilding || DestVehicle->IsClearing ) break;
				if ( DestVehicle->data.can_transport != iType ) break;
				if ( DestVehicle->data.cargo+iTranfer > DestVehicle->data.max_cargo || DestVehicle->data.cargo+iTranfer < 0 ) break;
				switch ( iType )
				{
					case TRANS_METAL:
						{
							if ( SrcBuilding->SubBase->Metal-iTranfer > SrcBuilding->SubBase->MaxMetal || SrcBuilding->SubBase->Metal-iTranfer < 0 ) bBreakSwitch = true;
							if ( !bBreakSwitch ) SrcBuilding->owner->base.AddMetal ( SrcBuilding->SubBase, -iTranfer );
						}
						break;
					case TRANS_OIL:
						{
							if ( SrcBuilding->SubBase->Oil-iTranfer > SrcBuilding->SubBase->MaxOil || SrcBuilding->SubBase->Oil-iTranfer < 0 ) bBreakSwitch = true;
							if ( !bBreakSwitch ) SrcBuilding->owner->base.AddOil ( SrcBuilding->SubBase, -iTranfer );
						}
						break;
					case TRANS_GOLD:
						{
							if ( SrcBuilding->SubBase->Gold-iTranfer > SrcBuilding->SubBase->MaxGold || SrcBuilding->SubBase->Gold-iTranfer < 0 ) bBreakSwitch = true;
							if ( !bBreakSwitch ) SrcBuilding->owner->base.AddGold ( SrcBuilding->SubBase, -iTranfer );
						}
						break;
				}
				if ( bBreakSwitch ) break;
				sendSubbaseValues ( SrcBuilding->SubBase, SrcBuilding->owner->Nr );
				DestVehicle->data.cargo += iTranfer;
				sendUnitData ( DestVehicle, DestVehicle->owner->Nr );
			}
			else
			{
				if ( SrcVehicle->data.can_transport != iType ) break;
				if ( SrcVehicle->data.cargo-iTranfer > SrcVehicle->data.max_cargo || SrcVehicle->data.cargo-iTranfer < 0 ) break;
				if ( DestBuilding )
				{
					bool bBreakSwitch = false;
					switch ( iType )
					{
						case TRANS_METAL:
							{
								if ( DestBuilding->SubBase->Metal+iTranfer > DestBuilding->SubBase->MaxMetal || DestBuilding->SubBase->Metal+iTranfer < 0 ) bBreakSwitch = true;
								if ( !bBreakSwitch ) DestBuilding->owner->base.AddMetal ( DestBuilding->SubBase, iTranfer );
							}
							break;
						case TRANS_OIL:
							{
								if ( DestBuilding->SubBase->Oil+iTranfer > DestBuilding->SubBase->MaxOil || DestBuilding->SubBase->Oil+iTranfer < 0 ) bBreakSwitch = true;
								if ( !bBreakSwitch ) DestBuilding->owner->base.AddOil ( DestBuilding->SubBase, iTranfer );
							}
							break;
						case TRANS_GOLD:
							{
								if ( DestBuilding->SubBase->Gold+iTranfer > DestBuilding->SubBase->MaxGold || DestBuilding->SubBase->Gold+iTranfer < 0 ) bBreakSwitch = true;
								if ( !bBreakSwitch ) DestBuilding->owner->base.AddGold ( DestBuilding->SubBase, iTranfer );
							}
							break;
					}
					if ( bBreakSwitch ) break;
					sendSubbaseValues ( DestBuilding->SubBase, DestBuilding->owner->Nr );
				}
				else
				{
					if ( DestVehicle->IsBuilding || DestVehicle->IsClearing ) break;
					if ( DestVehicle->data.can_transport != iType ) break;
					if ( DestVehicle->data.cargo+iTranfer > DestVehicle->data.max_cargo || DestVehicle->data.cargo+iTranfer < 0 ) break;
					DestVehicle->data.cargo += iTranfer;
					sendUnitData ( DestVehicle, DestVehicle->owner->Nr );
				}
				SrcVehicle->data.cargo -= iTranfer;
				sendUnitData ( SrcVehicle, SrcVehicle->owner->Nr );
			}
		}
		break;
	case GAME_EV_WANT_BUILDLIST:
		{
			int iTurboBuildRounds[3], iTurboBuildCosts[3];
			bool bLand = false, bWater = false;
			int iX, iY;

			cBuilding *Building = getBuildingFromID ( message->popInt16() );
			if ( Building == NULL ) break;

			// check whether the building has water and land fields around it
			iX = Building->PosX - 2;
			iY = Building->PosY - 1;
			for ( int i = 0; i < 12; i++ )
			{
				if ( i == 4 ||  i == 6 || i == 8 )
				{
					iX -= 3;
					iY += 1;
				}
				else
				{
					if ( i == 5 || i == 7 ) iX += 3;
					else iX++;
				}

				int iOff = iX + iY * Map->size;

				if ( !Map->IsWater ( iOff, true, true ) || ( Map->GO[iOff].base && ( Map->GO[iOff].base->data.is_bridge || Map->GO[iOff].base->data.is_platform || Map->GO[iOff].base->data.is_road ) ) ) bLand = true;
				else bWater = true;
			}

			// reset building status
			if ( Building->IsWorking )
			{
				Building->ServerStopWork ( false );
			}

			int iBuildSpeed = message->popInt16();
			if ( iBuildSpeed == 0 ) Building->MetalPerRound =  1 * Building->data.iNeeds_Metal;
			if ( iBuildSpeed == 1 ) Building->MetalPerRound =  4 * Building->data.iNeeds_Metal;
			if ( iBuildSpeed == 2 ) Building->MetalPerRound = 12 * Building->data.iNeeds_Metal;

			cList<sBuildList*> *NewBuildList = new cList<sBuildList*>;

			int iCount = message->popInt16();
			for ( int i = 0; i < iCount; i++ )
			{
				int iType = message->popInt16();

				// if the first unit hasn't changed copy it to the new buildlist
				if ( Building->BuildList->Size() > 0 && i == 0 && &UnitsData.vehicle[iType] == (*Building->BuildList)[0]->typ )
				{
					//recalculate costs, because build speed could have beed changed
					Building->CalcTurboBuild ( iTurboBuildRounds, iTurboBuildCosts, Building->owner->VehicleData[iType].iBuilt_Costs, (*Building->BuildList)[0]->metall_remaining );
					sBuildList *BuildListItem = new sBuildList;
					BuildListItem->metall_remaining = iTurboBuildCosts[iBuildSpeed];
					BuildListItem->typ = (*Building->BuildList)[0]->typ;
					NewBuildList->Add ( BuildListItem );
					continue;
				}

				// check whether this building can build this unit
				if ( UnitsData.vehicle[iType].data.can_drive == DRIVE_SEA && !bWater )
					continue;
				else if ( UnitsData.vehicle[iType].data.can_drive == DRIVE_LAND && !bLand )
					continue;

				if ( Building->data.can_build == BUILD_AIR && UnitsData.vehicle[iType].data.can_drive != DRIVE_AIR )
					continue;
				else if ( Building->data.can_build == BUILD_BIG && !UnitsData.vehicle[iType].data.build_by_big )
					continue;
				else if ( Building->data.can_build == BUILD_SEA && UnitsData.vehicle[iType].data.can_drive != DRIVE_SEA )
					continue;
				else if ( Building->data.can_build == BUILD_SMALL && ( UnitsData.vehicle[iType].data.can_drive == DRIVE_AIR || UnitsData.vehicle[iType].data.can_drive == DRIVE_SEA || UnitsData.vehicle[iType].data.build_by_big || UnitsData.vehicle[iType].data.is_human ) )
					continue;
				else if ( Building->data.can_build == BUILD_MAN && !UnitsData.vehicle[iType].data.is_human )
					continue;

				Building->CalcTurboBuild ( iTurboBuildRounds, iTurboBuildCosts, Building->owner->VehicleData[iType].iBuilt_Costs );

				sBuildList *BuildListItem = new sBuildList;
				BuildListItem->metall_remaining = iTurboBuildCosts[iBuildSpeed];
				BuildListItem->typ = &UnitsData.vehicle[iType];

				NewBuildList->Add( BuildListItem );
			}

			// delete all buildings from the list
			while ( Building->BuildList->Size() )
			{
				delete (*Building->BuildList)[0];
				Building->BuildList->Delete( 0 );
			}
			delete Building->BuildList;
			Building->BuildList = NewBuildList;

			if ( Building->BuildList->Size() > 0 )
			{
				Building->RepeatBuild = message->popBool();
				Building->BuildSpeed = iBuildSpeed;
				Building->ServerStartWork ();
			}
			sendBuildList ( Building );
		}
		break;
	case GAME_EV_WANT_EXIT_FIN_VEH:
		{
			sBuildList *BuildingListItem;

			cBuilding *Building = getBuildingFromID ( message->popInt16() );
			if ( Building == NULL ) break;

			int iX = message->popInt16();
			if ( iX < 0 || iX > Map->size ) break;
			int iY = message->popInt16();
			if ( iY < 0 || iY > Map->size ) break;

			if ( Building->BuildList->Size() <= 0 ) break;
			BuildingListItem = (*Building->BuildList)[0];
			if ( BuildingListItem->metall_remaining > 0 ) break;

			if ( checkExitBlocked ( iX, iY, BuildingListItem->typ ) ) break;
			addUnit ( iX, iY, BuildingListItem->typ, Building->owner, false );

			if ( Building->RepeatBuild )
			{
				Building->BuildList->Delete( 0 );
				int iTurboBuildCosts[3];
				int iTurboBuildRounds[3];
				Building->CalcTurboBuild(iTurboBuildRounds, iTurboBuildCosts, BuildingListItem->typ->data.iBuilt_Costs);
				BuildingListItem->metall_remaining = iTurboBuildCosts[Building->BuildSpeed];
				Building->BuildList->Add( BuildingListItem );
				Building->ServerStartWork();
			}
			else
			{
				delete BuildingListItem;
				Building->BuildList->Delete( 0 );
				if ( Building->BuildList->Size() > 0) Building->ServerStartWork();
			}
			sendBuildList ( Building );
		}
		break;
	case GAME_EV_CHANGE_RESOURCES:
		{
			sSubBase *SubBase;
			int iMaxMetal = 0, iMaxOil = 0, iMaxGold = 0;
			int iFreeMetal = 0, iFreeOil = 0, iFreeGold = 0;
			int iTempSBMetalProd, iTempSBOilProd, iTempSBGoldProd;

			cBuilding *Building = getBuildingFromID ( message->popInt16() );
			if ( Building == NULL ) break;

			int iMetalProd = message->popInt16();
			int iOilProd = message->popInt16();
			int iGoldProd = message->popInt16();

			SubBase = Building->SubBase;

			cList<sMineValues*> Mines;
			for ( unsigned int i = 0; i < SubBase->buildings.Size(); i++ )
			{
				cBuilding* const Mine = SubBase->buildings[i];
				if (Mine->data.is_mine && Mine->IsWorking)
				{
					sMineValues *MineValues = new sMineValues;
					MineValues->iMetalProd = Mine->MetalProd;
					MineValues->iOilProd = Mine->OilProd;
					MineValues->iGoldProd = Mine->GoldProd;

					MineValues->iMaxMetalProd = Mine->MaxMetalProd;
					MineValues->iMaxOilProd = Mine->MaxOilProd;
					MineValues->iMaxGoldProd = Mine->MaxGoldProd;

					MineValues->iBuildingID = Mine->iID;

					Mines.Add ( MineValues );

					iMaxMetal += Mine->MaxMetalProd;
					iMaxOil += Mine->MaxOilProd;
					iMaxGold += Mine->MaxGoldProd;
				}
			}

			iTempSBMetalProd = SubBase->MetalProd;
			iTempSBOilProd = SubBase->OilProd;
			iTempSBGoldProd = SubBase->GoldProd;

			if ( iMetalProd > iMaxMetal || iOilProd > iMaxOil || iGoldProd > iMaxGold ) break;

			Building->calcMineFree ( &Mines, &iFreeMetal, &iFreeOil, &iFreeGold );

			bool bDone = false;
			while ( iTempSBMetalProd != iMetalProd || iTempSBOilProd != iOilProd || iTempSBGoldProd != iGoldProd )
			{
				if ( iTempSBMetalProd > iMetalProd )
				{
					Building->doMineDec(TYPE_METAL, Mines);
					iTempSBMetalProd--;
					Building->calcMineFree ( &Mines, &iFreeMetal, &iFreeOil, &iFreeGold );
					bDone = true;
				}
				if ( iTempSBMetalProd < iMetalProd && iFreeMetal )
				{
					Building->doMineInc(TYPE_METAL, Mines);
					iTempSBMetalProd++;
					Building->calcMineFree ( &Mines, &iFreeMetal, &iFreeOil, &iFreeGold );
					bDone = true;
				}

				if ( iTempSBOilProd > iOilProd )
				{
					Building->doMineDec(TYPE_OIL, Mines);
					iTempSBOilProd--;
					Building->calcMineFree ( &Mines, &iFreeMetal, &iFreeOil, &iFreeGold );
					bDone = true;
				}
				if ( iTempSBOilProd < iOilProd && iFreeOil )
				{
					Building->doMineInc(TYPE_OIL, Mines);
					iTempSBOilProd++;
					Building->calcMineFree ( &Mines, &iFreeMetal, &iFreeOil, &iFreeGold );
					bDone = true;
				}

				if ( iTempSBGoldProd > iGoldProd )
				{
					Building->doMineDec(TYPE_GOLD, Mines);
					iTempSBGoldProd--;
					Building->calcMineFree ( &Mines, &iFreeMetal, &iFreeOil, &iFreeGold );
					bDone = true;
				}
				if ( iTempSBGoldProd < iGoldProd && iFreeGold )
				{
					Building->doMineInc(TYPE_GOLD, Mines);
					iTempSBGoldProd++;
					Building->calcMineFree ( &Mines, &iFreeMetal, &iFreeOil, &iFreeGold );
					bDone = true;
				}

				// if in one turn nothing has been done the client values have to be wrong
				if ( bDone == false ) break;
				// bDone must stay true when the server has reached the client values
				else if ( iTempSBMetalProd != iMetalProd || iTempSBOilProd != iOilProd || iTempSBGoldProd != iGoldProd ) bDone = false;
			}

			// set new values
			if ( bDone )
			{
				for ( unsigned int i = 0; i < Mines.Size(); i++ )
				{
					sMineValues *MineValues = Mines[i];
					cBuilding *Mine = getBuildingFromID ( MineValues->iBuildingID );

					Mine->MetalProd = MineValues->iMetalProd;
					Mine->OilProd = MineValues->iOilProd;
					Mine->GoldProd = MineValues->iGoldProd;

					Mine->MaxMetalProd = MineValues->iMaxMetalProd;
					Mine->MaxOilProd = MineValues->iMaxOilProd;
					Mine->MaxGoldProd = MineValues->iMaxGoldProd;

					sendProduceValues ( Mine );
				}

				SubBase->MetalProd = iTempSBMetalProd;
				SubBase->OilProd = iTempSBOilProd;
				SubBase->GoldProd = iTempSBGoldProd;

				sendSubbaseValues ( SubBase, Building->owner->Nr );
			}
		}
		break;
	case GAME_EV_WANT_CHANGE_SENTRY:
		{
			if ( message->popBool() )	// vehicle
			{
				cVehicle *Vehicle = getVehicleFromID ( message->popInt16() );
				if ( Vehicle == NULL ) break;

				Vehicle->bSentryStatus = !Vehicle->bSentryStatus;
				if ( Vehicle->bSentryStatus ) Vehicle->owner->addSentryVehicle ( Vehicle );
				else Vehicle->owner->deleteSentryVehicle ( Vehicle );

				sendUnitData ( Vehicle, Vehicle->owner->Nr );
				for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendUnitData ( Vehicle, *Vehicle->SeenByPlayerList[i] );
				}
			}
			else	// building
			{
				cBuilding *Building = getBuildingFromID ( message->popInt16() );
				if ( Building == NULL ) break;

				Building->bSentryStatus = !Building->bSentryStatus;
				if ( Building->bSentryStatus ) Building->owner->addSentryBuilding ( Building );
				else Building->owner->deleteSentryBuilding ( Building );

				sendUnitData ( Building, Building->owner->Nr );
				for ( unsigned int i = 0; i < Building->SeenByPlayerList.Size(); i++ )
				{
					sendUnitData ( Building, *Building->SeenByPlayerList[i] );
				}
			}
		}
		break;
	case GAME_EV_WANT_MARK_LOG:
		{
			cNetMessage* message2 = new cNetMessage( GAME_EV_MARK_LOG );
			message2->pushString( message->popString() );
			Server->sendNetMessage( message2 );
		}
		break;
	case GAME_EV_WANT_SUPPLY:
		{
			cVehicle *SrcVehicle, *DestVehicle = NULL;
			cBuilding *DestBuilding = NULL;
			int iType, iValue;

			// get the units
			iType = message->popChar();
			if ( iType > SUPPLY_TYPE_REPAIR ) break; // unknown type
			SrcVehicle = getVehicleFromID ( message->popInt16() );
			if ( message->popBool() ) DestVehicle = getVehicleFromID ( message->popInt16() );
			else DestBuilding = getBuildingFromID ( message->popInt16() );

			if ( !SrcVehicle || ( !DestVehicle && !DestBuilding ) ) break;

			// check whether the supply is ok
			if ( DestVehicle && !SrcVehicle->canSupply ( DestVehicle, iType ) ) break;
			if ( DestBuilding && !SrcVehicle->canSupply ( DestBuilding, iType ) ) break;

			// do the supply
			if ( iType == SUPPLY_TYPE_REARM )
			{
				SrcVehicle->data.cargo--;
				iValue = DestVehicle ? DestVehicle->data.max_ammo : DestBuilding->data.max_ammo;
			}
			else
			{
				// TODO: calculate costs for repair and/or maximal repair value
				// temporary a complete repair will cost 2 metal
				if ( SrcVehicle->data.cargo < 2 ) break;
				SrcVehicle->data.cargo -= 2;
				iValue = DestVehicle ? DestVehicle->data.max_hit_points : DestBuilding->data.max_hit_points;
			}
			sendUnitData ( SrcVehicle, SrcVehicle->owner->Nr );	// the changed values aren't interesting for enemy players, so only send the new data to the owner

			if ( DestVehicle )
			{
				if ( iType == SUPPLY_TYPE_REARM ) DestVehicle->data.ammo = DestVehicle->data.max_ammo;
				else DestVehicle->data.hit_points = DestVehicle->data.max_hit_points;

				sendSupply ( DestVehicle->iID, true, iValue, iType, DestVehicle->owner->Nr );
				
				//send unitdata to the players who are not the owner
				for ( int i = 0; i < DestVehicle->SeenByPlayerList.Size(); i++)
					sendUnitData( DestVehicle, *DestVehicle->SeenByPlayerList[i] );
			}
			else
			{
				if ( iType == SUPPLY_TYPE_REARM ) DestBuilding->data.ammo = DestBuilding->data.max_ammo;
				else DestBuilding->data.hit_points = DestBuilding->data.max_hit_points;

				sendSupply ( DestBuilding->iID, false, iValue, iType, DestBuilding->owner->Nr );
								
				//send unitdata to the players who are not the owner
				for ( int i = 0; i < DestBuilding->SeenByPlayerList.Size(); i++)
					sendUnitData( DestBuilding, *DestBuilding->SeenByPlayerList[i] );
			}
		}
		break;
	default:
		cLog::write("Server: Can not handle message, type " + message->getTypeAsString(), cLog::eLOG_TYPE_NET_ERROR);
	}
	
	sendCheckVehiclePositions();
	CHECK_MEMORY;
	return 0;
}

cVehicle *cServer::landVehicle ( int iX, int iY, int iWidth, int iHeight, sVehicle *Vehicle, cPlayer *Player )
{
	cVehicle *VehcilePtr = NULL;
	for ( int i = -iHeight / 2; i < iHeight / 2; i++ )
	{
		for ( int k = -iWidth / 2; k < iWidth / 2; k++ )
		{
			
			if ( !Map->possiblePlaceVehicle( Vehicle->data, iX+k, iY+i )) continue;

			addUnit ( iX+k, iY+i, Vehicle, Player, true );
			VehcilePtr = (*Map)[iX+k+ (iY+i)*Map->size].getVehicles();
			return VehcilePtr;
		}
	}
	return VehcilePtr;
}

void cServer::makeLanding( int iX, int iY, cPlayer *Player, cList<sLanding*> *List, bool bFixed )
{
	sLanding *Landing;
	cVehicle *Vehicle;
	int iWidth, iHeight;

	// Find place for mine if bridgehead is fixed
	if ( bFixed )
	{
		bool bPlaced = false;
		iWidth = 2;
		iHeight = 2;
		while ( !bPlaced )
		{
			for ( int i = -iHeight / 2; i < iHeight / 2; i++ )
			{
				for ( int k = -iWidth / 2; k < iWidth / 2; k++ )
				{
					if ( Map->possiblePlaceBuilding( UnitsData.building[BNrSmallGen].data, iX + k    , iY + i + 1) &&
						 Map->possiblePlaceBuilding( UnitsData.building[BNrMine    ].data, iX + k + 1, iY + i    ) &&
						 Map->possiblePlaceBuilding( UnitsData.building[BNrMine    ].data, iX + k + 2, iY + i    ) &&
						 Map->possiblePlaceBuilding( UnitsData.building[BNrMine    ].data, iX + k + 2, iY + i + 1) &&
						 Map->possiblePlaceBuilding( UnitsData.building[BNrMine    ].data, iX + k + 1, iY + i + 1) )
					{
						bPlaced = true;
						// TODO: the starting resources under a mine should depend on the game Options, like in org MAX
						// Rohstoffe platzieren / place resources under players' base based on resource density:
						Map->Resources[iX+k+1+ ( iY+i ) *Map->size].value = 10;
						Map->Resources[iX+k+1+ ( iY+i ) *Map->size].typ = RES_METAL;
						Map->Resources[iX+k+2+ ( iY+i+1 ) *Map->size].value = 6;
						Map->Resources[iX+k+2+ ( iY+i+1 ) *Map->size].typ = RES_OIL;
						Map->Resources[iX+k+ ( iY+i+1 ) *Map->size].value = 4;
						Map->Resources[iX+k+ ( iY+i+1 ) *Map->size].typ = RES_OIL;
						if ( iY+i-1 >= 0 )
						{
							Map->Resources[iX+k+ ( iY+i-1 ) *Map->size].value = 3;
							Map->Resources[iX+k+ ( iY+i-1 ) *Map->size].typ = RES_METAL;
							Map->Resources[iX+k+2+ ( iY+i-1 ) *Map->size].value = 1;
							Map->Resources[iX+k+2+ ( iY+i-1 ) *Map->size].typ = RES_GOLD;
						}

						// place buildings:
						addUnit(iX + k,     iY + i + 1, &UnitsData.building[BNrSmallGen], Player, true);
						addUnit(iX + k + 1, iY + i,     &UnitsData.building[BNrMine],     Player, true);
						break;
					}
				}
				if ( bPlaced ) break;
			}
			if ( bPlaced ) break;
			iWidth += 2;
			iHeight += 2;
		}
	}

	iWidth = 2;
	iHeight = 2;
	for ( int i = 0; i < List->Size(); i++ )
	{
		Landing = (*List)[i];
		Vehicle = landVehicle(iX, iY, iWidth, iHeight, &UnitsData.vehicle[Landing->id], Player);
		while ( !Vehicle )
		{
			iWidth += 2;
			iHeight += 2;
			Vehicle = landVehicle(iX, iY, iWidth, iHeight, &UnitsData.vehicle[Landing->id], Player);
		}
		if ( Landing->cargo && Vehicle )
		{
			Vehicle->data.cargo = Landing->cargo;
			sendUnitData ( Vehicle, Vehicle->owner->Nr );
		}
	}
}

void cServer::addUnit( int iPosX, int iPosY, sVehicle *Vehicle, cPlayer *Player, bool bInit )
{
	cVehicle *AddedVehicle;
	// generate the vehicle:
	AddedVehicle = Player->AddVehicle ( iPosX, iPosY, Vehicle );
	AddedVehicle->iID = iNextUnitID;
	iNextUnitID++;
	
	// place the vehicle:
	Map->addVehicle( AddedVehicle, iPosX, iPosY );

	// scan with surveyor:
	if ( AddedVehicle->data.can_survey )
	{
		sendResources( AddedVehicle, Map );
		AddedVehicle->doSurvey();
	}
	if ( !bInit ) AddedVehicle->InSentryRange();

	sendAddUnit ( iPosX, iPosY, AddedVehicle->iID, true, Vehicle->nr, Player->Nr, bInit );

	//detection must be done, after the vehicle has been sent to clients
	AddedVehicle->makeDetection();
}

void cServer::addUnit( int iPosX, int iPosY, sBuilding *Building, cPlayer *Player, bool bInit )
{
	cBuilding *AddedBuilding;
	// generate the building:
	AddedBuilding = Player->addBuilding ( iPosX, iPosY, Building );
	if ( AddedBuilding->data.is_mine ) AddedBuilding->CheckRessourceProd();
	if ( AddedBuilding->bSentryStatus ) Player->addSentryBuilding ( AddedBuilding );

	AddedBuilding->iID = iNextUnitID;
	iNextUnitID++;

	int iOff = iPosX + Map->size*iPosY;
	
	//if this is a top building, delete connectors, mines and roads
	if ( !AddedBuilding->data.is_base )
	{
		if ( AddedBuilding->data.is_big )
		{
			deleteUnit ( Map->GO[iOff].top );
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteUnit ( Map->GO[iOff].base );
			}
			iOff++;
			deleteUnit ( Map->GO[iOff].top );
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteUnit ( Map->GO[iOff].base );
			}
			iOff+=Map->size;
			deleteUnit ( Map->GO[iOff].top );
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteUnit ( Map->GO[iOff].base );
			}
			iOff--;
			deleteUnit ( Map->GO[iOff].top );
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteUnit ( Map->GO[iOff].base );
			}
		}
		else
		{
			deleteUnit ( Map->GO[iOff].top );
			if ( !AddedBuilding->data.is_connector&&Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteUnit ( Map->GO[iOff].base );
			}
		}
	}

	Map->addBuilding( AddedBuilding, iPosX, iPosY );

	AddedBuilding->makeDetection();

	sendAddUnit ( iPosX, iPosY, AddedBuilding->iID, false, Building->nr, Player->Nr, bInit );
	if ( AddedBuilding->data.is_mine ) sendProduceValues ( AddedBuilding );
	// integrate the building to the base:
	Player->base.AddBuilding ( AddedBuilding );
}

void cServer::deleteUnit( cBuilding *Building, bool notifyClient )
{
	if( !Building ) return;

	if ( !Building->owner )
	{
		deleteRubble( Building );
		return;
	}

	if( Building->prev )
	{
		Building->prev->next = Building->next;
		if( Building->next )
		{
			Building->next->prev = Building->prev;
		}
	}
	else
	{
		Building->owner->BuildingList = Building->next;
		if( Building->next )
		{
			Building->next->prev = NULL;
		}
	}
	if( Building->base )
	{
		Building->base->DeleteBuilding( Building );
	}

	//detach from attack job
	if (Building->Attacking)
	{
		for ( int i = 0; i < AJobs.Size(); i++ )
		{
			if ( AJobs[i]->building == Building ) AJobs[i]->building = NULL;
		}
	}

	Map->deleteBuilding( Building );

	if ( notifyClient ) sendDeleteUnit( Building, -1 );

	cPlayer* owner = Building->owner;
	delete Building;

	owner->DoScan();
}

void cServer::deleteUnit( cVehicle* vehicle, bool notifyClient )
{
	if( !vehicle ) return;

	if( vehicle->prev )
	{
		vehicle->prev->next = vehicle->next;
		if( vehicle->next )
		{
			vehicle->next->prev = vehicle->prev;
		}
	}
	else
	{
		vehicle->owner->VehicleList = vehicle->next;
		if( vehicle->next )
		{
			vehicle->next->prev = NULL;
		}
	}

	//detach from attack job
	if (vehicle->Attacking)
	{
		for ( int i = 0; i < AJobs.Size(); i++ )
		{
			if ( AJobs[i]->vehicle == vehicle ) AJobs[i]->vehicle = NULL;
		}
	}

	Map->deleteVehicle( vehicle );

	if ( notifyClient ) sendDeleteUnit( vehicle, -1 );


	cPlayer* owner = vehicle->owner;
	delete vehicle;

	if ( owner ) owner->DoScan();
}

void cServer::checkPlayerUnits ()
{
	cPlayer *UnitPlayer;	// The player whos unit is it
	cPlayer *MapPlayer;		// The player who is scaning for new units

	for ( unsigned int iUnitPlayerNum = 0; iUnitPlayerNum < PlayerList->Size(); iUnitPlayerNum++ )
	{
		UnitPlayer = (*PlayerList)[iUnitPlayerNum];
		cVehicle *NextVehicle = UnitPlayer->VehicleList;
		while ( NextVehicle != NULL )
		{
			for ( unsigned int iMapPlayerNum = 0; iMapPlayerNum < PlayerList->Size(); iMapPlayerNum++ )
			{
				if ( iMapPlayerNum == iUnitPlayerNum ) continue;
				MapPlayer = (*PlayerList)[iMapPlayerNum];
				int iOff = NextVehicle->PosX+NextVehicle->PosY*Map->size;

				bool stealthUnit = NextVehicle->data.is_stealth_land || NextVehicle->data.is_stealth_sea;
				if ( MapPlayer->ScanMap[iOff] == 1 && (!stealthUnit || NextVehicle->isDetectedByPlayer( MapPlayer )) )
				{
					unsigned int i;
					for ( i = 0; i < NextVehicle->SeenByPlayerList.Size(); i++ )
					{
						if (*NextVehicle->SeenByPlayerList[i] == MapPlayer->Nr) break;
					}
					if ( i == NextVehicle->SeenByPlayerList.Size() )
					{
						NextVehicle->SeenByPlayerList.Add ( &MapPlayer->Nr );
						sendAddEnemyUnit( NextVehicle, MapPlayer->Nr );
						sendUnitData( NextVehicle, MapPlayer->Nr );
						if ( NextVehicle->ServerMoveJob ) sendMoveJobServer ( NextVehicle->ServerMoveJob, MapPlayer->Nr );
					}
				}
				else
				{
					unsigned int i;
					for ( i = 0; i < NextVehicle->SeenByPlayerList.Size(); i++ )
					{
						if (*NextVehicle->SeenByPlayerList[i] == MapPlayer->Nr)
						{
							NextVehicle->SeenByPlayerList.Delete ( i );
							sendDeleteUnit( NextVehicle, MapPlayer->Nr );
							break;
						}
					}
				}
			}
			NextVehicle = NextVehicle->next;
		}
		cBuilding *NextBuilding = UnitPlayer->BuildingList;
		while ( NextBuilding != NULL )
		{
			for ( unsigned int iMapPlayerNum = 0; iMapPlayerNum < PlayerList->Size(); iMapPlayerNum++ )
			{
				if ( iMapPlayerNum == iUnitPlayerNum ) continue;
				MapPlayer = (*PlayerList)[iMapPlayerNum];
				int iOff = NextBuilding->PosX + NextBuilding->PosY * Map->size;
				bool stealthUnit = NextBuilding->data.is_expl_mine;

				if ( MapPlayer->ScanMap[iOff] == 1  && (!stealthUnit || NextBuilding->isDetectedByPlayer( MapPlayer )) )
				{
					unsigned int i;
					for ( i = 0; i < NextBuilding->SeenByPlayerList.Size(); i++ )
					{
						if (*NextBuilding->SeenByPlayerList[i] == MapPlayer->Nr) break;
					}
					if ( i == NextBuilding->SeenByPlayerList.Size() )
					{
						NextBuilding->SeenByPlayerList.Add ( &MapPlayer->Nr );
						sendAddEnemyUnit( NextBuilding, MapPlayer->Nr );
						sendUnitData( NextBuilding, MapPlayer->Nr );
					}
				}
				else
				{
					unsigned int i;
					for ( i = 0; i < NextBuilding->SeenByPlayerList.Size(); i++ )
					{
						if (*NextBuilding->SeenByPlayerList[i] == MapPlayer->Nr)
						{
							NextBuilding->SeenByPlayerList.Delete ( i );
							sendDeleteUnit( NextBuilding, MapPlayer->Nr );

							break;
						}
					}
				}
			}
			NextBuilding = NextBuilding->next;
		}
	}
	
	//check the neutral objects
	cBuilding *building = neutralBuildings;
	while ( building != NULL )
	{
		for ( unsigned int iMapPlayerNum = 0; iMapPlayerNum < PlayerList->Size(); iMapPlayerNum++ )
		{
			MapPlayer = (*PlayerList)[iMapPlayerNum];
			int iOff = building->PosX + building->PosY * Map->size;
			
			if ( MapPlayer->ScanMap[iOff] == 1 )
			{
				unsigned int i;
				for ( i = 0; i < building->SeenByPlayerList.Size(); i++ )
				{
					if (*building->SeenByPlayerList[i] == MapPlayer->Nr) break;
				}
				if ( i == building->SeenByPlayerList.Size() )
				{
					building->SeenByPlayerList.Add ( &MapPlayer->Nr );
					sendAddRubble( building, MapPlayer->Nr );
				}
			}
			else
			{
				unsigned int i;
				for ( i = 0; i < building->SeenByPlayerList.Size(); i++ )
				{
					if (*building->SeenByPlayerList[i] == MapPlayer->Nr)
					{
						building->SeenByPlayerList.Delete ( i );
						sendDeleteUnit( building, MapPlayer->Nr );
						break;
					}
				}
			}
		}
		building = building->next;
	}
}

cPlayer *cServer::getPlayerFromNumber ( int iNum )
{
	for ( int i = 0; i < PlayerList->Size(); i++ )
	{
		cPlayer* const p = (*PlayerList)[i];
		if (p->Nr == iNum) return p;
	}
	return NULL;
}

void cServer::handleEnd ( int iPlayerNum )
{
	if ( iGameType == GAME_TYPE_SINGLE )
	{
		if ( checkEndActions( iPlayerNum ) )
		{
			iWantPlayerEndNum = iPlayerNum;
			return;
		}
		sendMakeTurnEnd ( true, false, -1, iPlayerNum );
		iTurn++;
		makeTurnEnd ();
		// send report to player
		sendTurnReport ( (*PlayerList)[0] );
	}
	else if ( iGameType == GAME_TYPE_HOTSEAT || bPlayTurns )
	{
		bool bWaitForPlayer = ( iGameType == GAME_TYPE_TCPIP && bPlayTurns );
		if ( checkEndActions( iPlayerNum ) )
		{
			iWantPlayerEndNum = iPlayerNum;
			return;
		}
		iActiveTurnPlayerNr++;
		if ( iActiveTurnPlayerNr >= PlayerList->Size() )
		{
			iActiveTurnPlayerNr = 0;
			if ( iGameType == GAME_TYPE_HOTSEAT )
			{
				sendMakeTurnEnd(true, bWaitForPlayer, (*PlayerList)[iActiveTurnPlayerNr]->Nr, iPlayerNum);
			}
			else
			{
				for ( int i = 0; i < PlayerList->Size(); i++ )
				{
					sendMakeTurnEnd(true, bWaitForPlayer, (*PlayerList)[iActiveTurnPlayerNr]->Nr, (*PlayerList)[i]->Nr);
				}
			}
			iTurn++;
			makeTurnEnd ();
		}
		else
		{
			if ( iGameType == GAME_TYPE_HOTSEAT )
			{
				sendMakeTurnEnd(false, bWaitForPlayer, (*PlayerList)[iActiveTurnPlayerNr]->Nr, iPlayerNum);
				// TODO: in hotseat: maybe send information to client about the next player
			}
			else
			{
				for ( int i = 0; i < PlayerList->Size(); i++ )
				{
					sendMakeTurnEnd(false, bWaitForPlayer, (*PlayerList)[iActiveTurnPlayerNr]->Nr, i);
				}
			}
		}
		// send report to next player
		sendTurnReport ( (*PlayerList)[iActiveTurnPlayerNr] );
	}
	else // it's a simultanous TCP/IP multiplayer game
	{
		// check whether this player has already finished his turn
		for ( int i = 0; i < PlayerEndList.Size(); i++ )
		{
			if (PlayerEndList[i]->Nr == iPlayerNum) return;
		}
		PlayerEndList.Add ( getPlayerFromNumber ( iPlayerNum ) );

		if ( PlayerEndList.Size() >= PlayerList->Size() )
		{
			iDeadlineStartTime = 0;
			if ( checkEndActions( -1 ) )
			{
				iWantPlayerEndNum = iPlayerNum;
				return;
			}
			while ( PlayerEndList.Size() )
			{
				PlayerEndList.Delete ( 0 );
			}
			for ( int i = 0; i < PlayerList->Size(); i++ )
			{
				sendMakeTurnEnd ( true, false, -1, i );
			}

			iTurn++;
			makeTurnEnd ();
			// send reports to all players
			for ( int i = 0; i < PlayerList->Size(); i++ )
			{
				sendTurnReport ( (*PlayerList)[i] );
			}
		}
		else
		{
			if ( PlayerEndList.Size() == 1 )
			{
				sendTurnFinished ( iPlayerNum, iTurnDeadline );
				iDeadlineStartTime = SDL_GetTicks();
			}
			else
			{
				sendTurnFinished ( iPlayerNum, -1 );
			}
		}
	}
}

void cServer::handleWantEnd()
{
	if ( !iTimer0 ) return;
	if ( iWantPlayerEndNum != -1 && iWantPlayerEndNum != -2 )
	{
		for ( unsigned int i = 0; i < PlayerEndList.Size(); i++ )
		{
			if ( iWantPlayerEndNum == PlayerEndList[i]->Nr ) PlayerEndList.Delete( i );
		}
		handleEnd ( iWantPlayerEndNum );
	}
}

bool cServer::checkEndActions ( int iPlayer )
{
	string sMessage = "";
	if ( ActiveMJobs.Size() > 0)
	{
		sMessage = "Text~Comp~Turn_Wait";
	}
	else
	{
		for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
		{
			cVehicle *NextVehicle;
			NextVehicle = (*PlayerList)[i]->VehicleList;
			while ( NextVehicle != NULL )
			{
				if ( NextVehicle->ServerMoveJob && NextVehicle->data.speed > 0 && !NextVehicle->moving )
				{
					// restart movejob
					NextVehicle->ServerMoveJob->calcNextDir();
					NextVehicle->ServerMoveJob->bEndForNow = false;
					NextVehicle->ServerMoveJob->ScrX = NextVehicle->PosX;
					NextVehicle->ServerMoveJob->ScrY = NextVehicle->PosY;
					for ( unsigned int j = 0; j < NextVehicle->SeenByPlayerList.Size(); j++ )
					{
						sendMoveJobServer ( NextVehicle->ServerMoveJob, *NextVehicle->SeenByPlayerList[j] );
					}
					sendMoveJobServer ( NextVehicle->ServerMoveJob, NextVehicle->owner->Nr );
					addActiveMoveJob ( NextVehicle->ServerMoveJob );
					sMessage = "Text~Comp~Turn_Automove";
				}
				NextVehicle = NextVehicle->next;
			}
		}
	}
	if ( sMessage.length() > 0 )
	{
		if ( iPlayer != -1 )
		{
			if ( iWantPlayerEndNum == -1 )
			{
				sendChatMessageToClient ( sMessage, SERVER_INFO_MESSAGE, iPlayer );
			}
		}
		else
		{
			if ( iWantPlayerEndNum == -1 )
			{
				for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
				{
					sendChatMessageToClient ( sMessage, SERVER_INFO_MESSAGE, (*PlayerList)[i]->Nr );
				}
			}
		}
		return true;
	}
	return false;
}

void cServer::makeTurnEnd ()
{
	// reload all buildings
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
	{
		cBuilding *Building;
		cPlayer *Player;
		Player = (*PlayerList)[i];

		Building = Player->BuildingList;
		while ( Building )
		{
			if ( Building->Disabled )
			{
				Building->Disabled--;
				if ( Building->Disabled )
				{
					for ( unsigned int k = 0; k < Building->SeenByPlayerList.Size(); k++ )
					{
						sendUnitData(Building, *Building->SeenByPlayerList[k]);
					}
					sendUnitData ( Building, Building->owner->Nr );

					Building = Building->next;
					continue;
				}
			}
			if ( Building->data.can_attack && Building->refreshData() )
			{
				for ( unsigned int k = 0; k < Building->SeenByPlayerList.Size(); k++ )
				{
					sendUnitData(Building, *Building->SeenByPlayerList[k]);
				}
				sendUnitData ( Building, Building->owner->Nr );
			}
			Building = Building->next;
		}
	}

	// reload all vehicles
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
	{
		cVehicle *Vehicle;
		cPlayer *Player;
		Player = (*PlayerList)[i];

		Vehicle = Player->VehicleList;
		while ( Vehicle )
		{
			if ( Vehicle->Disabled )
			{
				Vehicle->Disabled--;
				if ( Vehicle->Disabled )
				{
					for ( unsigned int k = 0; k < Vehicle->SeenByPlayerList.Size(); k++ )
					{
						sendUnitData(Vehicle, *Vehicle->SeenByPlayerList[k]);
					}
					sendUnitData ( Vehicle, Vehicle->owner->Nr );

					Vehicle = Vehicle->next;
					continue;
				}
			}

			if ( Vehicle->refreshData() )
			{
				for ( unsigned int k = 0; k < Vehicle->SeenByPlayerList.Size(); k++ )
				{
					sendUnitData(Vehicle, *Vehicle->SeenByPlayerList[k]);
				}
				sendUnitData ( Vehicle, Vehicle->owner->Nr );
			}

			if ( Vehicle->ServerMoveJob ) Vehicle->ServerMoveJob->bEndForNow = false;
			Vehicle = Vehicle->next;
		}
	}

	//hide stealth units
	for ( int i = 0; i < PlayerList->Size(); i++ )
	{
		cPlayer* player = (*PlayerList)[i];
		player->DoScan();							//make sure the detection maps are up to date
		cVehicle* vehicle = player->VehicleList;
		while ( vehicle )
		{
			while ( vehicle->DetectedByPlayerList.Size() ) vehicle->resetDetectedByPlayer(vehicle->DetectedByPlayerList[0]);
			vehicle->makeDetection();
			vehicle = vehicle->next;
		}
	}

	// Gun'em down:
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
	{
		cVehicle *Vehicle;
		cPlayer *Player;
		Player = (*PlayerList)[i];

		Vehicle = Player->VehicleList;
		while ( Vehicle )
		{
			Vehicle->InSentryRange();
			Vehicle = Vehicle->next;
		}
	}

	// TODO: implement these things

	// produce resources
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
	{
		(*PlayerList)[i]->base.handleTurnend();
	}

	// do research:
	//game->ActivePlayer->DoResearch();

	// collect trash:
	//collectTrash();

	// make autosave
	if ( SettingsData.bAutoSave )
	{
		//makeAutosave();
	}

	//checkDefeat();

	iWantPlayerEndNum = -1;
}

void cServer::addReport ( int iType, bool bVehicle, int iPlayerNum )
{
	sTurnstartReport *Report;
	cPlayer *Player = getPlayerFromNumber ( iPlayerNum );
	if ( bVehicle )
	{
		for ( int i = 0; i < Player->ReportVehicles.Size(); i++ )
		{
			Report = Player->ReportVehicles[i];
			if ( Report->iType == iType )
			{
				Report->iAnz++;
				return;
			}
		}
		Report = new sTurnstartReport;
		Report->iType = iType;
		Report->iAnz = 1;
		Player->ReportVehicles.Add ( Report );
	}
	else
	{
		for ( int i = 0; i < Player->ReportBuildings.Size(); i++ )
		{
			Report = Player->ReportBuildings[i];
			if ( Report->iType == iType )
			{
				Report->iAnz++;
				return;
			}
		}
		Report = new sTurnstartReport;
		Report->iType = iType;
		Report->iAnz = 1;
		Player->ReportBuildings.Add ( Report );
	}
}

void cServer::checkDeadline ()
{
	if ( !iTimer0 ) return;
	if ( iTurnDeadline >= 0 && iDeadlineStartTime > 0 )
	{
		if ( SDL_GetTicks() - iDeadlineStartTime > (unsigned int)iTurnDeadline*1000 )
		{
			if ( checkEndActions( -1 ) )
			{
				iWantPlayerEndNum = -2;
				return;
			}

			while ( PlayerEndList.Size() ) PlayerEndList.Delete ( 0 );

			for ( int i = 0; i < PlayerList->Size(); i++ )
			{
				sendMakeTurnEnd ( true, false, -1, i );
			}
			iTurn++;
			iDeadlineStartTime = 0;
			makeTurnEnd();
			for ( int i = 0; i < PlayerList->Size(); i++ )
			{
				sendTurnReport ( (*PlayerList)[i] );
			}
		}
	}
}

void cServer::addActiveMoveJob ( cServerMoveJob *MoveJob )
{
	ActiveMJobs.Add ( MoveJob );
}

void cServer::handleMoveJobs ()
{
	for ( int i = 0; i < ActiveMJobs.Size(); i++ )
	{
		cServerMoveJob *MoveJob;
		cVehicle *Vehicle;

		MoveJob = ActiveMJobs[i];
		Vehicle = MoveJob->Vehicle;

		//suspend movejobs of attacked vehicles
		if ( Vehicle && Vehicle->bIsBeeingAttacked ) 
			continue;

		if ( MoveJob->bFinished || MoveJob->bEndForNow )
		{
			// stop the job
			if ( MoveJob->bEndForNow && Vehicle )
			{
				cLog::write(" Server: Movejob has end for now and will be stoped (delete from active ones)", cLog::eLOG_TYPE_NET_DEBUG);
				for ( int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendNextMove( Vehicle->iID, Vehicle->PosX+Vehicle->PosY * Map->size, MJOB_STOP, *Vehicle->SeenByPlayerList[i]);
				}
				sendNextMove ( Vehicle->iID, Vehicle->PosX+Vehicle->PosY*Map->size, MJOB_STOP, Vehicle->owner->Nr );
			}
			else
			{
				if ( Vehicle && Vehicle->ServerMoveJob == MoveJob )
				{
					cLog::write(" Server: Movejob is finished and will be deleted now", cLog::eLOG_TYPE_NET_DEBUG);
					Vehicle->ServerMoveJob = NULL;
					Vehicle->moving = false;
					Vehicle->MoveJobActive = false;

					for ( int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
					{
						sendNextMove( Vehicle->iID, Vehicle->PosX+Vehicle->PosY * Map->size, MJOB_FINISHED, *Vehicle->SeenByPlayerList[i]);
					}
					sendNextMove ( Vehicle->iID, Vehicle->PosX+Vehicle->PosY*Map->size, MJOB_FINISHED, Vehicle->owner->Nr );
				}
				else cLog::write(" Server: Delete movejob with nonactive vehicle (released one)", cLog::eLOG_TYPE_NET_DEBUG);
				delete MoveJob;
			}
			ActiveMJobs.Delete ( i );
			continue;
		}

		if ( Vehicle == NULL ) continue;

		// rotate vehicle
		if ( MoveJob->iNextDir != Vehicle->dir && Vehicle->data.speed )
		{
			Vehicle->rotating = true;
			if ( iTimer1 )
			{
				Vehicle->RotateTo ( MoveJob->iNextDir );
			}
			continue;
		}
		else
		{
			Vehicle->rotating = false;
		}

		if ( !Vehicle->moving )
		{
			if ( !MoveJob->checkMove() && !MoveJob->bFinished )
			{
				for ( unsigned int i = 0; i < ActiveMJobs.Size(); i++ )
				{
					if ( MoveJob == ActiveMJobs[i] )
					{
						ActiveMJobs.Delete ( i );
						break;
					}
				}
				delete MoveJob;
				Vehicle->ServerMoveJob = NULL;
				cLog::write( " Server: Movejob deleted and informed the clients to stop this movejob", LOG_TYPE_NET_DEBUG );
			}
		}
		else
		{
			if ( Server->iTimer0 ) MoveJob->moveVehicle ();
		}
	}
}

void cServer::Timer()
{
	iTimerTime++;
}

void cServer::handleTimer()
{
	//iTimer0: 50ms
	//iTimer1: 100ms
	//iTimer2: 400ms

	static unsigned int iLast = 0, i = 0;
	iTimer0 = 0 ;
	iTimer1 = 0;
	iTimer2 = 0;
	if ( iTimerTime != iLast )
	{
		iLast = iTimerTime;
		i++;
		iTimer0 = 1;
		if ( i&0x1 ) iTimer1 = 1;
		if ( ( i&0x3 ) == 3 ) iTimer2 = 1;
	}
}

cVehicle *cServer::getVehicleFromID ( int iID )
{
	cVehicle *Vehicle;
	for ( int i = 0; i < PlayerList->Size(); i++ )
	{
		Vehicle = (*PlayerList)[i]->VehicleList;
		while ( Vehicle )
		{
			if ( Vehicle->iID == iID ) return Vehicle;
			Vehicle = Vehicle->next;
		}
	}
	return NULL;
}

cBuilding *cServer::getBuildingFromID ( int iID )
{
	cBuilding *Building;
	for ( int i = 0; i < PlayerList->Size(); i++ )
	{
		Building = (*PlayerList)[i]->BuildingList;
		while ( Building )
		{
			if ( Building->iID == iID ) return Building;
			Building = Building->next;
		}
	}
	return NULL;
}

bool cServer::checkBlockedBuildField ( int iOff, cVehicle *Vehicle, sUnitData *Data )
{
	// cannot build on dirt
	if ( Map->GO[iOff].subbase && !Map->GO[iOff].subbase->owner ) return true;

	// cannot build e.g. landingplattforms on waterplattforms or bridges
	if ( Map->GO[iOff].base && ( Map->GO[iOff].base->data.is_platform || Map->GO[iOff].base->data.is_bridge ) && ( Data->is_base && !Data->is_road ) ) return true;

	// the rest has only to be checked if the building is no connector and if there is no base building under it excepting an waterplattform
	if ( Map->GO[iOff].base && Map->GO[iOff].base->data.is_platform && !Data->is_connector )
	{
		// cannot build normal buildings on water without platform
		if ( Map->IsWater ( iOff ) && !Data->build_on_water && !Map->GO[iOff].base->data.is_platform ) return true;

		// cannot build water buildings excepting a bridge or a waterplattform on not water terrain but maybe coasts
		if ( !Map->IsWater ( iOff ) && Data->build_on_water && !( Data->is_bridge || Data->is_platform ) ) return true;

		// only platforms and bridges can be build on coasts without a platform
		if ( Map->terrain[Map->Kacheln[iOff]].coast && !Data->is_bridge && !Data->is_platform && !Map->GO[iOff].base->data.is_platform ) return true;

		// cannot build plattforms or bridges on nowater or nocoast terrain
		if ( !Map->terrain[Map->Kacheln[iOff]].coast && !Map->IsWater ( iOff ) && ( Data->is_bridge || Data->is_platform ) ) return true;
	}

	return false;
}

bool cServer::checkExitBlocked ( int iX, int iY, sVehicle *Type )
{
	int iOff = iX+iY*Map->size;

	if ( iOff < 0 || iOff >= Map->size*Map->size ) return true;

	if ( Type->data.can_drive == DRIVE_AIR && Map->GO[iOff].plane ) return true;
	if ( Type->data.can_drive != DRIVE_AIR && ( Map->GO[iOff].vehicle || ( Map->GO[iOff].top && !Map->GO[iOff].top->data.is_connector ) ) ) return true;
	if ( Type->data.can_drive == DRIVE_SEA && ( !Map->IsWater ( iOff, true ) || Map->GO[iOff].base || Map->GO[iOff].subbase ) ) return true;
	if ( Type->data.can_drive == DRIVE_LAND && Map->IsWater ( iOff ) && ( !Map->GO[iOff].base || ( !Map->GO[iOff].base->data.is_platform && !Map->GO[iOff].subbase->data.is_road && !Map->GO[iOff].base->data.is_expl_mine ) ) ) return true;

	return false;
}

void cServer::destroyUnit( cVehicle* vehicle )
{
	int offset = vehicle->PosX + vehicle->PosY*Map->size;
	int value = vehicle->data.iBuilt_Costs/2;

	//delete other units here!


	if ( vehicle->data.can_drive != DRIVE_AIR && !vehicle->data.is_human )
	{
		addRubble( offset, value, false );
	}
	deleteUnit( vehicle, false );

	
}

void cServer::destroyUnit(cBuilding *building)
{
	int offset = building->PosX + building->PosY * Map->size;
	int value = 0;
	bool big = false;

	if ( Map->GO[offset].top && Map->GO[offset].top->data.is_big )
	{
		big = true;

		if ( Map->GO[offset + 1            ].base )    value += Map->GO[offset + 1            ].base->data.iBuilt_Costs;
		if ( Map->GO[offset + Map->size    ].base )    value += Map->GO[offset + Map->size    ].base->data.iBuilt_Costs;
		if ( Map->GO[offset + Map->size + 1].base )    value += Map->GO[offset + Map->size + 1].base->data.iBuilt_Costs;
		if ( Map->GO[offset + 1            ].subbase && Map->GO[offset + 1            ].subbase->owner ) value += Map->GO[offset + 1            ].subbase->data.iBuilt_Costs;
		if ( Map->GO[offset + Map->size    ].subbase && Map->GO[offset + Map->size    ].subbase->owner ) value += Map->GO[offset + Map->size    ].subbase->data.iBuilt_Costs;
		if ( Map->GO[offset + Map->size + 1].subbase && Map->GO[offset + Map->size + 1].subbase->owner ) value += Map->GO[offset + Map->size + 1].subbase->data.iBuilt_Costs;

		deleteUnit( Map->GO[offset + 1            ].base, false );
		deleteUnit( Map->GO[offset + Map->size    ].base, false );
		deleteUnit( Map->GO[offset + Map->size + 1].base, false );
		deleteUnit( Map->GO[offset + 1            ].subbase, false );
		deleteUnit( Map->GO[offset + Map->size    ].subbase, false );
		deleteUnit( Map->GO[offset + Map->size + 1].subbase, false );
	}
 
	if ( Map->GO[offset].top )     value += Map->GO[offset].top->data.iBuilt_Costs;
	if ( Map->GO[offset].base )    value += Map->GO[offset].base->data.iBuilt_Costs;
	if ( Map->GO[offset].subbase && Map->GO[offset].subbase->owner ) value += Map->GO[offset].subbase->data.iBuilt_Costs;

	deleteUnit( Map->GO[offset].top, false );
	deleteUnit( Map->GO[offset].base, false );
	deleteUnit( Map->GO[offset].subbase, false );

	if ( !building->data.is_connector )
	{
		addRubble( offset, value/2, big );
	}
}

void cServer::addRubble( int offset, int value, bool big )
{
	if ( value <= 0 ) value = 1;

	if ( Map->terrain[Map->Kacheln[offset]].water ||
		 Map->terrain[Map->Kacheln[offset]].coast ||
		 Map->GO[offset].base ||
		 Map->GO[offset].top ||
		 Map->GO[offset].subbase )
	{
		if ( big )
		{
			addRubble( offset + 1, value/4, false);
			addRubble( offset + Map->size, value/4, false);
			addRubble( offset + Map->size + 1, value/4, false);
		}
		return;
	}

	if ( big && (
		 Map->terrain[Map->Kacheln[offset + 1]].water ||
		 Map->terrain[Map->Kacheln[offset + 1]].coast ||
		 Map->GO[offset].top ||
		 Map->GO[offset + 1].base ||
		 Map->GO[offset + 1].subbase ))
	{
		addRubble( offset, value/4, false);
		addRubble( offset + Map->size, value/4, false);
		addRubble( offset + Map->size + 1, value/4, false);
		return;
	}

	if ( big && (
		Map->terrain[Map->Kacheln[offset + Map->size]].water ||
		Map->terrain[Map->Kacheln[offset + Map->size]].coast ||
		Map->GO[offset].top ||
		Map->GO[offset + Map->size].base ||
		Map->GO[offset + Map->size].subbase ))
	{
		addRubble( offset, value/4, false);
		addRubble( offset + 1, value/4, false);
		addRubble( offset + Map->size + 1, value/4, false);
		return;
	}

	if ( big && (
		Map->terrain[Map->Kacheln[offset + Map->size + 1]].water ||
		Map->terrain[Map->Kacheln[offset + Map->size + 1]].coast ||
		Map->GO[offset].top ||
		Map->GO[offset + Map->size + 1].base ||
		Map->GO[offset + Map->size + 1].subbase ))
	{
		addRubble( offset, value/4, false);
		addRubble( offset + 1, value/4, false);
		addRubble( offset + Map->size, value/4, false);
		return;
	}

	cBuilding* rubble = new cBuilding( NULL, NULL, NULL );
	rubble->next = neutralBuildings;
	if ( neutralBuildings ) neutralBuildings->prev = rubble;
	neutralBuildings = rubble;
	rubble->prev = NULL;

	rubble->iID = iNextUnitID;
	iNextUnitID++;

	rubble->PosX = offset % Map->size;
	rubble->PosY = offset / Map->size;

	rubble->data.is_big = big;
	rubble->RubbleValue = value;

	Map->addBuilding( rubble, offset );

	if ( big )
	{
		rubble->RubbleTyp = random(2);
	}
	else
	{
		rubble->RubbleTyp = random(5);
	}
}

void cServer::deleteRubble( cBuilding* rubble )
{
	Map->deleteBuilding( rubble );

	if ( !rubble->prev )
	{
		neutralBuildings = rubble->next;
		rubble->next->prev = NULL;
	}
	else
	{
		rubble->prev->next = rubble->next;
		if ( rubble->next )
			rubble->next->prev = rubble->prev;
	}
	delete rubble;

	sendDeleteUnit( rubble, -1 );
}
