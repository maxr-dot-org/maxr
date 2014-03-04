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

#include "temp/animationtimer.h"

#include "../application.h"
#include "../menu/dialogs/dialogpreferences.h"

#include "../../keys.h"
#include "../../player.h"
#include "../../map.h"
#include "../../vehicles.h"
#include "../../buildings.h"
#include "../../sound.h"
#include "../../client.h"
#include "../../clientevents.h"
#include "../../attackjobs.h"
#include "../../log.h"
#include "../../input/mouse/mouse.h"

//------------------------------------------------------------------------------
cNewGameGUI::cNewGameGUI (std::shared_ptr<const cStaticMap> staticMap_) :
	cWindow (nullptr),
	animationTimer (std::make_shared<cAnimationTimer>()),
	staticMap (std::move (staticMap_)),
	dynamicMap (nullptr),
	mouseScrollDirection (0, 0),
	selectedUnitSoundStream (-1)
{
	auto hudOwning = std::make_unique<cHud> (animationTimer);

	resize (hudOwning->getSize ());

	gameMap = addChild (std::make_unique<cGameMapWidget> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, cHud::panelTopHeight), getEndPosition () - cPosition (cHud::panelRightWidth, cHud::panelBottomHeight)), staticMap, animationTimer));

	messageList = addChild (std::make_unique<cGameMessageListView> (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 2, cHud::panelTopHeight + 7), cPosition (getEndPosition ().x () - cHud::panelRightWidth - 2, cHud::panelTopHeight + 200))));

	hud = addChild (std::move (hudOwning));

	miniMap = addChild (std::make_unique<cMiniMapWidget> (cBox<cPosition> (cPosition (15, 356), cPosition (15 + 112, 356 + 112)), staticMap));

	using namespace std::placeholders;

	signalConnectionManager.connect (hud->preferencesClicked, std::bind (&cNewGameGUI::showPreferencesDialog, this));
	signalConnectionManager.connect (hud->filesClicked, std::bind (&cNewGameGUI::showFilesMenu, this));

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
		if (selectedUnit) gameMap->centerAt (cPosition (selectedUnit->PosX, selectedUnit->PosY));
	});

	signalConnectionManager.connect (hud->miniMapZoomFactorToggled, [&](){ miniMap->setZoomFactor (hud->getMiniMapZoomFactorActive () ? 2 : 1); });
	signalConnectionManager.connect (hud->miniMapAttackUnitsOnlyToggled, [&](){ miniMap->setAttackUnitsUnly (hud->getMiniMapAttackUnitsOnly ()); });

	signalConnectionManager.connect (gameMap->scrolled, std::bind(&cNewGameGUI::resetMiniMapViewWindow, this));
	signalConnectionManager.connect (gameMap->zoomFactorChanged, std::bind (&cNewGameGUI::resetMiniMapViewWindow, this));
	signalConnectionManager.connect (gameMap->tileUnderMouseChanged, std::bind (&cNewGameGUI::updateHudCoordinates, this, _1));
	signalConnectionManager.connect (gameMap->tileUnderMouseChanged, std::bind (&cNewGameGUI::updateHudUnitName, this, _1));

	signalConnectionManager.connect (gameMap->getUnitSelection ().mainSelectionChanged, [&](){ hud->setActiveUnit (gameMap->getUnitSelection ().getSelectedUnit ()); });
	signalConnectionManager.connect (gameMap->getUnitSelection ().mainSelectionChanged, std::bind (&cNewGameGUI::updateSelectedUnitIdleSound, this));


	clientSignalConnectionManager.connect (gameMap->triggeredUnitHelp, [&](const cUnit&)
	{
		//cUnitHelpMenu helpMenu (&overVehicle->data, overVehicle->owner);
		//switchTo (helpMenu, client);
	});
	clientSignalConnectionManager.connect (gameMap->triggeredTransfer, [&](const cUnit&, const cUnit&)
	{
		//if (overVehicle)
		//{
		//	cDialogTransfer transferDialog (*this, *selectedUnit, *overVehicle);
		//	switchTo (transferDialog, client);
		//}
		//else if (overBuilding)
		//{
		//	cDialogTransfer transferDialog (*this, *selectedUnit, *overBuilding);
		//	switchTo (transferDialog, client);
		//}
	});
	signalConnectionManager.connect (gameMap->triggeredBuild, [&](const cUnit& unit)
	{
		//unit.executeBuildCommand (*this);
	});
	signalConnectionManager.connect (gameMap->triggeredResourceDistribution, [&](const cUnit& unit)
	{
		//building->executeMineManagerCommand (*this);
	});
	signalConnectionManager.connect (gameMap->triggeredResearchMenu, [&](const cUnit& unit)
	{
		//cDialogResearch researchDialog (*client);
		//switchTo (researchDialog, client);
	});
	signalConnectionManager.connect (gameMap->triggeredUpgradesMenu, [&](const cUnit& unit)
	{
		//cUpgradeMenu upgradeMenu (*client);
		//switchTo (upgradeMenu, client);
	});
	signalConnectionManager.connect (gameMap->triggeredActivate, [&](const cUnit& unit)
	{
		//unit.executeActivateStoredVehiclesCommand (*this);
	});
	signalConnectionManager.connect (gameMap->triggeredSelfDestruction, [&](const cUnit& unit)
	{
		//building->executeSelfDestroyCommand (*this);
	});

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
}

