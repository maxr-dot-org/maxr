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

#include "ui/graphical/game/widgets/minimapwidget.h"

#include "SDLutility/tosdl.h"
#include "game/data/map/map.h"
#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "input/mouse/mouse.h"
#include "output/video/video.h"
#include "ui/widgets/application.h"
#include "ui/uidefines.h"

static constexpr cRgbColor neutralColor = cRgbColor::black();

//------------------------------------------------------------------------------
cMiniMapWidget::cMiniMapWidget (const cBox<cPosition>& area, std::shared_ptr<const cStaticMap> staticMap) :
	cClickableWidget (area),
	staticMap (std::move (staticMap)),
	mapView (nullptr),
	offset (0, 0),
	mapViewWindow (cPosition (0, 0), cPosition (0, 0))
{
	surface = UniqueSurface (SDL_CreateRGBSurface (0, getSize().x(), getSize().y(), 32, 0, 0, 0, 0));
	viewWindowSurface = UniqueSurface (SDL_CreateRGBSurface (0, getSize().x(), getSize().y(), 32, 0, 0, 0, 0));
}

//------------------------------------------------------------------------------
void cMiniMapWidget::setMapView (std::shared_ptr<const cMapView> mapView_)
{
	mapView = std::move (mapView_);

	dynamicMapSignalConnectionManager.disconnectAll();
	if (mapView != nullptr)
	{
		dynamicMapSignalConnectionManager.connect (mapView->unitAppeared, [this] (const cUnit&) { surfaceOutdated = true; });
		dynamicMapSignalConnectionManager.connect (mapView->unitDissappeared, [this] (const cUnit&) { surfaceOutdated = true; });
		dynamicMapSignalConnectionManager.connect (mapView->unitMoved, [this] (const cUnit&, const cPosition&) { surfaceOutdated = true; });
		dynamicMapSignalConnectionManager.connect (mapView->scanAreaChanged, [this]() { surfaceOutdated = true; });
		//TODO: update on ownerChanged?
	}

	surfaceOutdated = true;
}

//------------------------------------------------------------------------------
void cMiniMapWidget::setViewWindow (const cBox<cPosition>& mapViewWindow_)
{
	mapViewWindow = mapViewWindow_;

	viewWindowSurfaceOutdated = true;

	updateOffset();
}

//------------------------------------------------------------------------------
void cMiniMapWidget::setAttackUnitsUnly (bool attackUnitsOnly_)
{
	attackUnitsOnly = attackUnitsOnly_;

	surfaceOutdated = true;
}

//------------------------------------------------------------------------------
void cMiniMapWidget::setZoomFactor (int zoomFactor_)
{
	zoomFactor = zoomFactor_;

	if (!updateOffset())
	{
		surfaceOutdated = true;
	}
	viewWindowSurfaceOutdated = true;
}

//------------------------------------------------------------------------------
bool cMiniMapWidget::updateOffset()
{
	auto oldOffset = offset;

	offset.x() = std::min (mapViewWindow.getMinCorner().x(), offset.x());
	offset.y() = std::min (mapViewWindow.getMinCorner().y(), offset.y());

	const cPosition displaySize = staticMap->getSize() / zoomFactor;
	const cPosition endDisplay = offset + displaySize;

	offset.x() = std::max (std::max (mapViewWindow.getMaxCorner().x(), endDisplay.x()) - displaySize.x(), offset.x());
	offset.y() = std::max (std::max (mapViewWindow.getMaxCorner().y(), endDisplay.y()) - displaySize.y(), offset.y());

	offset.x() = std::min (staticMap->getSize().x() - displaySize.x(), offset.x());
	offset.y() = std::min (staticMap->getSize().y() - displaySize.y(), offset.y());

	offset.x() = std::max (0, offset.x());
	offset.y() = std::max (0, offset.y());

	if (oldOffset != offset)
	{
		surfaceOutdated = true;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
cPosition cMiniMapWidget::computeMapPosition (const cPosition& screenPosition)
{
	if (!isAt (screenPosition)) return cPosition (0, 0);

	const cPosition screenOffset = screenPosition - getPosition();

	const cPosition displaySize = staticMap->getSize() / zoomFactor;

	cPosition mapPosition;

	mapPosition.x() = offset.x() + (int) ((double) screenOffset.x() / getSize().x() * displaySize.x());
	mapPosition.y() = offset.y() + (int) ((double) screenOffset.y() / getSize().y() * displaySize.y());

	return mapPosition;
}

//------------------------------------------------------------------------------
void cMiniMapWidget::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (surfaceOutdated) renewSurface();
	if (viewWindowSurfaceOutdated) renewViewWindowSurface();

	if (surface != nullptr)
	{
		auto position = toSdlRect (getArea());
		SDL_BlitSurface (surface.get(), nullptr, &destination, &position);
	}
	if (viewWindowSurface != nullptr)
	{
		auto position = toSdlRect (getArea());
		SDL_BlitSurface (viewWindowSurface.get(), nullptr, &destination, &position);
	}
	cWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
bool cMiniMapWidget::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	if (application.hasMouseFocus (*this) && startedMoving && isAt (mouse.getPosition()))
	{
		focus (computeMapPosition (mouse.getPosition()));
		return true;
	}
	return cClickableWidget::handleMouseMoved (application, mouse, offset);
}

//------------------------------------------------------------------------------
bool cMiniMapWidget::handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left)
	{
		application.grapMouseFocus (*this);
		focus (computeMapPosition (mouse.getPosition()));
		startedMoving = true;
		return true;
	}
	return cClickableWidget::handleMousePressed (application, mouse, button);
}

