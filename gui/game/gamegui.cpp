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

#include <sstream>
#include <iomanip>
#include <iostream>

#include "gamegui.h"
#include "hud.h"
#include "widgets/gamemapwidget.h"
#include "widgets/minimapwidget.h"
#include "widgets/unitcontextmenuwidget.h"
#include "widgets/gamemessagelistview.h"
#include "widgets/hudpanels.h"
#include "widgets/chatbox.h"
#include "../menu/widgets/label.h"

#include "temp/animationtimer.h"

#include "../application.h"
#include "../menu/dialogs/dialogok.h"
#include "../menu/dialogs/dialogyesno.h"
#include "../menu/dialogs/dialogpreferences.h"
#include "../menu/dialogs/dialogtransfer.h"
#include "../menu/dialogs/dialogresearch.h"
#include "../menu/dialogs/dialogselfdestruction.h"
#include "../menu/windows/windowunitinfo/windowunitinfo.h"
#include "../menu/windows/windowbuildbuildings/windowbuildbuildings.h"
#include "../menu/windows/windowbuildvehicles/windowbuildvehicles.h"
#include "../menu/windows/windowstorage/windowstorage.h"
#include "../menu/windows/windowresourcedistribution/windowresourcedistribution.h"
#include "../menu/windows/windowupgrades/windowupgrades.h"
#include "../menu/windows/windowreports/windowreports.h"
#include "../menu/windows/windowloadsave/windowloadsave.h"
#include "../menu/windows/windowload/savegamedata.h"

#include "../../keys.h"
#include "../../player.h"
#include "../../map.h"
#include "../../vehicles.h"
#include "../../buildings.h"
#include "../../sound.h"
#include "../../client.h"
#include "../../server.h"
#include "../../clientevents.h"
#include "../../attackjobs.h"
#include "../../log.h"
#include "../../netmessage.h"
#include "../../game/game.h"
#include "../../input/mouse/mouse.h"
#include "../../input/mouse/cursor/mousecursorsimple.h"
#include "../../game/data/report/savedreportsimple.h"
#include "../../game/data/report/savedreportchat.h"
#include "../../game/data/report/savedreportunit.h"
#include "../../game/logic/turnclock.h"

//------------------------------------------------------------------------------
cGameGui::cGameGui (std::shared_ptr<const cStaticMap> staticMap_) :
	cWindow (nullptr),
	animationTimer (std::make_shared<cAnimationTimer>()),
	staticMap (std::move (staticMap_)),
	dynamicMap (nullptr),
	mouseScrollDirection (0, 0),
	selectedUnitSoundStream (-1),
	openPanelOnActivation (true),
	savedReportPosition (false, cPosition ())
{
	auto hudOwning = std::make_unique<cHud> (animationTimer);

	resize (hudOwning->getSize ());

	gameMap = addChild (std::make_unique<cGameMapWidget> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, cHud::panelTopHeight), getEndPosition () - cPosition (cHud::panelRightWidth, cHud::panelBottomHeight)), staticMap, animationTimer));

	messageList = addChild (std::make_unique<cGameMessageListView> (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, cHud::panelTopHeight + 7), cPosition (getEndPosition ().x () - cHud::panelRightWidth - 4, cHud::panelTopHeight + 200))));

	hud = addChild (std::move (hudOwning));

	hud->setMinimalZoomFactor (gameMap->computeMinimalZoomFactor ());

    chatBox = addChild (std::make_unique<cChatBox> (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, getEndPosition ().y () - cHud::panelBottomHeight - 12 - 100), getEndPosition () - cPosition (cHud::panelRightWidth + 4, cHud::panelBottomHeight + 12))));

	miniMap = addChild (std::make_unique<cMiniMapWidget> (cBox<cPosition> (cPosition (15, 356), cPosition (15 + 112, 356 + 112)), staticMap));

	hudPanels = addChild (std::make_unique<cHudPanels> (getPosition (), getSize ().y (), animationTimer));

	primiaryInfoLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, 235), cPosition (getEndPosition ().x () - cHud::panelRightWidth, 235 + font->getFontHeight (FONT_LATIN_BIG))), "", FONT_LATIN_BIG, toEnumFlag (eAlignmentType::CenterHorizontal)  | eAlignmentType::Top));
	primiaryInfoLabel->disable ();
	primiaryInfoLabel->hide ();

	additionalInfoLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, 235 + font->getFontHeight (FONT_LATIN_BIG)), cPosition (getEndPosition ().x () - cHud::panelRightWidth, 235 + font->getFontHeight (FONT_LATIN_BIG) + font->getFontHeight (FONT_LATIN_NORMAL))), "", FONT_LATIN_NORMAL, toEnumFlag (eAlignmentType::CenterHorizontal)  | eAlignmentType::Top));
	additionalInfoLabel->disable ();
	additionalInfoLabel->hide ();

	signalConnectionManager.connect (hudPanels->opened, [&]()
	{
		hudPanels->disable ();
		hudPanels->hide ();
	});

	using namespace std::placeholders;

	signalConnectionManager.connect (chatBox->commandEntered, std::bind (&cGameGui::handleChatCommand, this, _1));

	signalConnectionManager.connect (hud->preferencesClicked, std::bind (&cGameGui::showPreferencesDialog, this));
	signalConnectionManager.connect (hud->filesClicked, std::bind (&cGameGui::showFilesWindow, this));

	signalConnectionManager.connect (hud->zoomChanged, [&](){ gameMap->setZoomFactor (hud->getZoomFactor (), true); });

	signalConnectionManager.connect (hud->surveyToggled, [&](){ gameMap->setDrawSurvey (hud->getSurveyActive ()); });
	signalConnectionManager.connect (hud->hitsToggled, [&](){ gameMap->setDrawHits (hud->getHitsActive ()); });
	signalConnectionManager.connect (hud->scanToggled, [&](){ gameMap->setDrawScan (hud->getScanActive ()); });
	signalConnectionManager.connect (hud->statusToggled, [&](){ gameMap->setDrawStatus (hud->getStatusActive ()); });
	signalConnectionManager.connect (hud->ammoToggled, [&](){ gameMap->setDrawAmmo (hud->getAmmoActive ()); });
	signalConnectionManager.connect (hud->gridToggled, [&](){ gameMap->setDrawGrid (hud->getGridActive ()); });
	signalConnectionManager.connect (hud->colorToggled, [&](){ gameMap->setDrawColor (hud->getColorActive ()); });
	signalConnectionManager.connect (hud->rangeToggled, [&](){ gameMap->setDrawRange (hud->getRangeActive ()); });
	signalConnectionManager.connect (hud->fogToggled, [&](){ gameMap->setDrawFog (hud->getFogActive ()); });

	signalConnectionManager.connect (hud->helpClicked, [&](){ gameMap->toggleHelpMode (); });
	signalConnectionManager.connect (hud->centerClicked, [&]()
	{
		const auto selectedUnit = gameMap->getUnitSelection ().getSelectedUnit ();
		if (selectedUnit) gameMap->centerAt (selectedUnit->getPosition());
	});

    signalConnectionManager.connect (hud->reportsClicked, std::bind (&cGameGui::showReportsWindow, this));
	signalConnectionManager.connect (hud->chatClicked, std::bind (&cGameGui::toggleChatBox, this));

	signalConnectionManager.connect (hud->miniMapZoomFactorToggled, [&](){ miniMap->setZoomFactor (hud->getMiniMapZoomFactorActive () ? 2 : 1); });
	signalConnectionManager.connect (hud->miniMapAttackUnitsOnlyToggled, [&](){ miniMap->setAttackUnitsUnly (hud->getMiniMapAttackUnitsOnly ()); });

	signalConnectionManager.connect (gameMap->scrolled, std::bind(&cGameGui::resetMiniMapViewWindow, this));
	signalConnectionManager.connect (gameMap->zoomFactorChanged, std::bind (&cGameGui::resetMiniMapViewWindow, this));
	signalConnectionManager.connect (gameMap->tileUnderMouseChanged, std::bind (&cGameGui::updateHudCoordinates, this, _1));
	signalConnectionManager.connect (gameMap->tileUnderMouseChanged, std::bind (&cGameGui::updateHudUnitName, this, _1));

	signalConnectionManager.connect (gameMap->getUnitSelection ().mainSelectionChanged, [&](){ hud->setActiveUnit (gameMap->getUnitSelection ().getSelectedUnit ()); });
	signalConnectionManager.connect (gameMap->getUnitSelection ().mainSelectionChanged, std::bind (&cGameGui::updateSelectedUnitIdleSound, this));

	signalConnectionManager.connect (gameMap->triggeredUnitHelp, std::bind (&cGameGui::showUnitHelpWindow, this, _1));
	signalConnectionManager.connect (gameMap->triggeredTransfer, std::bind (&cGameGui::showUnitTransferDialog, this, _1, _2));
	signalConnectionManager.connect (gameMap->triggeredBuild, [&](const cUnit& unit)
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
	signalConnectionManager.connect (gameMap->triggeredResourceDistribution, std::bind (&cGameGui::showResourceDistributionDialog, this, _1));
	signalConnectionManager.connect (gameMap->triggeredResearchMenu, std::bind (&cGameGui::showResearchDialog, this, _1));
	signalConnectionManager.connect (gameMap->triggeredUpgradesMenu, std::bind (&cGameGui::showUpgradesWindow, this, _1));
	signalConnectionManager.connect (gameMap->triggeredActivate, std::bind (&cGameGui::showStorageWindow, this, _1));
	signalConnectionManager.connect (gameMap->triggeredSelfDestruction, std::bind (&cGameGui::showSelfDestroyDialog, this, _1));

	signalConnectionManager.connect (miniMap->focus, [&](const cPosition& position){ gameMap->centerAt (position); });

	signalConnectionManager.connect (animationTimer->triggered10ms, [&]()
	{
		if (mouseScrollDirection != cPosition (0, 0))
		{
			gameMap->scroll (mouseScrollDirection);
		}
	});
	signalConnectionManager.connect (animationTimer->triggered400ms, [&]()
	{
		messageList->removeOldMessages ();
	});

	terminated.connect ([&]()
	{
		stopSelectedUnitSound ();
	});

	initShortcuts ();

	signalConnectionManager.connect (Video.resolutionChanged, std::bind (&cGameGui::handleResolutionChange, this));

	signalConnectionManager.connect (Video.screenShotTaken, [this](const std::string& path)
	{
		messageList->addMessage (lngPack.i18n ("Text~Comp~Screenshot_Done", path));
	});
}

