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

#include "ui/graphical/game/control/gameguicontroller.h"
#include "ui/graphical/game/gamegui.h"
#include "ui/graphical/game/hud.h"
#include "ui/graphical/game/widgets/gamemapwidget.h"
#include "ui/graphical/game/widgets/gamemessagelistview.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "ui/graphical/game/widgets/debugoutputwidget.h"

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
#include "game/logic/attackjobs.h"
#include "game/data/player/player.h"
#include "game/data/report/savedreportsimple.h"
#include "game/data/report/savedreportchat.h"
#include "game/data/report/savedreportunit.h"
#include "game/data/report/special/savedreporthostcommand.h"

//------------------------------------------------------------------------------
cGameGuiController::cGameGuiController (cApplication& application_, std::shared_ptr<const cStaticMap> staticMap) :
	application (application_),
	soundManager (std::make_shared<cSoundManager> ()),
	animationTimer (std::make_shared<cAnimationTimer> ()),
	gameGui (std::make_shared<cGameGui> (std::move (staticMap), soundManager, animationTimer)),
	savedReportPosition (false, cPosition ())
{
	connectGuiStaticCommands ();
	initShortcuts ();
	application.addRunnable (animationTimer);

	for (size_t i = 0; i < savedPositions.size (); ++i)
	{
		savedPositions[i] = std::make_pair (false, cPosition ());
	}
}

//------------------------------------------------------------------------------
cGameGuiController::~cGameGuiController ()
{
	application.removeRunnable (*animationTimer);
}

//------------------------------------------------------------------------------
void cGameGuiController::start ()
{
	application.show (gameGui);

	if (activeClient)
	{
		auto iter = playerGameGuiStates.find (activeClient->getActivePlayer ().getNr ());
		if (iter != playerGameGuiStates.end ())
		{
			gameGui->restoreState (iter->second);
		}

		if (activeClient->getGameSettings ()->getGameType () == eGameSettingsGameType::HotSeat)
		{
			showNextPlayerDialog ();
		}
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::addPlayerGameGuiState (const cPlayer& player, cGameGuiState playerGameGuiState)
{
	playerGameGuiStates[player.getNr ()] = std::move (playerGameGuiState);
}

//------------------------------------------------------------------------------
void cGameGuiController::setSingleClient (std::shared_ptr<cClient> client)
{
	std::vector<std::shared_ptr<cClient>> clients;
	int activePlayerNumber = 0;
	if (client != nullptr)
	{
		clients.push_back (client);
		activePlayerNumber = client->getActivePlayer ().getNr ();
	}
	setClients (std::move (clients), activePlayerNumber);
}

//------------------------------------------------------------------------------
void cGameGuiController::setClients (std::vector<std::shared_ptr<cClient>> clients_, int activePlayerNumber)
{
	allClientsSignalConnectionManager.disconnectAll ();

	clients = std::move (clients_);

	auto iter = std::find_if (clients.begin (), clients.end (), [=](const std::shared_ptr<cClient>& client){ return client->getActivePlayer ().getNr () == activePlayerNumber; });
	if (iter != clients.end()) setActiveClient (*iter);
	else setActiveClient (nullptr);

	for (size_t i = 0; i < clients.size (); ++i)
	{
		auto client = clients[i].get();
		allClientsSignalConnectionManager.connect (clients[i]->additionalSaveInfoRequested, [this, client](int saveingId)
		{
			if (client == activeClient.get ())
			{
				sendGameGuiState (*client, gameGui->getCurrentState (), client->getActivePlayer (), saveingId);
			}
			else
			{
				sendGameGuiState (*client, playerGameGuiStates[client->getActivePlayer ().getNr ()], client->getActivePlayer (), saveingId);
			}
		});

		allClientsSignalConnectionManager.connect (clients[i]->gameGuiStateReceived, [this, client](const cGameGuiState& state)
		{
			playerGameGuiStates[client->getActivePlayer ().getNr ()] = state;
			if (client == activeClient.get ())
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

	gameGui->setDynamicMap (getDynamicMap ());
	gameGui->setPlayers (getPlayers ());
	gameGui->setPlayer (getActivePlayer ());
	gameGui->setTurnClock (getTurnClock ());
	gameGui->setTurnTimeClock (getTurnTimeClock ());
	gameGui->setGameSettings (getGameSettings ());

	gameGui->getDebugOutput ().setClient (activeClient.get ());
	gameGui->getDebugOutput ().setServer (activeClient->getServer ());

	if (activeClient != nullptr)
	{
		connectClient (*activeClient);
		auto iter = playerGameGuiStates.find (activeClient->getActivePlayer ().getNr ());
		if (iter != playerGameGuiStates.end ())
		{
			gameGui->restoreState (iter->second);
		}
	}
	else
	{
		clientSignalConnectionManager.disconnectAll ();
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::initShortcuts ()
{
	auto exitShortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keyExit));
	signalConnectionManager.connect (exitShortcut->triggered, [&]()
	{
		auto yesNoDialog = application.show (std::make_shared<cDialogYesNo> (lngPack.i18n ("Text~Comp~End_Game")));
		signalConnectionManager.connect (yesNoDialog->yesClicked, [&]()
		{
			gameGui->exit ();
		});
	});

	auto jumpToActionShortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keyJumpToAction));
	signalConnectionManager.connect (jumpToActionShortcut->triggered, [&]()
	{
		if (savedReportPosition.first)
		{
			gameGui->getGameMap ().centerAt (savedReportPosition.second);
		}
	});

	auto doneAndNextShortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitDoneAndNext));
	signalConnectionManager.connect (doneAndNextShortcut->triggered, [&]()
	{
		markSelectedUnitAsDone ();
		selectNextUnit ();
	});

	auto allDoneAndNextShortcut = gameGui->addShortcut (std::make_unique<cShortcut> (KeysList.keyAllDoneAndNext));
	signalConnectionManager.connect (allDoneAndNextShortcut->triggered, [&]()
	{
		resumeAllMoveJobsTriggered ();
		selectNextUnit ();
	});

	auto savePosition1Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (cKeySequence (cKeyCombination (toEnumFlag (eKeyModifierType::AltLeft) | eKeyModifierType::AltRight, SDLK_F5))));
	signalConnectionManager.connect (savePosition1Shortcut->triggered, std::bind (&cGameGuiController::savePosition, this, 0));

	auto savePosition2Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (cKeySequence (cKeyCombination (toEnumFlag (eKeyModifierType::AltLeft) | eKeyModifierType::AltRight, SDLK_F6))));
	signalConnectionManager.connect (savePosition2Shortcut->triggered, std::bind (&cGameGuiController::savePosition, this, 1));

	auto savePosition3Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (cKeySequence (cKeyCombination (toEnumFlag (eKeyModifierType::AltLeft) | eKeyModifierType::AltRight, SDLK_F7))));
	signalConnectionManager.connect (savePosition3Shortcut->triggered, std::bind (&cGameGuiController::savePosition, this, 2));

	auto savePosition4Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (cKeySequence (cKeyCombination (toEnumFlag (eKeyModifierType::AltLeft) | eKeyModifierType::AltRight, SDLK_F8))));
	signalConnectionManager.connect (savePosition4Shortcut->triggered, std::bind (&cGameGuiController::savePosition, this, 3));

	auto loadPosition1Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_F5))));
	signalConnectionManager.connect (loadPosition1Shortcut->triggered, std::bind (&cGameGuiController::jumpToSavedPosition, this, 0));

	auto loadPosition2Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_F6))));
	signalConnectionManager.connect (loadPosition2Shortcut->triggered, std::bind (&cGameGuiController::jumpToSavedPosition, this, 1));

	auto loadPosition3Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_F7))));
	signalConnectionManager.connect (loadPosition3Shortcut->triggered, std::bind (&cGameGuiController::jumpToSavedPosition, this, 2));

	auto loadPosition4Shortcut = gameGui->addShortcut (std::make_unique<cShortcut> (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_F8))));
	signalConnectionManager.connect (loadPosition4Shortcut->triggered, std::bind (&cGameGuiController::jumpToSavedPosition, this, 3));

}

