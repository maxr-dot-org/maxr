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

#include "lobbyserver.h"

#include "game/data/units/unitdata.h"
#include "game/logic/server.h"
#include "game/startup/lobbyclient.h"
#include "game/startup/lobbyutils.h"
#include "mapdownloader/mapdownload.h"
#include "mapdownloader/mapuploadmessagehandler.h"
#include "maxrversion.h"
#include "utility/listhelpers.h"
#include "utility/log.h"
#include "utility/ranges.h"

#include <cassert>

//------------------------------------------------------------------------------
cLobbyServer::cLobbyServer (std::shared_ptr<cConnectionManager> connectionManager) :
	connectionManager (connectionManager)
{
	connectionManager->setLocalServer (this);

	auto mapUploadMessageHandler = std::make_unique<cMapUploadMessageHandler> (connectionManager, [this]() {
		return staticMap.get();
	});

	signalConnectionManager.connect (mapUploadMessageHandler->onRequested, [this] (int playerNr) {
		if (auto player = getPlayer (playerNr)) onMapRequested (*player);
	});

	signalConnectionManager.connect (mapUploadMessageHandler->onFinished, [this] (int playerNr) {
		if (auto player = getPlayer (playerNr)) onMapUploaded (*player);
	});
	lobbyMessageHandlers.push_back (std::move (mapUploadMessageHandler));
}

//------------------------------------------------------------------------------
void cLobbyServer::addLobbyMessageHandler (std::unique_ptr<ILobbyMessageHandler> messageHandler)
{
	lobbyMessageHandlers.push_back (std::move (messageHandler));
}

//------------------------------------------------------------------------------
void cLobbyServer::pushMessage (std::unique_ptr<cNetMessage> message)
{
	messageQueue.push (std::move (message));
}

//------------------------------------------------------------------------------
std::unique_ptr<cNetMessage> cLobbyServer::popMessage()
{
	std::unique_ptr<cNetMessage> message;
	messageQueue.try_pop (message);
	return message;
}

//------------------------------------------------------------------------------
std::string cLobbyServer::getGameState() const
{
	std::stringstream result;
	result << "GameState: ";

	if (landingPositionManager == nullptr)
	{
		result << "Game is open for new players" << std::endl;
		if (saveGameInfo.number != -1)
		{
			result << "Waiting players from save game:" << std::endl;
			for (const auto& player : saveGameInfo.players)
				result << " " << player.getName() << std::endl;
			result << "Turn: " << saveGameInfo.turn << std::endl;
		}
	}
	else
		result << "Game has started, players are setting up" << std::endl;

	result << "Map: " << (staticMap != nullptr ? staticMap->getFilename() : "none") << std::endl;

	result << "Players:" << std::endl;
	for (const auto& player : players)
		result << " " << player.getName() << std::endl;
	return result.str();
}

//------------------------------------------------------------------------------
const cPlayerBasicData* cLobbyServer::getConstPlayer (int playerNr) const
{
	auto it = ranges::find_if (players, byPlayerNr (playerNr));

	return it == players.end() ? nullptr : &*it;
}

//------------------------------------------------------------------------------
cPlayerBasicData* cLobbyServer::getPlayer (int playerNr)
{
	auto it = ranges::find_if (players, byPlayerNr (playerNr));

	return it == players.end() ? nullptr : &*it;
}

//------------------------------------------------------------------------------
eOpenServerResult cLobbyServer::startServer (int port)
{
	if (connectionManager->isServerOpen()) return eOpenServerResult::AlreadyOpened;

	if (connectionManager->openServer (port))
	{
		Log.warn ("Error opening socket");
		return eOpenServerResult::Failed;
	}
	else
	{
		Log.info ("Game open (Port: " + std::to_string (port) + ")");
		return eOpenServerResult::Success;
	}
}

//------------------------------------------------------------------------------
void cLobbyServer::run()
{
	std::unique_ptr<cNetMessage> message;

	while (messageQueue.try_pop (message))
	{
		handleNetMessage (*message);
	}
}

//------------------------------------------------------------------------------
void cLobbyServer::sendNetMessage (const cNetMessage& message, int receiverPlayerNr /*= -1*/)
{
	nlohmann::json json;
	cJsonArchiveOut jsonarchive (json);
	jsonarchive << message;
	NetLog.debug ("LobbyServer: --> " + json.dump (-1) + " to " + std::to_string (receiverPlayerNr));

	if (receiverPlayerNr == -1)
		connectionManager->sendToPlayers (message);
	else
		connectionManager->sendToPlayer (message, receiverPlayerNr);
}

