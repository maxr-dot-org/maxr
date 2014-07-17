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

#include "log.h"
#include "menuevents.h"
#include "netmessage.h"
#include "game/data/player/player.h"
#include "savegame.h"
#include "serverevents.h"
#include "map.h"
#include "server.h"
#include "game/logic/turnclock.h"

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
cServerGame::cServerGame (std::shared_ptr<cTCP> network_) :
	server (NULL),
	network (std::move(network_)),
	thread (NULL),
	canceled (false),
	shouldSave (false),
	saveGameNumber (-1)
{
}

//------------------------------------------------------------------------------
cServerGame::~cServerGame()
{
	if (thread != 0)
	{
		canceled = true;
		SDL_WaitThread (thread, NULL);
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
	cSavegame savegame (saveGameNumber);
	server = new cServer (network);
	if (savegame.load (*server) == false)
		return false;
	server->markAllPlayersAsDisconnected();
	server->serverState = SERVER_STATE_INGAME;
	return true;
}

//------------------------------------------------------------------------------
void cServerGame::saveGame (int saveGameNumber)
{
	if (server == NULL)
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
	settings.setMetalAmount(eGameSettingsResourceAmount::Normal);
	settings.setOilAmount (eGameSettingsResourceAmount::Normal);
	settings.setGoldAmount (eGameSettingsResourceAmount::Normal);
	settings.setResourceDensity(eGameSettingsResourceDensity::Normal);
	settings.setStartCredits (cGameSettings::defaultCreditsNormal);
	settings.setBridgeheadType(eGameSettingsBridgeheadType::Definite);
	//settings.alienTech = SETTING_ALIENTECH_OFF;
	settings.setClansEnabled(true);
	settings.setGameType(eGameSettingsGameType::Simultaneous);
	settings.setVictoryCondition(eGameSettingsVictoryCondition::Death);
	map = std::make_shared<cStaticMap> ();
	const std::string mapName = "Mushroom.wrl";
	map->loadMap (mapName);
}

//------------------------------------------------------------------------------
void cServerGame::run()
{
	while (canceled == false)
	{
		std::unique_ptr<cNetMessage> message;
		if(eventQueue.try_pop (message))
		{
			if (server != NULL)
			{
				server->handleNetMessage (*message);
				server->checkPlayerUnits();
			}
			else
			{
				handleNetMessage (*message);
			}
		}

		static unsigned int lastTime = 0;
		if (server)
			lastTime = server->gameTimer->gameTime;

		// don't do anything if games hasn't been started yet!
		if (server && server->serverState == SERVER_STATE_INGAME)
		{
			server->gameTimer->run (*server);

			if (shouldSave)
			{
				cSavegame saveGame (saveGameNumber);
				saveGame.save (*server, "Dedicated Server Savegame");
				cout << "...saved to slot " << saveGameNumber << endl;
				shouldSave = false;
			}
		}

		if (!message && (!server || lastTime == server->gameTimer->gameTime)) //nothing to do
			SDL_Delay (10);
	}
	if (server)
		terminateServer();
}

//------------------------------------------------------------------------------
void cServerGame::handleNetMessage_TCP_ACCEPT (cNetMessage& message)
{
	assert (message.iType == TCP_ACCEPT);

	auto player = std::make_shared<sPlayer> ("unidentified", cPlayerColor(0), menuPlayers.size(), message.popInt16());
	menuPlayers.push_back (player);
	sendMenuChatMessage (*network, "type --server help for dedicated server help", player.get());
	sendRequestIdentification (*network, *player);
}

//------------------------------------------------------------------------------
void cServerGame::handleNetMessage_TCP_CLOSE (cNetMessage& message)
{
	assert (message.iType == TCP_CLOSE);

	// TODO: this is only ok in cNetwork(Host)Menu.
	// when server runs already, it must be treated another way
	int socket = message.popInt16();
	network->close (socket);
	string playerName;

	// delete menuPlayer
	for (size_t i = 0; i < menuPlayers.size(); i++)
	{
		if (menuPlayers[i]->getSocketIndex() == socket)
		{
			playerName = menuPlayers[i]->getName();
			menuPlayers.erase (menuPlayers.begin() + i);
			break;
		}
	}

	// resort socket numbers
	for (size_t playerNr = 0; playerNr < menuPlayers.size(); playerNr++)
	{
		menuPlayers[playerNr]->onSocketIndexDisconnected (socket);
	}

	// resort player numbers
	for (size_t i = 0; i < menuPlayers.size(); i++)
	{
		menuPlayers[i]->setNr (i);
		sendRequestIdentification (*network, *menuPlayers[i]);
	}
	//sendPlayerList (*network, menuPlayers);
}

//------------------------------------------------------------------------------
void cServerGame::handleNetMessage_MU_MSG_IDENTIFIKATION (cNetMessage& message)
{
	assert (message.iType == MU_MSG_IDENTIFIKATION);

	unsigned int playerNr = message.popInt16();
	if (playerNr >= menuPlayers.size())
		return;
	const auto& player = menuPlayers[playerNr];

	//bool freshJoined = (player->name.compare ("unidentified") == 0);
	player->setColor (cPlayerColor(message.popInt16()));
	player->setName (message.popString());
	player->setReady (message.popBool());

	Log.write ("game version of client " + iToStr (playerNr) + " is: " + message.popString(), cLog::eLOG_TYPE_NET_DEBUG);

	//if (freshJoined)
	//	chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Player_Joined", player->name)); // TODO: instead send a chat message to all players?

	// search double taken name or color
	//checkTakenPlayerAttr (player);

	sendPlayerList (*network, menuPlayers);

	//sendGameData (*network, map.get(), settings, saveGameString, player);
	sendGameData (*network, map.get(), &settings, -1, player.get());
}

//------------------------------------------------------------------------------
void cServerGame::handleNetMessage_MU_MSG_CHAT (cNetMessage& message)
{
	assert (message.iType == MU_MSG_CHAT);

	bool translationText = message.popBool();
	string chatText = message.popString();

	unsigned int senderPlyerNr = message.iPlayerNr;
	if (senderPlyerNr >= menuPlayers.size())
		return;
	const auto& senderPlayer = menuPlayers[senderPlyerNr];

	// temporary workaround. TODO: good solution - player, who opened games must have "host" gui and new commands to send options/go to server
	size_t serverStringPos = chatText.find ("--server");
	if (serverStringPos != string::npos && chatText.length() > serverStringPos + 9)
	{
		string command = chatText.substr (serverStringPos + 9);
		vector<string> tokens;
		istringstream iss (command);
		copy (istream_iterator<string> (iss), istream_iterator<string> (), back_inserter<vector<string> > (tokens));
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
						break;
					}
				}
				if (allReady)
				{
					server = new cServer (network);
					// copy playerlist for server
					for (size_t i = 0; i != menuPlayers.size(); ++i)
					{
						server->addPlayer (std::make_unique<cPlayer> (*menuPlayers[i]));
					}

					server->setMap (map);
					server->setGameSettings (settings);
					//server->changeStateToInitGame();
					//sendGo (*server);
				}
				else
					sendMenuChatMessage (*network, "Not all players are ready...", senderPlayer.get());
			}
		}
		else if (tokens.size() >= 2)
		{
			if (tokens[0].compare ("map") == 0)
			{
				string mapName = tokens[1];
				for (size_t i = 2; i < tokens.size(); i++)
				{
					mapName += " ";
					mapName += tokens[i];
				}
				if (map != NULL && map->loadMap (mapName))
				{
					sendGameData (*network, map.get(), &settings, -1);
					string reply = senderPlayer->getName();
					reply += " changed the map.";
					sendMenuChatMessage (*network, reply);
				}
				else
				{
					string reply = "Could not load map ";
					reply += mapName;
					sendMenuChatMessage (*network, reply, senderPlayer.get());
				}
			}
			if (tokens.size() == 2)
			{
				if (tokens[0].compare ("credits") == 0)
				{
					int credits = atoi (tokens[1].c_str());
					settings.setStartCredits(credits);
					sendGameData (*network, map.get(), &settings, -1);
					string reply = senderPlayer->getName();
					reply += " changed the starting credits.";
					sendMenuChatMessage (*network, reply);
				}
				else if (tokens[0].compare ("oil") == 0 || tokens[0].compare ("gold") == 0 || tokens[0].compare ("metal") == 0
						 || tokens[0].compare ("res") == 0)
					configRessources (tokens, senderPlayer.get());
			}
		}
	}
	else
	{
		// send to other clients
		for (size_t i = 0; i < menuPlayers.size(); i++)
		{
			if (menuPlayers[i]->getNr() == message.iPlayerNr)
				continue;
			sendMenuChatMessage (*network, chatText, menuPlayers[i].get(), -1, translationText);
		}
	}
}