//------------------------------------------------------------------------------
void cGameGui::updateHudCoordinates (const cPosition& tilePosition)
{
	std::stringstream coordsString;
	coordsString << std::setw (3) << std::setfill ('0') << tilePosition.x () << "-" << std::setw (3) << std::setfill ('0') << tilePosition.y ();
	hud->setCoordinatesText (coordsString.str ());
}

//------------------------------------------------------------------------------
void cGameGui::updateHudUnitName (const cPosition& tilePosition)
{
	std::string unitNameString;
	if (dynamicMap && (!player || player->canSeeAt (tilePosition)))
	{
		const auto field = dynamicMap->getField (tilePosition);

		cUnit* unit = nullptr;
		if (field.getVehicle () != nullptr) unit = field.getVehicle ();
		else if (field.getPlane () != nullptr) unit = field.getPlane ();
		else if (field.getTopBuilding () != nullptr) unit = field.getTopBuilding ();
		else if (field.getBaseBuilding () != nullptr && field.getBaseBuilding ()->owner) unit = field.getBaseBuilding ();

		if (unit != nullptr)
		{
			// FIXME: string may be to long.
			unitNameString = unit->getDisplayName () + " (" + unit->owner->getName () + ")";
		}
	}
	hud->setUnitNameText (unitNameString);
}

//------------------------------------------------------------------------------
void cGameGui::setDynamicMap (std::shared_ptr<const cMap> dynamicMap_)
{
	dynamicMap = std::move(dynamicMap_);
	gameMap->setDynamicMap (dynamicMap);
	miniMap->setDynamicMap (dynamicMap);

	dynamicMapSignalConnectionManager.disconnectAll ();

	if (dynamicMap != nullptr)
	{
		dynamicMapSignalConnectionManager.connect (dynamicMap->addedUnit, [&](const cUnit& unit)
		{
			if (unit.data.ID == UnitsData.specialIDLandMine) PlayFX (SoundData.SNDLandMinePlace.get ());
			else if (unit.data.ID == UnitsData.specialIDSeaMine) PlayFX (SoundData.SNDSeaMinePlace.get ());
		});
		dynamicMapSignalConnectionManager.connect (dynamicMap->removedUnit, [&](const cUnit& unit)
		{
			if (unit.data.ID == UnitsData.specialIDLandMine) PlayFX (SoundData.SNDLandMineClear.get ());
			else if (unit.data.ID == UnitsData.specialIDSeaMine) PlayFX (SoundData.SNDSeaMineClear.get ());
		});
		//dynamicMapSignalConnectionManager.connect (dynamicMap->movedVehicle, [&](const cVehicle&){ });
	}
}

//------------------------------------------------------------------------------
void cGameGui::setPlayer (std::shared_ptr<const cPlayer> player_)
{
	player = std::move(player_);
	gameMap->setPlayer (player);
	miniMap->setPlayer (player);
	hud->setPlayer (player);

	playerSignalConnectionManager.disconnectAll ();

	if (player != nullptr)
	{
		using namespace std::placeholders;
		playerSignalConnectionManager.connect (player->reportAdded, std::bind(&cGameGui::handleReport, this, _1));
	}
}

//------------------------------------------------------------------------------
void cGameGui::setPlayers (std::vector<std::shared_ptr<const cPlayer>> players_)
{
	players = std::move (players_);

	chatBox->clearPlayers ();
	for (size_t i = 0; i < players.size (); ++i)
	{
		chatBox->addPlayer (*players[i]);
	}
}

//------------------------------------------------------------------------------
void cGameGui::setCasualtiesTracker (std::shared_ptr<const cCasualtiesTracker> casualtiesTracker_)
{
	casualtiesTracker = std::move (casualtiesTracker_);
}

//------------------------------------------------------------------------------
void cGameGui::setTurnClock (std::shared_ptr<const cTurnClock> turnClock_)
{
	turnClock = std::move (turnClock_);
	hud->setTurnClock (turnClock);
}

//------------------------------------------------------------------------------
void cGameGui::setGameSettings (std::shared_ptr<const cGameSettings> gameSettings_)
{
	gameSettings = std::move (gameSettings_);
}

