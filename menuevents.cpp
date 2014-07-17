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
#include "game/data/player/player.h"
#include "serverevents.h"
#include "map.h"
#include "utility/tounderlyingtype.h"

using namespace std;

namespace {

//------------------------------------------------------------------------------
void sendMessage (cTCP& network, cNetMessage* message, const sPlayer* player = nullptr, int fromPlayerNr = -1)
{
	// Attention: The playernumber will only be the real player number
	// when it is passed to this function explicitly.
	// Otherwise it is only -1!
	message->iPlayerNr = fromPlayerNr;

	if (player == nullptr) network.send (message->iLength, message->serialize ());
	else network.sendTo (player->getSocketIndex (), message->iLength, message->serialize ());

	Log.write ("Menu: --> " + message->getTypeAsString () + ", Hexdump: " + message->getHexDump (), cLog::eLOG_TYPE_NET_DEBUG);
	delete message;
}

}

void sendMenuChatMessage (cTCP& network, const string& chatMsg, const sPlayer* player, int fromPlayerNr, bool translationText)
{
	cNetMessage* message = new cNetMessage (MU_MSG_CHAT);
	message->pushString (chatMsg);
	message->pushBool (translationText);
	sendMessage (network, message, player, fromPlayerNr);
}

void sendRequestIdentification (cTCP& network, const sPlayer& player)
{
	cNetMessage* message = new cNetMessage (MU_MSG_REQ_IDENTIFIKATION);
	message->pushInt16 (player.getNr());
	message->pushString (string (PACKAGE_VERSION) + " " + PACKAGE_REV);
	sendMessage (network, message, &player);
}

void sendPlayerNumber (cTCP& network, const sPlayer& player)
{
	cNetMessage* message = new cNetMessage (MU_MSG_PLAYER_NUMBER);
	message->pushInt16 (player.getNr ());
	sendMessage (network, message, &player);
}

void sendPlayerList (cTCP& network, const std::vector<std::shared_ptr<sPlayer>>& players)
{
	cNetMessage* message = new cNetMessage (MU_MSG_PLAYERLIST);

	for (int i = (int) players.size() - 1; i >= 0; i--)
	{
		const sPlayer& player = *players[i];
		message->pushInt16 (player.getNr());
		message->pushBool (player.isReady());
		message->pushInt16 (player.getColor().getIndex());
		message->pushString (player.getName());
	}
	message->pushInt16 ((int) players.size());
	sendMessage (network, message);
}

void sendGameData (cTCP& network, const cStaticMap* map, const cGameSettings* settings, int saveGameNumber, const sPlayer* player)
{
	cNetMessage* message = new cNetMessage (MU_MSG_OPTINS);

	message->pushInt32 (saveGameNumber);

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

	sendMessage (network, message, player);
}

void sendIdentification (cTCP& network, const sPlayer& player)
{
	cNetMessage* message = new cNetMessage (MU_MSG_IDENTIFIKATION);
	message->pushString (string (PACKAGE_VERSION) + " " + PACKAGE_REV);
	message->pushBool (player.isReady());
	message->pushString (player.getName());
	message->pushInt16 (player.getColor().getIndex());
	message->pushInt16 (player.getNr());
	sendMessage (network, message);
}

void sendGameIdentification (cTCP& network, const sPlayer& player, int socket)
{
	cNetMessage* message = new cNetMessage (GAME_EV_IDENTIFICATION);
	message->pushInt16 (socket);
	message->pushString (player.getName());
	sendMessage (network, message);
}

void sendRequestMap (cTCP& network, const string& mapName, int playerNr)
{
	cNetMessage* msg = new cNetMessage (MU_MSG_REQUEST_MAP);
	msg->pushString (mapName);
	msg->pushInt16 (playerNr);
	sendMessage (network, msg);
}

//------------------------------------------------------------------------------
void sendGo (cTCP& network)
{
	cNetMessage* msg = new cNetMessage (MU_MSG_GO);
	sendMessage (network, msg);
}

//------------------------------------------------------------------------------
void sendLandingState (cTCP& network, eLandingPositionState state, const sPlayer& player)
{
	cNetMessage* msg = new cNetMessage (MU_MSG_LANDING_STATE);
	msg->pushInt32 (toUnderlyingType (state));

	sendMessage (network, msg, &player);
}

//------------------------------------------------------------------------------
void sendAllLanded (cTCP& network)
{
	cNetMessage* msg = new cNetMessage (MU_MSG_ALL_LANDED);
	sendMessage (network, msg);
}

//------------------------------------------------------------------------------
void sendLandingPosition (cTCP& network, const cPosition& position, const sPlayer& player)
{
	cNetMessage* msg = new cNetMessage (MU_MSG_LANDING_POSITION);
	msg->pushPosition (position);
	msg->pushInt32 (player.getNr ());

	sendMessage (network, msg);
}