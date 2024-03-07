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

#include "ui/graphical/game/control/mousemode/mousemodeactivateloaded.h"

#include "game/data/gui/unitselection.h"
#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "input/mouse/mouse.h"
#include "ui/graphical/game/control/mouseaction/mouseactionactivateloaded.h"

//------------------------------------------------------------------------------
cMouseModeActivateLoaded::cMouseModeActivateLoaded (const cMapView* map_, const cUnitSelection& unitSelection_, const cPlayer* player_, int vehicleToActivateIndex_) :
	cMouseMode (map_, unitSelection_, player_),
	vehicleToActivateIndex (vehicleToActivateIndex_)
{
	establishUnitSelectionConnections();
}

//------------------------------------------------------------------------------
eMouseModeType cMouseModeActivateLoaded::getType() const
{
	return eMouseModeType::Activate;
}

//------------------------------------------------------------------------------
void cMouseModeActivateLoaded::setCursor (cMouse& mouse, const cPosition& mapPosition, const cUnitsData&) const /* override */
{
	const auto cursorType = canExecuteAction (mapPosition) ? eMouseCursorSimpleType::Activate : eMouseCursorSimpleType::No;
	mouse.setCursor (std::make_unique<cMouseCursorSimple> (cursorType));
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseAction> cMouseModeActivateLoaded::getMouseAction (const cPosition& mapPosition, const cUnitsData&) const /* override */
{
	if (canExecuteAction (mapPosition))
	{
		return std::make_unique<cMouseActionActivateLoaded> (vehicleToActivateIndex);
	}
	return nullptr;
}

//------------------------------------------------------------------------------
bool cMouseModeActivateLoaded::canExecuteAction (const cPosition& mapPosition) const
{
	if (!map) return false;

	const auto* selectedUnit = unitSelection.getSelectedUnit();

	if (selectedUnit == nullptr || selectedUnit->isDisabled()) return false;
	return selectedUnit->canExitTo (mapPosition, *map, selectedUnit->storedUnits[vehicleToActivateIndex]->getStaticUnitData());
}

//------------------------------------------------------------------------------
void cMouseModeActivateLoaded::establishUnitSelectionConnections()
{
	const auto selectedUnit = unitSelection.getSelectedUnit();

	if (selectedUnit)
	{
		selectedUnitSignalConnectionManager.connect (selectedUnit->disabledChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedUnit->positionChanged, [this]() { needRefresh(); });
	}
}

//------------------------------------------------------------------------------
void cMouseModeActivateLoaded::establishMapFieldConnections (const cMapFieldView& field)
{
	mapFieldSignalConnectionManager.connect (field.unitsChanged, [this]() { needRefresh(); });
}
