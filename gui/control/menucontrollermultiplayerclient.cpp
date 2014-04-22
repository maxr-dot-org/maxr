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
#include "../application.h"
#include "../menu/windows/windownetworklobbyclient/windownetworklobbyclient.h"
#include "../menu/windows/windowclanselection/windowclanselection.h"
#include "../menu/windows/windowlandingunitselection/windowlandingunitselection.h"
#include "../menu/windows/windowlandingpositionselection/windowlandingpositionselection.h"
#include "../menu/windows/windowgamesettings/gamesettings.h"
#include "../menu/dialogs/dialogok.h"
#include "../../game/network/host/networkhostgamenew.h"
#include "../../game/logic/landingpositionmanager.h"
#include "../../main.h"
#include "../../map.h"
#include "../../player.h"
#include "../../menus.h"
#include "../../network.h"
#include "../../log.h"
#include "../../menuevents.h"
#include "../../netmessage.h"
#include "../../mapdownload.h"

// FIXME: remove
std::vector<std::pair<sID, int>> createInitialLandingUnitsList (int clan, const cGameSettings& gameSettings); // defined in windowsingleplayer.cpp

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::start (cApplication& application)
{
	network = std::make_shared<cTCP> ();

	network->setMessageReceiver (this);

	windowNetworkLobby = std::make_shared<cWindowNetworkLobbyClient> ();
	triedLoadMapName = "";

	application.show (windowNetworkLobby);
	application.addRunnable (shared_from_this ());

	signalConnectionManager.connect (windowNetworkLobby->terminated, [&]()
	{
		application.removeRunnable (*this);
	});

	signalConnectionManager.connect (windowNetworkLobby->wantLocalPlayerReadyChange, std::bind (&cMenuControllerMultiplayerClient::handleWantLocalPlayerReadyChange, this));
	signalConnectionManager.connect (windowNetworkLobby->triggeredChatMessage, std::bind (&cMenuControllerMultiplayerClient::handleChatMessageTriggered, this));

	signalConnectionManager.connect (windowNetworkLobby->triggeredConnect, std::bind (&cMenuControllerMultiplayerClient::connect, this));

	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer ()->nameChanged, std::bind (&cMenuControllerMultiplayerClient::handleLocalPlayerAttributesChanged, this));
	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer ()->colorChanged, std::bind (&cMenuControllerMultiplayerClient::handleLocalPlayerAttributesChanged, this));
	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer ()->readyChanged, std::bind (&cMenuControllerMultiplayerClient::handleLocalPlayerAttributesChanged, this));
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

	// FIXME: make this non blocking!
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
void cMenuControllerMultiplayerClient::startGamePreparation (cApplication& application)
{
	//auto game = std::make_shared<cNetworkHostGameNew> ();

	//game->setPlayers (windowNetworkLobby->getPlayers (), *windowNetworkLobby->getLocalPlayer ());
	//game->setGameSettings (windowNetworkLobby->getGameSettings ());
	//game->setStaticMap (windowNetworkLobby->getStaticMap ());

	//if (game->getGameSettings()->getClansEnabled ())
	//{
	//	startClanSelection (application, game);
	//}
	//else
	//{
	//	startLandingUnitSelection (application, game);
	//}
}

