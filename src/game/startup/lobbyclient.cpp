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

#include "lobbyclient.h"

#include "game/startup/lobbyserver.h"
#include "game/startup/lobbyutils.h"
#include "mapdownloader/mapdownloadmessagehandler.h"
#include "maxrversion.h"
#include "protocol/netmessage.h"
#include "utility/log.h"
#include "utility/ranges.h"

//------------------------------------------------------------------------------
cLobbyClient::cLobbyClient (std::shared_ptr<cConnectionManager> connectionManager, const cPlayerBasicData& player) :
	connectionManager (connectionManager),
	localPlayer (player)
{
	connectionManager->setLocalClient (this, -1);

	auto mapDownloadMessageHandler = std::make_unique <cMapDownloadMessageHandler>();

	signalConnectionManager.connect (mapDownloadMessageHandler->onPercentChanged, [this](std::size_t percent){
		onDownloadMapPercentChanged (percent);
	});
	signalConnectionManager.connect (mapDownloadMessageHandler->onCancelled, [this](){
		onDownloadMapCancelled ();
	});
	signalConnectionManager.connect (mapDownloadMessageHandler->onDownloaded, [this](std::shared_ptr<cStaticMap> staticMap){
		onDownloadMapFinished (staticMap);
	});

	lobbyMessageHandlers.push_back (std::move (mapDownloadMessageHandler));
}

//------------------------------------------------------------------------------
void cLobbyClient::pushMessage (std::unique_ptr<cNetMessage2> message)
{
	messageQueue.push (std::move (message));
}

//------------------------------------------------------------------------------
std::unique_ptr<cNetMessage2> cLobbyClient::popMessage()
{
	std::unique_ptr<cNetMessage2> message;
	messageQueue.try_pop(message);
	return message;
}

//------------------------------------------------------------------------------
void cLobbyClient::run()
{
	std::unique_ptr<cNetMessage2> message;
	while (messageQueue.try_pop (message))
	{
		handleNetMessage (*message);
	}
}

//------------------------------------------------------------------------------
bool cLobbyClient::isConnectedToServer() const
{
	return connectionManager->isConnectedToServer();
}

//------------------------------------------------------------------------------
void cLobbyClient::connectToServer(std::string ip, int port)
{
	// Connect only if there isn't a connection yet
	if (connectionManager->isConnectedToServer()) return;

	Log.write (("Connecting to " + ip + ":" + iToStr (port)), cLog::eLOG_TYPE_NET_DEBUG);

	connectionManager->connectToServer(ip, port, localPlayer);
}

//------------------------------------------------------------------------------
void cLobbyClient::connectToLocalServer (cLobbyServer& server)
{
	Log.write ("Connecting to local server", cLog::eLOG_TYPE_NET_DEBUG);

	server.localClientConnects (*this, localPlayer);
	// For network clients, similar to :
	// sendNetMessage (cNetMessageTcpWantConnect (..))
	// sendNetMessage (cMuMsgIdentification (localPlayer));
}

//------------------------------------------------------------------------------
cPlayerBasicData* cLobbyClient::getPlayer (int playerNr)
{
	auto it = ranges::find_if (players, byPlayerNr(playerNr));

	return it == players.end() ? nullptr : &*it;
}

//------------------------------------------------------------------------------
void cLobbyClient::sendNetMessage (cNetMessage2& message)
{
	message.From (localPlayer.getNr());
	cTextArchiveIn archive;
	archive << message;
	Log.write("LobbyClient: --> " + archive.data() + " to host", cLog::eLOG_TYPE_NET_DEBUG);

	connectionManager->sendToServer (message);
}

//------------------------------------------------------------------------------
void cLobbyClient::sendNetMessage (cNetMessage2&& message)
{
	sendNetMessage (message); // l-value overload (not const to fix sender)
}

//------------------------------------------------------------------------------
void cLobbyClient::sendChatMessage (const std::string& message)
{
	sendNetMessage (cMuMsgChat (message));
}

//------------------------------------------------------------------------------
void cLobbyClient::tryToSwitchReadyState()
{
	bool ready;
	if (!staticMap)
	{
		const auto& downloadingMapName = getDownloadingMapName();
		if (!downloadingMapName.empty() && !localPlayer.isReady()) onNoMapNoReady (downloadingMapName);
		ready = false;
	}
	else ready = !localPlayer.isReady();
	changeLocalPlayerProperties (localPlayer.getName(), localPlayer.getColor(), ready);
}

//------------------------------------------------------------------------------
void cLobbyClient::changeLocalPlayerProperties (const std::string& name, cPlayerColor color, bool ready)
{
	const auto old = localPlayer;
	localPlayer.setName (name);
	localPlayer.setColor (color);
	localPlayer.setReady (ready);

	switch (checkTakenPlayerAttributes (players, localPlayer))
	{
		case eLobbyPlayerStatus::DuplicatedColor: onDuplicatedPlayerColor(); localPlayer.setReady (false); break;
		case eLobbyPlayerStatus::DuplicatedName: onDuplicatedPlayerName(); localPlayer.setReady (false); break;
		case eLobbyPlayerStatus::Ok: break;
	}

	if (connectionManager->isConnectedToServer() && old != localPlayer) sendNetMessage (cMuMsgIdentification (localPlayer));
}

