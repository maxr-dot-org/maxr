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

#include "clientevents.h"
#include "clist.h"
#include "defines.h"
#include "files.h"
#include "menuevents.h"
#include "netmessage.h"
#include "network.h"
#include "savegame.h"
#include "serverevents.h"
#include "servergame.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

using namespace std;

//------------------------------------------------------------------------
class cDedicatedServerConfig
{
public:
	cDedicatedServerConfig();

	int port;
};

//------------------------------------------------------------------------
cDedicatedServerConfig::cDedicatedServerConfig()
	: port (DEFAULTPORT)
{
}


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
cDedicatedServer::cDedicatedServer()
{
	network = 0;
	configuration = new cDedicatedServerConfig();
}

//------------------------------------------------------------------------
cDedicatedServer::~cDedicatedServer()
{
	for (size_t i = 0; i < games.size(); i++)
		delete games[i];
	delete network;

	delete configuration;
}

//------------------------------------------------------------------------
void cDedicatedServer::run()
{
	printHelp (kHelpHelp);
	printPrompt();
	bool done = false;
	while (done == false)
	{
		string input;
		getline (cin, input);  // waits for user input in the terminal
		if (cin.good() == false)   // happens during debugging
			cin.clear();
		if (handleInput (input) == false)
			done = true;
		printPrompt();
	}
}

//------------------------------------------------------------------------
bool cDedicatedServer::handleInput (const string& command)
{
	vector<string> tokens;
	istringstream iss (command);
	copy (istream_iterator<string> (iss), istream_iterator<string>(), back_inserter<vector<string> > (tokens));
	if (tokens.empty())
		return true;
	if (tokens.at (0).compare ("help") == 0)
	{
		if (tokens.size() == 1) printHelp (kHelpGeneral);
		else if (tokens.at (1).compare ("newGame") == 0) printHelp (kHelpNewGame);
		else if (tokens.at (1).compare ("loadGame") == 0) printHelp (kHelpLoadGame);
		else if (tokens.at (1).compare ("saveGame") == 0) printHelp (kHelpSaveGame);
		else if (tokens.at (1).compare ("stop") == 0) printHelp (kHelpStop);
		else if (tokens.at (1).compare ("games") == 0) printHelp (kHelpGames);
		else if (tokens.at (1).compare ("maps") == 0) printHelp (kHelpMaps);
		else if (tokens.at (1).compare ("set") == 0) printHelp (kHelpSet);
		else if (tokens.at (1).compare ("printconfig") == 0) printHelp (kHelpPrintConfig);
		else if (tokens.at (1).compare ("exit") == 0) printHelp (kHelpExit);
		// ...
	}
	else if (tokens.at (0).compare ("newGame") == 0)
		return startServer();
	else if (tokens.at (0).compare ("loadGame") == 0)
	{
		if (tokens.size() == 2)
			return startServer (atoi (tokens.at (1).c_str()));
		else
		{
			cout << "No savegame number given. Trying to load auto save (savegame number " << kAutoSaveSlot << ")." << endl;
			return startServer (kAutoSaveSlot);
		}
	}
	else if (tokens.at (0).compare ("saveGame") == 0)
	{
		if (tokens.size() == 2)
			saveGame (atoi (tokens.at (1).c_str()));
		else
			printHelp (kHelpWrongArguments);
	}
	else if (tokens.at (0).compare ("set") == 0)
	{
		if (tokens.size() == 3)
			setProperty (tokens.at (1), tokens.at (2));
		else
			printHelp (kHelpWrongArguments);
	}
	else if (tokens.at (0).compare ("games") == 0)
		printGames();
	else if (tokens.at (0).compare ("maps") == 0)
		printMaps();
	else if (tokens.at (0).compare ("printconfig") == 0)
		printConfiguration();
	else if (tokens.at (0).compare ("exit") == 0)
		return false;
	// ...
	else if (tokens.at (0).compare ("stop") == 0)
		printHelp (kNotImplementedYet);
	else
		printHelp (kHelpUnknownCommand);
	return true;
}

