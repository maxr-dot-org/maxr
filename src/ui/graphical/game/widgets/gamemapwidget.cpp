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

#include "ui/graphical/game/widgets/gamemapwidget.h"
#include "ui/graphical/game/widgets/unitcontextmenuwidget.h"
#include "ui/graphical/game/widgets/mousemode/mousemode.h"
#include "ui/graphical/game/widgets/mousemode/mousemodedefault.h"
#include "ui/graphical/game/widgets/mousemode/mousemodeactivateloaded.h"
#include "ui/graphical/game/widgets/mousemode/mousemodeattack.h"
#include "ui/graphical/game/widgets/mousemode/mousemodedisable.h"
#include "ui/graphical/game/widgets/mousemode/mousemodehelp.h"
#include "ui/graphical/game/widgets/mousemode/mousemodeload.h"
#include "ui/graphical/game/widgets/mousemode/mousemoderepair.h"
#include "ui/graphical/game/widgets/mousemode/mousemodeselectbuildpathdestination.h"
#include "ui/graphical/game/widgets/mousemode/mousemodeselectbuildposition.h"
#include "ui/graphical/game/widgets/mousemode/mousemodesteal.h"
#include "ui/graphical/game/widgets/mousemode/mousemodesupplyammo.h"
#include "ui/graphical/game/widgets/mousemode/mousemodetransfer.h"
#include "ui/graphical/game/widgets/mouseaction/mouseaction.h"
#include "ui/graphical/game/animations/animation.h"
#include "ui/graphical/game/animations/animationwork.h"
#include "ui/graphical/game/animations/animationstartup.h"
#include "ui/graphical/game/animations/animationstartupbuildingsite.h"
#include "ui/graphical/game/animations/animationdither.h"
#include "ui/graphical/game/hud.h"
#include "ui/sound/soundmanager.h"
#include "ui/graphical/game/animations/animationtimer.h"
#include "ui/graphical/application.h"
#include "game/data/map/map.h"
#include "settings.h"
#include "video.h"
#include "main.h"
#include "game/data/player/player.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"
#include "keys.h"
#include "utility/listhelpers.h"
#include "game/logic/attackjobs.h"
#include "sound.h"
#include "game/logic/movejobs.h"
#include "utility/indexiterator.h"
#include "utility/random.h"
#include "input/mouse/mouse.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "input/mouse/cursor/mousecursoramount.h"
#include "input/mouse/cursor/mousecursorattack.h"
#include "output/sound/sounddevice.h"
#include "output/sound/soundchannel.h"

//------------------------------------------------------------------------------
cGameMapWidget::cGameMapWidget (const cBox<cPosition>& area, std::shared_ptr<const cStaticMap> staticMap_, std::shared_ptr<cAnimationTimer> animationTimer_, std::shared_ptr<cSoundManager> soundManager_) :
	cClickableWidget (area),
	animationTimer (animationTimer_),
	soundManager (soundManager_),
	staticMap (std::move (staticMap_)),
	dynamicMap (nullptr),
	player (nullptr),
	unitDrawingEngine (animationTimer),
	pixelOffset (0, 0),
	internalZoomFactor (1.f),
	shouldDrawSurvey (false),
	shouldDrawScan (false),
	shouldDrawGrid (false),
	shouldDrawRange (false),
	shouldDrawFog (false),
	lockActive (false)
{
	assert (staticMap != nullptr);
	assert (animationTimer != nullptr);
	assert (soundManager != nullptr);

	signalConnectionManager.connect (cSettings::getInstance ().animationsChanged, [this]()
	{
		if (cSettings::getInstance ().isAnimations ())
		{
			updateActiveAnimations ();
		}
		else
		{
			animations.clear ();
		}
	});

	setMouseInputMode (std::make_unique<cMouseModeDefault> (dynamicMap.get (), unitSelection, player.get ()));

	// TODO: should this really be done here?
	signalConnectionManager.connect (animationTimer->triggered400ms, [&]()
	{
		const_cast<cStaticMap&>(*staticMap).generateNextAnimationFrame ();
	});

	setWindDirection (random (360));
	signalConnectionManager.connect (animationTimer->triggered400ms, std::bind (&cGameMapWidget::changeWindDirection, this));

	signalConnectionManager.connect (animationTimer->triggered50ms, std::bind (&cGameMapWidget::runOwnedEffects, this));
	signalConnectionManager.connect (animationTimer->triggered100ms, std::bind (&cGameMapWidget::renewDamageEffects, this));

	unitMenu = addChild (std::make_unique<cUnitContextMenuWidget> ());
	unitMenu->disable ();
	unitMenu->hide ();

	mouseInputModeChanged.connect (std::bind (static_cast<void (cGameMapWidget::*)()>(&cGameMapWidget::updateMouseCursor), this));

	scrolled.connect (std::bind (&cGameMapWidget::updateUnitMenuPosition, this));
	scrolled.connect ([this]()
	{
		soundManager->setListenerPosition (getMapCenterOffset ());
	});
	tileUnderMouseChanged.connect ([this](const cPosition& tilePosition)
	{
		if (mouseMode)
		{
			mouseMode->handleMapTilePositionChanged (tilePosition);
		}
	});

	zoomFactorChanged.connect ([this]()
	{
		const auto tileDrawingRange = computeTileDrawingRange ();
		const cPosition difference = tileDrawingRange.first - tileDrawingRange.second;
		const auto diameter = difference.l2Norm ();
		soundManager->setMaxListeningDistance ((int)(diameter * 2));
	});

	unitSelection.mainSelectionChanged.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitSelection.mainSelectionChanged.connect ([&]()
	{
		setMouseInputMode (std::make_unique<cMouseModeDefault> (dynamicMap.get (), unitSelection, player.get ()));
	});

	unitMenu->attackToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::Attack));
	unitMenu->transferToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::Transfer));
	unitMenu->loadToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::Load));
	unitMenu->supplyAmmoToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::SupplyAmmo));
	unitMenu->repairToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::Repair));
	unitMenu->sabotageToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::Disable));
	unitMenu->stealToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::Steal));

	unitMenu->buildClicked.connect ([&](){ if (unitMenu->getUnit ()) triggeredBuild (*unitMenu->getUnit ()); });
	unitMenu->distributeClicked.connect ([&](){ if (unitMenu->getUnit ()) triggeredResourceDistribution (*unitMenu->getUnit ()); });
	unitMenu->startClicked.connect ([&](){ if (unitMenu->getUnit ()) triggeredStartWork (*unitMenu->getUnit ()); });
	unitMenu->stopClicked.connect ([&](){ if (unitMenu->getUnit ()) triggeredStopWork (*unitMenu->getUnit ()); });
	unitMenu->autoToggled.connect ([&](){ if (unitMenu->getUnit ()) triggeredAutoMoveJob (*unitMenu->getUnit ()); });
	unitMenu->removeClicked.connect ([&](){ if (unitMenu->getUnit ()) triggeredStartClear (*unitMenu->getUnit ()); });
	unitMenu->manualFireToggled.connect ([&](){ if (unitMenu->getUnit ()) triggeredManualFire (*unitMenu->getUnit ()); });
	unitMenu->sentryToggled.connect ([&](){ if (unitMenu->getUnit ()) triggeredSentry (*unitMenu->getUnit ()); });
	unitMenu->activateClicked.connect ([&](){ if (unitMenu->getUnit ()) triggeredActivate (*unitMenu->getUnit ()); });
	unitMenu->researchClicked.connect ([&](){ if (unitMenu->getUnit ()) triggeredResearchMenu (*unitMenu->getUnit ()); });
	unitMenu->buyUpgradesClicked.connect ([&](){ if (unitMenu->getUnit ()) triggeredUpgradesMenu (*unitMenu->getUnit ()); });
	unitMenu->upgradeThisClicked.connect ([&](){ if (unitMenu->getUnit ()) triggeredUpgradeThis (*unitMenu->getUnit ()); });
	unitMenu->upgradeAllClicked.connect ([&](){ if (unitMenu->getUnit ()) triggeredUpgradeAll (*unitMenu->getUnit ()); });
	unitMenu->selfDestroyClicked.connect ([&](){ if (unitMenu->getUnit ()) triggeredSelfDestruction (*unitMenu->getUnit ()); });
	unitMenu->layMinesToggled.connect ([&](){ if (unitMenu->getUnit ()) triggeredLayMines (*unitMenu->getUnit ()); });
	unitMenu->collectMinesToggled.connect ([&](){ if (unitMenu->getUnit ()) triggeredCollectMines (*unitMenu->getUnit ()); });
	unitMenu->infoClicked.connect ([&](){ if (unitMenu->getUnit ()) triggeredUnitHelp (*unitMenu->getUnit ()); });
	unitMenu->doneClicked.connect ([&](){ if (unitMenu->getUnit ()) triggeredUnitDone (*unitMenu->getUnit ()); });

	unitMenu->attackToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->buildClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->distributeClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->transferToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->startClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->autoToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->stopClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->removeClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->manualFireToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->sentryToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->activateClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->loadToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->researchClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->buyUpgradesClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->upgradeThisClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->upgradeAllClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->selfDestroyClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->supplyAmmoToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->repairToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->layMinesToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->collectMinesToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->sabotageToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->stealToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->infoClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->doneClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));

	auto attackShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuAttack));
	attackShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.canAttack && unit->data.getShots ())
		{
			toggleMouseInputMode (eMouseModeType::Attack);
		}
	});

	auto buildShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuBuild));
	buildShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.canBuild.empty () == false && unit->isUnitBuildingABuilding () == false)
		{
			triggeredBuild (*unitSelection.getSelectedUnit ());
		}
	});

	auto transferShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuTransfer));
	transferShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.storeResType != sUnitData::STORE_RES_NONE && unit->isUnitBuildingABuilding () == false && unit->isUnitClearing () == false)
		{
			toggleMouseInputMode (eMouseModeType::Transfer);
		}
	});

	auto automoveShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuAutomove));
	automoveShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.canSurvey)
		{
			triggeredAutoMoveJob (*unitSelection.getSelectedUnit ());
		}
	});

	auto startShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuStart));
	startShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.canWork && unit->buildingCanBeStarted ())
		{
			triggeredStartWork (*unitSelection.getSelectedUnit ());
		}
	});

	auto stopShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuStop));
	stopShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->canBeStoppedViaUnitMenu ())
		{
			triggeredStopWork (*unitSelection.getSelectedUnit ());
		}
	});

	auto clearShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuClear));
	clearShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.canClearArea && dynamicMap && dynamicMap->getField (unit->getPosition ()).getRubble () && unit->isUnitClearing () == false)
		{
			triggeredStartClear (*unitSelection.getSelectedUnit ());
		}
	});

	auto sentryShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuSentry));
	sentryShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && (unit->isSentryActive () || unit->data.canAttack || (!unit->isABuilding () && !unit->canBeStoppedViaUnitMenu ())))
		{
			triggeredSentry (*unitSelection.getSelectedUnit ());
		}
	});

	auto manualFireShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuManualFire));
	manualFireShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && (unit->isManualFireActive () || unit->data.canAttack))
		{
			triggeredManualFire (*unitSelection.getSelectedUnit ());
		}
	});

	auto activateShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuActivate));
	activateShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.storageUnitsMax > 0)
		{
			triggeredActivate (*unitSelection.getSelectedUnit ());
		}
	});

	auto loadShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuLoad));
	loadShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.storageUnitsMax > 0)
		{
			toggleMouseInputMode (eMouseModeType::Load);
		}
	});

	auto relaodShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuReload));
	relaodShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.canRearm && unit->data.getStoredResources () >= 1)
		{
			toggleMouseInputMode (eMouseModeType::SupplyAmmo);
		}
	});

	auto repairShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuRepair));
	repairShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.canRepair && unit->data.getStoredResources () >= 1)
		{
			toggleMouseInputMode (eMouseModeType::Repair);
		}
	});

	auto layMineShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuLayMine));
	layMineShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.canPlaceMines && unit->data.getStoredResources () > 0)
		{
			triggeredLayMines (*unitSelection.getSelectedUnit ());
		}
	});

	auto clearMineShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuClearMine));
	clearMineShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.canPlaceMines && unit->data.getStoredResources () < unit->data.storageResMax)
		{
			triggeredCollectMines (*unitSelection.getSelectedUnit ());
		}
	});

	auto disableShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuDisable));
	disableShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.canDisable && unit->data.getShots ())
		{
			toggleMouseInputMode (eMouseModeType::Disable);
		}
	});

	auto stealShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuSteal));
	stealShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.canCapture && unit->data.getShots ())
		{
			toggleMouseInputMode (eMouseModeType::Steal);
		}
	});

	auto infoShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuInfo));
	infoShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit)
		{
			triggeredUnitHelp (*unitSelection.getSelectedUnit ());
		}
	});

	auto distributeShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuDistribute));
	distributeShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.canMineMaxRes > 0 && unit->isUnitWorking ())
		{
			triggeredResourceDistribution (*unitSelection.getSelectedUnit ());
		}
	});

	auto researchShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuResearch));
	researchShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.canResearch && unit->isUnitWorking ())
		{
			triggeredResearchMenu (*unitSelection.getSelectedUnit ());
		}
	});

	auto upgradeShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuUpgrade));
	upgradeShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.convertsGold)
		{
			triggeredUpgradesMenu (*unitSelection.getSelectedUnit ());
		}
	});

	auto destroyShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuDestroy));
	destroyShortcut->triggered.connect ([this]()
	{
		auto unit = unitSelection.getSelectedUnit ();
		if (unit && !unit->isDisabled () && unit->owner == player.get () && unit->data.canSelfDestroy)
		{
			triggeredSelfDestruction (*unitSelection.getSelectedUnit ());
		}
	});
}

