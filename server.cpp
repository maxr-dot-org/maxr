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
#include "client.h"
#include "events.h"
#include "network.h"
#include "serverevents.h"
#include "menu.h"
#include "netmessage.h"
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
	bHotSeat = false;
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
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
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
					sendRequestIdentification ( ((Sint16*)event->user.data1)[0] );
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
						int iSocketNumber = ((Sint16 *)event->user.data1)[0];
						network->close ( iSocketNumber );

						// resort socket numbers of the players
						for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
						{
							if ( (*PlayerList)[i]->iSocketNum > iSocketNumber && (*PlayerList)[i]->iSocketNum != MAX_CLIENTS ) (*PlayerList)[i]->iSocketNum--;
						}
						// push lost connection message
						cNetMessage message( GAME_EV_LOST_CONNECTION );
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
			cPlayer *Player = NULL;
			// get the player to who the connection has been lost
			for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
			{
				if ( (*PlayerList)[i]->iSocketNum == iSocketNum )
				{
					Player = (*PlayerList)[i];
					break;
				}
			}
			if ( !Player ) break;

			// freeze clients
			sendFreeze ();
			sendChatMessageToClient(  "Text~Multiplayer~Lost_Connection", SERVER_INFO_MESSAGE, -1, Player->name );

			DisconnectedPlayerList.Add ( Player );

			memset( Player->ScanMap, 0, Map->size*Map->size );
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
			for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
			{
				sendMoveJobServer( MoveJob, Vehicle->SeenByPlayerList[i]->Nr );
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
				//the target offset doesn't need to match the vehicle position, when it is big
				if ( !targetVehicle->data.is_big ) 
				{
					targetOffset = targetVehicle->PosX + targetVehicle->PosY * Map->size;
				}
				cLog::write( " Server: attacking vehicle " + targetVehicle->name + ", " + iToStr(targetVehicle->iID), cLog::eLOG_TYPE_NET_DEBUG );
				if ( oldOffset != targetOffset ) cLog::write(" Server: target offset changed from " + iToStr( oldOffset ) + " to " + iToStr( targetOffset ), cLog::eLOG_TYPE_NET_DEBUG );
			}

			//check if attack is possible
			if ( bIsVehicle )
			{
				if ( !attackingVehicle->CanAttackObject( targetOffset, true ) )
				{
					cLog::write(" Server: The server decided, that the attack is not possible", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				AJobs.Add( new cServerAttackJob( attackingVehicle, targetOffset ));
			}
			else
			{
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

			unsigned int i = 0;
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
			if ( !Vehicle ) break;

			Vehicle->ClearMines = message->popBool();
			Vehicle->LayMines = message->popBool();

			if ( Vehicle->ClearMines && Vehicle->LayMines )
			{
				Vehicle->ClearMines = false;
				Vehicle->LayMines = false;
				break;
			}

			bool result = false;
			if ( Vehicle->ClearMines ) result = Vehicle->clearMine();
			if ( Vehicle->LayMines ) result = Vehicle->layMine();

			if ( result )
			{
				sendUnitData( Vehicle, Vehicle->owner->Nr );
				for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendUnitData(Vehicle, Vehicle->SeenByPlayerList[i]->Nr );
				}
			}
		}
		break;
	case GAME_EV_WANT_BUILD:
		{
			cVehicle *Vehicle;
			int iBuildSpeed, iBuildOff, iPathOff;
			int iTurboBuildRounds[3];
			int iTurboBuildCosts[3];
			sID BuildingType;

			Vehicle = getVehicleFromID ( message->popInt16() );
			if ( Vehicle == NULL ) break;

			BuildingType.iFirstPart = message->popInt16();
			BuildingType.iSecondPart = message->popInt16();
			const sUnitData& Data = *BuildingType.getUnitData();
			iBuildSpeed = message->popInt16();
			iBuildOff = message->popInt32();

			if ( Data.is_big )
			{
				if ( Vehicle->data.can_build != BUILD_BIG ) break;

				if ( !( Map->possiblePlaceBuilding( Data, iBuildOff                , Vehicle ) &&
						Map->possiblePlaceBuilding( Data, iBuildOff + 1            , Vehicle ) &&
						Map->possiblePlaceBuilding( Data, iBuildOff + Map->size    , Vehicle ) &&
						Map->possiblePlaceBuilding( Data, iBuildOff + Map->size + 1, Vehicle )) )
				{
					sendBuildAnswer ( false, Vehicle->iID, 0, sID(), 0, 0, Vehicle->owner->Nr );
					break;
				}
				Vehicle->BuildBigSavedPos = Vehicle->PosX+Vehicle->PosY*Map->size;

				// set vehicle to build position
				Map->moveVehicleBig( Vehicle, iBuildOff );
				Vehicle->owner->DoScan();
			}
			else
			{
				if ( iBuildOff != Vehicle->PosX+Vehicle->PosY*Map->size ) break;

				if ( !Map->possiblePlaceBuilding( Data, iBuildOff, Vehicle ))
				{
					sendBuildAnswer ( false, Vehicle->iID, 0, sID(), 0, 0, Vehicle->owner->Nr );
					break;
				}
			}

			Vehicle->BuildingTyp = BuildingType;
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
				Vehicle->calcTurboBuild( iTurboBuildRounds, iTurboBuildCosts, BuildingType.getUnitData( Vehicle->owner )->iBuilt_Costs, BuildingType.getUnitData( Vehicle->owner )->iBuilt_Costs_Max );

				Vehicle->BuildCosts = iTurboBuildCosts[iBuildSpeed];
				Vehicle->BuildRounds = iTurboBuildRounds[iBuildSpeed];
				Vehicle->BuildCostsStart = Vehicle->BuildCosts;
				Vehicle->BuildRoundsStart = Vehicle->BuildRounds;
			}

			Vehicle->IsBuilding = true;

			if ( Vehicle->BuildCosts > Vehicle->data.cargo )
			{
				sendBuildAnswer ( false, Vehicle->iID, 0, sID(), 0, 0, Vehicle->owner->Nr );
				break;
			}

			for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
			{
				sendBuildAnswer(true, Vehicle->iID, iBuildOff, BuildingType, Vehicle->BuildRounds, Vehicle->BuildCosts, Vehicle->SeenByPlayerList[i]->Nr );
			}
			sendBuildAnswer ( true, Vehicle->iID, iBuildOff, BuildingType, Vehicle->BuildRounds, Vehicle->BuildCosts, Vehicle->owner->Nr );

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

			addUnit( Vehicle->PosX, Vehicle->PosY, Vehicle->BuildingTyp.getBuilding(), Vehicle->owner );

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
				sendUnitData(Vehicle, Vehicle->SeenByPlayerList[i]->Nr );
			}

			// drive away from the building lot
			cServerMoveJob *MoveJob = new cServerMoveJob( Vehicle->PosX+Vehicle->PosY*Map->size, iEscapeX+iEscapeY*Map->size, false, Vehicle );
			if ( MoveJob->calcPath() )
			{
				sendMoveJobServer ( MoveJob, Vehicle->owner->Nr );
				for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendMoveJobServer( MoveJob, Vehicle->SeenByPlayerList[i]->Nr );
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
			if ( Map->possiblePlaceBuilding(*Vehicle->BuildingTyp.getUnitData(), iNextX, iNextY ) && MoveJob->calcPath() )
			{
				addUnit( Vehicle->PosX, Vehicle->PosY, Vehicle->BuildingTyp.getBuilding(), Vehicle->owner );
				Vehicle->IsBuilding = false;
				Vehicle->BuildPath = false;

				sendUnitData ( Vehicle, Vehicle->owner->Nr );
				sendContinuePathAnswer ( true, Vehicle->iID, Vehicle->owner->Nr );
				sendMoveJobServer ( MoveJob, Vehicle->owner->Nr );
				for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendUnitData(Vehicle, Vehicle->SeenByPlayerList[i]->Nr );
					sendContinuePathAnswer ( true, Vehicle->iID, Vehicle->SeenByPlayerList[i]->Nr );
					sendMoveJobServer( MoveJob, Vehicle->SeenByPlayerList[i]->Nr );
				}
				addActiveMoveJob ( MoveJob );
			}
			else
			{
				delete MoveJob;
				Vehicle->ServerMoveJob = NULL;

				if ( Vehicle->BuildingTyp.getUnitData()->is_base || Vehicle->BuildingTyp.getUnitData()->is_connector )
				{
					addUnit( Vehicle->PosX, Vehicle->PosY, Vehicle->BuildingTyp.getBuilding(), Vehicle->owner );
					Vehicle->IsBuilding = false;
					Vehicle->BuildPath = false;
					sendUnitData ( Vehicle, Vehicle->owner->Nr );
					for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
					{
						sendUnitData(Vehicle, Vehicle->SeenByPlayerList[i]->Nr );
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
				Vehicle->owner->DoScan();
			}
			sendStopBuild ( Vehicle->iID, iPos, Vehicle->owner->Nr );
			for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
			{
				sendStopBuild ( Vehicle->iID, iPos, Vehicle->SeenByPlayerList[i]->Nr );
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
				sID Type;
				Type.iFirstPart = message->popInt16();
				Type.iSecondPart = message->popInt16();

				// if the first unit hasn't changed copy it to the new buildlist
				if ( Building->BuildList->Size() > 0 && i == 0 && Type.getVehicle() == (*Building->BuildList)[0]->typ )
				{
					//recalculate costs, because build speed could have beed changed
					Building->CalcTurboBuild ( iTurboBuildRounds, iTurboBuildCosts, Type.getUnitData ( Building->owner )->iBuilt_Costs, (*Building->BuildList)[0]->metall_remaining );
					sBuildList *BuildListItem = new sBuildList;
					BuildListItem->metall_remaining = iTurboBuildCosts[iBuildSpeed];
					BuildListItem->typ = (*Building->BuildList)[0]->typ;
					NewBuildList->Add ( BuildListItem );
					continue;
				}

				// check whether this building can build this unit
				if ( Type.getUnitData()->can_drive == DRIVE_SEA && !bWater )
					continue;
				else if ( Type.getUnitData()->can_drive == DRIVE_LAND && !bLand )
					continue;

				if ( Building->data.can_build == BUILD_AIR && Type.getUnitData()->can_drive != DRIVE_AIR )
					continue;
				else if ( Building->data.can_build == BUILD_BIG && !Type.getUnitData()->build_by_big )
					continue;
				else if ( Building->data.can_build == BUILD_SEA && Type.getUnitData()->can_drive != DRIVE_SEA )
					continue;
				else if ( Building->data.can_build == BUILD_SMALL && ( Type.getUnitData()->can_drive == DRIVE_AIR || Type.getUnitData()->can_drive == DRIVE_SEA || Type.getUnitData()->build_by_big || Type.getUnitData()->is_human ) )
					continue;
				else if ( Building->data.can_build == BUILD_MAN && !Type.getUnitData()->is_human )
					continue;

				Building->CalcTurboBuild ( iTurboBuildRounds, iTurboBuildCosts, Type.getUnitData( Building->owner )->iBuilt_Costs );

				sBuildList *BuildListItem = new sBuildList;
				BuildListItem->metall_remaining = iTurboBuildCosts[iBuildSpeed];
				BuildListItem->typ = Type.getVehicle();

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

			if ( !Building->canExitTo( iX, iY, Server->Map, BuildingListItem->typ ) ) break;

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
				if (!bDone) break;
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
					sendUnitData ( Vehicle, Vehicle->SeenByPlayerList[i]->Nr );
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
					sendUnitData ( Building, Building->SeenByPlayerList[i]->Nr );
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
				for ( unsigned int i = 0; i < DestVehicle->SeenByPlayerList.Size(); i++)
					sendUnitData( DestVehicle, DestVehicle->SeenByPlayerList[i]->Nr );
			}
			else
			{
				if ( iType == SUPPLY_TYPE_REARM ) DestBuilding->data.ammo = DestBuilding->data.max_ammo;
				else DestBuilding->data.hit_points = DestBuilding->data.max_hit_points;

				sendSupply ( DestBuilding->iID, false, iValue, iType, DestBuilding->owner->Nr );
								
				//send unitdata to the players who are not the owner
				for ( unsigned int i = 0; i < DestBuilding->SeenByPlayerList.Size(); i++)
					sendUnitData( DestBuilding, DestBuilding->SeenByPlayerList[i]->Nr );
			}
		}
		break;
	case GAME_EV_WANT_START_CLEAR:
		{
			int id = message->popInt16();
			cVehicle *Vehicle = getVehicleFromID ( id );
			if ( Vehicle == NULL )
			{
				cLog::write("Server: Can not find vehicle with id " + iToStr ( id ) + " for clearing", LOG_TYPE_NET_WARNING);
				break;
			}

			int off = Vehicle->PosX+Vehicle->PosY*Map->size;
			cBuildingIterator Buildings = (*Map)[off].getBuildings();

			while ( Buildings && Buildings->owner ) Buildings++;

			if ( !Buildings.end )
			{
				int rubbleoffset = -1;
				if ( Buildings->data.is_big )
				{
					rubbleoffset = Buildings->PosX+Buildings->PosY*Map->size;
					if ( ( !Map->possiblePlace ( Vehicle, rubbleoffset ) && rubbleoffset != off ) ||
						( !Map->possiblePlace ( Vehicle, rubbleoffset+1 ) && rubbleoffset+1 != off ) ||
						( !Map->possiblePlace ( Vehicle, rubbleoffset+Map->size ) && rubbleoffset+Map->size != off ) ||
						( !Map->possiblePlace ( Vehicle, rubbleoffset+Map->size+1 ) && rubbleoffset+Map->size+1 != off ) )
					{
						sendClearAnswer ( 1, Vehicle, 0, -1, Vehicle->owner->Nr );
						break;
					}
					else
					{
						Vehicle->BuildBigSavedPos = off;
						Map->moveVehicleBig ( Vehicle, rubbleoffset );
					}
				}

				Vehicle->IsClearing = true;
				Vehicle->ClearingRounds = Buildings->RubbleValue/4+1;

				sendClearAnswer ( 0, Vehicle, Vehicle->ClearingRounds, rubbleoffset, Vehicle->owner->Nr );
				for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++)
				{
					sendClearAnswer( 0, Vehicle, 0, rubbleoffset, Vehicle->SeenByPlayerList[i]->Nr );
				}
			}
			else sendClearAnswer ( 2, Vehicle, 0, -1, Vehicle->owner->Nr );
		}
		break;
	case GAME_EV_WANT_STOP_CLEAR:
		{
			int id = message->popInt16();
			cVehicle *Vehicle = getVehicleFromID ( id );
			if ( Vehicle == NULL )
			{
				cLog::write("Server: Can not find vehicle with id " + iToStr ( id ) + " for stop clearing", LOG_TYPE_NET_WARNING);
				break;
			}

			if ( Vehicle->IsClearing )
			{
				Vehicle->IsClearing = false;
				Vehicle->ClearingRounds = 0;

				if ( Vehicle->data.is_big )
				{
					Map->moveVehicle ( Vehicle, Vehicle->BuildBigSavedPos );
					sendStopClear ( Vehicle, Vehicle->BuildBigSavedPos, Vehicle->owner->Nr );
					for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
					{
						sendStopClear ( Vehicle, Vehicle->BuildBigSavedPos, Vehicle->SeenByPlayerList[i]->Nr );
					}
				}
				else
				{
					sendStopClear ( Vehicle, -1, Vehicle->owner->Nr );
					for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
					{
						sendStopClear ( Vehicle, -1, Vehicle->SeenByPlayerList[i]->Nr );
					}
				}
			}
		}
	case GAME_EV_ABORT_WAITING:
		{
			if ( DisconnectedPlayerList.Size() < 1 ) break; 
			// only server player can abort the waiting
			cPlayer *LocalPlayer;
			for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
			{
				if ( (*PlayerList)[i]->iSocketNum == MAX_CLIENTS )
				{
					LocalPlayer = (*PlayerList)[i];
					break;
				}
			}
			if ( message->iPlayerNr != LocalPlayer->Nr ) break;

			// delete disconnected players
			for ( unsigned int i = 0; i < DisconnectedPlayerList.Size(); i++ )
			{
				deletePlayer ( DisconnectedPlayerList[i] );
				DisconnectedPlayerList.Delete( i );
			}
			sendDefreeze();
		}
		break;
	case GAME_EV_IDENTIFICATION:
		{
			string playerName = message->popString();
			for ( unsigned int i = 0; i < DisconnectedPlayerList.Size(); i++ )
			{
				if ( !playerName.compare ( DisconnectedPlayerList[i]->name ) )
				{
					DisconnectedPlayerList[i]->iSocketNum = message->popInt16();
					sendOKReconnect ( DisconnectedPlayerList[i] );
					break;
				}
			}
		}
		break;
	case GAME_EV_RECON_SUCESS:
		{
			cPlayer *Player = NULL;
			int playerNum = message->popInt16();
			// remove the player from the disconnected list
			for ( unsigned int i = 0; i < DisconnectedPlayerList.Size(); i++ )
			{
				if ( DisconnectedPlayerList[i]->Nr == playerNum )
				{
					Player = DisconnectedPlayerList[i];
					DisconnectedPlayerList.Delete ( i );
					break;
				}
			}
			resyncPlayer ( Player );

			sendDefreeze ();
		}
		break;
	case GAME_EV_WANT_LOAD:
		{
			cVehicle *StoredVehicle = getVehicleFromID ( message->popInt16() );
			if ( !StoredVehicle ) break;

			if ( message->popBool() )
			{
				cVehicle *StoringVehicle = getVehicleFromID ( message->popInt16() );
				if ( !StoringVehicle ) break;

				if ( StoringVehicle->canLoad ( StoredVehicle ) )
				{
					StoringVehicle->storeVehicle ( StoredVehicle, Map );
					if ( StoredVehicle->ServerMoveJob ) StoredVehicle->ServerMoveJob->release();
					sendStoreVehicle ( StoringVehicle->iID, true, StoredVehicle->iID, StoringVehicle->owner->Nr );
					// TODO: other players
				}
			}
			else
			{
				cBuilding *StoringBuilding = getBuildingFromID ( message->popInt16() );
				if ( !StoringBuilding ) break;

				if ( StoringBuilding->canLoad ( StoredVehicle ) )
				{
					StoringBuilding->storeVehicle ( StoredVehicle, Map );
					if ( StoredVehicle->ServerMoveJob ) StoredVehicle->ServerMoveJob->release();
					sendStoreVehicle ( StoringBuilding->iID, false, StoredVehicle->iID, StoringBuilding->owner->Nr );
					// TODO: other players
				}
			}
		}
		break;
	case GAME_EV_WANT_EXIT:
		{
			cVehicle *StoredVehicle = getVehicleFromID ( message->popInt16() );
			if ( !StoredVehicle ) break;

			if ( message->popBool() )
			{
				cVehicle *StoringVehicle = getVehicleFromID ( message->popInt16() );
				if ( !StoringVehicle ) break;

				int x = message->popInt16 ();
				int y = message->popInt16 ();
				if ( StoringVehicle->canExitTo ( x, y, Server->Map, StoredVehicle->typ ) )
				{
					StoringVehicle->exitVehicleTo ( StoredVehicle, x+y*Map->size, Map );
					// TODO: other players
					sendActivateVehicle ( StoringVehicle->iID, true, StoredVehicle->iID, x, y, StoringVehicle->owner->Nr );
					StoredVehicle->InSentryRange();
				}
			}
			else
			{
				cBuilding *StoringBuilding = getBuildingFromID ( message->popInt16() );
				if ( !StoringBuilding ) break;

				int x = message->popInt16 ();
				int y = message->popInt16 ();
				if ( StoringBuilding->canExitTo ( x, y, Server->Map, StoredVehicle->typ ) )
				{
					StoringBuilding->exitVehicleTo ( StoredVehicle, x+y*Map->size, Map );
					// TODO: other players
					sendActivateVehicle ( StoringBuilding->iID, false, StoredVehicle->iID, x, y, StoringBuilding->owner->Nr );
				}
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

void cServer::makeLanding( int iX, int iY, cPlayer *Player, const cList<sLanding*>& List, bool bFixed )
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
	for ( unsigned int i = 0; i < List.Size(); i++ )
	{
		Landing = List[i];
		Vehicle = landVehicle(iX, iY, iWidth, iHeight, Landing->UnitID.getVehicle(), Player);
		while ( !Vehicle )
		{
			iWidth += 2;
			iHeight += 2;
			Vehicle = landVehicle(iX, iY, iWidth, iHeight, Landing->UnitID.getVehicle(), Player);
		}
		if ( Landing->cargo && Vehicle )
		{
			Vehicle->data.cargo = Landing->cargo;
			sendUnitData ( Vehicle, Vehicle->owner->Nr );
		}
	}
}

cVehicle * cServer::addUnit( int iPosX, int iPosY, sVehicle *Vehicle, cPlayer *Player, bool bInit, bool bAddToMap )
{
	cVehicle *AddedVehicle;
	// generate the vehicle:
	AddedVehicle = Player->AddVehicle ( iPosX, iPosY, Vehicle );
	AddedVehicle->iID = iNextUnitID;
	iNextUnitID++;
	
	// place the vehicle:
	if ( bAddToMap ) Map->addVehicle( AddedVehicle, iPosX, iPosY );

	// scan with surveyor:
	if ( AddedVehicle->data.can_survey )
	{
		sendVehicleResources( AddedVehicle, Map );
		AddedVehicle->doSurvey();
	}
	if ( !bInit ) AddedVehicle->InSentryRange();

	sendAddUnit ( iPosX, iPosY, AddedVehicle->iID, true, Vehicle->data.ID, Player->Nr, bInit );

	//detection must be done, after the vehicle has been sent to clients
	AddedVehicle->makeDetection();
	return AddedVehicle;
}

cBuilding * cServer::addUnit( int iPosX, int iPosY, sBuilding *Building, cPlayer *Player, bool bInit )
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

	sendAddUnit ( iPosX, iPosY, AddedBuilding->iID, false, Building->data.ID, Player->Nr, bInit );
	if ( AddedBuilding->data.is_mine ) sendProduceValues ( AddedBuilding );
	// integrate the building to the base:
	Player->base.AddBuilding ( AddedBuilding );
	AddedBuilding->makeDetection();
	return AddedBuilding;
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
		for ( unsigned int i = 0; i < AJobs.Size(); i++ )
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
		for ( unsigned int i = 0; i < AJobs.Size(); i++ )
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
						if ( NextVehicle->SeenByPlayerList[i] == MapPlayer ) break;
					}
					if ( i == NextVehicle->SeenByPlayerList.Size() )
					{
						NextVehicle->SeenByPlayerList.Add ( MapPlayer );
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
						if ( NextVehicle->SeenByPlayerList[i] == MapPlayer )
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
						if ( NextBuilding->SeenByPlayerList[i] == MapPlayer ) break;
					}
					if ( i == NextBuilding->SeenByPlayerList.Size() )
					{
						NextBuilding->SeenByPlayerList.Add ( MapPlayer );
						sendAddEnemyUnit( NextBuilding, MapPlayer->Nr );
						sendUnitData( NextBuilding, MapPlayer->Nr );
					}
				}
				else
				{
					unsigned int i;
					for ( i = 0; i < NextBuilding->SeenByPlayerList.Size(); i++ )
					{
						if ( NextBuilding->SeenByPlayerList[i] == MapPlayer )
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
					if ( building->SeenByPlayerList[i] == MapPlayer ) break;
				}
				if ( i == building->SeenByPlayerList.Size() )
				{
					building->SeenByPlayerList.Add ( MapPlayer );
					sendAddRubble( building, MapPlayer->Nr );
				}
			}
			else
			{
				unsigned int i;
				for ( i = 0; i < building->SeenByPlayerList.Size(); i++ )
				{
					if ( building->SeenByPlayerList[i] == MapPlayer )
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
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
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
		if ( iActiveTurnPlayerNr >= (int)PlayerList->Size() )
		{
			iActiveTurnPlayerNr = 0;
			if ( iGameType == GAME_TYPE_HOTSEAT )
			{
				sendMakeTurnEnd(true, bWaitForPlayer, (*PlayerList)[iActiveTurnPlayerNr]->Nr, iPlayerNum);
			}
			else
			{
				for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
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
				for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
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
		for ( unsigned int i = 0; i < PlayerEndList.Size(); i++ )
		{
			if (PlayerEndList[i]->Nr == iPlayerNum) return;
		}
		PlayerEndList.Add ( getPlayerFromNumber ( iPlayerNum ) );

		// make sure that all defeated players are added to the endlist
		for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
		{
			if ( (*PlayerList)[i]->isDefeated )
			{
				bool isAdded = false;
				for ( unsigned int j = 0; j < PlayerEndList.Size(); j++ ) if ( PlayerEndList[j] == (*PlayerList)[i] ) isAdded = true;
				if ( !isAdded ) PlayerEndList.Add ( (*PlayerList)[i] );
			}
		}

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
			for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
			{
				sendMakeTurnEnd ( true, false, -1, i );
			}

			iTurn++;
			makeTurnEnd ();
			// send reports to all players
			for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
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
						sendMoveJobServer ( NextVehicle->ServerMoveJob, NextVehicle->SeenByPlayerList[j]->Nr );
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
						sendUnitData(Building, Building->SeenByPlayerList[k]->Nr );
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
					sendUnitData(Building, Building->SeenByPlayerList[k]->Nr );
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
						sendUnitData(Vehicle, Vehicle->SeenByPlayerList[k]->Nr );
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
					sendUnitData(Vehicle, Vehicle->SeenByPlayerList[k]->Nr );
				}
				sendUnitData ( Vehicle, Vehicle->owner->Nr );
			}

			if ( Vehicle->ServerMoveJob ) Vehicle->ServerMoveJob->bEndForNow = false;
			Vehicle = Vehicle->next;
		}
	}

	//hide stealth units
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
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


	// make autosave
	if ( SettingsData.bAutoSave )
	{
		cSavegame Savegame ( 10 );	// autosaves are always in slot 10
		Savegame.save ( "Autosave" );
	}

	checkDefeats();

	iWantPlayerEndNum = -1;
}

