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

#include "game/startup/network/client/networkclientgamenew.h"
#include "game/data/gamesettings.h"
#include "ui/graphical/application.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/data/player/player.h"
#include "game/data/units/landingunit.h"
#include "game/logic/clientevents.h"

// TODO: remove
void applyUnitUpgrades (cPlayer& player, const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades);

//------------------------------------------------------------------------------
cNetworkClientGameNew::cNetworkClientGameNew() :
	localPlayerClan (-1)
{}

//------------------------------------------------------------------------------
void cNetworkClientGameNew::start (cApplication& application)
{
	assert (gameSettings != nullptr);

	//localClient = std::make_shared<cClient> (nullptr, network); //TODO

	localClient->setPlayers (players, localPlayerIndex);
//	localClient->setMap (staticMap);
	localClient->setGameSettings (*gameSettings);

	auto& clientPlayer = localClient->getActivePlayer();
//	if (localPlayerClan != -1) clientPlayer.setClan (localPlayerClan);
//	applyUnitUpgrades (clientPlayer, localPlayerUnitUpgrades);

	sendClan (*localClient);
	sendLandingUnits (*localClient, localPlayerLandingUnits);
	sendUnitUpgrades (*localClient);

	sendLandingCoords (*localClient, localPlayerLandingPosition);

	sendReadyToStart (*localClient);

	gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);

	gameGuiController->setSingleClient (localClient);

	cGameGuiState playerGameGuiState;
	playerGameGuiState.setMapPosition (localPlayerLandingPosition);
	gameGuiController->addPlayerGameGuiState (clientPlayer, std::move (playerGameGuiState));

	gameGuiController->start();

	using namespace std::placeholders;
	signalConnectionManager.connect (gameGuiController->triggeredSave, std::bind (&cNetworkClientGameNew::save, this, _1, _2));

	terminate = false;

	application.addRunnable (shared_from_this());

	signalConnectionManager.connect (gameGuiController->terminated, [&]() { terminate = true; });
}

//------------------------------------------------------------------------------
void cNetworkClientGameNew::setPlayers (std::vector<cPlayerBasicData> players_, const cPlayerBasicData& localPlayer)
{
	players = players_;
	auto localPlayerIter = std::find_if (players.begin(), players.end(), [&] (const cPlayerBasicData & player) { return player.getNr() == localPlayer.getNr(); });
	assert (localPlayerIter != players.end());
	localPlayerIndex = localPlayerIter - players.begin();
}

//------------------------------------------------------------------------------
void cNetworkClientGameNew::setGameSettings (std::shared_ptr<cGameSettings> gameSettings_)
{
	gameSettings = gameSettings_;
}

//------------------------------------------------------------------------------
void cNetworkClientGameNew::setStaticMap (std::shared_ptr<cStaticMap> staticMap_)
{
	staticMap = staticMap_;
}

//------------------------------------------------------------------------------
void cNetworkClientGameNew::setLocalPlayerClan (int clan)
{
	localPlayerClan = clan;
}

//------------------------------------------------------------------------------
void cNetworkClientGameNew::setLocalPlayerLandingUnits (std::vector<sLandingUnit> landingUnits_)
{
	localPlayerLandingUnits = std::move (landingUnits_);
}

//------------------------------------------------------------------------------
void cNetworkClientGameNew::setLocalPlayerUnitUpgrades (std::vector<std::pair<sID, cUnitUpgrade>> unitUpgrades_)
{
	localPlayerUnitUpgrades = std::move (unitUpgrades_);
}

//------------------------------------------------------------------------------
void cNetworkClientGameNew::setLocalPlayerLandingPosition (const cPosition& landingPosition_)
{
	localPlayerLandingPosition = landingPosition_;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cGameSettings>& cNetworkClientGameNew::getGameSettings()
{
	return gameSettings;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cStaticMap>& cNetworkClientGameNew::getStaticMap()
{
	return staticMap;
}

//------------------------------------------------------------------------------
const std::vector<cPlayerBasicData>& cNetworkClientGameNew::getPlayers()
{
	return players;
}

//------------------------------------------------------------------------------
const cPlayerBasicData& cNetworkClientGameNew::getLocalPlayer()
{
	return players[localPlayerIndex];
}

//------------------------------------------------------------------------------
int cNetworkClientGameNew::getLocalPlayerClan() const
{
	return localPlayerClan;
}