void cServerGame::handleNetMessage (cNetMessage& message)
{
	cout << "Msg received: " << message.getTypeAsString() << endl;

	// TODO: reduce/avoid duplicate code with cNetwork(Host)Menu
	switch (message.iType)
	{
		case TCP_ACCEPT: handleNetMessage_TCP_ACCEPT (message); break;
		case TCP_CLOSE: handleNetMessage_TCP_CLOSE (message); break;
		case MU_MSG_IDENTIFIKATION: handleNetMessage_MU_MSG_IDENTIFIKATION (message); break;
		case MU_MSG_CHAT: handleNetMessage_MU_MSG_CHAT (message); break;
		default: break;
	}
}

//------------------------------------------------------------------------------
void cServerGame::configRessources (vector<string>& tokens, sPlayer* senderPlayer)
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
			settings.setResourceDensity(density);
			sendGameData (*network, map.get(), &settings, -1);
			string reply = senderPlayer->getName();
			reply += " changed the resource frequency to ";
			reply += tokens[1];
			reply += ".";
			sendMenuChatMessage (*network, reply);
		}
		else
			sendMenuChatMessage (*network, "res must be one of: sparse normal dense most", senderPlayer);
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
			if (tokens[0].compare ("oil") == 0) settings.setOilAmount(amount);
			else if (tokens[0].compare ("metal") == 0) settings.setMetalAmount(amount);
			else if (tokens[0].compare ("gold") == 0) settings.setGoldAmount(amount);
			sendGameData (*network, map.get(), &settings, -1);
			string reply = senderPlayer->getName();
			reply += " changed the resource density of ";
			reply += tokens[0];
			reply += " to ";
			reply += tokens[1];
			reply += ".";
			sendMenuChatMessage (*network, reply);
		}
		else
			sendMenuChatMessage (*network, "oil|gold|metal must be one of: low normal much most", senderPlayer);
	}
}

