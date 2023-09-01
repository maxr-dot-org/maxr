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

#include "dedicatedserver/dedicatedservergame.h"
#include "defines.h"
#include "settings.h"
#include "utility/listhelpers.h"
#include "utility/os.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

static const int kAutoSaveSlot = 11;

//------------------------------------------------------------------------
namespace
{
	void printHelpUnknownCommand()
	{
		std::cout << "Unknown command." << std::endl;
	}
	void printHelpHelp()
	{
		std::cout << "Type \"help\" for help." << std::endl;
	}
	void printHelpWrongArguments()
	{
		std::cout << "Wrong number or wrong formatted arguments" << std::endl;
	}
	void printHelpGeneral()
	{
		std::cout << "Available commands: newGame loadGame saveGame stop games maps set printconfig exit" << std::endl;
		std::cout << "Type \"help command\" to see help to a sepecific command" << std::endl;
	}
	void printHelpNewGame()
	{
		std::cout << "Starts the server and allows players to open games and connect to the server." << std::endl;
	}
	void printHelpLoadGame()
	{
		std::cout << "Starts the server, loads a savegame and allows players to reconnect (resync) to the server." << std::endl;
		std::cout << "Type: \"loadGame x\" to load savegame x (x is the number of the savegame slot; 11 is autosave)." << std::endl;
	}
	void printHelpSaveGame()
	{
		std::cout << "Save the current game to the given savegame slot." << std::endl;
		std::cout << "Type: \"saveGame x\" to save the game to slot x (x is a number >= 0)." << std::endl;
	}
	void printHelpStop()
	{
		std::cout << "Stops the server and all running games." << std::endl;
	}
	void printHelpGames()
	{
		std::cout << "Prints the list of currently running games." << std::endl;
	}
	void printHelpMaps()
	{
		std::cout << "Prints the names of the available maps." << std::endl;
	}
	void printHelpSet()
	{
		std::cout << "Sets a configuration property of the server." << std::endl;
		std::cout << "For some properties, the server must not run already." << std::endl;
		std::cout << "Type \"set property-name value\". Available properties:" << std::endl;
		std::cout << " port  - the tcp port, on which the server listens." << std::endl;
	}
	void printHelpPrintConfig()
	{
		std::cout << "Prints the configuration of the server to the console" << std::endl;
	}
	void printHelpExit()
	{
		std::cout << "Quits without bothering to stop a running server before." << std::endl
				  << "You should call stop before." << std::endl;
	}

} // namespace

//------------------------------------------------------------------------
// cDedicatedServer implementation
//------------------------------------------------------------------------

//------------------------------------------------------------------------
cDedicatedServer::cDedicatedServer (int port) :
	port (port)
{
}

//------------------------------------------------------------------------
cDedicatedServer::~cDedicatedServer()
{
}

//------------------------------------------------------------------------
void cDedicatedServer::run()
{
	printHelpHelp();
	printPrompt();

	while (true)
	{
		std::string input;
		std::getline (std::cin, input); // waits for user input in the terminal
		if (std::cin.good() == false) // happens during debugging
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
		if (tokens.size() == 1)
			printHelpGeneral();
		else if (tokens.at (1) == "newGame")
			printHelpNewGame();
		else if (tokens.at (1) == "loadGame")
			printHelpLoadGame();
		else if (tokens.at (1) == "saveGame")
			printHelpSaveGame();
		else if (tokens.at (1) == "stop")
			printHelpStop();
		else if (tokens.at (1) == "games")
			printHelpGames();
		else if (tokens.at (1) == "maps")
			printHelpMaps();
		else if (tokens.at (1) == "set")
			printHelpSet();
		else if (tokens.at (1) == "printconfig")
			printHelpPrintConfig();
		else if (tokens.at (1) == "exit")
			printHelpExit();
		// ...
	}
	else if (tokens.at (0) == "newGame")
	{
		startServer();
	}
	else if (tokens.at (0) == "loadGame")
	{
		if (tokens.size() == 2)
			startServer (atoi (tokens.at (1).c_str()));
		else
		{
			std::cout << "No savegame number given. Trying to load auto save (savegame number " << kAutoSaveSlot << ")." << std::endl;
			startServer (kAutoSaveSlot);
		}
	}
	else if (tokens.at (0) == "saveGame")
	{
		// TODO: select game, currently apply to first game
		if (tokens.size() == 2)
			saveGame (atoi (tokens.at (1).c_str()));
		else
			printHelpWrongArguments();
	}
	else if (tokens.at (0) == "set")
	{
		if (tokens.size() == 3)
			setProperty (tokens.at (1), tokens.at (2));
		else
			printHelpWrongArguments();
	}
	else if (tokens.at (0) == "games")
		printGames();
	else if (tokens.at (0) == "maps")
		printMaps();
	else if (tokens.at (0) == "printconfig")
		printConfiguration();
	else if (tokens.at (0) == "exit")
		return false;
	else if (tokens.at (0) == "stop")
		stopGames();
	else
		printHelpUnknownCommand();
	return true;
}

//------------------------------------------------------------------------
bool cDedicatedServer::startServer (int saveGameNumber)
{
	if (ranges::find_if (games, [=] (const auto& game) { return game->getPort() == port; }) != games.end())
	{
		std::cout << "WARNING: Server is already open." << std::endl;
		return true;
	}
	if (saveGameNumber >= 0)
	{
		std::cout << "Setting up game from saved game number " << saveGameNumber << " ..." << std::endl;
	}
	else
	{
		std::cout << "Setting up new game..." << std::endl;
	}
	auto game = std::make_unique<cDedicatedServerGame> (saveGameNumber);
	game->getGamesString = [this]() { return getGamesString(); };
	game->getAvailableMapsString = [this]() { return getAvailableMapsString(); };

	std::cout << "Starting server on port " << port << "..." << std::endl;
	switch (game->startServer (port))
	{
		case eOpenServerResult::AlreadyOpened: // should not happen
		case eOpenServerResult::Failed:
			std::cout << "ERROR: Initializing network failed." << std::endl;
			return false;
		case eOpenServerResult::Success: break;
	}

	games.push_back (std::move (game));
	games.back()->runInThread();
	return true;
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
void cDedicatedServer::stopGames()
{
	games.clear();
}

//------------------------------------------------------------------------
void cDedicatedServer::setProperty (const std::string& property, const std::string& value)
{
	if (property == "port")
	{
		int newPort = atoi (value.c_str());
		if (newPort < 0 || newPort >= 65536)
			newPort = DEFAULTPORT;
		port = newPort;
		std::cout << "Port set to: " << newPort << std::endl;
	}
}

//------------------------------------------------------------------------
void cDedicatedServer::printConfiguration() const
{
	std::cout << "--- Server Configuration ---" << std::endl;
	std::cout << "port: " << port << std::endl;
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
	auto maps = os::getFilesOfDirectory (cSettings::getInstance().getMapsPath());
	if (cSettings::getInstance().getUserMapsDir().empty() == false)
	{
		const auto userMaps = os::getFilesOfDirectory (cSettings::getInstance().getUserMapsDir());
		for (const auto& userMap : userMaps)
		{
			if (ranges::contains (maps, userMap) == false)
				maps.push_back (userMap);
		}
	}
	oss << "----- Available maps: ------" << std::endl;
	for (const auto& mapFilename : maps)
	{
		if (mapFilename.extension() == ".WRL" || mapFilename.extension() == ".wrl")
		{
			oss << mapFilename.u8string() << std::endl;
		}
	}
	return oss.str();
}
