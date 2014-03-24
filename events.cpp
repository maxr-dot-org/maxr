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

static void HandleNetMessage (cClient* client, cNetMessage& message)
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
			client->HandleNetMessage (&message);
			break;
		case NET_MSG_SERVER:
			//should not happen!
			Log.write ("Client: got a server message! Type: " + message.getTypeAsString(), cLog::eLOG_TYPE_NET_ERROR);
			break;
		case NET_MSG_MENU:
			//if (!activeMenu)
			//{
			//	Log.write ("Got a menu message, but no menu active!", cLog::eLOG_TYPE_NET_ERROR);
			//	break;
			//}
			//activeMenu->handleNetMessage (&message);
			break;
		case NET_MSG_STATUS:
			if (client) client->HandleNetMessage (&message);
			//else if (activeMenu) activeMenu->handleNetMessage (&message);
			break;
		default:
			break;
	}
}

void cEventHandling::handleNetMessages (cClient* client)
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
		AutoPtr<cNetMessage> message (eventQueue.read());
		HandleNetMessage (client, *message);
	}
}
