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

#ifndef game_startup_lobbyserverH
#define game_startup_lobbyserverH

#include "game/connectionmanager.h"
#include "game/data/map/map.h"
#include "game/logic/landingpositionmanager.h"
#include "game/logic/server.h"
#include "game/protocol/lobbymessage.h"
#include "game/protocol/netmessage.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/thread/concurrentqueue.h"

#include <memory>
#include <set>
#include <string>
#include <vector>

class cLobbyClient;

enum class eOpenServerResult
{
	AlreadyOpened,
	Success,
	Failed
};

class cLobbyServer : public INetMessageReceiver
{
public:
	explicit cLobbyServer (std::shared_ptr<cConnectionManager>);

	void addLobbyMessageHandler (std::unique_ptr<ILobbyMessageHandler>);

	void pushMessage (std::unique_ptr<cNetMessage> message) override;
	std::unique_ptr<cNetMessage> popMessage() override;
	void run();

	std::string getGameState() const;
	const cPlayerBasicData* getConstPlayer (int) const;

	eOpenServerResult startServer (int port);

	void localClientConnects (cLobbyClient&, cPlayerBasicData&);

	void selectSaveGameInfo (cSaveGameInfo);
	void selectMap (std::shared_ptr<cStaticMap>);
	void selectGameSettings (std::shared_ptr<cGameSettings>);
	void askedToFinishLobby (int fromPlayer);

	const cGameSettings* getGameSettings() const { return gameSettings.get(); }

	cServer* getServer() { return server.get(); }

#if 1
	// Avoid that:
	// - (no translations (which should be client side anyway))
	// - mostly workaround for missing cMessage
	void sendChatMessage (std::string&&, int receiverPlayerNr = -1 /*, int senderPlayerNr = -1*/);
#endif

	cSignal<void (const cPlayerBasicData&)> onClientConnected;
	cSignal<void (const cPlayerBasicData&)> onClientDisconnected;
	cSignal<void (const std::string& version, const std::string& revision)> onDifferentVersion;

	cSignal<void (const cPlayerBasicData& fromPlayer)> onMapRequested;
	cSignal<void (const cPlayerBasicData& fromPlayer)> onMapUploaded;

	cSignal<void (cServer&)> onStartNewGame;
	cSignal<void (int slot)> onErrorLoadSavedGame;
	cSignal<void (cServer&, const cSaveGameInfo&)> onStartSavedGame;

private:
	cPlayerBasicData* getPlayer (int);

	void sendNetMessage (const cNetMessage&, int receiverPlayerNr = -1);
	void forwardMessage (const cNetMessage&);

	void sendPlayerList();
	void sendSaveSlots (int playerNr);
	void sendGameData (int playerNr = -1);

	void handleNetMessage (const cNetMessage&);
	void handleLobbyMessage (const cMultiplayerLobbyMessage&);

	void clientConnects (const cNetMessageTcpWantConnect&);
	void clientLeaves (const cNetMessageTcpClose&);
	void handleNetMessage_MU_MSG_CHAT (const cMuMsgChat&);
	void changeOptions (const cMuMsgOptions&);
	void changePlayerAttributes (const cMuMsgIdentification&);
	void handleAskToFinishLobby (const cMuMsgAskToFinishLobby&);

	void landingRoomStatus (const cMuMsgInLandingPositionSelectionStatus&);
	void clientLands (const cMuMsgLandingPosition&);
	void clientAbortsPreparation (const cMuMsgPlayerAbortedGamePreparations&);

private:
	cSignalConnectionManager signalConnectionManager;

	cConcurrentQueue<std::unique_ptr<cNetMessage>> messageQueue;
	std::shared_ptr<cConnectionManager> connectionManager;

	std::vector<std::unique_ptr<ILobbyMessageHandler>> lobbyMessageHandlers;

	int nextPlayerNumber = 0; // id of next player connecting

	std::vector<cPlayerBasicData> players;
	std::shared_ptr<cStaticMap> staticMap;
	std::shared_ptr<cGameSettings> gameSettings;
	cSaveGameInfo saveGameInfo{-1};

	std::shared_ptr<cLandingPositionManager> landingPositionManager;
	std::set<int> landedPlayers;

	std::unique_ptr<cServer> server;
};

#endif