//------------------------------------------------------------------------------
void cGameGuiController::connectGuiStaticCommands ()
{
	using namespace std::placeholders;

	signalConnectionManager.connect (gameGui->terminated, [this](){ terminated (); });

	signalConnectionManager.connect (gameGui->getChatBox ().commandEntered, std::bind (&cGameGuiController::handleChatCommand, this, _1));

	signalConnectionManager.connect (gameGui->getHud().preferencesClicked, std::bind (&cGameGuiController::showPreferencesDialog, this));
	signalConnectionManager.connect (gameGui->getHud().filesClicked, std::bind (&cGameGuiController::showFilesWindow, this));

	signalConnectionManager.connect (gameGui->getHud().centerClicked, std::bind (&cGameGuiController::centerSelectedUnit, this));

	signalConnectionManager.connect (gameGui->getHud().nextClicked, std::bind (&cGameGuiController::selectNextUnit, this));
	signalConnectionManager.connect (gameGui->getHud().prevClicked, std::bind (&cGameGuiController::selectPreviousUnit, this));
	signalConnectionManager.connect (gameGui->getHud().doneClicked, std::bind (&cGameGuiController::markSelectedUnitAsDone, this));

	signalConnectionManager.connect (gameGui->getHud().reportsClicked, std::bind (&cGameGuiController::showReportsWindow, this));

	signalConnectionManager.connect (gameGui->getGameMap ().triggeredUnitHelp, std::bind (&cGameGuiController::showUnitHelpWindow, this, _1));
	signalConnectionManager.connect (gameGui->getGameMap ().triggeredTransfer, std::bind (&cGameGuiController::showUnitTransferDialog, this, _1, _2));
	signalConnectionManager.connect (gameGui->getGameMap ().triggeredBuild, [&](const cUnit& unit)
	{
		if (unit.isAVehicle ())
		{
			showBuildBuildingsWindow (static_cast<const cVehicle&>(unit));
		}
		else if (unit.isABuilding ())
		{
			showBuildVehiclesWindow (static_cast<const cBuilding&>(unit));
		}
	});
	signalConnectionManager.connect (gameGui->getGameMap ().triggeredResourceDistribution, std::bind (&cGameGuiController::showResourceDistributionDialog, this, _1));
	signalConnectionManager.connect (gameGui->getGameMap ().triggeredResearchMenu, std::bind (&cGameGuiController::showResearchDialog, this, _1));
	signalConnectionManager.connect (gameGui->getGameMap ().triggeredUpgradesMenu, std::bind (&cGameGuiController::showUpgradesWindow, this, _1));
	signalConnectionManager.connect (gameGui->getGameMap ().triggeredActivate, std::bind (&cGameGuiController::showStorageWindow, this, _1));
	signalConnectionManager.connect (gameGui->getGameMap ().triggeredSelfDestruction, std::bind (&cGameGuiController::showSelfDestroyDialog, this, _1));
}

