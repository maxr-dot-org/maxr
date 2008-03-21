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
#include "eventmessages.h"
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
					sDataBuffer *DataBuffer = new sDataBuffer;
					((Sint16*)DataBuffer->data)[0] = MU_MSG_NEW_PLAYER;
					((Sint16*)DataBuffer->data)[1] = ((Sint16 *)event.user.data1)[0];
					MultiPlayerMenu->MessageList->Add ( DataBuffer );
					free ( event.user.data1 );
				}
				break;
			case TCP_RECEIVEEVENT:
				// new Data received
				{
				sDataBuffer *DataBuffer = new sDataBuffer;
				memset ( DataBuffer->data, 0, PACKAGE_LENGHT );
				if ( !network ) break;
				if ( ( DataBuffer->iLenght = network->read ( SDL_SwapLE16( ((Sint16 *)event.user.data1)[0] ), PACKAGE_LENGHT, DataBuffer->data ) ) != 0 )
				{
					if ( SDL_SwapLE16( ((Sint16*)DataBuffer->data)[0] ) < FIRST_MENU_MESSAGE ) // Eventtypes for the client
					{
						// will look like something this way:
						SDL_Event NewEvent;
						NewEvent.type = GAME_EVENT;
						NewEvent.user.code = SDL_SwapLE16( ((Sint16*)DataBuffer->data)[0] );

						// data1 is the real data
						NewEvent.user.data1 = malloc ( PACKAGE_LENGHT );
						memcpy ( NewEvent.user.data1, DataBuffer->data+2, PACKAGE_LENGHT-2 );

						NewEvent.user.data2 = NULL;
						pushEvent( &NewEvent );

						delete DataBuffer;
					}
					else // No events for the menus, here the data is a simple message
					{
						if ( MultiPlayerMenu )
						{
							MultiPlayerMenu->MessageList->Add ( DataBuffer );
						}
					}
				}
				free ( event.user.data1 );
				break;
				}
			case TCP_CLOSEEVENT:
				// Socket should be closed
				network->close ( ((Sint16 *)event.user.data1)[0] );
				if ( MultiPlayerMenu )
				{
					sDataBuffer *DataBuffer = new sDataBuffer;
					((Sint16*)DataBuffer->data)[0] = MU_MSG_DEL_PLAYER;
					((Sint16*)DataBuffer->data)[1] = ((Sint16 *)event.user.data1)[0];
					MultiPlayerMenu->MessageList->Add ( DataBuffer );
				}
				free ( event.user.data1 );
				break;
			}
			break;
		case GAME_EVENT:
			Client->HandleEvent( &event );
			break;

		default:
			break;
		}
	}
	return 0;
}
