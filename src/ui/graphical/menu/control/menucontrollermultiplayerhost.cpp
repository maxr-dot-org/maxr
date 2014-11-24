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
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "ui/graphical/menu/windows/windowlandingunitselection/windowlandingunitselection.h"
#include "ui/graphical/menu/windows/windowlandingpositionselection/windowlandingpositionselection.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/menu/windows/windowgamesettings/windowgamesettings.h"
#include "ui/graphical/menu/windows/windowmapselection/windowmapselection.h"
#include "ui/graphical/menu/windows/windowload/windowload.h"
#include "ui/graphical/menu/windows/windowload/savegamedata.h"
#include "ui/graphical/menu/widgets/special/lobbychatboxlistviewitem.h"
#include "ui/graphical/menu/widgets/special/chatboxlandingplayerlistviewitem.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "game/startup/network/host/networkhostgamenew.h"
#include "game/startup/network/host/networkhostgamesaved.h"
#include "main.h"
#include "game/data/map/map.h"
#include "game/data/player/player.h"
#include "game/data/units/landingunit.h"
#include "network.h"
#include "utility/log.h"
#include "menuevents.h"
#include "netmessage.h"
#include "mapdownload.h"
#include "game/logic/savegame.h"
#include "game/logic/client.h"
#include "game/logic/server.h"

// TODO: remove
std::vector<std::pair<sID, int>> createInitialLandingUnitsList (int clan, const cGameSettings& gameSettings); // defined in windowsingleplayer.cpp

//------------------------------------------------------------------------------
cMenuControllerMultiplayerHost::cMenuControllerMultiplayerHost (cApplication& application_) :
	application (application_),
	nextPlayerNumber (0)
{}

