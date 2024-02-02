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

#include "ui/graphical/game/control/mousemode/mousemodeenter.h"

#include "game/data/gui/unitselection.h"
#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "input/mouse/mouse.h"
#include "ui/graphical/game/control/mouseaction/mouseactionenter.h"
#include "ui/graphical/game/control/mouseaction/mouseactionload.h"

//------------------------------------------------------------------------------
cMouseModeEnter::cMouseModeEnter (const cMapView* map_, const cUnitSelection& unitSelection_, const cPlayer* player_) :
	cMouseMode (map_, unitSelection_, player_)
{
	establishUnitSelectionConnections();
}

//------------------------------------------------------------------------------
eMouseModeType cMouseModeEnter::getType() const
{
	return eMouseModeType::Enter;
}

//------------------------------------------------------------------------------
void cMouseModeEnter::setCursor (cMouse& mouse, const cPosition& mapPosition, const cUnitsData&) const
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
std::unique_ptr<cMouseAction> cMouseModeEnter::getMouseAction (const cPosition& mapPosition, const cUnitsData&) const
{
	if (canExecuteAction (mapPosition))
	{
		return std::make_unique<cMouseActionEnter>();
	}
	else
		return nullptr;
}

//------------------------------------------------------------------------------
bool cMouseModeEnter::canExecuteAction (const cPosition& mapPosition) const
{
	if (!map) return false;

	const auto selectedVehicle = unitSelection.getSelectedVehicle();
	if (!selectedVehicle) return false;

	const auto overBuilding = map->getField (mapPosition).getBuilding();
	const auto overVehicle = map->getField (mapPosition).getVehicle();

	return (overBuilding && overBuilding->canLoad (selectedVehicle, false))
	    || (overVehicle && overVehicle->canLoad (selectedVehicle, false));
}

//------------------------------------------------------------------------------
void cMouseModeEnter::establishUnitSelectionConnections()
{
	const auto selectedUnit = unitSelection.getSelectedUnit();

	if (!selectedUnit)
	{
		return;
	}
	selectedUnitSignalConnectionManager.connect (selectedUnit->movingChanged, [this]() { needRefresh(); });
	selectedUnitSignalConnectionManager.connect (selectedUnit->disabledChanged, [this]() { needRefresh(); });
	selectedUnitSignalConnectionManager.connect (selectedUnit->ownerChanged, [this]() { needRefresh(); });
	selectedUnitSignalConnectionManager.connect (selectedUnit->positionChanged, [this]() { needRefresh(); });

	if (auto selectedVehicle = dynamic_cast<cVehicle*>(selectedUnit)) {

		selectedUnitSignalConnectionManager.connect (selectedVehicle->buildingChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedVehicle->clearingChanged, [this]() { needRefresh(); });
	}
}

//------------------------------------------------------------------------------
void cMouseModeEnter::establishMapFieldConnections (const cMapFieldView& field)
{
	mapFieldSignalConnectionManager.connect (field.unitsChanged, [this, &field]() { updateFieldUnitConnections (field); needRefresh(); });

	updateFieldUnitConnections (field);
}

//------------------------------------------------------------------------------
void cMouseModeEnter::updateFieldUnitConnections (const cMapFieldView& field)
{
	mapFieldUnitsSignalConnectionManager.disconnectAll();

	auto building = field.getBuilding();
	if (building)
	{
		mapFieldUnitsSignalConnectionManager.connect (building->storedUnitsChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (building->disabledChanged, [this]() { needRefresh(); });
	}
	auto vehicle = field.getVehicle();
	if (vehicle)
	{
		mapFieldUnitsSignalConnectionManager.connect (vehicle->moveJobChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->disabledChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->beeingAttackedChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->attackingChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->buildingChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->movingChanged, [this]() { needRefresh(); });
	}
}
