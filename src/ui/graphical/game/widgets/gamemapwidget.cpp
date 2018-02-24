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
#include "ui/graphical/game/control/mousemode/mousemode.h"
#include "ui/graphical/game/control/mousemode/mousemodedefault.h"
#include "ui/graphical/game/control/mousemode/mousemodeactivateloaded.h"
#include "ui/graphical/game/control/mousemode/mousemodeattack.h"
#include "ui/graphical/game/control/mousemode/mousemodedisable.h"
#include "ui/graphical/game/control/mousemode/mousemodehelp.h"
#include "ui/graphical/game/control/mousemode/mousemodeload.h"
#include "ui/graphical/game/control/mousemode/mousemoderepair.h"
#include "ui/graphical/game/control/mousemode/mousemodeselectbuildpathdestination.h"
#include "ui/graphical/game/control/mousemode/mousemodeselectbuildposition.h"
#include "ui/graphical/game/control/mousemode/mousemodesteal.h"
#include "ui/graphical/game/control/mousemode/mousemodesupplyammo.h"
#include "ui/graphical/game/control/mousemode/mousemodetransfer.h"
#include "ui/graphical/game/control/mouseaction/mouseaction.h"
#include "ui/graphical/game/animations/animation.h"
#include "ui/graphical/game/animations/animationwork.h"
#include "ui/graphical/game/animations/animationstartup.h"
#include "ui/graphical/game/animations/animationstartupbuildingsite.h"
#include "ui/graphical/game/animations/animationdither.h"
#include "ui/graphical/game/hud.h"
#include "ui/sound/soundmanager.h"
#include "ui/graphical/game/animations/animationtimer.h"
#include "ui/graphical/application.h"
#include "ui/graphical/game/control/rightmousebuttonscroller.h"
#include "game/data/map/map.h"
#include "settings.h"
#include "video.h"
#include "main.h"
#include "game/data/player/player.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"
#include "keys.h"
#include "utility/listhelpers.h"
#include "sound.h"
#include "utility/indexiterator.h"
#include "utility/random.h"
#include "input/mouse/mouse.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "input/mouse/cursor/mousecursoramount.h"
#include "input/mouse/cursor/mousecursorattack.h"
#include "output/sound/sounddevice.h"
#include "output/sound/soundchannel.h"
#include "game/logic/movejob.h"
#include "game/data/map/mapview.h"
#include "game/logic/pathcalculator.h"
#include "game/data/map/mapfieldview.h"
#include "ui/graphical/game/control/mousemode/mousemodeenter.h"

//------------------------------------------------------------------------------
cGameMapWidget::cGameMapWidget (const cBox<cPosition>& area, std::shared_ptr<const cStaticMap> staticMap_, std::shared_ptr<cAnimationTimer> animationTimer_, std::shared_ptr<cSoundManager> soundManager_, std::shared_ptr<const cFrameCounter> frameCounter) :
	cClickableWidget (area),
	animationTimer (animationTimer_),
	soundManager (soundManager_),
	staticMap (std::move (staticMap_)),
	player (nullptr),
	unitDrawingEngine (animationTimer, frameCounter),
	changeAllowed (true),
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

	using namespace std::placeholders;

	signalConnectionManager.connect (cSettings::getInstance().animationsChanged, [this]()
	{
		if (cSettings::getInstance().isAnimations())
		{
			updateActiveAnimations();
		}
		else
		{
			animations.clear();
		}
	});

	setMouseInputMode (std::make_unique<cMouseModeDefault> (mapView.get(), unitSelection, player.get()));

	// TODO: should this really be done here?
	signalConnectionManager.connect (animationTimer->triggered400ms, [&]()
	{
		const_cast<cStaticMap&> (*staticMap).generateNextAnimationFrame();
	});

	setWindDirection (random (360));
	signalConnectionManager.connect (animationTimer->triggered400ms, std::bind (&cGameMapWidget::changeWindDirection, this));

	signalConnectionManager.connect (animationTimer->triggered50ms, std::bind (&cGameMapWidget::runOwnedEffects, this));
	signalConnectionManager.connect (animationTimer->triggered100ms, std::bind (&cGameMapWidget::renewDamageEffects, this));

	unitMenu = addChild (std::make_unique<cUnitContextMenuWidget> ());
	unitMenu->disable();
	unitMenu->hide();

	rightMouseButtonScrollerWidget = addChild (std::make_unique<cRightMouseButtonScrollerWidget> (animationTimer));
	signalConnectionManager.connect (rightMouseButtonScrollerWidget->scroll, std::bind (&cGameMapWidget::scroll, this, _1));
	signalConnectionManager.connect (rightMouseButtonScrollerWidget->mouseFocusReleased, [this]() { mouseFocusReleased(); });
	signalConnectionManager.connect (rightMouseButtonScrollerWidget->stoppedScrolling, [this]() { updateMouseCursor(); });
	rightMouseButtonScrollerWidget->disable();  // mouse events will be forwarded explicitly

	mouseInputModeChanged.connect (std::bind (static_cast<void (cGameMapWidget::*)()> (&cGameMapWidget::updateMouseCursor), this));

	scrolled.connect (std::bind (&cGameMapWidget::updateUnitMenuPosition, this));
	scrolled.connect ([this]()
	{
		soundManager->setListenerPosition (getMapCenterOffset());
	});
	tileUnderMouseChanged.connect ([this] (const cPosition & tilePosition)
	{
		if (mouseMode)
		{
			mouseMode->handleMapTilePositionChanged (tilePosition);
		}
	});

	zoomFactorChanged.connect ([this]()
	{
		const auto tileDrawingRange = computeTileDrawingRange();
		const cPosition difference = tileDrawingRange.first - tileDrawingRange.second;
		const auto diameter = difference.l2Norm();
		soundManager->setMaxListeningDistance ((int) (diameter * 2));
	});

	unitSelection.mainSelectionChanged.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitSelection.mainSelectionChanged.connect (std::bind (&cGameMapWidget::updateActiveUnitCommandShortcuts, this));
	unitSelection.mainSelectionChanged.connect ([&]()
	{
		setMouseInputMode (std::make_unique<cMouseModeDefault> (mapView.get(), unitSelection, player.get()));
	});
	unitSelection.mainSelectionChanged.connect ([&]()
	{
		selectedUnitSignalConnectionManager.disconnectAll();
		const auto selectedUnit = unitSelection.getSelectedUnit();
		if (!selectedUnit) return;

		selectedUnitSignalConnectionManager.connect (selectedUnit->data.shotsChanged, std::bind (&cGameMapWidget::updateActiveUnitCommandShortcuts, this));
		selectedUnitSignalConnectionManager.connect (selectedUnit->storedResourcesChanged, std::bind (&cGameMapWidget::updateActiveUnitCommandShortcuts, this));
		selectedUnitSignalConnectionManager.connect (selectedUnit->positionChanged, std::bind (&cGameMapWidget::updateActiveUnitCommandShortcuts, this));
		selectedUnitSignalConnectionManager.connect (selectedUnit->sentryChanged, std::bind (&cGameMapWidget::updateActiveUnitCommandShortcuts, this));
		selectedUnitSignalConnectionManager.connect (selectedUnit->manualFireChanged, std::bind (&cGameMapWidget::updateActiveUnitCommandShortcuts, this));
		selectedUnitSignalConnectionManager.connect (selectedUnit->workingChanged, std::bind (&cGameMapWidget::updateActiveUnitCommandShortcuts, this));
		selectedUnitSignalConnectionManager.connect (selectedUnit->clearingChanged, std::bind (&cGameMapWidget::updateActiveUnitCommandShortcuts, this));
		selectedUnitSignalConnectionManager.connect (selectedUnit->buildingChanged, std::bind (&cGameMapWidget::updateActiveUnitCommandShortcuts, this));
		if (selectedUnit->isAVehicle())
		{
			const auto selectedVehicle = static_cast<const cVehicle*> (selectedUnit);
			selectedUnitSignalConnectionManager.connect (selectedVehicle->moveJobChanged, std::bind (&cGameMapWidget::updateActiveUnitCommandShortcuts, this));
			selectedUnitSignalConnectionManager.connect (selectedVehicle->clearingTurnsChanged, std::bind (&cGameMapWidget::updateActiveUnitCommandShortcuts, this));
			selectedUnitSignalConnectionManager.connect (selectedVehicle->buildingTurnsChanged, std::bind (&cGameMapWidget::updateActiveUnitCommandShortcuts, this));
		}
	});

	unitMenu->attackToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::Attack));
	unitMenu->transferToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::Transfer));
	unitMenu->enterToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::Enter));
	unitMenu->loadToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::Load));
	unitMenu->supplyAmmoToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::SupplyAmmo));
	unitMenu->repairToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::Repair));
	unitMenu->sabotageToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::Disable));
	unitMenu->stealToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eMouseModeType::Steal));

	unitMenu->buildClicked.connect ([&]() { if (unitMenu->getUnit()) triggeredBuild (*unitMenu->getUnit()); });
	unitMenu->distributeClicked.connect ([&]() { if (unitMenu->getUnit()) triggeredResourceDistribution (*unitMenu->getUnit()); });
	unitMenu->startClicked.connect ([&]() { if (unitMenu->getUnit()) triggeredStartWork (*unitMenu->getUnit()); });
	unitMenu->stopClicked.connect ([&]() { if (unitMenu->getUnit()) triggeredStopWork (*unitMenu->getUnit()); });
	unitMenu->autoToggled.connect ([&]() { if (unitMenu->getUnit()) triggeredAutoMoveJob (*unitMenu->getUnit()); });
	unitMenu->removeClicked.connect ([&]() { if (unitMenu->getUnit()) triggeredStartClear (*unitMenu->getUnit()); });
	unitMenu->manualFireToggled.connect ([&]() { if (unitMenu->getUnit()) triggeredManualFire (*unitMenu->getUnit()); });
	unitMenu->sentryToggled.connect ([&]() { if (unitMenu->getUnit()) triggeredSentry (*unitMenu->getUnit()); });
	unitMenu->activateClicked.connect ([&]() { if (unitMenu->getUnit()) triggeredActivate (*unitMenu->getUnit()); });
	unitMenu->researchClicked.connect ([&]() { if (unitMenu->getUnit()) triggeredResearchMenu (*unitMenu->getUnit()); });
	unitMenu->buyUpgradesClicked.connect ([&]() { if (unitMenu->getUnit()) triggeredUpgradesMenu (*unitMenu->getUnit()); });
	unitMenu->upgradeThisClicked.connect ([&]() { if (unitMenu->getUnit()) triggeredUpgradeThis (*unitMenu->getUnit()); });
	unitMenu->upgradeAllClicked.connect ([&]() { if (unitMenu->getUnit()) triggeredUpgradeAll (*unitMenu->getUnit()); });
	unitMenu->selfDestroyClicked.connect ([&]() { if (unitMenu->getUnit()) triggeredSelfDestruction (*unitMenu->getUnit()); });
	unitMenu->layMinesToggled.connect ([&]() { if (unitMenu->getUnit()) triggeredLayMines (*unitMenu->getUnit()); });
	unitMenu->collectMinesToggled.connect ([&]() { if (unitMenu->getUnit()) triggeredCollectMines (*unitMenu->getUnit()); });
	unitMenu->infoClicked.connect ([&]() { if (unitMenu->getUnit()) triggeredUnitHelp (*unitMenu->getUnit()); });
	unitMenu->doneClicked.connect ([&]() { if (unitMenu->getUnit()) triggeredUnitDone (*unitMenu->getUnit()); });

	unitMenu->attackToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->buildClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->distributeClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->transferToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->enterToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
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

	attackShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuAttack));
	attackShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasAttackEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			toggleMouseInputMode (eMouseModeType::Attack);
		}
	});

	buildShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuBuild));
	buildShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasBuildEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredBuild (*unitSelection.getSelectedUnit());
		}
	});

	transferShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuTransfer));
	transferShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasTransferEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			toggleMouseInputMode (eMouseModeType::Transfer);
		}
	});

	automoveShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuAutomove));
	automoveShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasAutoEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredAutoMoveJob (*unitSelection.getSelectedUnit());
		}
	});

	startShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuStart));
	startShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasStartEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredStartWork (*unitSelection.getSelectedUnit());
		}
	});

	stopShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuStop));
	stopShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasStopEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredStopWork (*unitSelection.getSelectedUnit());
		}
	});

	clearShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuClear));
	clearShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasRemoveEntry (unitSelection.getSelectedUnit(), player.get(), mapView.get()))
		{
			triggeredStartClear (*unitSelection.getSelectedUnit());
		}
	});

	sentryShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuSentry));
	sentryShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasSentryEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredSentry (*unitSelection.getSelectedUnit());
		}
	});

	manualFireShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuManualFire));
	manualFireShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasManualFireEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredManualFire (*unitSelection.getSelectedUnit());
		}
	});

	activateShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuActivate));
	activateShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasActivateEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredActivate (*unitSelection.getSelectedUnit());
		}
	});

	loadShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuLoad));
	loadShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasLoadEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			toggleMouseInputMode (eMouseModeType::Load);
		}
	});

	relaodShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuReload));
	relaodShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasSupplyEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			toggleMouseInputMode (eMouseModeType::SupplyAmmo);
		}
	});

	enterShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuEnter));
	enterShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasEnterEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			toggleMouseInputMode (eMouseModeType::Enter);
		}
	});

	repairShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuRepair));
	repairShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasRepairEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			toggleMouseInputMode (eMouseModeType::Repair);
		}
	});

	layMineShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuLayMine));
	layMineShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasLayMinesEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredLayMines (*unitSelection.getSelectedUnit());
		}
	});

	clearMineShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuClearMine));
	clearMineShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasCollectMinesEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredCollectMines (*unitSelection.getSelectedUnit());
		}
	});

	disableShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuDisable));
	disableShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasSabotageEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			toggleMouseInputMode (eMouseModeType::Disable);
		}
	});

	stealShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuSteal));
	stealShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasStealEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			toggleMouseInputMode (eMouseModeType::Steal);
		}
	});

	infoShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuInfo));
	infoShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasInfoEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredUnitHelp (*unitSelection.getSelectedUnit());
		}
	});

	distributeShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuDistribute));
	distributeShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasDistributeEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredResourceDistribution (*unitSelection.getSelectedUnit());
		}
	});

	researchShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuResearch));
	researchShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasResearchEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredResearchMenu (*unitSelection.getSelectedUnit());
		}
	});

	upgradeShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuUpgrade));
	upgradeShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasBuyEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredUpgradesMenu (*unitSelection.getSelectedUnit());
		}
	});

	destroyShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuDestroy));
	destroyShortcut->triggered.connect ([this]()
	{
		if (cUnitContextMenuWidget::unitHasSelfDestroyEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredSelfDestruction (*unitSelection.getSelectedUnit());
		}
	});

	buildCollidingShortcutsMap();
}

