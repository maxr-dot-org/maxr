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
