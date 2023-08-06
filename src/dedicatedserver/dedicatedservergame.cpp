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

#include "dedicatedservergame.h"

#include "game/protocol/lobbymessage.h"
#include "utility/log.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

namespace
{
	//--------------------------------------------------------------------------
	class cDedicatedServerChatMessageHandler : public ILobbyMessageHandler
	{
	public:
		explicit cDedicatedServerChatMessageHandler (std::function<void (int, const std::vector<std::string>&)> handleChatCommand) :
			handleChatCommand (handleChatCommand)
		{}

	private:
		bool handleMessage (const cMultiplayerLobbyMessage& message) final
		{
			if (message.getType() != cMultiplayerLobbyMessage::eMessageType::MU_MSG_CHAT) return false;
			const auto& chatMessage = static_cast<const cMuMsgChat&> (message);

			const auto& chatText = chatMessage.message;
			size_t serverStringPos = chatText.find ("--server");
			if (serverStringPos == std::string::npos || chatText.length() <= serverStringPos + 9)
			{
				return false;
			}
			std::string command = chatText.substr (serverStringPos + 9);
			std::vector<std::string> tokens;
			std::istringstream iss (command);
			std::copy (std::istream_iterator<std::string> (iss), std::istream_iterator<std::string>(), std::back_inserter<std::vector<std::string>> (tokens));
			handleChatCommand (message.playerNr, tokens);
			return true;
		}

	private:
		std::function<void (int, const std::vector<std::string>&)> handleChatCommand;
	};

	//--------------------------------------------------------------------------
	std::string getServerHelpString()
	{
		std::stringstream oss;
		oss << "--- Dedicated server help ---" << std::endl;
		oss << "Type --server and then one of the following:" << std::endl;
		oss << "go: starts the game" << std::endl;
		oss << "games: shows the running games and their players" << std::endl;
		oss << "maps: shows the available maps" << std::endl;
		oss << "map mapname.wrl: changes the map" << std::endl;
		return oss.str();
	}

} // namespace

//------------------------------------------------------------------------------
cDedicatedServerGame::cDedicatedServerGame (int saveGameNumber) :
	lobbyServer (std::make_shared<cConnectionManager>())
{
	lobbyServer.addLobbyMessageHandler (std::make_unique<cDedicatedServerChatMessageHandler> ([this] (int playerId, const std::vector<std::string>& tokens) { handleChatCommand (playerId, tokens); }));

	signalConnectionManager.connect (lobbyServer.onClientConnected, [this] (const cPlayerBasicData& player) {
		lobbyServer.sendChatMessage ("type --server help for dedicated server help", player.getNr());
	});
	signalConnectionManager.connect (lobbyServer.onDifferentVersion, [] (const std::string& version, const std::string& revision) {
		std::cout << "player connects with different version:" << version << " " << revision << std::endl;
	});

	signalConnectionManager.connect (lobbyServer.onMapRequested, [] (const cPlayerBasicData& player) {
		std::cout << player.getName() << " requests map." << std::endl;
	});
	signalConnectionManager.connect (lobbyServer.onMapUploaded, [] (const cPlayerBasicData& player) {
		std::cout << player.getName() << " finished to download map." << std::endl;
	});

	signalConnectionManager.connect (lobbyServer.onErrorLoadSavedGame, [] (int slot) {
		std::cout << "Cannot load savegame " << slot << std::endl;
	});

	signalConnectionManager.connect (lobbyServer.onStartNewGame, [this] (cServer& server) {
		this->server = &server;
	});
	signalConnectionManager.connect (lobbyServer.onStartSavedGame, [this] (cServer& server, const cSaveGameInfo&) {
		this->server = &server;
	});

	// Nothing for:
	// lobbyServer.onClientDisconnect

	if (saveGameNumber > 0)
	{
		loadGame (saveGameNumber);
	}
	else
	{
		prepareGameData();
	}
}

//------------------------------------------------------------------------------
cDedicatedServerGame::~cDedicatedServerGame()
{
	canceled = true;
	if (thread.joinable()) { thread.join(); }
}

