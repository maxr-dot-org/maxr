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

#include "mouseactiondisable.h"
#include "../gamemapwidget.h"
#include "../../unitselection.h"
#include "../../../../map.h"
#include "../../../../vehicles.h"
#include "../../../../buildings.h"
#include "../../../../input/mouse/mouse.h"
#include "../../../../input/mouse/cursor/mousecursorsimple.h"

//------------------------------------------------------------------------------
bool cMouseActionDisable::executeLeftClick (cGameMapWidget& gameMapWidget, const cMap& map, const cPosition& mapPosition, cUnitSelection& unitSelection) const
{
	const auto selectedVehicle = unitSelection.getSelectedVehicle ();

	if (!selectedVehicle) return false;

	const auto& field = map.getField (mapPosition);

	const auto overVehicle = field.getVehicle ();
	const auto overPlane = field.getPlane ();
	const auto overBuilding = field.getBuilding ();

	if (overVehicle)
	{
		gameMapWidget.triggeredDisable (*selectedVehicle, *overVehicle);
	}
	else if (overPlane && overPlane->FlightHigh == 0)
	{
		gameMapWidget.triggeredDisable (*selectedVehicle, *overPlane);
	}
	else if (overBuilding)
	{
		gameMapWidget.triggeredDisable (*selectedVehicle, *overBuilding);
	}
	else
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
bool cMouseActionDisable::doesChangeState () const
{
	return true;
}

//------------------------------------------------------------------------------
bool cMouseActionDisable::isSingleAction () const
{
	return false;
}