//------------------------------------------------------------------------------
cGameMapWidget::~cGameMapWidget()
{
}

//------------------------------------------------------------------------------
void cGameMapWidget::setMapView(std::shared_ptr<const cMapView> mapView_)
{
	std::swap (mapView, mapView_);

	mapViewSignalConnectionManager.disconnectAll();

	if (mapView != nullptr)
	{
		mapViewSignalConnectionManager.connect (mapView->unitDissappeared, [&](const cUnit & unit)
		{
			if (unitSelection.isSelected (unit))
			{
				unitSelection.deselectUnit (unit);
			}
		});
		mapViewSignalConnectionManager.connect (mapView->unitAppeared, [&](const cUnit & unit)
		{
			if (!cSettings::getInstance().isAnimations()) return;

			const auto tileDrawingRange = computeTileDrawingRange();
			const auto tileDrawingArea = cBox<cPosition> (tileDrawingRange.first, tileDrawingRange.second - cPosition (1, 1));

			if (tileDrawingArea.intersects (unit.getArea()))
			{
				addAnimationsForUnit (unit);
				animations.push_back (std::make_unique<cAnimationStartUp> (*animationTimer, unit));
			}
		});
		mapViewSignalConnectionManager.connect (mapView->unitMoved, [&](const cUnit & unit, const cPosition & oldPosition)
		{
			if (!cSettings::getInstance().isAnimations()) return;

			const auto tileDrawingRange = computeTileDrawingRange();
			const auto tileDrawingArea = cBox<cPosition>(tileDrawingRange.first, tileDrawingRange.second - cPosition(1, 1));

			if (tileDrawingArea.intersects(unit.getArea()) && !tileDrawingArea.intersects(cBox<cPosition>(oldPosition, oldPosition + unit.getArea().getSize() - cPosition(1, 1))))
			{
				addAnimationsForUnit(unit);
			}
		});
	}

	if (mouseMode != nullptr)
	{
		mouseMode->setMap(mapView.get());
	}

	if (mapView != mapView_)
	{
		unitSelection.deselectUnits();
	}

	updateActiveAnimations();
}

//------------------------------------------------------------------------------
void cGameMapWidget::setPlayer (std::shared_ptr<const cPlayer> player_)
{
	player = std::move (player_);

	unitLockList.setPlayer (player.get());

	if (mouseMode != nullptr)
	{
		mouseMode->setPlayer (player.get());
	}
}

void cGameMapWidget::setUnitsData(std::shared_ptr<const cUnitsData> unitsData_)
{
	unitsData = unitsData_;
}

//------------------------------------------------------------------------------
void cGameMapWidget::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	drawTerrain();

	if (shouldDrawGrid) drawGrid();

	drawEffects (true);

	unitDrawingEngine.drawingCache.resetStatistics();

	drawBaseUnits();
	drawTopBuildings();
	drawShips();
	drawAboveSeaBaseUnits();
	drawVehicles();
	drawConnectors();
	drawPlanes();

	auto selectedVehicle = unitSelection.getSelectedVehicle();
	if (shouldDrawSurvey || (selectedVehicle && selectedVehicle->getOwner() == player.get() && selectedVehicle->getStaticUnitData().canSurvey))
	{
		drawResources();
	}

	if (selectedVehicle && ((selectedVehicle->getMoveJob() && !selectedVehicle->isUnitMoving()) || selectedVehicle->BuildPath))
	{
		drawPath (*selectedVehicle);
	}

	drawSelectionBox();

	SDL_SetClipRect (cVideo::buffer, nullptr);

	drawUnitCircles();
	drawExitPoints();
	drawBuildBand();

	if (lockActive) drawLockList ();

	drawEffects (false);

	//displayMessages ();

	cWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
