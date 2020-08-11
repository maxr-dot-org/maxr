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

#include "servergame.h"

#include "utility/log.h"
#include "game/data/player/player.h"
#include "game/data/savegame.h"
#include "game/data/map/map.h"
#include "game/logic/turncounter.h"
#include "game/logic/landingpositionmanager.h"
#include "game/logic/server2.h"
#include "maxrversion.h"
#include "protocol/lobbymessage.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

using namespace std;

//------------------------------------------------------------------------------
int serverGameThreadFunction (void* data)
{
	cServerGame* serverGame = reinterpret_cast<cServerGame*> (data);
	serverGame->run();
	return 0;
}

//------------------------------------------------------------------------------
cServerGame::cServerGame (std::shared_ptr<cConnectionManager> connectionManager) :
	connectionManager (std::move (connectionManager)),
	thread (nullptr),
	canceled (false),
	shouldSave (false),
	saveGameNumber (-1),
	nextPlayerNumber (0)
{}

//------------------------------------------------------------------------------
cServerGame::~cServerGame()
{
	if (thread != 0)
	{
		canceled = true;
		SDL_WaitThread (thread, nullptr);
		thread = 0;
	}
}

//------------------------------------------------------------------------------
void cServerGame::runInThread()
{
	thread = SDL_CreateThread (serverGameThreadFunction, "servergame", this);
}

//------------------------------------------------------------------------------
bool cServerGame::loadGame (int saveGameNumber)
{
	/*cSavegame savegame (saveGameNumber);
	server = std::make_unique<cServer> (network);
	if (savegame.load (*server) == false)
		return false;
	server->markAllPlayersAsDisconnected();
	server->serverState = SERVER_STATE_INGAME;
	*/
	return true;
}

//------------------------------------------------------------------------------
void cServerGame::saveGame (int saveGameNumber)
{
	if (server == nullptr)
	{
		cout << "Server not running. Can't save game." << endl;
		return;
	}
	cout << "Scheduling save command for server thread..." << endl;
	this->saveGameNumber = saveGameNumber;
	shouldSave = true;
}

//------------------------------------------------------------------------------
void cServerGame::prepareGameData()
{
	settings.setMetalAmount (eGameSettingsResourceAmount::Normal);
	settings.setOilAmount (eGameSettingsResourceAmount::Normal);
	settings.setGoldAmount (eGameSettingsResourceAmount::Normal);
	settings.setResourceDensity (eGameSettingsResourceDensity::Normal);
	settings.setStartCredits (cGameSettings::defaultCreditsNormal);
	settings.setBridgeheadType (eGameSettingsBridgeheadType::Definite);
	//settings.alienTech = SETTING_ALIENTECH_OFF;
	settings.setClansEnabled (true);
	settings.setGameType (eGameSettingsGameType::Simultaneous);
	settings.setVictoryCondition (eGameSettingsVictoryCondition::Death);
	map = std::make_shared<cStaticMap> ();
	const std::string mapName = "Mushroom.wrl";
	map->loadMap (mapName);
}

//------------------------------------------------------------------------------
void cServerGame::run()
{
	while (canceled == false)
	{
		std::unique_ptr<cNetMessage2> message;
		while (eventQueue.try_pop(message))
		{
			handleNetMessage (*message);
		}
		SDL_Delay(10);
#if 0
//		static unsigned int lastTime = 0;
//		if (server)
//			lastTime = server->getModel().getGameTime();

		// don't do anything if games hasn't been started yet!
		if (server && server->serverState == SERVER_STATE_INGAME)
		{
			server->gameTimer->run (*server);

			if (shouldSave)
			{
				/*cSavegame saveGame (saveGameNumber);
				saveGame.save (*server, "Dedicated Server Savegame");
				cout << "...saved to slot " << saveGameNumber << endl;
				shouldSave = false;
				*/
			}
		}
#endif
	}
	if (server)
		terminateServer();
}

//------------------------------------------------------------------------------
void cServerGame::sendNetMessage (const cNetMessage2& message, int receiverPlayerNr /*= -1*/)
{
	cTextArchiveIn archive;
	archive << message;
	Log.write("ServerGame: --> " + archive.data() + " to " + toString(receiverPlayerNr), cLog::eLOG_TYPE_NET_DEBUG);

	if (receiverPlayerNr == -1)
		connectionManager->sendToPlayers(message);
	else
		connectionManager->sendToPlayer(message, receiverPlayerNr);
}

