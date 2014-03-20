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

#include "mousemodeselectbuildposition.h"
#include "../mouseaction/mouseactionselectbuildposition.h"
#include "../../unitselection.h"
#include "../../../../map.h"
#include "../../../../unit.h"
#include "../../../../input/mouse/mouse.h"
#include "../../../../input/mouse/cursor/mousecursorsimple.h"

//------------------------------------------------------------------------------
eMouseModeType cMouseModeSelectBuildPosition::getType () const
{
	return eMouseModeType::SelectBuildPosition;
}

//------------------------------------------------------------------------------
cMouseModeSelectBuildPosition::cMouseModeSelectBuildPosition (sID buildId_) :
	buildId (buildId_)
{}

//------------------------------------------------------------------------------
void cMouseModeSelectBuildPosition::setCursor (cMouse& mouse, const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection, const cPlayer* player) const
{
	mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Band));
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseAction> cMouseModeSelectBuildPosition::getMouseAction (const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection, const cPlayer* player) const
{
	const auto selectedUnit = unitSelection.getSelectedUnit ();

	if (!selectedUnit) return nullptr;

	bool validPosition;
	cPosition destination;
	std::tie (validPosition, destination) = findNextBuildPosition (map, cPosition (selectedUnit->PosX, selectedUnit->PosY), mapPosition);
	if (!validPosition) return nullptr;

	return std::make_unique<cMouseActionSelectBuildPosition> (buildId, destination);
}

//------------------------------------------------------------------------------
std::pair<bool, cPosition> cMouseModeSelectBuildPosition::findNextBuildPosition (const cMap& map, const cPosition& sourcePosition, const cPosition& desiredPosition) const
{
	bool pos[4] = {false, false, false, false};

	//check, which positions are available
	const auto& unitData = *buildId.getUnitDataOriginalVersion ();
	if (map.possiblePlaceBuilding (unitData, sourcePosition.x () - 1, sourcePosition.y () - 1)
		&& map.possiblePlaceBuilding (unitData, sourcePosition.x (), sourcePosition.y () - 1)
		&& map.possiblePlaceBuilding (unitData, sourcePosition.x () - 1, sourcePosition.y ()))
	{
		pos[0] = true;
	}

	if (map.possiblePlaceBuilding (unitData, sourcePosition.x (), sourcePosition.y () - 1)
		&& map.possiblePlaceBuilding (unitData, sourcePosition.x () + 1, sourcePosition.y () - 1)
		&& map.possiblePlaceBuilding (unitData, sourcePosition.x () + 1, sourcePosition.y ()))
	{
		pos[1] = true;
	}

	if (map.possiblePlaceBuilding (unitData, sourcePosition.x () + 1, sourcePosition.y ())
		&& map.possiblePlaceBuilding (unitData, sourcePosition.x () + 1, sourcePosition.y () + 1)
		&& map.possiblePlaceBuilding (unitData, sourcePosition.x (), sourcePosition.y () + 1))
	{
		pos[2] = true;
	}

	if (map.possiblePlaceBuilding (unitData, sourcePosition.x () - 1, sourcePosition.y ())
		&& map.possiblePlaceBuilding (unitData, sourcePosition.x () - 1, sourcePosition.y () + 1)
		&& map.possiblePlaceBuilding (unitData, sourcePosition.x (), sourcePosition.y () + 1))
	{
		pos[3] = true;
	}

	// chose the position, which matches the cursor position, if available
	if (desiredPosition.x () <= sourcePosition.x () && desiredPosition.y () <= sourcePosition.y () && pos[0])
	{
		return std::make_pair (true, cPosition (sourcePosition.x () - 1, sourcePosition.y () - 1));
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
