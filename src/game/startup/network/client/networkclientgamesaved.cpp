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

#include "game/startup/network/client/networkclientgamesaved.h"
#include "game/data/gamesettings.h"
#include "ui/graphical/application.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/data/player/player.h"
#include "game/logic/clientevents.h"

//------------------------------------------------------------------------------
cNetworkClientGameSaved::cNetworkClientGameSaved()
{}

//------------------------------------------------------------------------------
void cNetworkClientGameSaved::start (cApplication& application)
{
	//localClient = std::make_shared<cClient> (nullptr, network); //TODO

	localClient->setPlayers (players, localPlayerIndex);
//	localClient->setMap (staticMap);
	localClient->setGameSettings (*gameSettings);

	sendRequestResync (*localClient, localClient->getActivePlayer().getId(), true);

	gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);

	gameGuiController->setSingleClient (localClient);

	gameGuiController->start();

	using namespace std::placeholders;
	signalConnectionManager.connect (gameGuiController->triggeredSave, std::bind (&cNetworkClientGameSaved::save, this, _1, _2));

	terminate = false;

	application.addRunnable (shared_from_this());

	signalConnectionManager.connect (gameGuiController->terminated, [&]() { terminate = true; });
}

//------------------------------------------------------------------------------
void cNetworkClientGameSaved::setPlayers (std::vector<cPlayerBasicData> players_, const cPlayerBasicData& localPlayer)
{
	players = players_;
	auto localPlayerIter = std::find_if (players.begin(), players.end(), [&] (const cPlayerBasicData & player) { return player.getNr() == localPlayer.getNr(); });
	assert (localPlayerIter != players.end());
	localPlayerIndex = localPlayerIter - players.begin();
}

//------------------------------------------------------------------------------
void cNetworkClientGameSaved::setGameSettings (std::shared_ptr<cGameSettings> gameSettings_)
{
	gameSettings = gameSettings_;
}


//------------------------------------------------------------------------------
void cNetworkClientGameSaved::setStaticMap (std::shared_ptr<cStaticMap> staticMap_)
{
	staticMap = staticMap_;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cGameSettings>& cNetworkClientGameSaved::getGameSettings()
{
	return gameSettings;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cStaticMap>& cNetworkClientGameSaved::getStaticMap()
{
	return staticMap;
}

//------------------------------------------------------------------------------
const std::vector<cPlayerBasicData>& cNetworkClientGameSaved::getPlayers()
{
	return players;
}

//------------------------------------------------------------------------------
const cPlayerBasicData& cNetworkClientGameSaved::getLocalPlayer()
{
	return players[localPlayerIndex];
}