//------------------------------------------------------------------------------
void cLobbyClient::abortGamePreparation()
{
	sendNetMessage (cMuMsgPlayerAbortedGamePreparations());
}

//------------------------------------------------------------------------------
void cLobbyClient::enterLandingSelection()
{
	sendNetMessage (cMuMsgInLandingPositionSelectionStatus (localPlayer.getNr(), true));
}

//------------------------------------------------------------------------------
void cLobbyClient::exitLandingSelection()
{
	sendNetMessage (cMuMsgInLandingPositionSelectionStatus (localPlayer.getNr(), false));
}

//------------------------------------------------------------------------------
void cLobbyClient::selectLandingPosition (cPosition landingPosition)
{
	sendNetMessage (cMuMsgLandingPosition (landingPosition));
}

//------------------------------------------------------------------------------
void cLobbyClient::wantToRejoinGame()
{
	sendNetMessage (cNetMessageWantRejoinGame());
}
//------------------------------------------------------------------------------
void cLobbyClient::disconnect()
{
	connectionManager->disconnect (localPlayer.getNr());
}
//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage (const cNetMessage2& message)
{
	cTextArchiveIn archive;
	archive << message;
	Log.write("LobbyClient: <-- " + archive.data(), cLog::eLOG_TYPE_NET_DEBUG);

	switch (message.getType())
	{
		case eNetMessageType::TCP_CONNECTED:
			handleNetMessage_TCP_CONNECTED(static_cast<const cNetMessageTcpConnected&>(message));
			return;
		case eNetMessageType::TCP_CONNECT_FAILED:
			handleNetMessage_TCP_CONNECT_FAILED(static_cast<const cNetMessageTcpConnectFailed&>(message));
			return;
		case eNetMessageType::TCP_CLOSE:
			handleNetMessage_TCP_CLOSE(static_cast<const cNetMessageTcpClose&>(message));
			return;
		case eNetMessageType::MULTIPLAYER_LOBBY:
			handleLobbyMessage (static_cast<const cMultiplayerLobbyMessage&>(message));
			return;
		case eNetMessageType::GAME_ALREADY_RUNNING:
			handleNetMessage_GAME_ALREADY_RUNNING(static_cast<const cNetMessageGameAlreadyRunning&>(message));
			return;
		default:
			Log.write("LobbyClient: Can not handle message", cLog::eLOG_TYPE_NET_ERROR);
			return;
	}
}