//------------------------------------------------------------------------------
void cNewGameGUI::updateHudCoordinates (const cPosition& tilePosition)
{
	std::stringstream coordsString;
	coordsString << std::setw (3) << std::setfill ('0') << tilePosition.x () << "-" << std::setw (3) << std::setfill ('0') << tilePosition.y ();
	hud->setCoordinatesText (coordsString.str ());
}

//------------------------------------------------------------------------------
void cNewGameGUI::updateHudUnitName (const cPosition& tilePosition)
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
void cNewGameGUI::setDynamicMap (const cMap* dynamicMap_)
{
	dynamicMap = dynamicMap_;
	gameMap->setDynamicMap (dynamicMap);
	miniMap->setDynamicMap (dynamicMap);

	dynamicMapSignalConnectionManager.disconnectAll ();

	if (dynamicMap != nullptr)
	{
		dynamicMapSignalConnectionManager.connect (dynamicMap->addedUnit, [&](const cUnit& unit)
		{
			if (unit.data.ID == UnitsData.specialIDLandMine) PlayFX (SoundData.SNDLandMinePlace);
			else if (unit.data.ID == UnitsData.specialIDSeaMine) PlayFX (SoundData.SNDSeaMinePlace);
		});
		dynamicMapSignalConnectionManager.connect (dynamicMap->removedUnit, [&](const cUnit& unit)
		{
			if (unit.data.ID == UnitsData.specialIDLandMine) PlayFX (SoundData.SNDLandMineClear);
			else if (unit.data.ID == UnitsData.specialIDSeaMine) PlayFX (SoundData.SNDSeaMineClear);
		});
		//dynamicMapSignalConnectionManager.connect (dynamicMap->movedVehicle, [&](const cVehicle&){ });
	}
}

//------------------------------------------------------------------------------
void cNewGameGUI::setPlayer (const cPlayer* player_)
{
	player = player_;
	gameMap->setPlayer (player);
	miniMap->setPlayer (player);
	hud->setPlayer (player);

	playerSignalConnectionManager.disconnectAll ();

	if (player != nullptr)
	{
		playerSignalConnectionManager.connect (player->reportAdded, [&](const sSavedReportMessage& report)
		{
			if (report.xPos == -1 || report.yPos == -1)
			{
				messageList->addMessage (report.getFullMessage ());
			}
			else
			{
				messageList->addMessage (report.getFullMessage () + " (" + GetKeyString (KeysList.KeyJumpToAction) + ")");
				// TODO: save position for jump!
			}

			if (cSettings::getInstance ().isDebug ()) Log.write (report.getFullMessage (), cLog::eLOG_TYPE_DEBUG);
		});
	}
}