void cServer::checkDefeats ()
{
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
	{
		cPlayer *Player = (*PlayerList)[i];
		if ( !Player->isDefeated )
		{
			cBuilding *Building = Player->BuildingList;
			cVehicle *Vehicle = Player->VehicleList;
			while ( Vehicle )
			{
				if ( Vehicle->data.can_attack || 
					Vehicle->data.can_build ) break;
				Vehicle = Vehicle->next;
			}
			if ( Vehicle != NULL ) continue;
			while ( Building )
			{
				if ( !Building->data.is_bridge &&
					!Building->data.is_connector &&
					!Building->data.is_expl_mine &&
					!Building->data.is_pad &&
					!Building->data.is_platform &&
					!Building->data.is_road ) break;
				Building = Building->next;
			}
			if ( Building != NULL ) continue;

			Player->isDefeated = true;
			sendDefeated ( Player );

			if ( openMapDefeat && Player->iSocketNum != -1 )
			{
				memset ( Player->ScanMap, 1, Map->size*Map->size );
				checkPlayerUnits();
				sendNoFog ( Player->Nr );
			}
		}
	}
}

void cServer::addReport ( sID Type, bool bVehicle, int iPlayerNum )
{
	sTurnstartReport *Report;
	cPlayer *Player = getPlayerFromNumber ( iPlayerNum );
	if ( bVehicle )
	{
		for ( unsigned int i = 0; i < Player->ReportVehicles.Size(); i++ )
		{
			Report = Player->ReportVehicles[i];
			if ( Report->Type == Type )
			{
				Report->iAnz++;
				return;
			}
		}
		Report = new sTurnstartReport;
		Report->Type = Type;
		Report->iAnz = 1;
		Player->ReportVehicles.Add ( Report );
	}
	else
	{
		for ( unsigned int i = 0; i < Player->ReportBuildings.Size(); i++ )
		{
			Report = Player->ReportBuildings[i];
			if ( Report->Type == Type )
			{
				Report->iAnz++;
				return;
			}
		}
		Report = new sTurnstartReport;
		Report->Type = Type;
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

			for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
			{
				sendMakeTurnEnd ( true, false, -1, i );
			}
			iTurn++;
			iDeadlineStartTime = 0;
			makeTurnEnd();
			for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
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
	for ( unsigned int i = 0; i < ActiveMJobs.Size(); i++ )
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
				for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendNextMove( Vehicle->iID, Vehicle->PosX+Vehicle->PosY * Map->size, MJOB_STOP, Vehicle->SeenByPlayerList[i]->Nr );
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

					for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
					{
						sendNextMove( Vehicle->iID, Vehicle->PosX+Vehicle->PosY * Map->size, MJOB_FINISHED, Vehicle->SeenByPlayerList[i]->Nr );
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
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
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
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
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

void cServer::destroyUnit( cVehicle* vehicle )
{
	int offset = vehicle->PosX + vehicle->PosY*Map->size;
	int value = vehicle->data.iBuilt_Costs;

	//delete all buildings on the field, except connectors
	cBuildingIterator bi = (*Map)[offset].getBuildings();
	if ( bi && bi->data.is_connector ) bi++;
	
	while ( !bi.end )
	{
		if (!bi->owner) 
		{
			bi++;
			continue;
		}
		value += bi->data.iBuilt_Costs;
		deleteUnit( bi, false );
		bi++;
	}

	if ( vehicle->data.is_big )
	{
		bi = (*Map)[offset + 1].getBuildings();
		if ( bi && bi->data.is_connector ) bi++;
		while ( !bi.end )
		{
			value += bi->data.iBuilt_Costs;
			deleteUnit( bi, false );
			bi++;
		}

		bi = (*Map)[offset + Map->size].getBuildings();
		if ( bi && bi->data.is_connector ) bi++;
		while ( !bi.end )
		{
			value += bi->data.iBuilt_Costs;
			deleteUnit( bi, false );
			bi++;
		}

		bi = (*Map)[offset + 1 + Map->size].getBuildings();
		if ( bi && bi->data.is_connector ) bi++;
		while ( !bi.end )
		{
			value += bi->data.iBuilt_Costs;
			deleteUnit( bi, false );
			bi++;
		}
	}

	if ( (vehicle->data.can_drive != DRIVE_AIR || vehicle->FlightHigh == 0) && !vehicle->data.is_human )
	{
		addRubble( offset, value/2, vehicle->data.is_big );
	}

	deleteUnit( vehicle, false );

	
}

void cServer::destroyUnit(cBuilding *building)
{
	int offset = building->PosX + building->PosY * Map->size;
	int value = 0;
	bool big = false;
	bool isConnector = building->data.is_connector;

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

	if ( !isConnector || value > 2 )
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
		if ( rubble->next ) rubble->next->prev = NULL;
	}
	else
	{
		rubble->prev->next = rubble->next;
		if ( rubble->next )
			rubble->next->prev = rubble->prev;
	}
	sendDeleteUnit( rubble, -1 );

	delete rubble;

}

void cServer::deletePlayer( cPlayer *Player )
{
	cVehicle *Vehicle = Player->VehicleList;
	while ( Vehicle )
	{
		deleteUnit ( Vehicle );
		Vehicle = Player->VehicleList;
	}
	cBuilding *Building = Player->BuildingList;
	while ( Building )
	{
		deleteUnit ( Building );
		Building = Player->BuildingList;
	}
	sendDeletePlayer ( Player );
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
	{
		if ( Player == (*PlayerList)[i] )
		{
			PlayerList->Delete ( i );
			delete Player;
		}
	}
}

void cServer::resyncPlayer ( cPlayer *Player, bool firstDelete )
{
	if ( firstDelete ) sendDeleteEverything ( Player->Nr );
	sendTurn ( iTurn, Player );
	sendResources ( Player );
	// send all units to the client
	cVehicle *Vehicle = Player->VehicleList;
	while ( Vehicle )
	{
		if ( !Vehicle->Loaded ) resyncVehicle ( Vehicle, Player );
		Vehicle = Vehicle->next;
	}
	cBuilding *Building = Player->BuildingList;
	while ( Building )
	{
		sendAddUnit ( Building->PosX, Building->PosY, Building->iID, false, Building->data.ID, Player->Nr, false );
		for ( unsigned int i = 0; i < Building->StoredVehicles.Size(); i++ )
		{
			cVehicle *StoredVehicle = Building->StoredVehicles[i];
			resyncVehicle ( StoredVehicle, Player );
			sendStoreVehicle ( Building->iID, false, StoredVehicle->iID, Player->Nr );
		}
		sendUnitData ( Building, Player->Nr );
		if ( Building->IsWorking && Building->data.is_mine ) sendProduceValues ( Building );
		if ( Building->BuildList && Building->BuildList->Size() > 0 ) sendBuildList ( Building );
		Building = Building->next;
		if ( Client ) EventHandler->HandleEvents ();
	}
	// send all subbases
	for ( unsigned int i = 0; i < Player->base.SubBases.Size(); i++ )
	{
		sendNewSubbase ( Player->base.SubBases[i], Player->Nr );
		sendAddSubbaseBuildings ( NULL, Player->base.SubBases[i], Player->Nr );
		sendSubbaseValues ( Player->base.SubBases[i], Player->Nr );
		if ( Client ) EventHandler->HandleEvents ();
	}
	// refresh enemy units
	Player->DoScan();
	checkPlayerUnits();
	// FIXME: sending hudsettings doesn't work form yet
	//sendHudSettings ( &Player->HotHud, Player );
	// TODO: send upgrades
}

void cServer::resyncVehicle ( cVehicle *Vehicle, cPlayer *Player )
{
	sendAddUnit ( Vehicle->PosX, Vehicle->PosY, Vehicle->iID, true, Vehicle->data.ID, Player->Nr, false, !Vehicle->Loaded );
	if ( Vehicle->ServerMoveJob ) sendMoveJobServer ( Vehicle->ServerMoveJob, Player->Nr );
	for ( unsigned int i = 0; i < Vehicle->StoredVehicles.Size(); i++ )
	{
		cVehicle *StoredVehicle = Vehicle->StoredVehicles[i];
		resyncVehicle ( StoredVehicle, Player );
		sendStoreVehicle ( Vehicle->iID, true, StoredVehicle->iID, Player->Nr );
	}
	sendUnitData ( Vehicle, Player->Nr );
	sendSpecificUnitData ( Vehicle );
	if ( Client ) EventHandler->HandleEvents ();
}
