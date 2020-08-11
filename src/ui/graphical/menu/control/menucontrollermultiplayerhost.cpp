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

#include "ui/graphical/application.h"
#include "ui/graphical/menu/windows/windownetworklobbyhost/windownetworklobbyhost.h"
#include "game/data/gamesettings.h"
#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "ui/graphical/menu/windows/windowlandingunitselection/windowlandingunitselection.h"
#include "ui/graphical/menu/windows/windowlandingpositionselection/windowlandingpositionselection.h"
#include "ui/graphical/menu/windows/windowgamesettings/windowgamesettings.h"
#include "ui/graphical/menu/windows/windowmapselection/windowmapselection.h"
#include "ui/graphical/menu/windows/windowload/windowload.h"
#include "ui/graphical/menu/widgets/special/lobbychatboxlistviewitem.h"
#include "ui/graphical/menu/widgets/special/chatboxlandingplayerlistviewitem.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/dialogs/dialogyesno.h"
#include "game/startup/network/host/networkhostgamenew.h"
#include "game/startup/network/host/networkhostgamesaved.h"
#include "maxrversion.h"
#include "game/data/map/map.h"
#include "game/data/player/player.h"
#include "game/data/units/landingunit.h"
#include "utility/language.h"
#include "utility/log.h"
#include "protocol/lobbymessage.h"
#include "mapdownload.h"
#include "game/data/savegame.h"
#include "game/logic/client.h"
#include "game/logic/server2.h"
#include "game/data/savegameinfo.h"
#include "utility/string/toString.h"
#include "game/logic/action/action.h"

// TODO: remove
std::vector<std::pair<sID, int>> createInitialLandingUnitsList(int clan, const cGameSettings& gameSettings, const cUnitsData& unitsData); // defined in windowsingleplayer.cpp

//------------------------------------------------------------------------------
cMenuControllerMultiplayerHost::cMenuControllerMultiplayerHost (cApplication& application_) :
	application (application_),
	nextPlayerNumber (1)
{}

//------------------------------------------------------------------------------
cMenuControllerMultiplayerHost::~cMenuControllerMultiplayerHost()
{}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::start()
{
	assert (windowNetworkLobby == nullptr); // should not be started twice

	connectionManager = std::make_shared<cConnectionManager> ();

	connectionManager->setLocalServer (this);

	windowNetworkLobby = std::make_shared<cWindowNetworkLobbyHost> ();

	triedLoadMapName = "";
	nextPlayerNumber = windowNetworkLobby->getLocalPlayer()->getNr() + 1;

	application.show (windowNetworkLobby);
	application.addRunnable (shared_from_this());

	signalConnectionManager.connect (windowNetworkLobby->terminated, std::bind (&cMenuControllerMultiplayerHost::reset, this));
	signalConnectionManager.connect (windowNetworkLobby->backClicked, [this]()
	{
		windowNetworkLobby->close();
		saveOptions();
	});

	signalConnectionManager.connect (windowNetworkLobby->wantLocalPlayerReadyChange, std::bind (&cMenuControllerMultiplayerHost::handleWantLocalPlayerReadyChange, this));
	signalConnectionManager.connect (windowNetworkLobby->triggeredChatMessage, std::bind (&cMenuControllerMultiplayerHost::handleChatMessageTriggered, this));

	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer()->nameChanged, std::bind (&cMenuControllerMultiplayerHost::handleLocalPlayerAttributesChanged, this));
	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer()->colorChanged, std::bind (&cMenuControllerMultiplayerHost::handleLocalPlayerAttributesChanged, this));
	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer()->readyChanged, std::bind (&cMenuControllerMultiplayerHost::handleLocalPlayerAttributesChanged, this));

	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectMap, std::bind (&cMenuControllerMultiplayerHost::handleSelectMap, this, std::ref (application)));
	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectSettings, std::bind (&cMenuControllerMultiplayerHost::handleSelectSettings, this, std::ref (application)));
	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectSaveGame, std::bind (&cMenuControllerMultiplayerHost::handleSelectSaveGame, this, std::ref (application)));

	signalConnectionManager.connect (windowNetworkLobby->triggeredStartHost, std::bind (&cMenuControllerMultiplayerHost::startHost, this));
	signalConnectionManager.connect (windowNetworkLobby->triggeredStartGame, std::bind (&cMenuControllerMultiplayerHost::checkGameStart, this));

	signalConnectionManager.connect(windowNetworkLobby->staticMapChanged, [this]() {sendGameData(); });
	signalConnectionManager.connect(windowNetworkLobby->gameSettingsChanged, [this]() {sendGameData(); });
	signalConnectionManager.connect(windowNetworkLobby->saveGameChanged, [this]() {sendGameData(); });
}

