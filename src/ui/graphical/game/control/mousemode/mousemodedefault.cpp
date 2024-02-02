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

#include "ui/graphical/game/control/mousemode/mousemodedefault.h"

#include "game/data/gui/unitselection.h"
#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/logic/action/actionstartmove.h"
#include "input/keyboard/keyboard.h"
#include "input/mouse/cursor/mousecursoramount.h"
#include "input/mouse/cursor/mousecursorattack.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "input/mouse/mouse.h"
#include "resources/keys.h"
#include "ui/graphical/game/control/mouseaction/mouseactionactivatefinished.h"
#include "ui/graphical/game/control/mouseaction/mouseactionattack.h"
#include "ui/graphical/game/control/mouseaction/mouseactiondisable.h"
#include "ui/graphical/game/control/mouseaction/mouseactionmove.h"
#include "ui/graphical/game/control/mouseaction/mouseactionselect.h"
#include "ui/graphical/game/control/mouseaction/mouseactionsteal.h"

//------------------------------------------------------------------------------
cMouseModeDefault::cMouseModeDefault (const cMapView* map, const cUnitSelection& unitSelection, const cPlayer* player) :
	cMouseMode (map, unitSelection, player)
{
	establishUnitSelectionConnections();
	cKeyboard& keyboard = cKeyboard::getInstance();
	keyboardConnectionManager.connect (keyboard.modifierChanged, [this]() { needRefresh(); });
}

//------------------------------------------------------------------------------
eMouseModeType cMouseModeDefault::getType() const
{
	return eMouseModeType::Default;
}

//------------------------------------------------------------------------------
void cMouseModeDefault::setCursor (cMouse& mouse, const cPosition& mapPosition, const cUnitsData& unitsData) const
{
	const auto mouseClickAction = selectAction (mapPosition, unitsData);

	switch (mouseClickAction)
	{
		case eActionType::Steal:
		{
			const auto selectedVehicle = unitSelection.getSelectedVehicle();
			if (selectedVehicle && map)
			{
				const auto& field = map->getField (mapPosition);
				const cUnit* unit = field.getVehicle();

				mouse.setCursor (std::make_unique<cMouseCursorAmount> (eMouseCursorAmountType::Steal, selectedVehicle->getCommandoData().computeChance (unit, true)));
			}
			else
				mouse.setCursor (std::make_unique<cMouseCursorAmount> (eMouseCursorAmountType::Steal));
		}
		break;
		case eActionType::Disable:
		{
			const auto selectedVehicle = unitSelection.getSelectedVehicle();
			if (selectedVehicle && map)
			{
				const auto& field = map->getField (mapPosition);
				const cUnit* unit = field.getVehicle();
				if (!unit) unit = field.getTopBuilding();

				mouse.setCursor (std::make_unique<cMouseCursorAmount> (eMouseCursorAmountType::Disable, selectedVehicle->getCommandoData().computeChance (unit, false)));
			}
			else
				mouse.setCursor (std::make_unique<cMouseCursorAmount> (eMouseCursorAmountType::Disable));
		}
		break;
		case eActionType::Attack:
		{
			const auto selectedUnit = unitSelection.getSelectedUnit();
			if (selectedUnit && map)
			{
				mouse.setCursor (std::make_unique<cMouseCursorAttack> (*selectedUnit, mapPosition, *map));
			}
		}
		break;
		case eActionType::Select:
			mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Select));
			break;
		case eActionType::ActivateFinished:
			mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Activate));
			break;
		case eActionType::Move:
			mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Move));
			break;
		case eActionType::None:
			mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::No));
			break;
		default:
		case eActionType::Unknown:
			mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
			break;
	}
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseAction> cMouseModeDefault::getMouseAction (const cPosition& mapPosition, const cUnitsData& unitsData) const
{
	const auto mouseClickAction = selectAction (mapPosition, unitsData);

	switch (mouseClickAction)
	{
		case eActionType::Steal: return std::make_unique<cMouseActionSteal>();
		case eActionType::Disable: return std::make_unique<cMouseActionDisable>();
		case eActionType::Attack: return std::make_unique<cMouseActionAttack>();
		case eActionType::Select: return std::make_unique<cMouseActionSelect>();
		case eActionType::ActivateFinished: return std::make_unique<cMouseActionActivateFinished>();
		case eActionType::Move:
		{
			eStart start = cKeyboard::getInstance().isAnyModifierActive (KeyModifierFlags (eKeyModifierType::Shift)) ? eStart::Deferred : eStart::Immediate;
			return std::make_unique<cMouseActionMove> (start);
		}
		default: return nullptr;
	}
}

