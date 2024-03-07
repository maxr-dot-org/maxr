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

#include "ui/graphical/game/control/mousemode/mousemodeselectbuildposition.h"

#include "game/data/gui/unitselection.h"
#include "game/data/map/mapview.h"
#include "game/data/units/unit.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "input/mouse/mouse.h"
#include "ui/graphical/game/control/mouseaction/mouseactionselectbuildposition.h"

//------------------------------------------------------------------------------
cMouseModeSelectBuildPosition::cMouseModeSelectBuildPosition (const cMapView* map_, const cUnitSelection& unitSelection_, const cPlayer* player_, sID buildId_) :
	cMouseMode (map_, unitSelection_, player_),
	buildId (buildId_)
{}

//------------------------------------------------------------------------------
eMouseModeType cMouseModeSelectBuildPosition::getType() const
{
	return eMouseModeType::SelectBuildPosition;
}

//------------------------------------------------------------------------------
void cMouseModeSelectBuildPosition::setCursor (cMouse& mouse, const cPosition&, const cUnitsData&) const /* override */
{
	mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Band));
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseAction> cMouseModeSelectBuildPosition::getMouseAction (const cPosition& mapPosition, const cUnitsData& unitsData) const
{
	const auto selectedUnit = unitSelection.getSelectedUnit();

	if (!selectedUnit) return nullptr;

	auto destination = findNextBuildPosition (selectedUnit->getPosition(), mapPosition, unitsData);
	if (!destination) return nullptr;

	return std::make_unique<cMouseActionSelectBuildPosition> (buildId, *destination);
}

//------------------------------------------------------------------------------
std::optional<cPosition> cMouseModeSelectBuildPosition::findNextBuildPosition (const cPosition& sourcePosition, const cPosition& desiredPosition, const cUnitsData& unitsData) const
{
	if (!map) return std::nullopt;

	bool pos[4] = {false, false, false, false};

	//check, which positions are available
	const auto& unitData = unitsData.getStaticUnitData (buildId);
	if (map->possiblePlaceBuilding (unitData, cPosition (sourcePosition.x() - 1, sourcePosition.y() - 1))
	    && map->possiblePlaceBuilding (unitData, cPosition (sourcePosition.x(), sourcePosition.y() - 1))
	    && map->possiblePlaceBuilding (unitData, cPosition (sourcePosition.x() - 1, sourcePosition.y())))
	{
		pos[0] = true;
	}

	if (map->possiblePlaceBuilding (unitData, cPosition (sourcePosition.x(), sourcePosition.y() - 1))
	    && map->possiblePlaceBuilding (unitData, cPosition (sourcePosition.x() + 1, sourcePosition.y() - 1))
	    && map->possiblePlaceBuilding (unitData, cPosition (sourcePosition.x() + 1, sourcePosition.y())))
	{
		pos[1] = true;
	}

	if (map->possiblePlaceBuilding (unitData, cPosition (sourcePosition.x() + 1, sourcePosition.y()))
	    && map->possiblePlaceBuilding (unitData, cPosition (sourcePosition.x() + 1, sourcePosition.y() + 1))
	    && map->possiblePlaceBuilding (unitData, cPosition (sourcePosition.x(), sourcePosition.y() + 1)))
	{
		pos[2] = true;
	}

	if (map->possiblePlaceBuilding (unitData, cPosition (sourcePosition.x() - 1, sourcePosition.y()))
	    && map->possiblePlaceBuilding (unitData, cPosition (sourcePosition.x() - 1, sourcePosition.y() + 1))
	    && map->possiblePlaceBuilding (unitData, cPosition (sourcePosition.x(), sourcePosition.y() + 1)))
	{
		pos[3] = true;
	}

	// chose the position, which matches the cursor position, if available
	if (desiredPosition.x() <= sourcePosition.x() && desiredPosition.y() <= sourcePosition.y() && pos[0])
	{
		return cPosition (sourcePosition.x() - 1, sourcePosition.y() - 1);
	}

	if (desiredPosition.x() >= sourcePosition.x() && desiredPosition.y() <= sourcePosition.y() && pos[1])
	{
		return cPosition (sourcePosition.x(), sourcePosition.y() - 1);
	}

	if (desiredPosition.x() >= sourcePosition.x() && desiredPosition.y() >= sourcePosition.y() && pos[2])
	{
		return sourcePosition;
	}

	if (desiredPosition.x() <= sourcePosition.x() && desiredPosition.y() >= sourcePosition.y() && pos[3])
	{
		return cPosition (sourcePosition.x() - 1, sourcePosition.y());
	}

	// if the best position is not available, chose the next free one
	if (pos[0])
	{
		return cPosition (sourcePosition.x() - 1, sourcePosition.y() - 1);
	}

	if (pos[1])
	{
		return cPosition (sourcePosition.x(), sourcePosition.y() - 1);
	}

	if (pos[2])
	{
		return sourcePosition;
	}

	if (pos[3])
	{
		return cPosition (sourcePosition.x() - 1, sourcePosition.y());
	}

	return std::nullopt;
}