//------------------------------------------------------------------------------
void cServerGame::forwardMessage (const cNetMessage2& message)
{
	for(auto& player : menuPlayers)
	{
		if (message.playerNr == player->getNr()) continue;

		sendNetMessage(message, player->getNr());
	}
}

void cServerGame::sendGameData(int playerNr /* = -1 */)
{
	cMuMsgOptions message;
	//message.saveInfo = windowNetworkLobby->getSaveGameInfo();

	if (map)
	{
		message.mapName = map->getName();
		message.mapCrc = MapDownload::calculateCheckSum(map->getName());
	}

	message.settings = settings;
	message.settingsValid = true;

	sendNetMessage(message, playerNr);
}

void cServerGame::sendChatMessage (const std::string& message, int receiverPlayerNr, int senderPlayerNr)
{
	cTextArchiveIn archive;
	archive << message;
	Log.write("ServerGame: --> " + archive.data() + " to " + toString(receiverPlayerNr), cLog::eLOG_TYPE_NET_DEBUG);

	if (receiverPlayerNr == -1)
		connectionManager->sendToPlayers(cMuMsgChat(message));
	else
		connectionManager->sendToPlayer(cMuMsgChat(message).From(senderPlayerNr), receiverPlayerNr);
}

void cServerGame::sendTranslatedChatMessage (const std::string& message, const std::string& insertText, int receiverPlayerNr, int senderPlayerNr)
{
	cTextArchiveIn archive;
	archive << message;
	Log.write("ServerGame: --> " + archive.data() + " to " + toString(receiverPlayerNr), cLog::eLOG_TYPE_NET_DEBUG);

	if (receiverPlayerNr == -1)
		connectionManager->sendToPlayers(cMuMsgChat(message, true, insertText));
	else
		connectionManager->sendToPlayer(cMuMsgChat(message, true, insertText).From(senderPlayerNr), receiverPlayerNr);
}


//------------------------------------------------------------------------------
void cServerGame::handleNetMessage (cNetMessageTcpWantConnect& message)
{
	if (!connectionManager) return;

	//add player
	auto newPlayer = std::make_shared<cPlayerBasicData>(message.playerName, cPlayerColor(message.playerColor), nextPlayerNumber++, message.ready);
	menuPlayers.push_back (newPlayer);

	if (message.packageVersion != PACKAGE_VERSION || message.packageRev != PACKAGE_REV)
	{
		sendTranslatedChatMessage ("Text~Multiplayer~Gameversion_Warning_Server", message.packageVersion + " " + message.packageRev, newPlayer->getNr());
		sendTranslatedChatMessage ("Text~Multiplayer~Gameversion_Own", (std::string)PACKAGE_VERSION + " " + PACKAGE_REV, newPlayer->getNr());
	}

	//accept the connection and assign the new player number
	connectionManager->acceptConnection(message.socket, newPlayer->getNr());

	//update playerlist of the clients
	connectionManager->sendToPlayers(cMuMsgPlayerList(menuPlayers));
	sendGameData(newPlayer->getNr());

	sendChatMessage ("type --server help for dedicated server help", newPlayer->getNr());
}