//------------------------------------------------------------------------------
void cServerGame::terminateServer()
{
	delete server;
	server = NULL;
}

//------------------------------------------------------------------------------
void cServerGame::pushEvent (std::unique_ptr<cNetMessage> message)
{
	eventQueue.push (std::move(message));
}

//------------------------------------------------------------------------------
std::string cServerGame::getGameState() const
{
	std::stringstream result;
	result << "GameState: ";

	//if (server == NULL || server->serverState == SERVER_STATE_ROOM)
	//	result << "Game is open for new players" << endl;
	/*else*/ if (server->serverState == SERVER_STATE_INITGAME)
		result << "Game has started, players are setting up" << endl;
	else if (server->serverState == SERVER_STATE_INGAME)
		result << "Game is active" << endl;

	result << "Map: " << (map != NULL ? map->getName() : "none") << endl;
	if (server != NULL)
		result << "Turn: " << server->getTurnClock ()->getTurn () << endl;

	result << "Players:" << endl;
	if (server != NULL && server->playerList.empty() == false)
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
}

//------------------------------------------------------------------------------
int cServerGame::getSocketForPlayerNr (int playerNr) const
{
	if (server != NULL)
	{
		const cPlayer& player = server->getPlayerFromNumber (playerNr);
		return player.getSocketNum();
	}
	for (size_t i = 0; i < menuPlayers.size(); i++)
	{
		if (menuPlayers[i]->getNr() == playerNr)
			return menuPlayers[i]->getSocketIndex();
	}
	return -1;
}
