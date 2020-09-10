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

#ifndef game_startup_network_host_networkhostgameH
#define game_startup_network_host_networkhostgameH

#include <memory>
#include <string>

#include "game/startup/game.h"
#include "maxrconfig.h"
#include "ui/graphical/game/control/gameguicontroller.h"

class cClient;
class cServer;
class cConnectionManager;

class cNetworkHostGame : public cGame
{
public:
	~cNetworkHostGame();

	void run() MAXR_OVERRIDE_FUNCTION;

	void setConnectionManager (std::shared_ptr<cConnectionManager> network);
protected:
	std::shared_ptr<cConnectionManager> connectionManager;

	std::shared_ptr<cClient> localClient;
	std::unique_ptr<cServer> server;

	std::unique_ptr<cGameGuiController> gameGuiController;
};

#endif // game_startup_network_host_networkhostgameH