//------------------------------------------------------------------------------
void cGameGuiController::connectClient (cClient& client)
{
	clientSignalConnectionManager.disconnectAll ();

	soundManager->setGameTimer (client.getGameTimer ());

	//
	// GUI to client (action)
	//
	clientSignalConnectionManager.connect (transferTriggered, [&](const cUnit& sourceUnit, const cUnit& destinationUnit, int transferValue, int resourceType)
	{
		if (transferValue != 0)
		{
			sendWantTransfer (client, sourceUnit.isAVehicle (), sourceUnit.iID, destinationUnit.isAVehicle (), destinationUnit.iID, transferValue, resourceType);
		}
	});
	clientSignalConnectionManager.connect (buildBuildingTriggered, [&](const cVehicle& vehicle, const cPosition& destination, const sID& unitId, int buildSpeed)
	{
		sendWantBuild (client, vehicle.iID, unitId, buildSpeed, destination, false, cPosition (0, 0));
	});
	clientSignalConnectionManager.connect (buildBuildingPathTriggered, [&](const cVehicle& vehicle, const cPosition& destination, const sID& unitId, int buildSpeed)
	{
		sendWantBuild (client, vehicle.iID, unitId, buildSpeed, vehicle.getPosition (), true, destination);
	});
	clientSignalConnectionManager.connect (buildVehiclesTriggered, [&](const cBuilding& building, const std::vector<cBuildListItem>& buildList, int buildSpeed, bool repeat)
	{
		sendWantBuildList (client, building, buildList, repeat, buildSpeed);
	});
	clientSignalConnectionManager.connect (activateAtTriggered, [&](const cUnit& unit, size_t index, const cPosition& position)
	{
		sendWantActivate (client, unit.iID, unit.isAVehicle (), unit.storedUnits[index]->iID, position);
	});
	clientSignalConnectionManager.connect (reloadTriggered, [&](const cUnit& sourceUnit, const cUnit& destinationUnit)
	{
		sendWantSupply (client, destinationUnit.iID, destinationUnit.isAVehicle (), sourceUnit.iID, sourceUnit.isAVehicle (), SUPPLY_TYPE_REARM);
	});
	clientSignalConnectionManager.connect (repairTriggered, [&](const cUnit& sourceUnit, const cUnit& destinationUnit)
	{
		sendWantSupply (client, destinationUnit.iID, destinationUnit.isAVehicle (), sourceUnit.iID, sourceUnit.isAVehicle (), SUPPLY_TYPE_REPAIR);
	});
	clientSignalConnectionManager.connect (upgradeTriggered, [&](const cUnit& unit, size_t index)
	{
		sendWantUpgrade (client, unit.iID, index, false);
	});
	clientSignalConnectionManager.connect (upgradeAllTriggered, [&](const cUnit& unit)
	{
		sendWantUpgrade (client, unit.iID, 0, true);
	});
	clientSignalConnectionManager.connect (changeResourceDistributionTriggered, [&](const cBuilding& building, int metalProduction, int oilProduction, int goldProduction)
	{
		sendChangeResources (client, building, metalProduction, oilProduction, goldProduction);
	});
	clientSignalConnectionManager.connect (changeResearchSettingsTriggered, [&](const std::array<int, cResearch::kNrResearchAreas>& newResearchSettings)
	{
		sendWantResearchChange (client, newResearchSettings);
	});
	clientSignalConnectionManager.connect (takeUnitUpgradesTriggered, [&](const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades)
	{
		sendTakenUpgrades (client, unitUpgrades);
	});
	clientSignalConnectionManager.connect (selfDestructionTriggered, [&](const cUnit& unit)
	{
		if (!unit.data.ID.isABuilding ()) return;
		sendWantSelfDestroy (client, static_cast<const cBuilding&>(unit));
	});
	clientSignalConnectionManager.connect (resumeMoveJobTriggered, [&](const cUnit& unit)
	{
		sendMoveJobResume (client, unit.iID);
	});
	clientSignalConnectionManager.connect (resumeAllMoveJobsTriggered, [&]()
	{
		sendMoveJobResume (client, 0);
	});
	clientSignalConnectionManager.connect (gameGui->getHud ().endClicked, [&]()
	{
		client.handleEnd ();
	});
	clientSignalConnectionManager.connect (gameGui->getHud ().triggeredRenameUnit, [&](const cUnit& unit, const std::string& name)
	{
		sendWantChangeUnitName (client, name, unit.iID);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredStartWork, [&](const cUnit& unit)
	{
		sendWantStartWork (client, unit);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredStopWork, [&](const cUnit& unit)
	{
		unit.executeStopCommand (client);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredAutoMoveJob, [&](const cUnit& unit)
	{
		if (unit.data.ID.isAVehicle ())
		{
			auto vehicle = client.getVehicleFromID (unit.iID);
			if (!vehicle) return;
			vehicle->executeAutoMoveJobCommand (client);
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredStartClear, [&](const cUnit& unit)
	{
		if (unit.data.ID.isAVehicle ()) sendWantStartClear (client, static_cast<const cVehicle&> (unit));
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredManualFire, [&](const cUnit& unit)
	{
		sendChangeManualFireStatus (client, unit.iID, unit.isAVehicle ());
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredSentry, [&](const cUnit& unit)
	{
		sendChangeSentry (client, unit.iID, unit.isAVehicle ());
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredUpgradeThis, [&](const cUnit& unit)
	{
		if (unit.data.ID.isABuilding ()) static_cast<const cBuilding&>(unit).executeUpdateBuildingCommmand (client, false);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredUpgradeAll, [&](const cUnit& unit)
	{
		if (unit.data.ID.isABuilding ()) static_cast<const cBuilding&>(unit).executeUpdateBuildingCommmand (client, true);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredLayMines, [&](const cUnit& unit)
	{
		if (unit.data.ID.isAVehicle ())
		{
			auto vehicle = client.getVehicleFromID (unit.iID);
			if (!vehicle) return;
			vehicle->executeLayMinesCommand (client);
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredCollectMines, [&](const cUnit& unit)
	{
		if (unit.data.ID.isAVehicle ())
		{
			auto vehicle = client.getVehicleFromID (unit.iID);
			if (!vehicle) return;
			vehicle->executeClearMinesCommand (client);
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredUnitDone, [&](const cUnit& unit)
	{
		if (unit.data.ID.isAVehicle ())
		{
			auto vehicle = client.getVehicleFromID (unit.iID);
			if (!vehicle) return;
			vehicle->setMarkedAsDone (true);
			sendMoveJobResume (client, vehicle->iID);
		}
		else if (unit.data.ID.isABuilding ())
		{
			auto building = client.getBuildingFromID (unit.iID);
			if (!building) return;
			building->setMarkedAsDone (true);
		}
	});

	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredEndBuilding, [&](cVehicle& vehicle, const cPosition& destination)
	{
		sendWantEndBuilding (client, vehicle, destination);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredMoveSingle, [&](cVehicle& vehicle, const cPosition& destination)
	{
		const auto successfull = client.addMoveJob (vehicle, destination);
		if (!successfull)
		{
			soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredMoveGroup, [&](const std::vector<cVehicle*>& vehicles, const cPosition& destination)
	{
		client.startGroupMove (vehicles, destination);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredActivateAt, [&](const cUnit& unit, size_t index, const cPosition& position)
	{
		sendWantActivate (client, unit.iID, unit.isAVehicle (), unit.storedUnits[index]->iID, position);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredExitFinishedUnit, [&](const cBuilding& building, const cPosition& position)
	{
		sendWantExitFinishedVehicle (client, building, position);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredLoadAt, [&](const cUnit& unit, const cPosition& position)
	{
		const auto& field = client.getMap ()->getField (position);
		auto overVehicle = field.getVehicle ();
		auto overPlane = field.getPlane ();
		if (unit.isAVehicle ())
		{
			const auto& vehicle = static_cast<const cVehicle&>(unit);
			if (vehicle.data.factorAir > 0 && overVehicle)
			{
				if (overVehicle->getPosition () == vehicle.getPosition ()) sendWantLoad (client, vehicle.iID, true, overVehicle->iID);
				else
				{
					cPathCalculator pc (vehicle.getPosition (), overVehicle->getPosition (), *client.getMap (), vehicle);
					sWaypoint* path = pc.calcPath ();
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
				if (vehicle.isNextTo (overVehicle->getPosition ())) sendWantLoad (client, vehicle.iID, true, overVehicle->iID);
				else
				{
					cPathCalculator pc (overVehicle->getPosition (), vehicle, *client.getMap (), *overVehicle, true);
					sWaypoint* path = pc.calcPath ();
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
		else if (unit.isABuilding ())
		{
			const auto& building = static_cast<const cBuilding&>(unit);
			if (overVehicle && building.canLoad (overVehicle, false))
			{
				if (building.isNextTo (overVehicle->getPosition ())) sendWantLoad (client, building.iID, false, overVehicle->iID);
				else
				{
					cPathCalculator pc (overVehicle->getPosition (), building, *client.getMap (), *overVehicle, true);
					sWaypoint* path = pc.calcPath ();
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
				if (building.isNextTo (overPlane->getPosition ())) sendWantLoad (client, building.iID, false, overPlane->iID);
				else
				{
					cPathCalculator pc (overPlane->getPosition (), building, *client.getMap (), *overPlane, true);
					sWaypoint* path = pc.calcPath ();
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
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredSupplyAmmo, [&](const cUnit& sourceUnit, const cUnit& destinationUnit)
	{
		sendWantSupply (client, destinationUnit.iID, destinationUnit.isAVehicle (), sourceUnit.iID, sourceUnit.isAVehicle (), SUPPLY_TYPE_REARM);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredRepair, [&](const cUnit& sourceUnit, const cUnit& destinationUnit)
	{
		sendWantSupply (client, destinationUnit.iID, destinationUnit.isAVehicle (), sourceUnit.iID, sourceUnit.isAVehicle (), SUPPLY_TYPE_REPAIR);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredAttack, [&](const cUnit& unit, const cPosition& position)
	{
		if (unit.isAVehicle ())
		{
			const auto& vehicle = static_cast<const cVehicle&>(unit);

			cUnit* target = selectTarget (position, vehicle.data.canAttack, *client.getMap ());

			if (vehicle.isInRange (position))
			{
				// find target ID
				int targetId = 0;
				if (target && target->isAVehicle ()) targetId = target->iID;

				Log.write (" Client: want to attack " + iToStr (position.x ()) + ":" + iToStr (position.y ()) + ", Vehicle ID: " + iToStr (targetId), cLog::eLOG_TYPE_NET_DEBUG);
				sendWantVehicleAttack (client, targetId, position, vehicle.iID);
			}
			else if (target)
			{
				cPathCalculator pc (vehicle.getPosition (), *client.getMap (), vehicle, position);
				sWaypoint* path = pc.calcPath ();
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
		else if (unit.isABuilding ())
		{
			const auto& building = static_cast<const cBuilding&>(unit);
			const cMap& map = *client.getMap ();

			int targetId = 0;
			cUnit* target = selectTarget (position, building.data.canAttack, map);
			if (target && target->isAVehicle ()) targetId = target->iID;

			sendWantBuildingAttack (client, targetId, position, building.getPosition ());
		}
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredSteal, [&](const cUnit& sourceUnit, const cUnit& destinationUnit)
	{
		sendWantComAction (client, sourceUnit.iID, destinationUnit.iID, destinationUnit.isAVehicle (), true);
	});
	clientSignalConnectionManager.connect (gameGui->getGameMap ().triggeredDisable, [&](const cUnit& sourceUnit, const cUnit& destinationUnit)
	{
		sendWantComAction (client, sourceUnit.iID, destinationUnit.iID, destinationUnit.isAVehicle (), false);
	});


	//
	// client to GUI (reaction)
	//
	clientSignalConnectionManager.connect (client.playerFinishedTurn, [&](int currentPlayerNumber, int nextPlayerNumber)
	{
		if (currentPlayerNumber != client.getActivePlayer ().getNr ()) return;

		if (client.getGameSettings ()->getGameType () == eGameSettingsGameType::HotSeat)
		{
			gameGui->getHud ().unlockEndButton ();

			auto iter = std::find_if (clients.begin (), clients.end (), [=](const std::shared_ptr<cClient>& client){ return client->getActivePlayer ().getNr () == nextPlayerNumber; });
			if (iter != clients.end ())
			{
				playerGameGuiStates[currentPlayerNumber] = gameGui->getCurrentState ();

				setActiveClient (*iter);

				showNextPlayerDialog ();
			}
			else setActiveClient (nullptr);
		}
	});

	clientSignalConnectionManager.connect (client.freezeModeChanged, [&](eFreezeMode mode)
	{
		const int playerNumber = client.getFreezeInfoPlayerNumber ();
		const cPlayer* player = client.getPlayerFromNumber (playerNumber);

		if (mode == FREEZE_WAIT_FOR_OTHERS || mode == FREEZE_WAIT_FOR_TURNEND)
		{
			if (client.getFreezeMode (FREEZE_WAIT_FOR_OTHERS) || client.getFreezeMode (FREEZE_WAIT_FOR_TURNEND)) gameGui->getHud ().lockEndButton ();
			else gameGui->getHud ().unlockEndButton ();
		}

		if (client.getFreezeMode (FREEZE_WAIT_FOR_OTHERS))
		{
			// TODO: Fix message
			const std::string& name = player ? player->getName () : "other players";
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
			std::string s = client.getServer () ? lngPack.i18n ("Text~Multiplayer~Abort_Waiting") : "";
			gameGui->setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_Reconnect"), s);
		}
		else if (client.getFreezeMode (FREEZE_WAIT_FOR_PLAYER))
		{
			gameGui->setInfoTexts (lngPack.i18n ("Text~Multiplayer~No_Response", player->getName ()), "");
		}
		else if (client.getFreezeMode (FREEZE_WAIT_FOR_TURNEND))
		{
			gameGui->setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_TurnEnd"), "");
		}
		else
		{
			gameGui->setInfoTexts ("", "");
		}
	});

	clientSignalConnectionManager.connect (client.unitStored, [&](const cUnit& storingUnit, const cUnit& /*storedUnit*/)
	{
		soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectLoad, SoundData.SNDLoad, storingUnit));
	});

	clientSignalConnectionManager.connect (client.unitActivated, [&](const cUnit& storingUnit, const cUnit& /*storedUnit*/)
	{
		soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectActivate, SoundData.SNDActivate, storingUnit));
	});

	clientSignalConnectionManager.connect (client.unitHasStolenSuccessfully, [&](const cUnit&)
	{
		soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceCommandoAction, getRandom (VoiceData.VOIUnitStolen)));
	});

	clientSignalConnectionManager.connect (client.unitHasDisabledSuccessfully, [&](const cUnit&)
	{
		soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceCommandoAction, VoiceData.VOIUnitDisabled));
	});

	clientSignalConnectionManager.connect (client.unitStealDisableFailed, [&](const cUnit&)
	{
		soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceCommandoAction, getRandom (VoiceData.VOICommandoFailed)));
	});

	clientSignalConnectionManager.connect (client.unitSuppliedWithAmmo, [&](const cUnit& unit)
	{
		soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectReload, SoundData.SNDReload, unit));
		soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceReload, VoiceData.VOIReammo));
	});

	clientSignalConnectionManager.connect (client.unitRepaired, [&](const cUnit& unit)
	{
		soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectRepair, SoundData.SNDRepair, unit));
		soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceRepair, getRandom (VoiceData.VOIRepaired)));
	});

	clientSignalConnectionManager.connect (client.addedEffect, [&](const std::shared_ptr<cFx>& effect)
	{
		gameGui->getGameMap ().addEffect (effect);
	});

	clientSignalConnectionManager.connect (client.getTurnTimeClock ()->alertTimeReached, [this]()
	{
		soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceTurnAlertTimeReached, getRandom (VoiceData.VOITurnEnd20Sec)));
	});

	//
	// client player to GUI
	//
	auto& player = client.getActivePlayer ();

	using namespace std::placeholders;
	clientSignalConnectionManager.connect (player.reportAdded, std::bind (&cGameGuiController::handleReport, this, _1));
}

//------------------------------------------------------------------------------
void cGameGuiController::showNextPlayerDialog ()
{
	soundManager->mute ();
	auto dialog = application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Text~Multiplayer~Player_Turn", activeClient->getActivePlayer ().getName ()), eWindowBackgrounds::Black));
	signalConnectionManager.connect (dialog->done, [this]()
	{
		soundManager->unmute ();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showFilesWindow ()
{
	auto loadSaveWindow = application.show (std::make_shared<cWindowLoadSave> (getTurnTimeClock()));
	loadSaveWindow->exit.connect ([this, loadSaveWindow]()
	{
		auto yesNoDialog = application.show (std::make_shared<cDialogYesNo> (lngPack.i18n ("Text~Comp~End_Game")));
		signalConnectionManager.connect (yesNoDialog->yesClicked, [&, loadSaveWindow]()
		{
			loadSaveWindow->close ();
			gameGui->exit ();
		});
	});
	loadSaveWindow->load.connect ([this, loadSaveWindow](int saveNumber)
	{
		// loading games while game is running is not yet implemented
		application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Text~Error_Messages~INFO_Not_Implemented")));
	});
	loadSaveWindow->save.connect ([this, loadSaveWindow](int saveNumber, const std::string& name)
	{
		try
		{
			triggeredSave (saveNumber, name);

			cSoundDevice::getInstance ().getFreeVoiceChannel ().play (VoiceData.VOISaved);

			loadSaveWindow->update ();
		}
		catch (std::runtime_error& e)
		{
			application.show (std::make_shared<cDialogOk> (e.what ()));
		}
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showPreferencesDialog ()
{
	application.show (std::make_shared<cDialogPreferences> ());
}

//------------------------------------------------------------------------------
void cGameGuiController::showReportsWindow ()
{
	auto reportsWindow = application.show (std::make_shared<cWindowReports> (getPlayers(), getActivePlayer(), getCasualtiesTracker(), getTurnClock(), getTurnTimeClock(), getGameSettings()));

	signalConnectionManager.connect (reportsWindow->unitClickedSecondTime, [this, reportsWindow](cUnit& unit)
	{
		gameGui->getGameMap ().getUnitSelection ().selectUnit (unit);
		gameGui->getGameMap ().centerAt (unit.getPosition ());
		reportsWindow->close ();
	});

	signalConnectionManager.connect (reportsWindow->reportClickedSecondTime, [this, reportsWindow](const cSavedReport& report)
	{
		if (report.hasPosition ())
		{
			gameGui->getGameMap ().centerAt (report.getPosition ());
			reportsWindow->close ();
		}
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showUnitHelpWindow (const cUnit& unit)
{
	application.show (std::make_shared<cWindowUnitInfo> (unit.data, *unit.getOwner ()));
}

//------------------------------------------------------------------------------
void cGameGuiController::showUnitTransferDialog (const cUnit& sourceUnit, const cUnit& destinationUnit)
{
	auto transferDialog = application.show (std::make_shared<cNewDialogTransfer> (sourceUnit, destinationUnit));
	transferDialog->done.connect ([&, transferDialog]()
	{
		transferTriggered (sourceUnit, destinationUnit, transferDialog->getTransferValue (), transferDialog->getResourceType ());
		transferDialog->close ();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showBuildBuildingsWindow (const cVehicle& vehicle)
{
	auto buildWindow = application.show (std::make_shared<cWindowBuildBuildings> (vehicle, getTurnTimeClock()));

	buildWindow->canceled.connect ([buildWindow]() { buildWindow->close (); });
	buildWindow->done.connect ([&, buildWindow]()
	{
		if (buildWindow->getSelectedUnitId ())
		{
			if (buildWindow->getSelectedUnitId ()->getUnitDataOriginalVersion ()->isBig)
			{
				gameGui->getGameMap ().startFindBuildPosition (*buildWindow->getSelectedUnitId ());
				auto buildType = *buildWindow->getSelectedUnitId ();
				auto buildSpeed = buildWindow->getSelectedBuildSpeed ();

				buildPositionSelectionConnectionManager.disconnectAll ();
				buildPositionSelectionConnectionManager.connect (gameGui->getGameMap ().selectedBuildPosition, [this, buildType, buildSpeed](const cVehicle& selectedVehicle, const cPosition& destination)
				{
					buildBuildingTriggered (selectedVehicle, destination, buildType, buildSpeed);
					buildPositionSelectionConnectionManager.disconnectAll ();
				});
			}
			else
			{
				buildBuildingTriggered (vehicle, vehicle.getPosition (), *buildWindow->getSelectedUnitId (), buildWindow->getSelectedBuildSpeed ());
			}
		}
		buildWindow->close ();
	});
	buildWindow->donePath.connect ([&, buildWindow]()
	{
		if (buildWindow->getSelectedUnitId ())
		{
			gameGui->getGameMap ().startFindPathBuildPosition ();
			auto buildType = *buildWindow->getSelectedUnitId ();
			auto buildSpeed = buildWindow->getSelectedBuildSpeed ();

			buildPositionSelectionConnectionManager.disconnectAll ();
			buildPositionSelectionConnectionManager.connect (gameGui->getGameMap ().selectedBuildPathDestination, [this, buildType, buildSpeed](const cVehicle& selectedVehicle, const cPosition& destination)
			{
				buildBuildingPathTriggered (selectedVehicle, destination, buildType, buildSpeed);
				buildPositionSelectionConnectionManager.disconnectAll ();
			});
		}
		buildWindow->close ();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showBuildVehiclesWindow (const cBuilding& building)
{
	const auto dynamicMap = getDynamicMap ();

	if (!dynamicMap) return;

	auto buildWindow = application.show (std::make_shared<cWindowBuildVehicles> (building, *dynamicMap, getTurnTimeClock()));

	buildWindow->canceled.connect ([buildWindow]() { buildWindow->close (); });
	buildWindow->done.connect ([&, buildWindow]()
	{
		buildVehiclesTriggered (building, buildWindow->getBuildList (), buildWindow->getSelectedBuildSpeed (), buildWindow->isRepeatActive ());
		buildWindow->close ();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showResourceDistributionDialog (const cUnit& unit)
{
	if (!unit.isABuilding ()) return;

	const auto& building = static_cast<const cBuilding&>(unit);

	auto resourceDistributionWindow = application.show (std::make_shared<cWindowResourceDistribution> (*building.SubBase, getTurnTimeClock ()));
	resourceDistributionWindow->done.connect ([&, resourceDistributionWindow]()
	{
		changeResourceDistributionTriggered (building, resourceDistributionWindow->getMetalProduction (), resourceDistributionWindow->getOilProduction (), resourceDistributionWindow->getGoldProduction ());
		resourceDistributionWindow->close ();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showResearchDialog (const cUnit& unit)
{
	const auto player = getActivePlayer ();
	if (unit.getOwner () != player.get ()) return;
	if (!player) return;

	// clear list with research areas finished this turn.
	// NOTE: do we really want to do this here?
	unit.getOwner ()->setCurrentTurnResearchAreasFinished (std::vector<int> ());

	auto researchDialog = application.show (std::make_shared<cDialogResearch> (*player));
	researchDialog->done.connect ([&, researchDialog]()
	{
		changeResearchSettingsTriggered (researchDialog->getResearchSettings ());
		researchDialog->close ();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showUpgradesWindow (const cUnit& unit)
{
	const auto player = getActivePlayer ();
	if (unit.getOwner () != player.get ()) return;
	if (!player) return;

	auto upgradesWindow = application.show (std::make_shared<cWindowUpgrades> (*player, getTurnTimeClock ()));

	upgradesWindow->canceled.connect ([upgradesWindow]() { upgradesWindow->close (); });
	upgradesWindow->done.connect ([&, upgradesWindow]()
	{
		takeUnitUpgradesTriggered (upgradesWindow->getUnitUpgrades ());
		upgradesWindow->close ();
	});
}

//------------------------------------------------------------------------------
void cGameGuiController::showStorageWindow (const cUnit& unit)
{
	auto storageWindow = application.show (std::make_shared<cWindowStorage> (unit, getTurnTimeClock ()));
	storageWindow->activate.connect ([&, storageWindow](size_t index)
	{
		if (unit.isAVehicle () && unit.data.factorAir > 0)
		{
			activateAtTriggered (unit, index, unit.getPosition ());
		}
		else
		{
			gameGui->getGameMap ().startActivateVehicle (unit, index);
		}
		storageWindow->close ();
	});
	storageWindow->reload.connect ([&](size_t index)
	{
		reloadTriggered (unit, *unit.storedUnits[index]);
	});
	storageWindow->repair.connect ([&](size_t index)
	{
		repairTriggered (unit, *unit.storedUnits[index]);
	});
	storageWindow->upgrade.connect ([&](size_t index)
	{
		upgradeTriggered (unit, index);
	});
	storageWindow->activateAll.connect ([&, storageWindow]()
	{
		const auto dynamicMap = getDynamicMap ();

		if (dynamicMap)
		{
			std::array<bool, 16> hasCheckedPlace;
			hasCheckedPlace.fill (false);

			for (size_t i = 0; i < unit.storedUnits.size (); ++i)
			{
				const auto& storedUnit = *unit.storedUnits[i];

				bool activated = false;
				for (int ypos = unit.getPosition ().y () - 1, poscount = 0; ypos <= unit.getPosition ().y () + (unit.data.isBig ? 2 : 1); ypos++)
				{
					if (ypos < 0 || ypos >= dynamicMap->getSize ().y ()) continue;
					for (int xpos = unit.getPosition ().x () - 1; xpos <= unit.getPosition ().x () + (unit.data.isBig ? 2 : 1); xpos++, poscount++)
					{
						if (hasCheckedPlace[poscount]) continue;

						if (xpos < 0 || xpos >= dynamicMap->getSize ().x ()) continue;

						if (((ypos == unit.getPosition ().y () && unit.data.factorAir == 0) || (ypos == unit.getPosition ().y () + 1 && unit.data.isBig)) &&
							((xpos == unit.getPosition ().x () && unit.data.factorAir == 0) || (xpos == unit.getPosition ().x () + 1 && unit.data.isBig))) continue;

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

		storageWindow->close ();
	});
	storageWindow->reloadAll.connect ([&, storageWindow]()
	{
		if (!unit.isABuilding ()) return;
		auto remainingResources = static_cast<const cBuilding&>(unit).SubBase->getMetal ();
		for (size_t i = 0; i < unit.storedUnits.size () && remainingResources > 0; ++i)
		{
			const auto& storedUnit = *unit.storedUnits[i];

			if (storedUnit.data.getAmmo () < storedUnit.data.ammoMax)
			{
				reloadTriggered (unit, storedUnit);
				remainingResources--;
			}
		}
	});
	storageWindow->repairAll.connect ([&, storageWindow]()
	{
		if (!unit.isABuilding ()) return;
		auto remainingResources = static_cast<const cBuilding&>(unit).SubBase->getMetal ();
		for (size_t i = 0; i < unit.storedUnits.size () && remainingResources > 0; ++i)
		{
			const auto& storedUnit = *unit.storedUnits[i];

			if (storedUnit.data.getHitpoints () < storedUnit.data.hitpointsMax)
			{
				repairTriggered (unit, storedUnit);
				auto value = storedUnit.data.getHitpoints ();
				while (value < storedUnit.data.hitpointsMax && remainingResources > 0)
				{
					value += Round (((float)storedUnit.data.hitpointsMax / storedUnit.data.buildCosts) * 4);
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
			selfDestroyDialog->close ();
		});
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::handleChatCommand (const std::string& command)
{
	if (command.empty ()) return;

	// Special commands start with a '/'
	if (command[0] == '/')
	{
		//
		// commands that control the GUI itself
		//
		if (command.compare ("/base client") == 0) { gameGui->getDebugOutput ().setDebugBaseClient (true);  gameGui->getDebugOutput ().setDebugBaseServer (false); }
		else if (command.compare ("/base server") == 0) { gameGui->getDebugOutput ().setDebugBaseServer (true);  gameGui->getDebugOutput ().setDebugBaseClient (false); }
		else if (command.compare ("/base off") == 0) { gameGui->getDebugOutput ().setDebugBaseServer (false);  gameGui->getDebugOutput ().setDebugBaseClient (false); }
		else if (command.compare ("/sentry server") == 0) { gameGui->getDebugOutput ().setDebugSentry (true); }
		else if (command.compare ("/sentry off") == 0) { gameGui->getDebugOutput ().setDebugSentry (false); }
		else if (command.compare ("/fx on") == 0) { gameGui->getDebugOutput ().setDebugFX (true); }
		else if (command.compare ("/fx off") == 0) { gameGui->getDebugOutput ().setDebugFX (false); }
		else if (command.compare ("/trace server") == 0) { gameGui->getDebugOutput ().setDebugTraceServer (true);  gameGui->getDebugOutput ().setDebugTraceClient (false); }
		else if (command.compare ("/trace client") == 0) { gameGui->getDebugOutput ().setDebugTraceClient (true);  gameGui->getDebugOutput ().setDebugTraceServer (false); }
		else if (command.compare ("/trace off") == 0) { gameGui->getDebugOutput ().setDebugTraceServer (false);  gameGui->getDebugOutput ().setDebugTraceClient (false); }
		else if (command.compare ("/ajobs on") == 0) { gameGui->getDebugOutput ().setDebugAjobs (true); }
		else if (command.compare ("/ajobs off") == 0) { gameGui->getDebugOutput ().setDebugAjobs (false); }
		else if (command.compare ("/players on") == 0) { gameGui->getDebugOutput ().setDebugPlayers (true); }
		else if (command.compare ("/players off") == 0) { gameGui->getDebugOutput ().setDebugPlayers (false); }
		else if (command.compare ("/singlestep") == 0) { cGameTimer::syncDebugSingleStep = !cGameTimer::syncDebugSingleStep; }
		else if (command.compare (0, 12, "/cache size ") == 0)
		{
		    int size = atoi (command.substr (12, command.length ()).c_str ());
		    // since atoi is too stupid to report an error,
		    // do an extra check, when the number is 0
		    if (size == 0 && command[12] != '0')
		    {
				gameGui->getGameMessageList ().addMessage ("Wrong parameter");
		        return;
		    }
		    gameGui->getGameMap().getDrawingCache().setMaxCacheSize (size);
		}
		else if (command.compare ("/cache flush") == 0)
		{
			gameGui->getGameMap ().getDrawingCache ().flush ();
		}
		else if (command.compare ("/cache debug on") == 0)
		{
			gameGui->getDebugOutput ().setDebugCache(true);
		}
		else if (command.compare ("/cache debug off") == 0)
		{
			gameGui->getDebugOutput ().setDebugCache(false);
		}
		else if (command.compare ("/sync debug on") == 0)
		{
			gameGui->getDebugOutput ().setDebugSync (true);
		}
		else if (command.compare ("/sync debug off") == 0)
		{
			gameGui->getDebugOutput ().setDebugSync (false);
		}

		//
		// Commands for client or server
		//
		else if (activeClient)
		{
			auto server = activeClient->getServer ();
			if (command.compare (0, 6, "/kick ") == 0)
			{
				if (!server)
				{
					gameGui->getGameMessageList().addMessage ("Command can only be used by Host");
					return;
				}
				if (command.length () <= 6)
				{
					gameGui->getGameMessageList().addMessage ("Wrong parameter");
					return;
				}
				const cPlayer* player = activeClient->getPlayerFromString (command.substr (6, command.length ()));

				// server can not be kicked
				if (!player)
				{
					gameGui->getGameMessageList().addMessage ("Wrong parameter");
					return;
				}

				sentWantKickPlayer (*activeClient, *player);
			}
			else if (command.compare (0, 9, "/credits ") == 0)
			{
				if (!server)
				{
					gameGui->getGameMessageList().addMessage ("Command can only be used by Host");
					return;
				}
				if (command.length () <= 9)
				{
					gameGui->getGameMessageList().addMessage ("Wrong parameter");
					return;
				}
				const auto playerStr = command.substr (9, command.find_first_of (" ", 9) - 9);
				const auto creditsStr = command.substr (command.find_first_of (" ", 9) + 1, command.length ());

				cPlayer* player = server->getPlayerFromString (playerStr);

				if (!player)
				{
					gameGui->getGameMessageList().addMessage ("Wrong parameter");
					return;
				}
				const int credits = atoi (creditsStr.c_str ());

				// FIXME: do not do changes on server data that are not synchronized with the server thread!
				player->setCredits(credits);

				sendCredits (*server, credits, *player);
			}
			else if (command.compare (0, 12, "/disconnect ") == 0)
			{
				if (!server)
				{
					gameGui->getGameMessageList().addMessage ("Command can only be used by Host");
					return;
				}
				if (command.length () <= 12)
				{
					gameGui->getGameMessageList().addMessage ("Wrong parameter");
					return;
				}
				cPlayer* player = server->getPlayerFromString (command.substr (12, command.length ()));

				// server cannot be disconnected
				// can not disconnect local players
				if (!player || player->getNr () == 0 || player->isLocal ())
				{
					gameGui->getGameMessageList().addMessage ("Wrong parameter");
					return;
				}

				auto message = std::make_unique<cNetMessage> (TCP_CLOSE);
				message->pushInt16 (player->getSocketNum ());
				server->pushEvent (std::move (message));
			}
			else if (command.compare (0, 9, "/deadline") == 0)
			{
				if (!server)
				{
					gameGui->getGameMessageList().addMessage ("Command can only be used by Host");
					return;
				}
				if (command.length () <= 9)
				{
					gameGui->getGameMessageList().addMessage ("Wrong parameter");
					return;
				}

				const int i = atoi (command.substr (9, command.length ()).c_str ());
				if (i == 0 && command[10] != '0')
				{
					gameGui->getGameMessageList().addMessage ("Wrong parameter");
					return;
				}

				// FIXME: do not do changes on server data that are not synchronized with the server thread!
				if (i >= 0)
				{
					server->setTurnEndDeadline (std::chrono::seconds (i));
					server->setTurnEndDeadlineActive (true);
				}
				else
				{
					server->setTurnEndDeadlineActive (false);
				}
				Log.write ("Deadline changed to " + iToStr (i), cLog::eLOG_TYPE_INFO);
			}
			else if (command.compare (0, 6, "/limit") == 0)
			{
				if (!server)
				{
					gameGui->getGameMessageList().addMessage ("Command can only be used by Host");
					return;
				}
				if (command.length () <= 6)
				{
					gameGui->getGameMessageList().addMessage ("Wrong parameter");
					return;
				}

				const int i = atoi (command.substr (6, command.length ()).c_str ());
				if (i == 0 && command[7] != '0')
				{
					gameGui->getGameMessageList().addMessage ("Wrong parameter");
					return;
				}

				// FIXME: do not do changes on server data that are not synchronized with the server thread!
				if (i > 0)
				{
					server->setTurnLimit (std::chrono::seconds (i));
					server->setTurnLimitActive (true);
				}
				else
				{
					server->setTurnLimitActive (false);
				}
				Log.write ("Limit changed to " + iToStr (i), cLog::eLOG_TYPE_INFO);
			}
			else if (command.compare (0, 7, "/resync") == 0)
			{
				if (command.length () > 7)
				{
					if (!server)
					{
						gameGui->getGameMessageList().addMessage ("Command can only be used by Host");
						return;
					}
					cPlayer* player = activeClient->getPlayerFromString (command.substr (7, 8));
					if (!player)
					{
						gameGui->getGameMessageList().addMessage ("Wrong parameter");
						return;
					}
					sendRequestResync (*activeClient, player->getNr ());
				}
				else
				{
					if (server)
					{
						const auto& playerList = server->playerList;
						for (unsigned int i = 0; i < playerList.size (); i++)
						{
							sendRequestResync (*activeClient, playerList[i]->getNr ());
						}
					}
					else
					{
						sendRequestResync (*activeClient, activeClient->getActivePlayer ().getNr ());
					}
				}
			}
			else if (command.compare (0, 5, "/mark") == 0)
			{
				std::string cmdArg (command);
				cmdArg.erase (0, 5);
				cNetMessage* message = new cNetMessage (GAME_EV_WANT_MARK_LOG);
				message->pushString (cmdArg);
				activeClient->sendNetMessage (message);
			}
			else if (command.compare (0, 7, "/color ") == 0)
			{
				int cl = 0;
				sscanf (command.c_str (), "/color %d", &cl);
				cl %= cPlayerColor::predefinedColorsCount;
				activeClient->getActivePlayer ().setColor (cPlayerColor (cPlayerColor::predefinedColors[cl]));
			}
			else if (command.compare (0, 8, "/fog off") == 0)
			{
				if (!server)
				{
					gameGui->getGameMessageList().addMessage ("Command can only be used by Host");
					return;
				}
				cPlayer* serverPlayer = 0;
				if (command.length () > 8)
				{
					serverPlayer = server->getPlayerFromString (command.substr (9));
				}
				else
				{
					serverPlayer = &server->getPlayerFromNumber (activeClient->getActivePlayer ().getNr ());
				}
				// FIXME: do not do changes on server data that are not synchronized with the server thread!
				if (serverPlayer)
				{
					serverPlayer->revealMap ();
					sendRevealMap (*server, *serverPlayer);
				}
			}
			else if (command.compare ("/survey") == 0)
			{
				if (!server)
				{
					gameGui->getGameMessageList().addMessage ("Command can only be used by Host");
					return;
				}
				activeClient->getMap ()->assignRessources (*server->Map);
				activeClient->getActivePlayer ().revealResource ();
			}
			else if (command.compare (0, 6, "/pause") == 0)
			{
				if (!server)
				{
					gameGui->getGameMessageList().addMessage ("Command can only be used by Host");
					return;
				}
				// FIXME: do not do changes on server data that are not synchronized with the server thread!
				server->enableFreezeMode (FREEZE_PAUSE);
			}
			else if (command.compare (0, 7, "/resume") == 0)
			{
				if (!server)
				{
					gameGui->getGameMessageList().addMessage ("Command can only be used by Host");
					return;
				}
				// FIXME: do not do changes on server data that are not synchronized with the server thread!
				server->disableFreezeMode (FREEZE_PAUSE);
			}
			if (server)
			{
				sendSavedReport (*server, cSavedReportHostCommand (command), nullptr);
			}
		}
	}
	// normal chat message
	else if (activeClient)
	{
		activeClient->handleChatMessage (command);
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::handleReport (const cSavedReport& report)
{
	if (report.getType () == eSavedReportType::Chat)
	{
		auto& savedChatReport = static_cast<const cSavedReportChat&>(report);
		auto player = gameGui->getChatBox ().getPlayerFromNumber (savedChatReport.getPlayerNumber ());

		if (player)
		{
			gameGui->getChatBox ().addChatMessage (*player, savedChatReport.getText ());
			cSoundDevice::getInstance ().getFreeSoundEffectChannel ().play (SoundData.SNDChat);
		}
		else // message from non in-game player (e.g. dedicated server)
		{
			gameGui->getChatBox ().addChatMessage (savedChatReport.getPlayerName (), savedChatReport.getText ());
		}
	}
	else if (report.hasPosition ())
	{
		savedReportPosition.first = true;
		savedReportPosition.second = report.getPosition ();

		gameGui->getGameMessageList().addMessage (report.getMessage () + " (" + KeysList.keyJumpToAction.toString () + ")", eGameMessageListViewItemBackgroundColor::LightGray);
	}
	else
	{
		gameGui->getGameMessageList().addMessage (report.getMessage (), report.isAlert () ? eGameMessageListViewItemBackgroundColor::Red : eGameMessageListViewItemBackgroundColor::DarkGray);
		if (report.isAlert ()) soundManager->playSound (std::make_shared<cSoundEffect> (eSoundEffectType::EffectAlert, SoundData.SNDQuitsch));
	}

	report.playSound (*soundManager);

	if (cSettings::getInstance ().isDebug ()) Log.write (report.getMessage (), cLog::eLOG_TYPE_DEBUG);
}

//------------------------------------------------------------------------------
void cGameGuiController::selectNextUnit ()
{
	const auto player = getActivePlayer ();
	if (!player) return;

	const auto nextUnit = player->getNextUnit (gameGui->getGameMap().getUnitSelection ().getSelectedUnit ());
	if (nextUnit)
	{
		gameGui->getGameMap ().getUnitSelection ().selectUnit (*nextUnit);
		gameGui->getGameMap ().centerAt (nextUnit->getPosition ());
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::selectPreviousUnit ()
{
	const auto player = getActivePlayer ();
	if (!player) return;

	const auto prevUnit = player->getPrevUnit (gameGui->getGameMap ().getUnitSelection ().getSelectedUnit ());
	if (prevUnit)
	{
		gameGui->getGameMap ().getUnitSelection ().selectUnit (*prevUnit);
		gameGui->getGameMap ().centerAt (prevUnit->getPosition ());
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::markSelectedUnitAsDone ()
{
	const auto player = getActivePlayer ();
	if (!player) return;

	const auto unit = gameGui->getGameMap ().getUnitSelection ().getSelectedUnit ();

	if (unit && unit->getOwner () == player.get ())
	{
		gameGui->getGameMap ().centerAt (unit->getPosition ());
		unit->setMarkedAsDone (true);
		resumeMoveJobTriggered (*unit);
	}
}

//------------------------------------------------------------------------------
void cGameGuiController::centerSelectedUnit ()
{
	const auto player = getActivePlayer ();
	const auto selectedUnit = gameGui->getGameMap ().getUnitSelection ().getSelectedUnit ();
	if (selectedUnit) gameGui->getGameMap ().centerAt (selectedUnit->getPosition ());
}

//------------------------------------------------------------------------------
void cGameGuiController::savePosition (size_t index)
{
	if (index > savedPositions.size ()) return;

	savedPositions[index] = std::make_pair (true, gameGui->getGameMap ().getMapCenterOffset ());
}

//------------------------------------------------------------------------------
void cGameGuiController::jumpToSavedPosition (size_t index)
{
	if (index > savedPositions.size ()) return;

	if (!savedPositions[index].first) return;

	gameGui->getGameMap ().centerAt (savedPositions[index].second);
}

//------------------------------------------------------------------------------
std::vector<std::shared_ptr<const cPlayer>> cGameGuiController::getPlayers () const
{
	std::vector<std::shared_ptr<const cPlayer>> result;

	if (!activeClient) return result;

	const auto& clientPlayerList = activeClient->getPlayerList ();

	result.resize (clientPlayerList.size ());
	std::copy (clientPlayerList.begin (), clientPlayerList.end (), result.begin ());

	return result;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cPlayer> cGameGuiController::getActivePlayer () const
{
	if (!activeClient) return nullptr;

	const auto& clientPlayerList = activeClient->getPlayerList ();

	auto iter = std::find_if (clientPlayerList.begin (), clientPlayerList.end (), [this](const std::shared_ptr<cPlayer>& player) { return player->getNr () == activeClient->getActivePlayer ().getNr (); });

	if (iter == clientPlayerList.end ()) return nullptr; // should never happen; just to be on the safe side

	return *iter;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cTurnClock> cGameGuiController::getTurnClock () const
{
	return activeClient ? activeClient->getTurnClock () : nullptr;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cTurnTimeClock> cGameGuiController::getTurnTimeClock () const
{
	return activeClient ? activeClient->getTurnTimeClock () : nullptr;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cGameSettings> cGameGuiController::getGameSettings () const
{
	return activeClient ? activeClient->getGameSettings () : nullptr;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cCasualtiesTracker> cGameGuiController::getCasualtiesTracker () const
{
	return activeClient ? activeClient->getCasualtiesTracker () : nullptr;
}

//------------------------------------------------------------------------------
std::shared_ptr<const cMap> cGameGuiController::getDynamicMap () const
{
	return activeClient ? activeClient->getMap () : nullptr;
}
