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

#include "game/startup/local/singleplayer/localsingleplayergamenew.h"

#include "game/data/gamesettings.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/landingunit.h"
#include "game/data/units/vehicle.h"
#include "game/logic/action/actioninitnewgame.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/startup/lobbypreparationdata.h"
#include "ui/graphical/application.h"

//------------------------------------------------------------------------------
cLocalSingleplayerGameNew::cLocalSingleplayerGameNew() :
	playerClan (-1)
{}

//------------------------------------------------------------------------------
void cLocalSingleplayerGameNew::start (cApplication& application)
{
	assert (gameSettings != nullptr);
	auto connectionManager = std::make_shared<cConnectionManager>();

	server = std::make_unique<cServer>(connectionManager);
	server->setPreparationData ({unitsData, clanData, gameSettings, staticMap});

	client = std::make_shared<cClient>(connectionManager);
	client->setPreparationData({unitsData, clanData, gameSettings, staticMap});

	auto player = createPlayer();
	std::vector<cPlayerBasicData> players;
	players.push_back (player);
	client->setPlayers (players, 0);
	server->setPlayers(players);

	connectionManager->setLocalServer(server.get());
	connectionManager->setLocalClient(client.get(), 0);

	server->start();

	cActionInitNewGame action;
	action.clan = playerClan;
	action.landingUnits = landingUnits;
	action.landingPosition = landingPosition;
	action.unitUpgrades = unitUpgrades;
	client->sendNetMessage(action);

	gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);

	gameGuiController->setSingleClient (client);
	gameGuiController->setServer(server.get());

	cGameGuiState playerGameGuiState;
	playerGameGuiState.mapPosition = landingPosition;
	gameGuiController->addPlayerGameGuiState (0, std::move (playerGameGuiState));

	gameGuiController->start();

	resetTerminating();

	application.addRunnable (shared_from_this());

	signalConnectionManager.connect(gameGuiController->terminated, [&]() { exit(); });
}

//------------------------------------------------------------------------------
void cLocalSingleplayerGameNew::setGameSettings (std::shared_ptr<cGameSettings> gameSettings_)
{
	gameSettings = gameSettings_;
}

//------------------------------------------------------------------------------
void cLocalSingleplayerGameNew::setStaticMap (std::shared_ptr<cStaticMap> staticMap_)
{
	staticMap = staticMap_;
}

//------------------------------------------------------------------------------
void cLocalSingleplayerGameNew::setPlayerClan (int clan)
{
	playerClan = clan;
}

//------------------------------------------------------------------------------
void cLocalSingleplayerGameNew::setLandingUnits (std::vector<sLandingUnit> landingUnits_)
{
	landingUnits = std::move (landingUnits_);
}

//------------------------------------------------------------------------------
void cLocalSingleplayerGameNew::setUnitUpgrades (std::vector<std::pair<sID, cUnitUpgrade>> unitUpgrades_)
{
	unitUpgrades = std::move (unitUpgrades_);
}

//------------------------------------------------------------------------------
void cLocalSingleplayerGameNew::setLandingPosition (const cPosition& landingPosition_)
{
	landingPosition = landingPosition_;
}

//------------------------------------------------------------------------------
cPlayerBasicData cLocalSingleplayerGameNew::createPlayer()
{
	auto player = cPlayerBasicData::fromSettings();

	player.setNr (0);
	return player;
}
