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

#include "actionrepairreload.h"

#include "game/data/model.h"
#include "game/data/units/unit.h"
#include "utility/mathtools.h"

//------------------------------------------------------------------------------
cActionRepairReload::cActionRepairReload (const cUnit& sourceUnit, const cUnit& destUnit, eSupplyType supplyType) :
	sourceUnitId (sourceUnit.getId()),
	destUnitId (destUnit.getId()),
	supplyType (supplyType)
{}

//------------------------------------------------------------------------------
cActionRepairReload::cActionRepairReload (cBinaryArchiveIn& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionRepairReload::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	auto sourceUnit = model.getUnitFromID (sourceUnitId);
	if (sourceUnit == nullptr) return;

	auto destUnit = model.getUnitFromID (destUnitId);
	if (destUnit == nullptr) return;

	if (!sourceUnit->canSupply (destUnit, supplyType)) return;

	if (supplyType == eSupplyType::REARM)
	{
		// reduce cargo
		if (sourceUnit->isAVehicle())
		{
			sourceUnit->setStoredResources (sourceUnit->getStoredResources() - 1);
		}
		else
		{
			auto sourceBuilding = static_cast<cBuilding*> (sourceUnit);
			sourceBuilding->subBase->addMetal (-1);
		}

		// refill ammo
		destUnit->data.setAmmo (destUnit->data.getAmmoMax());
	}
	else if (supplyType == eSupplyType::REPAIR)
	{
		int availableMetal = 0;
		if (sourceUnit->isAVehicle())
		{
			availableMetal = sourceUnit->getStoredResources();
		}
		else
		{
			availableMetal = static_cast<cBuilding*> (sourceUnit)->subBase->getResourcesStored().metal;
		}
		int newHitpoints = destUnit->data.getHitpoints();

		int hitpointsPerMetal = Round (((float) destUnit->data.getHitpointsMax() / destUnit->data.getBuildCost()) * 4);
		while (availableMetal > 0 && newHitpoints < destUnit->data.getHitpointsMax())
		{
			newHitpoints += hitpointsPerMetal;
			availableMetal--;
		}

		if (sourceUnit->isAVehicle())
		{
			sourceUnit->setStoredResources (availableMetal);
		}
		else
		{
			auto subBase = static_cast<cBuilding*> (sourceUnit)->subBase;
			subBase->addMetal (availableMetal - subBase->getResourcesStored().metal);
		}
		destUnit->data.setHitpoints (std::min (newHitpoints, destUnit->data.getHitpointsMax()));
	}

	if (supplyType == eSupplyType::REARM)
	{
		model.unitSuppliedWithAmmo (*destUnit);
	}
	else if (supplyType == eSupplyType::REPAIR)
	{
		model.unitRepaired (*destUnit);
	}
}
