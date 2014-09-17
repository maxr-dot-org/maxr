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

#include "ui/graphical/game/control/mousemode/mousemodetransfer.h"
#include "ui/graphical/game/control/mouseaction/mouseactiontransfer.h"
#include "ui/graphical/game/widgets/gamemapwidget.h"
#include "ui/graphical/game/unitselection.h"
#include "game/data/map/map.h"
#include "game/data/units/unit.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"
#include "input/mouse/mouse.h"
#include "input/mouse/cursor/mousecursorsimple.h"

//------------------------------------------------------------------------------
cMouseModeTransfer::cMouseModeTransfer (const cMap* map_, const cUnitSelection& unitSelection_, const cPlayer* player_) :
	cMouseMode (map_, unitSelection_, player_)
{
	updateSelectedUnitConnections ();
}

//------------------------------------------------------------------------------
eMouseModeType cMouseModeTransfer::getType () const
{
	return eMouseModeType::Transfer;
}

//------------------------------------------------------------------------------
void cMouseModeTransfer::setCursor (cMouse& mouse, const cPosition& mapPosition) const
{
	if (canExecuteAction (mapPosition))
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Transfer));
	}
	else
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::No));
	}
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseAction> cMouseModeTransfer::getMouseAction (const cPosition& mapPosition) const
{
	if (canExecuteAction (mapPosition))
	{
		return std::make_unique<cMouseActionTransfer> ();
	}
	else return nullptr;
}

//------------------------------------------------------------------------------
bool cMouseModeTransfer::canExecuteAction (const cPosition& mapPosition) const
{
	if (!map) return false;

	const auto& field = map->getField (mapPosition);

	const auto selectedUnit = unitSelection.getSelectedUnit ();

	return selectedUnit && selectedUnit->canTransferTo (mapPosition, field);
}

//------------------------------------------------------------------------------
void cMouseModeTransfer::establishUnitSelectionConnections ()
{
	const auto selectedUnit = unitSelection.getSelectedUnit ();
	if (selectedUnit)
	{
		selectedUnitSignalConnectionManager.connect (selectedUnit->positionChanged, [this](){ needRefresh (); });
	}

	const auto selectedBuilding = unitSelection.getSelectedBuilding ();
	if (selectedBuilding)
	{
		// TODO: react on:
		//  - sub base change
		assert (selectedBuilding == selectedUnit);
		//selectedUnitSignalConnectionManager.connect (selectedBuilding->xyz, [this](){ needRefresh (); });
	}
}

//------------------------------------------------------------------------------
void cMouseModeTransfer::establishMapFieldConnections (const cMapField& field)
{
	mapFieldSignalConnectionManager.connect (field.unitsChanged, [this, &field](){ updateFieldUnitConnections (field); needRefresh (); });

	updateFieldUnitConnections (field);
}

//------------------------------------------------------------------------------
void cMouseModeTransfer::updateFieldUnitConnections (const cMapField& field)
{
	mapFieldUnitsSignalConnectionManager.disconnectAll ();

	auto vehicle = field.getVehicle ();
	if (vehicle)
	{
		mapFieldUnitsSignalConnectionManager.connect (vehicle->buildingChanged, [this](){ needRefresh (); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->clearingChanged, [this](){ needRefresh (); });
	}
	auto building = field.getBuilding ();
	if (building)
	{
		// TODO: react on:
		//  - sub base change
		//mapFieldUnitsSignalConnectionManager.connect (building->xyz, [this](){ needRefresh (); });
	}
}
