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

#include "SDLutility/tosdl.h"
#include "game/data/map/map.h"
#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/logic/movejob.h"
#include "game/logic/pathcalculator.h"
#include "input/mouse/cursor/mousecursoramount.h"
#include "input/mouse/cursor/mousecursorattack.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "input/mouse/mouse.h"
#include "output/sound/soundchannel.h"
#include "output/sound/sounddevice.h"
#include "output/video/video.h"
#include "resources/buildinguidata.h"
#include "resources/keys.h"
#include "resources/sound.h"
#include "resources/uidata.h"
#include "settings.h"
#include "ui/widgets/application.h"
#include "ui/graphical/game/animations/animation.h"
#include "ui/graphical/game/animations/animationdither.h"
#include "ui/graphical/game/animations/animationstartup.h"
#include "ui/graphical/game/animations/animationstartupbuildingsite.h"
#include "ui/graphical/game/animations/animationtimer.h"
#include "ui/graphical/game/animations/animationwork.h"
#include "ui/graphical/game/control/mouseaction/mouseaction.h"
#include "ui/graphical/game/control/mousemode/mousemode.h"
#include "ui/graphical/game/control/mousemode/mousemodeactivateloaded.h"
#include "ui/graphical/game/control/mousemode/mousemodeattack.h"
#include "ui/graphical/game/control/mousemode/mousemodedefault.h"
#include "ui/graphical/game/control/mousemode/mousemodedisable.h"
#include "ui/graphical/game/control/mousemode/mousemodeenter.h"
#include "ui/graphical/game/control/mousemode/mousemodehelp.h"
#include "ui/graphical/game/control/mousemode/mousemodeload.h"
#include "ui/graphical/game/control/mousemode/mousemoderepair.h"
#include "ui/graphical/game/control/mousemode/mousemodeselectbuildpathdestination.h"
#include "ui/graphical/game/control/mousemode/mousemodeselectbuildposition.h"
#include "ui/graphical/game/control/mousemode/mousemodesteal.h"
#include "ui/graphical/game/control/mousemode/mousemodesupplyammo.h"
#include "ui/graphical/game/control/mousemode/mousemodetransfer.h"
#include "ui/graphical/game/control/rightmousebuttonscroller.h"
#include "ui/graphical/game/drawfxeffect.h"
#include "ui/graphical/game/hud.h"
#include "ui/graphical/game/widgets/unitcontextmenuwidget.h"
#include "ui/sound/game/fxsound.h"
#include "ui/sound/soundmanager.h"
#include "ui/uidefines.h"
#include "utility/indexiterator.h"
#include "utility/listhelpers.h"
#include "utility/mathtools.h"
#include "utility/narrow_cast.h"
#include "utility/random.h"
#include "utility/ranges.h"

#include <cassert>