//------------------------------------------------------------------------------
void cLobbyServer::forwardMessage (const cNetMessage& message)
{
	nlohmann::json json;
	cJsonArchiveOut jsonarchive (json);
	jsonarchive << message;
	NetLog.debug ("LobbyServer: forward --> " + json.dump (-1) + " from " + std::to_string (message.playerNr));

	for (auto& player : players)
	{
		if (message.playerNr == player.getNr()) continue;

		connectionManager->sendToPlayer (message, player.getNr());
	}
}

//------------------------------------------------------------------------------
void cLobbyServer::sendPlayerList()
{
	sendNetMessage (cMuMsgPlayerList (players));
}

//------------------------------------------------------------------------------
void cLobbyServer::sendSaveSlots (int playerNr)
{
	cMuMsgSaveSlots message;

	fillSaveGames (0, 100, message.saveGames);
	sendNetMessage (message, playerNr);
}

//------------------------------------------------------------------------------
void cLobbyServer::sendGameData (int playerNr /* = -1 */)
{
	cMuMsgOptions message;

	message.saveInfo = saveGameInfo;
	if (staticMap)
	{
		message.mapFilename = staticMap->getFilename();
		message.mapCrc = MapDownload::calculateCheckSum (staticMap->getFilename());
	}
	if (gameSettings)
	{
		message.settings = *gameSettings;
	}
	sendNetMessage (message, playerNr);
}

//------------------------------------------------------------------------------
void cLobbyServer::selectSaveGameInfo (cSaveGameInfo gameInfo)
{
	saveGameInfo = gameInfo;
	if (saveGameInfo.number >= 0)
	{
		staticMap = std::make_shared<cStaticMap>();
		if (!staticMap->loadMap (saveGameInfo.mapFilename))
		{
			staticMap = nullptr;
			//"Map \"" + saveGameInfo_.mapFilename + "\" not found";
			return;
		}
		else if (MapDownload::calculateCheckSum (saveGameInfo.mapFilename) != saveGameInfo.mapCrc)
		{
			staticMap = nullptr;
			//"The map \"" + saveGameInfo_.mapFilename + "\" does not match the map the game was started with"
			return;
		}
	}

	sendGameData();
}

//------------------------------------------------------------------------------
void cLobbyServer::selectMap (std::shared_ptr<cStaticMap> map)
{
	staticMap = map;
	sendGameData();
}

//------------------------------------------------------------------------------
void cLobbyServer::selectGameSettings (std::shared_ptr<cGameSettings> settings)
{
	gameSettings = settings;
	sendGameData();
}

//------------------------------------------------------------------------------
void cLobbyServer::askedToFinishLobby (int fromPlayer)
{
	auto message = std::make_unique<cMuMsgAskToFinishLobby>();
	message->playerNr = fromPlayer;
	pushMessage (std::move (message));
}

//------------------------------------------------------------------------------
void cLobbyServer::sendChatMessage (const std::string& message, int receiverPlayerNr /*= -1*/)
{
	NetLog.debug ("LobbyServer: --> " + message + " to " + std::to_string (receiverPlayerNr));

	if (receiverPlayerNr == -1)
		connectionManager->sendToPlayers (cMuMsgChat (message));
	else
		connectionManager->sendToPlayer (cMuMsgChat (message), receiverPlayerNr);
}

//------------------------------------------------------------------------------
void cLobbyServer::handleNetMessage (const cNetMessage& message)
{
	nlohmann::json json;
	cJsonArchiveOut jsonarchive (json);
	jsonarchive << message;
	NetLog.debug ("LobbyServer: <-- " + json.dump (-1));

	switch (message.getType())
	{
		case eNetMessageType::TCP_WANT_CONNECT:
			clientConnects (static_cast<const cNetMessageTcpWantConnect&> (message));
			return;
		case eNetMessageType::TCP_CLOSE:
			clientLeaves (static_cast<const cNetMessageTcpClose&> (message));
			return;
		case eNetMessageType::MULTIPLAYER_LOBBY:
			handleLobbyMessage (static_cast<const cMultiplayerLobbyMessage&> (message));
			return;
		default:
			NetLog.error ("Lobby Server: Can not handle message");
			return;
	}
}

