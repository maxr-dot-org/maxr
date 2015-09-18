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

#include "game/startup/network/host/networkhostgamesaved.h"
#include "game/data/gamesettings.h"
#include "ui/graphical/application.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/data/player/player.h"
#include "game/logic/clientevents.h"
#include "game/data/savegame.h"
#include "game/data/report/savedreport.h"

//------------------------------------------------------------------------------
void cNetworkHostGameSaved::start (cApplication& application)
{
	server = std::make_unique<cServer> (network);
	//localClient = std::make_shared<cClient> (server.get(), nullptr); //TODO: use new server

	//cSavegame savegame (saveGameNumber);
	//if (savegame.load (*server) == false) return; // TODO: error message

	auto staticMap = server->Map->staticMap;
//	localClient->setMap (staticMap);

	const auto& serverPlayerList = server->playerList;
	if (serverPlayerList.empty()) return;

	// Following may be simplified according to serverGame::loadGame
	size_t serverListLocalPlayerIndex = 0;
	// copy players for client
	for (size_t i = 0; i != serverPlayerList.size(); ++i)
	{
		auto& serverPlayer = *serverPlayerList[i];

		auto iter = std::find_if (players.begin(), players.end(), [&] (const cPlayerBasicData & player) { return player.getNr() == serverPlayer.getId(); });

		assert (iter != players.end());
		const auto& listPlayer = *iter;

		if (listPlayer.getNr() == players[localPlayerIndex].getNr())
		{
			serverPlayer.setLocal();
			serverListLocalPlayerIndex = i;
		}
		else
		{
			serverPlayer.setSocketIndex (listPlayer.getSocketIndex());
		}
	}
	localClient->setPlayers (players, serverListLocalPlayerIndex);

	/*if (server->getGameSettings()->getGameType() == eGameSettingsGameType::Turns)
	{
		sendWaitFor (*server, *server->getActiveTurnPlayer(), nullptr);
	}*/

	server->start();

	sendRequestResync (*localClient, serverPlayerList[serverListLocalPlayerIndex]->getId(), true);

	// TODO: move that in server
	for (size_t i = 0; i != serverPlayerList.size(); ++i)
	{
//		sendGameSettings (*server, *serverPlayerList[i]);
		sendGameGuiState (*server, server->getPlayerGameGuiState (*serverPlayerList[i]), *serverPlayerList[i]);
		auto& reportList = serverPlayerList[i]->savedReportsList;
		for (size_t j = 0; j != reportList.size(); ++j)
		{
			sendSavedReport (*server, *reportList[j], serverPlayerList[i].get());
		}
		reportList.clear();
	}

	// start game
	server->serverState = SERVER_STATE_INGAME;

	// TODO: save/load game time
	server->startTurnTimers();

	gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);

	gameGuiController->setSingleClient (localClient);

	gameGuiController->start();

	using namespace std::placeholders;
	signalConnectionManager.connect (gameGuiController->triggeredSave, std::bind (&cNetworkHostGameSaved::save, this, _1, _2));

	terminate = false;

	application.addRunnable (shared_from_this());

	signalConnectionManager.connect (gameGuiController->terminated, [&]() { terminate = true; });
}

//------------------------------------------------------------------------------
void cNetworkHostGameSaved::setSaveGameNumber (int saveGameNumber_)
{
	saveGameNumber = saveGameNumber_;
}

//------------------------------------------------------------------------------
void cNetworkHostGameSaved::setPlayers (std::vector<cPlayerBasicData> players_, const cPlayerBasicData& localPlayer)
{
	players = players_;
	auto localPlayerIter = std::find_if (players.begin(), players.end(), [&] (const cPlayerBasicData & player) { return player.getNr() == localPlayer.getNr(); });
	assert (localPlayerIter != players.end());
	localPlayerIndex = localPlayerIter - players.begin();
}

//------------------------------------------------------------------------------
const std::vector<cPlayerBasicData>& cNetworkHostGameSaved::getPlayers()
{
	return players;
}

//------------------------------------------------------------------------------
const cPlayerBasicData& cNetworkHostGameSaved::getLocalPlayer()
{
	return players[localPlayerIndex];
}
