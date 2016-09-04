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
#include "ui/graphical/application.h"
#include "ui/graphical/menu/windows/windownetworklobbyclient/windownetworklobbyclient.h"
#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "ui/graphical/menu/windows/windowlandingunitselection/windowlandingunitselection.h"
#include "ui/graphical/menu/windows/windowlandingpositionselection/windowlandingpositionselection.h"
#include "game/data/gamesettings.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/dialogs/dialogyesno.h"
#include "ui/graphical/menu/widgets/special/lobbychatboxlistviewitem.h"
#include "ui/graphical/menu/widgets/special/chatboxlandingplayerlistviewitem.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "game/startup/network/client/networkclientgamenew.h"
#include "game/startup/network/client/networkclientgamereconnection.h"
#include "game/startup/network/client/networkclientgamesaved.h"
#include "main.h"
#include "game/data/map/map.h"
#include "game/data/player/player.h"
#include "game/data/units/landingunit.h"
#include "network.h"
#include "utility/log.h"
#include "menuevents.h"
#include "game/logic/serverevents.h"
#include "netmessage.h"
#include "mapdownload.h"
#include "utility/files.h"

// TODO: remove
std::vector<std::pair<sID, int>> createInitialLandingUnitsList(int clan, const cGameSettings& gameSettings, const cUnitsData& unitsData); // defined in windowsingleplayer.cpp

//------------------------------------------------------------------------------
cMenuControllerMultiplayerClient::cMenuControllerMultiplayerClient (cApplication& application_) :
	application (application_),
	windowLandingPositionSelection (nullptr)
{}

