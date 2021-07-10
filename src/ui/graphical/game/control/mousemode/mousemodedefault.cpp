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
#include "ui/graphical/game/control/mouseaction/mouseactionattack.h"
#include "ui/graphical/game/control/mouseaction/mouseactionsteal.h"
#include "ui/graphical/game/control/mouseaction/mouseactiondisable.h"
#include "ui/graphical/game/control/mouseaction/mouseactionselect.h"
#include "ui/graphical/game/control/mouseaction/mouseactionmove.h"
#include "ui/graphical/game/control/mouseaction/mouseactionactivatefinished.h"
#include "ui/graphical/game/unitselection.h"
#include "game/data/map/mapview.h"
#include "ui/keys.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"
#include "input/mouse/mouse.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "input/mouse/cursor/mousecursoramount.h"
#include "input/mouse/cursor/mousecursorattack.h"
#include "input/keyboard/keyboard.h"
#include "game/data/map/mapfieldview.h"

//------------------------------------------------------------------------------
cMouseModeDefault::cMouseModeDefault (const cMapView* map_, const cUnitSelection& unitSelection_, const cPlayer* player_) :
	cMouseMode (map_, unitSelection_, player_)
{
	establishUnitSelectionConnections();
	cKeyboard& keyboard = cKeyboard::getInstance();
	keyboardConnectionManager.connect (keyboard.modifierChanged, [this]() {needRefresh(); });
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
			else mouse.setCursor (std::make_unique<cMouseCursorAmount> (eMouseCursorAmountType::Steal));
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
			else mouse.setCursor (std::make_unique<cMouseCursorAmount> (eMouseCursorAmountType::Disable));
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
		case eActionType::Steal:
			return std::make_unique<cMouseActionSteal>();
			break;
		case eActionType::Disable:
			return std::make_unique<cMouseActionDisable>();
			break;
		case eActionType::Attack:
			return std::make_unique<cMouseActionAttack>();
			break;
		case eActionType::Select:
			return std::make_unique<cMouseActionSelect>();
			break;
		case eActionType::ActivateFinished:
			return std::make_unique<cMouseActionActivateFinished>();
			break;
		case eActionType::Move:
			return std::make_unique<cMouseActionMove>();
			break;
		default:
			return nullptr;
			break;
	}

	return nullptr;
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
				 field.getVehicle() ||
				 field.getPlane() ||
				 (
					 field.getBuilding() &&
					 !field.getBuilding()->isRubble()
				 )
			 ) &&
			 (
				 !selectedVehicle ||
				 selectedVehicle->getOwner() != player ||
				 (
					 (
						 selectedVehicle->getStaticUnitData().factorAir > 0 ||
						 field.getVehicle() ||
						 (
							 field.getTopBuilding() &&
							 field.getTopBuilding()->getStaticUnitData().surfacePosition != eSurfacePosition::Above
						 ) ||
						 (
							 (KeysList.getMouseStyle() == eMouseStyle::OldSchool || !modifierForceMoveActive) &&
							 field.getPlane()
						 )
					 ) &&
					 (
						 selectedVehicle->getStaticUnitData().factorAir == 0 ||
						 (field.getPlane() && !modifierForceMoveActive) ||
						 (
							 (KeysList.getMouseStyle() == eMouseStyle::OldSchool || !modifierForceMoveActive) &&
							 (
								 field.getVehicle() ||
								 (
									 field.getTopBuilding() &&
									 field.getTopBuilding()->getStaticUnitData().surfacePosition != eSurfacePosition::Above &&
									 !field.getTopBuilding()->getStaticData().canBeLandedOn
								 )
							 )
						 )
					 )
				 )
			 ) &&
			 (
				 !selectedBuilding ||
				 selectedBuilding->getOwner() != player ||
				 (
					 selectedBuilding->isBuildListEmpty() ||
					 selectedBuilding->isUnitWorking() ||
					 selectedBuilding->getBuildListItem (0).getRemainingMetal() > 0
				 )
			 )
			)
	{
		return eActionType::Select;
	}
	else if (selectedVehicle && selectedVehicle->getOwner() == player)
	{
		if (!selectedVehicle->isUnitBuildingABuilding() && !selectedVehicle->isUnitClearing())
		{
			if (selectedVehicle->isUnitMoving())
			{
				return eActionType::None;
			}
			else if (map->possiblePlace (*selectedVehicle, mapPosition))
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
			if (((selectedVehicle->isUnitBuildingABuilding() && selectedVehicle->getBuildTurns() == 0) ||
				 (selectedVehicle->isUnitClearing() && selectedVehicle->getClearingTurns() == 0)) &&
				map->possiblePlace (*selectedVehicle, mapPosition) && selectedVehicle->isNextTo (mapPosition))
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
	else if (selectedBuilding && selectedBuilding->getOwner() == player &&
			 !selectedBuilding->isBuildListEmpty() &&
			 !selectedBuilding->isUnitWorking() &&
			 selectedBuilding->getBuildListItem (0).getRemainingMetal() <= 0)
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
	const auto selectedUnit = unitSelection.getSelectedUnit();
	if (selectedUnit)
	{
		selectedUnitSignalConnectionManager.connect (selectedUnit->data.rangeChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedUnit->data.damageChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedUnit->data.shotsChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedUnit->data.ammoChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedUnit->attackingChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedUnit->beenAttackedChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedUnit->clearingChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedUnit->buildingChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedUnit->workingChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedUnit->disabledChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedUnit->positionChanged, [this]() { needRefresh(); });
	}

	const auto selectedVehicle = unitSelection.getSelectedVehicle();
	if (selectedVehicle)
	{
		assert (selectedVehicle == selectedUnit);
		selectedUnitSignalConnectionManager.connect (selectedVehicle->moveJobChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedVehicle->movingChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedVehicle->buildingTurnsChanged, [this]() { needRefresh(); });
		selectedUnitSignalConnectionManager.connect (selectedVehicle->clearingTurnsChanged, [this]() { needRefresh(); });
	}

	const auto selectedBuilding = unitSelection.getSelectedBuilding();
	if (selectedBuilding)
	{
		assert (selectedBuilding == selectedUnit);
		selectedUnitSignalConnectionManager.connect (selectedBuilding->buildListChanged, [this]()
		{
			needRefresh();
		});
		selectedUnitSignalConnectionManager.connect (selectedBuilding->buildListFirstItemDataChanged, [this]()
		{
			needRefresh();
		});
	}
}

//------------------------------------------------------------------------------
void cMouseModeDefault::establishMapFieldConnections (const cMapFieldView& field)
{
	mapFieldSignalConnectionManager.connect (field.unitsChanged, [this, field]()
	{
		updateFieldUnitConnections (field);
		needRefresh();
	});

	updateFieldUnitConnections (field);
}

//------------------------------------------------------------------------------
void cMouseModeDefault::updateFieldUnitConnections (const cMapFieldView& field)
{
	mapFieldUnitsSignalConnectionManager.disconnectAll();

	auto plane = field.getPlane();
	if (plane)
	{
		mapFieldUnitsSignalConnectionManager.connect (plane->flightHeightChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (plane->data.hitpointsChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (plane->disabledChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (plane->storedUnitsChanged, [this]() { needRefresh(); });
	}
	auto vehicle = field.getVehicle();
	if (vehicle)
	{
		mapFieldUnitsSignalConnectionManager.connect (vehicle->storedUnitsChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->disabledChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (vehicle->data.hitpointsChanged, [this]() { needRefresh(); });
	}
	auto building = field.getBuilding();
	if (building)
	{
		mapFieldUnitsSignalConnectionManager.connect (building->storedUnitsChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (building->disabledChanged, [this]() { needRefresh(); });
		mapFieldUnitsSignalConnectionManager.connect (building->data.hitpointsChanged, [this]() { needRefresh(); });
	}
}
