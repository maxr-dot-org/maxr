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

#include "minimapwidget.h"
#include "../application.h"
#include "../../map.h"
#include "../../video.h"
#include "../../player.h"
#include "../../buildings.h"
#include "../../vehicles.h"
#include "../../input/mouse/mouse.h"

//------------------------------------------------------------------------------
cMiniMapWidget::cMiniMapWidget (const cBox<cPosition>& area, std::shared_ptr<const cStaticMap> staticMap) :
	cWidget (area),
	staticMap (std::move (staticMap)),
	dynamicMap (nullptr),
	player (nullptr),
	zoomFactor (1),
	offset (0,0),
	attackUnitsOnly (false),
	mapViewWindow (cPosition (0,0), cPosition (0,0))
{
	surface = SDL_CreateRGBSurface (0, getSize ().x (), getSize ().y (), 32, 0, 0, 0, 0);
	viewWindowSurface = SDL_CreateRGBSurface (0, getSize ().x (), getSize ().y (), 32, 0, 0, 0, 0);

	renewSurface ();
	renewViewWindowSurface ();
}

//------------------------------------------------------------------------------
void cMiniMapWidget::setDynamicMap (const cMap* dynamicMap_)
{
	dynamicMap = dynamicMap_;
}

//------------------------------------------------------------------------------
void cMiniMapWidget::setPlayer (const cPlayer* player_)
{
	player = player_;
}

//------------------------------------------------------------------------------
void cMiniMapWidget::setViewWindow (const cBox<cPosition>& mapViewWindow_)
{
	mapViewWindow = mapViewWindow_;

	renewViewWindowSurface ();

	updateOffset ();
}

//------------------------------------------------------------------------------
void cMiniMapWidget::setAttackUnitsUnly (bool attackUnitsOnly_)
{
	attackUnitsOnly = attackUnitsOnly_;

	renewSurface ();
}

//------------------------------------------------------------------------------
void cMiniMapWidget::setZoomFactor (int zoomFactor_)
{
	zoomFactor = zoomFactor_;

	if (!updateOffset ())
	{
		renewSurface ();
	}
	renewViewWindowSurface ();
}

