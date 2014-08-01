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

#include "game/network/client/networkclientgamereconnection.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/application.h"
#include "client.h"
#include "server.h"
#include "game/data/player/player.h"
#include "clientevents.h"

//------------------------------------------------------------------------------
cNetworkClientGameReconnection::cNetworkClientGameReconnection ()
{}

//------------------------------------------------------------------------------
void cNetworkClientGameReconnection::start (cApplication& application)
{
	localClient = std::make_unique<cClient> (nullptr, network);

	localClient->setPlayers (players, localPlayerIndex);
	localClient->setMap (staticMap);

	sendReconnectionSuccess (*localClient);

	gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);

	gameGuiController->setClient (localClient);

	gameGuiController->start ();

	using namespace std::placeholders;
	signalConnectionManager.connect (gameGuiController->triggeredSave, std::bind (&cNetworkClientGameReconnection::save, this, _1, _2));

	application.addRunnable (shared_from_this ());

	signalConnectionManager.connect (gameGuiController->terminated, [&]()
	{
        // me pointer ensures that game object stays alive till this call has terminated
		auto me = application.removeRunnable (*this);
		terminated ();
	});
}

//------------------------------------------------------------------------------
void cNetworkClientGameReconnection::setPlayers (std::vector<cPlayerBasicData> players_, const cPlayerBasicData& localPlayer)
{
	players = players_;
	auto localPlayerIter = std::find_if (players.begin (), players.end (), [&](const cPlayerBasicData& player){ return player.getNr () == localPlayer.getNr (); });
	assert (localPlayerIter != players.end());
	localPlayerIndex = localPlayerIter - players.begin ();
}

//------------------------------------------------------------------------------
void cNetworkClientGameReconnection::setStaticMap (std::shared_ptr<cStaticMap> staticMap_)
{
	staticMap = staticMap_;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cStaticMap>& cNetworkClientGameReconnection::getStaticMap ()
{
	return staticMap;
}

//------------------------------------------------------------------------------
const std::vector<cPlayerBasicData>& cNetworkClientGameReconnection::getPlayers ()
{
	return players;
}

//------------------------------------------------------------------------------
const cPlayerBasicData& cNetworkClientGameReconnection::getLocalPlayer ()
{
	return players[localPlayerIndex];
}
