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

#include "utility/log.h"
#include "maxrversion.h"
#include "mapdownloader/mapdownload.h"
#include "mapdownloader/mapuploadmessagehandler.h"
#include "resources/uidata.h"

//------------------------------------------------------------------------------
cLobbyServer::cLobbyServer (std::shared_ptr<cConnectionManager> connectionManager) :
	connectionManager (connectionManager)
{
	connectionManager->setLocalServer (this);

	auto mapUploadMessageHandler = std::make_unique<cMapUploadMessageHandler> (connectionManager, [this]()
	{
		return staticMap.get();
	});

	signalConnectionManager.connect (mapUploadMessageHandler->onRequested, [this](int playerNr)
	{
		if (auto player = getPlayer (playerNr)) onMapRequested (*player);
	});

	signalConnectionManager.connect (mapUploadMessageHandler->onFinished, [this](int playerNr)
	{
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
void cLobbyServer::pushMessage (std::unique_ptr<cNetMessage2> message)
{
	messageQueue.push (std::move (message));
}

//------------------------------------------------------------------------------
std::unique_ptr<cNetMessage2> cLobbyServer::popMessage()
{
	std::unique_ptr<cNetMessage2> message;
	messageQueue.try_pop(message);
	return message;
}


//------------------------------------------------------------------------------
const cPlayerBasicData* cLobbyServer::getConstPlayer (int playerNr) const
{
	auto it = std::find_if (players.begin(), players.end(), byPlayerNr(playerNr));

	return it == players.end() ? nullptr : &*it;
}

//------------------------------------------------------------------------------
cPlayerBasicData* cLobbyServer::getPlayer (int playerNr)
{
	auto it = std::find_if (players.begin(), players.end(), byPlayerNr(playerNr));

	return it == players.end() ? nullptr : &*it;
}

#if 0 // startServer (int port)
//------------------------------------------------------------------------------
void cLobbyServer::startServer (int port)
{
	if (!connectionManager) return;

	if (connectionManager->isServerOpen()) return;

	if (connectionManager->openServer(port))
	{
		//windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Error_Socket"));
		Log.write ("Error opening socket", cLog::eLOG_TYPE_WARNING);
	}
	else
	{
		Log.write ("Game open (Port: " + iToStr (port) + ")", cLog::eLOG_TYPE_INFO);
		//windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Open") + " (" + lngPack.i18n ("Text~Title~Port") + lngPack.i18n ("Text~Punctuation~Colon")  + iToStr (port) + ")");
		//windowNetworkLobby->disablePortEdit();
	}
}
#endif
//------------------------------------------------------------------------------
void cLobbyServer::run()
{
	std::unique_ptr<cNetMessage2> message;

	while (messageQueue.try_pop (message))
	{
		handleNetMessage (*message);
	}
}

//------------------------------------------------------------------------------
void cLobbyServer::sendNetMessage(const cNetMessage2& message, int receiverPlayerNr /*= -1*/)
{
	cTextArchiveIn archive;
	archive << message;
	Log.write ("LobbyServer: --> " + archive.data() + " to " + toString (receiverPlayerNr), cLog::eLOG_TYPE_NET_DEBUG);

	if (receiverPlayerNr == -1)
		connectionManager->sendToPlayers (message);
	else
		connectionManager->sendToPlayer (message, receiverPlayerNr);
}

//------------------------------------------------------------------------------
void cLobbyServer::forwardMessage (const cNetMessage2& message)
{
	cTextArchiveIn archive;
	archive << message;
	Log.write ("LobbyServer: forward --> " + archive.data() + " from " + toString (message.playerNr), cLog::eLOG_TYPE_NET_DEBUG);

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
void cLobbyServer::sendGameData(int playerNr /* = -1 */)
{
	cMuMsgOptions message;

	message.saveInfo = saveGameInfo;
	if (staticMap)
	{
		message.mapName = staticMap->getName();
		message.mapCrc = MapDownload::calculateCheckSum (staticMap->getName());
	}
	if (gameSettings)
	{
		message.settings = *gameSettings;
		message.settingsValid = true;
	}
	sendNetMessage (message, playerNr);
}

//------------------------------------------------------------------------------
void cLobbyServer::selectSaveGameInfo (cSaveGameInfo gameInfo)
{
	saveGameInfo = gameInfo;
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
void cLobbyServer::sendChatMessage (const std::string& message, int receiverPlayerNr /*= -1*/)
{
	cTextArchiveIn archive;
	archive << message;
	Log.write("LobbyServer: --> " + archive.data() + " to " + toString(receiverPlayerNr), cLog::eLOG_TYPE_NET_DEBUG);

	if (receiverPlayerNr == -1)
		connectionManager->sendToPlayers (cMuMsgChat (message));
	else
		connectionManager->sendToPlayer (cMuMsgChat (message), receiverPlayerNr);
}

//------------------------------------------------------------------------------
void cLobbyServer::startGamePreparation (int fromPlayer)
{
	const auto notReadyPlayer = findNotReadyPlayer();

	if (notReadyPlayer) {
		sendChatMessage (notReadyPlayer->getName() + " is not ready...", fromPlayer);
		sendChatMessage ("Not all players are ready...", fromPlayer);
		return;
	}
	landingPositionManager = std::make_shared<cLandingPositionManager> (players);

	signalConnectionManager.connect (landingPositionManager->landingPositionStateChanged, [this] (const cPlayerBasicData& player, eLandingPositionState state)
	{
		sendNetMessage (cMuMsgLandingState(state), player.getNr());
	});

	signalConnectionManager.connect (landingPositionManager->allPositionsValid, [this]()
	{
		sendNetMessage (cMuMsgStartGame());
		auto unitsData = std::make_shared<const cUnitsData>(UnitsDataGlobal);
		auto clanData = std::make_shared<const cClanData>(ClanDataGlobal);
		onStartNewGame (sLobbyPreparationData{unitsData, clanData, gameSettings, staticMap, players}, connectionManager);
	});
	auto unitsData = std::make_shared<const cUnitsData>(UnitsDataGlobal);
	auto clanData = std::make_shared<const cClanData>(ClanDataGlobal);
	sendNetMessage (cMuMsgStartGamePreparations (unitsData, clanData));
}

//------------------------------------------------------------------------------
const cPlayerBasicData* cLobbyServer::findNotReadyPlayer() const
{
	auto it = std::find_if (players.begin(), players.end(), [](const auto& player){ return !player.isReady(); });

	return it == players.end() ? nullptr : &*it;
}


//------------------------------------------------------------------------------
void cLobbyServer::handleNetMessage (const cNetMessage2& message)
{
	cTextArchiveIn archive;
	archive << message;
	Log.write("lobbyServer: <-- " + archive.data(), cLog::eLOG_TYPE_NET_DEBUG);

	switch (message.getType())
	{
		case eNetMessageType::TCP_WANT_CONNECT:
			clientConnects(static_cast<const cNetMessageTcpWantConnect&>(message));
			return;
		case eNetMessageType::TCP_CLOSE:
			clientLeaves(static_cast<const cNetMessageTcpClose&>(message));
			return;
		case eNetMessageType::MULTIPLAYER_LOBBY:
			handleLobbyMessage (static_cast<const cMultiplayerLobbyMessage&>(message));
			return;
		default:
			Log.write("Lobby Server: Can not handle message", cLog::eLOG_TYPE_NET_ERROR);
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
			handleNetMessage_MU_MSG_CHAT (static_cast<const cMuMsgChat&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_IDENTIFIKATION:
			changePlayerAttributes(static_cast<const cMuMsgIdentification&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS:
			landingRoomStatus(static_cast<const cMuMsgInLandingPositionSelectionStatus&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_LANDING_POSITION:
			clientLands (static_cast<const cMuMsgLandingPosition&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION:
			clientAbortsPreparation (static_cast<const cMuMsgPlayerAbortedGamePreparations&>(message));
			break;
		default:
			Log.write("LobbyServer: Can not handle message", cLog::eLOG_TYPE_NET_ERROR);
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
	}

	players.emplace_back(message.playerName, cPlayerColor(message.playerColor), nextPlayerNumber++, false);
	const auto& newPlayer = players.back();

	connectionManager->acceptConnection (message.socket, newPlayer.getNr());

	sendPlayerList();
	sendGameData (newPlayer.getNr());

	onClientConnected (newPlayer);
}

//------------------------------------------------------------------------------
void cLobbyServer::clientLeaves (const cNetMessageTcpClose& message)
{
	auto it = std::find_if (players.begin(), players.end(), byPlayerNr (message.playerNr));
	if (it == players.end()) return;
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
	auto player = getPlayer(message.playerNr);
	if (player == nullptr) return;

	player->setColor (cPlayerColor (message.playerColor));
	player->setName (message.playerName);
	player->setReady (message.ready);

#if 0
	// search double taken name or color
	checkTakenPlayerAttributes (*player);
#endif
	sendPlayerList();
}

//------------------------------------------------------------------------------
void cLobbyServer::clientLands (const cMuMsgLandingPosition& message)
{
	if (!landingPositionManager) return;

	Log.write ("LobbyServer: received landing position from Player " + iToStr (message.playerNr), cLog::eLOG_TYPE_NET_DEBUG);

	auto player = getPlayer (message.playerNr);
	if (player == nullptr) return;

	landingPositionManager->setLandingPosition (*player, message.position);

	auto p = landedPlayers.insert (player->getNr());
	if (p.second)
	{
		sendNetMessage (cMuMsgPlayerHasSelectedLandingPosition (player->getNr()));
	}
}

//------------------------------------------------------------------------------
void cLobbyServer::landingRoomStatus (const cMuMsgInLandingPositionSelectionStatus& message)
{
	if (!connectionManager) return;
	assert (landingPositionManager != nullptr);

	auto player = getPlayer(message.landingPlayer);
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
	auto player = getPlayer(message.playerNr);
	if (player == nullptr) return;

	for (auto& receiver : players)
	{
		receiver.setReady(false);
	}
	forwardMessage (message);
	sendPlayerList();

#if 0
	auto okDialog = application.show(std::make_shared<cDialogOk>("Player " + player->getName() + " has quit from game preparation")); // TODO: translate

	signalConnectionManager.connect(okDialog->done, [this]()
	{
		application.closeTill(*windowNetworkLobby);
	});
#endif
}
