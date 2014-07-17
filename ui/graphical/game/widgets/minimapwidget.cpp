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
#include "ui/graphical/application.h"
#include "map.h"
#include "video.h"
#include "game/data/player/player.h"
#include "buildings.h"
#include "vehicles.h"
#include "input/mouse/mouse.h"

//------------------------------------------------------------------------------
cMiniMapWidget::cMiniMapWidget (const cBox<cPosition>& area, std::shared_ptr<const cStaticMap> staticMap) :
	cWidget (area),
	surfaceOutdated (true),
	viewWindowSurfaeOutdated (true),
	staticMap (std::move (staticMap)),
	dynamicMap (nullptr),
	player (nullptr),
	zoomFactor (1),
	offset (0,0),
	attackUnitsOnly (false),
	mapViewWindow (cPosition (0,0), cPosition (0,0))
{
    surface = AutoSurface (SDL_CreateRGBSurface (0, getSize ().x (), getSize ().y (), 32, 0, 0, 0, 0));
    viewWindowSurface = AutoSurface (SDL_CreateRGBSurface (0, getSize ().x (), getSize ().y (), 32, 0, 0, 0, 0));
}

//------------------------------------------------------------------------------
void cMiniMapWidget::setDynamicMap (std::shared_ptr<const cMap> dynamicMap_)
{
	dynamicMap = std::move(dynamicMap_);

	dynamicMapSignalConnectionManager.disconnectAll ();

	if (dynamicMap != nullptr)
	{
		dynamicMapSignalConnectionManager.connect (dynamicMap->addedUnit, [&](const cUnit&){ surfaceOutdated = true; });
		dynamicMapSignalConnectionManager.connect (dynamicMap->removedUnit, [&](const cUnit&){ surfaceOutdated = true; });
		dynamicMapSignalConnectionManager.connect (dynamicMap->movedVehicle, [&](const cVehicle&){ surfaceOutdated = true; });
	}
}

//------------------------------------------------------------------------------
void cMiniMapWidget::setPlayer (std::shared_ptr<const cPlayer> player_)
{
	player = std::move(player_);
}

//------------------------------------------------------------------------------
void cMiniMapWidget::setViewWindow (const cBox<cPosition>& mapViewWindow_)
{
	mapViewWindow = mapViewWindow_;

	viewWindowSurfaeOutdated = true;

	updateOffset ();
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

	if (!updateOffset ())
	{
		surfaceOutdated = true;
	}
	viewWindowSurfaeOutdated = true;
}

//------------------------------------------------------------------------------
bool cMiniMapWidget::updateOffset ()
{
	auto oldOffset = offset;

	offset.x () = std::min (mapViewWindow.getMinCorner ().x (), offset.x ());
	offset.y () = std::min (mapViewWindow.getMinCorner ().y (), offset.y ());

	const cPosition displaySize = staticMap->getSize () / zoomFactor;
	const cPosition endDisplay = offset + displaySize;

	offset.x () = std::max (std::max (mapViewWindow.getMaxCorner ().x (), endDisplay.x ()) - displaySize.x (), offset.x ());
	offset.y () = std::max (std::max (mapViewWindow.getMaxCorner ().y (), endDisplay.y ()) - displaySize.y (), offset.y ());

	offset.x () = std::min (staticMap->getSize ().x () - displaySize.x (), offset.x ());
	offset.y () = std::min (staticMap->getSize ().y () - displaySize.y (), offset.y ());

	offset.x () = std::max (0, offset.x ());
	offset.y () = std::max (0, offset.y ());

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

	const cPosition screenOffset = screenPosition - getPosition ();

	const cPosition displaySize = staticMap->getSize () / zoomFactor;

	cPosition mapPosition;

	mapPosition.x () = offset.x () + (int)((double)screenOffset.x () / getSize ().x () * displaySize.x ());
	mapPosition.y () = offset.y () + (int)((double)screenOffset.y () / getSize ().y () * displaySize.y ());
	
	return mapPosition;
}

