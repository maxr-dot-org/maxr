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
#include "serverevents.h"
#include "menuevents.h"
#include "client.h"
#include "input.h"
#include "log.h"
#include "menus.h"

void cEventHandling::pushEvent(SDL_Event* const e)
{
	while (SDL_PushEvent(e) != 0) {}
	delete e;
}

void cEventHandling::HandleEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch ( event.type )
		{
		case SDL_KEYDOWN:
			if ( event.key.keysym.sym == SDLK_RETURN && event.key.keysym.mod &KMOD_ALT ) //alt+enter makes us go fullscreen|windowmode
			{
				SettingsData.bWindowMode = !SettingsData.bWindowMode;
				screen = SDL_SetVideoMode(SettingsData.iScreenW,SettingsData.iScreenH,SettingsData.iColourDepth,SDL_HWSURFACE|(SettingsData.bWindowMode?0:SDL_FULLSCREEN));
				SHOW_SCREEN
			}
			// Screenshot
			else if ( event.key.keysym.sym == SDLK_c && event.key.keysym.mod & KMOD_ALT )
			{
				time_t tTime;
				tm *tmTime;
				char timestr[16];
				string sTime;
				tTime = time ( NULL );
				tmTime = localtime ( &tTime );
				strftime( timestr, 16, "%d.%m.%y-%H%M%S", tmTime );

				string screenshotfile ="";
				#ifdef WIN32
					screenshotfile = (string)"Screen_" + timestr + ".bmp";
				#elif __amigaos4__
					screenshotfile = (string)"Screen_" + timestr + ".bmp";
				#else
					screenshotfile = SettingsData.sHome+PATH_DELIMITER+"Screen_" + timestr + ".bmp";
				#endif
				Log.write ( "Screenshot saved to "+screenshotfile, cLog::eLOG_TYPE_INFO );
				SDL_SaveBMP ( screen, screenshotfile.c_str() );
			}
			else
			{
				InputHandler->inputkey ( event.key.keysym );
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			InputHandler->inputMouseButton ( event.button );
			break;
		case NETWORK_EVENT:
			switch ( event.user.code )
			{
			case TCP_ACCEPTEVENT:
				// new socket accepted
				if ( ActiveMenu )
				{
					cNetMessage message ( MU_MSG_NEW_PLAYER );
					message.pushInt16 ( ((Sint16 *)event.user.data1)[0] );
					ActiveMenu->handleNetMessage ( &message );
					free ( event.user.data1 );
				}
				break;
			case TCP_RECEIVEEVENT:
				// new Data received
				{
				if ( !network ) break;
				sDataBuffer DataBuffer;
				memset(DataBuffer.data, 0, PACKAGE_LENGTH);
				DataBuffer.iLenght = network->read(SDL_SwapLE16(((Sint16*)event.user.data1)[0]), ((Sint16*)event.user.data1)[2], DataBuffer.data);

				if ( DataBuffer.iLenght != 0 )
				{
					if ( SDL_SwapLE16( ((Sint16*)(DataBuffer.data+1))[1] ) < FIRST_MENU_MESSAGE ) // Eventtypes for the client
					{
						// devite into messages
						SDL_Event* NewEvent = new SDL_Event;
						NewEvent->type = GAME_EVENT;

						// data1 is the real data
						NewEvent->user.data1 = malloc(DataBuffer.iLenght);
						memcpy(NewEvent->user.data1, DataBuffer.data, DataBuffer.iLenght);

						NewEvent->user.data2 = NULL;
						pushEvent( NewEvent );
					}
					else //message for the MultiPlayerMenu
					{
						if ( ActiveMenu )
						{
							cNetMessage message((char*)DataBuffer.data);
							ActiveMenu->handleNetMessage ( &message );
						}
					}
				}
				free ( event.user.data1 );
				break;
				}
			case TCP_CLOSEEVENT:
				if ( !network ) break;
				// Socket should be closed
				network->close ( ((Sint16 *)event.user.data1)[0] );
				if ( ActiveMenu && !Client )
				{
					cNetMessage message( MU_MSG_DEL_PLAYER );
					message.pushInt16 ( ((Sint16 *)event.user.data1)[0] );
					ActiveMenu->handleNetMessage ( &message );
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
				if ( Client) Client->HandleNetMessage( &message );
				break;
			}

		default:
			break;
		}
	}
}
