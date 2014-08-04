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

#include "ui/graphical/game/widgets/mouseaction/mouseactionselect.h"
#include "ui/graphical/game/widgets/gamemapwidget.h"
#include "ui/graphical/game/unitselection.h"
#include "game/data/map/map.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"
#include "keys.h"
#include "utility/listhelpers.h"
#include "input/mouse/mouse.h"

//------------------------------------------------------------------------------
bool cMouseActionSelect::executeLeftClick (cGameMapWidget& gameMapWidget, const cMap& map, const cPosition& mapPosition, cUnitSelection& unitSelection) const
{
	const auto selectedVehicle = unitSelection.getSelectedVehicle ();
	const auto selectedBuilding = unitSelection.getSelectedBuilding ();

	const auto& field = map.getField (mapPosition);

	const auto overVehicle = field.getVehicle ();
	const auto overBuilding = field.getBuilding ();
	const auto overBaseBuilding = field.getBaseBuilding ();

	const bool changeAllowed = true; // FIXME: parameter?!

	if (KeysList.getMouseStyle () == eMouseStyle::OldSchool && unitSelection.selectUnitAt (field, false))
	{
		/*do nothing here*/
	}
	else if (KeysList.getMouseStyle () == eMouseStyle::Modern &&
		(
			!changeAllowed ||
			(
				(!selectedVehicle || (!Contains (field.getPlanes (), selectedVehicle) && selectedVehicle != overVehicle)) &&
				(!selectedBuilding || (overBaseBuilding != selectedBuilding && overBuilding != selectedBuilding))
			)
		) &&
		unitSelection.selectUnitAt (field, true))
	{
		/*do nothing here*/
	}
	else
	{
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
bool cMouseActionSelect::doesChangeState () const
{
	return false;
}

//------------------------------------------------------------------------------
bool cMouseActionSelect::isSingleAction () const
{
	return false;
}