//------------------------------------------------------------------------------
cMenuControllerMultiplayerClient::~cMenuControllerMultiplayerClient()
{
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::start()
{
	network = std::make_shared<cTCP> ();

	network->setMessageReceiver (this);

	windowNetworkLobby = std::make_shared<cWindowNetworkLobbyClient> ();
	triedLoadMapName = "";

	application.show (windowNetworkLobby);
	application.addRunnable (shared_from_this());

	signalConnectionManager.connect (windowNetworkLobby->terminated, std::bind (&cMenuControllerMultiplayerClient::reset, this));
	signalConnectionManager.connect (windowNetworkLobby->backClicked, [this]()
	{
		windowNetworkLobby->close();
		saveOptions();
	});

	signalConnectionManager.connect (windowNetworkLobby->wantLocalPlayerReadyChange, std::bind (&cMenuControllerMultiplayerClient::handleWantLocalPlayerReadyChange, this));
	signalConnectionManager.connect (windowNetworkLobby->triggeredChatMessage, std::bind (&cMenuControllerMultiplayerClient::handleChatMessageTriggered, this));

	signalConnectionManager.connect (windowNetworkLobby->triggeredConnect, std::bind (&cMenuControllerMultiplayerClient::connect, this));

	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer()->nameChanged, std::bind (&cMenuControllerMultiplayerClient::handleLocalPlayerAttributesChanged, this));
	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer()->colorChanged, std::bind (&cMenuControllerMultiplayerClient::handleLocalPlayerAttributesChanged, this));
	signalConnectionManager.connect (windowNetworkLobby->getLocalPlayer()->readyChanged, std::bind (&cMenuControllerMultiplayerClient::handleLocalPlayerAttributesChanged, this));
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::reset()
{
	network = nullptr;
	windowNetworkLobby = nullptr;
	windowLandingPositionSelection = nullptr;
	playersLandingStatus.clear();
	newGame = nullptr;
	application.removeRunnable (*this);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::pushEvent (std::unique_ptr<cNetMessage> message)
{
	messageQueue.push (std::move (message));
}

//------------------------------------------------------------------------------
std::unique_ptr<cNetMessage> cMenuControllerMultiplayerClient::popEvent()
{
	std::unique_ptr<cNetMessage> message;
	messageQueue.try_pop(message);
	return message;
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::run()
{
	std::unique_ptr<cNetMessage> message;
	while (messageQueue.try_pop (message))
	{
		handleNetMessage (*message);
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleWantLocalPlayerReadyChange()
{
	if (!network || !windowNetworkLobby) return;

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
void cMenuControllerMultiplayerClient::handleChatMessageTriggered()
{
	if (!network || !windowNetworkLobby) return;

	const auto& chatMessage = windowNetworkLobby->getChatMessage();

	if (chatMessage.empty()) return;

	auto& localPlayer = windowNetworkLobby->getLocalPlayer();

	if (localPlayer)
	{
		windowNetworkLobby->addChatEntry (localPlayer->getName(), chatMessage);
		sendMenuChatMessage (*network, chatMessage, nullptr, localPlayer->getNr());
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleLocalPlayerAttributesChanged()
{
	if (!network || !windowNetworkLobby) return;

	if (network->getConnectionStatus() == 0) return;

	sendIdentification (*network, *windowNetworkLobby->getLocalPlayer());
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::connect()
{
	if (!network || !windowNetworkLobby) return;

	// Connect only if there isn't a connection yet
	if (network->getConnectionStatus() != 0) return;

	const auto& ip = windowNetworkLobby->getIp();
	const auto& port = windowNetworkLobby->getPort();

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
		windowNetworkLobby->disablePortEdit();
		windowNetworkLobby->disableIpEdit();
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::startSavedGame()
{
	if (!network || !windowNetworkLobby || !windowNetworkLobby->getStaticMap() || !windowNetworkLobby->getGameSettings()) return;

	auto savedGame = std::make_shared<cNetworkClientGameSaved> ();

	savedGame->setNetwork (network);
	savedGame->setStaticMap (windowNetworkLobby->getStaticMap());
	savedGame->setGameSettings (windowNetworkLobby->getGameSettings());
	savedGame->setPlayers (windowNetworkLobby->getPlayersNotShared(), *windowNetworkLobby->getLocalPlayer());

	application.closeTill (*windowNetworkLobby);
	windowNetworkLobby->close();
	signalConnectionManager.connect (windowNetworkLobby->terminated, [&]() { windowNetworkLobby = nullptr; });

	savedGame->start (application);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::startGamePreparation()
{
	const auto& staticMap = windowNetworkLobby->getStaticMap();
	const auto& gameSettings = windowNetworkLobby->getGameSettings();

	if (!staticMap || !gameSettings || !network) return;

	newGame = std::make_shared<cNetworkClientGameNew> ();
	
	newGame->setPlayers (windowNetworkLobby->getPlayersNotShared(), *windowNetworkLobby->getLocalPlayer());
	newGame->setGameSettings (gameSettings);
	newGame->setStaticMap (staticMap);
	newGame->setNetwork (network);

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
void cMenuControllerMultiplayerClient::startClanSelection(bool isFirstWindowOnGamePreparation)
{
	if (!newGame) return;

	auto windowClanSelection = application.show (std::make_shared<cWindowClanSelection> (newGame->getUnitsData()));

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
void cMenuControllerMultiplayerClient::startLandingUnitSelection(bool isFirstWindowOnGamePreparation)
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
void cMenuControllerMultiplayerClient::startLandingPositionSelection()
{
	if (!newGame || !newGame->getStaticMap() || !network) return;

	auto& map = newGame->getStaticMap();
	bool fixedBridgeHead = newGame->getGameSettings()->getBridgeheadType() == eGameSettingsBridgeheadType::Definite;
	auto& landingUnits = newGame->getLandingUnits();
	auto& unitsData = newGame->getUnitsData();
	windowLandingPositionSelection = std::make_shared<cWindowLandingPositionSelection> (map, fixedBridgeHead, landingUnits, unitsData, true);

	signalConnectionManager.connect (windowLandingPositionSelection->opened, [this]()
	{
		sendInLandingPositionSelectionStatus (*network, newGame->getLocalPlayer(), true, nullptr);
	});
	signalConnectionManager.connect (windowLandingPositionSelection->closed, [this]()
	{
		sendInLandingPositionSelectionStatus (*network, newGame->getLocalPlayer(), false, nullptr);
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

		sendLandingPosition (*network, landingPosition, newGame->getLocalPlayer());
	});
	signalConnectionManager.connect (windowLandingPositionSelection->getChatBox()->commandEntered, [this] (const std::string & command)
	{
		const auto& localPlayer = newGame->getLocalPlayer();
		windowLandingPositionSelection->getChatBox()->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (localPlayer.getName(), command));
		cSoundDevice::getInstance().playSoundEffect (SoundData.SNDChat);
		sendMenuChatMessage (*network, command, nullptr, localPlayer.getNr());
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::startNewGame()
{
	if (!newGame) return;

	application.closeTill (*windowNetworkLobby);
	windowNetworkLobby->close();
	signalConnectionManager.connect (windowNetworkLobby->terminated, [&]() { windowNetworkLobby = nullptr; });

	newGame->start (application);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::checkReallyWantsToQuit()
{
	auto yesNoDialog = application.show(std::make_shared<cDialogYesNo>("Are you sure you want to abort the game preparation?")); // TODO: translate

	signalConnectionManager.connect(yesNoDialog->yesClicked, [this]()
	{
		sendPlayerHasAbortedGamePreparation(*network, newGame->getLocalPlayer());
		application.closeTill(*windowNetworkLobby);
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage (cNetMessage& message)
{
	Log.write ("Menu: <-- " + message.getTypeAsString() + ", Hexdump: " + message.getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);

	switch (message.iType)
	{
		case MU_MSG_CHAT: handleNetMessage_MU_MSG_CHAT (message); break;
		case TCP_CLOSE: handleNetMessage_TCP_CLOSE (message); break;
		case MU_MSG_REQ_IDENTIFIKATION: handleNetMessage_MU_MSG_REQ_IDENTIFIKATION (message); break;
		case MU_MSG_PLAYER_NUMBER: handleNetMessage_MU_MSG_PLAYER_NUMBER (message); break;
		case MU_MSG_PLAYERLIST: handleNetMessage_MU_MSG_PLAYERLIST (message); break;
		case MU_MSG_OPTINS: handleNetMessage_MU_MSG_OPTINS (message); break;
		case MU_MSG_START_MAP_DOWNLOAD: initMapDownload (message); break;
		case MU_MSG_MAP_DOWNLOAD_DATA: receiveMapData (message); break;
		case MU_MSG_CANCELED_MAP_DOWNLOAD: canceledMapDownload (message); break;
		case MU_MSG_FINISHED_MAP_DOWNLOAD: finishedMapDownload (message); break;
		case MU_MSG_GO: handleNetMessage_MU_MSG_GO (message); break;
		case MU_MSG_LANDING_STATE: handleNetMessage_MU_MSG_LANDING_STATE (message); break;
		case MU_MSG_ALL_LANDED: handleNetMessage_MU_MSG_ALL_LANDED (message); break;
		case GAME_EV_REQ_RECON_IDENT: handleNetMessage_GAME_EV_REQ_RECON_IDENT (message); break;
		case GAME_EV_RECONNECT_ANSWER: handleNetMessage_GAME_EV_RECONNECT_ANSWER (message); break;
		case MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS: handleNetMessage_MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS (message); break;
		case MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION: handleNetMessage_MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION (message); break;
		case MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION: handleNetMessage_MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION(message); break;
		default:
			Log.write ("Client Menu Controller: Can not handle message type " + message.getTypeAsString(), cLog::eLOG_TYPE_NET_ERROR);
			break;
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_TCP_CLOSE (cNetMessage& message)
{
	assert (message.iType == TCP_CLOSE);

	if (!network || !windowNetworkLobby) return;

	windowNetworkLobby->removeNonLocalPlayers();
	const auto& localPlayer = windowNetworkLobby->getLocalPlayer();
	localPlayer->setReady (false);
	windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Lost_Connection", "server"));
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_CHAT (cNetMessage& message)
{
	assert (message.iType == MU_MSG_CHAT);

	if (!network || !windowNetworkLobby) return;

	bool translationText = message.popBool();
	auto chatText = message.popString();

	auto players = windowNetworkLobby->getPlayers();
	auto iter = std::find_if (players.begin(), players.end(), [ = ] (const std::shared_ptr<cPlayerBasicData>& player) { return player->getNr() == message.iPlayerNr; });

	const auto playerName = iter == players.end() ? "unknown" : (*iter)->getName();

	if (windowLandingPositionSelection)
	{
		if (translationText)
		{
			windowLandingPositionSelection->getChatBox()->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (lngPack.i18n (chatText)));
		}
		else
		{
			windowLandingPositionSelection->getChatBox()->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (playerName, chatText));
			cSoundDevice::getInstance().playSoundEffect (SoundData.SNDChat);
		}
	}
	else
	{
		if (translationText)
		{
			windowNetworkLobby->addInfoEntry (lngPack.i18n (chatText));
		}
		else
		{
			windowNetworkLobby->addChatEntry (playerName, chatText);
		}
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_REQ_IDENTIFIKATION (cNetMessage& message)
{
	assert (message.iType == MU_MSG_REQ_IDENTIFIKATION);

	if (!network || !windowNetworkLobby) return;

	Log.write ("game version of server is: " + message.popString(), cLog::eLOG_TYPE_NET_DEBUG);
	windowNetworkLobby->getLocalPlayer()->setNr (message.popInt16());
	sendIdentification (*network, *windowNetworkLobby->getLocalPlayer());
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_PLAYER_NUMBER (cNetMessage& message)
{
	assert (message.iType == MU_MSG_PLAYER_NUMBER);

	if (!network || !windowNetworkLobby) return;

	windowNetworkLobby->getLocalPlayer()->setNr (message.popInt16());
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_PLAYERLIST (cNetMessage& message)
{
	assert (message.iType == MU_MSG_PLAYERLIST);

	if (!network || !windowNetworkLobby) return;

	const auto& localPlayer = windowNetworkLobby->getLocalPlayer();
	windowNetworkLobby->removeNonLocalPlayers();

	int playerCount = message.popInt16();
	for (int i = 0; i < playerCount; i++)
	{
		auto name = message.popString();
		auto color = message.popColor();
		auto ready = message.popBool();
		auto nr = message.popInt16();

		if (nr == localPlayer->getNr())
		{
			localPlayer->setName (name);
			localPlayer->setColor (cPlayerColor (color));
			localPlayer->setReady (ready);
		}
		else
		{
			auto newPlayer = std::make_shared<cPlayerBasicData> (name, cPlayerColor (color), nr);
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

	//TODO: reimplement

	//pop game settings
	if (message.popBool())
	{
		auto settings = std::make_unique<cGameSettings> ();
		//settings->popFrom (message);
		windowNetworkLobby->setGameSettings (std::move (settings));
	}
	else
	{
		windowNetworkLobby->setGameSettings (nullptr);
	}

	//pop map setting
	if (message.popBool())
	{
		auto mapName = message.popString();
		int32_t mapCheckSum = message.popInt32();
		const auto& staticMap = windowNetworkLobby->getStaticMap();
		if (!staticMap || staticMap->getName() != mapName)
		{
			bool mapCheckSumsEqual = (MapDownload::calculateCheckSum (mapName) == mapCheckSum);
			auto newStaticMap = std::make_shared<cStaticMap>();
			if (mapCheckSumsEqual && newStaticMap->loadMap (mapName))
			{
				triedLoadMapName = "";
			}
			else
			{
				const auto& localPlayer = windowNetworkLobby->getLocalPlayer();
				if (localPlayer->isReady())
				{
					windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~No_Map_No_Ready", mapName));
					localPlayer->setReady (false);
				}
				triedLoadMapName = mapName;

				auto existingMapFilePath = MapDownload::getExistingMapFilePath (mapName);
				bool existsMap = !existingMapFilePath.empty();
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
							sendRequestMap (*network, mapName, localPlayer->getNr());
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

	//pop save geame settings
	std::vector<cPlayerBasicData> savePlayerList;
	for (int nr = message.popInt32(); nr > 0; nr--)
	{
		std::string playerName = message.popString();
		int playerNr = message.popInt32();
		savePlayerList.push_back (cPlayerBasicData (playerName, cPlayerColor (cRgbColor (0, 0, 0)), playerNr));
	}
	std::string saveGameName = message.popString();

	windowNetworkLobby->setSaveGame (savePlayerList, saveGameName);

}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_GO (cNetMessage& message)
{
	assert (message.iType == MU_MSG_GO);

	windowLandingPositionSelection = nullptr;

	saveOptions();

	if (windowNetworkLobby->getSaveGamePlayers().size() != 0)
	{
		startSavedGame();
	}
	else
	{
		startGamePreparation();
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_LANDING_STATE (cNetMessage& message)
{
	assert (message.iType == MU_MSG_LANDING_STATE);

	if (!windowLandingPositionSelection) return;

	eLandingPositionState state = (eLandingPositionState)message.popInt32();

	windowLandingPositionSelection->applyReselectionState (state);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_ALL_LANDED (cNetMessage& message)
{
	assert (message.iType == MU_MSG_ALL_LANDED);

	if (!newGame) return;

	startNewGame();
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_GAME_EV_REQ_RECON_IDENT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REQ_RECON_IDENT);

	if (!network || !windowNetworkLobby) return;

	auto yesNoDialog = application.show (std::make_shared<cDialogYesNo> (lngPack.i18n ("Text~Multiplayer~Reconnect")));

	const auto socket = message.popInt16();

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

	if (message.popBool())
	{
		const int localPlayerNumber = message.popInt16();
		const auto localPlayerColor = message.popColor();

		const auto mapName = message.popString();

		auto staticMap = std::make_shared<cStaticMap> ();
		if (!staticMap->loadMap (mapName)) return; // TODO: error message

		auto reconnectionGame = std::make_shared<cNetworkClientGameReconnection> ();

		int playerCount = message.popInt16();

		std::vector<cPlayerBasicData> players;
		players.push_back (cPlayerBasicData (windowNetworkLobby->getLocalPlayer()->getName(), cPlayerColor (localPlayerColor), localPlayerNumber));
		const auto localPlayer = players.back();
		while (playerCount > 1)
		{
			const auto playerName = message.popString();
			const auto playerColor = message.popColor();
			const int playerNr = message.popInt16();
			players.push_back (cPlayerBasicData (playerName, cPlayerColor (playerColor), playerNr));
			playerCount--;
		}

		reconnectionGame->setNetwork (network);
		reconnectionGame->setStaticMap (staticMap);
		reconnectionGame->setPlayers (players, localPlayer);

		application.closeTill (*windowNetworkLobby);
		windowNetworkLobby->close();
		signalConnectionManager.connect (windowNetworkLobby->terminated, [&]() { windowNetworkLobby = nullptr; });

		reconnectionGame->start (application);
	}
	else
	{
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Reconnect_Forbidden"));
		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Connection_Terminated"));
		network->close (0);
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS (cNetMessage& message)
{
	assert (message.iType == MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS);

	if (!network || !windowNetworkLobby) return;

	auto players = windowNetworkLobby->getPlayers();

	const auto isIn = message.popBool();
	const auto playerNr = message.popInt32();

	if (isIn)
	{
		auto players = windowNetworkLobby->getPlayers();
		auto iter = std::find_if (players.begin(), players.end(), [playerNr] (const std::shared_ptr<cPlayerBasicData>& player) { return player->getNr() == playerNr; });

		assert (iter != players.end());

		const auto& player = **iter;

		playersLandingStatus.push_back (std::make_unique<cPlayerLandingStatus> (player));
		if (windowLandingPositionSelection) windowLandingPositionSelection->getChatBox()->addPlayerEntry (std::make_unique<cChatBoxLandingPlayerListViewItem> (*playersLandingStatus.back()));
	}
	else
	{
		if (windowLandingPositionSelection) windowLandingPositionSelection->getChatBox()->removePlayerEntry (playerNr);
		playersLandingStatus.erase (std::remove_if (playersLandingStatus.begin(), playersLandingStatus.end(), [playerNr] (const std::unique_ptr<cPlayerLandingStatus>& status) { return status->getPlayer().getNr() == playerNr; }), playersLandingStatus.end());
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION (cNetMessage& message)
{
	assert (message.iType == MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION);

	if (!network || !windowNetworkLobby) return;

	const auto playerNr = message.popInt32();

	auto iter = std::find_if (playersLandingStatus.begin(), playersLandingStatus.end(), [playerNr] (const std::unique_ptr<cPlayerLandingStatus>& entry) { return entry->getPlayer().getNr() == playerNr; });

	assert (iter != playersLandingStatus.end());

	auto& playerLandingStatus = **iter;

	playerLandingStatus.setHasSelectedPosition (true);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION(cNetMessage & message)
{
	auto players = windowNetworkLobby->getPlayers();

	const auto playerNr = message.popInt32();

	auto iter = std::find_if(players.begin(), players.end(), [playerNr](const std::shared_ptr<cPlayerBasicData>& player) { return player->getNr() == playerNr; });
	if(iter == players.end()) return;

	const auto& player = **iter;

	auto yesNoDialog = application.show(std::make_shared<cDialogOk>("Player " + player.getName() + " has quit from game preparation")); // TODO: translate

	signalConnectionManager.connect(yesNoDialog->done, [this]()
	{
		application.closeTill(*windowNetworkLobby);
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::initMapDownload (cNetMessage& message)
{
	const auto mapSize = message.popInt32();
	const auto mapName = message.popString();

	mapReceiver = std::make_unique<cMapReceiver> (mapName, mapSize);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::receiveMapData (cNetMessage& message)
{
	if (mapReceiver == nullptr) return;

	mapReceiver->receiveData (message);

	if (windowNetworkLobby != nullptr)
	{
		const int percent = mapReceiver->getBytesReceivedPercent();

		windowNetworkLobby->setMapDownloadPercent (percent);
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::canceledMapDownload (cNetMessage& message)
{
	if (mapReceiver == nullptr) return;

	mapReceiver = nullptr;

	if (windowNetworkLobby != nullptr)
	{
		windowNetworkLobby->setMapDownloadCanceled();
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::finishedMapDownload (cNetMessage& message)
{
	if (mapReceiver == nullptr) return;

	mapReceiver->finished();

	auto staticMap = std::make_shared<cStaticMap> ();

	if (!staticMap->loadMap (mapReceiver->getMapName())) staticMap = nullptr;

	if (windowNetworkLobby != nullptr)
	{
		windowNetworkLobby->setStaticMap (staticMap);

		windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_Finished"));
	}

	mapReceiver = nullptr;
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
