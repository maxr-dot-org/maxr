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
	iPlayerEndCount = 0;
	iTurn = 1;
	iDeadlineStartTime = 0;
	iTurnDeadline = 10; // just temporary set to 10 seconds

	EventQueue = new cList<SDL_Event *>;
	//NetMessageQueue = new cList<cNetMessage*>;

	QueueMutex = SDL_CreateMutex ();

	ServerThread = SDL_CreateThread( CallbackRunServerThread, this );
}

void cServer::kill()
{
	bExit = true;
	SDL_WaitThread ( ServerThread, NULL );

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

	cLog::write("Server: sending message,  type: " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NETWORK );

	if ( iPlayerNum == -1 )
	{
		if ( network ) network->send( PACKAGE_LENGHT, message->serialize() );
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
		cLog::write("Server: Error: Player " + iToStr(iPlayerNum) + " not found.", cLog::eLOG_TYPE_NETWORK);
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
		if ( network ) network->sendTo( Player->iSocketNum, PACKAGE_LENGHT, message->serialize() );
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
						memcpy ( NewEvent->user.data1, (char*)event->user.data1, PACKAGE_LENGHT );

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

		SDL_Delay( 10 );
	}
}

int cServer::HandleNetMessage( cNetMessage *message )
{
	cLog::write("Server: received message, type: " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NETWORK );

	switch ( message->iType )
	{
	case GAME_EV_LOST_CONNECTION:
		break;
	case GAME_EV_CHAT_CLIENT:
		cNetMessage* newMessage;
		newMessage = new cNetMessage( GAME_EV_CHAT_SERVER );
		newMessage->pushString( message->popString() );
		sendNetMessage( newMessage );
		break;
	case GAME_EV_WANT_TO_END_TURN:
		handleEnd ( message->iPlayerNr );
		break;

	default:
		cLog::write("Server: Error: Can not handle message, type " + iToStr(message->iType), cLog::eLOG_TYPE_NETWORK);
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
		AddedVehicle->DoSurvey();
	}
	if ( !bInit ) AddedVehicle->InWachRange();

	sendAddUnit ( iPosX, iPosY, true, Vehicle->nr, Player->Nr, bInit );
}

void cServer::addUnit( int iPosX, int iPosY, sBuilding *Building, cPlayer *Player, bool bInit )
{
	cBuilding *AddedBuilding;
	// generate the building:
	AddedBuilding = Player->AddBuilding ( iPosX, iPosY, Building );
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

	sendAddUnit ( iPosX, iPosY, false, Building->nr, Player->Nr, bInit );
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
		if ( Map->GO[Building->PosX+Building->PosY*Map->size].base == Building ) bBase = true;
		else bBase = false;
		if ( Map->GO[Building->PosX+Building->PosY*Map->size].subbase == Building ) bSubBase = true;
		else bSubBase = false;
		sendDeleteUnit( Building->PosX, Building->PosY, Building->owner->Nr, false, Building->owner->Nr, false, bBase, bSubBase );

		delete Building;
	}
}

void cServer::checkPlayerUnits ()
{
	cPlayer *UnitPlayer, *MapPlayer;
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
							sendDeleteUnit( NextVehicle->PosX, NextVehicle->PosY, NextVehicle->owner->Nr, true, NextVehicle->owner->Nr, bPlane );
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
							sendDeleteUnit( NextBuilding->PosX, NextBuilding->PosY, NextBuilding->owner->Nr, false, NextBuilding->owner->Nr, false, bBase, bSubBase );

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
	cPlayer *Player;
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
			iPlayerEndCount++;
			if ( iPlayerEndCount >= PlayerList->iCount )
			{
				iPlayerEndCount = 0;
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
				if ( iPlayerEndCount == 1 )
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
		bool bShieldChaned;
		cBuilding *Building;
		cPlayer *Player;
		Player = PlayerList->Items[i];

		bShieldChaned = false;
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
			if ( Building->IsWorking && Building->data.max_shield && Building->data.shield < Building->data.max_shield )
			{
				Building->data.shield += 10;
				if ( Building->data.shield > Building->data.max_shield ) Building->data.shield = Building->data.max_shield;
				bShieldChaned = true;
			}
			Building = Building->next;
			for ( int k = 0; k < Building->SeenByPlayerList->iCount; k++ )
			{
				sendUnitData ( Building, Map, *Building->SeenByPlayerList->Items[k] );
			}
		}
		if ( bShieldChaned )
		{
			Player->CalcShields();
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
			if ( Vehicle->detection_override && Vehicle->owner == CallerPlayer )
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

			Vehicle = Vehicle->next;
			for ( int k = 0; k < Vehicle->SeenByPlayerList->iCount; k++ )
			{
				sendUnitData ( Vehicle, *Vehicle->SeenByPlayerList->Items[k] );
			}
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
			iPlayerEndCount = 0;
			string sReportMsg = "";
			int iVoiceNum;

			for ( int i = 0; i < PlayerList->iCount; i++ )
			{
				getTurnstartReport ( i, &sReportMsg, &iVoiceNum );
				sendMakeTurnEnd ( true, false, -1, sReportMsg, iVoiceNum, i );
			}
			iTurn++;
			iDeadlineStartTime = 0;
		}
	}
}
