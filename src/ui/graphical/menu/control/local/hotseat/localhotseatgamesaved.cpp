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

#include "localhotseatgamesaved.h"

#include "game/data/gamesettings.h"
#include "game/data/player/player.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "ui/widgets/application.h"

//------------------------------------------------------------------------------
void cLocalHotSeatGameSaved::start (cApplication& application)
{
	auto connectionManager = std::make_shared<cConnectionManager>();

	server = std::make_unique<cServer> (connectionManager);
	connectionManager->setLocalServer (server.get());
	server->loadGameState (saveGameNumber);

	const auto& staticMap = server->getModel().getMap()->staticMap;
	const std::size_t nbPlayers = server->getModel().getPlayerList().size();
	clients.resize (nbPlayers);
	for (std::size_t i = 0; i != nbPlayers; ++i)
	{
		clients[i] = std::make_shared<cClient> (connectionManager);
		clients[i]->setMap (staticMap);
		clients[i]->loadModel (saveGameNumber, i); //TODO: resync model from server
	}

	std::vector<INetMessageReceiver*> hotseatClients = ranges::Transform (clients, [](auto& client) -> INetMessageReceiver* { return client.get(); });
	connectionManager->setLocalClients (std::move (hotseatClients));

	server->sendGuiInfoToClients (saveGameNumber);
	server->start();

	gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);
	gameGuiController->setClients (clients, server->getModel().getActiveTurnPlayer()->getId());
	gameGuiController->setServer (server.get());
	gameGuiController->start();

	resetTerminating();

	application.addRunnable (shared_from_this());

	signalConnectionManager.connect (gameGuiController->terminated, [this]() { exit(); });
}

void cLocalHotSeatGameSaved::setSaveGameNumber (int saveGameNumber_)
{
	saveGameNumber = saveGameNumber_;
}
