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

#include "localgame.h"
#include "../gui/menu/windows/windowgamesettings/gamesettings.h"
#include "../gui/application.h"
#include "../gui/game/gamegui.h"
#include "../client.h"
#include "../server.h"
#include "../events.h"
#include "../player.h"
#include "../clientevents.h"

//------------------------------------------------------------------------------
void cLocalGame::start (cApplication& application, const cPosition& landingPosition, const std::vector<sLandingUnit>& landingUnits)
{
	eventHandling = std::make_unique<cEventHandling> ();
	server = std::make_unique<cServer> (nullptr);
	client = std::make_unique<cClient> (server.get(), nullptr, *eventHandling);

	server->setMap (staticMap);
	client->setMap (staticMap);

	server->setGameSettings (gameSettings->toOldSettings());
	client->setGameSetting (gameSettings->toOldSettings ());

	auto player = createPlayer ();

	server->addPlayer (new cPlayer (player));
	server->changeStateToInitGame ();

	std::vector<sPlayer*> players;
	players.push_back (&player);
	client->setPlayers (players, player);

	auto& clientActivePlayer = client->getActivePlayer ();
	if (playerClan != -1) clientActivePlayer.setClan (playerClan);

	sendClan (*client);
	sendLandingUnits (*client, landingUnits);
	sendUnitUpgrades (*client);

	sClientLandData landData;
	landData.iLandX = landingPosition.x();
	landData.iLandY = landingPosition.y();

	sendLandingCoords (*client, landData);

	auto gameGui = std::make_shared<cNewGameGUI> (staticMap);

	gameGui->setDynamicMap (client->getMap ());
	gameGui->setPlayer (&client->getActivePlayer ());

	gameGui->connectToClient (*client);

	application.show (gameGui);

	application.setGame (shared_from_this ());
}

//------------------------------------------------------------------------------
void cLocalGame::setGameSettings (std::shared_ptr<cGameSettings> gameSettings_)
{
	gameSettings = gameSettings_;
}

//------------------------------------------------------------------------------
void cLocalGame::setStaticMap (std::shared_ptr<cStaticMap> staticMap_)
{
	staticMap = staticMap_;
}

//------------------------------------------------------------------------------
void cLocalGame::setPlayerClan (int clan)
{
	playerClan = clan;
}

//------------------------------------------------------------------------------
sPlayer cLocalGame::createPlayer ()
{
	sPlayer player(cSettings::getInstance ().getPlayerName (), 0, 0);

	player.setLocal ();

	return player;
}

//------------------------------------------------------------------------------
void cLocalGame::run ()
{
	if (client) client->gameTimer.run (nullptr);
}
