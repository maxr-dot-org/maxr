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
#include "clientevents.h"
#include "menuevents.h"
#include "netmessage.h"
#include "client.h"
#include "input.h"
#include "log.h"
#include "menus.h"
#include "settings.h"
#include "files.h"
#include "video.h"

void cEventHandling::pushEvent (cNetMessage* message)
{
	eventQueue.write (message);
}

static std::string TakeScreenShot()
{
	time_t tTime;
	tm* tmTime;
	char timestr[16];
	tTime = time (NULL);
	tmTime = localtime (&tTime);
	strftime (timestr, 16, "%d.%m.%y-%H%M%S", tmTime);

	std::string screenshotfile = getUserScreenshotsDir();
	screenshotfile.append ( (std::string) "Screen_" + timestr + ".bmp");
	Log.write ("Screenshot saved to " + screenshotfile, cLog::eLOG_TYPE_INFO);
	SDL_SaveBMP (screen, screenshotfile.c_str());
	return screenshotfile;
}

static void HandleInputEvent_SDL_ACTIVEEVENT (SDL_Event& event)
{
	assert (event.type == SDL_ACTIVEEVENT);
#if 1
	// This is an workaround for a SDL bug.
	// See Ticket #86 on bugtracker.
	// will hopefully not be required anymore in SDL 1.3
	if (event.active.state & SDL_APPINPUTFOCUS) SDL_SetModState (KMOD_NONE);
#endif
}

static void HandleInputEvent_KEY (cMenu& activeMenu, SDL_Event& event, cClient* client)
{
	assert (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP);

	if (event.key.state == SDL_PRESSED && event.key.keysym.sym == SDLK_RETURN && event.key.keysym.mod & KMOD_ALT)   //alt+enter makes us go fullscreen|windowmode
	{
		Video.setWindowMode (!Video.getWindowMode(), true);
	}
	else if (event.key.state == SDL_PRESSED && event.key.keysym.sym == SDLK_c && event.key.keysym.mod & KMOD_ALT)
	{
		// Screenshot
		const std::string screenshotfile = TakeScreenShot();
		if (client != NULL)
			client->gameGUI.addMessage (lngPack.i18n ("Text~Comp~Screenshot_Done", screenshotfile));
	}
	else
	{
		InputHandler->inputkey (activeMenu, event.key);
	}
}

static void HandleInputEvent (cMenu& activeMenu, SDL_Event& event, cClient* client)
{
	switch (event.type)
	{
		case SDL_ACTIVEEVENT: HandleInputEvent_SDL_ACTIVEEVENT (event); break;
		case SDL_KEYDOWN:
		case SDL_KEYUP: HandleInputEvent_KEY (activeMenu, event, client); break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP: InputHandler->inputMouseButton (activeMenu, event.button); break;
		case SDL_QUIT: Quit(); break;
		default: break;
	}
}

static void HandleNetMessage (cClient* client, cMenu* activeMenu, cNetMessage& message)
{
	switch (message.getClass())
	{
		case NET_MSG_CLIENT:
			if (!client)
			{
				Log.write ("Got a message for client, before the client was started!", cLog::eLOG_TYPE_NET_ERROR);
				break;
			}
			client->HandleNetMessage (&message, activeMenu);
			break;
		case NET_MSG_SERVER:
			//should not happen!
			Log.write ("Client: got a server message! Type: " + message.getTypeAsString(), cLog::eLOG_TYPE_NET_ERROR);
			break;
		case NET_MSG_MENU:
			if (!activeMenu)
			{
				Log.write ("Got a menu message, but no menu active!", cLog::eLOG_TYPE_NET_ERROR);
				break;
			}
			activeMenu->handleNetMessage (&message);
			break;
		case NET_MSG_STATUS:
			if (client)
			{
				client->HandleNetMessage (&message, activeMenu);
			}
			else if (activeMenu)
			{
				activeMenu->handleNetMessage (&message);
			}
			break;
		default:
			break;
	}
}

void cEventHandling::HandleEvents (cMenu& activeMenu, cClient* client)
{
	if (!client) handleNetMessages (client, &activeMenu);
	cEventHandling::handleInputEvents (activeMenu, client);
}

/*static*/ void cEventHandling::handleInputEvents (cMenu& activeMenu, cClient* client)
{
	SDL_Event event;
	while (SDL_PollEvent (&event))
		HandleInputEvent (activeMenu, event, client);
}

void cEventHandling::handleNetMessages (cClient* client, cMenu* activeMenu)
{
	while (eventQueue.size() > 0 && (!client || !client->gameTimer.nextMsgIsNextGameTime))
	{
		cNetMessage* message = eventQueue.read();
		HandleNetMessage (client, activeMenu, *message);
		delete message;
	}
}
