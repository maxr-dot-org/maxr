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

#include "game/data/base/base.h"
#include "game/data/model.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/unitdata.h"
#include "game/data/units/vehicle.h"
#include "game/logic/action/actiontransfer.h"
#include "testutility.h"

#include <doctest.h>
#include <memory>
#include <vector>

namespace
{
	const sID truckId{0, 100};
	const sID tankerId{0, 101};
	const sID metalStoreId{1, 100};
	const sID oilTankId{1, 101};
	const sID metalStoreWithoutBaseId{1, 102};

	//--------------------------------------------------------------------------
	void addUnitData (cUnitsData& unitsData, const sID& id, eResourceType storeResType, int storageResMax, bool connectsToBase)
	{
		cStaticUnitData staticData;
		staticData.ID = id;
		staticData.factorGround = 1.f;
		staticData.surfacePosition = eSurfacePosition::Ground;
		staticData.storeResType = storeResType;
		staticData.storageResMax = storageResMax;
		staticData.buildingData.connectsToBase = connectsToBase;
		unitsData.addData (staticData);

		cDynamicUnitData data;
		data.setId (id);
		data.setHitpointsMax (10);
		data.setSpeedMax (id.isAVehicle() ? 4 : 0);
		data.setMaximumCurrentValues();
		unitsData.addData (data);
	}

	//--------------------------------------------------------------------------
	std::shared_ptr<cUnitsData> makeUnitsData()
	{
		auto unitsData = std::make_shared<cUnitsData>();
		addUnitData (*unitsData, truckId, eResourceType::Metal, 50, false);
		addUnitData (*unitsData, tankerId, eResourceType::Oil, 50, false);
		addUnitData (*unitsData, metalStoreId, eResourceType::Metal, 100, true);
		addUnitData (*unitsData, oilTankId, eResourceType::Oil, 100, true);
		addUnitData (*unitsData, metalStoreWithoutBaseId, eResourceType::Metal, 100, false);
		return unitsData;
	}

	//--------------------------------------------------------------------------
	cVehicle& addVehicle (cModel& model, sID id, cPosition position, cPlayer& player)
	{
		auto& vehicle = model.addVehicle (position, id, &player);
		vehicle.setStoredResources (10);
		return vehicle;
	}

	//--------------------------------------------------------------------------
	std::vector<int> getResources (const cUnit& unit)
	{
		std::vector<int> resources{unit.getStoredResources()};
		if (const auto* building = dynamic_cast<const cBuilding*> (&unit); building && building->subBase)
		{
			const sMiningResource& stored = building->subBase->getResourcesStored();
			resources.insert (resources.end(), {stored.metal, stored.oil, stored.gold});
		}
		return resources;
	}

	//--------------------------------------------------------------------------
	// player 0 transfers one unit of the resource, that the game window chooses
	// (the one of the vehicle): does the server do it?
	bool transfer (cModel& model, const cUnit& source, const cUnit& destination)
	{
		const auto resourceType = (source.isAVehicle() ? source : destination).getStaticUnitData().storeResType;
		const auto sourceResources = getResources (source);
		const auto destinationResources = getResources (destination);

		cActionTransfer action (source, destination, 1, resourceType);
		action.From (0);
		action.execute (model);
		return getResources (source) != sourceResources || getResources (destination) != destinationResources;
	}
} // namespace

//------------------------------------------------------------------------------
// The server transfers, exactly when the game window offers the transfer.
TEST_CASE ("TransferBetweenVehicles")
{
	cModel model;
	REQUIRE (initModel (model, makeUnitsData()));
	cPlayer& player = *model.getPlayer (0);
	cPlayer& enemy = *model.getPlayer (1);

	auto& truck = addVehicle (model, truckId, cPosition (4, 4), player);
	const auto& nextTruck = addVehicle (model, truckId, cPosition (5, 4), player);
	const auto& farTruck = addVehicle (model, truckId, cPosition (8, 4), player);
	const auto& enemyTruck = addVehicle (model, truckId, cPosition (4, 5), enemy);
	const auto& tanker = addVehicle (model, tankerId, cPosition (3, 4), player);

	CHECK (truck.canTransferTo (nextTruck));
	CHECK (transfer (model, truck, nextTruck));
	CHECK_FALSE (truck.canTransferTo (farTruck));
	CHECK_FALSE (transfer (model, truck, farTruck));
	CHECK_FALSE (truck.canTransferTo (enemyTruck));
	CHECK_FALSE (transfer (model, truck, enemyTruck));
	CHECK_FALSE (truck.canTransferTo (tanker));
	CHECK_FALSE (transfer (model, truck, tanker));
	// a unit of another player
	CHECK_FALSE (transfer (model, enemyTruck, truck));

	truck.setDisabledTurns (1);
	CHECK_FALSE (truck.canTransferTo (nextTruck));
	CHECK_FALSE (transfer (model, truck, nextTruck));
	truck.setDisabledTurns (0);

	truck.setBuildingABuilding (true);
	CHECK_FALSE (truck.canTransferTo (nextTruck));
	CHECK_FALSE (transfer (model, truck, nextTruck));
	truck.setBuildingABuilding (false);

	CHECK (truck.canTransferTo (nextTruck));
	CHECK (transfer (model, truck, nextTruck));
}