//------------------------------------------------------------------------------
void cServerGame::handleNetMessage (cNetMessageTcpClose& message)
{
	auto it = std::find_if (menuPlayers.begin(), menuPlayers.end(), [&](const auto& player){ return player->getNr() == message.playerNr; });
	if (it == menuPlayers.end()) return;
	auto& playerToRemove = **it;
	sendTranslatedChatMessage("Text~Multiplayer~Player_Left", playerToRemove.getName());
	menuPlayers.erase (it);
	sendNetMessage(cMuMsgPlayerList(menuPlayers));
}
//------------------------------------------------------------------------------
void cServerGame::handleNetMessage (cMultiplayerLobbyMessage& message)
{
	switch (message.getType())
	{
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_IDENTIFIKATION:
			handleNetMessage (static_cast<cMuMsgIdentification&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_CHAT:
			handleNetMessage (static_cast<cMuMsgChat&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_REQUEST_MAP:
			handleNetMessage (static_cast<cMuMsgRequestMap&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_FINISHED_MAP_DOWNLOAD:
			handleNetMessage (static_cast<cMuMsgFinishedMapDownload&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_LANDING_POSITION:
			handleNetMessage (static_cast<cMuMsgLandingPosition&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS:
			handleNetMessage (static_cast<cMuMsgInLandingPositionSelectionStatus&>(message));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION:
			handleNetMessage (static_cast<cMuMsgPlayerAbortedGamePreparations&>(message));
			break;
		default:
			Log.write("Host Menu Controller: Can not handle message", cLog::eLOG_TYPE_NET_ERROR);
			break;
	}
}

//------------------------------------------------------------------------------
void cServerGame::handleNetMessage (cMuMsgIdentification& message)
{
	auto player = getPlayerForNr(message.playerNr);
	if (player == nullptr) return;

	player->setColor (cPlayerColor (message.playerColor));
	player->setName (message.playerName);
	player->setReady (message.ready);

	// search double taken name or color
	//checkTakenPlayerAttributes (player);

	connectionManager->sendToPlayers (cMuMsgPlayerList (menuPlayers));

	//sendGameData (player.getNr());
}

//------------------------------------------------------------------------------
void cServerGame::handleNetMessage (cMuMsgChat& message)
{
	auto senderPlayer = getPlayerForNr(message.playerNr);
	if (senderPlayer == nullptr) return;

	const auto& chatText = message.message;

	// temporary workaround. TODO: good solution - player, who opened games must have "host" gui and new commands to send options/go to server
	// TODO: use cChatCommand
	size_t serverStringPos = chatText.find ("--server");
	if (serverStringPos == string::npos || chatText.length() <= serverStringPos + 9)
	{
		forwardMessage (message);
		return;
	}
	std::string command = chatText.substr (serverStringPos + 9);
	std::vector<std::string> tokens;
	std::istringstream iss (command);
	std::copy (std::istream_iterator<std::string> (iss), std::istream_iterator<std::string> (), std::back_inserter<std::vector<std::string> > (tokens));

	if (tokens.size() == 1)
	{
		if (tokens[0].compare ("go") == 0)
		{
			bool allReady = true;
			for (size_t i = 0; i < menuPlayers.size(); i++)
			{
				if (menuPlayers[i]->isReady() == false)
				{
					allReady = false;
					sendChatMessage (menuPlayers[i]->getName() + "is not ready...", message.playerNr);

					break;
				}
			}
			if (allReady)
			{
				std::vector<cPlayerBasicData> players;
				std::transform (menuPlayers.begin(), menuPlayers.end(), std::back_inserter (players), [] (const std::shared_ptr<cPlayerBasicData>& player) { return *player; });

				landingPositionManager = std::make_shared<cLandingPositionManager> (players);

				signalConnectionManager.connect (landingPositionManager->landingPositionSet, [this] (const cPlayerBasicData & player, const cPosition & position)
				{
					auto iter = std::find_if (playersLandingStatus.begin(), playersLandingStatus.end(), [&] (const std::unique_ptr<cPlayerLandingStatus>& entry) { return entry->getPlayer().getNr() == player.getNr(); });
					assert (iter != playersLandingStatus.end());

					auto& entry = **iter;

					const auto hadSelectedPosition = entry.hasSelectedPosition();

					entry.setHasSelectedPosition (true);

					if (entry.hasSelectedPosition() && !hadSelectedPosition)
					{
						sendNetMessage (cMuMsgPlayerHasSelectedLandingPosition(entry.getPlayer().getNr()));
					}
				});

				signalConnectionManager.connect (landingPositionManager->landingPositionStateChanged, [this] (const cPlayerBasicData& player, eLandingPositionState state)
				{
					sendNetMessage(cMuMsgLandingState(state), player.getNr());
				});


				signalConnectionManager.connect (landingPositionManager->allPositionsValid, [this]()
				{
					sendNetMessage (cMuMsgStartGame());

					server = std::make_unique<cServer2> (connectionManager);

					server->setMap (map);
					auto unitsData = std::make_shared<const cUnitsData>(UnitsDataGlobal);
					auto clanData = std::make_shared<const cClanData>(ClanDataGlobal);
					server->setUnitsData(unitsData);
					//server->setClansData(clanData);

					std::vector<cPlayerBasicData> players;
					std::transform (menuPlayers.begin(), menuPlayers.end(), std::back_inserter (players), [] (const std::shared_ptr<cPlayerBasicData>& player) { return *player; });
					server->setPlayers(players);
					server->setGameSettings (settings);

					connectionManager->setLocalServer(server.get());

					server->start();
				});

				auto unitsData = std::make_shared<const cUnitsData>(UnitsDataGlobal);
				auto clanData = std::make_shared<const cClanData>(ClanDataGlobal);
				sendNetMessage (cMuMsgStartGamePreparations(unitsData, clanData));
			}
			else
			{
				sendChatMessage ("Not all players are ready...", message.playerNr);
			}
		}

	}
	else if (tokens.size() >= 2)
	{
		if (tokens[0].compare ("map") == 0)
		{
			std::string mapName = tokens[1];
			for (size_t i = 2; i < tokens.size(); i++)
			{
				mapName += " ";
				mapName += tokens[i];
			}
			if (map != nullptr && map->loadMap (mapName))
			{
				sendGameData();
				string reply = senderPlayer->getName();
				reply += " changed the map.";
				sendChatMessage (reply);
			}
			else
			{
				string reply = "Could not load map ";
				reply += mapName;
				sendChatMessage (reply, senderPlayer->getNr());
			}
		}
		if (tokens.size() == 2)
		{
			if (tokens[0].compare ("credits") == 0)
			{
				int credits = atoi (tokens[1].c_str());
				settings.setStartCredits (credits);
				sendGameData();
				string reply = senderPlayer->getName();
				reply += " changed the starting credits.";
				sendChatMessage (reply);
			}
			else if (tokens[0].compare ("oil") == 0 || tokens[0].compare ("gold") == 0 || tokens[0].compare ("metal") == 0
					 || tokens[0].compare ("res") == 0)
			 {
				configRessources (tokens, *senderPlayer);
			 }
		}
	}
}

//------------------------------------------------------------------------------
void cServerGame::handleNetMessage (cMuMsgRequestMap& message)
{
	sendChatMessage ("Not implemented", message.playerNr);
	// Factorize code with cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_REQUEST_MAP
}

//------------------------------------------------------------------------------
void cServerGame::handleNetMessage (cMuMsgFinishedMapDownload& message)
{
	sendTranslatedChatMessage ("Text~Multiplayer~MapDL_UploadFinished", message.playerName);
}

//------------------------------------------------------------------------------
void cServerGame::handleNetMessage (cMuMsgLandingPosition& message)
{
	if (!landingPositionManager) return;

	Log.write ("Server: received landing coords from Player " + iToStr (message.playerNr), cLog::eLOG_TYPE_NET_DEBUG);

	auto player = getPlayerForNr(message.playerNr);
	if (player == nullptr) return;

	landingPositionManager->setLandingPosition (*player, message.position);
}

//------------------------------------------------------------------------------
void cServerGame::handleNetMessage (cMuMsgInLandingPositionSelectionStatus& message)
{
	if (message.isIn)
	{
		auto player = getPlayerForNr(message.playerNr);
		if (player == nullptr) return;

		playersLandingStatus.push_back (std::make_unique<cPlayerLandingStatus> (*player));
	}
	else
	{
		playersLandingStatus.erase (std::remove_if (playersLandingStatus.begin(), playersLandingStatus.end(), [&message] (const std::unique_ptr<cPlayerLandingStatus>& status) { return status->getPlayer().getNr() == message.playerNr; }), playersLandingStatus.end());
	}

	sendNetMessage(message);
}

//------------------------------------------------------------------------------
void cServerGame::handleNetMessage (cMuMsgPlayerAbortedGamePreparations& message)
{
	forwardMessage (message);
	for(auto& player : menuPlayers)
	{
		player->setReady(false);
	}
	connectionManager->sendToPlayers(cMuMsgPlayerList(menuPlayers));
}

void cServerGame::handleNetMessage (cNetMessage2& message)
{
	cTextArchiveIn archive;
	message.serialize(archive);

	std::cout << "Msg received: " << archive.data() << endl;

	switch (message.getType())
	{
		case eNetMessageType::TCP_WANT_CONNECT:
			handleNetMessage (static_cast<cNetMessageTcpWantConnect&>(message));
			return;
		case eNetMessageType::TCP_CLOSE:
			handleNetMessage (static_cast<cNetMessageTcpClose&>(message));
			return;
		case eNetMessageType::MULTIPLAYER_LOBBY:
			handleNetMessage (static_cast<cMultiplayerLobbyMessage&>(message));
			break;

		default:
			Log.write("Host Menu Controller: Can not handle message", cLog::eLOG_TYPE_NET_ERROR);
			return;
	}
}

//------------------------------------------------------------------------------
void cServerGame::configRessources (std::vector<std::string>& tokens, const cPlayerBasicData& senderPlayer)
{
	if (tokens[0].compare ("res") == 0)
	{
		bool valid = true;
		eGameSettingsResourceDensity density;
		if (tokens[1].compare ("sparse") == 0) density = eGameSettingsResourceDensity::Sparse;
		else if (tokens[1].compare ("normal") == 0) density = eGameSettingsResourceDensity::Normal;
		else if (tokens[1].compare ("dense") == 0) density = eGameSettingsResourceDensity::Dense;
		else if (tokens[1].compare ("most") == 0) density = eGameSettingsResourceDensity::TooMuch;
		else valid = false;

		if (valid)
		{
			settings.setResourceDensity (density);
			sendGameData ();
			string reply = senderPlayer.getName();
			reply += " changed the resource frequency to ";
			reply += tokens[1];
			reply += ".";
			sendChatMessage (reply);
		}
		else
			sendChatMessage ("res must be one of: sparse normal dense most", senderPlayer.getNr());
	}
	if (tokens[0].compare ("oil") == 0 || tokens[0].compare ("gold") == 0 || tokens[0].compare ("metal") == 0)
	{
		bool valid = true;
		eGameSettingsResourceAmount amount;
		if (tokens[1].compare ("low") == 0) amount = eGameSettingsResourceAmount::Limited;
		else if (tokens[1].compare ("normal") == 0) amount = eGameSettingsResourceAmount::Normal;
		else if (tokens[1].compare ("much") == 0) amount = eGameSettingsResourceAmount::High;
		else if (tokens[1].compare ("most") == 0) amount = eGameSettingsResourceAmount::TooMuch;
		else valid = false;

		if (valid)
		{
			if (tokens[0].compare ("oil") == 0) settings.setOilAmount (amount);
			else if (tokens[0].compare ("metal") == 0) settings.setMetalAmount (amount);
			else if (tokens[0].compare ("gold") == 0) settings.setGoldAmount (amount);
			sendGameData ();
			string reply = senderPlayer.getName();
			reply += " changed the resource density of ";
			reply += tokens[0];
			reply += " to ";
			reply += tokens[1];
			reply += ".";
			sendChatMessage (reply);
		}
		else
			sendChatMessage ("oil|gold|metal must be one of: low normal much most", senderPlayer.getNr());
	}
}

//------------------------------------------------------------------------------
void cServerGame::terminateServer()
{
	server = nullptr;
}

//------------------------------------------------------------------------------
void cServerGame::pushMessage (std::unique_ptr<cNetMessage2> message)
{
 	eventQueue.push (std::move (message));
}

//------------------------------------------------------------------------------
std::unique_ptr<cNetMessage2> cServerGame::popMessage()
{
	std::unique_ptr<cNetMessage2> res;
	if (eventQueue.try_pop (res)) {
		return res;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
std::string cServerGame::getGameState() const
{
#if 0
	std::stringstream result;
	result << "GameState: ";

	if (server == nullptr)
		result << "Game is open for new players" << endl;
	else if (server-> serverState == SERVER_STATE_INITGAME)
		result << "Game has started, players are setting up" << endl;
	else if (server->serverState == SERVER_STATE_INGAME)
		result << "Game is active" << endl;

	result << "Map: " << (map != nullptr ? map->getName() : "none") << endl;
	if (server != nullptr)
		result << "Turn: " << server->getTurnClock()->getTurn() << endl;

	result << "Players:" << endl;
	if (server != nullptr && server->playerList.empty() == false)
	{
		for (size_t i = 0; i != server->playerList.size(); ++i)
		{
			const cPlayer& player = *server->playerList[i];
			result << " " << player.getName() << (server->isPlayerDisconnected (player) ? " (disconnected)" : " (online)") << endl;
		}
	}
	else if (menuPlayers.empty() == false)
	{
		for (size_t i = 0; i < menuPlayers.size(); i++)
			result << " " << menuPlayers[i]->getName() << endl;
	}
	return result.str();
#else
	return "Not yet implemented";
#endif
}

//------------------------------------------------------------------------------
std::shared_ptr<cPlayerBasicData> cServerGame::getPlayerForNr (int playerNr) const
{
	auto it = std::find_if (menuPlayers.begin(), menuPlayers.end(), [&](const auto& player){ return player->getNr() == playerNr; });
	return (it == menuPlayers.end()) ? nullptr : *it;
}
