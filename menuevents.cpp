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

#include "menuevents.h"

#include "log.h"
#include "mapdownload.h"
#include "netmessage.h"
#include "player.h"
#include "serverevents.h"
#include "map.h"

using namespace std;

void sendMenuChatMessage (cTCP& network, const string& chatMsg, const sPlayer* player, int fromPlayerNr, bool translationText)
{
	cNetMessage* message = new cNetMessage (MU_MSG_CHAT);
	message->pushString (chatMsg);
	message->pushBool (translationText);
	//cMenu::sendMessage (network, message, player, fromPlayerNr);
}

void sendRequestIdentification (cTCP& network, const sPlayer& player)
{
	cNetMessage* message = new cNetMessage (MU_MSG_REQ_IDENTIFIKATION);
	message->pushInt16 (player.getNr());
	message->pushString (string (PACKAGE_VERSION) + " " + PACKAGE_REV);
	//cMenu::sendMessage (network, message, &player);
}

void sendPlayerList (cTCP& network, const std::vector<sPlayer*>& players)
{
	cNetMessage* message = new cNetMessage (MU_MSG_PLAYERLIST);

	for (int i = (int) players.size() - 1; i >= 0; i--)
	{
		const sPlayer& player = *players[i];
		message->pushInt16 (player.getNr());
		message->pushBool (player.isReady());
		message->pushInt16 (player.getColorIndex());
		message->pushString (player.getName());
	}
	message->pushInt16 ((int) players.size());
	//cMenu::sendMessage (network, message);
}

void sendGameData (cTCP& network, const cStaticMap* map, const sSettings* settings, const string& saveGameString, const sPlayer* player)
{
	cNetMessage* message = new cNetMessage (MU_MSG_OPTINS);

	message->pushString (saveGameString);

	if (map)
	{
		const std::string mapName = map->getName();
		message->pushInt32 (MapDownload::calculateCheckSum (mapName));
		message->pushString (mapName);
	}
	message->pushBool (map != NULL);

	if (settings)
	{
		settings->pushInto (*message);
	}
	message->pushBool (settings != NULL);

	//cMenu::sendMessage (network, message, player);
}

void sendIdentification (cTCP& network, const sPlayer& player)
{
	cNetMessage* message = new cNetMessage (MU_MSG_IDENTIFIKATION);
	message->pushString (string (PACKAGE_VERSION) + " " + PACKAGE_REV);
	message->pushBool (player.isReady());
	message->pushString (player.getName());
	message->pushInt16 (player.getColorIndex());
	message->pushInt16 (player.getNr());
	//cMenu::sendMessage (network, message);
}

void sendGameIdentification (cTCP& network, const sPlayer& player, int socket)
{
	cNetMessage* message = new cNetMessage (GAME_EV_IDENTIFICATION);
	message->pushInt16 (socket);
	message->pushString (player.getName());
	//cMenu::sendMessage (network, message);
}

void sendRequestMap (cTCP& network, const string& mapName, int playerNr)
{
	cNetMessage* msg = new cNetMessage (MU_MSG_REQUEST_MAP);
	msg->pushString (mapName);
	msg->pushInt16 (playerNr);
	//cMenu::sendMessage (network, msg);
}