//------------------------------------------------------------------------------
// The server transfers, exactly when the game window offers the transfer.
TEST_CASE ("TransferBetweenBuildingsAndVehicles")
{
	cModel model;
	REQUIRE (initModel (model, makeUnitsData()));
	cPlayer& player = *model.getPlayer (0);
	cPlayer& enemy = *model.getPlayer (1);

	// a sub base: metal store, oil tank, metal store in a row; another sub base far away
	auto& store = model.addBuilding (cPosition (4, 4), metalStoreId, &player);
	model.addBuilding (cPosition (5, 4), oilTankId, &player);
	const auto& lastStore = model.addBuilding (cPosition (6, 4), metalStoreId, &player);
	const auto& farStore = model.addBuilding (cPosition (12, 12), metalStoreId, &player);
	const auto& enemyStore = model.addBuilding (cPosition (2, 4), metalStoreId, &enemy);
	REQUIRE (store.subBase);
	REQUIRE (store.subBase == lastStore.subBase);
	store.subBase->addMetal (10);
	store.subBase->addOil (10);
	farStore.subBase->addMetal (10);

	const auto& truck = addVehicle (model, truckId, cPosition (3, 3), player); // next to the first store
	const auto& truckAtTheEnd = addVehicle (model, truckId, cPosition (7, 4), player); // next to the last store
	const auto& farTruck = addVehicle (model, truckId, cPosition (12, 2), player);
	const auto& tanker = addVehicle (model, tankerId, cPosition (4, 3), player); // next to the first store

	CHECK (truck.canTransferTo (store));
	CHECK (transfer (model, truck, store));
	CHECK_FALSE (farTruck.canTransferTo (store));
	CHECK_FALSE (transfer (model, farTruck, store));
	CHECK_FALSE (truck.canTransferTo (enemyStore));
	CHECK_FALSE (transfer (model, truck, enemyStore));

	CHECK (store.canTransferTo (truck));
	CHECK (transfer (model, store, truck));
	CHECK (store.canTransferTo (truckAtTheEnd));
	CHECK (transfer (model, store, truckAtTheEnd));
	CHECK_FALSE (store.canTransferTo (farTruck));
	CHECK_FALSE (transfer (model, store, farTruck));
	CHECK_FALSE (farStore.canTransferTo (truck));
	CHECK_FALSE (transfer (model, farStore, truck));

	CHECK (store.canTransferTo (lastStore));
	CHECK (transfer (model, store, lastStore));
	CHECK_FALSE (store.canTransferTo (farStore));
	CHECK_FALSE (transfer (model, store, farStore));

	// the oil of the sub base: from the tanker, not from the metal store
	CHECK (tanker.canTransferTo (store));
	CHECK (transfer (model, tanker, store));
	CHECK_FALSE (store.canTransferTo (tanker));
	CHECK_FALSE (transfer (model, store, tanker));

	store.setDisabledTurns (1);
	CHECK_FALSE (store.canTransferTo (truck));
	CHECK_FALSE (transfer (model, store, truck));
}

//------------------------------------------------------------------------------
TEST_CASE ("TransferFromABuildingWithoutSubBase")
{
	cModel model;
	REQUIRE (initModel (model, makeUnitsData()));
	cPlayer& player = *model.getPlayer (0);

	auto& store = model.addBuilding (cPosition (4, 4), metalStoreWithoutBaseId, &player);
	store.setStoredResources (10);
	const auto& truck = addVehicle (model, truckId, cPosition (5, 4), player);
	REQUIRE_FALSE (store.subBase);

	CHECK_FALSE (store.canTransferTo (truck));
	CHECK_FALSE (transfer (model, store, truck));
}
