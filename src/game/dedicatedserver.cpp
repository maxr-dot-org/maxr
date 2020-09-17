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

#include "dedicatedserver.h"

#include "utility/listhelpers.h"
#include "defines.h"
#include "utility/files.h"
#include "protocol/netmessage.h"
#include "game/network.h"
#include "game/data/report/savedreportchat.h"
#include "game/servergame.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

//------------------------------------------------------------------------
class cDedicatedServerConfig
{
public:
	cDedicatedServerConfig() = default;

	int port = DEFAULTPORT;
};

//------------------------------------------------------------------------
// cDedicatedServer implementation
//------------------------------------------------------------------------

static const int kAutoSaveSlot = 11;

//--------------------------------------------------
cDedicatedServer& cDedicatedServer::instance()
{
	static cDedicatedServer _instance;
	return _instance;
}

//------------------------------------------------------------------------
cDedicatedServer::cDedicatedServer() :
	configuration (std::make_unique<cDedicatedServerConfig>())
{
	connectionManager->setLocalServer(this);
}

//------------------------------------------------------------------------
cDedicatedServer::~cDedicatedServer()
{
}

//------------------------------------------------------------------------
void cDedicatedServer::run()
{
	printHelp (kHelpHelp);
	printPrompt();

	while (true)
	{
		std::string input;
		std::getline (std::cin, input);  // waits for user input in the terminal
		if (std::cin.good() == false)   // happens during debugging
			std::cin.clear();
		if (handleInput (input) == false)
			break;
		printPrompt();
	}
}

//------------------------------------------------------------------------
bool cDedicatedServer::handleInput (const std::string& command)
{
	std::vector<std::string> tokens;
	std::istringstream iss (command);
	std::copy (std::istream_iterator<std::string> (iss), std::istream_iterator<std::string>(), std::back_inserter<std::vector<std::string>> (tokens));
	if (tokens.empty())
		return true;
	if (tokens.at (0) == "help")
	{
		if (tokens.size() == 1) printHelp (kHelpGeneral);
		else if (tokens.at (1) == "newGame") printHelp (kHelpNewGame);
		else if (tokens.at (1) == "loadGame") printHelp (kHelpLoadGame);
		else if (tokens.at (1) == "saveGame") printHelp (kHelpSaveGame);
		else if (tokens.at (1) == "stop") printHelp (kHelpStop);
		else if (tokens.at (1) == "games") printHelp (kHelpGames);
		else if (tokens.at (1) == "maps") printHelp (kHelpMaps);
		else if (tokens.at (1) == "set") printHelp (kHelpSet);
		else if (tokens.at (1) == "printconfig") printHelp (kHelpPrintConfig);
		else if (tokens.at (1) == "exit") printHelp (kHelpExit);
		// ...
	}
	else if (tokens.at (0) == "newGame")
		return startServer();
	else if (tokens.at (0) == "loadGame")
	{
		if (tokens.size() == 2)
			return startServer (atoi (tokens.at (1).c_str()));
		else
		{
			std::cout << "No savegame number given. Trying to load auto save (savegame number " << kAutoSaveSlot << ")." << std::endl;
			return startServer (kAutoSaveSlot);
		}
	}
	else if (tokens.at (0) == "saveGame")
	{
		if (tokens.size() == 2)
			saveGame (atoi (tokens.at (1).c_str()));
		else
			printHelp (kHelpWrongArguments);
	}
	else if (tokens.at (0) == "set")
	{
		if (tokens.size() == 3)
			setProperty (tokens.at (1), tokens.at (2));
		else
			printHelp (kHelpWrongArguments);
	}
	else if (tokens.at (0) == "games")
		printGames();
	else if (tokens.at (0) == "maps")
		printMaps();
	else if (tokens.at (0) == "printconfig")
		printConfiguration();
	else if (tokens.at (0) == "exit")
		return false;
	// ...
	else if (tokens.at (0) == "stop")
		printHelp (kNotImplementedYet);
	else
		printHelp (kHelpUnknownCommand);
	return true;
}

//------------------------------------------------------------------------
bool cDedicatedServer::startServer (int saveGameNumber)
{
	if (connectionManager->isServerOpen())
	{
		std::cout << "WARNING: Server is already open." << std::endl;
		return true;
	}
	std::cout << "Starting server on port " << configuration->port << "..." << std::endl;

	if (connectionManager->openServer(configuration->port))
	{
		std::cout << "ERROR: Initializing network failed." << std::endl;
		return false;
	}

	// TODO: muss von Clients ausgeloest werden. Aber, dann muss ganze Infrastruktur angepasst werden,
	// dass z.B. NetMessages an richtiges Game/cServer gehen und dass die Methoden nicht auf einem globalen
	// (oder zumindest dem aktuell richtigen) server Objekt arbeiten.
	if (saveGameNumber >= 0)
		loadSaveGame (saveGameNumber);
	else
		startNewGame();
	return true;
}

