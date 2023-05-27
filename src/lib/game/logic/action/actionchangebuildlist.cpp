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

#include "actionchangebuildlist.h"

#include "game/data/model.h"

cActionChangeBuildList::cActionChangeBuildList (const cBuilding& building, const std::vector<sID>& buildList, int buildSpeed, bool repeat) :
	buildingId (building.getId()),
	buildList (buildList),
	buildSpeed (buildSpeed),
	repeat (repeat)
{}

//------------------------------------------------------------------------------
cActionChangeBuildList::cActionChangeBuildList (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionChangeBuildList::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	cMap& map = *model.getMap();

	cBuilding* building = model.getBuildingFromID (buildingId);
	if (building == nullptr || !building->getOwner()) return;
	if (building->getOwner()->getId() != playerNr) return;

	if (buildSpeed < 0 || buildSpeed > 2) return;

	// check whether the building has water and land fields around it
	int x = building->getPosition().x() - 2;
	int y = building->getPosition().y() - 1;
	bool land = false;
	bool water = false;
	for (int i = 0; i < 12; ++i)
	{
		if (i == 4 || i == 6 || i == 8)
		{
			x -= 3;
			y += 1;
		}
		else
		{
			if (i == 5 || i == 7)
				x += 3;
			else
				x++;
		}
		if (map.isValidPosition (cPosition (x, y)) == false) continue;

		const auto& buildings = map.getField (cPosition (x, y)).getBuildings();
		auto b_it = buildings.begin();
		auto b_end = buildings.end();
		while (b_it != b_end && ((*b_it)->getStaticUnitData().surfacePosition == eSurfacePosition::Above || (*b_it)->getStaticUnitData().surfacePosition == eSurfacePosition::AboveBase))
		{
			++b_it;
		}

		if (!map.isWaterOrCoast (cPosition (x, y)) || (b_it != b_end && (*b_it)->getStaticUnitData().surfacePosition == eSurfacePosition::Base))
		{
			land = true;
		}
		else if (map.isWaterOrCoast (cPosition (x, y)) && b_it != b_end && (*b_it)->getStaticUnitData().surfacePosition == eSurfacePosition::AboveSea)
		{
			land = true;
			water = true;
			break;
		}
		else if (map.isWaterOrCoast (cPosition (x, y)))
		{
			water = true;
		}
	}

	// reset building status
	if (building->isUnitWorking())
	{
		building->stopWork (false);
	}

	if (buildSpeed == 0) building->setMetalPerRound (1 * building->getStaticUnitData().needsMetal);
	if (buildSpeed == 1) building->setMetalPerRound (4 * building->getStaticUnitData().needsMetal);
	if (buildSpeed == 2) building->setMetalPerRound (12 * building->getStaticUnitData().needsMetal);

	// if the first unit hasn't changed remember the build progress
	int remainingMetal = -1;
	if (!building->isBuildListEmpty() && !buildList.empty() && buildList[0] == building->getBuildListItem (0).getType())
	{
		remainingMetal = building->getBuildListItem (0).getRemainingMetal();
	}

	std::vector<cBuildListItem> newBuildList;
	for (const auto& id : buildList)
	{
		if (!model.getUnitsData()->isValidId (id)) continue;

		// check whether the building can build this unit
		if (model.getUnitsData()->getStaticUnitData (id).factorSea > 0 && model.getUnitsData()->getStaticUnitData (id).factorGround == 0 && !water)
			continue;
		else if (model.getUnitsData()->getStaticUnitData (id).factorGround > 0 && model.getUnitsData()->getStaticUnitData (id).factorSea == 0 && !land)
			continue;

		if (building->getStaticUnitData().canBuild != model.getUnitsData()->getStaticUnitData (id).buildAs)
			continue;

		cBuildListItem BuildListItem (id, -1);

		newBuildList.push_back (BuildListItem);
	}

	building->setBuildList (std::move (newBuildList));

	if (!building->isBuildListEmpty())
	{
		std::array<int, 3> turboBuildRounds;
		std::array<int, 3> turboBuildCosts;
		int cost = building->getOwner()->getLastUnitData (building->getBuildListItem (0).getType())->getBuildCost();
		building->calcTurboBuild (turboBuildRounds, turboBuildCosts, cost, remainingMetal);
		building->getBuildListItem (0).setRemainingMetal (turboBuildCosts[buildSpeed]);

		if (building->getBuildListItem (0).getRemainingMetal() > 0)
		{
			building->startWork();
		}
	}
	building->setRepeatBuild (repeat);
	building->setBuildSpeed (buildSpeed);
}