//------------------------------------------------------------------------------
void cLobbyClient::handleLobbyMessage (const cMultiplayerLobbyMessage& message)
{
	for (auto& lobbyMessageHandler : lobbyMessageHandlers)
	{
		if (lobbyMessageHandler->handleMessage (message))
		{
			return;
		}
	}

	switch (message.getType())
	{
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_CHAT:
			handleNetMessage_MU_MSG_CHAT(static_cast<const cMuMsgChat&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_NUMBER:
			handleNetMessage_MU_MSG_PLAYER_NUMBER(static_cast<const cMuMsgPlayerNr&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYERLIST:
			handleNetMessage_MU_MSG_PLAYERLIST(static_cast<const cMuMsgPlayerList&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_OPTIONS:
			handleNetMessage_MU_MSG_OPTIONS(static_cast<const cMuMsgOptions&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_START_GAME_PREPARATIONS:
			handleNetMessage_MU_MSG_START_GAME_PREPARATIONS(static_cast<const cMuMsgStartGamePreparations&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_LANDING_STATE:
			handleNetMessage_MU_MSG_LANDING_STATE(static_cast<const cMuMsgLandingState&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_START_GAME:
			handleNetMessage_MU_MSG_START_GAME(static_cast<const cMuMsgStartGame&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS:
			handleNetMessage_MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS(static_cast<const cMuMsgInLandingPositionSelectionStatus&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION:
			handleNetMessage_MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION(static_cast<const cMuMsgPlayerHasSelectedLandingPosition&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION:
			handleNetMessage_MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION(static_cast<const cMuMsgPlayerAbortedGamePreparations&>(message));
			break;
		default:
			Log.write ("LobbyClient: Can not handle message", cLog::eLOG_TYPE_NET_ERROR);
			break;
	}
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_TCP_CONNECTED(const cNetMessageTcpConnected& message)
{
	localPlayer.setNr (message.playerNr);

	sendNetMessage (cMuMsgIdentification (localPlayer));

	onLocalPlayerConnected();
	if (message.packageVersion != PACKAGE_VERSION || message.packageRev != PACKAGE_REV)
	{
		onDifferentVersion (message.packageVersion, message.packageRev);
	}
	Log.write("Connected and assigned playerNr: " + toString(message.playerNr), cLog::eLOG_TYPE_INFO);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_TCP_CONNECT_FAILED(const cNetMessageTcpConnectFailed& message)
{
	Log.write("Error on connecting to server", cLog::eLOG_TYPE_WARNING);

	onConnectionFailed (message.reason);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_TCP_CLOSE (const cNetMessageTcpClose& message)
{
	localPlayer.setReady (false);

	onConnectionClosed();
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_CHAT (const cMuMsgChat& message)
{
	auto player = getPlayer (message.playerNr);
	const auto playerName = player == nullptr ? "unknown" : player->getName();

	onChatMessage(playerName, message.translate, message.message, message.insertText);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_PLAYER_NUMBER (const cMuMsgPlayerNr& message)
{
	connectionManager->changePlayerNumber (localPlayer.getNr(), message.newPlayerNr);
	localPlayer.setNr (message.newPlayerNr);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_PLAYERLIST (const cMuMsgPlayerList& message)
{
	players.clear();

	for (const auto& playerData : message.playerList)
	{
		if (playerData.getNr() == localPlayer.getNr())
		{
			localPlayer = playerData;
			players.push_back (localPlayer);
		}
		else
		{
			players.push_back (playerData);
		}
	}
	onPlayersList (localPlayer, players);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_OPTIONS (const cMuMsgOptions& message)
{
	if (message.settingsValid)
	{
		gameSettings = std::make_unique<cGameSettings> (message.settings);
	}
	else
	{
		gameSettings = nullptr;
	}

	if (message.mapName.empty())
	{
		staticMap = nullptr;
	}
	else
	{
		if (!staticMap || staticMap->getName() != message.mapName)
		{
			bool mapCheckSumsEqual = (MapDownload::calculateCheckSum (message.mapName) == message.mapCrc);
			auto newStaticMap = std::make_shared<cStaticMap>();
			if (mapCheckSumsEqual && newStaticMap->loadMap (message.mapName))
			{
				triedLoadMapName = "";
			}
			else
			{
				if (localPlayer.isReady())
				{
					onNoMapNoReady (message.mapName);
					localPlayer.setReady (false);
					sendNetMessage (cMuMsgIdentification (localPlayer));
				}
				triedLoadMapName = message.mapName;

				auto existingMapFilePath = MapDownload::getExistingMapFilePath (message.mapName);
				bool existsMap = !existingMapFilePath.empty();
				if (!mapCheckSumsEqual && existsMap)
				{
					onIncompatibleMap(message.mapName, existingMapFilePath);
				}
				else
				{
					if (MapDownload::isMapOriginal (message.mapName, message.mapCrc) == false)
					{
						if (message.mapName != lastRequestedMapName)
						{
							lastRequestedMapName = message.mapName;
							sendNetMessage (cMuMsgRequestMap (message.mapName));
							onMapDownloadRequest (message.mapName);
						}
					}
					else
					{
						onMissingOriginalMap (message.mapName);
					}
				}
			}
			staticMap = std::move (newStaticMap);
		}
	}
	onOptionsChanged (gameSettings, staticMap, saveGameInfo);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_START_GAME_PREPARATIONS (const cMuMsgStartGamePreparations& message)
{
	if (saveGameInfo.number != -1) return;

	onStartGamePreparation (sLobbyPreparationData {message.unitsData, message.clanData, gameSettings, staticMap, players}, localPlayer, connectionManager);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_LANDING_STATE (const cMuMsgLandingState& message)
{
	onLandingDone (message.state);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_START_GAME (const cMuMsgStartGame& message)
{
	if (saveGameInfo.number != -1)
	{
		onStartSavedGame (saveGameInfo, staticMap, connectionManager, localPlayer);
	}
	else
	{
		onStartNewGame();
	}
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_GAME_ALREADY_RUNNING(const cNetMessageGameAlreadyRunning& message)
{
	staticMap = std::make_shared<cStaticMap>();

	if (!staticMap->loadMap(message.mapName))
	{
		onFailToReconnectGameNoMap (message.mapName);
		disconnect();
		return;
	}
	else if (MapDownload::calculateCheckSum(message.mapName) != message.mapCrc)
	{
		onFailToReconnectGameInvalidMap (message.mapName);
		disconnect();
		return;
	}
	onReconnectGame (staticMap, connectionManager, localPlayer, message.playerList);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS (const cMuMsgInLandingPositionSelectionStatus& message)
{
	auto player = getPlayer (message.landingPlayer);
	if (player == nullptr) return;

	onPlayerEnterLeaveLandingSelectionRoom (*player, message.isIn);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION (const cMuMsgPlayerHasSelectedLandingPosition& message)
{
	auto player = getPlayer (message.landedPlayer);
	if (player == nullptr) return;

	onPlayerSelectLandingPosition (*player);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION (const cMuMsgPlayerAbortedGamePreparations& message)
{
	const auto* player = getPlayer (message.playerNr);
	if (player == nullptr) return;

	onPlayerAbortGamePreparation (player->getName());
}
