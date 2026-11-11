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

#include "actionstartbuild.h"

#include "game/data/model.h"
#include "game/logic/jobs/startbuildjob.h"
#include "game/logic/jobs/stealthstepwaitjob.h"

//------------------------------------------------------------------------------
cActionStartBuild::cActionStartBuild (const cVehicle& vehicle, sID buildingTypeID, int buildSpeed, const cPosition& buildPosition) :
	vehicleID (vehicle.getId()),
	buildingTypeID (buildingTypeID),
	buildSpeed (buildSpeed),
	buildPosition (buildPosition)
{}

//------------------------------------------------------------------------------
cActionStartBuild::cActionStartBuild (const cVehicle& vehicle, sID buildingTypeID, int buildSpeed, const cPosition& buildPosition, const cPosition& pathEndPosition) :
	vehicleID (vehicle.getId()),
	buildingTypeID (buildingTypeID),
	buildSpeed (buildSpeed),
	buildPosition (buildPosition),
	pathEndPosition (pathEndPosition)
{}

//------------------------------------------------------------------------------
cActionStartBuild::cActionStartBuild (cBinaryArchiveIn& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionStartBuild::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	cMap& map = *model.getMap();

	cVehicle* vehicle = model.getVehicleFromID (vehicleID);
	if (vehicle == nullptr || !vehicle->getOwner()) return;
	if (vehicle->getOwner()->getId() != playerNr) return;

	if (!model.getUnitsData()->isValidId (buildingTypeID)) return;
	if (!buildingTypeID.isABuilding()) return;
	if (!map.isValidPosition (buildPosition)) return;
	if (pathEndPosition && !map.isValidPosition (*pathEndPosition)) return;
	if (buildSpeed > 2 || buildSpeed < 0) return;

	if (vehicle->isUnitBuildingABuilding() || vehicle->bandPosition) return;
	if (vehicle->isDisabled()) return;
	if (vehicle->isUnitMoving()) return;

	const cStaticUnitData& data = model.getUnitsData()->getStaticUnitData (buildingTypeID);

	if (vehicle->getStaticUnitData().canBuild != data.buildAs) return;

	vehicle->setSentryActive (false);

	std::array<int, 3> turboBuildRounds;
	std::array<int, 3> turboBuildCosts;
	int buildcost = vehicle->getOwner()->getLastUnitData (buildingTypeID)->getBuildCost();
	vehicle->calcTurboBuild (turboBuildRounds, turboBuildCosts, buildcost);

	if (turboBuildCosts[buildSpeed] > vehicle->getStoredResources() || turboBuildRounds[buildSpeed] <= 0)
	{
		vehicle->getOwner()->buildErrorInsufficientMaterial();
		return;
	}

	cPosition oldPosition = vehicle->getPosition();
	if (data.buildingData.isBig)
	{
		std::vector<cVehicle*> stealthVehicles;
		for (const cPosition& offset : {cPosition (0, 0), cPosition (1, 0), cPosition (0, 1), cPosition (1, 1)})
		{
			if (auto* stealthVehicle = model.sideStepStealthUnit (buildPosition + offset, *vehicle, buildPosition))
			{
				stealthVehicles.push_back (stealthVehicle);
			}
		}
		if (!stealthVehicles.empty())
		{
			// the building is started, when the stealth units have made their steps
			model.addJob (std::make_unique<cStealthStepWaitJob> (*vehicle, stealthVehicles, this));
			return;
		}

		if (!(map.possiblePlaceBuilding (data, buildPosition, nullptr, vehicle) && map.possiblePlaceBuilding (data, buildPosition + cPosition (1, 0), nullptr, vehicle) && map.possiblePlaceBuilding (data, buildPosition + cPosition (0, 1), nullptr, vehicle) && map.possiblePlaceBuilding (data, buildPosition + cPosition (1, 1), nullptr, vehicle)))
		{
			vehicle->getOwner()->buildErrorBuildPositionBlocked();
			return;
		}

		// set vehicle to build position
		vehicle->getOwner()->updateScan (*vehicle, buildPosition, true);
		map.moveVehicleBig (*vehicle, buildPosition);
	}
	else
	{
		if (buildPosition != vehicle->getPosition()) return;

		if (!map.possiblePlaceBuilding (data, buildPosition, nullptr, vehicle))
		{
			vehicle->getOwner()->buildErrorBuildPositionBlocked();
			return;
		}
	}

	vehicle->setBuildingType (buildingTypeID);
	vehicle->bandPosition = pathEndPosition;

	vehicle->setBuildCosts (turboBuildCosts[buildSpeed]);
	vehicle->setBuildTurns (turboBuildRounds[buildSpeed]);
	vehicle->setBuildCostsStart (vehicle->getBuildCosts());
	vehicle->setBuildTurnsStart (vehicle->getBuildTurns());

	vehicle->setBuildingABuilding (true);

	model.addJob (std::make_unique<cStartBuildJob> (*vehicle, oldPosition, data.buildingData.isBig));

	if (vehicle->getMoveJob()) vehicle->getMoveJob()->stop (*vehicle);
}