//------------------------------------------------------------------------------
cMouseModeDefault::eActionType cMouseModeDefault::selectAction (const cPosition& mapPosition, const cUnitsData& unitsData) const
{
	if (!map) return eActionType::Unknown;

	const auto& field = map->getField (mapPosition);

	const auto selectedUnit = unitSelection.getSelectedUnit();
	const auto selectedVehicle = unitSelection.getSelectedVehicle();
	const auto selectedBuilding = unitSelection.getSelectedBuilding();

	const bool modifierForceMoveActive = cKeyboard::getInstance().isAnyModifierActive (toEnumFlag (eKeyModifierType::Ctrl));

	// Infiltrators: auto selected disable vs. vehicle/building
	if (selectedVehicle && selectedVehicle->getOwner() == player && cCommandoData::canDoAction (*selectedVehicle, mapPosition, *map, false))
	{
		return eActionType::Disable;
	}
	// Infiltrators: auto selected steal vs. vehicle/building
	else if (selectedVehicle && selectedVehicle->getOwner() == player && cCommandoData::canDoAction (*selectedVehicle, mapPosition, *map, true))
	{
		return eActionType::Steal;
	}
	else if (selectedVehicle && selectedVehicle->getOwner() == player && selectedVehicle->canAttackObjectAt (mapPosition, *map, false, false) && !modifierForceMoveActive)
	{
		return eActionType::Attack;
	}
	else if (selectedBuilding && selectedBuilding->getOwner() == player && selectedBuilding->canAttackObjectAt (mapPosition, *map))
	{
		return eActionType::Attack;
	}
	else if ((
				 field.getVehicle() || field.getPlane() || (field.getBuilding() && !field.getBuilding()->isRubble()))
	         && (!selectedVehicle || selectedVehicle->getOwner() != player || ((selectedVehicle->getStaticUnitData().factorAir > 0 || field.getVehicle() || (field.getTopBuilding() && field.getTopBuilding()->getStaticUnitData().surfacePosition != eSurfacePosition::Above) || ((KeysList.getMouseStyle() == eMouseStyle::OldSchool || !modifierForceMoveActive) && field.getPlane())) && (selectedVehicle->getStaticUnitData().factorAir == 0 || (field.getPlane() && !modifierForceMoveActive) || ((KeysList.getMouseStyle() == eMouseStyle::OldSchool || !modifierForceMoveActive) && (field.getVehicle() || (field.getTopBuilding() && field.getTopBuilding()->getStaticUnitData().surfacePosition != eSurfacePosition::Above && !field.getTopBuilding()->getStaticData().canBeLandedOn)))))) && (!selectedBuilding || selectedBuilding->getOwner() != player || (selectedBuilding->isBuildListEmpty() || selectedBuilding->isUnitWorking() || selectedBuilding->getBuildListItem (0).getRemainingMetal() > 0)))
	{
		return eActionType::Select;
	}
	else if (selectedVehicle && selectedVehicle->getOwner() == player)
	{
		if (!selectedVehicle->isUnitBuildingABuilding() && !selectedVehicle->isUnitClearing())
		{
			if (!selectedVehicle->isUnitMoving() && map->possiblePlace (*selectedVehicle, mapPosition))
			{
				return eActionType::Move;
			}
			else
			{
				return eActionType::None;
			}
		}
		else if (selectedVehicle->isUnitBuildingABuilding() || selectedVehicle->isUnitClearing())
		{
			if (((selectedVehicle->isUnitBuildingABuilding() && selectedVehicle->getBuildTurns() == 0) || (selectedVehicle->isUnitClearing() && selectedVehicle->getClearingTurns() == 0)) && map->possiblePlace (*selectedVehicle, mapPosition) && selectedVehicle->isNextTo (mapPosition))
			{
				//exit from construction site
				return eActionType::Move;
			}
			else
			{
				//exit from construction site not possible (still working)
				return eActionType::None;
			}
		}
	}
	else if (selectedBuilding && selectedBuilding->getOwner() == player && !selectedBuilding->isBuildListEmpty() && !selectedBuilding->isUnitWorking() && selectedBuilding->getBuildListItem (0).getRemainingMetal() <= 0)
	{
		if (selectedBuilding->canExitTo (mapPosition, *map, unitsData.getStaticUnitData (selectedBuilding->getBuildListItem (0).getType())) && selectedUnit->isDisabled() == false)
		{
			return eActionType::ActivateFinished;
		}
		else
		{
			return eActionType::None;
		}
	}

	return eActionType::Unknown;
}

