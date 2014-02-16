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

#include "client.h"
#include "clientevents.h"
#include "files.h"
#include "hud.h"
#include "input.h"
#include "log.h"
#include "menuevents.h"
#include "menus.h"
#include "netmessage.h"
#include "network.h"
#include "serverevents.h"
#include "settings.h"
#include "video.h"

#include <ctime>

void cEventHandling::pushEvent (cNetMessage* message)
{
	eventQueue.write (message);
}

static std::string TakeScreenShot()
{
	time_t tTime;
	tm* tmTime;
	char timestr[18];
	tTime = time (NULL);
	tmTime = localtime (&tTime);
	strftime (timestr, sizeof (timestr), "%Y-%m-%d_%H%M%S", tmTime);
	std::string screenshotfile;
	int counter = 0;
	do
	{
		counter += 1;
		screenshotfile = getUserScreenshotsDir() + "screenie_" + timestr + "_" + iToStr (counter) + ".bmp";
	}
	while (FileExists (screenshotfile.c_str()));
	Log.write ("Screenshot saved to " + screenshotfile, cLog::eLOG_TYPE_INFO);
	Video.takeScreenShot (screenshotfile);
	return screenshotfile;
}

static void HandleInputEvent_KEY (cMenu& activeMenu,
								  const SDL_KeyboardEvent& event,
								  cClient* client)
{
	assert (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP);

	// Do not send events to a menu,
	// after an event triggered the termination
	// the user wouldn't expects the menu to execute further events
	// after clicking the exit button
	if (activeMenu.exiting()) return;

	if (event.state == SDL_PRESSED && event.keysym.sym == SDLK_RETURN && event.keysym.mod & KMOD_ALT)   //alt+enter makes us go fullscreen|windowmode
	{
		Video.setWindowMode (!Video.getWindowMode(), true);
	}
	else if (event.state == SDL_PRESSED && event.keysym.sym == SDLK_c && event.keysym.mod & KMOD_ALT)
	{
		// Screenshot
		const std::string screenshotfile = TakeScreenShot();
		if (client != NULL)
			client->getGameGUI().addMessage (lngPack.i18n ("Text~Comp~Screenshot_Done", screenshotfile));
	}
	else
	{
		activeMenu.handleKeyInput (event);
	}
}

static void HandleInputEvent (cMenu& activeMenu, const SDL_Event& event, cClient* client)
{
	switch (event.type)
	{
		case SDL_KEYDOWN:
		case SDL_KEYUP: HandleInputEvent_KEY (activeMenu, event.key, client); break;
		//case SDL_TEXTEDITING: HandleTextEditingEvent (activeMenu, event.edit, client); break;
		case SDL_TEXTINPUT: activeMenu.handleTextInputEvent (event.text); break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP: InputHandler->inputMouseButton (activeMenu, event.button); break;
		case SDL_MOUSEWHEEL: InputHandler->inputMouseButton (activeMenu, event.wheel); break;
		case SDL_QUIT: Quit(); break;
		case SDL_MOUSEMOTION: break;
		case SDL_WINDOWEVENT: Video.draw(); break;
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
				//should not happen
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
			if (client) client->HandleNetMessage (&message, activeMenu);
			else if (activeMenu) activeMenu->handleNetMessage (&message);
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
	// Do not read client messages, until client is started
	if (!client && eventQueue.size() > 0 && eventQueue.peep()->getClass() == NET_MSG_CLIENT)
	{
		Log.write ("Netmessage for Client received, but no client active. Net event handling paused", cLog::eLOG_TYPE_NET_DEBUG);
		return;
	}

	while (eventQueue.size() > 0)
	{
		if (client && client->gameTimer.nextMsgIsNextGameTime) break;
		if (activeMenu && activeMenu->exiting()) break;
		AutoPtr<cNetMessage> message (eventQueue.read());
		HandleNetMessage (client, activeMenu, *message);
	}
}