//------------------------------------------------------------------------------
void cGameGui::connectToClient (cClient& client)
{
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
		sendWantBuild (client, vehicle.iID, unitId, buildSpeed, destination, false, cPosition(0, 0));
		buildPositionSelectionConnectionManager.disconnectAll ();
	});
	clientSignalConnectionManager.connect (buildBuildingPathTriggered, [&](const cVehicle& vehicle, const cPosition& destination, const sID& unitId, int buildSpeed)
	{
		sendWantBuild (client, vehicle.iID, unitId, buildSpeed, vehicle.getPosition(), true, destination);
		buildPositionSelectionConnectionManager.disconnectAll ();
	});
	clientSignalConnectionManager.connect (buildVehiclesTriggered, [&](const cBuilding& building, const std::vector<sBuildList>& buildList, int buildSpeed, bool repeat)
	{
		sendWantBuildList (client, building, buildList, repeat, buildSpeed);
	});
	clientSignalConnectionManager.connect (activateAtTriggered, [&](const cUnit& unit, size_t index, const cPosition& position)
	{
		sendWantActivate (client, unit.iID, unit.isAVehicle (), unit.storedUnits[index]->iID, position.x (), position.y ());
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
		if (!player) return;
		sendWantResearchChange (client, newResearchSettings, player->getNr());
	});
	clientSignalConnectionManager.connect (takeUnitUpgradesTriggered, [&](const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades)
	{
		if (!player) return;
		sendTakenUpgrades (client, unitUpgrades);
	});
	clientSignalConnectionManager.connect (chatCommandTriggered, [&](const std::string& command)
	{
		// TODO: where should this code go?!
		if (command[0] == '/')
		{
			auto server = client.getServer ();
			if (command.compare (0, 6, "/kick ") == 0)
			{
				if (!server)
				{
					messageList->addMessage ("Command can only be used by Host");
					return;
				}
				if (command.length () <= 6)
				{
					messageList->addMessage ("Wrong parameter");
					return;
				}
				cPlayer* player = server->getPlayerFromString (command.substr (6, command.length ()));

				// server can not be kicked
				if (!player || player->getNr () == 0)
				{
					messageList->addMessage ("Wrong parameter");
					return;
				}

				server->kickPlayer (player);
			}
			else if (command.compare (0, 9, "/credits ") == 0)
			{
				if (!server)
				{
					messageList->addMessage ("Command can only be used by Host");
					return;
				}
				if (command.length () <= 9)
				{
					messageList->addMessage ("Wrong parameter");
					return;
				}
				const auto playerStr = command.substr (9, command.find_first_of (" ", 9) - 9);
				const auto creditsStr = command.substr (command.find_first_of (" ", 9) + 1, command.length ());

				cPlayer* player = server->getPlayerFromString (playerStr);

				if (!player)
				{
					messageList->addMessage ("Wrong parameter");
					return;
				}
				const int credits = atoi (creditsStr.c_str ());

				player->Credits = credits;

				sendCredits (*server, credits, *player);
			}
			else if (command.compare (0, 12, "/disconnect ") == 0)
			{
				if (!server)
				{
					messageList->addMessage ("Command can only be used by Host");
					return;
				}
				if (command.length () <= 12)
				{
					messageList->addMessage ("Wrong parameter");
					return;
				}
				cPlayer* player = server->getPlayerFromString (command.substr (12, command.length ()));

				// server cannot be disconnected
				// can not disconnect local players
				if (!player || player->getNr () == 0 || player->isLocal ())
				{
					messageList->addMessage ("Wrong parameter");
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
					messageList->addMessage ("Command can only be used by Host");
					return;
				}
				if (command.length () <= 9)
				{
					messageList->addMessage ("Wrong parameter");
					return;
				}

				const int i = atoi (command.substr (9, command.length ()).c_str ());
				if (i == 0 && command[10] != '0')
				{
					messageList->addMessage ("Wrong parameter");
					return;
				}

				server->setDeadline (i);
				Log.write ("Deadline changed to " + iToStr (i), cLog::eLOG_TYPE_INFO);
			}
			else if (command.compare (0, 7, "/resync") == 0)
			{
				if (command.length () > 7)
				{
					if (!server)
					{
						messageList->addMessage ("Command can only be used by Host");
						return;
					}
					cPlayer* player = client.getPlayerFromString (command.substr (7, 8));
					if (!player)
					{
						messageList->addMessage ("Wrong parameter");
						return;
					}
					sendRequestResync (client, player->getNr ());
				}
				else
				{
					if (server)
					{
						const auto& playerList = server->playerList;
						for (unsigned int i = 0; i < playerList.size (); i++)
						{
							sendRequestResync (client, playerList[i]->getNr ());
						}
					}
					else
					{
						sendRequestResync (client, client.getActivePlayer ().getNr ());
					}
				}
			}
			else if (command.compare (0, 5, "/mark") == 0)
			{
				std::string cmdArg (command);
				cmdArg.erase (0, 5);
				cNetMessage* message = new cNetMessage (GAME_EV_WANT_MARK_LOG);
				message->pushString (cmdArg);
				client.sendNetMessage (message);
			}
			else if (command.compare (0, 7, "/color ") == 0)
			{
				int cl = 0;
				sscanf (command.c_str (), "/color %d", &cl);
				cl %= 8;
				client.getActivePlayer ().setColor (cPlayerColor(cl));
			}
			else if (command.compare (0, 8, "/fog off") == 0)
			{
				if (!server)
				{
					messageList->addMessage ("Command can only be used by Host");
					return;
				}
				cPlayer* serverPlayer = 0;
				if (command.length () > 8)
				{
					serverPlayer = server->getPlayerFromString (command.substr (9));
				}
				else
				{
					serverPlayer = &server->getPlayerFromNumber (client.getActivePlayer ().getNr ());
				}
				if (serverPlayer)
				{
					sendSavedReport (*server, cSavedReportSimple ("Server entered command: '" + command + "'"), nullptr);
					serverPlayer->revealMap ();
					sendRevealMap (*server, *serverPlayer);
				}
			}
			else if (command.compare ("/survey") == 0)
			{
				if (!server)
				{
					messageList->addMessage ("Command can only be used by Host");
					return;
				}
				client.getMap ()->assignRessources (*server->Map);
				client.getActivePlayer ().revealResource ();
			}
			else if (command.compare (0, 6, "/pause") == 0)
			{
				if (!server)
				{
					messageList->addMessage ("Command can only be used by Host");
					return;
				}
				server->enableFreezeMode (FREEZE_PAUSE);
			}
			else if (command.compare (0, 7, "/resume") == 0)
			{
				if (!server)
				{
					messageList->addMessage ("Command can only be used by Host");
					return;
				}
				server->disableFreezeMode (FREEZE_PAUSE);
			}
		}
		else
		{
			client.handleChatMessage (command);
		}
	});
	clientSignalConnectionManager.connect (selfDestructionTriggered, [&](const cUnit& unit)
	{
		if (!unit.data.ID.isABuilding ()) return;
		sendWantSelfDestroy (client, static_cast<const cBuilding&>(unit));
	});
	clientSignalConnectionManager.connect (hud->endClicked, [&]()
	{
		client.handleEnd ();
	});
	clientSignalConnectionManager.connect (hud->triggeredRenameUnit, [&](const cUnit& unit, const std::string& name)
	{
		sendWantChangeUnitName (client, name, unit.iID);
	});
	clientSignalConnectionManager.connect (gameMap->triggeredStartWork, [&](const cUnit& unit)
	{
		sendWantStartWork (client, unit);
	});
	clientSignalConnectionManager.connect (gameMap->triggeredStopWork, [&](const cUnit& unit)
	{
		unit.executeStopCommand (client);
	});
	clientSignalConnectionManager.connect (gameMap->triggeredAutoMoveJob, [&](const cUnit& unit)
	{
		if (unit.data.ID.isAVehicle ())
		{
			auto vehicle = client.getVehicleFromID (unit.iID);
			if (!vehicle) return;
			vehicle->executeAutoMoveJobCommand (client);
		}
	});
	clientSignalConnectionManager.connect (gameMap->triggeredStartClear, [&](const cUnit& unit)
	{
		if (unit.data.ID.isAVehicle ()) sendWantStartClear (client, static_cast<const cVehicle&> (unit));
	});
	clientSignalConnectionManager.connect (gameMap->triggeredManualFire, [&](const cUnit& unit)
	{
		sendChangeManualFireStatus (client, unit.iID, unit.isAVehicle ());
	});
	clientSignalConnectionManager.connect (gameMap->triggeredSentry, [&](const cUnit& unit)
	{
		sendChangeSentry (client, unit.iID, unit.isAVehicle ());
	});
	clientSignalConnectionManager.connect (gameMap->triggeredUpgradeThis, [&](const cUnit& unit)
	{
		if (unit.data.ID.isABuilding ()) static_cast<const cBuilding&>(unit).executeUpdateBuildingCommmand (client, false);
	});
	clientSignalConnectionManager.connect (gameMap->triggeredUpgradeAll, [&](const cUnit& unit)
	{
		if (unit.data.ID.isABuilding ()) static_cast<const cBuilding&>(unit).executeUpdateBuildingCommmand (client, true);
	});
	clientSignalConnectionManager.connect (gameMap->triggeredLayMines, [&](const cUnit& unit)
	{
		if (unit.data.ID.isAVehicle ())
		{
			auto vehicle = client.getVehicleFromID (unit.iID);
			if (!vehicle) return;
			vehicle->executeLayMinesCommand (client);
		}
	});
	clientSignalConnectionManager.connect (gameMap->triggeredCollectMines, [&](const cUnit& unit)
	{
		if (unit.data.ID.isAVehicle ())
		{
			auto vehicle = client.getVehicleFromID (unit.iID);
			if (!vehicle) return;
			vehicle->executeClearMinesCommand (client);
		}
	});
	clientSignalConnectionManager.connect (gameMap->triggeredUnitDone, [&](const cUnit& unit)
	{
		if (unit.owner == player.get())
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
		}
	});

	clientSignalConnectionManager.connect (gameMap->triggeredEndBuilding, [&](cVehicle& vehicle, const cPosition& destination)
	{
		sendWantEndBuilding (client, vehicle, destination.x (), destination.y ());
	});
	clientSignalConnectionManager.connect (gameMap->triggeredMoveSingle, [&](cVehicle& vehicle, const cPosition& destination)
	{
		client.addMoveJob (vehicle, destination);
	});
	clientSignalConnectionManager.connect (gameMap->triggeredMoveGroup, [&](const std::vector<cVehicle*>& vehicles, const cPosition& destination)
	{
		client.startGroupMove (vehicles, destination);
	});
	clientSignalConnectionManager.connect (gameMap->triggeredActivateAt, [&](const cUnit& unit, size_t index, const cPosition& position)
	{
		sendWantActivate (client, unit.iID, unit.isAVehicle (), unit.storedUnits[index]->iID, position.x (), position.y ());
	});
	clientSignalConnectionManager.connect (gameMap->triggeredExitFinishedUnit, [&](const cBuilding& building, const cPosition& position)
	{
		sendWantExitFinishedVehicle (client, building, position);
	});
	clientSignalConnectionManager.connect (gameMap->triggeredLoadAt, [&](const cUnit& unit, const cPosition& position)
	{
		const auto field = client.getMap ()->getField (position);
		auto overVehicle = field.getVehicle ();
		auto overPlane = field.getPlane ();
		if (unit.isAVehicle ())
		{
			const auto& vehicle = static_cast<const cVehicle&>(unit);
			if (vehicle.data.factorAir > 0 && overVehicle)
			{
				if (overVehicle->getPosition() == vehicle.getPosition()) sendWantLoad (client, vehicle.iID, true, overVehicle->iID);
				else
				{
					cPathCalculator pc (vehicle.getPosition(), overVehicle->getPosition(), *client.getMap (), vehicle);
					sWaypoint* path = pc.calcPath ();
					if (path)
					{
						sendMoveJob (client, path, vehicle.iID);
						sendEndMoveAction (client, vehicle.iID, overVehicle->iID, EMAT_LOAD);
					}
					else
					{
						PlayRandomVoice (VoiceData.VOINoPath);
					}
				}
			}
			else if (overVehicle)
			{
				if (vehicle.isNextTo (overVehicle->getPosition())) sendWantLoad (client, vehicle.iID, true, overVehicle->iID);
				else
				{
					cPathCalculator pc (overVehicle->getPosition(), vehicle, *client.getMap (), *overVehicle, true);
					sWaypoint* path = pc.calcPath ();
					if (path)
					{
						sendMoveJob (client, path, overVehicle->iID);
						sendEndMoveAction (client, overVehicle->iID, vehicle.iID, EMAT_GET_IN);
					}
					else
					{
						PlayRandomVoice (VoiceData.VOINoPath);
					}
				}
			}
		}
		else if (unit.isABuilding ())
		{
			const auto& building = static_cast<const cBuilding&>(unit);
			if (overVehicle && building.canLoad (overVehicle, false))
			{
				if (building.isNextTo (overVehicle->getPosition())) sendWantLoad (client, building.iID, false, overVehicle->iID);
				else
				{
					cPathCalculator pc (overVehicle->getPosition(), building, *client.getMap (), *overVehicle, true);
					sWaypoint* path = pc.calcPath ();
					if (path)
					{
						sendMoveJob (client, path, overVehicle->iID);
						sendEndMoveAction (client, overVehicle->iID, building.iID, EMAT_GET_IN);
					}
					else
					{
						PlayRandomVoice (VoiceData.VOINoPath);
					}
				}
			}
			else if (overPlane && building.canLoad (overPlane, false))
			{
				if (building.isNextTo (overPlane->getPosition())) sendWantLoad (client, building.iID, false, overPlane->iID);
				else
				{
					cPathCalculator pc (overPlane->getPosition(), building, *client.getMap (), *overPlane, true);
					sWaypoint* path = pc.calcPath ();
					if (path)
					{
						sendMoveJob (client, path, overPlane->iID);
						sendEndMoveAction (client, overPlane->iID, building.iID, EMAT_GET_IN);
					}
					else
					{
						PlayRandomVoice (VoiceData.VOINoPath);
					}
				}
			}
		}
	});
	clientSignalConnectionManager.connect (gameMap->triggeredSupplyAmmo, [&](const cUnit& sourceUnit, const cUnit& destinationUnit)
	{
		sendWantSupply (client, destinationUnit.iID, destinationUnit.isAVehicle (), sourceUnit.iID, sourceUnit.isAVehicle (), SUPPLY_TYPE_REARM);
	});
	clientSignalConnectionManager.connect (gameMap->triggeredRepair, [&](const cUnit& sourceUnit, const cUnit& destinationUnit)
	{
		sendWantSupply (client, destinationUnit.iID, destinationUnit.isAVehicle (), sourceUnit.iID, sourceUnit.isAVehicle (), SUPPLY_TYPE_REPAIR);
	});
	clientSignalConnectionManager.connect (gameMap->triggeredAttack, [&](const cUnit& unit, const cPosition& position)
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
				cPathCalculator pc (vehicle.getPosition(), *client.getMap (), vehicle, position);
				sWaypoint* path = pc.calcPath ();
				if (path)
				{
					sendMoveJob (client, path, vehicle.iID);
					sendEndMoveAction (client, vehicle.iID, target->iID, EMAT_ATTACK);
				}
				else
				{
					PlayRandomVoice (VoiceData.VOINoPath);
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
	clientSignalConnectionManager.connect (gameMap->triggeredSteal, [&](const cUnit& sourceUnit, const cUnit& destinationUnit)
	{
		sendWantComAction (client, sourceUnit.iID, destinationUnit.iID, destinationUnit.isAVehicle(), true);
	});
	clientSignalConnectionManager.connect (gameMap->triggeredDisable, [&](const cUnit& sourceUnit, const cUnit& destinationUnit)
	{
		sendWantComAction (client, sourceUnit.iID, destinationUnit.iID, destinationUnit.isAVehicle (), false);
	});


	//
	// client to GUI (reaction)
	//
	clientSignalConnectionManager.connect (client.startedTurnEndProcess, [&]()
	{
		hud->lockEndButton ();
	});

	clientSignalConnectionManager.connect (client.finishedTurnEndProcess, [&]()
	{
		hud->unlockEndButton ();
	});

	clientSignalConnectionManager.connect (client.freezeModeChanged, [&](eFreezeMode mode)
	{
		const int playerNumber = client.getFreezeInfoPlayerNumber ();
		const cPlayer* player = client.getPlayerFromNumber (playerNumber);

		if (mode == FREEZE_WAIT_FOR_OTHERS)
		{
			if (client.getFreezeMode (FREEZE_WAIT_FOR_OTHERS)) hud->lockEndButton ();
			else hud->unlockEndButton ();
		}

		if (client.getFreezeMode (FREEZE_WAIT_FOR_OTHERS))
		{
			// TODO: Fix message
			const std::string& name = player ? player->getName () : "other players";
			setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_Until", name), "");
		}
		else if (client.getFreezeMode (FREEZE_PAUSE))
		{
			setInfoTexts (lngPack.i18n ("Text~Multiplayer~Pause"), "");
		}
		else if (client.getFreezeMode (FREEZE_WAIT_FOR_SERVER))
		{
			setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_For_Server"), "");
		}
		else if (client.getFreezeMode (FREEZE_WAIT_FOR_RECONNECT))
		{
			std::string s = client.getServer () ? lngPack.i18n ("Text~Multiplayer~Abort_Waiting") : "";
			setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_Reconnect"), s);
		}
		else if (client.getFreezeMode (FREEZE_WAIT_FOR_PLAYER))
		{
			setInfoTexts (lngPack.i18n ("Text~Multiplayer~No_Response", player->getName ()), "");
		}
		else if (client.getFreezeMode (FREEZE_WAIT_FOR_TURNEND))
		{
			setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_TurnEnd"), "");
		}
		else
		{
			setInfoTexts ("", "");
		}
	});

	clientSignalConnectionManager.connect (client.unitStartedWorking, [&](const cUnit& unit)
	{
		if (&unit == gameMap->getUnitSelection ().getSelectedUnit ())
		{
			stopSelectedUnitSound ();
			if (unit.data.ID.isABuilding ()) PlayFX (static_cast<const cBuilding&>(unit).uiData->Start);
			updateSelectedUnitIdleSound ();
		}
	});

	clientSignalConnectionManager.connect (client.unitStoppedWorking, [&](const cUnit& unit)
	{
		if (&unit == gameMap->getUnitSelection ().getSelectedUnit ())
		{
			stopSelectedUnitSound ();
			if (unit.data.ID.isABuilding ()) PlayFX (static_cast<const cBuilding&>(unit).uiData->Stop);
			updateSelectedUnitIdleSound ();
		}
	});

	clientSignalConnectionManager.connect (client.unitStartedBuilding, [&](const cUnit& unit)
	{
		if (&unit == gameMap->getUnitSelection().getSelectedUnit())
		{
			updateSelectedUnitIdleSound ();
		}
	});

	clientSignalConnectionManager.connect (client.unitStoppedBuilding, [&](const cUnit& unit)
	{
		if (&unit == gameMap->getUnitSelection ().getSelectedUnit ())
		{
			updateSelectedUnitIdleSound ();
		}
	});

	clientSignalConnectionManager.connect (client.unitStartedClearing, [&](const cUnit& unit)
	{
		if (&unit == gameMap->getUnitSelection ().getSelectedUnit ())
		{
			updateSelectedUnitIdleSound ();
		}
	});

	clientSignalConnectionManager.connect (client.unitStoppedClearing, [&](const cUnit& unit)
	{
		if (&unit == gameMap->getUnitSelection ().getSelectedUnit ())
		{
			updateSelectedUnitIdleSound ();
		}
	});

	clientSignalConnectionManager.connect (client.unitStored, [&](const cUnit&, const cUnit&) // storing, stored
	{
		//auto mouseTilePosition = getTilePosition (cMouse::getInstance ().getPosition ());
		//if (storedVehicle.PosX == mouseTilePosition.x () && storedVehicle.PosY == mouseTilePosition.y ()) updateMouseCursor ();

		//checkMouseInputMode ();

		//if (&storedVehicle == getSelectedUnit ()) deselectUnit ();

		PlayFX (SoundData.SNDLoad.get ());
	});

	clientSignalConnectionManager.connect (client.unitActivated, [&](const cUnit&, const cUnit&) // storing, stored
	{
		//if (gameGUI->getSelectedUnit() == StoringVehicle && gameGUI->mouseInputMode == activateVehicle)
		//{
		//	gameGUI->mouseInputMode = normalInput;
		//}

		PlayFX (SoundData.SNDActivate.get ());
	});

	clientSignalConnectionManager.connect (client.unitHasStolenSuccessfully, [&](const cUnit&)
	{
		PlayRandomVoice (VoiceData.VOIUnitStolen);
	});

	clientSignalConnectionManager.connect (client.unitHasDisabledSuccessfully, [&](const cUnit&)
	{
		PlayVoice (VoiceData.VOIUnitDisabled.get ());
	});

	clientSignalConnectionManager.connect (client.unitStealDisableFailed, [&](const cUnit&)
	{
		PlayRandomVoice (VoiceData.VOICommandoFailed);
	});

	clientSignalConnectionManager.connect (client.unitSuppliedWithAmmo, [&](const cUnit&)
	{
		PlayFX (SoundData.SNDReload.get ());
		PlayVoice (VoiceData.VOIReammo.get ());
	});

	clientSignalConnectionManager.connect (client.unitRepaired, [&](const cUnit&)
	{
		PlayFX (SoundData.SNDRepair.get ());
		PlayRandomVoice (VoiceData.VOIRepaired);
	});

	clientSignalConnectionManager.connect (client.unitDisabled, [&](const cUnit& unit)
	{
		PlayVoice (VoiceData.VOIUnitDisabled.get ());
	});

	clientSignalConnectionManager.connect (client.unitStolen, [&](const cUnit& unit)
	{
		PlayVoice (VoiceData.VOIUnitStolenByEnemy.get ());
	});

	clientSignalConnectionManager.connect (client.unitDetected, [&](const cUnit& unit)
	{
		if (unit.data.isStealthOn & TERRAIN_SEA && unit.data.canAttack)
		{
			PlayVoice (VoiceData.VOISubDetected.get ());
		}
		else
		{
			PlayRandomVoice (VoiceData.VOIDetected);
		}
	});
	clientSignalConnectionManager.connect (client.addedEffect, [&](const std::shared_ptr<cFx>& effect)
	{
		gameMap->addEffect (effect);
	});

	clientSignalConnectionManager.connect (client.moveJobCreated, [&](const cVehicle& vehicle)
	{
		if (&vehicle == gameMap->getUnitSelection ().getSelectedVehicle ())
		{
			connectMoveJob (vehicle);
		}
	});
	clientSignalConnectionManager.connect (client.moveJobBlocked, [&](const cVehicle& vehicle)
	{
		if (&vehicle == gameMap->getUnitSelection ().getSelectedVehicle () && !vehicle.autoMJob)
		{
			PlayRandomVoice (VoiceData.VOINoPath);
		}
	});
}

//------------------------------------------------------------------------------
void cGameGui::connectMoveJob (const cVehicle& vehicle)
{
	moveJobSignalConnectionManager.disconnectAll ();

	if (&vehicle == gameMap->getUnitSelection ().getSelectedVehicle () && vehicle.ClientMoveJob)
	{
		auto& moveJob = *vehicle.ClientMoveJob;

		moveJobSignalConnectionManager.connect (moveJob.activated, [&](const cVehicle& vehicle)
		{
			if (&vehicle == gameMap->getUnitSelection ().getSelectedVehicle ())
			{
				updateSelectedUnitMoveSound ();
			}
		});

		moveJobSignalConnectionManager.connect (moveJob.stopped, [&](const cVehicle& vehicle)
		{
			if (&vehicle == gameMap->getUnitSelection ().getSelectedVehicle () && dynamicMap)
			{
				const auto building = dynamicMap->getField (vehicle.getPosition()).getBaseBuilding ();
				bool water = dynamicMap->isWater (vehicle.getPosition());
				if (vehicle.data.factorGround > 0 && building && (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)) water = false;

				stopSelectedUnitSound ();
				if (water && vehicle.data.factorSea > 0) PlayFX (vehicle.uiData->StopWater);
				else PlayFX (vehicle.uiData->Stop);

				updateSelectedUnitIdleSound ();
			}
		});

		moveJobSignalConnectionManager.connect (moveJob.blocked, [&](const cVehicle& vehicle)
		{
			if (&vehicle == gameMap->getUnitSelection ().getSelectedVehicle ())
			{
				PlayRandomVoice (VoiceData.VOINoPath);
			}
		});

		moveJobSignalConnectionManager.connect (moveJob.moved, [&](const cVehicle& vehicle)
		{
			if (&vehicle == gameMap->getUnitSelection ().getSelectedVehicle ())
			{
				if (!vehicle.ClientMoveJob) return;

				bool wasWater = dynamicMap->isWater (vehicle.ClientMoveJob->Waypoints->position);
				bool water = dynamicMap->isWater (vehicle.ClientMoveJob->Waypoints->next->position);

				if (wasWater != water)
				{
					updateSelectedUnitMoveSound ();
				}
			}
		});
	}
}

//------------------------------------------------------------------------------
void cGameGui::disconnectCurrentClient ()
{
	clientSignalConnectionManager.disconnectAll ();
}

//------------------------------------------------------------------------------
void cGameGui::centerAt (const cPosition& position)
{
	gameMap->centerAt (position);
}

//------------------------------------------------------------------------------
void cGameGui::setInfoTexts (const std::string& primiaryText, const std::string& additionalText)
{
	primiaryInfoLabel->setText (primiaryText);
	if (primiaryText.empty ()) primiaryInfoLabel->hide ();
	else primiaryInfoLabel->show ();

	additionalInfoLabel->setText (additionalText);
	if (additionalText.empty ()) additionalInfoLabel->hide ();
	else additionalInfoLabel->show ();
}

//------------------------------------------------------------------------------
bool cGameGui::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	const auto currentMousePosition = mouse.getPosition ();
	const auto mouseLastPosition = currentMousePosition - offset;
	if (hud->isAt (currentMousePosition) && !hud->isAt (mouseLastPosition))
	{
		hud->setCoordinatesText ("");
		hud->setUnitNameText ("");
    }
    if (chatBox->isAt (currentMousePosition) && !chatBox->isAt (mouseLastPosition))
    {
        hud->setCoordinatesText ("");
        hud->setUnitNameText ("");
    }

	const int scrollFrameWidth = 5;

	mouseScrollDirection = 0;
	if (currentMousePosition.x () <= scrollFrameWidth) mouseScrollDirection.x () = -cSettings::getInstance ().getScrollSpeed () / 4;
	else if (currentMousePosition.x () >= getEndPosition ().x () - scrollFrameWidth) mouseScrollDirection.x () = +cSettings::getInstance ().getScrollSpeed () / 4;
	if (currentMousePosition.y () <= scrollFrameWidth) mouseScrollDirection.y () = -cSettings::getInstance ().getScrollSpeed () / 4;
	else if (currentMousePosition.y () >= getEndPosition ().y () - scrollFrameWidth) mouseScrollDirection.y () = +cSettings::getInstance ().getScrollSpeed () / 4;

	if (mouseScrollDirection.x () > 0 &&  mouseScrollDirection.y () == 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowRight));
	else if (mouseScrollDirection.x () < 0 &&  mouseScrollDirection.y () == 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowLeft));
	else if (mouseScrollDirection.x () == 0 &&  mouseScrollDirection.y () > 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowDown));
	else if (mouseScrollDirection.x () == 0 &&  mouseScrollDirection.y () < 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowUp));
	else if (mouseScrollDirection.x () > 0 &&  mouseScrollDirection.y () > 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowRightDown));
	else if (mouseScrollDirection.x () > 0 &&  mouseScrollDirection.y () < 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowRightUp));
	else if (mouseScrollDirection.x () < 0 &&  mouseScrollDirection.y () > 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowLeftDown));
	else if (mouseScrollDirection.x () < 0 &&  mouseScrollDirection.y () < 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowLeftUp));
	else if (hud->isAt (currentMousePosition)) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
    else if (chatBox->isAt (currentMousePosition)) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));

	return cWindow::handleMouseMoved (application, mouse, offset);
}

