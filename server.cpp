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
	Map = map;
	this->PlayerList = PlayerList;
	this->iGameType = iGameType;
	this->bPlayTurns = bPlayTurns;
	bExit = false;
	bStarted = false;
	iActiveTurnPlayerNr = 0;
	iTurn = 1;
	iDeadlineStartTime = 0;
	iTurnDeadline = 10; // just temporary set to 10 seconds
	iNextUnitID = 1;
	iTimerTime = 0;
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

	cLog::write("Server: <-- " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );

	if ( iPlayerNum == -1 )
	{
		EventHandler->pushEvent( message->getGameEvent() );
		if ( network ) network->send( message->iLength, message->serialize( true ) );
		delete message;
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
		if ( network ) network->sendTo( Player->iSocketNum, message->iLength, message->serialize( true ) );
	}
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
						SDL_Event* NewEvent = new SDL_Event;
						NewEvent->type = GAME_EVENT;

						// data1 is the real data
						NewEvent->user.data1 = malloc ( MAX_MESSAGE_LENGTH );
						memcpy ( NewEvent->user.data1, &((char*)event->user.data1)[2], MAX_MESSAGE_LENGTH );

						NewEvent->user.data2 = NULL;
						pushEvent( NewEvent );
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
					message.refertControlChars();
					HandleNetMessage( &message );
					break;
				}

			default:
				break;
			}
		}

		// don't do anything if games hasn't been started yet!
		if ( !bStarted ) { SDL_Delay( 10 ); continue; }

		checkPlayerUnits();

		checkDeadline();

		handleMoveJobs ();

		handleTimer();

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
		handleEnd ( message->iPlayerNr );
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
			cMJobs *MJob = NULL;
			int iCount = 0;
			int iWaypointOff;

			int iID = message->popInt16();
			int iSrcOff = message->popInt32();
			int iDestOff = message->popInt32();
			bool bPlane = message->popBool();
			int iReceivedCount = message->popInt16();

			cLog::write(" Server: Received MoveJob: VehicleID: " + iToStr( iID ) + ", SrcX: " + iToStr( iSrcOff%Map->size ) + ", SrcY: " + iToStr( iSrcOff/Map->size ) + ", DestX: " + iToStr( iDestOff%Map->size ) + ", DestY: " + iToStr( iDestOff/Map->size ) + ", WaypointCount: " + iToStr( iReceivedCount ), cLog::eLOG_TYPE_NET_DEBUG);
			// Add the waypoints to the movejob
			sWaypoint *Waypoint = ( sWaypoint* ) malloc ( sizeof ( sWaypoint ) );
			while ( iCount < iReceivedCount )
			{
				iWaypointOff = message->popInt32();
				Waypoint->X = iWaypointOff%Map->size;
				Waypoint->Y = iWaypointOff/Map->size;
				Waypoint->Costs = message->popInt16();
				Waypoint->next = NULL;

				if ( iCount == 0 )
				{
					MJob = new cMJobs( Map, iSrcOff, iDestOff, bPlane, iID, PlayerList, true );
					if ( MJob->vehicle == NULL )
					{
						// warning, here is something wrong! ( out of sync? )
						cLog::write(" Server: Created new movejob but no vehicle found!", cLog::eLOG_TYPE_NET_WARNING);
						delete MJob; MJob = NULL;
						break;
					}
					MJob->waypoints = Waypoint;

					// unset sentry status when moving vehicle
					if ( MJob->vehicle->bSentryStatus )
					{
						MJob->vehicle->owner->deleteSentryVehicle ( MJob->vehicle );
						MJob->vehicle->bSentryStatus = false;
					}
					sendUnitData ( MJob->vehicle, MJob->vehicle->owner->Nr );
					for ( unsigned int i = 0; i < MJob->vehicle->SeenByPlayerList.Size(); i++ )
					{
						sendUnitData ( MJob->vehicle, *MJob->vehicle->SeenByPlayerList[i] );
					}
				}
				iCount++;

				if ( iCount < iReceivedCount )
				{
					Waypoint->next = ( sWaypoint* ) malloc ( sizeof ( sWaypoint ) );
					Waypoint = Waypoint->next;
				}
			}
			//if the vehicle is under attack, cancel the movejob
			if ( MJob && MJob->vehicle->bIsBeeingAttacked )
			{
				cLog::write(" Server: cannot move a vehicle currently under attack", cLog::eLOG_TYPE_NET_DEBUG );
				sendNextMove( iID, iSrcOff, MJOB_FINISHED, MJob->vehicle->owner->Nr );
			}

			if ( MJob )
			{
				MJob->CalcNextDir();
				addActiveMoveJob ( MJob );
				cLog::write(" Server: Added received movejob", cLog::eLOG_TYPE_NET_DEBUG);
				// send the movejob to all other player who can see this unit
				for ( int i = 0; i < MJob->vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendMoveJobServer(MJob, *MJob->vehicle->SeenByPlayerList[i]);
				}
			}
		}
		break;
	case GAME_EV_WANT_STOP_MOVE:
		{
			cVehicle *Vehicle = getVehicleFromID ( message->popInt16() );
			if ( Vehicle == NULL ) break;
			if ( Vehicle->mjob == NULL ) break;
			
			Vehicle->mjob->release();
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
				cLog::write( "Server: attacking vehicle " + targetVehicle->name + ", " + iToStr(targetVehicle->iID), cLog::eLOG_TYPE_NET_DEBUG );
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
				Vehicle->PosX = iBuildOff%Map->size;
				Vehicle->PosY = iBuildOff/Map->size;
				Map->GO[iBuildOff].vehicle = Vehicle;
				Map->GO[iBuildOff+1].vehicle = Vehicle;
				Map->GO[iBuildOff+Map->size].vehicle = Vehicle;
				Map->GO[iBuildOff+Map->size+1].vehicle = Vehicle;
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
				calcBuildRoundsAndCosts ( Vehicle, iBuildingType, iTurboBuildRounds, iTurboBuildCosts );

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

			if ( Vehicle->data.can_build == BUILD_BIG )
			{
				Map->GO[Vehicle->PosX+1+Vehicle->PosY*Map->size].vehicle = NULL;
				Map->GO[Vehicle->PosX+1+ ( Vehicle->PosY+1 )*Map->size].vehicle = NULL;
				Map->GO[Vehicle->PosX+ ( Vehicle->PosY+1 )*Map->size].vehicle = NULL;
			}
			addUnit( Vehicle->PosX, Vehicle->PosY, &UnitsData.building[Vehicle->BuildingTyp], Vehicle->owner );

			// set the vehicle to the border
			if ( Vehicle->data.can_build == BUILD_BIG )
			{
				Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].vehicle = NULL;
				if ( abs ( iEscapeX - ( Vehicle->PosX+1 ) ) <= 1 && abs ( iEscapeY - Vehicle->PosY ) <= 1 )
				{
					Vehicle->PosX++;
				}
				else if ( abs ( iEscapeX -( Vehicle->PosX+1 ) ) <= 1 && abs ( iEscapeY - ( Vehicle->PosY+1 ) ) <= 1 )
				{
					Vehicle->PosX++;
					Vehicle->PosY++;
				}
				else if ( abs ( iEscapeX - Vehicle->PosX ) <= 1 && abs ( iEscapeY - ( Vehicle->PosY+1 ) ) <= 1 )
				{
					Vehicle->PosY++;
				}
				Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].vehicle = Vehicle;
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
			cMJobs *MJob = new cMJobs( Map, Vehicle->PosX+Vehicle->PosY*Map->size, iEscapeX+iEscapeY*Map->size, false, Vehicle->iID, PlayerList, true );
			if ( MJob->CalcPath() )
			{
				MJob->CalcNextDir();
				sendMoveJobServer ( MJob, Vehicle->owner->Nr );
				for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendMoveJobServer(MJob, *Vehicle->SeenByPlayerList[i]);
				}
				addActiveMoveJob ( MJob );
			}
			else
			{
				delete MJob;
				Vehicle->mjob = NULL;
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
			cMJobs *MJob = new cMJobs( Map, Vehicle->PosX+Vehicle->PosY*Map->size, iNextX+iNextY*Map->size, false, Vehicle->iID, PlayerList, true );
			if ( Vehicle->checkPathBuild ( iNextX+iNextY*Map->size, Vehicle->BuildingTyp, Map ) && MJob->CalcPath() )
			{
				MJob->CalcNextDir();

				addUnit( Vehicle->PosX, Vehicle->PosY, &UnitsData.building[Vehicle->BuildingTyp], Vehicle->owner );
				Vehicle->IsBuilding = false;
				Vehicle->BuildPath = false;

				sendUnitData ( Vehicle, Vehicle->owner->Nr );
				sendContinuePathAnswer ( true, Vehicle->iID, Vehicle->owner->Nr );
				sendMoveJobServer ( MJob, Vehicle->owner->Nr );
				for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendUnitData(Vehicle, *Vehicle->SeenByPlayerList[i]);
					sendContinuePathAnswer ( true, Vehicle->iID, *Vehicle->SeenByPlayerList[i]);
					sendMoveJobServer(MJob, *Vehicle->SeenByPlayerList[i]);
				}
				addActiveMoveJob ( MJob );
			}
			else
			{
				delete MJob;
				Vehicle->mjob = NULL;

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

			int iOldPos = Vehicle->PosX+Vehicle->PosY*Map->size;
			int iNewPos = Vehicle->PosX+Vehicle->PosY*Map->size;
			if ( Vehicle->IsBuilding )
			{
				Vehicle->IsBuilding = false;
				Vehicle->BuildPath = false;

				if ( Vehicle->data.can_build == BUILD_BIG )
				{
					Map->GO[iOldPos].vehicle = NULL;
					Map->GO[iOldPos+1].vehicle = NULL;
					Map->GO[iOldPos+Map->size].vehicle = NULL;
					Map->GO[iOldPos+Map->size+1].vehicle = NULL;
					Map->GO[Vehicle->BuildBigSavedPos].vehicle = Vehicle;
					Vehicle->PosX = Vehicle->BuildBigSavedPos % Map->size;
					Vehicle->PosY = Vehicle->BuildBigSavedPos / Map->size;

					iNewPos = Vehicle->BuildBigSavedPos;
				}
				sendStopBuild ( Vehicle->iID, iOldPos, iNewPos, Vehicle->owner->Nr );
				for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendStopBuild ( Vehicle->iID, iOldPos, iNewPos, *Vehicle->SeenByPlayerList[i] );
				}
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

			while ( Building->BuildList->Size() )
			{
				delete (*Building->BuildList)[0];
				Building->BuildList->Delete( 0 );
			}

			int iCount = message->popInt16();
			for ( int i = 0; i < iCount; i++ )
			{
				int iType = message->popInt16();

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

				Building->BuildList->Add( BuildListItem );
			}


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
			int iY = message->popInt16();

			BuildingListItem = (*Building->BuildList)[0];

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
	default:
		cLog::write("Server: Can not handle message, type " + iToStr(message->iType), cLog::eLOG_TYPE_NET_ERROR);
	}

	return 0;
}