//------------------------------------------------------------------------------
bool cMiniMapWidget::updateOffset ()
{
	auto oldOffset = offset;

	offset.x () = std::min (mapViewWindow.getMinCorner ().x (), offset.x ());
	offset.y () = std::min (mapViewWindow.getMinCorner ().y (), offset.y ());

	const cPosition displaySize = staticMap->getSizeNew () / zoomFactor;
	const cPosition endDisplay = offset + displaySize;

	offset.x () = std::max (std::max (mapViewWindow.getMaxCorner ().x (), endDisplay.x ()) - displaySize.x (), offset.x ());
	offset.y () = std::max (std::max (mapViewWindow.getMaxCorner ().y (), endDisplay.y ()) - displaySize.y (), offset.y ());

	offset.x () = std::min (staticMap->getSizeNew ().x () - displaySize.x (), offset.x ());
	offset.y () = std::min (staticMap->getSizeNew ().y () - displaySize.y (), offset.y ());

	offset.x () = std::max (0, offset.x ());
	offset.y () = std::max (0, offset.y ());

	if (oldOffset != offset)
	{
		renewSurface ();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
cPosition cMiniMapWidget::computeMapPosition (const cPosition& screenPosition)
{
	if (!isAt (screenPosition)) return cPosition (0, 0);

	const cPosition screenOffset = screenPosition - getPosition ();

	const cPosition displaySize = staticMap->getSizeNew () / zoomFactor;

	cPosition mapPosition;

	mapPosition.x () = offset.x () + (int)((double)screenOffset.x () / getSize ().x () * displaySize.x ());
	mapPosition.y () = offset.y () + (int)((double)screenOffset.y () / getSize ().y () * displaySize.y ());
	
	return mapPosition;
}

//------------------------------------------------------------------------------
void cMiniMapWidget::draw ()
{
	if (surface)
	{
		auto position = getArea ().toSdlRect ();
		SDL_BlitSurface (surface, nullptr, cVideo::buffer, &position);
	}
	if (viewWindowSurface)
	{
		auto position = getArea ().toSdlRect ();
		SDL_BlitSurface (viewWindowSurface, nullptr, cVideo::buffer, &position);
	}
	cWidget::draw ();
}

//------------------------------------------------------------------------------
bool cMiniMapWidget::handleMouseMoved (cApplication& application, cMouse& mouse)
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
}

//------------------------------------------------------------------------------
void cMiniMapWidget::drawLandscape ()
{
	auto minimap = static_cast<Uint32*> (surface->pixels);
	for (int miniMapX = 0; miniMapX < getSize ().x (); ++miniMapX)
	{
		// calculate the field on the map
		int terrainx = (miniMapX * staticMap->getSizeNew(). x ()) / (getSize ().x () * zoomFactor) + offset.x ();
		terrainx = std::min (terrainx, staticMap->getSizeNew(). x () - 1);

		// calculate the position within the terrain graphic
		// (for better rendering of maps < getSize())
		const int offsetx = ((miniMapX * staticMap->getSizeNew(). x ()) % (getSize ().x () * zoomFactor)) * 64 / (getSize ().x () * zoomFactor);

		for (int miniMapY = 0; miniMapY < getSize ().y (); ++miniMapY)
		{
			int terrainy = (miniMapY * staticMap->getSizeNew(). y ()) / (getSize ().y () * zoomFactor) + offset.y ();
			terrainy = std::min (terrainy, staticMap->getSizeNew(). y () - 1);
			const int offsety = ((miniMapY * staticMap->getSizeNew(). y ()) % (getSize ().y () * zoomFactor)) * 64 / (getSize ().y () * zoomFactor);

			const auto& terrain = staticMap->getTerrain (terrainx, terrainy);
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
		const auto terrainX = (miniMapX * staticMap->getSizeNew(). x()) / (getSize ().x () * zoomFactor) + offset.x ();
		for (int miniMapY = 0; miniMapY < getSize ().y (); ++miniMapY)
		{
			const auto terrainY = (miniMapY * staticMap->getSizeNew(). y ()) / (getSize ().y () * zoomFactor) +  offset.y ();

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
	rect.w = std::max (2, getSize ().x () * zoomFactor / staticMap->getSizeNew(). x ());
	rect.h = std::max (2, getSize ().y () * zoomFactor / staticMap->getSizeNew(). y ());

	for (int mapx = 0; mapx < staticMap->getSizeNew(). x (); ++mapx)
	{
		rect.x = ((mapx - offset.x ()) * getSize().x () * zoomFactor) / staticMap->getSizeNew(). x ();
		if (rect.x < 0 || rect.x >= getSize ().x ()) continue;
		for (int mapy = 0; mapy < staticMap->getSizeNew(). y (); ++mapy)
		{
			rect.y = ((mapy - offset.y ()) * getSize ().y () * zoomFactor) / staticMap->getSizeNew(). y ();
			if (rect.y < 0 || rect.y >= getSize ().y ()) continue;

			if (player && !player->canSeeAt (cPosition (mapx, mapy))) continue;

			const int offset = dynamicMap->getOffset (mapx, mapy);
			cMapField& field = (*dynamicMap)[offset];

			// draw building
			const cBuilding* building = field.getBuilding ();
			if (building && building->owner)
			{
				if (!attackUnitsOnly || building->data.canAttack)
				{
					unsigned int color = *static_cast<Uint32*> (building->owner->getColorSurface ()->pixels);
					SDL_FillRect (surface, &rect, color);
				}
			}

			// draw vehicle
			const cVehicle* vehicle = field.getVehicle ();
			if (vehicle)
			{
				if (!attackUnitsOnly || vehicle->data.canAttack)
				{
					unsigned int color = *static_cast<Uint32*> (vehicle->owner->getColorSurface ()->pixels);
					SDL_FillRect (surface, &rect, color);
				}
			}

			// draw plane
			vehicle = field.getPlane ();
			if (vehicle)
			{
				if (!attackUnitsOnly || vehicle->data.canAttack)
				{
					unsigned int color = *static_cast<Uint32*> (vehicle->owner->getColorSurface ()->pixels);
					SDL_FillRect (surface, &rect, color);
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
void cMiniMapWidget::renewViewWindowSurface ()
{
	SDL_FillRect (viewWindowSurface, nullptr, 0xFF00FF);
	SDL_SetColorKey (viewWindowSurface, SDL_TRUE, 0xFF00FF);

	const cPosition start = (mapViewWindow.getMinCorner () - offset) * getSize () * zoomFactor / staticMap->getSizeNew ();
	const cPosition end = (mapViewWindow.getMaxCorner () - offset) * getSize () * zoomFactor / staticMap->getSizeNew ();

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
}
