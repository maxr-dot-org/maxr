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

#include "ui/graphical/control/menucontrollermultiplayerclient.h"
#include "ui/graphical/application.h"
#include "ui/graphical/menu/windows/windownetworklobbyclient/windownetworklobbyclient.h"
#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "ui/graphical/menu/windows/windowlandingunitselection/windowlandingunitselection.h"
#include "ui/graphical/menu/windows/windowlandingpositionselection/windowlandingpositionselection.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/dialogs/dialogyesno.h"
#include "game/network/client/networkclientgamenew.h"
#include "game/network/client/networkclientgamereconnection.h"
#include "game/network/client/networkclientgamesaved.h"
#include "main.h"
#include "map.h"
#include "player.h"
#include "menus.h"
#include "network.h"
#include "log.h"
#include "menuevents.h"
#include "serverevents.h"
#include "netmessage.h"
#include "mapdownload.h"

// TODO: remove
std::vector<std::pair<sID, int>> createInitialLandingUnitsList (int clan, const cGameSettings& gameSettings); // defined in windowsingleplayer.cpp

//------------------------------------------------------------------------------
cMenuControllerMultiplayerClient::cMenuControllerMultiplayerClient (cApplication& application_) :
	application (application_),
	windowLandingPositionSelection (nullptr)
{}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::start ()
{
	network = std::make_shared<cTCP> ();

	network->setMessageReceiver (this);

	windowNetworkLobby = std::make_shared<cWindowNetworkLobbyClient> ();
	triedLoadMapName = "";

	application.show (windowNetworkLobby);
	application.addRunnable (shared_from_this ());

	signalConnectionManager.connect (windowNetworkLobby->terminated, std::bind (&cMenuControllerMultiplayerClient::reset, this));

	signalConnectionManager.connect (windowNetworkLobby->wantLocalPlayerReadyChange, std::bind (&cMenuControllerMultiplayerClient::handleWantLocalPlayerReadyChange, this));
	signalConnectionManager.connect (windowNetworkLobby->triggeredChatMessage, std::bind (&cMenuControllerMultiplayerClient::handleChatMessageTriggered, this));

	signalConnectionManager.connect (windowNetworkLobby->triggeredConnect, std::bind (&cMenuControllerMultiplayerClient::connect, this));

	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer ()->nameChanged, std::bind (&cMenuControllerMultiplayerClient::handleLocalPlayerAttributesChanged, this));
	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer ()->colorChanged, std::bind (&cMenuControllerMultiplayerClient::handleLocalPlayerAttributesChanged, this));
	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer ()->readyChanged, std::bind (&cMenuControllerMultiplayerClient::handleLocalPlayerAttributesChanged, this));
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::reset ()
{
	network = nullptr;
	windowNetworkLobby = nullptr;
	windowLandingPositionSelection = nullptr;
	newGame = nullptr;
	application.removeRunnable (*this);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::pushEvent (std::unique_ptr<cNetMessage> message)
{
	messageQueue.push (std::move (message));
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::run ()
{
	std::unique_ptr<cNetMessage> message;
	while (messageQueue.try_pop (message))
	{
		handleNetMessage (*message);
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleWantLocalPlayerReadyChange ()
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
void cMenuControllerMultiplayerClient::handleChatMessageTriggered ()
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
void cMenuControllerMultiplayerClient::handleLocalPlayerAttributesChanged ()
{
	if (!network || !windowNetworkLobby) return;

	if (network->getConnectionStatus () == 0) return;

	sendIdentification (*network, *windowNetworkLobby->getLocalPlayer ());
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::connect ()
{
	if (!network || !windowNetworkLobby) return;

	// Connect only if there isn't a connection yet
	if (network->getConnectionStatus () != 0) return;

	const auto& ip = windowNetworkLobby->getIp ();
	const auto& port = windowNetworkLobby->getPort ();

	windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Connecting") + ip + ":" + iToStr (port));    // e.g. Connecting to 127.0.0.1:55800
	Log.write (("Connecting to " + ip + ":" + iToStr (port)), cLog::eLOG_TYPE_INFO);

	// TODO: make this non blocking!
	if (network->connect (ip, port) == -1)
	{
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Error_Connect") + ip + ":" + iToStr (port));
		Log.write ("Error on connecting " + ip + ":" + iToStr (port), cLog::eLOG_TYPE_WARNING);
	}
	else
	{
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Connected"));
		Log.write ("Connected", cLog::eLOG_TYPE_INFO);
		windowNetworkLobby->disablePortEdit ();
		windowNetworkLobby->disableIpEdit ();
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::startSavedGame ()
{
	if (!network || !windowNetworkLobby || !windowNetworkLobby->getStaticMap () || !windowNetworkLobby->getGameSettings()) return;

	auto savedGame = std::make_shared<cNetworkClientGameSaved> ();

	savedGame->setNetwork (network);
	savedGame->setStaticMap (windowNetworkLobby->getStaticMap());
	savedGame->setGameSettings (windowNetworkLobby->getGameSettings());
	savedGame->setPlayers (windowNetworkLobby->getPlayers (), *windowNetworkLobby->getLocalPlayer());

	savedGame->start (application);

	signalConnectionManager.connect (savedGame->terminated, [&]()
	{
		application.closeTill (*windowNetworkLobby);
		windowNetworkLobby->close ();
		windowNetworkLobby = nullptr;
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::startGamePreparation ()
{
	const auto& staticMap = windowNetworkLobby->getStaticMap ();
	const auto& gameSettings = windowNetworkLobby->getGameSettings ();

	if (!staticMap || !gameSettings || !network) return;

	newGame = std::make_shared<cNetworkClientGameNew> ();

	newGame->setPlayers (windowNetworkLobby->getPlayers (), *windowNetworkLobby->getLocalPlayer ());
	newGame->setGameSettings (gameSettings);
	newGame->setStaticMap (staticMap);
	newGame->setNetwork (network);

	if (newGame->getGameSettings()->getClansEnabled ())
	{
		startClanSelection ();
	}
	else
	{
		startLandingUnitSelection ();
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::startClanSelection ()
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
void cMenuControllerMultiplayerClient::startLandingUnitSelection ()
{
	if (!newGame || !newGame->getGameSettings ()) return;

	auto initialLandingUnits = createInitialLandingUnitsList (newGame->getLocalPlayerClan(), *newGame->getGameSettings());

	auto windowLandingUnitSelection = application.show (std::make_shared<cWindowLandingUnitSelection> (cPlayerColor(0), newGame->getLocalPlayerClan (), initialLandingUnits, newGame->getGameSettings ()->getStartCredits ()));

	signalConnectionManager.connect (windowLandingUnitSelection->done, [this, windowLandingUnitSelection]()
	{
		newGame->setLocalPlayerLandingUnits (windowLandingUnitSelection->getLandingUnits ());
		newGame->setLocalPlayerUnitUpgrades (windowLandingUnitSelection->getUnitUpgrades ());

		startLandingPositionSelection ();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::startLandingPositionSelection ()
{
	if (!newGame || !newGame->getStaticMap () || !newGame->getLocalPlayer () || !network) return;

	windowLandingPositionSelection = std::make_shared<cWindowLandingPositionSelection> (newGame->getStaticMap ());
	application.show (windowLandingPositionSelection);

	signalConnectionManager.connect (windowLandingPositionSelection->selectedPosition, [this](cPosition landingPosition)
	{
		newGame->setLocalPlayerLandingPosition (windowLandingPositionSelection->getSelectedPosition ());

		sendLandingPosition (*network, landingPosition, *newGame->getLocalPlayer());
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::startNewGame ()
{
	if (!newGame) return;

	newGame->start (application);

	signalConnectionManager.connect(newGame->terminated, [&]()
	{
		application.closeTill (*windowNetworkLobby);
		windowNetworkLobby->close ();
		windowNetworkLobby = nullptr;
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage (cNetMessage& message)
{
	Log.write ("Menu: <-- " + message.getTypeAsString () + ", Hexdump: " + message.getHexDump (), cLog::eLOG_TYPE_NET_DEBUG);

	switch (message.iType)
	{
	case MU_MSG_CHAT: handleNetMessage_MU_MSG_CHAT (message); break;
	case TCP_CLOSE: handleNetMessage_TCP_CLOSE (message); break;
	case MU_MSG_REQ_IDENTIFIKATION: handleNetMessage_MU_MSG_REQ_IDENTIFIKATION (message); break;
	case MU_MSG_PLAYER_NUMBER: handleNetMessage_MU_MSG_PLAYER_NUMBER (message); break;
	case MU_MSG_PLAYERLIST: handleNetMessage_MU_MSG_PLAYERLIST (message); break;
	case MU_MSG_OPTINS: handleNetMessage_MU_MSG_OPTINS (message); break;
		//case MU_MSG_START_MAP_DOWNLOAD: initMapDownload (message); break;
		//case MU_MSG_MAP_DOWNLOAD_DATA: receiveMapData (message); break;
		//case MU_MSG_CANCELED_MAP_DOWNLOAD: canceledMapDownload (message); break;
		//case MU_MSG_FINISHED_MAP_DOWNLOAD: finishedMapDownload (message); break;
	case MU_MSG_GO: handleNetMessage_MU_MSG_GO (message); break;
	case MU_MSG_LANDING_STATE: handleNetMessage_MU_MSG_LANDING_STATE (message); break;
	case MU_MSG_ALL_LANDED: handleNetMessage_MU_MSG_ALL_LANDED (message); break;
	case GAME_EV_REQ_RECON_IDENT: handleNetMessage_GAME_EV_REQ_RECON_IDENT (message); break;
	case GAME_EV_RECONNECT_ANSWER: handleNetMessage_GAME_EV_RECONNECT_ANSWER (message); break;
    default:
        Log.write ("Client Menu Controller: Can not handle message type " + message.getTypeAsString (), cLog::eLOG_TYPE_NET_ERROR);
        break;
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_TCP_CLOSE (cNetMessage& message)
{
	assert (message.iType == TCP_CLOSE);

	if (!network || !windowNetworkLobby) return;

	windowNetworkLobby->removeNonLocalPlayers ();
	const auto& localPlayer = windowNetworkLobby->getLocalPlayer ();
	localPlayer->setReady (false);
	windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Lost_Connection", "server"));
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_CHAT (cNetMessage& message)
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
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_REQ_IDENTIFIKATION (cNetMessage& message)
{
	assert (message.iType == MU_MSG_REQ_IDENTIFIKATION);

	if (!network || !windowNetworkLobby) return;

	Log.write ("game version of server is: " + message.popString (), cLog::eLOG_TYPE_NET_DEBUG);
	windowNetworkLobby->getLocalPlayer ()->setNr (message.popInt16 ());
	sendIdentification (*network, *windowNetworkLobby->getLocalPlayer ());
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_PLAYER_NUMBER (cNetMessage& message)
{
	assert (message.iType == MU_MSG_PLAYER_NUMBER);

	if (!network || !windowNetworkLobby) return;

	windowNetworkLobby->getLocalPlayer ()->setNr (message.popInt16 ());
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_PLAYERLIST (cNetMessage& message)
{
	assert (message.iType == MU_MSG_PLAYERLIST);

	if (!network || !windowNetworkLobby) return;

	const auto& localPlayer = windowNetworkLobby->getLocalPlayer ();
	windowNetworkLobby->removeNonLocalPlayers ();

	int playerCount = message.popInt16 ();
	for (int i = 0; i < playerCount; i++)
	{
		auto name = message.popString ();
		auto colorIndex = message.popInt16 ();
		auto ready = message.popBool ();
		auto nr = message.popInt16 ();

		if (nr == localPlayer->getNr ())
		{
			localPlayer->setName (name);
			localPlayer->setColor (cPlayerColor(colorIndex));
			localPlayer->setReady (ready);
		}
		else
		{
			auto newPlayer = std::make_shared<sPlayer> (name, cPlayerColor(colorIndex), nr);
			newPlayer->setReady (ready);
			windowNetworkLobby->addPlayer (std::move (newPlayer));
		}
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_OPTINS (cNetMessage& message)
{
	assert (message.iType == MU_MSG_OPTINS);

	if (!network || !windowNetworkLobby) return;

	if (message.popBool ())
	{
		auto settings = std::make_unique<cGameSettings> ();
		settings->popFrom (message);
		windowNetworkLobby->setGameSettings (std::move (settings));
	}
	else
	{
		windowNetworkLobby->setGameSettings (nullptr);
	}

	if (message.popBool ())
	{
		auto mapName = message.popString ();
		int32_t mapCheckSum = message.popInt32 ();
		const auto& staticMap = windowNetworkLobby->getStaticMap ();
		if (!staticMap || staticMap->getName () != mapName)
		{
			bool mapCheckSumsEqual = (MapDownload::calculateCheckSum (mapName) == mapCheckSum);
			auto newStaticMap = std::make_shared<cStaticMap>();
			if (mapCheckSumsEqual && newStaticMap->loadMap (mapName))
			{
				triedLoadMapName = "";
			}
			else
			{
				const auto& localPlayer = windowNetworkLobby->getLocalPlayer ();
				if (localPlayer->isReady ())
				{
					windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~No_Map_No_Ready", mapName));
					localPlayer->setReady (false);
				}
				triedLoadMapName = mapName;

				auto existingMapFilePath = MapDownload::getExistingMapFilePath (mapName);
				bool existsMap = !existingMapFilePath.empty ();
				if (!mapCheckSumsEqual && existsMap)
				{
					windowNetworkLobby->addInfoEntry ("You have an incompatible version of the");
					windowNetworkLobby->addInfoEntry (std::string ("map \"") + mapName + "\" at");
					windowNetworkLobby->addInfoEntry (std::string ("\"") + existingMapFilePath + "\" !");
					windowNetworkLobby->addInfoEntry ("Move it away or delete it, then reconnect.");
				}
				else
				{
					if (MapDownload::isMapOriginal (mapName, mapCheckSum) == false)
					{
						if (mapName != lastRequestedMapName)
						{
							lastRequestedMapName = mapName;
							sendRequestMap (*network, mapName, localPlayer->getNr ());
							windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_DownloadRequest"));
							windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_Download", mapName));
						}
					}
					else
					{
						windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_DownloadRequestInvalid"));
						windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_DownloadInvalid", mapName));
					}
				}
			}
			windowNetworkLobby->setStaticMap (std::move (newStaticMap));
		}
	}
	else
	{
		windowNetworkLobby->setStaticMap (nullptr);
	}

	windowNetworkLobby->setSaveGame (message.popInt32());
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_GO (cNetMessage& message)
{
	assert (message.iType == MU_MSG_GO);

	windowLandingPositionSelection = nullptr;

	//saveOptions ();

	if (windowNetworkLobby->getSaveGameNumber () != -1)
	{
		startSavedGame ();
	}
	else
	{
		startGamePreparation ();
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_LANDING_STATE (cNetMessage& message)
{
	assert (message.iType == MU_MSG_LANDING_STATE);

	if (!windowLandingPositionSelection) return;

	eLandingPositionState state = (eLandingPositionState)message.popInt32 ();

	windowLandingPositionSelection->applyReselectionState (state);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_ALL_LANDED (cNetMessage& message)
{
	assert (message.iType == MU_MSG_ALL_LANDED);

	if (!newGame) return;

	startNewGame ();
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_GAME_EV_REQ_RECON_IDENT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REQ_RECON_IDENT);

	if (!network || !windowNetworkLobby) return;

	auto yesNoDialog = application.show (std::make_shared<cDialogYesNo>(lngPack.i18n ("Text~Multiplayer~Reconnect")));

	const auto socket = message.popInt16 ();

	signalConnectionManager.connect (yesNoDialog->yesClicked, [this, socket]()
	{
		sendGameIdentification (*network, *windowNetworkLobby->getLocalPlayer(), socket);
	});

	signalConnectionManager.connect (yesNoDialog->noClicked, [this]()
	{
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Connection_Terminated"));
		network->close (0);
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_GAME_EV_RECONNECT_ANSWER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_RECONNECT_ANSWER);

	if (!network || !windowNetworkLobby) return;

	if (message.popBool ())
	{
		const int localPlayerNumber = message.popInt16 ();
		const int localPlayerColorIndex = message.popInt16 ();

		const auto mapName = message.popString ();

		auto staticMap = std::make_shared<cStaticMap> ();
		if (!staticMap->loadMap (mapName)) return; // TODO: error message

		auto reconnectionGame = std::make_shared<cNetworkClientGameReconnection> ();

		int playerCount = message.popInt16 ();

		std::vector<std::shared_ptr<sPlayer>> players;
		players.push_back (std::make_shared<sPlayer> (windowNetworkLobby->getLocalPlayer ()->getName (), cPlayerColor(localPlayerColorIndex), localPlayerNumber));
		auto& localPlayer = *players.back ();
		while (playerCount > 1)
		{
			const auto playerName = message.popString ();
			const int playerColorIndex = message.popInt16 ();
			const int playerNr = message.popInt16 ();
			players.push_back (std::make_shared<sPlayer> (playerName, cPlayerColor(playerColorIndex), playerNr));
			playerCount--;
		}

		reconnectionGame->setNetwork (network);
		reconnectionGame->setStaticMap (staticMap);
		reconnectionGame->setPlayers (players, localPlayer);

		reconnectionGame->start (application);


		signalConnectionManager.connect (reconnectionGame->terminated, [&]()
		{
			application.closeTill (*windowNetworkLobby);
			windowNetworkLobby->close ();
			windowNetworkLobby = nullptr;
		});
	}
	else
	{
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Reconnect_Forbidden"));
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Connection_Terminated"));
		network->close (0);
	}
}
