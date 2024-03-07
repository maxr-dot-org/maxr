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

#include "ui/graphical/game/control/mousemode/mousemodesupplyammo.h"

#include "game/data/gui/unitselection.h"
#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/units/vehicle.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "input/mouse/mouse.h"
#include "ui/graphical/game/control/mouseaction/mouseactionsupplyammo.h"

//------------------------------------------------------------------------------
cMouseModeSupplyAmmo::cMouseModeSupplyAmmo (const cMapView* map_, const cUnitSelection& unitSelection_, const cPlayer* player_) :
	cMouseMode (map_, unitSelection_, player_)
{
	establishUnitSelectionConnections();
}

//------------------------------------------------------------------------------
eMouseModeType cMouseModeSupplyAmmo::getType() const
{
	return eMouseModeType::SupplyAmmo;
}

//------------------------------------------------------------------------------
void cMouseModeSupplyAmmo::setCursor (cMouse& mouse, const cPosition& mapPosition, const cUnitsData&) const /* override */
{
	if (canExecuteAction (mapPosition))
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Muni));
	}
	else
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::No));
	}
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseAction> cMouseModeSupplyAmmo::getMouseAction (const cPosition& mapPosition, const cUnitsData&) const /* override */
{
	if (canExecuteAction (mapPosition))
	{
		return std::make_unique<cMouseActionSupplyAmmo>();
	}
	else
		return nullptr;
}

//------------------------------------------------------------------------------
bool cMouseModeSupplyAmmo::canExecuteAction (const cPosition& mapPosition) const
{
	if (!map) return false;

	const auto selectedVehicle = unitSelection.getSelectedVehicle();

	return selectedVehicle && selectedVehicle->canSupply (*map, mapPosition, eSupplyType::REARM);
}

//------------------------------------------------------------------------------
void cMouseModeSupplyAmmo::establishUnitSelectionConnections()
{
	const auto selectedUnit = unitSelection.getSelectedUnit();

	if (selectedUnit)
	{
		selectedUnitSignalConnectionManager.connect (selectedUnit->storedResourcesChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedUnit->positionChanged, [this]() { needRefresh(); });
	}
}

//------------------------------------------------------------------------------
void cMouseModeSupplyAmmo::establishMapFieldConnections (const cMapFieldView& field)
{
	mapFieldSignalConnectionManager.connect (field.unitsChanged, [this, field]() { updateFieldUnitConnections (field); needRefresh(); });

	updateFieldUnitConnections (field);
}

//------------------------------------------------------------------------------
void cMouseModeSupplyAmmo::updateFieldUnitConnections (const cMapFieldView& field)
{
	mapFieldUnitsSignalConnectionManager.disconnectAll();

	auto plane = field.getPlane();
	if (plane)
	{
		mapFieldUnitsSignalConnectionManager.connect (plane->flightHeightChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (plane->data.ammoChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (plane->attackingChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (plane->movingChanged, [this]() { needRefresh(); });
	}
	auto vehicle = field.getVehicle();
	if (vehicle)
	{
		mapFieldUnitsSignalConnectionManager.connect (vehicle->data.ammoChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->attackingChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->movingChanged, [this]() { needRefresh(); });
	}
}