//------------------------------------------------------------------------------
void cMiniMapWidget::draw ()
{
	if(surfaceOutdated) renewSurface ();
	if(viewWindowSurfaeOutdated) renewViewWindowSurface ();

	if (surface != nullptr)
	{
		auto position = getArea ().toSdlRect ();
		SDL_BlitSurface (surface.get (), nullptr, cVideo::buffer, &position);
	}
	if (viewWindowSurface != nullptr)
	{
		auto position = getArea ().toSdlRect ();
		SDL_BlitSurface (viewWindowSurface.get (), nullptr, cVideo::buffer, &position);
	}
	cWidget::draw ();
}

//------------------------------------------------------------------------------
bool cMiniMapWidget::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	if (application.hasMouseFocus (*this) && isAt (mouse.getPosition ()))
	{
		focus (computeMapPosition (mouse.getPosition ()));
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cMiniMapWidget::handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left)
	{
		application.grapMouseFocus (*this);
		focus (computeMapPosition (mouse.getPosition ()));
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cMiniMapWidget::handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left)
	{
		application.releaseMouseFocus (*this);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void cMiniMapWidget::renewSurface ()
{
	drawLandscape ();
	drawFog ();
	drawUnits ();

	surfaceOutdated = false;
}

//------------------------------------------------------------------------------
void cMiniMapWidget::drawLandscape ()
{
	auto minimap = static_cast<Uint32*> (surface->pixels);
	for (int miniMapX = 0; miniMapX < getSize ().x (); ++miniMapX)
	{
		// calculate the field on the map
		int terrainx = (miniMapX * staticMap->getSize(). x ()) / (getSize ().x () * zoomFactor) + offset.x ();
		terrainx = std::min (terrainx, staticMap->getSize(). x () - 1);

		// calculate the position within the terrain graphic
		// (for better rendering of maps < getSize())
		const int offsetx = ((miniMapX * staticMap->getSize(). x ()) % (getSize ().x () * zoomFactor)) * 64 / (getSize ().x () * zoomFactor);

		for (int miniMapY = 0; miniMapY < getSize ().y (); ++miniMapY)
		{
			int terrainy = (miniMapY * staticMap->getSize(). y ()) / (getSize ().y () * zoomFactor) + offset.y ();
			terrainy = std::min (terrainy, staticMap->getSize(). y () - 1);
			const int offsety = ((miniMapY * staticMap->getSize(). y ()) % (getSize ().y () * zoomFactor)) * 64 / (getSize ().y () * zoomFactor);

			const auto& terrain = staticMap->getTerrain (cPosition(terrainx, terrainy));
			const auto* terrainPixels = reinterpret_cast<const Uint8*> (terrain.sf_org->pixels);
			const auto index = terrainPixels[offsetx + offsety * 64];
			const auto sdlcolor = terrain.sf_org->format->palette->colors[index];
			const auto color = (sdlcolor.r << 16) + (sdlcolor.g << 8) + sdlcolor.b;

			minimap[miniMapX + miniMapY * getSize ().x ()] = color;
		}
	}
}

//------------------------------------------------------------------------------
void cMiniMapWidget::drawFog ()
{
	if (!player) return;

	auto minimap = static_cast<Uint32*> (surface->pixels);

	for (int miniMapX = 0; miniMapX < getSize ().x (); ++miniMapX)
	{
		const auto terrainX = (miniMapX * staticMap->getSize(). x()) / (getSize ().x () * zoomFactor) + offset.x ();
		for (int miniMapY = 0; miniMapY < getSize ().y (); ++miniMapY)
		{
			const auto terrainY = (miniMapY * staticMap->getSize(). y ()) / (getSize ().y () * zoomFactor) +  offset.y ();

			if (player->canSeeAt (cPosition (terrainX, terrainY))) continue;

			Uint8* color = reinterpret_cast<Uint8*> (&minimap[miniMapX + miniMapY * getSize ().x ()]);
			color[0] = static_cast<Uint8> (color[0] * 0.6f);
			color[1] = static_cast<Uint8> (color[1] * 0.6f);
			color[2] = static_cast<Uint8> (color[2] * 0.6f);
		}
	}
}

//------------------------------------------------------------------------------
void cMiniMapWidget::drawUnits ()
{
	if (!dynamicMap) return;


	// here we go through each map field instead of
	// through each minimap pixel, to make sure,
	// that every unit is displayed and has the same size on the minimap.

	// the size of the rect, that is drawn for each unit
	SDL_Rect rect;
	rect.w = std::max (2, getSize ().x () * zoomFactor / staticMap->getSize(). x ());
	rect.h = std::max (2, getSize ().y () * zoomFactor / staticMap->getSize(). y ());

	for (int mapx = 0; mapx < staticMap->getSize(). x (); ++mapx)
	{
		rect.x = ((mapx - offset.x ()) * getSize().x () * zoomFactor) / staticMap->getSize(). x ();

		if (rect.x < 0 || rect.x >= getSize ().x ()) continue;

		for (int mapy = 0; mapy < staticMap->getSize(). y (); ++mapy)
		{
			const cPosition mapPosition (mapx, mapy);

			rect.y = ((mapy - offset.y ()) * getSize ().y () * zoomFactor) / staticMap->getSize(). y ();
			if (rect.y < 0 || rect.y >= getSize ().y ()) continue;

			if (player && !player->canSeeAt (mapPosition)) continue;

			const cMapField& field = dynamicMap->getField (mapPosition);

			// draw building
			const cBuilding* building = field.getBuilding ();
			if (building && building->owner)
			{
				if (!attackUnitsOnly || building->data.canAttack)
				{
					SDL_FillRect (surface.get (), &rect, building->owner->getColor ().getColor ().toMappedSdlRGBAColor (surface->format));
				}
			}

			// draw vehicle
			const cVehicle* vehicle = field.getVehicle ();
			if (vehicle)
			{
				if (!attackUnitsOnly || vehicle->data.canAttack)
				{
					SDL_FillRect (surface.get (), &rect, vehicle->owner->getColor ().getColor ().toMappedSdlRGBAColor (surface->format));
				}
			}

			// draw plane
			vehicle = field.getPlane ();
			if (vehicle)
			{
				if (!attackUnitsOnly || vehicle->data.canAttack)
				{
					SDL_FillRect (surface.get (), &rect, vehicle->owner->getColor ().getColor ().toMappedSdlRGBAColor (surface->format));
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
void cMiniMapWidget::renewViewWindowSurface ()
{
	SDL_FillRect (viewWindowSurface.get (), nullptr, 0xFF00FF);
	SDL_SetColorKey (viewWindowSurface.get (), SDL_TRUE, 0xFF00FF);

	const cPosition start = (mapViewWindow.getMinCorner () - offset) * getSize () * zoomFactor / staticMap->getSize ();
	const cPosition end = (mapViewWindow.getMaxCorner () - offset) * getSize () * zoomFactor / staticMap->getSize ();

	Uint32* minimap = static_cast<Uint32*> (viewWindowSurface->pixels);
	for (int y = start.y (); y <= end.y (); ++y)
	{
		if (y < 0 || y >= getSize ().y ()) continue;
		if (start.x () >= 0 && start.x () < MINIMAP_SIZE)
		{
			minimap[y * getSize ().x () + start.x ()] = MINIMAP_COLOR;
		}
		if (end.x () >= 0 && end.x () < MINIMAP_SIZE)
		{
			minimap[y * getSize ().x () + end.x ()] = MINIMAP_COLOR;
		}
	}
	for (int x = start.x (); x <= end.x (); ++x)
	{
		if (x < 0 || x >= getSize ().x ()) continue;
		if (start.y () >= 0 && start.y () < getSize ().y ())
		{
			minimap[start.y () * getSize ().x () + x] = MINIMAP_COLOR;
		}
		if (end.y () >= 0 && end.y () < getSize ().y ())
		{
			minimap[end.y () * getSize ().x () + x] = MINIMAP_COLOR;
		}
	}
	viewWindowSurfaeOutdated = false;
}
