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

#include "actionupgradebuilding.h"

#include "game/data/model.h"
#include "game/data/player/player.h"

//------------------------------------------------------------------------------
cActionUpgradeBuilding::cActionUpgradeBuilding (const cBuilding& building, bool allBuildings) :
	buildingId (building.getId()),
	allBuildings (allBuildings)
{}

//------------------------------------------------------------------------------
cActionUpgradeBuilding::cActionUpgradeBuilding (cBinaryArchiveIn& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionUpgradeBuilding::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	cBuilding* building = model.getBuildingFromID (buildingId);
	if (building == nullptr || !building->getOwner()) return;
	if (building->getOwner()->getId() != playerNr) return;

	std::vector<cBuilding*> upgradedBuildings;
	int totalCosts = 0;
	cSubBase& subbase = *building->subBase;
	int availableMetal = subbase.getResourcesStored().metal;
	cDynamicUnitData& upgradedData = *building->getOwner()->getLastUnitData (building->data.getId());
	upgradedData.markLastVersionUsed();
	cUpgradeCalculator& uc = cUpgradeCalculator::instance();
	const int upgradeCost = uc.getMaterialCostForUpgrading (upgradedData.getBuildCost());

	// first update the selected building
	if (availableMetal >= upgradeCost && building->data.getVersion() < upgradedData.getVersion())
	{
		upgradedBuildings.push_back (building);
		totalCosts += upgradeCost;
		availableMetal -= upgradeCost;
	}

	if (allBuildings)
	{
		for (auto b : subbase.getBuildings())
		{
			if (b->data.getId() != building->data.getId()) continue;
			if (b == building) continue;

			// check unit version
			if (b->data.getVersion() >= upgradedData.getVersion()) continue; // already up to date

			// check upgrade costs
			if (upgradeCost > availableMetal) break;

			upgradedBuildings.push_back (b);
			totalCosts += upgradeCost;
			availableMetal -= upgradeCost;
		}
	}

	// execute the upgrades
	for (auto b : upgradedBuildings)
	{
		// update scan & sentry
		if (b->getOwner() && b->data.getScan() < upgradedData.getScan())
		{
			b->getOwner()->updateScan (*b, upgradedData.getScan());
		}
		if (b->getOwner() && b->isSentryActive() && b->data.getRange() < upgradedData.getRange())
		{
			b->getOwner()->updateSentry (*b, upgradedData.getRange());
		}

		b->upgradeToCurrentVersion();
	}
	subbase.addMetal (-totalCosts);

	building->getOwner()->unitsUpgraded (building->data.getId(), upgradedBuildings.size(), totalCosts);
}
