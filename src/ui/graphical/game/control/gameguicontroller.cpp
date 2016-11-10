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

#include <cassert>
#include <sstream>

#include "ui/graphical/game/control/gameguicontroller.h"
#include "ui/graphical/game/control/chatcommand/chatcommand.h"
#include "ui/graphical/game/control/chatcommand/chatcommandexecutor.h"
#include "ui/graphical/game/control/chatcommand/chatcommandparser.h"
#include "ui/graphical/game/control/chatcommand/chatcommandarguments.h"
#include "ui/graphical/game/gamegui.h"
#include "ui/graphical/game/hud.h"
#include "ui/graphical/game/widgets/gamemapwidget.h"
#include "ui/graphical/game/widgets/minimapwidget.h"
#include "ui/graphical/game/widgets/gamemessagelistview.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "ui/graphical/game/widgets/debugoutputwidget.h"
#include "ui/graphical/game/widgets/chatboxplayerlistviewitem.h"
#include "ui/graphical/menu/widgets/special/lobbychatboxlistviewitem.h"

#include "ui/graphical/game/animations/animationtimer.h"

#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/dialogs/dialogyesno.h"
#include "ui/graphical/menu/dialogs/dialogpreferences.h"
#include "ui/graphical/menu/dialogs/dialogtransfer.h"
#include "ui/graphical/menu/dialogs/dialogresearch.h"
#include "ui/graphical/menu/dialogs/dialogselfdestruction.h"
#include "ui/graphical/menu/windows/windowunitinfo/windowunitinfo.h"
#include "ui/graphical/menu/windows/windowbuildbuildings/windowbuildbuildings.h"
#include "ui/graphical/menu/windows/windowbuildvehicles/windowbuildvehicles.h"
#include "ui/graphical/menu/windows/windowstorage/windowstorage.h"
#include "ui/graphical/menu/windows/windowresourcedistribution/windowresourcedistribution.h"
#include "ui/graphical/menu/windows/windowupgrades/windowupgrades.h"
#include "ui/graphical/menu/windows/windowreports/windowreports.h"
#include "ui/graphical/menu/windows/windowloadsave/windowloadsave.h"
#include "ui/graphical/menu/windows/windowload/savegamedata.h"

#include "ui/sound/soundmanager.h"
#include "ui/sound/effects/soundeffect.h"
#include "ui/sound/effects/soundeffectvoice.h"
#include "ui/sound/effects/soundeffectunit.h"

#include "input/keyboard/keyboard.h"

#include "output/sound/sounddevice.h"
#include "output/sound/sounddevice.h"
#include "output/sound/soundchannel.h"

#include "utility/random.h"
#include "utility/position.h"

#include "keys.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/logic/clientevents.h"
#include "game/logic/turntimeclock.h"
#include "game/data/units/unit.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"
#include "utility/log.h"
#include "netmessage.h"
#include "network.h"
#include "game/logic/attackjob.h"
#include "game/data/player/player.h"
#include "game/data/report/savedreportsimple.h"
#include "game/data/report/savedreportchat.h"
#include "game/data/report/savedreportunit.h"
#include "game/data/report/special/savedreporthostcommand.h"

#include "debug.h"

//------------------------------------------------------------------------------
cGameGuiController::cGameGuiController (cApplication& application_, std::shared_ptr<const cStaticMap> staticMap) :
	application (application_),
	soundManager (std::make_shared<cSoundManager> ()),
	animationTimer (std::make_shared<cAnimationTimer> ()),
	gameGui (std::make_shared<cGameGui> (std::move (staticMap), soundManager, animationTimer, application_.frameCounter)),
	savedReportPosition (false, cPosition()),
	upgradesFilterState(std::make_shared<cWindowUpgradesFilterState>())
{
	connectGuiStaticCommands();
	initShortcuts();
	initChatCommands();
	application.addRunnable (animationTimer);

	for (size_t i = 0; i < savedPositions.size(); ++i)
	{
		savedPositions[i] = std::make_pair (false, cPosition());
	}
}

//------------------------------------------------------------------------------
cGameGuiController::~cGameGuiController()
{
	application.removeRunnable (*animationTimer);
}

//------------------------------------------------------------------------------
void cGameGuiController::start()
{
	application.show (gameGui);

	if (activeClient)
	{
		auto iter = playerGameGuiStates.find (activeClient->getActivePlayer().getNr());
		if (iter != playerGameGuiStates.end())
		{
			gameGui->restoreState (iter->second);
		}

		if (activeClient->getGameSettings()->getGameType() == eGameSettingsGameType::HotSeat)
		{
			showNextPlayerDialog();
		}
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::addPlayerGameGuiState (const cPlayer& player, cGameGuiState playerGameGuiState)
{
	playerGameGuiStates[player.getNr()] = std::move (playerGameGuiState);
}

//------------------------------------------------------------------------------
void cGameGuiController::setSingleClient (std::shared_ptr<cClient> client)
{
	std::vector<std::shared_ptr<cClient>> clients;
	int activePlayerNumber = 0;
	if (client != nullptr)
	{
		clients.push_back (client);
		activePlayerNumber = client->getActivePlayer().getNr();
	}
	setClients (std::move (clients), activePlayerNumber);
}

//------------------------------------------------------------------------------
void cGameGuiController::setClients (std::vector<std::shared_ptr<cClient>> clients_, int activePlayerNumber)
{
	allClientsSignalConnectionManager.disconnectAll();

	clients = std::move (clients_);

	auto iter = std::find_if (clients.begin(), clients.end(), [ = ] (const std::shared_ptr<cClient>& client) { return client->getActivePlayer().getNr() == activePlayerNumber; });
	if (iter != clients.end()) setActiveClient (*iter);
	else setActiveClient (nullptr);

	for (size_t i = 0; i < clients.size(); ++i)
	{
		auto client = clients[i].get();
		allClientsSignalConnectionManager.connect (clients[i]->additionalSaveInfoRequested, [this, client] (int saveingId)
		{
			if (client == activeClient.get())
			{
				sendGameGuiState (*client, gameGui->getCurrentState(), client->getActivePlayer(), saveingId);
			}
			else
			{
				sendGameGuiState (*client, playerGameGuiStates[client->getActivePlayer().getNr()], client->getActivePlayer(), saveingId);
			}
		});

		allClientsSignalConnectionManager.connect (clients[i]->gameGuiStateReceived, [this, client] (const cGameGuiState & state)
		{
			playerGameGuiStates[client->getActivePlayer().getNr()] = state;
			if (client == activeClient.get())
			{
				gameGui->restoreState (state);
			}
		});
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::setActiveClient (std::shared_ptr<cClient> client_)
{
	activeClient = std::move (client_);

	gameGui->setDynamicMap (getDynamicMap());
	gameGui->setPlayers (getPlayers());
	gameGui->setPlayer (getActivePlayer());
	gameGui->setTurnClock (getTurnClock());
	gameGui->setTurnTimeClock (getTurnTimeClock());
	gameGui->setGameSettings (getGameSettings());

	gameGui->getDebugOutput().setClient (activeClient.get());
	gameGui->getDebugOutput().setServer (activeClient->getServer());

	if (activeClient != nullptr)
	{
		connectClient (*activeClient);
		auto iter = playerGameGuiStates.find (activeClient->getActivePlayer().getNr());
		if (iter != playerGameGuiStates.end())
		{
			gameGui->restoreState (iter->second);
		}
	}
	else
	{
		clientSignalConnectionManager.disconnectAll();
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::initShortcuts()
{
	auto exitShortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keyExit));
	signalConnectionManager.connect (exitShortcut->triggered, [&]()
	{
		auto yesNoDialog = application.show (std::make_shared<cDialogYesNo> (lngPack.i18n ("Text~Comp~End_Game")));
		signalConnectionManager.connect (yesNoDialog->yesClicked, [&]()
		{
			gameGui->exit();
		});
	});

	auto jumpToActionShortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keyJumpToAction));
	signalConnectionManager.connect (jumpToActionShortcut->triggered, [&]()
	{
		if (savedReportPosition.first)
		{
			gameGui->getGameMap().centerAt (savedReportPosition.second);
		}
	});

	auto doneAndNextShortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitDoneAndNext));
	signalConnectionManager.connect (doneAndNextShortcut->triggered, [&]()
	{
		markSelectedUnitAsDone();
		selectNextUnit();
	});

	auto allDoneAndNextShortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keyAllDoneAndNext));
	signalConnectionManager.connect (allDoneAndNextShortcut->triggered, [&]()
	{
		resumeAllMoveJobsTriggered();
		selectNextUnit();
	});

	auto savePosition1Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keySavePosition1));
	signalConnectionManager.connect (savePosition1Shortcut->triggered, std::bind (&cGameGuiController::savePosition, this, 0));

	auto savePosition2Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keySavePosition2));
	signalConnectionManager.connect (savePosition2Shortcut->triggered, std::bind (&cGameGuiController::savePosition, this, 1));

	auto savePosition3Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keySavePosition3));
	signalConnectionManager.connect (savePosition3Shortcut->triggered, std::bind (&cGameGuiController::savePosition, this, 2));

	auto savePosition4Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keySavePosition4));
	signalConnectionManager.connect (savePosition4Shortcut->triggered, std::bind (&cGameGuiController::savePosition, this, 3));

	auto loadPosition1Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keyPosition1));
	signalConnectionManager.connect (loadPosition1Shortcut->triggered, std::bind (&cGameGuiController::jumpToSavedPosition, this, 0));

	auto loadPosition2Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keyPosition2));
	signalConnectionManager.connect (loadPosition2Shortcut->triggered, std::bind (&cGameGuiController::jumpToSavedPosition, this, 1));

	auto loadPosition3Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keyPosition3));
	signalConnectionManager.connect (loadPosition3Shortcut->triggered, std::bind (&cGameGuiController::jumpToSavedPosition, this, 2));

	auto loadPosition4Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keyPosition4));
	signalConnectionManager.connect (loadPosition4Shortcut->triggered, std::bind (&cGameGuiController::jumpToSavedPosition, this, 3));

}

