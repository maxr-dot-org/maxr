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

#include "game/data/gamesettings.h"
#include "game/data/map/map.h"
#include "game/data/model.h"
#include "game/data/player/player.h"
#include "game/data/player/playerbasicdata.h"
#include "game/data/units/building.h"
#include "game/data/units/unitdata.h"

#include <doctest.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <random>
#include <string>

namespace
{
	const sID buildingId{1, 100};

	//--------------------------------------------------------------------------
	void writeLE16 (std::ofstream& file, int value)
	{
		file.put (static_cast<char> (value & 0xFF));
		file.put (static_cast<char> ((value >> 8) & 0xFF));
	}

	//--------------------------------------------------------------------------
	// writes a map file (WRL format) of plain ground, with one terrain
	std::filesystem::path writeGroundMap (int size)
	{
		const auto filename = std::filesystem::temp_directory_path() / ("maxr_tests_" + std::to_string (std::random_device{}()) + ".wrl");
		std::ofstream file (filename, std::ios::binary);
		file.write ("WRL", 3);
		writeLE16 (file, 1);
		writeLE16 (file, size);
		writeLE16 (file, size);
		file << std::string (size * size, '\0'); // mini map
		file << std::string (size * size * 2, '\0'); // all fields use terrain 0
		writeLE16 (file, 1); // number of terrains
		file << std::string (64 * 64, '\0'); // terrain graphic
		file << std::string (256 * 3, '\0'); // palette
		file.put (0); // terrain 0: ground
		return filename;
	}
} // namespace

//------------------------------------------------------------------------------
// A client can send an upgrade for any building of its player: a building
// without a sub base (it does not connect to one: a mine, a road) must not
// crash the server (cActionUpgradeBuilding::execute() dereferenced
// building->subBase).
TEST_CASE ("UpgradeOfABuildingWithoutASubBaseIsRefused")
{
	auto staticMap = std::make_shared<cStaticMap>();
	const auto mapFilename = writeGroundMap (16);
	REQUIRE (staticMap->loadMap (mapFilename));
	std::filesystem::remove (mapFilename);

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

	cModel model;
	model.setUnitsData (unitsData);
	model.setGameSettings (cGameSettings{});
	model.setMap (staticMap);
	model.setPlayerList ({cPlayerBasicData ({"Player 0", cRgbColor::red()}, 0, false), cPlayerBasicData ({"Player 1", cRgbColor::blue()}, 1, false)});
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
