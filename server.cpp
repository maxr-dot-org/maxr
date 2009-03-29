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
#include "upgradecalculator.h"

//-------------------------------------------------------------------------------------
Uint32 ServerTimerCallback(Uint32 interval, void *arg)
{
	((cServer *)arg)->Timer();
	return interval;
}

//-------------------------------------------------------------------------------------
int CallbackRunServerThread( void *arg )
{
	cServer *Server = (cServer *) arg;
	Server->run();
	return 0;
}

//-------------------------------------------------------------------------------------
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
	iTurnDeadline = 90; // just temporary set to 45 seconds
	iNextUnitID = 1;
	iTimerTime = 0;
	iWantPlayerEndNum = -1;
	TimerID = SDL_AddTimer ( 50, ServerTimerCallback, this );

	ServerThread = SDL_CreateThread( CallbackRunServerThread, this );
}

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
void cServer::setDeadline(int iDeadline)
{
	iTurnDeadline = iDeadline;
}

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
int cServer::pushEvent( SDL_Event *event )
{
	cMutex::Lock l(QueueMutex);
	EventQueue.Add ( event );
	return 0;
}

//-------------------------------------------------------------------------------------
void cServer::sendNetMessage( cNetMessage* message, int iPlayerNum )
{
	message->iPlayerNr = iPlayerNum;

	if (message->iType != DEBUG_CHECK_VEHICLE_POSITIONS)  //do not pollute log file with debug events
		Log.write("Server: <-- " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );

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
		Log.write("Server: Can't send message. Player " + iToStr(iPlayerNum) + " not found.", cLog::eLOG_TYPE_NET_WARNING);
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

//-------------------------------------------------------------------------------------
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

						cPlayer *Player = NULL;
						// resort socket numbers of the players
						for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
						{
							if ( (*PlayerList)[i]->iSocketNum == iSocketNumber ) Player = (*PlayerList)[i];
							else if ( (*PlayerList)[i]->iSocketNum > iSocketNumber && (*PlayerList)[i]->iSocketNum != MAX_CLIENTS ) (*PlayerList)[i]->iSocketNum--;
						}
						// push lost connection message
						if ( Player )
						{
							cNetMessage message( GAME_EV_LOST_CONNECTION );
							message.pushInt16( Player->Nr );
							pushEvent( message.getGameEvent() );
						}
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
		checkDeadline();
		handleMoveJobs ();
		handleTimer();
		handleWantEnd();
		SDL_Delay( 10 );
	}
}