//------------------------------------------------------------------------------
cGameMapWidget::~cGameMapWidget()
{
}

//------------------------------------------------------------------------------
void cGameMapWidget::setDynamicMap (std::shared_ptr<const cMap> dynamicMap_)
{
	std::swap(dynamicMap, dynamicMap_);

	dynamicMapSignalConnectionManager.disconnectAll ();

	if (dynamicMap != nullptr)
	{
		dynamicMapSignalConnectionManager.connect (dynamicMap->removedUnit, [&](const cUnit& unit)
		{
			if (unitSelection.isSelected (unit))
			{
				unitSelection.deselectUnit (unit);
			}
		});
		dynamicMapSignalConnectionManager.connect (dynamicMap->addedUnit, [&](const cUnit& unit)
		{
			if (!cSettings::getInstance ().isAnimations ()) return;

			const auto tileDrawingRange = computeTileDrawingRange ();
			const auto tileDrawingArea = cBox<cPosition> (tileDrawingRange.first, tileDrawingRange.second - cPosition (1, 1));

			if (tileDrawingArea.intersects (unit.getArea ()))
			{
				addAnimationsForUnit (unit);
				animations.push_back (std::make_unique<cAnimationStartUp> (*animationTimer, unit));
			}
		});
		dynamicMapSignalConnectionManager.connect (dynamicMap->movedVehicle, [&](const cUnit& unit, const cPosition& oldPosition)
		{
			if (!cSettings::getInstance ().isAnimations ()) return;

			const auto tileDrawingRange = computeTileDrawingRange ();
			const auto tileDrawingArea = cBox<cPosition> (tileDrawingRange.first, tileDrawingRange.second - cPosition (1, 1));

			if (tileDrawingArea.intersects (unit.getArea ()) && !tileDrawingArea.intersects (cBox<cPosition>(oldPosition, oldPosition + unit.getArea().getSize() - cPosition(1,1))))
			{
				addAnimationsForUnit (unit);
			}
		});
	}

	if (mouseMode != nullptr)
	{
		mouseMode->setMap (dynamicMap.get ());
	}

	if (dynamicMap != dynamicMap_)
	{
		unitSelection.deselectUnits ();
	}

	updateActiveAnimations ();
}

