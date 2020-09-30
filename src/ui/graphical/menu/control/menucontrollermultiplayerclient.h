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

#include <memory>

#include "game/startup/lobbyclient.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/runnable.h"

class cApplication;
class cWindowNetworkLobbyClient;
class cWindowLandingPositionSelection;
class cNetworkClientGameNew;
class cPlayerLandingStatus;

class cMenuControllerMultiplayerClient : public cRunnable, public std::enable_shared_from_this<cMenuControllerMultiplayerClient>
{
public:
	cMenuControllerMultiplayerClient (cApplication& application);
	~cMenuControllerMultiplayerClient();

	void start();

	void run() MAXR_OVERRIDE_FUNCTION;
private:
	cSignalConnectionManager signalConnectionManager;

	cLobbyClient lobbyClient;

	cApplication& application;

	std::shared_ptr<cWindowNetworkLobbyClient> windowNetworkLobby;
	std::shared_ptr<cWindowLandingPositionSelection> windowLandingPositionSelection;
	std::shared_ptr<cNetworkClientGameNew> newGame;

	std::vector<std::unique_ptr<cPlayerLandingStatus>> playersLandingStatus;

	void reset();

	void handleSelectSettings (cApplication&);
	void handleSelectMap (cApplication&);

	void startSavedGame (const cSaveGameInfo&, std::shared_ptr<cStaticMap>, std::shared_ptr<cConnectionManager>, cPlayerBasicData);
	void startGamePreparation(const sLobbyPreparationData&, const cPlayerBasicData&, std::shared_ptr<cConnectionManager>);

	void startClanSelection(bool isFirstWindowOnGamePreparation);
	void startLandingUnitSelection(bool isFirstWindowOnGamePreparation);
	void startLandingPositionSelection();
	void startNewGame();
	void reconnectToGame (std::shared_ptr<cStaticMap>, std::shared_ptr<cConnectionManager>, cPlayerBasicData, const std::vector<cPlayerBasicData>&);
	void checkReallyWantsToQuit();

	void saveOptions();
	bool connectionLost;
};

#endif // ui_graphical_menu_control_menucontrollermultiplayerclientH
