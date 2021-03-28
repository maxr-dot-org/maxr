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

#include <cassert>
#include <memory>

#include "ui/graphical/game/control/gameguicontroller.h"
#include "ui/graphical/menu/control/game.h"

class cClient;
class cServer;
class cConnectionManager;

class cNetworkGame : public cGame
{
public:
	~cNetworkGame();

	void run() override;

	void setConnectionManager (std::shared_ptr<cConnectionManager>);

	cClient& getLocalClient() { assert (localClient); return *localClient; }
protected:
	std::shared_ptr<cConnectionManager> connectionManager;

	std::shared_ptr<cClient> localClient;
	cServer* server = nullptr;

	std::unique_ptr<cGameGuiController> gameGuiController;
};

#endif