bool cServer::freeForLanding ( int iX, int iY )
{
	if ( iX < 0 || iX >= Map->size || iY < 0 || iY >= Map->size ||
	        Map->GO[iX+iY*Map->size].vehicle ||
	        Map->GO[iX+iY*Map->size].top ||
	        Map->IsWater ( iX+iY*Map->size,false ) ||
	        Map->terrain[Map->Kacheln[iX+iY*Map->size]].blocked )
	{
		return false;
	}
	return true;
}

cVehicle *cServer::landVehicle ( int iX, int iY, int iWidth, int iHeight, sVehicle *Vehicle, cPlayer *Player )
{
	cVehicle *VehcilePtr = NULL;
	for ( int i = -iHeight / 2; i < iHeight / 2; i++ )
	{
		for ( int k = -iWidth / 2; k < iWidth / 2; k++ )
		{
			if ( !freeForLanding ( iX+k,iY+i ) )
			{
				continue;
			}
			addUnit ( iX+k, iY+i, Vehicle, Player, true );
			VehcilePtr = Map->GO[iX+k+ ( iY+i ) *Map->size].vehicle;
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
		cBuilding *Building;
		iWidth = 2;
		iHeight = 2;
		while ( !bPlaced )
		{
			for ( int i = -iHeight / 2; i < iHeight / 2; i++ )
			{
				for ( int k = -iWidth / 2; k < iWidth / 2; k++ )
				{
					if ( freeForLanding ( iX+k,iY+i ) && freeForLanding ( iX+k+1,iY+i ) && freeForLanding ( iX+k+2,iY+i ) &&
					        freeForLanding ( iX+k,iY+i+1 ) && freeForLanding ( iX+k+1,iY+i+1 ) && freeForLanding ( iX+k+2,iY+i+1 ) )
					{
						bPlaced = true;
						// Rohstoffe platzieren:
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
						addUnit(iX + k,     iY + i,     &UnitsData.building[BNrOilStore], Player, true);
						addUnit(iX + k,     iY + i + 1, &UnitsData.building[BNrSmallGen], Player, true);
						addUnit(iX + k + 1, iY + i,     &UnitsData.building[BNrMine],     Player, true);
						Building = Map->GO[iX+k+ ( iY+i ) *Map->size].top;
						Player->base.AddOil ( Building->SubBase, 4 );
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
	if ( AddedVehicle->data.can_drive != DRIVE_AIR )
	{
		int iOff = iPosX+Map->size*iPosY;
		if ( Map->GO[iOff].vehicle == NULL ) Map->GO[iOff].vehicle = AddedVehicle;
	}
	else
	{
		int iOff = iPosX+Map->size*iPosY;
		if ( Map->GO[iOff].plane == NULL ) Map->GO[iOff].plane = AddedVehicle;
	}
	// startup:
	if ( !bInit ) AddedVehicle->StartUp = 10;
	// scan with surveyor:
	if ( AddedVehicle->data.can_survey )
	{
		sendResources( AddedVehicle, Map );
		AddedVehicle->doSurvey();
	}
	if ( !bInit ) AddedVehicle->InSentryRange();

	// if this is not a commando unit the vehicle doesn't need to be detected
	if ( !AddedVehicle->data.is_commando )
	{
		for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
		{
			AddedVehicle->DetectedByPlayerList.Add(&(*PlayerList)[i]->Nr);
		}
	}

	sendAddUnit ( iPosX, iPosY, AddedVehicle->iID, true, Vehicle->nr, Player->Nr, bInit );
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
	// place the building:
	int iOff = iPosX + Map->size*iPosY;
	if ( AddedBuilding->data.is_base )
	{
		if(Map->GO[iOff].base)
		{
			//TODO: delete subbase building, if there is one
			Map->GO[iOff].subbase = Map->GO[iOff].base;
			Map->GO[iOff].base = AddedBuilding;
		}
		else
		{
			Map->GO[iOff].base = AddedBuilding;
		}
	}
	else
	{
		if ( AddedBuilding->data.is_big )
		{
			Map->GO[iOff].top;
			Map->GO[iOff+1].top;
			Map->GO[iOff+Map->size].top;
			Map->GO[iOff+Map->size+1].top;
			deleteUnit ( Map->GO[iOff].top );
			Map->GO[iOff].top=AddedBuilding;
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteUnit ( Map->GO[iOff].base );
				Map->GO[iOff].base = NULL;
			}
			iOff++;
			deleteUnit ( Map->GO[iOff].top );
			Map->GO[iOff].top=AddedBuilding;
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteUnit ( Map->GO[iOff].base );
				Map->GO[iOff].base=NULL;
			}
			iOff+=Map->size;
			deleteUnit ( Map->GO[iOff].top );
			Map->GO[iOff].top=AddedBuilding;
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteUnit ( Map->GO[iOff].base );
				Map->GO[iOff].base=NULL;
			}
			iOff--;
			deleteUnit ( Map->GO[iOff].top );
			Map->GO[iOff].top=AddedBuilding;
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteUnit ( Map->GO[iOff].base );
				Map->GO[iOff].base=NULL;
			}
		}
		else
		{
			deleteUnit ( Map->GO[iOff].top );
			Map->GO[iOff].top=AddedBuilding;
			if ( !AddedBuilding->data.is_connector&&Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteUnit ( Map->GO[iOff].base );
				Map->GO[iOff].base=NULL;
			}
		}
	}
	if ( !bInit ) AddedBuilding->StartUp=10;
	// if this is not an explode mine the building doesn't need to be detected
	if ( !AddedBuilding->data.is_expl_mine )
	{
		for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
		{
			AddedBuilding->DetectedByPlayerList.Add(&(*PlayerList)[i]->Nr);
		}
	}

	sendAddUnit ( iPosX, iPosY, AddedBuilding->iID, false, Building->nr, Player->Nr, bInit );
	if ( AddedBuilding->data.is_mine ) sendProduceValues ( AddedBuilding );
	// intigrate the building to the base:
	Player->base.AddBuilding ( AddedBuilding );
}

void cServer::deleteUnit( cBuilding *Building, bool notifyClient )
{


	if( !Building ) return;

	if ( !Building->owner ) 
	{
		Map->deleteRubble( Building );
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

	bool bBase, bSubBase;
	if ( Map->GO[Building->PosX+Building->PosY*Map->size].base == Building )
	{
		Map->GO[Building->PosX+Building->PosY*Map->size].base = NULL;
		bBase = true;
	}
	else bBase = false;
	if ( Map->GO[Building->PosX+Building->PosY*Map->size].subbase == Building )
	{
		Map->GO[Building->PosX+Building->PosY*Map->size].subbase = NULL;
		bSubBase = true;
	}
	else bSubBase = false;
	if ( !bBase && !bSubBase )
	{
		Map->GO[Building->PosX+Building->PosY*Map->size].top = NULL;
		if ( Building->data.is_big )
		{
			Map->GO[Building->PosX+Building->PosY*Map->size+1].top = NULL;
			Map->GO[Building->PosX+Building->PosY*Map->size+Map->size].top = NULL;
			Map->GO[Building->PosX+Building->PosY*Map->size+Map->size+1].top = NULL;
		}
	}

	if ( notifyClient ) sendDeleteUnit( Building, -1 );

	cPlayer* owner = Building->owner;
	delete Building;

	if ( owner ) owner->DoScan();
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

	int offset = vehicle->PosX + vehicle->PosY * Map->size;

	if ( vehicle->data.can_drive == DRIVE_AIR ) Map->GO[offset].plane = NULL;
	else if ( vehicle->IsBuilding && vehicle->data.can_build == BUILD_BIG )
	{
		Map->GO[offset			 	  ].vehicle = NULL;
		Map->GO[offset + 1		 	  ].vehicle = NULL;
		Map->GO[offset + Map->size 	  ].vehicle = NULL;
		Map->GO[offset + Map->size + 1].vehicle = NULL;
	}
	else
		Map->GO[offset].vehicle = NULL;

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
				if ( MapPlayer->ScanMap[iOff] == 1 &&
					( !NextVehicle->data.is_stealth_land || Map->terrain[Map->Kacheln[iOff]].water || MapPlayer->DetectLandMap[iOff] == 1 ) &&
					( !NextVehicle->data.is_stealth_sea || !Map->terrain[Map->Kacheln[iOff]].water || MapPlayer->DetectSeaMap[iOff] == 1 ) &&
					NextVehicle->isDetectedByPlayer( MapPlayer->Nr ) )
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
						if ( NextVehicle->mjob ) sendMoveJobServer ( NextVehicle->mjob, MapPlayer->Nr );
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

							bool bPlane;
							if ( Map->GO[NextVehicle->PosX+NextVehicle->PosY*Map->size].plane == NextVehicle ) bPlane = true;
							else bPlane = false;
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
				if ( MapPlayer->ScanMap[NextBuilding->PosX+NextBuilding->PosY*Map->size] == 1  &&
					NextBuilding->isDetectedByPlayer( MapPlayer->Nr ) )
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

							bool bBase, bSubBase;
							if ( Map->GO[NextBuilding->PosX+NextBuilding->PosY*Map->size].base == NextBuilding ) bBase = true;
							else bBase = false;
							if ( Map->GO[NextBuilding->PosX+NextBuilding->PosY*Map->size].subbase == NextBuilding ) bSubBase = true;
							else bSubBase = false;
							sendDeleteUnit( NextBuilding, MapPlayer->Nr );

							break;
						}
					}
				}
			}
			NextBuilding = NextBuilding->next;
		}
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
	string sReportMsg = "";
	int iVoiceNum;

	bool bChangeTurn = false;
	if ( iGameType == GAME_TYPE_SINGLE )
	{
		sendMakeTurnEnd ( true, false, -1, iPlayerNum );
		bChangeTurn = true;
	}
	else if ( iGameType == GAME_TYPE_HOTSEAT || bPlayTurns )
	{
		bool bWaitForPlayer = ( iGameType == GAME_TYPE_TCPIP && bPlayTurns );
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
					sendMakeTurnEnd(true, bWaitForPlayer, (*PlayerList)[iActiveTurnPlayerNr]->Nr, i);
				}
			}
			bChangeTurn = true;
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
	}
	else // it's a simultanous TCP/IP multiplayer game
	{
		// check whether this player has already finished his turn
		for ( int i = 0; i < PlayerEndList.Size(); i++ )
		{
			if (*PlayerEndList[i] == iPlayerNum) return;
		}
		PlayerEndList.Add ( &getPlayerFromNumber ( iPlayerNum )->Nr );

		if ( PlayerEndList.Size() >= PlayerList->Size() )
		{
			while ( PlayerEndList.Size() )
			{
				PlayerEndList.Delete ( 0 );
			}
			for ( int i = 0; i < PlayerList->Size(); i++ )
			{
				sendMakeTurnEnd ( true, false, -1, i );
			}
			iDeadlineStartTime = 0;
			bChangeTurn = true;
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
	if ( bChangeTurn )
	{
		iTurn++;
		makeTurnEnd ();
		for ( int i = 0; i < PlayerList->Size(); i++ )
		{
			getTurnstartReport ( i, &sReportMsg, &iVoiceNum );
			sendTurnReport ( iVoiceNum, sReportMsg, i );
			sReportMsg = "";
		}
	}
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

			if ( Vehicle->mjob ) Vehicle->mjob->EndForNow = false;
			Vehicle = Vehicle->next;
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
}

void cServer::getTurnstartReport ( int iPlayerNum, string *sReportMsg, int *iVoiceNum )
{
	sReport *Report;
	string sTmp;
	int iCount = 0;

	cPlayer *Player = getPlayerFromNumber ( iPlayerNum );
	if ( Player == NULL ) return;
	while ( Player->ReportBuildings.Size() )
	{
		Report = Player->ReportBuildings[0];
		if ( iCount ) *sReportMsg += ", ";
		iCount += Report->anz;
		sTmp = iToStr( Report->anz ) + " " + Report->name;
		*sReportMsg += Report->anz > 1 ? sTmp : Report->name;
		Player->ReportBuildings.Delete ( 0 );
		delete Report;
	}
	while ( Player->ReportVehicles.Size() )
	{
		Report = Player->ReportVehicles[0];
		if ( iCount ) *sReportMsg+=", ";
		iCount += Report->anz;
		sTmp = iToStr( Report->anz ) + " " + Report->name;
		*sReportMsg += Report->anz > 1 ? sTmp : Report->name;
		Player->ReportVehicles.Delete ( 0 );
		delete Report;
	}

	if ( iCount == 0 )
	{
		if ( !Player->ReportForschungFinished ) *iVoiceNum = 0;
	}
	else if ( iCount == 1 )
	{
		*sReportMsg += " " + lngPack.i18n( "Text~Comp~Finished") + ".";
		if ( !Player->ReportForschungFinished ) *iVoiceNum = 1;
	}
	else
	{
		*sReportMsg += " " + lngPack.i18n( "Text~Comp~Finished2") + ".";
		if ( !Player->ReportForschungFinished ) *iVoiceNum = 2;
	}
	Player->ReportForschungFinished = false;
}

void cServer::addReport ( string sName, bool bVehicle, int iPlayerNum )
{
	sReport *Report;
	cPlayer *Player = getPlayerFromNumber ( iPlayerNum );
	if ( bVehicle )
	{
		for ( int i = 0; i < Player->ReportVehicles.Size(); i++ )
		{
			Report = Player->ReportVehicles[i];
			if ( Report->name.compare ( sName ) == 0 )
			{
				Report->anz++;
				return;
			}
		}
		Report = new sReport;
		Report->name = sName;
		Report->anz = 1;
		Player->ReportVehicles.Add ( Report );
	}
	else
	{
		for ( int i = 0; i < Player->ReportBuildings.Size(); i++ )
		{
			Report = Player->ReportBuildings[i];
			if ( Report->name.compare ( sName ) == 0 )
			{
				Report->anz++;
				return;
			}
		}
		Report = new sReport;
		Report->name = sName;
		Report->anz = 1;
		Player->ReportBuildings.Add ( Report );
	}
}

void cServer::checkDeadline ()
{
	if ( iTurnDeadline >= 0 && iDeadlineStartTime > 0 )
	{
		if ( SDL_GetTicks() - iDeadlineStartTime > (unsigned int)iTurnDeadline*1000 )
		{
			while ( PlayerEndList.Size() )
			{
				PlayerEndList.Delete ( 0 );
			}
			string sReportMsg = "";
			int iVoiceNum;

			for ( int i = 0; i < PlayerList->Size(); i++ )
			{
				sendMakeTurnEnd ( true, false, -1, i );
			}
			iTurn++;
			iDeadlineStartTime = 0;
			makeTurnEnd();
			for ( int i = 0; i < PlayerList->Size(); i++ )
			{
				getTurnstartReport ( i, &sReportMsg, &iVoiceNum );
				sendTurnReport ( iVoiceNum, sReportMsg, i );
			}
		}
	}
}

void cServer::addActiveMoveJob ( cMJobs *MJob )
{
	ActiveMJobs.Add ( MJob );
	MJob->Suspended = false;
}

void cServer::handleMoveJobs ()
{
	for ( int i = 0; i < ActiveMJobs.Size(); i++ )
	{
		cMJobs *MJob;
		cVehicle *Vehicle;

		MJob = ActiveMJobs[i];
		Vehicle = MJob->vehicle;

		if ( MJob->finished || MJob->EndForNow )
		{
			// stop the job
			if ( MJob->EndForNow && Vehicle )
			{
				cLog::write(" Server: Movejob has end for now and will be stoped (delete from active ones)", cLog::eLOG_TYPE_NET_DEBUG);
				for ( int i = 0; i < MJob->vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendNextMove(MJob->vehicle->iID, MJob->vehicle->PosX + MJob->vehicle->PosY * Map->size, MJOB_STOP, *MJob->vehicle->SeenByPlayerList[i]);
				}
				sendNextMove ( MJob->vehicle->iID, MJob->vehicle->PosX+MJob->vehicle->PosY*Map->size, MJOB_STOP, MJob->vehicle->owner->Nr );
			}
			else
			{
				if ( Vehicle && Vehicle->mjob == MJob )
				{
					cLog::write(" Server: Movejob is finished and will be deleted now", cLog::eLOG_TYPE_NET_DEBUG);
					Vehicle->mjob = NULL;
					Vehicle->moving = false;
					Vehicle->MoveJobActive = false;

					for ( int i = 0; i < MJob->vehicle->SeenByPlayerList.Size(); i++ )
					{
						sendNextMove(MJob->vehicle->iID, MJob->vehicle->PosX + MJob->vehicle->PosY * Map->size, MJOB_FINISHED, *MJob->vehicle->SeenByPlayerList[i]);
					}
					sendNextMove ( MJob->vehicle->iID, MJob->vehicle->PosX+MJob->vehicle->PosY*Map->size, MJOB_FINISHED, MJob->vehicle->owner->Nr );
				}
				else cLog::write(" Server: Delete movejob with nonactive vehicle (released one)", cLog::eLOG_TYPE_NET_DEBUG);
				delete MJob;
			}
			ActiveMJobs.Delete ( i );
			continue;
		}

		if ( Vehicle == NULL ) continue;

		// rotate vehicle
		if ( MJob->next_dir != Vehicle->dir && Vehicle->data.speed )
		{
			Vehicle->rotating = true;
			if ( iTimer1 )
			{
				Vehicle->RotateTo ( MJob->next_dir );
			}
			continue;
		}
		else
		{
			Vehicle->rotating = false;
		}

		if ( !MJob->vehicle->moving )
		{
			checkMove( MJob );
		}
		else
		{
			moveVehicle ( MJob->vehicle );
		}
	}
}

void cServer::checkMove ( cMJobs *MJob )
{
	bool bInSentryRange;
	if ( !MJob->vehicle || !MJob->waypoints || !MJob->waypoints->next ) return;
	bInSentryRange = MJob->vehicle->InSentryRange();
	if ( !MJob->CheckPointNotBlocked ( MJob->waypoints->next->X, MJob->waypoints->next->Y ) || bInSentryRange )
	{
		cLog::write( " Server: Next point is blocked: ID: " + iToStr ( MJob->vehicle->iID ) + ", X: " + iToStr ( MJob->waypoints->next->X ) + ", Y: " + iToStr ( MJob->waypoints->next->Y ), LOG_TYPE_NET_DEBUG );
		// if the next point would be the last, finish the job here
		if ( MJob->waypoints->next->X == MJob->DestX && MJob->waypoints->next->Y == MJob->DestY )
		{
			MJob->finished = true;
		}
		// else delete the movejob and inform the client that he has to find a new path
		else
		{
			for ( int i = 0; i < MJob->vehicle->SeenByPlayerList.Size(); i++ )
			{
				sendNextMove(MJob->vehicle->iID, MJob->vehicle->PosX + MJob->vehicle->PosY * Map->size, MJOB_BLOCKED, *MJob->vehicle->SeenByPlayerList[i]);
			}
			sendNextMove ( MJob->vehicle->iID, MJob->vehicle->PosX+MJob->vehicle->PosY*Map->size, MJOB_BLOCKED, MJob->vehicle->owner->Nr );

			for ( int i = 0; i < ActiveMJobs.Size(); i++ )
			{
				if (MJob == ActiveMJobs[i])
				{
					ActiveMJobs.Delete ( i );
					break;
				}
			}
			MJob->vehicle->mjob = NULL;
			delete MJob;
			cLog::write( " Server: Movejob deleted and informed the clients to stop this movejob", LOG_TYPE_NET_DEBUG );
		}
		return;
	}

	// not enough waypoints for this move
	if ( MJob->vehicle->data.speed < MJob->waypoints->next->Costs )
	{
		cLog::write( " Server: Vehicle has not enough waypoints for the next move -> EndForNow: ID: " + iToStr ( MJob->vehicle->iID ) + ", X: " + iToStr ( MJob->waypoints->next->X ) + ", Y: " + iToStr ( MJob->waypoints->next->Y ), LOG_TYPE_NET_DEBUG );
		MJob->SavedSpeed += MJob->vehicle->data.speed;
		MJob->vehicle->data.speed = 0;
		MJob->EndForNow = true;
		return;
	}

	MJob->vehicle->MoveJobActive = true;
	MJob->vehicle->moving = true;

	// reserv the next field
	if ( !MJob->plane ) Map->GO[MJob->waypoints->next->X+MJob->waypoints->next->Y*Map->size].reserviert = true;
	else Map->GO[MJob->waypoints->next->X+MJob->waypoints->next->Y*Map->size].air_reserviert = true;

	// send move command to all players who can see the unit
	for ( int i = 0; i < MJob->vehicle->SeenByPlayerList.Size(); i++ )
	{
		sendNextMove(MJob->vehicle->iID, MJob->vehicle->PosX + MJob->vehicle->PosY * Map->size, MJOB_OK, *MJob->vehicle->SeenByPlayerList[i]);
	}
	sendNextMove ( MJob->vehicle->iID, MJob->vehicle->PosX+MJob->vehicle->PosY*Map->size, MJOB_OK, MJob->vehicle->owner->Nr );
}

void cServer::moveVehicle ( cVehicle *Vehicle )
{
	// just do this every 50 miliseconds
	if ( !iTimer0 ) return;

	cMJobs *MJob = Vehicle->mjob;
	int iSpeed;
	if ( !Vehicle ) return;
	if ( Vehicle->data.is_human )
	{
		Vehicle->WalkFrame++;
		if ( Vehicle->WalkFrame >= 13 ) Vehicle->WalkFrame = 0;
		iSpeed = MOVE_SPEED/2;
	}
	else if ( !(Vehicle->data.can_drive == DRIVE_AIR) && !(Vehicle->data.can_drive == DRIVE_SEA) )
	{
		iSpeed = MOVE_SPEED;
		if ( MJob->waypoints && MJob->waypoints->next && Map->GO[MJob->waypoints->next->X+MJob->waypoints->next->Y*Map->size].base&& ( Map->GO[MJob->waypoints->next->X+MJob->waypoints->next->Y*Map->size].base->data.is_road || Map->GO[MJob->waypoints->next->X+MJob->waypoints->next->Y*Map->size].base->data.is_bridge ) ) iSpeed*=2;
	}
	else if ( Vehicle->data.can_drive == DRIVE_AIR ) iSpeed = MOVE_SPEED*2;
	else iSpeed = MOVE_SPEED;

	switch ( MJob->next_dir )
	{
		case 0:
			Vehicle->OffY -= iSpeed;
			break;
		case 1:
			Vehicle->OffY -= iSpeed;
			Vehicle->OffX += iSpeed;
			break;
		case 2:
			Vehicle->OffX += iSpeed;
			break;
		case 3:
			Vehicle->OffX += iSpeed;
			Vehicle->OffY += iSpeed;
			break;
		case 4:
			Vehicle->OffY += iSpeed;
			break;
		case 5:
			Vehicle->OffX -= iSpeed;
			Vehicle->OffY += iSpeed;
			break;
		case 6:
			Vehicle->OffX -= iSpeed;
			break;
		case 7:
			Vehicle->OffX -= iSpeed;
			Vehicle->OffY -= iSpeed;
			break;
	}


	// check whether the point has been reached:
	if ( Vehicle->OffX >= 64 || Vehicle->OffY >= 64 || Vehicle->OffX <= -64 || Vehicle->OffY <= -64 )
	{
		cLog::write(" Server: Vehicle reached the next field: ID: " + iToStr ( Vehicle->iID )+ ", X: " + iToStr ( Vehicle->mjob->waypoints->next->X ) + ", Y: " + iToStr ( Vehicle->mjob->waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);
		cMJobs *MJob = Vehicle->mjob;
		sWaypoint *Waypoint;
		Waypoint = MJob->waypoints->next;
		free ( MJob->waypoints );
		MJob->waypoints = Waypoint;

		if ( Vehicle->data.can_drive == DRIVE_AIR )
		{
			Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].plane = NULL;
			Map->GO[MJob->waypoints->X+MJob->waypoints->Y*Map->size].plane = Vehicle;
			Map->GO[MJob->waypoints->X+MJob->waypoints->Y*Map->size].air_reserviert = false;
		}
		else
		{
			Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].vehicle = NULL;
			Map->GO[MJob->waypoints->X+MJob->waypoints->Y*Map->size].vehicle = Vehicle;
			Map->GO[MJob->waypoints->X+MJob->waypoints->Y*Map->size].reserviert = false;
		}
		Vehicle->OffX = 0;
		Vehicle->OffY = 0;
		Vehicle->PosX = MJob->waypoints->X;
		Vehicle->PosY = MJob->waypoints->Y;

		if ( MJob->waypoints->next == NULL )
		{
			MJob->finished = true;
		}

		Vehicle->data.speed += MJob->SavedSpeed;
		MJob->SavedSpeed = 0;
		Vehicle->DecSpeed ( MJob->waypoints->Costs );

		// check for results of the move

		// make mines explode if necessary
		cBuilding* building = Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].base;
		if ( Vehicle->data.can_drive != DRIVE_AIR && building && building->data.is_expl_mine && building->owner != Vehicle->owner )
		{
			AJobs.Add( new cServerAttackJob( building, Vehicle->PosX+Vehicle->PosY*Map->size ));
			
			if ( Vehicle->mjob )  Vehicle->mjob->EndForNow = true;
		}

		// search for resources if necessary
		if ( Vehicle->data.can_survey )
		{
			sendResources( Vehicle, Map );
			Vehicle->doSurvey();
		}

		// let other units fire on this one
		Vehicle->InSentryRange();

		// search for mines if necessary
		if ( Vehicle->data.can_detect_mines )
		{
			Vehicle->detectMines();
		}

		// lay/clear mines if necessary
		if ( Vehicle->data.can_lay_mines )
		{
			bool bResult = false;
			if ( Vehicle->LayMines ) bResult = Vehicle->layMine();
			else if ( Vehicle->ClearMines ) bResult = Vehicle->clearMine();
			if ( bResult )
			{
				// send new unit values
				sendUnitData( Vehicle, Vehicle->owner->Nr );
				for ( int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendUnitData(Vehicle, *Vehicle->SeenByPlayerList[i]);
				}
			}
		}

		Vehicle->owner->DoScan();

		Vehicle->moving = false;
		MJob->CalcNextDir();
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

void cServer::releaseMoveJob ( cMJobs *MJob )
{
	cLog::write ( " Server: Released old movejob", cLog::eLOG_TYPE_NET_DEBUG );
	for ( int i = 0; i < ActiveMJobs.Size(); i++ )
	{
		if (MJob == ActiveMJobs[i]) return;
	}
	addActiveMoveJob ( MJob );
	cLog::write ( " Server: Added released movejob to avtive ones", cLog::eLOG_TYPE_NET_DEBUG );
}

bool cServer::checkBlockedBuildField ( int iOff, cVehicle *Vehicle, sUnitData *Data )
{
	// cannot build on dirt
	if ( Map->GO[iOff].subbase && !Map->GO[iOff].subbase->owner ) return true;

	// cannot build e.g. landingplattforms on waterplattforms or bridges
	if ( Map->GO[iOff].base && ( Map->GO[iOff].base->data.is_platform || Map->GO[iOff].base->data.is_bridge ) && ( Data->is_base && !Data->is_road ) ) return true;

	// the rest has only to be checked if the building is no connector and if there is no base building under it excepting an waterplattform
	if ( ( !Map->GO[iOff].base || Map->GO[iOff].base->data.is_platform ) && !Data->is_connector )
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

void cServer::calcBuildRoundsAndCosts( cVehicle *Vehicle, int iBuildingType, int iTurboBuildRounds[3], int iTurboBuildCosts[3] )
{
	iTurboBuildRounds[0] = 0;
	iTurboBuildRounds[1] = 0;
	iTurboBuildRounds[2] = 0;

	//prevent division by zero
	if ( Vehicle->data.iNeeds_Metal == 0 ) Vehicle->data.iNeeds_Metal = 1;

	//step 1x
	if ( Vehicle->data.cargo >= Vehicle->owner->BuildingData[iBuildingType].iBuilt_Costs )
	{
		iTurboBuildCosts[0] = Vehicle->owner->BuildingData[iBuildingType].iBuilt_Costs;

		iTurboBuildRounds[0] = iTurboBuildCosts[0] / Vehicle->data.iNeeds_Metal;
	}

	//step 2x
	if ( ( iTurboBuildRounds[0] > 1 ) && ( iTurboBuildCosts[0] + 4 <= Vehicle->owner->BuildingData[iBuildingType].iBuilt_Costs_Max ) && ( Vehicle->data.cargo >= iTurboBuildCosts[0] + 4 ) )
	{
		iTurboBuildRounds[1] = iTurboBuildRounds[0];
		iTurboBuildCosts[1] = iTurboBuildCosts[0];

		while ( ( Vehicle->data.cargo >= iTurboBuildCosts[1] + 4 ) && ( iTurboBuildRounds[1] > 1 ) )
		{
			iTurboBuildRounds[1]--;
			iTurboBuildCosts[1] += 4;

			if ( iTurboBuildCosts[1] + 4 > 2*iTurboBuildCosts[0] )
				break;

			if ( iTurboBuildCosts[1] + 4 > Vehicle->owner->BuildingData[iBuildingType].iBuilt_Costs_Max )
				break;
		}
	}

	//step 4x
	if ( ( iTurboBuildRounds[1] > 1 ) && ( iTurboBuildCosts[1] + 8 <= Vehicle->owner->BuildingData[iBuildingType].iBuilt_Costs_Max ) && ( Vehicle->data.cargo >= iTurboBuildCosts[1] + 8 ) )
	{
		iTurboBuildRounds[2] = iTurboBuildRounds[1];
		iTurboBuildCosts[2] = iTurboBuildCosts[1];

		while ( ( Vehicle->data.cargo >= iTurboBuildCosts[2] + 8 ) && ( iTurboBuildRounds[2] > 1 ) )
		{
			iTurboBuildRounds[2]--;
			iTurboBuildCosts[2] += 8;

			if ( iTurboBuildCosts[2] + 8 > Vehicle->owner->BuildingData[iBuildingType].iBuilt_Costs_Max )
				break;
		}
	}
}

bool cServer::checkExitBlocked ( int iX, int iY, sVehicle *Type )
{
	int iOff = iX+iY*Map->size;

	if ( iOff < 0 || iOff >= Map->size*Map->size ) return true;

	if ( Type->data.can_drive == DRIVE_AIR && Map->GO[iOff].plane ) return true;
	if ( Map->GO[iOff].vehicle || Map->GO[iOff].top ) return true;
	if ( Type->data.can_drive == DRIVE_SEA && ( !Map->IsWater ( iOff, true ) || Map->GO[iOff].base || Map->GO[iOff].subbase ) ) return true;
	if ( Type->data.can_drive == DRIVE_LAND && Map->IsWater ( iOff ) && ( !Map->GO[iOff].base || ( !Map->GO[iOff].base->data.is_platform && !Map->GO[iOff].subbase->data.is_road && !Map->GO[iOff].base->data.is_expl_mine ) ) ) return true;

	return false;
}

void cServer::destroyUnit( cVehicle* vehicle )
{
	int offset = vehicle->PosX + vehicle->PosY*Map->size;
	int value = vehicle->data.iBuilt_Costs/2;

	//delete other units here!

	deleteUnit( vehicle, false );

	Map->addRubble( offset, value, false );
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
		
		deleteUnit( Map->GO[offset + 1            ].base );
		deleteUnit( Map->GO[offset + Map->size    ].base );
		deleteUnit( Map->GO[offset + Map->size + 1].base );
		deleteUnit( Map->GO[offset + 1            ].subbase );
		deleteUnit( Map->GO[offset + Map->size    ].subbase );
		deleteUnit( Map->GO[offset + Map->size + 1].subbase );
	}

	if ( Map->GO[offset].top )     value += Map->GO[offset].top->data.iBuilt_Costs;
	if ( Map->GO[offset].base )    value += Map->GO[offset].base->data.iBuilt_Costs;
	if ( Map->GO[offset].subbase && Map->GO[offset].subbase->owner ) value += Map->GO[offset].subbase->data.iBuilt_Costs;

	deleteUnit( Map->GO[offset].top );
	deleteUnit( Map->GO[offset].base );
	deleteUnit( Map->GO[offset].subbase );

	Map->addRubble( offset, value/2, big );

}