//------------------------------------------------------------------------------
void cMouseModeDefault::establishUnitSelectionConnections()
{
	const auto refresher = [this]() { needRefresh(); };
	auto* unit = unitSelection.getSelectedUnit();
	if (unit)
	{
		selectedUnitSignalConnectionManager.connect (unit->data.rangeChanged, refresher);
		selectedUnitSignalConnectionManager.connect (unit->data.damageChanged, refresher);
		selectedUnitSignalConnectionManager.connect (unit->data.shotsChanged, refresher);
		selectedUnitSignalConnectionManager.connect (unit->data.ammoChanged, refresher);
		selectedUnitSignalConnectionManager.connect (unit->attackingChanged, refresher);
		selectedUnitSignalConnectionManager.connect (unit->beenAttackedChanged, refresher);
		selectedUnitSignalConnectionManager.connect (unit->clearingChanged, refresher);
		selectedUnitSignalConnectionManager.connect (unit->buildingChanged, refresher);
		selectedUnitSignalConnectionManager.connect (unit->disabledChanged, refresher);
		selectedUnitSignalConnectionManager.connect (unit->positionChanged, refresher);
	}

	if (auto* vehicle = dynamic_cast<cVehicle*> (unit))
	{
		selectedUnitSignalConnectionManager.connect (vehicle->moveJobChanged, refresher);
		selectedUnitSignalConnectionManager.connect (vehicle->movingChanged, refresher);
		selectedUnitSignalConnectionManager.connect (vehicle->buildingTurnsChanged, refresher);
		selectedUnitSignalConnectionManager.connect (vehicle->clearingTurnsChanged, refresher);
	}

	if (auto* building = dynamic_cast<cBuilding*> (unit))
	{
		selectedUnitSignalConnectionManager.connect (building->workingChanged, refresher);
		selectedUnitSignalConnectionManager.connect (building->buildListChanged, refresher);
		selectedUnitSignalConnectionManager.connect (building->buildListFirstItemDataChanged, refresher);
	}
}

//------------------------------------------------------------------------------
void cMouseModeDefault::establishMapFieldConnections (const cMapFieldView& field)
{
	mapFieldSignalConnectionManager.connect (field.unitsChanged, [this, field]() {
		updateFieldUnitConnections (field);
		needRefresh();
	});

	updateFieldUnitConnections (field);
}

//------------------------------------------------------------------------------
void cMouseModeDefault::updateFieldUnitConnections (const cMapFieldView& field)
{
	mapFieldUnitsSignalConnectionManager.disconnectAll();
	const auto refresher = [this]() { needRefresh(); };

	if (auto plane = field.getPlane())
	{
		mapFieldUnitsSignalConnectionManager.connect (plane->flightHeightChanged, refresher);
		mapFieldUnitsSignalConnectionManager.connect (plane->data.hitpointsChanged, refresher);
		mapFieldUnitsSignalConnectionManager.connect (plane->disabledChanged, refresher);
		mapFieldUnitsSignalConnectionManager.connect (plane->storedUnitsChanged, refresher);
	}
	if (auto vehicle = field.getVehicle())
	{
		mapFieldUnitsSignalConnectionManager.connect (vehicle->storedUnitsChanged, refresher);
		mapFieldUnitsSignalConnectionManager.connect (vehicle->disabledChanged, refresher);
		mapFieldUnitsSignalConnectionManager.connect (vehicle->data.hitpointsChanged, refresher);
	}
	if (auto building = field.getBuilding())
	{
		mapFieldUnitsSignalConnectionManager.connect (building->storedUnitsChanged, refresher);
		mapFieldUnitsSignalConnectionManager.connect (building->disabledChanged, refresher);
		mapFieldUnitsSignalConnectionManager.connect (building->data.hitpointsChanged, refresher);
	}
}
