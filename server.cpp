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

void cServer::init( cMap *map, cList<cPlayer*> *PlayerList, int iGameType, bool bPlayTurns )
{
	Map = map;
	this->PlayerList = PlayerList;
	this->iGameType = iGameType;
	this->bPlayTurns = bPlayTurns;
	bExit = false;
	bStarted = false;
	iActiveTurnPlayerNr = 0;
	PlayerEndList = new cList<int*>;
	iTurn = 1;
	iDeadlineStartTime = 0;
	iTurnDeadline = 10; // just temporary set to 10 seconds
	ActiveMJobs = new cList<cMJobs *>;
	AJobs = new cList<cServerAttackJob*>;
	iNextUnitID = 1;
	iTimerTime = 0;
	TimerID = SDL_AddTimer ( 50, ServerTimerCallback, this );

	EventQueue = new cList<SDL_Event *>;
	//NetMessageQueue = new cList<cNetMessage*>;

	QueueMutex = SDL_CreateMutex ();

	ServerThread = SDL_CreateThread( CallbackRunServerThread, this );
}

void cServer::kill()
{
	bExit = true;
	SDL_WaitThread ( ServerThread, NULL );
	SDL_RemoveTimer ( TimerID );

	while ( EventQueue->iCount )
	{
		delete EventQueue->Items[0];
		EventQueue->Delete (0);
	}
	delete EventQueue;

	/*while ( NetMessageQueue->iCount )
	{
		delete NetMessageQueue->Items[0];
		NetMessageQueue->Delete(0);
	}
	delete NetMessageQueue; */

	SDL_DestroyMutex ( QueueMutex );
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
	if ( EventQueue->iCount <= 0 )
	{
		return NULL;
	}

	SDL_LockMutex( QueueMutex );
	event = EventQueue->Items[0];
	lastEvent = event;
	EventQueue->Delete( 0 );
	SDL_UnlockMutex( QueueMutex );
	return event;
}

/*
cNetMessage* cServer::pollNetMessage()
{
	if ( NetMessageQueue->iCount <= 0 )
		return NULL;

	cNetMessage* message;
	SDL_LockMutex( QueueMutex );
	message = NetMessageQueue->Items[0];
	NetMessageQueue->Delete(0);
	SDL_UnlockMutex( QueueMutex );
	return message;
}
*/

int cServer::pushEvent( SDL_Event *event )
{
	SDL_LockMutex( QueueMutex );
	EventQueue->Add ( event );
	SDL_UnlockMutex( QueueMutex );
	return 0;
}

/*int cServer::pushNetMessage( cNetMessage* message )
{
	SDL_LockMutex( QueueMutex );
	NetMessageQueue->Add( message );
	SDL_UnlockMutex( QueueMutex );

	return 0;
}
*/

