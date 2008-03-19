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
#include "eventmessages.h"

int CallbackRunServerThread( void *arg )
{
	cServer *Server = (cServer *) arg;
	Server->run();
	return 0;
}

void cServer::init()
{
	bExit = false;

	QueueMutex = SDL_CreateMutex ();

	ServerThread = SDL_CreateThread( CallbackRunServerThread, this );
}

int cServer::pollEvent( SDL_Event *event )
{
	if ( EventQueue->iCount <= 0 )
	{
		event = NULL;
		return 0;
	}

	SDL_LockMutex( QueueMutex );
	event = EventQueue->Items[0];
	EventQueue->Delete( 0 );
	SDL_UnlockMutex( QueueMutex );
	return 1;
}

int cServer::pushEvent( SDL_Event *event )
{
	SDL_LockMutex( QueueMutex );
	EventQueue->Add ( event );
	SDL_UnlockMutex( QueueMutex );
	return 0;
}

void cServer::run()
{
	while ( !bExit )
	{
		SDL_Event event;
		if ( pollEvent ( &event ) != NULL )
		{
			switch ( event.type )
			{
			case NETWORK_EVENT:
				switch ( event.user.code )
				{
				case TCP_ACCEPTEVENT:
					break;
				case TCP_RECEIVEEVENT:
					// new Data received
					{
						SDL_Event NewEvent;
						NewEvent.type = GAME_EVENT;
						NewEvent.user.code = SDL_SwapLE16( ((Sint16*)event.user.data1)[0] );

						// data1 is the real data
						NewEvent.user.data1 = malloc ( PACKAGE_LENGHT-2 );
						memcpy ( NewEvent.user.data1, (char*)event.user.data1+2, PACKAGE_LENGHT-2 );

						NewEvent.user.data2 = NULL;
						pushEvent( &NewEvent );
					}
					break;
				case TCP_CLOSEEVENT:
					{
					// Socket should be closed
					network->close ( ((Sint16 *)event.user.data1)[0] );
					// Lost Connection
					SDL_Event NewEvent;
					NewEvent.type = GAME_EVENT;
					NewEvent.user.code = GAME_EV_LOST_CONNECTION;
					NewEvent.user.data1 = malloc ( sizeof ( Sint16 ) );
					((Sint16*)NewEvent.user.data1)[0] = ((Sint16*)event.user.data1)[0];
					NewEvent.user.data2 = NULL;
					pushEvent( &NewEvent );
					}
					break;
				}
			case GAME_EVENT:
				HandleEvent( &event );
				break;

			default:
				break;
			}
		}
		SDL_Delay( 10 );
	}
}

int cServer::HandleEvent( SDL_Event *event )
{
	void *data = event->user.data1;
	switch ( event->user.code )
	{
	case GAME_EV_LOST_CONNECTION:
		break;
	case GAME_EV_DEL_PLAYER:
		break;
	case GAME_EV_CHAT:
		break;
	}
	return 0;
}
