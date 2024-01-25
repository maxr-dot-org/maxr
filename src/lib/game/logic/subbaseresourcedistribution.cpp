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

#include "subbaseresourcedistribution.h"

#include "game/data/base/base.h"
#include "game/data/miningresource.h"
#include "game/data/units/building.h"
#include "utility/listhelpers.h"
#include "utility/ranges.h"

namespace
{

	//--------------------------------------------------------------------------
	[[nodiscard]] std::vector<cBuilding*> ExtractOnLineMiningStations (std::vector<cBuilding*> buildings)
	{
		return Filter (buildings, [] (const cBuilding* building) { return building->getStaticData().canMineMaxRes > 0 && building->isUnitWorking(); });
	}

	//--------------------------------------------------------------------------
	struct sResourcesLimit
	{
		sMiningResource min; //<! minimal, assuming maximum for others
		sMiningResource extraForOthers; //!< extra amount of resource which can be distribute freely to other resource without impact
		sMiningResource max; //<! total of maxProd
	};

	//--------------------------------------------------------------------------
	sResourcesLimit computeResourcesLimit (const std::vector<cBuilding*>& buildings)
	{
		sResourcesLimit res;

		for (const cBuilding* building : buildings)
		{
			const auto total = building->getStaticData().canMineMaxRes;
			if (total <= 0 || !building->isUnitWorking()) continue;
			const int metal = building->getMaxProd().get (eResourceType::Metal);
			const int oil = building->getMaxProd().get (eResourceType::Oil);
			const int gold = building->getMaxProd().get (eResourceType::Gold);

			// resources amount which doesn't decrease impose limit on other resources
			const int minMetal = std::max (0, std::min (metal, total - oil - gold));
			const int minOil = std::max (0, std::min (oil, total - metal - gold));
			const int minGold = std::max (0, std::min (gold, total - metal - oil));

			// extra resource amount (above above minimal) which can be distribute to other resources without impact on current resource
			const int freeNoMetal = std::min ({total - metal - minOil, oil - minOil, total - metal - minGold, gold - minGold});
			const int freeNoOil = std::min ({total - oil - minMetal, metal - minMetal, total - oil - minGold, gold - minGold});
			const int freeNoGold = std::min ({total - gold - minOil, oil - minOil, total - gold - minMetal, metal - minMetal});

			res.min.metal += minMetal;
			res.min.oil += minOil;
			res.min.gold += minGold;
			res.extraForOthers.metal += freeNoMetal;
			res.extraForOthers.oil += freeNoOil;
			res.extraForOthers.gold += freeNoGold;
			res.max.metal += metal;
			res.max.oil += oil;
			res.max.gold += gold;
		}

		return res;
	}

	//--------------------------------------------------------------------------
	sMiningResource calcMaxAllowedProduction (const sResourcesLimit& limits, const sMiningResource& current)
	{
		sMiningResource res;

		int metalToDistribute = std::max (0, current.metal - limits.min.metal);
		int oilToDistribute = std::max (0, current.oil - limits.min.oil);
		int goldToDistribute = std::max (0, current.gold - limits.min.gold);

		res.metal = limits.max.metal - std::max (0, oilToDistribute + goldToDistribute - limits.extraForOthers.metal);
		res.oil = limits.max.oil - std::max (0, metalToDistribute + goldToDistribute - limits.extraForOthers.oil);
		res.gold = limits.max.gold - std::max (0, oilToDistribute + metalToDistribute - limits.extraForOthers.gold);

		return res;
	}

	//--------------------------------------------------------------------------
	sMiningResource adjustResourceToMaxAllowed (const sResourcesLimit& limits, const sMiningResource& wanted, const eResourceType (&order)[3])
	{
		sMiningResource prod;

		auto allowed = calcMaxAllowedProduction (limits, prod);
		prod.get (order[0]) = std::min (wanted.get (order[0]), allowed.get (order[0]));

		allowed = calcMaxAllowedProduction (limits, prod);
		prod.get (order[1]) = std::min (wanted.get (order[1]), allowed.get (order[1]));

		allowed = calcMaxAllowedProduction (limits, prod);
		prod.get (order[2]) = std::min (wanted.get (order[2]), allowed.get (order[2]));

		return prod;
	}

	//--------------------------------------------------------------------------
	bool canIncreaseProd (const cBuilding* b)
	{
		return b->prod.total() < b->getStaticData().canMineMaxRes;
	}

	//--------------------------------------------------------------------------
	auto canDecreaseRes (eResourceType res)
	{
		return [=] (const cBuilding* b) { return 0 < b->prod.get (res); };
	}

	//--------------------------------------------------------------------------
	auto canIncreaseRes (eResourceType res)
	{
		return [=] (const cBuilding* b) { return b->prod.get (res) < b->getMaxProd().get (res); };
	}

	//--------------------------------------------------------------------------
	template <typename F1, typename F2>
	auto combine (F1 f1, F2 f2)
	{
		return [=] (auto&& e) { return f1 (e) && f2 (e); };
	}

