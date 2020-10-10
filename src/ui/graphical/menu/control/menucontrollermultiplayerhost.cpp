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

#include "ui/graphical/menu/control/menucontrollermultiplayerhost.h"

#include "game/data/player/player.h"
#include "game/data/savegameinfo.h"
#include "game/data/savegame.h"
#include "game/data/units/landingunit.h"
#include "game/logic/server.h"
#include "game/startup/gamepreparation.h"
#include "game/startup/network/host/networkhostgamenew.h"
#include "game/startup/network/host/networkhostgamesaved.h"
#include "mapdownloader/mapuploadmessagehandler.h"
#include "ui/graphical/application.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/dialogs/dialogyesno.h"
#include "ui/graphical/menu/widgets/special/chatboxlandingplayerlistviewitem.h"
#include "ui/graphical/menu/widgets/special/lobbychatboxlistviewitem.h"
#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "ui/graphical/menu/windows/windowgamesettings/windowgamesettings.h"
#include "ui/graphical/menu/windows/windowlandingpositionselection/windowlandingpositionselection.h"
#include "ui/graphical/menu/windows/windowlandingunitselection/windowlandingunitselection.h"
#include "ui/graphical/menu/windows/windowload/windowload.h"
#include "ui/graphical/menu/windows/windowmapselection/windowmapselection.h"
#include "ui/graphical/menu/windows/windownetworklobbyhost/windownetworklobbyhost.h"
#include "utility/language.h"
#include "utility/log.h"

//------------------------------------------------------------------------------
cMenuControllerMultiplayerHost::cMenuControllerMultiplayerHost (cApplication& application_) :
	application (application_),
	lobbyServer (connectionManager),
	lobbyClient (connectionManager, cPlayerBasicData::fromSettings())
{
	signalConnectionManager.connect (lobbyServer.onMapRequested, [this](const cPlayerBasicData& player)
	{
		if (!windowNetworkLobby) return;
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_Upload", player.getName()));
	});

	signalConnectionManager.connect (lobbyServer.onMapUploaded, [this](const cPlayerBasicData& player)
	{
		if (!windowNetworkLobby) return;
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_UploadFinished", player.getName()));
	});

	signalConnectionManager.connect (lobbyClient.onChatMessage, [this](const std::string& playerName, bool translate, const std::string& message, const std::string& insertText){
		if (windowNetworkLobby != nullptr)
		{
			if (windowLandingPositionSelection)
			{
				if (translate)
				{
					windowLandingPositionSelection->getChatBox()->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (lngPack.i18n (message, insertText)));
				}
				else
				{
					windowLandingPositionSelection->getChatBox()->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (playerName, message));
					cSoundDevice::getInstance().playSoundEffect (SoundData.SNDChat);
				}
			}
			else
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
		}
	});

	signalConnectionManager.connect (lobbyClient.onStartGamePreparation, [this](const sLobbyPreparationData& lobbyData, const cPlayerBasicData& localPlayer, std::shared_ptr<cConnectionManager> connectionManager){
		windowLandingPositionSelection = nullptr;
		saveOptions();

		startGamePreparation (lobbyData, localPlayer, connectionManager);
	});

	signalConnectionManager.connect (lobbyClient.onPlayerAbortGamePreparation, [this](const std::string& playerName){
		if (windowNetworkLobby != nullptr)
		{
			auto okDialog = application.show (std::make_shared<cDialogOk> ("Player " + playerName + " has quit from game preparation")); // TODO: translate

			signalConnectionManager.connect (okDialog->done, [this]()
			{
				application.closeTill (*windowNetworkLobby);
			});
		}
	});

	signalConnectionManager.connect (lobbyClient.onPlayerEnterLeaveLandingSelectionRoom, [this](const cPlayerBasicData& player, bool isIn){
		if (isIn)
		{
			playersLandingStatus.push_back (std::make_unique<cPlayerLandingStatus> (player));
			if (windowLandingPositionSelection) windowLandingPositionSelection->getChatBox()->addPlayerEntry (std::make_unique<cChatBoxLandingPlayerListViewItem> (*playersLandingStatus.back()));
		}
		else
		{
			if (windowLandingPositionSelection) windowLandingPositionSelection->getChatBox()->removePlayerEntry (player.getNr());
			playersLandingStatus.erase (std::remove_if (playersLandingStatus.begin(), playersLandingStatus.end(), [&] (const std::unique_ptr<cPlayerLandingStatus>& status) { return status->getPlayer().getNr() == player.getNr(); }), playersLandingStatus.end());
		}
	});

	signalConnectionManager.connect (lobbyClient.onPlayerSelectLandingPosition, [this](const cPlayerBasicData& player){
		auto it = ranges::find_if (playersLandingStatus, [&] (const std::unique_ptr<cPlayerLandingStatus>& entry) { return entry->getPlayer().getNr() == player.getNr(); });

		if (it == playersLandingStatus.end()) return;

		auto& playerLandingStatus = **it;

		playerLandingStatus.setHasSelectedPosition (true);
	});

	signalConnectionManager.connect (lobbyClient.onLandingDone, [this](eLandingPositionState state){
		if (!windowLandingPositionSelection) return;

		windowLandingPositionSelection->applyReselectionState (state);
	});

	signalConnectionManager.connect (lobbyClient.onStartNewGame, [this](){
		if (!newGame) return;

		startNewGame();
	});

	signalConnectionManager.connect (lobbyClient.onStartSavedGame, [this](const cSaveGameInfo& saveGameInfo, std::shared_ptr<cStaticMap> staticMap, std::shared_ptr<cConnectionManager> connectionManager, cPlayerBasicData localPlayer){
		saveOptions();
		startSavedGame (saveGameInfo, staticMap, connectionManager, localPlayer);
	});
	// lobbyClient.onReconnectGame
	// lobbyClient.onFailToReconnectGameNoMap
	// lobbyClient.onFailToReconnectGameInvalidMap
	// Doesn't apply as client is directly connected to server

	lobbyClient.connectToLocalServer (lobbyServer);
}

