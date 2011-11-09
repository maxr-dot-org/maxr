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
#include "settings.h"
#include "files.h"
#include "video.h"

void cEventHandling::pushEvent(cNetMessage* message)
{
	eventQueue.write(message);
}

void cEventHandling::HandleEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch ( event.type )
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if ( event.key.state == SDL_PRESSED && event.key.keysym.sym == SDLK_RETURN && event.key.keysym.mod &KMOD_ALT ) //alt+enter makes us go fullscreen|windowmode
			{
				Video.setWindowMode(!Video.getWindowMode(), true);
			}
			// Screenshot
			else if ( event.key.state == SDL_PRESSED && event.key.keysym.sym == SDLK_c && event.key.keysym.mod & KMOD_ALT )
			{
				time_t tTime;
				tm *tmTime;
				char timestr[16];
				string sTime;
				tTime = time ( NULL );
				tmTime = localtime ( &tTime );
				strftime( timestr, 16, "%d.%m.%y-%H%M%S", tmTime );

				string screenshotfile = getUserScreenshotsDir();
				screenshotfile.append ((string)"Screen_" + timestr + ".bmp");
				Log.write ( "Screenshot saved to " + screenshotfile, cLog::eLOG_TYPE_INFO );
				SDL_SaveBMP ( screen, screenshotfile.c_str() );
				if (Client != 0)
					Client->addMessage(lngPack.i18n ( "Text~Comp~Screenshot_Done", screenshotfile ));
			}
			else
			{
				InputHandler->inputkey ( event.key );
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			InputHandler->inputMouseButton ( event.button );
			break;
		case SDL_QUIT:
			{
				Quit();
				break;
			}

		default:
			break;
		}
	}

	cNetMessage *message;
	while ( eventQueue.size() > 0 )
	{
		message = eventQueue.read();
		switch (message->getClass())
		{
		case NET_MSG_CLIENT:
			if ( !Client )
			{
				Log.write("Got a message for client, before the client was started!", cLog::eLOG_TYPE_NET_ERROR);
				break;
			}
			Client->HandleNetMessage( message );
			break;
		case NET_MSG_SERVER:
			//should not happen!
			Log.write("Client: got a server message! Type: " + message->getTypeAsString(),cLog::eLOG_TYPE_NET_ERROR);
			break;
		case NET_MSG_MENU:
			if ( !ActiveMenu )
			{
				Log.write("Got a menu message, but no menu active!", cLog::eLOG_TYPE_NET_ERROR);
				break;
			}
			ActiveMenu->handleNetMessage( message );
			break;
		case NET_MSG_STATUS:
			if ( Client )
			{
				Client->HandleNetMessage( message );
			}
			else if ( ActiveMenu )
			{
				ActiveMenu->handleNetMessage( message );
			}
			break;
		default:
			break;
		}

		delete message;
	}
}