//------------------------------------------------------------------------------
cMenuControllerMultiplayerHost::~cMenuControllerMultiplayerHost ()
{}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::start ()
{
	assert (windowNetworkLobby == nullptr); // should not be started twice

	network = std::make_shared<cTCP> ();

	network->setMessageReceiver (this);

	windowNetworkLobby = std::make_shared<cWindowNetworkLobbyHost> ();

	triedLoadMapName = "";
	nextPlayerNumber = windowNetworkLobby->getLocalPlayer ()->getNr () + 1;

	application.show (windowNetworkLobby);
	application.addRunnable (shared_from_this ());

	signalConnectionManager.connect (windowNetworkLobby->terminated, std::bind (&cMenuControllerMultiplayerHost::reset, this));
	signalConnectionManager.connect (windowNetworkLobby->backClicked, [this]()
	{
		windowNetworkLobby->close ();
		saveOptions ();
	});

	signalConnectionManager.connect (windowNetworkLobby->wantLocalPlayerReadyChange, std::bind (&cMenuControllerMultiplayerHost::handleWantLocalPlayerReadyChange, this));
	signalConnectionManager.connect (windowNetworkLobby->triggeredChatMessage, std::bind (&cMenuControllerMultiplayerHost::handleChatMessageTriggered, this));

	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer ()->nameChanged, std::bind (&cMenuControllerMultiplayerHost::handleLocalPlayerAttributesChanged, this));
	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer ()->colorChanged, std::bind (&cMenuControllerMultiplayerHost::handleLocalPlayerAttributesChanged, this));
	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer ()->readyChanged, std::bind (&cMenuControllerMultiplayerHost::handleLocalPlayerAttributesChanged, this));

	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectMap, std::bind (&cMenuControllerMultiplayerHost::handleSelectMap, this, std::ref (application)));
	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectSettings, std::bind (&cMenuControllerMultiplayerHost::handleSelectSettings, this, std::ref (application)));
	signalConnectionManager.connect (windowNetworkLobby->triggeredSelectSaveGame, std::bind (&cMenuControllerMultiplayerHost::handleSelectSaveGame, this, std::ref (application)));

	signalConnectionManager.connect (windowNetworkLobby->triggeredStartHost, std::bind (&cMenuControllerMultiplayerHost::startHost, this));
	signalConnectionManager.connect (windowNetworkLobby->triggeredStartGame, std::bind (&cMenuControllerMultiplayerHost::checkGameStart, this));

	signalConnectionManager.connect (windowNetworkLobby->staticMapChanged, [this](){sendGameData (*network, windowNetworkLobby->getStaticMap().get(), windowNetworkLobby->getGameSettings().get(), windowNetworkLobby->getSaveGamePlayers(), windowNetworkLobby->getSaveGameName()); });
	signalConnectionManager.connect (windowNetworkLobby->gameSettingsChanged, [this](){sendGameData (*network, windowNetworkLobby->getStaticMap().get(), windowNetworkLobby->getGameSettings().get(), windowNetworkLobby->getSaveGamePlayers(), windowNetworkLobby->getSaveGameName()); });
	signalConnectionManager.connect (windowNetworkLobby->saveGameChanged, [this](){sendGameData (*network, windowNetworkLobby->getStaticMap().get(), windowNetworkLobby->getGameSettings().get(), windowNetworkLobby->getSaveGamePlayers(), windowNetworkLobby->getSaveGameName()); });
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::reset ()
{
	network = nullptr;
	windowNetworkLobby = nullptr;
	windowLandingPositionSelection = nullptr;
	newGame = nullptr;
	application.removeRunnable (*this);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::pushEvent (std::unique_ptr<cNetMessage> message)
{
	messageQueue.push (std::move (message));
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::run ()
{
	std::unique_ptr<cNetMessage> message;
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
			windowMapSelection->close ();
		}
		else
		{
			application.show (std::make_shared<cDialogOk> ("Error while loading map!")); // TODO: translate
		}
		triedLoadMapName = windowMapSelection->getSelectedMapName ();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleSelectSettings (cApplication& application)
{
	if (!windowNetworkLobby) return;

	auto windowGameSettings = application.show (std::make_shared<cWindowGameSettings> ());

	if (windowNetworkLobby->getGameSettings ())	windowGameSettings->applySettings (*windowNetworkLobby->getGameSettings ());
	else windowGameSettings->applySettings (cGameSettings ());

	windowGameSettings->done.connect ([&, windowGameSettings]()
	{
		windowNetworkLobby->setGameSettings (std::make_unique<cGameSettings> (windowGameSettings->getGameSettings ()));
		windowGameSettings->close ();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleSelectSaveGame (cApplication& application)
{
	if (!windowNetworkLobby) return;

	auto windowLoad = application.show (std::make_shared<cWindowLoad> ());
	windowLoad->load.connect ([&, windowLoad](int saveGameNumber)
	{
		windowNetworkLobby->setSaveGame (saveGameNumber);
		windowLoad->close ();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleWantLocalPlayerReadyChange ()
{
	if (!network || !windowNetworkLobby) return;

	auto& localPlayer = windowNetworkLobby->getLocalPlayer ();

	if (!localPlayer) return;

	if (!windowNetworkLobby->getStaticMap() && !triedLoadMapName.empty ())
	{
		if (!localPlayer->isReady ()) windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~No_Map_No_Ready", triedLoadMapName));
		localPlayer->setReady (false);
	}
	else localPlayer->setReady (!localPlayer->isReady ());
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleChatMessageTriggered ()
{
	if (!network || !windowNetworkLobby) return;

	const auto& chatMessage = windowNetworkLobby->getChatMessage ();

	if (chatMessage.empty ()) return;

	auto& localPlayer = windowNetworkLobby->getLocalPlayer ();

	if (localPlayer)
	{
		windowNetworkLobby->addChatEntry (localPlayer->getName (), chatMessage);
		sendMenuChatMessage (*network, chatMessage, nullptr, localPlayer->getNr ());
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleLocalPlayerAttributesChanged ()
{
	if (!network || !windowNetworkLobby) return;

	checkTakenPlayerAttributes (*windowNetworkLobby->getLocalPlayer());
	sendPlayerList (*network, windowNetworkLobby->getPlayers());
}

void cMenuControllerMultiplayerHost::checkTakenPlayerAttributes (cPlayerBasicData& player)
{
	if (!network || !windowNetworkLobby) return;

	if (!player.isReady ()) return;

	auto players = windowNetworkLobby->getPlayers ();
	const auto& localPlayer = windowNetworkLobby->getLocalPlayer ();

	for (size_t i = 0; i != players.size (); ++i)
	{
		if (players[i].get() == &player) continue;
		if (players[i]->getName () == player.getName ())
		{
			if (player.getNr () != localPlayer->getNr ()) sendMenuChatMessage (*network, "Text~Multiplayer~Player_Name_Taken", &player, localPlayer->getNr (), true);
			else windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Player_Name_Taken"));
			player.setReady (false);
			break;
		}
		if (players[i]->getColor () == player.getColor ())
		{
			if (player.getNr () != localPlayer->getNr ()) sendMenuChatMessage (*network, "Text~Multiplayer~Player_Color_Taken", &player, localPlayer->getNr (), true);
			else windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Player_Color_Taken"));
			player.setReady (false);
			break;
		}
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::checkGameStart ()
{
	if (!network || !windowNetworkLobby) return;

	if ((!windowNetworkLobby->getGameSettings () || !windowNetworkLobby->getStaticMap ()) && windowNetworkLobby->getSaveGameNumber () == -1)
	{
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Missing_Settings"));
		return;
	}

	auto players = windowNetworkLobby->getPlayers ();
	auto notReadyPlayerIter = std::find_if (players.begin (), players.end (), [ ](const std::shared_ptr<cPlayerBasicData>& player) { return !player->isReady (); });

	if (notReadyPlayerIter != players.end ())
	{
		windowNetworkLobby->addInfoEntry ((*notReadyPlayerIter)->getName () + " " + lngPack.i18n ("Text~Multiplayer~Not_Ready"));
		return;
	}

	if (network->getConnectionStatus () == 0)
	{
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Server_Not_Running"));
		return;
	}

	if (windowNetworkLobby->getSaveGameNumber () != -1)
	{
		cSavegame savegame (windowNetworkLobby->getSaveGameNumber());
		auto savegamePlayers = savegame.loadPlayers ();

		auto menuPlayers = windowNetworkLobby->getPlayers ();

		// check whether all necessary players are connected
		for (size_t i = 0; i < savegamePlayers.size (); ++i)
		{
			auto iter = std::find_if (menuPlayers.begin (), menuPlayers.end (), [&](const std::shared_ptr<cPlayerBasicData>& player) { return player->getName () == savegamePlayers[i].getName (); });
			if (iter == menuPlayers.end ())
			{
				windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Player_Wrong"));
				return;
			}
		}

		// disconnect or update menu players
		for (auto i = menuPlayers.begin(); i < menuPlayers.end ();)
		{
			auto menuPlayer = *i;
			auto iter = std::find_if (savegamePlayers.begin (), savegamePlayers.end (), [&](const cPlayerBasicData& player) { return player.getName () == menuPlayer->getName (); });

			if (iter == savegamePlayers.end ())
			{
				// the player does not belong to the save game: disconnect him
				sendMenuChatMessage (*network, "Text~Multiplayer~Disconnect_Not_In_Save", menuPlayer.get(), -1, true);
				const int socketIndex = menuPlayer->getSocketIndex ();
				network->close (socketIndex);
				for (size_t k = 0; k != menuPlayers.size (); ++k)
				{
					menuPlayers[k]->onSocketIndexDisconnected (socketIndex);
				}
				i = menuPlayers.erase (i);
			}
			else
			{
				auto& savegamePlayer = *iter;
				menuPlayer->setNr (savegamePlayer.getNr());
				menuPlayer->setColor (savegamePlayer.getColor ());
				sendPlayerNumber (*network, *menuPlayer);
				++i;
			}
		}

		sendPlayerList (*network, menuPlayers);

		auto staticMap = windowNetworkLobby->getStaticMap ();
		assert (staticMap != nullptr);

		auto gameSettings = std::make_shared<cGameSettings> (savegame.loadGameSettings());

		for (size_t i = 0; i < menuPlayers.size (); ++i)
		{
			sendGameData (*network, staticMap.get(), gameSettings.get(), windowNetworkLobby->getSaveGamePlayers (), windowNetworkLobby->getSaveGameName(), menuPlayers[i].get ());
		}
		saveOptions ();

		sendGo (*network);
		startSavedGame ();
	}
	else
	{
		saveOptions ();

		sendGo (*network);
		startGamePreparation ();
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startSavedGame ()
{
	if (!windowNetworkLobby || windowNetworkLobby->getSaveGameNumber () == -1) return;

	auto savedGame = std::make_shared<cNetworkHostGameSaved> ();

	savedGame->setNetwork (network);
    savedGame->setSaveGameNumber (windowNetworkLobby->getSaveGameNumber ());
	savedGame->setPlayers (windowNetworkLobby->getPlayersNotShared (), *windowNetworkLobby->getLocalPlayer ());

	application.closeTill (*windowNetworkLobby);
	windowNetworkLobby->close ();
	signalConnectionManager.connect (windowNetworkLobby->terminated, [&]() { windowNetworkLobby = nullptr; });

	savedGame->start (application);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startGamePreparation ()
{
	const auto& staticMap = windowNetworkLobby->getStaticMap ();
	const auto& gameSettings = windowNetworkLobby->getGameSettings ();

	if (!staticMap || !gameSettings || !network) return;

	newGame = std::make_shared<cNetworkHostGameNew> ();

	newGame->setPlayers (windowNetworkLobby->getPlayersNotShared (), *windowNetworkLobby->getLocalPlayer ());
	newGame->setGameSettings (gameSettings);
	newGame->setStaticMap (staticMap);
	newGame->setNetwork (network);

	landingPositionManager = std::make_shared<cLandingPositionManager> (newGame->getPlayers ());

	signalConnectionManager.connect (landingPositionManager->landingPositionSet, [this](const cPlayerBasicData& player, const cPosition& position)
	{
		auto iter = std::find_if (playersLandingStatus.begin (), playersLandingStatus.end (), [&](const std::unique_ptr<cPlayerLandingStatus>& entry){ return entry->getPlayer ().getNr () == player.getNr (); });
		assert (iter != playersLandingStatus.end ());

		auto& entry = **iter;

		const auto hadSelectedPosition = entry.hasSelectedPosition ();

		entry.setHasSelectedPosition (true);

		if (entry.hasSelectedPosition () && !hadSelectedPosition)
		{
			sendPlayerHasSelectedLandingPosition (*network, entry.getPlayer (), nullptr);
		}
	});

	if (newGame->getGameSettings ()->getClansEnabled ())
	{
		startClanSelection ();
	}
	else
	{
		startLandingUnitSelection ();
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startClanSelection ()
{
	if (!newGame) return;

	auto windowClanSelection = application.show (std::make_shared<cWindowClanSelection> ());

	signalConnectionManager.connect (windowClanSelection->canceled, [windowClanSelection]() { windowClanSelection->close (); });
	signalConnectionManager.connect (windowClanSelection->done, [this, windowClanSelection]()
	{
		newGame->setLocalPlayerClan (windowClanSelection->getSelectedClan ());

		startLandingUnitSelection ();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startLandingUnitSelection ()
{
	if (!newGame || !newGame->getGameSettings ()) return;

	auto initialLandingUnits = createInitialLandingUnitsList (newGame->getLocalPlayerClan (), *newGame->getGameSettings ());

	auto windowLandingUnitSelection = application.show (std::make_shared<cWindowLandingUnitSelection> (cPlayerColor(), newGame->getLocalPlayerClan (), initialLandingUnits, newGame->getGameSettings ()->getStartCredits ()));

	signalConnectionManager.connect (windowLandingUnitSelection->canceled, [windowLandingUnitSelection]() { windowLandingUnitSelection->close (); });
	signalConnectionManager.connect (windowLandingUnitSelection->done, [this, windowLandingUnitSelection]()
	{
		newGame->setLocalPlayerLandingUnits (windowLandingUnitSelection->getLandingUnits ());
		newGame->setLocalPlayerUnitUpgrades (windowLandingUnitSelection->getUnitUpgrades ());

		startLandingPositionSelection ();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startLandingPositionSelection ()
{
	if (!newGame || !newGame->getStaticMap () || !network) return;

	windowLandingPositionSelection = std::make_shared<cWindowLandingPositionSelection> (newGame->getStaticMap (), true);

	application.show (windowLandingPositionSelection);

	signalConnectionManager.connect (windowLandingPositionSelection->opened, [this]()
	{
		const auto& localPlayer = newGame->getLocalPlayer ();
		const auto& players = newGame->getPlayers ();

		playersLandingStatus.push_back (std::make_unique<cPlayerLandingStatus> (localPlayer));
		windowLandingPositionSelection->getChatBox ()->addPlayerEntry (std::make_unique<cChatBoxLandingPlayerListViewItem> (*playersLandingStatus.back ()));

		for (const auto& receiver : players)
		{
			if (receiver.getNr () == localPlayer.getNr ()) continue;
			sendInLandingPositionSelectionStatus (*network, localPlayer, true, &receiver);
		}
	});
	signalConnectionManager.connect (windowLandingPositionSelection->closed, [this]()
	{
		const auto& localPlayer = newGame->getLocalPlayer ();
		const auto& players = newGame->getPlayers ();

		windowLandingPositionSelection->getChatBox ()->removePlayerEntry (localPlayer.getNr ());
		playersLandingStatus.erase (std::remove_if (playersLandingStatus.begin (), playersLandingStatus.end (), [&](const std::unique_ptr<cPlayerLandingStatus>& status){ return status->getPlayer ().getNr () == localPlayer.getNr (); }), playersLandingStatus.end ());

		landingPositionManager->deleteLandingPosition (localPlayer);

		for (const auto& receiver : players)
		{
			if (receiver.getNr () == localPlayer.getNr ()) continue;
			sendInLandingPositionSelectionStatus (*network, localPlayer, false, &receiver);
		}
	});

	for (const auto& status : playersLandingStatus)
	{
		windowLandingPositionSelection->getChatBox ()->addPlayerEntry (std::make_unique<cChatBoxLandingPlayerListViewItem> (*status));
	}

	signalConnectionManager.connect (windowLandingPositionSelection->canceled, [this]() { windowLandingPositionSelection->close (); });
	signalConnectionManager.connect (windowLandingPositionSelection->selectedPosition, [this](cPosition landingPosition)
	{
		landingPositionManager->setLandingPosition (newGame->getLocalPlayer (), landingPosition);
	});

	signalConnectionManager.connect (windowLandingPositionSelection->getChatBox()->commandEntered, [this](const std::string& command)
	{
		const auto& localPlayer = newGame->getLocalPlayer ();
		windowLandingPositionSelection->getChatBox ()->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (localPlayer.getName (), command));
		cSoundDevice::getInstance ().playSoundEffect (SoundData.SNDChat);
		sendMenuChatMessage (*network, command, nullptr, localPlayer.getNr ());
	});

	signalConnectionManager.connect (landingPositionManager->landingPositionStateChanged, [this](const cPlayerBasicData& player, eLandingPositionState state)
	{
		if (player.getNr() == newGame->getLocalPlayer ().getNr())
		{
			windowLandingPositionSelection->applyReselectionState (state);
		}
		else
		{
			sendLandingState (*network, state, player);
		}
    });

	signalConnectionManager.connect (landingPositionManager->allPositionsValid, [this]()
	{
		sendAllLanded (*network);

		newGame->setLocalPlayerLandingPosition (windowLandingPositionSelection->getSelectedPosition ());

		startNewGame ();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startNewGame ()
{
	if (!newGame) return;

	application.closeTill (*windowNetworkLobby);
	windowNetworkLobby->close ();
	signalConnectionManager.connect (windowNetworkLobby->terminated, [&]() { windowNetworkLobby = nullptr; });

	newGame->start (application);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startHost ()
{
	if (!network || !windowNetworkLobby) return;

	if (network->getConnectionStatus () != 0) return;

	if (network->create (windowNetworkLobby->getPort ()))
	{
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Error_Socket"));
		Log.write ("Error opening socket", cLog::eLOG_TYPE_WARNING);
	}
	else
	{
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Open") + " (" + lngPack.i18n ("Text~Title~Port") + ": "  + iToStr (windowNetworkLobby->getPort ()) + ")");
		Log.write ("Game open (Port: " + iToStr (windowNetworkLobby->getPort ()) + ")", cLog::eLOG_TYPE_INFO);
		windowNetworkLobby->disablePortEdit ();
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage (cNetMessage& message)
{
	Log.write ("Menu: <-- " + message.getTypeAsString () + ", Hexdump: " + message.getHexDump (), cLog::eLOG_TYPE_NET_DEBUG);

	switch (message.iType)
	{
	case MU_MSG_CHAT: handleNetMessage_MU_MSG_CHAT (message); break;
	case TCP_ACCEPT: handleNetMessage_TCP_ACCEPT (message); break;
	case TCP_CLOSE: handleNetMessage_TCP_CLOSE (message); break;
	case MU_MSG_IDENTIFIKATION: handleNetMessage_MU_MSG_IDENTIFIKATION (message); break;
	case MU_MSG_REQUEST_MAP: handleNetMessage_MU_MSG_REQUEST_MAP (message); break;
	case MU_MSG_FINISHED_MAP_DOWNLOAD: handleNetMessage_MU_MSG_FINISHED_MAP_DOWNLOAD (message); break;
	case MU_MSG_LANDING_POSITION: handleNetMessage_MU_MSG_LANDING_POSITION (message); break;
	case MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS: handleNetMessage_MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS (message); break;
    default:
        Log.write ("Host Menu Controller: Can not handle message type " + message.getTypeAsString (), cLog::eLOG_TYPE_NET_ERROR);
        break;
	}
}

#define UNIDENTIFIED_PLAYER_NAME "unidentified"

void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_CHAT (cNetMessage& message)
{
	assert (message.iType == MU_MSG_CHAT);

	if (!network) return;

	const bool translationText = message.popBool ();
	const auto chatText = message.popString ();

	if (newGame)
	{
		const auto& players = newGame->getPlayers ();
		auto iter = std::find_if (players.begin (), players.end (), [=](const cPlayerBasicData& player){ return player.getNr () == message.iPlayerNr; });
		if (iter == players.end ()) return;

		const auto& player = *iter;

		if (windowLandingPositionSelection)
		{
			if (translationText)
			{
				windowLandingPositionSelection->getChatBox()->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem>(lngPack.i18n (chatText)));
			}
			else
			{
				windowLandingPositionSelection->getChatBox ()->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (player.getName (), chatText));
				cSoundDevice::getInstance ().playSoundEffect (SoundData.SNDChat);
			}
		}

		// send to other clients
		for (size_t i = 0; i != players.size (); ++i)
		{
			if (players[i].getNr () == message.iPlayerNr || players[i].getNr() == newGame->getLocalPlayer().getNr()) continue;

			sendMenuChatMessage (*network, chatText, &players[i], message.iPlayerNr, translationText);
		}
	}
	else if (windowNetworkLobby)
	{
		auto players = windowNetworkLobby->getPlayers ();
		auto iter = std::find_if (players.begin (), players.end (), [=](const std::shared_ptr<cPlayerBasicData>& player){ return player->getNr () == message.iPlayerNr; });
		if (iter == players.end ()) return;

		const auto& player = **iter;

		if (translationText)
		{
			windowNetworkLobby->addInfoEntry (lngPack.i18n (chatText));
		}
		else
		{
			windowNetworkLobby->addChatEntry (player.getName (), chatText);
		}

		// send to other clients
		for (size_t i = 0; i != players.size (); ++i)
		{
			if (players[i]->getNr () == message.iPlayerNr || players[i]->getNr () == windowNetworkLobby->getLocalPlayer()->getNr()) continue;

			sendMenuChatMessage (*network, chatText, players[i].get (), message.iPlayerNr, translationText);
		}
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_TCP_ACCEPT (cNetMessage& message)
{
	assert (message.iType == TCP_ACCEPT);

	if (!network || !windowNetworkLobby) return;

	auto newPlayer = std::make_shared<cPlayerBasicData> (UNIDENTIFIED_PLAYER_NAME, cPlayerColor(), nextPlayerNumber++, message.popInt16 ());
	windowNetworkLobby->addPlayer (newPlayer);
	sendRequestIdentification (*network, *newPlayer);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_TCP_CLOSE (cNetMessage& message)
{
	assert (message.iType == TCP_CLOSE);

	if (!network || !windowNetworkLobby) return;

	int socket = message.popInt16 ();
	network->close (socket);

	// delete player
	auto players = windowNetworkLobby->getPlayers ();
	auto iter = std::find_if (players.begin (), players.end (), [=](const std::shared_ptr<cPlayerBasicData>& player){ return player->getSocketIndex () == socket; });
	if (iter == players.end ()) return;

	auto playerToRemove = *iter;
	players.erase (iter);

	// resort socket numbers
	for (size_t i = 0; i != players.size (); ++i)
	{
		players[i]->onSocketIndexDisconnected (socket);
	}

	windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Player_Left", playerToRemove->getName ()));

	sendPlayerList (*network, players);

	windowNetworkLobby->removePlayer (*playerToRemove);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_IDENTIFIKATION (cNetMessage& message)
{
	assert (message.iType == MU_MSG_IDENTIFIKATION);

	if (!network || !windowNetworkLobby) return;

	const auto playerNr = message.popInt16 ();

	auto players = windowNetworkLobby->getPlayers ();
	auto iter = std::find_if (players.begin (), players.end (), [=](const std::shared_ptr<cPlayerBasicData>& player){ return player->getNr () == playerNr; });
	if (iter == players.end ()) return;

	auto& player = **iter;

	bool freshJoined = (player.getName ().compare (UNIDENTIFIED_PLAYER_NAME) == 0);
	player.setColor (cPlayerColor(message.popColor ()));
	player.setName (message.popString ());
	player.setReady (message.popBool ());

	Log.write ("game version of client " + iToStr (player.getNr ()) + " is: " + message.popString (), cLog::eLOG_TYPE_NET_DEBUG);

	if (freshJoined) windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Player_Joined", player.getName ()));

	// search double taken name or color
	checkTakenPlayerAttributes (player);

	sendPlayerList (*network, players);
	sendGameData (*network, windowNetworkLobby->getStaticMap ().get (), windowNetworkLobby->getGameSettings ().get (), windowNetworkLobby->getSaveGamePlayers(), windowNetworkLobby->getSaveGameName(), &player);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_REQUEST_MAP (cNetMessage& message)
{
	assert (message.iType == MU_MSG_REQUEST_MAP);

	if (!network || !windowNetworkLobby) return;

	auto& map = windowNetworkLobby->getStaticMap ();

	if (map == nullptr || MapDownload::isMapOriginal (map->getName ())) return;

	const int playerNr = message.popInt16 ();

	auto players = windowNetworkLobby->getPlayers ();
	auto iter = std::find_if (players.begin (), players.end (), [=](const std::shared_ptr<cPlayerBasicData>& player){ return player->getNr () == playerNr; });
	if (iter == players.end ()) return;

	auto& player = **iter;

	// check, if there is already a map sender,
	// that uploads to the same socketNr.
	// If yes, terminate the old map sender.
	for (auto i = mapSenders.begin (); i != mapSenders.end (); /*erase in loop*/)
	{
		auto& sender = *i;
		if (sender->getToSocket () == player.getSocketIndex ())
		{
			i = mapSenders.erase (i);
		}
		else
		{
			++i;
		}
	}
	auto mapSender = std::make_unique<cMapSender> (*network, player.getSocketIndex (), map->getName (), player.getName ());
	mapSender->runInThread ();
	mapSenders.push_back (std::move(mapSender));
	windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_Upload", player.getName ()));
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_FINISHED_MAP_DOWNLOAD (cNetMessage& message)
{
	assert (message.iType == MU_MSG_FINISHED_MAP_DOWNLOAD);

	if (!windowNetworkLobby) return;

	auto receivingPlayerName = message.popString ();
	windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_UploadFinished", receivingPlayerName));
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_LANDING_POSITION (cNetMessage& message)
{
	assert (message.iType == MU_MSG_LANDING_POSITION);

	if (!windowNetworkLobby) return;

	assert (landingPositionManager != nullptr);

	int playerNr = message.popInt32 ();
	const auto position = message.popPosition ();

	Log.write ("Server: received landing coords from Player " + iToStr (playerNr), cLog::eLOG_TYPE_NET_DEBUG);

	auto players = windowNetworkLobby->getPlayers ();

	auto iter = std::find_if (players.begin (), players.end (), [playerNr](const std::shared_ptr<cPlayerBasicData>& player){ return player->getNr () == playerNr; });
	if (iter == players.end ()) return;

	auto& player = **iter;

	landingPositionManager->setLandingPosition (player, position);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS (cNetMessage& message)
{
	assert (message.iType == MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS);

	if (!network || !windowNetworkLobby) return;

	assert (landingPositionManager != nullptr);

	auto players = windowNetworkLobby->getPlayers ();

	const auto isIn = message.popBool ();
	const auto playerNr = message.popInt32 ();

	auto iter = std::find_if (players.begin (), players.end (), [playerNr](const std::shared_ptr<cPlayerBasicData>& player){ return player->getNr () == playerNr; });
	if (iter == players.end ()) return;

	const auto& player = **iter;

	if (isIn)
	{
		playersLandingStatus.push_back (std::make_unique<cPlayerLandingStatus> (player));
		if (windowLandingPositionSelection) windowLandingPositionSelection->getChatBox ()->addPlayerEntry (std::make_unique<cChatBoxLandingPlayerListViewItem> (*playersLandingStatus.back ()));
	}
	else
	{
		if(windowLandingPositionSelection) windowLandingPositionSelection->getChatBox()->removePlayerEntry(playerNr);
		playersLandingStatus.erase(std::remove_if(playersLandingStatus.begin(), playersLandingStatus.end(), [playerNr](const std::unique_ptr<cPlayerLandingStatus>& status){ return status->getPlayer().getNr() == playerNr; }), playersLandingStatus.end());
		
		landingPositionManager->deleteLandingPosition (player);
	}

	// send to all other clients
	for (const auto& receiver : players)
	{
		if (receiver->getNr () == windowNetworkLobby->getLocalPlayer ()->getNr ()) continue;

		sendInLandingPositionSelectionStatus (*network, player, isIn, receiver.get());
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::saveOptions ()
{
	if (!windowNetworkLobby) return;

	cSettings::getInstance ().setPlayerName (windowNetworkLobby->getLocalPlayer()->getName ().c_str ());
	cSettings::getInstance ().setPort (windowNetworkLobby->getPort());
	cSettings::getInstance ().setPlayerColor (windowNetworkLobby->getLocalPlayer()->getColor().getColor());
}