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

#include "ui/graphical/game/control/mousemode/mousemodeload.h"
#include "ui/graphical/game/control/mouseaction/mouseactionload.h"
#include "ui/graphical/game/unitselection.h"
#include "game/data/map/map.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"
#include "input/mouse/mouse.h"
#include "input/mouse/cursor/mousecursorsimple.h"

//------------------------------------------------------------------------------
cMouseModeLoad::cMouseModeLoad (const cMap* map_, const cUnitSelection& unitSelection_, const cPlayer* player_) :
	cMouseMode (map_, unitSelection_, player_)
{
	establishUnitSelectionConnections();
}

//------------------------------------------------------------------------------
eMouseModeType cMouseModeLoad::getType() const
{
	return eMouseModeType::Load;
}

//------------------------------------------------------------------------------
void cMouseModeLoad::setCursor(cMouse& mouse, const cPosition& mapPosition, const cUnitsData& unitsData) const
{
	if (canExecuteAction (mapPosition))
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Load));
	}
	else
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::No));
	}
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseAction> cMouseModeLoad::getMouseAction(const cPosition& mapPosition, const cUnitsData& unitsData) const
{
	if (canExecuteAction (mapPosition))
	{
		return std::make_unique<cMouseActionLoad> ();
	}
	else return nullptr;
}

//------------------------------------------------------------------------------
bool cMouseModeLoad::canExecuteAction (const cPosition& mapPosition) const
{
	if (!map) return false;

	const auto selectedVehicle = unitSelection.getSelectedVehicle();
	const auto selectedBuilding = unitSelection.getSelectedBuilding();

	return (selectedVehicle && selectedVehicle->canLoad (mapPosition, *map, false)) ||
		   (selectedBuilding && selectedBuilding->canLoad (mapPosition, *map, false));
}

//------------------------------------------------------------------------------
void cMouseModeLoad::establishUnitSelectionConnections()
{
	const auto selectedUnit = unitSelection.getSelectedUnit();

	if (selectedUnit)
	{
		selectedUnitSignalConnectionManager.connect (selectedUnit->storedUnitsChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedUnit->positionChanged, [this]() { needRefresh(); });
	}
}

//------------------------------------------------------------------------------
void cMouseModeLoad::establishMapFieldConnections (const cMapField& field)
{
	mapFieldSignalConnectionManager.connect (field.unitsChanged, [this, &field]() { updateFieldUnitConnections (field); needRefresh(); });

	updateFieldUnitConnections (field);
}

//------------------------------------------------------------------------------
void cMouseModeLoad::updateFieldUnitConnections (const cMapField& field)
{
	mapFieldUnitsSignalConnectionManager.disconnectAll();

	auto plane = field.getPlane();
	if (plane)
	{
		mapFieldUnitsSignalConnectionManager.connect (plane->clearingChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (plane->beeingAttackedChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (plane->attackingChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (plane->buildingChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (plane->movingChanged, [this]() { needRefresh(); });
	}
	auto vehicle = field.getVehicle();
	if (vehicle)
	{
		mapFieldUnitsSignalConnectionManager.connect (vehicle->moveJobChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->clearingChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->beeingAttackedChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->attackingChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->buildingChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->movingChanged, [this]() { needRefresh(); });
	}
}
