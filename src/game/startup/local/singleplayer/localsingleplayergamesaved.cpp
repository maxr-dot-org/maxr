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

#include "game/startup/local/singleplayer/localsingleplayergamesaved.h"
#include "game/data/gamesettings.h"
#include "ui/graphical/application.h"
#include "game/logic/client.h"
#include "game/logic/server2.h"
#include "game/data/player/player.h"
#include "game/logic/clientevents.h"
#include "game/data/savegame.h"
#include "game/data/report/savedreport.h"

//------------------------------------------------------------------------------
void cLocalSingleplayerGameSaved::start (cApplication& application)
{
	auto connectionManager = std::make_shared<cConnectionManager>();

	//set up server
	server = std::make_unique<cServer2>(connectionManager);
	connectionManager->setLocalServer(server.get());
	server->loadGameState(saveGameNumber);

	//setup client
	int gameId = server->getModel().getGameId();
	client = std::make_shared<cClient> (connectionManager, gameId);
	auto staticMap = server->getModel().getMap()->staticMap;
	client->setMap(staticMap);
	client->loadModel(saveGameNumber); //TODO: resync model from server

	//use player 0 as local player
	auto localPlayer = client->getModel().getPlayerList()[0];
	client->setActivePlayer(localPlayer.get());
	connectionManager->setLocalClient(client.get(), 0);

	server->sendGuiInfoToClients(saveGameNumber);

	server->start();

	gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);

	gameGuiController->setSingleClient (client);
	gameGuiController->start();

	terminate = false;

	application.addRunnable (shared_from_this());

	signalConnectionManager.connect (gameGuiController->terminated, [&]() { terminate = true; });
}

void cLocalSingleplayerGameSaved::setSaveGameNumber (int saveGameNumber_)
{
	saveGameNumber = saveGameNumber_;
}
