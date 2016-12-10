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
#include <map>

#include "utility/signal/signalconnectionmanager.h"
#include "connectionmanager.h"
#include "utility/runnable.h"
#include "utility/thread/concurrentqueue.h"
#include "game/logic/landingpositionmanager.h"
#include "menuevents.h"

class cApplication;
class cWindowNetworkLobbyHost;
class cWindowLandingPositionSelection;
class cNetworkHostGameNew;
class cPlayerBasicData;
class cMapSender;
class cPlayerLandingStatus;
class cActionInitNewGame;


class cMenuControllerMultiplayerHost : public INetMessageReceiver, public cRunnable, public std::enable_shared_from_this<cMenuControllerMultiplayerHost>
{
public:
	cMenuControllerMultiplayerHost (cApplication& application);
	~cMenuControllerMultiplayerHost();

	void start();

	virtual void pushMessage (std::unique_ptr<cNetMessage2> message) MAXR_OVERRIDE_FUNCTION;
	virtual std::unique_ptr<cNetMessage2> popMessage() MAXR_OVERRIDE_FUNCTION;

	virtual void run() MAXR_OVERRIDE_FUNCTION;
private:
	cSignalConnectionManager signalConnectionManager;

	cConcurrentQueue<std::unique_ptr<cNetMessage2>> messageQueue;

	std::shared_ptr<cConnectionManager> connectionManager;

	cApplication& application;

	std::shared_ptr<cWindowNetworkLobbyHost> windowNetworkLobby;

	std::shared_ptr<cWindowLandingPositionSelection> windowLandingPositionSelection;

	std::shared_ptr<cLandingPositionManager> landingPositionManager;

	std::shared_ptr<cNetworkHostGameNew> newGame;

	int nextPlayerNumber;

	std::string triedLoadMapName;

	std::vector<std::unique_ptr<cMapSender>> mapSenders;

	std::vector<std::unique_ptr<cPlayerLandingStatus>> playersLandingStatus;

	void reset();

	void sendGameData(int playerNr = -1);
	
	void handleSelectMap (cApplication& application);
	void handleSelectSettings (cApplication& application);
	void handleSelectSaveGame (cApplication& application);

	void handleWantLocalPlayerReadyChange();
	void handleChatMessageTriggered();

	void handleLocalPlayerAttributesChanged();

	void startHost();

	void checkTakenPlayerAttributes (cPlayerBasicData& player);

	void checkGameStart();

	void startSavedGame();

	void startGamePreparation();

	void startClanSelection(bool isFirstWindowOnGamePreparation);
	void startLandingUnitSelection(bool isFirstWindowOnGamePreparation);
	void startLandingPositionSelection();
	void startNewGame();
	void checkReallyWantsToQuit();

	void handleNetMessage (cNetMessage2& message);

	void handleNetMessage_TCP_WANT_CONNECT(cNetMessageTcpWantConnect& message);
	void handleNetMessage_TCP_CLOSE(cNetMessageTcpClose& message);

	void handleNetMessage_MU_MSG_CHAT (cMuMsgChat& message);
	void handleNetMessage_MU_MSG_IDENTIFIKATION(cMuMsgIdentification& message);
	void handleNetMessage_MU_MSG_REQUEST_MAP(cMuMsgRequestMap& message);
	void handleNetMessage_MU_MSG_FINISHED_MAP_DOWNLOAD(cMuMsgFinishedMapDownload& message);
	void handleNetMessage_MU_MSG_LANDING_POSITION(cMuMsgLandingPosition& message);
	void handleNetMessage_MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS(cMuMsgInLandingPositionSelectionStatus& message);
	void handleNetMessage_MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION(cMuMsgPlayerAbortedGamePreparations& message);

	void sendNetMessage(cNetMessage2& message, int receiverPlayerNr = -1, int senderPlayerNr = -1);

	void saveOptions();
};

#endif // ui_graphical_menu_control_menucontrollermultiplayerhostH
