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

#include "actionupgradevehicle.h"

#include "game/data/model.h"

//------------------------------------------------------------------------------
cActionUpgradeVehicle::cActionUpgradeVehicle (const cBuilding& containingBuilding, const cVehicle* vehicle) :
	buildingId (containingBuilding.getId()),
	vehicleId (vehicle ? vehicle->getId() : 0)
{}

//------------------------------------------------------------------------------
cActionUpgradeVehicle::cActionUpgradeVehicle (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionUpgradeVehicle::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	cBuilding* containingBuilding = model.getBuildingFromID (buildingId);
	if (containingBuilding == nullptr || !containingBuilding->getOwner()) return;
	if (containingBuilding->getOwner()->getId() != playerNr) return;

	std::map<sID, sUpgradeResult> result;
	for (auto vehicle : containingBuilding->storedUnits)
	{
		if (!vehicle->getOwner()) continue;
		if (vehicle->getId() == vehicleId || vehicleId == 0)
		{
			// check unit version
			cDynamicUnitData& upgradedData = *vehicle->getOwner()->getUnitDataCurrentVersion (vehicle->data.getId());
			upgradedData.markLastVersionUsed();
			if (vehicle->data.getVersion() >= upgradedData.getVersion()) continue; // already up to date

			// check upgrade costs
			cUpgradeCalculator& uc = cUpgradeCalculator::instance();
			const int upgradeCost = uc.getMaterialCostForUpgrading (upgradedData.getBuildCost());
			if (upgradeCost > containingBuilding->subBase->getResourcesStored().metal) continue;

			// ok, execute upgrade
			vehicle->upgradeToCurrentVersion();
			containingBuilding->subBase->addMetal (-upgradeCost);
			result[vehicle->data.getId()].costs += upgradeCost;
			result[vehicle->data.getId()].nr++;
		}
	}

	for (const auto& [id, upgrade] : result)
	{
		containingBuilding->getOwner()->unitsUpgraded (id, upgrade.nr, upgrade.costs);
	}
}