//------------------------------------------------------------------------------
void cGameGuiController::initChatCommands()
{
	// TODO: translate descriptions?
	chatCommands.push_back(
		cChatCommand("help", "Show this help message")
		.setAction([&]()
		{
			int maxPrefixLabelWidth = 0;
			std::vector<cLobbyChatBoxListViewItem*> chatBoxCommandEntries;
			gameGui->getChatBox ().addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> ("Available commands:"));
			for (const auto& commandExecutor : chatCommands)
			{
				const auto& command = commandExecutor->getCommand ();
				if (command.getIsServerOnly () && (!activeClient || !activeClient->getServer ())) continue;

				std::stringstream commandName;
				commandName << command.getName ();
				commandExecutor->printArguments (commandName);
				commandName << " ";
				auto entry = gameGui->getChatBox ().addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (commandName.str (), command.getDescription (), false));
				maxPrefixLabelWidth = std::max(maxPrefixLabelWidth, entry->getPrefixLabelWidth ());
				chatBoxCommandEntries.push_back (entry);
			}
			for (auto& entry : chatBoxCommandEntries)
			{
				entry->setDesiredPrefixLabelWidth (maxPrefixLabelWidth);
			}
		})
	);
	chatCommands.push_back(
		cChatCommand("base", "Enable/disable debug information about bases")
		.addArgument<cChatCommandArgumentChoice>(std::vector<std::string>{"client", "server", "off"})
		.setAction([&](const std::string& value)
		{
			if(value == "server")
			{
				gameGui->getDebugOutput().setDebugBaseServer(true);
				gameGui->getDebugOutput().setDebugBaseClient(false);
			}
			else if(value == "client")
			{
				gameGui->getDebugOutput().setDebugBaseServer(false);
				gameGui->getDebugOutput().setDebugBaseClient(true);
			}
			else
			{
				gameGui->getDebugOutput().setDebugBaseServer(false);
				gameGui->getDebugOutput().setDebugBaseClient(false);
			}
		})
	);
	chatCommands.push_back(
		cChatCommand("sentry", "Enable/disable debug information about the sentry status")
		.addArgument<cChatCommandArgumentChoice>(std::vector<std::string>{"server", "off"})
		.setAction([&](const std::string& value)
		{
			gameGui->getDebugOutput().setDebugSentry(value == "server");
		})
	);
	chatCommands.push_back(
		cChatCommand("fx", "Enable/disable debug information about effects")
		.addArgument<cChatCommandArgumentBool>()
		.setAction([&](bool flag)
		{
			gameGui->getDebugOutput().setDebugFX(flag);
		})
	);
	chatCommands.push_back(
		cChatCommand("trace", "Enable/disable debug information about the unit currently under the cursor")
		.addArgument<cChatCommandArgumentChoice>(std::vector<std::string>{"client", "server", "off"})
		.setAction([&](const std::string& value)
		{
			if(value == "server")
			{
				gameGui->getDebugOutput().setDebugTraceServer(true);
				gameGui->getDebugOutput().setDebugTraceClient(false);
			}
			else if(value == "client")
			{
				gameGui->getDebugOutput().setDebugTraceServer(false);
				gameGui->getDebugOutput().setDebugTraceClient(true);
			}
			else
			{
				gameGui->getDebugOutput().setDebugTraceServer(false);
				gameGui->getDebugOutput().setDebugTraceClient(false);
			}
		})
	);
	chatCommands.push_back(
		cChatCommand("ajobs", "Enable/disable debug information about attack jobs")
		.addArgument<cChatCommandArgumentBool>()
		.setAction([&](bool flag)
		{
			gameGui->getDebugOutput().setDebugAjobs(flag);
		})
	);
	chatCommands.push_back(
		cChatCommand("players", "Enable/disable debug information about players")
		.addArgument<cChatCommandArgumentBool>()
		.setAction([&](bool flag)
		{
			gameGui->getDebugOutput().setDebugPlayers(flag);
		})
	);
	chatCommands.push_back(
		cChatCommand("singlestep", "Toggle single step") // TODO: description: what exactly is the effect of this?!
		.setAction([&]()
		{
			cGameTimer::syncDebugSingleStep = !cGameTimer::syncDebugSingleStep;
		})
	);
	chatCommands.push_back(
		cChatCommand("cache size", "Set the drawing cache size")
		.addArgument<cChatCommandArgumentInt<unsigned int>>("size")
		.setAction([&](unsigned int size)
		{
			gameGui->getGameMap().getDrawingCache().setMaxCacheSize(size);
		})
	);
	chatCommands.push_back(
		cChatCommand("cache flush", "Flush the drawing cache")
		.setAction([&]()
		{
			gameGui->getGameMap().getDrawingCache().flush();
		})
	);
	chatCommands.push_back(
		cChatCommand("cache debug", "Enable/disable debug information about the drawing cache")
		.addArgument<cChatCommandArgumentBool>()
		.setAction([&](bool flag)
		{
			gameGui->getDebugOutput().setDebugCache(flag);
		})
	);
	chatCommands.push_back(
		cChatCommand("sync debug", "Enable/disable debug information about the sync state of the game data")
		.addArgument<cChatCommandArgumentBool>()
		.setAction([&](bool flag)
		{
			gameGui->getDebugOutput().setDebugSync(flag);
		})
	);

	chatCommands.push_back(
		cChatCommand("kick", "Remove a player from the game")
		.addArgument<cChatCommandArgumentClientPlayer>(activeClient)
		.addArgument<cChatCommandArgumentClient>(activeClient)
		.setAction([&](cPlayer* player, cClient* client)
		{
			sentWantKickPlayer(*client, *player);
		})
	);
	chatCommands.push_back(
		cChatCommand("credits", "Set a given amount of credits to a player")
		.setShouldBeReported(true)
		.setIsServerOnly (true)
		.addArgument<cChatCommandArgumentServerPlayer>(activeClient)
		.addArgument<cChatCommandArgumentInt<int>>("credits")
		.addArgument<cChatCommandArgumentServer>(activeClient)
		.setAction([&](cPlayer* player, int credits, cServer* server)
		{
			player->setCredits(credits);
			sendCredits(*server, credits, *player);
		})
	);
	chatCommands.push_back(
		cChatCommand("turnend", "Set a new turn end deadline. Use a value < 0 to disable the deadline entirely")
		.setShouldBeReported (true)
		.setIsServerOnly (true)
		.addArgument<cChatCommandArgumentInt<int>>("seconds")
		.addArgument<cChatCommandArgumentServer>(activeClient)
		.setAction([&](int seconds, cServer* server)
		{
			// FIXME: do not do changes on server data that are not synchronized with the server thread!
			if(seconds >= 0)
			{
				server->setTurnEndDeadline(std::chrono::seconds(seconds));
				server->setTurnEndDeadlineActive(true);
			}
			else
			{
				server->setTurnEndDeadlineActive(false);
			}
			Log.write("Turn end deadline changed to " + std::to_string(seconds), cLog::eLOG_TYPE_INFO);
		})
	);
	chatCommands.push_back(
		cChatCommand("turnlimit", "Set a new turn limit. Use a value <= 0 to disable the limit entirely")
		.setShouldBeReported (true)
		.setIsServerOnly (true)
		.addArgument<cChatCommandArgumentInt<int>>("seconds")
		.addArgument<cChatCommandArgumentServer>(activeClient)
		.setAction([&](int seconds, cServer* server)
		{
			// FIXME: do not do changes on server data that are not synchronized with the server thread!
			if(seconds > 0)
			{
				server->setTurnLimit(std::chrono::seconds(seconds));
				server->setTurnLimitActive(true);
			}
			else
			{
				server->setTurnLimitActive(false);
			}
			Log.write("Turn limit changed to " + std::to_string(seconds), cLog::eLOG_TYPE_INFO);
		})
	);
	chatCommands.push_back(
		cChatCommand("mark", "Add a mark to the log file")
		.addArgument<cChatCommandArgumentString>("text", true)
		.addArgument<cChatCommandArgumentClient>(activeClient)
		.setAction([&](const std::string& text, cClient* client)
		{
			auto message = std::make_unique<cNetMessage>(GAME_EV_WANT_MARK_LOG);
			message->pushString(text);
			client->sendNetMessage(std::move(message));
		})
	);
	chatCommands.push_back(
		cChatCommand("color", "Change the color of the current player")
		.addArgument<cChatCommandArgumentInt<size_t>>("colornum")
		.addArgument<cChatCommandArgumentClient>(activeClient)
		.setAction([&](size_t colorNum, cClient* client)
		{
			colorNum %= cPlayerColor::predefinedColorsCount;
			client->getActivePlayer().setColor(cPlayerColor(cPlayerColor::predefinedColors[colorNum]));
		})
	);
	chatCommands.push_back(
		cChatCommand("survey", "Reveal all resources on the map")
		.setShouldBeReported (true)
		.setIsServerOnly (true)
		.addArgument<cChatCommandArgumentServer>(activeClient)
		.addArgument<cChatCommandArgumentClient>(activeClient)
		.setAction([&](cServer* server, cClient* client)
		{
			client->getMap()->assignRessources(*server->Map);
			client->getActivePlayer().revealResource();
		})
	);
	chatCommands.push_back(
		cChatCommand("pause", "Pause the game")
		.setIsServerOnly (true)
		.addArgument<cChatCommandArgumentServer>(activeClient)
		.setAction([&](cServer* server)
		{
			// FIXME: do not do changes on server data that are not synchronized with the server thread!
			server->enableFreezeMode(FREEZE_PAUSE);
		})
	);
	chatCommands.push_back(
		cChatCommand("resume", "Resume a paused game")
		.setIsServerOnly (true)
		.addArgument<cChatCommandArgumentServer>(activeClient)
		.setAction([&](cServer* server)
		{
			// FIXME: do not do changes on server data that are not synchronized with the server thread!
			server->disableFreezeMode(FREEZE_PAUSE);
		})
	);
	chatCommands.push_back(
		cChatCommand("crash", "Emulates a crash")
		.setAction([&]()
		{
			CR_EMULATE_CRASH();
		})
	);
	chatCommands.push_back(
		cChatCommand("disconnect", "Disconnect a player")
		.setIsServerOnly (true)
		.addArgument<cChatCommandArgumentServerPlayer>(activeClient)
		.addArgument<cChatCommandArgumentServer>(activeClient)
		.setAction([&](cPlayer* player, cServer* server)
		{
			if(player->isLocal())
			{
				// TODO: translate
				throw std::runtime_error("Can not disconnect this player");
			}

			auto message = std::make_unique<cNetMessage>(TCP_CLOSE);
			message->pushInt16(player->getSocketNum());
			server->pushEvent(std::move(message));
		})
	);
	chatCommands.push_back(
		cChatCommand("resync", "Resync a player")
		.addArgument<cChatCommandArgumentClientPlayer>(activeClient, true)
		.addArgument<cChatCommandArgumentClient>(activeClient)
		.addArgument<cChatCommandArgumentServer>(activeClient, true)
		.setAction([&](cPlayer* player, cClient* client, cServer* server)
		{
			if(player != nullptr)
			{
				sendRequestResync(*client, player->getNr(), false);
			}
			else
			{
				if(server)
				{
					const auto& playerList = server->playerList;
					for(unsigned int i = 0; i < playerList.size(); i++)
					{
						sendRequestResync(*client, playerList[i]->getNr(), false);
					}
				}
				else
				{
					sendRequestResync(*client, client->getActivePlayer().getNr(), false);
				}
			}
		})
	);
	chatCommands.push_back(
		cChatCommand("fog off", "Reveal the whole map for a player")
		.setShouldBeReported (true)
		.setIsServerOnly (true)
		.addArgument<cChatCommandArgumentServerPlayer>(activeClient, true)
		.addArgument<cChatCommandArgumentClient>(activeClient)
		.addArgument<cChatCommandArgumentServer>(activeClient)
		.setAction([&](cPlayer* player, cClient* client, cServer* server)
		{
			if(player == nullptr)
			{
				player = &server->getPlayerFromNumber(client->getActivePlayer().getNr());
			}
			// FIXME: do not do changes on server data that are not synchronized with the server thread!
			player->revealMap();
			sendRevealMap(*server, *player);
		})
	);
}

