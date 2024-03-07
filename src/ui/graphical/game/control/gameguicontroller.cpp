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

#include "ui/graphical/game/control/gameguicontroller.h"

#include "chatcommand/chatcommand.h"
#include "chatcommand/chatcommandarguments.h"
#include "chatcommand/chatcommandexecutor.h"
#include "chatcommand/chatcommandparser.h"
#include "crashreporter/debug.h"
#include "game/data/map/mapview.h"
#include "game/data/player/player.h"
#include "game/data/report/savedreportchat.h"
#include "game/data/report/savedreportsimple.h"
#include "game/data/report/savedreportunit.h"
#include "game/data/report/special/savedreporthostcommand.h"
#include "game/data/report/special/savedreportplayerdefeated.h"
#include "game/data/report/special/savedreportplayerendedturn.h"
#include "game/data/report/special/savedreportplayerwins.h"
#include "game/data/report/special/savedreportresourcechanged.h"
#include "game/data/report/special/savedreportturnstart.h"
#include "game/data/report/special/savedreportupgraded.h"
#include "game/data/report/unit/savedreportattacked.h"
#include "game/data/report/unit/savedreportcapturedbyenemy.h"
#include "game/data/report/unit/savedreportdestroyed.h"
#include "game/data/report/unit/savedreportdetected.h"
#include "game/data/report/unit/savedreportdisabled.h"
#include "game/data/report/unit/savedreportpathinterrupted.h"
#include "game/data/report/unit/savedreportsurveyoraiconfused.h"
#include "game/data/resourcetype.h"
#include "game/data/units/building.h"
#include "game/data/units/unit.h"
#include "game/data/units/vehicle.h"
#include "game/logic/attackjob.h"
#include "game/logic/client.h"
#include "game/logic/movejob.h"
#include "game/logic/server.h"
#include "game/logic/turncounter.h"
#include "game/logic/turntimeclock.h"
#include "input/keyboard/keyboard.h"
#include "output/sound/soundchannel.h"
#include "output/sound/sounddevice.h"
#include "resources/keys.h"
#include "ui/graphical/game/animations/animationtimer.h"
#include "ui/graphical/game/gamegui.h"
#include "ui/graphical/game/hud.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "ui/graphical/game/widgets/chatboxplayerlistviewitem.h"
#include "ui/graphical/game/widgets/debugoutputwidget.h"
#include "ui/graphical/game/widgets/gamemapwidget.h"
#include "ui/graphical/game/widgets/gamemessagelistview.h"
#include "ui/graphical/game/widgets/minimapwidget.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/dialogs/dialogpreferences.h"
#include "ui/graphical/menu/dialogs/dialogresearch.h"
#include "ui/graphical/menu/dialogs/dialogselfdestruction.h"
#include "ui/graphical/menu/dialogs/dialogtransfer.h"
#include "ui/graphical/menu/dialogs/dialogyesno.h"
#include "ui/graphical/menu/widgets/special/lobbychatboxlistviewitem.h"
#include "ui/graphical/menu/windows/windowbuildbuildings/windowbuildbuildings.h"
#include "ui/graphical/menu/windows/windowbuildvehicles/windowbuildvehicles.h"
#include "ui/graphical/menu/windows/windowendgame.h"
#include "ui/graphical/menu/windows/windowloadsave/windowloadsave.h"
#include "ui/graphical/menu/windows/windowreports/windowreports.h"
#include "ui/graphical/menu/windows/windowresourcedistribution/windowresourcedistribution.h"
#include "ui/graphical/menu/windows/windowstorage/windowstorage.h"
#include "ui/graphical/menu/windows/windowunitinfo/windowunitinfo.h"
#include "ui/graphical/menu/windows/windowupgrades/windowupgrades.h"
#include "ui/sound/effects/soundeffect.h"
#include "ui/sound/effects/soundeffectunit.h"
#include "ui/sound/effects/soundeffectvoice.h"
#include "ui/sound/game/savedreportssound.h"
#include "ui/sound/soundmanager.h"
#include "ui/translations.h"
#include "utility/language.h"
#include "utility/listhelpers.h"
#include "utility/log.h"
#include "utility/mathtools.h"
#include "utility/position.h"
#include "utility/random.h"

#include <cassert>
#include <sstream>

//------------------------------------------------------------------------------
cGameGuiController::cGameGuiController (cApplication& application_, std::shared_ptr<const cStaticMap> staticMap) :
	application (application_),
	soundManager (std::make_shared<cSoundManager>()),
	animationTimer (std::make_shared<cAnimationTimer>()),
	gameGui (std::make_shared<cGameGui> (std::move (staticMap), soundManager, animationTimer, application_.frameCounter)),
	upgradesFilterState (std::make_shared<cWindowUpgradesFilterState>())
{
	connectGuiStaticCommands();
	initShortcuts();
	initChatCommands();
	application.addRunnable (animationTimer);
}

//------------------------------------------------------------------------------
cGameGuiController::~cGameGuiController()
{
	application.removeRunnable (animationTimer);
}

