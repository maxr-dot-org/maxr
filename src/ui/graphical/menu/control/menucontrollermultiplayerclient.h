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

#ifndef ui_graphical_menu_control_menucontrollermultiplayerclientH
#define ui_graphical_menu_control_menucontrollermultiplayerclientH

#include "game/startup/lobbyclient.h"
#include "ui/graphical/menu/control/initgamepreparation.h"
#include "ui/widgets/runnable.h"
#include "utility/signal/signalconnectionmanager.h"

#include <memory>

class cApplication;
class cWindowNetworkLobbyClient;

class cMenuControllerMultiplayerClient : public cRunnable, public std::enable_shared_from_this<cMenuControllerMultiplayerClient>
{
public:
	cMenuControllerMultiplayerClient (cApplication& application);
	~cMenuControllerMultiplayerClient();

	void start();

	void run() override;

private:
	cSignalConnectionManager signalConnectionManager;

	cApplication& application;

	cLobbyClient lobbyClient;

	std::shared_ptr<cWindowNetworkLobbyClient> windowNetworkLobby;
	std::shared_ptr<cInitGamePreparation> initGamePreparation;

	void reset();

	void handleSelectSettings();
	void handleSelectMap();
	void handleSelectSaveGame();

	void handleStartGame();

	void startSavedGame (std::shared_ptr<cClient>);
	void startGamePreparation();

	void startNewGame (std::shared_ptr<cClient>);
	void reconnectToGame (std::shared_ptr<cClient>);

	void saveOptions();
	bool connectionLost = false;
};

#endif // ui_graphical_menu_control_menucontrollermultiplayerclientH
