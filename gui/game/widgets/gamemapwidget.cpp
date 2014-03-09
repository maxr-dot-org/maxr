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

#include "gamemapwidget.h"
#include "unitcontextmenuwidget.h"
#include "../hud.h"
#include "../temp/animationtimer.h"
#include "../../application.h"
#include "../../../map.h"
#include "../../../settings.h"
#include "../../../video.h"
#include "../../../main.h"
#include "../../../player.h"
#include "../../../vehicles.h"
#include "../../../buildings.h"
#include "../../../keys.h"
#include "../../../clist.h"
#include "../../../attackJobs.h"
#include "../../../sound.h"
#include "../../../movejobs.h"
#include "../../../utility/indexiterator.h"
#include "../../../input/mouse/mouse.h"

//------------------------------------------------------------------------------
cGameMapWidget::cGameMapWidget (const cBox<cPosition>& area, std::shared_ptr<const cStaticMap> staticMap_, std::shared_ptr<cAnimationTimer> animationTimer_) :
	cClickableWidget (area),
	animationTimer (animationTimer_),
	staticMap (std::move (staticMap_)),
	dynamicMap (nullptr),
	player (nullptr),
	unitDrawingEngine (animationTimer),
	pixelOffset (0, 0),
	internalZoomFactor (1.),
	shouldDrawSurvey (false),
	shouldDrawScan (false),
	shouldDrawGrid (false),
	shouldDrawRange (false),
	shouldDrawFog (false),
	lockActive (false),
	mouseInputMode (eNewMouseInputMode::Default)
{
	assert (staticMap != nullptr);

	// FIXME: should this really be done here?
	signalConnectionManager.connect (animationTimer->triggered400ms, [&]()
	{
		const_cast<cStaticMap&>(*staticMap).generateNextAnimationFrame ();
	});

	unitMenu = addChild (std::make_unique<cUnitContextMenuWidget> ());
	unitMenu->disable ();
	unitMenu->hide ();

	mouseInputModeChanged.connect (std::bind (static_cast<void (cGameMapWidget::*)()>(&cGameMapWidget::updateMouseCursor), this));

	scrolled.connect (std::bind (static_cast<void (cGameMapWidget::*)()>(&cGameMapWidget::updateMouseCursor), this));
	scrolled.connect (std::bind (&cGameMapWidget::updateUnitMenuPosition, this));

	unitSelection.mainSelectionChanged.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitSelection.mainSelectionChanged.connect (std::bind (&cGameMapWidget::setMouseInputMode, this, eNewMouseInputMode::Default));
	unitSelection.selectionChanged.connect (std::bind (static_cast<void (cGameMapWidget::*)()>(&cGameMapWidget::updateMouseCursor), this));

	unitMenu->attackToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eNewMouseInputMode::Attack));
	unitMenu->transferToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eNewMouseInputMode::Transfer));
	unitMenu->loadToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eNewMouseInputMode::Load));
	unitMenu->supplyAmmoToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eNewMouseInputMode::SupplyAmmo));
	unitMenu->repairToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eNewMouseInputMode::Repair));
	unitMenu->sabotageToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eNewMouseInputMode::Disable));
	unitMenu->stealToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eNewMouseInputMode::Steal));

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
}

