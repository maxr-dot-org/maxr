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

#include "game/startup/local/singleplayer/localsingleplayergamesaved.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/application.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/data/player/player.h"
#include "game/logic/clientevents.h"
#include "game/logic/savegame.h"
#include "game/data/report/savedreport.h"

//------------------------------------------------------------------------------
void cLocalSingleplayerGameSaved::start (cApplication& application)
{
	server = std::make_unique<cServer> (nullptr);
	client = std::make_shared<cClient> (server.get (), nullptr);

	cSavegame savegame (saveGameNumber);
	if (savegame.load (*server) == false) return;

	auto staticMap = server->Map->staticMap;
	client->setMap (staticMap);

	const auto& serverPlayerList = server->playerList;
	if (serverPlayerList.empty ()) return;

	const int player = 0;

	// Following may be simplified according to serverGame::loadGame
	std::vector<cPlayerBasicData> clientPlayerList;

	// copy players for client
	for (size_t i = 0; i != serverPlayerList.size (); ++i)
	{
		const auto& p = *serverPlayerList[i];
		clientPlayerList.push_back (cPlayerBasicData (p.getName (), p.getColor (), p.getNr (), p.getSocketNum ()));
	}
	client->setPlayers (clientPlayerList, player);

	server->start ();

	// in single player only the first player is important
	serverPlayerList[player]->setLocal ();
	sendRequestResync (*client, serverPlayerList[player]->getNr ());

	// TODO: move that in server
	for (size_t i = 0; i != serverPlayerList.size (); ++i)
	{
		sendGameSettings (*server, *serverPlayerList[i]);
		sendGameGuiState (*server, server->getPlayerGameGuiState (*serverPlayerList[i]), *serverPlayerList[i]);
		auto& reportList = serverPlayerList[i]->savedReportsList;
		for (size_t j = 0; j != reportList.size (); ++j)
		{
			sendSavedReport (*server, *reportList[j], serverPlayerList[i].get());
		}
		reportList.clear ();
	}

	// start game
	server->serverState = SERVER_STATE_INGAME;

	// TODO: save/load game time
	server->startTurnTimers ();

	gameGuiController = std::make_unique<cGameGuiController> (application, staticMap);

	gameGuiController->setSingleClient (client);

	gameGuiController->start ();

	using namespace std::placeholders;
	signalConnectionManager.connect (gameGuiController->triggeredSave, std::bind (&cLocalSingleplayerGameSaved::save, this, _1, _2));

	terminate = false;

	application.addRunnable (shared_from_this ());

	signalConnectionManager.connect (gameGuiController->terminated, [&]() { terminate = true; });
}

void cLocalSingleplayerGameSaved::setSaveGameNumber (int saveGameNumber_)
{
	saveGameNumber = saveGameNumber_;
}
