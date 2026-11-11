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
#include "game/data/units/building.h"
#include "game/data/units/unitdata.h"
#include "game/data/units/vehicle.h"
#include "game/logic/action/actionupgradebuilding.h"
#include "testutility.h"

#include <doctest.h>
#include <memory>

namespace
{
	const sID detectorBuildingId{1, 100};
	const sID stealthVehicleId{0, 100};

	//--------------------------------------------------------------------------
	std::shared_ptr<cUnitsData> makeUnitsData()
	{
		auto unitsData = std::make_shared<cUnitsData>();

		// no building of the standard unit data detects stealth units
		cStaticUnitData detectorStatic;
		detectorStatic.ID = detectorBuildingId;
		detectorStatic.surfacePosition = eSurfacePosition::Ground;
		detectorStatic.buildingData.connectsToBase = true; // an upgrade needs a sub base
		detectorStatic.canDetectStealthOn = eTerrainFlag::Ground;
		unitsData->addData (detectorStatic);

		cDynamicUnitData detector;
		detector.setId (detectorBuildingId);
		detector.setHitpointsMax (10);
		detector.setScan (3);
		detector.setMaximumCurrentValues();
		unitsData->addData (detector);

		cStaticUnitData stealthStatic;
		stealthStatic.ID = stealthVehicleId;
		stealthStatic.factorGround = 1.f;
		stealthStatic.isStealthOn = eTerrainFlag::Ground;
		unitsData->addData (stealthStatic);

		cDynamicUnitData stealth;
		stealth.setId (stealthVehicleId);
		stealth.setHitpointsMax (10);
		stealth.setSpeedMax (4);
		stealth.setScan (1);
		stealth.setMaximumCurrentValues();
		unitsData->addData (stealth);

		return unitsData;
	}
} // namespace

//------------------------------------------------------------------------------
TEST_CASE ("UpgradeBuildingDetectsStealthUnitsInTheNewScanRange")
{
	cModel model;
	REQUIRE (initModel (model, makeUnitsData()));
	cPlayer& player = *model.getPlayer (0);
	cPlayer& enemy = *model.getPlayer (1);

	// a building, that detects stealth units on ground, at (3, 8): scan 3, after the upgrade 6
	const auto& nearVehicle = model.addVehicle (cPosition (5, 8), stealthVehicleId, &enemy); // distance 2
	const auto& ringVehicle = model.addVehicle (cPosition (8, 8), stealthVehicleId, &enemy); // distance 5
	const auto& farVehicle = model.addVehicle (cPosition (13, 8), stealthVehicleId, &enemy); // distance 10
	const auto& detector = model.addBuilding (cPosition (3, 8), detectorBuildingId, &player);

	CHECK (nearVehicle.isDetectedByPlayer (&player));
	CHECK_FALSE (ringVehicle.isDetectedByPlayer (&player));
	CHECK_FALSE (farVehicle.isDetectedByPlayer (&player));

	cDynamicUnitData& upgradedData = *player.getLastUnitData (detectorBuildingId);
	upgradedData.setScan (6);
	upgradedData.makeVersionDirty();

	cActionUpgradeBuilding action (detector, false);
	action.From (player.getId());
	action.execute (model);

	REQUIRE (detector.data.getScan() == 6);
	CHECK (player.hasLandDetection (ringVehicle.getPosition()));
	CHECK (nearVehicle.isDetectedByPlayer (&player));
	CHECK (ringVehicle.isDetectedByPlayer (&player));
	CHECK_FALSE (farVehicle.isDetectedByPlayer (&player));
}

//------------------------------------------------------------------------------
TEST_CASE ("DetectorSearchesWhenItRecoversFromBeingDisabled")
{
	cModel model;
	REQUIRE (initModel (model, makeUnitsData()));
	cPlayer& player = *model.getPlayer (0);
	cPlayer& enemy = *model.getPlayer (1);

	auto& detector = model.addBuilding (cPosition (3, 8), detectorBuildingId, &player);
	// disabled for one turn, as by an infiltrator (cActionStealDisable)
	detector.setDisabledTurns (1);
	player.removeFromScan (detector);

	// a stealth vehicle in the range of the disabled detector
	const auto& vehicle = model.addVehicle (cPosition (5, 8), stealthVehicleId, &enemy);
	CHECK_FALSE (vehicle.isDetectedByPlayer (&player));

	player.makeTurnStart (model);

	REQUIRE_FALSE (detector.isDisabled());
	CHECK (player.hasLandDetection (vehicle.getPosition()));
	CHECK (vehicle.isDetectedByPlayer (&player));
}
