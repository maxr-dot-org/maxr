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

#ifndef ui_graphical_menu_control_local_singleplayer_localsingleplayergameH
#define ui_graphical_menu_control_local_singleplayer_localsingleplayergameH

#include <memory>
#include <string>

#include "ui/graphical/menu/control/game.h"
#include "ui/graphical/game/control/gameguicontroller.h"

class cClient;
class cServer;
class cGameGuiController;

class cLocalSingleplayerGame : public cGame
{
public:
	~cLocalSingleplayerGame();

	void run() override;

protected:
	std::shared_ptr<cClient> client;
	std::unique_ptr<cServer> server;

	std::unique_ptr<cGameGuiController> gameGuiController;

};

#endif
