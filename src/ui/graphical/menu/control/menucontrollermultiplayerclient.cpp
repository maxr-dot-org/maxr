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

#include "ui/graphical/menu/control/menucontrollermultiplayerclient.h"

#include "game/data/units/landingunit.h"
#include "game/logic/client.h"
#include "game/startup/gamepreparation.h"
#include "game/startup/network/client/networkclientgamenew.h"
#include "game/startup/network/client/networkclientgamereconnection.h"
#include "game/startup/network/client/networkclientgamesaved.h"
#include "ui/graphical/application.h"
#include "ui/graphical/menu/windows/windownetworklobbyclient/windownetworklobbyclient.h"
#include "ui/graphical/menu/windows/windowgamesettings/windowgamesettings.h"
#include "ui/graphical/menu/windows/windowload/windowload.h"
#include "ui/graphical/menu/windows/windowmapselection/windowmapselection.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/dialogs/dialogyesno.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "utility/language.h"
#include "utility/log.h"

//------------------------------------------------------------------------------
cMenuControllerMultiplayerClient::cMenuControllerMultiplayerClient (cApplication& application_) :
	application (application_),
	lobbyClient (std::make_shared<cConnectionManager>(), cPlayerBasicData::fromSettings())
{
	signalConnectionManager.connect (lobbyClient.onChatMessage, [this](const std::string& playerName, bool translate, const std::string& message, const std::string& insertText){
		if (initGamePreparation)
		{
			initGamePreparation->onChatMessage (playerName, translate, message, insertText);
		}
		else if (windowNetworkLobby != nullptr)
		{
			if (translate)
			{
				windowNetworkLobby->addInfoEntry (lngPack.i18n (message, insertText));
			}
			else
			{
				windowNetworkLobby->addChatEntry (playerName, message);
			}
		}
	});

	signalConnectionManager.connect (lobbyClient.onStartGamePreparation, [this](const sLobbyPreparationData& lobbyData, const cPlayerBasicData& localPlayer, std::shared_ptr<cConnectionManager> connectionManager){
		if (windowNetworkLobby != nullptr)
		{
			saveOptions();

			startGamePreparation (lobbyData, localPlayer, connectionManager);
		}
	});


	signalConnectionManager.connect (lobbyClient.onPlayerAbortGamePreparation, [this](const std::string& playerName){
		if (windowNetworkLobby != nullptr)
		{
			auto okDialog = application.show(std::make_shared<cDialogOk>("Player " + playerName + " has quit from game preparation")); // TODO: translate

			signalConnectionManager.connect(okDialog->done, [this]()
			{
				application.closeTill(*windowNetworkLobby);
				initGamePreparation.reset();
			});
		}
	});

	signalConnectionManager.connect (lobbyClient.onStartNewGame, [this](){
		if (!newGame) return;

		startNewGame();
	});

	signalConnectionManager.connect (lobbyClient.onStartSavedGame, [this](const cSaveGameInfo& saveGameInfo, std::shared_ptr<cStaticMap> staticMap, std::shared_ptr<cConnectionManager> connectionManager, cPlayerBasicData localPlayer){
		startSavedGame (saveGameInfo, staticMap, connectionManager, localPlayer);
	});

	signalConnectionManager.connect (lobbyClient.onReconnectGame, [this](std::shared_ptr<cStaticMap> staticMap, std::shared_ptr<cConnectionManager> connectionManager, cPlayerBasicData localPlayer, const std::vector<cPlayerBasicData>& playerList){
		auto yesNoDialog = application.show(std::make_shared<cDialogYesNo>(lngPack.i18n("Text~Multiplayer~Reconnect")));
		signalConnectionManager.connect(yesNoDialog->yesClicked, [=]()
		{
			reconnectToGame (staticMap, connectionManager, localPlayer, playerList);
		});

		signalConnectionManager.connect(yesNoDialog->noClicked, [this]()
		{
			lobbyClient.disconnect();
		});
	});

	signalConnectionManager.connect (lobbyClient.onFailToReconnectGameNoMap, [this](const std::string& mapName){
		application.show(std::make_shared<cDialogOk>("Map \"" + mapName + "\" not found")); //TODO: translate
	});
	signalConnectionManager.connect (lobbyClient.onFailToReconnectGameInvalidMap, [this](const std::string& mapName){
		application.show(std::make_shared<cDialogOk>("The map \"" + mapName + "\" does not match the map of the server")); // TODO: translate
	});
}

//------------------------------------------------------------------------------
cMenuControllerMultiplayerClient::~cMenuControllerMultiplayerClient()
{
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::start()
{
	connectionLost = false;

	windowNetworkLobby = std::make_shared<cWindowNetworkLobbyClient> ();

	windowNetworkLobby->bindConnections (lobbyClient);

	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectMap, [this](){ handleSelectMap(); });
	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectSettings, [this]() { handleSelectSettings(); });
	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectSaveGame, [this]() { handleSelectSaveGame(); });

	application.show (windowNetworkLobby);
	application.addRunnable (shared_from_this());

	signalConnectionManager.connect (windowNetworkLobby->terminated, std::bind (&cMenuControllerMultiplayerClient::reset, this));
	signalConnectionManager.connect (windowNetworkLobby->triggeredStartGame, [this](){ handleStartGame(); });
	signalConnectionManager.connect (windowNetworkLobby->backClicked, [this]()
	{
		windowNetworkLobby->close();
		saveOptions();
	});
}


