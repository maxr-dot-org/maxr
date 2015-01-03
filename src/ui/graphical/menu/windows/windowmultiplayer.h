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

#ifndef ui_graphical_menu_windows_windowmultiplayerH
#define ui_graphical_menu_windows_windowmultiplayerH

#include "ui/graphical/menu/windows/windowmain.h"
#include "utility/signal/signalconnectionmanager.h"

class cMenuControllerMultiplayerHost;
class cMenuControllerMultiplayerClient;
class cMenuControllerMultiplayerHotSeat;

class cWindowMultiPlayer : public cWindowMain
{
public:
	cWindowMultiPlayer();
	~cWindowMultiPlayer();

private:
	cSignalConnectionManager signalConnectionManager;

	void tcpHostClicked();
	void tcpClientClicked();
	void newHotSeatClicked();
	void loadHotSeatClicked();
	void backClicked();

	std::shared_ptr<cMenuControllerMultiplayerHost> multiplayerHostController;
	std::shared_ptr<cMenuControllerMultiplayerClient> multiplayerClientController;
	std::shared_ptr<cMenuControllerMultiplayerHotSeat> multiplayerHotSeatController;
};

#endif // ui_graphical_menu_windows_windowmultiplayerH
