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

#include "networkclientgamereconnection.h"
#include "../../../gui/menu/windows/windowgamesettings/gamesettings.h"
#include "../../../gui/application.h"
#include "../../../gui/game/gamegui.h"
#include "../../../client.h"
#include "../../../server.h"
#include "../../../player.h"
#include "../../../clientevents.h"

//------------------------------------------------------------------------------
cNetworkClientGameReconnection::cNetworkClientGameReconnection ()
{}

//------------------------------------------------------------------------------
void cNetworkClientGameReconnection::start (cApplication& application)
{
	localClient = std::make_unique<cClient> (nullptr, network);

	std::vector<sPlayer> clientPlayers;
	for (size_t i = 0; i < players.size (); ++i)
	{
		clientPlayers.push_back (*players[i]);
	}
	localClient->setPlayers (clientPlayers, localPlayerIndex);
	localClient->setMap (staticMap);

	sendReconnectionSuccess (*localClient);

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

	gameGui->connectToClient (*localClient);

	using namespace std::placeholders;
	signalConnectionManager.connect (gameGui->triggeredSave, std::bind (&cNetworkClientGameReconnection::save, this, _1, _2));

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
void cNetworkClientGameReconnection::setPlayers (std::vector<std::shared_ptr<sPlayer>> players_, const sPlayer& localPlayer)
{
	players = players_;
	auto localPlayerIter = std::find_if (players.begin (), players.end (), [&](const std::shared_ptr<sPlayer>& player){ return player->getNr () == localPlayer.getNr (); });
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
const std::vector<std::shared_ptr<sPlayer>>& cNetworkClientGameReconnection::getPlayers ()
{
	return players;
}

//------------------------------------------------------------------------------
const std::shared_ptr<sPlayer>& cNetworkClientGameReconnection::getLocalPlayer ()
{
	return players[localPlayerIndex];
}