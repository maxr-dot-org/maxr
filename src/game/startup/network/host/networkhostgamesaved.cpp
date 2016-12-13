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
#include "game/logic/server2.h"
#include "game/data/player/player.h"
#include "game/logic/clientevents.h"
#include "game/data/savegame.h"
#include "game/data/report/savedreport.h"

void cNetworkHostGameSaved::loadGameData()
{
	// cNetworkHostGameSaved::start() must not throw any errors,
	// because clients are already started. So try to load 
	// game data in this function, before initializing gameGui & clients

	server = std::make_unique<cServer2>(connectionManager);
	server->loadGameState(saveGameNumber);

}

//------------------------------------------------------------------------------
void cNetworkHostGameSaved::start (cApplication& application)
{
	// setup server
	connectionManager->setLocalServer(server.get());

	//setup client
	localClient = std::make_shared<cClient> (connectionManager);
	connectionManager->setLocalClient(localClient.get(), localPlayerNr);
	localClient->setPlayers(players, localPlayerNr);
	auto staticMap = server->getModel().getMap()->staticMap;
	localClient->setMap (staticMap);

	
	//TODO: turnbased mode
	//if (server->getGameSettings()->getGameType() == eGameSettingsGameType::Turns)
	//{
	//	sendWaitFor (*server, *server->getActiveTurnPlayer(), nullptr);
	//}

	localClient->sendNetMessage(cNetMessageRequestResync(-1, saveGameNumber));
	server->start();

	// TODO: implement timers
	//server->startTurnTimers();

	gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);

	gameGuiController->setSingleClient (localClient);
	gameGuiController->setServer(server.get());

	gameGuiController->start();

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
	localPlayerNr = localPlayer.getNr();
}

//------------------------------------------------------------------------------
const std::vector<cPlayerBasicData>& cNetworkHostGameSaved::getPlayers()
{
	return players;
}

//------------------------------------------------------------------------------
const cPlayerBasicData& cNetworkHostGameSaved::getLocalPlayer()
{
	return *std::find_if(players.begin(), players.end(), [&](const cPlayerBasicData& player) { return player.getNr() == localPlayerNr; });
}