//------------------------------------------------------------------------------
cGameMapWidget::cGameMapWidget (const cBox<cPosition>& area, std::shared_ptr<const cStaticMap> staticMap_, std::shared_ptr<cAnimationTimer> animationTimer_, std::shared_ptr<cSoundManager> soundManager_, std::shared_ptr<const cFrameCounter> frameCounter) :
	cClickableWidget (area),
	animationTimer (animationTimer_),
	soundManager (soundManager_),
	staticMap (std::move (staticMap_)),
	unitDrawingEngine (animationTimer, frameCounter)
{
	assert (staticMap != nullptr);
	assert (animationTimer != nullptr);
	assert (soundManager != nullptr);

	signalConnectionManager.connect (cSettings::getInstance().animationsChanged, [this]() {
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
	signalConnectionManager.connect (animationTimer->triggered400ms, [this]() {
		staticMap->getGraphic().generateNextAnimationFrame();
	});

	setWindDirection (random (360));
	signalConnectionManager.connect (animationTimer->triggered400ms, [this]() { changeWindDirection(); });

	signalConnectionManager.connect (animationTimer->triggered50ms, [this]() { runOwnedEffects(); });
	signalConnectionManager.connect (animationTimer->triggered100ms, [this]() { renewDamageEffects(); });

	unitMenu = emplaceChild<cUnitContextMenuWidget>();
	unitMenu->disable();
	unitMenu->hide();

	rightMouseButtonScrollerWidget = emplaceChild<cRightMouseButtonScrollerWidget> (animationTimer);
	signalConnectionManager.connect (rightMouseButtonScrollerWidget->scroll, [this] (const cPosition& offset) { scroll (offset); });
	signalConnectionManager.connect (rightMouseButtonScrollerWidget->mouseFocusReleased, [this]() { mouseFocusReleased(); });
	signalConnectionManager.connect (rightMouseButtonScrollerWidget->stoppedScrolling, [this]() { updateMouseCursor(); });
	rightMouseButtonScrollerWidget->disable(); // mouse events will be forwarded explicitly

	mouseInputModeChanged.connect ([this]() { updateMouseCursor(); });

	scrolled.connect ([this]() { updateUnitMenuPosition(); });
	scrolled.connect ([this]() {
		soundManager->setListenerPosition (getMapCenterOffset());
	});
	tileUnderMouseChanged.connect ([this] (const cPosition& tilePosition) {
		if (mouseMode)
		{
			mouseMode->handleMapTilePositionChanged (tilePosition);
		}
	});

	zoomFactorChanged.connect ([this]() {
		const auto& [topLeft, bottomRight] = computeTileDrawingRange();
		const cPosition difference = bottomRight - topLeft;
		const auto diameter = difference.l2Norm();
		soundManager->setMaxListeningDistance ((int) (diameter * 2));
	});

	unitSelection.mainSelectionChanged.connect ([this]() {
		toggleUnitContextMenu (nullptr);
		updateActiveUnitCommandShortcuts();
		setMouseInputMode (std::make_unique<cMouseModeDefault> (mapView.get(), unitSelection, player.get()));
		selectedUnitSignalConnectionManager.disconnectAll();
		const auto selectedUnit = unitSelection.getSelectedUnit();
		if (!selectedUnit) return;

		const auto shortcutsUpdater = [this]() { updateActiveUnitCommandShortcuts(); };
		selectedUnitSignalConnectionManager.connect (selectedUnit->data.shotsChanged, shortcutsUpdater);
		selectedUnitSignalConnectionManager.connect (selectedUnit->storedResourcesChanged, shortcutsUpdater);
		selectedUnitSignalConnectionManager.connect (selectedUnit->positionChanged, shortcutsUpdater);
		selectedUnitSignalConnectionManager.connect (selectedUnit->sentryChanged, shortcutsUpdater);
		selectedUnitSignalConnectionManager.connect (selectedUnit->manualFireChanged, shortcutsUpdater);

		if (auto* selectedBuilding = dynamic_cast<cBuilding*> (selectedUnit))
		{
			selectedUnitSignalConnectionManager.connect (selectedBuilding->workingChanged, shortcutsUpdater);
		}
		if (auto* selectedVehicle = dynamic_cast<cVehicle*> (selectedUnit))
		{
			selectedUnitSignalConnectionManager.connect (selectedVehicle->clearingChanged, shortcutsUpdater);
			selectedUnitSignalConnectionManager.connect (selectedVehicle->buildingChanged, shortcutsUpdater);
			selectedUnitSignalConnectionManager.connect (selectedVehicle->moveJobChanged, shortcutsUpdater);
			selectedUnitSignalConnectionManager.connect (selectedVehicle->clearingTurnsChanged, shortcutsUpdater);
			selectedUnitSignalConnectionManager.connect (selectedVehicle->buildingTurnsChanged, shortcutsUpdater);
		}
	});

	unitMenu->attackToggled.connect ([this]() { toggleMouseInputMode (eMouseModeType::Attack); });
	unitMenu->transferToggled.connect ([this]() { toggleMouseInputMode (eMouseModeType::Transfer); });
	unitMenu->enterToggled.connect ([this]() { toggleMouseInputMode (eMouseModeType::Enter); });
	unitMenu->loadToggled.connect ([this]() { toggleMouseInputMode (eMouseModeType::Load); });
	unitMenu->supplyAmmoToggled.connect ([this]() { toggleMouseInputMode (eMouseModeType::SupplyAmmo); });
	unitMenu->repairToggled.connect ([this]() { toggleMouseInputMode (eMouseModeType::Repair); });
	unitMenu->sabotageToggled.connect ([this]() { toggleMouseInputMode (eMouseModeType::Disable); });
	unitMenu->stealToggled.connect ([this]() { toggleMouseInputMode (eMouseModeType::Steal); });

	unitMenu->buildClicked.connect ([this]() { if (const auto* unit = unitMenu->getUnit()) triggeredBuild (*unit); });
	unitMenu->distributeClicked.connect ([this]() { if (const auto* unit = unitMenu->getUnit()) triggeredResourceDistribution (*unit); });
	unitMenu->startClicked.connect ([this]() { if (const auto* building = unitMenu->getBuilding()) triggeredStartWork (*building); });
	unitMenu->stopClicked.connect ([this]() { if (const auto* unit = unitMenu->getUnit()) triggeredStopWork (*unit); });
	unitMenu->autoToggled.connect ([this]() { if (const auto* unit = unitMenu->getUnit()) triggeredAutoMoveJob (*unit); });
	unitMenu->removeClicked.connect ([this]() { if (const auto* vehicle = unitMenu->getVehicle()) triggeredStartClear (*vehicle); });
	unitMenu->manualFireToggled.connect ([this]() { if (const auto* unit = unitMenu->getUnit()) triggeredManualFire (*unit); });
	unitMenu->sentryToggled.connect ([this]() { if (const auto* unit = unitMenu->getUnit()) triggeredSentry (*unit); });
	unitMenu->activateClicked.connect ([this]() { if (const auto* unit = unitMenu->getUnit()) triggeredActivate (*unit); });
	unitMenu->researchClicked.connect ([this]() { if (const auto* unit = unitMenu->getUnit()) triggeredResearchMenu (*unit); });
	unitMenu->buyUpgradesClicked.connect ([this]() { if (const auto* unit = unitMenu->getUnit()) triggeredUpgradesMenu (*unit); });
	unitMenu->upgradeThisClicked.connect ([this]() { if (const auto* building = unitMenu->getBuilding()) triggeredUpgradeThis (*building); });
	unitMenu->upgradeAllClicked.connect ([this]() { if (const auto* building = unitMenu->getBuilding()) triggeredUpgradeAll (*building); });
	unitMenu->selfDestroyClicked.connect ([this]() { if (const auto* building = unitMenu->getBuilding()) triggeredSelfDestruction (*building); });
	unitMenu->layMinesToggled.connect ([this]() { if (const auto* vehicle = unitMenu->getVehicle()) triggeredLayMines (*vehicle); });
	unitMenu->collectMinesToggled.connect ([this]() { if (const auto* vehicle = unitMenu->getVehicle()) triggeredCollectMines (*vehicle); });
	unitMenu->infoClicked.connect ([this]() { if (const auto* unit = unitMenu->getUnit()) triggeredUnitHelp (*unit); });
	unitMenu->doneClicked.connect ([this]() { if (const auto* unit = unitMenu->getUnit()) triggeredUnitDone (*unit); });

	unitMenu->attackToggled.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->buildClicked.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->distributeClicked.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->transferToggled.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->enterToggled.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->startClicked.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->autoToggled.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->stopClicked.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->removeClicked.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->manualFireToggled.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->sentryToggled.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->activateClicked.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->loadToggled.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->researchClicked.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->buyUpgradesClicked.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->upgradeThisClicked.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->upgradeAllClicked.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->selfDestroyClicked.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->supplyAmmoToggled.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->repairToggled.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->layMinesToggled.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->collectMinesToggled.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->sabotageToggled.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->stealToggled.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->infoClicked.connect ([this]() { toggleUnitContextMenu (nullptr); });
	unitMenu->doneClicked.connect ([this]() { toggleUnitContextMenu (nullptr); });

	attackShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuAttack));
	attackShortcut->triggered.connect ([this]() {
		if (cUnitContextMenuWidget::unitHasAttackEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			toggleMouseInputMode (eMouseModeType::Attack);
		}
	});

	buildShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuBuild));
	buildShortcut->triggered.connect ([this]() {
		if (cUnitContextMenuWidget::unitHasBuildEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredBuild (*unitSelection.getSelectedUnit());
		}
	});

	transferShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuTransfer));
	transferShortcut->triggered.connect ([this]() {
		if (cUnitContextMenuWidget::unitHasTransferEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			toggleMouseInputMode (eMouseModeType::Transfer);
		}
	});

	automoveShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuAutomove));
	automoveShortcut->triggered.connect ([this]() {
		const cVehicle* vehicle = unitSelection.getSelectedVehicle();
		if (cUnitContextMenuWidget::unitHasAutoEntry (vehicle, player.get()))
		{
			triggeredAutoMoveJob (*vehicle);
		}
	});

	startShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuStart));
	startShortcut->triggered.connect ([this]() {
		const auto* building = unitSelection.getSelectedBuilding();
		if (cUnitContextMenuWidget::unitHasStartEntry (building, player.get()))
		{
			triggeredStartWork (*building);
		}
	});

	stopShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuStop));
	stopShortcut->triggered.connect ([this]() {
		for (const auto& unit : unitSelection.getSelectedUnits())
		{
			if (cUnitContextMenuWidget::unitHasStopEntry (unit, player.get()))
			{
				triggeredStopWork (*unit);
			}
		}
	});

	clearShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuClear));
	clearShortcut->triggered.connect ([this]() {
		const auto* vehicle = unitSelection.getSelectedVehicle();
		if (cUnitContextMenuWidget::unitHasRemoveEntry (vehicle, player.get(), mapView.get()))
		{
			triggeredStartClear (*vehicle);
		}
	});

	sentryShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuSentry));
	sentryShortcut->triggered.connect ([this]() {
		if (cUnitContextMenuWidget::unitHasSentryEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredSentry (*unitSelection.getSelectedUnit());
		}
	});

	manualFireShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuManualFire));
	manualFireShortcut->triggered.connect ([this]() {
		if (cUnitContextMenuWidget::unitHasManualFireEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredManualFire (*unitSelection.getSelectedUnit());
		}
	});

	activateShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuActivate));
	activateShortcut->triggered.connect ([this]() {
		if (cUnitContextMenuWidget::unitHasActivateEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredActivate (*unitSelection.getSelectedUnit());
		}
	});

	loadShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuLoad));
	loadShortcut->triggered.connect ([this]() {
		if (cUnitContextMenuWidget::unitHasLoadEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			toggleMouseInputMode (eMouseModeType::Load);
		}
	});

	relaodShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuReload));
	relaodShortcut->triggered.connect ([this]() {
		const auto* vehicle = unitSelection.getSelectedVehicle();
		if (cUnitContextMenuWidget::unitHasSupplyEntry (vehicle, player.get()))
		{
			toggleMouseInputMode (eMouseModeType::SupplyAmmo);
		}
	});

	enterShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuEnter));
	enterShortcut->triggered.connect ([this]() {
		const auto* vehicle = unitSelection.getSelectedVehicle();
		if (cUnitContextMenuWidget::unitHasEnterEntry (vehicle, player.get()))
		{
			toggleMouseInputMode (eMouseModeType::Enter);
		}
	});

	repairShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuRepair));
	repairShortcut->triggered.connect ([this]() {
		const auto* vehicle = unitSelection.getSelectedVehicle();
		if (cUnitContextMenuWidget::unitHasRepairEntry (vehicle, player.get()))
		{
			toggleMouseInputMode (eMouseModeType::Repair);
		}
	});

	layMineShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuLayMine));
	layMineShortcut->triggered.connect ([this]() {
		const cVehicle* vehicle = unitSelection.getSelectedVehicle();
		if (cUnitContextMenuWidget::unitHasLayMinesEntry (vehicle, player.get()))
		{
			triggeredLayMines (*vehicle);
		}
	});

	clearMineShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuClearMine));
	clearMineShortcut->triggered.connect ([this]() {
		const cVehicle* vehicle = unitSelection.getSelectedVehicle();
		if (cUnitContextMenuWidget::unitHasCollectMinesEntry (vehicle, player.get()))
		{
			triggeredCollectMines (*vehicle);
		}
	});

	disableShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuDisable));
	disableShortcut->triggered.connect ([this]() {
		const auto* vehicle = unitSelection.getSelectedVehicle();

		if (cUnitContextMenuWidget::unitHasSabotageEntry (vehicle, player.get()))
		{
			toggleMouseInputMode (eMouseModeType::Disable);
		}
	});

	stealShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuSteal));
	stealShortcut->triggered.connect ([this]() {
		const auto* vehicle = unitSelection.getSelectedVehicle();
		if (cUnitContextMenuWidget::unitHasStealEntry (vehicle, player.get()))
		{
			toggleMouseInputMode (eMouseModeType::Steal);
		}
	});

	infoShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuInfo));
	infoShortcut->triggered.connect ([this]() {
		if (cUnitContextMenuWidget::unitHasInfoEntry (unitSelection.getSelectedUnit(), player.get()))
		{
			triggeredUnitHelp (*unitSelection.getSelectedUnit());
		}
	});

	distributeShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuDistribute));
	distributeShortcut->triggered.connect ([this]() {
		const auto* building = unitSelection.getSelectedBuilding();
		if (cUnitContextMenuWidget::unitHasDistributeEntry (building, player.get()))
		{
			triggeredResourceDistribution (*building);
		}
	});

	researchShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuResearch));
	researchShortcut->triggered.connect ([this]() {
		const auto* building = unitSelection.getSelectedBuilding();
		if (cUnitContextMenuWidget::unitHasResearchEntry (building, player.get()))
		{
			triggeredResearchMenu (*building);
		}
	});

	upgradeShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuUpgrade));
	upgradeShortcut->triggered.connect ([this]() {
		const auto* building = unitSelection.getSelectedBuilding();
		if (cUnitContextMenuWidget::unitHasBuyEntry (building, player.get()))
		{
			triggeredUpgradesMenu (*building);
		}
	});

	destroyShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyUnitMenuDestroy));
	destroyShortcut->triggered.connect ([this]() {
		const auto* building = unitSelection.getSelectedBuilding();
		if (cUnitContextMenuWidget::unitHasSelfDestroyEntry (building, player.get()))
		{
			triggeredSelfDestruction (*building);
		}
	});

	buildCollidingShortcutsMap();
}

