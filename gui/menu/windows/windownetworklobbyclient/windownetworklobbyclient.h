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

#ifndef gui_menu_windows_windownetworklobbyclient_windownetworklobbyclientH
#define gui_menu_windows_windownetworklobbyclient_windownetworklobbyclientH

#include "../windownetworklobby/windownetworklobby.h"
#include "../../../../utility/signal/signalconnectionmanager.h"

class cWindowNetworkLobbyClient : public cWindowNetworkLobby
{
public:
	cWindowNetworkLobbyClient ();

private:
	cSignalConnectionManager signalConnectionManager;

	void handleConnectClicked ();
};

#endif // gui_menu_windows_windownetworklobbyclient_windownetworklobbyclientH