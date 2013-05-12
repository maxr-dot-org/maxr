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
#include "netmessage.h"
#include "menuevents.h"
#include "serverevents.h"
#include "player.h"
#include "savegame.h"

// TODO: remove these dependencies:
#include "menus.h"
#include "menuitems.h"

#include <sstream>
#include <iterator>
#include <vector>
#include <algorithm>

using namespace std;

//-------------------------------------------------------------------------------
int serverGameThreadFunction (void* data)
{
	cServerGame* serverGame = reinterpret_cast<cServerGame*> (data);
	serverGame->run();
	return 0;
}

//------------------------------------------------------------------------
cServerGame::cServerGame(cTCP& network_)
	: network(&network_)
	, thread (0)
	, canceled (false)
	, shouldSave (false)
	, saveGameNumber (-1)
	, gameData (0)
	, serverMap (0)
{
}

//------------------------------------------------------------------------
cServerGame::~cServerGame()
{
	if (thread != 0)
	{
		canceled = true;
		SDL_WaitThread (thread, NULL);
		thread = 0;
	}

	for (size_t i = 0; i < menuPlayers.Size(); i++)
		delete menuPlayers[i];

	delete gameData;
	gameData = 0;
}

//-------------------------------------------------------------------------------
void cServerGame::runInThread()
{
	thread = SDL_CreateThread (serverGameThreadFunction, this);
}

//------------------------------------------------------------------------
bool cServerGame::loadGame (int saveGameNumber)
{
	cSavegame savegame (saveGameNumber);
	if (savegame.load (network) != 1)
		return false;
	if (Server != 0)
	{
		Server->markAllPlayersAsDisconnected();
		Server->bStarted = true;
	}
	return true;
}

//------------------------------------------------------------------------
void cServerGame::saveGame (int saveGameNumber)
{
	if (Server == 0)
	{
		cout << "Server not running. Can't save game." << endl;
		return;
	}
	cout << "Scheduling save command for server thread..." << endl;
	this->saveGameNumber = saveGameNumber;
	shouldSave = true;
}

//------------------------------------------------------------------------
void cServerGame::prepareGameData()
{
	gameData = new cGameDataContainer();
	gameData->type = GAME_TYPE_TCPIP;
	gameData->settings = new sSettings();
	gameData->settings->metal = SETTING_RESVAL_MUCH;
	gameData->settings->oil = SETTING_RESVAL_NORMAL;
	gameData->settings->gold = SETTING_RESVAL_NORMAL;
	gameData->settings->resFrequency = SETTING_RESFREQ_NORMAL;
	gameData->settings->credits = SETTING_CREDITS_NORMAL;
	gameData->settings->bridgeHead = SETTING_BRIDGEHEAD_DEFINITE;
	gameData->settings->alienTech = SETTING_ALIENTECH_OFF;
	gameData->settings->clans = SETTING_CLANS_ON;
	gameData->settings->gameType = SETTINGS_GAMETYPE_SIMU;
	gameData->settings->victoryType = SETTINGS_VICTORY_ANNIHILATION;
	gameData->settings->duration = SETTINGS_DUR_LONG;
	gameData->map = new cMap();
	string mapName = "Mushroom.wrl";
	gameData->map->LoadMap (mapName);
}

//------------------------------------------------------------------------
void cServerGame::run()
{
	while (canceled == false)
	{
		AutoPtr<cNetMessage>::type event (pollEvent());

		if (event)
		{
			if (Server != 0)
			{
				Server->HandleNetMessage (event);
				Server->checkPlayerUnits();
			}
			else
			{
				handleNetMessage (event);
			}
		}

		static unsigned int lastTime = 0;
		if (Server)
			lastTime = Server->gameTimer.gameTime;

		// don't do anything if games hasn't been started yet!
		if (Server && Server->bStarted)
		{
			Server->gameTimer.run (*Server);

			if (shouldSave)
			{
				cSavegame saveGame (saveGameNumber);
				saveGame.save (*Server, "Dedicated Server Savegame");
				cout << "...saved to slot " << saveGameNumber << endl;
				shouldSave = false;
			}
		}

		if (!event && (!Server || lastTime == Server->gameTimer.gameTime)) //nothing to do
			SDL_Delay (10);
	}
	if (Server)
		terminateServer();
}