//------------------------------------------------------------------------------
bool cMiniMapWidget::handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left)
	{
		application.releaseMouseFocus (*this);
		return true;
	}
	return cClickableWidget::handleMouseReleased (application, mouse, button);
}

//------------------------------------------------------------------------------
void cMiniMapWidget::handleLooseMouseFocus (cApplication& application)
{
	startedMoving = false;
	cClickableWidget::handleLooseMouseFocus (application);
}

//------------------------------------------------------------------------------
bool cMiniMapWidget::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Right)
	{
		triggeredMove (computeMapPosition (mouse.getPosition()));
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cMiniMapWidget::acceptButton (eMouseButtonType button) const
{
	return button == eMouseButtonType::Right;
}

//------------------------------------------------------------------------------
void cMiniMapWidget::renewSurface()
{
	drawLandscape();
	drawFog();
	drawUnits();

	surfaceOutdated = false;
}

//------------------------------------------------------------------------------
void cMiniMapWidget::drawLandscape()
{
	auto minimap = static_cast<Uint32*> (surface->pixels);
	for (int miniMapX = 0; miniMapX < getSize().x(); ++miniMapX)
	{
		// calculate the field on the map
		int terrainx = (miniMapX * staticMap->getSize().x()) / (getSize().x() * zoomFactor) + offset.x();
		terrainx = std::min (terrainx, staticMap->getSize().x() - 1);

		// calculate the position within the terrain graphic
		// (for better rendering of maps < getSize())
		const int offsetx = ((miniMapX * staticMap->getSize().x()) % (getSize().x() * zoomFactor)) * 64 / (getSize().x() * zoomFactor);

		for (int miniMapY = 0; miniMapY < getSize().y(); ++miniMapY)
		{
			int terrainy = (miniMapY * staticMap->getSize().y()) / (getSize().y() * zoomFactor) + offset.y();
			terrainy = std::min (terrainy, staticMap->getSize().y() - 1);
			const int offsety = ((miniMapY * staticMap->getSize().y()) % (getSize().y() * zoomFactor)) * 64 / (getSize().y() * zoomFactor);

			const auto& terrain = staticMap->getGraphic().getTile (staticMap->getTileIndex (cPosition (terrainx, terrainy)));
			const auto* terrainPixels = reinterpret_cast<const Uint8*> (terrain.sf_org->pixels);
			const auto index = terrainPixels[offsetx + offsety * 64];
			const auto sdlcolor = terrain.sf_org->format->palette->colors[index];
			const auto color = (sdlcolor.r << 16) + (sdlcolor.g << 8) + sdlcolor.b;

			minimap[miniMapX + miniMapY * getSize().x()] = color;
		}
	}
}

//------------------------------------------------------------------------------
void cMiniMapWidget::drawFog()
{
	if (!mapView) return;

	auto minimap = static_cast<Uint32*> (surface->pixels);

	for (int miniMapX = 0; miniMapX < getSize().x(); ++miniMapX)
	{
		const auto terrainX = (miniMapX * staticMap->getSize().x()) / (getSize().x() * zoomFactor) + offset.x();
		for (int miniMapY = 0; miniMapY < getSize().y(); ++miniMapY)
		{
			const auto terrainY = (miniMapY * staticMap->getSize().y()) / (getSize().y() * zoomFactor) + offset.y();

			if (mapView->isPositionVisible (cPosition (terrainX, terrainY))) continue;

			Uint8* color = reinterpret_cast<Uint8*> (&minimap[miniMapX + miniMapY * getSize().x()]);
			color[0] = static_cast<Uint8> (color[0] * 0.6f);
			color[1] = static_cast<Uint8> (color[1] * 0.6f);
			color[2] = static_cast<Uint8> (color[2] * 0.6f);
		}
	}
}

//------------------------------------------------------------------------------
void cMiniMapWidget::drawUnits()
{
	if (!mapView) return;

	// here we go through each map field instead of
	// through each minimap pixel, to make sure,
	// that every unit is displayed and has the same size on the minimap.

	// the size of the rect, that is drawn for each unit
	SDL_Rect rect;
	rect.w = std::max (2, getSize().x() * zoomFactor / staticMap->getSize().x());
	rect.h = std::max (2, getSize().y() * zoomFactor / staticMap->getSize().y());

	for (int mapx = 0; mapx < staticMap->getSize().x(); ++mapx)
	{
		rect.x = ((mapx - offset.x()) * getSize().x() * zoomFactor) / staticMap->getSize().x();

		if (rect.x < 0 || rect.x >= getSize().x()) continue;

		for (int mapy = 0; mapy < staticMap->getSize().y(); ++mapy)
		{
			const cPosition mapPosition (mapx, mapy);

			rect.y = ((mapy - offset.y()) * getSize().y() * zoomFactor) / staticMap->getSize().y();
			if (rect.y < 0 || rect.y >= getSize().y()) continue;

			if (!mapView->isPositionVisible (mapPosition)) continue;

			const cMapFieldView& field = mapView->getField (mapPosition);

			// draw building
			if (const cBuilding* building = field.getBuilding())
			{
				if (!attackUnitsOnly || building->getStaticUnitData().canAttack)
				{
					const auto color = building->getOwner() ? building->getOwner()->getColor() : neutralColor;
					SDL_FillRect (surface.get(), &rect, toSdlAlphaColor (color, *surface));
				}
			}

			// draw vehicle
			if (const cVehicle* vehicle = field.getVehicle())
			{
				if (!attackUnitsOnly || vehicle->getStaticUnitData().canAttack)
				{
					const auto color = vehicle->getOwner() ? vehicle->getOwner()->getColor() : neutralColor;
					SDL_FillRect (surface.get(), &rect, toSdlAlphaColor (color, *surface));
				}
			}

			// draw plane
			if (const cVehicle* vehicle = field.getPlane())
			{
				if (!attackUnitsOnly || vehicle->getStaticUnitData().canAttack)
				{
					const auto color = vehicle->getOwner() ? vehicle->getOwner()->getColor() : neutralColor;
					SDL_FillRect (surface.get(), &rect, toSdlAlphaColor (color, *surface));
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
void cMiniMapWidget::renewViewWindowSurface()
{
	SDL_FillRect (viewWindowSurface.get(), nullptr, 0xFF00FF);
	SDL_SetColorKey (viewWindowSurface.get(), SDL_TRUE, 0xFF00FF);

	const cPosition start = (mapViewWindow.getMinCorner() - offset) * getSize() * zoomFactor / staticMap->getSize();
	const cPosition end = (mapViewWindow.getMaxCorner() - offset) * getSize() * zoomFactor / staticMap->getSize();

	Uint32* minimap = static_cast<Uint32*> (viewWindowSurface->pixels);
	for (int y = start.y(); y <= end.y(); ++y)
	{
		if (y < 0 || y >= getSize().y()) continue;
		if (start.x() >= 0 && start.x() < getSize().x())
		{
			minimap[y * getSize().x() + start.x()] = MINIMAP_COLOR;
		}
		if (end.x() >= 0 && end.x() < getSize().x())
		{
			minimap[y * getSize().x() + end.x()] = MINIMAP_COLOR;
		}
	}
	for (int x = start.x(); x <= end.x(); ++x)
	{
		if (x < 0 || x >= getSize().x()) continue;
		if (start.y() >= 0 && start.y() < getSize().y())
		{
			minimap[start.y() * getSize().x() + x] = MINIMAP_COLOR;
		}
		if (end.y() >= 0 && end.y() < getSize().y())
		{
			minimap[end.y() * getSize().x() + x] = MINIMAP_COLOR;
		}
	}
	viewWindowSurfaceOutdated = false;
}
