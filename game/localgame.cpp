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
#include "../buildings.h"
#include "../vehicles.h"
#include "../clientevents.h"
#include "../savegame.h"
#include "../loaddata.h"

//------------------------------------------------------------------------------
cLocalGame::~cLocalGame ()
{
	if (server) server->stop ();

	reloadUnitValues ();
}

//------------------------------------------------------------------------------
void cLocalGame::start (cApplication& application, const cPosition& landingPosition, const std::vector<sLandingUnit>& landingUnits, const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades)
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

	std::vector<sPlayer> players;
	players.push_back (player);
	client->setPlayers (players, 0);

	auto& clientPlayer = client->getActivePlayer ();
	if (playerClan != -1) clientPlayer.setClan (playerClan);
	applyUnitUpgrades (clientPlayer, unitUpgrades);

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

	gameGui->centerAt (landingPosition);

	application.show (gameGui);

	application.setGame (shared_from_this ());

	signalConnectionManager.connect (gameGui->terminated, [&]()
	{
		application.setGame (nullptr);
	});
}

//------------------------------------------------------------------------------
void cLocalGame::startSaved (cApplication& application, int saveGameNumber)
{
	eventHandling = std::make_unique<cEventHandling> ();
	server = std::make_unique<cServer> (nullptr);
	client = std::make_unique<cClient> (server.get (), nullptr, *eventHandling);

	cSavegame savegame (saveGameNumber);
	if (savegame.load (*server) == false) return;

	staticMap = server->Map->staticMap;
	client->setMap (staticMap);

	const auto& serverPlayerList = server->PlayerList;
	if (serverPlayerList.empty ()) return;

	const int player = 0;

	// Following may be simplified according to serverGame::loadGame
	std::vector<sPlayer> clientPlayerList;

	// copy players for client
	for (size_t i = 0; i != serverPlayerList.size (); ++i)
	{
		const auto& p = *serverPlayerList[i];
		clientPlayerList.push_back (sPlayer (p.getName (), p.getColor (), p.getNr (), p.getSocketNum ()));
	}
	client->setPlayers (clientPlayerList, player);

	// in single player only the first player is important
	serverPlayerList[player]->setLocal ();
	sendRequestResync (*client, serverPlayerList[player]->getNr ());

	// TODO: move that in server
	for (size_t i = 0; i != serverPlayerList.size (); ++i)
	{
		sendHudSettings (*server, *serverPlayerList[i]);
		std::vector<sSavedReportMessage>& reportList = serverPlayerList[i]->savedReportsList;
		for (size_t j = 0; j != reportList.size (); ++j)
		{
			sendSavedReport (*server, reportList[j], serverPlayerList[i]->getNr ());
		}
		reportList.clear ();
	}

	// start game
	server->serverState = SERVER_STATE_INGAME;

	auto gameGui = std::make_shared<cNewGameGUI> (staticMap);

	gameGui->setDynamicMap (client->getMap ());
	gameGui->setPlayer (&client->getActivePlayer ());

	gameGui->connectToClient (*client);

	application.show (gameGui);

	application.setGame (shared_from_this ());

	signalConnectionManager.connect (gameGui->terminated, [&]()
	{
		application.setGame (nullptr);
	});
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

//------------------------------------------------------------------------------
void cLocalGame::save (int saveNumber, const std::string& saveName)
{
	//if (!server)
	//{
	//	cDialogOK okDialog (lngPack.i18n ("Text~Multiplayer~Save_Only_Host"));
	//	okDialog.show (menu->client);
	//	menu->draw ();
	//	return;
	//}

	if (!server) throw std::runtime_error ("Game not started!"); // should never happen (hence a translation is not necessary).

	cSavegame savegame (saveNumber);
	savegame.save (*server, saveName);
	server->makeAdditionalSaveRequest (saveNumber);
}

//------------------------------------------------------------------------------
void cLocalGame::applyUnitUpgrades (cPlayer& player, const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades)
{
	for (size_t i = 0; i < unitUpgrades.size (); ++i)
	{
		const auto& unitId = unitUpgrades[i].first;

		auto unitData = player.getUnitDataCurrentVersion (unitId);

		if (unitData)
		{
			const auto& upgrades = unitUpgrades[i].second;
			upgrades.updateUnitData (*unitData);
		}
	}
}