//------------------------------------------------------------------------------
cGameMapWidget::~cGameMapWidget()
{
}

//------------------------------------------------------------------------------
void cGameMapWidget::setMapView (std::shared_ptr<const cMapView> mapView_)
{
	std::swap (mapView, mapView_);

	mapViewSignalConnectionManager.disconnectAll();

	if (mapView != nullptr)
	{
		mapViewSignalConnectionManager.connect (mapView->unitDissappeared, [this] (const cUnit& unit) {
			if (unitSelection.isSelected (unit))
			{
				unitSelection.deselectUnit (unit);
			}
		});
		mapViewSignalConnectionManager.connect (mapView->unitAppeared, [this] (const cUnit& unit) {
			if (!cSettings::getInstance().isAnimations()) return;

			const auto [topLeft, bottomRight] = computeTileDrawingRange();
			const auto tileDrawingArea = cBox<cPosition> (topLeft, bottomRight - cPosition (1, 1));

			if (tileDrawingArea.intersects (unit.getArea()))
			{
				addAnimationsForUnit (unit);
				animations.push_back (std::make_unique<cAnimationStartUp> (*animationTimer, unit));
			}
		});
		mapViewSignalConnectionManager.connect (mapView->unitMoved, [this] (const cUnit& unit, const cPosition& oldPosition) {
			if (!cSettings::getInstance().isAnimations()) return;

			const auto [topLeft, bottomRight] = computeTileDrawingRange();
			const auto tileDrawingArea = cBox<cPosition> (topLeft, bottomRight - cPosition (1, 1));

			if (tileDrawingArea.intersects (unit.getArea()) && !tileDrawingArea.intersects (cBox<cPosition> (oldPosition, oldPosition + unit.getArea().getSize() - cPosition (1, 1))))
			{
				addAnimationsForUnit (unit);
			}
		});
	}

	if (mouseMode != nullptr)
	{
		mouseMode->setMap (mapView.get());
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

void cGameMapWidget::setUnitsData (std::shared_ptr<const cUnitsData> unitsData_)
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
	if (shouldDrawSurvey || (selectedVehicle && selectedVehicle->getOwner() == player.get() && selectedVehicle->getStaticData().canSurvey))
	{
		drawResources();
	}

	if (selectedVehicle && ((selectedVehicle->getMoveJob() && !selectedVehicle->isUnitMoving()) || selectedVehicle->bandPosition))
	{
		drawPath (*selectedVehicle);
	}

	drawSelectionBox();

	SDL_SetClipRect (cVideo::buffer, nullptr);

	drawUnitCircles();
	drawExitPoints();
	drawBuildBand();

	if (lockActive) drawLockList();

	drawEffects (false);

	//displayMessages();

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
	auto [topLeft, bottomRight] = computeTileDrawingRange();

	return cBox<cPosition> (topLeft, bottomRight - 1);
}

//------------------------------------------------------------------------------
float cGameMapWidget::getZoomFactor() const
{
	return (float) getZoomedTileSize().x() / sGraphicTile::tilePixelWidth; // should make no difference if we use y instead
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

		unitContextMenuSignalConnectionManager.connect (unit->positionChanged, [this, unit]() {
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
	mouseModeSignalConnectionManager.connect (mouseMode->needRefresh, [this]() { updateMouseCursor(); });

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
				setMouseInputMode (std::make_unique<cMouseModeEnter> (mapView.get(), unitSelection, player.get()));
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
	newPixelOffset.x() = position.x() * sGraphicTile::tilePixelWidth - (sGraphicTile::tilePixelWidth * getSize().x() / (2 * zoomedTileSize.x())) + sGraphicTile::tilePixelWidth / 2;
	newPixelOffset.y() = position.y() * sGraphicTile::tilePixelHeight - (sGraphicTile::tilePixelHeight * getSize().y() / (2 * zoomedTileSize.y())) + sGraphicTile::tilePixelHeight / 2;

	scroll (newPixelOffset - pixelOffset);
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::getMapCenterOffset()
{
	const auto zoomedTileSize = getZoomedTileSize();

	cPosition center;
	center.x() = pixelOffset.x() / sGraphicTile::tilePixelWidth + (getSize().x() / (2 * zoomedTileSize.x()));
	center.y() = pixelOffset.y() / sGraphicTile::tilePixelHeight + (getSize().y() / (2 * zoomedTileSize.y()));

	return center;
}

//------------------------------------------------------------------------------
bool cGameMapWidget::startFindBuildPosition (const sID& buildId)
{
	auto mouseMode = std::make_unique<cMouseModeSelectBuildPosition> (mapView.get(), unitSelection, player.get(), buildId);

	// validate if there is any valid position, before setting mouse mode
	const auto selectedVehicle = unitSelection.getSelectedVehicle();
	if (selectedVehicle)
	{
		auto buildPos = mouseMode->findNextBuildPosition (selectedVehicle->getPosition(), selectedVehicle->getPosition(), *unitsData);
		if (!buildPos)
		{
			return false;
		}
	}
	setMouseInputMode (std::move (mouseMode));

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
		if (playSound) playEffectSound (*soundManager, *effect);
		effects.push_back (std::move (effect));
	}
}

//------------------------------------------------------------------------------
float cGameMapWidget::computeMinimalZoomFactor() const
{
	// inequality to be fulfilled:
	//
	//   round (tile_x * zoom) * map_x <= size_x
	//
	// we first discard the 'round' and solve for zoom:
	//
	//   zoom = size_x / (map_x * tile_x)

	auto xZoom = (float) getSize().x() / (staticMap->getSize().x() * sGraphicTile::tilePixelWidth);
	auto yZoom = (float) getSize().y() / (staticMap->getSize().y() * sGraphicTile::tilePixelHeight);

	// then we try to fix if round would have rounded up:

	xZoom = std::max (xZoom, (float) ((int) (sGraphicTile::tilePixelWidth * xZoom) + (xZoom >= 1.0f ? 0 : 1)) / sGraphicTile::tilePixelWidth);
	yZoom = std::max (yZoom, (float) ((int) (sGraphicTile::tilePixelHeight * yZoom) + (yZoom >= 1.0f ? 0 : 1)) / sGraphicTile::tilePixelHeight);

	return std::max (xZoom, yZoom);
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::computeMaximalPixelOffset() const
{
	const auto x = staticMap->getSize().x() * sGraphicTile::tilePixelWidth - (int) (getSize().x() / getZoomFactor());
	const auto y = staticMap->getSize().y() * sGraphicTile::tilePixelHeight - (int) (getSize().y() / getZoomFactor());

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
	return zoomSize (cPosition (sGraphicTile::tilePixelWidth, sGraphicTile::tilePixelHeight), internalZoomFactor);
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::getZoomedStartTilePixelOffset() const
{
	return zoomSize (cPosition (pixelOffset.x() % sGraphicTile::tilePixelWidth, pixelOffset.y() % sGraphicTile::tilePixelHeight), getZoomFactor());
}

//------------------------------------------------------------------------------
std::pair<cPosition, cPosition> cGameMapWidget::computeTileDrawingRange() const
{
	const auto zoomedTileSize = getZoomedTileSize();

	const cPosition drawingPixelRange = getSize() + getZoomedStartTilePixelOffset();

	const cPosition tilesSize ((int) std::ceil (drawingPixelRange.x() / zoomedTileSize.x()), (int) std::ceil (drawingPixelRange.y() / zoomedTileSize.y()));

	cPosition startTile ((int) std::floor (pixelOffset.x() / sGraphicTile::tilePixelWidth), (int) std::floor (pixelOffset.y() / sGraphicTile::tilePixelHeight));
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
	const auto [topLeft, bottomRight] = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	for (auto i = makeIndexIterator (topLeft, bottomRight); i.hasMore(); i.next())
	{
		const auto& terrain = staticMap->getGraphic().getTile (staticMap->getTileIndex (*i));

		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, topLeft, *i);
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
	SDL_Rect clipRect = toSdlRect (getArea());
	SDL_SetClipRect (cVideo::buffer, &clipRect);

	const cPosition originalTileSize (sGraphicTile::tilePixelWidth, sGraphicTile::tilePixelHeight);

	EraseIf (effects, [] (const auto& effect) { return effect->isFinished(); });
	for (const auto& effect : effects)
	{
		if (effect->bottom == bottom && (!player || player->canSeeAt (effect->getPixelPosition() / originalTileSize)))
		{
			cPosition screenDestination;
			screenDestination.x() = getPosition().x() + static_cast<int> ((effect->getPixelPosition().x() - pixelOffset.x()) * getZoomFactor());
			screenDestination.y() = getPosition().y() + static_cast<int> ((effect->getPixelPosition().y() - pixelOffset.y()) * getZoomFactor());
			drawFx (*effect, getZoomFactor(), screenDestination);
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

			const auto& building = *(*it);

			if (!building.isRubble() && (building.getStaticUnitData().surfacePosition != eSurfacePosition::BeneathSea && building.getStaticUnitData().surfacePosition != eSurfacePosition::Base))
				break;

			if (shouldDrawUnit (building, *i, tileDrawingRange))
			{
				const auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, building.getPosition());
				unitDrawingEngine.drawUnit (building, drawDestination, getZoomFactor(), &unitSelection, player.get(), currentTurnResearchAreasFinished);
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
		if (building->getStaticUnitData().surfacePosition != eSurfacePosition::Ground) continue;
		if (!shouldDrawUnit (*building, *i, tileDrawingRange)) continue;

		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, building->getPosition());
		unitDrawingEngine.drawUnit (*building, drawDestination, getZoomFactor(), &unitSelection, player.get(), currentTurnResearchAreasFinished);

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
	const auto [topLeft, bottomRight] = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	for (auto i = makeIndexIterator (topLeft, bottomRight); i.hasMore(); i.next())
	{
		auto& mapField = mapView->getField (*i);
		auto vehicle = mapField.getVehicle();
		if (vehicle == nullptr) continue;
		if (vehicle->getStaticUnitData().factorSea > 0 && vehicle->getStaticUnitData().factorGround == 0)
		{
			auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, topLeft, *i);
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
		for (const auto* building : buildings)
		{
			if (building->getStaticUnitData().surfacePosition == eSurfacePosition::AboveSea)
			{
				const auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, building->getPosition());
				unitDrawingEngine.drawUnit (*building, drawDestination, getZoomFactor(), &unitSelection, player.get(), currentTurnResearchAreasFinished);
			}
		}
		for (const auto* building : buildings)
		{
			if (building->getStaticUnitData().surfacePosition == eSurfacePosition::AboveBase)
			{
				const auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, building->getPosition());
				unitDrawingEngine.drawUnit (*building, drawDestination, getZoomFactor(), &unitSelection, player.get(), currentTurnResearchAreasFinished);
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
		else
			return visitingPosition == unit.getPosition();
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawVehicles()
{
	if (!mapView) return;

	const auto zoomedTileSize = getZoomedTileSize();
	const auto [topLeft, bottomRight] = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	for (auto i = makeIndexIterator (topLeft, bottomRight); i.hasMore(); i.next())
	{
		auto& mapField = mapView->getField (*i);
		auto vehicle = mapField.getVehicle();
		if (vehicle == nullptr) continue;
		if (vehicle->getStaticUnitData().factorGround != 0 && !vehicle->isUnitBuildingABuilding() && !vehicle->isUnitClearing())
		{
			auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, topLeft, *i);
			unitDrawingEngine.drawUnit (*vehicle, drawDestination, getZoomFactor(), *mapView, &unitSelection, player.get());
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawConnectors()
{
	if (!mapView) return;

	const auto zoomedTileSize = getZoomedTileSize();
	const auto [topLeft, bottomRight] = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	for (auto i = makeIndexIterator (topLeft, bottomRight); i.hasMore(); i.next())
	{
		auto& mapField = mapView->getField (*i);
		auto building = mapField.getTopBuilding();
		if (building == nullptr) continue;
		if (building->getStaticUnitData().surfacePosition == eSurfacePosition::Above)
		{
			auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, topLeft, *i);
			unitDrawingEngine.drawUnit (*building, drawDestination, getZoomFactor(), &unitSelection, player.get(), currentTurnResearchAreasFinished);
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawPlanes()
{
	if (!mapView) return;

	const auto zoomedTileSize = getZoomedTileSize();
	const auto [topLeft, bottomRight] = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	for (auto i = makeIndexIterator (topLeft, bottomRight); i.hasMore(); i.next())
	{
		auto& mapField = mapView->getField (*i);
		const auto& planes = mapField.getPlanes();

		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, topLeft, *i);
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
	const auto [topLeft, bottomRight] = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	SDL_Rect tmp, src = {0, 0, Uint16 (zoomedTileSize.x()), Uint16 (zoomedTileSize.y())};
	for (auto i = makeIndexIterator (topLeft, bottomRight); i.hasMore(); i.next())
	{
		if (player && !player->hasResourceExplored (*i)) continue;
		if (mapView->isBlocked (*i)) continue;

		const auto& resource = mapView->getResource (*i);
		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, topLeft, *i);

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

	const int mouseTopX = static_cast<int> (std::min (unitSelectionBox.getBox().getMinCorner()[0], unitSelectionBox.getBox().getMaxCorner()[0]) * zoomedTileSize.x());
	const int mouseTopY = static_cast<int> (std::min (unitSelectionBox.getBox().getMinCorner()[1], unitSelectionBox.getBox().getMaxCorner()[1]) * zoomedTileSize.y());
	const int mouseBottomX = static_cast<int> (std::max (unitSelectionBox.getBox().getMinCorner()[0], unitSelectionBox.getBox().getMaxCorner()[0]) * zoomedTileSize.x());
	const int mouseBottomY = static_cast<int> (std::max (unitSelectionBox.getBox().getMinCorner()[1], unitSelectionBox.getBox().getMaxCorner()[1]) * zoomedTileSize.y());
	const Uint32 color = 0xFFFFFF00;
	SDL_Rect d;

	d.x = mouseTopX - zoomOffX + getPosition().x();
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
	auto clipRect = toSdlRect (getArea());
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
			if (selectedVehicle->getStaticUnitData().canAttack & eTerrainFlag::Air)
				drawCircle (screenPosition.x() + zoomedTileSize.x() / 2, screenPosition.y() + zoomedTileSize.y() / 2, selectedVehicle->data.getRange() * zoomedTileSize.x() + 2, RANGE_AIR_COLOR, *cVideo::buffer);
			else
				drawCircle (screenPosition.x() + zoomedTileSize.x() / 2, screenPosition.y() + zoomedTileSize.y() / 2, selectedVehicle->data.getRange() * zoomedTileSize.x() + 1, RANGE_GROUND_COLOR, *cVideo::buffer);
		}
	}
	else if (selectedBuilding && selectedBuilding->isDisabled() == false)
	{
		const auto screenPosition = getScreenPosition (*selectedBuilding);
		if (shouldDrawScan)
		{
			if (selectedBuilding->getIsBig())
			{
				drawCircle (screenPosition.x() + zoomedTileSize.x(),
				            screenPosition.y() + zoomedTileSize.y(),
				            selectedBuilding->data.getScan() * zoomedTileSize.x(),
				            SCAN_COLOR,
				            *cVideo::buffer);
			}
			else
			{
				drawCircle (screenPosition.x() + zoomedTileSize.x() / 2,
				            screenPosition.y() + zoomedTileSize.y() / 2,
				            selectedBuilding->data.getScan() * zoomedTileSize.x(),
				            SCAN_COLOR,
				            *cVideo::buffer);
			}
		}
		if (shouldDrawRange && (selectedBuilding->getStaticUnitData().canAttack & eTerrainFlag::Ground) && !selectedBuilding->getStaticData().explodesOnContact)
		{
			drawCircle (screenPosition.x() + zoomedTileSize.x() / 2,
			            screenPosition.y() + zoomedTileSize.y() / 2,
			            selectedBuilding->data.getRange() * zoomedTileSize.x() + 2,
			            RANGE_GROUND_COLOR,
			            *cVideo::buffer);
		}
		if (shouldDrawRange && (selectedBuilding->getStaticUnitData().canAttack & eTerrainFlag::Air))
		{
			drawCircle (screenPosition.x() + zoomedTileSize.x() / 2,
			            screenPosition.y() + zoomedTileSize.y() / 2,
			            selectedBuilding->data.getRange() * zoomedTileSize.x() + 2,
			            RANGE_AIR_COLOR,
			            *cVideo::buffer);
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
		if (mapView && selectedVehicle->getOwner() == player.get() && ((selectedVehicle->isUnitBuildingABuilding() && selectedVehicle->getBuildTurns() == 0) || (selectedVehicle->isUnitClearing() && selectedVehicle->getClearingTurns() == 0)) && !selectedVehicle->bandPosition)
		{
			drawExitPointsIf (*selectedVehicle, [&] (const cPosition& position) { return mapView->possiblePlace (*selectedVehicle, position); });
		}
		if (mouseMode->getType() == eMouseModeType::Activate && selectedVehicle->getOwner() == player.get())
		{
			auto activateMouseMode = static_cast<cMouseModeActivateLoaded*> (mouseMode.get());
			auto unitToExit = selectedVehicle->storedUnits[activateMouseMode->getVehicleToActivateIndex()]->getStaticUnitData();
			drawExitPointsIf (*selectedVehicle, [&] (const cPosition& position) { return selectedVehicle->canExitTo (position, *mapView, unitToExit); });
		}
	}
	else if (selectedBuilding && selectedBuilding->isDisabled() == false)
	{
		if (!selectedBuilding->isBuildListEmpty() && !selectedBuilding->isUnitWorking() && selectedBuilding->getBuildListItem (0).getRemainingMetal() <= 0 && selectedBuilding->getOwner() == player.get())
		{
			auto unitToExit = unitsData->getStaticUnitData (selectedBuilding->getBuildListItem (0).getType());
			drawExitPointsIf (*selectedBuilding, [&] (const cPosition& position) { return selectedBuilding->canExitTo (position, *mapView, unitToExit); });
		}
		if (mouseMode->getType() == eMouseModeType::Activate && selectedBuilding->getOwner() == player.get())
		{
			auto activateMouseMode = static_cast<cMouseModeActivateLoaded*> (mouseMode.get());
			auto unitToExit = selectedBuilding->storedUnits[activateMouseMode->getVehicleToActivateIndex()]->getStaticUnitData();
			drawExitPointsIf (*selectedBuilding, [&] (const cPosition& position) { return selectedBuilding->canExitTo (position, *mapView, unitToExit); });
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawExitPointsIf (const cUnit& unit, const std::function<bool (const cPosition&)>& predicate)
{
	if (!mapView) return;

	for (const auto& adjacentPosition : unit.getAdjacentPositions())
	{
		if (predicate (adjacentPosition))
		{
			drawExitPoint (adjacentPosition);
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawExitPoint (const cPosition& position)
{
	const auto zoomedTileSize = getZoomedTileSize();
	const auto [topLeft, bottomRight] = computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = getZoomedStartTilePixelOffset();

	auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, topLeft, position);

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
			const auto destination = selectBuildPositionMode->findNextBuildPosition (selectedVehicle->getPosition(), getMapTilePosition (mouse->getPosition()), *unitsData);
			if (!destination) return;

			SDL_Rect dest;
			dest.x = getPosition().x() - (int) (pixelOffset.x() * getZoomFactor()) + zoomedTileSize.x() * destination->x();
			dest.y = getPosition().y() - (int) (pixelOffset.y() * getZoomFactor()) + zoomedTileSize.y() * destination->y();
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
void cGameMapWidget::drawLockList()
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
		if (shouldDrawRange && (unit->getStaticUnitData().canAttack & eTerrainFlag::Ground))
			drawCircle (screenPosition.x() + zoomedTileSize.x() / 2, screenPosition.y() + zoomedTileSize.y() / 2, unit->data.getRange() * zoomedTileSize.x() + 1, RANGE_GROUND_COLOR, *cVideo::buffer);
		if (shouldDrawRange && (unit->getIsBig() & eTerrainFlag::Air))
			drawCircle (screenPosition.x() + zoomedTileSize.x() / 2, screenPosition.y() + zoomedTileSize.y() / 2, unit->data.getRange() * zoomedTileSize.x() + 2, RANGE_AIR_COLOR, *cVideo::buffer);
		//if (ammoChecked() && unit->data.canAttack)
		//	drawMunBar (*unit, screenPos);
		//if (hitsChecked())
		//	drawHealthBar (*unit, screenPos);
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawBuildPath (const cVehicle& vehicle)
{
	if (!vehicle.bandPosition || (*vehicle.bandPosition == vehicle.getPosition()) || mouseMode->getType() == eMouseModeType::SelectBuildPathDestintaion) return;

	const auto zoomedTileSize = getZoomedTileSize();

	cPosition m = vehicle.getPosition();
	int sp;
	if (m.x() < vehicle.bandPosition->x())
		sp = 4;
	else if (m.x() > vehicle.bandPosition->x())
		sp = 3;
	else if (m.y() < vehicle.bandPosition->y())
		sp = 1;
	else
		sp = 6;

	while (m != vehicle.bandPosition)
	{
		SDL_Rect dest;
		dest.x = getPosition().x() - (int) (pixelOffset.x() * getZoomFactor()) + zoomedTileSize.x() * m.x();
		dest.y = getPosition().y() - (int) (pixelOffset.y() * getZoomFactor()) + zoomedTileSize.y() * m.y();

		SDL_BlitSurface (OtherData.WayPointPfeileSpecial[sp][64 - zoomedTileSize.x()].get(), nullptr, cVideo::buffer, &dest);

		if (m.x() < vehicle.bandPosition->x())
			m.x()++;
		else if (m.x() > vehicle.bandPosition->x())
			m.x()--;

		if (m.y() < vehicle.bandPosition->y())
			m.y()++;
		else if (m.y() > vehicle.bandPosition->y())
			m.y()--;
	}
	SDL_Rect dest;
	dest.x = getPosition().x() - (int) (pixelOffset.x() * getZoomFactor()) + zoomedTileSize.x() * m.x();
	dest.y = getPosition().y() - (int) (pixelOffset.y() * getZoomFactor()) + zoomedTileSize.y() * m.y();

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
	// Draw remaining shots at bottom, centered horizontally of the tile
	const auto drawShots = [&]() {
		const int shotCount = vehicle.getPossibleShotCountForSpeed (sp);
		if (shotCount)
		{
			SDL_Rect src = cGraphicsData::getRect_SmallSymbol_Shots();
			SDL_Rect shotDest = dest;
			shotDest.x += (zoomedTileSize.x() - shotCount * (src.w + 2)) / 2;
			shotDest.y += zoomedTileSize.y() - narrow_cast<int> ((src.h + 4) * getZoomFactor());
			for (int i = 0; i != shotCount; ++i) {
				SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get(), &src, cVideo::buffer, &shotDest);
				shotDest.x += narrow_cast<int> ((src.w + 2) * getZoomFactor());
			}
		}
	};
	const auto& path = moveJob->getPath();
	for (const auto& nextWp : path)
	{
		ndest.x += mx = nextWp.x() * zoomedTileSize.x() - wp.x() * zoomedTileSize.x();
		ndest.y += my = nextWp.y() * zoomedTileSize.y() - wp.y() * zoomedTileSize.y();

		const int costs = cPathCalculator::calcNextCost (wp, nextWp, &vehicle, mapView.get());

		if (sp < costs)
		{
			drawPathArrow (dest, ndest, true);
			sp = vehicle.data.getSpeedMax();
			drawShots();
		}
		else
		{
			drawPathArrow (dest, ndest, false);
			drawShots();
		}
		sp -= costs;

		wp = nextWp;
		dest = ndest;
	}

	ndest.x += mx;
	ndest.y += my;
	drawPathArrow (dest, ndest, sp == 0);
	drawShots();
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawPathArrow (SDL_Rect dest, const SDL_Rect& lastDest, bool spezialColor) const
{
	int index;
	if (dest.x > lastDest.x && dest.y < lastDest.y)
		index = 0;
	else if (dest.x == lastDest.x && dest.y < lastDest.y)
		index = 1;
	else if (dest.x < lastDest.x && dest.y < lastDest.y)
		index = 2;
	else if (dest.x > lastDest.x && dest.y == lastDest.y)
		index = 3;
	else if (dest.x < lastDest.x && dest.y == lastDest.y)
		index = 4;
	else if (dest.x > lastDest.x && dest.y > lastDest.y)
		index = 5;
	else if (dest.x == lastDest.x && dest.y > lastDest.y)
		index = 6;
	else if (dest.x < lastDest.x && dest.y > lastDest.y)
		index = 7;
	else
		return;

	if (spezialColor)
	{
		SDL_BlitSurface (OtherData.WayPointPfeileSpecial[index][64 - dest.w].get(), nullptr, cVideo::buffer, &dest);
	}
	else
	{
		SDL_BlitSurface (OtherData.WayPointPfeile[index][64 - dest.w].get(), nullptr, cVideo::buffer, &dest);
	}
}

//------------------------------------------------------------------------------
SDL_Rect cGameMapWidget::computeTileDrawingArea (const cPosition& zoomedTileSize, const cPosition& zoomedStartTilePixelOffset, const cPosition& tileStartIndex, const cPosition& tileIndex) const
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

	const cPosition tilePosition (std::clamp (x, 0, staticMap->getSize().x() - 1), std::clamp (y, 0, staticMap->getSize().y() - 1));

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

	if (unitSelectionBox.isValidStart() && isAt (mouse.getPosition()) && mouse.isButtonPressed (eMouseButtonType::Left) && !mouse.isButtonPressed (eMouseButtonType::Right))
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

	if (button == eMouseButtonType::Left && !mouse.isButtonPressed (eMouseButtonType::Right) && !unitSelectionBox.isValidStart() && isAt (mouse.getPosition()))
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

	if (button == eMouseButtonType::Left && !mouse.isButtonPressed (eMouseButtonType::Right) && !unitSelectionBox.isTooSmall() && mapView && player)
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
			if (selectedUnit && (overVehicle == selectedUnit || overPlane == selectedUnit || overBuilding == selectedUnit || overBaseBuilding == selectedUnit))
			{
				triggeredUnitHelp (*selectedUnit);
			}
			else
			{
				unitSelection.selectUnitAt (field, true);
			}
		}
		else if ((!mouse.isButtonPressed (eMouseButtonType::Left) /*&& rightMouseBox.isTooSmall()*/) || (KeysList.getMouseStyle() == eMouseStyle::OldSchool && mouse.isButtonPressed (eMouseButtonType::Left)))
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

					auto it = ranges::find (units, selectedUnit);
					if (it != units.end())
					{
						it++;
						if (it == units.end()) it = units.begin();
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
			if (changeAllowed && selectedVehicle && (ranges::contains (field.getPlanes(), selectedVehicle) || selectedVehicle == overVehicle))
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
	EraseIf (animations, [&] (const auto& animation) { return animation->isFinished() || !animation->isLocatedIn (tileDrawingArea); });

	// add animations for units that just entered the visible area.
	if (mapView)
	{
		std::vector<cUnit*> units;
		for (auto i = makeIndexIterator (tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
		{
			const auto position = *i;
			const auto& field = mapView->getField (position);

			units = field.getUnits();

			for (const auto* unit : units)
			{
				if (shouldDrawUnit (*unit, position, tileDrawingRange) && !oldTileDrawingArea.intersects (unit->getArea()))
				{
					addAnimationsForUnit (*unit);
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::addAnimationsForUnit (const cUnit& unit)
{
	if (!cSettings::getInstance().isAnimations()) return;

	if (const cBuilding* building = dynamic_cast<const cBuilding*> (&unit))
	{
		if (building->isRubble()) return;
		auto& uiData = UnitsUiData.getBuildingUI (*building);

		if (uiData.staticData.powerOnGraphic || building->getStaticData().canWork)
		{
			animations.push_back (std::make_unique<cAnimationWork> (*animationTimer, *building));
		}
	}
	else if (const auto* vehicle = dynamic_cast<const cVehicle*> (&unit))
	{
		if (unit.getStaticUnitData().factorAir > 0)
		{
			animations.push_back (std::make_unique<cAnimationDither> (*animationTimer, *vehicle));
		}
		if (unit.getStaticUnitData().canBuild.compare ("BigBuilding") == 0)
		{
			animations.push_back (std::make_unique<cAnimationStartUpBuildingSite> (*animationTimer, *vehicle));
		}
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
		position.y() += -(position.y() - getPosition().y());
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

	collidingUnitCommandShortcuts.insert (std::make_pair (attackShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (buildShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (transferShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (enterShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (automoveShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (startShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (stopShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (clearShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (sentryShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (manualFireShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (activateShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (loadShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (relaodShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (repairShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (layMineShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (clearMineShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (disableShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (stealShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (infoShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (distributeShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (researchShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (upgradeShortcut, std::set<const cShortcut*>()));
	collidingUnitCommandShortcuts.insert (std::make_pair (destroyShortcut, std::set<const cShortcut*>()));

	for (auto it = collidingUnitCommandShortcuts.begin(); it != collidingUnitCommandShortcuts.end(); ++it)
	{
		auto it2 = it;
		++it2;
		for (; it2 != collidingUnitCommandShortcuts.end(); ++it2)
		{
			if (it->first->getKeySequence() == it2->first->getKeySequence())
			{
				it->second.insert (it2->first);
				it2->second.insert (it->first);
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

	auto* selectedVehicle = dynamic_cast<cVehicle*> (selectedUnit);
	auto* selectedBuilding = dynamic_cast<cBuilding*> (selectedUnit);
	// NOTE: the order in which we activate the shortcuts here marks the priority in which
	//       colliding shortcuts will be executed.
	if (cUnitContextMenuWidget::unitHasBuildEntry (selectedUnit, player.get())) activateShortcutConditional (*buildShortcut, blockedShortcuts, collidingUnitCommandShortcuts[buildShortcut]);
	if (cUnitContextMenuWidget::unitHasTransferEntry (selectedUnit, player.get())) activateShortcutConditional (*transferShortcut, blockedShortcuts, collidingUnitCommandShortcuts[transferShortcut]);
	if (cUnitContextMenuWidget::unitHasStartEntry (selectedBuilding, player.get())) activateShortcutConditional (*startShortcut, blockedShortcuts, collidingUnitCommandShortcuts[startShortcut]);
	if (cUnitContextMenuWidget::unitHasStopEntry (selectedUnit, player.get())) activateShortcutConditional (*stopShortcut, blockedShortcuts, collidingUnitCommandShortcuts[stopShortcut]);
	if (cUnitContextMenuWidget::unitHasSentryEntry (selectedUnit, player.get())) activateShortcutConditional (*sentryShortcut, blockedShortcuts, collidingUnitCommandShortcuts[sentryShortcut]);
	if (cUnitContextMenuWidget::unitHasManualFireEntry (selectedUnit, player.get())) activateShortcutConditional (*manualFireShortcut, blockedShortcuts, collidingUnitCommandShortcuts[manualFireShortcut]);
	if (cUnitContextMenuWidget::unitHasAttackEntry (selectedUnit, player.get())) activateShortcutConditional (*attackShortcut, blockedShortcuts, collidingUnitCommandShortcuts[attackShortcut]);
	if (cUnitContextMenuWidget::unitHasLayMinesEntry (selectedVehicle, player.get())) activateShortcutConditional (*layMineShortcut, blockedShortcuts, collidingUnitCommandShortcuts[layMineShortcut]);
	if (cUnitContextMenuWidget::unitHasCollectMinesEntry (selectedVehicle, player.get())) activateShortcutConditional (*clearMineShortcut, blockedShortcuts, collidingUnitCommandShortcuts[clearMineShortcut]);
	if (cUnitContextMenuWidget::unitHasLoadEntry (selectedUnit, player.get())) activateShortcutConditional (*loadShortcut, blockedShortcuts, collidingUnitCommandShortcuts[loadShortcut]);
	if (cUnitContextMenuWidget::unitHasEnterEntry (selectedVehicle, player.get())) activateShortcutConditional (*enterShortcut, blockedShortcuts, collidingUnitCommandShortcuts[enterShortcut]);
	if (cUnitContextMenuWidget::unitHasActivateEntry (selectedUnit, player.get())) activateShortcutConditional (*activateShortcut, blockedShortcuts, collidingUnitCommandShortcuts[activateShortcut]);
	if (cUnitContextMenuWidget::unitHasBuyEntry (selectedBuilding, player.get())) activateShortcutConditional (*upgradeShortcut, blockedShortcuts, collidingUnitCommandShortcuts[upgradeShortcut]);
	if (cUnitContextMenuWidget::unitHasResearchEntry (selectedBuilding, player.get())) activateShortcutConditional (*researchShortcut, blockedShortcuts, collidingUnitCommandShortcuts[researchShortcut]);
	if (cUnitContextMenuWidget::unitHasSabotageEntry (selectedVehicle, player.get())) activateShortcutConditional (*disableShortcut, blockedShortcuts, collidingUnitCommandShortcuts[disableShortcut]);
	if (cUnitContextMenuWidget::unitHasStealEntry (selectedVehicle, player.get())) activateShortcutConditional (*stealShortcut, blockedShortcuts, collidingUnitCommandShortcuts[stealShortcut]);
	if (cUnitContextMenuWidget::unitHasAutoEntry (selectedVehicle, player.get())) activateShortcutConditional (*automoveShortcut, blockedShortcuts, collidingUnitCommandShortcuts[automoveShortcut]);
	if (cUnitContextMenuWidget::unitHasRemoveEntry (selectedVehicle, player.get(), mapView.get())) activateShortcutConditional (*clearShortcut, blockedShortcuts, collidingUnitCommandShortcuts[clearShortcut]);
	if (cUnitContextMenuWidget::unitHasSupplyEntry (selectedVehicle, player.get())) activateShortcutConditional (*relaodShortcut, blockedShortcuts, collidingUnitCommandShortcuts[relaodShortcut]);
	if (cUnitContextMenuWidget::unitHasRepairEntry (selectedVehicle, player.get())) activateShortcutConditional (*repairShortcut, blockedShortcuts, collidingUnitCommandShortcuts[repairShortcut]);
	if (cUnitContextMenuWidget::unitHasDistributeEntry (selectedBuilding, player.get())) activateShortcutConditional (*distributeShortcut, blockedShortcuts, collidingUnitCommandShortcuts[distributeShortcut]);
	//if (cUnitContextMenuWidget::unitHasUpgradeThisEntry (selectedBuilding, player.get(), dynamicMap.get())) activateShortcutConditional (*shortcut, blockedShortcuts, collidingUnitCommandShortcuts[shortcut]);
	//if (cUnitContextMenuWidget::unitHasUpgradeAllEntry (selectedBuilding, player.get(), dynamicMap.get())) activateShortcutConditional (*shortcut, blockedShortcuts, collidingUnitCommandShortcuts[shortcut]);
	if (cUnitContextMenuWidget::unitHasSelfDestroyEntry (selectedBuilding, player.get())) activateShortcutConditional (*destroyShortcut, blockedShortcuts, collidingUnitCommandShortcuts[destroyShortcut]);
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
	for (auto& effect : effects)
	{
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

	const auto [topLeft, bottomRight] = computeTileDrawingRange();

	for (auto i = makeIndexIterator (topLeft, bottomRight); i.hasMore(); i.next())
	{
		auto& mapField = mapView->getField (*i);

		for (const auto& building : mapField.getBuildings())
		{
			renewDamageEffect (*building);
		}
		for (const auto& plane : mapField.getPlanes())
		{
			renewDamageEffect (*plane);
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
	auto& uiData = UnitsUiData.getBuildingUI (building);

	if (uiData.staticData.hasDamageEffect && building.data.getHitpoints() < building.data.getHitpointsMax() && (building.getOwner() == player.get() || (!player || mapView->canSeeUnit (building))))
	{
		int intense = (int) (200 - 200 * ((float) building.data.getHitpoints() / building.data.getHitpointsMax()));
		addEffect (std::make_shared<cFxDarkSmoke> (cPosition (building.getPosition().x() * 64 + building.getDamageFXPoint().x(), building.getPosition().y() * 64 + building.getDamageFXPoint().y()), intense, windDirection));

		if (building.getIsBig() && intense > 50)
		{
			intense -= 50;
			addEffect (std::make_shared<cFxDarkSmoke> (cPosition (building.getPosition().x() * 64 + building.getDamageFXPoint2().x(), building.getPosition().y() * 64 + building.getDamageFXPoint2().y()), intense, windDirection));
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::renewDamageEffect (const cVehicle& vehicle)
{
	if (vehicle.data.getHitpoints() < vehicle.data.getHitpointsMax() && (vehicle.getOwner() == player.get() || (!player || mapView->canSeeUnit (vehicle))))
	{
		int intense = (int) (100 - 100 * ((float) vehicle.data.getHitpoints() / vehicle.data.getHitpointsMax()));
		addEffect (std::make_shared<cFxDarkSmoke> (cPosition (vehicle.getPosition().x() * 64 + vehicle.DamageFXPoint.x() + vehicle.getMovementOffset().x(), vehicle.getPosition().y() * 64 + vehicle.DamageFXPoint.y() + vehicle.getMovementOffset().y()), intense, windDirection));
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
		if (dir >= 360)
			dir -= 360;
		else if (dir < 0)
			dir += 360;

		if (nextDirChange == 0)
		{
			nextDirChange = random (25) + 10;
			change = random (11) - 5;
		}
		else
			nextDirChange--;
	}
	else
		nextChange--;
}