//------------------------------------------------------------------------
void cDedicatedServer::startNewGame()
{
	std::cout << "Setting up new game..." << std::endl;
	auto game = std::make_unique<cServerGame> (connectionManager);
	game->prepareGameData();
	games.push_back (std::move (game));
	games.back()->runInThread();
}

//------------------------------------------------------------------------
void cDedicatedServer::loadSaveGame (int saveGameNumber)
{
	std::cout << "Setting up game from saved game number " << saveGameNumber << " ..." << std::endl;
	auto game = std::make_unique<cServerGame> (connectionManager);
	if (game->loadGame (saveGameNumber) == false)
	{
		std::cout << "Loading game failed. Game is not setup." << std::endl;
		return;
	}
	games.push_back (std::move (game));
	games.back()->runInThread();
}

//------------------------------------------------------------------------
void cDedicatedServer::saveGame (int saveGameNumber)
{
	if (games.empty() == false)
	{
		std::cout << "Save game in slot " << saveGameNumber << std::endl;
		games[0]->saveGame (saveGameNumber);
	}
	else
		std::cout << "No game running. Can't save." << std::endl;
}

//------------------------------------------------------------------------
void cDedicatedServer::setProperty (const std::string& property, const std::string& value)
{
	if (property == "port")
	{
		int newPort = atoi (value.c_str());
		if (newPort < 0 || newPort >= 65536)
			newPort = DEFAULTPORT;
		configuration->port = newPort;
		std::cout << "Port set to: " << newPort << std::endl;
	}
}

//------------------------------------------------------------------------
void cDedicatedServer::printConfiguration() const
{
	std::cout << "--- Server Configuration ---" << std::endl;
	std::cout << "port: " << configuration->port << std::endl;
}

//------------------------------------------------------------------------
void cDedicatedServer::printPrompt() const
{
	std::cout << "> ";
}

//------------------------------------------------------------------------
void cDedicatedServer::printGames() const
{
	std::cout << getGamesString();
}

//------------------------------------------------------------------------
std::string cDedicatedServer::getGamesString() const
{
	std::stringstream oss;
	if (games.empty())
		oss << "No games started" << std::endl;
	for (size_t i = 0; i < games.size(); i++)
	{
		oss << "--------- Game " << i << ": -----------" << std::endl;
		const auto& game = games[i];
		oss << game->getGameState();
	}
	return oss.str();
}

//------------------------------------------------------------------------
void cDedicatedServer::printMaps() const
{
	std::cout << getAvailableMapsString();
}

//------------------------------------------------------------------------
std::string cDedicatedServer::getAvailableMapsString() const
{
	std::stringstream oss;
	std::vector<std::string> maps = getFilesOfDirectory (cSettings::getInstance().getMapsPath());
	if (getUserMapsDir().empty() == false)
	{
		std::vector<std::string> userMaps = getFilesOfDirectory (getUserMapsDir());
		for (size_t i = 0; i != userMaps.size(); ++i)
		{
			if (Contains (maps, userMaps[i]) == false)
				maps.push_back (userMaps[i]);
		}
	}
	oss << "----- Available maps: ------" << std::endl;
	for (size_t i = 0; i != maps.size(); ++i)
	{
		std::string mapFilename = maps[i];
		if (mapFilename.compare (mapFilename.length() - 3, 3, "WRL") == 0
			|| mapFilename.compare (mapFilename.length() - 3, 3, "wrl") == 0)
		{
			oss << mapFilename << std::endl;
		}
	}
	return oss.str();
}

//------------------------------------------------------------------------
std::string cDedicatedServer::getServerHelpString() const
{
	std::stringstream oss;
	oss << "--- Dedicated server help ---" << std::endl;
	oss << "Type --server and then one of the following:" << std::endl;
	oss << "go : starts the game" << std::endl;
	oss << "games : shows the running games and their players" << std::endl;
	oss << "maps : shows the available maps" << std::endl;
	oss << "map mapname.wrl : changes the map" << std::endl;
	oss << "credits 0 | 50 | 100 | 150 | 200 | 250 : changes the starting credits" << std::endl;
	oss << "oil | gold | metal  low | normal | much | most : resource density" << std::endl;
	oss << "res sparse | normal | dense | most : changes the resource frequency" << std::endl;
	return oss.str();
}

