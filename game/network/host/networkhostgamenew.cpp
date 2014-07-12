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

#include "game/network/host/networkhostgamenew.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/application.h"
#include "ui/graphical/game/gamegui.h"
#include "client.h"
#include "server.h"
#include "player.h"
#include "clientevents.h"

// FIXME: remove
void applyUnitUpgrades (cPlayer& player, const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades);

//------------------------------------------------------------------------------
cNetworkHostGameNew::cNetworkHostGameNew () :
	localPlayerClan (-1)
{}

//------------------------------------------------------------------------------
void cNetworkHostGameNew::start (cApplication& application)
{
	assert (gameSettings != nullptr);

	server = std::make_unique<cServer> (network);
	localClient = std::make_unique<cClient> (server.get(), nullptr);

	std::vector<sPlayer> clientPlayers;
	for (size_t i = 0; i < players.size (); ++i)
	{
		server->addPlayer (std::make_unique<cPlayer>(*players[i]));
		clientPlayers.push_back (*players[i]);
	}
	localClient->setPlayers (clientPlayers, localPlayerIndex);

	server->setMap (staticMap);
	localClient->setMap (staticMap);

	server->setGameSettings (*gameSettings);
	localClient->setGameSettings (*gameSettings);

	auto& clientPlayer = localClient->getActivePlayer ();
	if (localPlayerClan != -1) clientPlayer.setClan (localPlayerClan);

	server->start ();

	applyUnitUpgrades (clientPlayer, localPlayerUnitUpgrades);

	sendClan (*localClient);
	sendLandingUnits (*localClient, localPlayerLandingUnits);
	sendUnitUpgrades (*localClient);

	sendLandingCoords (*localClient, localPlayerLandingPosition);

	sendReadyToStart (*localClient);

	auto gameGui = std::make_shared<cGameGui> (staticMap);

	gameGui->setDynamicMap (localClient->getMap ());

	std::vector<std::shared_ptr<const cPlayer>> guiPlayers;
	for (size_t i = 0; i < localClient->getPlayerList ().size (); ++i)
	{
		const auto& player = localClient->getPlayerList ()[i];
		guiPlayers.push_back (player);
		if (player.get () == &localClient->getActivePlayer ())
		{
			gameGui->setPlayer (player);
		}
	}
	gameGui->setPlayers (guiPlayers);
	gameGui->setCasualtiesTracker (localClient->getCasualtiesTracker ());
	gameGui->setTurnClock (localClient->getTurnClock ());
	gameGui->setTurnTimeClock (localClient->getTurnTimeClock ());
	gameGui->setGameSettings (localClient->getGameSettings ());

	gameGui->connectToClient (*localClient);

	gameGui->centerAt (localPlayerLandingPosition);

	using namespace std::placeholders;
	signalConnectionManager.connect (gameGui->triggeredSave, std::bind (&cNetworkHostGameNew::save, this, _1, _2));

	application.show (gameGui);

	application.addRunnable (shared_from_this ());

	signalConnectionManager.connect (gameGui->terminated, [&]()
    {
        // me pointer ensures that game object stays alive till this call has terminated
        auto me = application.removeRunnable (*this);
		terminated ();
	});
}

//------------------------------------------------------------------------------
void cNetworkHostGameNew::setPlayers (std::vector<std::shared_ptr<sPlayer>> players_, const sPlayer& localPlayer)
{
	players = players_;
	auto localPlayerIter = std::find_if (players.begin (), players.end (), [&](const std::shared_ptr<sPlayer>& player){ return player->getNr () == localPlayer.getNr (); });
	assert (localPlayerIter != players.end());
	localPlayerIndex = localPlayerIter - players.begin ();
}

//------------------------------------------------------------------------------
void cNetworkHostGameNew::setGameSettings (std::shared_ptr<cGameSettings> gameSettings_)
{
	gameSettings = gameSettings_;
}

//------------------------------------------------------------------------------
void cNetworkHostGameNew::setStaticMap (std::shared_ptr<cStaticMap> staticMap_)
{
	staticMap = staticMap_;
}

//------------------------------------------------------------------------------
void cNetworkHostGameNew::setLocalPlayerClan (int clan)
{
	localPlayerClan = clan;
}

//------------------------------------------------------------------------------
void cNetworkHostGameNew::setLocalPlayerLandingUnits (std::vector<sLandingUnit> landingUnits_)
{
	localPlayerLandingUnits = std::move (landingUnits_);
}

//------------------------------------------------------------------------------
void cNetworkHostGameNew::setLocalPlayerUnitUpgrades (std::vector<std::pair<sID, cUnitUpgrade>> unitUpgrades_)
{
	localPlayerUnitUpgrades = std::move (unitUpgrades_);
}

//------------------------------------------------------------------------------
void cNetworkHostGameNew::setLocalPlayerLandingPosition (const cPosition& landingPosition_)
{
	localPlayerLandingPosition = landingPosition_;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cGameSettings>& cNetworkHostGameNew::getGameSettings ()
{
	return gameSettings;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cStaticMap>& cNetworkHostGameNew::getStaticMap ()
{
	return staticMap;
}

//------------------------------------------------------------------------------
const std::vector<std::shared_ptr<sPlayer>>& cNetworkHostGameNew::getPlayers ()
{
	return players;
}

//------------------------------------------------------------------------------
const std::shared_ptr<sPlayer>& cNetworkHostGameNew::getLocalPlayer ()
{
	return players[localPlayerIndex];
}

//------------------------------------------------------------------------------
int cNetworkHostGameNew::getLocalPlayerClan () const
{
	return localPlayerClan;
}