void cMenuControllerMultiplayerHost::sendGameData(int playerNr /* = -1 */)
{
	cMuMsgOptions message;
	message.saveInfo = windowNetworkLobby->getSaveGameInfo();
	
	const cStaticMap* map = windowNetworkLobby->getStaticMap().get();
	if (map)
	{
		message.mapName = map->getName();
		message.mapCrc = MapDownload::calculateCheckSum(map->getName());
	}
	const cGameSettings* settings = windowNetworkLobby->getGameSettings().get();
	if (settings)
	{
		message.settings = *settings;
		message.settingsValid = true;
	}
	sendNetMessage(message, playerNr);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::reset()
{
	connectionManager->setLocalServer(nullptr);
	connectionManager = nullptr;
	windowNetworkLobby = nullptr;
	windowLandingPositionSelection = nullptr;
	newGame = nullptr;
	application.removeRunnable (shared_from_this());
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::pushMessage (std::unique_ptr<cNetMessage2> message)
{
	messageQueue.push (std::move (message));
}

//------------------------------------------------------------------------------
std::unique_ptr<cNetMessage2> cMenuControllerMultiplayerHost::popMessage()
{
	std::unique_ptr<cNetMessage2> message;
	messageQueue.try_pop(message);
	return message;
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::run()
{
	std::unique_ptr<cNetMessage2> message;
	while (messageQueue.try_pop (message))
	{
		handleNetMessage (*message);
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleSelectMap (cApplication& application)
{
	if (!windowNetworkLobby) return;

	auto windowMapSelection = application.show (std::make_shared<cWindowMapSelection> ());
	windowMapSelection->done.connect ([&, windowMapSelection]()
	{
		auto staticMap = std::make_shared<cStaticMap> ();
		if (windowMapSelection->loadSelectedMap (*staticMap))
		{
			windowNetworkLobby->setStaticMap (std::move (staticMap));
			windowMapSelection->close();
		}
		else
		{
			application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Text~Others~ERROR_Map_Loading")));
		}
		triedLoadMapName = windowMapSelection->getSelectedMapName();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleSelectSettings (cApplication& application)
{
	if (!windowNetworkLobby) return;

	auto windowGameSettings = application.show (std::make_shared<cWindowGameSettings> ());

	if (windowNetworkLobby->getGameSettings())	windowGameSettings->applySettings (*windowNetworkLobby->getGameSettings());
	else windowGameSettings->applySettings (cGameSettings());

	windowGameSettings->done.connect ([&, windowGameSettings]()
	{
		windowNetworkLobby->setGameSettings (std::make_unique<cGameSettings> (windowGameSettings->getGameSettings()));
		windowGameSettings->close();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleSelectSaveGame (cApplication& application)
{
	if (!windowNetworkLobby) return;

	auto windowLoad = application.show (std::make_shared<cWindowLoad> ());
	windowLoad->load.connect ([&, windowLoad] (const cSaveGameInfo& saveInfo)
	{
		if (windowNetworkLobby->setSaveGame (saveInfo, &application))
		{
			windowLoad->close();
		}
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleWantLocalPlayerReadyChange()
{
	if (!connectionManager || !windowNetworkLobby) return;

	auto& localPlayer = windowNetworkLobby->getLocalPlayer();

	if (!localPlayer) return;

	if (!windowNetworkLobby->getStaticMap() && !triedLoadMapName.empty())
	{
		if (!localPlayer->isReady()) windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~No_Map_No_Ready", triedLoadMapName));
		localPlayer->setReady (false);
	}
	else localPlayer->setReady (!localPlayer->isReady());
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleChatMessageTriggered()
{
	if (!connectionManager || !windowNetworkLobby) return;

	const auto& chatMessage = windowNetworkLobby->getChatMessage();

	if (chatMessage.empty()) return;

	auto& localPlayer = windowNetworkLobby->getLocalPlayer();

	if (localPlayer)
	{
		windowNetworkLobby->addChatEntry (localPlayer->getName(), chatMessage);
		sendNetMessage(cMuMsgChat(chatMessage));
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleLocalPlayerAttributesChanged()
{
	if (!connectionManager || !windowNetworkLobby) return;

	checkTakenPlayerAttributes (*windowNetworkLobby->getLocalPlayer());
	sendNetMessage (cMuMsgPlayerList(windowNetworkLobby->getPlayers()));
}

void cMenuControllerMultiplayerHost::checkTakenPlayerAttributes (cPlayerBasicData& player)
{
	if (!connectionManager || !windowNetworkLobby) return;

	if (!player.isReady()) return;

	auto players = windowNetworkLobby->getPlayers();
	const auto& localPlayer = windowNetworkLobby->getLocalPlayer();

	const double colorDeltaETolerance = 10;

	for (size_t i = 0; i != players.size(); ++i)
	{
		if (players[i].get() == &player) continue;
		if (players[i]->getName() == player.getName())
		{
			if (player.getNr() != localPlayer->getNr()) sendNetMessage(cMuMsgChat("Text~Multiplayer~Player_Name_Taken", true), player.getNr());
			else windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Player_Name_Taken"));
			player.setReady (false);
			break;
		}
		if (players[i]->getColor().getColor().toLab().deltaE (player.getColor().getColor().toLab()) < colorDeltaETolerance)
		{
			if (player.getNr() != localPlayer->getNr()) sendNetMessage (cMuMsgChat("Text~Multiplayer~Player_Color_Taken", true), player.getNr());
			else windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Player_Color_Taken"));
			player.setReady (false);
			break;
		}
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::checkGameStart()
{
	if (!connectionManager || !windowNetworkLobby) return;

	if (!windowNetworkLobby->getStaticMap() || (!windowNetworkLobby->getGameSettings() && windowNetworkLobby->getSaveGameInfo().number < 0))
	{
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Missing_Settings"));
		return;
	}

	auto players = windowNetworkLobby->getPlayers();
	auto notReadyPlayerIter = std::find_if (players.begin(), players.end(), [ ] (const std::shared_ptr<cPlayerBasicData>& player) { return !player->isReady(); });

	if (notReadyPlayerIter != players.end())
	{
		windowNetworkLobby->addInfoEntry ((*notReadyPlayerIter)->getName() + " " + lngPack.i18n ("Text~Multiplayer~Not_Ready"));
		return;
	}

	if (!connectionManager->isServerOpen())
	{
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Server_Not_Running"));
		return;
	}

	const cSaveGameInfo& saveInfo = windowNetworkLobby->getSaveGameInfo();
	if (saveInfo.number > -1)
	{
		auto savegamePlayers = saveInfo.players;
		auto menuPlayers = windowNetworkLobby->getPlayers();

		// check whether all necessary players are connected
		for (size_t i = 0; i < savegamePlayers.size(); ++i)
		{
			auto iter = std::find_if (menuPlayers.begin(), menuPlayers.end(), [&] (const std::shared_ptr<cPlayerBasicData>& player) { return player->getName() == savegamePlayers[i].getName(); });
			if (iter == menuPlayers.end() && !savegamePlayers[i].isDefeated())
			{
				windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Player_Wrong"));
				return;
			}
		}

		//check host is part of the game
		{
			auto hostPlayer = windowNetworkLobby->getLocalPlayer();
			auto iter = std::find_if(savegamePlayers.begin(), savegamePlayers.end(), [&](const cPlayerBasicData & player) { return player.getName() == hostPlayer->getName(); });
			if (iter == savegamePlayers.end())
			{
				windowNetworkLobby->addInfoEntry("Unable to start: Host must be part of the saved game"); //TODO: translate
				return;
			}
		}

		// disconnect or update menu players
		for (auto i = menuPlayers.begin(); i < menuPlayers.end();)
		{
			auto menuPlayer = *i;
			auto iter = std::find_if (savegamePlayers.begin(), savegamePlayers.end(), [&] (const cPlayerBasicData & player) { return player.getName() == menuPlayer->getName(); });

			if (iter == savegamePlayers.end())
			{
				// the player does not belong to the save game: disconnect him
				sendNetMessage(cMuMsgChat("Text~Multiplayer~Disconnect_Not_In_Save", true), menuPlayer->getNr());
				connectionManager->disconnect(menuPlayer->getNr());

				i = menuPlayers.erase (i);
			}
			else
			{
				auto& savegamePlayer = *iter;
				int newPlayerNr = savegamePlayer.getNr();
				int oldPlayerNr = menuPlayer->getNr();

				menuPlayer->setNr (newPlayerNr);
				menuPlayer->setColor (savegamePlayer.getColor());
				if (windowNetworkLobby->getLocalPlayer()->getNr() != newPlayerNr)
				{
					sendNetMessage (cMuMsgPlayerNr(newPlayerNr), oldPlayerNr);
					connectionManager->changePlayerNumber(oldPlayerNr, newPlayerNr);
				}
				++i;
			}
		}

		sendNetMessage(cMuMsgPlayerList(menuPlayers));

		sendGameData();
		saveOptions();

		startSavedGame();

	}
	else
	{
		saveOptions();


		startGamePreparation();
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startSavedGame()
{
	if (!windowNetworkLobby || windowNetworkLobby->getSaveGameInfo().number == -1) return;
	auto savedGame = std::make_shared<cNetworkHostGameSaved> ();

	savedGame->setConnectionManager (connectionManager);
	savedGame->setSaveGameNumber(windowNetworkLobby->getSaveGameInfo().number);

	savedGame->setPlayers (windowNetworkLobby->getSaveGameInfo().players, *windowNetworkLobby->getLocalPlayer());

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

	sendNetMessage(cMuMsgStartGame());

	application.closeTill(*windowNetworkLobby);
	windowNetworkLobby->close();
	signalConnectionManager.connect(windowNetworkLobby->terminated, [&]() { windowNetworkLobby = nullptr; });

	savedGame->start(application);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startGamePreparation()
{
	const auto& staticMap = windowNetworkLobby->getStaticMap();
	const auto& gameSettings = windowNetworkLobby->getGameSettings();

	if (!staticMap || !gameSettings || !connectionManager) return;

	newGame = std::make_shared<cNetworkHostGameNew> ();

	//initialize copy of unitsData that will be used in game
	auto unitsData = std::make_shared<const cUnitsData>(UnitsDataGlobal);
	newGame->setUnitsData(unitsData);
	auto clanData = std::make_shared<const cClanData>(ClanDataGlobal);
	newGame->setClanData(clanData);


	newGame->setPlayers (windowNetworkLobby->getPlayersNotShared(), *windowNetworkLobby->getLocalPlayer());
	newGame->setGameSettings (gameSettings);
	newGame->setStaticMap (staticMap);
	newGame->setConnectionManager (connectionManager);

	sendNetMessage(cMuMsgStartGamePreparations(unitsData, clanData));

	landingPositionManager = std::make_shared<cLandingPositionManager> (newGame->getPlayers());

	signalConnectionManager.connect (landingPositionManager->landingPositionSet, [this] (const cPlayerBasicData & player, const cPosition & position)
	{
		auto iter = std::find_if (playersLandingStatus.begin(), playersLandingStatus.end(), [&] (const std::unique_ptr<cPlayerLandingStatus>& entry) { return entry->getPlayer().getNr() == player.getNr(); });
		assert (iter != playersLandingStatus.end());

		auto& entry = **iter;

		const auto hadSelectedPosition = entry.hasSelectedPosition();

		entry.setHasSelectedPosition (true);

		if (entry.hasSelectedPosition() && !hadSelectedPosition)
		{
			sendNetMessage (cMuMsgPlayerHasSelectedLandingPosition(entry.getPlayer().getNr()));
		}
	});

	if (newGame->getGameSettings()->getClansEnabled())
	{
		startClanSelection(true);
	}
	else
	{
		startLandingUnitSelection(true);
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startClanSelection(bool isFirstWindowOnGamePreparation)
{
	if (!newGame) return;

	auto windowClanSelection = application.show (std::make_shared<cWindowClanSelection> (newGame->getUnitsData(), newGame->getClanData()));

	signalConnectionManager.connect (windowClanSelection->canceled, [this, windowClanSelection, isFirstWindowOnGamePreparation]()
	{
		if(isFirstWindowOnGamePreparation)
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

		startLandingUnitSelection(false);
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startLandingUnitSelection(bool isFirstWindowOnGamePreparation)
{
	if (!newGame || !newGame->getGameSettings()) return;

	auto initialLandingUnits = createInitialLandingUnitsList (newGame->getLocalPlayerClan(), *newGame->getGameSettings(), *newGame->getUnitsData());

	auto windowLandingUnitSelection = application.show (std::make_shared<cWindowLandingUnitSelection> (cPlayerColor(), newGame->getLocalPlayerClan(), initialLandingUnits, newGame->getGameSettings()->getStartCredits(), newGame->getUnitsData()));

	signalConnectionManager.connect (windowLandingUnitSelection->canceled, [this, windowLandingUnitSelection, isFirstWindowOnGamePreparation]()
	{
		if(isFirstWindowOnGamePreparation)
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
	if (!newGame || !newGame->getStaticMap() || !connectionManager) return;

	auto& map = newGame->getStaticMap();
	bool fixedBridgeHead = newGame->getGameSettings()->getBridgeheadType() == eGameSettingsBridgeheadType::Definite;
	auto& landingUnits = newGame->getLandingUnits();
	auto unitsData = newGame->getUnitsData();
	windowLandingPositionSelection = std::make_shared<cWindowLandingPositionSelection>(map, fixedBridgeHead, landingUnits, unitsData, true);

	application.show (windowLandingPositionSelection);

	signalConnectionManager.connect (windowLandingPositionSelection->opened, [this]()
	{
		const auto& localPlayer = newGame->getLocalPlayer();

		playersLandingStatus.push_back (std::make_unique<cPlayerLandingStatus> (localPlayer));
		windowLandingPositionSelection->getChatBox()->addPlayerEntry (std::make_unique<cChatBoxLandingPlayerListViewItem> (*playersLandingStatus.back()));

		sendNetMessage (cMuMsgInLandingPositionSelectionStatus(localPlayer.getNr(), true));
	});
	signalConnectionManager.connect (windowLandingPositionSelection->closed, [this]()
	{
		const auto& localPlayer = newGame->getLocalPlayer();

		windowLandingPositionSelection->getChatBox()->removePlayerEntry (localPlayer.getNr());
		playersLandingStatus.erase (std::remove_if (playersLandingStatus.begin(), playersLandingStatus.end(), [&] (const std::unique_ptr<cPlayerLandingStatus>& status) { return status->getPlayer().getNr() == localPlayer.getNr(); }), playersLandingStatus.end());

		landingPositionManager->deleteLandingPosition (localPlayer);

		sendNetMessage(cMuMsgInLandingPositionSelectionStatus(localPlayer.getNr(), true));
	});

	for (const auto& status : playersLandingStatus)
	{
		windowLandingPositionSelection->getChatBox()->addPlayerEntry (std::make_unique<cChatBoxLandingPlayerListViewItem> (*status));
	}

	signalConnectionManager.connect (windowLandingPositionSelection->canceled, [this]() { windowLandingPositionSelection->close(); });
	signalConnectionManager.connect (windowLandingPositionSelection->selectedPosition, [this] (cPosition landingPosition)
	{
		landingPositionManager->setLandingPosition (newGame->getLocalPlayer(), landingPosition);
	});

	signalConnectionManager.connect (windowLandingPositionSelection->getChatBox()->commandEntered, [this] (const std::string & command)
	{
		if (command == "/details")
		{
			const auto& players = newGame->getPlayers();
			for (const auto& player : players)
			{
				const auto entry = windowLandingPositionSelection->getChatBox()->getPlayerEntryFromNumber (player.getNr());
				if (entry)
				{
					entry->setLandingPositionManager (landingPositionManager.get());
				}
			}
		}
		else
		{
			const auto& localPlayer = newGame->getLocalPlayer();
			windowLandingPositionSelection->getChatBox()->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (localPlayer.getName(), command));
			cSoundDevice::getInstance().playSoundEffect (SoundData.SNDChat);
			sendNetMessage(cMuMsgChat(command));
		}
	});

	signalConnectionManager.connect (landingPositionManager->landingPositionStateChanged, [this] (const cPlayerBasicData & player, eLandingPositionState state)
	{
		if (player.getNr() == newGame->getLocalPlayer().getNr())
		{
			windowLandingPositionSelection->applyReselectionState (state);
		}
		else
		{
			sendNetMessage(cMuMsgLandingState(state), player.getNr());
		}
	});

	signalConnectionManager.connect (landingPositionManager->allPositionsValid, [this]()
	{
		sendNetMessage(cMuMsgStartGame());

		newGame->setLocalPlayerLandingPosition (windowLandingPositionSelection->getSelectedPosition());

		startNewGame();
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
	auto yesNoDialog = application.show(std::make_shared<cDialogYesNo>("Are you sure you want to abort the game preparation?")); // TODO: translate

	signalConnectionManager.connect(yesNoDialog->yesClicked, [this]()
	{
		auto players = windowNetworkLobby->getPlayers();
		for(const auto& player : players)
		{
			player->setReady(false);			
		}
		sendNetMessage(cMuMsgPlayerAbortedGamePreparations());

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
void cMenuControllerMultiplayerHost::handleNetMessage (cNetMessage2& message)
{
	cTextArchiveIn archive;
	archive << message;
	Log.write("Menu: <-- " + archive.data(), cLog::eLOG_TYPE_NET_DEBUG);

	switch (message.getType())
	{
		case eNetMessageType::TCP_WANT_CONNECT:
			handleNetMessage_TCP_WANT_CONNECT(static_cast<cNetMessageTcpWantConnect&>(message));
			return;
		case eNetMessageType::TCP_CLOSE:
			handleNetMessage_TCP_CLOSE(static_cast<cNetMessageTcpClose&>(message));
			return;
		case eNetMessageType::MULTIPLAYER_LOBBY:
			//will be handled in the next switch statement
			break;
		default:
			Log.write("Host Menu Controller: Can not handle message", cLog::eLOG_TYPE_NET_ERROR);
			return;
	}
	
	cMultiplayerLobbyMessage& muMessage = static_cast<cMultiplayerLobbyMessage&>(message);

	switch (muMessage.getType())
	{
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_CHAT: 
			handleNetMessage_MU_MSG_CHAT (static_cast<cMuMsgChat&>(muMessage)); 
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_IDENTIFIKATION: 
			handleNetMessage_MU_MSG_IDENTIFIKATION(static_cast<cMuMsgIdentification&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_REQUEST_MAP: 
			handleNetMessage_MU_MSG_REQUEST_MAP(static_cast<cMuMsgRequestMap&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_FINISHED_MAP_DOWNLOAD: 
			handleNetMessage_MU_MSG_FINISHED_MAP_DOWNLOAD(static_cast<cMuMsgFinishedMapDownload&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_LANDING_POSITION: 
			handleNetMessage_MU_MSG_LANDING_POSITION(static_cast<cMuMsgLandingPosition&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS: 
			handleNetMessage_MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS(static_cast<cMuMsgInLandingPositionSelectionStatus&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION: 
			handleNetMessage_MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION(static_cast<cMuMsgPlayerAbortedGamePreparations&>(muMessage));
			break;
		default:
			Log.write("Host Menu Controller: Can not handle message", cLog::eLOG_TYPE_NET_ERROR);
			break;
	}
}

void cMenuControllerMultiplayerHost::handleNetMessage_TCP_WANT_CONNECT(cNetMessageTcpWantConnect& message)
{
	if (!connectionManager || !windowNetworkLobby) return;

	//add player
	auto newPlayer = std::make_shared<cPlayerBasicData>(message.playerName, cPlayerColor(message.playerColor), nextPlayerNumber++, false);
	windowNetworkLobby->addPlayer(newPlayer);

	windowNetworkLobby->addInfoEntry(lngPack.i18n("Text~Multiplayer~Player_Joined", newPlayer->getName()));
	
	if (message.packageVersion != PACKAGE_VERSION || message.packageRev != PACKAGE_REV)
	{
		windowNetworkLobby->addInfoEntry(lngPack.i18n("Text~Multiplayer~Gameversion_Warning_Server", message.packageVersion + " " + message.packageRev));
		windowNetworkLobby->addInfoEntry(lngPack.i18n("Text~Multiplayer~Gameversion_Own", (std::string)PACKAGE_VERSION + " " + PACKAGE_REV));
	}

	//accept the connection and assign the new player number
	connectionManager->acceptConnection(message.socket, newPlayer->getNr());

	//update playerlist of the clients
	sendNetMessage(cMuMsgPlayerList(windowNetworkLobby->getPlayers()));
	sendGameData(newPlayer->getNr());
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_TCP_CLOSE(cNetMessageTcpClose& message)
{
	if (!connectionManager || !windowNetworkLobby) return;

	auto playerToRemove = windowNetworkLobby->getPlayer(message.playerNr);
	if (playerToRemove == nullptr) return;
	
	windowNetworkLobby->addInfoEntry(lngPack.i18n("Text~Multiplayer~Player_Left", playerToRemove->getName()));
	sendNetMessage(cMuMsgChat("Text~Multiplayer~Player_Left", true, playerToRemove->getName()));
	windowNetworkLobby->removePlayer(*playerToRemove);

	sendNetMessage(cMuMsgPlayerList(windowNetworkLobby->getPlayers()));
}

void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_CHAT (cMuMsgChat& message)
{
	if (!connectionManager) return;

	if (newGame)
	{
		const auto& players = newGame->getPlayers();
		auto iter = std::find_if (players.begin(), players.end(), [ = ] (const cPlayerBasicData & player) { return player.getNr() == message.playerNr; });
		if (iter == players.end()) return;

		const auto& player = *iter;

		if (windowLandingPositionSelection)
		{
			if (message.translate)
			{
				windowLandingPositionSelection->getChatBox()->addChatEntry(std::make_unique<cLobbyChatBoxListViewItem>(lngPack.i18n(message.message, message.insertText)));
			}
			else
			{
				windowLandingPositionSelection->getChatBox()->addChatEntry(std::make_unique<cLobbyChatBoxListViewItem>(player.getName(), message.message));
				cSoundDevice::getInstance().playSoundEffect (SoundData.SNDChat);
			}
		}

		// send to other clients
		for (size_t i = 0; i != players.size(); ++i)
		{
			if (players[i].getNr() == message.playerNr || players[i].getNr() == newGame->getLocalPlayer().getNr()) continue;
			sendNetMessage(message, players[i].getNr(), message.playerNr);
		}
	}
	else if (windowNetworkLobby)
	{
		auto player = windowNetworkLobby->getPlayer(message.playerNr);
		if (player == nullptr) return;

		if (message.translate)
		{
			windowNetworkLobby->addInfoEntry (lngPack.i18n (message.message, message.insertText));
		}
		else
		{
			windowNetworkLobby->addChatEntry (player->getName(), message.message);
		}

		// send to other clients
		auto players = windowNetworkLobby->getPlayers();
		for (size_t i = 0; i != players.size(); ++i)
		{
			if (players[i]->getNr() == message.playerNr || players[i]->getNr() == windowNetworkLobby->getLocalPlayer()->getNr()) continue;
			sendNetMessage(message, players[i]->getNr(), message.playerNr);
		}
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_IDENTIFIKATION (cMuMsgIdentification& message)
{
	if (!connectionManager || !windowNetworkLobby) return;

	auto player = windowNetworkLobby->getPlayer(message.playerNr);
	if (player == nullptr) return;

	player->setColor (cPlayerColor (message.playerColor));
	player->setName (message.playerName);
	player->setReady (message.ready);

	// search double taken name or color
	checkTakenPlayerAttributes (*player);

	sendNetMessage(cMuMsgPlayerList(windowNetworkLobby->getPlayers()));
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_REQUEST_MAP (cMuMsgRequestMap& message)
{
	if (!connectionManager || !windowNetworkLobby) return;

	auto& map = windowNetworkLobby->getStaticMap();

	if (map == nullptr || MapDownload::isMapOriginal (map->getName())) return;

	auto player = windowNetworkLobby->getPlayer(message.playerNr);
	if (player == nullptr) return;

	// check, if there is already a map sender,
	// that uploads to the same player.
	// If yes, terminate the old map sender.
	for (auto i = mapSenders.begin(); i != mapSenders.end(); /*erase in loop*/)
	{
		auto& sender = *i;
		if (sender->getToPlayerNr() == player->getNr())
		{
			i = mapSenders.erase (i);
		}
		else
		{
			++i;
		}
	}
	auto mapSender = std::make_unique<cMapSender> (*connectionManager, player->getNr(), map->getName(), player->getName());
	mapSender->runInThread();
	mapSenders.push_back (std::move (mapSender));
	windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_Upload", player->getName()));
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_FINISHED_MAP_DOWNLOAD (cMuMsgFinishedMapDownload& message)
{
	if (!windowNetworkLobby) return;

	windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_UploadFinished", message.playerName));
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_LANDING_POSITION(cMuMsgLandingPosition& message)
{
	if (!windowNetworkLobby) return;

	assert (landingPositionManager != nullptr);

	Log.write ("Server: received landing coords from Player " + iToStr (message.playerNr), cLog::eLOG_TYPE_NET_DEBUG);

	auto player = windowNetworkLobby->getPlayer(message.playerNr);
	if (player == nullptr) return;

	landingPositionManager->setLandingPosition (*player, message.position);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS(cMuMsgInLandingPositionSelectionStatus& message)
{
	if (!connectionManager || !windowNetworkLobby) return;

	assert (landingPositionManager != nullptr);

	auto player = windowNetworkLobby->getPlayer(message.landingPlayer);
	if (player == nullptr) return;

	if (message.isIn)
	{
		playersLandingStatus.push_back (std::make_unique<cPlayerLandingStatus> (*player));
		if (windowLandingPositionSelection) windowLandingPositionSelection->getChatBox()->addPlayerEntry (std::make_unique<cChatBoxLandingPlayerListViewItem> (*playersLandingStatus.back()));
	}
	else
	{
		if (windowLandingPositionSelection) windowLandingPositionSelection->getChatBox()->removePlayerEntry(message.landingPlayer);
		playersLandingStatus.erase(std::remove_if(playersLandingStatus.begin(), playersLandingStatus.end(), [message](const std::unique_ptr<cPlayerLandingStatus>& status) { return status->getPlayer().getNr() == message.landingPlayer; }), playersLandingStatus.end());

		landingPositionManager->deleteLandingPosition (*player);
	}

	// send to all other clients
	auto players = windowNetworkLobby->getPlayers();
	for (const auto& receiver : players)
	{
		if (receiver->getNr() == windowNetworkLobby->getLocalPlayer()->getNr()) continue;

		sendNetMessage(message, receiver->getNr(), message.playerNr);
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION(cMuMsgPlayerAbortedGamePreparations & message)
{
	auto players = windowNetworkLobby->getPlayers();
	auto player = windowNetworkLobby->getPlayer(message.playerNr);
	if (player == nullptr) return;

	for(const auto& receiver : players)
	{
		receiver->setReady(false);

		if(receiver->getNr() == player->getNr() || receiver->getNr() == windowNetworkLobby->getLocalPlayer()->getNr()) continue;

		sendNetMessage(message, receiver->getNr(), message.playerNr);
	}

	auto yesNoDialog = application.show(std::make_shared<cDialogOk>("Player " + player->getName() + " has quit from game preparation")); // TODO: translate

	signalConnectionManager.connect(yesNoDialog->done, [this]()
	{
		application.closeTill(*windowNetworkLobby);
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::sendNetMessage(cNetMessage2& message, int receiverPlayerNr /*= -1*/, int senderPlayerNr /*= -1*/)
{
	if (senderPlayerNr == -1 && windowNetworkLobby)
	{
		auto& localPlayer = windowNetworkLobby->getLocalPlayer();
		if (localPlayer)
		{
			message.playerNr = localPlayer->getNr();
		}
	}
	
	cTextArchiveIn archive;
	archive << message;
	Log.write("Menu: --> " + archive.data() + " to " + toString(receiverPlayerNr), cLog::eLOG_TYPE_NET_DEBUG);
	
	if (receiverPlayerNr == -1)
		connectionManager->sendToPlayers(message);
	else
		connectionManager->sendToPlayer(message, receiverPlayerNr);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::sendNetMessage(cNetMessage2&& message, int receiverPlayerNr /*= -1*/, int senderPlayerNr /*= -1*/)
{
	sendNetMessage(static_cast<cNetMessage2&>(message), receiverPlayerNr, senderPlayerNr);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::saveOptions()
{
	if (!windowNetworkLobby) return;

	cSettings::getInstance().setPlayerName (windowNetworkLobby->getLocalPlayer()->getName().c_str());
	cSettings::getInstance().setPort (windowNetworkLobby->getPort());
	cSettings::getInstance().setPlayerColor (windowNetworkLobby->getLocalPlayer()->getColor().getColor());
}
