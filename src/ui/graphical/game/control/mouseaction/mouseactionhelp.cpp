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

#include "ui/graphical/game/control/mouseaction/mouseactionhelp.h"

#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/units/building.h"
#include "game/data/units/unit.h"
#include "game/data/units/vehicle.h"
#include "ui/graphical/game/widgets/gamemapwidget.h"

//------------------------------------------------------------------------------
bool cMouseActionHelp::executeLeftClick (cGameMapWidget& gameMapWidget, const cMapView& map, const cPosition& mapPosition, cUnitSelection&, bool /* changeAllowed */) const /* override */
{
	const auto& field = map.getField (mapPosition);

	cUnit* unit = nullptr;

	const auto overVehicle = field.getVehicle();
	const auto overPlane = field.getPlane();
	const auto overBuilding = field.getBuilding();
	const auto overBaseBuilding = field.getBaseBuilding();

	if (overPlane)
		unit = overPlane;
	else if (overVehicle)
		unit = overVehicle;
	else if (overBuilding)
		unit = overBuilding;
	else if (overBaseBuilding)
		unit = overBaseBuilding;

	if (unit)
	{
		gameMapWidget.triggeredUnitHelp (*unit);
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
bool cMouseActionHelp::doesChangeState() const
{
	return false;
}

//------------------------------------------------------------------------------
bool cMouseActionHelp::isSingleAction() const
{
	return true;
}