//------------------------------------------------------------------------
bool cDedicatedServer::startServer (int saveGameNumber)
{
	if (network != 0)
	{
		cout << "WARNING: Server is already open." << endl;
		return true;
	}

	cout << "Starting server on port " << configuration->port << "..." << endl;

	assert (network == 0);
	network = new cTCP();
	network->setMessageReceiver (this);
	if (network->create (configuration->port) == -1)
	{
		cout << "ERROR: Initializing network failed." << endl;
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
	cout << "Setting up new game..." << endl;
	cServerGame* game = new cServerGame (*network);
	game->prepareGameData();
	games.push_back (game);
	game->runInThread();
}

//------------------------------------------------------------------------
void cDedicatedServer::loadSaveGame (int saveGameNumber)
{
	cout << "Setting up game from saved game number " << saveGameNumber << " ..." << endl;
	cServerGame* game = new cServerGame (*network);
	if (game->loadGame (saveGameNumber) == false)
	{
		delete game;
		cout << "Loading game failed. Game is not setup." << endl;
		return;
	}
	games.push_back (game);
	game->runInThread();
}

//------------------------------------------------------------------------
void cDedicatedServer::saveGame (int saveGameNumber)
{
	if (games.empty() == false)
	{
		cout << "Save game in slot " << saveGameNumber << endl;
		games[0]->saveGame (saveGameNumber);
	}
	else
		cout << "No game running. Can't save." << endl;
}

//------------------------------------------------------------------------
void cDedicatedServer::setProperty (const string& property, const string& value)
{
	if (property.compare ("port") == 0)
	{
		int newPort = atoi (value.c_str());
		if (newPort < 0 || newPort >= 65536)
			newPort = DEFAULTPORT;
		configuration->port = newPort;
		cout << "Port set to: " << newPort << endl;
	}
}

//------------------------------------------------------------------------
void cDedicatedServer::printConfiguration() const
{
	cout << "--- Server Configuration ---" << endl;
	cout << "port: " << configuration->port << endl;
}

//------------------------------------------------------------------------
void cDedicatedServer::printPrompt() const
{
	cout << "> ";
}

//------------------------------------------------------------------------
void cDedicatedServer::printGames() const
{
	cout << getGamesString();
}

//------------------------------------------------------------------------
string cDedicatedServer::getGamesString() const
{
	stringstream oss;
	if (games.empty())
		oss << "No games started" << endl;
	for (size_t i = 0; i < games.size(); i++)
	{
		oss << "--------- Game " << i << ": -----------" << endl;
		cServerGame* game = games[i];
		oss << game->getGameState();
	}
	return oss.str();
}

//------------------------------------------------------------------------
void cDedicatedServer::printMaps() const
{
	cout << getAvailableMapsString();
}

//------------------------------------------------------------------------
string cDedicatedServer::getAvailableMapsString() const
{
	stringstream oss;
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
	oss << "----- Available maps: ------" << endl;
	for (size_t i = 0; i != maps.size(); ++i)
	{
		string mapFilename = maps[i];
		if (mapFilename.compare (mapFilename.length() - 3, 3, "WRL") == 0
			|| mapFilename.compare (mapFilename.length() - 3, 3, "wrl") == 0)
		{
			oss << mapFilename << endl;
		}
	}
	return oss.str();
}

//------------------------------------------------------------------------
string cDedicatedServer::getServerHelpString() const
{
	stringstream oss;
	oss << "--- Dedicated server help ---" << endl;
	oss << "Type --server and then one of the following:" << endl;
	oss << "go : starts the game" << endl;
	oss << "games : shows the running games and their players" << endl;
	oss << "maps : shows the available maps" << endl;
	oss << "map mapname.wrl : changes the map" << endl;
	oss << "credits 0 | 50 | 100 | 150 | 200 | 250 : changes the starting credits" << endl;
	oss << "oil | gold | metal  low | normal | much | most : resource density" << endl;
	oss << "res sparse | normal | dense | most : changes the resource frequency" << endl;
	return oss.str();
}

//------------------------------------------------------------------------
void cDedicatedServer::printHelp (eHelpCommands helpCommand) const
{
	switch (helpCommand)
	{
		case kHelpUnknownCommand:
			cout << "Unknown command." << endl;
			break;
		case kNotImplementedYet:
			cout << "Not implemented yet." << endl;
			break;
		case kHelpHelp:
			cout << "Type \"help\" for help." << endl;
			break;
		case kHelpWrongArguments:
			cout << "Wrong number or wrong formated arguments" << endl;
			break;
		case kHelpGeneral:
			cout << "Available commands: newGame loadGame saveGame stop games maps set printconfig exit" << endl;
			cout << "Type \"help command\" to see help to a sepecific command" << endl;
			break;
		case kHelpNewGame:
			cout << "Starts the server and allows players to open games and connect to the server." << endl;
			break;
		case kHelpLoadGame:
			cout << "Starts the server, loads a savegame and allows players to reconnect (resync) to the server." << endl;
			cout << "Type: \"loadGame x\" to load savegame x (x is the number of the savegame slot; 11 is autosave)." << endl;
			break;
		case kHelpSaveGame:
			cout << "Save the current game to the given savegame slot." << endl;
			cout << "Type: \"saveGame x\" to save the game to slot x (x is a number >= 0)." << endl;
			break;
		case kHelpStop:
			cout << "Stops the server and all running games." << endl;
			break;
		case kHelpGames:
			cout << "Prints the list of currently running games." << endl;
			break;
		case kHelpMaps:
			cout << "Prints the names of the available maps." << endl;
			break;
		case kHelpSet:
			cout << "Sets a configuration property of the server." << endl << "For some properties, the server must not run already." << endl;
			cout << "Type \"set property-name value\". Available properties:" << endl;
			cout << " port  - the tcp port, on which the server listens." << endl;
			break;
		case kHelpPrintConfig:
			cout << "Prints the configuration of the server to the console" << endl;
			break;
		case kHelpExit:
			cout << "Quits without bothering to stop a running server before." << endl << "You should call stop before." << endl;
			break;
		default:
			cout << "Error in printHelp: no help page implemented. Please contact developers." << endl;
			break;
	}
}

//------------------------------------------------------------------------
void cDedicatedServer::pushEvent (cNetMessage* message)
{
	if (handleDedicatedServerEvents (message))
		return;
	// TODO: delegate to correct game (and not simply first game)
	if (games.empty() == false)
		games[0]->pushEvent (message);
}

//------------------------------------------------------------------------
bool cDedicatedServer::handleDedicatedServerEvents (cNetMessage* message)
{
	switch (message->getType())
	{
		case GAME_EV_CHAT_CLIENT:
		case MU_MSG_CHAT:
		{
			if (message->getType() == MU_MSG_CHAT)
				message->popBool();
			string chatText = message->popString();
			message->rewind();
			int senderSocket = -1;
			if (games.empty() == false && message->iPlayerNr >= 0)
				senderSocket = games[0]->getSocketForPlayerNr (message->iPlayerNr);
			if (senderSocket < 0)
				return false;

			size_t serverStringPos = chatText.find ("--server");
			if (serverStringPos != string::npos && chatText.length() > serverStringPos + 9)
			{
				string command = chatText.substr (serverStringPos + 9);
				vector<string> tokens;
				istringstream iss (command);
				copy (istream_iterator<string> (iss), istream_iterator<string>(), back_inserter<vector<string> > (tokens));
				if (tokens.size() == 1)
				{
					if (tokens[0].compare ("games") == 0)
					{
						sendChatMessage (getGamesString(),
										 message->getType() == MU_MSG_CHAT ? (int) MU_MSG_CHAT : (int) GAME_EV_CHAT_SERVER,
										 senderSocket);
						return true;
					}
					else if (tokens[0].compare ("maps") == 0)
					{
						sendChatMessage (getAvailableMapsString(), MU_MSG_CHAT, senderSocket);
						return true;
					}
					else if (tokens[0].compare ("help") == 0)
					{
						sendChatMessage (getServerHelpString(),
										 message->getType() == MU_MSG_CHAT ? (int) MU_MSG_CHAT : (int) GAME_EV_CHAT_SERVER,
										 senderSocket);
						return true;
					}
				}
			}
			break;
		}
		default: return false;
	}
	return false;
}

//------------------------------------------------------------------------
void cDedicatedServer::sendChatMessage (const string& text, int type, int socket)
{
	stringstream ss (text);
	string line;
	while (getline (ss, line))
	{
		cNetMessage* msg = new cNetMessage (type);
		if (msg->getType() == GAME_EV_CHAT_SERVER)
		{
			msg->pushString ("");
			msg->pushString (line);
			msg->pushChar (SERVER_INFO_MESSAGE);
		}
		else
		{
			msg->pushString (line);
			msg->pushBool (false);
		}
		msg->iPlayerNr = -1;
		if (socket < 0)
			network->send (msg->iLength, msg->serialize());
		else
			network->sendTo (socket, msg->iLength, msg->serialize());
	}
}

//------------------------------------------------------------------------
void cDedicatedServer::doAutoSave (cServer& server)
{
	cSavegame Savegame (kAutoSaveSlot);	// dedicated server autosaves are always in slot kAutoSaveSlot
	Savegame.save (server, "Dedicated Server Autosave");
}
