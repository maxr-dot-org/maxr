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
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/application.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/landingunit.h"
#include "game/logic/clientevents.h"

// TODO: find nice place
//------------------------------------------------------------------------------
void applyUnitUpgrades (cPlayer& player, const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades)
{
	for (size_t i = 0; i < unitUpgrades.size(); ++i)
	{
		const auto& unitId = unitUpgrades[i].first;

		auto unitData = player.getUnitDataCurrentVersion (unitId);

		if (unitData)
		{
			const auto& upgrades = unitUpgrades[i].second;
			upgrades.updateUnitData (*unitData);
		}
	}
}

//------------------------------------------------------------------------------
cLocalSingleplayerGameNew::cLocalSingleplayerGameNew() :
	playerClan (-1)
{}

//------------------------------------------------------------------------------
void cLocalSingleplayerGameNew::start (cApplication& application)
{
	assert (gameSettings != nullptr);

	server = std::make_unique<cServer> (nullptr);
	client = std::make_shared<cClient> (server.get(), nullptr);

	server->setMap (staticMap);
	client->setMap (staticMap);

	server->setGameSettings (*gameSettings);
	client->setGameSettings (*gameSettings);

	auto player = createPlayer();

	server->addPlayer (std::make_unique<cPlayer> (player));
	//server->changeStateToInitGame ();

	std::vector<cPlayerBasicData> players;
	players.push_back (player);
	client->setPlayers (players, 0);

	if (gameSettings->getGameType() == eGameSettingsGameType::Turns)
	{
		server->setActiveTurnPlayer (*server->playerList[0]);
	}

	auto& clientPlayer = client->getActivePlayer();
	if (playerClan != -1) clientPlayer.setClan (playerClan);

	server->start();

	applyUnitUpgrades (clientPlayer, unitUpgrades);

	sendClan (*client);
	sendLandingUnits (*client, landingUnits);
	sendUnitUpgrades (*client);

	sendLandingCoords (*client, landingPosition);

	sendReadyToStart (*client);

	server->startTurnTimers();

	gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);

	gameGuiController->setSingleClient (client);

	cGameGuiState playerGameGuiState;
	playerGameGuiState.setMapPosition (landingPosition);
	gameGuiController->addPlayerGameGuiState (clientPlayer, std::move (playerGameGuiState));

	gameGuiController->start();

	using namespace std::placeholders;
	signalConnectionManager.connect (gameGuiController->triggeredSave, std::bind (&cLocalSingleplayerGameNew::save, this, _1, _2));

	terminate = false;

	application.addRunnable (shared_from_this());

	signalConnectionManager.connect (gameGuiController->terminated, [&]() { terminate = true; });
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
	cPlayerBasicData player (cSettings::getInstance().getPlayerName(), cPlayerColor(), 0);

	player.setLocal();

	return player;
}