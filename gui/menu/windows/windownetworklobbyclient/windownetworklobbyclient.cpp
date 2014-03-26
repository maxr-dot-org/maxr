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
	// Connect only if there isn't a connection yet
	if (getNetwork ().getConnectionStatus () != 0) return;

	addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Connecting") + getIp() + ":" + iToStr (getPort()));    // e.g. Connecting to 127.0.0.1:55800
	Log.write (("Connecting to " + getIp () + ":" + iToStr (getPort ())), cLog::eLOG_TYPE_INFO);

	// FIXME: make this non blocking!
	if (getNetwork ().connect (getIp (), getPort ()) == -1)
	{
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Error_Connect") + getIp () + ":" + iToStr (getPort ()));
		Log.write ("Error on connecting " + getIp () + ":" + iToStr (getPort ()), cLog::eLOG_TYPE_WARNING);
	}
	else
	{
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Connected"));
		Log.write ("Connected", cLog::eLOG_TYPE_INFO);
		disablePortEdit ();
		disableIpEdit ();
	}
}