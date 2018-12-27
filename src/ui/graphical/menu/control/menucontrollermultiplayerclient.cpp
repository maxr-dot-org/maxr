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
#include "utility/log.h"
#include "menuevents.h"
#include "mapdownload.h"
#include "utility/files.h"
#include "utility/string/toString.h"
#include "game/logic/client.h"

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
	connectionLost = false;

	connectionManager = std::make_shared<cConnectionManager> ();
	connectionManager->setLocalClient (this, -1);

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
	connectionManager = nullptr;
	windowNetworkLobby = nullptr;
	windowLandingPositionSelection = nullptr;
	playersLandingStatus.clear();
	newGame = nullptr;
	application.removeRunnable (*this);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::pushMessage (std::unique_ptr<cNetMessage2> message)
{
	messageQueue.push (std::move (message));
}

//------------------------------------------------------------------------------
std::unique_ptr<cNetMessage2> cMenuControllerMultiplayerClient::popMessage()
{
	std::unique_ptr<cNetMessage2> message;
	messageQueue.try_pop(message);
	return message;
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::run()
{
	std::unique_ptr<cNetMessage2> message;
	while (messageQueue.try_pop (message))
	{
		handleNetMessage (*message);
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleWantLocalPlayerReadyChange()
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
void cMenuControllerMultiplayerClient::handleChatMessageTriggered()
{
	if (!connectionManager || !windowNetworkLobby) return;

	const auto& chatMessage = windowNetworkLobby->getChatMessage();

	if (chatMessage.empty()) return;

	const auto& localPlayer = windowNetworkLobby->getLocalPlayer();

	if (localPlayer)
	{
		windowNetworkLobby->addChatEntry (localPlayer->getName(), chatMessage);
		sendNetMessage(cMuMsgChat(chatMessage));
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleLocalPlayerAttributesChanged()
{
	if (!connectionManager || !windowNetworkLobby) return;
	if (!connectionManager->isConnectedToServer()) return;

	const auto& player = windowNetworkLobby->getLocalPlayer();

	sendNetMessage(cMuMsgIdentification(*player));
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::connect()
{
	if (!connectionManager || !windowNetworkLobby) return;

	// Connect only if there isn't a connection yet
	if (connectionManager->isConnectedToServer()) return;

	const auto& ip = windowNetworkLobby->getIp();
	const auto& port = windowNetworkLobby->getPort();
	const auto& localPlayer = windowNetworkLobby->getLocalPlayer();

	windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Connecting") + ip + ":" + iToStr (port));    // e.g. Connecting to 127.0.0.1:55800
	windowNetworkLobby->disablePortEdit();
	windowNetworkLobby->disableIpEdit();

	Log.write (("Connecting to " + ip + ":" + iToStr (port)), cLog::eLOG_TYPE_NET_DEBUG);

	connectionManager->connectToServer(ip, port, *localPlayer);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::startSavedGame()
{
	if (!connectionManager || !windowNetworkLobby || !windowNetworkLobby->getStaticMap()) return;

	auto savedGame = std::make_shared<cNetworkClientGameSaved> ();

	savedGame->setConnectionManager (connectionManager);
	savedGame->setStaticMap (windowNetworkLobby->getStaticMap());
	savedGame->setPlayers(windowNetworkLobby->getSaveGameInfo().players, *windowNetworkLobby->getLocalPlayer());

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
void cMenuControllerMultiplayerClient::startGamePreparation(cMuMsgStartGamePreparations& message)
{
	const auto& staticMap = windowNetworkLobby->getStaticMap();
	const auto& gameSettings = windowNetworkLobby->getGameSettings();

	if (!staticMap || !gameSettings || !connectionManager) return;

	newGame = std::make_shared<cNetworkClientGameNew> ();

	newGame->setUnitsData(message.unitsData);
	newGame->setClanData(message.clanData);
	
	newGame->setPlayers (windowNetworkLobby->getPlayersNotShared(), *windowNetworkLobby->getLocalPlayer());
	newGame->setGameSettings (gameSettings);
	newGame->setStaticMap (staticMap);
	newGame->setConnectionManager (connectionManager);

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
	if (!newGame || !newGame->getStaticMap() || !connectionManager) return;

	auto& map = newGame->getStaticMap();
	bool fixedBridgeHead = newGame->getGameSettings()->getBridgeheadType() == eGameSettingsBridgeheadType::Definite;
	auto& landingUnits = newGame->getLandingUnits();
	auto unitsData = newGame->getUnitsData();
	windowLandingPositionSelection = std::make_shared<cWindowLandingPositionSelection> (map, fixedBridgeHead, landingUnits, unitsData, true);

	signalConnectionManager.connect (windowLandingPositionSelection->opened, [this]()
	{
		if (newGame)
		{
			sendNetMessage(cMuMsgInLandingPositionSelectionStatus(newGame->getLocalPlayer().getNr(), true));
		}
	});
	signalConnectionManager.connect (windowLandingPositionSelection->closed, [this]()
	{
		if (newGame)
		{
			sendNetMessage(cMuMsgInLandingPositionSelectionStatus(newGame->getLocalPlayer().getNr(), false));
		}
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

		sendNetMessage(cMuMsgLandingPosition(landingPosition));
	});
	signalConnectionManager.connect (windowLandingPositionSelection->getChatBox()->commandEntered, [this] (const std::string & command)
	{
		const auto& localPlayer = newGame->getLocalPlayer();
		windowLandingPositionSelection->getChatBox()->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (localPlayer.getName(), command));
		cSoundDevice::getInstance().playSoundEffect (SoundData.SNDChat);
		sendNetMessage(cMuMsgChat(command));
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
void cMenuControllerMultiplayerClient::reconnectToGame(const cNetMessageGameAlreadyRunning& message)
{
	auto staticMap = std::make_shared<cStaticMap>();
	
	if (!staticMap->loadMap(message.mapName))
	{
		application.show(std::make_shared<cDialogOk>("Map \"" + message.mapName + "\" not found")); //TODO: translate
		connectionManager->disconnectAll();
		return;
	}
	else if (MapDownload::calculateCheckSum(message.mapName) != message.mapCrc)
	{
		application.show(std::make_shared<cDialogOk>("The map \"" + message.mapName + "\" does not match the map of the server")); // TODO: translate
		connectionManager->disconnectAll();
		return;
	}
	
	auto reconnectionGame = std::make_shared<cNetworkClientGameReconnection>();

	reconnectionGame->setConnectionManager(connectionManager);
	reconnectionGame->setStaticMap(staticMap);
	reconnectionGame->setPlayers(message.playerList, *windowNetworkLobby->getLocalPlayer());

	sendNetMessage(cNetMessageWantRejoinGame());

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
void cMenuControllerMultiplayerClient::checkReallyWantsToQuit()
{
	auto yesNoDialog = application.show(std::make_shared<cDialogYesNo>("Are you sure you want to abort the game preparation?")); // TODO: translate

	signalConnectionManager.connect(yesNoDialog->yesClicked, [this]()
	{
		sendNetMessage(cMuMsgPlayerAbortedGamePreparations());
		application.closeTill(*windowNetworkLobby);
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage (cNetMessage2& message)
{
	cTextArchiveIn archive;
	archive << message;
	Log.write("Menu: <-- " + archive.data(), cLog::eLOG_TYPE_NET_DEBUG);

	switch (message.getType())
	{
		case eNetMessageType::TCP_CONNECTED:
			handleNetMessage_TCP_CONNECTED(static_cast<cNetMessageTcpConnected&>(message));
			return;	
		case eNetMessageType::TCP_CONNECT_FAILED:
			handleNetMessage_TCP_CONNECT_FAILED(static_cast<cNetMessageTcpConnectFailed&>(message));
			return;
		case eNetMessageType::TCP_CLOSE:
			handleNetMessage_TCP_CLOSE(static_cast<cNetMessageTcpClose&>(message));
			return;
		case eNetMessageType::MULTIPLAYER_LOBBY:
			//will be handled in the next switch statement
			break;
		case eNetMessageType::GAME_ALREADY_RUNNING:
			handleNetMessage_GAME_ALREADY_RUNNING(static_cast<cNetMessageGameAlreadyRunning&>(message));
			return;
		default:
			Log.write("Client Menu Controller: Can not handle message", cLog::eLOG_TYPE_NET_ERROR);
			return;
	}

	cMultiplayerLobbyMessage& muMessage = static_cast<cMultiplayerLobbyMessage&>(message);

	switch (muMessage.getType())
	{
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_CHAT: 
			handleNetMessage_MU_MSG_CHAT(static_cast<cMuMsgChat&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_NUMBER: 
			handleNetMessage_MU_MSG_PLAYER_NUMBER(static_cast<cMuMsgPlayerNr&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYERLIST: 
			handleNetMessage_MU_MSG_PLAYERLIST(static_cast<cMuMsgPlayerList&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_OPTIONS: 
			handleNetMessage_MU_MSG_OPTIONS(static_cast<cMuMsgOptions&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_START_MAP_DOWNLOAD:
			initMapDownload(static_cast<cMuMsgStartMapDownload&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_MAP_DOWNLOAD_DATA: 
			receiveMapData(static_cast<cMuMsgMapDownloadData&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_CANCELED_MAP_DOWNLOAD:
			canceledMapDownload(static_cast<cMuMsgCanceledMapDownload&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_FINISHED_MAP_DOWNLOAD:
			finishedMapDownload(static_cast<cMuMsgFinishedMapDownload&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_START_GAME_PREPARATIONS: 
			handleNetMessage_MU_MSG_START_GAME_PREPARATIONS(static_cast<cMuMsgStartGamePreparations&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_LANDING_STATE: 
			handleNetMessage_MU_MSG_LANDING_STATE(static_cast<cMuMsgLandingState&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_START_GAME: 
			handleNetMessage_MU_MSG_START_GAME(static_cast<cMuMsgStartGame&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS: 
			handleNetMessage_MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS(static_cast<cMuMsgInLandingPositionSelectionStatus&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION: 
			handleNetMessage_MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION(static_cast<cMuMsgPlayerHasSelectedLandingPosition&>(muMessage));
			break;
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION: 
			handleNetMessage_MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION(static_cast<cMuMsgPlayerAbortedGamePreparations&>(muMessage));
			break;
		default:
			Log.write ("Client Menu Controller: Can not handle message", cLog::eLOG_TYPE_NET_ERROR);
			break;
	}
}
//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_TCP_CONNECTED(cNetMessageTcpConnected& message)
{
	if (!windowNetworkLobby) return;

	auto& localPlayer = windowNetworkLobby->getLocalPlayer();
	localPlayer->setNr(message.playerNr);

	sendNetMessage(cMuMsgIdentification(*localPlayer));

	windowNetworkLobby->addInfoEntry(lngPack.i18n("Text~Multiplayer~Network_Connected"));

	if (message.packageVersion != PACKAGE_VERSION || message.packageRev != PACKAGE_REV)
	{
		windowNetworkLobby->addInfoEntry(lngPack.i18n("Text~Multiplayer~Gameversion_Warning_Client", message.packageVersion + " " + message.packageRev));
		windowNetworkLobby->addInfoEntry(lngPack.i18n("Text~Multiplayer~Gameversion_Own", (std::string)PACKAGE_VERSION + " " + PACKAGE_REV));
	}

	Log.write("Connected and assigned playerNr: " + toString(message.playerNr), cLog::eLOG_TYPE_INFO);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_TCP_CONNECT_FAILED(cNetMessageTcpConnectFailed& message)
{
	if (message.reason.empty())
	{
		windowNetworkLobby->addInfoEntry(lngPack.i18n("Text~Multiplayer~Network_Error_Connect", "server"));
	}
	else
	{
		windowNetworkLobby->addInfoEntry(lngPack.i18n(message.reason));
	}

	Log.write("Error on connecting to server", cLog::eLOG_TYPE_WARNING);

	windowNetworkLobby->enablePortEdit();
	windowNetworkLobby->enableIpEdit();
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_TCP_CLOSE (cNetMessageTcpClose& message)
{
	if (!connectionManager || !windowNetworkLobby) return;

	windowNetworkLobby->removePlayers();
	const auto& localPlayer = windowNetworkLobby->getLocalPlayer();
	localPlayer->setReady (false);
	windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Lost_Connection", "server"));

	windowNetworkLobby->enablePortEdit();
	windowNetworkLobby->enableIpEdit();
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_CHAT (cMuMsgChat& message)
{
	if (!connectionManager || !windowNetworkLobby) return;
	
	auto player = windowNetworkLobby->getPlayer(message.playerNr);
	const auto playerName = player == nullptr ? "unknown" : player->getName();

	if (windowLandingPositionSelection)
	{
		if (message.translate)
		{
			windowLandingPositionSelection->getChatBox()->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (lngPack.i18n (message.message, message.insertText)));
		}
		else
		{
			windowLandingPositionSelection->getChatBox()->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (playerName, message.message));
			cSoundDevice::getInstance().playSoundEffect (SoundData.SNDChat);
		}
	}
	else
	{
		if (message.translate)
		{
			windowNetworkLobby->addInfoEntry (lngPack.i18n (message.message, message.insertText));
		}
		else
		{
			windowNetworkLobby->addChatEntry (playerName, message.message);
		}
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_PLAYER_NUMBER(cMuMsgPlayerNr& message)
{
	if (!connectionManager || !windowNetworkLobby) return;

	windowNetworkLobby->getLocalPlayer()->setNr (message.newPlayerNr);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_PLAYERLIST(cMuMsgPlayerList& message)
{
	if (!connectionManager || !windowNetworkLobby) return;

	const auto localPlayer = windowNetworkLobby->getLocalPlayer();
	windowNetworkLobby->removePlayers();

	for (const auto& playerData : message.playerList)
	{
		if (playerData.getNr() == localPlayer->getNr())
		{
			*localPlayer = playerData;
			windowNetworkLobby->addPlayer(std::move(localPlayer));
		}
		else
		{
			auto newPlayer = std::make_shared<cPlayerBasicData>(playerData);
			windowNetworkLobby->addPlayer(std::move(newPlayer));
		}
	}
	windowNetworkLobby->updatePlayerListView();
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_OPTIONS(cMuMsgOptions& message)
{
	if (!connectionManager || !windowNetworkLobby) return;

	if (message.settingsValid)
	{
		auto settings = std::make_unique<cGameSettings> (message.settings);
		windowNetworkLobby->setGameSettings (std::move (settings));
	}
	else
	{
		windowNetworkLobby->setGameSettings (nullptr);
	}

	if (!message.mapName.empty())
	{
		const auto& staticMap = windowNetworkLobby->getStaticMap();
		if (!staticMap || staticMap->getName() != message.mapName)
		{
			bool mapCheckSumsEqual = (MapDownload::calculateCheckSum (message.mapName) == message.mapCrc);
			auto newStaticMap = std::make_shared<cStaticMap>();
			if (mapCheckSumsEqual && newStaticMap->loadMap (message.mapName))
			{
				triedLoadMapName = "";
			}
			else
			{
				const auto& localPlayer = windowNetworkLobby->getLocalPlayer();
				if (localPlayer->isReady())
				{
					windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~No_Map_No_Ready", message.mapName));
					localPlayer->setReady (false);
				}
				triedLoadMapName = message.mapName;

				auto existingMapFilePath = MapDownload::getExistingMapFilePath (message.mapName);
				bool existsMap = !existingMapFilePath.empty();
				if (!mapCheckSumsEqual && existsMap)
				{
					windowNetworkLobby->addInfoEntry ("You have an incompatible version of the");  //TODO: translate
					windowNetworkLobby->addInfoEntry (std::string ("map \"") + message.mapName + "\" at");
					windowNetworkLobby->addInfoEntry (std::string ("\"") + existingMapFilePath + "\" !");
					windowNetworkLobby->addInfoEntry ("Move it away or delete it, then reconnect.");
				}
				else
				{
					if (MapDownload::isMapOriginal (message.mapName, message.mapCrc) == false)
					{
						if (message.mapName != lastRequestedMapName)
						{
							lastRequestedMapName = message.mapName;
							sendNetMessage(cMuMsgRequestMap(message.mapName));
							windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_DownloadRequest"));
							windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_Download", message.mapName));
						}
					}
					else
					{
						windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_DownloadRequestInvalid"));
						windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_DownloadInvalid", message.mapName));
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
	
	windowNetworkLobby->setSaveGame (message.saveInfo);

}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_START_GAME_PREPARATIONS(cMuMsgStartGamePreparations& message)
{
	if (windowNetworkLobby->getSaveGameInfo().number != -1) return;

	windowLandingPositionSelection = nullptr;

	saveOptions();

	startGamePreparation(message);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_LANDING_STATE(cMuMsgLandingState& message)
{
	if (!windowLandingPositionSelection) return;

	windowLandingPositionSelection->applyReselectionState (message.state);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_START_GAME(cMuMsgStartGame& message)
{
	if (windowNetworkLobby->getSaveGameInfo().number != -1)
	{
		startSavedGame();
	}
	else
	{
		if (!newGame) return;

		startNewGame();
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_GAME_ALREADY_RUNNING(cNetMessageGameAlreadyRunning& message)
{
	if (!connectionManager || !windowNetworkLobby) return;

	auto yesNoDialog = application.show(std::make_shared<cDialogYesNo>(lngPack.i18n("Text~Multiplayer~Reconnect")));
	signalConnectionManager.connect(yesNoDialog->yesClicked, [this, message]()
	{
		reconnectToGame(message);
	});

	signalConnectionManager.connect(yesNoDialog->noClicked, [this]()
	{
		connectionManager->disconnectAll();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS(cMuMsgInLandingPositionSelectionStatus& message)
{
	if (!connectionManager || !windowNetworkLobby) return;

	if (message.isIn)
	{
		auto player = windowNetworkLobby->getPlayer(message.playerNr);
		if (player == nullptr) return;

		playersLandingStatus.push_back (std::make_unique<cPlayerLandingStatus> (*player));
		if (windowLandingPositionSelection) windowLandingPositionSelection->getChatBox()->addPlayerEntry (std::make_unique<cChatBoxLandingPlayerListViewItem> (*playersLandingStatus.back()));
	}
	else
	{
		if (windowLandingPositionSelection) windowLandingPositionSelection->getChatBox()->removePlayerEntry (message.playerNr);
		playersLandingStatus.erase (std::remove_if (playersLandingStatus.begin(), playersLandingStatus.end(), [message] (const std::unique_ptr<cPlayerLandingStatus>& status) { return status->getPlayer().getNr() == message.playerNr; }), playersLandingStatus.end());
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION(cMuMsgPlayerHasSelectedLandingPosition& message)
{
	if (!connectionManager || !windowNetworkLobby) return;

	auto iter = std::find_if (playersLandingStatus.begin(), playersLandingStatus.end(), [message] (const std::unique_ptr<cPlayerLandingStatus>& entry) { return entry->getPlayer().getNr() == message.landedPlayer; });

	if (iter == playersLandingStatus.end()) return;

	auto& playerLandingStatus = **iter;

	playerLandingStatus.setHasSelectedPosition (true);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::handleNetMessage_MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION(cMuMsgPlayerAbortedGamePreparations& message)
{
	const auto& player = windowNetworkLobby->getPlayer(message.playerNr);
	if (player == nullptr) return;

	auto yesNoDialog = application.show(std::make_shared<cDialogOk>("Player " + player->getName() + " has quit from game preparation")); // TODO: translate

	signalConnectionManager.connect(yesNoDialog->done, [this]()
	{
		application.closeTill(*windowNetworkLobby);
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::initMapDownload(cMuMsgStartMapDownload& message)
{
	mapReceiver = std::make_unique<cMapReceiver> (message.mapName, message.mapSize);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::receiveMapData(cMuMsgMapDownloadData& message)
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
void cMenuControllerMultiplayerClient::canceledMapDownload(cMuMsgCanceledMapDownload& message)
{
	if (mapReceiver == nullptr) return;

	mapReceiver = nullptr;

	if (windowNetworkLobby != nullptr)
	{
		windowNetworkLobby->setMapDownloadCanceled();
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::finishedMapDownload(cMuMsgFinishedMapDownload& message)
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
void cMenuControllerMultiplayerClient::sendNetMessage(cNetMessage2& message)
{
	if (windowNetworkLobby)
	{
		auto& localPlayer = windowNetworkLobby->getLocalPlayer();
		if (localPlayer)
		{
			message.playerNr = localPlayer->getNr();
		}
	}

	cTextArchiveIn archive;
	archive << message;
	Log.write("Menu: --> " + archive.data() + " to host", cLog::eLOG_TYPE_NET_DEBUG);

	connectionManager->sendToServer(message);
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerClient::sendNetMessage(cNetMessage2&& message)
{
	sendNetMessage(static_cast<cNetMessage2&>(message));
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
