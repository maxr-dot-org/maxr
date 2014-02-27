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
#include "hud.h"
#include "../../map.h"
#include "../../settings.h"
#include "../../video.h"
#include "../../main.h"
#include "../../player.h"
#include "../../vehicles.h"
#include "../../buildings.h"
#include "../../utility/indexiterator.h"
#include "../../input/mouse/mouse.h"

//------------------------------------------------------------------------------
cGameMapWidget::cGameMapWidget (const cBox<cPosition>& area, std::shared_ptr<const cStaticMap> staticMap_) :
	cClickableWidget (area),
	staticMap (std::move (staticMap_)),
	dynamicMap (nullptr),
	player (nullptr),
	pixelOffset (0, 0),
	internalZoomFactor (1.),
	lastMouseOverTilePosition (-1, -1),
	shouldDrawSurvey (false),
	shouldDrawScan (false),
	shouldDrawGrid (false),
	shouldDrawRange (false),
	shouldDrawFog (false)
{
	assert (staticMap != nullptr);
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
	unitDrawingEngine.handleNewFrame ();

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

	//cVehicle* selectedVehicle = getSelectedVehicle ();
	if (shouldDrawSurvey/* || (selectedVehicle && selectedVehicle->owner == &client->getActivePlayer () && selectedVehicle->data.canSurvey)*/)
	{
		drawResources ();
	}

	//if (selectedVehicle && ((selectedVehicle->ClientMoveJob && selectedVehicle->ClientMoveJob->bSuspended) || selectedVehicle->BuildPath))
	//{
	//	selectedVehicle->DrawPath (*this);
	//}

	//debugOutput.draw ();

	//drawSelectionBox (zoomOffX, zoomOffY);

	//SDL_SetClipRect (cVideo::buffer, nullptr);

	drawUnitCircles ();

	//if (selectedUnit && unitMenuActive) drawMenu (*selectedUnit);

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

		// calling scroll here even if scrollOffset = (0,0) is important
		// because this will re validate the offset.
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
					unitDrawingEngine.drawUnit (building, drawDestination, getZoomFactor(), player);
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
		unitDrawingEngine.drawUnit (*building, drawDestination, getZoomFactor (), player);

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
			unitDrawingEngine.drawUnit (*vehicle, drawDestination, getZoomFactor (), *dynamicMap, player);
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
				unitDrawingEngine.drawUnit (building, drawDestination, getZoomFactor (), player);
			}
		}
		for (auto it = buildings.begin (); it != buildings.end (); ++it)
		{
			const auto& building = *(*it);
			if ((*it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)
			{
				unitDrawingEngine.drawUnit (building, drawDestination, getZoomFactor (), player);
			}
		}

		auto vehicle = mapField.getVehicle();
		if (vehicle && (vehicle->IsClearing || vehicle->IsBuilding) && (player && player->canSeeAnyAreaUnder (*vehicle)))
		{
			// make sure a big vehicle is drawn only once
			// TODO: BUG: when PosX,PosY is outside of drawing screen
			if (vehicle->PosX == i->x() && vehicle->PosY == i->y())
			{
				unitDrawingEngine.drawUnit (*vehicle, drawDestination, getZoomFactor (), *dynamicMap, player);
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
			unitDrawingEngine.drawUnit (*vehicle, drawDestination, getZoomFactor (), *dynamicMap, player);
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
			unitDrawingEngine.drawUnit (*building, drawDestination, getZoomFactor (), player);
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
			unitDrawingEngine.drawUnit (plane, drawDestination, getZoomFactor (), *dynamicMap, player);
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
cPosition cGameMapWidget::getMapTilePosition (const cPosition& pixelPosition)
{
	assert (getArea ().withinOrTouches (pixelPosition));

	const auto zoomedTileSize = getZoomedTileSize ();

	const auto x = (int)((pixelPosition.x () - getPosition ().x () + pixelOffset.x () * getZoomFactor ()) / zoomedTileSize.x ());
	const auto y = (int)((pixelPosition.y () - getPosition ().y () + pixelOffset.y () * getZoomFactor ()) / zoomedTileSize.y ());

	const cPosition tilePosition (std::min (x, staticMap->getSizeNew ().x ()), std::min(y, staticMap->getSizeNew ().y ()));

	return tilePosition;
}

//------------------------------------------------------------------------------
bool cGameMapWidget::handleMouseMoved (cApplication& application, cMouse& mouse)
{
	auto consumed = cClickableWidget::handleMouseMoved (application, mouse);

	if (getArea ().withinOrTouches (mouse.getPosition ()))
	{
		const auto tilePosition = getMapTilePosition (mouse.getPosition ());
		if (tilePosition != lastMouseOverTilePosition)
		{
			tileUnderMouseChanged (tilePosition);
			lastMouseOverTilePosition = tilePosition;
		}
	}

	return consumed;
}

//------------------------------------------------------------------------------
bool cGameMapWidget::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	const auto tilePosition = getMapTilePosition (mouse.getPosition());
	tileClicked (tilePosition);
	return true;
}