//------------------------------------------------------------------------
void cDedicatedServer::printHelp (eHelpCommands helpCommand) const
{
	switch (helpCommand)
	{
		case kHelpUnknownCommand:
			std::cout << "Unknown command." << std::endl;
			break;
		case kNotImplementedYet:
			std::cout << "Not implemented yet." << std::endl;
			break;
		case kHelpHelp:
			std::cout << "Type \"help\" for help." << std::endl;
			break;
		case kHelpWrongArguments:
			std::cout << "Wrong number or wrong formatted arguments" << std::endl;
			break;
		case kHelpGeneral:
			std::cout << "Available commands: newGame loadGame saveGame stop games maps set printconfig exit" << std::endl;
			std::cout << "Type \"help command\" to see help to a sepecific command" << std::endl;
			break;
		case kHelpNewGame:
			std::cout << "Starts the server and allows players to open games and connect to the server." << std::endl;
			break;
		case kHelpLoadGame:
			std::cout << "Starts the server, loads a savegame and allows players to reconnect (resync) to the server." << std::endl;
			std::cout << "Type: \"loadGame x\" to load savegame x (x is the number of the savegame slot; 11 is autosave)." << std::endl;
			break;
		case kHelpSaveGame:
			std::cout << "Save the current game to the given savegame slot." << std::endl;
			std::cout << "Type: \"saveGame x\" to save the game to slot x (x is a number >= 0)." << std::endl;
			break;
		case kHelpStop:
			std::cout << "Stops the server and all running games." << std::endl;
			break;
		case kHelpGames:
			std::cout << "Prints the list of currently running games." << std::endl;
			break;
		case kHelpMaps:
			std::cout << "Prints the names of the available maps." << std::endl;
			break;
		case kHelpSet:
			std::cout << "Sets a configuration property of the server." << std::endl << "For some properties, the server must not run already." << std::endl;
			std::cout << "Type \"set property-name value\". Available properties:" << std::endl;
			std::cout << " port  - the tcp port, on which the server listens." << std::endl;
			break;
		case kHelpPrintConfig:
			std::cout << "Prints the configuration of the server to the console" << std::endl;
			break;
		case kHelpExit:
			std::cout << "Quits without bothering to stop a running server before." << std::endl << "You should call stop before." << std::endl;
			break;
		default:
			std::cout << "Error in printHelp: no help page implemented. Please contact developers." << std::endl;
			break;
	}
}

//------------------------------------------------------------------------
void cDedicatedServer::pushMessage (std::unique_ptr<cNetMessage> message)
{
	if (handleDedicatedServerEvents (*message))
		return;
	// TODO: delegate to correct game (and not simply first game)
	if (games.empty() == false)
		games[0]->pushMessage (std::move (message));
}

//------------------------------------------------------------------------
std::unique_ptr<cNetMessage> cDedicatedServer::popMessage()
{
	if (games.empty() == false)
		return games[0]->popMessage();
	return nullptr;
}

//------------------------------------------------------------------------
bool cDedicatedServer::handleDedicatedServerEvents (cNetMessage& message)
{
	if (message.getType() != eNetMessageType::MULTIPLAYER_LOBBY) { return false; }

	auto& lobbyMessage = static_cast<cMultiplayerLobbyMessage&>(message);

	if (lobbyMessage.getType() != cMultiplayerLobbyMessage::eMessageType::MU_MSG_CHAT) { return false; }

	auto& chatMessage = static_cast<cMuMsgChat&>(lobbyMessage);

	const auto& chatText = chatMessage.message;

	size_t serverStringPos = chatText.find ("--server");
	if (serverStringPos != std::string::npos && chatText.length() > serverStringPos + 9)
	{
		std::string command = chatText.substr (serverStringPos + 9);
		std::vector<std::string> tokens;
		std::istringstream iss (command);
		std::copy (std::istream_iterator<std::string> (iss), std::istream_iterator<std::string>(), std::back_inserter<std::vector<std::string> > (tokens));
		if (tokens.size() == 1)
		{
			if (tokens[0] == "games")
			{
				sendChatMessage (getGamesString(), message.playerNr);
				return true;
			}
			else if (tokens[0] == "maps")
			{
				sendChatMessage (getAvailableMapsString(), message.playerNr);
				return true;
			}
			else if (tokens[0] == "help")
			{
				sendChatMessage (getServerHelpString(), message.playerNr);
				return true;
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------
void cDedicatedServer::sendChatMessage (const std::string& text, int receiver)
{
	cMuMsgChat message(text, false, "");

	if (receiver == -1)
	{
		connectionManager->sendToPlayers(message);
	}
	else
	{
		connectionManager->sendToPlayer(message, receiver);
	}
}
