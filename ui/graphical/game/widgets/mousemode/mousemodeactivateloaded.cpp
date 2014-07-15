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

#include "ui/graphical/game/widgets/mousemode/mousemodeactivateloaded.h"
#include "ui/graphical/game/widgets/mouseaction/mouseactionactivateloaded.h"
#include "ui/graphical/game/unitselection.h"
#include "map.h"
#include "vehicles.h"
#include "buildings.h"
#include "input/mouse/mouse.h"
#include "input/mouse/cursor/mousecursorsimple.h"

//------------------------------------------------------------------------------
cMouseModeActivateLoaded::cMouseModeActivateLoaded (const cMap* map_, const cUnitSelection& unitSelection_, const cPlayer* player_, int vehicleToActivateIndex_) :
	cMouseMode (map_, unitSelection_, player_),
	vehicleToActivateIndex (vehicleToActivateIndex_)
{}

//------------------------------------------------------------------------------
eMouseModeType cMouseModeActivateLoaded::getType () const
{
	return eMouseModeType::Activate;
}

//------------------------------------------------------------------------------
void cMouseModeActivateLoaded::setCursor (cMouse& mouse, const cPosition& mapPosition) const
{
	if (canExecuteAction (mapPosition))
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Activate));
	}
	else
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::No));
	}
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseAction> cMouseModeActivateLoaded::getMouseAction (const cPosition& mapPosition) const
{
	if (canExecuteAction (mapPosition))
	{
		return std::make_unique<cMouseActionActivateLoaded> (vehicleToActivateIndex);
	}
	else return nullptr;
}

//------------------------------------------------------------------------------
bool cMouseModeActivateLoaded::canExecuteAction (const cPosition& mapPosition) const
{
	if (!map) return false;

	const auto selectedVehicle = unitSelection.getSelectedVehicle ();
	const auto selectedBuilding = unitSelection.getSelectedBuilding ();

	return (selectedVehicle && !selectedVehicle->isDisabled () && selectedVehicle->canExitTo (mapPosition, *map, selectedVehicle->storedUnits[vehicleToActivateIndex]->data)) ||
		(selectedBuilding && !selectedBuilding->isDisabled () && selectedBuilding->canExitTo (mapPosition, *map, selectedBuilding->storedUnits[vehicleToActivateIndex]->data));
}

//------------------------------------------------------------------------------
size_t cMouseModeActivateLoaded::getVehicleToActivateIndex () const
{
	return vehicleToActivateIndex;
}

//------------------------------------------------------------------------------
void cMouseModeActivateLoaded::establishUnitSelectionConnections ()
{
	const auto selectedUnit = unitSelection.getSelectedUnit ();

	if (selectedUnit)
	{
		selectedUnitSignalConnectionManager.connect (selectedUnit->disabledChanged, [this](){ needRefresh (); });
		selectedUnitSignalConnectionManager.connect (selectedUnit->positionChanged, [this](){ needRefresh (); });
	}
}

//------------------------------------------------------------------------------
void cMouseModeActivateLoaded::establishMapFieldConnections (const cMapField& field)
{
	mapFieldSignalConnectionManager.connect (field.unitsChanged, [this](){ needRefresh (); });
}