void cGameMapWidget::setZoomFactor (float zoomFactor_, bool center)
{
	const auto oldZoom = getZoomFactor();
	const auto oldTileDrawingRange = computeTileDrawingRange();

	internalZoomFactor = zoomFactor_;

	internalZoomFactor = std::max (internalZoomFactor, computeMinimalZoomFactor());
	internalZoomFactor = std::min (1.f, internalZoomFactor);

	const auto newZoom = getZoomFactor();

	if (oldZoom != newZoom)
	{
		updateActiveAnimations (oldTileDrawingRange);

		zoomFactorChanged();

		cPosition scrollOffset (0, 0);
		if (center)
		{
			const auto oldScreenPixelX = getSize().x() / oldZoom;
			const auto newScreenPixelX = getSize().x() / newZoom;
			scrollOffset.x() = (int) ((oldScreenPixelX - newScreenPixelX) / 2);

			const auto oldScreenPixelY = getSize().y() / oldZoom;
			const auto newScreenPixelY = getSize().y() / newZoom;
			scrollOffset.y() = (int) ((oldScreenPixelY - newScreenPixelY) / 2);
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
void cGameMapWidget::toggleHelpMode()
{
	toggleMouseInputMode (eMouseModeType::Help);
}

//------------------------------------------------------------------------------
cBox<cPosition> cGameMapWidget::getDisplayedMapArea() const
{
	auto tileDrawingRange = computeTileDrawingRange();

	return cBox<cPosition> (tileDrawingRange.first, tileDrawingRange.second - 1);
}

//------------------------------------------------------------------------------
float cGameMapWidget::getZoomFactor() const
{
	return (float)getZoomedTileSize().x() / cStaticMap::tilePixelWidth;   // should make no difference if we use y instead
}

//------------------------------------------------------------------------------
cUnitSelection& cGameMapWidget::getUnitSelection()
{
	return unitSelection;
}

//------------------------------------------------------------------------------
const cUnitSelection& cGameMapWidget::getUnitSelection() const
{
	return unitSelection;
}

//------------------------------------------------------------------------------
cUnitLockList& cGameMapWidget::getUnitLockList()
{
	return unitLockList;
}

//------------------------------------------------------------------------------
const cUnitLockList& cGameMapWidget::getUnitLockList() const
{
	return unitLockList;
}

//------------------------------------------------------------------------------
void cGameMapWidget::toggleUnitContextMenu (const cUnit* unit)
{
	unitContextMenuSignalConnectionManager.disconnectAll();
	if (unitMenu->isEnabled() || unit == nullptr)
	{
		unitMenu->disable();
		unitMenu->hide();
		updateMouseCursor();
	}
	else
	{
		unitMenu->setUnit (unit, mouseMode->getType(), player.get(), mapView.get());
		unitMenu->enable();
		unitMenu->show();
		updateUnitMenuPosition();

		unitContextMenuSignalConnectionManager.connect (unit->positionChanged, [this, unit]()
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

	mouseModeSignalConnectionManager.disconnectAll();
	mouseModeSignalConnectionManager.connect (mouseMode->needRefresh, std::bind (static_cast<void (cGameMapWidget::*)()> (&cGameMapWidget::updateMouseCursor), this));

	auto activeMouse = getActiveMouse();
	if (activeMouse && getArea().withinOrTouches (activeMouse->getPosition()))
	{
		mouseMode->handleMapTilePositionChanged (getMapTilePosition (activeMouse->getPosition()));
	}
	else
	{
		mouseMode->handleMapTilePositionChanged (cPosition (-1, -1));
	}

	if (!newMouseMode || newMouseMode->getType() != mouseMode->getType()) mouseInputModeChanged();
}

//------------------------------------------------------------------------------
void cGameMapWidget::toggleMouseInputMode (eMouseModeType mouseModeType)
{
	if (mouseMode->getType() == mouseModeType)
	{
		setMouseInputMode (std::make_unique<cMouseModeDefault> (mapView.get(), unitSelection, player.get()));
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
				setMouseInputMode (std::make_unique<cMouseModeDefault> (mapView.get(), unitSelection, player.get()));
				break;
			case eMouseModeType::Attack:
				setMouseInputMode (std::make_unique<cMouseModeAttack> (mapView.get(), unitSelection, player.get()));
				break;
			case eMouseModeType::SelectBuildPathDestintaion:
				setMouseInputMode (std::make_unique<cMouseModeSelectBuildPathDestination> (mapView.get(), unitSelection, player.get()));
				break;
			case eMouseModeType::Transfer:
				setMouseInputMode (std::make_unique<cMouseModeTransfer> (mapView.get(), unitSelection, player.get()));
				break;
			case eMouseModeType::Load:
				setMouseInputMode (std::make_unique<cMouseModeLoad> (mapView.get(), unitSelection, player.get()));
				break;
			case eMouseModeType::Enter:
				setMouseInputMode(std::make_unique<cMouseModeEnter>(mapView.get(), unitSelection, player.get()));
				break;
			case eMouseModeType::SupplyAmmo:
				setMouseInputMode (std::make_unique<cMouseModeSupplyAmmo> (mapView.get(), unitSelection, player.get()));
				break;
			case eMouseModeType::Repair:
				setMouseInputMode (std::make_unique<cMouseModeRepair> (mapView.get(), unitSelection, player.get()));
				break;
			case eMouseModeType::Disable:
				setMouseInputMode (std::make_unique<cMouseModeDisable> (mapView.get(), unitSelection, player.get()));
				break;
			case eMouseModeType::Steal:
				setMouseInputMode (std::make_unique<cMouseModeSteal> (mapView.get(), unitSelection, player.get()));
				break;
			case eMouseModeType::Help:
				setMouseInputMode (std::make_unique<cMouseModeHelp> (mapView.get(), unitSelection, player.get()));
				break;
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::scroll (const cPosition& offset)
{
	const auto activeMouse = getActiveMouse();

	cPosition oldTileUnderMouse (-1, -1);
	if (activeMouse && getArea().withinOrTouches (activeMouse->getPosition()))
	{
		oldTileUnderMouse = getMapTilePosition (activeMouse->getPosition());
	}

	const auto oldPixelOffset = pixelOffset;
	const auto oldTileDrawingRange = computeTileDrawingRange();

	pixelOffset += offset;

	pixelOffset.x() = std::max (0, pixelOffset.x());
	pixelOffset.y() = std::max (0, pixelOffset.y());

	const auto maximalPixelOffset = computeMaximalPixelOffset();

	pixelOffset.x() = std::min (maximalPixelOffset.x(), pixelOffset.x());
	pixelOffset.y() = std::min (maximalPixelOffset.y(), pixelOffset.y());

	if (oldPixelOffset != pixelOffset)
	{
		cPosition newTileUnderMouse (-1, -1);
		if (activeMouse && getArea().withinOrTouches (activeMouse->getPosition()))
		{
			newTileUnderMouse = getMapTilePosition (activeMouse->getPosition());
		}

		if (newTileUnderMouse != oldTileUnderMouse) tileUnderMouseChanged (newTileUnderMouse);

		updateActiveAnimations (oldTileDrawingRange);

		scrolled();
	}
}

//------------------------------------------------------------------------------
const cPosition& cGameMapWidget::getPixelOffset() const
{
	return pixelOffset;
}

//------------------------------------------------------------------------------
void cGameMapWidget::centerAt (const cPosition& position)
{
	const auto zoomedTileSize = getZoomedTileSize();

	cPosition newPixelOffset;
	newPixelOffset.x() = position.x() * cStaticMap::tilePixelWidth - ((int) (((float)getSize().x() / (2 * zoomedTileSize.x())) * cStaticMap::tilePixelWidth)) + cStaticMap::tilePixelWidth / 2;
	newPixelOffset.y() = position.y() * cStaticMap::tilePixelHeight - ((int) (((float)getSize().y() / (2 * zoomedTileSize.y())) * cStaticMap::tilePixelHeight)) + cStaticMap::tilePixelHeight / 2;

	scroll (newPixelOffset - pixelOffset);
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::getMapCenterOffset()
{
	const auto zoomedTileSize = getZoomedTileSize();

	cPosition center;
	center.x() = pixelOffset.x() / cStaticMap::tilePixelWidth + (getSize().x() / (2 * zoomedTileSize.x()));
	center.y() = pixelOffset.y() / cStaticMap::tilePixelHeight + (getSize().y() / (2 * zoomedTileSize.y()));

	return center;
}

//------------------------------------------------------------------------------
bool cGameMapWidget::startFindBuildPosition(const sID& buildId)
{
	auto mouseMode = std::make_unique<cMouseModeSelectBuildPosition>(mapView.get(), unitSelection, player.get(), buildId);
	
	// validate if there is any valid position, before setting mouse mode
	const auto selectedVehicle = unitSelection.getSelectedVehicle();
	if (selectedVehicle)
	{
		auto buildPos = mouseMode->findNextBuildPosition(selectedVehicle->getPosition(), selectedVehicle->getPosition(), *unitsData);
		if (!buildPos.first)
		{
			return false;
		}
	}
	setMouseInputMode (std::move(mouseMode));

	return true;
}

//------------------------------------------------------------------------------
void cGameMapWidget::startFindPathBuildPosition()
{
	setMouseInputMode (std::make_unique<cMouseModeSelectBuildPathDestination> (mapView.get(), unitSelection, player.get()));
}

//------------------------------------------------------------------------------
void cGameMapWidget::startActivateVehicle (const cUnit& unit, size_t index)
{
	setMouseInputMode (std::make_unique<cMouseModeActivateLoaded> (mapView.get(), unitSelection, player.get(), index));
}

//------------------------------------------------------------------------------
void cGameMapWidget::addEffect (std::shared_ptr<cFx> effect, bool playSound)
{
	if (effect != nullptr)
	{
		if (playSound) effect->playSound (*soundManager);
		effects.push_back (std::move (effect));
	}
}

//------------------------------------------------------------------------------
float cGameMapWidget::computeMinimalZoomFactor() const
{
	// inequality to be fulfilled:
	//
	//   round(tile_x * zoom) * map_x <= size_x
	//
	// we first discard the 'round' and solve for zoom:
	//
	//   zoom = size_x / (map_x * tile_x)

	auto xZoom = (float)getSize().x() / (staticMap->getSize().x() * cStaticMap::tilePixelWidth);
	auto yZoom = (float)getSize().y() / (staticMap->getSize().y() * cStaticMap::tilePixelHeight);

	// then we try to fix if round would have rounded up:

	xZoom = std::max (xZoom, (float) ((int) (cStaticMap::tilePixelWidth * xZoom) + (xZoom >= 1.0f ? 0 : 1)) / cStaticMap::tilePixelWidth);
	yZoom = std::max (yZoom, (float) ((int) (cStaticMap::tilePixelHeight * yZoom) + (yZoom >= 1.0f ? 0 : 1)) / cStaticMap::tilePixelHeight);

	return std::max (xZoom, yZoom);
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::computeMaximalPixelOffset() const
{
	const auto x = staticMap->getSize(). x() * cStaticMap::tilePixelWidth - (int) (getSize().x() / getZoomFactor());
	const auto y = staticMap->getSize(). y() * cStaticMap::tilePixelHeight - (int) (getSize().y() / getZoomFactor());

	return cPosition (x, y);
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::zoomSize (const cPosition& size, float zoomFactor) const
{
	return cPosition (Round (size.x() * zoomFactor), Round (size.y() * zoomFactor));
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::getZoomedTileSize() const
{
	// this should be the only place where the internalZoomFactor is used directly
	return zoomSize (cPosition (cStaticMap::tilePixelHeight, cStaticMap::tilePixelWidth), internalZoomFactor);
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::getZoomedStartTilePixelOffset() const
{
	return zoomSize (cPosition (pixelOffset.x() % cStaticMap::tilePixelWidth, pixelOffset.y() % cStaticMap::tilePixelHeight), getZoomFactor());
}

//------------------------------------------------------------------------------
std::pair<cPosition, cPosition> cGameMapWidget::computeTileDrawingRange() const
{
	const auto zoomedTileSize = getZoomedTileSize();

	const cPosition drawingPixelRange = getSize() + getZoomedStartTilePixelOffset();

	const cPosition tilesSize ((int)std::ceil (drawingPixelRange.x() / zoomedTileSize.x()), (int)std::ceil (drawingPixelRange.y() / zoomedTileSize.y()));

	cPosition startTile ((int)std::floor (pixelOffset.x() / cStaticMap::tilePixelWidth), (int)std::floor (pixelOffset.y() / cStaticMap::tilePixelHeight));
	cPosition endTile (startTile + tilesSize + 1);

	startTile.x() = std::max (0, startTile.x());
	startTile.y() = std::max (0, startTile.y());

	endTile.x() = std::min (staticMap->getSize().x(), endTile.x());
	endTile.y() = std::min (staticMap->getSize().y(), endTile.y());

	return std::make_pair (startTile, endTile);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawTerrain()
{
	const auto zoomedTileSize = getZoomedTileSize();
	const auto tileDrawingRange = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
	{
		const auto& terrain = staticMap->getTerrain (*i);

		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);
		if (shouldDrawFog && (!player || !player->canSeeAt (*i)))
		{
			if (!cSettings::getInstance().shouldDoPrescale() && (terrain.shw->w != zoomedTileSize.x() || terrain.shw->h != zoomedTileSize.y()))
			{
				scaleSurface (terrain.shw_org.get(), terrain.shw.get(), zoomedTileSize.x(), zoomedTileSize.y());
			}
			SDL_BlitSurface (terrain.shw.get(), nullptr, cVideo::buffer, &drawDestination);
		}
		else
		{
			if (!cSettings::getInstance().shouldDoPrescale() && (terrain.sf->w != zoomedTileSize.x() || terrain.sf->h != zoomedTileSize.y()))
			{
				scaleSurface (terrain.sf_org.get(), terrain.sf.get(), zoomedTileSize.x(), zoomedTileSize.y());
			}
			SDL_BlitSurface (terrain.sf.get(), nullptr, cVideo::buffer, &drawDestination);
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawGrid()
{
	const auto zoomedTileSize = getZoomedTileSize();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	SDL_Rect destY = {getPosition().x(), getPosition().y() + zoomedTileSize.y() - zoomedStartTilePixelOffset.y(), getSize().x(), 1};
	for (; destY.y < getEndPosition().y(); destY.y += zoomedTileSize.y())
	{
		SDL_FillRect (cVideo::buffer, &destY, GRID_COLOR);
	}

	SDL_Rect destX = {getPosition().x() + zoomedTileSize.x() - zoomedStartTilePixelOffset.x(), getPosition().y(), 1, getSize().y()};
	for (; destX.x < getEndPosition().x(); destX.x += zoomedTileSize.x())
	{
		SDL_FillRect (cVideo::buffer, &destX, GRID_COLOR);
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawEffects (bool bottom)
{
	SDL_Rect clipRect = getArea().toSdlRect();
	SDL_SetClipRect (cVideo::buffer, &clipRect);

	const cPosition originalTileSize (cStaticMap::tilePixelWidth, cStaticMap::tilePixelHeight);

	for (auto it = effects.begin(); it != effects.end();)   // ATTENTION: erase in loop. do not use continue;
	{
		auto& effect = *it;

		if (effect->isFinished())
		{
			it = effects.erase (it);
		}
		else
		{
			if (effect->bottom == bottom &&
				(!player || player->canSeeAt (effect->getPosition() / originalTileSize)))
			{
				cPosition screenDestination;
				screenDestination.x() = getPosition().x() + static_cast<int> ((effect->getPosition().x() - pixelOffset.x()) * getZoomFactor());
				screenDestination.y() = getPosition().y() + static_cast<int> ((effect->getPosition().y() - pixelOffset.y()) * getZoomFactor());
				effect->draw (getZoomFactor(), screenDestination);
			}

			++it;
		}
	}
	SDL_SetClipRect (cVideo::buffer, nullptr);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawBaseUnits()
{
	if (!mapView) return;

	const auto zoomedTileSize = getZoomedTileSize();
	const auto tileDrawingRange = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
	{
		auto& mapField = mapView->getField (*i);
		const auto& buildings = mapField.getBuildings();
		for (auto it = buildings.rbegin(); it != buildings.rend(); ++it)
		{
			if (*it == nullptr) continue; // should never happen

			const auto& building = * (*it);

			if (!building.isRubble() && (
				building.getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_BENEATH_SEA &&
				building.getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_BASE))
				break;

			if (shouldDrawUnit (building, *i, tileDrawingRange))
			{
				const auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, building.getPosition());
				unitDrawingEngine.drawUnit (building, drawDestination, getZoomFactor(), &unitSelection, player.get());
			}
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawTopBuildings()
{
	if (!mapView) return;

	const auto zoomedTileSize = getZoomedTileSize();
	const auto tileDrawingRange = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
	{
		auto& mapField = mapView->getField (*i);
		auto building = mapField.getTopBuilding();
		if (building == nullptr) continue;
		if (building->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_GROUND) continue;
		if (!shouldDrawUnit (*building, *i, tileDrawingRange)) continue;

		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, building->getPosition());
		unitDrawingEngine.drawUnit (*building, drawDestination, getZoomFactor(), &unitSelection, player.get());

		//if (debugOutput.debugBaseClient && building->SubBase)
		//	drawTopBuildings_DebugBaseClient (*building, drawDestination);
		//if (debugOutput.debugBaseServer && building->SubBase)
		//	drawTopBuildings_DebugBaseServer (*building, drawDestination, pos);
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawShips()
{
	if (!mapView) return;

	const auto zoomedTileSize = getZoomedTileSize();
	const auto tileDrawingRange = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
	{
		auto& mapField = mapView->getField (*i);
		auto vehicle = mapField.getVehicle();
		if (vehicle == nullptr) continue;
		if (vehicle->getStaticUnitData().factorSea > 0 && vehicle->getStaticUnitData().factorGround == 0)
		{
			auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);
			unitDrawingEngine.drawUnit (*vehicle, drawDestination, getZoomFactor(), *mapView, &unitSelection, player.get());
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawAboveSeaBaseUnits()
{
	if (!mapView) return;

	const auto zoomedTileSize = getZoomedTileSize();
	const auto tileDrawingRange = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
	{
		auto& mapField = mapView->getField (*i);

		const auto& buildings = mapField.getBuildings();
		for (auto it = buildings.begin(); it != buildings.end(); ++it)
		{
			const auto& building = * (*it);
			if (building.getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE_SEA)
			{
				const auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, building.getPosition());
				unitDrawingEngine.drawUnit (building, drawDestination, getZoomFactor(), &unitSelection, player.get());
			}
		}
		for (auto it = buildings.begin(); it != buildings.end(); ++it)
		{
			const auto& building = * (*it);
			if ((*it)->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE_BASE)
			{
				const auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, building.getPosition());
				unitDrawingEngine.drawUnit (building, drawDestination, getZoomFactor(), &unitSelection, player.get());
			}
		}

		auto vehicle = mapField.getVehicle();
		if (vehicle && (vehicle->isUnitClearing() || vehicle->isUnitBuildingABuilding()))
		{
			if (shouldDrawUnit (*vehicle, *i, tileDrawingRange))
			{
				const auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, vehicle->getPosition());
				unitDrawingEngine.drawUnit (*vehicle, drawDestination, getZoomFactor(), *mapView, &unitSelection, player.get());
			}
		}
	}
}

//------------------------------------------------------------------------------
bool cGameMapWidget::shouldDrawUnit (const cUnit& unit, const cPosition& visitingPosition, const std::pair<cPosition, cPosition>& tileDrawingRange)
{
	assert (unit.isAbove (visitingPosition));

	if (!unit.getIsBig())
	{
		return true;
	}
	else
	{
		if (unit.getPosition().x() < tileDrawingRange.first.x() || unit.getPosition().y() < tileDrawingRange.first.y())
		{
			cBox<cPosition> tileDrawingBox (tileDrawingRange.first, tileDrawingRange.second - cPosition (1, 1));
			auto intersectedArea = tileDrawingBox.intersection (unit.getArea());

			return visitingPosition == intersectedArea.getMinCorner();
		}
		else return visitingPosition == unit.getPosition();
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawVehicles()
{
	if (!mapView) return;

	const auto zoomedTileSize = getZoomedTileSize();
	const auto tileDrawingRange = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
	{
		auto& mapField = mapView->getField (*i);
		auto vehicle = mapField.getVehicle();
		if (vehicle == nullptr) continue;
		if (vehicle->getStaticUnitData().factorGround != 0 && !vehicle->isUnitBuildingABuilding() && !vehicle->isUnitClearing())
		{
			auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);
			unitDrawingEngine.drawUnit (*vehicle, drawDestination, getZoomFactor(), *mapView, &unitSelection, player.get());
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawConnectors()
{
	if (!mapView) return;

	const auto zoomedTileSize = getZoomedTileSize();
	const auto tileDrawingRange = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
	{
		auto& mapField = mapView->getField (*i);
		auto building = mapField.getTopBuilding();
		if (building == nullptr) continue;
		if (building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE)
		{
			auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);
			unitDrawingEngine.drawUnit (*building, drawDestination, getZoomFactor(), &unitSelection, player.get());
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawPlanes()
{
	if (!mapView) return;

	const auto zoomedTileSize = getZoomedTileSize();
	const auto tileDrawingRange = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
	{
		auto& mapField = mapView->getField (*i);
		const auto& planes = mapField.getPlanes();

		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);
		for (auto it = planes.rbegin(); it != planes.rend(); ++it)
		{
			auto& plane = **it;
			unitDrawingEngine.drawUnit (plane, drawDestination, getZoomFactor(), *mapView, &unitSelection, player.get());
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawResources()
{
	if (!mapView) return;

	const auto zoomedTileSize = getZoomedTileSize();
	const auto tileDrawingRange = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	SDL_Rect tmp, src = {0, 0, Uint16 (zoomedTileSize.x()), Uint16 (zoomedTileSize.y())};
	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
	{
		if (player && !player->hasResourceExplored (*i)) continue;
		if (mapView->isBlocked (*i)) continue;

		const auto& resource = mapView->getResource (*i);
		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);

		if (resource.typ == eResourceType::None)
		{
			src.x = 0;
			tmp = drawDestination;
			if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_metal->w != ResourceData.res_metal_org->w / 64 * zoomedTileSize.x() || ResourceData.res_metal->h != zoomedTileSize.y())) scaleSurface (ResourceData.res_metal_org.get(), ResourceData.res_metal.get(), ResourceData.res_metal_org->w / 64 * zoomedTileSize.x(), zoomedTileSize.y());
			SDL_BlitSurface (ResourceData.res_metal.get(), &src, cVideo::buffer, &tmp);
		}
		else
		{
			src.x = resource.value * zoomedTileSize.x();
			tmp = drawDestination;
			if (resource.typ == eResourceType::Metal)
			{
				if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_metal->w != ResourceData.res_metal_org->w / 64 * zoomedTileSize.x() || ResourceData.res_metal->h != zoomedTileSize.y())) scaleSurface (ResourceData.res_metal_org.get(), ResourceData.res_metal.get(), ResourceData.res_metal_org->w / 64 * zoomedTileSize.x(), zoomedTileSize.y());
				SDL_BlitSurface (ResourceData.res_metal.get(), &src, cVideo::buffer, &tmp);
			}
			else if (resource.typ == eResourceType::Oil)
			{
				if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_oil->w != ResourceData.res_oil_org->w / 64 * zoomedTileSize.x() || ResourceData.res_oil->h != zoomedTileSize.y())) scaleSurface (ResourceData.res_oil_org.get(), ResourceData.res_oil.get(), ResourceData.res_oil_org->w / 64 * zoomedTileSize.x(), zoomedTileSize.y());
				SDL_BlitSurface (ResourceData.res_oil.get(), &src, cVideo::buffer, &tmp);
			}
			else if (resource.typ == eResourceType::Gold)
			{
				if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_gold->w != ResourceData.res_gold_org->w / 64 * zoomedTileSize.x() || ResourceData.res_gold->h != zoomedTileSize.y())) scaleSurface (ResourceData.res_gold_org.get(), ResourceData.res_gold.get(), ResourceData.res_gold_org->w / 64 * zoomedTileSize.x(), zoomedTileSize.y());
				SDL_BlitSurface (ResourceData.res_gold.get(), &src, cVideo::buffer, &tmp);
			}
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawSelectionBox()
{
	if (!unitSelectionBox.isValid()) return;

	const auto zoomedTileSize = getZoomedTileSize();

	const auto zoomOffX = (int) (pixelOffset.x() * getZoomFactor());
	const auto zoomOffY = (int) (pixelOffset.y() * getZoomFactor());

	const int mouseTopX = static_cast<int> (std::min (unitSelectionBox.getBox().getMinCorner()[0], unitSelectionBox.getBox().getMaxCorner()[0]) * zoomedTileSize. x());
	const int mouseTopY = static_cast<int> (std::min (unitSelectionBox.getBox().getMinCorner()[1], unitSelectionBox.getBox().getMaxCorner()[1]) * zoomedTileSize. y());
	const int mouseBottomX = static_cast<int> (std::max (unitSelectionBox.getBox().getMinCorner()[0], unitSelectionBox.getBox().getMaxCorner()[0]) * zoomedTileSize. x());
	const int mouseBottomY = static_cast<int> (std::max (unitSelectionBox.getBox().getMinCorner()[1], unitSelectionBox.getBox().getMaxCorner()[1]) * zoomedTileSize. y());
	const Uint32 color = 0xFFFFFF00;
	SDL_Rect d;

	d.x = mouseTopX - zoomOffX + getPosition(). x();
	d.y = mouseBottomY - zoomOffY + getPosition().y();
	d.w = mouseBottomX - mouseTopX;
	d.h = 1;
	SDL_FillRect (cVideo::buffer, &d, color);

	d.x = mouseTopX - zoomOffX + getPosition().x();
	d.y = mouseTopY - zoomOffY + getPosition().y();
	d.w = mouseBottomX - mouseTopX;
	d.h = 1;
	SDL_FillRect (cVideo::buffer, &d, color);

	d.x = mouseTopX - zoomOffX + getPosition().x();
	d.y = mouseTopY - zoomOffY + getPosition().y();
	d.w = 1;
	d.h = mouseBottomY - mouseTopY;
	SDL_FillRect (cVideo::buffer, &d, color);

	d.x = mouseBottomX - zoomOffX + getPosition().x();
	d.y = mouseTopY - zoomOffY + getPosition().y();
	d.w = 1;
	d.h = mouseBottomY - mouseTopY;
	SDL_FillRect (cVideo::buffer, &d, color);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawUnitCircles()
{
	auto clipRect = getArea().toSdlRect();
	SDL_SetClipRect (cVideo::buffer, &clipRect);

	auto selectedVehicle = unitSelection.getSelectedVehicle();
	auto selectedBuilding = unitSelection.getSelectedBuilding();

	const auto zoomedTileSize = getZoomedTileSize();

	if (selectedVehicle && selectedVehicle->isDisabled() == false)
	{
		const bool movementOffset = !selectedVehicle->isUnitBuildingABuilding() && !selectedVehicle->isUnitClearing();
		const auto screenPosition = getScreenPosition (*selectedVehicle, movementOffset);
		if (shouldDrawScan)
		{
			if (selectedVehicle->getIsBig())
			{
				drawCircle (screenPosition.x() + zoomedTileSize.x(), screenPosition.y() + zoomedTileSize.y(), selectedVehicle->data.getScan() * zoomedTileSize.x(), SCAN_COLOR, *cVideo::buffer);
			}
			else
			{
				drawCircle (screenPosition.x() + zoomedTileSize.x() / 2, screenPosition.y() + zoomedTileSize.y() / 2, selectedVehicle->data.getScan() * zoomedTileSize.x(), SCAN_COLOR, *cVideo::buffer);
			}
		}
		if (shouldDrawRange)
		{
			if (selectedVehicle->getStaticUnitData().canAttack & TERRAIN_AIR) drawCircle(screenPosition.x() + zoomedTileSize.x() / 2, screenPosition.y() + zoomedTileSize.y() / 2, selectedVehicle->data.getRange() * zoomedTileSize.x() + 2, RANGE_AIR_COLOR, *cVideo::buffer);
			else drawCircle (screenPosition.x() + zoomedTileSize.x() / 2, screenPosition.y() + zoomedTileSize.y() / 2, selectedVehicle->data.getRange() * zoomedTileSize.x() + 1, RANGE_GROUND_COLOR, *cVideo::buffer);
		}
	}
	else if (selectedBuilding && selectedBuilding->isDisabled() == false)
	{
		const auto screenPosition = getScreenPosition (*selectedBuilding);
		if (shouldDrawScan)
		{
			if (selectedBuilding->getIsBig())
			{
				drawCircle (screenPosition. x() + zoomedTileSize.x(),
							screenPosition. y() + zoomedTileSize.y(),
							selectedBuilding->data.getScan() * zoomedTileSize.x(), SCAN_COLOR, *cVideo::buffer);
			}
			else
			{
				drawCircle (screenPosition. x() + zoomedTileSize.x() / 2,
							screenPosition. y() + zoomedTileSize.y() / 2,
							selectedBuilding->data.getScan() * zoomedTileSize.x(), SCAN_COLOR, *cVideo::buffer);
			}
		}
		if (shouldDrawRange && (selectedBuilding->getStaticUnitData().canAttack & TERRAIN_GROUND) && !selectedBuilding->getStaticUnitData().explodesOnContact)
		{
			drawCircle (screenPosition. x() + zoomedTileSize.x() / 2,
						screenPosition. y() + zoomedTileSize.y() / 2,
						selectedBuilding->data.getRange() * zoomedTileSize.x() + 2, RANGE_GROUND_COLOR, *cVideo::buffer);
		}
		if (shouldDrawRange && (selectedBuilding->getStaticUnitData().canAttack & TERRAIN_AIR))
		{
			drawCircle (screenPosition. x() + zoomedTileSize.x() / 2,
						screenPosition. y() + zoomedTileSize.y() / 2,
						selectedBuilding->data.getRange() * zoomedTileSize.x() + 2, RANGE_AIR_COLOR, *cVideo::buffer);
		}
	}

	SDL_SetClipRect (cVideo::buffer, nullptr);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawExitPoints()
{
	auto selectedVehicle = unitSelection.getSelectedVehicle();
	auto selectedBuilding = unitSelection.getSelectedBuilding();

	if (selectedVehicle && selectedVehicle->isDisabled() == false)
	{
		if (mapView && selectedVehicle->getOwner() == player.get() &&
			(
				(selectedVehicle->isUnitBuildingABuilding() && selectedVehicle->getBuildTurns() == 0) ||
				(selectedVehicle->isUnitClearing() && selectedVehicle->getClearingTurns() == 0)
			) && !selectedVehicle->BuildPath)
		{
			drawExitPointsIf (*selectedVehicle, [&] (const cPosition & position) { return mapView->possiblePlace (*selectedVehicle, position); });
		}
		if (mouseMode->getType() == eMouseModeType::Activate && selectedVehicle->getOwner() == player.get())
		{
			auto activateMouseMode = static_cast<cMouseModeActivateLoaded*> (mouseMode.get());
			auto unitToExit = selectedVehicle->storedUnits[activateMouseMode->getVehicleToActivateIndex()]->getStaticUnitData();
			drawExitPointsIf (*selectedVehicle, [&] (const cPosition & position) { return selectedVehicle->canExitTo (position, *mapView, unitToExit); });
		}
	}
	else if (selectedBuilding && selectedBuilding->isDisabled() == false)
	{
		if (!selectedBuilding->isBuildListEmpty() &&
			!selectedBuilding->isUnitWorking() &&
			selectedBuilding->getBuildListItem (0).getRemainingMetal() <= 0 &&
			selectedBuilding->getOwner() == player.get())
		{
			auto unitToExit = unitsData->getStaticUnitData(selectedBuilding->getBuildListItem (0).getType());
			drawExitPointsIf (*selectedBuilding, [&] (const cPosition & position) { return selectedBuilding->canExitTo (position, *mapView, unitToExit); });
		}
		if (mouseMode->getType() == eMouseModeType::Activate && selectedBuilding->getOwner() == player.get())
		{
			auto activateMouseMode = static_cast<cMouseModeActivateLoaded*> (mouseMode.get());
			auto unitToExit = selectedBuilding->storedUnits[activateMouseMode->getVehicleToActivateIndex ()]->getStaticUnitData ();
			drawExitPointsIf (*selectedBuilding, [&] (const cPosition & position) { return selectedBuilding->canExitTo (position, *mapView, unitToExit); });
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawExitPointsIf (const cUnit& unit, const std::function<bool (const cPosition&)>& predicate)
{
	if (!mapView) return;

	auto adjacentPositions = unit.getAdjacentPositions();

	for (size_t i = 0; i != adjacentPositions.size(); ++i)
	{
		if (predicate (adjacentPositions[i]))
		{
			drawExitPoint (adjacentPositions[i]);
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawExitPoint (const cPosition& position)
{
	const auto zoomedTileSize = getZoomedTileSize();
	const auto tileDrawingRange = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, position);

	const int nr = animationTimer->getAnimationTime() % 5;
	SDL_Rect src;
	src.x = zoomedTileSize.x() * nr;
	src.y = 0;
	src.w = zoomedTileSize.x();
	src.h = zoomedTileSize.y();

	CHECK_SCALING (*GraphicsData.gfx_exitpoints, *GraphicsData.gfx_exitpoints_org, getZoomFactor());
	SDL_BlitSurface (GraphicsData.gfx_exitpoints.get(), &src, cVideo::buffer, &drawDestination);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawBuildBand()
{
	auto selectedVehicle = unitSelection.getSelectedVehicle();

	const auto zoomedTileSize = getZoomedTileSize();

	if (selectedVehicle && !selectedVehicle->isDisabled())
	{
		auto mouse = getActiveMouse();

		if (!mouse || !getArea().withinOrTouches (mouse->getPosition())) return;

		if (mouseMode->getType() == eMouseModeType::SelectBuildPosition)
		{
			if (!mapView) return;

			auto selectBuildPositionMode = static_cast<const cMouseModeSelectBuildPosition*> (mouseMode.get());
			bool validPosition;
			cPosition destination;
			std::tie (validPosition, destination) = selectBuildPositionMode->findNextBuildPosition (selectedVehicle->getPosition(), getMapTilePosition (mouse->getPosition()), *unitsData);
			if (!validPosition) return;

			SDL_Rect dest;
			dest.x = getPosition().x() - (int) (pixelOffset.x() * getZoomFactor()) + zoomedTileSize.x() * destination.x();
			dest.y = getPosition().y() - (int) (pixelOffset.y() * getZoomFactor()) + zoomedTileSize.y() * destination.y();
			CHECK_SCALING (*GraphicsData.gfx_band_big, *GraphicsData.gfx_band_big_org, getZoomFactor());
			SDL_BlitSurface (GraphicsData.gfx_band_big.get(), nullptr, cVideo::buffer, &dest);
		}
		else if (mouseMode->getType() == eMouseModeType::SelectBuildPathDestintaion)
		{
			const auto mouseTilePosition = getMapTilePosition (mouse->getPosition());
			if (mouseTilePosition.x() == selectedVehicle->getPosition().x() || mouseTilePosition.y() == selectedVehicle->getPosition().y())
			{
				SDL_Rect dest;
				dest.x = getPosition().x() - (int) (pixelOffset.x() * getZoomFactor()) + zoomedTileSize.x() * mouseTilePosition.x();
				dest.y = getPosition().y() - (int) (pixelOffset.y() * getZoomFactor()) + zoomedTileSize.y() * mouseTilePosition.y();
				CHECK_SCALING (*GraphicsData.gfx_band_small, *GraphicsData.gfx_band_small_org, getZoomFactor());
				SDL_BlitSurface (GraphicsData.gfx_band_small.get(), nullptr, cVideo::buffer, &dest);
			}
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawLockList ()
{
	const auto zoomedTileSize = getZoomedTileSize();

	for (size_t i = 0; i < unitLockList.getLockedUnitsCount(); ++i)
	{
		const cUnit* unit = unitLockList.getLockedUnit (i);

		if (!mapView->canSeeUnit (*unit))
		{
			continue;
		}

		const auto screenPosition = getScreenPosition (*unit);

		if (shouldDrawScan)
		{
			if (unit->getIsBig())
				drawCircle (screenPosition.x() + zoomedTileSize.x(), screenPosition.y() + zoomedTileSize.y(), unit->data.getScan() * zoomedTileSize.x(), SCAN_COLOR, *cVideo::buffer);
			else
				drawCircle (screenPosition.x() + zoomedTileSize.x() / 2, screenPosition.y() + zoomedTileSize.y() / 2, unit->data.getScan() * zoomedTileSize.x(), SCAN_COLOR, *cVideo::buffer);
		}
		if (shouldDrawRange && (unit->getStaticUnitData().canAttack & TERRAIN_GROUND))
			drawCircle (screenPosition.x() + zoomedTileSize.x() / 2, screenPosition.y() + zoomedTileSize.y() / 2,
						unit->data.getRange() * zoomedTileSize.x() + 1, RANGE_GROUND_COLOR, *cVideo::buffer);
		if (shouldDrawRange && (unit->getIsBig() & TERRAIN_AIR))
			drawCircle (screenPosition.x() + zoomedTileSize.x() / 2, screenPosition.y() + zoomedTileSize.y() / 2,
						unit->data.getRange() * zoomedTileSize.x() + 2, RANGE_AIR_COLOR, *cVideo::buffer);
		//if (ammoChecked () && unit->data.canAttack)
		//	drawMunBar (*unit, screenPos);
		//if (hitsChecked ())
		//	drawHealthBar (*unit, screenPos);
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawBuildPath (const cVehicle& vehicle)
{
	if (!vehicle.BuildPath || (vehicle.bandPosition == vehicle.getPosition()) || mouseMode->getType() == eMouseModeType::SelectBuildPathDestintaion) return;

	const auto zoomedTileSize = getZoomedTileSize();

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
		dest.x = getPosition().x() - (int) (pixelOffset.x() * getZoomFactor()) + zoomedTileSize.x() * mx;
		dest.y = getPosition().y() - (int) (pixelOffset.y() * getZoomFactor()) + zoomedTileSize.y() * my;

		SDL_BlitSurface (OtherData.WayPointPfeileSpecial[sp][64 - zoomedTileSize.x()].get(), nullptr, cVideo::buffer, &dest);

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
	dest.x = getPosition().x() - (int) (pixelOffset.x() * getZoomFactor()) + zoomedTileSize.x() * mx;
	dest.y = getPosition().y() - (int) (pixelOffset.y() * getZoomFactor()) + zoomedTileSize.y() * my;

	SDL_BlitSurface (OtherData.WayPointPfeileSpecial[sp][64 - zoomedTileSize.x()].get(), nullptr, cVideo::buffer, &dest);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawPath (const cVehicle& vehicle)
{
	const auto moveJob = vehicle.getMoveJob();

	if (!moveJob || vehicle.getOwner() != player.get())
	{
		drawBuildPath (vehicle);
		return;
	}

	const auto zoomedTileSize = getZoomedTileSize();

	int sp = vehicle.data.getSpeed() + moveJob->getSavedSpeed();

	SDL_Rect dest;
	dest.x = getPosition().x() - (int) (pixelOffset.x() * getZoomFactor()) + zoomedTileSize.x() * vehicle.getPosition().x();
	dest.y = getPosition().y() - (int) (pixelOffset.y() * getZoomFactor()) + zoomedTileSize.y() * vehicle.getPosition().y();
	dest.w = zoomedTileSize.x();
	dest.h = zoomedTileSize.y();
	SDL_Rect ndest = dest;

	int mx = 0;
	int my = 0;
	cPosition wp = vehicle.getPosition();
	const auto& path = moveJob->getPath();
	for (const auto& nextWp : path)
	{
		ndest.x += mx = nextWp.x() * zoomedTileSize.x() - wp.x() * zoomedTileSize.x();
		ndest.y += my = nextWp.y() * zoomedTileSize.y() - wp.y() * zoomedTileSize.y();

		
		int costs = cPathCalculator::calcNextCost(wp, nextWp, &vehicle, mapView.get());
		if (sp < costs)
		{
			drawPathArrow (dest, ndest, true);
			sp += vehicle.data.getSpeedMax();
		}
		else
		{
			drawPathArrow (dest, ndest, false);
		}
		sp -= costs;

		wp = nextWp;
		dest = ndest;
	}

	ndest.x += mx;
	ndest.y += my;
	drawPathArrow (dest, ndest, false);

}

//------------------------------------------------------------------------------
void cGameMapWidget::drawPathArrow(SDL_Rect dest, const SDL_Rect& lastDest, bool spezialColor) const
{
	int index;
	if      (dest.x >  lastDest.x && dest.y <  lastDest.y) index = 0;
	else if (dest.x == lastDest.x && dest.y <  lastDest.y) index = 1;
	else if (dest.x <  lastDest.x && dest.y <  lastDest.y) index = 2;
	else if (dest.x >  lastDest.x && dest.y == lastDest.y) index = 3;
	else if (dest.x <  lastDest.x && dest.y == lastDest.y) index = 4;
	else if (dest.x >  lastDest.x && dest.y >  lastDest.y) index = 5;
	else if (dest.x == lastDest.x && dest.y >  lastDest.y) index = 6;
	else if (dest.x <  lastDest.x && dest.y >  lastDest.y) index = 7;
	else return;

	if (spezialColor)
	{
		SDL_BlitSurface(OtherData.WayPointPfeileSpecial[index][64 - dest.w].get(), nullptr, cVideo::buffer, &dest);
	}
	else
	{
		SDL_BlitSurface(OtherData.WayPointPfeile[index][64 - dest.w].get(), nullptr, cVideo::buffer, &dest);
	}
}

//------------------------------------------------------------------------------
SDL_Rect cGameMapWidget::computeTileDrawingArea (const cPosition& zoomedTileSize, const cPosition& zoomedStartTilePixelOffset, const cPosition& tileStartIndex, const cPosition& tileIndex)
{
	const cPosition startDrawPosition = getPosition() + (tileIndex - tileStartIndex) * zoomedTileSize - zoomedStartTilePixelOffset;

	SDL_Rect dest = {startDrawPosition.x(), startDrawPosition.y(), zoomedTileSize.x(), zoomedTileSize.y()};

	return dest;
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::getMapTilePosition (const cPosition& pixelPosition) const
{
	assert (getArea().withinOrTouches (pixelPosition));

	const auto zoomedTileSize = getZoomedTileSize();

	const auto x = (int) ((pixelPosition.x() - getPosition().x() + pixelOffset.x() * getZoomFactor()) / zoomedTileSize.x());
	const auto y = (int) ((pixelPosition.y() - getPosition().y() + pixelOffset.y() * getZoomFactor()) / zoomedTileSize.y());

	const cPosition tilePosition (std::max (std::min (x, staticMap->getSize().x() - 1), 0), std::max (std::min (y, staticMap->getSize().y() - 1), 0));

	return tilePosition;
}

//------------------------------------------------------------------------------
bool cGameMapWidget::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	auto consumed = cClickableWidget::handleMouseMoved (application, mouse, offset);

	cPosition lastMouseOverTilePosition;
	const auto lastMousePosition = mouse.getPosition() - offset;
	if (getArea().withinOrTouches (lastMousePosition))
	{
		lastMouseOverTilePosition = getMapTilePosition (lastMousePosition);
	}
	else
	{
		lastMouseOverTilePosition = cPosition (-1, -1);
	}

	if (getArea().withinOrTouches (mouse.getPosition()))
	{
		const auto tilePosition = getMapTilePosition (mouse.getPosition());
		if (tilePosition != lastMouseOverTilePosition)
		{
			tileUnderMouseChanged (tilePosition);
		}
		updateMouseCursor();
	}
	else
	{
		tileUnderMouseChanged (cPosition (-1, -1));
	}

	if (unitSelectionBox.isValidStart() && isAt (mouse.getPosition()) &&
		mouse.isButtonPressed (eMouseButtonType::Left) && !mouse.isButtonPressed (eMouseButtonType::Right))
	{
		const auto zoomedTileSize = getZoomedTileSize();

		unitSelectionBox.getBox().getMaxCorner()[0] = (mouse.getPosition().x() - getPosition().x() + (pixelOffset.x() * getZoomFactor())) / zoomedTileSize.x();
		unitSelectionBox.getBox().getMaxCorner()[1] = (mouse.getPosition().y() - getPosition().y() + (pixelOffset.y() * getZoomFactor())) / zoomedTileSize.y();
	}

	return consumed;
}

//------------------------------------------------------------------------------
bool cGameMapWidget::handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (rightMouseButtonScrollerWidget->handleMousePressed (application, mouse, button)) return true;

	if (button == eMouseButtonType::Left && !mouse.isButtonPressed (eMouseButtonType::Right) &&
		!unitSelectionBox.isValidStart() && isAt (mouse.getPosition()))
	{
		const auto zoomedTileSize = getZoomedTileSize();

		unitSelectionBox.getBox().getMinCorner()[0] = (mouse.getPosition().x() - getPosition().x() + (pixelOffset.x() * getZoomFactor())) / zoomedTileSize.x();
		unitSelectionBox.getBox().getMinCorner()[1] = (mouse.getPosition().y() - getPosition().y() + (pixelOffset.y() * getZoomFactor())) / zoomedTileSize.y();
	}

	return cClickableWidget::handleMousePressed (application, mouse, button);
}

//------------------------------------------------------------------------------
bool cGameMapWidget::handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (rightMouseButtonScrollerWidget->handleMouseReleased (application, mouse, button)) return true;

	if (button == eMouseButtonType::Left && !mouse.isButtonPressed (eMouseButtonType::Right) &&
		!unitSelectionBox.isTooSmall() && mapView && player)
	{
		unitSelection.selectVehiclesAt (unitSelectionBox.getCorrectedMapBox(), *mapView, *player);
		unitSelectionBox.invalidate();
		cClickableWidget::finishMousePressed (application, mouse, button);
		return true;
	}
	else
	{
		unitSelectionBox.invalidate();
		return cClickableWidget::handleMouseReleased (application, mouse, button);
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::handleLooseMouseFocus (cApplication& application)
{
	mouseFocusReleased();
}

//------------------------------------------------------------------------------
void cGameMapWidget::handleResized (const cPosition& oldSize)
{
	cClickableWidget::handleResized (oldSize);

	setZoomFactor (internalZoomFactor, true); // revalidates zoom and offset
}

//------------------------------------------------------------------------------
bool cGameMapWidget::acceptButton (eMouseButtonType button) const
{
	return button == eMouseButtonType::Left || button == eMouseButtonType::Right;
}

//------------------------------------------------------------------------------
bool cGameMapWidget::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (!getArea().withinOrTouches (mouse.getPosition())) return false;

	if (!mapView) return false;

	const auto tilePosition = getMapTilePosition (mouse.getPosition());

	const auto& field = mapView->getField (tilePosition);

	// Some useful aliases
	const auto selectedUnit = unitSelection.getSelectedUnit();
	const auto selectedVehicle = unitSelection.getSelectedVehicle();
	const auto selectedBuilding = unitSelection.getSelectedBuilding();

	const auto overVehicle = field.getVehicle();
	const auto overPlane = field.getPlane();
	const auto overBuilding = field.getBuilding();
	const auto overBaseBuilding = field.getBaseBuilding();

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
				 (KeysList.getMouseStyle() == eMouseStyle::OldSchool && mouse.isButtonPressed (eMouseButtonType::Left)))
		{
			if (mouseMode->getType() == eMouseModeType::Help)
			{
				toggleMouseInputMode (eMouseModeType::Help);
			}
			else
			{
				if (selectedUnit && selectedUnit->isAbove (tilePosition))
				{
					cUnit* next = nullptr;

                    auto units = field.getUnits();

                    auto it = std::find(units.begin(), units.end(), selectedUnit);
                    if(it != units.end())
                    {
                        it++;
                        if(it == units.end()) it = units.begin();
                        next = *it;
                    }

					if (next)
					{
						unitSelection.selectUnit (*next);
					}
				}
				else
				{
					unitSelection.deselectUnits();
				}
			}
		}
	}
	else if (button == eMouseButtonType::Left && !mouse.isButtonPressed (eMouseButtonType::Right))
	{
		bool consumed = false;

		auto action = mouseMode->getMouseAction (tilePosition, *unitsData);
		if (action && (changeAllowed || !action->doesChangeState()))
		{
			consumed = action->executeLeftClick (*this, *mapView, tilePosition, unitSelection, changeAllowed);

			if (action->isSingleAction() && mouseMode->getType() != eMouseModeType::Default)
			{
				setMouseInputMode (std::make_unique<cMouseModeDefault> (mapView.get(), unitSelection, player.get()));
			}
		}

		// toggle unit context menu if no other click action has been performed
		if (!consumed)
		{
			if (changeAllowed && selectedVehicle && (Contains (field.getPlanes(), selectedVehicle) || selectedVehicle == overVehicle))
			{
				if (!selectedVehicle->isUnitMoving())
				{
					toggleUnitContextMenu (selectedVehicle);
					cSoundDevice::getInstance().playSoundEffect (SoundData.SNDHudButton);
				}
			}
			else if (changeAllowed && selectedBuilding && (overBaseBuilding == selectedBuilding || overBuilding == selectedBuilding))
			{
				toggleUnitContextMenu (selectedBuilding);
				cSoundDevice::getInstance().playSoundEffect (SoundData.SNDHudButton);
			}
		}

		// toggle the lock state of an enemy unit,
		// if the selection has changed
		if (player && lockActive)
		{
			const auto newSelectedUnit = unitSelection.getSelectedUnit();
			if (newSelectedUnit && newSelectedUnit != selectedUnit && newSelectedUnit->getOwner() != player.get())
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

	const int offsetX = movementOffset ? unit.getMovementOffset().x() : 0;
	position.x() = getPosition().x() - ((int) ((pixelOffset.x() - offsetX) * getZoomFactor())) + getZoomedTileSize().x() * unit.getPosition().x();

	const int offsetY = movementOffset ? unit.getMovementOffset().y() : 0;
	position.y() = getPosition().y() - ((int) ((pixelOffset.y() - offsetY) * getZoomFactor())) + getZoomedTileSize().y() * unit.getPosition().y();

	return position;
}

//------------------------------------------------------------------------------
void cGameMapWidget::updateActiveAnimations()
{
	animations.clear();
	updateActiveAnimations (std::make_pair (cPosition (-1, -1), cPosition (-1, -1)));
}

//------------------------------------------------------------------------------
void cGameMapWidget::updateActiveAnimations (const std::pair<cPosition, cPosition>& oldTileDrawingRange)
{
	if (!cSettings::getInstance().isAnimations()) return;

	const auto tileDrawingRange = computeTileDrawingRange();

	const auto tileDrawingArea = cBox<cPosition> (tileDrawingRange.first, tileDrawingRange.second - cPosition (1, 1));
	const auto oldTileDrawingArea = cBox<cPosition> (oldTileDrawingRange.first, oldTileDrawingRange.second - cPosition (1, 1));

	// delete finished animations or animations that are no longer in the visible area.
	for (auto i = animations.begin(); i != animations.end(); /*erase in loop*/)
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

	// add animations for units that just entered the visible area.
	if (mapView)
	{
		std::vector<cUnit*> units;
		for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
		{
			const auto position = *i;
			const auto& field = mapView->getField (position);

			units = field.getUnits();

			for (size_t j = 0; j < units.size(); ++j)
			{
				const auto& unit = *units[j];
				if (shouldDrawUnit (unit, position, tileDrawingRange) && !oldTileDrawingArea.intersects (unit.getArea()))
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
	if (!cSettings::getInstance().isAnimations()) return;
	
	if (unit.isABuilding())
	{
		const cBuilding& building = static_cast<const cBuilding&>(unit);
		if (building.isRubble()) return;

		if (building.uiData->powerOnGraphic || unit.getStaticUnitData().canWork)
		{
			assert(unit.isABuilding());
			auto& building = static_cast<const cBuilding&> (unit);

			animations.push_back(std::make_unique<cAnimationWork>(*animationTimer, building));
		}
	}
	if (unit.getStaticUnitData().factorAir > 0)
	{
		assert (unit.isAVehicle());
		auto& vehicle = static_cast<const cVehicle&> (unit);

		animations.push_back (std::make_unique<cAnimationDither> (*animationTimer, vehicle));
	}
	if (unit.getStaticUnitData().canBuild.compare("BigBuilding") == 0)
	{
		assert (unit.isAVehicle());
		auto& vehicle = static_cast<const cVehicle&> (unit);

		animations.push_back (std::make_unique<cAnimationStartUpBuildingSite> (*animationTimer, vehicle));
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::updateUnitMenuPosition()
{
	if (!unitMenu->getUnit()) return;
	if (unitMenu->isHidden()) return;

	const auto& unit = *unitMenu->getUnit();

	const auto menuSize = unitMenu->getSize();

	auto position = getScreenPosition (unit);

	auto unitSize = getZoomedTileSize();
	if (unit.getIsBig()) unitSize *= 2;

	if (position.x() + unitSize.x() + menuSize.x() >= getEndPosition().x())
	{
		position.x() -= menuSize.x();
	}
	else
	{
		position.x() += unitSize.x();
	}

	if (position.y() - (menuSize.y() - unitSize.y()) / 2 <= getPosition().y())
	{
		position.y() -= (menuSize.y() - unitSize.y()) / 2;
		position.y() += - (position.y() - getPosition().y());
	}
	else if (position.y() - (menuSize.y() - unitSize.y()) / 2 + menuSize.y() >= getEndPosition().y())
	{
		position.y() -= (menuSize.y() - unitSize.y()) / 2;
		position.y() -= (position.y() + menuSize.y()) - getEndPosition().y();
	}
	else
	{
		position.y() -= (menuSize.y() - unitSize.y()) / 2;
	}

	unitMenu->moveTo (position);

	updateMouseCursor();
}

//------------------------------------------------------------------------------
void cGameMapWidget::updateMouseCursor()
{
	auto activeMouse = getActiveMouse();
	if (activeMouse)
	{
		updateMouseCursor (*activeMouse);
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::updateMouseCursor (cMouse& mouse)
{
	if (!isAt (mouse.getPosition())) return;

	if (rightMouseButtonScrollerWidget->isScrolling()) return;

	if (!staticMap || (unitMenu->isEnabled() && !unitMenu->isHidden() && unitMenu->isAt (mouse.getPosition())))
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
	}
	else
	{
		mouseMode->setCursor (mouse, getMapTilePosition (mouse.getPosition()), *unitsData);
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::setChangeAllowed (bool value)
{
	changeAllowed = value;
}

//------------------------------------------------------------------------------
cDrawingCache& cGameMapWidget::getDrawingCache()
{
	return unitDrawingEngine.drawingCache;
}

//------------------------------------------------------------------------------
const cDrawingCache& cGameMapWidget::getDrawingCache() const
{
	return unitDrawingEngine.drawingCache;
}

//------------------------------------------------------------------------------
void cGameMapWidget::buildCollidingShortcutsMap()
{
	collidingUnitCommandShortcuts.clear();

	collidingUnitCommandShortcuts.insert (std::make_pair (attackShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (buildShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (transferShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (enterShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (automoveShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (startShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (stopShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (clearShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (sentryShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (manualFireShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (activateShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (loadShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (relaodShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (repairShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (layMineShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (clearMineShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (disableShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (stealShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (infoShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (distributeShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (researchShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (upgradeShortcut, std::set<const cShortcut*> ()));
	collidingUnitCommandShortcuts.insert (std::make_pair (destroyShortcut, std::set<const cShortcut*> ()));

	for (auto i = collidingUnitCommandShortcuts.begin(); i != collidingUnitCommandShortcuts.end(); ++i)
	{
		auto j = i;
		++j;
		for (; j != collidingUnitCommandShortcuts.end(); ++j)
		{
			if (i->first->getKeySequence() == j->first->getKeySequence())
			{
				i->second.insert (j->first);
				j->second.insert (i->first);
			}
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::activateShortcutConditional (cShortcut& shortcut, std::set<const cShortcut*>& blockedShortcuts, const std::set<const cShortcut*>& collidingShortcuts)
{
	if (blockedShortcuts.find (&shortcut) == blockedShortcuts.end())
	{
		shortcut.activate();
		blockedShortcuts.insert (collidingShortcuts.begin(), collidingShortcuts.end());
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::updateActiveUnitCommandShortcuts()
{
	deactivateUnitCommandShortcuts();

	const auto selectedUnit = unitSelection.getSelectedUnit();
	if (selectedUnit == nullptr) return;

	std::set<const cShortcut*> blockedShortcuts;

	// NOTE: the order in which we activate the shortcuts here marks the priority in which
	//       colliding shortcuts will be executed.
	if (cUnitContextMenuWidget::unitHasBuildEntry (selectedUnit, player.get())) activateShortcutConditional (*buildShortcut, blockedShortcuts, collidingUnitCommandShortcuts[buildShortcut]);
	if (cUnitContextMenuWidget::unitHasTransferEntry (selectedUnit, player.get())) activateShortcutConditional (*transferShortcut, blockedShortcuts, collidingUnitCommandShortcuts[transferShortcut]);
	if (cUnitContextMenuWidget::unitHasStartEntry (selectedUnit, player.get())) activateShortcutConditional (*startShortcut, blockedShortcuts, collidingUnitCommandShortcuts[startShortcut]);
	if (cUnitContextMenuWidget::unitHasStopEntry (selectedUnit, player.get())) activateShortcutConditional (*stopShortcut, blockedShortcuts, collidingUnitCommandShortcuts[stopShortcut]);
	if (cUnitContextMenuWidget::unitHasSentryEntry (selectedUnit, player.get())) activateShortcutConditional (*sentryShortcut, blockedShortcuts, collidingUnitCommandShortcuts[sentryShortcut]);
	if (cUnitContextMenuWidget::unitHasManualFireEntry (selectedUnit, player.get())) activateShortcutConditional (*manualFireShortcut, blockedShortcuts, collidingUnitCommandShortcuts[manualFireShortcut]);
	if (cUnitContextMenuWidget::unitHasAttackEntry (selectedUnit, player.get())) activateShortcutConditional (*attackShortcut, blockedShortcuts, collidingUnitCommandShortcuts[attackShortcut]);
	if (cUnitContextMenuWidget::unitHasLayMinesEntry (selectedUnit, player.get())) activateShortcutConditional (*layMineShortcut, blockedShortcuts, collidingUnitCommandShortcuts[layMineShortcut]);
	if (cUnitContextMenuWidget::unitHasCollectMinesEntry (selectedUnit, player.get())) activateShortcutConditional (*clearMineShortcut, blockedShortcuts, collidingUnitCommandShortcuts[clearMineShortcut]);
	if (cUnitContextMenuWidget::unitHasLoadEntry(selectedUnit, player.get())) activateShortcutConditional(*loadShortcut, blockedShortcuts, collidingUnitCommandShortcuts[loadShortcut]);
	if (cUnitContextMenuWidget::unitHasEnterEntry (selectedUnit, player.get())) activateShortcutConditional (*enterShortcut, blockedShortcuts, collidingUnitCommandShortcuts[enterShortcut]);
	if (cUnitContextMenuWidget::unitHasActivateEntry (selectedUnit, player.get())) activateShortcutConditional (*activateShortcut, blockedShortcuts, collidingUnitCommandShortcuts[activateShortcut]);
	if (cUnitContextMenuWidget::unitHasBuyEntry (selectedUnit, player.get())) activateShortcutConditional (*upgradeShortcut, blockedShortcuts, collidingUnitCommandShortcuts[upgradeShortcut]);
	if (cUnitContextMenuWidget::unitHasResearchEntry (selectedUnit, player.get())) activateShortcutConditional (*researchShortcut, blockedShortcuts, collidingUnitCommandShortcuts[researchShortcut]);
	if (cUnitContextMenuWidget::unitHasSabotageEntry (selectedUnit, player.get())) activateShortcutConditional (*disableShortcut, blockedShortcuts, collidingUnitCommandShortcuts[disableShortcut]);
	if (cUnitContextMenuWidget::unitHasStealEntry (selectedUnit, player.get())) activateShortcutConditional (*stealShortcut, blockedShortcuts, collidingUnitCommandShortcuts[stealShortcut]);
	if (cUnitContextMenuWidget::unitHasAutoEntry (selectedUnit, player.get())) activateShortcutConditional (*automoveShortcut, blockedShortcuts, collidingUnitCommandShortcuts[automoveShortcut]);
	if (cUnitContextMenuWidget::unitHasRemoveEntry (selectedUnit, player.get(), mapView.get())) activateShortcutConditional (*clearShortcut, blockedShortcuts, collidingUnitCommandShortcuts[clearShortcut]);
	if (cUnitContextMenuWidget::unitHasSupplyEntry (selectedUnit, player.get())) activateShortcutConditional (*relaodShortcut, blockedShortcuts, collidingUnitCommandShortcuts[relaodShortcut]);
	if (cUnitContextMenuWidget::unitHasRepairEntry (selectedUnit, player.get())) activateShortcutConditional (*repairShortcut, blockedShortcuts, collidingUnitCommandShortcuts[repairShortcut]);
	if (cUnitContextMenuWidget::unitHasDistributeEntry (selectedUnit, player.get())) activateShortcutConditional (*distributeShortcut, blockedShortcuts, collidingUnitCommandShortcuts[distributeShortcut]);
	//if (cUnitContextMenuWidget::unitHasUpgradeThisEntry (selectedUnit, player.get (), dynamicMap.get ())) activateShortcutConditional (*shortcut, blockedShortcuts, collidingUnitCommandShortcuts[shortcut]);
	//if (cUnitContextMenuWidget::unitHasUpgradeAllEntry (selectedUnit, player.get (), dynamicMap.get ())) activateShortcutConditional (*shortcut, blockedShortcuts, collidingUnitCommandShortcuts[shortcut]);
	if (cUnitContextMenuWidget::unitHasSelfDestroyEntry (selectedUnit, player.get())) activateShortcutConditional (*destroyShortcut, blockedShortcuts, collidingUnitCommandShortcuts[destroyShortcut]);
	if (cUnitContextMenuWidget::unitHasInfoEntry (selectedUnit, player.get())) activateShortcutConditional (*infoShortcut, blockedShortcuts, collidingUnitCommandShortcuts[infoShortcut]);
}

//------------------------------------------------------------------------------
void cGameMapWidget::deactivateUnitCommandShortcuts()
{
	attackShortcut->deactivate();
	buildShortcut->deactivate();
	transferShortcut->deactivate();
	enterShortcut->deactivate();
	automoveShortcut->deactivate();
	startShortcut->deactivate();
	stopShortcut->deactivate();
	clearShortcut->deactivate();
	sentryShortcut->deactivate();
	manualFireShortcut->deactivate();
	activateShortcut->deactivate();
	loadShortcut->deactivate();
	relaodShortcut->deactivate();
	repairShortcut->deactivate();
	layMineShortcut->deactivate();
	clearMineShortcut->deactivate();
	disableShortcut->deactivate();
	stealShortcut->deactivate();
	infoShortcut->deactivate();
	distributeShortcut->deactivate();
	researchShortcut->deactivate();
	upgradeShortcut->deactivate();
	destroyShortcut->deactivate();
}

//------------------------------------------------------------------------------
void cGameMapWidget::runOwnedEffects()
{
	for (size_t i = 0; i < effects.size(); ++i)
	{
		auto& effect = effects[i];

		if (effect.use_count() == 1)
		{
			effect->run();
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::renewDamageEffects()
{
	if (!cSettings::getInstance().isDamageEffects()) return;
	if (!mapView) return;

	const auto tileDrawingRange = computeTileDrawingRange();

	for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
	{
		auto& mapField = mapView->getField (*i);

		const auto& buildings = mapField.getBuildings();
		for (size_t i = 0; i < buildings.size(); ++i)
		{
			renewDamageEffect (*buildings[i]);
		}

		const auto& planes = mapField.getPlanes();
		for (size_t i = 0; i < planes.size(); ++i)
		{
			renewDamageEffect (*planes[i]);
		}

		if (mapField.getVehicle())
		{
			renewDamageEffect (*mapField.getVehicle());
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::renewDamageEffect (const cBuilding& building)
{
	if (building.isRubble()) return;

	if (building.uiData->hasDamageEffect &&
		building.data.getHitpoints() < building.data.getHitpointsMax() &&
		(building.getOwner() == player.get() || (!player || mapView->canSeeUnit (building))))
	{
		int intense = (int) (200 - 200 * ((float)building.data.getHitpoints() / building.data.getHitpointsMax()));
		addEffect (std::make_shared<cFxDarkSmoke> (cPosition (building.getPosition().x() * 64 + building.DamageFXPointX, building.getPosition().y() * 64 + building.DamageFXPointY), intense, windDirection));

		if (building.getIsBig() && intense > 50)
		{
			intense -= 50;
			addEffect (std::make_shared<cFxDarkSmoke> (cPosition (building.getPosition().x() * 64 + building.DamageFXPointX2, building.getPosition().y() * 64 + building.DamageFXPointY2), intense, windDirection));
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::renewDamageEffect (const cVehicle& vehicle)
{
	if (vehicle.data.getHitpoints() < vehicle.data.getHitpointsMax() &&
		(vehicle.getOwner() == player.get() || (!player || mapView->canSeeUnit (vehicle))))
	{
		int intense = (int) (100 - 100 * ((float)vehicle.data.getHitpoints() / vehicle.data.getHitpointsMax()));
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
void cGameMapWidget::changeWindDirection()
{
	if (!cSettings::getInstance().isDamageEffects()) return;

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
