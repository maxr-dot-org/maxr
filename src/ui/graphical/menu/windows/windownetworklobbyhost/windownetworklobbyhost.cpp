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

#include "ui/graphical/menu/windows/windownetworklobbyhost/windownetworklobbyhost.h"

#include "ui/graphical/menu/widgets/pushbutton.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cWindowNetworkLobbyHost::cWindowNetworkLobbyHost() :
	cWindowNetworkLobby (lngPack.i18n ("Others~TCPIP_Host"), true)
{
	startButton = emplaceChild<cPushButton> (getPosition() + cPosition (470, 200), ePushButtonType::StandardSmall, lngPack.i18n ("Others~Host_Start"));
	signalConnectionManager.connect (startButton->clicked, [this]() { triggeredStartHost(); });
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyHost::retranslate()
{
	cWindowNetworkLobby::retranslate();

	setTitle (lngPack.i18n ("Others~TCPIP_Host"));
	startButton->setText (lngPack.i18n ("Others~Host_Start"));
}