////------------------------------------------------------------------------------
//void cMenuControllerMultiplayerClient::startClanSelection (cApplication& application, const std::shared_ptr<cNetworkHostGameNew>& game)
//{
//	auto windowClanSelection = application.show (std::make_shared<cWindowClanSelection> ());
//
//	signalConnectionManager.connect (windowClanSelection->done, [this, windowClanSelection, game, &application]()
//	{
//		game->setLocalPlayerClan (windowClanSelection->getSelectedClan ());
//
//		startLandingUnitSelection (application, game);
//	});
//}
//
////------------------------------------------------------------------------------
//void cMenuControllerMultiplayerClient::startLandingUnitSelection (cApplication& application, const std::shared_ptr<cNetworkHostGameNew>& game)
//{
//	auto initialLandingUnits = createInitialLandingUnitsList (game->getLocalPlayerClan(), *game->getGameSettings());
//
//	auto windowLandingUnitSelection = application.show (std::make_shared<cWindowLandingUnitSelection> (0, game->getLocalPlayerClan (), initialLandingUnits, game->getGameSettings ()->getStartCredits ()));
//
//	signalConnectionManager.connect (windowLandingUnitSelection->done, [this, game, windowLandingUnitSelection, &application]()
//	{
//		game->setLocalPlayerLandingUnits (windowLandingUnitSelection->getLandingUnits ());
//		game->setLocalPlayerUnitUpgrades (windowLandingUnitSelection->getUnitUpgrades ());
//
//		startLandingPositionSelection (application, game);
//	});
//}
//
////------------------------------------------------------------------------------
//void cMenuControllerMultiplayerClient::startLandingPositionSelection (cApplication& application, const std::shared_ptr<cNetworkHostGameNew>& game)
//{
//	auto landingPositionManager = std::make_shared<cLandingPositionManager> (game->getPlayers());
//
//	auto windowLandingPositionSelection = application.show (std::make_shared<cWindowLandingPositionSelection> (game->getStaticMap()));
//
//	signalConnectionManager.connect (windowLandingPositionSelection->selectedPosition, [landingPositionManager, game](cPosition landingPosition)
//	{
//		landingPositionManager->setLandingPosition (*game->getLocalPlayer(), landingPosition);
//	});
//
//	signalConnectionManager.connect (landingPositionManager->allPositionsValid, [this, game, windowLandingPositionSelection, &application]()
//	{
//		game->setLocalPlayerLandingPosition (windowLandingPositionSelection->getSelectedPosition ());
//
//		startGame (application, game);
//	});
//}
//
////------------------------------------------------------------------------------
//void cMenuControllerMultiplayerClient::startGame (cApplication& application, const std::shared_ptr<cNetworkHostGameNew>& game)
//{
//	game->start (application);
//
//	signalConnectionManager.connect(game->terminated, [&]()
//	{
//		application.closeTill (*windowNetworkLobby);
//		windowNetworkLobby->close ();
//		windowNetworkLobby = nullptr;
//	});
//}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage (cNetMessage& message)
{
	Log.write ("Menu: <-- " + message.getTypeAsString () + ", Hexdump: " + message.getHexDump (), cLog::eLOG_TYPE_NET_DEBUG);

	switch (message.iType)
	{
	case MU_MSG_CHAT: handleNetMessage_MU_MSG_CHAT (message); break;
	case TCP_CLOSE: handleNetMessage_TCP_CLOSE (message); break;
	case MU_MSG_REQ_IDENTIFIKATION: handleNetMessage_MU_MSG_REQ_IDENTIFIKATION (message); break;
	case MU_MSG_PLAYERLIST: handleNetMessage_MU_MSG_PLAYERLIST (message); break;
	case MU_MSG_OPTINS: handleNetMessage_MU_MSG_OPTINS (message); break;
		//case MU_MSG_START_MAP_DOWNLOAD: initMapDownload (message); break;
		//case MU_MSG_MAP_DOWNLOAD_DATA: receiveMapData (message); break;
		//case MU_MSG_CANCELED_MAP_DOWNLOAD: canceledMapDownload (message); break;
		//case MU_MSG_FINISHED_MAP_DOWNLOAD: finishedMapDownload (message); break;
		//case MU_MSG_GO: handleNetMessage_MU_MSG_GO (message); break;
		//case GAME_EV_REQ_RECON_IDENT: handleNetMessage_GAME_EV_REQ_RECON_IDENT (message); break;
		//case GAME_EV_RECONNECT_ANSWER: handleNetMessage_GAME_EV_RECONNECT_ANSWER (message); break;
	default: break;
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
		auto color = message.popInt16 ();
		auto ready = message.popBool ();
		auto nr = message.popInt16 ();

		if (nr == localPlayer->getNr ())
		{
			localPlayer->setName (name);
			localPlayer->setColorIndex (color);
			localPlayer->setReady (ready);
		}
		else
		{
			auto newPlayer = std::make_shared<sPlayer> (name, color, nr);
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