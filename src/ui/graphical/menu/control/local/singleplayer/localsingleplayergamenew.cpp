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

#include "localsingleplayergamenew.h"

#include "game/data/gamesettings.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/startup/lobbypreparationdata.h"
#include "settings.h"
#include "ui/graphical/intro.h"
#include "ui/widgets/application.h"

//------------------------------------------------------------------------------
cLocalSingleplayerGameNew::cLocalSingleplayerGameNew() :
	lobbyServer (connectionManager),
	lobbyClient (connectionManager, cPlayerBasicData::fromSettings())
{
	lobbyClient.connectToLocalServer (lobbyServer);
	run();
}

//------------------------------------------------------------------------------
void cLocalSingleplayerGameNew::run()
{
	if (client)
	{
		client->run();
	}
	else
	{
		// communication is async as for network
		// but network game "succeeds" as there are delay to click
		// between "consecutive" messages.

		// TODO: Simplify async communication:
		// remove the need of `run()` used before/after sending message
		lobbyClient.run();
		lobbyServer.run();
		lobbyClient.run();
		lobbyServer.run();
		lobbyClient.run();
		lobbyServer.run();
		lobbyClient.run();
		lobbyServer.run();
	}
}

//------------------------------------------------------------------------------
void cLocalSingleplayerGameNew::runGamePreparation (cApplication& application)
{
	run();
	lobbyClient.tryToSwitchReadyState();
	run();
	lobbyClient.askToFinishLobby();
	run();

	initGamePreparation = std::make_unique<cInitGamePreparation> (application, lobbyClient);

	signalConnectionManager.connect (lobbyServer.onStartNewGame, [&, this] (cServer& server) {
		initGamePreparation->close();
		start (application, server);
	});

	initGamePreparation->bindConnections (lobbyClient);

	application.addRunnable (shared_from_this());

	initGamePreparation->startGamePreparation();
}

//------------------------------------------------------------------------------
void cLocalSingleplayerGameNew::start (cApplication& application, cServer& server)
{
	if (cSettings::getInstance().shouldShowIntro())
	{
		showBeginGameScene();
	}

	client = std::make_shared<cClient> (connectionManager);
	const auto& lobbyPreparation = lobbyClient.getLobbyPreparationData();
	client->setPreparationData (lobbyPreparation);

	auto player = createPlayer();
	std::vector<cPlayerBasicData> players;
	players.push_back (player);
	client->setPlayers (players, 0);

	connectionManager->setLocalClient (client.get(), 0);

	server.start();
	auto initPlayerData = initGamePreparation->getInitPlayerData();
	client->initNewGame (initPlayerData);

	gameGuiController = std::make_unique<cGameGuiController> (application, lobbyPreparation.staticMap);

	gameGuiController->setSingleClient (client);
	gameGuiController->setServer (&server);

	cGameGuiState playerGameGuiState;
	playerGameGuiState.mapPosition = initPlayerData.landingPosition;
	gameGuiController->addPlayerGameGuiState (0, std::move (playerGameGuiState));

	gameGuiController->start();

	resetTerminating();

	signalConnectionManager.connect (gameGuiController->terminated, [this]() { exit(); });
}

//------------------------------------------------------------------------------
void cLocalSingleplayerGameNew::setGameSettings (const cGameSettings& gameSettings)
{
	lobbyClient.selectGameSettings (gameSettings);
	run();
}

//------------------------------------------------------------------------------
void cLocalSingleplayerGameNew::selectMapName (const std::string& mapName)
{
	lobbyClient.selectMapName (mapName);
	run();
}

//------------------------------------------------------------------------------
cPlayerBasicData cLocalSingleplayerGameNew::createPlayer()
{
	auto player = cPlayerBasicData::fromSettings();

	player.setNr (0);
	return player;
}