//-------------------------------------------------------------------------------------
void cServerGame::handleNetMessage (cNetMessage* message)
{
	cout << "Msg received: " << message->getTypeAsString() << endl;

	switch (message->iType)
	{
			// TODO: reduce/avoid duplicate code with cNetwork(Host)Menu
		case TCP_ACCEPT:
		{
			sMenuPlayer* player = new sMenuPlayer ("unidentified", 0, false, menuPlayers.Size(), message->popInt16());
			menuPlayers.Add (player);
			sendMenuChatMessage (*network, "type --server help for dedicated server help", player);
			sendRequestIdentification (*network, player);
			break;
		}
		case TCP_CLOSE: // TODO: this is only ok in cNetwork(Host)Menu. when server runs already, it must be treated another way
		{
			int socket = message->popInt16();
			network->close (socket);
			string playerName;

			// delete menuPlayer
			for (size_t i = 0; i < menuPlayers.Size(); i++)
			{
				if (menuPlayers[i]->socket == socket)
				{
					playerName = menuPlayers[i]->name;
					menuPlayers.Delete (i);
				}
			}

			// resort socket numbers
			for (size_t playerNr = 0; playerNr < menuPlayers.Size(); playerNr++)
			{
				if (menuPlayers[playerNr]->socket > socket && menuPlayers[playerNr]->socket < MAX_CLIENTS)
					menuPlayers[playerNr]->socket--;
			}

			// resort player numbers
			for (size_t i = 0; i < menuPlayers.Size(); i++)
			{
				menuPlayers[i]->nr = i;
				sendRequestIdentification (*network, menuPlayers[i]);
			}
			sendPlayerList (*network, &menuPlayers);

			break;
		}
		case MU_MSG_IDENTIFIKATION:
		{
			unsigned int playerNr = message->popInt16();
			if (playerNr >= menuPlayers.Size())
				break;
			sMenuPlayer* player = menuPlayers[playerNr];

			//bool freshJoined = (player->name.compare ("unidentified") == 0);
			player->color = message->popInt16();
			player->name = message->popString();
			player->ready = message->popBool();

			Log.write ("game version of client " + iToStr (playerNr) + " is: " + message->popString(), cLog::eLOG_TYPE_NET_DEBUG);

			//if (freshJoined)
			//	chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Player_Joined", player->name)); // TODO: instead send a chat message to all players?

			// search double taken name or color
			//checkTakenPlayerAttr( player );

			sendPlayerList (*network, &menuPlayers);
			//sendGameData (*network, gameData, saveGameString, player);
			sendGameData (*network, gameData, "", player);

			break;
		}
		case MU_MSG_CHAT:
		{
			bool translationText = message->popBool();
			string chatText = message->popString();

			unsigned int senderPlyerNr = message->iPlayerNr;
			if (senderPlyerNr >= menuPlayers.Size())
				break;
			sMenuPlayer* senderPlayer = menuPlayers[senderPlyerNr];


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
						for (size_t i = 0; i < menuPlayers.Size(); i++)
						{
							if (menuPlayers[i]->ready == false)
							{
								allReady = false;
								break;
							}
						}
						if (allReady)
						{
							for (size_t i = 0; i < menuPlayers.Size(); i++)
							{
								cPlayer* player = new cPlayer (menuPlayers[i]->name, OtherData.colors[menuPlayers[i]->color], menuPlayers[i]->nr, menuPlayers[i]->socket);
								gameData->players.Add (player);
							}

							sendGo (*network);
						}
						else
							sendMenuChatMessage (*network, "Not all players are ready...", senderPlayer);
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
						if (gameData->map != 0 && gameData->map->LoadMap (mapName))
						{
							sendGameData (*network, gameData, "");
							string reply = senderPlayer->name;
							reply += " changed the map.";
							sendMenuChatMessage (*network, reply);
						}
						else
						{
							string reply = "Could not load map ";
							reply += mapName;
							sendMenuChatMessage (*network, reply, senderPlayer);
						}
					}
					if (tokens.size() == 2)
					{
						if (tokens[0].compare ("credits") == 0)
						{
							int credits = atoi (tokens[1].c_str());
							if (credits != SETTING_CREDITS_LOWEST
								&& credits != SETTING_CREDITS_LOWER
								&& credits != SETTING_CREDITS_LOW
								&& credits != SETTING_CREDITS_NORMAL
								&& credits != SETTING_CREDITS_MUCH
								&& credits != SETTING_CREDITS_MORE)
							{
								sendMenuChatMessage (*network, "Credits must be one of: 0 50 100 150 200 250", senderPlayer);
							}
							else
							{
								gameData->settings->credits = (eSettingsCredits) credits;
								sendGameData (*network, gameData, "");
								string reply = senderPlayer->name;
								reply += " changed the starting credits.";
								sendMenuChatMessage (*network, reply);
							}
						}
						else if (tokens[0].compare ("oil") == 0 || tokens[0].compare ("gold") == 0 || tokens[0].compare ("metal") == 0
								 || tokens[0].compare ("res") == 0)
							configRessources (tokens, senderPlayer);
					}
				}
			}
			else
			{
				// send to other clients
				for (size_t i = 0; i < menuPlayers.Size(); i++)
				{
					if (menuPlayers[i]->nr == message->iPlayerNr)
						continue;
					sendMenuChatMessage (*network, chatText, menuPlayers[i], -1, translationText);
				}
			}
			break;
		}
		case MU_MSG_CLAN:
			gameData->receiveClan (message);
			break;
		case MU_MSG_LANDING_VEHICLES:
			gameData->receiveLandingUnits (message);
			break;
		case MU_MSG_UPGRADES:
			gameData->receiveUnitUpgrades (message);
			break;
		case MU_MSG_LANDING_COORDS:
			gameData->receiveLandingPosition (*network, message);
			if (gameData->allLanded)
				startGameServer();
			break;
			//		case MU_MSG_ALL_LANDED:
			//			break;
	}
}

