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
#include "events.h"
#include "network.h"
#include "menu.h"
#include "serverevents.h"
#include "client.h"

Uint32 eventTimerCallback(Uint32 interval, void *param)
{
	SDL_CondBroadcast( EventHandler->EventWait );
	return interval;
}

cEventHandling::cEventHandling()
{
	EventLock = NULL;
	EventWait = NULL;
	EventTimer = 0;
	init();
}

cEventHandling::~cEventHandling()
{
	quit();
}

int cEventHandling::init()
{
	EventLock = SDL_CreateMutex();
	if ( EventLock == NULL )
	{
		return -1;
	}

	EventWait = SDL_CreateCond();
	if ( EventWait == NULL )
	{
		return -1;
	}

	EventTimer = SDL_AddTimer(10, eventTimerCallback, NULL);
	if ( EventTimer == NULL  )
	{
		return -1;
	}

	return 0;
}

void cEventHandling::quit()
{
	SDL_DestroyMutex( EventLock );
	EventLock = NULL;

	SDL_DestroyCond( EventWait );
	EventWait = NULL;

	SDL_RemoveTimer( EventTimer );
}

void cEventHandling::pumpEvents()
{
	SDL_LockMutex( EventLock );
	SDL_PumpEvents();
	SDL_UnlockMutex( EventLock );
}

int cEventHandling::waitEvent( SDL_Event *event )
{
	int iVal = 0;
	SDL_LockMutex( EventLock );
	while ( ( iVal = SDL_PollEvent( event ) ) <= 0 )
	{
		SDL_CondWait( EventWait, EventLock );
	}
	SDL_UnlockMutex( EventLock );
	SDL_CondSignal( EventWait );

	return iVal;
}

int cEventHandling::pollEvent( SDL_Event *event )
{
	int iVal = 0;

	SDL_LockMutex ( EventLock );
	iVal = SDL_PollEvent ( event );
	SDL_UnlockMutex ( EventLock );

	if ( 0 < iVal )
	{
		SDL_CondSignal( EventWait );
	}

	return iVal;
}

int cEventHandling::pushEvent( SDL_Event *event )
{
	SDL_LockMutex( EventLock );
	while ( SDL_PushEvent( event ) == -1 )
	{
		SDL_CondWait( EventWait, EventLock );
	}
	//since SDL_PushEvent() copies the event, the old one has to be deleted
	delete event;
	SDL_UnlockMutex( EventLock );
	SDL_CondSignal( EventWait );

	return 1;
}

int cEventHandling::HandleEvents()
{
	SDL_Event event;
	while ( pollEvent ( &event ) )
	{
		switch ( event.type )
		{
		case SDL_KEYDOWN:
			if ( event.key.keysym.sym == SDLK_RETURN )
			{
				if( event.key.keysym.mod &KMOD_ALT ) //alt+enter makes us go fullscreen|windowmode
				{
					SettingsData.bWindowMode = !SettingsData.bWindowMode;
					screen = SDL_SetVideoMode(SettingsData.iScreenW,SettingsData.iScreenH,SettingsData.iColourDepth,SDL_HWSURFACE|(SettingsData.bWindowMode?0:SDL_FULLSCREEN));
					SHOW_SCREEN
				}
			}
			break;
		case NETWORK_EVENT:
			switch ( event.user.code )
			{
			case TCP_ACCEPTEVENT:
				// new socket accepted
				if ( MultiPlayerMenu )
				{
					cNetMessage *Message = new cNetMessage( MU_MSG_NEW_PLAYER );
					Message->pushInt16 ( ((Sint16 *)event.user.data1)[0] );
					MultiPlayerMenu->MessageList->Add ( Message );
					free ( event.user.data1 );
				}
				break;
			case TCP_RECEIVEEVENT:
				// new Data received
				{
				sDataBuffer *DataBuffer = new sDataBuffer;
				memset ( DataBuffer->data, 0, PACKAGE_LENGHT );
				if ( !network ) break;
				if ( ( DataBuffer->iLenght = network->read ( SDL_SwapLE16( ((Sint16 *)event.user.data1)[0] ), ((Sint16 *)event.user.data1)[2], DataBuffer->data ) ) != 0 )
				{
					// remove startcharacters from data
					memmove( DataBuffer->data, (DataBuffer->data+2), DataBuffer->iLenght-2 );
					DataBuffer->iLenght -= 2;

					if ( SDL_SwapLE16( ((Sint16*)DataBuffer->data)[0] ) < FIRST_MENU_MESSAGE ) // Eventtypes for the client
					{
						// devite into messages
						int iPos = 0;
						/*while ( )*/
						SDL_Event* NewEvent = new SDL_Event;
						NewEvent->type = GAME_EVENT;
						
						// data1 is the real data
						NewEvent->user.data1 = malloc ( DataBuffer->iLenght );
						memcpy ( NewEvent->user.data1, DataBuffer->data, DataBuffer->iLenght );

						NewEvent->user.data2 = NULL;
						pushEvent( NewEvent );
					}
					else //message for the MultiPlayerMenu
					{
						if ( MultiPlayerMenu )
						{
							cNetMessage *Message = new cNetMessage ( (char*) DataBuffer->data );
							Message->refertControlChars();
							MultiPlayerMenu->MessageList->Add ( Message );
						}
					}
					delete DataBuffer;
				}
				free ( event.user.data1 );
				break;
				}
			case TCP_CLOSEEVENT:
				// Socket should be closed
				network->close ( ((Sint16 *)event.user.data1)[0] );
				if ( MultiPlayerMenu && !Client )
				{
					cNetMessage *Message = new cNetMessage( MU_MSG_DEL_PLAYER );
					Message->pushInt16 ( ((Sint16 *)event.user.data1)[0] );
					MultiPlayerMenu->MessageList->Add ( Message );
				}
				else if ( Client )
				{
					cNetMessage message( GAME_EV_LOST_CONNECTION );
					message.pushInt16( ((Sint16*)event.user.data1)[0] );
					pushEvent( message.getGameEvent() );
				}
				free ( event.user.data1 );
				break;
			}
			break;
		case GAME_EVENT:
			{
				cNetMessage message( (char*) event.user.data1);
				free ( event.user.data1 );
				message.refertControlChars();
				Client->HandleNetMessage( &message );
				break;
			}

		default:
			break;
		}
	}
	return 0;
}
