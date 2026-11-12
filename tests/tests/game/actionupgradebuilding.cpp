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

#include "game/logic/action/actionupgradebuilding.h"

#include "game/data/model.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/unitdata.h"
#include "testutility.h"

#include <doctest.h>
#include <memory>

namespace
{
	const sID buildingId{1, 100};

	//--------------------------------------------------------------------------
	std::shared_ptr<cUnitsData> makeUnitsData()
	{
		auto unitsData = std::make_shared<cUnitsData>();

		cStaticUnitData buildingStatic;
		buildingStatic.ID = buildingId;
		buildingStatic.surfacePosition = eSurfacePosition::Ground;
		// connectsToBase stays false: such a building has no sub base
		unitsData->addData (buildingStatic);

		cDynamicUnitData buildingData;
		buildingData.setId (buildingId);
		buildingData.setHitpointsMax (10);
		buildingData.setScan (3);
		buildingData.setMaximumCurrentValues();
		unitsData->addData (buildingData);

		return unitsData;
	}
} // namespace

//------------------------------------------------------------------------------
// A client can send an upgrade for any building of its player: a building
// without a sub base (it does not connect to one: a mine, a road) must not
// crash the server (cActionUpgradeBuilding::execute() dereferenced
// building->subBase).
TEST_CASE ("UpgradeOfABuildingWithoutASubBaseIsRefused")
{
	cModel model;
	REQUIRE (initModel (model, makeUnitsData()));
	cPlayer& player = *model.getPlayer (0);

	const auto& building = model.addBuilding (cPosition (3, 8), buildingId, &player);
	REQUIRE (building.subBase == nullptr);

	cDynamicUnitData& upgradedData = *player.getLastUnitData (buildingId);
	upgradedData.setScan (6);
	upgradedData.makeVersionDirty();

	cActionUpgradeBuilding action (building, false);
	action.From (player.getId());
	CHECK_NOTHROW (action.execute (model));

	// the action is refused: the building keeps its scan range
	CHECK (building.data.getScan() == 3);
}
