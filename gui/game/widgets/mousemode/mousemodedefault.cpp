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

#include "mousemodedefault.h"
#include "../mouseaction/mouseactionattack.h"
#include "../mouseaction/mouseactionsteal.h"
#include "../mouseaction/mouseactiondisable.h"
#include "../mouseaction/mouseactionselect.h"
#include "../mouseaction/mouseactionmove.h"
#include "../mouseaction/mouseactionactivatefinished.h"
#include "../../unitselection.h"
#include "../../../../map.h"
#include "../../../../keys.h"
#include "../../../../vehicles.h"
#include "../../../../buildings.h"
#include "../../../../input/mouse/mouse.h"
#include "../../../../input/mouse/cursor/mousecursorsimple.h"
#include "../../../../input/mouse/cursor/mousecursoramount.h"
#include "../../../../input/mouse/cursor/mousecursorattack.h"

//------------------------------------------------------------------------------
eMouseModeType cMouseModeDefault::getType () const
{
	return eMouseModeType::Default;
}

//------------------------------------------------------------------------------
void cMouseModeDefault::setCursor (cMouse& mouse, const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection, const cPlayer* player) const
{
	const auto mouseClickAction = selectAction (map, mapPosition, unitSelection, player);

	switch (mouseClickAction)
	{
	case eActionType::Steal:
		{
			const auto selectedVehicle = unitSelection.getSelectedVehicle ();
			if (selectedVehicle)
			{
				const auto& field = map.getField (mapPosition);
				const cUnit* unit = field.getVehicle ();

				mouse.setCursor (std::make_unique<cMouseCursorAmount> (eMouseCursorAmountType::Steal, selectedVehicle->calcCommandoChance (unit, true)));
			}
			else mouse.setCursor (std::make_unique<cMouseCursorAmount> (eMouseCursorAmountType::Steal));
		}
		break;
	case eActionType::Disable:
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
		break;
	case eActionType::Attack:
		{
			const auto selectedUnit = unitSelection.getSelectedUnit ();
			if (selectedUnit != nullptr)
			{
				mouse.setCursor (std::make_unique<cMouseCursorAttack> (*selectedUnit, mapPosition, map));
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
std::unique_ptr<cMouseAction> cMouseModeDefault::getMouseAction (const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection, const cPlayer* player) const
{
	const auto mouseClickAction = selectAction (map, mapPosition, unitSelection, player);

	switch (mouseClickAction)
	{
	case eActionType::Steal:
		return std::make_unique<cMouseActionSteal> ();
		break;
	case eActionType::Disable:
		return std::make_unique<cMouseActionDisable> ();
		break;
	case eActionType::Attack:
		return std::make_unique<cMouseActionAttack> ();
		break;
	case eActionType::Select:
		return std::make_unique<cMouseActionSelect> ();
		break;
	case eActionType::ActivateFinished:
		return std::make_unique<cMouseActionActivateFinished> ();
		break;
	case eActionType::Move:
		return std::make_unique<cMouseActionMove> ();
		break;
	default:
		return nullptr;
		break;
	}

	return nullptr;
}

//------------------------------------------------------------------------------
cMouseModeDefault::eActionType cMouseModeDefault::selectAction (const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection, const cPlayer* player) const
{
	const auto& field = map.getField (mapPosition);

	const auto selectedUnit = unitSelection.getSelectedUnit ();
	const auto selectedVehicle = unitSelection.getSelectedVehicle ();
	const auto selectedBuilding = unitSelection.getSelectedBuilding ();

	// Infiltrators: auto selected disable vs. vehicle/building
	if (selectedVehicle && selectedVehicle->owner == player && selectedVehicle->canDoCommandoAction (mapPosition, map, false))
	{
		return eActionType::Disable;
	}
	// Infiltrators: auto selected steal vs. vehicle/building
	else if (selectedVehicle && selectedVehicle->owner == player && selectedVehicle->canDoCommandoAction (mapPosition, map, true))
	{
		return eActionType::Steal;
	}
	else if (selectedVehicle && selectedVehicle->owner == player && selectedVehicle->canAttackObjectAt (mapPosition, map, false, false))
	{
		return eActionType::Attack;
	}
	else if (selectedBuilding && selectedBuilding->owner == player && selectedBuilding->canAttackObjectAt (mapPosition, map))
	{
		return eActionType::Attack;
	}
	else if ((
				field.getVehicle() ||
				field.getPlane() ||
				(
					field.getBuilding() &&
					field.getBuilding()->owner
				)
			) &&
			(
				!selectedVehicle ||
				selectedVehicle->owner != player ||
				(
					(
						selectedVehicle->data.factorAir > 0 ||
						field.getVehicle() ||
						(
							field.getTopBuilding() &&
							field.getTopBuilding()->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE
						) ||
						(
							MouseStyle == OldSchool &&
							field.getPlane()
						)
					) &&
					(
						selectedVehicle->data.factorAir == 0 ||
						field.getPlane() ||
						(
							MouseStyle == OldSchool &&
							(
								field.getVehicle() ||
								(
									field.getTopBuilding() &&
									field.getTopBuilding()->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE &&
									!field.getTopBuilding()->data.canBeLandedOn
								)
							)
						)
					)
				)
			) &&
			(
				!selectedBuilding ||
				selectedBuilding->owner != player ||
				(
					selectedBuilding->BuildList.empty() ||
					selectedBuilding->isUnitWorking () ||
					selectedBuilding->BuildList[0].metall_remaining > 0
				)
			)
		)
	{
		return eActionType::Select;
	}
	else if (selectedVehicle && selectedVehicle->owner == player)
	{
		if (!selectedVehicle->isUnitBuildingABuilding () && !selectedVehicle->isUnitClearing ())
		{
			if (selectedVehicle->MoveJobActive)
			{
				return eActionType::None;
			}
			else if (map.possiblePlace (*selectedVehicle, mapPosition, true))
			{
				return eActionType::Move;
			}
			else
			{
				return eActionType::None;
			}
		}
		else if (selectedVehicle->isUnitBuildingABuilding () || selectedVehicle->isUnitClearing ())
		{
			if (((selectedVehicle->isUnitBuildingABuilding () && selectedVehicle->getBuildTurns() == 0) ||
				(selectedVehicle->isUnitClearing () && selectedVehicle->getClearingTurns () == 0)) &&
				map.possiblePlace (*selectedVehicle, mapPosition) && selectedVehicle->isNextTo (mapPosition))
			{
				return eActionType::Move;
			}
			else
			{
				return eActionType::None;
			}
		}
	}
	else if (selectedBuilding && selectedBuilding->owner == player &&
		!selectedBuilding->BuildList.empty () &&
		!selectedBuilding->isUnitWorking () &&
		selectedBuilding->BuildList[0].metall_remaining <= 0)
	{
		if (selectedBuilding->canExitTo (mapPosition, map, *selectedBuilding->BuildList[0].type.getUnitDataOriginalVersion ()) && selectedUnit->isDisabled () == false)
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