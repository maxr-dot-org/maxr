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

#include "game/data/units/unitdata.h"
#include "utility/serialization/binaryarchive.h"
#include "utility/serialization/jsonarchive.h"

#include <doctest.h>
#include <vector>

//------------------------------------------------------------------------------
// The current unit versions of a player (cPlayer::dynamicUnitsData) are resynced with a
// binary archive: a dirty version (upgraded, not yet used) must stay dirty, like in a saved game.
TEST_CASE ("DynamicUnitDataVectorKeepsDirtyVersion")
{
	std::vector<cDynamicUnitData> expected (2);
	expected[1].makeVersionDirty();
	const cDynamicUnitData clean;
	REQUIRE (!clean.canBeUpgradedTo (expected[0]));
	REQUIRE (clean.canBeUpgradedTo (expected[1]));

	SUBCASE ("binary")
	{
		std::vector<unsigned char> buffer;
		cBinaryArchiveOut out (buffer);
		out << serialization::makeNvp ("data", expected);

		std::vector<cDynamicUnitData> loaded;
		cBinaryArchiveIn in (buffer.data(), buffer.size());
		in >> serialization::makeNvp ("data", loaded);

		REQUIRE (loaded.size() == 2);
		CHECK (!clean.canBeUpgradedTo (loaded[0]));
		CHECK (clean.canBeUpgradedTo (loaded[1]));
		CHECK (loaded[1].getChecksum (0) == expected[1].getChecksum (0));
	}
	SUBCASE ("json")
	{
		nlohmann::json json;
		cJsonArchiveOut out (json);
		out << serialization::makeNvp ("data", expected);

		std::vector<cDynamicUnitData> loaded;
		cJsonArchiveIn in (json);
		in >> serialization::makeNvp ("data", loaded);

		REQUIRE (loaded.size() == 2);
		CHECK (!clean.canBeUpgradedTo (loaded[0]));
		CHECK (clean.canBeUpgradedTo (loaded[1]));
		CHECK (loaded[1].getChecksum (0) == expected[1].getChecksum (0));
	}
}
