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
#include "../application.h"
#include "../menu/windows/windownetworklobbyhost/windownetworklobbyhost.h"
#include "../menu/windows/windowgamesettings/gamesettings.h"
#include "../menu/windows/windowclanselection/windowclanselection.h"
#include "../menu/windows/windowlandingunitselection/windowlandingunitselection.h"
#include "../menu/windows/windowlandingpositionselection/windowlandingpositionselection.h"
#include "../menu/windows/windowgamesettings/gamesettings.h"
#include "../menu/windows/windowgamesettings/windowgamesettings.h"
#include "../menu/windows/windowmapselection/windowmapselection.h"
#include "../menu/windows/windowload/windowload.h"
#include "../menu/dialogs/dialogok.h"
#include "../../game/network/host/networkhostgamenew.h"
#include "../../game/network/host/networkhostgamesaved.h"
#include "../../game/logic/landingpositionmanager.h"
#include "../../main.h"
#include "../../map.h"
#include "../../player.h"
#include "../../menus.h"
#include "../../network.h"
#include "../../log.h"
#include "../../menuevents.h"
#include "../../netmessage.h"
#include "../../savegame.h"
#include "../../client.h"
#include "../../server.h"

// FIXME: remove
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

	signalConnectionManager.connect (windowNetworkLobby->staticMapChanged, [this](){sendGameData (*network, windowNetworkLobby->getStaticMap ().get (), windowNetworkLobby->getGameSettings ().get (), windowNetworkLobby->getSaveGameNumber (), nullptr); });
	signalConnectionManager.connect (windowNetworkLobby->gameSettingsChanged, [this](){sendGameData (*network, windowNetworkLobby->getStaticMap ().get (), windowNetworkLobby->getGameSettings ().get (), windowNetworkLobby->getSaveGameNumber (), nullptr); });
	signalConnectionManager.connect (windowNetworkLobby->saveGameChanged, [this](){sendGameData (*network, windowNetworkLobby->getStaticMap ().get (), windowNetworkLobby->getGameSettings ().get (), windowNetworkLobby->getSaveGameNumber (), nullptr); });
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::reset ()
{
	network = nullptr;
	windowNetworkLobby = nullptr;
	newGame = nullptr;
	savedGame = nullptr;
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

void cMenuControllerMultiplayerHost::checkTakenPlayerAttributes (sPlayer& player)
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
		if (players[i]->getColorIndex () == player.getColorIndex ())
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
	auto notReadyPlayerIter = std::find_if (players.begin (), players.end (), [ ](const std::shared_ptr<sPlayer>& player) { return !player->isReady (); });

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
			auto iter = std::find_if (menuPlayers.begin (), menuPlayers.end (), [&](const std::shared_ptr<sPlayer>& player) { return player->getName () == savegamePlayers[i].getName (); });
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
			auto iter = std::find_if (savegamePlayers.begin (), savegamePlayers.end (), [&](const sPlayer& player) { return player.getName () == menuPlayer->getName (); });
			
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
				menuPlayer->setColorIndex (savegamePlayer.getColorIndex ());
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
			sendGameData (*network, staticMap.get(), gameSettings.get(), windowNetworkLobby->getSaveGameNumber (), menuPlayers[i].get ());
		}

		sendGo (*network);

		startSavedGame ();
	}
	else
	{
		sendGo (*network);
		startGamePreparation ();
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startSavedGame ()
{
	if (!windowNetworkLobby || windowNetworkLobby->getSaveGameNumber () == -1) return;

	savedGame = std::make_shared<cNetworkHostGameSaved> ();

	savedGame->setNetwork (network);
	savedGame->setSaveGameNumber (windowNetworkLobby->getSaveGameNumber());

	savedGame->start (application);

	signalConnectionManager.connect (savedGame->terminated, [&]()
	{
		application.closeTill (*windowNetworkLobby);
		windowNetworkLobby->close ();
		windowNetworkLobby = nullptr;
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::startGamePreparation ()
{
	const auto& staticMap = windowNetworkLobby->getStaticMap ();
	const auto& gameSettings = windowNetworkLobby->getGameSettings ();

	if (!staticMap || !gameSettings || !network) return;

	newGame = std::make_shared<cNetworkHostGameNew> ();

	newGame->setPlayers (windowNetworkLobby->getPlayers (), *windowNetworkLobby->getLocalPlayer ());
	newGame->setGameSettings (gameSettings);
	newGame->setStaticMap (staticMap);
	newGame->setNetwork (network);

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

	auto windowLandingUnitSelection = application.show (std::make_shared<cWindowLandingUnitSelection> (0, newGame->getLocalPlayerClan (), initialLandingUnits, newGame->getGameSettings ()->getStartCredits ()));

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
	if (!newGame || !newGame->getStaticMap () || !newGame->getLocalPlayer () || !network) return;

	landingPositionManager = std::make_shared<cLandingPositionManager> (newGame->getPlayers ());

	auto windowLandingPositionSelection = application.show (std::make_shared<cWindowLandingPositionSelection> (newGame->getStaticMap ()));

	signalConnectionManager.connect (windowLandingPositionSelection->selectedPosition, [this](cPosition landingPosition)
	{
		landingPositionManager->setLandingPosition (*newGame->getLocalPlayer (), landingPosition);
	});

	signalConnectionManager.connect (landingPositionManager->landingPositionStateChanged, [this, windowLandingPositionSelection](const sPlayer& player, eLandingPositionState state)
	{
		if (&player == newGame->getLocalPlayer ().get ())
		{
			windowLandingPositionSelection->applyReselectionState (state);
		}
		else
		{
			sendLandingState (*network, state, player);
		}
	});

	signalConnectionManager.connect (landingPositionManager->allPositionsValid, [this, windowLandingPositionSelection]()
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

	newGame->start (application);

	signalConnectionManager.connect (newGame->terminated, [&]()
	{
		application.closeTill (*windowNetworkLobby);
		windowNetworkLobby->close ();
		windowNetworkLobby = nullptr;
	});
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
	default: break;
	}
}

#define UNIDENTIFIED_PLAYER_NAME "unidentified"

void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_CHAT (cNetMessage& message)
{
	assert (message.iType == MU_MSG_CHAT);

	if (!network || !windowNetworkLobby) return;

	auto players = windowNetworkLobby->getPlayers ();
	auto iter = std::find_if (players.begin (), players.end (), [=](const std::shared_ptr<sPlayer>& player){ return player->getNr () == message.iPlayerNr; });
	if (iter == players.end ()) return;

	const auto& player = **iter;

	bool translationText = message.popBool ();
	auto chatText = message.popString ();

	if (translationText) windowNetworkLobby->addInfoEntry (lngPack.i18n (chatText));
	else
	{
		windowNetworkLobby->addChatEntry (player.getName (), chatText);
		//PlayFX (SoundData.SNDChat.get ());
	}
	// send to other clients
	for (size_t i = 1; i != players.size (); ++i)
	{
		if (players[i]->getNr () == message.iPlayerNr) continue;
		sendMenuChatMessage (*network, chatText, players[i].get (), -1, translationText);
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHost::handleNetMessage_TCP_ACCEPT (cNetMessage& message)
{
	assert (message.iType == TCP_ACCEPT);

	if (!network || !windowNetworkLobby) return;

	auto newPlayer = std::make_shared<sPlayer> (UNIDENTIFIED_PLAYER_NAME, 0, nextPlayerNumber++, message.popInt16 ());
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
	auto iter = std::find_if (players.begin (), players.end (), [=](const std::shared_ptr<sPlayer>& player){ return player->getSocketIndex () == socket; });
	if (iter == players.end ()) return;

	auto playerToRemove = *iter;
	players.erase (iter);

	// resort socket numbers
	for (size_t i = 0; i != players.size (); ++i)
	{
		players[i]->onSocketIndexDisconnected (socket);
	}

	// resort player numbers
	for (size_t i = 0; i != players.size (); ++i)
	{
		players[i]->setNr (i);
		sendRequestIdentification (*network, *players[i]);
	}

	windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Player_Left", playerToRemove->getName ()));

	sendPlayerList (*network, players);

	windowNetworkLobby->removePlayer (*playerToRemove);
}


void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_IDENTIFIKATION (cNetMessage& message)
{
	assert (message.iType == MU_MSG_IDENTIFIKATION);

	if (!network || !windowNetworkLobby) return;

	const auto playerNr = message.popInt16 ();

	auto players = windowNetworkLobby->getPlayers ();
	auto iter = std::find_if (players.begin (), players.end (), [=](const std::shared_ptr<sPlayer>& player){ return player->getNr () == playerNr; });
	if (iter == players.end ()) return;

	auto& player = **iter;

	bool freshJoined = (player.getName ().compare (UNIDENTIFIED_PLAYER_NAME) == 0);
	player.setColorIndex (message.popInt16 ());
	player.setName (message.popString ());
	player.setReady (message.popBool ());

	Log.write ("game version of client " + iToStr (player.getNr ()) + " is: " + message.popString (), cLog::eLOG_TYPE_NET_DEBUG);

	if (freshJoined) windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Player_Joined", player.getName ()));

	// search double taken name or color
	checkTakenPlayerAttributes (player);

	sendPlayerList (*network, players);
	sendGameData (*network, windowNetworkLobby->getStaticMap ().get (), windowNetworkLobby->getGameSettings ().get (), windowNetworkLobby->getSaveGameNumber(), &player);
}

void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_REQUEST_MAP (cNetMessage& message)
{
	assert (message.iType == MU_MSG_REQUEST_MAP);

	if (!network || !windowNetworkLobby) return;

	//if (map == NULL || MapDownload::isMapOriginal (map->getName ())) return;

	//const size_t receiverNr = message.popInt16 ();
	//if (receiverNr >= players.size ()) return;
	//int socketNr = players[receiverNr]->getSocketIndex ();
	//// check, if there is already a map sender,
	//// that uploads to the same socketNr.
	//// If yes, terminate the old map sender.
	//for (int i = (int)mapSenders.size () - 1; i >= 0; i--)
	//{
	//	if (mapSenders[i] != 0 && mapSenders[i]->getToSocket () == socketNr)
	//	{
	//		delete mapSenders[i];
	//		mapSenders.erase (mapSenders.begin () + i);
	//	}
	//}
	//cMapSender* mapSender = new cMapSender (*network, socketNr, eventHandler.get (), map->getName (), players[receiverNr]->getName ());
	//mapSenders.push_back (mapSender);
	//mapSender->runInThread ();
	//addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_Upload", players[receiverNr]->getName ()));
}

void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_FINISHED_MAP_DOWNLOAD (cNetMessage& message)
{
	assert (message.iType == MU_MSG_FINISHED_MAP_DOWNLOAD);

	if (!windowNetworkLobby) return;

	auto receivingPlayerName = message.popString ();
	windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_UploadFinished", receivingPlayerName));
}

void cMenuControllerMultiplayerHost::handleNetMessage_MU_MSG_LANDING_POSITION (cNetMessage& message)
{
	assert (message.iType == MU_MSG_LANDING_POSITION);

	if (!windowNetworkLobby || !landingPositionManager) return;

	int playerNr = message.popInt32 ();
	const auto position = message.popPosition ();

	Log.write ("Server: received landing coords from Player " + iToStr (playerNr), cLog::eLOG_TYPE_NET_DEBUG);

	auto players = windowNetworkLobby->getPlayers ();

	auto iter = std::find_if (players.begin (), players.end (), [playerNr](const std::shared_ptr<sPlayer>& player){ return player->getNr () == playerNr; });
	if (iter == players.end ()) return;

	auto& player = **iter;

	landingPositionManager->setLandingPosition (player, position);
}