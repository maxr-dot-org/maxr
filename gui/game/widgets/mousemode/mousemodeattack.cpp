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

#include "mousemodeattack.h"
#include "../mouseaction/mouseactionattack.h"
#include "../../unitselection.h"
#include "../../../../map.h"
#include "../../../../vehicles.h"
#include "../../../../buildings.h"
#include "../../../../attackjobs.h"
#include "../../../../input/mouse/mouse.h"
#include "../../../../input/mouse/cursor/mousecursorsimple.h"
#include "../../../../input/mouse/cursor/mousecursorattack.h"

//------------------------------------------------------------------------------
eMouseModeType cMouseModeAttack::getType () const
{
	return eMouseModeType::Attack;
}

//------------------------------------------------------------------------------
void cMouseModeAttack::setCursor (cMouse& mouse, const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection, const cPlayer* player) const
{
	if (canExecuteAction (map, mapPosition, unitSelection))
	{
		const auto selectedUnit = unitSelection.getSelectedUnit ();

		if (selectedUnit != nullptr)
		{
			mouse.setCursor (std::make_unique<cMouseCursorAttack> (*selectedUnit, mapPosition, map));
		}
		else
		{
			mouse.setCursor (std::make_unique<cMouseCursorAttack> ());
		}
	}
	else
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::No));
	}
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseAction> cMouseModeAttack::getMouseAction (const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection, const cPlayer* player) const
{
	if (canExecuteAction (map, mapPosition, unitSelection))
	{
		return std::make_unique<cMouseActionAttack> ();
	}
	else return nullptr;
}

//------------------------------------------------------------------------------
bool cMouseModeAttack::canExecuteAction (const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection) const
{
	const auto selectedVehicle = unitSelection.getSelectedVehicle ();
	const auto selectedBuilding = unitSelection.getSelectedBuilding ();

	return (selectedVehicle && (selectedVehicle->data.muzzleType != sUnitData::MUZZLE_TYPE_TORPEDO || map.isWaterOrCoast (mapPosition))) ||
		(selectedBuilding && selectedBuilding->isInRange (mapPosition));
}