//-------------------------------------------------------------------------------------
void cServerGame::configRessources (vector<string>& tokens, sMenuPlayer* senderPlayer)
{
	if (tokens[0].compare ("res") == 0)
	{
		int density = -1;
		if (tokens[1].compare ("sparse") == 0) density = SETTING_RESFREQ_THIN;
		else if (tokens[1].compare ("normal") == 0) density = SETTING_RESFREQ_NORMAL;
		else if (tokens[1].compare ("dense") == 0) density = SETTING_RESFREQ_THICK;
		else if (tokens[1].compare ("most") == 0) density = SETTING_RESFREQ_MOST;
		if (density != -1)
		{
			gameData->settings->resFrequency = (eSettingResFrequency) density;
			sendGameData (*network, gameData, "");
			string reply = senderPlayer->name;
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
		int ammount = -1;
		if (tokens[1].compare ("low") == 0) ammount = SETTING_RESVAL_LOW;
		else if (tokens[1].compare ("normal") == 0) ammount = SETTING_RESVAL_NORMAL;
		else if (tokens[1].compare ("much") == 0) ammount = SETTING_RESVAL_MUCH;
		else if (tokens[1].compare ("most") == 0) ammount = SETTING_RESVAL_MOST;
		if (ammount != -1)
		{
			if (tokens[0].compare ("oil") == 0) gameData->settings->oil = (eSettingResourceValue) ammount;
			else if (tokens[0].compare ("metal") == 0) gameData->settings->metal = (eSettingResourceValue) ammount;
			else if (tokens[0].compare ("gold") == 0) gameData->settings->gold = (eSettingResourceValue) ammount;
			sendGameData (*network, gameData, "");
			string reply = senderPlayer->name;
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

//-------------------------------------------------------------------------------------
void cServerGame::startGameServer()
{
	serverMap = new cMap();

	// copy map for server
	serverMap->NewMap (gameData->map->size, gameData->map->iNumberOfTerrains);
	serverMap->MapName = gameData->map->MapName;
	memcpy (serverMap->Kacheln, gameData->map->Kacheln, sizeof (int) * gameData->map->size * gameData->map->size);
	for (int i = 0; i < gameData->map->iNumberOfTerrains; i++)
	{
		serverMap->terrain[i].blocked = gameData->map->terrain[i].blocked;
		serverMap->terrain[i].coast = gameData->map->terrain[i].coast;
		serverMap->terrain[i].water = gameData->map->terrain[i].water;
	}

	// copy playerlist for server
	for (unsigned int i = 0; i < gameData->players.Size(); i++)
	{
		serverPlayers.Add (new cPlayer (* (gameData->players[i])));
		serverPlayers[i]->InitMaps (serverMap->size, serverMap);
	}

	// init server
	int nTurns = 0;
	int nScore = 0;
	switch (gameData->settings->victoryType)
	{
		case SETTINGS_VICTORY_TURNS:
			nTurns = gameData->settings->duration;
			nScore = 0;
			break;
		case SETTINGS_VICTORY_POINTS:
			nScore = gameData->settings->duration;
			nTurns = 0;
			break;
		case SETTINGS_VICTORY_ANNIHILATION:
			nTurns = 0;
			nScore = 0;
			break;
		default:
			assert (0);
	}
	Server = new cServer (network, serverMap, &serverPlayers, gameData->type, gameData->settings->gameType == SETTINGS_GAMETYPE_TURNS, nTurns, nScore);

	// send victory conditions to clients
	for (unsigned n = 0; n < gameData->players.Size(); n++)
		sendVictoryConditions (*Server, nTurns, nScore, gameData->players[n]);

	// place resources
	for (unsigned int i = 0; i < gameData->players.Size(); i++)
	{
		Server->correctLandingPos (gameData->landData[i]->iLandX, gameData->landData[i]->iLandY);
		serverMap->placeRessourcesAddPlayer (gameData->landData[i]->iLandX, gameData->landData[i]->iLandY, gameData->settings->resFrequency);
	}
	serverMap->placeRessources (gameData->settings->metal, gameData->settings->oil, gameData->settings->gold);


	// send clan info to clients
	if (gameData->settings->clans == SETTING_CLANS_ON)
		sendClansToClients (*Server, &gameData->players);

	// make the landing
	for (unsigned int i = 0; i < gameData->players.Size(); i++)
	{
		Server->makeLanding (gameData->landData[i]->iLandX, gameData->landData[i]->iLandY,
							 serverPlayers[i], gameData->landingUnits[i], gameData->settings->bridgeHead == SETTING_BRIDGEHEAD_DEFINITE);
		delete gameData->landData[i];
		delete gameData->landingUnits[i];
	}

	Server->bStarted = true;
}

//-------------------------------------------------------------------------------------
void cServerGame::terminateServer()
{
	if (gameData != 0)
	{
		for (size_t i = 0; i != gameData->players.Size(); ++i)
		{
			delete gameData->players[i];
		}
		gameData->players.Clear();
	}

	delete serverMap;
	serverMap = NULL;

	delete Server;
	Server = NULL;
}

//-------------------------------------------------------------------------------------
cNetMessage* cServerGame::pollEvent()
{
	if (eventQueue.size() <= 0)
		return NULL;
	return eventQueue.read();
}

//------------------------------------------------------------------------
void cServerGame::pushEvent (cNetMessage* message)
{
	eventQueue.write (message);
}

//------------------------------------------------------------------------
std::string cServerGame::getGameState() const
{
	std::stringstream result;
	result << "GameState: ";
	if (Server != 0)
		result << "Game is active" << endl;
	else if (gameData != 0 && gameData->players.Size() == 0)
		result << "Game is open for new players" << endl;
	else
		result << "Game has started, players are setting up" << endl;

	result << "Map: " << (gameData != 0 ? gameData->map->MapName : "none") << endl;
	if (Server != 0)
		result << "Turn: " << Server->getTurn() << endl;

	result << "Players:" << endl;
	if (Server != 0 && serverPlayers.Size() > 0)
	{
		for (size_t i = 0; i < serverPlayers.Size(); i++)
		{
			cPlayer* player = serverPlayers[i];
			result << " " << player->name << (Server->isPlayerDisconnected (player) ? " (disconnected)" : " (online)") << endl;
		}
	}
	else if (gameData->players.Size() > 0)
	{
		for (size_t i = 0; i < gameData->players.Size(); i++)
			result << " " << gameData->players[i]->name << endl;
	}
	else if (menuPlayers.Size() > 0)
	{
		for (size_t i = 0; i < menuPlayers.Size(); i++)
			result << " " << menuPlayers[i]->name << endl;
	}
	return result.str();
}

//------------------------------------------------------------------------
int cServerGame::getSocketForPlayerNr (int playerNr)
{
	if (Server != 0)
	{
		cPlayer* player = Server->getPlayerFromNumber (playerNr);
		if (player != 0)
			return player->iSocketNum;
	}
	for (size_t i = 0; i < menuPlayers.Size(); i++)
	{
		if (menuPlayers[i]->nr == playerNr)
			return menuPlayers[i]->socket;
	}
	return -1;
}
