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

#include "game/networkaddress.h"
#include "game/startup/lobbyclient.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/widgets/lineedit.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cWindowNetworkLobbyClient::cWindowNetworkLobbyClient() :
	cWindowNetworkLobby (lngPack.i18n ("Others~TCPIP_Client"), false)
{
	setIsHost (false);

	connectButton = emplaceChild<cPushButton> (getPosition() + cPosition (470, 200), ePushButtonType::StandardSmall, lngPack.i18n ("Title~Connect"));
	signalConnectionManager.connect (connectButton->clicked, [this]() { triggeredConnect(); });

	signalConnectionManager.connect (ipLineEdit->returnPressed, [this]() {
		triggeredConnect();
	});
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyClient::retranslate()
{
	cWindowNetworkLobby::retranslate();
	setTitle (lngPack.i18n ("Others~TCPIP_Client"));

	connectButton->setText (lngPack.i18n ("Title~Connect"));
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyClient::bindConnections (cLobbyClient& lobbyClient)
{
	cWindowNetworkLobby::bindConnections (lobbyClient);

	signalConnectionManager.connect (lobbyClient.onPlayersList, [this] (const cPlayerBasicData& localPlayer, const std::vector<cPlayerBasicData>& players) {
		setIsHost (!players.empty() && localPlayer.getNr() == players[0].getNr());
	});

	signalConnectionManager.connect (triggeredConnect, [&lobbyClient, this]() {
		// Connect only if there isn't a connection yet
		if (lobbyClient.isConnectedToServer()) return;

		sNetworkAddress address{getIp(), getPort()};

		addInfoEntry (lngPack.i18n ("Multiplayer~Network_Connecting", address.toString())); // e.g. Connecting to 127.0.0.1:55800
		disablePortEdit();
		disableIpEdit();

		lobbyClient.connectToServer (address);
	});
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyClient::setIsHost (bool isHost)
{
	if (isHost)
	{
		loadButton->show();
		mapButton->show();
		okButton->show();
		settingsButton->show();
	}
	else
	{
		loadButton->hide();
		mapButton->hide();
		okButton->hide();
		settingsButton->hide();
	}
}