	//--------------------------------------------------------------------------
	// increase production of `res1` by `prod` by transfering `res2` production
	void fixConflict (cBuilding& b, std::vector<cBuilding*>& mines, int& prod, eResourceType res1, eResourceType res2)
	{
		const auto canIncreaseRes1 = canIncreaseRes (res1);
		const auto canDecreaseRes2 = canDecreaseRes (res2);

		while (prod != 0 && canIncreaseRes1 (&b) && canDecreaseRes2 (&b))
		{
			auto it = ranges::find_if (mines, combine (canIncreaseRes (res2), canIncreaseProd));

			if (it == mines.end()) break;
			auto& b2 = **it;
			const auto transfer = std::min ({b2.getMaxProd().get (res2) - b2.prod.get (res2), b.prod.get (res2), b.getMaxProd().get (res1) - b.prod.get (res1), prod});
			b2.prod.get (res2) += transfer;
			b.prod.get (res2) -= transfer;
			b.prod.get (res1) += transfer;
			prod -= transfer;
		}
	}

	//--------------------------------------------------------------------------
	// increase production of `res1` by `prod` by transfering `res2` production
	void fixConflict2 (cBuilding& b, std::vector<cBuilding*>& mines, int& prod, eResourceType res1, eResourceType res2, eResourceType res3)
	{
		const auto canIncreaseRes1 = canIncreaseRes (res1);
		const auto canDecreaseRes2 = canDecreaseRes (res2);

		while (prod != 0 && canIncreaseRes1 (&b) && canDecreaseRes2 (&b))
		{
			auto it2 = ranges::find_if (mines, combine (canIncreaseRes (res2), canDecreaseRes (res3)));
			if (it2 == mines.end()) break;
			auto& b2 = **it2;

			auto it3 = ranges::find_if (mines, combine (canIncreaseRes (res3), canIncreaseProd));
			if (it3 == mines.end()) break;
			auto& b3 = **it3;

			const auto transfer = std::min ({b3.getMaxProd().get (res3) - b3.prod.get (res3), b2.prod.get (res3), b2.getMaxProd().get (res2) - b2.prod.get (res2), b.prod.get (res2), b.getMaxProd().get (res1) - b.prod.get (res1), prod});
			b3.prod.get (res3) += transfer;
			b2.prod.get (res3) -= transfer;
			b2.prod.get (res2) += transfer;
			b.prod.get (res2) -= transfer;
			b.prod.get (res1) += transfer;
			prod -= transfer;
		}
	}

	//--------------------------------------------------------------------------
	void setMinesProduction (std::vector<cBuilding*>& mines, sMiningResource prod)
	{
		// First pass, fulfil building to max (up to requirement), metal then oil, then gold
		for (cBuilding* mine : mines)
		{
			auto free = mine->getStaticData().canMineMaxRes;
			const auto& maxProd = mine->getMaxProd();
			mine->prod.metal = std::min (prod.metal, maxProd.metal);
			free -= mine->prod.metal;
			mine->prod.oil = std::max (0, std::min ({prod.oil, maxProd.oil, free}));
			free -= mine->prod.oil;
			mine->prod.gold = std::max (0, std::min ({prod.gold, maxProd.gold, free}));
			prod -= mine->prod;
		}
		// Second pass, handle oil/metal conflicts
		for (auto& mine : mines)
		{
			if (prod.oil == 0) break;
			fixConflict (*mine, mines, prod.oil, eResourceType::Oil, eResourceType::Metal);
		}
		// Third pass, handle gold/metal and gold/oil conflicts
		for (auto& mine : mines)
		{
			if (prod.gold == 0) break;
			fixConflict (*mine, mines, prod.gold, eResourceType::Gold, eResourceType::Metal);
			fixConflict (*mine, mines, prod.gold, eResourceType::Gold, eResourceType::Oil);
			fixConflict2 (*mine, mines, prod.gold, eResourceType::Gold, eResourceType::Oil, eResourceType::Metal);
			fixConflict2 (*mine, mines, prod.gold, eResourceType::Gold, eResourceType::Metal, eResourceType::Oil);
		}
	}

} // namespace

//------------------------------------------------------------------------------
sMiningResource computeMaxAllowedProduction (const cSubBase& subBase, const sMiningResource& prod)
{
	return calcMaxAllowedProduction (computeResourcesLimit (subBase.getBuildings()), prod);
}

//------------------------------------------------------------------------------
sMiningResource computeProduction (const std::vector<cBuilding*>& buildings)
{
	sMiningResource res;

	for (const cBuilding* building : buildings)
	{
		const auto total = building->getStaticData().canMineMaxRes;
		if (total <= 0 || !building->isUnitWorking()) continue;
		res += building->prod;
	}
	return res;
}

//--------------------------------------------------------------------------
sMiningResource setBuildingsProduction (std::vector<cBuilding*>& buildings, sMiningResource wanted)
{
	auto mines = ExtractOnLineMiningStations (buildings);
	const auto limits = computeResourcesLimit (mines);
	auto newProd = adjustResourceToMaxAllowed (limits, wanted, {eResourceType::Metal, eResourceType::Oil, eResourceType::Gold});
	setMinesProduction (mines, newProd);
	return newProd;
}

//------------------------------------------------------------------------------
sMiningResource increaseOilProduction (std::vector<cBuilding*>& buildings, int missingOil)
{
	auto mines = ExtractOnLineMiningStations (buildings);
	const auto limits = computeResourcesLimit (mines);
	const auto prod = computeProduction (mines);
	auto wanted = prod;
	wanted.oil += missingOil;
	auto newProd = adjustResourceToMaxAllowed (limits, wanted, {eResourceType::Oil, eResourceType::Gold, eResourceType::Metal});

	setMinesProduction (mines, newProd);
	return newProd;
}