//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::reset()
{
	windowNetworkLobby = nullptr;
	newGame = nullptr;
	application.removeRunnable (shared_from_this());
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::run()
{
	lobbyClient.run();
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleSelectSettings()
{
	if (!windowNetworkLobby) return;

	auto windowGameSettings = application.show (std::make_shared<cWindowGameSettings>());

	if (lobbyClient.getGameSettings()) windowGameSettings->applySettings (*lobbyClient.getGameSettings());
	else windowGameSettings->applySettings (cGameSettings());

	windowGameSettings->done.connect ([this, windowGameSettings]()
	{
		lobbyClient.selectGameSettings (windowGameSettings->getGameSettings());
		windowGameSettings->close();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleSelectMap()
{
	if (!windowNetworkLobby) return;

	auto windowMapSelection = application.show (std::make_shared<cWindowMapSelection>());
	windowMapSelection->done.connect ([=]()
	{
		lobbyClient.selectMapName (windowMapSelection->getSelectedMapName());
		windowMapSelection->close();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleSelectSaveGame()
{
	if (!windowNetworkLobby) return;

	auto windowLoad = application.show (std::make_shared<cWindowLoad> (nullptr, [this](){ return lobbyClient.getSaveGames(); }));
	windowLoad->load.connect ([=](const cSaveGameInfo& saveGame)
	{
		lobbyClient.selectLoadGame (saveGame);
		windowLoad->close();
	});
}
//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleStartGame()
{
	lobbyClient.askToFinishLobby();
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::startSavedGame (const cSaveGameInfo& saveGameInfo, std::shared_ptr<cStaticMap> staticMap, std::shared_ptr<cConnectionManager> connectionManager, cPlayerBasicData localPlayer)
{
	if (!windowNetworkLobby) return;

	auto savedGame = std::make_shared<cNetworkClientGameSaved> ();

	savedGame->setConnectionManager (connectionManager);
	savedGame->setStaticMap (staticMap);
	savedGame->setPlayers(saveGameInfo.players, localPlayer);

	application.closeTill (*windowNetworkLobby);
	windowNetworkLobby->close();
	signalConnectionManager.connect (windowNetworkLobby->terminated, [&]() { windowNetworkLobby = nullptr; });

	savedGame->start (application);

	signalConnectionManager.connect(savedGame->getLocalClient().connectionToServerLost, [&]() { connectionLost = true; });
	signalConnectionManager.connect(savedGame->terminated, [&]()
	{
		if (connectionLost)
		{
			reset();
			start();
			windowNetworkLobby->addInfoEntry(lngPack.i18n("Text~Multiplayer~Lost_Connection", "server"));
		}
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::startGamePreparation(const sLobbyPreparationData& lobbyData, const cPlayerBasicData& localPlayer, std::shared_ptr<cConnectionManager> connectionManager)
{
	newGame = std::make_shared<cNetworkClientGameNew>();

	newGame->setUnitsData(lobbyData.unitsData);
	newGame->setClanData(lobbyData.clanData);

	newGame->setPlayers (lobbyData.players, localPlayer);
	newGame->setGameSettings (lobbyData.gameSettings);
	newGame->setStaticMap (lobbyData.staticMap);
	newGame->setConnectionManager (connectionManager);

	initGamePreparation = std::make_unique<cInitGamePreparation> (application, lobbyClient);
	initGamePreparation->bindConnections (lobbyClient);
	initGamePreparation->startGamePreparation (lobbyData);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::startNewGame()
{
	if (!newGame) return;

	newGame->setLocalPlayerClan (initGamePreparation->getClan());
	newGame->setLocalPlayerLandingUnits (initGamePreparation->getLandingUnits());
	newGame->setLocalPlayerUnitUpgrades (initGamePreparation->getUnitUpgrades());
	newGame->setLocalPlayerLandingPosition (initGamePreparation->getLandingPosition());

	application.closeTill (*windowNetworkLobby);
	windowNetworkLobby->close();
	signalConnectionManager.connect (windowNetworkLobby->terminated, [&]() { windowNetworkLobby = nullptr; });

	newGame->start (application);

	signalConnectionManager.connect(newGame->getLocalClient().connectionToServerLost, [&]() { connectionLost = true; });
	signalConnectionManager.connect(newGame->terminated, [&]()
	{
		if (connectionLost)
		{
			reset();
			start();
			windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Lost_Connection", "server"));
		}
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::reconnectToGame (std::shared_ptr<cStaticMap> staticMap, std::shared_ptr<cConnectionManager> connectionManager, cPlayerBasicData localPlayer, const std::vector<cPlayerBasicData>& playerList)
{
	auto reconnectionGame = std::make_shared<cNetworkClientGameReconnection>();

	reconnectionGame->setConnectionManager(connectionManager);
	reconnectionGame->setStaticMap(staticMap);
	reconnectionGame->setPlayers(playerList, localPlayer);

	lobbyClient.wantToRejoinGame();

	application.closeTill(*windowNetworkLobby);
	windowNetworkLobby->close();
	signalConnectionManager.connect(windowNetworkLobby->terminated, [&]() { windowNetworkLobby = nullptr; });

	reconnectionGame->start(application);

	signalConnectionManager.connect(reconnectionGame->getLocalClient().connectionToServerLost, [&]() { connectionLost = true; });
	signalConnectionManager.connect(reconnectionGame->terminated, [&]()
	{
		if (connectionLost)
		{
			reset();
			start();
			windowNetworkLobby->addInfoEntry(lngPack.i18n("Text~Multiplayer~Lost_Connection", "server"));
		}
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::saveOptions()
{
	if (!windowNetworkLobby) return;

	cSettings::getInstance().setPlayerName (windowNetworkLobby->getLocalPlayer()->getName().c_str());
	cSettings::getInstance().setPort (windowNetworkLobby->getPort());
	cSettings::getInstance().setPlayerColor (windowNetworkLobby->getLocalPlayer()->getColor().getColor());
	cSettings::getInstance().setIP (windowNetworkLobby->getIp().c_str());
}
