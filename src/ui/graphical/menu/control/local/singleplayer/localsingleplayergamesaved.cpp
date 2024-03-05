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

#include "localsingleplayergamesaved.h"

#include "game/data/gamesettings.h"
#include "game/data/player/player.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "settings.h"
#include "ui/graphical/intro.h"
#include "ui/widgets/application.h"

//------------------------------------------------------------------------------
void cLocalSingleplayerGameSaved::start (cApplication& application)
{
	if (cSettings::getInstance().shouldShowIntro())
	{
		showBeginGameScene();
	}

	auto connectionManager = std::make_shared<cConnectionManager>();

	//set up server
	server = std::make_unique<cServer> (connectionManager);
	connectionManager->setLocalServer (server.get());
	server->loadGameState (saveGameNumber);

	//setup client
	client = std::make_shared<cClient> (connectionManager);
	auto staticMap = server->getModel().getMap()->staticMap;
	client->setMap (staticMap);
	//use player 0 as local player
	client->loadModel (saveGameNumber, 0); //TODO: resync model from server
	connectionManager->setLocalClient (client.get(), 0);

	server->sendGuiInfoToClients (saveGameNumber);
	server->start();

	gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);
	gameGuiController->setSingleClient (client);
	gameGuiController->setServer (server.get());
	gameGuiController->start();

	resetTerminating();

	application.addRunnable (shared_from_this());

	signalConnectionManager.connect (gameGuiController->terminated, [this]() { exit(); });
}