void cServer::sendNetMessage( cNetMessage* message, int iPlayerNum )
{
	message->iPlayerNr = iPlayerNum;

	cLog::write("Server: sending message,  type: " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );

	if ( iPlayerNum == -1 )
	{
		if ( network ) network->send( message->iLength, message->serialize( true ) );
		EventHandler->pushEvent( message->getGameEvent() );
		delete message;
		return;
	}

	cPlayer *Player = NULL;
	for ( int i = 0; i < PlayerList->iCount; i++ )
	{
		if ( ( Player = PlayerList->Items[i])->Nr == iPlayerNum ) break;
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
						NewEvent->user.data1 = malloc ( PACKAGE_LENGHT );
						memcpy ( NewEvent->user.data1, &((char*)event->user.data1)[2], PACKAGE_LENGHT-2 );

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

		/*cNetMessage* message = pollNetMessage();

		if ( message )
		{
			HandleNetMessage( message );
		}
		*/

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
	cLog::write("Server: received message, type: " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );

	switch ( message->iType )
	{
	case GAME_EV_LOST_CONNECTION:
		{
			int iSocketNum = message->popInt16();
			// This is just temporary so doesn't need to be translated
			string sMessage = "Lost connection to ";
			// get the name of player to who the connection has been lost
			for ( int i = 0; i < PlayerList->iCount; i++ )
			{
				if ( PlayerList->Items[i]->iSocketNum == iSocketNum )
				{
					sMessage += PlayerList->Items[i]->name;
					break;
				}
			}
			// get the lokal player number
			int iPlayerNum;
			for ( int i = 0; i < PlayerList->iCount; i++ )
			{
				if ( PlayerList->Items[i]->iSocketNum == MAX_CLIENTS )
				{
					iPlayerNum = PlayerList->Items[i]->Nr;
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
			cMJobs *MJob;
			int iCount = 0;
			int iWaypointOff;

			int iID = message->popInt16();
			int iSrcOff = message->popInt16();
			int iDestOff = message->popInt16();
			bool bPlane = message->popBool();
			int iReceivedCount = message->popInt16();

			cLog::write("(Server) Received MoveJob: VehicleID: " + iToStr( iID ) + ", SrcX: " + iToStr( iSrcOff%Map->size ) + ", SrcY: " + iToStr( iSrcOff/Map->size ) + ", DestX: " + iToStr( iDestOff%Map->size ) + ", DestY: " + iToStr( iDestOff/Map->size ) + ", WaypointCount: " + iToStr( iReceivedCount ), cLog::eLOG_TYPE_NET_DEBUG);
			// Add the waypoints to the movejob
			sWaypoint *Waypoint = ( sWaypoint* ) malloc ( sizeof ( sWaypoint ) );
			while ( iCount < iReceivedCount )
			{
				iWaypointOff = message->popInt16();
				Waypoint->X = iWaypointOff%Map->size;
				Waypoint->Y = iWaypointOff/Map->size;
				Waypoint->Costs = message->popInt16();
				Waypoint->next = NULL;

				if ( iCount == 0 )
				{
					if ( iWaypointOff == iSrcOff || iWaypointOff == iDestOff )
					{
						MJob = new cMJobs( Map, iSrcOff, iDestOff, bPlane, iID, PlayerList, true );
						if ( MJob->vehicle == NULL )
						{
							// warning, here is something wrong! ( out of sync? )
							cLog::write("(Server) Created new movejob but no vehicle found!", cLog::eLOG_TYPE_NET_WARNING);
							break;
						}
						MJob->waypoints = Waypoint;
					}
					else
					{
						cVehicle *Vehicle = getVehicleFromID( iID );
						if ( Vehicle == NULL || Vehicle->mjob == NULL )
						{
							// warning, here is something wrong! ( out of sync? )
							cLog::write("(Server) Error while adding waypoints: Can't find vehicle or movejob", cLog::eLOG_TYPE_NET_WARNING);
							break;
						}
						MJob = Vehicle->mjob;
						sWaypoint *LastWaypoint = MJob->waypoints;
						while ( LastWaypoint->next )
						{
							LastWaypoint = LastWaypoint->next;
						}
						LastWaypoint->next = Waypoint;
					}
				}
				iCount++;

				if ( iCount < iReceivedCount )
				{
					Waypoint->next = ( sWaypoint* ) malloc ( sizeof ( sWaypoint ) );
					Waypoint = Waypoint->next;
				}
			}
			// is the last waypoint in this message?
			if ( iWaypointOff == iDestOff )
			{
				MJob->CalcNextDir();
				addActiveMoveJob ( MJob );
				cLog::write("(Server) Added received movejob", cLog::eLOG_TYPE_NET_DEBUG);
				// send the movejob to all other player who can see this unit
				for ( int i = 0; i < MJob->vehicle->SeenByPlayerList->iCount; i++ )
				{
					sendMoveJobServer ( MJob, *MJob->vehicle->SeenByPlayerList->Items[i] );
				}
			}
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
				attackingVehicle = getVehicleFromID( message->popInt32() );
				if ( attackingVehicle == NULL ) break;
				if ( attackingVehicle->owner->Nr != message->iPlayerNr ) break;
			}
			else
			{
				int offset = message->popInt32();
				if ( offset < 0 || offset > Map->size * Map->size ) break;
				attackingBuilding = Map->GO[offset].top;
				if ( attackingBuilding == NULL ) break;
				if ( attackingBuilding->owner->Nr != message->iPlayerNr ) break;
			}

			//find target offset
			int targetOffset = message->popInt32();
			if ( targetOffset < 0 || targetOffset > Map->size * Map->size ) break;

			int targetID = message->popInt32();
			if ( targetID != 0 )
			{
				cVehicle* targetVehicle = getVehicleFromID( targetID );
				if ( targetVehicle == NULL ) break;
				targetOffset = targetVehicle->PosX + targetVehicle->PosY * Map->size;
			}

			//check if attack is possible
			//TODO: allow attacking empty terains
			//TODO: implement deleting of cAttackJobs
			if ( bIsVehicle )
			{
				if ( !attackingVehicle->CanAttackObject( targetOffset ) ) break;
				AJobs->Add( new cServerAttackJob( attackingVehicle, targetOffset ));
			}
			else
			{
				if ( !attackingBuilding->CanAttackObject( targetOffset ) ) break;
				AJobs->Add( new cServerAttackJob( attackingBuilding, targetOffset ));
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
						addUnit ( iX+k, iY+i, UnitsData.building+BNrOilStore, Player, true );
						addUnit ( iX+k, iY+i+1, UnitsData.building+BNrSmallGen, Player, true );
						addUnit ( iX+k+1, iY+i, UnitsData.building+BNrMine, Player, true );
						Building = Map->GO[iX+k+ ( iY+i ) *Map->size].top;
						Player->base->AddOil ( Building->SubBase, 4 );
						// TODO: send message that oil should be added here
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
	for ( int i = 0; i < List->iCount; i++ )
	{
		Landing = List->Items[i];
		Vehicle = landVehicle ( iX, iY, iWidth, iHeight, UnitsData.vehicle+Landing->id, Player );
		while ( !Vehicle )
		{
			iWidth += 2;
			iHeight += 2;
			Vehicle = landVehicle ( iX, iY, iWidth, iHeight, UnitsData.vehicle+Landing->id, Player );
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
	if ( !bInit ) AddedVehicle->InWachRange();

	sendAddUnit ( iPosX, iPosY, AddedVehicle->iID, true, Vehicle->nr, Player->Nr, bInit );
}

void cServer::addUnit( int iPosX, int iPosY, sBuilding *Building, cPlayer *Player, bool bInit )
{
	cBuilding *AddedBuilding;
	// generate the building:
	AddedBuilding = Player->AddBuilding ( iPosX, iPosY, Building );
	AddedBuilding->iID = iNextUnitID;
	iNextUnitID++;
	// place the building:
	int iOff = iPosX + Map->size*iPosY;
	if ( AddedBuilding->data.is_base )
	{
		if(Map->GO[iOff].base)
		{
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
			deleteBuilding ( Map->GO[iOff].top );
			Map->GO[iOff].top=AddedBuilding;
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteBuilding ( Map->GO[iOff].base );
				Map->GO[iOff].base = NULL;
			}
			iOff++;
			deleteBuilding ( Map->GO[iOff].top );
			Map->GO[iOff].top=AddedBuilding;
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteBuilding ( Map->GO[iOff].base );
				Map->GO[iOff].base=NULL;
			}
			iOff+=Map->size;
			deleteBuilding ( Map->GO[iOff].top );
			Map->GO[iOff].top=AddedBuilding;
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteBuilding ( Map->GO[iOff].base );
				Map->GO[iOff].base=NULL;
			}
			iOff--;
			deleteBuilding ( Map->GO[iOff].top );
			Map->GO[iOff].top=AddedBuilding;
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteBuilding ( Map->GO[iOff].base );
				Map->GO[iOff].base=NULL;
			}
		}
		else
		{
			deleteBuilding ( Map->GO[iOff].top );
			Map->GO[iOff].top=AddedBuilding;
			if ( !AddedBuilding->data.is_connector&&Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				deleteBuilding ( Map->GO[iOff].base );
				Map->GO[iOff].base=NULL;
			}
		}
	}
	if ( !bInit ) AddedBuilding->StartUp=10;
	// intigrate the building to the base:
	Player->base->AddBuilding ( AddedBuilding );

	sendAddUnit ( iPosX, iPosY, AddedBuilding->iID, false, Building->nr, Player->Nr, bInit );
}

void cServer::deleteBuilding( cBuilding *Building )
{
	if( Building )
	{
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
		}
		sendDeleteUnit( Building->PosX, Building->PosY, Building->owner->Nr, Building->iID, false, Building->owner->Nr, false, bBase, bSubBase );

		delete Building;
	}
}

void cServer::checkPlayerUnits ()
{
	cPlayer *UnitPlayer;	// The player whos unit is it
	cPlayer *MapPlayer;		// The player who is scaning for new units

	for ( int iUnitPlayerNum = 0; iUnitPlayerNum < PlayerList->iCount; iUnitPlayerNum++ )
	{
		UnitPlayer = PlayerList->Items[iUnitPlayerNum];
		cVehicle *NextVehicle = UnitPlayer->VehicleList;
		while ( NextVehicle != NULL )
		{
			for ( int iMapPlayerNum = 0; iMapPlayerNum < PlayerList->iCount; iMapPlayerNum++ )
			{
				if ( iMapPlayerNum == iUnitPlayerNum ) continue;
				MapPlayer = PlayerList->Items[iMapPlayerNum];
				if ( MapPlayer->ScanMap[NextVehicle->PosX+NextVehicle->PosY*Map->size] == 1 )
				{
					int i;
					for ( i = 0; i < NextVehicle->SeenByPlayerList->iCount; i++ )
					{
						if ( *NextVehicle->SeenByPlayerList->Items[i] == MapPlayer->Nr ) break;
					}
					if ( i == NextVehicle->SeenByPlayerList->iCount )
					{
						NextVehicle->SeenByPlayerList->Add ( &MapPlayer->Nr );
						sendAddEnemyUnit( NextVehicle, MapPlayer->Nr );
						sendUnitData( NextVehicle, MapPlayer->Nr );
						if ( NextVehicle->mjob ) sendMoveJobServer ( NextVehicle->mjob, MapPlayer->Nr );
					}
				}
				else
				{
					int i;
					for ( i = 0; i < NextVehicle->SeenByPlayerList->iCount; i++ )
					{
						if ( *NextVehicle->SeenByPlayerList->Items[i] == MapPlayer->Nr )
						{
							NextVehicle->SeenByPlayerList->Delete ( i );

							bool bPlane;
							if ( Map->GO[NextVehicle->PosX+NextVehicle->PosY*Map->size].plane == NextVehicle ) bPlane = true;
							else bPlane = false;
							sendDeleteUnit( NextVehicle->PosX, NextVehicle->PosY, NextVehicle->owner->Nr, NextVehicle->iID, true, MapPlayer->Nr, bPlane );
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
			for ( int iMapPlayerNum = 0; iMapPlayerNum < PlayerList->iCount; iMapPlayerNum++ )
			{
				if ( iMapPlayerNum == iUnitPlayerNum ) continue;
				MapPlayer = PlayerList->Items[iMapPlayerNum];
				if ( MapPlayer->ScanMap[NextBuilding->PosX+NextBuilding->PosY*Map->size] == 1 )
				{
					int i;
					for ( i = 0; i < NextBuilding->SeenByPlayerList->iCount; i++ )
					{
						if ( *NextBuilding->SeenByPlayerList->Items[i] == MapPlayer->Nr ) break;
					}
					if ( i == NextBuilding->SeenByPlayerList->iCount )
					{
						NextBuilding->SeenByPlayerList->Add ( &MapPlayer->Nr );
						sendAddEnemyUnit( NextBuilding, MapPlayer->Nr );
						sendUnitData( NextBuilding, Map, MapPlayer->Nr );
					}
				}
				else
				{
					int i;
					for ( i = 0; i < NextBuilding->SeenByPlayerList->iCount; i++ )
					{
						if ( *NextBuilding->SeenByPlayerList->Items[i] == MapPlayer->Nr )
						{
							NextBuilding->SeenByPlayerList->Delete ( i );

							bool bBase, bSubBase;
							if ( Map->GO[NextBuilding->PosX+NextBuilding->PosY*Map->size].base == NextBuilding ) bBase = true;
							else bBase = false;
							if ( Map->GO[NextBuilding->PosX+NextBuilding->PosY*Map->size].subbase == NextBuilding ) bSubBase = true;
							else bSubBase = false;
							sendDeleteUnit( NextBuilding->PosX, NextBuilding->PosY, NextBuilding->owner->Nr, NextBuilding->iID, false, MapPlayer->Nr, false, bBase, bSubBase );

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
	cPlayer *Player = NULL;
	for ( int i = 0; i < PlayerList->iCount; i++ )
	{
		if ( PlayerList->Items[i]->Nr == iNum )
		{
			Player = PlayerList->Items[i];
			break;
		}
	}
	return Player;
}

void cServer::handleEnd ( int iPlayerNum )
{
	if ( /* look if there are some things to to at this turnend "engine->DoEndActions()" */ false )
	{
		// send message to client what he has to do
		//sendChatMessage ( lngPack.i18n( "Text~Comp~Turn_Automove") );
	}
	else
	{
		string sReportMsg = "";
		int iVoiceNum;

		bool bChangeTurn = false;
		if ( iGameType == GAME_TYPE_SINGLE )
		{
			getTurnstartReport ( iPlayerNum, &sReportMsg, &iVoiceNum );

			sendMakeTurnEnd ( true, false, -1, sReportMsg, iVoiceNum, iPlayerNum );
			bChangeTurn = true;
		}
		else if ( iGameType == GAME_TYPE_HOTSEAT || bPlayTurns )
		{
			bool bWaitForPlayer = ( iGameType == GAME_TYPE_TCPIP && bPlayTurns );
			iActiveTurnPlayerNr++;
			if ( iActiveTurnPlayerNr >= PlayerList->iCount )
			{
				iActiveTurnPlayerNr = 0;
				if ( iGameType == GAME_TYPE_HOTSEAT )
				{
					getTurnstartReport ( iPlayerNum, &sReportMsg, &iVoiceNum );
					sendMakeTurnEnd ( true, bWaitForPlayer, PlayerList->Items[iActiveTurnPlayerNr]->Nr, sReportMsg, iVoiceNum, iPlayerNum );
				}
				else
				{
					for ( int i = 0; i < PlayerList->iCount; i++ )
					{
						getTurnstartReport ( i, &sReportMsg, &iVoiceNum );
					sendMakeTurnEnd ( true, bWaitForPlayer, PlayerList->Items[iActiveTurnPlayerNr]->Nr, sReportMsg, iVoiceNum, i );
					}
				}
				bChangeTurn = true;
			}
			else
			{
				if ( iGameType == GAME_TYPE_HOTSEAT )
				{
					getTurnstartReport ( iPlayerNum, &sReportMsg, &iVoiceNum );
					sendMakeTurnEnd ( false, bWaitForPlayer, PlayerList->Items[iActiveTurnPlayerNr]->Nr, sReportMsg, iVoiceNum, iPlayerNum );
					// TODO: in hotseat: maybe send information to client about the next player
				}
				else
				{
					for ( int i = 0; i < PlayerList->iCount; i++ )
					{
						getTurnstartReport ( i, &sReportMsg, &iVoiceNum );
						sendMakeTurnEnd ( false, bWaitForPlayer, PlayerList->Items[iActiveTurnPlayerNr]->Nr, sReportMsg, iVoiceNum, i );
					}
				}
			}
		}
		else // it's a simultanous TCP/IP multiplayer game
		{
			// check whether this player has already finished his turn
			for ( int i = 0; i < PlayerEndList->iCount; i++ )
			{
				if ( *PlayerEndList->Items[i] == iPlayerNum ) return;
			}
			PlayerEndList->Add ( &getPlayerFromNumber ( iPlayerNum )->Nr );

			if ( PlayerEndList->iCount >= PlayerList->iCount )
			{
				while ( PlayerEndList->iCount )
				{
					PlayerEndList->Delete ( 0 );
				}
				for ( int i = 0; i < PlayerList->iCount; i++ )
				{
					getTurnstartReport ( i, &sReportMsg, &iVoiceNum );
					sendMakeTurnEnd ( true, false, -1, sReportMsg, iVoiceNum, i );
				}
				iDeadlineStartTime = 0;
				bChangeTurn = true;
			}
			else
			{
				if ( PlayerEndList->iCount == 1 )
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
		if ( bChangeTurn ) iTurn++;
		makeTurnEnd ( iPlayerNum, bChangeTurn );
	}
}

void cServer::makeTurnEnd ( int iPlayerNum, bool bChangeTurn )
{
	cPlayer *CallerPlayer = getPlayerFromNumber ( iPlayerNum );
	// reload all buildings
	for ( int i = 0; i < PlayerList->iCount; i++ )
	{
		cBuilding *Building;
		cPlayer *Player;
		Player = PlayerList->Items[i];

		Building = Player->BuildingList;
		while ( Building )
		{
			if ( Building->Disabled )
			{
				Building->Disabled--;
				if ( Building->Disabled )
				{
					Building = Building->next;
					continue;
				}
			}
			if ( Building->data.can_attack && bChangeTurn ) Building->RefreshData();

			for ( int k = 0; k < Building->SeenByPlayerList->iCount; k++ )
			{
				sendUnitData ( Building, Map, *Building->SeenByPlayerList->Items[k] );
			}
			sendUnitData ( Building, Map, Building->owner->Nr );
			Building = Building->next;
		}
	}

	// reload all vehicles
	for ( int i = 0; i < PlayerList->iCount; i++ )
	{
		cVehicle *Vehicle;
		cPlayer *Player;
		Player = PlayerList->Items[i];

		Vehicle = Player->VehicleList;
		while ( Vehicle )
		{
			if ( CallerPlayer && Vehicle->detection_override && Vehicle->owner == CallerPlayer )
			{
				Vehicle->detected = false;
				Vehicle->detection_override = false;
			}
			if ( Vehicle->Disabled )
			{
				Vehicle->Disabled--;
				if ( Vehicle->Disabled )
				{
					Vehicle = Vehicle->next;
					continue;
				}
			}

			if ( bChangeTurn ) Vehicle->RefreshData();
			if ( Vehicle->mjob ) Vehicle->mjob->EndForNow = false;

			for ( int k = 0; k < Vehicle->SeenByPlayerList->iCount; k++ )
			{
				sendUnitData ( Vehicle, *Vehicle->SeenByPlayerList->Items[k] );
			}
			sendUnitData ( Vehicle, Vehicle->owner->Nr );
			Vehicle = Vehicle->next;
		}
	}
	// Gun'em down:
	for ( int i = 0; i < PlayerList->iCount; i++ )
	{
		cVehicle *Vehicle;
		cPlayer *Player;
		Player = PlayerList->Items[i];

		Vehicle = Player->VehicleList;
		while ( Vehicle )
		{
			//Vehicle->InWachRange();
			Vehicle = Vehicle->next;
		}
	}

	// TODO: implement these things

	// produce resources:
	//game->ActivePlayer->base->Rundenende();

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
	while ( Player->ReportBuildings->iCount )
	{
		Report = Player->ReportBuildings->Items[0];
		if ( iCount ) *sReportMsg += ", ";
		iCount += Report->anz;
		sTmp = iToStr( Report->anz ) + " " + Report->name;
		*sReportMsg += Report->anz > 1 ? sTmp : Report->name;
		Player->ReportBuildings->Delete ( 0 );
		delete Report;
	}
	while ( Player->ReportVehicles->iCount )
	{
		Report = Player->ReportVehicles->Items[0];
		if ( iCount ) *sReportMsg+=", ";
		iCount += Report->anz;
		sTmp = iToStr( Report->anz ) + " " + Report->name;
		*sReportMsg += Report->anz > 1 ? sTmp : Report->name;
		Player->ReportVehicles->Delete ( 0 );
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
		for ( int i = 0; i < Player->ReportVehicles->iCount; i++ )
		{
			Report = Player->ReportVehicles->Items[i];
			if ( Report->name.compare ( sName ) == 0 )
			{
				Report->anz++;
				return;
			}
		}
		Report = new sReport;
		Report->name = sName;
		Report->anz = 1;
		Player->ReportVehicles->Add ( Report );
	}
	else
	{
		for ( int i = 0; i < Player->ReportBuildings->iCount; i++ )
		{
			Report = Player->ReportBuildings->Items[i];
			if ( Report->name.compare ( sName ) == 0 )
			{
				Report->anz++;
				return;
			}
		}
		Report = new sReport;
		Report->name = sName;
		Report->anz = 1;
		Player->ReportBuildings->Add ( Report );
	}
}

void cServer::checkDeadline ()
{
	if ( iTurnDeadline >= 0 && iDeadlineStartTime > 0 )
	{
		if ( SDL_GetTicks() - iDeadlineStartTime > (unsigned int)iTurnDeadline*1000 )
		{
			while ( PlayerEndList->iCount )
			{
				PlayerEndList->Delete ( 0 );
			}
			string sReportMsg = "";
			int iVoiceNum;

			for ( int i = 0; i < PlayerList->iCount; i++ )
			{
				getTurnstartReport ( i, &sReportMsg, &iVoiceNum );
				sendMakeTurnEnd ( true, false, -1, sReportMsg, iVoiceNum, i );
			}
			iTurn++;
			iDeadlineStartTime = 0;
			makeTurnEnd( -1, true );
		}
	}
}

void cServer::addActiveMoveJob ( cMJobs *MJob )
{
	ActiveMJobs->Add ( MJob );
	MJob->Suspended = false;
}

void cServer::handleMoveJobs ()
{
	for ( int i = 0; i < ActiveMJobs->iCount; i++ )
	{
		cMJobs *MJob;
		cVehicle *Vehicle;

		MJob = ActiveMJobs->Items[i];
		Vehicle = MJob->vehicle;

		if ( MJob->finished || MJob->EndForNow )
		{
			// stop the job
			if ( MJob->EndForNow && Vehicle )
			{
				cLog::write("(Server) Movejob has end for now and will be stoped (delete from active ones)", cLog::eLOG_TYPE_NET_DEBUG);
				for ( int i = 0; i < MJob->vehicle->SeenByPlayerList->iCount; i++ )
				{
					sendNextMove ( MJob->vehicle->iID, MJob->vehicle->PosX+MJob->vehicle->PosY*Map->size, MJOB_STOP, *MJob->vehicle->SeenByPlayerList->Items[i] );
				}
				sendNextMove ( MJob->vehicle->iID, MJob->vehicle->PosX+MJob->vehicle->PosY*Map->size, MJOB_STOP, MJob->vehicle->owner->Nr );
			}
			else
			{
				if ( Vehicle && Vehicle->mjob == MJob )
				{
					cLog::write("(Server) Movejob is finished and will be deleted now", cLog::eLOG_TYPE_NET_DEBUG);
					Vehicle->mjob = NULL;
					Vehicle->moving = false;
					Vehicle->MoveJobActive = false;

					for ( int i = 0; i < MJob->vehicle->SeenByPlayerList->iCount; i++ )
					{
						sendNextMove ( MJob->vehicle->iID, MJob->vehicle->PosX+MJob->vehicle->PosY*Map->size, MJOB_STOP, *MJob->vehicle->SeenByPlayerList->Items[i] );
					}
					sendNextMove ( MJob->vehicle->iID, MJob->vehicle->PosX+MJob->vehicle->PosY*Map->size, MJOB_STOP, MJob->vehicle->owner->Nr );
				}
				else cLog::write("(Server) Delete movejob with nonactive vehicle (released one)", cLog::eLOG_TYPE_NET_DEBUG);
				delete MJob;
			}
			ActiveMJobs->Delete ( i );
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
	bool bWachRange;
	if ( !MJob->vehicle || !MJob->waypoints || !MJob->waypoints->next ) return;
	bWachRange = false;//vehicle->InWachRange();
	if ( !MJob->CheckPointNotBlocked ( MJob->waypoints->next->X, MJob->waypoints->next->Y ) || bWachRange )
	{
		cLog::write( "(Server) Next point is blocked: ID: " + iToStr ( MJob->vehicle->iID ) + ", X: " + iToStr ( MJob->waypoints->next->X ) + ", Y: " + iToStr ( MJob->waypoints->next->Y ), LOG_TYPE_NET_DEBUG );
		// if the next point would be the last, finish the job here
		if ( MJob->waypoints->next->X == MJob->DestX && MJob->waypoints->next->Y == MJob->DestY )
		{
			MJob->finished = true;
		}
		// else delete the movejob and inform the client that he has to find a new path
		else
		{
			for ( int i = 0; i < MJob->vehicle->SeenByPlayerList->iCount; i++ )
			{
				sendNextMove ( MJob->vehicle->iID, MJob->vehicle->PosX+MJob->vehicle->PosY*Map->size, MJOB_BLOCKED, *MJob->vehicle->SeenByPlayerList->Items[i] );
			}
			sendNextMove ( MJob->vehicle->iID, MJob->vehicle->PosX+MJob->vehicle->PosY*Map->size, MJOB_BLOCKED, MJob->vehicle->owner->Nr );

			for ( int i = 0; i < ActiveMJobs->iCount; i++ )
			{
				if ( MJob == ActiveMJobs->Items[i] )
				{
					ActiveMJobs->Delete ( i );
					break;
				}
			}
			MJob->vehicle->mjob = NULL;
			delete MJob;
			cLog::write( "(Server) Movejob deleted and informed the clients to stop this movejob", LOG_TYPE_NET_DEBUG );
		}
		return;
	}

	// not enough waypoints for this move
	if ( MJob->vehicle->data.speed < MJob->waypoints->next->Costs )
	{
		cLog::write( "(Server) Vehicle has not enough waypoints for the next move -> EndForNow: ID: " + iToStr ( MJob->vehicle->iID ) + ", X: " + iToStr ( MJob->waypoints->next->X ) + ", Y: " + iToStr ( MJob->waypoints->next->Y ), LOG_TYPE_NET_DEBUG );
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
	for ( int i = 0; i < MJob->vehicle->SeenByPlayerList->iCount; i++ )
	{
		sendNextMove ( MJob->vehicle->iID, MJob->vehicle->PosX+MJob->vehicle->PosY*Map->size, MJOB_OK, *MJob->vehicle->SeenByPlayerList->Items[i] );
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
		cLog::write("(Server) Vehicle reached the next field: ID: " + iToStr ( Vehicle->iID )+ ", X: " + iToStr ( Vehicle->mjob->waypoints->next->X ) + ", Y: " + iToStr ( Vehicle->mjob->waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);
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
		if ( !Vehicle->data.can_detect_mines && Vehicle->data.can_drive != DRIVE_AIR && Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].base && Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].base->data.is_expl_mine && Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].base->owner != Vehicle->owner )
		{
			Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].base->detonate();
			Vehicle->moving = false;
			Vehicle->WalkFrame = 0;
			if ( Vehicle->mjob )
			{
				Vehicle->mjob->release();
			}
		}

		// hide again if necessary
		if ( Vehicle->detection_override )
		{
			Vehicle->detected = false;
			Vehicle->detection_override = false;
		}

		// search for resources if necessary
		if ( Vehicle->data.can_survey )
		{
			sendResources( Vehicle, Map );
			Vehicle->doSurvey();
		}

		// TODO: let other units fire on this one
		//Vehicle->InWachRange();

		// search for mines if necessary
		if ( Vehicle->data.can_detect_mines )
		{
			// TODO: implement this function
			//Vehicle->detectMines();
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
				for ( int i = 0; i < Vehicle->SeenByPlayerList->iCount; i++ )
				{
					sendUnitData( Vehicle, *Vehicle->SeenByPlayerList->Items[i] );
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
	for ( int i = 0; i < PlayerList->iCount; i++ )
	{
		Vehicle = PlayerList->Items[i]->VehicleList;
		while ( Vehicle )
		{
			if ( Vehicle->iID == iID ) return Vehicle;
			Vehicle = Vehicle->next;
		}
	}
	return NULL;
}

void cServer::releaseMoveJob ( cMJobs *MJob )
{
	cLog::write ( "(Server) Released old movejob", cLog::eLOG_TYPE_NET_DEBUG );
	for ( int i = 0; i < ActiveMJobs->iCount; i++ )
	{
		if ( MJob == ActiveMJobs->Items[i] ) return;
	}
	addActiveMoveJob ( MJob );
	cLog::write ( "(Server) Added released movejob to avtive ones", cLog::eLOG_TYPE_NET_DEBUG );
}