//------------------------------------------------------------------------------
void cGameMapWidget::setPlayer (std::shared_ptr<const cPlayer> player_)
{
	player = std::move (player_);

	unitLockList.setPlayer (player.get ());

	if (mouseMode != nullptr)
	{
		mouseMode->setPlayer (player.get ());
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::draw ()
{
	drawTerrain();

	if (shouldDrawGrid) drawGrid ();

	drawEffects (true);

	//dCache.resetStatistics ();

	drawBaseUnits ();
	drawTopBuildings ();
	drawShips ();
	drawAboveSeaBaseUnits ();
	drawVehicles ();
	drawConnectors ();
	drawPlanes ();

	auto selectedVehicle = unitSelection.getSelectedVehicle ();
	if (shouldDrawSurvey || (selectedVehicle && selectedVehicle->owner == player.get() && selectedVehicle->data.canSurvey))
	{
		drawResources ();
	}

	if (selectedVehicle && ((selectedVehicle->getClientMoveJob () && selectedVehicle->getClientMoveJob ()->bSuspended) || selectedVehicle->BuildPath))
	{
		drawPath (*selectedVehicle);
	}

	//debugOutput.draw ();

	drawSelectionBox ();

	SDL_SetClipRect (cVideo::buffer, nullptr);

	drawUnitCircles ();
	drawExitPoints ();
	drawBuildBand ();

	if (lockActive && player) drawLockList (*player);

	drawEffects (false);

	//displayMessages ();

	cWidget::draw ();
}

//------------------------------------------------------------------------------
void cGameMapWidget::setZoomFactor (float zoomFactor_, bool center)
{
	const auto oldZoom = getZoomFactor ();
	const auto oldTileDrawingRange = computeTileDrawingRange ();

	internalZoomFactor = zoomFactor_;

	internalZoomFactor = std::max (internalZoomFactor, computeMinimalZoomFactor ());
	internalZoomFactor = std::min (1.f, internalZoomFactor);

	const auto newZoom = getZoomFactor ();

	if (oldZoom != newZoom)
	{
		updateActiveAnimations (oldTileDrawingRange);

		zoomFactorChanged ();

		cPosition scrollOffset (0, 0);
		if (center)
		{
			const auto oldScreenPixelX = getSize ().x () / oldZoom;
			const auto newScreenPixelX = getSize ().x () / newZoom;
			scrollOffset.x () = (int)((oldScreenPixelX - newScreenPixelX) / 2);

			const auto oldScreenPixelY = getSize ().y () / oldZoom;
			const auto newScreenPixelY = getSize ().y () / newZoom;
			scrollOffset.y () = (int)((oldScreenPixelY - newScreenPixelY) / 2);
		}

		// calling scroll here - even if scrollOffset = (0,0) - is important
		// because this will re-validate the offset.
		scroll (scrollOffset);
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::setDrawSurvey (bool shouldDrawSurvey_)
{
	shouldDrawSurvey = shouldDrawSurvey_;
}

//------------------------------------------------------------------------------
void cGameMapWidget::setDrawHits (bool shouldDrawHits)
{
	unitDrawingEngine.setDrawHits (shouldDrawHits);
}

//------------------------------------------------------------------------------
void cGameMapWidget::setDrawScan (bool shouldDrawScan_)
{
	shouldDrawScan = shouldDrawScan_;
}

//------------------------------------------------------------------------------
void cGameMapWidget::setDrawStatus (bool shouldDrawStatus)
{
	unitDrawingEngine.setDrawStatus (shouldDrawStatus);
}

//------------------------------------------------------------------------------
void cGameMapWidget::setDrawAmmo (bool shouldDrawAmmo)
{
	unitDrawingEngine.setDrawAmmo (shouldDrawAmmo);
}

//------------------------------------------------------------------------------
void cGameMapWidget::setDrawGrid (bool shouldDrawGrid_)
{
	shouldDrawGrid = shouldDrawGrid_;
}

//------------------------------------------------------------------------------
void cGameMapWidget::setDrawColor (bool shouldDrawColor)
{
	unitDrawingEngine.setDrawColor (shouldDrawColor);
}

//------------------------------------------------------------------------------
void cGameMapWidget::setDrawRange (bool shouldDrawRange_)
{
	shouldDrawRange = shouldDrawRange_;
}

//------------------------------------------------------------------------------
void cGameMapWidget::setDrawFog (bool drawFog_)
{
	shouldDrawFog = drawFog_;
}

//------------------------------------------------------------------------------
void cGameMapWidget::setLockActive (bool lockActive_)
{
	lockActive = lockActive_;
}

//------------------------------------------------------------------------------
void cGameMapWidget::toggleHelpMode ()
{
	toggleMouseInputMode (eMouseModeType::Help);
}

//------------------------------------------------------------------------------
cBox<cPosition> cGameMapWidget::getDisplayedMapArea () const
{
	auto tileDrawingRange = computeTileDrawingRange ();

	return cBox<cPosition> (tileDrawingRange.first, tileDrawingRange.second-1);
}

//------------------------------------------------------------------------------
float cGameMapWidget::getZoomFactor () const
{
	return (float)getZoomedTileSize ().x () / cStaticMap::tilePixelWidth; // should make no difference if we use y instead
}

//------------------------------------------------------------------------------
cUnitSelection& cGameMapWidget::getUnitSelection ()
{
	return unitSelection;
}

//------------------------------------------------------------------------------
const cUnitSelection& cGameMapWidget::getUnitSelection () const
{
	return unitSelection;
}

//------------------------------------------------------------------------------
cUnitLockList& cGameMapWidget::getUnitLockList ()
{
	return unitLockList;
}

//------------------------------------------------------------------------------
const cUnitLockList& cGameMapWidget::getUnitLockList () const
{
	return unitLockList;
}

//------------------------------------------------------------------------------
void cGameMapWidget::toggleUnitContextMenu (const cUnit* unit)
{
	unitContextMenuSignalConnectionManager.disconnectAll ();
	if (unitMenu->isEnabled () || unit == nullptr)
	{
		unitMenu->disable ();
		unitMenu->hide ();
		updateMouseCursor ();
	}
	else
	{
		unitMenu->setUnit (unit, mouseMode->getType (), player.get (), dynamicMap.get ());
		unitMenu->enable ();
		unitMenu->show ();
		updateUnitMenuPosition ();

		unitContextMenuSignalConnectionManager.connect (unit->positionChanged, [this,unit]()
		{
			toggleUnitContextMenu (unit);
		});
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::setMouseInputMode (std::unique_ptr<cMouseMode> newMouseMode)
{
	assert (newMouseMode != nullptr);

	std::swap (newMouseMode, mouseMode);

	mouseModeSignalConnectionManager.disconnectAll ();
	mouseModeSignalConnectionManager.connect (mouseMode->needRefresh, std::bind (static_cast<void(cGameMapWidget::*)()>(&cGameMapWidget::updateMouseCursor), this));

	auto activeMouse = getActiveMouse ();
	if (activeMouse && getArea().withinOrTouches(activeMouse->getPosition()))
	{
		mouseMode->handleMapTilePositionChanged (getMapTilePosition (activeMouse->getPosition ()));
	}
	else
	{
		mouseMode->handleMapTilePositionChanged (cPosition (-1, -1));
	}

	if (!newMouseMode || newMouseMode->getType () != mouseMode->getType ()) mouseInputModeChanged ();
}

//------------------------------------------------------------------------------
void cGameMapWidget::toggleMouseInputMode (eMouseModeType mouseModeType)
{
	if (mouseMode->getType () == mouseModeType)
	{
		setMouseInputMode (std::make_unique<cMouseModeDefault> (dynamicMap.get (), unitSelection, player.get ()));
	}
	else
	{
		switch (mouseModeType)
		{
		case eMouseModeType::SelectBuildPosition:
		case eMouseModeType::Activate:
			assert (false);
			// fall through
		default:
		case eMouseModeType::Default:
			setMouseInputMode (std::make_unique<cMouseModeDefault> (dynamicMap.get (), unitSelection, player.get ()));
			break;
		case eMouseModeType::Attack:
			setMouseInputMode (std::make_unique<cMouseModeAttack> (dynamicMap.get (), unitSelection, player.get ()));
			break;
		case eMouseModeType::SelectBuildPathDestintaion:
			setMouseInputMode (std::make_unique<cMouseModeSelectBuildPathDestination> (dynamicMap.get (), unitSelection, player.get ()));
			break;
		case eMouseModeType::Transfer:
			setMouseInputMode (std::make_unique<cMouseModeTransfer> (dynamicMap.get (), unitSelection, player.get ()));
			break;
		case eMouseModeType::Load:
			setMouseInputMode (std::make_unique<cMouseModeLoad> (dynamicMap.get (), unitSelection, player.get ()));
			break;
		case eMouseModeType::SupplyAmmo:
			setMouseInputMode (std::make_unique<cMouseModeSupplyAmmo> (dynamicMap.get (), unitSelection, player.get ()));
			break;
		case eMouseModeType::Repair:
			setMouseInputMode (std::make_unique<cMouseModeRepair> (dynamicMap.get (), unitSelection, player.get ()));
			break;
		case eMouseModeType::Disable:
			setMouseInputMode (std::make_unique<cMouseModeDisable> (dynamicMap.get (), unitSelection, player.get ()));
			break;
		case eMouseModeType::Steal:
			setMouseInputMode (std::make_unique<cMouseModeSteal> (dynamicMap.get (), unitSelection, player.get ()));
			break;
		case eMouseModeType::Help:
			setMouseInputMode (std::make_unique<cMouseModeHelp> (dynamicMap.get (), unitSelection, player.get ()));
			break;
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::scroll (const cPosition& offset)
{
	const auto activeMouse = getActiveMouse ();

	cPosition oldTileUnderMouse(-1, -1);
	if (activeMouse && getArea().withinOrTouches(activeMouse->getPosition()))
	{
		oldTileUnderMouse = getMapTilePosition (activeMouse->getPosition ());
	}

	const auto oldPixelOffset = pixelOffset;
	const auto oldTileDrawingRange = computeTileDrawingRange ();

	pixelOffset += offset;

	pixelOffset.x () = std::max (0, pixelOffset.x ());
	pixelOffset.y () = std::max (0, pixelOffset.y ());

	const auto maximalPixelOffset = computeMaximalPixelOffset ();

	pixelOffset.x () = std::min (maximalPixelOffset.x (), pixelOffset.x ());
	pixelOffset.y () = std::min (maximalPixelOffset.y (), pixelOffset.y ());

	if (oldPixelOffset != pixelOffset)
	{
		cPosition newTileUnderMouse (-1, -1);
		if (activeMouse && getArea ().withinOrTouches (activeMouse->getPosition ()))
		{
			newTileUnderMouse = getMapTilePosition (activeMouse->getPosition ());
		}

		if (newTileUnderMouse != oldTileUnderMouse) tileUnderMouseChanged (newTileUnderMouse);

		updateActiveAnimations (oldTileDrawingRange);

		scrolled ();
	}
}

//------------------------------------------------------------------------------
const cPosition& cGameMapWidget::getPixelOffset () const
{
	return pixelOffset;
}

//------------------------------------------------------------------------------
void cGameMapWidget::centerAt (const cPosition& position)
{
	const auto zoomedTileSize = getZoomedTileSize ();

	cPosition newPixelOffset;
	newPixelOffset.x () = position.x () * cStaticMap::tilePixelWidth - ((int)(((float)getSize ().x () / (2 * zoomedTileSize.x ())) * cStaticMap::tilePixelWidth)) + cStaticMap::tilePixelWidth / 2;
	newPixelOffset.y () = position.y () * cStaticMap::tilePixelHeight - ((int)(((float)getSize ().y () / (2 * zoomedTileSize.y ())) * cStaticMap::tilePixelHeight)) + cStaticMap::tilePixelHeight / 2;

	scroll (newPixelOffset - pixelOffset);
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::getMapCenterOffset ()
{
    const auto zoomedTileSize = getZoomedTileSize ();
    
    cPosition center;
    center.x () = pixelOffset.x () / cStaticMap::tilePixelWidth + (getSize ().x () / (2 * zoomedTileSize.x ()));
    center.y () = pixelOffset.y () / cStaticMap::tilePixelHeight + (getSize ().y () / (2 * zoomedTileSize.y ()));

    return center;
}

//------------------------------------------------------------------------------
void cGameMapWidget::startFindBuildPosition (const sID& buildId)
{
	setMouseInputMode (std::make_unique<cMouseModeSelectBuildPosition> (dynamicMap.get (), unitSelection, player.get (), buildId));
}

//------------------------------------------------------------------------------
void cGameMapWidget::startFindPathBuildPosition ()
{
	setMouseInputMode (std::make_unique<cMouseModeSelectBuildPathDestination> (dynamicMap.get (), unitSelection, player.get ()));
}

//------------------------------------------------------------------------------
void cGameMapWidget::startActivateVehicle (const cUnit& unit, size_t index)
{
	setMouseInputMode (std::make_unique<cMouseModeActivateLoaded> (dynamicMap.get (), unitSelection, player.get (), index));
}

//------------------------------------------------------------------------------
void cGameMapWidget::addEffect (std::shared_ptr<cFx> effect)
{
	if (effect != nullptr)
	{
		effect->playSound (*soundManager);
		effects.push_back (std::move (effect));
	}
}

//------------------------------------------------------------------------------
float cGameMapWidget::computeMinimalZoomFactor () const
{
	// inequality to be fulfilled:
	//
	//   round(tile_x * zoom) * map_x <= size_x
	//
	// we first discard the 'round' and solve for zoom:
	//
	//   zoom = size_x / (map_x * tile_x)

	auto xZoom = (float)getSize ().x () / (staticMap->getSize ().x () * cStaticMap::tilePixelWidth);
	auto yZoom = (float)getSize ().y () / (staticMap->getSize ().y () * cStaticMap::tilePixelHeight);

	// then we try to fix if round would have rounded up:

	xZoom = std::max (xZoom, (float)((int)(cStaticMap::tilePixelWidth * xZoom) + (xZoom >= 1.0f ? 0 : 1)) / cStaticMap::tilePixelWidth);
	yZoom = std::max (yZoom, (float)((int)(cStaticMap::tilePixelHeight * yZoom) + (yZoom >= 1.0f ? 0 : 1)) / cStaticMap::tilePixelHeight);

	return std::max(xZoom, yZoom);
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::computeMaximalPixelOffset () const
{
	const auto x = staticMap->getSize(). x () * cStaticMap::tilePixelWidth - (int)(getSize ().x () / getZoomFactor ());
	const auto y = staticMap->getSize(). y () * cStaticMap::tilePixelHeight - (int)(getSize ().y () / getZoomFactor ());

	return cPosition (x, y);
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::zoomSize (const cPosition& size, float zoomFactor) const
{
	return cPosition (Round (size.x () * zoomFactor), Round (size.y () * zoomFactor));
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::getZoomedTileSize () const
{
	// this should be the only place where the internalZoomFactor is used directly
	return zoomSize (cPosition (cStaticMap::tilePixelHeight, cStaticMap::tilePixelWidth), internalZoomFactor);
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::getZoomedStartTilePixelOffset () const
{
	return zoomSize(cPosition (pixelOffset.x () % cStaticMap::tilePixelWidth, pixelOffset.y () % cStaticMap::tilePixelHeight), getZoomFactor());
}

//------------------------------------------------------------------------------
std::pair<cPosition, cPosition> cGameMapWidget::computeTileDrawingRange () const
{
	const auto zoomedTileSize = getZoomedTileSize ();

	const cPosition drawingPixelRange = getSize () + getZoomedStartTilePixelOffset ();

	const cPosition tilesSize ((int)std::ceil (drawingPixelRange.x () / zoomedTileSize.x ()), (int)std::ceil (drawingPixelRange.y () / zoomedTileSize.y ()));

	cPosition startTile ((int)std::floor (pixelOffset.x () / cStaticMap::tilePixelWidth), (int)std::floor (pixelOffset.y () / cStaticMap::tilePixelHeight));
	cPosition endTile (startTile + tilesSize + 1);

	startTile.x () = std::max (0, startTile.x ());
	startTile.y () = std::max (0, startTile.y ());

	endTile.x () = std::min (staticMap->getSize ().x (), endTile.x ());
	endTile.y () = std::min (staticMap->getSize ().y (), endTile.y ());

	return std::make_pair(startTile, endTile);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawTerrain ()
{
	const auto zoomedTileSize = getZoomedTileSize ();
	const auto tileDrawingRange = computeTileDrawingRange ();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset ();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore (); i.next ())
	{
		const auto& terrain = staticMap->getTerrain (*i);

		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);
		if (shouldDrawFog && (!player || !player->canSeeAt (*i)))
		{
			if (!cSettings::getInstance ().shouldDoPrescale () && (terrain.shw->w != zoomedTileSize.x () || terrain.shw->h != zoomedTileSize.y ()))
			{
				scaleSurface (terrain.shw_org.get (), terrain.shw.get (), zoomedTileSize.x (), zoomedTileSize.y ());
			}
			SDL_BlitSurface (terrain.shw.get (), nullptr, cVideo::buffer, &drawDestination);
		}
		else
		{
			if (!cSettings::getInstance ().shouldDoPrescale () && (terrain.sf->w != zoomedTileSize.x () || terrain.sf->h != zoomedTileSize.y ()))
			{
				scaleSurface (terrain.sf_org.get (), terrain.sf.get (), zoomedTileSize.x (), zoomedTileSize.y ());
			}
			SDL_BlitSurface (terrain.sf.get (), nullptr, cVideo::buffer, &drawDestination);
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawGrid ()
{
	const auto zoomedTileSize = getZoomedTileSize ();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset ();

	SDL_Rect destY = {getPosition ().x (), getPosition ().y () + zoomedTileSize.y () - zoomedStartTilePixelOffset.y (), getSize ().x (), 1};
	for (; destY.y < getEndPosition ().y (); destY.y += zoomedTileSize.y ())
	{
		SDL_FillRect (cVideo::buffer, &destY, GRID_COLOR);
	}

	SDL_Rect destX = {getPosition ().x ()+ zoomedTileSize.x () - zoomedStartTilePixelOffset.x (), getPosition ().y (), 1, getSize ().y ()};
	for (; destX.x < getEndPosition ().x (); destX.x += zoomedTileSize.x ())
	{
		SDL_FillRect (cVideo::buffer, &destX, GRID_COLOR);
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawEffects (bool bottom)
{
	SDL_Rect clipRect = getArea ().toSdlRect ();
	SDL_SetClipRect (cVideo::buffer, &clipRect);

	const cPosition originalTileSize (cStaticMap::tilePixelWidth, cStaticMap::tilePixelHeight);

	for (auto it = effects.begin (); it != effects.end ();) // ATTENTION: erase in loop. do not use continue;
	{
		auto& effect = *it;

		if (effect->isFinished())
		{
			it = effects.erase (it);
		}
		else
		{
			if (effect->bottom == bottom &&
				(!player || player->canSeeAt (effect->getPosition () / originalTileSize)))
			{
				cPosition screenDestination;
				screenDestination.x () = getPosition ().x () + static_cast<int>((effect->getPosition ().x () - pixelOffset.x ()) * getZoomFactor ());
				screenDestination.y () = getPosition ().y () + static_cast<int>((effect->getPosition ().y () - pixelOffset.y ()) * getZoomFactor ());
				effect->draw (getZoomFactor (), screenDestination);
			}

			++it;
		}
	}
	SDL_SetClipRect (cVideo::buffer, nullptr);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawBaseUnits ()
{
	if (!dynamicMap) return;

	const auto zoomedTileSize = getZoomedTileSize ();
	const auto tileDrawingRange = computeTileDrawingRange ();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset ();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore (); i.next ())
	{
		auto& mapField = dynamicMap->getField (*i);
		const auto& buildings = mapField.getBuildings ();
		for (auto it = buildings.rbegin (); it != buildings.rend (); ++it)
		{
			if (*it == nullptr) continue; // should never happen

			const auto& building = *(*it);

			if (building.data.surfacePosition != sUnitData::SURFACE_POS_BENEATH_SEA &&
				building.data.surfacePosition != sUnitData::SURFACE_POS_BASE &&
				building.owner) break;

			if (!player || player->canSeeAnyAreaUnder (building))
			{
				if (shouldDrawUnit (building, *i, tileDrawingRange))
				{
					const auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, building.getPosition());
					unitDrawingEngine.drawUnit (building, drawDestination, getZoomFactor (), &unitSelection, player.get ());
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawTopBuildings ()
{
	if (!dynamicMap) return;

	const auto zoomedTileSize = getZoomedTileSize ();
	const auto tileDrawingRange = computeTileDrawingRange ();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset ();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore (); i.next ())
	{
		auto& mapField = dynamicMap->getField (*i);
		auto building = mapField.getTopBuilding ();
		if (building == nullptr) continue;
		if (building->data.surfacePosition != sUnitData::SURFACE_POS_GROUND) continue;
		if (!player || !player->canSeeAnyAreaUnder (*building)) continue;
		if (!shouldDrawUnit(*building, *i, tileDrawingRange)) continue;

		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, building->getPosition());
		unitDrawingEngine.drawUnit (*building, drawDestination, getZoomFactor (), &unitSelection, player.get ());

		//if (debugOutput.debugBaseClient && building->SubBase)
		//	drawTopBuildings_DebugBaseClient (*building, drawDestination);
		//if (debugOutput.debugBaseServer && building->SubBase)
		//	drawTopBuildings_DebugBaseServer (*building, drawDestination, pos);
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawShips ()
{
	if (!dynamicMap) return;

	const auto zoomedTileSize = getZoomedTileSize ();
	const auto tileDrawingRange = computeTileDrawingRange ();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset ();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore (); i.next ())
	{
		auto& mapField = dynamicMap->getField (*i);
		auto vehicle = mapField.getVehicle ();
		if (vehicle == nullptr) continue;
		if (vehicle->data.factorSea > 0 && vehicle->data.factorGround == 0)
		{
			auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);
			unitDrawingEngine.drawUnit (*vehicle, drawDestination, getZoomFactor (), *dynamicMap, &unitSelection, player.get ());
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawAboveSeaBaseUnits ()
{
	if (!dynamicMap) return;

	const auto zoomedTileSize = getZoomedTileSize ();
	const auto tileDrawingRange = computeTileDrawingRange ();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset ();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore (); i.next ())
	{
		auto& mapField = dynamicMap->getField (*i);

		const auto& buildings = mapField.getBuildings ();
		for (auto it = buildings.begin (); it != buildings.end (); ++it)
		{
			const auto& building = *(*it);
			if (building.data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)
			{
				const auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, building.getPosition ());
				unitDrawingEngine.drawUnit (building, drawDestination, getZoomFactor (), &unitSelection, player.get ());
			}
		}
		for (auto it = buildings.begin (); it != buildings.end (); ++it)
		{
			const auto& building = *(*it);
			if ((*it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)
			{
				const auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, building.getPosition ());
				unitDrawingEngine.drawUnit (building, drawDestination, getZoomFactor (), &unitSelection, player.get ());
			}
		}

		auto vehicle = mapField.getVehicle();
		if (vehicle && (vehicle->isUnitClearing () || vehicle->isUnitBuildingABuilding ()) && (player && player->canSeeAnyAreaUnder (*vehicle)))
		{
			if (shouldDrawUnit(*vehicle, *i, tileDrawingRange))
			{
				const auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, vehicle->getPosition ());
				unitDrawingEngine.drawUnit (*vehicle, drawDestination, getZoomFactor (), *dynamicMap, &unitSelection, player.get ());
			}
		}
	}
}

//------------------------------------------------------------------------------
bool cGameMapWidget::shouldDrawUnit (const cUnit& unit, const cPosition& visitingPosition, const std::pair<cPosition, cPosition>& tileDrawingRange)
{
	assert (unit.isAbove (visitingPosition));

	if (!unit.data.isBig)
	{
		return true;
	}
	else
	{
		if (unit.getPosition ().x () < tileDrawingRange.first.x () || unit.getPosition ().y () < tileDrawingRange.first.y ())
		{
			cBox<cPosition> tileDrawingBox (tileDrawingRange.first, tileDrawingRange.second - cPosition (1, 1));
			auto intersectedArea = tileDrawingBox.intersection (unit.getArea ());

			return visitingPosition == intersectedArea.getMinCorner ();
		}
		else return visitingPosition == unit.getPosition ();
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawVehicles ()
{
	if (!dynamicMap) return;

	const auto zoomedTileSize = getZoomedTileSize ();
	const auto tileDrawingRange = computeTileDrawingRange ();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset ();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore (); i.next ())
	{
		auto& mapField = dynamicMap->getField (*i);
		auto vehicle = mapField.getVehicle ();
		if (vehicle == nullptr) continue;
		if (vehicle->data.factorGround != 0 && !vehicle->isUnitBuildingABuilding () && !vehicle->isUnitClearing ())
		{
			auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);
			unitDrawingEngine.drawUnit (*vehicle, drawDestination, getZoomFactor (), *dynamicMap, &unitSelection, player.get ());
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawConnectors ()
{
	if (!dynamicMap) return;

	const auto zoomedTileSize = getZoomedTileSize ();
	const auto tileDrawingRange = computeTileDrawingRange ();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset ();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore (); i.next ())
	{
		auto& mapField = dynamicMap->getField (*i);
		auto building = mapField.getTopBuilding ();
		if (building == nullptr) continue;
		if (building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE)
		{
			auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);
			unitDrawingEngine.drawUnit (*building, drawDestination, getZoomFactor (), &unitSelection, player.get ());
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawPlanes ()
{
	if (!dynamicMap) return;

	const auto zoomedTileSize = getZoomedTileSize ();
	const auto tileDrawingRange = computeTileDrawingRange ();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset ();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore (); i.next ())
	{
		auto& mapField = dynamicMap->getField (*i);
		const auto& planes = mapField.getPlanes ();

		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);
		for (auto it = planes.rbegin (); it != planes.rend (); ++it)
		{
			auto& plane = **it;
			unitDrawingEngine.drawUnit (plane, drawDestination, getZoomFactor (), *dynamicMap, &unitSelection, player.get ());
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawResources ()
{
	if (!dynamicMap) return;

	const auto zoomedTileSize = getZoomedTileSize ();
	const auto tileDrawingRange = computeTileDrawingRange ();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset ();

	SDL_Rect tmp, src = {0, 0, Uint16 (zoomedTileSize.x ()), Uint16 (zoomedTileSize.y ())};
	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore (); i.next ())
	{
		if (player && !player->hasResourceExplored (*i)) continue;
		if (dynamicMap->isBlocked (*i)) continue;

		const auto& resource = dynamicMap->getResource (*i);
		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);

		if (resource.typ == RES_NONE)
		{
			src.x = 0;
			tmp = drawDestination;
			if (!cSettings::getInstance ().shouldDoPrescale () && (ResourceData.res_metal->w != ResourceData.res_metal_org->w / 64 * zoomedTileSize.x () || ResourceData.res_metal->h != zoomedTileSize.y ())) scaleSurface (ResourceData.res_metal_org.get (), ResourceData.res_metal.get (), ResourceData.res_metal_org->w / 64 * zoomedTileSize.x (), zoomedTileSize.y ());
			SDL_BlitSurface (ResourceData.res_metal.get (), &src, cVideo::buffer, &tmp);
		}
		else
		{
			src.x = resource.value * zoomedTileSize.x ();
			tmp = drawDestination;
			if (resource.typ == RES_METAL)
			{
				if (!cSettings::getInstance ().shouldDoPrescale () && (ResourceData.res_metal->w != ResourceData.res_metal_org->w / 64 * zoomedTileSize.x () || ResourceData.res_metal->h != zoomedTileSize.y ())) scaleSurface (ResourceData.res_metal_org.get (), ResourceData.res_metal.get (), ResourceData.res_metal_org->w / 64 * zoomedTileSize.x (), zoomedTileSize.y ());
				SDL_BlitSurface (ResourceData.res_metal.get (), &src, cVideo::buffer, &tmp);
			}
			else if (resource.typ == RES_OIL)
			{
				if (!cSettings::getInstance ().shouldDoPrescale () && (ResourceData.res_oil->w != ResourceData.res_oil_org->w / 64 * zoomedTileSize.x () || ResourceData.res_oil->h != zoomedTileSize.y ())) scaleSurface (ResourceData.res_oil_org.get (), ResourceData.res_oil.get (), ResourceData.res_oil_org->w / 64 * zoomedTileSize.x (), zoomedTileSize.y ());
				SDL_BlitSurface (ResourceData.res_oil.get (), &src, cVideo::buffer, &tmp);
			}
			else // Gold
			{
				if (!cSettings::getInstance ().shouldDoPrescale () && (ResourceData.res_gold->w != ResourceData.res_gold_org->w / 64 * zoomedTileSize.x () || ResourceData.res_gold->h != zoomedTileSize.y ())) scaleSurface (ResourceData.res_gold_org.get (), ResourceData.res_gold.get (), ResourceData.res_gold_org->w / 64 * zoomedTileSize.x (), zoomedTileSize.y ());
				SDL_BlitSurface (ResourceData.res_gold.get (), &src, cVideo::buffer, &tmp);
			}
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawSelectionBox ()
{
	if (!unitSelectionBox.isValid ()) return;

	const auto zoomedTileSize = getZoomedTileSize ();

	const auto zoomOffX = (int)(pixelOffset.x () * getZoomFactor ());
	const auto zoomOffY = (int)(pixelOffset.y () * getZoomFactor ());

	const int mouseTopX = static_cast<int> (std::min (unitSelectionBox.getBox ().getMinCorner ()[0], unitSelectionBox.getBox ().getMaxCorner ()[0]) * zoomedTileSize. x());
	const int mouseTopY = static_cast<int> (std::min (unitSelectionBox.getBox ().getMinCorner ()[1], unitSelectionBox.getBox ().getMaxCorner ()[1]) * zoomedTileSize. y());
	const int mouseBottomX = static_cast<int> (std::max (unitSelectionBox.getBox ().getMinCorner ()[0], unitSelectionBox.getBox ().getMaxCorner ()[0]) * zoomedTileSize. x());
	const int mouseBottomY = static_cast<int> (std::max (unitSelectionBox.getBox ().getMinCorner ()[1], unitSelectionBox.getBox ().getMaxCorner ()[1]) * zoomedTileSize. y());
	const Uint32 color = 0xFFFFFF00;
	SDL_Rect d;

	d.x = mouseTopX - zoomOffX + getPosition(). x();
	d.y = mouseBottomY - zoomOffY + getPosition ().y ();
	d.w = mouseBottomX - mouseTopX;
	d.h = 1;
	SDL_FillRect (cVideo::buffer, &d, color);

	d.x = mouseTopX - zoomOffX + getPosition ().x ();
	d.y = mouseTopY - zoomOffY + getPosition ().y ();
	d.w = mouseBottomX - mouseTopX;
	d.h = 1;
	SDL_FillRect (cVideo::buffer, &d, color);

	d.x = mouseTopX - zoomOffX + getPosition ().x ();
	d.y = mouseTopY - zoomOffY + getPosition ().y ();
	d.w = 1;
	d.h = mouseBottomY - mouseTopY;
	SDL_FillRect (cVideo::buffer, &d, color);

	d.x = mouseBottomX - zoomOffX + getPosition ().x ();
	d.y = mouseTopY - zoomOffY + getPosition ().y ();
	d.w = 1;
	d.h = mouseBottomY - mouseTopY;
	SDL_FillRect (cVideo::buffer, &d, color);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawUnitCircles ()
{
	auto clipRect = getArea ().toSdlRect ();
	SDL_SetClipRect (cVideo::buffer, &clipRect);

	auto selectedVehicle = unitSelection.getSelectedVehicle();
	auto selectedBuilding = unitSelection.getSelectedBuilding ();

	const auto zoomedTileSize = getZoomedTileSize ();

	if (selectedVehicle && selectedVehicle->isDisabled () == false)
	{
		const bool movementOffset = !selectedVehicle->isUnitBuildingABuilding () && !selectedVehicle->isUnitClearing ();
		const auto screenPosition = getScreenPosition (*selectedVehicle, movementOffset);
		if (shouldDrawScan)
		{
			if (selectedVehicle->data.isBig)
			{
				drawCircle (screenPosition.x () + zoomedTileSize.x (), screenPosition.y () + zoomedTileSize.y (), selectedVehicle->data.getScan () * zoomedTileSize.x (), SCAN_COLOR, *cVideo::buffer);
			}
			else
			{
				drawCircle (screenPosition.x () + zoomedTileSize.x () / 2, screenPosition.y () + zoomedTileSize.y () / 2, selectedVehicle->data.getScan () * zoomedTileSize.x (), SCAN_COLOR, *cVideo::buffer);
			}
		}
		if (shouldDrawRange)
		{
			if (selectedVehicle->data.canAttack & TERRAIN_AIR) drawCircle (screenPosition.x () +zoomedTileSize.x () / 2, screenPosition.y () + zoomedTileSize.y () / 2, selectedVehicle->data.getRange () * zoomedTileSize.x () + 2, RANGE_AIR_COLOR, *cVideo::buffer);
			else drawCircle (screenPosition.x () + zoomedTileSize.x () / 2, screenPosition.y () + zoomedTileSize.y () / 2, selectedVehicle->data.getRange () * zoomedTileSize.x () + 1, RANGE_GROUND_COLOR, *cVideo::buffer);
		}
	}
	else if (selectedBuilding && selectedBuilding->isDisabled () == false)
	{
		const auto screenPosition = getScreenPosition (*selectedBuilding);
		if (shouldDrawScan)
		{
			if (selectedBuilding->data.isBig)
			{
				drawCircle (screenPosition. x() + zoomedTileSize.x (),
							screenPosition. y() + zoomedTileSize.y (),
							selectedBuilding->data.getScan () * zoomedTileSize.x (), SCAN_COLOR, *cVideo::buffer);
			}
			else
			{
				drawCircle (screenPosition. x() + zoomedTileSize.x () / 2,
							screenPosition. y() + zoomedTileSize.y () / 2,
							selectedBuilding->data.getScan () * zoomedTileSize.x (), SCAN_COLOR, *cVideo::buffer);
			}
		}
		if (shouldDrawRange && (selectedBuilding->data.canAttack & TERRAIN_GROUND) && !selectedBuilding->data.explodesOnContact)
		{
			drawCircle (screenPosition. x() + zoomedTileSize.x () / 2,
						screenPosition. y() + zoomedTileSize.y () / 2,
						selectedBuilding->data.getRange () * zoomedTileSize.x () + 2, RANGE_GROUND_COLOR, *cVideo::buffer);
		}
		if (shouldDrawRange && (selectedBuilding->data.canAttack & TERRAIN_AIR))
		{
			drawCircle (screenPosition. x() + zoomedTileSize.x () / 2,
						screenPosition. y() + zoomedTileSize.y () / 2,
						selectedBuilding->data.getRange () * zoomedTileSize.x () + 2, RANGE_AIR_COLOR, *cVideo::buffer);
		}
	}

	SDL_SetClipRect (cVideo::buffer, NULL);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawExitPoints ()
{
	auto selectedVehicle = unitSelection.getSelectedVehicle ();
	auto selectedBuilding = unitSelection.getSelectedBuilding ();

	if (selectedVehicle && selectedVehicle->isDisabled () == false)
	{
		if (dynamicMap && selectedVehicle->owner == player.get () &&
			(
				(selectedVehicle->isUnitBuildingABuilding() && selectedVehicle->getBuildTurns() == 0) ||
				(selectedVehicle->isUnitClearing () && selectedVehicle->getClearingTurns() == 0)
			) && !selectedVehicle->BuildPath)
		{
			drawExitPointsIf (*selectedVehicle, [&](const cPosition& position){ return dynamicMap->possiblePlace (*selectedVehicle, position); });
		}
		if (mouseMode->getType () == eMouseModeType::Activate && selectedVehicle->owner == player.get ())
		{
			auto activateMouseMode = static_cast<cMouseModeActivateLoaded*>(mouseMode.get());
			auto unitToExit = selectedVehicle->storedUnits[activateMouseMode->getVehicleToActivateIndex()]->data;
			drawExitPointsIf (*selectedVehicle, [&](const cPosition& position){ return selectedVehicle->canExitTo (position, *dynamicMap, unitToExit); });
		}
	}
	else if (selectedBuilding && selectedBuilding->isDisabled () == false)
	{
		if (!selectedBuilding->isBuildListEmpty() &&
			!selectedBuilding->isUnitWorking () &&
			selectedBuilding->getBuildListItem(0).getRemainingMetal () <= 0 &&
			selectedBuilding->owner == player.get ())
		{
			auto unitToExit = selectedBuilding->getBuildListItem (0).getType ().getUnitDataOriginalVersion ();
			drawExitPointsIf (*selectedBuilding, [&](const cPosition& position){ return selectedBuilding->canExitTo (position, *dynamicMap, *unitToExit); });
		}
		if (mouseMode->getType () == eMouseModeType::Activate && selectedBuilding->owner == player.get ())
		{
			auto activateMouseMode = static_cast<cMouseModeActivateLoaded*>(mouseMode.get ());
			auto unitToExit = selectedBuilding->storedUnits[activateMouseMode->getVehicleToActivateIndex ()]->data;
			drawExitPointsIf (*selectedBuilding, [&](const cPosition& position){ return selectedBuilding->canExitTo (position, *dynamicMap, unitToExit); });
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawExitPointsIf (const cUnit& unit, const std::function<bool (const cPosition&)>& predicate)
{
	if (!dynamicMap) return;

	auto adjacentPositions = unit.getAdjacentPositions ();

	for (size_t i = 0; i != adjacentPositions.size(); ++i)
	{
		if (predicate(adjacentPositions[i]))
		{
			drawExitPoint (adjacentPositions[i]);
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawExitPoint (const cPosition& position)
{
	const auto zoomedTileSize = getZoomedTileSize ();
	const auto tileDrawingRange = computeTileDrawingRange ();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset ();

	auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, position);

	const int nr = animationTimer->getAnimationTime () % 5;
	SDL_Rect src;
	src.x = zoomedTileSize.x () * nr;
	src.y = 0;
	src.w = zoomedTileSize.x ();
	src.h = zoomedTileSize.y ();

	CHECK_SCALING (*GraphicsData.gfx_exitpoints, *GraphicsData.gfx_exitpoints_org, getZoomFactor());
	SDL_BlitSurface (GraphicsData.gfx_exitpoints.get (), &src, cVideo::buffer, &drawDestination);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawBuildBand ()
{
	auto selectedVehicle = unitSelection.getSelectedVehicle ();

	const auto zoomedTileSize = getZoomedTileSize ();

	if (selectedVehicle && !selectedVehicle->isDisabled ())
	{
		auto mouse = getActiveMouse ();

		if (!mouse || !getArea().withinOrTouches(mouse->getPosition())) return;

		if (mouseMode->getType () == eMouseModeType::SelectBuildPosition)
		{
			if (!dynamicMap) return;

			auto selectBuildPositionMode = static_cast<const cMouseModeSelectBuildPosition*>(mouseMode.get ());
			bool validPosition;
			cPosition destination;
			std::tie (validPosition, destination) = selectBuildPositionMode->findNextBuildPosition (selectedVehicle->getPosition(), getMapTilePosition (mouse->getPosition ()));
			if (!validPosition) return;

			SDL_Rect dest;
			dest.x = getPosition ().x () - (int)(pixelOffset.x () * getZoomFactor ()) + zoomedTileSize.x () * destination.x ();
			dest.y = getPosition ().y () - (int)(pixelOffset.y () * getZoomFactor ()) + zoomedTileSize.y () * destination.y ();
			CHECK_SCALING (*GraphicsData.gfx_band_big, *GraphicsData.gfx_band_big_org, getZoomFactor ());
			SDL_BlitSurface (GraphicsData.gfx_band_big.get (), NULL, cVideo::buffer, &dest);
		}
		else if (mouseMode->getType () == eMouseModeType::SelectBuildPathDestintaion)
		{
			const auto mouseTilePosition = getMapTilePosition (mouse->getPosition ());
			if (mouseTilePosition.x () == selectedVehicle->getPosition().x() || mouseTilePosition.y () == selectedVehicle->getPosition().y())
			{
				SDL_Rect dest;
				dest.x = getPosition ().x () - (int)(pixelOffset.x () * getZoomFactor ()) + zoomedTileSize.x () * mouseTilePosition.x ();
				dest.y = getPosition ().y () - (int)(pixelOffset.y () * getZoomFactor ()) + zoomedTileSize.y () * mouseTilePosition.y ();
				CHECK_SCALING (*GraphicsData.gfx_band_small, *GraphicsData.gfx_band_small_org, getZoomFactor ());
				SDL_BlitSurface (GraphicsData.gfx_band_small.get (), NULL, cVideo::buffer, &dest);
			}
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawLockList (const cPlayer& player)
{
	const auto zoomedTileSize = getZoomedTileSize ();

	for (size_t i = 0; i < unitLockList.getLockedUnitsCount(); ++i)
	{
		const cUnit* unit = unitLockList.getLockedUnit(i);

		if (!player.canSeeAnyAreaUnder (*unit))
		{
			continue;
		}

		const auto screenPosition = getScreenPosition (*unit);

		if (shouldDrawScan)
		{
			if (unit->data.isBig)
				drawCircle (screenPosition.x () + zoomedTileSize.x (), screenPosition.y () + zoomedTileSize.y (), unit->data.getScan () * zoomedTileSize.x (), SCAN_COLOR, *cVideo::buffer);
			else
				drawCircle (screenPosition.x () + zoomedTileSize.x () / 2, screenPosition.y () + zoomedTileSize.y () / 2, unit->data.getScan () * zoomedTileSize.x (), SCAN_COLOR, *cVideo::buffer);
		}
		if (shouldDrawRange && (unit->data.canAttack & TERRAIN_GROUND))
			drawCircle (screenPosition.x () + zoomedTileSize.x () / 2, screenPosition.y () + zoomedTileSize.y () / 2,
						unit->data.getRange () * zoomedTileSize.x () + 1, RANGE_GROUND_COLOR, *cVideo::buffer);
		if (shouldDrawRange && (unit->data.canAttack & TERRAIN_AIR))
			drawCircle (screenPosition.x () + zoomedTileSize.x () / 2, screenPosition.y () + zoomedTileSize.y () / 2,
						unit->data.getRange () * zoomedTileSize.x () + 2, RANGE_AIR_COLOR, *cVideo::buffer);
		//if (ammoChecked () && unit->data.canAttack)
		//	drawMunBar (*unit, screenPos);
		//if (hitsChecked ())
		//	drawHealthBar (*unit, screenPos);
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawBuildPath (const cVehicle& vehicle)
{
	if (!vehicle.BuildPath || (vehicle.bandPosition == vehicle.getPosition()) || mouseMode->getType () == eMouseModeType::SelectBuildPathDestintaion) return;

	const auto zoomedTileSize = getZoomedTileSize ();

	int mx = vehicle.getPosition().x();
	int my = vehicle.getPosition().y();
	int sp;
	if (mx < vehicle.bandPosition.x())
		sp = 4;
	else if (mx > vehicle.bandPosition.x())
		sp = 3;
	else if (my < vehicle.bandPosition.y())
		sp = 1;
	else
		sp = 6;

	while (mx != vehicle.bandPosition.x() || my != vehicle.bandPosition.y())
	{
		SDL_Rect dest;
		dest.x = getPosition ().x () - (int)(pixelOffset.x () * getZoomFactor ()) + zoomedTileSize.x () * mx;
		dest.y = getPosition ().y () - (int)(pixelOffset.y () * getZoomFactor ()) + zoomedTileSize.y () * my;

		SDL_BlitSurface (OtherData.WayPointPfeileSpecial[sp][64 - zoomedTileSize.x ()].get (), NULL, cVideo::buffer, &dest);

		if (mx < vehicle.bandPosition.x())
			mx++;
		else if (mx > vehicle.bandPosition.x())
			mx--;

		if (my < vehicle.bandPosition.y())
			my++;
		else if (my > vehicle.bandPosition.y())
			my--;
	}
	SDL_Rect dest;
	dest.x = getPosition ().x () - (int)(pixelOffset.x () * getZoomFactor ()) + zoomedTileSize.x () * mx;
	dest.y = getPosition ().y () - (int)(pixelOffset.y () * getZoomFactor ()) + zoomedTileSize.y () * my;

	SDL_BlitSurface (OtherData.WayPointPfeileSpecial[sp][64 - zoomedTileSize.x ()].get (), NULL, cVideo::buffer, &dest);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawPath (const cVehicle& vehicle)
{
	auto moveJob = vehicle.getClientMoveJob ();

	if (!moveJob || !moveJob->Waypoints || vehicle.owner != player.get ())
	{
		drawBuildPath (vehicle);
		return;
	}

	const auto zoomedTileSize = getZoomedTileSize ();

	int sp = vehicle.data.speedCur;
	int save;

	if (sp)
	{
		save = 0;
		sp += moveJob->iSavedSpeed;
	}
	else save = moveJob->iSavedSpeed;

	SDL_Rect dest;
	dest.x = getPosition ().x () - (int)(pixelOffset.x () * getZoomFactor ()) + zoomedTileSize.x () * vehicle.getPosition().x();
	dest.y = getPosition ().y () - (int)(pixelOffset.y () * getZoomFactor ()) + zoomedTileSize.y () * vehicle.getPosition().y();
	dest.w = zoomedTileSize.x ();
	dest.h = zoomedTileSize.y ();
	SDL_Rect ndest = dest;

	int mx = 0;
	int my = 0;
	sWaypoint* wp = moveJob->Waypoints;
	while (wp)
	{
		if (wp->next)
		{
			ndest.x += mx = wp->next->position.x() * zoomedTileSize.x () - wp->position.x() * zoomedTileSize.x ();
			ndest.y += my = wp->next->position.y() * zoomedTileSize.y () - wp->position.y() * zoomedTileSize.y ();
		}
		else
		{
			ndest.x += mx;
			ndest.y += my;
		}

		if (sp == 0)
		{
			moveJob->drawArrow (dest, &ndest, true);
			sp += vehicle.data.speedMax + save;
			save = 0;
		}
		else
		{
			moveJob->drawArrow (dest, &ndest, false);
		}

		dest = ndest;
		wp = wp->next;

		if (wp)
		{
			sp -= wp->Costs;

			if (wp->next && sp < wp->next->Costs)
			{
				save = sp;
				sp = 0;
			}
		}
	}
}

//------------------------------------------------------------------------------
SDL_Rect cGameMapWidget::computeTileDrawingArea (const cPosition& zoomedTileSize, const cPosition& zoomedStartTilePixelOffset, const cPosition& tileStartIndex, const cPosition& tileIndex)
{
	const cPosition startDrawPosition = getPosition () + (tileIndex - tileStartIndex) * zoomedTileSize - zoomedStartTilePixelOffset;

	SDL_Rect dest = {startDrawPosition.x (), startDrawPosition.y (), zoomedTileSize.x (), zoomedTileSize.y ()};

	return dest;
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::getMapTilePosition (const cPosition& pixelPosition) const
{
	assert (getArea ().withinOrTouches (pixelPosition));

	const auto zoomedTileSize = getZoomedTileSize ();

	const auto x = (int)((pixelPosition.x () - getPosition ().x () + pixelOffset.x () * getZoomFactor ()) / zoomedTileSize.x ());
	const auto y = (int)((pixelPosition.y () - getPosition ().y () + pixelOffset.y () * getZoomFactor ()) / zoomedTileSize.y ());

	const cPosition tilePosition (std::min (x, staticMap->getSize ().x ()-1), std::min(y, staticMap->getSize ().y ()-1));

	return tilePosition;
}

//------------------------------------------------------------------------------
bool cGameMapWidget::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	auto consumed = cClickableWidget::handleMouseMoved (application, mouse, offset);

	cPosition lastMouseOverTilePosition;
	const auto lastMousePosition = mouse.getPosition () - offset;
	if (getArea ().withinOrTouches (lastMousePosition))
	{
		lastMouseOverTilePosition = getMapTilePosition (lastMousePosition);
	}
	else
	{
		lastMouseOverTilePosition = cPosition (-1, -1);
	}

	if (getArea ().withinOrTouches (mouse.getPosition ()))
	{
		const auto tilePosition = getMapTilePosition (mouse.getPosition ());
		if (tilePosition != lastMouseOverTilePosition)
		{
			tileUnderMouseChanged (tilePosition);
		}
		updateMouseCursor ();
	}
	else
	{
		tileUnderMouseChanged (cPosition(-1,-1));
	}

	if (unitSelectionBox.isValidStart () && isAt (mouse.getPosition ()) &&
		mouse.isButtonPressed (eMouseButtonType::Left) && !mouse.isButtonPressed (eMouseButtonType::Right))
	{
		const auto zoomedTileSize = getZoomedTileSize ();

		unitSelectionBox.getBox ().getMaxCorner ()[0] = (mouse.getPosition ().x () - getPosition ().x () + (pixelOffset.x () * getZoomFactor ())) / zoomedTileSize.x ();
		unitSelectionBox.getBox ().getMaxCorner ()[1] = (mouse.getPosition ().y () - getPosition ().y () + (pixelOffset.y () * getZoomFactor ())) / zoomedTileSize.y ();
	}

	return consumed;
}

//------------------------------------------------------------------------------
bool cGameMapWidget::handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (!unitSelectionBox.isValidStart () && isAt (mouse.getPosition ()) &&
		button == eMouseButtonType::Left && !mouse.isButtonPressed (eMouseButtonType::Right))
	{
		const auto zoomedTileSize = getZoomedTileSize ();

		unitSelectionBox.getBox ().getMinCorner ()[0] = (mouse.getPosition ().x () - getPosition ().x () + (pixelOffset.x () * getZoomFactor ())) / zoomedTileSize.x ();
		unitSelectionBox.getBox ().getMinCorner ()[1] = (mouse.getPosition ().y () - getPosition ().y () + (pixelOffset.y () * getZoomFactor ())) / zoomedTileSize.y ();
	}

	return cClickableWidget::handleMousePressed (application, mouse, button);
}

//------------------------------------------------------------------------------
bool cGameMapWidget::handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left && !mouse.isButtonPressed (eMouseButtonType::Right) &&
		!unitSelectionBox.isTooSmall () && dynamicMap && player)
	{
		unitSelection.selectVehiclesAt (unitSelectionBox.getCorrectedMapBox (), *dynamicMap, *player);
		unitSelectionBox.invalidate ();
		cClickableWidget::finishMousePressed (application, mouse, button);
		return true;
	}
	else
	{
		unitSelectionBox.invalidate ();
		return cClickableWidget::handleMouseReleased (application, mouse, button);
	}
}

//------------------------------------------------------------------------------
bool cGameMapWidget::acceptButton (eMouseButtonType button) const
{
	return button == eMouseButtonType::Left || button == eMouseButtonType::Right;
}

//------------------------------------------------------------------------------
bool cGameMapWidget::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (!getArea ().withinOrTouches (mouse.getPosition ())) return false;

	if (!dynamicMap) return false;

	const auto tilePosition = getMapTilePosition (mouse.getPosition ());

	const auto& field = dynamicMap->getField (tilePosition);

	bool changeAllowed = true;

	// Some useful aliases
	const auto selectedUnit = unitSelection.getSelectedUnit ();
	const auto selectedVehicle = unitSelection.getSelectedVehicle ();
	const auto selectedBuilding = unitSelection.getSelectedBuilding ();

	const auto overVehicle = field.getVehicle ();
	const auto overPlane = field.getPlane ();
	const auto overBuilding = field.getBuilding ();
	const auto overBaseBuilding = field.getBaseBuilding ();

	if (button == eMouseButtonType::Right)
	{
		if (KeysList.getMouseStyle() == eMouseStyle::OldSchool && !mouse.isButtonPressed (eMouseButtonType::Left))
		{
			if (selectedUnit && (overVehicle == selectedUnit ||
								 overPlane == selectedUnit ||
								 overBuilding == selectedUnit ||
								 overBaseBuilding == selectedUnit))
			{
				triggeredUnitHelp (*selectedUnit);
			}
			else
			{
				unitSelection.selectUnitAt (field, true);
			}
		}
		else if ((!mouse.isButtonPressed (eMouseButtonType::Left) /*&& rightMouseBox.isTooSmall ()*/) ||
				 (KeysList.getMouseStyle () == eMouseStyle::OldSchool && mouse.isButtonPressed (eMouseButtonType::Left)))
		{
			if (mouseMode->getType() == eMouseModeType::Help)
			{
				toggleMouseInputMode (eMouseModeType::Help);
			}
			else
			{
				if (selectedUnit && selectedUnit->isAbove (tilePosition))
				{
					const auto& planes = field.getPlanes ();
					cUnit* next = NULL;

					if (selectedVehicle)
					{
						auto it = std::find (planes.begin (), planes.end (), selectedVehicle);

						if (it == planes.end ())
						{
							if (overBuilding) next = overBuilding;
							else if (overBaseBuilding) next = overBaseBuilding;
							else if (overPlane) next = overPlane;
						}
						else
						{
							++it;

							if (it != planes.end ()) next = *it;
							else if (overVehicle) next = overVehicle;
							else if (overBuilding) next = overBuilding;
							else if (overBaseBuilding) next = overBaseBuilding;
							else if (planes.size () > 1)
							{
								next = planes[0];
							}
						}
					}
					else if (selectedBuilding)
					{
						if (overBuilding == selectedBuilding)
						{
							if (overBaseBuilding) next = overBaseBuilding;
							else if (overPlane) next = overPlane;
							else if (overVehicle) next = overVehicle;
						}
						else
						{
							if (overPlane) next = overPlane;
							else if (overVehicle) next = overVehicle;
							else if (overBuilding) next = overBuilding;
						}
					}

					if (next)
					{
						unitSelection.selectUnit (*next);
					}
					else
					{
						unitSelection.deselectUnits ();
					}
				}
				else
				{
					unitSelection.deselectUnits ();
				}
			}
		}
	}
	else if (button == eMouseButtonType::Left && !mouse.isButtonPressed (eMouseButtonType::Right))
	{
		bool consumed = false;

		auto action = mouseMode->getMouseAction (tilePosition);
		if (action && (changeAllowed || !action->doesChangeState()))
		{
			consumed = action->executeLeftClick (*this, *dynamicMap, tilePosition, unitSelection);

			if (action->isSingleAction () && mouseMode->getType() != eMouseModeType::Default)
			{
				setMouseInputMode (std::make_unique<cMouseModeDefault> (dynamicMap.get (), unitSelection, player.get ()));
			}
		}

		// toggle unit context menu if no other click action has been performed
		if (!consumed)
		{
			if (changeAllowed && selectedVehicle && (Contains (field.getPlanes (), selectedVehicle) || selectedVehicle == overVehicle))
			{
				if (!selectedVehicle->isUnitMoving ())
				{
					toggleUnitContextMenu (selectedVehicle);
					cSoundDevice::getInstance().getFreeSoundEffectChannel().play (SoundData.SNDHudButton);
				}
			}
			else if (changeAllowed && selectedBuilding && (overBaseBuilding == selectedBuilding || overBuilding == selectedBuilding))
			{
				toggleUnitContextMenu (selectedBuilding);
                cSoundDevice::getInstance ().getFreeSoundEffectChannel().play (SoundData.SNDHudButton);
			}
		}

		// toggle the lock state of an enemy unit,
		// if the selection has changed
		if (player && lockActive)
		{
			const auto newSelectedUnit = unitSelection.getSelectedUnit ();
			if (newSelectedUnit && newSelectedUnit != selectedUnit && newSelectedUnit->owner != player.get ())
			{
				unitLockList.toggleLockAt (field);
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::getScreenPosition (const cUnit& unit, bool movementOffset) const
{
	cPosition position;

	const int offsetX = movementOffset ? unit.getMovementOffset ().x() : 0;
	position.x() = getPosition ().x () - ((int)((pixelOffset.x () - offsetX) * getZoomFactor ())) + getZoomedTileSize ().x () * unit.getPosition().x();

	const int offsetY = movementOffset ? unit.getMovementOffset ().y() : 0;
	position.y() = getPosition ().y () - ((int)((pixelOffset.y () - offsetY) * getZoomFactor ())) + getZoomedTileSize ().y () * unit.getPosition().y();

	return position;
}

//------------------------------------------------------------------------------
void cGameMapWidget::updateActiveAnimations ()
{
	animations.clear ();
	updateActiveAnimations (std::make_pair (cPosition (-1, -1), cPosition (-1, -1)));
}

//------------------------------------------------------------------------------
void cGameMapWidget::updateActiveAnimations (const std::pair<cPosition, cPosition>& oldTileDrawingRange)
{
	if (!cSettings::getInstance ().isAnimations ()) return;

	const auto tileDrawingRange = computeTileDrawingRange ();

	const auto tileDrawingArea = cBox<cPosition> (tileDrawingRange.first, tileDrawingRange.second - cPosition(1,1));
	const auto oldTileDrawingArea = cBox<cPosition> (oldTileDrawingRange.first, oldTileDrawingRange.second - cPosition (1, 1));

	for (auto i = animations.begin (); i != animations.end (); /*erase in loop*/)
	{
		auto& animation = **i;
		if (animation.isFinished() || !animation.isLocatedIn (tileDrawingArea))
		{
			i = animations.erase (i);
		}
		else
		{
			++i;
		}
	}

	if (dynamicMap)
	{
		std::vector<cUnit*> units;
		for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore (); i.next ())
		{
			const auto position = *i;
			const auto& field = dynamicMap->getField (position);

			field.getUnits (units);

			for (size_t j = 0; j < units.size (); ++j)
			{
				const auto& unit = *units[j];
				if (shouldDrawUnit (unit, position, tileDrawingRange) && !oldTileDrawingArea.intersects (unit.getArea ()))
				{
					addAnimationsForUnit (unit);
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::addAnimationsForUnit (const cUnit& unit)
{
	if (!cSettings::getInstance ().isAnimations ()) return;

	if (unit.data.powerOnGraphic || unit.data.canWork)
	{
		assert (unit.data.ID.isABuilding());
		auto& building = static_cast<const cBuilding&>(unit);

		animations.push_back (std::make_unique<cAnimationWork> (*animationTimer, building));
	}
	if (unit.data.factorAir > 0)
	{
		assert (unit.data.ID.isAVehicle ());
		auto& vehicle = static_cast<const cVehicle&>(unit);

		animations.push_back (std::make_unique<cAnimationDither> (*animationTimer, vehicle));
	}
	if (unit.data.canBuild.compare ("BigBuilding") == 0)
	{
		assert (unit.data.ID.isAVehicle ());
		auto& vehicle = static_cast<const cVehicle&>(unit);

		animations.push_back (std::make_unique<cAnimationStartUpBuildingSite> (*animationTimer, vehicle));
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::updateUnitMenuPosition ()
{
	if (!unitMenu->getUnit ()) return;
	if (unitMenu->isHidden ()) return;

	const auto& unit = *unitMenu->getUnit ();

	const auto menuSize = unitMenu->getSize ();

	auto position = getScreenPosition (unit);

	auto unitSize = getZoomedTileSize ();
	if (unit.data.isBig) unitSize *= 2;

	if (position.x () + unitSize.x () + menuSize.x () >= getEndPosition ().x ())
	{
		position.x () -= menuSize.x ();
	}
	else
	{
		position.x () += unitSize.x ();
	}

	if (position.y () - (menuSize.y() - unitSize.y ()) / 2 <= getPosition().y())
	{
		position.y () -= (menuSize.y () - unitSize.y ()) / 2;
		position.y () += -(position.y () - getPosition ().y ());
	}
	else if (position.y () - (menuSize.y () - unitSize.y ()) / 2 + menuSize.y () >= getEndPosition ().y ())
	{
		position.y () -= (menuSize.y () - unitSize.y ()) / 2;
		position.y () -= (position.y () + menuSize.y ()) - getEndPosition ().y ();
	}
	else
	{
		position.y () -= (menuSize.y () - unitSize.y ()) / 2;
	}

	unitMenu->moveTo (position);

	updateMouseCursor ();
}

//------------------------------------------------------------------------------
void cGameMapWidget::updateMouseCursor ()
{
	auto activeMouse = getActiveMouse ();
	if (activeMouse)
	{
		updateMouseCursor (*activeMouse);
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::updateMouseCursor (cMouse& mouse)
{
	if (!isAt (mouse.getPosition ())) return;

	if (!staticMap || (unitMenu->isEnabled () && !unitMenu->isHidden () && unitMenu->isAt (mouse.getPosition ())))
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
	}
	else
	{
		mouseMode->setCursor (mouse, getMapTilePosition (mouse.getPosition ()));
	}
}

//------------------------------------------------------------------------------
cDrawingCache& cGameMapWidget::getDrawingCache ()
{
	return unitDrawingEngine.drawingCache;
}

//------------------------------------------------------------------------------
const cDrawingCache& cGameMapWidget::getDrawingCache () const
{
	return unitDrawingEngine.drawingCache;
}

//------------------------------------------------------------------------------
void cGameMapWidget::runOwnedEffects ()
{
	for (size_t i = 0; i < effects.size (); ++i)
	{
		auto& effect = effects[i];

		if (effect.use_count () == 1)
		{
			effect->run ();
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::renewDamageEffects ()
{
	if (!cSettings::getInstance ().isDamageEffects ()) return;
	if (!dynamicMap) return;

	const auto tileDrawingRange = computeTileDrawingRange ();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore (); i.next ())
	{
		auto& mapField = dynamicMap->getField (*i);

		const auto& buildings = mapField.getBuildings ();
		for (size_t i = 0; i < buildings.size (); ++i)
		{
			renewDamageEffect (*buildings[i]);
		}

		const auto& planes = mapField.getPlanes ();
		for (size_t i = 0; i < planes.size (); ++i)
		{
			renewDamageEffect (*planes[i]);
		}

		if (mapField.getVehicle ())
		{
			renewDamageEffect (*mapField.getVehicle ());
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::renewDamageEffect (const cBuilding& building)
{
	if (building.data.hasDamageEffect &&
		building.data.getHitpoints () < building.data.hitpointsMax &&
		(building.owner == player.get () || (!player || player->canSeeAnyAreaUnder (building))))
	{
		int intense = (int)(200 - 200 * ((float)building.data.getHitpoints () / building.data.hitpointsMax));
		addEffect (std::make_shared<cFxDarkSmoke> (cPosition(building.getPosition().x() * 64 + building.DamageFXPointX, building.getPosition().y() * 64 + building.DamageFXPointY), intense, windDirection));

		if (building.data.isBig && intense > 50)
		{
			intense -= 50;
			addEffect (std::make_shared<cFxDarkSmoke> (cPosition (building.getPosition().x() * 64 + building.DamageFXPointX2, building.getPosition().y() * 64 + building.DamageFXPointY2), intense, windDirection));
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::renewDamageEffect (const cVehicle& vehicle)
{
	if (vehicle.data.getHitpoints () < vehicle.data.hitpointsMax &&
		(vehicle.owner == player.get () || (!player || player->canSeeAnyAreaUnder (vehicle))))
	{
		int intense = (int)(100 - 100 * ((float)vehicle.data.getHitpoints () / vehicle.data.hitpointsMax));
		addEffect (std::make_shared<cFxDarkSmoke> (cPosition (vehicle.getPosition().x() * 64 + vehicle.DamageFXPointX + vehicle.getMovementOffset().x(), vehicle.getPosition().y() * 64 + vehicle.DamageFXPointY + vehicle.getMovementOffset().y()), intense, windDirection));
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::setWindDirection (int direction)
{
	// 360 / (2 * PI) = 57.29577f;
	windDirection = direction / 57.29577f;
}

//------------------------------------------------------------------------------
void cGameMapWidget::changeWindDirection ()
{
	if (!cSettings::getInstance ().isDamageEffects ()) return;

	static int nextChange = 25;
	static int nextDirChange = 25;
	static int dir = 90;
	static int change = 3;
	if (nextChange == 0)
	{
		nextChange = 10 + random (20);
		dir += change;
		setWindDirection (dir);
		if (dir >= 360) dir -= 360;
		else if (dir < 0) dir += 360;

		if (nextDirChange == 0)
		{
			nextDirChange = random (25) + 10;
			change = random (11) - 5;
		}
		else nextDirChange--;
	}
	else nextChange--;
}
