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

#ifndef game_startup_lobbyclientH
#define game_startup_lobbyclientH

#include "connectionmanager.h"
#include "game/data/map/map.h"
#include "game/startup/lobbypreparationdata.h"
#include "protocol/lobbymessage.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/thread/concurrentqueue.h"

class cLobbyServer;

class cLobbyClient : public INetMessageReceiver
{
public:
	cLobbyClient (std::shared_ptr<cConnectionManager>, const cPlayerBasicData&);

	void pushMessage (std::unique_ptr<cNetMessage>) override;
	std::unique_ptr<cNetMessage> popMessage() override;
	void run();

	bool isConnectedToServer() const;
	void connectToServer (std::string ip, int port);
	void connectToLocalServer (cLobbyServer&);

	void sendChatMessage (const std::string&);
	void tryToSwitchReadyState();
	void changeLocalPlayerProperties (const std::string& name, cPlayerColor, bool ready);

	void abortGamePreparation();

	void enterLandingSelection();
	void exitLandingSelection();
	void selectLandingPosition (cPosition);

	void wantToRejoinGame();
	void disconnect();

	const std::string& getDownloadingMapName() const { return triedLoadMapName; }

	cSignal<void()> onLocalPlayerConnected;
	cSignal<void (const std::string& version, const std::string& revision)> onDifferentVersion;
	cSignal<void (const std::string& reason)> onConnectionFailed;
	cSignal<void()> onConnectionClosed;

	cSignal<void (const std::string&)> onNoMapNoReady;
	cSignal<void (const std::string& mapName, const std::string& localPath)> onIncompatibleMap;
	cSignal<void (const std::string&)> onMapDownloadRequest;
	cSignal<void (const std::string&)> onMissingOriginalMap;
	cSignal<void (int)> onDownloadMapPercentChanged;
	cSignal<void()> onDownloadMapCancelled;
	cSignal<void (std::shared_ptr<cStaticMap> staticMap)> onDownloadMapFinished;

	cSignal<void (const cPlayerBasicData&, const std::vector<cPlayerBasicData>&)> onPlayersList;
	cSignal<void ()> onDuplicatedPlayerColor;
	cSignal<void ()> onDuplicatedPlayerName;
	cSignal<void (std::shared_ptr<cGameSettings>, std::shared_ptr<cStaticMap>, const cSaveGameInfo&)> onOptionsChanged;

	cSignal<void (const std::string& playerName, bool translate, const std::string& message, const std::string& insertText)> onChatMessage;

	cSignal<void (const sLobbyPreparationData&, cPlayerBasicData, std::shared_ptr<cConnectionManager>)> onStartGamePreparation;
	cSignal<void (const std::string& playerName)> onPlayerAbortGamePreparation;

	cSignal<void (cPlayerBasicData, bool isIn)> onPlayerEnterLeaveLandingSelectionRoom;
	cSignal<void (cPlayerBasicData)> onPlayerSelectLandingPosition;
	cSignal<void (eLandingPositionState)> onLandingDone;

	cSignal<void()> onStartNewGame;
	cSignal<void (const cSaveGameInfo&, std::shared_ptr<cStaticMap>, std::shared_ptr<cConnectionManager>, cPlayerBasicData)> onStartSavedGame;
	cSignal<void (std::shared_ptr<cStaticMap>, std::shared_ptr<cConnectionManager>, cPlayerBasicData, const std::vector<cPlayerBasicData>&)> onReconnectGame;

	cSignal<void (const std::string& mapName)> onFailToReconnectGameNoMap;
	cSignal<void (const std::string& mapName)> onFailToReconnectGameInvalidMap;

private:
	cPlayerBasicData* getPlayer (int playerNr);

	void sendNetMessage(cNetMessage&);
	void sendNetMessage (cNetMessage&&);

	void handleNetMessage (const cNetMessage&);
	void handleLobbyMessage (const cMultiplayerLobbyMessage&);

	void handleNetMessage_TCP_CONNECTED (const cNetMessageTcpConnected&);
	void handleNetMessage_TCP_CONNECT_FAILED (const cNetMessageTcpConnectFailed&);
	void handleNetMessage_TCP_CLOSE (const cNetMessageTcpClose&);

	void handleNetMessage_MU_MSG_CHAT (const cMuMsgChat&);
	void handleNetMessage_MU_MSG_PLAYER_NUMBER (const cMuMsgPlayerNr&);
	void handleNetMessage_MU_MSG_PLAYERLIST (const cMuMsgPlayerList&);
	void handleNetMessage_MU_MSG_OPTIONS (const cMuMsgOptions&);
	void handleNetMessage_MU_MSG_START_GAME_PREPARATIONS (const cMuMsgStartGamePreparations&);

	void handleNetMessage_MU_MSG_LANDING_STATE (const cMuMsgLandingState&);
	void handleNetMessage_MU_MSG_START_GAME (const cMuMsgStartGame&);
	void handleNetMessage_GAME_ALREADY_RUNNING (const cNetMessageGameAlreadyRunning&);
	void handleNetMessage_MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS (const cMuMsgInLandingPositionSelectionStatus&);
	void handleNetMessage_MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION (const cMuMsgPlayerHasSelectedLandingPosition&);
	void handleNetMessage_MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION (const cMuMsgPlayerAbortedGamePreparations&);

private:
	cSignalConnectionManager signalConnectionManager;

	cConcurrentQueue<std::unique_ptr<cNetMessage>> messageQueue;
	std::shared_ptr<cConnectionManager> connectionManager;

	std::vector<std::unique_ptr<ILobbyMessageHandler>> lobbyMessageHandlers;

	cPlayerBasicData localPlayer;
	std::vector<cPlayerBasicData> players;
	std::shared_ptr<cStaticMap> staticMap;
	std::shared_ptr<cGameSettings> gameSettings;
	cSaveGameInfo saveGameInfo{-1};

	std::string triedLoadMapName;
	std::string lastRequestedMapName;
};

#endif