//------------------------------------------------------------------------------
void cNewGameGUI::connectToClient (cClient& client)
{
	hud->setTurnNumberText (iToStr (client.getTurn ()));

	//
	// GUI to client (action)
	//
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
		if (unit.owner == player)
		{
			if (unit.data.ID.isAVehicle ())
			{
				auto vehicle = client.getVehicleFromID (unit.iID);
				if (!vehicle) return;
				vehicle->setMarkedAsDone(true);
				sendMoveJobResume (client, vehicle->iID);
			}
			else if (unit.data.ID.isABuilding ())
			{
				auto building = client.getBuildingFromID (unit.iID);
				if (!building) return;
				building->setMarkedAsDone(true);
			}
		}
	});

	clientSignalConnectionManager.connect(gameMap->triggeredMoveSingle, [&](cVehicle& vehicle, const cPosition& destination)
	{
		client.addMoveJob (vehicle, destination.x (), destination.y ());
	});
	clientSignalConnectionManager.connect (gameMap->triggeredMoveGroup, [&](const std::vector<cVehicle*>& vehicles, const cPosition& destination)
	{
		client.startGroupMove (vehicles, destination.x (), destination.y ());
	});
	clientSignalConnectionManager.connect (gameMap->placedBand, [&](const cVehicle& vehicle)
	{
		if (vehicle.BuildingTyp.getUnitDataOriginalVersion ()->isBig)
		{
			sendWantBuild (client, vehicle.iID, vehicle.BuildingTyp, vehicle.BuildRounds, client.getMap ()->getOffset (vehicle.BandX, vehicle.BandY), false, 0);
		}
		else
		{
			sendWantBuild (client, vehicle.iID, vehicle.BuildingTyp, vehicle.BuildRounds, client.getMap ()->getOffset (vehicle.PosX, vehicle.PosY), true, client.getMap ()->getOffset (vehicle.BandX, vehicle.BandY));
		}
	});
	clientSignalConnectionManager.connect (gameMap->triggeredActivateAt, [&](const cUnit& unit, const cPosition& position)
	{
		//sendWantActivate (client, unit.iID, unit.isAVehicle (), unit.storedUnits[vehicleToActivate]->iID, position.x (), position.y ());
	});
	clientSignalConnectionManager.connect (gameMap->triggeredExitFinishedUnit, [&](const cBuilding& building, const cPosition& position)
	{
		sendWantExitFinishedVehicle (client, building, position.x (), position.y ());
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
				if (overVehicle->PosX == vehicle.PosX && overVehicle->PosY == vehicle.PosY) sendWantLoad (client, vehicle.iID, true, overVehicle->iID);
				else
				{
					cPathCalculator pc (vehicle.PosX, vehicle.PosY, overVehicle->PosX, overVehicle->PosY, *client.getMap (), vehicle);
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
				if (vehicle.isNextTo (overVehicle->PosX, overVehicle->PosY)) sendWantLoad (client, vehicle.iID, true, overVehicle->iID);
				else
				{
					cPathCalculator pc (overVehicle->PosX, overVehicle->PosY, vehicle, *client.getMap (), *overVehicle, true);
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
				if (building.isNextTo (overVehicle->PosX, overVehicle->PosY)) sendWantLoad (client, building.iID, false, overVehicle->iID);
				else
				{
					cPathCalculator pc (overVehicle->PosX, overVehicle->PosY, building, *client.getMap (), *overVehicle, true);
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
				if (building.isNextTo (overPlane->PosX, overPlane->PosY)) sendWantLoad (client, building.iID, false, overPlane->iID);
				else
				{
					cPathCalculator pc (overPlane->PosX, overPlane->PosY, building, *client.getMap (), *overPlane, true);
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
		sendWantSupply (client, destinationUnit.iID, destinationUnit.isAVehicle (), sourceUnit.iID, destinationUnit.isAVehicle(), SUPPLY_TYPE_REARM);
	});
	clientSignalConnectionManager.connect (gameMap->triggeredRepair, [&](const cUnit& sourceUnit, const cUnit& destinationUnit)
	{
		sendWantSupply (client, destinationUnit.iID, destinationUnit.isAVehicle (), sourceUnit.iID, destinationUnit.isAVehicle (), SUPPLY_TYPE_REPAIR);
	});
	clientSignalConnectionManager.connect (gameMap->triggeredAttack, [&](const cUnit& unit, const cPosition& position)
	{
		if (unit.isAVehicle ())
		{
			const auto& vehicle = static_cast<const cVehicle&>(unit);

			cUnit* target = selectTarget (position.x (), position.y (), vehicle.data.canAttack, *client.getMap ());

			if (vehicle.isInRange (position.x (), position.y ()))
			{
				// find target ID
				int targetId = 0;
				if (target && target->isAVehicle ()) targetId = target->iID;

				Log.write (" Client: want to attack " + iToStr (position.x ()) + ":" + iToStr (position.y ()) + ", Vehicle ID: " + iToStr (targetId), cLog::eLOG_TYPE_NET_DEBUG);
				sendWantAttack (client, targetId, client.getMap ()->getOffset (position.x (), position.y ()), vehicle.iID, true);
			}
			else if (target)
			{
				cPathCalculator pc (vehicle.PosX, vehicle.PosY, *client.getMap (), vehicle, position.x (), position.y ());
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
			cUnit* target = selectTarget (position.x (), position.y (), building.data.canAttack, map);
			if (target && target->isAVehicle ()) targetId = target->iID;

			const int offset = map.getOffset (building.PosX, building.PosY);
			sendWantAttack (client, targetId, map.getOffset (position.x (), position.y ()), offset, false);
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
	clientSignalConnectionManager.connect (client.turnChanged, [&]()
	{
		hud->setTurnNumberText (iToStr(client.getTurn ()));
	});

	clientSignalConnectionManager.connect (client.startedTurnEndProcess, [&]()
	{
		hud->lockEndButton ();
	});

	clientSignalConnectionManager.connect (client.finishedTurnEndProcess, [&]()
	{
		hud->unlockEndButton ();
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

		PlayFX (SoundData.SNDLoad);
	});

	clientSignalConnectionManager.connect (client.unitActivated, [&](const cUnit&, const cUnit&) // storing, stored
	{
		//if (gameGUI->getSelectedUnit() == StoringVehicle && gameGUI->mouseInputMode == activateVehicle)
		//{
		//	gameGUI->mouseInputMode = normalInput;
		//}

		PlayFX (SoundData.SNDActivate);
	});

	clientSignalConnectionManager.connect (client.unitHasStolenSuccessfully, [&](const cUnit&)
	{
		PlayRandomVoice (VoiceData.VOIUnitStolen);
	});

	clientSignalConnectionManager.connect (client.unitHasDisabledSuccessfully, [&](const cUnit&)
	{
		PlayVoice (VoiceData.VOIUnitDisabled);
	});

	clientSignalConnectionManager.connect (client.unitStealDisableFailed, [&](const cUnit&)
	{
		PlayRandomVoice (VoiceData.VOICommandoFailed);
	});

	clientSignalConnectionManager.connect (client.unitSuppliedWithAmmo, [&](const cUnit&)
	{
		PlayFX (SoundData.SNDReload);// play order changed else no VOIReammo-sound - nonsinn
		PlayVoice (VoiceData.VOIReammo);
	});

	clientSignalConnectionManager.connect (client.unitRepaired, [&](const cUnit&)
	{
		PlayFX (SoundData.SNDRepair);// play order changed else no VOIRepaired-sound - nonsinn
		PlayRandomVoice (VoiceData.VOIRepaired);
	});

	clientSignalConnectionManager.connect (client.unitDisabled, [&](const cUnit& unit)
	{
		//const std::string msg = Vehicle->getDisplayName () + " " + lngPack.i18n ("Text~Comp~Disabled");
		//const sSavedReportMessage& report = ActivePlayer->addSavedReport (msg, sSavedReportMessage::REPORT_TYPE_UNIT, Vehicle->data.ID, Vehicle->PosX, Vehicle->PosY);
		//gameGUI->addCoords (report);
		//PlayVoice (VoiceData.VOIUnitDisabled);

		//const std::string msg = Building->getDisplayName() + " " + lngPack.i18n ("Text~Comp~Disabled");
		//const sSavedReportMessage& report = ActivePlayer->addSavedReport (msg, sSavedReportMessage::REPORT_TYPE_UNIT, Building->data.ID, Building->PosX, Building->PosY);
		//gameGUI->addCoords (report);
		PlayVoice (VoiceData.VOIUnitDisabled);
	});

	clientSignalConnectionManager.connect (client.unitStolen, [&](const cUnit& unit)
	{
		//const std::string msg = lngPack.i18n ("Text~Comp~CapturedByEnemy", unit.getDisplayName ());
		//const sSavedReportMessage& report = getActivePlayer ().addSavedReport (msg, sSavedReportMessage::REPORT_TYPE_UNIT, unit.data.ID, unit.PosX, unit.PosY);
		//gameGUI->addCoords (report);

		PlayVoice (VoiceData.VOIUnitStolenByEnemy);
	});

	clientSignalConnectionManager.connect (client.unitDetected, [&](const cUnit& unit)
	{
		// make report
		//const string message = addedVehicle.getDisplayName () + " (" + addedVehicle.owner->getName () + ") " + lngPack.i18n ("Text~Comp~Detected");
		//const sSavedReportMessage& report = getActivePlayer ().addSavedReport (message, sSavedReportMessage::REPORT_TYPE_UNIT, addedVehicle.data.ID, iPosX, iPosY);
		//gameGUI->addCoords (report);

		if (unit.data.isStealthOn & TERRAIN_SEA && unit.data.canAttack)
		{
			PlayVoice (VoiceData.VOISubDetected);
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
void cNewGameGUI::connectMoveJob (const cVehicle& vehicle)
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
				const auto building = dynamicMap->getField (cPosition (vehicle.PosX, vehicle.PosY)).getBaseBuilding ();
				bool water = dynamicMap->isWater (vehicle.PosX, vehicle.PosY);
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
				
				bool wasWater = dynamicMap->isWater (vehicle.ClientMoveJob->Waypoints->X, vehicle.ClientMoveJob->Waypoints->Y);
				bool water = dynamicMap->isWater (vehicle.ClientMoveJob->Waypoints->next->X, vehicle.ClientMoveJob->Waypoints->next->Y);

				if (wasWater != water)
				{
					updateSelectedUnitMoveSound ();
				}
			}
		});
	}
}

//------------------------------------------------------------------------------
void cNewGameGUI::disconnectCurrentClient ()
{
	clientSignalConnectionManager.disconnectAll ();
}

//------------------------------------------------------------------------------
void cNewGameGUI::draw ()
{
	animationTimer->updateAnimationFlags (); // TODO: remove this

	cWindow::draw ();
}

//------------------------------------------------------------------------------
bool cNewGameGUI::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	const auto currentMousePosition = mouse.getPosition ();
	const auto mouseLastPosition = currentMousePosition - offset;
	if (hud->isAt (currentMousePosition) && !hud->isAt (mouseLastPosition))
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

	if (mouseScrollDirection.x () > 0 &&  mouseScrollDirection.y () == 0) mouse.setCursorType (eMouseCursorType::ArrowRight);
	else if (mouseScrollDirection.x () < 0 &&  mouseScrollDirection.y () == 0) mouse.setCursorType (eMouseCursorType::ArrowLeft);
	else if (mouseScrollDirection.x () == 0 &&  mouseScrollDirection.y () > 0) mouse.setCursorType (eMouseCursorType::ArrowDown);
	else if (mouseScrollDirection.x () == 0 &&  mouseScrollDirection.y () < 0) mouse.setCursorType (eMouseCursorType::ArrowUp);
	else if (mouseScrollDirection.x () > 0 &&  mouseScrollDirection.y () > 0) mouse.setCursorType (eMouseCursorType::ArrowRightDown);
	else if (mouseScrollDirection.x () > 0 &&  mouseScrollDirection.y () < 0) mouse.setCursorType (eMouseCursorType::ArrowRightUp);
	else if (mouseScrollDirection.x () < 0 &&  mouseScrollDirection.y () > 0) mouse.setCursorType (eMouseCursorType::ArrowLeftDown);
	else if (mouseScrollDirection.x () < 0 &&  mouseScrollDirection.y () < 0) mouse.setCursorType (eMouseCursorType::ArrowLeftUp);
	else if (hud->isAt (currentMousePosition)) mouse.setCursorType (eMouseCursorType::Hand);

	return cWindow::handleMouseMoved (application, mouse, offset);
}

//------------------------------------------------------------------------------
bool cNewGameGUI::handleMouseWheelMoved (cApplication& application, cMouse& mouse, const cPosition& amount)
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
bool cNewGameGUI::handleKeyPressed (cApplication& application, cKeyboard& keyboard, SDL_Keycode key)
{
	if (key == KeysList.KeyScroll1)
	{
		gameMap->scroll (cPosition (-cSettings::getInstance ().getScrollSpeed (), -cSettings::getInstance ().getScrollSpeed ()));
		return true;
	}
	if (key == KeysList.KeyScroll3)
	{
		gameMap->scroll (cPosition (+cSettings::getInstance ().getScrollSpeed (), -cSettings::getInstance ().getScrollSpeed ()));
		return true;
	}
	if (key == KeysList.KeyScroll7)
	{
		gameMap->scroll (cPosition (-cSettings::getInstance ().getScrollSpeed (), +cSettings::getInstance ().getScrollSpeed ()));
		return true;
	}
	if (key == KeysList.KeyScroll9)
	{
		gameMap->scroll (cPosition (+cSettings::getInstance ().getScrollSpeed (), +cSettings::getInstance ().getScrollSpeed ()));
		return true;
	}
	if (key == KeysList.KeyScroll2a || key == KeysList.KeyScroll2b)
	{
		gameMap->scroll (cPosition (0, +cSettings::getInstance ().getScrollSpeed ()));
		return true;
	}
	if (key == KeysList.KeyScroll4a || key == KeysList.KeyScroll4b)
	{
		gameMap->scroll (cPosition (-cSettings::getInstance ().getScrollSpeed (), 0));
		return true;
	}
	if (key == KeysList.KeyScroll6a || key == KeysList.KeyScroll6b)
	{
		gameMap->scroll (cPosition (+cSettings::getInstance ().getScrollSpeed (), 0));
		return true;
	}
	if (key == KeysList.KeyScroll8a || key == KeysList.KeyScroll8b)
	{
		gameMap->scroll (cPosition (0, -cSettings::getInstance ().getScrollSpeed ()));
		return true;
	}
	return cWindow::handleKeyPressed(application, keyboard, key);
}

//------------------------------------------------------------------------------
void cNewGameGUI::handleLooseMouseFocus (cApplication& application)
{

}

//------------------------------------------------------------------------------
void cNewGameGUI::resetMiniMapViewWindow ()
{
	miniMap->setViewWindow (gameMap->getDisplayedMapArea ());
}

//------------------------------------------------------------------------------
void cNewGameGUI::showFilesMenu ()
{

}

//------------------------------------------------------------------------------
void cNewGameGUI::showPreferencesDialog ()
{
	auto application = getActiveApplication ();
	if (!application) return;

	auto preferencesDialog = std::make_shared<cDialogNewPreferences> ();
	application->show (preferencesDialog);
}

//------------------------------------------------------------------------------
void cNewGameGUI::updateSelectedUnitIdleSound ()
{
	auto selectedUnit = gameMap->getUnitSelection().getSelectedUnit ();
	if (selectedUnit == nullptr)
	{
		stopSelectedUnitSound ();
	}
	else if (selectedUnit->data.ID.isABuilding ())
	{
		const auto& building = static_cast<const cBuilding&>(*selectedUnit);
		if (building.IsWorking)
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

		const cBuilding* building = dynamicMap ? dynamicMap->getField (cPosition (vehicle.PosX, vehicle.PosY)).getBaseBuilding () : nullptr;
		bool water = staticMap->isWater (vehicle.PosX, vehicle.PosY);
		if (vehicle.data.factorGround > 0 && building && (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)) water = false;

		if (vehicle.IsBuilding && (vehicle.BuildRounds || player != vehicle.owner))
		{
			startSelectedUnitSound (SoundData.SNDBuilding);
		}
		else if (vehicle.IsClearing)
		{
			startSelectedUnitSound (SoundData.SNDClearing);
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
void cNewGameGUI::updateSelectedUnitMoveSound ()
{
	auto selectedVehicle = gameMap->getUnitSelection ().getSelectedVehicle ();
	if (!selectedVehicle) return;
	if (!dynamicMap) return;

	const auto& vehicle = *selectedVehicle;

	const auto building = dynamicMap->getField (cPosition (vehicle.PosX, vehicle.PosY)).getBaseBuilding ();
	bool water = dynamicMap->isWater (vehicle.PosX, vehicle.PosY);
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
void cNewGameGUI::startSelectedUnitSound (sSOUND* sound)
{
	stopSelectedUnitSound ();
	selectedUnitSoundStream = PlayFXLoop (sound);
}

//------------------------------------------------------------------------------
void cNewGameGUI::stopSelectedUnitSound ()
{
	if (selectedUnitSoundStream != -1) StopFXLoop (selectedUnitSoundStream);
	selectedUnitSoundStream = -1;
}