//------------------------------------------------------------------------------
bool cGameGui::handleMouseWheelMoved (cApplication& application, cMouse& mouse, const cPosition& amount)
{
	const auto oldZoomFactor = gameMap->getZoomFactor ();
	bool consumed = false;
	if (amount.y () > 0)
	{
		hud->decreaseZoomFactor (0.05);
		consumed = true;
	}
	else if (amount.y () < 0)
	{
		hud->increaseZoomFactor (0.05);
		consumed = true;
	}

	// scroll so that the zoom has been performed to center the mouse
	const auto newZoomFactor = gameMap->getZoomFactor ();
	if (newZoomFactor != oldZoomFactor)
	{
		cPosition scrollOffset;

		const auto oldScreenPixelX = gameMap->getSize ().x () / oldZoomFactor;
		const auto newScreenPixelX = gameMap->getSize ().x () / newZoomFactor;
		scrollOffset.x () = (int)((oldScreenPixelX - newScreenPixelX) * (mouse.getPosition ().x () - gameMap->getPosition ().x ()) / gameMap->getSize ().x () - (oldScreenPixelX - newScreenPixelX) / 2);

		const auto oldScreenPixelY = gameMap->getSize ().y () / oldZoomFactor;
		const auto newScreenPixelY = gameMap->getSize ().y () / newZoomFactor;
		scrollOffset.y () = (int)((oldScreenPixelY - newScreenPixelY) * (mouse.getPosition ().y () - gameMap->getPosition ().y ()) / gameMap->getSize ().y () - (oldScreenPixelY - newScreenPixelY) / 2);

		gameMap->scroll (scrollOffset);
	}

	return consumed;
}

