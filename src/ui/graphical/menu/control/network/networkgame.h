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

#ifndef ui_graphical_menu_control_network_networkgameH
#define ui_graphical_menu_control_network_networkgameH

#include <memory>

#include "ui/graphical/game/control/gameguicontroller.h"
#include "ui/graphical/menu/control/game.h"
#include "utility/signal/signalconnectionmanager.h"

class cApplication;
class cClient;
class cConnectionManager;
class cServer;
struct sInitPlayerData;

class cNetworkGame : public cGame
{
public:
	~cNetworkGame();

	void startNewGame (cApplication&, std::shared_ptr<cClient>, const sInitPlayerData&, cServer*);
	void start (cApplication&, std::shared_ptr<cClient>, cServer*);

#if 1 // To remove
	void run() override;
#endif

private:
	cSignalConnectionManager signalConnectionManager;

	std::unique_ptr<cGameGuiController> gameGuiController;
};

#endif
