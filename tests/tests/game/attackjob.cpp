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

#include "game/logic/attackjob.h"

#include "game/data/model.h"
#include "game/data/player/player.h"
#include "game/data/units/unitdata.h"
#include "game/data/units/vehicle.h"
#include "testutility.h"

#include <doctest.h>
#include <memory>

namespace
{
	const sID rocketLauncherId{0, 100};
	const sID tankId{0, 101};

	//--------------------------------------------------------------------------
	void addVehicleData (cUnitsData& unitsData, const sID& id, eMuzzleType muzzleType)
	{
		cStaticUnitData staticData;
		staticData.ID = id;
		staticData.factorGround = 1.f;
		staticData.muzzleType = muzzleType;
		staticData.canAttack = eTerrainFlag::Ground;
		unitsData.addData (staticData);

		cDynamicUnitData data;
		data.setId (id);
		data.setHitpointsMax (10);
		data.setArmor (1);
		data.setSpeedMax (4);
		data.setScan (12);
		data.setRange (12);
		data.setDamage (5);
		data.setShotsMax (1);
		data.setAmmoMax (1);
		data.setMaximumCurrentValues();
		unitsData.addData (data);
	}

	//--------------------------------------------------------------------------
	std::shared_ptr<cUnitsData> makeUnitsData()
	{
		auto unitsData = std::make_shared<cUnitsData>();
		addVehicleData (*unitsData, rocketLauncherId, eMuzzleType::Rocket);
		addVehicleData (*unitsData, tankId, eMuzzleType::Big);
		return unitsData;
	}

	//--------------------------------------------------------------------------
	void advanceGameTime (cModel& model, int gameTimes)
	{
		for (int i = 0; i != gameTimes; ++i)
		{
			model.advanceGameTime();
		}
	}

	//--------------------------------------------------------------------------
	// as the impact of an attack job destroys a unit
	void destroy (cModel& model, cUnit& unit)
	{
		unit.data.setHitpoints (0);
		model.destroyUnit (unit);
	}
} // namespace

//------------------------------------------------------------------------------
TEST_CASE ("AttackJobEndsWhenTheAggressorIsRemovedWhileItsRocketFlies")
{
	cModel model;
	REQUIRE (initModel (model, makeUnitsData()));
	cPlayer& player = *model.getPlayer (0);
	cPlayer& enemy = *model.getPlayer (1);

	auto& rocketLauncher = model.addVehicle (cPosition (1, 8), rocketLauncherId, &player);
	const auto& target = model.addVehicle (cPosition (12, 8), tankId, &enemy);
	const auto aggressorId = rocketLauncher.getId();

	model.addAttackJob (rocketLauncher, target.getPosition());
	CHECK (target.isBeingAttacked());

	// rotate to the target and fire the rocket
	for (int i = 0; i != 100 && rocketLauncher.data.getAmmo() != 0; ++i)
	{
		model.advanceGameTime();
	}
	REQUIRE (rocketLauncher.data.getAmmo() == 0);

	// the rocket launcher is removed while its rocket flies,
	// as a unit destroyed by another attack in the meantime (cDestroyJob)
	model.deleteUnit (&rocketLauncher);
	REQUIRE (model.getUnitFromID (aggressorId) == nullptr);

	for (int i = 0; i != 200; ++i)
	{
		model.advanceGameTime();
	}
	CHECK_FALSE (target.isBeingAttacked());
}

//------------------------------------------------------------------------------
TEST_CASE ("AttackJobOfADestroyedAggressorEndsWithoutFiring")
{
	cModel model;
	REQUIRE (initModel (model, makeUnitsData()));
	cPlayer& player = *model.getPlayer (0);
	cPlayer& enemy = *model.getPlayer (1);

	auto& tank = model.addVehicle (cPosition (2, 8), tankId, &player);
	const auto& target = model.addVehicle (cPosition (6, 8), tankId, &enemy);
	const auto aggressorId = tank.getId();
	int attackReports = 0;
	enemy.unitAttacked.connect ([&] (const cUnit&) { ++attackReports; });

	// the tank faces north: it has to turn east before it fires
	REQUIRE (tank.dir == EDirection::North);
	model.addAttackJob (tank, target.getPosition());
	advanceGameTime (model, 5);
	REQUIRE (tank.data.getAmmo() == 1);

	// the tank is destroyed while it turns
	destroy (model, tank);

	advanceGameTime (model, 200);
	REQUIRE (model.getUnitFromID (aggressorId) == nullptr);
	CHECK (target.data.getHitpoints() == target.data.getHitpointsMax());
	CHECK (attackReports == 0);
	CHECK_FALSE (target.isBeingAttacked());
}

