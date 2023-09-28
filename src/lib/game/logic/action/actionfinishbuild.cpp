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

#include "actionfinishbuild.h"

#include "game/data/model.h"

//------------------------------------------------------------------------------
cActionFinishBuild::cActionFinishBuild (const cUnit& unit, const cPosition& escapePosition) :
	unitId (unit.getId()),
	escapePosition (escapePosition)
{}

//------------------------------------------------------------------------------
cActionFinishBuild::cActionFinishBuild (cBinaryArchiveIn& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionFinishBuild::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	cUnit* unit = model.getUnitFromID (unitId);
	if (unit == nullptr) return;
	auto* player = unit->getOwner();
	if (player == nullptr) return;
	if (player->getId() != playerNr) return;

	if (unit->isAVehicle())
	{
		finishABuilding (model, *static_cast<cVehicle*> (unit));
	}
	else
	{
		finishAVehicle (model, *static_cast<cBuilding*> (unit));
	}
}

//------------------------------------------------------------------------------
void cActionFinishBuild::finishABuilding (cModel& model, cVehicle& vehicle) const
{
	auto map = model.getMap();

	if (!vehicle.isUnitBuildingABuilding() || vehicle.getBuildTurns() > 0) return;
	if (!map->isValidPosition (escapePosition)) return;
	if (!vehicle.isNextTo (escapePosition)) return;

	model.sideStepStealthUnit (escapePosition, vehicle);
	if (!map->possiblePlace (vehicle, escapePosition, false)) return;

	auto player = vehicle.getOwner();
	if (player)
	{
		if (auto* unitData = player->getLastUnitData (vehicle.getBuildingType()))
		{
			unitData->markLastVersionUsed();
		}
		player->getGameOverStat().builtBuildingsCount++;
		const auto& buildingUnitdata = model.getUnitsData()->getStaticUnitData (vehicle.getBuildingType());
		if (!buildingUnitdata.canBuild.empty())
		{
			player->getGameOverStat().builtFactoriesCount++;
		}
		if (buildingUnitdata.buildingData.canMineMaxRes != 0)
		{
			player->getGameOverStat().builtMineStationCount++;
		}
	}
	model.addBuilding (vehicle.getPosition(), vehicle.getBuildingType(), player);

	// end building
	vehicle.setBuildingABuilding (false);
	vehicle.BuildPath = false;

	// set the vehicle to the border
	if (vehicle.getIsBig())
	{
		cPosition pos = vehicle.getPosition();
		if (escapePosition.x() > vehicle.getPosition().x()) pos.x()++;
		if (escapePosition.y() > vehicle.getPosition().y()) pos.y()++;

		vehicle.getOwner()->updateScan (vehicle, pos);
		map->moveVehicle (vehicle, pos);
	}

	// drive away from the building lot
	model.addMoveJob (vehicle, escapePosition)->resume();
}

//------------------------------------------------------------------------------
void cActionFinishBuild::finishAVehicle (cModel& model, cBuilding& building) const
{
	auto map = model.getMap();

	if (!map->isValidPosition (escapePosition)) return;
	if (!building.isNextTo (escapePosition)) return;

	if (building.isBuildListEmpty()) return;
	cBuildListItem& buildingListItem = building.getBuildListItem (0);
	if (buildingListItem.getRemainingMetal() > 0) return;

	const cStaticUnitData& unitData = model.getUnitsData()->getStaticUnitData (buildingListItem.getType());

	auto* player = building.getOwner();
	model.sideStepStealthUnit (escapePosition, unitData, player);
	if (!map->possiblePlaceVehicle (unitData, escapePosition, player)) return;

	if (player)
	{
		if (auto* dynamicUnitData = player->getLastUnitData (buildingListItem.getType()))
		{
			dynamicUnitData->markLastVersionUsed();
		}
		player->getGameOverStat().builtVehiclesCount++;
	}
	auto& vehicle = model.addVehicle (escapePosition, buildingListItem.getType(), player);
	if (!vehicle.canLand (*map))
	{
		// start with flight height > 0, so that ground attack units
		// will not be able to attack the plane in the moment it leaves
		// the factory
		vehicle.setFlightHeight (1);
		vehicle.triggerLandingTakeOff (model);
	}

	// start new buildjob
	if (building.getRepeatBuild())
	{
		buildingListItem.setRemainingMetal (-1);
		building.addBuildListItem (buildingListItem);
	}
	building.removeBuildListItem (0);

	if (!building.isBuildListEmpty())
	{
		cBuildListItem& buildingListItem = building.getBuildListItem (0);
		if (buildingListItem.getRemainingMetal() == -1)
		{
			std::array<int, 3> turboBuildRounds;
			std::array<int, 3> turboBuildCosts;
			building.calcTurboBuild (turboBuildRounds, turboBuildCosts, player->getLastUnitData (buildingListItem.getType())->getBuildCost());
			buildingListItem.setRemainingMetal (turboBuildCosts[building.getBuildSpeed()]);
		}
		building.startWork();
	}
}
