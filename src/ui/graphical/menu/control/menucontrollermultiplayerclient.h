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

#include "utility/signal/signalconnectionmanager.h"
#include "utility/runnable.h"
#include "utility/thread/concurrentqueue.h"
#include "connectionmanager.h"
#include "menuevents.h"

class cApplication;
class cWindowNetworkLobbyClient;
class cWindowLandingPositionSelection;
class cNetworkClientGameNew;
class cMapReceiver;
class cPlayerLandingStatus;


class cMenuControllerMultiplayerClient : public INetMessageReceiver, public cRunnable, public std::enable_shared_from_this<cMenuControllerMultiplayerClient>
{
public:
	cMenuControllerMultiplayerClient (cApplication& application);
	~cMenuControllerMultiplayerClient();

	void start();

	virtual void pushMessage (std::unique_ptr<cNetMessage2> message) MAXR_OVERRIDE_FUNCTION;
	std::unique_ptr<cNetMessage2> popMessage() MAXR_OVERRIDE_FUNCTION;

	virtual void run() MAXR_OVERRIDE_FUNCTION;
private:
	cSignalConnectionManager signalConnectionManager;

	cConcurrentQueue<std::unique_ptr<cNetMessage2>> messageQueue;

	std::shared_ptr<cConnectionManager> connectionManager;

	cApplication& application;

	std::shared_ptr<cWindowNetworkLobbyClient> windowNetworkLobby;

	std::shared_ptr<cWindowLandingPositionSelection> windowLandingPositionSelection;

	std::shared_ptr<cNetworkClientGameNew> newGame;

	std::unique_ptr<cMapReceiver> mapReceiver;

	std::vector<std::unique_ptr<cPlayerLandingStatus>> playersLandingStatus;

	std::string triedLoadMapName;
	std::string lastRequestedMapName;

	void reset();

	void handleWantLocalPlayerReadyChange();
	void handleChatMessageTriggered();
	void handleLocalPlayerAttributesChanged();

	void connect();

	void startSavedGame();

	void startGamePreparation(cMuMsgStartGamePreparations& message);

	void startClanSelection(bool isFirstWindowOnGamePreparation);
	void startLandingUnitSelection(bool isFirstWindowOnGamePreparation);
	void startLandingPositionSelection();
	void startNewGame();
	void reconnectToGame(const cNetMessageGameAlreadyRunning& message);
	void checkReallyWantsToQuit();

	void handleNetMessage (cNetMessage2& message);

	void handleNetMessage_TCP_CONNECTED(cNetMessageTcpConnected& message);
	void handleNetMessage_TCP_CONNECT_FAILED(cNetMessageTcpConnectFailed& message);
	void handleNetMessage_TCP_CLOSE(cNetMessageTcpClose& message);

	void handleNetMessage_MU_MSG_CHAT(cMuMsgChat& message);
	void handleNetMessage_MU_MSG_PLAYER_NUMBER(cMuMsgPlayerNr& message);
	void handleNetMessage_MU_MSG_PLAYERLIST(cMuMsgPlayerList& message);
	void handleNetMessage_MU_MSG_OPTIONS(cMuMsgOptions& message);
	void handleNetMessage_MU_MSG_START_GAME_PREPARATIONS(cMuMsgStartGamePreparations& message);
	void handleNetMessage_MU_MSG_LANDING_STATE(cMuMsgLandingState& message);
	void handleNetMessage_MU_MSG_START_GAME(cMuMsgStartGame& message);
	void handleNetMessage_GAME_ALREADY_RUNNING(cNetMessageGameAlreadyRunning& message);
	void handleNetMessage_MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS(cMuMsgInLandingPositionSelectionStatus& message);
	void handleNetMessage_MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION(cMuMsgPlayerHasSelectedLandingPosition& message);
	void handleNetMessage_MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION(cMuMsgPlayerAbortedGamePreparations& message);

	void initMapDownload(cMuMsgStartMapDownload& message);
	void receiveMapData(cMuMsgMapDownloadData& message);
	void canceledMapDownload(cMuMsgCanceledMapDownload& message);
	void finishedMapDownload(cMuMsgFinishedMapDownload& message);

	void sendNetMessage(cNetMessage2& message);
	void sendNetMessage(cNetMessage2&& message);
	void saveOptions();
	bool connectionLost;
};

#endif // ui_graphical_menu_control_menucontrollermultiplayerclientH