//------------------------------------------------------------------------------
void cGameMapWidget::setDynamicMap (const cMap* dynamicMap_)
{
	dynamicMap = dynamicMap_;

	dynamicMapSignalConnectionManager.disconnectAll ();

	if (dynamicMap != nullptr)
	{
		dynamicMapSignalConnectionManager.connect (dynamicMap->addedUnit, [&](const cUnit&){ updateMouseCursor (); });
		dynamicMapSignalConnectionManager.connect (dynamicMap->removedUnit, [&](const cUnit& unit)
		{
			updateMouseCursor ();
			if (unitSelection.isSelected (unit))
			{
				unitSelection.deselectUnit (unit);
			}
		});
		dynamicMapSignalConnectionManager.connect (dynamicMap->movedVehicle, [&](const cVehicle&){ updateMouseCursor (); });
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::setPlayer (const cPlayer* player_)
{
	player = player_;
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
	if (shouldDrawSurvey || (selectedVehicle && selectedVehicle->owner == player && selectedVehicle->data.canSurvey))
	{
		drawResources ();
	}

	if (selectedVehicle && ((selectedVehicle->ClientMoveJob && selectedVehicle->ClientMoveJob->bSuspended) || selectedVehicle->BuildPath))
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
void cGameMapWidget::setZoomFactor (double zoomFactor_, bool center)
{
	const auto oldZoom = getZoomFactor();

	internalZoomFactor = zoomFactor_;

	internalZoomFactor = std::max (internalZoomFactor, computeMinimalZoomFactor ());
	internalZoomFactor = std::min (1., internalZoomFactor);

	const auto newZoom = getZoomFactor ();

	if (oldZoom != newZoom)
	{
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
void cGameMapWidget::toggleHelpMode ()
{
	toggleMouseInputMode (eNewMouseInputMode::Help);
}

//------------------------------------------------------------------------------
cBox<cPosition> cGameMapWidget::getDisplayedMapArea () const
{
	auto tileDrawingRange = computeTileDrawingRange ();

	return cBox<cPosition> (tileDrawingRange.first, tileDrawingRange.second-1);
}

//------------------------------------------------------------------------------
double cGameMapWidget::getZoomFactor () const
{
	return (double)getZoomedTileSize ().x () / cStaticMap::tilePixelWidth; // should make no difference if we use y instead
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
void cGameMapWidget::toggleUnitContextMenu (const cUnit* unit)
{
	if (unitMenu->isEnabled () || unit == nullptr)
	{
		unitMenu->disable ();
		unitMenu->hide ();
	}
	else
	{
		unitMenu->setUnit (unit, mouseInputMode, player, dynamicMap);
		unitMenu->enable ();
		unitMenu->show ();
		updateUnitMenuPosition ();
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::setMouseInputMode (eNewMouseInputMode mouseInputMode_)
{
	const auto oldMode = mouseInputMode;
	mouseInputMode = mouseInputMode_;

	if (mouseInputMode != oldMode) mouseInputModeChanged ();
}

//------------------------------------------------------------------------------
void cGameMapWidget::toggleMouseInputMode (eNewMouseInputMode mouseInputMode_)
{
	if (mouseInputMode == mouseInputMode_)
	{
		setMouseInputMode (eNewMouseInputMode::Default);
	}
	else
	{
		setMouseInputMode (mouseInputMode_);
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

	auto oldPixelOffset = pixelOffset;

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

		scrolled ();
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::centerAt (const cPosition& position)
{
	const auto zoomedTileSize = getZoomedTileSize ();

	cPosition newPixelOffset;
	newPixelOffset.x () = position.x () * cStaticMap::tilePixelWidth - ((int)(((double)getSize ().x () / (2 * zoomedTileSize.x ())) * cStaticMap::tilePixelWidth)) + cStaticMap::tilePixelWidth / 2;
	newPixelOffset.y () = position.y () * cStaticMap::tilePixelHeight - ((int)(((double)getSize ().y () / (2 * zoomedTileSize.y ())) * cStaticMap::tilePixelHeight)) + cStaticMap::tilePixelHeight / 2;

	scroll (newPixelOffset - pixelOffset);
}

//------------------------------------------------------------------------------
void cGameMapWidget::startFindBuildPosition (const sID& buildId)
{
	currentBuildUnitId = buildId;
	setMouseInputMode (eNewMouseInputMode::SelectBuildPosition);
}

//------------------------------------------------------------------------------
void cGameMapWidget::startFindPathBuildPosition ()
{
	setMouseInputMode (eNewMouseInputMode::SelectBuildPathDestintaion);
}

//------------------------------------------------------------------------------
void cGameMapWidget::addEffect (std::shared_ptr<cFx> effect)
{
	if (effect != nullptr)
	{
		effect->playSound ();
		effects.push_back (std::move (effect));
	}
}

//------------------------------------------------------------------------------
double cGameMapWidget::computeMinimalZoomFactor () const
{
	// inequality to be fulfilled:
	//
	//   round(tile_x * zoom) * map_x <= size_x
	//
	// we first discard the 'round' and solve for zoom:
	//
	//   zoom = size_x / (map_x * tile_x)

	auto xZoom = (double)getSize ().x () / (staticMap->getSizeNew ().x () * cStaticMap::tilePixelWidth);
	auto yZoom = (double)getSize ().y () / (staticMap->getSizeNew ().y () * cStaticMap::tilePixelHeight);

	// then we try to fix if round would have rounded up:

	xZoom = std::max (xZoom, (double)((int)(cStaticMap::tilePixelWidth * xZoom) + (xZoom >= 1.0f ? 0 : 1)) / cStaticMap::tilePixelWidth);
	yZoom = std::max (yZoom, (double)((int)(cStaticMap::tilePixelHeight * yZoom) + (yZoom >= 1.0f ? 0 : 1)) / cStaticMap::tilePixelHeight);

	return std::max(xZoom, yZoom);
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::computeMaximalPixelOffset () const
{
	const auto x = staticMap->getSizeNew(). x () * cStaticMap::tilePixelWidth - (int)(getSize ().x () / getZoomFactor ());
	const auto y = staticMap->getSizeNew(). y () * cStaticMap::tilePixelHeight - (int)(getSize ().y () / getZoomFactor ());

	return cPosition (x, y);
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::zoomSize (const cPosition& size, double zoomFactor) const
{
	return cPosition ((int)std::round (size.x () * zoomFactor), (int)std::round (size.y () * zoomFactor));
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

	endTile.x () = std::min (staticMap->getSizeNew ().x (), endTile.x ());
	endTile.y () = std::min (staticMap->getSizeNew ().y (), endTile.y ());

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
				scaleSurface (terrain.shw_org, terrain.shw, zoomedTileSize.x (), zoomedTileSize.y ());
			}
			SDL_BlitSurface (terrain.shw, nullptr, cVideo::buffer, &drawDestination);
		}
		else
		{
			if (!cSettings::getInstance ().shouldDoPrescale () && (terrain.sf->w != zoomedTileSize.x () || terrain.sf->h != zoomedTileSize.y ()))
			{
				scaleSurface (terrain.sf_org, terrain.sf, zoomedTileSize.x (), zoomedTileSize.y ());
			}
			SDL_BlitSurface (terrain.sf, nullptr, cVideo::buffer, &drawDestination);
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

		if (effect->isFinished() || it->use_count() == 1)
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
		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);
		for (auto it = buildings.rbegin (); it != buildings.rend (); ++it)
		{
			if (*it == nullptr) continue; // should never happen

			const auto& building = *(*it);

			if (building.data.surfacePosition != sUnitData::SURFACE_POS_BENEATH_SEA &&
				building.data.surfacePosition != sUnitData::SURFACE_POS_BASE &&
				building.owner) break;

			if (!player || player->canSeeAnyAreaUnder (building))
			{
				// Draw big unit only once
				// FIXME: bug when (x,y) is outside of the drawing screen.
				if (building.PosX == i->x() && building.PosY == i->y())
				{
					unitDrawingEngine.drawUnit (building, drawDestination, getZoomFactor(), &unitSelection, player);
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
		// make sure a big building is drawn only once
		// FIXME: BUG: when PosX,PosY is outside of drawing screen
		if (building->PosX != i->x() || building->PosY != i->y()) continue;

		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);
		unitDrawingEngine.drawUnit (*building, drawDestination, getZoomFactor (), &unitSelection, player);

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
			unitDrawingEngine.drawUnit (*vehicle, drawDestination, getZoomFactor (), *dynamicMap, &unitSelection, player);
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

		auto drawDestination = computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);

		const auto& buildings = mapField.getBuildings ();
		for (auto it = buildings.begin (); it != buildings.end (); ++it)
		{
			const auto& building = *(*it);
			if (building.data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)
			{
				unitDrawingEngine.drawUnit (building, drawDestination, getZoomFactor (), &unitSelection, player);
			}
		}
		for (auto it = buildings.begin (); it != buildings.end (); ++it)
		{
			const auto& building = *(*it);
			if ((*it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)
			{
				unitDrawingEngine.drawUnit (building, drawDestination, getZoomFactor (), &unitSelection, player);
			}
		}

		auto vehicle = mapField.getVehicle();
		if (vehicle && (vehicle->isUnitClearing () || vehicle->isUnitBuildingABuilding ()) && (player && player->canSeeAnyAreaUnder (*vehicle)))
		{
			// make sure a big vehicle is drawn only once
			// FIXME: BUG: when PosX,PosY is outside of drawing screen
			if (vehicle->PosX == i->x() && vehicle->PosY == i->y())
			{
				unitDrawingEngine.drawUnit (*vehicle, drawDestination, getZoomFactor (), *dynamicMap, &unitSelection, player);
			}
		}
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
			unitDrawingEngine.drawUnit (*vehicle, drawDestination, getZoomFactor (), *dynamicMap, &unitSelection, player);
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
			unitDrawingEngine.drawUnit (*building, drawDestination, getZoomFactor (), &unitSelection, player);
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
			unitDrawingEngine.drawUnit (plane, drawDestination, getZoomFactor (), *dynamicMap, &unitSelection, player);
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
			if (!cSettings::getInstance ().shouldDoPrescale () && (ResourceData.res_metal->w != ResourceData.res_metal_org->w / 64 * zoomedTileSize.x () || ResourceData.res_metal->h != zoomedTileSize.y ())) scaleSurface (ResourceData.res_metal_org, ResourceData.res_metal, ResourceData.res_metal_org->w / 64 * zoomedTileSize.x (), zoomedTileSize.y ());
			SDL_BlitSurface (ResourceData.res_metal, &src, cVideo::buffer, &tmp);
		}
		else
		{
			src.x = resource.value * zoomedTileSize.x ();
			tmp = drawDestination;
			if (resource.typ == RES_METAL)
			{
				if (!cSettings::getInstance ().shouldDoPrescale () && (ResourceData.res_metal->w != ResourceData.res_metal_org->w / 64 * zoomedTileSize.x () || ResourceData.res_metal->h != zoomedTileSize.y ())) scaleSurface (ResourceData.res_metal_org, ResourceData.res_metal, ResourceData.res_metal_org->w / 64 * zoomedTileSize.x (), zoomedTileSize.y ());
				SDL_BlitSurface (ResourceData.res_metal, &src, cVideo::buffer, &tmp);
			}
			else if (resource.typ == RES_OIL)
			{
				if (!cSettings::getInstance ().shouldDoPrescale () && (ResourceData.res_oil->w != ResourceData.res_oil_org->w / 64 * zoomedTileSize.x () || ResourceData.res_oil->h != zoomedTileSize.y ())) scaleSurface (ResourceData.res_oil_org, ResourceData.res_oil, ResourceData.res_oil_org->w / 64 * zoomedTileSize.x (), zoomedTileSize.y ());
				SDL_BlitSurface (ResourceData.res_oil, &src, cVideo::buffer, &tmp);
			}
			else // Gold
			{
				if (!cSettings::getInstance ().shouldDoPrescale () && (ResourceData.res_gold->w != ResourceData.res_gold_org->w / 64 * zoomedTileSize.x () || ResourceData.res_gold->h != zoomedTileSize.y ())) scaleSurface (ResourceData.res_gold_org, ResourceData.res_gold, ResourceData.res_gold_org->w / 64 * zoomedTileSize.x (), zoomedTileSize.y ());
				SDL_BlitSurface (ResourceData.res_gold, &src, cVideo::buffer, &tmp);
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
				drawCircle (screenPosition.x () + zoomedTileSize.x (), screenPosition.y () + zoomedTileSize.y (), selectedVehicle->data.scan * zoomedTileSize.x (), SCAN_COLOR, cVideo::buffer);
			}
			else
			{
				drawCircle (screenPosition.x () + zoomedTileSize.x () / 2, screenPosition.y () + zoomedTileSize.y () / 2, selectedVehicle->data.scan * zoomedTileSize.x (), SCAN_COLOR, cVideo::buffer);
			}
		}
		if (shouldDrawRange)
		{
			if (selectedVehicle->data.canAttack & TERRAIN_AIR) drawCircle (screenPosition.x () +zoomedTileSize.x () / 2, screenPosition.y () + zoomedTileSize.y () / 2, selectedVehicle->data.range * zoomedTileSize.x () + 2, RANGE_AIR_COLOR, cVideo::buffer);
			else drawCircle (screenPosition.x () + zoomedTileSize.x () / 2, screenPosition.y () + zoomedTileSize.y () / 2, selectedVehicle->data.range * zoomedTileSize.x () + 1, RANGE_GROUND_COLOR, cVideo::buffer);
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
							selectedBuilding->data.scan * zoomedTileSize.x (), SCAN_COLOR, cVideo::buffer);
			}
			else
			{
				drawCircle (screenPosition. x() + zoomedTileSize.x () / 2,
							screenPosition. y() + zoomedTileSize.y () / 2,
							selectedBuilding->data.scan * zoomedTileSize.x (), SCAN_COLOR, cVideo::buffer);
			}
		}
		if (shouldDrawRange && (selectedBuilding->data.canAttack & TERRAIN_GROUND) && !selectedBuilding->data.explodesOnContact)
		{
			drawCircle (screenPosition. x() + zoomedTileSize.x () / 2,
						screenPosition. y() + zoomedTileSize.y () / 2,
						selectedBuilding->data.range * zoomedTileSize.x () + 2, RANGE_GROUND_COLOR, cVideo::buffer);
		}
		if (shouldDrawRange && (selectedBuilding->data.canAttack & TERRAIN_AIR))
		{
			drawCircle (screenPosition. x() + zoomedTileSize.x () / 2,
						screenPosition. y() + zoomedTileSize.y () / 2,
						selectedBuilding->data.range * zoomedTileSize.x () + 2, RANGE_AIR_COLOR, cVideo::buffer);
		}
	}

	SDL_SetClipRect (cVideo::buffer, NULL);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawExitPoints ()
{

	auto selectedVehicle = unitSelection.getSelectedVehicle ();
	auto selectedBuilding = unitSelection.getSelectedBuilding ();

	const auto zoomedTileSize = getZoomedTileSize ();

	if (selectedVehicle && selectedVehicle->isDisabled () == false)
	{
		if (dynamicMap && selectedVehicle->owner == player &&
			(
				(selectedVehicle->isUnitBuildingABuilding() && selectedVehicle->getBuildTurns() == 0) ||
				(selectedVehicle->isUnitClearing () && selectedVehicle->getClearingTurns() == 0)
			) && !selectedVehicle->BuildPath)
		{
			drawExitPointsIf (*selectedVehicle, [&](const cPosition& position){ return dynamicMap->possiblePlace (*selectedVehicle, position.x (), position.y ()); });
		}
		if (mouseInputMode == eNewMouseInputMode::Activate && selectedVehicle->owner == player)
		{
			//auto unitToExit = selectedVehicle->storedUnits[vehicleToActivate]->data;
			//drawExitPointsIf (*selectedVehicle, [&](const cPosition& position){ return selectedVehicle->canExitTo (position, *dynamicMap, *unitToExit); });
		}
	}
	else if (selectedBuilding && selectedBuilding->isDisabled () == false)
	{
		if (selectedBuilding->BuildList.empty () == false &&
			!selectedBuilding->isUnitWorking () &&
			selectedBuilding->BuildList[0].metall_remaining <= 0 &&
			selectedBuilding->owner == player)
		{
			auto unitToExit = selectedBuilding->BuildList[0].type.getUnitDataOriginalVersion ();
			drawExitPointsIf (*selectedBuilding, [&](const cPosition& position){ return selectedBuilding->canExitTo (position, *dynamicMap, *unitToExit); });
		}
		if (mouseInputMode == eNewMouseInputMode::Activate && selectedBuilding->owner == player)
		{
			//auto unitToExit = selectedBuilding->storedUnits[vehicleToActivate]->data;
			//drawExitPointsIf (*selectedBuilding, [&](const cPosition& position){ return selectedBuilding->canExitTo (position, *dynamicMap, *unitToExit); });
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawExitPointsIf (const cUnit& unit, const std::function<bool (const cPosition&)>& predicate)
{
	if (!dynamicMap) return;

	auto adjacentPositions = unit.getAdjacentPositions ();

	for (int i = 0; i != adjacentPositions.size(); ++i)
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

	CHECK_SCALING (GraphicsData.gfx_exitpoints, GraphicsData.gfx_exitpoints_org, getZoomFactor());
	SDL_BlitSurface (GraphicsData.gfx_exitpoints, &src, cVideo::buffer, &drawDestination);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawBuildBand ()
{
	auto selectedVehicle = unitSelection.getSelectedVehicle ();
	auto selectedBuilding = unitSelection.getSelectedBuilding ();

	const auto zoomedTileSize = getZoomedTileSize ();

	if (selectedVehicle && !selectedVehicle->isDisabled ())
	{
		auto mouse = getActiveMouse ();

		if (!mouse || !getArea().withinOrTouches(mouse->getPosition())) return;

		if (mouseInputMode == eNewMouseInputMode::SelectBuildPosition)
		{
			bool validPosition;
			cPosition destination;
			std::tie (validPosition, destination) = findNextBuildPosition (cPosition (selectedVehicle->PosX, selectedVehicle->PosY), getMapTilePosition (mouse->getPosition ()), currentBuildUnitId);
			if (!validPosition) return;

			SDL_Rect dest;
			dest.x = getPosition ().x () - (int)(pixelOffset.x () * getZoomFactor ()) + zoomedTileSize.x () * destination.x ();
			dest.y = getPosition ().y () - (int)(pixelOffset.y () * getZoomFactor ()) + zoomedTileSize.y () * destination.y ();
			CHECK_SCALING (GraphicsData.gfx_band_big, GraphicsData.gfx_band_big_org, getZoomFactor ());
			SDL_BlitSurface (GraphicsData.gfx_band_big, NULL, cVideo::buffer, &dest);
		}
		else if (mouseInputMode == eNewMouseInputMode::SelectBuildPathDestintaion)
		{
			const auto mouseTilePosition = getMapTilePosition (mouse->getPosition ());
			if (mouseTilePosition.x () == selectedVehicle->PosX || mouseTilePosition.y () == selectedVehicle->PosY)
			{
				SDL_Rect dest;
				dest.x = getPosition ().x () - (int)(pixelOffset.x () * getZoomFactor ()) + zoomedTileSize.x () * mouseTilePosition.x ();
				dest.y = getPosition ().y () - (int)(pixelOffset.y () * getZoomFactor ()) + zoomedTileSize.y () * mouseTilePosition.y ();
				CHECK_SCALING (GraphicsData.gfx_band_small, GraphicsData.gfx_band_small_org, getZoomFactor ());
				SDL_BlitSurface (GraphicsData.gfx_band_small, NULL, cVideo::buffer, &dest);
			}
		}
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawLockList (const cPlayer& player)
{
	const auto zoomedTileSize = getZoomedTileSize ();

	for (size_t i = 0; i < player.LockList.size (); i++)
	{
		cUnit* unit = player.LockList[i];

		if (!player.canSeeAnyAreaUnder (*unit))
		{
			// FIXME: Do not change the lock state during drawing.
			//        Instead remove the locked unit when it goes out of range directly
			//unit->lockerPlayer = NULL;
			//player.LockList.erase (player.LockList.begin () + i);
			//i--;
			continue;
		}

		const auto screenPosition = getScreenPosition (*unit);

		if (shouldDrawScan)
		{
			if (unit->data.isBig)
				drawCircle (screenPosition.x () + zoomedTileSize.x (), screenPosition.y () + zoomedTileSize.y (), unit->data.scan * zoomedTileSize.x (), SCAN_COLOR, cVideo::buffer);
			else
				drawCircle (screenPosition.x () + zoomedTileSize.x () / 2, screenPosition.y () + zoomedTileSize.y () / 2, unit->data.scan * zoomedTileSize.x (), SCAN_COLOR, cVideo::buffer);
		}
		if (shouldDrawRange && (unit->data.canAttack & TERRAIN_GROUND))
			drawCircle (screenPosition.x () + zoomedTileSize.x () / 2, screenPosition.y () + zoomedTileSize.y () / 2,
						unit->data.range * zoomedTileSize.x () + 1, RANGE_GROUND_COLOR, cVideo::buffer);
		if (shouldDrawRange && (unit->data.canAttack & TERRAIN_AIR))
			drawCircle (screenPosition.x () + zoomedTileSize.x () / 2, screenPosition.y () + zoomedTileSize.y () / 2,
						unit->data.range * zoomedTileSize.x () + 2, RANGE_AIR_COLOR, cVideo::buffer);
		//if (ammoChecked () && unit->data.canAttack)
		//	drawMunBar (*unit, screenPos);
		//if (hitsChecked ())
		//	drawHealthBar (*unit, screenPos);
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawBuildPath (const cVehicle& vehicle)
{
	if (!vehicle.BuildPath || (vehicle.BandX == vehicle.PosX && vehicle.BandY == vehicle.PosY) || mouseInputMode == eNewMouseInputMode::SelectBuildPathDestintaion) return;

	const auto zoomedTileSize = getZoomedTileSize ();

	int mx = vehicle.PosX;
	int my = vehicle.PosY;
	int sp;
	if (mx < vehicle.BandX)
		sp = 4;
	else if (mx > vehicle.BandX)
		sp = 3;
	else if (my < vehicle.BandY)
		sp = 1;
	else
		sp = 6;

	while (mx != vehicle.BandX || my != vehicle.BandY)
	{
		SDL_Rect dest;
		dest.x = getPosition ().x () - (int)(pixelOffset.x () * getZoomFactor ()) + zoomedTileSize.x () * mx;
		dest.y = getPosition ().y () - (int)(pixelOffset.y () * getZoomFactor ()) + zoomedTileSize.y () * my;

		SDL_BlitSurface (OtherData.WayPointPfeileSpecial[sp][64 - zoomedTileSize.x ()], NULL, cVideo::buffer, &dest);

		if (mx < vehicle.BandX)
			mx++;
		else if (mx > vehicle.BandX)
			mx--;

		if (my < vehicle.BandY)
			my++;
		else if (my > vehicle.BandY)
			my--;
	}
	SDL_Rect dest;
	dest.x = getPosition ().x () - (int)(pixelOffset.x () * getZoomFactor ()) + zoomedTileSize.x () * mx;
	dest.y = getPosition ().y () - (int)(pixelOffset.y () * getZoomFactor ()) + zoomedTileSize.y () * my;

	SDL_BlitSurface (OtherData.WayPointPfeileSpecial[sp][64 - zoomedTileSize.x ()], NULL, cVideo::buffer, &dest);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawPath (const cVehicle& vehicle)
{
	auto moveJob = vehicle.ClientMoveJob;

	if (!moveJob || !moveJob->Waypoints || vehicle.owner != player)
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
	dest.x = getPosition ().x () - (int)(pixelOffset.x () * getZoomFactor ()) + zoomedTileSize.x () * vehicle.PosX;
	dest.y = getPosition ().y () - (int)(pixelOffset.y () * getZoomFactor ()) + zoomedTileSize.y () * vehicle.PosY;
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
			ndest.x += mx = wp->next->X * zoomedTileSize.x () - wp->X * zoomedTileSize.x ();
			ndest.y += my = wp->next->Y * zoomedTileSize.y () - wp->Y * zoomedTileSize.y ();
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

	const cPosition tilePosition (std::min (x, staticMap->getSizeNew ().x ()-1), std::min(y, staticMap->getSizeNew ().y ()-1));

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
		updateMouseCursor (mouse);
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
		if (MouseStyle == OldSchool && !mouse.isButtonPressed (eMouseButtonType::Left))
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
				if (unitSelection.selectUnitAt (field, true))
				{
					auto vehicle = unitSelection.getSelectedVehicle ();
					if (vehicle) vehicle->makeReport ();
				}
			}
		}
		else if ((!mouse.isButtonPressed (eMouseButtonType::Left) /*&& rightMouseBox.isTooSmall ()*/) ||
				 (MouseStyle == OldSchool && mouse.isButtonPressed (eMouseButtonType::Left)))
		{
			if (mouseInputMode == eNewMouseInputMode::Help)
			{
				toggleMouseInputMode (eNewMouseInputMode::Help);
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
		// Store the currently selected unit to determine
		// if the lock state of the clicked unit maybe has to be changed.
		// If the selected unit changes during the click handling,
		// then the newly selected unit has to be added / removed
		// from the "locked units" list.
		auto oldSelectedUnitForLock = selectedUnit;

		const auto mouseClickAction = getMouseClickAction (mouse);

		if (changeAllowed && mouseClickAction == eMouseClickAction::Transfer && selectedUnit)
		{
			if (overVehicle)
			{
				triggeredTransfer (*selectedUnit, *overVehicle);
			}
			else if (overBuilding)
			{
				triggeredTransfer (*selectedUnit, *overBuilding);
			}
			if (mouseInputMode == eNewMouseInputMode::Transfer) setMouseInputMode (eNewMouseInputMode::Default);
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::SelectBuildPosition && selectedVehicle && mouseInputMode == eNewMouseInputMode::SelectBuildPosition)
		{
			bool validPosition;
			cPosition destination;
			std::tie (validPosition, destination) = findNextBuildPosition (cPosition (selectedVehicle->PosX, selectedVehicle->PosY), tilePosition, currentBuildUnitId);
			if (validPosition)
			{
				selectedBuildPosition (*selectedVehicle, destination);
			}

			toggleMouseInputMode (eNewMouseInputMode::SelectBuildPosition);
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::SelectBuildPathDestintaion && selectedVehicle && mouseInputMode == eNewMouseInputMode::SelectBuildPathDestintaion)
		{
			cPosition destination;
			if (tilePosition.x () == selectedVehicle->PosX || tilePosition.y () == selectedVehicle->PosY) destination = tilePosition;
			else destination = cPosition (selectedVehicle->PosX, selectedVehicle->PosY);

			selectedBuildPathDestination (*selectedVehicle, destination);
			toggleMouseInputMode (eNewMouseInputMode::SelectBuildPathDestintaion);
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Activate && selectedUnit && mouseInputMode == eNewMouseInputMode::Activate)
		{
			triggeredActivateAt (*selectedUnit, tilePosition);
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Activate && selectedBuilding && selectedBuilding->BuildList.size ())
		{
			triggeredExitFinishedUnit (*selectedBuilding, tilePosition);
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Load && selectedUnit && mouseInputMode == eNewMouseInputMode::Load)
		{
			triggeredLoadAt (*selectedUnit, tilePosition);
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::SupplyAmmo && selectedVehicle && mouseInputMode == eNewMouseInputMode::SupplyAmmo)
		{
			if (overVehicle) triggeredSupplyAmmo (*selectedVehicle, *overVehicle);
			else if (overPlane && overPlane->FlightHigh == 0) triggeredSupplyAmmo (*selectedVehicle, *overPlane);
			else if (overBuilding) triggeredSupplyAmmo (*selectedVehicle, *overBuilding);
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Repair && selectedVehicle && mouseInputMode == eNewMouseInputMode::Repair)
		{
			if (overVehicle) triggeredRepair (*selectedVehicle, *overVehicle);
			else if (overPlane && overPlane->FlightHigh == 0) triggeredRepair (*selectedVehicle, *overPlane);
			else if (overBuilding) triggeredRepair (*selectedVehicle, *overBuilding);
		}
		else if (mouseInputMode == eNewMouseInputMode::Help)
		{
			if (overPlane)
			{
				triggeredUnitHelp (*overPlane);
			}
			else if (overVehicle)
			{
				triggeredUnitHelp (*overVehicle);
			}
			else if (overBuilding)
			{
				triggeredUnitHelp (*overBuilding);
			}
			else if (overBaseBuilding)
			{
				triggeredUnitHelp (*overBaseBuilding);
			}
			toggleMouseInputMode (eNewMouseInputMode::Help);
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Attack && selectedVehicle && !selectedVehicle->isAttacking() && !selectedVehicle->MoveJobActive)
		{
			triggeredAttack (*selectedVehicle, tilePosition);
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Attack && selectedBuilding && !selectedBuilding->isAttacking ())
		{
			triggeredAttack (*selectedBuilding, tilePosition);
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Steal && selectedVehicle)
		{
			if (overVehicle) triggeredSteal (*selectedVehicle, *overVehicle);
			else if (overPlane && overPlane->FlightHigh == 0) triggeredSteal (*selectedVehicle, *overPlane);
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Disable && selectedVehicle)
		{
			if (overVehicle) triggeredDisable (*selectedVehicle, *overVehicle);
			else if (overPlane && overPlane->FlightHigh == 0) triggeredDisable (*selectedVehicle, *overPlane);
			else if (overBuilding) triggeredDisable (*selectedVehicle, *overBuilding);
		}
		else if (MouseStyle == OldSchool && mouseClickAction == eMouseClickAction::Select && unitSelection.selectUnitAt (field, false))
		{
			auto vehicle = unitSelection.getSelectedVehicle ();
			if (vehicle) vehicle->makeReport ();
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Move && selectedVehicle && !selectedVehicle->moving && !selectedVehicle->isAttacking ())
		{
			if (selectedVehicle->isUnitBuildingABuilding ())
			{
				triggeredEndBuilding (*unitSelection.getSelectedVehicle (), tilePosition);
			}
			else
			{
				if (unitSelection.getSelectedVehiclesCount () > 1) triggeredMoveGroup (unitSelection.getSelectedVehicles (), tilePosition);
				else triggeredMoveSingle (*unitSelection.getSelectedVehicle (), tilePosition);
			}
		}
		else if (changeAllowed && selectedVehicle && (Contains (field.getPlanes (), selectedVehicle) || selectedVehicle == overVehicle))
		{
			if (!selectedVehicle->moving)
			{
				toggleUnitContextMenu (selectedVehicle);
				PlayFX (SoundData.SNDHudButton);
			}
		}
		else if (changeAllowed && selectedBuilding && (overBaseBuilding == selectedBuilding || overBuilding == selectedBuilding))
		{
			toggleUnitContextMenu (selectedBuilding);
			PlayFX (SoundData.SNDHudButton);
		}
		else if (MouseStyle == Modern && mouseClickAction == eMouseClickAction::Select && unitSelection.selectUnitAt (field, true))
		{
			auto vehicle = unitSelection.getSelectedVehicle ();
			if (vehicle) vehicle->makeReport ();
		}

		// toggle the lock state of an enemy unit,
		// if it is newly selected / deselected
		if (player && lockActive)
		{
			if (selectedUnit && selectedUnit != oldSelectedUnitForLock && selectedUnit->owner != player)
			{
				triggeredToggleUnitLock (tilePosition);
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------------
cPosition cGameMapWidget::getScreenPosition (const cUnit& unit, bool movementOffset) const
{
	cPosition position;

	const int offsetX = movementOffset ? unit.getMovementOffsetX () : 0;
	position.x() = getPosition ().x () - ((int)((pixelOffset.x () - offsetX) * getZoomFactor ())) + getZoomedTileSize ().x () * unit.PosX;

	const int offsetY = movementOffset ? unit.getMovementOffsetY () : 0;
	position.y() = getPosition ().y () - ((int)((pixelOffset.y () - offsetY) * getZoomFactor ())) + getZoomedTileSize ().y () * unit.PosY;

	return position;
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
eMouseClickAction cGameMapWidget::getMouseClickAction (const cMouse& mouse)
{
	if (!isAt (mouse.getPosition()) || !dynamicMap)
	{
		return eMouseClickAction::Unknown;
	}

	if (unitMenu->isEnabled () && !unitMenu->isHidden () && unitMenu->isAt (mouse.getPosition ()))
	{
		return eMouseClickAction::Unknown;
	}
	
	auto mapTilePosition = getMapTilePosition (mouse.getPosition ());
	
	const auto& field = dynamicMap->getField (mapTilePosition);

	const auto selectedUnit = unitSelection.getSelectedUnit ();
	const auto selectedVehicle = unitSelection.getSelectedVehicle ();
	const auto selectedBuilding = unitSelection.getSelectedBuilding ();

	if (selectedVehicle && selectedVehicle->owner == player && mouseInputMode == eNewMouseInputMode::SelectBuildPosition)
	{
		return eMouseClickAction::SelectBuildPosition;
	}
	else  if (selectedVehicle && selectedVehicle->owner == player && mouseInputMode == eNewMouseInputMode::SelectBuildPathDestintaion)
	{
		return eMouseClickAction::SelectBuildPathDestintaion;
	}
	else if (selectedUnit && selectedUnit->owner == player && mouseInputMode == eNewMouseInputMode::Transfer)
	{
		if (selectedUnit->canTransferTo (mapTilePosition, field))
		{
			return eMouseClickAction::Transfer;
		}
		else
		{
			return eMouseClickAction::None;
		}
	}
	else if (mouseInputMode == eNewMouseInputMode::Help)
	{
		return eMouseClickAction::Help;
	}
	else if (selectedVehicle && selectedVehicle->owner == player && mouseInputMode == eNewMouseInputMode::Attack)
	{
		if (selectedVehicle->data.muzzleType != sUnitData::MUZZLE_TYPE_TORPEDO || dynamicMap->isWaterOrCoast (mapTilePosition.x (), mapTilePosition.y ()))
		{
			return eMouseClickAction::Attack;
		}
		else
		{
			return eMouseClickAction::None;
		}
	}
	// Infiltrators: manual action from unit-menu
	// disable vs. vehicle/building
	else if (selectedVehicle && selectedVehicle->owner == player && mouseInputMode == eNewMouseInputMode::Disable)
	{
		if (selectedVehicle->canDoCommandoAction (mapTilePosition.x (), mapTilePosition.y (), *dynamicMap, false)
			&& (!field.getVehicle () || field.getVehicle ()->isDisabled () == false)
			&& (!field.getBuilding () || field.getBuilding ()->isDisabled () == false)
			)
		{
			return eMouseClickAction::Disable;
		}
		else
		{
			return eMouseClickAction::None;
		}
	}
	// steal vs. vehicle
	else if (selectedVehicle && selectedVehicle->owner == player && mouseInputMode == eNewMouseInputMode::Steal)
	{
		if (selectedVehicle->canDoCommandoAction (mapTilePosition.x (), mapTilePosition.y (), *dynamicMap, true))
		{
			return eMouseClickAction::Steal;
		}
		else
		{
			return eMouseClickAction::None;
		}
	}
	// Infiltrators: auto-action
	// no disable vs. disabled building
	else if (selectedVehicle && selectedVehicle->owner == player && selectedVehicle->canDoCommandoAction (mapTilePosition.x (), mapTilePosition.y (), *dynamicMap, false) && field.getBuilding () && field.getBuilding ()->isDisabled ())
	{
		return eMouseClickAction::None;
	}
	// vehicle can be disabled, and if it is ...
	else if (selectedVehicle && selectedVehicle->owner == player && selectedVehicle->canDoCommandoAction (mapTilePosition.x (), mapTilePosition.y (), *dynamicMap, false) && (!field.getVehicle () || !field.getVehicle ()->isDisabled ()))
	{
		return eMouseClickAction::Disable;
	}
	// ... disabled (the) vehicle can be stolen
	// (without selecting the 'steal' from menu)
	else if (selectedVehicle && selectedVehicle->owner == player && selectedVehicle->canDoCommandoAction (mapTilePosition.x (), mapTilePosition.y (), *dynamicMap, true))
	{
		return eMouseClickAction::Steal;
	}
	else if (selectedBuilding && selectedBuilding->owner == player && mouseInputMode == eNewMouseInputMode::Attack)
	{
		if (selectedBuilding->isInRange (mapTilePosition.x (), mapTilePosition.y ()))
		{
			return eMouseClickAction::Attack;
		}
		else
		{
			return eMouseClickAction::None;
		}
	}
	else if (selectedVehicle && selectedVehicle->owner == player && selectedVehicle->canAttackObjectAt (mapTilePosition.x (), mapTilePosition.y (), *dynamicMap, false, false))
	{
		return eMouseClickAction::Attack;
	}
	else if (selectedBuilding && selectedBuilding->owner == player && selectedBuilding->canAttackObjectAt (mapTilePosition.x (), mapTilePosition.y (), *dynamicMap))
	{
		return eMouseClickAction::Attack;
	}
	else if (selectedVehicle && selectedVehicle->owner == player && mouseInputMode == eNewMouseInputMode::SupplyAmmo)
	{
		if (selectedVehicle->canSupply (*dynamicMap, mapTilePosition.x (), mapTilePosition.y (), SUPPLY_TYPE_REARM))
		{
			return eMouseClickAction::SupplyAmmo;
		}
		else
		{
			return eMouseClickAction::None;
		}
	}
	else if (selectedVehicle && selectedVehicle->owner == player && mouseInputMode == eNewMouseInputMode::Repair)
	{
		if (selectedVehicle->canSupply (*dynamicMap, mapTilePosition.x (), mapTilePosition.y (), SUPPLY_TYPE_REPAIR))
		{
			return eMouseClickAction::Repair;
		}
		else
		{
			return eMouseClickAction::None;
		}
	}
	else if ((
				field.getVehicle() ||
				field.getPlane() ||
				(
					field.getBuilding() &&
					field.getBuilding()->owner
				)
			) &&
			(
				!selectedVehicle ||
				selectedVehicle->owner != player ||
				(
					(
						selectedVehicle->data.factorAir > 0 ||
						field.getVehicle() ||
						(
							field.getTopBuilding() &&
							field.getTopBuilding()->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE
						) ||
						(
							MouseStyle == OldSchool &&
							field.getPlane()
						)
					) &&
					(
						selectedVehicle->data.factorAir == 0 ||
						field.getPlane() ||
						(
							MouseStyle == OldSchool &&
							(
								field.getVehicle() ||
								(
									field.getTopBuilding() &&
									field.getTopBuilding()->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE &&
									!field.getTopBuilding()->data.canBeLandedOn
								)
							)
						)
					) &&
					mouseInputMode != eNewMouseInputMode::Load &&
					mouseInputMode != eNewMouseInputMode::Activate
				)
			) &&
			(
				!selectedBuilding ||
				selectedBuilding->owner != player ||
				(
					(
						selectedBuilding->BuildList.empty() ||
						selectedBuilding->isUnitWorking () ||
						selectedBuilding->BuildList[0].metall_remaining > 0
					) &&
					mouseInputMode != eNewMouseInputMode::Load &&
					mouseInputMode != eNewMouseInputMode::Activate
				)
			)
		)
	{
		return eMouseClickAction::Select;
	}
	else if (selectedVehicle && selectedVehicle->owner == player && mouseInputMode == eNewMouseInputMode::Load)
	{
		if (selectedVehicle->canLoad (mapTilePosition.x (), mapTilePosition.y (), *dynamicMap, false))
		{
			return eMouseClickAction::Load;
		}
		else
		{
			return eMouseClickAction::None;
		}
	}
	else if (selectedVehicle && selectedVehicle->owner == player && mouseInputMode == eNewMouseInputMode::Activate)
	{
		//if (selectedVehicle->canExitTo (mapTilePosition.x (), mapTilePosition.y (), *dynamicMap, selectedVehicle->storedUnits[vehicleToActivate]->data) && selectedUnit->isDisabled () == false)
		//{
		//	return eMouseClickAction::Activate;
		//}
		//else
		//{
		//	return eMouseClickAction::None;
		//}
	}
	else if (selectedVehicle && selectedVehicle->owner == player)
	{
		if (!selectedVehicle->isUnitBuildingABuilding () && !selectedVehicle->isUnitClearing () && mouseInputMode != eNewMouseInputMode::Load && mouseInputMode != eNewMouseInputMode::Activate)
		{
			if (selectedVehicle->MoveJobActive)
			{
				return eMouseClickAction::None;
			}
			else if (dynamicMap->possiblePlace (*selectedVehicle, mapTilePosition.x (), mapTilePosition.y (), true))
			{
				return eMouseClickAction::Move;
			}
			else
			{
				return eMouseClickAction::None;
			}
		}
		else if (selectedVehicle->isUnitBuildingABuilding () || selectedVehicle->isUnitClearing ())
		{
			if (((selectedVehicle->isUnitBuildingABuilding () && selectedVehicle->getBuildTurns() == 0) ||
				(selectedVehicle->isUnitClearing () && selectedVehicle->getClearingTurns () == 0)) &&
				dynamicMap->possiblePlace (*selectedVehicle, mapTilePosition.x (), mapTilePosition.y ()) && selectedVehicle->isNextTo (mapTilePosition.x (), mapTilePosition.y ()))
			{
				return eMouseClickAction::Move;
			}
			else
			{
				return eMouseClickAction::None;
			}
		}
	}
	else if (
		selectedBuilding &&
		selectedBuilding->owner == player &&
		!selectedBuilding->BuildList.empty () &&
		!selectedBuilding->isUnitWorking () &&
		selectedBuilding->BuildList[0].metall_remaining <= 0)
	{
		if (selectedBuilding->canExitTo (mapTilePosition, *dynamicMap, *selectedBuilding->BuildList[0].type.getUnitDataOriginalVersion ()) && selectedUnit->isDisabled () == false)
		{
			return eMouseClickAction::Activate;
		}
		else
		{
			return eMouseClickAction::None;
		}
	}
	else if (selectedBuilding && selectedBuilding->owner == player && mouseInputMode == eNewMouseInputMode::Activate && selectedUnit->isDisabled () == false)
	{
		//if (selectedBuilding->canExitTo (mapTilePosition.x (), mapTilePosition.y (), *dynamicMap, selectedBuilding->storedUnits[vehicleToActivate]->data))
		//{
		//	return eMouseClickAction::Activate;
		//}
		//else
		//{
		//	return eMouseClickAction::None;
		//}
	}
	else if (selectedBuilding && selectedBuilding->owner == player && mouseInputMode == eNewMouseInputMode::Load)
	{
		if (selectedBuilding->canLoad (mapTilePosition.x (), mapTilePosition.y (), *dynamicMap, false))
		{
			return eMouseClickAction::Load;
		}
		else
		{
			return eMouseClickAction::None;
		}
	}
	
	return eMouseClickAction::Unknown;
}

//------------------------------------------------------------------------------
std::pair<bool, cPosition> cGameMapWidget::findNextBuildPosition (const cPosition& sourcePosition, const cPosition& desiredPosition, const sID& unitId)
{
	if (!dynamicMap) return std::make_pair (false, cPosition ());

	bool pos[4] = {false, false, false, false};

	//check, which positions are available
	const auto& unitData = *unitId.getUnitDataOriginalVersion ();
	if (dynamicMap->possiblePlaceBuilding (unitData, sourcePosition.x () - 1, sourcePosition.y () - 1)
		&& dynamicMap->possiblePlaceBuilding (unitData, sourcePosition.x (), sourcePosition.y () - 1)
		&& dynamicMap->possiblePlaceBuilding (unitData, sourcePosition.x () - 1, sourcePosition.y ()))
	{
		pos[0] = true;
	}

	if (dynamicMap->possiblePlaceBuilding (unitData, sourcePosition.x (), sourcePosition.y () - 1)
		&& dynamicMap->possiblePlaceBuilding (unitData, sourcePosition.x () + 1, sourcePosition.y () - 1)
		&& dynamicMap->possiblePlaceBuilding (unitData, sourcePosition.x () + 1, sourcePosition.y ()))
	{
		pos[1] = true;
	}

	if (dynamicMap->possiblePlaceBuilding (unitData, sourcePosition.x () + 1, sourcePosition.y ())
		&& dynamicMap->possiblePlaceBuilding (unitData, sourcePosition.x () + 1, sourcePosition.y () + 1)
		&& dynamicMap->possiblePlaceBuilding (unitData, sourcePosition.x (), sourcePosition.y () + 1))
	{
		pos[2] = true;
	}

	if (dynamicMap->possiblePlaceBuilding (unitData, sourcePosition.x () - 1, sourcePosition.y ())
		&& dynamicMap->possiblePlaceBuilding (unitData, sourcePosition.x () - 1, sourcePosition.y () + 1)
		&& dynamicMap->possiblePlaceBuilding (unitData, sourcePosition.x (), sourcePosition.y () + 1))
	{
		pos[3] = true;
	}

	// chose the position, which matches the cursor position, if available
	if (desiredPosition.x () <= sourcePosition.x () && desiredPosition.y () <= sourcePosition.y () && pos[0])
	{
		return std::make_pair(true, cPosition(sourcePosition.x () - 1, sourcePosition.y () - 1));
	}

	if (desiredPosition.x () >= sourcePosition.x () && desiredPosition.y () <= sourcePosition.y () && pos[1])
	{
		return std::make_pair (true, cPosition (sourcePosition.x (), sourcePosition.y () - 1));
	}

	if (desiredPosition.x () >= sourcePosition.x () && desiredPosition.y () >= sourcePosition.y () && pos[2])
	{
		return std::make_pair (true, cPosition (sourcePosition.x (), sourcePosition.y ()));
	}

	if (desiredPosition.x () <= sourcePosition.x () && desiredPosition.y () >= sourcePosition.y () && pos[3])
	{
		return std::make_pair (true, cPosition (sourcePosition.x () - 1, sourcePosition.y ()));
	}

	// if the best position is not available, chose the next free one
	if (pos[0])
	{
		return std::make_pair (true, cPosition (sourcePosition.x () - 1, sourcePosition.y () - 1));
	}

	if (pos[1])
	{
		return std::make_pair (true, cPosition (sourcePosition.x (), sourcePosition.y () - 1));
	}

	if (pos[2])
	{
		return std::make_pair (true, cPosition (sourcePosition.x (), sourcePosition.y ()));
	}

	if (pos[3])
	{
		return std::make_pair (true, cPosition (sourcePosition.x () - 1, sourcePosition.y ()));
	}

	if (unitData.isBig)
	{
		return std::make_pair (true, cPosition (sourcePosition.x (), sourcePosition.y ()));
	}

	return std::make_pair (false, cPosition ());
}

//------------------------------------------------------------------------------
void cGameMapWidget::updateMouseCursor (cMouse& mouse)
{
	if (!isAt (mouse.getPosition ())) return;

	const auto mouseClickAction = getMouseClickAction (mouse);

	auto mapTilePosition = getMapTilePosition (mouse.getPosition ());

	auto selectedVehicle = unitSelection.getSelectedVehicle ();

	switch (mouseClickAction)
	{
	case eMouseClickAction::SelectBuildPosition:
		mouse.setCursorType (eMouseCursorType::Band);
		break;
	case eMouseClickAction::SelectBuildPathDestintaion:
		mouse.setCursorType (eMouseCursorType::Band);
		break;
	case eMouseClickAction::Transfer:
		mouse.setCursorType (eMouseCursorType::Transfer);
		break;
	case eMouseClickAction::Help:
		mouse.setCursorType (eMouseCursorType::Help);
		break;
	case eMouseClickAction::Attack:
		if (mouse.setCursorType (eMouseCursorType::Attack))
		{
			drawAttackCursor (mapTilePosition);
		}
		break;
	case eMouseClickAction::Disable:
		if (mouse.setCursorType (eMouseCursorType::Disable))
		{
			if (selectedVehicle) drawCommandoCursor (mapTilePosition, *selectedVehicle, false);
		}
		break;
	case eMouseClickAction::Steal:
		if (mouse.setCursorType (eMouseCursorType::Steal))
		{
			if (selectedVehicle) drawCommandoCursor (mapTilePosition, *selectedVehicle, true);
		}
		break;
	case eMouseClickAction::SupplyAmmo:
		mouse.setCursorType (eMouseCursorType::Muni);
		break;
	case eMouseClickAction::Repair:
		mouse.setCursorType (eMouseCursorType::Repair);
		break;
	case eMouseClickAction::Select:
		mouse.setCursorType (eMouseCursorType::Select);
		break;
	case eMouseClickAction::Load:
		mouse.setCursorType (eMouseCursorType::Load);
		break;
	case eMouseClickAction::Activate:
		mouse.setCursorType (eMouseCursorType::Activate);
		break;
	case eMouseClickAction::Move:
		mouse.setCursorType (eMouseCursorType::Move);
		break;
	case eMouseClickAction::None:
		mouse.setCursorType (eMouseCursorType::No);
		break;
	default:
	case eMouseClickAction::Unknown:
		mouse.setCursorType (eMouseCursorType::Hand);
		break;
	}
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawAttackCursor (const cPosition& position) const
{
	auto selectedUnit = unitSelection.getSelectedUnit ();
	if (selectedUnit == NULL) return;

	const sUnitData& data = selectedUnit->data;
	cUnit* target = selectTarget (position.x (), position.y (), data.canAttack, *dynamicMap);

	if (!target || (target == selectedUnit))
	{
		SDL_Rect r = {1, 29, 35, 3};
		SDL_FillRect (GraphicsData.gfx_Cattack, &r, 0);
		return;
	}

	int t = target->data.hitpointsCur;
	int wc = (int)((float)t / target->data.hitpointsMax * 35);

	t = target->calcHealth (data.damage);

	int wp = 0;
	if (t)
	{
		wp = (int)((float)t / target->data.hitpointsMax * 35);
	}
	SDL_Rect r = {1, 29, Uint16 (wp), 3};

	if (r.w) SDL_FillRect (GraphicsData.gfx_Cattack, &r, 0x00FF00);

	r.x += r.w;
	r.w = wc - wp;

	if (r.w) SDL_FillRect (GraphicsData.gfx_Cattack, &r, 0xFF0000);

	r.x += r.w;
	r.w = 35 - wc;

	if (r.w) SDL_FillRect (GraphicsData.gfx_Cattack, &r, 0);
}

//------------------------------------------------------------------------------
void cGameMapWidget::drawCommandoCursor (const cPosition& position, const cVehicle& vehicle, bool steal) const
{
	const auto& field = dynamicMap->getField(position);
	SDL_Surface* sf;
	const cUnit* unit = 0;

	if (steal)
	{
		unit = field.getVehicle ();
		sf = GraphicsData.gfx_Csteal;
	}
	else
	{
		unit = field.getVehicle ();
		if (unit == 0)
			unit = field.getTopBuilding ();
		sf = GraphicsData.gfx_Cdisable;
	}

	SDL_Rect r = {1, 28, 35, 3};

	if (unit == 0)
	{
		SDL_FillRect (sf, &r, 0);
		return;
	}

	SDL_FillRect (sf, &r, 0x00FF0000);
	r.w = 35 * vehicle.calcCommandoChance (unit, steal) / 100;
	SDL_FillRect (sf, &r, 0x0000FF00);
}