//------------------------------------------------------------------------------
TEST_CASE ("RocketHitsWhenItsLauncherIsDestroyedAndRemovedBeforeTheImpact")
{
	cModel model;
	REQUIRE (initModel (model, makeUnitsData()));
	cPlayer& player = *model.getPlayer (0);
	cPlayer& enemy = *model.getPlayer (1);

	auto& rocketLauncher = model.addVehicle (cPosition (1, 8), rocketLauncherId, &player);
	const auto& target = model.addVehicle (cPosition (12, 8), tankId, &enemy);
	const auto aggressorId = rocketLauncher.getId();
	int attackReports = 0;
	enemy.unitAttacked.connect ([&] (const cUnit&) { ++attackReports; });

	model.addAttackJob (rocketLauncher, target.getPosition());

	// rotate to the target and fire the rocket
	for (int i = 0; i != 100 && rocketLauncher.data.getAmmo() != 0; ++i)
	{
		model.advanceGameTime();
	}
	REQUIRE (rocketLauncher.data.getAmmo() == 0);

	// the rocket launcher is destroyed while its rocket flies 11 fields:
	// cDestroyJob removes it before the impact
	destroy (model, rocketLauncher);

	advanceGameTime (model, 200);
	REQUIRE (model.getUnitFromID (aggressorId) == nullptr);
	// damage 5 - armor 1
	CHECK (target.data.getHitpoints() == target.data.getHitpointsMax() - 4);
	CHECK (attackReports == 1);
	CHECK_FALSE (target.isBeingAttacked());
}

//------------------------------------------------------------------------------
TEST_CASE ("AttackJobFiresInTheGameTimeInWhichItsAggressorIsDestroyed")
{
	cModel model;
	REQUIRE (initModel (model, makeUnitsData()));
	cPlayer& player = *model.getPlayer (0);
	cPlayer& enemy = *model.getPlayer (1);

	auto& tank = model.addVehicle (cPosition (2, 8), tankId, &player);
	auto& rocketLauncher = model.addVehicle (cPosition (12, 8), rocketLauncherId, &enemy);
	const auto tankUnitId = tank.getId();
	const auto rocketLauncherUnitId = rocketLauncher.getId();
	tank.dir = EDirection::East;
	rocketLauncher.dir = EDirection::West;
	// one hit destroys each of them
	tank.data.setHitpoints (1);
	rocketLauncher.data.setHitpoints (1);
	int playerDestroyedReports = 0;
	int enemyDestroyedReports = 0;
	player.unitDestroyed.connect ([&] (const cUnit&) { ++playerDestroyedReports; });
	enemy.unitDestroyed.connect ([&] (const cUnit&) { ++enemyDestroyedReports; });

	// The tank fires 10 game times after its attack began, its shell hits 16 game times later.
	// The attack of the rocket launcher begins 16 game times after the one of the tank:
	// it fires in the game time of the impact of the shell, after the attack job of the tank ran.
	model.addAttackJob (tank, rocketLauncher.getPosition());
	advanceGameTime (model, 16);
	model.addAttackJob (rocketLauncher, tank.getPosition());
	advanceGameTime (model, 10);
	REQUIRE (rocketLauncher.data.getHitpoints() == 0);
	CHECK (rocketLauncher.data.getAmmo() == 0);

	// the rocket flies 10 fields: cDestroyJob removes the rocket launcher before the impact
	advanceGameTime (model, 200);
	CHECK (model.getUnitFromID (rocketLauncherUnitId) == nullptr);
	CHECK (model.getUnitFromID (tankUnitId) == nullptr);
	CHECK (enemyDestroyedReports == 1);
	CHECK (playerDestroyedReports == 1);
	CHECK (enemy.getGameOverStat().lostVehiclesCount == 1);
	CHECK (player.getGameOverStat().lostVehiclesCount == 1);
}
