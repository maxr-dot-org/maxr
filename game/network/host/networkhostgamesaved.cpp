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

#include "networkhostgamesaved.h"
#include "../../../gui/menu/windows/windowgamesettings/gamesettings.h"
#include "../../../gui/application.h"
#include "../../../gui/game/gamegui.h"
#include "../../../client.h"
#include "../../../server.h"
#include "../../../player.h"
#include "../../../clientevents.h"
#include "../../../savegame.h"
#include "../../data/report/savedreport.h"

//------------------------------------------------------------------------------
void cNetworkHostGameSaved::start (cApplication& application)
{
	server = std::make_unique<cServer> (network);
	localClient = std::make_unique<cClient> (server.get (), nullptr);

	cSavegame savegame (saveGameNumber);
	if (savegame.load (*server) == false) return; // TODO: error message

	auto staticMap = server->Map->staticMap;
	localClient->setMap (staticMap);

	const auto& serverPlayerList = server->playerList;
	if (serverPlayerList.empty ()) return;

	// Following may be simplified according to serverGame::loadGame
	std::vector<sPlayer> clientPlayerList;

    size_t serverListLocalPlayerIndex = 0;
	// copy players for client
	for (size_t i = 0; i != serverPlayerList.size (); ++i)
	{
		auto& serverPlayer = *serverPlayerList[i];
        
        auto iter = std::find_if (players.begin (), players.end (), [&](const std::shared_ptr<sPlayer>& player){ return player->getNr () == serverPlayer.getNr (); });
        
        assert (iter != players.end ());
        const auto& listPlayer = **iter;
        
        if (&listPlayer == players[localPlayerIndex].get ())
        {
            serverPlayer.setLocal ();
            serverListLocalPlayerIndex = i;
        }
        else
        {
            serverPlayer.setSocketIndex (listPlayer.getSocketIndex ());
        }

        clientPlayerList.push_back (sPlayer (serverPlayer.getName (), serverPlayer.getColor (), serverPlayer.getNr (), serverPlayer.getSocketNum ()));
	}
    localClient->setPlayers (clientPlayerList, serverListLocalPlayerIndex);

	server->start ();

    sendRequestResync (*localClient, serverPlayerList[serverListLocalPlayerIndex]->getNr ());

	// TODO: move that in server
	for (size_t i = 0; i != serverPlayerList.size (); ++i)
	{
		sendHudSettings (*server, *serverPlayerList[i]);
		auto& reportList = serverPlayerList[i]->savedReportsList;
		for (size_t j = 0; j != reportList.size (); ++j)
		{
			sendSavedReport (*server, *reportList[j], serverPlayerList[i].get());
		}
		reportList.clear ();
	}

	// start game
	server->serverState = SERVER_STATE_INGAME;

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
	signalConnectionManager.connect (gameGui->triggeredSave, std::bind (&cNetworkHostGameSaved::save, this, _1, _2));

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
void cNetworkHostGameSaved::setSaveGameNumber (int saveGameNumber_)
{
	saveGameNumber = saveGameNumber_;
}

//------------------------------------------------------------------------------
void cNetworkHostGameSaved::setPlayers (std::vector<std::shared_ptr<sPlayer>> players_, const sPlayer& localPlayer)
{
    players = players_;
    auto localPlayerIter = std::find_if (players.begin (), players.end (), [&](const std::shared_ptr<sPlayer>& player){ return player->getNr () == localPlayer.getNr (); });
    assert (localPlayerIter != players.end ());
    localPlayerIndex = localPlayerIter - players.begin ();
}

//------------------------------------------------------------------------------
const std::vector<std::shared_ptr<sPlayer>>& cNetworkHostGameSaved::getPlayers ()
{
    return players;
}

//------------------------------------------------------------------------------
const std::shared_ptr<sPlayer>& cNetworkHostGameSaved::getLocalPlayer ()
{
    return players[localPlayerIndex];
}