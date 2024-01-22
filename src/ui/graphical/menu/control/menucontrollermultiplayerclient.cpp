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

#include "menucontrollermultiplayerclient.h"

#include "game/data/units/landingunit.h"
#include "game/logic/client.h"
#include "game/startup/gamepreparation.h"
#include "ui/widgets/application.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "ui/graphical/menu/control/network/networkgame.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/dialogs/dialogyesno.h"
#include "ui/graphical/menu/windows/windowgamesettings/windowgamesettings.h"
#include "ui/graphical/menu/windows/windowload/windowload.h"
#include "ui/graphical/menu/windows/windowmapselection/windowmapselection.h"
#include "ui/graphical/menu/windows/windownetworklobbyclient/windownetworklobbyclient.h"
#include "utility/language.h"
#include "utility/log.h"

//------------------------------------------------------------------------------
cMenuControllerMultiplayerClient::cMenuControllerMultiplayerClient (cApplication& application_) :
	application (application_),
	lobbyClient (std::make_shared<cConnectionManager>(), cPlayerBasicData::fromSettings())
{
	signalConnectionManager.connect (lobbyClient.onChatMessage, [this] (const std::string& playerName, const std::string& message) {
		if (initGamePreparation)
		{
			initGamePreparation->onChatMessage (playerName, message);
		}
		else if (windowNetworkLobby != nullptr)
		{
			windowNetworkLobby->addChatEntry (playerName, message);
		}
	});

	signalConnectionManager.connect (lobbyClient.onStartGamePreparation, [this]() {
		if (windowNetworkLobby != nullptr)
		{
			saveOptions();

			startGamePreparation();
		}
	});

	signalConnectionManager.connect (lobbyClient.onPlayerAbortGamePreparation, [this] (const std::string& playerName) {
		if (windowNetworkLobby != nullptr)
		{
			auto okDialog = application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Multiplayer~Player_Left_Game_Preparation", playerName)));

			signalConnectionManager.connect (okDialog->done, [this]() {
				application.closeTill (*windowNetworkLobby);
				initGamePreparation.reset();
			});
		}
	});

	signalConnectionManager.connect (lobbyClient.onStartNewGame, [this] (std::shared_ptr<cClient> client) {
		startNewGame (client);
	});

	signalConnectionManager.connect (lobbyClient.onStartSavedGame, [this] (std::shared_ptr<cClient> client) {
		startSavedGame (client);
	});

	signalConnectionManager.connect (lobbyClient.onReconnectGame, [this] (std::shared_ptr<cClient> client) {
		auto yesNoDialog = application.show (std::make_shared<cDialogYesNo> (lngPack.i18n ("Multiplayer~Reconnect")));
		signalConnectionManager.connect (yesNoDialog->yesClicked, [=]() {
			reconnectToGame (client);
		});

		signalConnectionManager.connect (yesNoDialog->noClicked, [this]() {
			lobbyClient.disconnect();
		});
	});

	signalConnectionManager.connect (lobbyClient.onFailToReconnectGameNoMap, [this] (const std::filesystem::path& mapFilename) {
		application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Error_Messages~Map_Not_Found", mapFilename.u8string())));
	});
	signalConnectionManager.connect (lobbyClient.onFailToReconnectGameInvalidMap, [this] (const std::filesystem::path& mapFilename) {
		application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Error_Messages~Invalid_Map", mapFilename.u8string())));
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

	windowNetworkLobby = std::make_shared<cWindowNetworkLobbyClient>();

	windowNetworkLobby->bindConnections (lobbyClient);

	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectMap, [this]() {
		handleSelectMap();
	});
	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectSettings, [this]() {
		handleSelectSettings();
	});
	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectSaveGame, [this]() {
		handleSelectSaveGame();
	});

	application.show (windowNetworkLobby);
	application.addRunnable (shared_from_this());

	signalConnectionManager.connect (windowNetworkLobby->terminated, [this]() { reset(); });
	signalConnectionManager.connect (windowNetworkLobby->triggeredStartGame, [this]() {
		handleStartGame();
	});
	signalConnectionManager.connect (windowNetworkLobby->backClicked, [this]() {
		windowNetworkLobby->close();
		saveOptions();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::reset()
{
	windowNetworkLobby = nullptr;
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
	windowGameSettings->initFor (lobbyClient);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleSelectMap()
{
	if (!windowNetworkLobby) return;

	auto windowMapSelection = application.show (std::make_shared<cWindowMapSelection>());
	windowMapSelection->done.connect ([=] (const std::filesystem::path& mapFilename) {
		lobbyClient.selectMapFilename (mapFilename);
		windowMapSelection->close();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleSelectSaveGame()
{
	if (!windowNetworkLobby) return;

	auto windowLoad = application.show (std::make_shared<cWindowLoad> (nullptr, [this]() {
		return lobbyClient.getSaveGames();
	}));
	windowLoad->load.connect ([=] (const cSaveGameInfo& saveGame) {
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
void cMenuControllerMultiplayerClient::startSavedGame (std::shared_ptr<cClient> client)
{
	if (!windowNetworkLobby) return;

	application.closeTill (*windowNetworkLobby);
	windowNetworkLobby->close();
	signalConnectionManager.connect (windowNetworkLobby->terminated, [this]() {
		windowNetworkLobby = nullptr;
	});

	auto savedGame = std::make_shared<cNetworkGame>();
	savedGame->start (application, client, nullptr);

	signalConnectionManager.connect (client->connectionToServerLost, [this]() {
		connectionLost = true;
	});
	signalConnectionManager.connect (savedGame->terminated, [this]() {
		if (connectionLost)
		{
			reset();
			start();
			windowNetworkLobby->addInfoEntry (lngPack.i18n ("Multiplayer~Lost_Connection", "server"));
		}
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::startGamePreparation()
{
	initGamePreparation = std::make_unique<cInitGamePreparation> (application, lobbyClient);
	initGamePreparation->bindConnections (lobbyClient);
	initGamePreparation->startGamePreparation();
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::startNewGame (std::shared_ptr<cClient> client)
{
	const auto& initPlayerData = initGamePreparation->getInitPlayerData();
	application.closeTill (*windowNetworkLobby);
	windowNetworkLobby->close();
	signalConnectionManager.connect (windowNetworkLobby->terminated, [this]() {
		windowNetworkLobby = nullptr;
	});

	auto newGame = std::make_shared<cNetworkGame>();
	newGame->startNewGame (application, client, initPlayerData, nullptr);

	signalConnectionManager.connect (client->connectionToServerLost, [this]() {
		connectionLost = true;
	});
	signalConnectionManager.connect (newGame->terminated, [this]() {
		if (connectionLost)
		{
			reset();
			start();
			windowNetworkLobby->addInfoEntry (lngPack.i18n ("Multiplayer~Lost_Connection", "server"));
		}
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::reconnectToGame (std::shared_ptr<cClient> client)
{
	application.closeTill (*windowNetworkLobby);
	windowNetworkLobby->close();
	signalConnectionManager.connect (windowNetworkLobby->terminated, [this]() {
		windowNetworkLobby = nullptr;
	});

	auto reconnectionGame = std::make_shared<cNetworkGame>();
	reconnectionGame->start (application, client, nullptr);

	signalConnectionManager.connect (client->connectionToServerLost, [this]() {
		connectionLost = true;
	});
	signalConnectionManager.connect (reconnectionGame->terminated, [this]() {
		if (connectionLost)
		{
			reset();
			start();
			windowNetworkLobby->addInfoEntry (lngPack.i18n ("Multiplayer~Lost_Connection", "server"));
		}
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::saveOptions()
{
	if (!windowNetworkLobby) return;

	cSettings::getInstance().setNetworkAddress ({windowNetworkLobby->getIp(), windowNetworkLobby->getPort()});
	cSettings::getInstance().setPlayerSettings ({windowNetworkLobby->getLocalPlayer()->getName(), windowNetworkLobby->getLocalPlayer()->getColor()});
	cSettings::getInstance().saveInFile();
}
