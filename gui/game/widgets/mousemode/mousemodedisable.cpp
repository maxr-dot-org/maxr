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

#include "mousemodedisable.h"
#include "../mouseaction/mouseactiondisable.h"
#include "../../unitselection.h"
#include "../../../../map.h"
#include "../../../../vehicles.h"
#include "../../../../buildings.h"
#include "../../../../input/mouse/mouse.h"
#include "../../../../input/mouse/cursor/mousecursorsimple.h"
#include "../../../../input/mouse/cursor/mousecursoramount.h"

//------------------------------------------------------------------------------
eMouseModeType cMouseModeDisable::getType () const
{
	return eMouseModeType::Disable;
}

//------------------------------------------------------------------------------
void cMouseModeDisable::setCursor (cMouse& mouse, const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection, const cPlayer* player) const
{
	if (canExecuteAction (map, mapPosition, unitSelection))
	{
		const auto selectedVehicle = unitSelection.getSelectedVehicle ();

		if (selectedVehicle)
		{
			const auto& field = map.getField (mapPosition);
			const cUnit* unit = field.getVehicle ();
			if (!unit) unit = field.getTopBuilding ();

			mouse.setCursor (std::make_unique<cMouseCursorAmount> (eMouseCursorAmountType::Disable, selectedVehicle->calcCommandoChance (unit, false)));
		}
		else mouse.setCursor (std::make_unique<cMouseCursorAmount> (eMouseCursorAmountType::Disable));
	}
	else
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::No));
	}
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseAction> cMouseModeDisable::getMouseAction (const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection, const cPlayer* player) const
{
	if (canExecuteAction (map, mapPosition, unitSelection))
	{
		return std::make_unique<cMouseActionDisable> ();
	}
	else return nullptr;
}

//------------------------------------------------------------------------------
bool cMouseModeDisable::canExecuteAction (const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection) const
{
	const auto& field = map.getField (mapPosition);
	const auto selectedVehicle = unitSelection.getSelectedVehicle ();

	return (selectedVehicle && selectedVehicle->canDoCommandoAction (mapPosition.x (), mapPosition.y (), map, false) &&
			(!field.getVehicle () || field.getVehicle ()->isDisabled () == false) &&
			(!field.getBuilding () || field.getBuilding ()->isDisabled () == false));
}