//------------------------------------------------------------------------------
void cLobbyServer::handleLobbyMessage (const cMultiplayerLobbyMessage& message)
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
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_IDENTIFIKATION:
			changePlayerAttributes (static_cast<const cMuMsgIdentification&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_OPTIONS:
			changeOptions (static_cast<const cMuMsgOptions&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_ASK_TO_FINISH_LOBBY:
			handleAskToFinishLobby (static_cast<const cMuMsgAskToFinishLobby&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS:
			landingRoomStatus (static_cast<const cMuMsgInLandingPositionSelectionStatus&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_LANDING_POSITION:
			clientLands (static_cast<const cMuMsgLandingPosition&> (message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION:
			clientAbortsPreparation (static_cast<const cMuMsgPlayerAbortedGamePreparations&> (message));
			break;
		default:
			NetLog.error ("LobbyServer: Can not handle message");
			break;
	}
}

//------------------------------------------------------------------------------
void cLobbyServer::clientConnects (const cNetMessageTcpWantConnect& message)
{
	if (!connectionManager) return;

	if (message.packageVersion != PACKAGE_VERSION || message.packageRev != PACKAGE_REV)
	{
		onDifferentVersion (message.packageVersion, message.packageRev);
		if (message.packageVersion != PACKAGE_VERSION) return;
	}

	players.emplace_back (message.player, nextPlayerNumber++, false);
	const auto& newPlayer = players.back();

	connectionManager->acceptConnection (*message.socket, newPlayer.getNr());

	sendPlayerList();
	sendGameData (newPlayer.getNr());
	sendSaveSlots (newPlayer.getNr());

	onClientConnected (newPlayer);
}

//------------------------------------------------------------------------------
void cLobbyServer::localClientConnects (cLobbyClient& client, cPlayerBasicData& player)
{
	if (!connectionManager) return;

	player.setNr (nextPlayerNumber++);
	players.push_back (player);

	connectionManager->setLocalClient (&client, player.getNr());

	sendPlayerList();
	sendGameData (player.getNr());
}

//------------------------------------------------------------------------------
void cLobbyServer::clientLeaves (const cNetMessageTcpClose& message)
{
	auto it = ranges::find_if (players, byPlayerNr (message.playerNr));
	if (it == players.end()) return;
	onClientDisconnected (*it);
	players.erase (it);

	sendPlayerList();
}

//------------------------------------------------------------------------------
void cLobbyServer::handleNetMessage_MU_MSG_CHAT (const cMuMsgChat& message)
{
	// to handle special server command, you might use addLobbyMessageHandler
	forwardMessage (message);
}

//------------------------------------------------------------------------------
void cLobbyServer::changePlayerAttributes (const cMuMsgIdentification& message)
{
	auto player = getPlayer (message.playerNr);
	if (player == nullptr) return;

	player->setColor (message.playerColor);
	player->setName (message.playerName);
	player->setReady (message.ready);

	switch (checkTakenPlayerAttributes (players, *player))
	{
		case eLobbyPlayerStatus::Ok: break;

		case eLobbyPlayerStatus::DuplicatedColor:
		case eLobbyPlayerStatus::DuplicatedName: player->setReady (false); break;
	}

	sendPlayerList();
}

//------------------------------------------------------------------------------
void cLobbyServer::changeOptions (const cMuMsgOptions& message)
{
	if (message.mapFilename.empty())
	{
		staticMap.reset();
	}
	else
	{
		if (!staticMap) { staticMap = std::make_shared<cStaticMap>(); }
		staticMap->loadMap (message.mapFilename);
	}
	gameSettings = message.settings ? std::make_shared<cGameSettings> (*message.settings) : nullptr;
	selectSaveGameInfo (message.saveInfo);
}

namespace
{

	//--------------------------------------------------------------------------
	bool isInSaveGame (const cSaveGameInfo& saveGameInfo, const cPlayerBasicData* player)
	{
		if (player == nullptr) return false;

		const auto it = ranges::find_if (saveGameInfo.players, byPlayerName (player->getName()));
		return it != saveGameInfo.players.end();
	}

	//--------------------------------------------------------------------------
	std::vector<cPlayerBasicData> getMissingPlayers (const cSaveGameInfo& saveGameInfo, const std::vector<cPlayerBasicData>& players)
	{
		auto isMissingPlayer = [&] (const auto& player) { return !player.isDefeated() && ranges::find_if (players, byPlayerName (player.getName())) == players.end(); };
		return Filter (saveGameInfo.players, isMissingPlayer);
	}

} // namespace

//------------------------------------------------------------------------------
void cLobbyServer::handleAskToFinishLobby (const cMuMsgAskToFinishLobby& message)
{
	const int fromPlayer = message.playerNr;
	cMuMsgCannotEndLobby errorMessage;

	errorMessage.missingSettings = (!staticMap || (!gameSettings && saveGameInfo.number < 0));
	errorMessage.notReadyPlayers = Filter (players, [] (const auto& player) { return !player.isReady(); });
	if (saveGameInfo.number != -1)
	{
		errorMessage.hostNotInSavegame = !isInSaveGame (saveGameInfo, getPlayer (fromPlayer));
		errorMessage.missingPlayers = getMissingPlayers (saveGameInfo, players);
	}
	if (errorMessage.missingSettings || errorMessage.hostNotInSavegame || !errorMessage.notReadyPlayers.empty() || !errorMessage.missingPlayers.empty())
	{
		sendNetMessage (errorMessage, fromPlayer);
		return;
	}
	if (saveGameInfo.number != -1)
	{
		// disconnect or update menu players
		for (auto& player : players)
		{
			auto it = ranges::find_if (saveGameInfo.players, byPlayerName (player.getName()));

			if (it == saveGameInfo.players.end())
			{
				// the player does not belong to the save game: disconnect him
				sendNetMessage (cMuMsgDisconnectNotInSavedGame(), player.getNr());
				connectionManager->disconnect (player.getNr());

				player.setNr (-1); // Mark to deletion
			}
			else
			{
				const auto& savegamePlayer = *it;
				int newPlayerNr = savegamePlayer.getNr();
				int oldPlayerNr = player.getNr();

				player.setNr (newPlayerNr);
				player.setColor (savegamePlayer.getColor());

				sendNetMessage (cMuMsgPlayerNr (newPlayerNr), oldPlayerNr);
				connectionManager->changePlayerNumber (oldPlayerNr, newPlayerNr);
			}
		}
		EraseIf (players, byPlayerNr (-1));

		sendNetMessage (cMuMsgStartGame());

		server = std::make_unique<cServer> (connectionManager);

		try
		{
			server->loadGameState (saveGameInfo.number);
		}
		catch (const std::runtime_error& e)
		{
			NetLog.error ((std::string) "Error loading save game: " + e.what());
			server.reset();
			onErrorLoadSavedGame (saveGameInfo.number);
			return;
		}

		connectionManager->setLocalServer (server.get());
		server->start();
		server->resyncClientModel();

		onStartSavedGame (*server, saveGameInfo);
		return;
	}

	landingPositionManager = std::make_shared<cLandingPositionManager> (players);

	signalConnectionManager.connect (landingPositionManager->landingPositionStateChanged, [this] (const cPlayerBasicData& player, eLandingPositionState state) {
		sendNetMessage (cMuMsgLandingState (state), player.getNr());
	});

	signalConnectionManager.connect (landingPositionManager->allPositionsValid, [this]() {
		sendNetMessage (cMuMsgStartGame());
		auto unitsData = std::make_shared<const cUnitsData> (UnitsDataGlobal);
		auto clanData = std::make_shared<const cClanData> (ClanDataGlobal);

		server = std::make_unique<cServer> (connectionManager);

		server->setPreparationData ({unitsData, clanData, gameSettings, staticMap});
		server->setPlayers (players);

		connectionManager->setLocalServer (server.get());

		server->start();

		onStartNewGame (*server);
	});
	auto unitsData = std::make_shared<const cUnitsData> (UnitsDataGlobal);
	auto clanData = std::make_shared<const cClanData> (ClanDataGlobal);
	sendNetMessage (cMuMsgStartGamePreparations (unitsData, clanData));
}

//------------------------------------------------------------------------------
void cLobbyServer::clientLands (const cMuMsgLandingPosition& message)
{
	if (!landingPositionManager) return;

	NetLog.debug ("LobbyServer: received landing position from Player " + std::to_string (message.playerNr));

	auto player = getPlayer (message.playerNr);
	if (player == nullptr) return;

	auto p = landedPlayers.insert (player->getNr());
	if (p.second)
	{
		sendNetMessage (cMuMsgPlayerHasSelectedLandingPosition (player->getNr()));
	}

	landingPositionManager->setLandingPosition (*player, message.position);
}

//------------------------------------------------------------------------------
void cLobbyServer::landingRoomStatus (const cMuMsgInLandingPositionSelectionStatus& message)
{
	if (!connectionManager) return;
	assert (landingPositionManager != nullptr);

	auto player = getPlayer (message.landingPlayer);
	if (player == nullptr) return;

	if (!message.isIn)
	{
		landedPlayers.erase (player->getNr());
		landingPositionManager->deleteLandingPosition (*player);
	}
	forwardMessage (message);
}

//------------------------------------------------------------------------------
void cLobbyServer::clientAbortsPreparation (const cMuMsgPlayerAbortedGamePreparations& message)
{
	auto player = getPlayer (message.playerNr);
	if (player == nullptr) return;

	for (auto& receiver : players)
	{
		receiver.setReady (false);
	}
	forwardMessage (message);
	sendPlayerList();
}