//------------------------------------------------------------------------------
cMenuControllerMultiplayerHost::~cMenuControllerMultiplayerHost()
{}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::start()
{
	assert (windowNetworkLobby == nullptr); // should not be started twice
	windowNetworkLobby = std::make_shared<cWindowNetworkLobbyHost> ();

	application.show (windowNetworkLobby);
	application.addRunnable (shared_from_this());

	signalConnectionManager.connect (windowNetworkLobby->terminated, std::bind (&cMenuControllerMultiplayerHost::reset, this));
	signalConnectionManager.connect (windowNetworkLobby->backClicked, [this]()
	{
		windowNetworkLobby->close();
		saveOptions();
	});

	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectMap, [this](){ handleSelectMap(); });
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
	windowLandingPositionSelection = nullptr;
	newGame = nullptr;
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
	windowMapSelection->done.connect ([=]()
	{
		lobbyClient.selectMapName (windowMapSelection->getSelectedMapName());
		windowMapSelection->close();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleSelectSettings()
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
void cMenuControllerMultiplayerHost::handleSelectSaveGame()
{
	if (!windowNetworkLobby) return;

	auto windowLoad = application.show (std::make_shared<cWindowLoad>());
	windowLoad->load.connect ([=](const cSaveGameInfo& saveGame)
	{
		if (saveGame.number >= 0)
		{
			cStaticMap staticMap;
			if (!staticMap.loadMap (saveGame.mapName))
			{
				application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Text~Others~ERROR_Map_Loading")));
				return;
			}
			else if (MapDownload::calculateCheckSum(saveGame.mapName) != saveGame.mapCrc)
			{
				application.show (std::make_shared<cDialogOk> ("The map \"" + saveGame.mapName + "\" does not match the map the game was started with")); // TODO: translate
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
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Server_Not_Running"));
		return;
	}
	lobbyClient.askToFinishLobby (&lobbyServer);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startSavedGame (const cSaveGameInfo& saveGameInfo, std::shared_ptr<cStaticMap> staticMap, std::shared_ptr<cConnectionManager> connectionManager, cPlayerBasicData localPlayer)
{
	if (!windowNetworkLobby) return;
	auto savedGame = std::make_shared<cNetworkHostGameSaved> ();

	savedGame->setConnectionManager (connectionManager);
	savedGame->setSaveGameNumber(saveGameInfo.number);

	savedGame->setPlayers (saveGameInfo.players, localPlayer);

	try
	{
		savedGame->loadGameData();
	}
	catch (const std::runtime_error& e)
	{
		Log.write((std::string)"cMenuControllerMultiplayerHost: Error loading save game. " + e.what(), cLog::eLOG_TYPE_NET_ERROR);
		savedGame = nullptr;
		application.show(std::make_shared<cDialogOk>(lngPack.i18n("Text~Error_Messages~ERROR_Save_Loading")));
		return;
	}

	application.closeTill(*windowNetworkLobby);
	windowNetworkLobby->close();
	signalConnectionManager.connect(windowNetworkLobby->terminated, [&]() { windowNetworkLobby = nullptr; });

	savedGame->start (application);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startGamePreparation (const sLobbyPreparationData& lobbyData, const cPlayerBasicData& localPlayer, std::shared_ptr<cConnectionManager> connectionManager)
{
	newGame = std::make_shared<cNetworkHostGameNew>();

	newGame->setUnitsData(lobbyData.unitsData);
	newGame->setClanData(lobbyData.clanData);

	newGame->setPlayers (lobbyData.players, localPlayer);
	newGame->setGameSettings (lobbyData.gameSettings);
	newGame->setStaticMap (lobbyData.staticMap);
	newGame->setConnectionManager (connectionManager);

	if (newGame->getGameSettings()->getClansEnabled())
	{
		startClanSelection (true);
	}
	else
	{
		startLandingUnitSelection (true);
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startClanSelection(bool isFirstWindowOnGamePreparation)
{
	if (!newGame) return;

	auto windowClanSelection = application.show (std::make_shared<cWindowClanSelection> (newGame->getUnitsData(), newGame->getClanData()));

	signalConnectionManager.connect (windowClanSelection->canceled, [this, windowClanSelection, isFirstWindowOnGamePreparation]()
	{
		if (isFirstWindowOnGamePreparation)
		{
			checkReallyWantsToQuit();
		}
		else
		{
			windowClanSelection->close();
		}
	});
	signalConnectionManager.connect (windowClanSelection->done, [this, windowClanSelection]()
	{
		newGame->setLocalPlayerClan (windowClanSelection->getSelectedClan());

		startLandingUnitSelection (false);
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startLandingUnitSelection(bool isFirstWindowOnGamePreparation)
{
	if (!newGame || !newGame->getGameSettings()) return;

	auto initialLandingUnits = computeInitialLandingUnits (newGame->getLocalPlayerClan(), *newGame->getGameSettings(), *newGame->getUnitsData());

	auto windowLandingUnitSelection = application.show (std::make_shared<cWindowLandingUnitSelection> (cPlayerColor(), newGame->getLocalPlayerClan(), initialLandingUnits, newGame->getGameSettings()->getStartCredits(), newGame->getUnitsData()));

	signalConnectionManager.connect (windowLandingUnitSelection->canceled, [this, windowLandingUnitSelection, isFirstWindowOnGamePreparation]()
	{
		if (isFirstWindowOnGamePreparation)
		{
			checkReallyWantsToQuit();
		}
		else
		{
			windowLandingUnitSelection->close();
		}
	});
	signalConnectionManager.connect (windowLandingUnitSelection->done, [this, windowLandingUnitSelection]()
	{
		newGame->setLocalPlayerLandingUnits (windowLandingUnitSelection->getLandingUnits());
		newGame->setLocalPlayerUnitUpgrades (windowLandingUnitSelection->getUnitUpgrades());

		startLandingPositionSelection();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startLandingPositionSelection()
{
	if (!newGame || !newGame->getStaticMap()) return;
	auto& map = newGame->getStaticMap();
	bool fixedBridgeHead = newGame->getGameSettings()->getBridgeheadType() == eGameSettingsBridgeheadType::Definite;
	auto& landingUnits = newGame->getLandingUnits();
	auto unitsData = newGame->getUnitsData();
	windowLandingPositionSelection = std::make_shared<cWindowLandingPositionSelection> (map, fixedBridgeHead, landingUnits, unitsData, true);

	signalConnectionManager.connect (windowLandingPositionSelection->opened, [this]()
	{
		lobbyClient.enterLandingSelection();
	});
	signalConnectionManager.connect (windowLandingPositionSelection->closed, [this]()
	{
		lobbyClient.exitLandingSelection();
	});
	for (const auto& status : playersLandingStatus)
	{
		windowLandingPositionSelection->getChatBox()->addPlayerEntry (std::make_unique<cChatBoxLandingPlayerListViewItem> (*status));
	}

	application.show (windowLandingPositionSelection);

	signalConnectionManager.connect (windowLandingPositionSelection->canceled, [this]() { windowLandingPositionSelection->close(); });
	signalConnectionManager.connect (windowLandingPositionSelection->selectedPosition, [this] (cPosition landingPosition)
	{
		newGame->setLocalPlayerLandingPosition (landingPosition);

		lobbyClient.selectLandingPosition (landingPosition);
	});

	signalConnectionManager.connect (windowLandingPositionSelection->getChatBox()->commandEntered, [this] (const std::string& text)
	{
		const auto& localPlayer = newGame->getLocalPlayer();
		windowLandingPositionSelection->getChatBox()->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (localPlayer.getName(), text));
		cSoundDevice::getInstance().playSoundEffect (SoundData.SNDChat);
		lobbyClient.sendChatMessage (text);
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startNewGame()
{
	if (!newGame) return;

	application.closeTill (*windowNetworkLobby);
	windowNetworkLobby->close();
	signalConnectionManager.connect (windowNetworkLobby->terminated, [&]() { windowNetworkLobby = nullptr; });

	newGame->start (application);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::checkReallyWantsToQuit()
{
	auto yesNoDialog = application.show (std::make_shared<cDialogYesNo> ("Are you sure you want to abort the game preparation?")); // TODO: translate

	signalConnectionManager.connect(yesNoDialog->yesClicked, [this]()
	{
		lobbyClient.abortGamePreparation();
		application.closeTill(*windowNetworkLobby);
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startHost()
{
	if (!connectionManager || !windowNetworkLobby) return;

	if (connectionManager->isServerOpen()) return;

	if (connectionManager->openServer(windowNetworkLobby->getPort()))
	{
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Error_Socket"));
		Log.write ("Error opening socket", cLog::eLOG_TYPE_WARNING);
	}
	else
	{
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Open") + " (" + lngPack.i18n ("Text~Title~Port") + lngPack.i18n ("Text~Punctuation~Colon")  + iToStr (windowNetworkLobby->getPort()) + ")");
		Log.write ("Game open (Port: " + iToStr (windowNetworkLobby->getPort()) + ")", cLog::eLOG_TYPE_INFO);
		windowNetworkLobby->disablePortEdit();
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::saveOptions()
{
	if (!windowNetworkLobby) return;

	cSettings::getInstance().setPlayerName (windowNetworkLobby->getLocalPlayer()->getName().c_str());
	cSettings::getInstance().setPort (windowNetworkLobby->getPort());
	cSettings::getInstance().setPlayerColor (windowNetworkLobby->getLocalPlayer()->getColor().getColor());
}