//------------------------------------------------------------------------------
void cGameGuiController::connectGuiStaticCommands()
{
	using namespace std::placeholders;

	signalConnectionManager.connect (gameGui->terminated, [this]() { terminated(); });

	signalConnectionManager.connect (gameGui->getChatBox().commandEntered, std::bind (&cGameGuiController::handleChatCommand, this, _1));

	signalConnectionManager.connect (gameGui->getHud().preferencesClicked, std::bind (&cGameGuiController::showPreferencesDialog, this));
	signalConnectionManager.connect (gameGui->getHud().filesClicked, std::bind (&cGameGuiController::showFilesWindow, this));

	signalConnectionManager.connect (gameGui->getHud().centerClicked, std::bind (&cGameGuiController::centerSelectedUnit, this));

	signalConnectionManager.connect (gameGui->getHud().nextClicked, std::bind (&cGameGuiController::selectNextUnit, this));
	signalConnectionManager.connect (gameGui->getHud().prevClicked, std::bind (&cGameGuiController::selectPreviousUnit, this));
	signalConnectionManager.connect (gameGui->getHud().doneClicked, [this]()
	{
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

	signalConnectionManager.connect (gameGui->getHud().reportsClicked, std::bind (&cGameGuiController::showReportsWindow, this));

	signalConnectionManager.connect (gameGui->getGameMap().triggeredUnitHelp, std::bind (&cGameGuiController::showUnitHelpWindow, this, _1));
	signalConnectionManager.connect (gameGui->getGameMap().triggeredTransfer, std::bind (&cGameGuiController::showUnitTransferDialog, this, _1, _2));
	signalConnectionManager.connect (gameGui->getGameMap().triggeredBuild, [&] (const cUnit & unit)
	{
		if (unit.isAVehicle())
		{
			showBuildBuildingsWindow (static_cast<const cVehicle&> (unit));
		}
		else if (unit.isABuilding())
		{
			showBuildVehiclesWindow (static_cast<const cBuilding&> (unit));
		}
	});
	signalConnectionManager.connect (gameGui->getGameMap().triggeredResourceDistribution, std::bind (&cGameGuiController::showResourceDistributionDialog, this, _1));
	signalConnectionManager.connect (gameGui->getGameMap().triggeredResearchMenu, std::bind (&cGameGuiController::showResearchDialog, this, _1));
	signalConnectionManager.connect (gameGui->getGameMap().triggeredUpgradesMenu, std::bind (&cGameGuiController::showUpgradesWindow, this, _1));
	signalConnectionManager.connect (gameGui->getGameMap().triggeredActivate, std::bind (&cGameGuiController::showStorageWindow, this, _1));
	signalConnectionManager.connect (gameGui->getGameMap().triggeredSelfDestruction, std::bind (&cGameGuiController::showSelfDestroyDialog, this, _1));
}

//------------------------------------------------------------------------------
void cGameGuiController::connectClient (cClient& client)
{
	clientSignalConnectionManager.disconnectAll();

	soundManager->setGameTimer (client.getGameTimer());

	//
	// GUI to client (action)
	//
	clientSignalConnectionManager.connect (transferTriggered, [&] (const cUnit & sourceUnit, const cUnit & destinationUnit, int transferValue, int resourceType)
	{
		if (transferValue != 0)
		{
			sendWantTransfer (client, sourceUnit.isAVehicle(), sourceUnit.iID, destinationUnit.isAVehicle(), destinationUnit.iID, transferValue, resourceType);
		}
	});
	clientSignalConnectionManager.connect (buildBuildingTriggered, [&] (const cVehicle & vehicle, const cPosition & destination, const sID & unitId, int buildSpeed)
	{
		sendWantBuild (client, vehicle.iID, unitId, buildSpeed, destination, false, cPosition (0, 0));
	});
	clientSignalConnectionManager.connect (buildBuildingPathTriggered, [&] (const cVehicle & vehicle, const cPosition & destination, const sID & unitId, int buildSpeed)
	{
		sendWantBuild (client, vehicle.iID, unitId, buildSpeed, vehicle.getPosition(), true, destination);
	});
	clientSignalConnectionManager.connect (buildVehiclesTriggered, [&] (const cBuilding & building, const std::vector<cBuildListItem>& buildList, int buildSpeed, bool repeat)
	{
		sendWantBuildList (client, building, buildList, repeat, buildSpeed);
	});
	clientSignalConnectionManager.connect (activateAtTriggered, [&] (const cUnit & unit, size_t index, const cPosition & position)
	{
		sendWantActivate (client, unit.iID, unit.isAVehicle(), unit.storedUnits[index]->iID, position);
	});
	clientSignalConnectionManager.connect (reloadTriggered, [&] (const cUnit & sourceUnit, const cUnit & destinationUnit)
	{
		sendWantSupply (client, destinationUnit.iID, destinationUnit.isAVehicle(), sourceUnit.iID, sourceUnit.isAVehicle(), SUPPLY_TYPE_REARM);
	});
	clientSignalConnectionManager.connect (repairTriggered, [&] (const cUnit & sourceUnit, const cUnit & destinationUnit)
	{
		sendWantSupply (client, destinationUnit.iID, destinationUnit.isAVehicle(), sourceUnit.iID, sourceUnit.isAVehicle(), SUPPLY_TYPE_REPAIR);
	});
	clientSignalConnectionManager.connect (upgradeTriggered, [&] (const cUnit & unit, size_t index)
	{
		sendWantUpgrade (client, unit.iID, index, false);
	});
	clientSignalConnectionManager.connect (upgradeAllTriggered, [&] (const cUnit & unit)
	{
		sendWantUpgrade (client, unit.iID, 0, true);
	});
	clientSignalConnectionManager.connect (changeResourceDistributionTriggered, [&] (const cBuilding & building, int metalProduction, int oilProduction, int goldProduction)
	{
		sendChangeResources (client, building, metalProduction, oilProduction, goldProduction);
	});
	clientSignalConnectionManager.connect (changeResearchSettingsTriggered, [&] (const std::array<int, cResearch::kNrResearchAreas>& newResearchSettings)
	{
		sendWantResearchChange (client, newResearchSettings);
	});
	clientSignalConnectionManager.connect (takeUnitUpgradesTriggered, [&] (const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades)
	{
		sendTakenUpgrades (client, unitUpgrades);
	});
	clientSignalConnectionManager.connect (selfDestructionTriggered, [&] (const cUnit & unit)
	{
		if (!unit.data.ID.isABuilding()) return;
		sendWantSelfDestroy (client, static_cast<const cBuilding&> (unit));
	});
	clientSignalConnectionManager.connect (resumeMoveJobTriggered, [&] (const cUnit & unit)
	{
		sendMoveJobResume (client, unit.iID);
	});
	clientSignalConnectionManager.connect (resumeAllMoveJobsTriggered, [&]()
	{
		sendMoveJobResume (client, 0);
	});
	clientSignalConnectionManager.connect (gameGui->getHud().endClicked, [&]()
	{
		client.handleEnd();
	});
	clientSignalConnectionManager.connect (gameGui->getHud().triggeredRenameUnit, [&] (const cUnit & unit, const std::string & name)
	{
		sendWantChangeUnitName (client, name, unit.iID);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredStartWork, [&] (const cUnit & unit)
	{
		sendWantStartWork (client, unit);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredStopWork, [&] (const cUnit & unit)
	{
		unit.executeStopCommand (client);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredAutoMoveJob, [&] (const cUnit & unit)
	{
		if (unit.data.ID.isAVehicle())
		{
			auto vehicle = client.getVehicleFromID (unit.iID);
			if (!vehicle) return;
			vehicle->executeAutoMoveJobCommand (client);
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredStartClear, [&] (const cUnit & unit)
	{
		if (unit.data.ID.isAVehicle()) sendWantStartClear (client, static_cast<const cVehicle&> (unit));
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredManualFire, [&] (const cUnit & unit)
	{
		sendChangeManualFireStatus (client, unit.iID, unit.isAVehicle());
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredSentry, [&] (const cUnit & unit)
	{
		sendChangeSentry (client, unit.iID, unit.isAVehicle());
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredUpgradeThis, [&] (const cUnit & unit)
	{
		if (unit.data.ID.isABuilding()) static_cast<const cBuilding&> (unit).executeUpdateBuildingCommmand (client, false);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredUpgradeAll, [&] (const cUnit & unit)
	{
		if (unit.data.ID.isABuilding()) static_cast<const cBuilding&> (unit).executeUpdateBuildingCommmand (client, true);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredLayMines, [&] (const cUnit & unit)
	{
		if (unit.data.ID.isAVehicle())
		{
			auto vehicle = client.getVehicleFromID (unit.iID);
			if (!vehicle) return;
			vehicle->executeLayMinesCommand (client);
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredCollectMines, [&] (const cUnit & unit)
	{
		if (unit.data.ID.isAVehicle())
		{
			auto vehicle = client.getVehicleFromID (unit.iID);
			if (!vehicle) return;
			vehicle->executeClearMinesCommand (client);
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredUnitDone, [&] (const cUnit & unit)
	{
		if (unit.data.ID.isAVehicle())
		{
			auto vehicle = client.getVehicleFromID (unit.iID);
			if (!vehicle) return;
			vehicle->setMarkedAsDone (true);
			sendMoveJobResume (client, vehicle->iID);
		}
		else if (unit.data.ID.isABuilding())
		{
			auto building = client.getBuildingFromID (unit.iID);
			if (!building) return;
			building->setMarkedAsDone (true);
		}
	});

	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredEndBuilding, [&] (cVehicle & vehicle, const cPosition & destination)
	{
		sendWantEndBuilding (client, vehicle, destination);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredMoveSingle, [&] (cVehicle & vehicle, const cPosition & destination)
	{
		const auto successfull = client.addMoveJob (vehicle, destination);
		if (!successfull)
		{
			soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredMoveGroup, [&] (const std::vector<cVehicle*>& vehicles, const cPosition & destination)
	{
		client.startGroupMove (vehicles, destination);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredActivateAt, [&] (const cUnit & unit, size_t index, const cPosition & position)
	{
		sendWantActivate (client, unit.iID, unit.isAVehicle(), unit.storedUnits[index]->iID, position);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredExitFinishedUnit, [&] (const cBuilding & building, const cPosition & position)
	{
		sendWantExitFinishedVehicle (client, building, position);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredLoadAt, [&] (const cUnit & unit, const cPosition & position)
	{
		const auto& field = client.getMap()->getField (position);
		auto overVehicle = field.getVehicle();
		auto overPlane = field.getPlane();
		if (unit.isAVehicle())
		{
			const auto& vehicle = static_cast<const cVehicle&> (unit);
			if (vehicle.data.factorAir > 0 && overVehicle)
			{
				if (overVehicle->getPosition() == vehicle.getPosition()) sendWantLoad (client, vehicle.iID, true, overVehicle->iID);
				else
				{
					cPathCalculator pc (vehicle.getPosition(), overVehicle->getPosition(), *client.getMap(), vehicle);
					sWaypoint* path = pc.calcPath();
					if (path)
					{
						sendMoveJob (client, path, vehicle.iID);
						sendEndMoveAction (client, vehicle.iID, overVehicle->iID, EMAT_LOAD);
					}
					else
					{
						soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
					}
				}
			}
			else if (overVehicle)
			{
				if (vehicle.isNextTo (overVehicle->getPosition())) sendWantLoad (client, vehicle.iID, true, overVehicle->iID);
				else
				{
					cPathCalculator pc (overVehicle->getPosition(), vehicle, *client.getMap(), *overVehicle, true);
					sWaypoint* path = pc.calcPath();
					if (path)
					{
						sendMoveJob (client, path, overVehicle->iID);
						sendEndMoveAction (client, overVehicle->iID, vehicle.iID, EMAT_GET_IN);
					}
					else
					{
						soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
					}
				}
			}
		}
		else if (unit.isABuilding())
		{
			const auto& building = static_cast<const cBuilding&> (unit);
			if (overVehicle && building.canLoad (overVehicle, false))
			{
				if (building.isNextTo (overVehicle->getPosition())) sendWantLoad (client, building.iID, false, overVehicle->iID);
				else
				{
					cPathCalculator pc (overVehicle->getPosition(), building, *client.getMap(), *overVehicle, true);
					sWaypoint* path = pc.calcPath();
					if (path)
					{
						sendMoveJob (client, path, overVehicle->iID);
						sendEndMoveAction (client, overVehicle->iID, building.iID, EMAT_GET_IN);
					}
					else
					{
						soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
					}
				}
			}
			else if (overPlane && building.canLoad (overPlane, false))
			{
				if (building.isNextTo (overPlane->getPosition())) sendWantLoad (client, building.iID, false, overPlane->iID);
				else
				{
					cPathCalculator pc (overPlane->getPosition(), building, *client.getMap(), *overPlane, true);
					sWaypoint* path = pc.calcPath();
					if (path)
					{
						sendMoveJob (client, path, overPlane->iID);
						sendEndMoveAction (client, overPlane->iID, building.iID, EMAT_GET_IN);
					}
					else
					{
						soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
					}
				}
			}
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredSupplyAmmo, [&] (const cUnit & sourceUnit, const cUnit & destinationUnit)
	{
		sendWantSupply (client, destinationUnit.iID, destinationUnit.isAVehicle(), sourceUnit.iID, sourceUnit.isAVehicle(), SUPPLY_TYPE_REARM);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredRepair, [&] (const cUnit & sourceUnit, const cUnit & destinationUnit)
	{
		sendWantSupply (client, destinationUnit.iID, destinationUnit.isAVehicle(), sourceUnit.iID, sourceUnit.isAVehicle(), SUPPLY_TYPE_REPAIR);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredAttack, [&] (const cUnit & unit, const cPosition & position)
	{
		if (unit.isAVehicle())
		{
			const auto& vehicle = static_cast<const cVehicle&> (unit);

			cUnit* target = cAttackJob::selectTarget (position, vehicle.data.canAttack, *client.getMap(), vehicle.getOwner());

			if (vehicle.isInRange (position))
			{
				// find target ID
				int targetId = 0;
				if (target) targetId = target->iID;

				Log.write (" Client: want to attack " + iToStr (position.x()) + ":" + iToStr (position.y()) + ", Vehicle ID: " + iToStr (targetId), cLog::eLOG_TYPE_NET_DEBUG);
				sendWantAttack (client, vehicle.iID, position, targetId);
			}
			else if (target)
			{
				cPathCalculator pc (vehicle.getPosition(), *client.getMap(), vehicle, position);
				sWaypoint* path = pc.calcPath();
				if (path)
				{
					sendMoveJob (client, path, vehicle.iID);
					sendEndMoveAction (client, vehicle.iID, target->iID, EMAT_ATTACK);
				}
				else
				{
					soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
				}
			}
		}
		else if (unit.isABuilding())
		{
			const auto& building = static_cast<const cBuilding&> (unit);
			const cMap& map = *client.getMap();

			int targetId = 0;
			cUnit* target = cAttackJob::selectTarget (position, building.data.canAttack, map, building.getOwner());

			if (target) targetId = target->iID;

			sendWantAttack (client, building.iID, position, targetId);
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredSteal, [&] (const cUnit & sourceUnit, const cUnit & destinationUnit)
	{
		sendWantComAction (client, sourceUnit.iID, destinationUnit.iID, destinationUnit.isAVehicle(), true);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap().triggeredDisable, [&] (const cUnit & sourceUnit, const cUnit & destinationUnit)
	{
		sendWantComAction (client, sourceUnit.iID, destinationUnit.iID, destinationUnit.isAVehicle(), false);
	});
	clientSignalConnectionManager.connect (gameGui->getMiniMap().triggeredMove, [&] (const cPosition & destination)
	{
		const auto& unitSelection = gameGui->getGameMap().getUnitSelection();
		const auto selectedVehiclesCount = unitSelection.getSelectedVehiclesCount();
		if (selectedVehiclesCount > 1)
		{
			client.startGroupMove (unitSelection.getSelectedVehicles(), destination);
		}
		else if (selectedVehiclesCount == 1)
		{
			const auto successfull = client.addMoveJob (*unitSelection.getSelectedVehicle(), destination);
			if (!successfull)
			{
				soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
			}
		}
	});


	//
	// client to GUI (reaction)
	//
	clientSignalConnectionManager.connect (client.playerFinishedTurn, [&] (int currentPlayerNumber, int nextPlayerNumber)
	{
		if (currentPlayerNumber != client.getActivePlayer().getNr()) return;

		if (client.getGameSettings()->getGameType() == eGameSettingsGameType::HotSeat)
		{
			gameGui->getHud().unlockEndButton();

			auto iter = std::find_if (clients.begin(), clients.end(), [ = ] (const std::shared_ptr<cClient>& client) { return client->getActivePlayer().getNr() == nextPlayerNumber; });
			if (iter != clients.end())
			{
				playerGameGuiStates[currentPlayerNumber] = gameGui->getCurrentState();

				setActiveClient (*iter);

				showNextPlayerDialog();
			}
			else setActiveClient (nullptr);
		}
		else
		{
			gameGui->getHud().lockEndButton();
		}
	});

	clientSignalConnectionManager.connect (client.freezeModeChanged, [&] (eFreezeMode mode)
	{
		const int playerNumber = client.getFreezeInfoPlayerNumber();
		const cPlayer* player = client.getPlayerFromNumber (playerNumber);

		if (mode == FREEZE_WAIT_FOR_OTHERS || mode == FREEZE_WAIT_FOR_TURNEND)
		{
			if (client.getFreezeMode (FREEZE_WAIT_FOR_OTHERS) || client.getFreezeMode (FREEZE_WAIT_FOR_TURNEND)) gameGui->getHud().lockEndButton();
			else gameGui->getHud().unlockEndButton();
		}

		if (client.getFreezeMode (FREEZE_WAIT_FOR_OTHERS))
		{
			// TODO: Fix message
			const std::string& name = player ? player->getName() : "other players";
			gameGui->setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_Until", name), "");
		}
		else if (client.getFreezeMode (FREEZE_PAUSE))
		{
			gameGui->setInfoTexts (lngPack.i18n ("Text~Multiplayer~Pause"), "");
		}
		else if (client.getFreezeMode (FREEZE_WAIT_FOR_SERVER))
		{
			gameGui->setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_For_Server"), "");
		}
		else if (client.getFreezeMode (FREEZE_WAIT_FOR_RECONNECT))
		{
			std::string s = client.getServer() ? lngPack.i18n ("Text~Multiplayer~Abort_Waiting") : "";
			gameGui->setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_Reconnect"), s);
		}
		else if (client.getFreezeMode (FREEZE_WAIT_FOR_PLAYER))
		{
			gameGui->setInfoTexts (lngPack.i18n ("Text~Multiplayer~No_Response", player->getName()), "");
		}
		else if (client.getFreezeMode (FREEZE_WAIT_FOR_TURNEND))
		{
			gameGui->setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_TurnEnd"), "");
		}
		else
		{
			gameGui->setInfoTexts ("", "");
		}

		gameGui->getGameMap().setChangeAllowed (!client.isFreezed());
	});

	clientSignalConnectionManager.connect (client.unitStored, [&] (const cUnit & storingUnit, const cUnit& /*storedUnit*/)
	{
		soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectLoad, SoundData.SNDLoad, storingUnit));
	});

	clientSignalConnectionManager.connect (client.unitActivated, [&] (const cUnit & storingUnit, const cUnit& /*storedUnit*/)
	{
		soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectActivate, SoundData.SNDActivate, storingUnit));
	});

	clientSignalConnectionManager.connect (client.unitHasStolenSuccessfully, [&] (const cUnit&)
	{
		soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceCommandoAction, getRandom (VoiceData.VOIUnitStolen)));
	});

	clientSignalConnectionManager.connect (client.unitHasDisabledSuccessfully, [&] (const cUnit&)
	{
		soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceCommandoAction, VoiceData.VOIUnitDisabled));
	});

	clientSignalConnectionManager.connect (client.unitStealDisableFailed, [&] (const cUnit&)
	{
		soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceCommandoAction, getRandom (VoiceData.VOICommandoFailed)));
	});

	clientSignalConnectionManager.connect (client.unitSuppliedWithAmmo, [&] (const cUnit & unit)
	{
		soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectReload, SoundData.SNDReload, unit));
		soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceReload, VoiceData.VOIReammo));
	});

	clientSignalConnectionManager.connect (client.unitRepaired, [&] (const cUnit & unit)
	{
		soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectRepair, SoundData.SNDRepair, unit));
		soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceRepair, getRandom (VoiceData.VOIRepaired)));
	});

	clientSignalConnectionManager.connect (client.addedEffect, [&] (const std::shared_ptr<cFx>& effect, bool playSound)
	{
		gameGui->getGameMap().addEffect (effect, playSound);
	});

	clientSignalConnectionManager.connect (client.getTurnTimeClock()->alertTimeReached, [this]()
	{
		soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceTurnAlertTimeReached, getRandom (VoiceData.VOITurnEnd20Sec)));
	});

	//
	// client player to GUI
	//
	auto& player = client.getActivePlayer();

	using namespace std::placeholders;
	clientSignalConnectionManager.connect (player.reportAdded, std::bind (&cGameGuiController::handleReport, this, _1));
}

//------------------------------------------------------------------------------
void cGameGuiController::showNextPlayerDialog()
{
	soundManager->mute();
	auto dialog = application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Text~Multiplayer~Player_Turn", activeClient->getActivePlayer().getName()), eWindowBackgrounds::Black));
	signalConnectionManager.connect (dialog->done, [this]()
	{
		soundManager->unmute();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showFilesWindow()
{
	auto loadSaveWindow = application.show (std::make_shared<cWindowLoadSave> (getTurnTimeClock()));
	loadSaveWindow->exit.connect ([this, loadSaveWindow]()
	{
		auto yesNoDialog = application.show (std::make_shared<cDialogYesNo> (lngPack.i18n ("Text~Comp~End_Game")));
		signalConnectionManager.connect (yesNoDialog->yesClicked, [&, loadSaveWindow]()
		{
			loadSaveWindow->close();
			gameGui->exit();
		});
	});
	loadSaveWindow->load.connect ([this, loadSaveWindow] (int saveNumber)
	{
		// loading games while game is running is not yet implemented
		application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Text~Error_Messages~INFO_Not_Implemented")));
	});
	loadSaveWindow->save.connect ([this, loadSaveWindow] (int saveNumber, const std::string & name)
	{
		try
		{
			triggeredSave (saveNumber, name);

			cSoundDevice::getInstance().playVoice (VoiceData.VOISaved);

			loadSaveWindow->update();
		}
		catch (std::runtime_error& e)
		{
			application.show (std::make_shared<cDialogOk> (e.what()));
		}
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showPreferencesDialog()
{
	application.show (std::make_shared<cDialogPreferences> ());
}

//------------------------------------------------------------------------------
void cGameGuiController::showReportsWindow()
{
	auto reportsWindow = application.show (std::make_shared<cWindowReports> (getPlayers(), getActivePlayer(), getCasualtiesTracker(), getTurnClock(), getTurnTimeClock(), getGameSettings()));

	signalConnectionManager.connect (reportsWindow->unitClickedSecondTime, [this, reportsWindow] (cUnit & unit)
	{
		gameGui->getGameMap().getUnitSelection().selectUnit (unit);
		gameGui->getGameMap().centerAt (unit.getPosition());
		reportsWindow->close();
	});

	signalConnectionManager.connect (reportsWindow->reportClickedSecondTime, [this, reportsWindow] (const cSavedReport & report)
	{
		if (report.hasPosition())
		{
			gameGui->getGameMap().centerAt (report.getPosition());
			reportsWindow->close();
		}
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showUnitHelpWindow (const cUnit& unit)
{
	application.show (std::make_shared<cWindowUnitInfo> (unit.data, *unit.getOwner()));
}

//------------------------------------------------------------------------------
void cGameGuiController::showUnitTransferDialog (const cUnit& sourceUnit, const cUnit& destinationUnit)
{
	auto transferDialog = application.show (std::make_shared<cNewDialogTransfer> (sourceUnit, destinationUnit));
	transferDialog->done.connect ([&, transferDialog]()
	{
		transferTriggered (sourceUnit, destinationUnit, transferDialog->getTransferValue(), transferDialog->getResourceType());
		transferDialog->close();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showBuildBuildingsWindow (const cVehicle& vehicle)
{
	auto buildWindow = application.show (std::make_shared<cWindowBuildBuildings> (vehicle, getTurnTimeClock()));

	buildWindow->canceled.connect ([buildWindow]() { buildWindow->close(); });
	buildWindow->done.connect ([&, buildWindow]()
	{
		if (buildWindow->getSelectedUnitId())
		{
			if (buildWindow->getSelectedUnitId()->getUnitDataOriginalVersion()->isBig)
			{
				gameGui->getGameMap().startFindBuildPosition (*buildWindow->getSelectedUnitId());
				auto buildType = *buildWindow->getSelectedUnitId();
				auto buildSpeed = buildWindow->getSelectedBuildSpeed();

				buildPositionSelectionConnectionManager.disconnectAll();
				buildPositionSelectionConnectionManager.connect (gameGui->getGameMap().selectedBuildPosition, [this, buildType, buildSpeed] (const cVehicle & selectedVehicle, const cPosition & destination)
				{
					buildBuildingTriggered (selectedVehicle, destination, buildType, buildSpeed);
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
	buildWindow->donePath.connect ([&, buildWindow]()
	{
		if (buildWindow->getSelectedUnitId())
		{
			gameGui->getGameMap().startFindPathBuildPosition();
			auto buildType = *buildWindow->getSelectedUnitId();
			auto buildSpeed = buildWindow->getSelectedBuildSpeed();

			buildPositionSelectionConnectionManager.disconnectAll();
			buildPositionSelectionConnectionManager.connect (gameGui->getGameMap().selectedBuildPathDestination, [this, buildType, buildSpeed] (const cVehicle & selectedVehicle, const cPosition & destination)
			{
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
	const auto dynamicMap = getDynamicMap();

	if (!dynamicMap) return;

	auto buildWindow = application.show (std::make_shared<cWindowBuildVehicles> (building, *dynamicMap, getTurnTimeClock()));

	buildWindow->canceled.connect ([buildWindow]() { buildWindow->close(); });
	buildWindow->done.connect ([&, buildWindow]()
	{
		buildVehiclesTriggered (building, buildWindow->getBuildList(), buildWindow->getSelectedBuildSpeed(), buildWindow->isRepeatActive());
		buildWindow->close();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showResourceDistributionDialog (const cUnit& unit)
{
	if (!unit.isABuilding()) return;

	const auto& building = static_cast<const cBuilding&> (unit);

	auto resourceDistributionWindow = application.show (std::make_shared<cWindowResourceDistribution> (*building.SubBase, getTurnTimeClock()));
	resourceDistributionWindow->done.connect ([&, resourceDistributionWindow]()
	{
		changeResourceDistributionTriggered (building, resourceDistributionWindow->getMetalProduction(), resourceDistributionWindow->getOilProduction(), resourceDistributionWindow->getGoldProduction());
		resourceDistributionWindow->close();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showResearchDialog (const cUnit& unit)
{
	const auto player = getActivePlayer();
	if (unit.getOwner() != player.get()) return;
	if (!player) return;

	// clear list with research areas finished this turn.
	// NOTE: do we really want to do this here?
	unit.getOwner()->setCurrentTurnResearchAreasFinished (std::vector<int> ());

	auto researchDialog = application.show (std::make_shared<cDialogResearch> (*player));
	researchDialog->done.connect ([&, researchDialog]()
	{
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

	auto upgradesWindow = application.show (std::make_shared<cWindowUpgrades> (*player, getTurnTimeClock(), upgradesFilterState));


	upgradesWindow->canceled.connect ([upgradesWindow]() { upgradesWindow->close(); });
	upgradesWindow->done.connect ([&, upgradesWindow]()
	{
		takeUnitUpgradesTriggered (upgradesWindow->getUnitUpgrades());
		upgradesWindow->close();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showStorageWindow (const cUnit& unit)
{
	auto storageWindow = application.show (std::make_shared<cWindowStorage> (unit, getTurnTimeClock()));
	storageWindow->activate.connect ([&, storageWindow] (size_t index)
	{
		if (unit.isAVehicle() && unit.data.factorAir > 0)
		{
			activateAtTriggered (unit, index, unit.getPosition());
		}
		else
		{
			gameGui->getGameMap().startActivateVehicle (unit, index);
		}
		storageWindow->close();
	});
	storageWindow->reload.connect ([&] (size_t index)
	{
		reloadTriggered (unit, *unit.storedUnits[index]);
	});
	storageWindow->repair.connect ([&] (size_t index)
	{
		repairTriggered (unit, *unit.storedUnits[index]);
	});
	storageWindow->upgrade.connect ([&] (size_t index)
	{
		upgradeTriggered (unit, index);
	});
	storageWindow->activateAll.connect ([&, storageWindow]()
	{
		const auto dynamicMap = getDynamicMap();

		if (dynamicMap)
		{
			std::array<bool, 16> hasCheckedPlace;
			hasCheckedPlace.fill (false);

			for (size_t i = 0; i < unit.storedUnits.size(); ++i)
			{
				const auto& storedUnit = *unit.storedUnits[i];

				bool activated = false;
				for (int ypos = unit.getPosition().y() - 1, poscount = 0; ypos <= unit.getPosition().y() + (unit.data.isBig ? 2 : 1); ypos++)
				{
					if (ypos < 0 || ypos >= dynamicMap->getSize().y()) continue;
					for (int xpos = unit.getPosition().x() - 1; xpos <= unit.getPosition().x() + (unit.data.isBig ? 2 : 1); xpos++, poscount++)
					{
						if (hasCheckedPlace[poscount]) continue;

						if (xpos < 0 || xpos >= dynamicMap->getSize().x()) continue;

						if (((ypos == unit.getPosition().y() && unit.data.factorAir == 0) || (ypos == unit.getPosition().y() + 1 && unit.data.isBig)) &&
							((xpos == unit.getPosition().x() && unit.data.factorAir == 0) || (xpos == unit.getPosition().x() + 1 && unit.data.isBig))) continue;

						if (unit.canExitTo (cPosition (xpos, ypos), *dynamicMap, storedUnit.data))
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
	storageWindow->reloadAll.connect ([&, storageWindow]()
	{
		if (!unit.isABuilding()) return;
		auto remainingResources = static_cast<const cBuilding&> (unit).SubBase->getMetal();
		for (size_t i = 0; i < unit.storedUnits.size() && remainingResources > 0; ++i)
		{
			const auto& storedUnit = *unit.storedUnits[i];

			if (storedUnit.data.getAmmo() < storedUnit.data.getAmmoMax())
			{
				reloadTriggered (unit, storedUnit);
				remainingResources--;
			}
		}
	});
	storageWindow->repairAll.connect ([&, storageWindow]()
	{
		if (!unit.isABuilding()) return;
		auto remainingResources = static_cast<const cBuilding&> (unit).SubBase->getMetal();
		for (size_t i = 0; i < unit.storedUnits.size() && remainingResources > 0; ++i)
		{
			const auto& storedUnit = *unit.storedUnits[i];

			if (storedUnit.data.getHitpoints() < storedUnit.data.getHitpointsMax())
			{
				repairTriggered (unit, storedUnit);
				auto value = storedUnit.data.getHitpoints();
				while (value < storedUnit.data.getHitpointsMax() && remainingResources > 0)
				{
					value += Round (((float)storedUnit.data.getHitpointsMax() / storedUnit.data.buildCosts) * 4);
					remainingResources--;
				}
			}
		}
	});
	storageWindow->upgradeAll.connect ([&, storageWindow]()
	{
		upgradeAllTriggered (unit);
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showSelfDestroyDialog (const cUnit& unit)
{
	if (unit.data.canSelfDestroy)
	{
		auto selfDestroyDialog = application.show (std::make_shared<cDialogSelfDestruction> (unit, animationTimer));
		signalConnectionManager.connect (selfDestroyDialog->triggeredDestruction, [this, selfDestroyDialog, &unit]()
		{
			selfDestructionTriggered (unit);
			selfDestroyDialog->close();
		});
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::handleChatCommand(const std::string& chatString)
{
	if(cChatCommand::isCommand(chatString))
	{
		bool commandExecuted = false;
		bool errorDetected = false;
		try
		{
			for(const auto& commandExecutor : chatCommands)
			{
				if(commandExecutor->tryExecute(chatString))
				{
					commandExecuted = true;
					if(commandExecutor->getCommand().getShouldBeReported() && activeClient && activeClient->getServer())
					{
						sendSavedReport(*activeClient->getServer(), cSavedReportHostCommand(chatString), nullptr);
					}
					break;
				}
			}
		}
		catch(const std::runtime_error& e)
		{
			gameGui->getChatBox().addChatEntry(std::make_unique<cLobbyChatBoxListViewItem>(chatString));
			gameGui->getChatBox().addChatEntry(std::make_unique<cLobbyChatBoxListViewItem>(e.what()));
			errorDetected = true;
		}

		if(!commandExecuted && !errorDetected)
		{
			// TODO: translate
			gameGui->getChatBox().addChatEntry(std::make_unique<cLobbyChatBoxListViewItem>("Could not recognize chat command '" + chatString + "'"));
		}
	}
	else if(activeClient)
	{
		activeClient->handleChatMessage(chatString);
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::handleReport (const cSavedReport& report)
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
	else if (report.hasPosition())
	{
		savedReportPosition.first = true;
		savedReportPosition.second = report.getPosition();

		gameGui->getGameMessageList().addMessage (report.getMessage() + " (" + KeysList.keyJumpToAction.toString() + ")", eGameMessageListViewItemBackgroundColor::LightGray);
	}
	else
	{
		gameGui->getGameMessageList().addMessage (report.getMessage(), report.isAlert() ? eGameMessageListViewItemBackgroundColor::Red : eGameMessageListViewItemBackgroundColor::DarkGray);
		if (report.isAlert()) soundManager->playSound (std::make_shared<cSoundEffect> (eSoundEffectType::EffectAlert, SoundData.SNDQuitsch));
	}

	report.playSound (*soundManager);

	if (cSettings::getInstance().isDebug()) Log.write (report.getMessage(), cLog::eLOG_TYPE_DEBUG);
}

//------------------------------------------------------------------------------
void cGameGuiController::selectNextUnit()
{
	const auto player = getActivePlayer();
	if (!player) return;

	const auto nextUnit = player->getNextUnit (gameGui->getGameMap().getUnitSelection().getSelectedUnit());
	if (nextUnit)
	{
		gameGui->getGameMap().getUnitSelection().selectUnit (*nextUnit);
		gameGui->getGameMap().centerAt (nextUnit->getPosition());
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::selectPreviousUnit()
{
	const auto player = getActivePlayer();
	if (!player) return;

	const auto prevUnit = player->getPrevUnit (gameGui->getGameMap().getUnitSelection().getSelectedUnit());
	if (prevUnit)
	{
		gameGui->getGameMap().getUnitSelection().selectUnit (*prevUnit);
		gameGui->getGameMap().centerAt (prevUnit->getPosition());
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
		unit->setMarkedAsDone (true);
		resumeMoveJobTriggered (*unit);
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
	if (index > savedPositions.size()) return;

	savedPositions[index] = std::make_pair (true, gameGui->getGameMap().getMapCenterOffset());
}

//------------------------------------------------------------------------------
void cGameGuiController::jumpToSavedPosition (size_t index)
{
	if (index > savedPositions.size()) return;

	if (!savedPositions[index].first) return;

	gameGui->getGameMap().centerAt (savedPositions[index].second);
}

//------------------------------------------------------------------------------
std::vector<std::shared_ptr<const cPlayer>> cGameGuiController::getPlayers() const
{
	std::vector<std::shared_ptr<const cPlayer>> result;

	if (!activeClient) return result;

	const auto& clientPlayerList = activeClient->getPlayerList();

	result.resize (clientPlayerList.size());
	std::copy (clientPlayerList.begin(), clientPlayerList.end(), result.begin());

	return result;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cPlayer> cGameGuiController::getActivePlayer() const
{
	if (!activeClient) return nullptr;

	const auto& clientPlayerList = activeClient->getPlayerList();

	auto iter = std::find_if (clientPlayerList.begin(), clientPlayerList.end(), [this] (const std::shared_ptr<cPlayer>& player) { return player->getNr() == activeClient->getActivePlayer().getNr(); });

	if (iter == clientPlayerList.end()) return nullptr;  // should never happen; just to be on the safe side

	return *iter;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cTurnClock> cGameGuiController::getTurnClock() const
{
	return activeClient ? activeClient->getTurnClock() : nullptr;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cTurnTimeClock> cGameGuiController::getTurnTimeClock() const
{
	return activeClient ? activeClient->getTurnTimeClock() : nullptr;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cGameSettings> cGameGuiController::getGameSettings() const
{
	return activeClient ? activeClient->getGameSettings() : nullptr;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cCasualtiesTracker> cGameGuiController::getCasualtiesTracker() const
{
	return activeClient ? activeClient->getCasualtiesTracker() : nullptr;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cMap> cGameGuiController::getDynamicMap() const
{
	return activeClient ? activeClient->getMap() : nullptr;
}