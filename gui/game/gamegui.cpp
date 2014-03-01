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

#include "gamegui.h"
#include "hud.h"
#include "gamemapwidget.h"
#include "minimapwidget.h"
#include "unitcontextmenuwidget.h"

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
#include "../../input/mouse/mouse.h"

//------------------------------------------------------------------------------
cNewGameGUI::cNewGameGUI (std::shared_ptr<const cStaticMap> staticMap_) :
	cWindow (nullptr),
	animationTimer (std::make_shared<cAnimationTimer>()),
	staticMap (std::move (staticMap_)),
	dynamicMap (nullptr),
	selectedUnitSoundStream (-1)
{
	auto hudOwning = std::make_unique<cHud> (animationTimer);

	resize (hudOwning->getSize ());

	gameMap = addChild (std::make_unique<cGameMapWidget> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, cHud::panelTopHeight), getEndPosition () - cPosition (cHud::panelRightWidth, cHud::panelBottomHeight)), staticMap, animationTimer));

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

	signalConnectionManager.connect (gameMap->scrolled, std::bind(&cNewGameGUI::resetMiniMapViewWindow, this));
	signalConnectionManager.connect (gameMap->zoomFactorChanged, std::bind (&cNewGameGUI::resetMiniMapViewWindow, this));
	signalConnectionManager.connect (gameMap->tileUnderMouseChanged, std::bind (&cNewGameGUI::updateHudCoordinates, this, _1));
	signalConnectionManager.connect (gameMap->tileUnderMouseChanged, std::bind (&cNewGameGUI::updateHudUnitName, this, _1));

	signalConnectionManager.connect (gameMap->getUnitSelection ().selectionChanged, [&](){ hud->setActiveUnit (gameMap->getUnitSelection ().getSelectedUnit ()); });
	signalConnectionManager.connect (gameMap->getUnitSelection ().selectionChanged, std::bind (&cNewGameGUI::updateSelectedUnitIdleSound, this));

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
		//dynamicMapSignalConnectionManager.connect (dynamicMap->movedVehicle, [&](const cVehicle&){ surfaceOutdated = true; });
	}
}

//------------------------------------------------------------------------------
void cNewGameGUI::setPlayer (const cPlayer* player_)
{
	player = player_;
	gameMap->setPlayer (player);
	miniMap->setPlayer (player);
}

//------------------------------------------------------------------------------
void cNewGameGUI::connectToClient (cClient& client)
{
	//
	// GUI to client (action)
	//
	signalConnectionManager.connect (gameMap->triggeredStartWork, [&](const cUnit& unit)
	{
		sendWantStartWork (client, unit);
	});
	signalConnectionManager.connect (gameMap->triggeredStopWork, [&](const cUnit& unit)
	{
		unit.executeStopCommand (client);
	});
	signalConnectionManager.connect (gameMap->triggeredAutoMoveJob, [&](const cUnit& unit)
	{
		//if (unit.data.ID.isAVehicle ()) static_cast<const cVehicle&>(unit).executeAutoMoveJobCommand (client);
	});
	signalConnectionManager.connect (gameMap->triggeredStartClear, [&](const cUnit& unit)
	{
		if (unit.data.ID.isAVehicle ()) sendWantStartClear (client, static_cast<const cVehicle&> (unit));
	});
	signalConnectionManager.connect (gameMap->triggeredManualFire, [&](const cUnit& unit)
	{
		sendChangeManualFireStatus (client, unit.iID, unit.isAVehicle ());
	});
	signalConnectionManager.connect (gameMap->triggeredSentry, [&](const cUnit& unit)
	{
		sendChangeSentry (client, unit.iID, unit.isAVehicle ());
	});
	signalConnectionManager.connect (gameMap->triggeredUpgradeThis, [&](const cUnit& unit)
	{
		if (unit.data.ID.isABuilding ()) static_cast<const cBuilding&>(unit).executeUpdateBuildingCommmand (client, false);
	});
	signalConnectionManager.connect (gameMap->triggeredUpgradeAll, [&](const cUnit& unit)
	{
		if (unit.data.ID.isABuilding ()) static_cast<const cBuilding&>(unit).executeUpdateBuildingCommmand (client, true);
	});
	signalConnectionManager.connect (gameMap->triggeredLayMines, [&](const cUnit& unit)
	{
		//if (unit.data.ID.isAVehicle ()) static_cast<const cVehicle&>(unit).executeLayMinesCommand (client);
	});
	signalConnectionManager.connect (gameMap->triggeredCollectMines, [&](const cUnit& unit)
	{
		//if (unit.data.ID.isAVehicle ()) static_cast<const cVehicle&>(unit).executeClearMinesCommand (client);
	});
	signalConnectionManager.connect (gameMap->triggeredUnitDone, [&](const cUnit& unit)
	{
		//if (unit.owner == player)
		//{
		//	unit.isMarkedAsDone = true;
		//	sendMoveJobResume (client, unit.iID);
		//}
	});
	clientSignalConnectionManager.connect(gameMap->triggeredMoveSingle, [&](cVehicle& vehicle, const cPosition& destination)
	{
		client.addMoveJob (vehicle, destination.x (), destination.y ());
	});


	//
	// client to GUI (reaction)
	//
	clientSignalConnectionManager.connect (client.turnChanged, [&]()
	{
	});

	clientSignalConnectionManager.connect (client.startTurnEnd, [&]()
	{
	});

	clientSignalConnectionManager.connect (client.finishTurnEnd, [&]()
	{
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
		//gameGUI->updateMouseCursor();
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
		//PlayVoice (VoiceData.VOIUnitDisabled);
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
			PlayVoice (VoiceData.VOISubDetected);
		else
			PlayRandomVoice (VoiceData.VOIDetected);
	});

	clientSignalConnectionManager.connect (client.moveJobCreated, [&](const cVehicle& unit)
	{
		connectMoveJob (unit);
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
	const auto mouseLastPosition = mouse.getPosition () - offset;
	if (!gameMap->isAt (mouse.getPosition ()) && gameMap->isAt (mouseLastPosition))
	{
		mouse.setCursorType (eMouseCursorType::Hand);
		hud->setCoordinatesText ("");
		hud->setUnitNameText ("");
	}
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