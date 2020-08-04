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

#include "game/startup/local/hotseat/localhotseatgamenew.h"
#include "game/data/gamesettings.h"
#include "ui/graphical/application.h"
#include "game/logic/client.h"
#include "game/logic/server2.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/landingunit.h"
#include "game/logic/action/actioninitnewgame.h"

// TODO: remove
void applyUnitUpgrades (cPlayer& player, const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades);

//------------------------------------------------------------------------------
cLocalHotSeatGameNew::cLocalHotSeatGameNew()
{}

//------------------------------------------------------------------------------
void cLocalHotSeatGameNew::start (cApplication& application)
{
	assert (gameSettings != nullptr);
	auto connectionManager = std::make_shared<cConnectionManager>();

	server = std::make_unique<cServer2>(connectionManager);


	server->setMap (staticMap);
	server->setUnitsData (unitsData);
	server->setGameSettings (*gameSettings);

	clients.resize (playersData.size());

	std::vector<cPlayerBasicData> players;
	for (size_t i = 0; i < playersData.size(); ++i)
	{
		clients[i] = std::make_shared<cClient> (connectionManager);
		clients[i]->setMap (staticMap);
		clients[i]->setUnitsData (unitsData);
		clients[i]->setGameSettings (*gameSettings);

		players.push_back (playersData[i].basicData);

		//auto serverPlayer = std::make_unique<cPlayer>(playersData[i].basicData, unitsData);
		//auto& playerRef = *serverPlayer;

		//serverPlayer->setLocal();
		//server->addPlayer (std::move (serverPlayer));

		if (i == 0)
		{
			//server->setActiveTurnPlayer (playerRef);
		}
	}
	server->setPlayers(players);
	for (size_t i = 0; i != clients.size(); ++i)
	{
		clients[i]->setPlayers (players, i);
	}

	connectionManager->setLocalServer(server.get());
	std::vector<INetMessageReceiver*> hotseatClients;
	for (auto& client : clients)
	{
		hotseatClients.push_back(client.get());
	}
	connectionManager->setLocalClients(std::move(hotseatClients));

	server->start();

	for (size_t i = 0; i != playersData.size(); ++i)
	{
		cActionInitNewGame action;
		action.clan = playersData[i].clan;
		action.landingUnits = playersData[i].landingUnits;
		action.landingPosition = playersData[i].landingPosition;
		action.unitUpgrades = playersData[i].unitUpgrades;
		clients[i]->sendNetMessage(action);
	}

	gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);
	gameGuiController->setServer(server.get());
	gameGuiController->setClients(clients, 0);

	for (size_t i = 0; i != playersData.size(); ++i)
	{
		const auto& clientPlayer = clients[i]->getActivePlayer();
		cGameGuiState gameGuiState;
		gameGuiState.setMapPosition (playersData[i].landingPosition);
		gameGuiController->addPlayerGameGuiState (clientPlayer.getId(), gameGuiState);
	}

	gameGuiController->setClients (clients, 0 /*activePlayer->getId()*/);

	gameGuiController->start();

	application.addRunnable (shared_from_this());

	signalConnectionManager.connect(gameGuiController->terminated, [&]() { exit(); });
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
	for (size_t i = 0; i < players.size(); ++i)
	{
		playersData[i].basicData = players[i];
	}
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
const std::vector<sLandingUnit>& cLocalHotSeatGameNew::getLandingUnits(size_t playerIndex)
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
	return playersData[playerIndex].basicData;
}

//------------------------------------------------------------------------------
int cLocalHotSeatGameNew::getPlayerClan (size_t playerIndex) const
{
	return playersData[playerIndex].clan;
}
