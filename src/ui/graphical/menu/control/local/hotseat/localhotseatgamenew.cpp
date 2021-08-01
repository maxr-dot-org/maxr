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

#include "localhotseatgamenew.h"

#include "game/data/gamesettings.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/logic/action/actioninitnewgame.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/startup/lobbypreparationdata.h"
#include "ui/graphical/application.h"

#include <cassert>

//------------------------------------------------------------------------------
void cLocalHotSeatGameNew::start (cApplication& application)
{
	assert (gameSettings != nullptr);
	auto connectionManager = std::make_shared<cConnectionManager>();

	server = std::make_unique<cServer> (connectionManager);

	server->setPreparationData ({unitsData, clanData, gameSettings, staticMap});

	clients.resize (playersData.size());

	for (size_t i = 0; i != playersData.size(); ++i)
	{
		clients[i] = std::make_shared<cClient> (connectionManager);
		clients[i]->setPreparationData ({unitsData, clanData, gameSettings, staticMap});
	}
	server->setPlayers (playersBasicData);
	for (size_t i = 0; i != clients.size(); ++i)
	{
		clients[i]->setPlayers (playersBasicData, i);
	}

	connectionManager->setLocalServer (server.get());
	std::vector<INetMessageReceiver*> hotseatClients;
	for (auto& client : clients)
	{
		hotseatClients.push_back (client.get());
	}
	connectionManager->setLocalClients (std::move (hotseatClients));

	server->start();

	for (size_t i = 0; i != playersData.size(); ++i)
	{
		clients[i]->sendNetMessage (cActionInitNewGame (playersData[i]));
	}

	gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);
	gameGuiController->setServer (server.get());
	gameGuiController->setClients (clients, 0);

	for (size_t i = 0; i != playersData.size(); ++i)
	{
		const auto& clientPlayer = clients[i]->getActivePlayer();
		cGameGuiState gameGuiState;
		gameGuiState.mapPosition = playersData[i].landingPosition;
		gameGuiController->addPlayerGameGuiState (clientPlayer.getId(), gameGuiState);
	}

	gameGuiController->setClients (clients, 0);
	gameGuiController->start();

	application.addRunnable (shared_from_this());

	signalConnectionManager.connect (gameGuiController->terminated, [&]() { exit(); });
}

//------------------------------------------------------------------------------
void cLocalHotSeatGameNew::setGameSettings (std::shared_ptr<cGameSettings> gameSettings_)
{
	gameSettings = gameSettings_;
}

//------------------------------------------------------------------------------
void cLocalHotSeatGameNew::setStaticMap (std::shared_ptr<cStaticMap> staticMap_)
{
	staticMap = staticMap_;
}

//------------------------------------------------------------------------------
void cLocalHotSeatGameNew::setPlayers (const std::vector<cPlayerBasicData>& players)
{
	playersData.clear();
	playersData.resize (players.size());
	playersBasicData = players;
}

//------------------------------------------------------------------------------
void cLocalHotSeatGameNew::setPlayerClan (size_t playerIndex, int clan)
{
	playersData[playerIndex].clan = clan;
}

//------------------------------------------------------------------------------
void cLocalHotSeatGameNew::setLandingUnits (size_t playerIndex, std::vector<sLandingUnit> landingUnits_)
{
	playersData[playerIndex].landingUnits = std::move (landingUnits_);
}

//------------------------------------------------------------------------------
void cLocalHotSeatGameNew::setUnitUpgrades (size_t playerIndex, std::vector<std::pair<sID, cUnitUpgrade>> unitUpgrades_)
{
	playersData[playerIndex].unitUpgrades = std::move (unitUpgrades_);
}

//------------------------------------------------------------------------------
void cLocalHotSeatGameNew::setLandingPosition (size_t playerIndex, const cPosition& landingPosition_)
{
	playersData[playerIndex].landingPosition = landingPosition_;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cStaticMap>& cLocalHotSeatGameNew::getStaticMap()
{
	return staticMap;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cGameSettings>& cLocalHotSeatGameNew::getGameSettings()
{
	return gameSettings;
}

//------------------------------------------------------------------------------
const std::vector<sLandingUnit>& cLocalHotSeatGameNew::getLandingUnits (size_t playerIndex)
{
	return playersData[playerIndex].landingUnits;
}

//------------------------------------------------------------------------------
size_t cLocalHotSeatGameNew::getPlayerCount() const
{
	return playersData.size();
}

//------------------------------------------------------------------------------
const cPlayerBasicData& cLocalHotSeatGameNew::getPlayer (size_t playerIndex) const
{
	return playersBasicData[playerIndex];
}

//------------------------------------------------------------------------------
int cLocalHotSeatGameNew::getPlayerClan (size_t playerIndex) const
{
	return playersData[playerIndex].clan;
}
