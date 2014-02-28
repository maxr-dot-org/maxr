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
#include "hud.h"
#include "../../map.h"
#include "../../settings.h"
#include "../../video.h"
#include "../../main.h"
#include "../../player.h"
#include "../../vehicles.h"
#include "../../buildings.h"
#include "../../keys.h"
#include "../../clist.h"
#include "../../attackJobs.h"
#include "../../utility/indexiterator.h"
#include "../../input/mouse/mouse.h"

//------------------------------------------------------------------------------
cGameMapWidget::cGameMapWidget (const cBox<cPosition>& area, std::shared_ptr<const cStaticMap> staticMap_, std::shared_ptr<cAnimationTimer> animationTimer) :
	cClickableWidget (area),
	staticMap (std::move (staticMap_)),
	dynamicMap (nullptr),
	player (nullptr),
	unitDrawingEngine (std::move(animationTimer)),
	pixelOffset (0, 0),
	internalZoomFactor (1.),
	shouldDrawSurvey (false),
	shouldDrawScan (false),
	shouldDrawGrid (false),
	shouldDrawRange (false),
	shouldDrawFog (false),
	mouseInputMode (eNewMouseInputMode::Default)
{
	assert (staticMap != nullptr);

	unitMenu = addChild (std::make_unique<cUnitContextMenuWidget> ());
	unitMenu->disable ();
	unitMenu->hide ();

	mouseInputModeChanged.connect (std::bind (static_cast<void (cGameMapWidget::*)()>(&cGameMapWidget::updateMouseCursor), this));

	scrolled.connect (std::bind (static_cast<void (cGameMapWidget::*)()>(&cGameMapWidget::updateMouseCursor), this));
	scrolled.connect (std::bind (&cGameMapWidget::updateUnitMenuPosition, this));

	unitSelection.selectionChanged.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitSelection.selectionChanged.connect (std::bind (&cGameMapWidget::setMouseInputMode, this, eNewMouseInputMode::Default));

	unitMenu->attackToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eNewMouseInputMode::Attack));
	unitMenu->transferToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eNewMouseInputMode::Transfer));
	unitMenu->loadToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eNewMouseInputMode::Load));
	unitMenu->supplyAmmoToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eNewMouseInputMode::SupplyAmmo));
	unitMenu->repairToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eNewMouseInputMode::Repair));
	unitMenu->sabotageToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eNewMouseInputMode::Disable));
	unitMenu->stealToggled.connect (std::bind (&cGameMapWidget::toggleMouseInputMode, this, eNewMouseInputMode::Steal));

	unitMenu->attackToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->buildClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->distributeClicked.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
	unitMenu->transferToggled.connect (std::bind (&cGameMapWidget::toggleUnitContextMenu, this, nullptr));
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

	//if (selectedVehicle && ((selectedVehicle->ClientMoveJob && selectedVehicle->ClientMoveJob->bSuspended) || selectedVehicle->BuildPath))
	//{
	//	selectedVehicle->DrawPath (*this);
	//}

	//debugOutput.draw ();

	//drawSelectionBox (zoomOffX, zoomOffY);

	SDL_SetClipRect (cVideo::buffer, nullptr);

	drawUnitCircles ();

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
	//SDL_Rect clipRect = {cHud::panelLeftWidth, cHud::panelTopHeight, Uint16 (Video.getResolutionX () - cHud::panelTotalWidth), Uint16 (Video.getResolutionY () - cHud::panelTotalHeight)};
	//SDL_SetClipRect (cVideo::buffer, &clipRect);
	//SDL_SetClipRect (cVideo::buffer, &clipRect);

	//client->FxList->draw (*this, bottom);
	//FxList->draw (*this, bottom);

	//SDL_SetClipRect (cVideo::buffer, nullptr);
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
				// TODO: bug when (x,y) is outside of the drawing screen.
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
		// TODO: BUG: when PosX,PosY is outside of drawing screen
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
		if (vehicle && (vehicle->IsClearing || vehicle->IsBuilding) && (player && player->canSeeAnyAreaUnder (*vehicle)))
		{
			// make sure a big vehicle is drawn only once
			// TODO: BUG: when PosX,PosY is outside of drawing screen
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
		if (vehicle->data.factorGround != 0 && !vehicle->IsBuilding && !vehicle->IsClearing)
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
void cGameMapWidget::drawUnitCircles ()
{
	//SDL_Rect clipRect = {HUD_LEFT_WIDTH, HUD_TOP_HIGHT, Uint16 (Video.getResolutionX () - HUD_TOTAL_WIDTH), Uint16 (Video.getResolutionY () - HUD_TOTAL_HIGHT)};
	//SDL_SetClipRect (cVideo::buffer, &clipRect);

	//cVehicle* selectedVehicle = getSelectedVehicle ();
	//cBuilding* selectedBuilding = getSelectedBuilding ();
	//const cPlayer& player = client->getActivePlayer ();

	//if (selectedVehicle && selectedUnit->isDisabled () == false)
	//{
	//	cVehicle& v = *selectedVehicle;
	//	const bool movementOffset = !v.IsBuilding && !v.IsClearing;
	//	const int spx = getScreenPosX (v, movementOffset);
	//	const int spy = getScreenPosY (v, movementOffset);
	//	if (scanChecked ())
	//	{
	//		if (v.data.isBig)
	//		{
	//			drawCircle (spx + getTileSize (), spy + getTileSize (), v.data.scan * getTileSize (), SCAN_COLOR, cVideo::buffer);
	//		}
	//		else
	//		{
	//			drawCircle (spx + getTileSize () / 2, spy + getTileSize () / 2, v.data.scan * getTileSize (), SCAN_COLOR, cVideo::buffer);
	//		}
	//	}
	//	if (rangeChecked ())
	//	{
	//		if (v.data.canAttack & TERRAIN_AIR) drawCircle (spx + getTileSize () / 2, spy + getTileSize () / 2, v.data.range * getTileSize () + 2, RANGE_AIR_COLOR, cVideo::buffer);
	//		else drawCircle (spx + getTileSize () / 2, spy + getTileSize () / 2, v.data.range * getTileSize () + 1, RANGE_GROUND_COLOR, cVideo::buffer);
	//	}
	//	if (v.owner == &player &&
	//		(
	//		(v.IsBuilding && v.BuildRounds == 0) ||
	//		(v.IsClearing && v.ClearingRounds == 0)
	//		) && !v.BuildPath)
	//	{
	//		const cMap& map = *client->getMap ();

	//		if (v.data.isBig)
	//		{
	//			if (map.possiblePlace (v, v.PosX - 1, v.PosY - 1)) drawExitPoint (spx - getTileSize (), spy - getTileSize ());
	//			if (map.possiblePlace (v, v.PosX, v.PosY - 1)) drawExitPoint (spx, spy - getTileSize ());
	//			if (map.possiblePlace (v, v.PosX + 1, v.PosY - 1)) drawExitPoint (spx + getTileSize (), spy - getTileSize ());
	//			if (map.possiblePlace (v, v.PosX + 2, v.PosY - 1)) drawExitPoint (spx + getTileSize () * 2, spy - getTileSize ());
	//			if (map.possiblePlace (v, v.PosX - 1, v.PosY)) drawExitPoint (spx - getTileSize (), spy);
	//			if (map.possiblePlace (v, v.PosX + 2, v.PosY)) drawExitPoint (spx + getTileSize () * 2, spy);
	//			if (map.possiblePlace (v, v.PosX - 1, v.PosY + 1)) drawExitPoint (spx - getTileSize (), spy + getTileSize ());
	//			if (map.possiblePlace (v, v.PosX + 2, v.PosY + 1)) drawExitPoint (spx + getTileSize () * 2, spy + getTileSize ());
	//			if (map.possiblePlace (v, v.PosX - 1, v.PosY + 2)) drawExitPoint (spx - getTileSize (), spy + getTileSize () * 2);
	//			if (map.possiblePlace (v, v.PosX, v.PosY + 2)) drawExitPoint (spx, spy + getTileSize () * 2);
	//			if (map.possiblePlace (v, v.PosX + 1, v.PosY + 2)) drawExitPoint (spx + getTileSize (), spy + getTileSize () * 2);
	//			if (map.possiblePlace (v, v.PosX + 2, v.PosY + 2)) drawExitPoint (spx + getTileSize () * 2, spy + getTileSize () * 2);
	//		}
	//		else
	//		{
	//			if (map.possiblePlace (v, v.PosX - 1, v.PosY - 1)) drawExitPoint (spx - getTileSize (), spy - getTileSize ());
	//			if (map.possiblePlace (v, v.PosX, v.PosY - 1)) drawExitPoint (spx, spy - getTileSize ());
	//			if (map.possiblePlace (v, v.PosX + 1, v.PosY - 1)) drawExitPoint (spx + getTileSize (), spy - getTileSize ());
	//			if (map.possiblePlace (v, v.PosX - 1, v.PosY)) drawExitPoint (spx - getTileSize (), spy);
	//			if (map.possiblePlace (v, v.PosX + 1, v.PosY)) drawExitPoint (spx + getTileSize (), spy);
	//			if (map.possiblePlace (v, v.PosX - 1, v.PosY + 1)) drawExitPoint (spx - getTileSize (), spy + getTileSize ());
	//			if (map.possiblePlace (v, v.PosX, v.PosY + 1)) drawExitPoint (spx, spy + getTileSize ());
	//			if (map.possiblePlace (v, v.PosX + 1, v.PosY + 1)) drawExitPoint (spx + getTileSize (), spy + getTileSize ());
	//		}
	//	}
	//	if (mouseInputMode == placeBand)
	//	{
	//		if (v.BuildingTyp.getUnitDataOriginalVersion ()->isBig)
	//		{
	//			SDL_Rect dest;
	//			dest.x = HUD_LEFT_WIDTH - (int)(offX * getZoom ()) + getTileSize () * v.BandX;
	//			dest.y = HUD_TOP_HIGHT - (int)(offY * getZoom ()) + getTileSize () * v.BandY;
	//			CHECK_SCALING (GraphicsData.gfx_band_big, GraphicsData.gfx_band_big_org, getTileSize () / 64.0f);
	//			SDL_BlitSurface (GraphicsData.gfx_band_big, NULL, cVideo::buffer, &dest);
	//		}
	//		else
	//		{
	//			const auto mouseTilePosition = getTilePosition (cMouse::getInstance ().getPosition ());
	//			const int x = mouseTilePosition.x ();
	//			const int y = mouseTilePosition.y ();
	//			if (x == v.PosX || y == v.PosY)
	//			{
	//				SDL_Rect dest;
	//				dest.x = HUD_LEFT_WIDTH - (int)(offX * getZoom ()) + getTileSize () * x;
	//				dest.y = HUD_TOP_HIGHT - (int)(offY * getZoom ()) + getTileSize () * y;
	//				CHECK_SCALING (GraphicsData.gfx_band_small, GraphicsData.gfx_band_small_org, getTileSize () / 64.0f);
	//				SDL_BlitSurface (GraphicsData.gfx_band_small, NULL, cVideo::buffer, &dest);
	//				v.BandX = x;
	//				v.BandY = y;
	//			}
	//			else
	//			{
	//				v.BandX = v.PosX;
	//				v.BandY = v.PosY;
	//			}
	//		}
	//	}
	//	if (mouseInputMode == activateVehicle && v.owner == &player)
	//	{
	//		v.DrawExitPoints (v.storedUnits[vehicleToActivate]->data, *this);
	//	}
	//}
	//else if (selectedBuilding && selectedUnit->isDisabled () == false)
	//{
	//	const int spx = getScreenPosX (*selectedBuilding);
	//	const int spy = getScreenPosY (*selectedBuilding);
	//	if (scanChecked ())
	//	{
	//		if (selectedBuilding->data.isBig)
	//		{
	//			drawCircle (spx + getTileSize (),
	//						spy + getTileSize (),
	//						selectedBuilding->data.scan * getTileSize (), SCAN_COLOR, cVideo::buffer);
	//		}
	//		else
	//		{
	//			drawCircle (spx + getTileSize () / 2,
	//						spy + getTileSize () / 2,
	//						selectedBuilding->data.scan * getTileSize (), SCAN_COLOR, cVideo::buffer);
	//		}
	//	}
	//	if (rangeChecked () && (selectedBuilding->data.canAttack & TERRAIN_GROUND) && !selectedBuilding->data.explodesOnContact)
	//	{
	//		drawCircle (spx + getTileSize () / 2,
	//					spy + getTileSize () / 2,
	//					selectedBuilding->data.range * getTileSize () + 2, RANGE_GROUND_COLOR, cVideo::buffer);
	//	}
	//	if (rangeChecked () && (selectedBuilding->data.canAttack & TERRAIN_AIR))
	//	{
	//		drawCircle (spx + getTileSize () / 2,
	//					spy + getTileSize () / 2,
	//					selectedBuilding->data.range * getTileSize () + 2, RANGE_AIR_COLOR, cVideo::buffer);
	//	}

	//	if (selectedBuilding->BuildList.empty () == false &&
	//		!selectedBuilding->IsWorking &&
	//		selectedBuilding->BuildList[0].metall_remaining <= 0 &&
	//		selectedBuilding->owner == &player)
	//	{
	//		selectedBuilding->DrawExitPoints (*selectedBuilding->BuildList[0].type.getUnitDataOriginalVersion (), *this);
	//	}
	//	if (mouseInputMode == activateVehicle && selectedBuilding->owner == &player)
	//	{
	//		selectedBuilding->DrawExitPoints (selectedBuilding->storedUnits[vehicleToActivate]->data, *this);
	//	}
	//}
	//drawLockList (client->getActivePlayer ());

	//SDL_SetClipRect (cVideo::buffer, NULL);
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

	const cPosition tilePosition (std::min (x, staticMap->getSizeNew ().x ()), std::min(y, staticMap->getSizeNew ().y ()));

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

	return consumed;
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
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::PlaceBand && selectedVehicle && mouseInputMode == eNewMouseInputMode::PlaceBand) // Band
		{
			setMouseInputMode (eNewMouseInputMode::Default);

			//if (selectedVehicle->BuildingTyp.getUnitDataOriginalVersion ()->isBig)
			//{
			//	sendWantBuild (*client, selectedVehicle->iID, selectedVehicle->BuildingTyp, selectedVehicle->BuildRounds, map.getOffset (selectedVehicle->BandX, selectedVehicle->BandY), false, 0);
			//}
			//else
			//{
			//	sendWantBuild (*client, selectedVehicle->iID, selectedVehicle->BuildingTyp, selectedVehicle->BuildRounds, map.getOffset (selectedVehicle->PosX, selectedVehicle->PosY), true, map.getOffset (selectedVehicle->BandX, selectedVehicle->BandY));
			//}
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Activate && selectedUnit && mouseInputMode == eNewMouseInputMode::Activate)
		{

		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Activate && selectedBuilding && selectedBuilding->BuildList.size ())
		{

		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Load && selectedBuilding && mouseInputMode == eNewMouseInputMode::Load)
		{

		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Load && selectedVehicle && mouseInputMode == eNewMouseInputMode::Load)
		{

		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::SupplyAmmo && selectedVehicle && mouseInputMode == eNewMouseInputMode::SupplyAmmo)
		{

		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Repair && selectedVehicle && mouseInputMode == eNewMouseInputMode::Repair)
		{

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
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Attack && selectedVehicle && !selectedVehicle->attacking && !selectedVehicle->MoveJobActive)
		{

		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Attack && selectedBuilding && !selectedBuilding->attacking)
		{

		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Steal && selectedVehicle)
		{

		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Disable && selectedVehicle)
		{

		}
		else if (MouseStyle == OldSchool && mouseClickAction == eMouseClickAction::Select && unitSelection.selectUnitAt (field, false))
		{
			auto vehicle = unitSelection.getSelectedVehicle ();
			if (vehicle) vehicle->makeReport ();
		}
		else if (changeAllowed && mouseClickAction == eMouseClickAction::Move && selectedVehicle && !selectedVehicle->moving && !selectedVehicle->attacking)
		{
			if (selectedVehicle->IsBuilding)
			{
				//sendWantEndBuilding (*client, *selectedVehicle, mouseTilePosition.x (), mouseTilePosition.y ());
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
			}
		}
		else if (changeAllowed && selectedBuilding && (overBaseBuilding == selectedBuilding || overBuilding == selectedBuilding))
		{
			toggleUnitContextMenu (selectedBuilding);
		}
		else if (MouseStyle == Modern && mouseClickAction == eMouseClickAction::Select && unitSelection.selectUnitAt (field, true))
		{
			auto vehicle = unitSelection.getSelectedVehicle ();
			if (vehicle) vehicle->makeReport ();
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

	if (selectedVehicle && selectedVehicle->owner == player && mouseInputMode == eNewMouseInputMode::PlaceBand)
	{
		return eMouseClickAction::PlaceBand;
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
						selectedBuilding->IsWorking ||
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
		if (!selectedVehicle->IsBuilding && !selectedVehicle->IsClearing && mouseInputMode != eNewMouseInputMode::Load && mouseInputMode != eNewMouseInputMode::Activate)
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
		else if (selectedVehicle->IsBuilding || selectedVehicle->IsClearing)
		{
			if (((selectedVehicle->IsBuilding && selectedVehicle->BuildRounds == 0) ||
				(selectedVehicle->IsClearing && selectedVehicle->ClearingRounds == 0)) &&
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
		!selectedBuilding->IsWorking &&
		selectedBuilding->BuildList[0].metall_remaining <= 0)
	{
		if (selectedBuilding->canExitTo (mapTilePosition.x (), mapTilePosition.y (), *dynamicMap, *selectedBuilding->BuildList[0].type.getUnitDataOriginalVersion ()) && selectedUnit->isDisabled () == false)
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
void cGameMapWidget::updateMouseCursor (cMouse& mouse)
{
	if (!isAt (mouse.getPosition ())) return;

	const auto mouseClickAction = getMouseClickAction (mouse);

	auto mapTilePosition = getMapTilePosition (mouse.getPosition ());

	auto selectedVehicle = unitSelection.getSelectedVehicle ();

	switch (mouseClickAction)
	{
	case eMouseClickAction::PlaceBand:
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