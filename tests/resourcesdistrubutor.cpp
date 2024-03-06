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

#include "game/data/units/building.h"
#include "game/data/units/unitdata.h"
#include "game/logic/subbaseresourcedistribution.h"
#include "utility/listhelpers.h"

#include <doctest.h>
#include <iostream>

//------------------------------------------------------------------------------
template <typename OStream>
OStream& operator<< (OStream& os, const sMiningResource& res)
{
	return os << '{' << res.metal << ", " << res.oil << ", " << res.gold << '}';
}

//------------------------------------------------------------------------------
bool operator<(eResourceType lhs, eResourceType rhs)
{
	return int (lhs) < int (rhs);
}

namespace
{
	//--------------------------------------------------------------------------
	struct sMine
	{
		sMine (int metal, int oil, int gold) :
			sMine{{metal, oil, gold}} {}
		sMine (sMiningResource maxProd, int total = 16) :
			maxProd (maxProd)
		{
			staticData.buildingData.canMineMaxRes = total;
		}

		sMiningResource maxProd;
		cStaticUnitData staticData;
	};

} // namespace

//------------------------------------------------------------------------------
template <>
void cBuilding::serialize (const sMine& mine)
{
	isWorking = true;
	maxProd = mine.maxProd;
}

namespace
{
	//--------------------------------------------------------------------------
	std::unique_ptr<cBuilding> MakeBuilding (const sMine& mine)
	{
		auto ptr = std::make_unique<cBuilding> (&mine.staticData, nullptr, nullptr, 0);
		ptr->serialize (mine);
		return ptr;
	}

	//--------------------------------------------------------------------------
	std::vector<std::unique_ptr<cBuilding>> MakeBuildings (const std::vector<sMine>& mines)
	{
		return ranges::Transform (mines, [] (const sMine& mine) { return MakeBuilding (mine); });
	}

	//--------------------------------------------------------------------------
	sMiningResource makeOrdered (eResourceType (&indexes)[3], const sMiningResource& res)
	{
		return {res.get (indexes[0]), res.get (indexes[1]), res.get (indexes[2])};
	}

	//--------------------------------------------------------------------------
	sMine makeOrdered (eResourceType (&indexes)[3], const sMine& mine)
	{
		sMine res{mine};

		res.maxProd = makeOrdered (indexes, mine.maxProd);
		return res;
	}

	//--------------------------------------------------------------------------
	template <typename T>
	std::vector<T> makeOrdered (eResourceType (&indexes)[3], const std::vector<T>& v)
	{
		std::vector<T> res{v};

		for (auto& e : res)
		{
			e = makeOrdered (indexes, e);
		}

		return res;
	}

	//--------------------------------------------------------------------------
	void check (const cBuilding& mine)
	{
		REQUIRE (mine.getMaxProd().gold <= mine.getStaticData().canMineMaxRes);
		REQUIRE (mine.getMaxProd().metal <= mine.getStaticData().canMineMaxRes);
		REQUIRE (mine.getMaxProd().oil <= mine.getStaticData().canMineMaxRes);

		CHECK (mine.prod.total() <= mine.getStaticData().canMineMaxRes);

		CHECK (0 <= mine.prod.gold);
		CHECK (0 <= mine.prod.metal);
		CHECK (0 <= mine.prod.oil);

		CHECK (mine.prod.gold <= mine.getMaxProd().gold);
		CHECK (mine.prod.metal <= mine.getMaxProd().metal);
		CHECK (mine.prod.oil <= mine.getMaxProd().oil);
	}

	//--------------------------------------------------------------------------
	void check (const std::vector<cBuilding*>& mines)
	{
		for (const auto* mine : mines)
		{
			check (*mine);
		}
	}

	//--------------------------------------------------------------------------
	void CheckAnyResOrder (const sMiningResource& expected, const std::vector<sMine>& mines)
	{
		eResourceType indexes[3] = {eResourceType::Metal, eResourceType::Oil, eResourceType::Gold};
		std::sort (std::begin (indexes), std::end (indexes));

		do
		{
			const sMiningResource orderedExpected = makeOrdered (indexes, expected);
			std::vector<sMine> orderedMines = makeOrdered (indexes, mines);

			auto buildingPtrs = MakeBuildings (orderedMines);
			auto buildings = ExtractPtrs (buildingPtrs);

			setBuildingsProduction (buildings, orderedExpected);

			check (buildings);
			CHECK (orderedExpected == computeProduction (buildings));
		} while (std::next_permutation (std::begin (indexes), std::end (indexes)));
	}
} // namespace

//------------------------------------------------------------------------------
TEST_CASE ("TwoResConflict")
{
	const sMiningResource expected{16, 16, 0};
	std::vector<sMine> mines{sMine{16, 8, 0}, sMine{10, 10, 0}};

	CheckAnyResOrder (expected, mines);
}

//------------------------------------------------------------------------------
TEST_CASE ("ThreeResConflict")
{
	const sMiningResource expected{16, 12, 4};
	std::vector<sMine> mines{sMine{16, 8, 0}, sMine{8, 8, 0}, sMine{{0, 4, 4}, 4}};

	CheckAnyResOrder (expected, mines);
}