//------------------------------------------------------------------------------
eOpenServerResult cDedicatedServerGame::startServer (int port)
{
	return lobbyServer.startServer (port);
}

//------------------------------------------------------------------------------
std::string cDedicatedServerGame::getGameState() const
{
	std::lock_guard<std::recursive_mutex> l{mutex};
	return (server == nullptr) ? lobbyServer.getGameState() : server->getGameState();
}

//------------------------------------------------------------------------------
void cDedicatedServerGame::runInThread()
{
	thread = std::thread ([this]() {
		try
		{
			run();
		}
		catch (const std::exception& ex)
		{
			std::cerr << "Exception: " << ex.what() << std::endl;
			Log.error (std::string ("Exception: ") + ex.what());
		}
	});
}

//------------------------------------------------------------------------------
void cDedicatedServerGame::run()
{
	using namespace std::literals::chrono_literals;
	while (canceled == false)
	{
		std::this_thread::sleep_for (10ms);
		std::lock_guard<std::recursive_mutex> l{mutex};
		lobbyServer.run();

		// don't do anything if games haven't been started yet!
		if (server && shouldSave)
		{
			server->saveGameState (saveGameNumber, "Dedicated Server Savegame");
			std::cout << "...saved to slot " << saveGameNumber << std::endl;
			shouldSave = false;
		}
	}
	server = nullptr;
}

//------------------------------------------------------------------------------
void cDedicatedServerGame::saveGame (int saveGameNumber)
{
	std::lock_guard<std::recursive_mutex> l{mutex};
	if (server == nullptr)
	{
		std::cout << "Server not running. Can't save game." << std::endl;
		return;
	}
	std::cout << "Scheduling save command for server thread..." << std::endl;
	this->saveGameNumber = saveGameNumber;
	shouldSave = true;
}

//------------------------------------------------------------------------------
void cDedicatedServerGame::loadGame (int saveGameNumber)
{
	cSaveGameInfo saveGameInfo = cSavegame().loadSaveInfo (saveGameNumber);
	lobbyServer.selectSaveGameInfo (saveGameInfo);
}

//------------------------------------------------------------------------------
void cDedicatedServerGame::prepareGameData()
{
	auto map = std::make_shared<cStaticMap>();
	const std::filesystem::path mapFilename = "Mushroom.wrl";
	map->loadMap (mapFilename);

	lobbyServer.selectGameSettings (std::make_shared<cGameSettings>());
	lobbyServer.selectMap (map);
}

//--------------------------------------------------------------------------
void cDedicatedServerGame::handleChatCommand (int fromPlayer, const std::vector<std::string>& tokens)
{
	if (tokens.size() == 1)
	{
		if (tokens[0] == "go")
		{
			lobbyServer.askedToFinishLobby (fromPlayer);
		}
		else if (tokens[0] == "help")
		{
			lobbyServer.sendChatMessage (getServerHelpString(), fromPlayer);
		}
		else if (tokens[0] == "games")
		{
			lobbyServer.sendChatMessage (getGamesString(), fromPlayer);
		}
		else if (tokens[0] == "maps")
		{
			lobbyServer.sendChatMessage (getAvailableMapsString(), fromPlayer);
		}
	}
	else if (tokens.size() >= 2)
	{
		if (tokens[0] == "map")
		{
			std::string mapName = tokens[1];
			for (size_t i = 2; i < tokens.size(); i++)
			{
				mapName += " ";
				mapName += tokens[i];
			}
			auto map = std::make_shared<cStaticMap>();
			const auto* senderPlayer = lobbyServer.getConstPlayer (fromPlayer);
			if (map->loadMap (std::filesystem::u8path (mapName)))
			{
				lobbyServer.selectMap (map);
				std::string reply = senderPlayer->getName();
				reply += " changed the map.";
				lobbyServer.sendChatMessage (reply);
			}
			else
			{
				std::string reply = "Could not load map ";
				reply += mapName;
				lobbyServer.sendChatMessage (reply, senderPlayer->getNr());
			}
		}
	}
}