//-------------------------------------------------------------------------------------
int cServer::HandleNetMessage( cNetMessage *message )
{
	Log.write("Server: --> " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );

	switch ( message->iType )
	{
	case GAME_EV_LOST_CONNECTION:
		{
			int playerNum = message->popInt16();
			cPlayer *Player = getPlayerFromNumber ( playerNum );
			if ( !Player ) break;
			Player->iSocketNum = -1;

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
			int iID = message->popInt32();

			cBuilding* building = getBuildingFromID( iID );
			if ( building == NULL || building->owner->Nr != message->iPlayerNr ) break;

			//handle message
			building->ServerStartWork();
			break;
		}
	case GAME_EV_WANT_STOP_WORK:
		{
			int iID = message->popInt32();
			
			cBuilding* building = getBuildingFromID( iID );
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

			cVehicle *Vehicle = getVehicleFromID ( iVehicleID );
			if ( Vehicle == NULL )
			{
				Log.write(" Server: Can't find vehicle with id " + iToStr ( iVehicleID ) + " for movejob from " +  iToStr (iSrcOff%Map->size) + "x" + iToStr (iSrcOff/Map->size) + " to " + iToStr (iDestOff%Map->size) + "x" + iToStr (iDestOff/Map->size), cLog::eLOG_TYPE_NET_WARNING);
				break;
			}
			if ( Vehicle->PosX+Vehicle->PosY*Map->size != iSrcOff )
			{
				Log.write(" Server: Vehicle with id " + iToStr ( iVehicleID ) + " is at wrong position (" + iToStr (Vehicle->PosX) + "x" + iToStr(Vehicle->PosY) + ") for movejob from " +  iToStr (iSrcOff%Map->size) + "x" + iToStr (iSrcOff/Map->size) + " to " + iToStr (iDestOff%Map->size) + "x" + iToStr (iDestOff/Map->size), cLog::eLOG_TYPE_NET_WARNING);
				break;
			}
			//TODO: is this check really needed?
			if ( Vehicle->bIsBeeingAttacked )
			{
				Log.write(" Server: cannot move a vehicle currently under attack", cLog::eLOG_TYPE_NET_DEBUG );
				break;
			}
			if ( Vehicle->Attacking )
			{
				Log.write(" Server: cannot move a vehicle currently attacking", cLog::eLOG_TYPE_NET_DEBUG );
				break;
			}
			if ( Vehicle->IsBuilding || Vehicle->BuildPath )
			{
				Log.write(" Server: cannot move a vehicle currently building", cLog::eLOG_TYPE_NET_DEBUG );
				break;
			}
			if ( Vehicle->IsClearing )
			{
				Log.write(" Server: cannot move a vehicle currently building", cLog::eLOG_TYPE_NET_DEBUG );
				break;
			}
			
			cServerMoveJob *MoveJob = new cServerMoveJob ( iSrcOff, iDestOff, bPlane, Vehicle );
			if ( !MoveJob->generateFromMessage ( message ) )
			{
				delete MoveJob;
				break;
			}

			addActiveMoveJob ( MoveJob );
			Log.write(" Server: Added received movejob", cLog::eLOG_TYPE_NET_DEBUG);
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
					Log.write(" Server: vehicle with ID " + iToStr(ID) + " not found", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				if ( attackingVehicle->owner->Nr != message->iPlayerNr )
				{
					Log.write(" Server: Message was not send by vehicle owner!", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				if ( attackingVehicle->bIsBeeingAttacked ) break;
			}
			else
			{
				int offset = message->popInt32();
				if ( offset < 0 || offset > Map->size * Map->size ) 
				{
					Log.write(" Server: Invalid agressor offset", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				attackingBuilding = Map->fields[offset].getTopBuilding();
				if ( attackingBuilding == NULL )
				{
					Log.write(" Server: No Building at agressor offset", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				if ( attackingBuilding->owner->Nr != message->iPlayerNr )
				{
					Log.write(" Server: Message was not send by building owner!", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				if ( attackingBuilding->bIsBeeingAttacked ) break;
			}

			//find target offset
			int targetOffset = message->popInt32();
			if ( targetOffset < 0 || targetOffset > Map->size * Map->size )
			{
				Log.write(" Server: Invalid target offset!", cLog::eLOG_TYPE_NET_WARNING);
				break;
			}

			int targetID = message->popInt32();
			if ( targetID != 0 )
			{
				cVehicle* targetVehicle = getVehicleFromID( targetID );
				if ( targetVehicle == NULL )
				{
					Log.write(" Server: vehicle with ID " + iToStr(targetID) + " not found!", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				int oldOffset = targetOffset;
				//the target offset doesn't need to match the vehicle position, when it is big
				if ( !targetVehicle->data.is_big ) 
				{
					targetOffset = targetVehicle->PosX + targetVehicle->PosY * Map->size;
				}
				Log.write( " Server: attacking vehicle " + targetVehicle->name + ", " + iToStr(targetVehicle->iID), cLog::eLOG_TYPE_NET_DEBUG );
				if ( oldOffset != targetOffset ) Log.write(" Server: target offset changed from " + iToStr( oldOffset ) + " to " + iToStr( targetOffset ), cLog::eLOG_TYPE_NET_DEBUG );
			}

			//check if attack is possible
			if ( bIsVehicle )
			{
				if ( !attackingVehicle->CanAttackObject( targetOffset, Server->Map, true ) )
				{
					Log.write(" Server: The server decided, that the attack is not possible", cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				AJobs.Add( new cServerAttackJob( attackingVehicle, targetOffset ));
			}
			else
			{
				if ( !attackingBuilding->CanAttackObject( targetOffset, Server->Map, true ) )
				{
					Log.write(" Server: The server decided, that the attack is not possible", cLog::eLOG_TYPE_NET_WARNING);
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
				Log.write(" Server: ServerAttackJob not found",cLog::eLOG_TYPE_NET_ERROR);
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

			Vehicle = getVehicleFromID ( message->popInt16() );
			if ( Vehicle == NULL ) break;
			if ( Vehicle->IsBuilding || Vehicle->BuildPath ) break;

			sID BuildingTyp;
			BuildingTyp.iFirstPart = message->popInt16();
			BuildingTyp.iSecondPart = message->popInt16();
			if ( BuildingTyp.getUnitData() == NULL )
			{
				Log.write(" Server: invalid unit: " + iToStr(BuildingTyp.iFirstPart) + "."  + iToStr(BuildingTyp.iSecondPart), cLog::eLOG_TYPE_NET_ERROR);
				break;
			}
			const sUnitData& Data = *BuildingTyp.getUnitData();
			iBuildSpeed = message->popInt16();
			if ( iBuildSpeed > 2 || iBuildSpeed < 0 ) break;
			iBuildOff = message->popInt32();
			if ( iBuildOff < 0 || iBuildOff >= Map->size*Map->size ) break;
			int buildX = iBuildOff % Map->size;
			int buildY = iBuildOff / Map->size;

			if ( Data.is_big )
			{
				if ( Vehicle->data.can_build != BUILD_BIG ) break;

				sideStepStealthUnit( buildX    , buildY    , Vehicle, iBuildOff );
				sideStepStealthUnit( buildX + 1, buildY    , Vehicle, iBuildOff );
				sideStepStealthUnit( buildX    , buildY + 1, Vehicle, iBuildOff );
				sideStepStealthUnit( buildX + 1, buildY + 1, Vehicle, iBuildOff );

				if ( !( Map->possiblePlaceBuilding( Data, buildX,     buildY    , Vehicle ) &&
						Map->possiblePlaceBuilding( Data, buildX + 1, buildY    , Vehicle ) &&
						Map->possiblePlaceBuilding( Data, buildX,     buildY + 1, Vehicle ) &&
						Map->possiblePlaceBuilding( Data, buildX + 1, buildY + 1, Vehicle ) ))
				{
					sendBuildAnswer ( false, Vehicle );
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
					sendBuildAnswer ( false, Vehicle );
					break;
				}
			}

			Vehicle->BuildingTyp = BuildingTyp;
			bool bBuildPath = message->popBool();
			iPathOff = message->popInt32();
			if ( iPathOff < 0 || iPathOff >= Map->size*Map->size ) break;
			Vehicle->BandX = iPathOff%Map->size;
			Vehicle->BandY = iPathOff/Map->size;


			Vehicle->calcTurboBuild( iTurboBuildRounds, iTurboBuildCosts, BuildingTyp.getUnitData( Vehicle->owner )->iBuilt_Costs, BuildingTyp.getUnitData( Vehicle->owner )->iBuilt_Costs_Max );

			Vehicle->BuildCosts = iTurboBuildCosts[iBuildSpeed];
			Vehicle->BuildRounds = iTurboBuildRounds[iBuildSpeed];
			Vehicle->BuildCostsStart = Vehicle->BuildCosts;
			Vehicle->BuildRoundsStart = Vehicle->BuildRounds;

			if ( Vehicle->BuildCosts > Vehicle->data.cargo )
			{
				sendBuildAnswer ( false, Vehicle );
				break;
			}


			Vehicle->IsBuilding = true;
			Vehicle->BuildPath = bBuildPath;

			sendBuildAnswer ( true, Vehicle );

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
			if (!Map->possiblePlace( Vehicle, iEscapeX, iEscapeY ))
			{
				sideStepStealthUnit( iEscapeX, iEscapeY, Vehicle );
			}

			if (!Map->possiblePlace( Vehicle, iEscapeX, iEscapeY )) break;

			addUnit( Vehicle->PosX, Vehicle->PosY, Vehicle->BuildingTyp.getBuilding(), Vehicle->owner );

			// end building
			Vehicle->IsBuilding = false;
			Vehicle->BuildPath = false;


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

			/*// send new vehicle status and position		//done implicitly by addMoveJob()
			sendUnitData ( Vehicle, Vehicle->owner->Nr );
			for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
			{
				sendUnitData(Vehicle, Vehicle->SeenByPlayerList[i]->Nr );
			}*/

			// drive away from the building lot
			addMoveJob( Vehicle->PosX+Vehicle->PosY*Map->size, iEscapeX+iEscapeY*Map->size, Vehicle );
			Vehicle->ServerMoveJob->checkMove();	//begin the movment immediately, so no other unit can block the destination field
		}
		break;
	case GAME_EV_WANT_STOP_BUILDING:
		{
			cVehicle *Vehicle = getVehicleFromID ( message->popInt16() );
			if ( Vehicle == NULL ) break;
			if ( !Vehicle->IsBuilding ) break;
			stopVehicleBuilding ( Vehicle );
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

				cBuilding* building = Map->fields[iOff].getBaseBuilding();
				if ( !Map->IsWater ( iOff, true, true ) || ( building && ( building->data.is_bridge || building->data.is_platform || building->data.is_road ) ) ) bLand = true;
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

			if ( !Building->isNextTo( iX, iY )) break;

			if (!Map->possiblePlaceVehicle( BuildingListItem->typ->data, iX, iY ) )
			{
				sideStepStealthUnit(iX, iY, BuildingListItem->typ->data, Building->owner );
			}
			if ( !Map->possiblePlaceVehicle( BuildingListItem->typ->data, iX, iY )) break;

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
			cVehicle *SrcVehicle = NULL, *DestVehicle = NULL;
			cBuilding *SrcBuilding = NULL, *DestBuilding = NULL;
			int iType, iValue;

			// get the units
			iType = message->popChar();
			if ( iType > SUPPLY_TYPE_REPAIR ) break; // unknown type
			if ( message->popBool() ) SrcVehicle = getVehicleFromID ( message->popInt16() );
			else SrcBuilding = getBuildingFromID ( message->popInt16() );
			if ( message->popBool() ) DestVehicle = getVehicleFromID ( message->popInt16() );
			else DestBuilding = getBuildingFromID ( message->popInt16() );

			if ( ( !SrcVehicle && !SrcBuilding ) || ( !DestVehicle && !DestBuilding ) ) break;

			// check whether the supply is ok and reduce cargo of sourceunit
			if ( SrcVehicle )
			{
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
					sUnitData *DestData = DestVehicle ? &DestVehicle->data : &DestBuilding->data;
					// reduce cargo for repair and calculate maximal repair value
					iValue = DestData->hit_points;
					while ( SrcVehicle->data.cargo > 0 && iValue < DestData->max_hit_points )
					{
						iValue += Round(((float)DestData->max_hit_points/DestData->iBuilt_Costs)*4);
						SrcVehicle->data.cargo--;
					}
					if ( iValue > DestData->max_hit_points ) iValue = DestData->max_hit_points;
				}
				sendUnitData ( SrcVehicle, SrcVehicle->owner->Nr );	// the changed values aren't interesting for enemy players, so only send the new data to the owner
			}
			else
			{
				// buildings can only supply vehicles
				if ( !DestVehicle ) break;

				// do the supply
				if ( iType == SUPPLY_TYPE_REARM )
				{
					if ( SrcBuilding->SubBase->Metal < 1 ) break;
					SrcBuilding->owner->base.AddMetal ( SrcBuilding->SubBase, -1 );
					iValue = DestVehicle->data.max_ammo;
				}
				else
				{
					// reduce cargo for repair and calculate maximal repair value
					iValue = DestVehicle->data.hit_points;
					while ( SrcBuilding->SubBase->Metal > 0 && iValue < DestVehicle->data.max_hit_points )
					{
						iValue += Round(((float)DestVehicle->data.max_hit_points/DestVehicle->data.iBuilt_Costs)*4);
						SrcBuilding->owner->base.AddMetal ( SrcBuilding->SubBase, -1 );
					}
					if ( iValue > DestVehicle->data.max_hit_points ) iValue = DestVehicle->data.max_hit_points;
				}
				sendUnitData ( SrcBuilding, SrcBuilding->owner->Nr );	// the changed values aren't interesting for enemy players, so only send the new data to the owner
			}

			// repair or reload the destination unit
			if ( DestVehicle )
			{
				if ( iType == SUPPLY_TYPE_REARM ) DestVehicle->data.ammo = DestVehicle->data.max_ammo;
				else DestVehicle->data.hit_points = iValue;

				sendSupply ( DestVehicle->iID, true, iValue, iType, DestVehicle->owner->Nr );
				
				//send unitdata to the players who are not the owner
				for ( unsigned int i = 0; i < DestVehicle->SeenByPlayerList.Size(); i++)
					sendUnitData( DestVehicle, DestVehicle->SeenByPlayerList[i]->Nr );
			}
			else
			{
				if ( iType == SUPPLY_TYPE_REARM ) DestBuilding->data.ammo = DestBuilding->data.max_ammo;
				else DestBuilding->data.hit_points = iValue;

				sendSupply ( DestBuilding->iID, false, iValue, iType, DestBuilding->owner->Nr );
								
				//send unitdata to the players who are not the owner
				for ( unsigned int i = 0; i < DestBuilding->SeenByPlayerList.Size(); i++)
					sendUnitData( DestBuilding, DestBuilding->SeenByPlayerList[i]->Nr );
			}
		}
		break;
	case GAME_EV_WANT_VEHICLE_UPGRADE:
		{
			bool upgradeAll = message->popBool();
			int storageSlot = 0;
			if (!upgradeAll)
				storageSlot = message->popInt16();
			cBuilding* storingBuilding = getBuildingFromID(message->popInt32());
			if (storingBuilding == 0)
				break;

			int totalCosts = 0;
			int availableMetal = storingBuilding->SubBase->Metal;
			
			cList<cVehicle*> upgradedVehicles;
			for (unsigned int i = 0; i < storingBuilding->StoredVehicles.Size(); i++)
			{
				if (upgradeAll || i == storageSlot)
				{
					cVehicle* vehicle = storingBuilding->StoredVehicles[i];
					sUnitData& upgradedVersion = storingBuilding->owner->VehicleData[vehicle->typ->nr];
					
					if (vehicle->data.version >= upgradedVersion.version)
						continue; // already uptodate
					cUpgradeCalculator& uc = cUpgradeCalculator::instance();
					int upgradeCost = uc.getMaterialCostForUpgrading(upgradedVersion.iBuilt_Costs);

					if (availableMetal >= totalCosts + upgradeCost)
					{
						upgradedVehicles.Add(vehicle);
						totalCosts += upgradeCost;
					}
				}
			}
			
			if (upgradedVehicles.Size() > 0)
			{
				if (totalCosts > 0)
					storingBuilding->owner->base.AddMetal (storingBuilding->SubBase, -totalCosts);
				for (unsigned int i = 0; i < upgradedVehicles.Size(); i++)
					upgradedVehicles[i]->upgradeToCurrentVersion();
				sendUpgradeVehicles(upgradedVehicles, totalCosts, storingBuilding->iID, storingBuilding->owner->Nr);
			}
		}
		break;
	case GAME_EV_WANT_START_CLEAR:
		{
			int id = message->popInt16();
			cVehicle *Vehicle = getVehicleFromID ( id );
			if ( Vehicle == NULL )
			{
				Log.write("Server: Can not find vehicle with id " + iToStr ( id ) + " for clearing", LOG_TYPE_NET_WARNING);
				break;
			}

			int off = Vehicle->PosX+Vehicle->PosY*Map->size;
			cBuilding* building = (*Map)[off].getRubble();

			if ( !building )
			{
				sendClearAnswer ( 2, Vehicle, 0, -1, Vehicle->owner->Nr );
				break;
			}

			int rubbleoffset = -1;
			if ( building->data.is_big )
			{
				rubbleoffset = building->PosX+building->PosY*Map->size;

				sideStepStealthUnit( building->PosX    , building->PosY    , Vehicle, rubbleoffset );
				sideStepStealthUnit( building->PosX + 1, building->PosY    , Vehicle, rubbleoffset );
				sideStepStealthUnit( building->PosX    , building->PosY + 1, Vehicle, rubbleoffset );
				sideStepStealthUnit( building->PosX + 1, building->PosY + 1, Vehicle, rubbleoffset );

				if ( ( !Map->possiblePlace ( Vehicle, rubbleoffset ) && rubbleoffset != off ) ||
					( !Map->possiblePlace ( Vehicle, rubbleoffset+1 ) && rubbleoffset+1 != off ) ||
					( !Map->possiblePlace ( Vehicle, rubbleoffset+Map->size ) && rubbleoffset+Map->size != off ) ||
					( !Map->possiblePlace ( Vehicle, rubbleoffset+Map->size+1 ) && rubbleoffset+Map->size+1 != off ) )
				{
					sendClearAnswer ( 1, Vehicle, 0, -1, Vehicle->owner->Nr );
					break;
				}
				
				Vehicle->BuildBigSavedPos = off;
				Map->moveVehicleBig ( Vehicle, rubbleoffset );
			}

			Vehicle->IsClearing = true;
			Vehicle->ClearingRounds = building->RubbleValue/4+1;

			sendClearAnswer ( 0, Vehicle, Vehicle->ClearingRounds, rubbleoffset, Vehicle->owner->Nr );
			for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++)
			{
				sendClearAnswer( 0, Vehicle, 0, rubbleoffset, Vehicle->SeenByPlayerList[i]->Nr );
			}
		}
		break;
	case GAME_EV_WANT_STOP_CLEAR:
		{
			int id = message->popInt16();
			cVehicle *Vehicle = getVehicleFromID ( id );
			if ( Vehicle == NULL )
			{
				Log.write("Server: Can not find vehicle with id " + iToStr ( id ) + " for stop clearing", LOG_TYPE_NET_WARNING);
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
					//vehicle is removed from enemy clients by cServer::checkPlayerUnits()
					sendStoreVehicle ( StoringVehicle->iID, true, StoredVehicle->iID, StoringVehicle->owner->Nr );
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
					//vehicle is removed from enemy clients by cServer::checkPlayerUnits()
					sendStoreVehicle ( StoringBuilding->iID, false, StoredVehicle->iID, StoringBuilding->owner->Nr );
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
				if ( !StoringVehicle->isNextTo(x, y) ) break;

				//sidestep stealth units if nessesary
				sideStepStealthUnit(x, y, StoredVehicle);

				if ( StoringVehicle->canExitTo ( x, y, Server->Map, StoredVehicle->typ ) )
				{
					StoringVehicle->exitVehicleTo ( StoredVehicle, x+y*Map->size, Map );
					//vehicle is added to enemy clients by cServer::checkPlayerUnits()
					sendActivateVehicle ( StoringVehicle->iID, true, StoredVehicle->iID, x, y, StoringVehicle->owner->Nr );
					if ( StoredVehicle->data.can_survey ) 
					{
						sendVehicleResources( StoredVehicle, Map );
						StoredVehicle->doSurvey();
					}
					StoredVehicle->InSentryRange();
				}
			}
			else
			{
				cBuilding *StoringBuilding = getBuildingFromID ( message->popInt16() );
				if ( !StoringBuilding ) break;

				int x = message->popInt16 ();
				int y = message->popInt16 ();
				if ( !StoringBuilding->isNextTo(x, y )) break;
				
				//sidestep stealth units if nessesary
				sideStepStealthUnit(x, y, StoredVehicle);

				if ( StoringBuilding->canExitTo ( x, y, Server->Map, StoredVehicle->typ ) )
				{
					StoringBuilding->exitVehicleTo ( StoredVehicle, x+y*Map->size, Map );
					//vehicle is added to enemy clients by cServer::checkPlayerUnits()
					sendActivateVehicle ( StoringBuilding->iID, false, StoredVehicle->iID, x, y, StoringBuilding->owner->Nr );
					if ( StoredVehicle->data.can_survey ) 
					{
						sendVehicleResources( StoredVehicle, Map );
						StoredVehicle->doSurvey();
					}
					StoredVehicle->InSentryRange();
				}
			}
		}
		break;
	case GAME_EV_REQUEST_RESYNC:
		{
			cPlayer* player = getPlayerFromNumber( message->popChar());
			if ( player ) resyncPlayer( player, true);
		}
		break;
	case GAME_EV_WANT_BUY_UPGRADES:
		{
			int iPlayerNr = message->popInt16();
			cPlayer* player = getPlayerFromNumber(iPlayerNr);
			if (player == 0)
				break;

			bool updateCredits = false;
			
			int iCount = message->popInt16(); // get the number of upgrades in this message
			for (int i = 0; i < iCount; i++)
			{
				bool bVehicle = message->popBool();
				sID ID;
				ID.iFirstPart = message->popInt16();
				ID.iSecondPart = message->popInt16();

				int newDamage = message->popInt16();
				int newMaxShots = message->popInt16();
				int newRange = message->popInt16();
				int newMaxAmmo = message->popInt16();
				int newArmor = message->popInt16();
				int newMaxHitPoints = message->popInt16();
				int newScan = message->popInt16();
				int newMaxSpeed = 0;
				if (bVehicle)
					newMaxSpeed = message->popInt16();
				
				sUnitData* upgradedUnit = ID.getUnitData (player);
				if (upgradedUnit == 0)
					continue; // skip this upgrade, because there is no such unitData

				int costs = getUpgradeCosts (ID, player, bVehicle, newDamage, newMaxShots, newRange, newMaxAmmo, newArmor, newMaxHitPoints, newScan, newMaxSpeed);
				if (costs <= player->Credits)
				{
					// update the unitData of the player and send an ack-msg for this upgrade to the player
					upgradedUnit->damage = newDamage;
					upgradedUnit->max_shots = newMaxShots;
					upgradedUnit->range = newRange;
					upgradedUnit->max_ammo = newMaxAmmo;
					upgradedUnit->armor = newArmor;
					upgradedUnit->max_hit_points = newMaxHitPoints;
					upgradedUnit->scan = newScan;
					if (bVehicle) 
						upgradedUnit->max_speed = newMaxSpeed;
					upgradedUnit->version++;
					
					player->Credits -= costs;
					updateCredits = true;
					
					sendUnitUpgrades (upgradedUnit, iPlayerNr);
				}
			}
			if (updateCredits)
				sendCredits (player->Credits, iPlayerNr);
		}
		break;
	case GAME_EV_WANT_BUILDING_UPGRADE:
		{
			unsigned int unitID = message->popInt32();
			bool upgradeAll = message->popBool();

			cBuilding* building = getBuildingFromID(unitID);
			cPlayer* player = ((building != 0) ? building->owner : 0);
			if (player == 0)
				break;
			
			int availableMetal = building->SubBase->Metal;

			sUnitData& upgradedVersion = player->BuildingData[building->typ->nr];
			if (building->data.version >= upgradedVersion.version)
				break; // already uptodate
			cUpgradeCalculator& uc = cUpgradeCalculator::instance();
			int upgradeCostPerBuilding = uc.getMaterialCostForUpgrading(upgradedVersion.iBuilt_Costs);
			int totalCosts = 0;
			cList<cBuilding*> upgradedBuildings;
			
			// in any case update the selected building
			if (availableMetal >= totalCosts + upgradeCostPerBuilding)
			{
				upgradedBuildings.Add(building);
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
					if (otherBuilding->typ != building->typ)
						continue;
					if (otherBuilding->data.version >= upgradedVersion.version)
						continue;
					upgradedBuildings.Add(otherBuilding);
					totalCosts += upgradeCostPerBuilding;
					if (availableMetal < totalCosts + upgradeCostPerBuilding)
						break; // no more raw material left...
				}
			}

			if (totalCosts > 0)
				player->base.AddMetal (building->SubBase, -totalCosts);
			if (upgradedBuildings.Size() > 0)
			{
				bool scanNecessary = false;
				for (unsigned int i = 0; i < upgradedBuildings.Size(); i++)
				{
					if (!scanNecessary && upgradedBuildings[i]->data.scan < upgradedVersion.scan)
						scanNecessary = true; // Scan range was upgraded. So trigger a scan.
					upgradedBuildings[i]->upgradeToCurrentVersion();
				}
				sendUpgradeBuildings(upgradedBuildings, totalCosts, player->Nr);
				if (scanNecessary)
					player->DoScan();
			}
		}
		break;
	case GAME_EV_WANT_RESEARCH_CHANGE:
		{
			int iPlayerNr = message->popInt16();
			cPlayer* player = getPlayerFromNumber(iPlayerNr);
			if (player == 0)
				break;
			int newUsedResearch = 0;
			int newResearchSettings[cResearch::kNrResearchAreas];
			for (int area = cResearch::kNrResearchAreas - 1; area >= 0; area--)
			{
				newResearchSettings[area] = message->popInt16();
				newUsedResearch += newResearchSettings[area];
			}
			if (newUsedResearch > player->ResearchCount)
				break; // can't turn on research centers automatically!
			
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
					if (curBuilding->data.can_research && curBuilding->IsWorking)
					{
						researchCentersToChangeArea.Add(curBuilding);
						newAreasForResearchCenters.Add(newArea);
						centersToAssign--;
					}
					curBuilding = curBuilding->next;
				}
				if (curBuilding == 0 && centersToAssign > 0)
				{
					error = true; // not enough active research centers!
					break;
				}
			}
			while (curBuilding != 0) // shut down unused research centers
			{
				if (curBuilding->data.can_research && curBuilding->IsWorking)
					researchCentersToStop.Add(curBuilding);
				curBuilding = curBuilding->next;
			}
			if (error)
				break;
			
			for (unsigned int i = 0; i < researchCentersToStop.Size(); i++)
				researchCentersToStop[i]->ServerStopWork(false);
			
			for (unsigned int i = 0; i < researchCentersToChangeArea.Size(); i++)
				researchCentersToChangeArea[i]->researchArea = newAreasForResearchCenters[i];
			player->refreshResearchCentersWorkingOnArea();

			sendResearchSettings(researchCentersToChangeArea, newAreasForResearchCenters, iPlayerNr);
		}
		break;
	case GAME_EV_AUTOMOVE_STATUS:
		{
			cVehicle *Vehicle = getVehicleFromID ( message->popInt16() );
			if ( Vehicle ) Vehicle->hasAutoMoveJob = message->popBool();
		}
		break;
	case GAME_AV_WANT_COM_ACTION:
		{
			cVehicle *srcVehicle = getVehicleFromID ( message->popInt16() );
			if ( !srcVehicle ) break;

			cVehicle *destVehicle = NULL;
			cBuilding *destBuilding = NULL;
			if ( message->popBool() ) destVehicle = getVehicleFromID ( message->popInt16() );
			else destBuilding = getBuildingFromID ( message->popInt16() );

			bool steal = message->popBool();
			// check whether the commando action is possible
			if ( !( ( destVehicle && srcVehicle->canDoCommandoAction ( destVehicle->PosX, destVehicle->PosY, Client->Map, steal ) ) ||
				( destBuilding && srcVehicle->canDoCommandoAction ( destBuilding->PosX, destBuilding->PosY, Client->Map, steal ) ) ||
				( destBuilding && destBuilding->data.is_big && srcVehicle->canDoCommandoAction ( destBuilding->PosX, destBuilding->PosY+1, Client->Map, steal ) ) ||
				( destBuilding && destBuilding->data.is_big && srcVehicle->canDoCommandoAction ( destBuilding->PosX+1, destBuilding->PosY, Client->Map, steal ) ) ||
				( destBuilding && destBuilding->data.is_big && srcVehicle->canDoCommandoAction ( destBuilding->PosX+1, destBuilding->PosY+1, Client->Map, steal ) ) ) ) break;

			// check whether the action is successfull or not
			int chance = srcVehicle->calcCommandoChance ( destVehicle, destBuilding, steal );
			bool success = false;
			if ( random ( 100 ) < chance )
			{
				if ( steal )
				{
					if ( destVehicle )
					{
						// change the owner
						if ( destVehicle->IsBuilding ) stopVehicleBuilding ( destVehicle );
						if ( destVehicle->ServerMoveJob ) destVehicle->ServerMoveJob->release();
						changeUnitOwner ( destVehicle, srcVehicle->owner );
					}
				}
				else
				{
					// only on disabling units the infiltartor gets exp. As higher his level is as slower he rises onto the next one.
					// every 5 rankings he needs one succesfull disabling more, to get to the next ranking
					srcVehicle->CommandoRank += (float)1/((int)(((int)srcVehicle->CommandoRank+5)/5));

					int strength = srcVehicle->calcCommandoTurns ( destVehicle, destBuilding );
					if ( destVehicle )
					{
						// stop the vehicle and make it disabled
						destVehicle->Disabled = strength;
						destVehicle->data.speed = 0;
						destVehicle->data.shots = 0;
						if ( destVehicle->IsBuilding ) stopVehicleBuilding ( destVehicle );
						if ( destVehicle->ServerMoveJob ) destVehicle->ServerMoveJob->release();
						sendUnitData ( destVehicle, destVehicle->owner->Nr );
						for ( unsigned int i = 0; i < destVehicle->SeenByPlayerList.Size(); i++ )
						{
							sendUnitData ( destVehicle, destVehicle->SeenByPlayerList[i]->Nr );
						}
					}
					else if ( destBuilding )
					{
						// stop the vehicle and make it disabled
						destBuilding->Disabled = strength;
						destBuilding->data.shots = 0;
						destBuilding->ServerStopWork( true );
						sendDoStopWork ( destBuilding );
						sendUnitData ( destBuilding, destBuilding->owner->Nr );
						for ( unsigned int i = 0; i < destBuilding->SeenByPlayerList.Size(); i++ )
						{
							sendUnitData ( destBuilding, destBuilding->SeenByPlayerList[i]->Nr );
						}
					}
				}
				success = true;
			}
			else
			{
				// detect the infiltrator on failed action and let enemy units fire on him
				for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
				{
					cPlayer* player = (*PlayerList)[i];
					if ( player == srcVehicle->owner ) continue;
					if ( !player->ScanMap[srcVehicle->PosX+srcVehicle->PosY*Map->size] ) continue;
					
					srcVehicle->setDetectedByPlayer( player );
				}
				checkPlayerUnits();
				srcVehicle->InSentryRange();
			}
			srcVehicle->data.shots--;
			sendUnitData ( srcVehicle, srcVehicle->owner->Nr );
			sendCommandoAnswer ( success, steal, srcVehicle, srcVehicle->owner->Nr );
		}
		break;
	default:
		Log.write("Server: Can not handle message, type " + message->getTypeAsString(), cLog::eLOG_TYPE_NET_ERROR);
	}
	
	if ( bDebugCheckPos ) sendCheckVehiclePositions();
	CHECK_MEMORY;
	return 0;
}

//-------------------------------------------------------------------------------------
int cServer::getUpgradeCosts (sID& ID, cPlayer* player, bool bVehicle, 
							  int newDamage, int newMaxShots, int newRange, 
							  int newMaxAmmo, int newArmor, int newMaxHitPoints, 
							  int newScan, int newMaxSpeed)
{
	sUnitData* currentVersion = ID.getUnitData (player);
	sUnitData* startVersion = ID.getUnitData ();
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
	if (newMaxShots > currentVersion->max_shots)
	{
		int costForUpgrade = uc.getCostForUpgrade (startVersion->max_shots, currentVersion->max_shots, newMaxShots, cUpgradeCalculator::kShots, player->researchLevel);
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
	if (newMaxAmmo > currentVersion->max_ammo)
	{
		int costForUpgrade = uc.getCostForUpgrade (startVersion->max_ammo, currentVersion->max_ammo, newMaxAmmo, cUpgradeCalculator::kAmmo, player->researchLevel);
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
	if (newMaxHitPoints > currentVersion->max_hit_points)
	{
		int costForUpgrade = uc.getCostForUpgrade (startVersion->max_hit_points, currentVersion->max_hit_points, newMaxHitPoints, cUpgradeCalculator::kHitpoints, player->researchLevel);
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
	if (bVehicle && newMaxSpeed > currentVersion->max_speed)
	{
		int costForUpgrade = uc.getCostForUpgrade (startVersion->max_speed / 4, currentVersion->max_speed / 4, newMaxSpeed / 4, cUpgradeCalculator::kSpeed, player->researchLevel);
		if (costForUpgrade > 0)
			cost += costForUpgrade;
		else
			return 1000000; // error, invalid values received from client
	}
	
	return cost;
}

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
void cServer::makeLanding( int iX, int iY, cPlayer *Player, cList<sLanding>& List, bool bFixed )
{
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
		sLanding& Landing = List[i];
		Vehicle = landVehicle(iX, iY, iWidth, iHeight, Landing.UnitID.getVehicle(), Player);
		while ( !Vehicle )
		{
			iWidth += 2;
			iHeight += 2;
			Vehicle = landVehicle(iX, iY, iWidth, iHeight, Landing.UnitID.getVehicle(), Player);
		}
		if ( Landing.cargo && Vehicle )
		{
			Vehicle->data.cargo = Landing.cargo;
			sendUnitData ( Vehicle, Vehicle->owner->Nr );
		}
	}
}

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
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
			cBuildingIterator building = Map->fields[iOff].getBuildings();
			while ( building )
			{
				if ( building->data.is_road || building->data.is_expl_mine || building->data.is_connector )
				{
					deleteUnit ( building );
					building--;
				}
				building++;
			}
			iOff++;
			building = Map->fields[iOff].getBuildings();
			while ( building )
			{
				if ( building->data.is_road || building->data.is_expl_mine || building->data.is_connector )
				{
					deleteUnit ( building );
					building--;
				}
				building++;
			}
			iOff+=Map->size;
			building = Map->fields[iOff].getBuildings();
			while ( building )
			{
				if ( building->data.is_road || building->data.is_expl_mine || building->data.is_connector )
				{
					deleteUnit ( building );
					building--;
				}
				building++;
			}
			iOff--;
			building = Map->fields[iOff].getBuildings();
			while ( building )
			{
				if ( building->data.is_road || building->data.is_expl_mine || building->data.is_connector )
				{
					deleteUnit ( building );
					building--;
				}
				building++;
			}
		}
		else
		{
			deleteUnit ( Map->fields[iOff].getTopBuilding() );
			if ( !AddedBuilding->data.is_connector )
			{
				cBuildingIterator building = Map->fields[iOff].getBuildings();
				while ( building )
				{
					if ( building->data.is_road || building->data.is_expl_mine )
					{
						deleteUnit ( building );
						building--;
					}
					building++;
				}
			}
		}
	}

	Map->addBuilding( AddedBuilding, iPosX, iPosY );

	sendAddUnit ( iPosX, iPosY, AddedBuilding->iID, false, Building->data.ID, Player->Nr, bInit );

	// integrate the building to the base:
	Player->base.AddBuilding ( AddedBuilding );
	if ( AddedBuilding->data.is_mine )
	{
		sendProduceValues ( AddedBuilding );
		AddedBuilding->ServerStartWork();
	}
	AddedBuilding->makeDetection();
	return AddedBuilding;
}

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
void cServer::deleteUnit( cVehicle* vehicle )
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

	sendDeleteUnit( vehicle, -1 );


	cPlayer* owner = vehicle->owner;
	delete vehicle;

	if ( owner ) owner->DoScan();
}

//-------------------------------------------------------------------------------------
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
				if ( MapPlayer->ScanMap[iOff] == 1 && (!stealthUnit || NextVehicle->isDetectedByPlayer( MapPlayer ) || (MapPlayer->isDefeated && openMapDefeat) ) && !NextVehicle->Loaded )
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

				if ( MapPlayer->ScanMap[iOff] == 1  && (!stealthUnit || NextBuilding->isDetectedByPlayer( MapPlayer ) || (MapPlayer->isDefeated && openMapDefeat) ) )
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

//-------------------------------------------------------------------------------------
cPlayer *cServer::getPlayerFromNumber ( int iNum )
{
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
	{
		cPlayer* const p = (*PlayerList)[i];
		if (p->Nr == iNum) return p;
	}
	return NULL;
}

//-------------------------------------------------------------------------------------
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
		// defeated player are ignored when they hit the end button
		if ( getPlayerFromNumber( iPlayerNum )->isDefeated ) return;

		// check whether this player has already finished his turn
		for ( unsigned int i = 0; i < PlayerEndList.Size(); i++ )
		{
			if (PlayerEndList[i]->Nr == iPlayerNum) return;
		}
		PlayerEndList.Add ( getPlayerFromNumber ( iPlayerNum ) );
		bool firstTimeEnded = PlayerEndList.Size() == 1;

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
			if ( firstTimeEnded )
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

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
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
				for ( unsigned int k = 0; k < Building->SeenByPlayerList.Size(); k++ )
				{
					sendUnitData(Building, Building->SeenByPlayerList[k]->Nr );
				}
				sendUnitData ( Building, Building->owner->Nr );
				if ( Building->Disabled )
				{
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
				for ( unsigned int k = 0; k < Vehicle->SeenByPlayerList.Size(); k++ )
				{
					sendUnitData(Vehicle, Vehicle->SeenByPlayerList[k]->Nr );
				}
				sendUnitData ( Vehicle, Vehicle->owner->Nr );
				if ( Vehicle->Disabled )
				{
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

	// produce resources
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
	{
		(*PlayerList)[i]->base.handleTurnend();
	}

	// do research:
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
		(*PlayerList)[i]->doResearch();

	// make autosave
	if ( SettingsData.bAutoSave )
	{
		cSavegame Savegame ( 10 );	// autosaves are always in slot 10
		Savegame.save ( lngPack.i18n ( "Text~Settings~Autosave") + " " + lngPack.i18n ( "Text~Comp~Turn") + " " + iToStr ( iTurn ) );
	}

	checkDefeats();

	iWantPlayerEndNum = -1;
}

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
void cServer::checkDeadline ()
{
	static int lastCheckTime = SDL_GetTicks();
	if ( !iTimer0 ) return;
	if ( iTurnDeadline >= 0 && iDeadlineStartTime > 0 )
	{
		// stop time when waiting for reconnection
		if ( DisconnectedPlayerList.Size() > 0 )
		{
			iDeadlineStartTime += SDL_GetTicks()-lastCheckTime;
		}
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
	lastCheckTime = SDL_GetTicks();
}

//-------------------------------------------------------------------------------------
void cServer::addActiveMoveJob ( cServerMoveJob *MoveJob )
{
	ActiveMJobs.Add ( MoveJob );
}

//-------------------------------------------------------------------------------------
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

		// stop the job
		if ( MoveJob->bEndForNow && Vehicle )
		{
			Log.write(" Server: Movejob has end for now and will be stoped (delete from active ones)", cLog::eLOG_TYPE_NET_DEBUG);
			sendNextMove ( Vehicle, MJOB_STOP, MoveJob->iSavedSpeed );
			ActiveMJobs.Delete ( i );
			continue;
		}
		else if ( MoveJob->bFinished )
		{
			if ( Vehicle && Vehicle->ServerMoveJob == MoveJob )
			{
				Log.write(" Server: Movejob is finished and will be deleted now", cLog::eLOG_TYPE_NET_DEBUG);
				Vehicle->ServerMoveJob = NULL;
				Vehicle->moving = false;
				Vehicle->MoveJobActive = false;

				sendNextMove ( Vehicle, MJOB_FINISHED );
			}
			else Log.write(" Server: Delete movejob with nonactive vehicle (released one)", cLog::eLOG_TYPE_NET_DEBUG);
			delete MoveJob;

			//continue path building
			if ( Vehicle && Vehicle->BuildPath )
			{
				if ( Vehicle->data.cargo >= Vehicle->BuildCostsStart && Server->Map->possiblePlaceBuilding( *Vehicle->BuildingTyp.getUnitData(), Vehicle->PosX, Vehicle->PosY , Vehicle ))
				{
					Vehicle->IsBuilding = true;
					Vehicle->BuildCosts = Vehicle->BuildCostsStart;
					Vehicle->BuildRounds = Vehicle->BuildRoundsStart;
					sendBuildAnswer( true, Vehicle );
				}
				else
				{
					Vehicle->BuildPath = false;
					sendBuildAnswer( false, Vehicle );
				}
			}
			ActiveMJobs.Delete ( i );
			continue;
		}

		if ( Vehicle == NULL ) continue;


		if ( !Vehicle->moving )
		{
			if ( !MoveJob->checkMove() && !MoveJob->bFinished )
			{
				ActiveMJobs.Delete ( i );
				delete MoveJob;
				Vehicle->ServerMoveJob = NULL;
				Log.write( " Server: Movejob deleted and informed the clients to stop this movejob", LOG_TYPE_NET_DEBUG );
				continue;
			}
		}

		if ( MoveJob->iNextDir != Vehicle->dir )
		{
			// rotate vehicle
			if ( iTimer1 ) Vehicle->RotateTo ( MoveJob->iNextDir );
		}
		else
		{
			//move the vehicle
			if ( iTimer0 ) MoveJob->moveVehicle ();
		}
	}
}

//-------------------------------------------------------------------------------------
void cServer::Timer()
{
	iTimerTime++;
}

//-------------------------------------------------------------------------------------
void cServer::handleTimer()
{
	//iTimer0: 50ms
	//iTimer1: 100ms
	//iTimer2: 400ms

	static unsigned int iLast = 0;
	iTimer0 = 0 ;
	iTimer1 = 0;
	iTimer2 = 0;
	if ( iTimerTime != iLast )
	{
		iLast = iTimerTime;
		iTimer0 = 1;
		if (   iTimerTime & 0x1 ) iTimer1 = 1;
		if ( ( iTimerTime & 0x3 ) == 3 ) iTimer2 = 1;
	}
}

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
void cServer::destroyUnit( cVehicle* vehicle )
{
	int offset = vehicle->PosX + vehicle->PosY*Map->size;
	int value = 0;
	bool bigRubble = false;

	if ( vehicle->data.can_drive == DRIVE_AIR && vehicle->FlightHigh != 0 )
	{
		deleteUnit( vehicle );
		return;
	}

	//delete all buildings on the field, except connectors
	cBuildingIterator bi = (*Map)[offset].getBuildings();
	if ( bi && bi->data.is_connector ) bi++;
	
	while ( !bi.end )
	{
		if (!bi->owner) 
		{
			value += bi->RubbleValue*2;	
			if ( bi->data.is_big ) bigRubble = true;
		}
		else
		{
			value += bi->data.iBuilt_Costs;
		}
		deleteUnit( bi );
		bi++;
	}

	if ( vehicle->data.is_big )
	{
		bigRubble = true;
		bi = (*Map)[offset + 1].getBuildings();
		if ( bi && bi->data.is_connector ) bi++;
		while ( !bi.end )
		{
			value += bi->data.iBuilt_Costs;
			deleteUnit( bi );
			bi++;
		}

		bi = (*Map)[offset + Map->size].getBuildings();
		if ( bi && bi->data.is_connector ) bi++;
		while ( !bi.end )
		{
			value += bi->data.iBuilt_Costs;
			deleteUnit( bi );
			bi++;
		}

		bi = (*Map)[offset + 1 + Map->size].getBuildings();
		if ( bi && bi->data.is_connector ) bi++;
		while ( !bi.end )
		{
			value += bi->data.iBuilt_Costs;
			deleteUnit( bi );
			bi++;
		}
	}


	if ( !vehicle->data.is_human )
	{
		value+= vehicle->data.iBuilt_Costs;
	}
	if ( value > 0 )
	{
		addRubble( offset, value/2, bigRubble );
	}

	deleteUnit( vehicle );
}

//-------------------------------------------------------------------------------------
void cServer::destroyUnit(cBuilding *b)
{
	int offset = b->PosX + b->PosY * Map->size;
	int value = 0;
	bool big = false;
	bool isConnector = b->data.is_connector;

	cBuilding* topBuilding = Map->fields[offset].getTopBuilding();
	if ( topBuilding && topBuilding->data.is_big )
	{
		big = true;
		offset = topBuilding->PosX + topBuilding->PosY * Map->size;

		cBuildingIterator building = Map->fields[offset + 1].getBuildings();
		while ( building.size() > 0 )
		{
			if ( building->owner ) value += building->data.iBuilt_Costs;
			else value += building->RubbleValue*2;
			deleteUnit( building );
		}

		building = Map->fields[offset + Map->size].getBuildings();
		while ( building.size() > 0 )
		{
			if ( building->owner ) value += building->data.iBuilt_Costs;
			else value += building->RubbleValue*2;
			deleteUnit( building );
		}

		building = Map->fields[offset + Map->size + 1].getBuildings();
		while ( building.size() > 0 )
		{
			if ( building->owner ) value += building->data.iBuilt_Costs;
			else value += building->RubbleValue*2;
			deleteUnit( building );
		}
	}
 
	cBuildingIterator building = Map->fields[offset].getBuildings();
	while ( building.size() > 0 )
	{
		if ( building->owner ) value += building->data.iBuilt_Costs;
		else value += building->RubbleValue*2;
		deleteUnit( building );
	}

	if ( !isConnector || value > 2 )
	{
		addRubble( offset, value/2, big );
	}
}

//-------------------------------------------------------------------------------------
void cServer::addRubble( int offset, int value, bool big )
{
	if ( value <= 0 ) value = 1;

	if ( Map->IsWater(offset) ||
		 Map->fields[offset].getBuildings() )
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
		 Map->IsWater(offset + 1) ||
		 Map->fields[offset + 1].getBuildings() ))
	{
		addRubble( offset, value/4, false);
		addRubble( offset + Map->size, value/4, false);
		addRubble( offset + Map->size + 1, value/4, false);
		return;
	}

	if ( big && (
		Map->IsWater(offset + Map->size ) ||
		Map->fields[offset + Map->size].getBuildings() ))
	{
		addRubble( offset, value/4, false);
		addRubble( offset + 1, value/4, false);
		addRubble( offset + Map->size + 1, value/4, false);
		return;
	}

	if ( big && (
		Map->IsWater(offset + Map->size + 1 ) ||
		Map->fields[offset + Map->size + 1].getBuildings() ))
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

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
void cServer::deletePlayer( cPlayer *Player )
{
	// send messages to clients that the units have to be deleted
	cVehicle *Vehicle = Player->VehicleList;
	while ( Vehicle )
	{
		sendDeleteUnit( Vehicle, -1 );
		Vehicle = Vehicle->next;
	}
	cBuilding *Building = Player->BuildingList;
	while ( Building )
	{
		sendDeleteUnit( Building, -1 );
		Building = Building->next;
	}
	// remove the player of all detected by player lists
	for ( unsigned int playerNum = 0; playerNum < PlayerList->Size(); playerNum++ )
	{
		cPlayer *UnitPlayer = (*PlayerList)[playerNum];
		if ( UnitPlayer == Player ) continue;
		Vehicle = UnitPlayer->VehicleList;
		while ( Vehicle )
		{
			if ( ( Vehicle->data.is_stealth_land || Vehicle->data.is_stealth_sea ) && Vehicle->isDetectedByPlayer ( Player ) ) Vehicle->resetDetectedByPlayer ( Player );
			Vehicle = Vehicle->next;
		}
	}
	// delete the player
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

//-------------------------------------------------------------------------------------
void cServer::resyncPlayer ( cPlayer *Player, bool firstDelete )
{

	Log.write(" Server:  ============================= begin resync  ==========================", cLog::eLOG_TYPE_NET_DEBUG);
	cVehicle *Vehicle;
	cBuilding *Building;
	if ( firstDelete )
	{
		cPlayer *UnitPlayer;
		for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
		{
			UnitPlayer = (*PlayerList)[i];
			if ( UnitPlayer == Player ) continue;
			Vehicle = UnitPlayer->VehicleList;
			while ( Vehicle )
			{
				for ( unsigned int j = 0; j < Vehicle->SeenByPlayerList.Size(); j++ )
				{
					if ( Vehicle->SeenByPlayerList[j] == Player ) Vehicle->SeenByPlayerList.Delete ( j );
				}
				Vehicle = Vehicle->next;
			}
			Building = UnitPlayer->BuildingList;
			while ( Building )
			{
				for ( unsigned int j = 0; j < Building->SeenByPlayerList.Size(); j++ )
				{
					if ( Building->SeenByPlayerList[j] == Player ) Building->SeenByPlayerList.Delete ( j );
				}
				Building = Building->next;
			}
		}
		Building = neutralBuildings;
		while ( Building )
		{
			for ( unsigned int j = 0; j < Building->SeenByPlayerList.Size(); j++ )
			{
				if ( Building->SeenByPlayerList[j] == Player ) Building->SeenByPlayerList.Delete ( j );
			}
			Building = Building->next;
		}
		sendDeleteEverything ( Player->Nr );
	}
	sendTurn ( iTurn, Player );
	if ( iDeadlineStartTime > 0 ) sendTurnFinished ( -1, iTurnDeadline - Round( ( SDL_GetTicks() - iDeadlineStartTime )/1000 ), Player );
	sendResources ( Player );
	// send all units to the client
	Vehicle = Player->VehicleList;
	while ( Vehicle )
	{
		if ( !Vehicle->Loaded ) resyncVehicle ( Vehicle, Player );
		Vehicle = Vehicle->next;
	}
	Building = Player->BuildingList;
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
	}
	// send all subbases
	for ( unsigned int i = 0; i < Player->base.SubBases.Size(); i++ )
	{
		sendNewSubbase ( Player->base.SubBases[i], Player->Nr );
		sendAddSubbaseBuildings ( NULL, Player->base.SubBases[i], Player->Nr );
		sendSubbaseValues ( Player->base.SubBases[i], Player->Nr );
	}
	// refresh enemy units
	Player->DoScan();
	checkPlayerUnits();
	// send upgrades
	for ( unsigned int i = 0; i < UnitsData.vehicle.Size(); i++ )
	{
		if ( Player->VehicleData[i].version > 1 ) sendUnitUpgrades ( &Player->VehicleData[i], Player->Nr );
	}
	for ( unsigned int i = 0; i < UnitsData.building.Size(); i++ )
	{
		if ( Player->BuildingData[i].version > 1 ) sendUnitUpgrades ( &Player->BuildingData[i], Player->Nr );
	}
	// send credits
	sendCredits( Player->Credits, Player->Nr );
	// send research
	sendResearchLevel( &(Player->researchLevel), Player->Nr );
	sendRefreshResearchCount (Player->Nr);
	
	// FIXME: sending hudsettings doesn't work form yet
	//sendHudSettings ( &Player->HotHud, Player );

	Log.write(" Server:  ============================= end resync  ==========================", cLog::eLOG_TYPE_NET_DEBUG);
}

//-------------------------------------------------------------------------------------
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
	if ( Vehicle->hasAutoMoveJob ) sendSetAutomoving ( Vehicle );
	if ( Vehicle->DetectedByPlayerList.Size() > 0 ) sendDetectionState ( Vehicle );
}

//--------------------------------------------------------------------------
bool cServer::addMoveJob(int iSrc, int iDest, cVehicle* vehicle)
{
	bool bIsAir = ( vehicle->data.can_drive == DRIVE_AIR );
	cServerMoveJob *MoveJob = new cServerMoveJob( iSrc, iDest, bIsAir, vehicle );
	if ( !MoveJob->calcPath() )
	{
		delete MoveJob;
		vehicle->ServerMoveJob = NULL;
		return false;
	}

	sendMoveJobServer ( MoveJob, vehicle->owner->Nr );
	for ( unsigned int i = 0; i < vehicle->SeenByPlayerList.Size(); i++ )
	{
		sendMoveJobServer( MoveJob, vehicle->SeenByPlayerList[i]->Nr );
	}

	addActiveMoveJob ( MoveJob );
	return true;
}

//--------------------------------------------------------------------------
void cServer::changeUnitOwner ( cVehicle *vehicle, cPlayer *newOwner )
{
	// delete vehicle in the list of he old player
	cPlayer *oldOwner = vehicle->owner;
	cVehicle *vehicleList = oldOwner->VehicleList;
	while ( vehicleList )
	{
		if ( vehicleList == vehicle )
		{
			if ( vehicleList->prev )
			{
				vehicleList->prev->next = vehicleList->next;
				if ( vehicleList->next ) vehicleList->next->prev = vehicleList->prev;
			}
			else if ( vehicleList->next )
			{
				oldOwner->VehicleList = vehicleList->next;
				vehicleList->next->prev = NULL;
			}
			else oldOwner->VehicleList = NULL;
			break;
		}
		vehicleList = vehicleList->next;
	}
	// add the vehicle to the list of the new player
	vehicle->owner = newOwner;
	vehicle->next = newOwner->VehicleList;
	vehicle->prev = NULL;
	newOwner->VehicleList->prev = vehicle;
	newOwner->VehicleList = vehicle;

	// delete the unit on the clients and ad it with new owner again
	sendDeleteUnit ( vehicle, oldOwner->Nr );
	while ( vehicle->SeenByPlayerList.Size() )
	{
		sendDeleteUnit ( vehicle, vehicle->SeenByPlayerList[0]->Nr );
		vehicle->SeenByPlayerList.Delete ( 0 );
	}
	while ( vehicle->DetectedByPlayerList.Size() ) vehicle->DetectedByPlayerList.Delete ( 0 );
	sendAddUnit ( vehicle->PosX, vehicle->PosY, vehicle->iID, true, vehicle->data.ID, vehicle->owner->Nr, false );
	sendUnitData ( vehicle, vehicle->owner->Nr );
	sendSpecificUnitData ( vehicle );
	checkPlayerUnits();

	// let the unit work for his new owner
	if ( vehicle->data.can_survey )
	{
		sendVehicleResources( vehicle, Map );
		vehicle->doSurvey();
	}
	vehicle->makeDetection();
}

//--------------------------------------------------------------------------
void cServer::stopVehicleBuilding ( cVehicle *vehicle )
{
	if ( !vehicle->IsBuilding ) return;

	int iPos = vehicle->PosX+vehicle->PosY*Map->size;

	vehicle->IsBuilding = false;
	vehicle->BuildPath = false;

	if ( vehicle->data.can_build == BUILD_BIG )
	{
		Map->moveVehicle( vehicle, vehicle->BuildBigSavedPos );
		iPos = vehicle->BuildBigSavedPos;
		vehicle->owner->DoScan();
	}
	sendStopBuild ( vehicle->iID, iPos, vehicle->owner->Nr );
	for ( unsigned int i = 0; i < vehicle->SeenByPlayerList.Size(); i++ )
	{
		sendStopBuild ( vehicle->iID, iPos, vehicle->SeenByPlayerList[i]->Nr );
	}
}

void cServer::sideStepStealthUnit( int PosX, int PosY, cVehicle* vehicle, int bigOffset )
{
	sideStepStealthUnit( PosX, PosY, vehicle->data, vehicle->owner, bigOffset );
}

void cServer::sideStepStealthUnit( int PosX, int PosY, sUnitData& vehicleData, cPlayer* vehicleOwner, int bigOffset )
{
	//TODO: make sure, the stealth vehicle takes the direct diagonal move. Also when two straight moves would be shorter.

	if ( vehicleData.can_drive == DRIVE_AIR ) return;

	//first look for an undetected stealth unit
	cVehicle* stealthVehicle = Map->fields[PosX+PosY*Server->Map->size].getVehicles();
	if ( !stealthVehicle ) return;
	if ( stealthVehicle->owner == vehicleOwner ) return;
	if ( !stealthVehicle->data.is_stealth_land && !stealthVehicle->data.is_stealth_sea ) return;
	if ( stealthVehicle->isDetectedByPlayer( vehicleOwner )) return;

	//make sure a running movement is finished, before starting the side step move
	if ( stealthVehicle->moving ) stealthVehicle->ServerMoveJob->doEndMoveVehicle();

	//found a stealth unit. Try to find a place where the unit can move
	bool placeFound = false;
	int minCosts = 99;
	int bestX, bestY;
	for ( int x = PosX - 1; x <= PosX + 1; x++ )
	{
		if ( x < 0 || x >= Server->Map->size ) continue;
		for ( int y = PosY - 1; y <= PosY + 1; y++ )
		{
			if ( y < 0 || y >= Server->Map->size ) continue;
			if ( x == PosX && y == PosY ) continue;

			//when a bigOffet was passed, for example a contructor needs space for a big building
			//so not all directions are allowed for the side stepping
			if ( bigOffset != -1 )
			{
				int off = x + y*Server->Map->size;
				if (off == bigOffset ||
					off == bigOffset + 1 ||
					off == bigOffset + Server->Map->size ||
					off == bigOffset + Server->Map->size + 1 ) 	continue;
			}

			//check whether this field is a possible destination
			if ( !Server->Map->possiblePlace( stealthVehicle, x, y ) ) continue;

			//check costs of the move
			cPathCalculator pathCalculator(0, 0, 0, 0, Map, stealthVehicle );
			int costs = pathCalculator.calcNextCost(PosX, PosY, x, y);
			if ( costs > stealthVehicle->data.speed ) continue;

			//check whether the vehicle would be detected on the destination field
			bool detectOnDest = false;
			if ( stealthVehicle->data.is_stealth_land )
			{
				for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
				{
					if ( (*Server->PlayerList)[i] == stealthVehicle->owner ) continue;
					if ( (*Server->PlayerList)[i]->DetectLandMap[x+y*Map->size] ) detectOnDest = true;
				}
				if ( Server->Map->IsWater(x+y*Map->size,true) ) detectOnDest = true;
			}
			if ( stealthVehicle->data.is_stealth_sea )
			{
				for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
				{
					if ( (*Server->PlayerList)[i] == stealthVehicle->owner ) continue;
					if ( (*Server->PlayerList)[i]->DetectSeaMap[x+y*Map->size] ) detectOnDest = true;
				}
				if ( !Server->Map->IsWater(x+y*Map->size, true) ) detectOnDest = true;

				if ( stealthVehicle->data.can_drive == DRIVE_LANDnSEA )
				{
					cBuilding* b = Map->fields[x+y*Map->size].getBaseBuilding();
					if ( b && (b->data.is_road || b->data.is_bridge || b->data.is_platform )) detectOnDest = true;
				}
			}
			if ( detectOnDest ) continue;

			//take the move with the lowest costs. Decide randomly, when costs are equal
			if ( costs < minCosts || (costs == minCosts && random(2) ))
			{
				//this is a good candidate for a destination
				minCosts = costs;
				bestX = x;
				bestY = y;
				placeFound = true;
			}
		}
	}

	if ( placeFound )
	{
		Server->addMoveJob( PosX+PosY*Server->Map->size, bestX+bestY*Server->Map->size, stealthVehicle );
		stealthVehicle->ServerMoveJob->checkMove();	//begin the movment immediately, so no other unit can block the destination field
		return;
	}

	//sidestepping failed. Uncover the vehicle.
	stealthVehicle->setDetectedByPlayer( vehicleOwner );
	Server->checkPlayerUnits();
}