//------------------------------------------------------------------------------
void cGameGuiController::start()
{
	application.show (gameGui);

	if (activeClient)
	{
		auto iter = playerGameGuiStates.find (activeClient->getActivePlayer().getId());
		if (iter != playerGameGuiStates.end())
		{
			gameGui->restoreState (iter->second.gameGuiState);
		}

		if (activeClient->getModel().getGameSettings()->gameType == eGameSettingsGameType::HotSeat)
		{
			showNextPlayerDialog();
		}
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::addPlayerGameGuiState (int playerNr, cGameGuiState playerGameGuiState)
{
	playerGameGuiStates[playerNr].gameGuiState = std::move (playerGameGuiState);
}

//------------------------------------------------------------------------------
void cGameGuiController::addSavedReport (std::unique_ptr<cSavedReport> savedReport, int playerNr)
{
	if (savedReport == nullptr) return;

	playerGameGuiStates[playerNr].reports->push_back (std::move (savedReport));

	if (activeClient->getActivePlayer().getId() == playerNr)
		handleReportForActivePlayer (*playerGameGuiStates[playerNr].reports->back());
}

//------------------------------------------------------------------------------
const std::vector<std::unique_ptr<cSavedReport>>& cGameGuiController::getSavedReports (int playerNr) const
{
	return *playerGameGuiStates.at (playerNr).reports;
}

//------------------------------------------------------------------------------
void cGameGuiController::setSingleClient (std::shared_ptr<cClient> client)
{
	std::vector<std::shared_ptr<cClient>> clients;
	int activePlayerNumber = 0;
	if (client != nullptr)
	{
		clients.push_back (client);
		activePlayerNumber = client->getActivePlayer().getId();
	}
	setClients (std::move (clients), activePlayerNumber);
}

//------------------------------------------------------------------------------
void cGameGuiController::setClients (std::vector<std::shared_ptr<cClient>> clients_, int activePlayerNumber)
{
	allClientsSignalConnectionManager.disconnectAll();

	clients = std::move (clients_);

	auto iter = ranges::find_if (clients, [=] (const std::shared_ptr<cClient>& client) { return client->getActivePlayer().getId() == activePlayerNumber; });
	if (iter != clients.end())
		setActiveClient (*iter);
	else
		setActiveClient (nullptr);

	for (auto clientPtr : clients)
	{
		auto client = clientPtr.get();

		connectReportSources (*client);

		allClientsSignalConnectionManager.connect (client->guiSaveInfoRequested, [this, client] (int slot, int savingId) {
			auto gameGuiState = (client == activeClient.get() ? std::make_optional (gameGui->getCurrentState()) : std::nullopt);
			client->sendGUISaveInfo (slot, savingId, playerGameGuiStates[client->getActivePlayer().getId()], gameGuiState);
		});

		allClientsSignalConnectionManager.connect (client->guiSaveInfoReceived, [this, client] (const cNetMessageGUISaveInfo& guiInfo) {
			if (guiInfo.playerNr != client->getActivePlayer().getId()) return;

			const cMap& map = *client->getModel().getMap();
			if (ranges::any_of (guiInfo.guiInfo.savedPositions, [&] (const auto& savedPosition) { return savedPosition && !map.isValidPosition (*savedPosition); }))
			{
				return;
			}
			const cPosition& mapPosition = guiInfo.guiInfo.gameGuiState.mapPosition;
			if (!map.isValidPosition (mapPosition)) return;

			playerGameGuiStates[client->getActivePlayer().getId()] = guiInfo.guiInfo;

			if (client == activeClient.get())
			{
				gameGui->restoreState (guiInfo.guiInfo.gameGuiState);
				for (const auto& report : *guiInfo.guiInfo.reports)
				{
					handleReportForActivePlayer (*report);
				}
			}
		});
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::setServer (cServer* server_)
{
	server = server_;
	gameGui->getDebugOutput().setServer (server);
}

//------------------------------------------------------------------------------
void cGameGuiController::setActiveClient (std::shared_ptr<cClient> client_)
{
	const bool wasSame = activeClient == client_;

	activeClient = std::move (client_);
	if (activeClient)
	{
		mapView = std::make_shared<const cMapView> (activeClient->getModel().getMap(), getActivePlayer());
	}
	else
	{
		mapView = nullptr;
	}

	gameGui->setMapView (mapView);
	gameGui->setPlayers (getPlayers());
	gameGui->setPlayer (getActivePlayer());
	gameGui->setTurnClock (getTurnCounter());
	gameGui->setTurnTimeClock (getTurnTimeClock());
	gameGui->setGameSettings (getGameSettings());
	gameGui->getDebugOutput().setClient (activeClient.get());
	gameGui->getDebugOutput().setServer (server);
	gameGui->setUnitsData (getUnitsData());

	if (activeClient != nullptr)
	{
		if (!wasSame) { connectClient (*activeClient); }
		auto iter = playerGameGuiStates.find (activeClient->getActivePlayer().getId());
		if (iter != playerGameGuiStates.end())
		{
			gameGui->restoreState (iter->second.gameGuiState);
		}
		soundManager->setModel (&activeClient->getModel());

		updateEndButtonState();
		updateGuiInfoTexts();
		updateChangeAllowed();
	}
	else
	{
		clientSignalConnectionManager.disconnectAll();
		soundManager->setModel (nullptr);
	}
}

//------------------------------------------------------------------------------
template <typename Action>
void cGameGuiController::addShortcut (cKeySequence key, Action action)
{
	auto shortcut = std::make_unique<cShortcut> (key);
	signalConnectionManager.connect (shortcut->triggered, action);
	gameGui->cWidget::addShortcut (std::move (shortcut));
}

//------------------------------------------------------------------------------
void cGameGuiController::initShortcuts()
{
	addShortcut (KeysList.keyExit, [this]() {
		auto yesNoDialog = application.show (std::make_shared<cDialogYesNo> (lngPack.i18n ("Comp~End_Game")));
		signalConnectionManager.connect (yesNoDialog->yesClicked, [this]() {
			gameGui->exit();
		});
	});

	addShortcut (KeysList.keyJumpToAction, [this]() {
		if (savedReportPosition)
		{
			gameGui->getGameMap().centerAt (*savedReportPosition);
		}
	});

	addShortcut (KeysList.keyUnitDoneAndNext, [this]() {
		markSelectedUnitAsDone();
		selectNextUnit();
	});

	addShortcut (KeysList.keyAllDoneAndNext, [this]() {
		resumeAllMoveJobsTriggered();
		selectNextUnit();
	});

	addShortcut (KeysList.keySavePosition1, [this]() { savePosition (0); });
	addShortcut (KeysList.keySavePosition2, [this]() { savePosition (1); });
	addShortcut (KeysList.keySavePosition3, [this]() { savePosition (2); });
	addShortcut (KeysList.keySavePosition4, [this]() { savePosition (3); });

	addShortcut (KeysList.keyPosition1, [this]() { jumpToSavedPosition (0); });
	addShortcut (KeysList.keyPosition2, [this]() { jumpToSavedPosition (1); });
	addShortcut (KeysList.keyPosition3, [this]() { jumpToSavedPosition (2); });
	addShortcut (KeysList.keyPosition4, [this]() { jumpToSavedPosition (3); });
}

//------------------------------------------------------------------------------
void cGameGuiController::initChatCommands()
{
	chatCommands.push_back (
		cChatCommand ("help", []() { return lngPack.i18n ("ChatCmd~Desc~Help"); })
			.setAction ([this]() {
				int maxPrefixLabelWidth = 0;
				std::vector<cLobbyChatBoxListViewItem*> chatBoxCommandEntries;
				auto firstItem = gameGui->getChatBox().addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> ("Available commands:"));
				for (const auto& commandExecutor : chatCommands)
				{
					const auto& command = commandExecutor->getCommand();

					if (command.getIsServerOnly() && (!activeClient || !server)) continue;

					std::stringstream commandName;
					commandName << command.getName();
					commandExecutor->printArguments (commandName);
					commandName << " ";
					auto entry = gameGui->getChatBox().addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (commandName.str(), command.getDescription(), false));
					maxPrefixLabelWidth = std::max (maxPrefixLabelWidth, entry->getPrefixLabelWidth());
					chatBoxCommandEntries.push_back (entry);
				}
				for (auto& entry : chatBoxCommandEntries)
				{
					entry->setDesiredPrefixLabelWidth (maxPrefixLabelWidth);
				}
				gameGui->getChatBox().scrollToItem (firstItem);
			}));
	gameGui->getDebugOutput().initChatCommand (chatCommands);
	chatCommands.push_back (
		cChatCommand ("cache size", []() { return lngPack.i18n ("ChatCmd~Desc~CacheSize"); })
			.addArgument<cChatCommandArgumentInt<unsigned int>> ("size")
			.setAction ([this] (unsigned int size) {
				gameGui->getGameMap().getDrawingCache().setMaxCacheSize (size);
			}));
	chatCommands.push_back (
		cChatCommand ("cache flush", []() { return lngPack.i18n ("ChatCmd~Desc~CacheFlush"); })
			.setAction ([this]() {
				gameGui->getGameMap().getDrawingCache().flush();
			}));
#if 0 // Not implemented
	chatCommands.push_back (
		cChatCommand ("kick", []() { return "Remove a player from the game"; })
		.addArgument<cChatCommandArgumentClientPlayer> (activeClient)
		.addArgument<cChatCommandArgumentClient> (activeClient)
		.setAction ([this](const cPlayer* player, cClient* client)
		{
			throw std::runtime_error ("Command not implemented");
			//sentWantKickPlayer (*client, *player);
		})
	);
	chatCommands.push_back (
		cChatCommand ("credits", []() { return "Set a given amount of credits to a player"; })
		.setShouldBeReported (true)
		.setIsServerOnly (true)
		.addArgument<cChatCommandArgumentServerPlayer> (server)
		.addArgument<cChatCommandArgumentInt<int>> ("credits")
		.addArgument<cChatCommandArgumentServer> (server)
		.setAction ([](const cPlayer* player, int credits, cServer* server)
		{
			throw std::runtime_error ("Command not implemented");
			//player->setCredits (credits);
			//sendCredits (*server, credits, *player);
		})
	);
	chatCommands.push_back (
		cChatCommand ("turnend", []() { return "Set a new turn end deadline. Use a value < 0 to disable the deadline entirely"; })
		.setShouldBeReported (true)
		.setIsServerOnly (true)
		.addArgument<cChatCommandArgumentInt<int>> ("seconds")
		.addArgument<cChatCommandArgumentServer> (server)
		.setAction ([](int seconds, cServer* server)
		{
			throw std::runtime_error ("Command not implemented");
			// FIXME: do not do changes on server data that are not synchronized with the server thread!
			/*if (seconds >= 0)
			{
				server->setTurnEndDeadline (std::chrono::seconds (seconds));
				server->setTurnEndDeadlineActive (true);
			}
			else
			{
				server->setTurnEndDeadlineActive (false);
			}
			Log.info ("Turn end deadline changed to " + std::to_string (seconds));
			*/
		})
	);
	chatCommands.push_back (
		cChatCommand ("turnlimit", []() { return "Set a new turn limit. Use a value <= 0 to disable the limit entirely"; })
		.setShouldBeReported (true)
		.setIsServerOnly (true)
		.addArgument<cChatCommandArgumentInt<int>> ("seconds")
		.addArgument<cChatCommandArgumentServer> (server)
		.setAction ([](int seconds, cServer* server)
		{
			throw std::runtime_error ("Command not implemented");
			// FIXME: do not do changes on server data that are not synchronized with the server thread!
			/*if (seconds > 0)
			{
				server->setTurnLimit (std::chrono::seconds (seconds));
				server->setTurnLimitActive (true);
			}
			else
			{
				server->setTurnLimitActive (false);
			}
			Log.info ("Turn limit changed to " + std::to_string (seconds));
			*/
		})
	);
	chatCommands.push_back (
		cChatCommand ("mark", []() { return "Add a mark to the log file"; })
		.addArgument<cChatCommandArgumentString> ("text", true)
		.addArgument<cChatCommandArgumentClient> (activeClient)
		.setAction ([](const std::string& text, cClient* client)
		{
			/* auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_MARK_LOG);
			message->pushString (text);
			client->sendNetMessage (std::move (message)); */
		})
	);
	chatCommands.push_back (
		cChatCommand ("color", []() { return "Change the color of the current player"; })
		.addArgument<cChatCommandArgumentInt<size_t>> ("colornum")
		.addArgument<cChatCommandArgumentClient> (activeClient)
		.setAction ([](size_t colorNum, cClient* client)
		{
			throw std::runtime_error ("Command not implemented");
			//colorNum %= cPlayerColor::predefinedColorsCount;
			//client->getActivePlayer().setColor (cPlayerColor (cPlayerColor::predefinedColors[colorNum]));
		})
	);
	chatCommands.push_back (
		cChatCommand ("survey", []() { return "Reveal all resources on the map"; })
		.setShouldBeReported (true)
		.setIsServerOnly (true)
		.addArgument<cChatCommandArgumentServer> (server)
		.addArgument<cChatCommandArgumentClient> (activeClient)
		.setAction ([](cServer* server, cClient* client)
		{
			throw std::runtime_error ("Command not implemented");
			//client->getMap()->assignResources (*server->Map);
			//client->getActivePlayer().revealResource();
		})
	);
#endif
	chatCommands.push_back (
		cChatCommand ("pause", []() { return lngPack.i18n ("ChatCmd~Desc~Pause"); })
			.setIsServerOnly (true)
			.addArgument<cChatCommandArgumentServer> (server)
			.setAction ([] (cServer* server) {
				// FIXME: do not do changes on server data that are not synchronized with the server thread!
				server->enableFreezeMode (eFreezeMode::Pause);
			}));
	chatCommands.push_back (
		cChatCommand ("resume", []() { return lngPack.i18n ("ChatCmd~Desc~Resume"); })
			.setIsServerOnly (true)
			.addArgument<cChatCommandArgumentServer> (server)
			.setAction ([] (cServer* server) {
				// FIXME: do not do changes on server data that are not synchronized with the server thread!
				server->disableFreezeMode (eFreezeMode::Pause);
			}));
#ifdef USE_CRASH_RPT
	chatCommands.push_back (
		cChatCommand ("crash", []() { return lngPack.i18n ("ChatCmd~Desc~Crash"); })
			.setAction ([]() {
				CR_EMULATE_CRASH();
			}));
#endif
#if 0
	chatCommands.push_back (
		cChatCommand ("disconnect", []() { return "Disconnect a player"; })
		.setIsServerOnly (true)
		.addArgument<cChatCommandArgumentServerPlayer> (server)
		.addArgument<cChatCommandArgumentServer> (server)
		.setAction ([](const cPlayer* player, cServer* server)
		{
			throw std::runtime_error ("Command not implemented");
			/*if (player->isLocal())
			{
				// TODO: translate
				throw std::runtime_error ("Can not disconnect this player");
			}

			auto message = std::make_unique<cNetMessage> (TCP_CLOSE);
			message->pushInt16 (player->getSocketNum());
			server->pushEvent (std::move (message));
			*/
		})
	);
	chatCommands.push_back (
		cChatCommand ("resync", []() { return "Resync a player"; })
		.addArgument<cChatCommandArgumentClientPlayer> (activeClient, true)
		.addArgument<cChatCommandArgumentClient> (activeClient)
		.addArgument<cChatCommandArgumentServer> (server, true)
		.setAction ([](const cPlayer* player, cClient* client, cServer* server)
		{
			if (!server)
			{
				//resync command on clients not yet implemented
				client->sendNetMessage (cNetMessageRequestResync (client->getActivePlayer().getId()));
			}
			if (player != nullptr)
			{
				client->sendNetMessage (cNetMessageRequestResync (player->getId()));
			}
			else
			{
				client->sendNetMessage (cNetMessageRequestResync());
			}
		})
	);
	chatCommands.push_back (
		cChatCommand ("fog off", []() { return "Reveal the whole map for a player"; })
		.setShouldBeReported (true)
		.setIsServerOnly (true)
		.addArgument<cChatCommandArgumentServerPlayer> (server, true)
		.addArgument<cChatCommandArgumentClient> (activeClient)
		.addArgument<cChatCommandArgumentServer> (server)
		.setAction ([](const cPlayer* player, cClient* client, cServer* server)
		{
			throw std::runtime_error ("Command not implemented");
			/*if (player == nullptr)
			{
				player = &server->getPlayerFromNumber (client->getActivePlayer().getNr());
			}
			// FIXME: do not do changes on server data that are not synchronized with the server thread!
			player->revealMap();
			sendRevealMap (*server, *player);
			*/
		})
	);
#endif
}

//------------------------------------------------------------------------------
void cGameGuiController::connectGuiStaticCommands()
{
	signalConnectionManager.connect (gameGui->terminated, [this]() { terminated(); });

	signalConnectionManager.connect (gameGui->getChatBox().commandEntered, [this] (const std::string& text) { handleChatCommand (text); });

	signalConnectionManager.connect (gameGui->getHud().preferencesClicked, [this]() { showPreferencesDialog(); });
	signalConnectionManager.connect (gameGui->getHud().filesClicked, [this]() { showFilesWindow(); });

	signalConnectionManager.connect (gameGui->getHud().centerClicked, [this]() { centerSelectedUnit(); });

	signalConnectionManager.connect (gameGui->getHud().nextClicked, [this]() { selectNextUnit(); });
	signalConnectionManager.connect (gameGui->getHud().prevClicked, [this]() { selectPreviousUnit(); });
	signalConnectionManager.connect (gameGui->getHud().doneClicked, [this]() {
		auto keyboard = application.getActiveKeyboard();
		if (keyboard && keyboard->isAnyModifierActive (toEnumFlag (eKeyModifierType::Ctrl)))
		{
			resumeAllMoveJobsTriggered();
		}
		else
		{
			markSelectedUnitAsDone();
		}
	});

	signalConnectionManager.connect (gameGui->getHud().reportsClicked, [this]() { showReportsWindow(); });

	signalConnectionManager.connect (gameGui->getGameMap().triggeredUnitHelp, [this] (const cUnit& unit) { showUnitHelpWindow (unit); });
	signalConnectionManager.connect (gameGui->getGameMap().triggeredTransfer, [this] (const cUnit& source, const cUnit& dest) { showUnitTransferDialog (source, dest); });
	signalConnectionManager.connect (gameGui->getGameMap().triggeredBuild, [this] (const cUnit& unit) {
		if (const auto* vehicle = dynamic_cast<const cVehicle*> (&unit))
		{
			showBuildBuildingsWindow (*vehicle);
		}
		else if (const auto* building = dynamic_cast<const cBuilding*> (&unit))
		{
			showBuildVehiclesWindow (*building);
		}
	});
	signalConnectionManager.connect (gameGui->getGameMap().triggeredResourceDistribution, [this] (const cUnit& unit) { showResourceDistributionDialog (unit); });
	signalConnectionManager.connect (gameGui->getGameMap().triggeredResearchMenu, [this] (const cUnit& unit) { showResearchDialog (unit); });
	signalConnectionManager.connect (gameGui->getGameMap().triggeredUpgradesMenu, [this] (const cUnit& unit) { showUpgradesWindow (unit); });
	signalConnectionManager.connect (gameGui->getGameMap().triggeredActivate, [this] (const cUnit& unit) { showStorageWindow (unit); });
	signalConnectionManager.connect (gameGui->getGameMap().triggeredSelfDestruction, [this] (const cBuilding& building) { showSelfDestroyDialog (building); });
}

//------------------------------------------------------------------------------
void cGameGuiController::connectClient (cClient& client)
{
	clientSignalConnectionManager.disconnectAll();

	//
	// GUI to client (action)
	//
	clientSignalConnectionManager.connect (transferTriggered, [&] (const cUnit& sourceUnit, const cUnit& destinationUnit, int transferValue, eResourceType resourceType) {
		client.transfer (sourceUnit, destinationUnit, transferValue, resourceType);
	});
	clientSignalConnectionManager.connect (buildBuildingTriggered, [&] (const cVehicle& vehicle, const cPosition& destination, const sID& unitId, int buildSpeed) {
		client.startBuild (vehicle, unitId, buildSpeed, destination);
	});
	clientSignalConnectionManager.connect (buildBuildingPathTriggered, [&] (const cVehicle& vehicle, const cPosition& destination, const sID& unitId, int buildSpeed) {
		client.startBuildPath (vehicle, unitId, buildSpeed, vehicle.getPosition(), destination);
	});
	clientSignalConnectionManager.connect (buildVehiclesTriggered, [&] (const cBuilding& building, const std::vector<sID>& buildList, int buildSpeed, bool repeat) {
		client.changeBuildList (building, buildList, buildSpeed, repeat);
	});
	clientSignalConnectionManager.connect (activateAtTriggered, [&] (const cUnit& unit, size_t index, const cPosition& position) {
		client.activateUnit (unit, *unit.storedUnits[index], position);
	});
	clientSignalConnectionManager.connect (reloadTriggered, [&] (const cUnit& sourceUnit, const cUnit& destinationUnit) {
		client.rearm (sourceUnit, destinationUnit);
	});
	clientSignalConnectionManager.connect (repairTriggered, [&] (const cUnit& sourceUnit, const cUnit& destinationUnit) {
		client.repair (sourceUnit, destinationUnit);
	});
	clientSignalConnectionManager.connect (upgradeTriggered, [&] (const cBuilding& building, size_t index) {
		client.upgradeVehicle (building, *building.storedUnits[index]);
	});
	clientSignalConnectionManager.connect (upgradeAllTriggered, [&] (const cBuilding& building) {
		client.upgradeAllVehicles (building);
	});
	clientSignalConnectionManager.connect (changeResourceDistributionTriggered, [&] (const cBuilding& building, const sMiningResource& production) {
		client.changeResourceDistribution (building, production);
	});
	clientSignalConnectionManager.connect (changeResearchSettingsTriggered, [&] (const std::array<int, cResearch::kNrResearchAreas>& newResearchSettings) {
		client.changeResearch (newResearchSettings);
	});
	clientSignalConnectionManager.connect (takeUnitUpgradesTriggered, [&] (const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades) {
		client.buyUpgrades (unitUpgrades);
	});
	clientSignalConnectionManager.connect (selfDestructionTriggered, [&] (const cBuilding& building) {
		client.selfDestroy (building);
	});
	clientSignalConnectionManager.connect (resumeMoveJobTriggered, [&] (const cVehicle& vehicle) {
		client.resumeMoveJob (vehicle);
	});
	clientSignalConnectionManager.connect (resumeAllMoveJobsTriggered, [&]() {
		client.resumeAllMoveJobs();
	});
	clientSignalConnectionManager.connect (gameGui->getHud().endClicked, [&]() { client.endTurn(); });
	clientSignalConnectionManager.connect (gameGui->getHud().triggeredRenameUnit, [&] (const cUnit& unit, const std::string& name) {
		client.changeUnitName (unit, name);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredStartWork, [&] (const cBuilding& building) {
		client.startWork (building);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredStopWork, [&] (const cUnit& unit) {
		client.stopWork (unit);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredAutoMoveJob, [&] (const cUnit& unit) {
		if (const auto* vehicle = dynamic_cast<const cVehicle*> (&unit))
		{
			if (!vehicle->isSurveyorAutoMoveActive())
			{
				client.addSurveyorMoveJob (*vehicle);
			}
			else
			{
				client.removeSurveyorMoveJob (*vehicle);
			}
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredStartClear, [&] (const cVehicle& vehicle) {
		activeClient->startClearRubbles (vehicle);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredManualFire, [&] (const cUnit& unit) {
		activeClient->changeManualFire (unit);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredSentry, [&] (const cUnit& unit) {
		activeClient->changeSentry (unit);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredUpgradeThis, [&] (const cBuilding& building) {
		client.upgradeBuilding (building);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredUpgradeAll, [&] (const cBuilding& building) {
		client.upgradeAllBuildings (building);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredLayMines, [&] (const cVehicle& vehicle) {
		client.toggleLayMines (vehicle);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredCollectMines, [&] (const cVehicle& vehicle) {
		client.toggleCollectMines (vehicle);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredUnitDone, [this] (const cUnit& unit) {
		if (const auto* vehicle = dynamic_cast<const cVehicle*> (&unit))
		{
			if (vehicle->getMoveJob() && !vehicle->isUnitMoving())
			{
				resumeMoveJobTriggered (*vehicle);
			}
		}
		playerGameGuiStates[activeClient->getActivePlayer().getId()].doneList.push_back (unit.getId());
	});

	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredEndBuilding, [&] (const cVehicle& vehicle, const cPosition& destination) {
		client.finishBuild (vehicle, destination);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredMoveSingle, [&] (const cVehicle& vehicle, const cPosition& destination, eStart start) {
		cPathCalculator pc (vehicle, *mapView, destination, false);
		const auto path = pc.calcPath();
		if (!path.empty())
		{
			client.startMove (vehicle, path, start, eStopOn::Never, cEndMoveAction::None());
		}
		else
		{
			soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredMoveGroup, [&] (const std::vector<cVehicle*>& vehicles, const cPosition& destination, eStart start) {
		sendStartGroupMoveAction (vehicles, destination, start);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredActivateAt, [&] (const cUnit& unit, size_t index, const cPosition& position) {
		client.activateUnit (unit, *unit.storedUnits[index], position);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredExitFinishedUnit, [&] (const cBuilding& building, const cPosition& position) {
		client.finishBuild (building, position);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredLoadAt, [&] (const cUnit& unit, const cPosition& position) {
		const auto& field = client.getModel().getMap()->getField (position);
		auto overVehicle = field.getVehicle();
		auto overPlane = field.getPlane();
		if (const auto& vehicle = dynamic_cast<const cVehicle*> (&unit))
		{
			if (vehicle->getStaticUnitData().factorAir > 0 && overVehicle)
			{
				if (overVehicle->getPosition() == vehicle->getPosition())
				{
					client.load (unit, *overVehicle);
				}
				else
				{
					cPathCalculator pc (*vehicle, *mapView, position, false);
					const auto path = pc.calcPath();
					if (!path.empty())
					{
						client.startMove (*vehicle, path, eStart::Immediate, eStopOn::Never, cEndMoveAction::Load (*overVehicle));
					}
					else
					{
						soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
					}
				}
			}
			else if (overVehicle)
			{
				if (vehicle->isNextTo (overVehicle->getPosition()))
				{
					client.load (unit, *overVehicle);
				}
				else
				{
					cPathCalculator pc (*overVehicle, *mapView, *vehicle, true);
					const auto path = pc.calcPath();
					if (!path.empty())
					{
						client.startMove (*overVehicle, path, eStart::Immediate, eStopOn::Never, cEndMoveAction::GetIn (unit));
					}
					else
					{
						soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
					}
				}
			}
		}
		else if (const auto* building = dynamic_cast<const cBuilding*> (&unit))
		{
			
			if (overVehicle && building->canLoad (overVehicle, false))
			{
				if (building->isNextTo (overVehicle->getPosition()))
				{
					client.load (unit, *overVehicle);
				}
				else
				{
					cPathCalculator pc (*overVehicle, *mapView, *building, true);
					const auto path = pc.calcPath();
					if (!path.empty())
					{
						client.startMove (*overVehicle, path, eStart::Immediate, eStopOn::Never, cEndMoveAction::GetIn (unit));
					}
					else
					{
						soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
					}
				}
			}
			else if (overPlane && building->canLoad (overPlane, false))
			{
				if (building->isNextTo (overPlane->getPosition()))
				{
					client.load (unit, *overPlane);
				}
				else
				{
					cPathCalculator pc (*overPlane, *mapView, *building, true);
					const auto path = pc.calcPath();
					if (!path.empty())
					{
						client.startMove (*overPlane, path, eStart::Immediate, eStopOn::Never, cEndMoveAction::GetIn (unit));
					}
					else
					{
						soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
					}
				}
			}
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredSupplyAmmo, [&] (const cUnit& sourceUnit, const cUnit& destinationUnit) {
		client.rearm (sourceUnit, destinationUnit);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredRepair, [&] (const cUnit& sourceUnit, const cUnit& destinationUnit) {
		client.repair (sourceUnit, destinationUnit);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredAttack, [&] (const cUnit& unit, const cPosition& position) {
		if (const auto* vehicle = dynamic_cast<const cVehicle*> (&unit))
		{
			cUnit* target = cAttackJob::selectTarget (position, vehicle->getStaticUnitData().canAttack, *mapView, vehicle->getOwner());

			if (vehicle->isInRange (position))
			{
				client.attack (*vehicle, position, target);
			}
			else if (target)
			{
				cPathCalculator pc (*vehicle, *mapView, position, true);
				const auto path = pc.calcPath();
				if (!path.empty())
				{
					client.startMove (*vehicle, path, eStart::Immediate, eStopOn::Never, cEndMoveAction::Attacking (*target));
				}
				else
				{
					soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
				}
			}
		}
		else if (const auto* building = dynamic_cast<const cBuilding*> (&unit))
		{
			cUnit* target = cAttackJob::selectTarget (position, building->getStaticUnitData().canAttack, *mapView, building->getOwner());

			client.attack (*building, position, target);
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredSteal, [&] (const cVehicle& infiltrator, const cUnit& destinationUnit) {
		client.steal (infiltrator, destinationUnit);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredDisable, [&] (const cVehicle& infiltrator, const cUnit& destinationUnit) {
		client.disable (infiltrator, destinationUnit);
	});
	clientSignalConnectionManager.connect (gameGui->getMiniMap().triggeredMove, [&] (const cPosition& destination) {
		const auto& unitSelection = gameGui->getGameMap().getUnitSelection();
		const auto& selectedVehicle = *unitSelection.getSelectedVehicle();
		const auto selectedVehiclesCount = unitSelection.getSelectedVehiclesCount();
		if (selectedVehiclesCount > 1)
		{
			sendStartGroupMoveAction (unitSelection.getSelectedVehicles(), destination, eStart::Immediate);
		}
		else if (selectedVehiclesCount == 1)
		{
			cPathCalculator pc (selectedVehicle, *mapView, destination, false);
			const auto path = pc.calcPath();
			if (!path.empty())
			{
				client.startMove (selectedVehicle, path, eStart::Immediate, eStopOn::Never, cEndMoveAction::None());
			}
			else
			{
				soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
			}
		}
	});

	//
	// client to GUI (reaction)
	//
	const cModel& model = client.getModel();

	clientSignalConnectionManager.connect (model.playerFinishedTurn, [&] (const cPlayer& player) {
		if (player.getId() != getActivePlayer()->getId())
		{
			return;
		}
		gameGui->getHud().lockEndButton();
	});

	clientSignalConnectionManager.connect (model.turnEnded, [&]() {
		if (getGameSettings()->gameType == eGameSettingsGameType::HotSeat)
		{
			auto& player = *model.getActiveTurnPlayer();
			if (player.getId() != getActivePlayer()->getId())
			{
				return;
			}
			playerGameGuiStates[player.getId()].gameGuiState = gameGui->getCurrentState();

			auto it = ranges::find_if (clients, [&] (const std::shared_ptr<cClient>& client) { return client->getActivePlayer().getId() == player.getId(); });
			assert (it != clients.end());
			++it;
			//TODO: skip defeated player? (maybe show defeat splashscreen once)
			if (it == clients.end())
			{
				it = clients.begin();
			}
			setActiveClient (*it);
			gameGui->getHud().unlockEndButton();
			showNextPlayerDialog();
		}
	});

	clientSignalConnectionManager.connect (client.connectionToServerLost, [this]() {
		gameGui->exit();
	});

	clientSignalConnectionManager.connect (client.freezeModeChanged, [this]() {
		updateEndButtonState();
		updateGuiInfoTexts();
		updateChangeAllowed();
	});

	clientSignalConnectionManager.connect (client.resynced, [this]() {
		if (activeClient->getModel().getActiveTurnPlayer() == getActivePlayer().get())
		{
			setActiveClient (activeClient);
		}
	});

	clientSignalConnectionManager.connect (model.newTurnStarted, [this] (const sNewTurnReport&) {
		if (activeClient->getModel().getActiveTurnPlayer() == getActivePlayer().get())
		{
			playerGameGuiStates[activeClient->getActivePlayer().getId()].doneList.clear();
		}

		updateEndButtonState();
		updateGuiInfoTexts();
		updateChangeAllowed();
	});

	clientSignalConnectionManager.connect (client.getModel().triggeredAddTracks, [&] (const cVehicle& vehicle) {
		if (!cSettings::getInstance().isMakeTracks()) return;
		if (!client.getActivePlayer().canSeeUnit (vehicle, *client.getModel().getMap())) return;

		auto& map = gameGui->getGameMap();

		cPosition vehiclePixelPos = vehicle.getPosition() * 64 + vehicle.getMovementOffset();
		if (client.getModel().getMap()->isWaterOrCoast (vehiclePixelPos / 64)) return;

		if (abs (vehicle.getMovementOffset().x()) == 64 || abs (vehicle.getMovementOffset().y()) == 64)
		{
			switch (vehicle.dir)
			{
				case 0:
					map.addEffect (std::make_shared<cFxTracks> (vehiclePixelPos + cPosition (0, -10), 0));
					break;
				case 4:
					map.addEffect (std::make_shared<cFxTracks> (vehiclePixelPos + cPosition (0, 10), 0));
					break;
				case 2:
					map.addEffect (std::make_shared<cFxTracks> (vehiclePixelPos + cPosition (10, 0), 2));
					break;
				case 6:
					map.addEffect (std::make_shared<cFxTracks> (vehiclePixelPos + cPosition (-10, 0), 2));
					break;
				case 1:
					map.addEffect (std::make_shared<cFxTracks> (vehiclePixelPos + cPosition (10, -10), 1));
					break;
				case 5:
					map.addEffect (std::make_shared<cFxTracks> (vehiclePixelPos + cPosition (-10, 10), 1));
					break;
				case 3:
					map.addEffect (std::make_shared<cFxTracks> (vehiclePixelPos + cPosition (10, 10), 3));
					break;
				case 7:
					map.addEffect (std::make_shared<cFxTracks> (vehiclePixelPos + cPosition (-10, -10), 3));
					break;
			}
		}
		else
		{
			switch (vehicle.dir)
			{
				case 1:
				case 5:
					map.addEffect (std::make_shared<cFxTracks> (vehiclePixelPos, 1));
					break;
				case 3:
				case 7:
					map.addEffect (std::make_shared<cFxTracks> (vehiclePixelPos, 3));
					break;
					;
			}
		}
	});

	clientSignalConnectionManager.connect (model.unitStored, [this] (const cUnit& storingUnit, const cUnit& /*storedUnit*/) {
		if (mapView->canSeeUnit (storingUnit))
		{
			soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectLoad, SoundData.SNDLoad, storingUnit));
		}
	});

	clientSignalConnectionManager.connect (model.unitActivated, [this] (const cUnit& storingUnit, const cUnit& /*storedUnit*/) {
		if (mapView->canSeeUnit (storingUnit))
		{
			soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectActivate, SoundData.SNDActivate, storingUnit));
		}
	});

	clientSignalConnectionManager.connect (model.unitStolen, [this] (const cUnit& source, const cUnit&, const cPlayer*) {
		if (source.getOwner() && source.getOwner()->getId() == getActivePlayer()->getId())
		{
			soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceCommandoAction, getRandom (VoiceData.VOIUnitStolen)));
		}
	});

	clientSignalConnectionManager.connect (model.unitDisabled, [this] (const cUnit& source, const cUnit&) {
		if (source.getOwner() && source.getOwner()->getId() == getActivePlayer()->getId())
		{
			soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceCommandoAction, VoiceData.VOIUnitDisabled));
		}
	});

	clientSignalConnectionManager.connect (model.unitStealDisableFailed, [this] (const cUnit& source, const cUnit&) {
		if (source.getOwner() && source.getOwner()->getId() == getActivePlayer()->getId())
		{
			soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceCommandoAction, getRandom (VoiceData.VOICommandoFailed)));
		}
	});

	clientSignalConnectionManager.connect (model.unitSuppliedWithAmmo, [this] (const cUnit& unit) {
		if (mapView->canSeeUnit (unit))
		{
			soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectReload, SoundData.SNDReload, unit));
		}
		if (unit.getOwner() && unit.getOwner()->getId() == getActivePlayer()->getId())
		{
			soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceReload, VoiceData.VOIReammo));
		}
	});

	clientSignalConnectionManager.connect (model.unitRepaired, [this] (const cUnit& unit) {
		if (mapView->canSeeUnit (unit))
		{
			soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectRepair, SoundData.SNDRepair, unit));
		}
		if (unit.getOwner() && unit.getOwner()->getId() == getActivePlayer()->getId())
		{
			soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceRepair, getRandom (VoiceData.VOIRepaired)));
		}
	});

	clientSignalConnectionManager.connect (client.getModel().addedEffect, [this] (const std::shared_ptr<cFx>& effect) {
		cPosition fxPos = effect->getPixelPosition();
		cPosition mapPos = cPosition (fxPos.x() / sGraphicTile::tilePixelWidth, fxPos.y() / sGraphicTile::tilePixelHeight);
		gameGui->getGameMap().addEffect (effect, getActivePlayer()->canSeeAt (mapPos));
	});

	clientSignalConnectionManager.connect (model.getTurnTimeClock()->alertTimeReached, [this]() {
		soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceTurnAlertTimeReached, getRandom (VoiceData.VOITurnEnd20Sec)));
	});

	clientSignalConnectionManager.connect (mapView->unitAppeared, [&] (const cUnit& unit) {
		if (getActivePlayer().get() == unit.getOwner())
		{
			if (unit.data.getId() == client.getModel().getUnitsData()->getSeaMineID())
			{
				soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectPlaceMine, SoundData.SNDSeaMinePlace, unit));
			}
			else if (unit.data.getId() == client.getModel().getUnitsData()->getLandMineID())
			{
				soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectPlaceMine, SoundData.SNDLandMinePlace, unit));
			}
		}
	});
	clientSignalConnectionManager.connect (mapView->unitDissappeared, [&] (const cUnit& unit) {
		if (getActivePlayer().get() != unit.getOwner()) return;

		if (unit.data.getId() == client.getModel().getUnitsData()->getLandMineID())
			soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectClearMine, SoundData.SNDLandMineClear, unit));
		else if (unit.data.getId() == client.getModel().getUnitsData()->getSeaMineID())
			soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectClearMine, SoundData.SNDSeaMineClear, unit));
	});
	clientSignalConnectionManager.connect (model.planeLanding, [this] (const cVehicle& plane) {
		if (mapView->canSeeUnit (plane))
		{
			soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectPlaneLand, SoundData.SNDPlaneLand, plane));
		}
	});
	clientSignalConnectionManager.connect (model.planeTakeoff, [this] (const cVehicle& plane) {
		if (mapView->canSeeUnit (plane))
		{
			soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectPlaneTakeoff, SoundData.SNDPlaneTakeoff, plane));
		}
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::connectReportSources (cClient& client)
{
	//this is the place where all reports about certain events in the model are generated...

	const cModel& model = client.getModel();
	const cPlayer& player = client.getActivePlayer();

	playerGameGuiStates[player.getId()].reports = std::make_shared<std::vector<std::unique_ptr<cSavedReport>>>();

	//report message received from server
	allClientsSignalConnectionManager.connect (client.reportMessageReceived, [this] (int, std::unique_ptr<cSavedReport>& report, int toPlayerNr) {
		addSavedReport (std::move (report), toPlayerNr);
	});

	clientSignalConnectionManager.connect (client.surveyorAiConfused, [&] (const cVehicle& vehicle) {
		if (vehicle.getOwner() != nullptr && vehicle.getOwner()->getId() != getActivePlayer()->getId())
		{
			addSavedReport (std::make_unique<cSavedReportSurveyorAiConfused> (vehicle), player.getId());
		}
	});
	clientSignalConnectionManager.connect (mapView->unitAppeared, [&] (const cUnit& unit) {
		if (unit.getOwner() == nullptr || unit.getOwner()->getId() != getActivePlayer()->getId())
		{
			if (const auto* building = dynamic_cast<const cBuilding*> (&unit); building && building->isRubble())
			{
				return;
			}
			addSavedReport (std::make_unique<cSavedReportDetected> (unit), player.getId());
		}
	});
	allClientsSignalConnectionManager.connect (model.playerFinishedTurn, [&] (const cPlayer& otherPlayer) {
		if (otherPlayer.getId() != getActivePlayer()->getId())
		{
			addSavedReport (std::make_unique<cSavedReportPlayerEndedTurn> (otherPlayer), player.getId());
		}
	});
	allClientsSignalConnectionManager.connect (model.unitDisabled, [&] (const cUnit&, const cUnit& target) {
		if (target.getOwner() && target.getOwner()->getId() == player.getId())
		{
			addSavedReport (std::make_unique<cSavedReportDisabled> (target), player.getId());
		}
	});
	allClientsSignalConnectionManager.connect (model.unitStolen, [&] (const cUnit&, const cUnit& target, const cPlayer* previousOwner) {
		if (previousOwner && previousOwner->getId() == player.getId())
		{
			addSavedReport (std::make_unique<cSavedReportCapturedByEnemy> (target), player.getId());
		}
	});
	allClientsSignalConnectionManager.connect (player.turnEndMovementsStarted, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::TurnAutoMove), player.getId());
	});
	clientSignalConnectionManager.connect (model.newTurnStarted, [&] (const sNewTurnReport& newTurnReport) {
		if (model.getActiveTurnPlayer() == getActivePlayer().get() || model.getGameSettings()->gameType == eGameSettingsGameType::Simultaneous)
		{
			const auto& report = newTurnReport.reports.at (player.getId());
			const auto& researchs = report.finishedResearchs;
			const auto& unitReport = report.unitsBuilt;
			gameGui->getGameMap().currentTurnResearchAreasFinished = researchs;
			addSavedReport (std::make_unique<cSavedReportTurnStart> (model.getTurnCounter()->getTurn(), unitReport, researchs), player.getId());
		}
	});

	clientSignalConnectionManager.connect (player.unitDestroyed, [&] (const cUnit& unit) {
		addSavedReport (std::make_unique<cSavedReportDestroyed> (unit), player.getId());
	});
	clientSignalConnectionManager.connect (player.unitAttacked, [&] (const cUnit& unit) {
		addSavedReport (std::make_unique<cSavedReportAttacked> (unit), player.getId());
	});
	clientSignalConnectionManager.connect (player.buildPathInterrupted, [&] (const cUnit& unit) {
		addSavedReport (std::make_unique<cSavedReportPathInterrupted> (unit), player.getId());
	});
	clientSignalConnectionManager.connect (player.buildErrorBuildPositionBlocked, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::Producing_PositionBlocked), player.getId());
	});
	clientSignalConnectionManager.connect (player.buildErrorInsufficientMaterial, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::Producing_InsufficientMaterial), player.getId());
	});
	clientSignalConnectionManager.connect (player.unitsUpgraded, [&] (const sID& unitId, int unitsCount, int costs) {
		addSavedReport (std::make_unique<cSavedReportUpgraded> (unitId, unitsCount, costs), player.getId());
	});

	//reports from the players base:
	allClientsSignalConnectionManager.connect (player.base.forcedResourceProductionChance, [&] (eResourceType resourceType, int amount, bool increase) {
		addSavedReport (std::make_unique<cSavedReportResourceChanged> (resourceType, amount, increase), player.getId());
	});
	allClientsSignalConnectionManager.connect (player.base.fuelInsufficient, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::FuelInsufficient), player.getId());
	});
	allClientsSignalConnectionManager.connect (player.base.metalLow, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::MetalLow), player.getId());
	});
	allClientsSignalConnectionManager.connect (player.base.goldLow, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::GoldLow), player.getId());
	});
	allClientsSignalConnectionManager.connect (player.base.teamLow, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::TeamLow), player.getId());
	});
	allClientsSignalConnectionManager.connect (player.base.energyLow, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::EnergyLow), player.getId());
	});
	allClientsSignalConnectionManager.connect (player.base.fuelLow, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::FuelLow), player.getId());
	});
	allClientsSignalConnectionManager.connect (player.base.teamInsufficient, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::TeamInsufficient), player.getId());
	});
	allClientsSignalConnectionManager.connect (player.base.goldInsufficient, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::GoldInsufficient), player.getId());
	});
	allClientsSignalConnectionManager.connect (player.base.metalInsufficient, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::MetalInsufficient), player.getId());
	});
	allClientsSignalConnectionManager.connect (player.base.energyInsufficient, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::EnergyInsufficient), player.getId());
	});
	allClientsSignalConnectionManager.connect (player.base.energyToLow, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::EnergyToLow), player.getId());
	});
	allClientsSignalConnectionManager.connect (player.base.energyIsNeeded, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::EnergyIsNeeded), player.getId());
	});
	allClientsSignalConnectionManager.connect (model.playerHasLost, [&] (const cPlayer& looser) {
		addSavedReport (std::make_unique<cSavedReportPlayerDefeated> (looser), player.getId());
		if (looser.getId() == player.getId())
		{
			auto win = application.show (std::make_shared<cWindowEndGame> (getPlayers()));
			allClientsSignalConnectionManager.connect (win->closed, [=]() { gameGui->exit(); });
			// TODO: stop game
		}
	});
	allClientsSignalConnectionManager.connect (model.playerHasWon, [&] (const cPlayer& winner) {
		addSavedReport (std::make_unique<cSavedReportPlayerWins> (winner), player.getId());
		if (winner.getId() == player.getId())
		{
			auto win = application.show (std::make_shared<cWindowEndGame> (getPlayers()));
			allClientsSignalConnectionManager.connect (win->closed, [=]() { gameGui->exit(); });
			// TODO: stop game
		}
	});
	allClientsSignalConnectionManager.connect (model.suddenDeathMode, [&]() {
		addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::SuddenDeath), player.getId());
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showNextPlayerDialog()
{
	soundManager->mute();
	auto dialog = application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Multiplayer~Player_Turn", activeClient->getActivePlayer().getName()), eWindowBackgrounds::Black));
	signalConnectionManager.connect (dialog->done, [this]() {
		soundManager->unmute();
		activeClient->startTurn();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showFilesWindow()
{
	auto loadSaveWindow = application.show (std::make_shared<cWindowLoadSave> (getTurnTimeClock()));
	loadSaveWindow->exit.connect ([this, loadSaveWindow]() {
		auto yesNoDialog = application.show (std::make_shared<cDialogYesNo> (lngPack.i18n ("Comp~End_Game")));
		signalConnectionManager.connect (yesNoDialog->yesClicked, [&, loadSaveWindow]() {
			loadSaveWindow->close();
			gameGui->exit();
		});
	});
	loadSaveWindow->load.connect ([this, loadSaveWindow] (const cSaveGameInfo&) {
		// loading games while game is running is not yet implemented
		application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Error_Messages~INFO_Not_Implemented")));
	});
	loadSaveWindow->save.connect ([this, loadSaveWindow] (int saveNumber, const std::string& name) {
		try
		{
			if (server == nullptr)
			{
				application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Multiplayer~Save_Only_Host")));
				return;
			}
			server->saveGameState (saveNumber, name);
			cSoundDevice::getInstance().playVoice (VoiceData.VOISaved);

			loadSaveWindow->update();
		}
		catch (std::runtime_error& e)
		{
			Log.error (e.what());
			application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Error_Messages~ERROR_Save_Writing")));
		}
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showPreferencesDialog()
{
	application.show (std::make_shared<cDialogPreferences>());
}

//------------------------------------------------------------------------------
void cGameGuiController::showReportsWindow()
{
	if (!activeClient) return;

	auto reportsWindow = application.show (std::make_shared<cWindowReports> (activeClient->getModel(), getActivePlayer(), getSavedReports (getActivePlayer()->getId())));

	signalConnectionManager.connect (reportsWindow->unitClickedSecondTime, [this, reportsWindow] (cUnit& unit) {
		gameGui->getGameMap().getUnitSelection().selectUnit (unit);
		gameGui->getGameMap().centerAt (unit.getPosition());
		reportsWindow->close();
	});

	signalConnectionManager.connect (reportsWindow->reportClickedSecondTime, [this, reportsWindow] (const cSavedReport& report) {
		if (auto position = report.getPosition())
		{
			gameGui->getGameMap().centerAt (*position);
			reportsWindow->close();
		}
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showUnitHelpWindow (const cUnit& unit)
{
	application.show (std::make_shared<cWindowUnitInfo> (unit.data, unit.getOwner(), *getUnitsData()));
}

//------------------------------------------------------------------------------
void cGameGuiController::showUnitTransferDialog (const cUnit& sourceUnit, const cUnit& destinationUnit)
{
	auto transferDialog = application.show (std::make_shared<cNewDialogTransfer> (sourceUnit, destinationUnit));
	transferDialog->done.connect ([&, transferDialog]() {
		transferTriggered (sourceUnit, destinationUnit, transferDialog->getTransferValue(), transferDialog->getResourceType());
		transferDialog->close();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showBuildBuildingsWindow (const cVehicle& vehicle)
{
	auto buildWindow = application.show (std::make_shared<cWindowBuildBuildings> (vehicle, getTurnTimeClock(), getUnitsData()));

	buildWindow->canceled.connect ([buildWindow]() { buildWindow->close(); });
	buildWindow->done.connect ([&, buildWindow]() {
		if (buildWindow->getSelectedUnitId())
		{
			const sID buildingId = *buildWindow->getSelectedUnitId();
			const auto& model = activeClient->getModel();
			const auto& buildingData = model.getUnitsData()->getStaticUnitData (buildingId);
			if (buildingData.buildingData.isBig)
			{
				if (!gameGui->getGameMap().startFindBuildPosition (buildingId))
				{
					addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::Producing_PositionBlocked), activeClient->getActivePlayer().getId());
					buildWindow->close();
					return;
				}
				auto buildSpeed = buildWindow->getSelectedBuildSpeed();

				buildPositionSelectionConnectionManager.disconnectAll();
				buildPositionSelectionConnectionManager.connect (gameGui->getGameMap().selectedBuildPosition, [this, buildingId, buildSpeed] (const cVehicle& selectedVehicle, const cPosition& destination) {
					buildBuildingTriggered (selectedVehicle, destination, buildingId, buildSpeed);
					buildPositionSelectionConnectionManager.disconnectAll();
				});
			}
			else
			{
				buildBuildingTriggered (vehicle, vehicle.getPosition(), *buildWindow->getSelectedUnitId(), buildWindow->getSelectedBuildSpeed());
			}
		}
		buildWindow->close();
	});
	buildWindow->donePath.connect ([&, buildWindow]() {
		if (buildWindow->getSelectedUnitId())
		{
			gameGui->getGameMap().startFindPathBuildPosition();
			auto buildType = *buildWindow->getSelectedUnitId();
			auto buildSpeed = buildWindow->getSelectedBuildSpeed();

			buildPositionSelectionConnectionManager.disconnectAll();
			buildPositionSelectionConnectionManager.connect (gameGui->getGameMap().selectedBuildPathDestination, [this, buildType, buildSpeed] (const cVehicle& selectedVehicle, const cPosition& destination) {
				buildBuildingPathTriggered (selectedVehicle, destination, buildType, buildSpeed);
				buildPositionSelectionConnectionManager.disconnectAll();
			});
		}
		buildWindow->close();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showBuildVehiclesWindow (const cBuilding& building)
{
	if (!mapView) return;

	auto buildWindow = application.show (std::make_shared<cWindowBuildVehicles> (building, *mapView, getUnitsData(), getTurnTimeClock()));

	buildWindow->canceled.connect ([buildWindow]() { buildWindow->close(); });
	buildWindow->done.connect ([&, buildWindow]() {
		buildVehiclesTriggered (building, buildWindow->getBuildList(), buildWindow->getSelectedBuildSpeed(), buildWindow->isRepeatActive());
		buildWindow->close();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showResourceDistributionDialog (const cUnit& unit)
{
	if (!unit.isABuilding()) return;

	const auto& building = static_cast<const cBuilding&> (unit);

	auto resourceDistributionWindow = application.show (std::make_shared<cWindowResourceDistribution> (building, getTurnTimeClock()));
	resourceDistributionWindow->done.connect ([&, resourceDistributionWindow]() {
		changeResourceDistributionTriggered (building, resourceDistributionWindow->getProduction());
		resourceDistributionWindow->close();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showResearchDialog (const cUnit& unit)
{
	const auto player = getActivePlayer();
	if (unit.getOwner() != player.get()) return;
	if (!player) return;

	gameGui->getGameMap().currentTurnResearchAreasFinished.clear();

	auto researchDialog = application.show (std::make_shared<cDialogResearch> (*player));
	researchDialog->done.connect ([&, researchDialog]() {
		changeResearchSettingsTriggered (researchDialog->getResearchSettings());
		researchDialog->close();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showUpgradesWindow (const cUnit& unit)
{
	const auto player = getActivePlayer();
	if (unit.getOwner() != player.get()) return;
	if (!player) return;

	auto upgradesWindow = application.show (std::make_shared<cWindowUpgrades> (*player, getTurnTimeClock(), upgradesFilterState, getUnitsData()));

	upgradesWindow->canceled.connect ([upgradesWindow]() { upgradesWindow->close(); });
	upgradesWindow->done.connect ([&, upgradesWindow]() {
		takeUnitUpgradesTriggered (upgradesWindow->getUnitUpgrades());
		upgradesWindow->close();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showStorageWindow (const cUnit& unit)
{
	auto storageWindow = application.show (std::make_shared<cWindowStorage> (unit, getTurnTimeClock()));
	storageWindow->activate.connect ([&, storageWindow] (size_t index) {
		if (unit.isAVehicle() && unit.getStaticUnitData().factorAir > 0)
		{
			activateAtTriggered (unit, index, unit.getPosition());
		}
		else
		{
			gameGui->getGameMap().startActivateVehicle (index);
		}
		storageWindow->close();
	});
	storageWindow->activateAll.connect ([&, storageWindow]() {
		if (mapView)
		{
			std::array<bool, 16> hasCheckedPlace;
			hasCheckedPlace.fill (false);

			for (size_t i = 0; i < unit.storedUnits.size(); ++i)
			{
				const auto& storedUnit = *unit.storedUnits[i];

				bool activated = false;
				for (int ypos = unit.getPosition().y() - 1, poscount = 0; ypos <= unit.getPosition().y() + (unit.getIsBig() ? 2 : 1); ypos++)
				{
					if (ypos < 0 || ypos >= mapView->getSize().y()) continue;
					for (int xpos = unit.getPosition().x() - 1; xpos <= unit.getPosition().x() + (unit.getIsBig() ? 2 : 1); xpos++, poscount++)
					{
						if (hasCheckedPlace[poscount]) continue;

						if (xpos < 0 || xpos >= mapView->getSize().x()) continue;

						if (((ypos == unit.getPosition().y() && unit.getStaticUnitData().factorAir == 0) || (ypos == unit.getPosition().y() + 1 && unit.getIsBig())) && ((xpos == unit.getPosition().x() && unit.getStaticUnitData().factorAir == 0) || (xpos == unit.getPosition().x() + 1 && unit.getIsBig()))) continue;

						if (unit.canExitTo (cPosition (xpos, ypos), *mapView, storedUnit.getStaticUnitData()))
						{
							activateAtTriggered (unit, i, cPosition (xpos, ypos));
							hasCheckedPlace[poscount] = true;
							activated = true;
							break;
						}
					}
					if (activated) break;
				}
			}
		}

		storageWindow->close();
	});
	if (const auto* building = dynamic_cast<const cBuilding*> (&unit))
	{
		storageWindow->reload.connect ([building, this] (size_t index) {
			reloadTriggered (*building, *building->storedUnits[index]);
		});
		storageWindow->repair.connect ([building, this] (size_t index) {
			repairTriggered (*building, *building->storedUnits[index]);
		});
		storageWindow->upgrade.connect ([building, this] (size_t index) {
			upgradeTriggered (*building, index);
		});

		storageWindow->reloadAll.connect ([building, this]() {
			auto remainingResources = building->subBase->getResourcesStored().metal;
			for (const auto* storedUnit : building->storedUnits)
			{
				if (remainingResources == 0) break;

				if (storedUnit->data.getAmmo() < storedUnit->data.getAmmoMax())
				{
					reloadTriggered (*building, *storedUnit);
					remainingResources--;
				}
			}
		});
		storageWindow->repairAll.connect ([building, this]() {
			auto remainingResources = building->subBase->getResourcesStored().metal;
			for (const auto& storedUnit : building->storedUnits)
			{
				if (remainingResources == 0) break;

				if (storedUnit->data.getHitpoints() < storedUnit->data.getHitpointsMax())
				{
					repairTriggered (*building, *storedUnit);
					//TODO: don't decide, which units can be repaired in the GUI code
					auto value = storedUnit->data.getHitpoints();
					while (value < storedUnit->data.getHitpointsMax() && remainingResources > 0)
					{
						value += 4 * storedUnit->data.getHitpointsMax() / storedUnit->data.getBuildCost();
						remainingResources--;
					}
				}
			}
		});
		storageWindow->upgradeAll.connect ([building, this]() {
			upgradeAllTriggered (*building);
		});
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::showSelfDestroyDialog (const cBuilding& building)
{
	if (building.getStaticData().canSelfDestroy)
	{
		auto selfDestroyDialog = application.show (std::make_shared<cDialogSelfDestruction> (building, animationTimer));
		signalConnectionManager.connect (selfDestroyDialog->triggeredDestruction, [this, selfDestroyDialog, &building]() {
			selfDestructionTriggered (building);
			selfDestroyDialog->close();
		});
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::handleChatCommand (const std::string& chatString)
{
	if (cChatCommand::isCommand (chatString))
	{
		try
		{
			for (const auto& commandExecutor : chatCommands)
			{
				if (commandExecutor->tryExecute (chatString))
				{
					if (commandExecutor->getCommand().getShouldBeReported() && server)
					{
						activeClient->report (std::make_unique<cSavedReportHostCommand> (chatString));
					}
					return;
				}
			}
			gameGui->getChatBox().addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (lngPack.i18n ("ChatCmd~Unknown_Command", chatString)));
		}
		catch (const std::runtime_error& e)
		{
			gameGui->getChatBox().addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (chatString));
			gameGui->getChatBox().addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (e.what()));
		}
	}
	else if (activeClient)
	{
		activeClient->report (std::make_unique<cSavedReportChat> (*getActivePlayer(), chatString));
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::handleReportForActivePlayer (const cSavedReport& report)
{
	if (report.getType() == eSavedReportType::Chat)
	{
		auto& savedChatReport = static_cast<const cSavedReportChat&> (report);
		auto playerEntry = gameGui->getChatBox().getPlayerEntryFromNumber (savedChatReport.getPlayerNumber());

		if (playerEntry)
		{
			gameGui->getChatBox().addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (playerEntry->getPlayer().getName(), savedChatReport.getText()));
			cSoundDevice::getInstance().playSoundEffect (SoundData.SNDChat);
		}
		else // message from non in-game player (e.g. dedicated server)
		{
			gameGui->getChatBox().addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (savedChatReport.getPlayerName(), savedChatReport.getText()));
		}
	}
	else if (auto position = report.getPosition())
	{
		savedReportPosition = *position;

		if (activeClient)
			gameGui->getGameMessageList().addMessage (getMessage (report, activeClient->getModel()) + " (" + KeysList.keyJumpToAction.toString() + ")", eGameMessageListViewItemBackgroundColor::LightGray);
	}
	else
	{
		if (activeClient)
			gameGui->getGameMessageList().addMessage (getMessage (report, activeClient->getModel()), report.isAlert() ? eGameMessageListViewItemBackgroundColor::Red : eGameMessageListViewItemBackgroundColor::DarkGray);
		if (report.isAlert()) soundManager->playSound (std::make_shared<cSoundEffect> (eSoundEffectType::EffectAlert, SoundData.SNDQuitsch));
	}
	if (activeClient)
		playSound (*soundManager, activeClient->getModel(), report);

	if (cSettings::getInstance().isDebug() && activeClient) Log.debug (getMessage (report, activeClient->getModel()));
}

//------------------------------------------------------------------------------
void cGameGuiController::selectNextUnit()
{
	const auto player = getActivePlayer();
	if (!player) return;

	auto& unitSelection = gameGui->getGameMap().getUnitSelection();
	unitSelection.selectNextUnit (*player, playerGameGuiStates[player->getId()].doneList);

	const cUnit* selectedUnit = unitSelection.getSelectedUnit();
	if (selectedUnit)
	{
		gameGui->getGameMap().centerAt (selectedUnit->getPosition());
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::selectPreviousUnit()
{
	const auto player = getActivePlayer();
	if (!player) return;

	auto& unitSelection = gameGui->getGameMap().getUnitSelection();
	unitSelection.selectPrevUnit (*player, playerGameGuiStates[player->getId()].doneList);

	const cUnit* selectedUnit = unitSelection.getSelectedUnit();
	if (selectedUnit)
	{
		gameGui->getGameMap().centerAt (selectedUnit->getPosition());
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::markSelectedUnitAsDone()
{
	const auto player = getActivePlayer();
	if (!player) return;

	const auto unit = gameGui->getGameMap().getUnitSelection().getSelectedUnit();

	if (unit && unit->getOwner() == player.get())
	{
		playerGameGuiStates[player->getId()].doneList.push_back (unit->getId());
		if (const cVehicle* vehicle = dynamic_cast<const cVehicle*> (unit))
		{
			if (vehicle->getMoveJob() && !vehicle->isUnitMoving())
			{
				resumeMoveJobTriggered (*vehicle);
			}
		}
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::centerSelectedUnit()
{
	const auto player = getActivePlayer();
	const auto selectedUnit = gameGui->getGameMap().getUnitSelection().getSelectedUnit();
	if (selectedUnit) gameGui->getGameMap().centerAt (selectedUnit->getPosition());
}

//------------------------------------------------------------------------------
void cGameGuiController::savePosition (size_t index)
{
	auto& guiInfo = playerGameGuiStates[activeClient->getActivePlayer().getId()];
	if (index > guiInfo.savedPositions.size()) return;

	guiInfo.savedPositions[index] = gameGui->getGameMap().getMapCenterOffset();
}

//------------------------------------------------------------------------------
void cGameGuiController::jumpToSavedPosition (size_t index)
{
	const auto& guiInfo = playerGameGuiStates[activeClient->getActivePlayer().getId()];
	if (index > guiInfo.savedPositions.size()) return;

	if (!guiInfo.savedPositions[index]) return;

	gameGui->getGameMap().centerAt (*guiInfo.savedPositions[index]);
}

//------------------------------------------------------------------------------
std::vector<std::shared_ptr<const cPlayer>> cGameGuiController::getPlayers() const
{
	std::vector<std::shared_ptr<const cPlayer>> result;

	if (!activeClient) return result;

	const auto& clientPlayerList = activeClient->getModel().getPlayerList();

	result.resize (clientPlayerList.size());
	std::copy (clientPlayerList.begin(), clientPlayerList.end(), result.begin());

	return result;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cPlayer> cGameGuiController::getActivePlayer() const
{
	if (!activeClient) return nullptr;

	const auto& clientPlayerList = activeClient->getModel().getPlayerList();

	auto iter = ranges::find_if (clientPlayerList, [this] (const std::shared_ptr<cPlayer>& player) { return player->getId() == activeClient->getActivePlayer().getId(); });

	if (iter == clientPlayerList.end()) return nullptr; // should never happen; just to be on the safe side

	return *iter;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cTurnCounter> cGameGuiController::getTurnCounter() const
{
	return activeClient ? activeClient->getModel().getTurnCounter() : nullptr;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cTurnTimeClock> cGameGuiController::getTurnTimeClock() const
{
	return activeClient ? activeClient->getModel().getTurnTimeClock() : nullptr;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cGameSettings> cGameGuiController::getGameSettings() const
{
	return activeClient ? activeClient->getModel().getGameSettings() : nullptr;
}

//-----------------------------------------------------------------------------
std::shared_ptr<const cUnitsData> cGameGuiController::getUnitsData() const
{
	return activeClient ? activeClient->getModel().getUnitsData() : nullptr;
}

//------------------------------------------------------------------------------
void cGameGuiController::sendStartGroupMoveAction (std::vector<cVehicle*> group, const cPosition& destination, eStart start)
{
	if (group.size() == 0) return;

	const cPosition moveVector = destination - group[0]->getPosition();

	// calc paths for all units
	std::vector<std::forward_list<cPosition>> paths;
	for (auto it = group.begin(); it != group.end();)
	{
		const cVehicle& vehicle = **it;
		cPosition vehicleDestination = vehicle.getPosition() + moveVector;
		cPathCalculator pc (vehicle, *mapView, vehicleDestination, &group);
		auto path = pc.calcPath();
		if (path.empty())
		{
			soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
			it = group.erase (it);
		}
		else
		{
			paths.push_back (std::move (path));
			++it;
		}
	}

	// start movement of units, beginning with those, whose next waypoint is free
	std::vector<const cVehicle*> startedMoves;
	bool moveStarted = true;
	while (moveStarted)
	{
		moveStarted = false;
		for (size_t i = 0; i < group.size(); i++)
		{
			auto& vehicle = group[i];
			auto& path = paths[i];
			if (mapView->possiblePlace (*vehicle, path.front(), true) || ranges::contains (startedMoves, mapView->getField (path.front()).getVehicle()))
			{
				activeClient->startMove (*vehicle, path, start, eStopOn::Never, cEndMoveAction::None());
				moveStarted = true;
				startedMoves.push_back (vehicle);
				vehicle = nullptr;
				path.clear();
			}
		}
		Remove (group, nullptr);
		RemoveEmpty (paths);
	}

	// start remaining movements. This is necessary, when there are group members, that have no path
	// to destination, or not enough movement points.
	for (size_t i = 0; i < group.size(); i++)
	{
		const auto& vehicle = group[i];
		const auto& path = paths[i];
		activeClient->startMove (*vehicle, path, start, eStopOn::Never, cEndMoveAction::None());
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::updateChangeAllowed()
{
	const cFreezeModes& freezeModes = activeClient->getFreezeModes();
	const auto& model = activeClient->getModel();

	bool changeAllowed = !freezeModes.isFreezed() && (activeClient->getModel().getActiveTurnPlayer() == getActivePlayer().get() || model.getGameSettings()->gameType == eGameSettingsGameType::Simultaneous);
	gameGui->getGameMap().setChangeAllowed (changeAllowed);
}

//------------------------------------------------------------------------------
void cGameGuiController::updateEndButtonState()
{
	const cFreezeModes& freezeModes = activeClient->getFreezeModes();
	const auto& model = activeClient->getModel();

	if (freezeModes.isEnabled (eFreezeMode::WaitForTurnend) || (activeClient->getModel().getActiveTurnPlayer() != getActivePlayer().get() && model.getGameSettings()->gameType == eGameSettingsGameType::Turns) || getActivePlayer()->getHasFinishedTurn())
	{
		gameGui->getHud().lockEndButton();
	}
	else
	{
		gameGui->getHud().unlockEndButton();
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::updateGuiInfoTexts()
{
	const cFreezeModes& freezeModes = activeClient->getFreezeModes();
	const auto& playerConnectionStates = activeClient->getPlayerConnectionStates();
	const auto& model = activeClient->getModel();

	// set overlay into message
	if (freezeModes.isEnabled (eFreezeMode::Pause))
	{
		gameGui->setInfoTexts (lngPack.i18n ("Multiplayer~Pause"), "");
	}
	else if (freezeModes.isEnabled (eFreezeMode::WaitForClient))
	{
		std::string disconncetedPlayers;
		std::string notRespondingPlayers;
		for (const auto& [playerId, state] : playerConnectionStates)
		{
			const cPlayer& player = *activeClient->getModel().getPlayer (playerId);
			if (state == ePlayerConnectionState::Disconnected)
			{
				if (!disconncetedPlayers.empty())
				{
					disconncetedPlayers += ", ";
				}
				disconncetedPlayers += player.getName();
			}
			if (state == ePlayerConnectionState::NotResponding)
			{
				if (!notRespondingPlayers.empty())
				{
					notRespondingPlayers += ", ";
				}
				notRespondingPlayers += player.getName();
			}
		}
		if (!disconncetedPlayers.empty())
		{
			std::string s = server ? lngPack.i18n ("Multiplayer~Abort_Waiting") : "";
			gameGui->setInfoTexts (lngPack.i18n ("Multiplayer~Wait_Reconnect", disconncetedPlayers), s);
		}
		else if (!notRespondingPlayers.empty())
		{
			gameGui->setInfoTexts (lngPack.i18n ("Multiplayer~No_Response", notRespondingPlayers), "");
		}
	}
	else if (freezeModes.isEnabled (eFreezeMode::WaitForTurnend))
	{
		gameGui->setInfoTexts (lngPack.i18n ("Multiplayer~Wait_TurnEnd"), "");
	}
	else if (freezeModes.isEnabled (eFreezeMode::WaitForServer))
	{
		gameGui->setInfoTexts (lngPack.i18n ("Multiplayer~Wait_For_Server"), "");
	}
	else if (activeClient->getModel().getActiveTurnPlayer() != getActivePlayer().get() && model.getGameSettings()->gameType == eGameSettingsGameType::Turns)
	{
		const std::string& name = activeClient->getModel().getActiveTurnPlayer()->getName();
		gameGui->setInfoTexts (lngPack.i18n ("Multiplayer~Wait_Until", name), "");
	}
	else
	{
		gameGui->setInfoTexts ("", "");
	}
}
