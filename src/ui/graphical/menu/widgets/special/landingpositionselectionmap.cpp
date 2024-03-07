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

#include "ui/graphical/menu/widgets/special/landingpositionselectionmap.h"

#include "SDLutility/tosdl.h"
#include "game/data/map/map.h"
#include "game/data/units/landingunit.h"
#include "game/data/units/unitdata.h"
#include "game/logic/action/actioninitnewgame.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "input/mouse/mouse.h"
#include "output/video/video.h"

//------------------------------------------------------------------------------
cLandingPositionSelectionMap::cLandingPositionSelectionMap (const cBox<cPosition>& area, std::shared_ptr<cStaticMap> map_, bool fixedBridgeHead, const std::vector<sLandingUnit>& landingUnits, std::shared_ptr<const cUnitsData> unitsData) :
	cClickableWidget (area),
	map (std::move (map_)),
	fixedBridgeHead (fixedBridgeHead),
	landingUnits (landingUnits),
	unitsData (unitsData)
{
	mapSurface = map->getGraphic().createBigSurface (getSize().x(), getSize().y());
}

//------------------------------------------------------------------------------
void cLandingPositionSelectionMap::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (mapSurface != nullptr)
	{
		SDL_Rect position = toSdlRect (getArea());
		SDL_BlitSurface (mapSurface.get(), nullptr, &destination, &position);
	}

	cClickableWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
bool cLandingPositionSelectionMap::handleClicked (cApplication&, cMouse& mouse, eMouseButtonType) /* override */
{
	cPosition mapPosition = (mouse.getPosition() - getPosition()) * map->getSize() / getSize();

	if (isValidLandingLocation (mapPosition))
	{
		clickedTile (mapPosition);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cLandingPositionSelectionMap::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	cClickableWidget::handleMouseMoved (application, mouse, offset);

	cPosition mapPosition = (mouse.getPosition() - getPosition()) * map->getSize() / getSize();

	if (isValidLandingLocation (mapPosition))
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Move));
	else
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::No));

	return true;
}

//------------------------------------------------------------------------------
bool cLandingPositionSelectionMap::isValidLandingLocation (const cPosition& position)
{
	return cActionInitNewGame::isValidLandingPosition (position, *map, fixedBridgeHead, landingUnits, *unitsData);
}
