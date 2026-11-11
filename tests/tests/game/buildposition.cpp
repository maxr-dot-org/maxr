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

#include "game/data/model.h"
#include "game/data/player/player.h"
#include "game/data/units/unitdata.h"
#include "game/data/units/vehicle.h"
#include "game/logic/action/actionstartbuild.h"
#include "testutility.h"

#include <doctest.h>
#include <memory>

namespace
{
	const sID constructorId{0, 100};
	const sID bigBuildingId{1, 100};
	const sID smallBuildingId{1, 101};

	//--------------------------------------------------------------------------
	void addBuildingData (cUnitsData& unitsData, const sID& id, bool isBig)
	{
		cStaticUnitData buildingStatic;
		buildingStatic.ID = id;
		buildingStatic.buildAs = "Building";
		buildingStatic.surfacePosition = eSurfacePosition::Ground;
		buildingStatic.factorGround = 1.f;
		buildingStatic.buildingData.isBig = isBig;
		unitsData.addData (buildingStatic);

		cDynamicUnitData building;
		building.setId (id);
		building.setHitpointsMax (10);
		building.setBuildCost (8);
		building.setMaximumCurrentValues();
		unitsData.addData (building);
	}

	//--------------------------------------------------------------------------
	std::shared_ptr<cUnitsData> makeUnitsData()
	{
		auto unitsData = std::make_shared<cUnitsData>();

		cStaticUnitData constructorStatic;
		constructorStatic.ID = constructorId;
		constructorStatic.factorGround = 1.f;
		constructorStatic.canBuild = "Building";
		constructorStatic.storeResType = eResourceType::Metal;
		constructorStatic.storageResMax = 40;
		unitsData->addData (constructorStatic);

		cDynamicUnitData constructor;
		constructor.setId (constructorId);
		constructor.setHitpointsMax (10);
		constructor.setSpeedMax (4);
		constructor.setMaximumCurrentValues();
		unitsData->addData (constructor);

		addBuildingData (*unitsData, bigBuildingId, true);
		addBuildingData (*unitsData, smallBuildingId, false);
		return unitsData;
	}

	//--------------------------------------------------------------------------
	// a constructor at (8, 8) is told to build: does it start?
	bool startBuild (const sID& buildingId, const cPosition& buildPosition)
	{
		cModel model;
		REQUIRE (initModel (model, makeUnitsData()));
		cPlayer& player = *model.getPlayer (0);
		auto& constructor = model.addVehicle (cPosition (8, 8), constructorId, &player);
		constructor.setStoredResources (20);

		cActionStartBuild action (constructor, buildingId, 0, buildPosition);
		action.From (player.getId());
		action.execute (model);
		return constructor.isUnitBuildingABuilding();
	}
} // namespace

//------------------------------------------------------------------------------
TEST_CASE ("BigBuildingIsBuiltOnTheFieldOfTheConstructor")
{
	// the four areas of 2x2 fields, that contain the field of the constructor
	CHECK (startBuild (bigBuildingId, cPosition (7, 7)));
	CHECK (startBuild (bigBuildingId, cPosition (8, 7)));
	CHECK (startBuild (bigBuildingId, cPosition (7, 8)));
	CHECK (startBuild (bigBuildingId, cPosition (8, 8)));

	// next to the field of the constructor, and far away
	CHECK_FALSE (startBuild (bigBuildingId, cPosition (9, 8)));
	CHECK_FALSE (startBuild (bigBuildingId, cPosition (8, 9)));
	CHECK_FALSE (startBuild (bigBuildingId, cPosition (6, 7)));
	CHECK_FALSE (startBuild (bigBuildingId, cPosition (7, 6)));
	CHECK_FALSE (startBuild (bigBuildingId, cPosition (2, 12)));
}

//------------------------------------------------------------------------------
TEST_CASE ("SmallBuildingIsBuiltOnTheFieldOfTheConstructor")
{
	CHECK (startBuild (smallBuildingId, cPosition (8, 8)));
	CHECK_FALSE (startBuild (smallBuildingId, cPosition (9, 8)));
}
