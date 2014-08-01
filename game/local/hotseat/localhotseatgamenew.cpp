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

#include "game/local/hotseat/localhotseatgamenew.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/application.h"
#include "client.h"
#include "server.h"
#include "game/data/player/player.h"
#include "buildings.h"
#include "vehicles.h"
#include "clientevents.h"

// TODO: remove
void applyUnitUpgrades (cPlayer& player, const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades);

//------------------------------------------------------------------------------
cLocalHotSeatGameNew::cLocalHotSeatGameNew ()
{}

//------------------------------------------------------------------------------
void cLocalHotSeatGameNew::start (cApplication& application)
{
	assert (gameSettings != nullptr);

	server = std::make_unique<cServer> (nullptr);

	server->setMap (staticMap);

	server->setGameSettings (*gameSettings);

	clients.resize (playersData.size ());

	std::vector<cPlayerBasicData> players;
	for (size_t i = 0; i < playersData.size (); ++i)
	{
		clients[i] = std::make_shared<cClient> (server.get (), nullptr);
		clients[i]->setMap (staticMap);
		clients[i]->setGameSettings (*gameSettings);

		players.push_back (playersData[i].basicData);

		server->addPlayer (std::make_unique<cPlayer> (playersData[i].basicData));
	}

	server->start ();

	for (size_t i = 0; i < playersData.size (); ++i)
	{
		clients[i]->setPlayers (players, i);

		auto& clientPlayer = clients[i]->getActivePlayer ();
		if (gameSettings->getClansEnabled ()) clientPlayer.setClan (playersData[i].clan);

		applyUnitUpgrades (clientPlayer, playersData[i].unitUpgrades);

		sendClan (*clients[i]);
		sendLandingUnits (*clients[i], playersData[i].landingUnits);
		sendUnitUpgrades (*clients[i]);

		sendLandingCoords (*clients[i], playersData[i].landingPosition);

		sendReadyToStart (*clients[i]);
	}

	server->startTurnTimers ();

	gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);

	//gameGuiController->setClient (localClient);

	gameGuiController->start ();

	using namespace std::placeholders;
	signalConnectionManager.connect (gameGuiController->triggeredSave, std::bind (&cLocalHotSeatGameNew::save, this, _1, _2));

	application.addRunnable (shared_from_this ());

	signalConnectionManager.connect (gameGuiController->terminated, [&]()
	{
		// me pointer ensures that game object stays alive till this call has terminated
		auto me = application.removeRunnable (*this);
		terminated ();
	});
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
	playersData.clear ();
	playersData.resize (players.size ());
	for (size_t i = 0; i < players.size (); ++i)
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
const std::shared_ptr<cStaticMap>& cLocalHotSeatGameNew::getStaticMap ()
{
	return staticMap;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cGameSettings>& cLocalHotSeatGameNew::getGameSettings ()
{
	return gameSettings;
}

//------------------------------------------------------------------------------
size_t cLocalHotSeatGameNew::getPlayerCount () const
{
	return playersData.size ();
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
