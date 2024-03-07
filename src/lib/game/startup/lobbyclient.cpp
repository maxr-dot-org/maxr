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

#include "game/logic/client.h"
#include "game/networkaddress.h"
#include "game/protocol/netmessage.h"
#include "game/startup/lobbyserver.h"
#include "game/startup/lobbyutils.h"
#include "mapdownloader/mapdownloadmessagehandler.h"
#include "maxrversion.h"
#include "utility/log.h"
#include "utility/ranges.h"

//------------------------------------------------------------------------------
cLobbyClient::cLobbyClient (std::shared_ptr<cConnectionManager> connectionManager, const cPlayerBasicData& player) :
	connectionManager (connectionManager),
	localPlayer (player)
{
	connectionManager->setLocalClient (this, -1);

	auto mapDownloadMessageHandler = std::make_unique<cMapDownloadMessageHandler>();

	signalConnectionManager.connect (mapDownloadMessageHandler->onPercentChanged, [this] (std::size_t percent) {
		onDownloadMapPercentChanged (percent);
	});
	signalConnectionManager.connect (mapDownloadMessageHandler->onCancelled, [this]() {
		onDownloadMapCancelled();
	});
	signalConnectionManager.connect (mapDownloadMessageHandler->onDownloaded, [this] (std::shared_ptr<cStaticMap> staticMap) {
		onDownloadMapFinished (staticMap);
	});

	lobbyMessageHandlers.push_back (std::move (mapDownloadMessageHandler));
}

//------------------------------------------------------------------------------
void cLobbyClient::pushMessage (std::unique_ptr<cNetMessage> message)
{
	messageQueue.push (std::move (message));
}

//------------------------------------------------------------------------------
std::unique_ptr<cNetMessage> cLobbyClient::popMessage()
{
	std::unique_ptr<cNetMessage> message;
	messageQueue.try_pop (message);
	return message;
}

