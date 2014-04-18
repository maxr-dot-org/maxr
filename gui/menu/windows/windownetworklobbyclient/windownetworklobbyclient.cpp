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

#include "windownetworklobbyclient.h"
#include "../windowgamesettings/gamesettings.h"
#include "../../widgets/pushbutton.h"
#include "../../../../main.h"
#include "../../../../network.h"
#include "../../../../log.h"
#include "../../../../player.h"
#include "../../../../netmessage.h"
#include "../../../../menuevents.h"

//------------------------------------------------------------------------------
cWindowNetworkLobbyClient::cWindowNetworkLobbyClient () :
	cWindowNetworkLobby (lngPack.i18n ("Text~Others~TCPIP_Client"), false)
{
	auto connectButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (470, 200), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Title~Connect")));
	signalConnectionManager.connect (connectButton->clicked, std::bind (&cWindowNetworkLobbyClient::handleConnectClicked, this));
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyClient::handleConnectClicked ()
{
	triggeredConnect ();
}
/*
//------------------------------------------------------------------------------
bool cWindowNetworkLobbyClient::handleNetMessage (cNetMessage& message)
{
	Log.write ("Menu: <-- " + message.getTypeAsString () + ", Hexdump: " + message.getHexDump (), cLog::eLOG_TYPE_NET_DEBUG);

	switch (message.iType)
	{
	case MU_MSG_CHAT: handleNetMessage_MU_MSG_CHAT (message); return true;
	//case TCP_CLOSE: handleNetMessage_TCP_CLOSE (message); return true;
	case MU_MSG_REQ_IDENTIFIKATION: handleNetMessage_MU_MSG_REQ_IDENTIFIKATION (message); return true;
	//case MU_MSG_PLAYERLIST: handleNetMessage_MU_MSG_PLAYERLIST (message); return true;
	//case MU_MSG_OPTINS: handleNetMessage_MU_MSG_OPTINS (message); return true;
	//case MU_MSG_START_MAP_DOWNLOAD: initMapDownload (message); return true;
	//case MU_MSG_MAP_DOWNLOAD_DATA: receiveMapData (message); return true;
	//case MU_MSG_CANCELED_MAP_DOWNLOAD: canceledMapDownload (message); return true;
	//case MU_MSG_FINISHED_MAP_DOWNLOAD: finishedMapDownload (message); return true;
	//case MU_MSG_GO: handleNetMessage_MU_MSG_GO (message); return true;
	//case GAME_EV_REQ_RECON_IDENT: handleNetMessage_GAME_EV_REQ_RECON_IDENT (message); return true;
	//case GAME_EV_RECONNECT_ANSWER: handleNetMessage_GAME_EV_RECONNECT_ANSWER (message); return true;
	default: break;
	}

	return false;
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyClient::handleNetMessage_MU_MSG_CHAT (cNetMessage& message)
{
	assert (message.iType == MU_MSG_CHAT);

	auto players = getPlayers ();
	auto iter = std::find_if (players.begin (), players.end (), [=](const std::shared_ptr<sPlayer>& player){ return player->getNr () == message.iPlayerNr; });
	if (iter == players.end ()) return;

	const auto& player = **iter;

	bool translationText = message.popBool ();
	auto chatText = message.popString ();
	if (translationText) addInfoEntry (lngPack.i18n (chatText));
	else
	{
		addChatEntry (player.getName (), chatText);
		PlayFX (SoundData.SNDChat.get ());
	}
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyClient::handleNetMessage_MU_MSG_REQ_IDENTIFIKATION (cNetMessage& message)
{
	assert (message.iType == MU_MSG_REQ_IDENTIFIKATION);

	Log.write ("game version of server is: " + message.popString (), cLog::eLOG_TYPE_NET_DEBUG);
	getLocalPlayer()->setNr (message.popInt16 ());
	sendIdentification (getNetwork(), *getLocalPlayer ());
}
*/