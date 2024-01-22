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

#include "menucontrollermultiplayerhost.h"

#include "game/data/player/player.h"
#include "game/data/savegameinfo.h"
#include "game/data/units/landingunit.h"
#include "game/logic/server.h"
#include "game/startup/gamepreparation.h"
#include "mapdownloader/mapuploadmessagehandler.h"
#include "ui/widgets/application.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "ui/graphical/menu/control/network/networkgame.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/dialogs/dialogyesno.h"
#include "ui/graphical/menu/widgets/special/chatboxlandingplayerlistviewitem.h"
#include "ui/graphical/menu/widgets/special/lobbychatboxlistviewitem.h"
#include "ui/graphical/menu/windows/windowgamesettings/windowgamesettings.h"
#include "ui/graphical/menu/windows/windowload/windowload.h"
#include "ui/graphical/menu/windows/windowmapselection/windowmapselection.h"
#include "ui/graphical/menu/windows/windownetworklobbyhost/windownetworklobbyhost.h"
#include "utility/language.h"
#include "utility/log.h"

#include <cassert>

//------------------------------------------------------------------------------
cMenuControllerMultiplayerHost::cMenuControllerMultiplayerHost (cApplication& application_) :
	application (application_),
	lobbyServer (connectionManager),
	lobbyClient (connectionManager, cPlayerBasicData::fromSettings())
{
	signalConnectionManager.connect (lobbyServer.onMapRequested, [this] (const cPlayerBasicData& player) {
		if (!windowNetworkLobby) return;
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Multiplayer~MapDL_Upload", player.getName()));
	});

	signalConnectionManager.connect (lobbyServer.onMapUploaded, [this] (const cPlayerBasicData& player) {
		if (!windowNetworkLobby) return;
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Multiplayer~MapDL_UploadFinished", player.getName()));
	});

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
		saveOptions();

		startGamePreparation();
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
		saveOptions();
		startSavedGame (client);
	});
	// lobbyClient.onReconnectGame
	// lobbyClient.onFailToReconnectGameNoMap
	// lobbyClient.onFailToReconnectGameInvalidMap
	// Doesn't apply as client is directly connected to server

	// lobbyServer.onStartNewGame
	// lobbyServer.onStartSavedGame
	// Handled client side.

	signalConnectionManager.connect (lobbyServer.onErrorLoadSavedGame, [this] (int /*slot*/) {
		application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Error_Messages~ERROR_Save_Loading")));
	});
	lobbyClient.connectToLocalServer (lobbyServer);
}

//------------------------------------------------------------------------------
cMenuControllerMultiplayerHost::~cMenuControllerMultiplayerHost()
{}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::start()
{
	assert (windowNetworkLobby == nullptr); // should not be started twice
	windowNetworkLobby = std::make_shared<cWindowNetworkLobbyHost>();

	application.show (windowNetworkLobby);
	application.addRunnable (shared_from_this());

	signalConnectionManager.connect (windowNetworkLobby->terminated, [this]() { reset(); });
	signalConnectionManager.connect (windowNetworkLobby->backClicked, [this]() {
		windowNetworkLobby->close();
		saveOptions();
	});

	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectMap, [this]() { handleSelectMap(); });
	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectSettings, [this]() { handleSelectSettings(); });
	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectSaveGame, [this]() { handleSelectSaveGame(); });

	signalConnectionManager.connect (windowNetworkLobby->triggeredStartHost, [this]() { startHost(); });
	signalConnectionManager.connect (windowNetworkLobby->triggeredStartGame, [this]() { checkGameStart(); });

	windowNetworkLobby->bindConnections (lobbyClient);
	windowNetworkLobby->bindConnections (lobbyServer);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::reset()
{
	windowNetworkLobby = nullptr;
	application.removeRunnable (shared_from_this());
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::run()
{
	lobbyClient.run();
	lobbyServer.run();
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleSelectMap()
{
	auto windowMapSelection = application.show (std::make_shared<cWindowMapSelection>());
	windowMapSelection->done.connect ([=] (const std::filesystem::path& mapFilename) {
		lobbyClient.selectMapFilename (mapFilename);
		windowMapSelection->close();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleSelectSettings()
{
	if (!windowNetworkLobby) return;

	auto windowGameSettings = application.show (std::make_shared<cWindowGameSettings>());
	windowGameSettings->initFor (lobbyClient);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleSelectSaveGame()
{
	if (!windowNetworkLobby) return;

	auto windowLoad = application.show (std::make_shared<cWindowLoad>());
	windowLoad->load.connect ([=] (const cSaveGameInfo& saveGame) {
		if (saveGame.number >= 0)
		{
			cStaticMap staticMap;
			if (!staticMap.loadMap (saveGame.mapFilename))
			{
				application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Error_Messages~ERROR_Map_Loading")));
				return;
			}
			else if (MapDownload::calculateCheckSum (saveGame.mapFilename) != saveGame.mapCrc)
			{
				application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Error_Messages~ERROR_Map_Checksum", saveGame.mapFilename.u8string())));
				return;
			}
		}
		lobbyClient.selectLoadGame (saveGame);
		windowLoad->close();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::checkGameStart()
{
	if (!connectionManager->isServerOpen())
	{
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Multiplayer~Server_Not_Running"));
		return;
	}
	lobbyClient.askToFinishLobby();
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startSavedGame (std::shared_ptr<cClient> client)
{
	if (!windowNetworkLobby) return;

	application.closeTill (*windowNetworkLobby);
	windowNetworkLobby->close();
	signalConnectionManager.connect (windowNetworkLobby->terminated, [this]() { windowNetworkLobby = nullptr; });

	auto* server = lobbyServer.getServer();
	assert (server);

	auto savedGame = std::make_shared<cNetworkGame>();
	savedGame->start (application, client, server);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startGamePreparation()
{
	initGamePreparation = std::make_unique<cInitGamePreparation> (application, lobbyClient);

	initGamePreparation->bindConnections (lobbyClient);
	initGamePreparation->startGamePreparation();
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startNewGame (std::shared_ptr<cClient> client)
{
	application.closeTill (*windowNetworkLobby);
	windowNetworkLobby->close();
	signalConnectionManager.connect (windowNetworkLobby->terminated, [this]() { windowNetworkLobby = nullptr; });

	const auto& initPlayerData = initGamePreparation->getInitPlayerData();

	auto* server = lobbyServer.getServer();
	assert (server);
	auto newGame = std::make_shared<cNetworkGame>();
	newGame->startNewGame (application, client, initPlayerData, server);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startHost()
{
	if (!windowNetworkLobby) return;
	switch (lobbyServer.startServer (windowNetworkLobby->getPort()))
	{
		case eOpenServerResult::AlreadyOpened: return;
		case eOpenServerResult::Success:
		{
			windowNetworkLobby->addInfoEntry (lngPack.i18n ("Multiplayer~Network_Open", std::to_string (windowNetworkLobby->getPort())));
			windowNetworkLobby->disablePortEdit();
			break;
		}
		case eOpenServerResult::Failed:
		{
			windowNetworkLobby->addInfoEntry (lngPack.i18n ("Multiplayer~Network_Error_Socket"));
			break;
		}
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::saveOptions()
{
	if (!windowNetworkLobby) return;

	cSettings::getInstance().setNetworkAddress ({cSettings::getInstance().getNetworkAddress().ip, windowNetworkLobby->getPort()});
	cSettings::getInstance().setPlayerSettings ({windowNetworkLobby->getLocalPlayer()->getName(), windowNetworkLobby->getLocalPlayer()->getColor()});
	cSettings::getInstance().saveInFile();
}
