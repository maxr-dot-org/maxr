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

#include "landingpositionselectionmap.h"

#include "../../../../map.h"
#include "../../../../settings.h"
#include "../../../../video.h"
#include "../../../../input/mouse/mouse.h"
#include "../../../../input/mouse/cursor/mousecursorsimple.h"

//------------------------------------------------------------------------------
cLandingPositionSelectionMap::cLandingPositionSelectionMap (const cBox<cPosition>& area, std::shared_ptr<cStaticMap> map_) :
	cClickableWidget (area),
	map (std::move (map_))
{
	mapSurface = map->createBigSurface (getSize ().x (), getSize ().y ());
}

//------------------------------------------------------------------------------
void cLandingPositionSelectionMap::draw ()
{
	if (mapSurface != nullptr)
	{
		SDL_Rect position = getArea ().toSdlRect ();
		SDL_BlitSurface (mapSurface.get (), nullptr, cVideo::buffer, &position);
	}

	cClickableWidget::draw ();
}

//------------------------------------------------------------------------------
bool cLandingPositionSelectionMap::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	cPosition tilePosition;
	auto mapTile = getMapTile (mouse.getPosition (), tilePosition);

	if (mapTile && isAllowedTerrain (*mapTile))
	{
		clickedTile (tilePosition);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cLandingPositionSelectionMap::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	cClickableWidget::handleMouseMoved (application, mouse, offset);

	cPosition tilePosition;
	auto mapTile = getMapTile (mouse.getPosition (), tilePosition);

	if (mapTile)
	{
		if (isAllowedTerrain (*mapTile)) mouse.setCursor (std::make_unique<cMouseCursorSimple>(eMouseCursorSimpleType::Move));
		else mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::No));
	}
	else
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
	}
	return true;
}

//------------------------------------------------------------------------------
const sTerrain* cLandingPositionSelectionMap::getMapTile (const cPosition& position, cPosition& tilePosition)
{
	tilePosition = (position - getPosition ()) * map->getSize () / getSize ();

	return &map->getTerrain (tilePosition);
}

//------------------------------------------------------------------------------
bool cLandingPositionSelectionMap::isAllowedTerrain (const sTerrain& terrain)
{
	return !terrain.water && !terrain.coast && !terrain.blocked;
}
