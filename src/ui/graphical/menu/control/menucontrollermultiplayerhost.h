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

#ifndef ui_graphical_menu_control_menucontrollermultiplayerhostH
#define ui_graphical_menu_control_menucontrollermultiplayerhostH

#include <memory>

#include "utility/signal/signalconnectionmanager.h"
#include "network.h"
#include "utility/runnable.h"
#include "utility/concurrentqueue.h"
#include "game/logic/landingpositionmanager.h"

class cApplication;
class cWindowNetworkLobbyHost;
class cNetworkHostGameNew;
class cNetMessage;
class cPlayerBasicData;
class cMapSender;

class cTCP;

class cMenuControllerMultiplayerHost : public INetMessageReceiver, public cRunnable, public std::enable_shared_from_this<cMenuControllerMultiplayerHost>
{
public:
	cMenuControllerMultiplayerHost (cApplication& application);
	~cMenuControllerMultiplayerHost ();

	void start ();

	virtual void pushEvent (std::unique_ptr<cNetMessage> message) MAXR_OVERRIDE_FUNCTION;

	virtual void run () MAXR_OVERRIDE_FUNCTION;
private:
	cSignalConnectionManager signalConnectionManager;

	cConcurrentQueue<std::unique_ptr<cNetMessage>> messageQueue;

	std::shared_ptr<cTCP> network;

	cApplication& application;

	std::shared_ptr<cWindowNetworkLobbyHost> windowNetworkLobby;

	std::shared_ptr<cLandingPositionManager> landingPositionManager;

	std::shared_ptr<cNetworkHostGameNew> newGame;

	int nextPlayerNumber;

	std::string triedLoadMapName;

	std::vector<std::unique_ptr<cMapSender>> mapSenders;

	void reset ();

	void handleSelectMap (cApplication& application);
	void handleSelectSettings (cApplication& application);
	void handleSelectSaveGame (cApplication& application);

	void handleWantLocalPlayerReadyChange ();
	void handleChatMessageTriggered ();

	void handleLocalPlayerAttributesChanged ();

	void startHost ();

	void checkTakenPlayerAttributes (cPlayerBasicData& player);

	void checkGameStart ();

	void startSavedGame ();

	void startGamePreparation ();

	void startClanSelection ();
	void startLandingUnitSelection ();
	void startLandingPositionSelection ();
	void startNewGame ();

	void handleNetMessage (cNetMessage& message);

	void handleNetMessage_MU_MSG_CHAT (cNetMessage& message);
	void handleNetMessage_TCP_ACCEPT (cNetMessage& message);
	void handleNetMessage_TCP_CLOSE (cNetMessage& message);
	void handleNetMessage_MU_MSG_IDENTIFIKATION (cNetMessage& message);
	void handleNetMessage_MU_MSG_REQUEST_MAP (cNetMessage& message);
	void handleNetMessage_MU_MSG_FINISHED_MAP_DOWNLOAD (cNetMessage& message);
	void handleNetMessage_MU_MSG_LANDING_POSITION (cNetMessage& message);

	void saveOptions ();
};

#endif // ui_graphical_menu_control_menucontrollermultiplayerhostH
