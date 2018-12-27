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

#include "game/startup/local/hotseat/localhotseatgamesaved.h"
#include "game/data/gamesettings.h"
#include "ui/graphical/application.h"
#include "game/logic/client.h"
#include "game/logic/server2.h"
#include "game/data/player/player.h"
#include "game/data/savegame.h"
#include "game/data/report/savedreport.h"

//------------------------------------------------------------------------------
void cLocalHotSeatGameSaved::start (cApplication& application)
{
	//TODO: new server
	//server = std::make_unique<cServer> (nullptr);

	//cSavegame savegame (saveGameNumber);
	//if (savegame.load (*server) == false) return;

	//auto staticMap = server->Map->staticMap;

	//const auto& serverPlayerList = server->playerList;
	//if (serverPlayerList.empty()) return;

	// Following may be simplified according to serverGame::loadGame
	std::vector<cPlayerBasicData> clientPlayerList;

	// copy players for client
	//for (size_t i = 0; i != serverPlayerList.size(); ++i)
	{
		//const auto& p = *serverPlayerList[i];
		//clientPlayerList.push_back (cPlayerBasicData (p.getName(), p.getColor(), p.getId(), p.getSocketNum()));

		//serverPlayerList[i]->setLocal();
	}

	clients.resize (clientPlayerList.size());
	for (size_t i = 0; i < clientPlayerList.size(); ++i)
	{
		//clients[i] = std::make_shared<cClient> (server.get(), nullptr); //TODO
//		clients[i]->setMap (staticMap);
//		clients[i]->setGameSettings (*server->getGameSettings());
		clients[i]->setPlayers (clientPlayerList, i);
	}

	server->start();

	for (size_t i = 0; i < clients.size(); ++i)
	{
		//sendRequestResync (*clients[i], clients[i]->getActivePlayer().getId(), true);
	}

	// TODO: move that in server
//	for (size_t i = 0; i != serverPlayerList.size(); ++i)
	{
//		sendGameSettings (*server, *serverPlayerList[i]);
//		sendGameGuiState (*server, server->getPlayerGameGuiState (*serverPlayerList[i]), *serverPlayerList[i]);
		/*auto& reportList = serverPlayerList[i]->savedReportsList;
		for (size_t j = 0; j != reportList.size(); ++j)
		{
			sendSavedReport (*server, *reportList[j], serverPlayerList[i].get());
		}
		reportList.clear();
		*/
	}

	// start game
	//server->serverState = SERVER_STATE_INGAME;

	// TODO: save/load game time
	//server->startTurnTimers();

	//gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);

	//auto activePlayer = server->getActiveTurnPlayer();
	//if (activePlayer == nullptr) activePlayer = server->playerList[0].get();

	//gameGuiController->setClients (clients, activePlayer->getId());

	gameGuiController->start();

	//terminate = false;

	application.addRunnable (shared_from_this());

	//signalConnectionManager.connect (gameGuiController->terminated, [&]() { terminate = true; });
}

void cLocalHotSeatGameSaved::setSaveGameNumber (int saveGameNumber_)
{
	saveGameNumber = saveGameNumber_;
}