//------------------------------------------------------------------------------
void cLobbyClient::run()
{
	if (client)
	{
		client->run();
		return;
	}
	std::unique_ptr<cNetMessage> message;
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
void cLobbyClient::connectToServer (const sNetworkAddress& address)
{
	// Connect only if there isn't a connection yet
	if (connectionManager->isConnectedToServer()) return;

	NetLog.debug ("Connecting to " + address.toString());

	connectionManager->connectToServer (address);
}

//------------------------------------------------------------------------------
void cLobbyClient::connectToLocalServer (cLobbyServer& server)
{
	NetLog.debug ("Connecting to local server");

	server.localClientConnects (*this, localPlayer);
	// For network clients, similar to :
	// sendNetMessage (cNetMessageTcpWantConnect (..))
}

//------------------------------------------------------------------------------
cPlayerBasicData* cLobbyClient::getPlayer (int playerNr)
{
	auto it = ranges::find_if (players, byPlayerNr (playerNr));

	return it == players.end() ? nullptr : &*it;
}

//------------------------------------------------------------------------------
void cLobbyClient::sendNetMessage (cNetMessage& message)
{
	message.From (localPlayer.getNr());

	nlohmann::json json;
	cJsonArchiveOut jsonarchive (json);
	jsonarchive << message;
	NetLog.debug ("LobbyClient: --> " + json.dump (-1) + " to host");

	connectionManager->sendToServer (message);
}

//------------------------------------------------------------------------------
void cLobbyClient::sendNetMessage (cNetMessage&& message)
{
	sendNetMessage (message); // l-value overload (not const to fix sender)
}

//------------------------------------------------------------------------------
void cLobbyClient::sendChatMessage (const std::string& message)
{
	sendNetMessage (cMuMsgChat (message));
}

//------------------------------------------------------------------------------
void cLobbyClient::selectGameSettings (const cGameSettings& gameSettings)
{
	cMuMsgOptions message;

	message.mapFilename = lobbyPreparationData.staticMap ? lobbyPreparationData.staticMap->getFilename() : "";
	message.settings = gameSettings;

	sendNetMessage (message);
}

//------------------------------------------------------------------------------
void cLobbyClient::selectMapFilename (const std::filesystem::path& mapFilename)
{
	cMuMsgOptions message;

	message.mapFilename = mapFilename;
	if (lobbyPreparationData.gameSettings)
	{
		message.settings = *lobbyPreparationData.gameSettings;
	}
	sendNetMessage (message);
}

//------------------------------------------------------------------------------
void cLobbyClient::selectLoadGame (const cSaveGameInfo& saveGameInfo)
{
	cMuMsgOptions message;

	message.mapFilename = saveGameInfo.mapFilename;
	message.saveInfo = saveGameInfo;
	sendNetMessage (message);
}

//------------------------------------------------------------------------------
void cLobbyClient::tryToSwitchReadyState()
{
	bool ready;
	if (!lobbyPreparationData.staticMap)
	{
		const auto& downloadingMapFilename = getDownloadingMapFilename();
		if (!downloadingMapFilename.empty() && !localPlayer.isReady()) onNoMapNoReady (downloadingMapFilename);
		ready = false;
	}
	else
		ready = !localPlayer.isReady();
	changeLocalPlayerProperties (localPlayer.getName(), localPlayer.getColor(), ready);
}

//------------------------------------------------------------------------------
void cLobbyClient::changeLocalPlayerProperties (const std::string& name, cRgbColor color, bool ready)
{
	const auto old = localPlayer;
	localPlayer.setName (name);
	localPlayer.setColor (color);
	localPlayer.setReady (ready);

	switch (checkTakenPlayerAttributes (players, localPlayer))
	{
		case eLobbyPlayerStatus::DuplicatedColor:
			onDuplicatedPlayerColor();
			localPlayer.setReady (false);
			break;
		case eLobbyPlayerStatus::DuplicatedName:
			onDuplicatedPlayerName();
			localPlayer.setReady (false);
			break;
		case eLobbyPlayerStatus::Ok: break;
	}

	if (connectionManager->isConnectedToServer() && old != localPlayer) sendNetMessage (cMuMsgIdentification (localPlayer));
}

//------------------------------------------------------------------------------
void cLobbyClient::askToFinishLobby()
{
	sendNetMessage (cMuMsgAskToFinishLobby());
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
	localPlayer.setNr (-1);
}
//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage (const cNetMessage& message)
{
	nlohmann::json json;
	cJsonArchiveOut jsonarchive (json);
	jsonarchive << message;
	NetLog.debug ("LobbyClient: <-- " + json.dump (-1));

	switch (message.getType())
	{
		case eNetMessageType::TCP_HELLO:
			handleNetMessage_TCP_HELLO (static_cast<const cNetMessageTcpHello&> (message));
			return;
		case eNetMessageType::TCP_CONNECTED:
			handleNetMessage_TCP_CONNECTED (static_cast<const cNetMessageTcpConnected&> (message));
			return;
		case eNetMessageType::TCP_CONNECT_FAILED:
			handleNetMessage_TCP_CONNECT_FAILED (static_cast<const cNetMessageTcpConnectFailed&> (message));
			return;
		case eNetMessageType::TCP_CLOSE:
			handleNetMessage_TCP_CLOSE (static_cast<const cNetMessageTcpClose&> (message));
			return;
		case eNetMessageType::MULTIPLAYER_LOBBY:
			handleLobbyMessage (static_cast<const cMultiplayerLobbyMessage&> (message));
			return;
		case eNetMessageType::GAME_ALREADY_RUNNING:
			handleNetMessage_GAME_ALREADY_RUNNING (static_cast<const cNetMessageGameAlreadyRunning&> (message));
			return;
		default:
			NetLog.error ("LobbyClient: Can not handle message");
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
			handleNetMessage_MU_MSG_CHAT (static_cast<const cMuMsgChat&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_NUMBER:
			handleNetMessage_MU_MSG_PLAYER_NUMBER (static_cast<const cMuMsgPlayerNr&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYERLIST:
			handleNetMessage_MU_MSG_PLAYERLIST (static_cast<const cMuMsgPlayerList&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_OPTIONS:
			handleNetMessage_MU_MSG_OPTIONS (static_cast<const cMuMsgOptions&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_SAVESLOTS:
			handleNetMessage_MU_MSG_SAVESLOTS (static_cast<const cMuMsgSaveSlots&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_CANNOT_END_LOBBY:
			handleLobbyMessage_MU_MSG_CANNOT_END_LOBBY (static_cast<const cMuMsgCannotEndLobby&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_DISCONNECT_NOT_IN_SAVED_GAME:
			handleNetMessage_MU_MSG_DISCONNECT_NOT_IN_SAVED_GAME (static_cast<const cMuMsgDisconnectNotInSavedGame&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_START_GAME_PREPARATIONS:
			handleNetMessage_MU_MSG_START_GAME_PREPARATIONS (static_cast<const cMuMsgStartGamePreparations&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_LANDING_STATE:
			handleNetMessage_MU_MSG_LANDING_STATE (static_cast<const cMuMsgLandingState&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_START_GAME:
			handleNetMessage_MU_MSG_START_GAME (static_cast<const cMuMsgStartGame&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS:
			handleNetMessage_MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS (static_cast<const cMuMsgInLandingPositionSelectionStatus&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION:
			handleNetMessage_MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION (static_cast<const cMuMsgPlayerHasSelectedLandingPosition&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION:
			handleNetMessage_MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION (static_cast<const cMuMsgPlayerAbortedGamePreparations&> (message));
			break;
		default:
			NetLog.error ("LobbyClient: Can not handle message");
			break;
	}
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_TCP_HELLO (const cNetMessageTcpHello& message)
{
	if (message.packageVersion != PACKAGE_VERSION || message.packageRev != PACKAGE_REV)
	{
		onDifferentVersion (message.packageVersion, message.packageRev);
		if (message.packageVersion != PACKAGE_VERSION) return;
	}

	cNetMessageTcpWantConnect response;
	response.player = {localPlayer.getName(), localPlayer.getColor()};
	response.ready = localPlayer.isReady();
	sendNetMessage (response);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_TCP_CONNECTED (const cNetMessageTcpConnected& message)
{
	localPlayer.setNr (message.playerNr);

	onLocalPlayerConnected();
	if (message.packageVersion != PACKAGE_VERSION || message.packageRev != PACKAGE_REV)
	{
		onDifferentVersion (message.packageVersion, message.packageRev);
	}
	Log.info ("Connected and assigned playerNr: " + std::to_string (message.playerNr));
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_TCP_CONNECT_FAILED (const cNetMessageTcpConnectFailed& message)
{
	Log.warn ("Error on connecting to server");

	localPlayer.setNr (-1);
	onConnectionFailed (message.reason);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_TCP_CLOSE (const cNetMessageTcpClose&)
{
	localPlayer.setReady (false);
	localPlayer.setNr (-1);

	onConnectionClosed();
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_CHAT (const cMuMsgChat& message)
{
	auto player = getPlayer (message.playerNr);
	const auto playerName = player == nullptr ? "unknown" : player->getName();

	onChatMessage (playerName, message.message);
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
	if (message.settings)
	{
		lobbyPreparationData.gameSettings = std::make_unique<cGameSettings> (*message.settings);
	}
	else
	{
		lobbyPreparationData.gameSettings = nullptr;
	}

	if (message.mapFilename.empty())
	{
		lobbyPreparationData.staticMap = nullptr;
	}
	else
	{
		if (!lobbyPreparationData.staticMap || lobbyPreparationData.staticMap->getFilename() != message.mapFilename)
		{
			bool mapCheckSumsEqual = (MapDownload::calculateCheckSum (message.mapFilename) == message.mapCrc);
			auto newStaticMap = std::make_shared<cStaticMap>();
			if (mapCheckSumsEqual && newStaticMap->loadMap (message.mapFilename))
			{
				triedLoadMapFilename.clear();
			}
			else
			{
				if (localPlayer.isReady())
				{
					onNoMapNoReady (message.mapFilename);
					localPlayer.setReady (false);
					sendNetMessage (cMuMsgIdentification (localPlayer));
				}
				triedLoadMapFilename = message.mapFilename;

				auto existingMapFilePath = MapDownload::getExistingMapFilePath (message.mapFilename);
				bool existsMap = !existingMapFilePath.empty();
				if (!mapCheckSumsEqual && existsMap)
				{
					onIncompatibleMap (message.mapFilename, existingMapFilePath);
				}
				else
				{
					if (MapDownload::isMapOriginal (message.mapFilename, message.mapCrc) == false)
					{
						if (message.mapFilename != lastRequestedMapFilename)
						{
							lastRequestedMapFilename = message.mapFilename;
							sendNetMessage (cMuMsgRequestMap (message.mapFilename));
							onMapDownloadRequest (message.mapFilename);
						}
					}
					else
					{
						onMissingOriginalMap (message.mapFilename);
					}
				}
			}
			lobbyPreparationData.staticMap = std::move (newStaticMap);
		}
	}
	saveGameInfo = message.saveInfo;
	onOptionsChanged (lobbyPreparationData.gameSettings, lobbyPreparationData.staticMap, saveGameInfo);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_SAVESLOTS (const cMuMsgSaveSlots& message)
{
	saveGames = message.saveGames;
}

//------------------------------------------------------------------------------
void cLobbyClient::handleLobbyMessage_MU_MSG_CANNOT_END_LOBBY (const cMuMsgCannotEndLobby& message)
{
	onCannotEndLobby (message.missingSettings, message.notReadyPlayers, message.hostNotInSavegame, message.missingPlayers);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_DISCONNECT_NOT_IN_SAVED_GAME (const cMuMsgDisconnectNotInSavedGame&)
{
	onDisconnectNotInSavedGame();
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_START_GAME_PREPARATIONS (const cMuMsgStartGamePreparations& message)
{
	if (saveGameInfo.number != -1) return;

	lobbyPreparationData.unitsData = message.unitsData;
	lobbyPreparationData.clanData = message.clanData;
	onStartGamePreparation();
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_LANDING_STATE (const cMuMsgLandingState& message)
{
	onLandingDone (message.state);
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_MU_MSG_START_GAME (const cMuMsgStartGame&)
{
	client = std::make_shared<cClient> (connectionManager);
	client->setPlayers (players, localPlayer.getNr());

	connectionManager->setLocalClient (client.get(), localPlayer.getNr());
	if (saveGameInfo.number != -1)
	{
		client->setMap (lobbyPreparationData.staticMap);
		onStartSavedGame (client);
	}
	else
	{
		client->setPreparationData (lobbyPreparationData);
		//client->initNewGame (initPlayerData));
		onStartNewGame (client);
	}
}

//------------------------------------------------------------------------------
void cLobbyClient::handleNetMessage_GAME_ALREADY_RUNNING (const cNetMessageGameAlreadyRunning& message)
{
	lobbyPreparationData.staticMap = std::make_shared<cStaticMap>();
	players = message.playerList;

	if (!lobbyPreparationData.staticMap->loadMap (message.mapFilename))
	{
		onFailToReconnectGameNoMap (message.mapFilename);
		disconnect();
		return;
	}
	else if (MapDownload::calculateCheckSum (message.mapFilename) != message.mapCrc)
	{
		onFailToReconnectGameInvalidMap (message.mapFilename);
		disconnect();
		return;
	}

	wantToRejoinGame();

	client = std::make_shared<cClient> (connectionManager);
	connectionManager->setLocalClient (client.get(), localPlayer.getNr());

	//client->setPreparationData (lobbyPreparationData);
	client->setMap (lobbyPreparationData.staticMap);
	client->setPlayers (players, localPlayer.getNr());

	onReconnectGame (client);
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