//------------------------------------------------------------------------------
void cGameGui::handleActivated (cApplication& application, bool firstTime)
{
	cWindow::handleActivated (application, firstTime);

	auto mouse = getActiveMouse ();
	if (mouse)
	{
        if (hud->isAt (mouse->getPosition ())) mouse->setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
        else if (chatBox->isAt (mouse->getPosition ())) mouse->setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
		else gameMap->updateMouseCursor (*mouse);
	}

	if (openPanelOnActivation)
	{
		startOpenPanel ();
		openPanelOnActivation = false;
	}

	if (firstTime)
	{
		application.addRunnable (animationTimer);
	}
}

//------------------------------------------------------------------------------
void cGameGui::handleDeactivated (cApplication& application, bool removed)
{
	cWindow::handleDeactivated (application, removed);

	if (removed)
	{
		application.removeRunnable (*animationTimer);
	}
}

//------------------------------------------------------------------------------
bool cGameGui::wantsCentered () const
{
	return false;
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseCursor> cGameGui::getDefaultCursor () const
{
	return nullptr;
}

//------------------------------------------------------------------------------
void cGameGui::startOpenPanel ()
{
	hudPanels->open ();
}

//------------------------------------------------------------------------------
void cGameGui::startClosePanel ()
{
	hudPanels->show ();
	hudPanels->enable ();

	hudPanels->close ();
}

//------------------------------------------------------------------------------
void cGameGui::exit ()
{
	panelSignalConnectionManager.disconnectAll ();

	panelSignalConnectionManager.connect (hudPanels->closed, [&]()
	{
		close ();
	});
	startClosePanel ();
}

//------------------------------------------------------------------------------
void cGameGui::resetMiniMapViewWindow ()
{
	miniMap->setViewWindow (gameMap->getDisplayedMapArea ());
}

//------------------------------------------------------------------------------
void cGameGui::showFilesWindow ()
{
	auto application = getActiveApplication ();
	if (!application) return;

	auto loadSaveWindow = application->show (std::make_shared<cWindowLoadSave> ());
	loadSaveWindow->exit.connect ([&, loadSaveWindow, application]()
	{
		auto yesNoDialog = application->show (std::make_shared<cDialogYesNo> (lngPack.i18n ("Text~Comp~End_Game")));
		signalConnectionManager.connect (yesNoDialog->yesClicked, [&, loadSaveWindow]()
		{
			loadSaveWindow->close ();
			exit ();
		});
	});
	loadSaveWindow->load.connect ([loadSaveWindow, application](int saveNumber)
	{
		// loading games while game is running is not yet implemented
		application->show (std::make_shared<cDialogOk> (lngPack.i18n ("Text~Error_Messages~INFO_Not_Implemented")));
	});
	loadSaveWindow->save.connect ([this, loadSaveWindow, application](int saveNumber, const std::string& name)
	{
		try
		{
			triggeredSave (saveNumber, name);

			//PlayVoice (VoiceData.VOISaved.get ());

			loadSaveWindow->update ();
		}
		catch (std::runtime_error& e)
		{
			application->show (std::make_shared<cDialogOk> (e.what()));
		}
	});
}

//------------------------------------------------------------------------------
void cGameGui::toggleChatBox ()
{
    if (chatBox->isHidden ())
    {
        chatBox->show ();
        chatBox->enable ();
    }
    else
    {
        chatBox->hide ();
        chatBox->disable ();
    }
}

//------------------------------------------------------------------------------
void cGameGui::showPreferencesDialog ()
{
	auto application = getActiveApplication ();
	if (!application) return;

	application->show (std::make_shared<cDialogPreferences> ());
}

//------------------------------------------------------------------------------
void cGameGui::showReportsWindow ()
{
	auto application = getActiveApplication ();
	if (!application) return;

	auto reportsWindow = application->show (std::make_shared<cWindowReports> (players, player, casualtiesTracker, turnClock, gameSettings));

	signalConnectionManager.connect (reportsWindow->unitClickedSecondTime, [this, reportsWindow](cUnit& unit)
	{
		gameMap->getUnitSelection ().selectUnit (unit);
		gameMap->centerAt (unit.getPosition ());
		reportsWindow->close ();
	});

	signalConnectionManager.connect (reportsWindow->reportClickedSecondTime, [this, reportsWindow](const cSavedReport& report)
	{
		if (report.getType () == eSavedReportType::Unit)
		{
			auto& savedUnitReport = static_cast<const cSavedReportUnit&>(report);

			gameMap->centerAt (savedUnitReport.getPosition ());
			reportsWindow->close ();
		}
	});
}

//------------------------------------------------------------------------------
void cGameGui::showUnitHelpWindow (const cUnit& unit)
{
	auto application = getActiveApplication ();
	if (!application) return;

	application->show (std::make_shared<cWindowUnitInfo> (unit.data, *unit.owner));
}

//------------------------------------------------------------------------------
void cGameGui::showUnitTransferDialog (const cUnit& sourceUnit, const cUnit& destinationUnit)
{
	auto application = getActiveApplication ();
	if (!application) return;

	auto transferDialog = application->show (std::make_shared<cNewDialogTransfer> (sourceUnit, destinationUnit));
	transferDialog->done.connect ([&, transferDialog]()
	{
		transferTriggered (sourceUnit, destinationUnit, transferDialog->getTransferValue (), transferDialog->getResourceType ());
		transferDialog->close ();
	});
}

//------------------------------------------------------------------------------
void cGameGui::showBuildBuildingsWindow (const cVehicle& vehicle)
{
	auto application = getActiveApplication ();
	if (!application) return;

	auto buildWindow = application->show (std::make_shared<cWindowBuildBuildings> (vehicle));

	buildWindow->done.connect ([&, buildWindow]()
	{
		if (buildWindow->getSelectedUnitId ())
		{
			if (buildWindow->getSelectedUnitId ()->getUnitDataOriginalVersion ()->isBig)
			{
				gameMap->startFindBuildPosition (*buildWindow->getSelectedUnitId ());
				auto buildType = *buildWindow->getSelectedUnitId ();
				auto buildSpeed = buildWindow->getSelectedBuildSpeed ();

				buildPositionSelectionConnectionManager.disconnectAll ();
				buildPositionSelectionConnectionManager.connect (gameMap->selectedBuildPosition, [this, buildType, buildSpeed](const cVehicle& selectedVehicle, const cPosition& destination)
				{
					buildBuildingTriggered (selectedVehicle, destination, buildType, buildSpeed);
				});
			}
			else
			{
				buildBuildingTriggered (vehicle, vehicle.getPosition(), *buildWindow->getSelectedUnitId (), buildWindow->getSelectedBuildSpeed ());
			}
		}
		buildWindow->close ();
	});
	buildWindow->donePath.connect ([&, buildWindow]()
	{
		if (buildWindow->getSelectedUnitId ())
		{
			gameMap->startFindPathBuildPosition ();
			auto buildType = *buildWindow->getSelectedUnitId ();
			auto buildSpeed = buildWindow->getSelectedBuildSpeed ();

			buildPositionSelectionConnectionManager.disconnectAll ();
			buildPositionSelectionConnectionManager.connect (gameMap->selectedBuildPathDestination, [this, buildType, buildSpeed](const cVehicle& selectedVehicle, const cPosition& destination)
			{
				buildBuildingPathTriggered (selectedVehicle, destination, buildType, buildSpeed);
			});
		}
		buildWindow->close ();
	});
}

//------------------------------------------------------------------------------
void cGameGui::showBuildVehiclesWindow (const cBuilding& building)
{
	auto application = getActiveApplication ();
	if (!application) return;

	if (!dynamicMap) return;

	auto buildWindow = application->show (std::make_shared<cWindowBuildVehicles> (building, *dynamicMap));

	buildWindow->done.connect ([&, buildWindow]()
	{
		buildVehiclesTriggered (building, buildWindow->getBuildList (), buildWindow->getSelectedBuildSpeed (), buildWindow->isRepeatActive ());
		buildWindow->close ();
	});
}

//------------------------------------------------------------------------------
void cGameGui::showResourceDistributionDialog (const cUnit& unit)
{
	if (!unit.isABuilding ()) return;

	auto application = getActiveApplication ();
	if (!application) return;

	const auto& building = static_cast<const cBuilding&>(unit);

	auto resourceDistributionWindow = application->show (std::make_shared<cWindowResourceDistribution> (*building.SubBase));
	resourceDistributionWindow->done.connect ([&, resourceDistributionWindow]()
	{
		changeResourceDistributionTriggered (building, resourceDistributionWindow->getMetalProduction (), resourceDistributionWindow->getOilProduction (), resourceDistributionWindow->getGoldProduction());
		resourceDistributionWindow->close ();
	});
}

//------------------------------------------------------------------------------
void cGameGui::showResearchDialog (const cUnit& unit)
{
	auto application = getActiveApplication ();
	if (!application) return;

	if (unit.owner != player.get()) return;
	if (!player) return;

	auto researchDialog = application->show (std::make_shared<cDialogResearch> (*player));
	researchDialog->done.connect ([&, researchDialog]()
	{
		changeResearchSettingsTriggered (researchDialog->getResearchSettings ());
		researchDialog->close ();
	});
}

//------------------------------------------------------------------------------
void cGameGui::showUpgradesWindow (const cUnit& unit)
{
	auto application = getActiveApplication ();
	if (!application) return;

	if (unit.owner != player.get()) return;
	if (!player) return;

	auto upgradesWindow = application->show (std::make_shared<cWindowUpgrades> (*player));
	upgradesWindow->done.connect ([&, upgradesWindow]()
	{
		takeUnitUpgradesTriggered (upgradesWindow->getUnitUpgrades ());
		upgradesWindow->close ();
	});
}

//------------------------------------------------------------------------------
void cGameGui::showStorageWindow (const cUnit& unit)
{
	auto application = getActiveApplication ();
	if (!application) return;

	auto storageWindow = application->show (std::make_shared<cWindowStorage> (unit));
	storageWindow->activate.connect ([&, storageWindow](size_t index)
	{
		if (unit.isAVehicle () && unit.data.factorAir > 0)
		{
			activateAtTriggered (unit, index, unit.getPosition());
		}
		else
		{
			gameMap->startActivateVehicle (unit, index);
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
		if (dynamicMap)
		{
			std::array<bool, 16> hasCheckedPlace;
			hasCheckedPlace.fill (false);

			for (size_t i = 0; i < unit.storedUnits.size (); ++i)
			{
				const auto& storedUnit = *unit.storedUnits[i];

				bool activated = false;
				for(int ypos = unit.getPosition().y() - 1, poscount = 0; ypos <= unit.getPosition().y() + (unit.data.isBig ? 2 : 1); ypos++)
				{
					if (ypos < 0 || ypos >= dynamicMap->getSize ().y()) continue;
					for(int xpos = unit.getPosition().x() - 1; xpos <= unit.getPosition().x() + (unit.data.isBig ? 2 : 1); xpos++, poscount++)
					{
						if (hasCheckedPlace[poscount]) continue;

						if (xpos < 0 || xpos >= dynamicMap->getSize ().x()) continue;

						if(((ypos == unit.getPosition().y() && unit.data.factorAir == 0) || (ypos == unit.getPosition().y() + 1 && unit.data.isBig)) &&
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
void cGameGui::showSelfDestroyDialog (const cUnit& unit)
{
	if (unit.data.canSelfDestroy)
	{
		auto application = getActiveApplication ();

		if (application)
		{
			auto selfDestroyDialog = application->show (std::make_shared<cDialogSelfDestruction> (unit, animationTimer));
			signalConnectionManager.connect (selfDestroyDialog->triggeredDestruction, [this, selfDestroyDialog, &unit]()
			{
				selfDestructionTriggered (unit);
				selfDestroyDialog->close ();
			});
		}
	}
}

//------------------------------------------------------------------------------
void cGameGui::updateSelectedUnitIdleSound ()
{
	auto selectedUnit = gameMap->getUnitSelection().getSelectedUnit ();
	if (selectedUnit == nullptr)
	{
		stopSelectedUnitSound ();
	}
	else if (selectedUnit->data.ID.isABuilding ())
	{
		const auto& building = static_cast<const cBuilding&>(*selectedUnit);
		if (building.isUnitWorking ())
		{
			startSelectedUnitSound (building.uiData->Running);
		}
		else
		{
			startSelectedUnitSound (building.uiData->Wait);
		}
	}
	else if (selectedUnit->data.ID.isAVehicle ())
	{
		const auto& vehicle = static_cast<const cVehicle&>(*selectedUnit);

		const cBuilding* building = dynamicMap ? dynamicMap->getField (vehicle.getPosition()).getBaseBuilding () : nullptr;
		bool water = staticMap->isWater (vehicle.getPosition());
		if (vehicle.data.factorGround > 0 && building && (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)) water = false;

		if (vehicle.isUnitBuildingABuilding () && (vehicle.getBuildTurns () || player.get() != vehicle.owner))
		{
			startSelectedUnitSound (SoundData.SNDBuilding.get ());
		}
		else if (vehicle.isUnitClearing ())
		{
			startSelectedUnitSound (SoundData.SNDClearing.get ());
		}
		else if (water && vehicle.data.factorSea > 0)
		{
			startSelectedUnitSound (vehicle.uiData->WaitWater);
		}
		else
		{
			startSelectedUnitSound (vehicle.uiData->Wait);
		}
	}
}

//------------------------------------------------------------------------------
void cGameGui::updateSelectedUnitMoveSound ()
{
	auto selectedVehicle = gameMap->getUnitSelection ().getSelectedVehicle ();
	if (!selectedVehicle) return;
	if (!dynamicMap) return;

	const auto& vehicle = *selectedVehicle;

	const auto building = dynamicMap->getField (vehicle.getPosition()).getBaseBuilding ();
	bool water = dynamicMap->isWater (vehicle.getPosition());
	if (vehicle.data.factorGround > 0 && building && (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)) water = false;
	stopSelectedUnitSound ();

	if (!vehicle.MoveJobActive)
	{
		if (water && vehicle.data.factorSea != 0)
			PlayFX (vehicle.uiData->StartWater);
		else
			PlayFX (vehicle.uiData->Start);
	}

	if (water && vehicle.data.factorSea != 0)
		startSelectedUnitSound (vehicle.uiData->DriveWater);
	else
		startSelectedUnitSound (vehicle.uiData->Drive);
}

//------------------------------------------------------------------------------
void cGameGui::startSelectedUnitSound (sSOUND* sound)
{
	stopSelectedUnitSound ();
	selectedUnitSoundStream = PlayFXLoop (sound);
}

//------------------------------------------------------------------------------
void cGameGui::stopSelectedUnitSound ()
{
	if (selectedUnitSoundStream != -1) StopFXLoop (selectedUnitSoundStream);
	selectedUnitSoundStream = -1;
}

//------------------------------------------------------------------------------
void cGameGui::handleChatCommand (const std::string& command)
{
	if (command.empty ()) return;

	if (command[0] == '/')
	{
		//if (command.compare ("/fps on") == 0) { debugOutput.showFPS = true; }
		//else if (command.compare ("/fps off") == 0) { debugOutput.showFPS = false; }
		//else if (command.compare ("/base client") == 0) { debugOutput.debugBaseClient = true; debugOutput.debugBaseServer = false; }
		//else if (command.compare ("/base server") == 0) { if (server) debugOutput.debugBaseServer = true; debugOutput.debugBaseClient = false; }
		//else if (command.compare ("/base off") == 0) { debugOutput.debugBaseServer = false; debugOutput.debugBaseClient = false; }
		//else if (command.compare ("/sentry server") == 0) { if (server) debugOutput.debugSentry = true; }
		//else if (command.compare ("/sentry off") == 0) { debugOutput.debugSentry = false; }
		//else if (command.compare ("/fx on") == 0) { debugOutput.debugFX = true; }
		//else if (command.compare ("/fx off") == 0) { debugOutput.debugFX = false; }
		//else if (command.compare ("/trace server") == 0) { if (server) debugOutput.debugTraceServer = true; debugOutput.debugTraceClient = false; }
		//else if (command.compare ("/trace client") == 0) { debugOutput.debugTraceClient = true; debugOutput.debugTraceServer = false; }
		//else if (command.compare ("/trace off") == 0) { debugOutput.debugTraceServer = false; debugOutput.debugTraceClient = false; }
		//else if (command.compare ("/ajobs on") == 0) { debugOutput.debugAjobs = true; }
		//else if (command.compare ("/ajobs off") == 0) { debugOutput.debugAjobs = false; }
		//else if (command.compare ("/players on") == 0) { debugOutput.debugPlayers = true; }
		//else if (command.compare ("/players off") == 0) { debugOutput.debugPlayers = false; }
		//else if (command.compare ("/singlestep") == 0) { cGameTimer::syncDebugSingleStep = !cGameTimer::syncDebugSingleStep; }
		//else if (command.compare (0, 12, "/cache size ") == 0)
		//{
		//    int size = atoi (command.substr (12, command.length ()).c_str ());
		//    // since atoi is too stupid to report an error,
		//    // do an extra check, when the number is 0
		//    if (size == 0 && command[12] != '0')
		//    {
		//        messageList->addMessage ("Wrong parameter");
		//        return;
		//    }
		//    getDCache ()->setMaxCacheSize (size);
		//}
		//else if (command.compare ("/cache flush") == 0)
		//{
		//    getDCache ()->flush ();
		//}
		//else if (command.compare ("/cache debug on") == 0)
		//{
		//    debugOutput.debugCache = true;
		//}
		//else if (command.compare ("/cache debug off") == 0)
		//{
		//    debugOutput.debugCache = false;
		//}
	}

	chatCommandTriggered (command);
}

//------------------------------------------------------------------------------
void cGameGui::initShortcuts ()
{
	auto scroll1Shortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll1));
	signalConnectionManager.connect (scroll1Shortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (-cSettings::getInstance ().getScrollSpeed (), +cSettings::getInstance ().getScrollSpeed ())));

	auto scroll3Shortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll3));
	signalConnectionManager.connect (scroll3Shortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (+cSettings::getInstance ().getScrollSpeed (), +cSettings::getInstance ().getScrollSpeed ())));

	auto scroll7Shortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll7));
	signalConnectionManager.connect (scroll7Shortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (-cSettings::getInstance ().getScrollSpeed (), -cSettings::getInstance ().getScrollSpeed ())));

	auto scroll9Shortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll9));
	signalConnectionManager.connect (scroll9Shortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (+cSettings::getInstance ().getScrollSpeed (), -cSettings::getInstance ().getScrollSpeed ())));


	auto scroll2aShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll2a));
	signalConnectionManager.connect (scroll2aShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (0, +cSettings::getInstance ().getScrollSpeed ())));
	auto scroll2bShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll2b));
	signalConnectionManager.connect (scroll2bShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (0, +cSettings::getInstance ().getScrollSpeed ())));

	auto scroll4aShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll4a));
	signalConnectionManager.connect (scroll4aShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (-cSettings::getInstance ().getScrollSpeed (), 0)));
	auto scroll4bShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll4b));
	signalConnectionManager.connect (scroll4bShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (-cSettings::getInstance ().getScrollSpeed (), 0)));

	auto scroll6aShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll6a));
	signalConnectionManager.connect (scroll6aShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (+cSettings::getInstance ().getScrollSpeed (), 0)));
	auto scroll6bShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll6b));
	signalConnectionManager.connect (scroll6bShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (+cSettings::getInstance ().getScrollSpeed (), 0)));

	auto scroll8aShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll8a));
	signalConnectionManager.connect (scroll8aShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (0, -cSettings::getInstance ().getScrollSpeed ())));
	auto scroll8bShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll8b));
	signalConnectionManager.connect (scroll8bShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (0, -cSettings::getInstance ().getScrollSpeed ())));


	auto exitShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyExit));
	signalConnectionManager.connect (exitShortcut->triggered, [&]()
	{
		auto application = getActiveApplication ();

		if (!application) return;

		auto yesNoDialog = application->show (std::make_shared<cDialogYesNo> (lngPack.i18n ("Text~Comp~End_Game")));
		signalConnectionManager.connect (yesNoDialog->yesClicked, [&]()
		{
			exit ();
		});
	});

	auto chatShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyChat));
	signalConnectionManager.connect (chatShortcut->triggered, [&]()
	{
		chatBox->show ();
		chatBox->enable ();
		chatBox->focus ();
	});

	auto jumpToActionShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyJumpToAction));
	signalConnectionManager.connect (jumpToActionShortcut->triggered, [&]()
	{
		if (savedReportPosition.first)
		{
			centerAt (savedReportPosition.second);
		}
	});
}

//------------------------------------------------------------------------------
void cGameGui::handleReport (const cSavedReport& report)
{
	if (report.getType () == eSavedReportType::Chat)
	{
		auto& savedChatReport = static_cast<const cSavedReportChat&>(report);
		auto player = chatBox->getPlayerFromNumber (savedChatReport.getPlayerNumber ());

		if (player)
		{
			chatBox->addChatMessage (*player, savedChatReport.getText ());
		}
		else // should never happen!
		{
			messageList->addMessage (savedChatReport.getMessage ());
		}
	}
	else if (report.getType () == eSavedReportType::Unit)
	{
		auto& savedUnitReport = static_cast<const cSavedReportUnit&>(report);

		savedReportPosition.first = true;
		savedReportPosition.second = savedUnitReport.getPosition ();

		messageList->addMessage (savedUnitReport.getMessage () + " (" + KeysList.keyJumpToAction.toString () + ")", eGameMessageListViewItemBackgroundColor::LightGray);

	}
	else
	{
		messageList->addMessage (report.getMessage (), report.isAlert () ? eGameMessageListViewItemBackgroundColor::Red : eGameMessageListViewItemBackgroundColor::DarkGray);
	}

	if (cSettings::getInstance ().isDebug ()) Log.write (report.getMessage (), cLog::eLOG_TYPE_DEBUG);
}

//------------------------------------------------------------------------------
void cGameGui::handleResolutionChange ()
{
	hud->resizeToResolution ();

	resize (hud->getSize ());

	hudPanels->resize (cPosition (hudPanels->getSize ().x (), getSize ().y ()));

	gameMap->resize (getSize () - cPosition (cHud::panelTotalWidth, cHud::panelTopHeight));

	messageList->setArea (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, cHud::panelTopHeight + 7), cPosition (getEndPosition ().x () - cHud::panelRightWidth - 4, cHud::panelTopHeight + 200)));

	chatBox->setArea (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, getEndPosition ().y () - cHud::panelBottomHeight - 12 - 100), getEndPosition () - cPosition (cHud::panelRightWidth + 4, cHud::panelBottomHeight + 12)));
}