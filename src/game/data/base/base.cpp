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

#include "game/data/map/map.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "utility/crc.h"
#include "utility/listhelpers.h"
#include "utility/log.h"
#include "utility/mathtools.h"

namespace
{

	//--------------------------------------------------------------------------
	uint32_t calcCheckSum (const sRecoltableResources& res, uint32_t crc)
	{
		crc = ::calcCheckSum (res.metal, crc);
		crc = ::calcCheckSum (res.oil, crc);
		crc = ::calcCheckSum (res.gold, crc);
		return crc;
	}

	//--------------------------------------------------------------------------
	bool isAOnlineStation (const cBuilding* building)
	{
		return building->getStaticUnitData().produceEnergy > 1 && building->isUnitWorking();
	}

	//--------------------------------------------------------------------------
	bool isAOfflineStation (const cBuilding* building)
	{
		return building->getStaticUnitData().produceEnergy > 1 && !building->isUnitWorking();
	}

	//--------------------------------------------------------------------------
	bool isAOnlineGenerator (const cBuilding* building)
	{
		return building->getStaticUnitData().produceEnergy == 1 && building->isUnitWorking();
	}

	//--------------------------------------------------------------------------
	bool isAOfflineGenerator (const cBuilding* building)
	{
		return building->getStaticUnitData().produceEnergy == 1 && !building->isUnitWorking();
	}

	//--------------------------------------------------------------------------
	int calcMaxProd (const std::vector<cBuilding*>& buildings, eResourceType ressourceType)
	{
		int maxProd = 0;
		for (const cBuilding* building : buildings)
		{
			if (building->getStaticUnitData().canMineMaxRes <= 0 || !building->isUnitWorking()) continue;

			maxProd += building->getMaxProd (ressourceType);
		}
		return maxProd;
	}

	struct sResourcesLimit
	{
		sRecoltableResources min; //<! minimal, assuming maximum for others
		sRecoltableResources extraForOthers; //!< extra amount of resource which can be distribute freely to other resource without impact
		sRecoltableResources max; //<! total of maxProd
	};

	//--------------------------------------------------------------------------
	sResourcesLimit computeResourcesLimit (const std::vector<cBuilding*>& buildings)
	{
		sResourcesLimit res;

		for (const cBuilding* building : buildings)
		{
			const auto total = building->getStaticUnitData().canMineMaxRes;
			if (total <= 0 || !building->isUnitWorking()) continue;
			const int metal = building->getMaxProd (eResourceType::Metal);
			const int oil = building->getMaxProd (eResourceType::Oil);
			const int gold = building->getMaxProd (eResourceType::Gold);

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
	sRecoltableResources calcMaxAllowedProd (const sResourcesLimit& limits, const sRecoltableResources& current)
	{
		sRecoltableResources res;

		int metalToDistribute = std::max (0, current.metal - limits.min.metal);
		int oilToDistribute = std::max (0, current.oil - limits.min.oil);
		int goldToDistribute = std::max (0, current.gold - limits.min.gold);

		res.metal = limits.max.metal - std::max (0, oilToDistribute + goldToDistribute - limits.extraForOthers.metal);
		res.oil = limits.max.oil - std::max (0, metalToDistribute + goldToDistribute - limits.extraForOthers.oil);
		res.gold = limits.max.gold - std::max (0, oilToDistribute + metalToDistribute - limits.extraForOthers.gold);

		return res;
	}

}

//------------------------------------------------------------------------------
int sRecoltableResources::get (eResourceType ressourceType) const
{
	switch (ressourceType)
	{
		case eResourceType::Metal: return metal;
		case eResourceType::Oil: return oil;
		case eResourceType::Gold: return gold;
	}
	return 0;
}

//------------------------------------------------------------------------------
cSubBase::cSubBase (cBase& base) :
	base (base)
{}

//------------------------------------------------------------------------------
cSubBase::cSubBase (const cSubBase& other) :
	buildings (other.buildings),
	maxStored (other.maxStored),
	maxNeeded (other.maxNeeded),
	needed (other.needed),
	prod (other.prod),
	stored (other.stored),
	maxEnergyProd (other.maxEnergyProd),
	energyProd (other.energyProd),
	maxEnergyNeed (other.maxEnergyNeed),
	energyNeed (other.energyNeed),
	humanProd (other.humanProd),
	humanNeed (other.humanNeed),
	maxHumanNeed (other.maxHumanNeed),
	base (other.base)
{}

cSubBase::~cSubBase()
{
	// remove the current subbase
	for (auto b : buildings)
	{
		b->subBase = nullptr;
	}
}

int cSubBase::getMaxMetalProd() const
{
	return calcMaxProd (buildings, eResourceType::Metal);
}

int cSubBase::getMaxGoldProd() const
{
	return calcMaxProd (buildings, eResourceType::Gold);
}

int cSubBase::getMaxOilProd() const
{
	return calcMaxProd (buildings, eResourceType::Oil);
}

sRecoltableResources cSubBase::computeMaxAllowedProd (const sRecoltableResources& prod) const
{
	return calcMaxAllowedProd (computeResourcesLimit (buildings), prod);
}

int cSubBase::getMetalProd() const
{
	return prod.metal;
}

int cSubBase::getGoldProd() const
{
	return prod.gold;
}

int cSubBase::getOilProd() const
{
	return prod.oil;
}

void cSubBase::setProduction (const sRecoltableResources& newProd)
{
	const auto& limits = computeResourcesLimit (buildings);
	auto allowed = calcMaxAllowedProd (limits, {0, 0, 0});
	prod.metal = std::min (newProd.metal, allowed.metal);

	allowed = calcMaxAllowedProd (limits, {prod.metal, 0, 0});
	this->prod.oil = std::min (newProd.oil, allowed.oil);

	allowed = calcMaxAllowedProd (limits, {prod.metal, prod.oil, 0});
	prod.gold = std::min (newProd.gold, allowed.gold);

	// TODO: update buildings
}

void cSubBase::setMetal (int value)
{
	std::swap (stored.metal, value);
	if (stored.metal != value) metalChanged();
}

void cSubBase::setOil (int value)
{
	std::swap (stored.oil, value);
	if (stored.oil != value) oilChanged();
}

int cSubBase::getMaxEnergyProd() const
{
	return maxEnergyProd;
}

int cSubBase::getEnergyProd() const
{
	return energyProd;
}

int cSubBase::getMaxEnergyNeed() const
{
	return maxEnergyNeed;
}

int cSubBase::getEnergyNeed() const
{
	return energyNeed;
}

int cSubBase::getHumanProd() const
{
	return humanProd;
}

int cSubBase::getHumanNeed() const
{
	return humanNeed;
}

int cSubBase::getMaxHumanNeed() const
{
	return maxHumanNeed;
}

void cSubBase::setGold (int value)
{
	std::swap (stored.gold, value);
	if (stored.gold != value) goldChanged();
}

bool cSubBase::increaseEnergyProd (int value)
{
	// TODO: the energy production and fuel consumption
	// of generators and stations are hardcoded in this function
	std::vector<cBuilding*> onlineStations = Filter (buildings, &isAOnlineStation);
	std::vector<cBuilding*> onlineGenerators = Filter (buildings, &isAOnlineGenerator);
	std::vector<cBuilding*> offlineStations = Filter (buildings, &isAOfflineStation);
	std::vector<cBuilding*> offlineGenerators = Filter (buildings, &isAOfflineGenerator);
	const int availableStations = onlineStations.size() + offlineStations.size();
	const int availableGenerators = onlineGenerators.size() + offlineGenerators.size();

	// calc the optimum amount of energy stations and generators
	const int energy = energyProd + value;

	int stations   = std::min ((energy + 3) / 6, availableStations);
	int generators = std::max (energy - stations * 6, 0);

	if (generators > availableGenerators)
	{
		stations++;
		generators = 0;
	}

	if (stations > availableStations)
	{
		return false; // not enough free energy production capacity
	}

	// check available fuel
	int neededFuel = stations * 6 + generators * 2;
	if (neededFuel > stored.oil + getMaxOilProd())
	{
		// not possible to produce enough fuel
		base.fuelInsufficient();
		return false;
	}

	// stop unneeded buildings
	for (int i = (int) onlineStations.size() - stations; i > 0; --i)
	{
		onlineStations[0]->stopWork (true);
		onlineStations.erase (onlineStations.begin());
	}
	for (int i = (int) onlineGenerators.size() - generators; i > 0; --i)
	{
		onlineGenerators[0]->stopWork (true);
		onlineGenerators.erase (onlineGenerators.begin());
	}

	// start needed buildings
	for (int i = stations - (int) onlineStations.size(); i > 0; --i)
	{
		offlineStations[0]->startWork ();
		offlineStations.erase (offlineStations.begin());
	}
	for (int i = generators - (int) onlineGenerators.size(); i > 0; --i)
	{
		offlineGenerators[0]->startWork ();
		offlineGenerators.erase (offlineGenerators.begin());
	}
	return true;
}

void cSubBase::addMetal (int value)
{
	addRessouce (eResourceType::Metal, value);
}

void cSubBase::addOil (int value)
{
	addRessouce (eResourceType::Oil, value);
}

void cSubBase::addGold (int value)
{
	addRessouce (eResourceType::Gold, value);
}

int cSubBase::getResource (eResourceType storeResType) const
{
	switch (storeResType)
	{
		case eResourceType::Metal:
			return stored.metal;
		case eResourceType::Oil:
			return stored.oil;
		case eResourceType::Gold:
			return stored.gold;
		default:
			assert (0);
	}
	return 0;
}

void cSubBase::setResource (eResourceType storeResType, int value)
{
	switch (storeResType)
	{
		case eResourceType::Metal:
			setMetal (value);
			break;
		case eResourceType::Oil:
			setOil (value);
			break;
		case eResourceType::Gold:
			setGold (value);
			break;
		default:
			assert (0);
	}
}

void cSubBase::addRessouce (eResourceType storeResType, int value)
{
	int storedRessources = getResource (storeResType);
	value = std::max (value, -storedRessources);
	if (value == 0) return;
	setResource (storeResType, storedRessources + value);

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& b = *buildings[i];
		if (b.getStaticUnitData().storeResType != storeResType) continue;
		if (value < 0)
		{
			if (b.getStoredResources() > -value)
			{
				b.setStoredResources (b.getStoredResources() + value);
				value = 0;
			}
			else
			{
				value += b.getStoredResources();
				b.setStoredResources (0);
			}
		}
		else
		{
			if (b.getStaticUnitData().storageResMax - b.getStoredResources() > value)
			{
				b.setStoredResources (b.getStoredResources() + value);
				value = 0;
			}
			else
			{
				value -= b.getStaticUnitData().storageResMax - b.getStoredResources();
				b.setStoredResources (b.getStaticUnitData().storageResMax);
			}
		}

		if (value == 0) break;
	}
}

bool cSubBase::checkHumanConsumer ()
{
	if (humanNeed <= humanProd) return false;

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];
		if (!building.getStaticUnitData().needsHumans || !building.isUnitWorking()) continue;

		building.stopWork (false);

		if (humanNeed <= humanProd) break;
	}
	return true;
}

bool cSubBase::checkGoldConsumer()
{
	if (needed.gold <= prod.gold + stored.gold) return false;

	for (cBuilding* building : buildings)
	{
		if (!building->getStaticUnitData().convertsGold || !building->isUnitWorking()) continue;

		building->stopWork (false);

		if (needed.gold <= prod.gold + stored.gold) break;
	}
	return true;
}

bool cSubBase::checkMetalConsumer ()
{
	if (needed.metal <= prod.metal + stored.metal) return false;

	for (cBuilding* building : buildings)
	{
		if (!building->getStaticUnitData().needsMetal || !building->isUnitWorking()) continue;

		building->stopWork (false);

		if (needed.metal <= prod.metal + stored.metal) break;
	}
	return true;
}

void cSubBase::increaseOilProd (int missingOil)
{
	const auto limits = computeResourcesLimit (buildings);
	auto allowed = calcMaxAllowedProd (limits, {0, prod.oil + missingOil, 0});
	const int gold = std::min (prod.gold, allowed.gold);
	const int goldDecrease = prod.gold - gold;
	allowed = calcMaxAllowedProd (limits, {0, prod.oil + missingOil, gold});
	const int metal = std::min(prod.metal, allowed.metal);
	const int metalDecrease = prod.metal - metal;
	setProduction ({metal, prod.oil + missingOil, gold});

	base.forcedRessouceProductionChance(eResourceType::Oil, missingOil, true);
	if (metalDecrease)
		base.forcedRessouceProductionChance(eResourceType::Metal, metalDecrease, false);
	if (goldDecrease > 0)
		base.forcedRessouceProductionChance(eResourceType::Gold, goldDecrease, false);
}

bool cSubBase::checkOil()
{
	// TODO: the energy production and fuel consumption of generators and
	// stations are hardcoded in this function
	std::vector<cBuilding*> onlineStations = Filter (buildings, &isAOnlineStation);
	std::vector<cBuilding*> onlineGenerators = Filter (buildings, &isAOnlineGenerator);
	std::vector<cBuilding*> offlineStations = Filter (buildings, &isAOfflineStation);
	std::vector<cBuilding*> offlineGenerators = Filter (buildings, &isAOfflineGenerator);
	const int availableStations = onlineStations.size() + offlineStations.size();
	const int availableGenerators = onlineGenerators.size() + offlineGenerators.size();

	// calc the optimum amount of energy stations and generators
	int stations   = std::min ((energyNeed + 3) / 6, availableStations);
	int generators = std::max (energyNeed - stations * 6, 0);

	if (generators > availableGenerators)
	{
		if (stations < availableStations)
		{
			stations++;
			generators = 0;
		}
		else
		{
			generators = availableGenerators;
		}
	}

	// check needed oil
	int neededOil = stations * 6 + generators * 2;
	const int availableOil = getMaxOilProd() + stored.oil;
	bool oilMissing = false;
	if (neededOil > availableOil)
	{
		// reduce energy production to maximum possible value
		stations = std::min ((availableOil) / 6, availableStations);
		generators = std::min ((availableOil - (stations * 6)) / 2, availableGenerators);

		oilMissing = true;
	}

	// increase oil production, if necessary
	neededOil = stations * 6 + generators * 2;
	if (neededOil > prod.oil + stored.oil)
	{
		const int missingOil = neededOil - prod.oil - stored.oil;
		increaseOilProd (missingOil);
	}

	// stop unneeded buildings
	for (int i = (int) onlineStations.size() - stations; i > 0; i--)
	{
		onlineStations[0]->stopWork (true);
		onlineStations.erase (onlineStations.begin());
	}
	for (int i = (int) onlineGenerators.size() - generators; i > 0; i--)
	{
		onlineGenerators[0]->stopWork (true);
		onlineGenerators.erase (onlineGenerators.begin());
	}

	// start needed buildings
	for (int i = stations - (int) onlineStations.size(); i > 0; i--)
	{
		offlineStations[0]->startWork ();
		offlineStations.erase (offlineStations.begin());
	}
	for (int i = generators - (int) onlineGenerators.size(); i > 0; i--)
	{
		offlineGenerators[0]->startWork ();
		offlineGenerators.erase (offlineGenerators.begin());
	}

	return oilMissing;
}

bool cSubBase::checkEnergy ()
{
	if (energyNeed <= energyProd) return false;

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];
		if (!building.getStaticUnitData().needsEnergy || !building.isUnitWorking()) continue;

		// do not shut down ressource producers in the first run
		if (building.getMaxProd(eResourceType::Metal) > 0 ||
			building.getMaxProd(eResourceType::Gold) > 0 ||
			building.getMaxProd(eResourceType::Oil) > 0) continue;

		building.stopWork (false);

		if (energyNeed <= energyProd) return true;
	}

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];
		if (!building.getStaticUnitData().needsEnergy || !building.isUnitWorking()) continue;

		// do not shut down oil producers in the second run
		if (building.getMaxProd(eResourceType::Oil) > 0) continue;

		building.stopWork (false);

		if (energyNeed <= energyProd) return true;
	}

	// if energy is still missing, shut down also oil producers
	for (size_t i = 0; i < buildings.size(); i++)
	{
		cBuilding& building = *buildings[i];
		if (!building.getStaticUnitData().needsEnergy || !building.isUnitWorking()) continue;

		building.stopWork (false);

		if (energyNeed <= energyProd) return true;
	}
	return true;
}

bool cSubBase::checkTurnEnd ()
{
	bool changedSomething = false;

	if (checkMetalConsumer ())
	{
		base.metalLow();
		changedSomething = true;
	}

	if (checkHumanConsumer ())
	{
		base.teamLow();
		changedSomething = true;
	}

	if (checkGoldConsumer ())
	{
		base.goldLow();
		changedSomething = true;
	}

	// there is a loop around checkOil/checkEnergy,
	// because a lack of energy can lead a shutdown of fuel producers,
	// which can lead again to swiched off energy producers, etc...
	bool oilMissing = false;
	bool energyMissing = false;
	bool changed = true;
	while (changed)
	{
		changed = false;
		if (checkOil ())
		{
			changed = true;
			oilMissing = true;
			changedSomething = true;
		}

		if (checkEnergy ())
		{
			changed = true;
			energyMissing = true;
			changedSomething = true;
		}
	}
	if (oilMissing)
	{
		base.fuelLow();
		changedSomething = true;
	}

	if (energyMissing)
	{
		base.energyLow();
		changedSomething = true;
	}

	// recheck metal and gold,
	// because metal and gold producers could have been shut down,
	// due to a lack of energy
	if (checkMetalConsumer ())
	{
		base.metalLow();
		changedSomething = true;
	}

	if (checkGoldConsumer ())
	{
		base.goldLow();
		changedSomething = true;
	}

	return changedSomething;
}

void cSubBase::makeTurnStartRepairs (cBuilding& building)
{
	// repair (do not repair buildings that have been attacked in this turn):
	if (building.data.getHitpoints() >= building.data.getHitpointsMax()
		|| stored.metal <= 0 || building.hasBeenAttacked())
	{
		return;
	}
	// calc new hitpoints
	const auto newHitPoints = building.data.getHitpoints() + Round (((float)building.data.getHitpointsMax() / building.data.getBuildCost()) * 4);
	building.data.setHitpoints (std::min (building.data.getHitpointsMax(), newHitPoints));
	addMetal (-1);
}

void cSubBase::makeTurnStartReload (cBuilding& building)
{
	// reload:
	if (building.getStaticUnitData().canAttack && building.data.getAmmo() == 0 && stored.metal > 0)
	{
		building.data.setAmmo (building.data.getAmmoMax());
		addMetal (-1);
	}
}

void cSubBase::makeTurnStartBuild (cBuilding& building)
{
	// build:
	if (!building.isUnitWorking() || building.getStaticUnitData().canBuild.empty() || building.isBuildListEmpty())
	{
		return;
	}

	cBuildListItem& buildListItem = building.getBuildListItem (0);
	if (buildListItem.getRemainingMetal() > 0)
	{
		// in this block the metal consumption of the factory
		// in the next round can change
		// so we first subtract the old value from needed.metal and
		// then add the new one, to hold the base up to date
		needed.metal-= building.getMetalPerRound();

		auto value = buildListItem.getRemainingMetal() - building.getMetalPerRound();
		value = std::max (value, 0);
		buildListItem.setRemainingMetal (value);

		needed.metal += building.getMetalPerRound();
	}
	if (buildListItem.getRemainingMetal() <= 0)
	{
		base.owner.addTurnReportUnit (buildListItem.getType());
		building.stopWork (false);
	}
}

void cSubBase::makeTurnStart ()
{
	// produce ressources
	addOil (prod.oil - needed.oil);
	addMetal (prod.metal - needed.metal);
	addGold (prod.gold - needed.gold);

	// produce credits
	if (needed.gold)
	{
		base.owner.setCredits (base.owner.getCredits() + needed.gold);
	}

	// make repairs/build/reload
	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];

		makeTurnStartRepairs (building);
		building.setHasBeenAttacked (false);
		makeTurnStartReload (building);
		makeTurnStartBuild (building);
	}

	// check maximum storage limits
	auto newMetal = std::min (maxStored.metal, stored.metal);
	auto newOil = std::min (maxStored.oil, stored.oil);
	auto newGold = std::min (maxStored.gold, stored.gold);

	// should not happen, but to be sure:
	newMetal = std::max (newMetal, 0);
	newOil = std::max (newOil, 0);
	newGold = std::max (newGold, 0);

	setMetal (newMetal);
	setOil (newOil);
	setGold (newGold);
}

void cSubBase::merge (cSubBase& sb)
{
	for (cBuilding* building : sb.buildings)
	{
		addBuilding (*building);
	}
	sb.buildings.clear();

	RemoveIf (base.SubBases, ByGetTo (&sb));
}

void cSubBase::addBuilding (cBuilding& b)
{
	buildings.push_back (&b);
	b.subBase = this;

	// calculate storage level
	switch (b.getStaticUnitData().storeResType)
	{
		case eResourceType::Metal:
			maxStored.metal += b.getStaticUnitData().storageResMax;
			setMetal (stored.metal + b.getStoredResources());
			break;
		case eResourceType::Oil:
			maxStored.oil += b.getStaticUnitData().storageResMax;
			setOil (stored.oil + b.getStoredResources());
			break;
		case eResourceType::Gold:
			maxStored.gold += b.getStaticUnitData().storageResMax;
			setGold (stored.gold + b.getStoredResources());
			break;
		case eResourceType::None:
			break;
	}
	// calculate energy
	if (b.getStaticUnitData().produceEnergy)
	{
		maxEnergyProd += b.getStaticUnitData().produceEnergy;
		maxNeeded.oil += b.getStaticUnitData().needsOil;
		if (b.isUnitWorking())
		{
			energyProd += b.getStaticUnitData().produceEnergy;
			needed.oil += b.getStaticUnitData().needsOil;
		}
	}
	else if (b.getStaticUnitData().needsEnergy)
	{
		maxEnergyNeed += b.getStaticUnitData().needsEnergy;
		if (b.isUnitWorking())
		{
			energyNeed += b.getStaticUnitData().needsEnergy;
		}
	}
	// calculate resource consumption
	if (b.getStaticUnitData().needsMetal)
	{
		maxNeeded.metal += b.getStaticUnitData().needsMetal * 12;
		if (b.isUnitWorking())
		{
			needed.metal += std::min (b.getMetalPerRound(), b.getBuildListItem (0).getRemainingMetal());
		}
	}
	// calculate gold
	if (b.getStaticUnitData().convertsGold)
	{
		maxNeeded.gold += b.getStaticUnitData().convertsGold;
		if (b.isUnitWorking())
		{
			needed.gold += b.getStaticUnitData().convertsGold;
		}
	}
	// calculate resource production
	if (b.getStaticUnitData().canMineMaxRes > 0 && b.isUnitWorking())
	{
		prod.metal += b.metalProd;
		prod.oil += b.oilProd;
		prod.gold += b.goldProd;
	}
	// calculate humans
	if (b.getStaticUnitData().produceHumans)
	{
		humanProd += b.getStaticUnitData().produceHumans;
	}
	if (b.getStaticUnitData().needsHumans)
	{
		maxHumanNeed += b.getStaticUnitData().needsHumans;
		if (b.isUnitWorking())
		{
			humanNeed += b.getStaticUnitData().needsHumans;
		}
	}
}

bool cSubBase::startBuilding(cBuilding& b)
{
	const cStaticUnitData& staticData = b.getStaticUnitData();

	// needs human workers:
	if (staticData.needsHumans)
	{
		if (humanNeed + staticData.needsHumans > humanProd)
		{
			base.teamInsufficient();
			return false;
		}
	}

	// needs gold:
	if (staticData.convertsGold)
	{
		if (needed.gold + staticData.convertsGold > prod.gold + stored.gold)
		{
			base.goldInsufficient();
			return false;
		}
	}

	// needs raw material:
	if (staticData.needsMetal)
	{
		if (needed.metal + b.getMetalPerRound() > prod.metal + stored.metal)
		{
			base.metalInsufficient();
			return false;
		}
	}

	// needs oil:
	if (staticData.needsOil)
	{
		// check if there is enough Oil for the generators
		// (current production + reserves)
		if (staticData.needsOil + needed.oil > stored.oil + getMaxOilProd())
		{
			base.fuelInsufficient();
			return false;
		}
		else if (staticData.needsOil + needed.oil > stored.oil + prod.oil)
		{
			// increase oil production
			int missingOil = staticData.needsOil + needed.oil - (stored.oil + prod.oil);

			increaseOilProd (missingOil);
		}
	}

	// IsWorking is set to true before checking the energy production.
	// So if an energy generator has to be started,
	// it can use the fuel production of this building
	// (when this building is a mine).
	b.setWorking (true);

	// set mine values. This has to be undone, if the energy is insufficient
	if (staticData.canMineMaxRes > 0)
	{
		prod.metal += b.metalProd;
		prod.oil += b.oilProd;
		prod.gold += b.goldProd;
	}

	// Energy consumers:
	if (staticData.needsEnergy)
	{
		if (staticData.needsEnergy + energyNeed > energyProd)
		{
			// try to increase energy production
			if (!increaseEnergyProd(staticData.needsEnergy + energyNeed - energyProd))
			{
				b.setWorking(false);

				// reset mine values
				if (staticData.canMineMaxRes > 0)
				{
					prod.metal -= b.metalProd;
					prod.oil -= b.oilProd;
					prod.gold -= b.goldProd;
				}

				base.energyInsufficient();
				return false;
			}
			base.energyToLow();
		}
	}

	//-- everything is ready to start the building

	energyProd += staticData.produceEnergy;
	energyNeed += staticData.needsEnergy;

	humanNeed += staticData.needsHumans;
	humanProd += staticData.produceHumans;

	needed.oil += staticData.needsOil;

	// raw material consumer:
	if (staticData.needsMetal)
		needed.metal += b.getMetalPerRound();

	// gold consumer:
	needed.gold += staticData.convertsGold;

	return true;
}

bool cSubBase::stopBuilding(cBuilding& b, bool forced /*= false*/)
{
	const cStaticUnitData& staticData = b.getStaticUnitData();

	// energy generators
	if (staticData.produceEnergy)
	{
		if (energyNeed > energyProd - staticData.produceEnergy && !forced)
		{
			base.energyIsNeeded();
			return false;
		}

		energyProd -= staticData.produceEnergy;
		needed.oil -= staticData.needsOil;
	}

	b.setWorking(false);

	// Energy consumers:
	energyNeed -= staticData.needsEnergy;

	// raw material consumer:
	if (staticData.needsMetal)
		needed.metal -= b.getMetalPerRound();

	// gold consumer
	needed.gold -= staticData.convertsGold;

	// human consumer
	humanNeed -= staticData.needsHumans;

	// Minen:
	if (staticData.canMineMaxRes > 0)
	{
		prod.metal -= b.metalProd;
		prod.oil -= b.oilProd;
		prod.gold -= b.goldProd;
	}

	return true;
}

uint32_t cSubBase::getChecksum(uint32_t crc) const
{
	crc = calcCheckSum (maxStored, crc);
	crc = calcCheckSum (maxNeeded, crc);
	crc = calcCheckSum (needed, crc);
	crc = calcCheckSum (prod, crc);
	crc = calcCheckSum (stored, crc);

	crc = calcCheckSum (maxEnergyProd, crc);
	crc = calcCheckSum (energyProd, crc);
	crc = calcCheckSum (maxEnergyNeed, crc);
	crc = calcCheckSum (energyNeed, crc);

	crc = calcCheckSum (humanProd, crc);
	crc = calcCheckSum (humanNeed, crc);
	crc = calcCheckSum (maxHumanNeed, crc);

	return crc;
}

cBase::cBase(cPlayer& owner) :
	owner(owner)
{}

cBase::~cBase()
{
}

cSubBase* cBase::checkNeighbour (const cPosition& position, const cBuilding& building, const cMap& map)
{
	if (map.isValidPosition (position) == false) return nullptr;
	cBuilding* b = map.getField (position).getBuilding();

	if (b && b->getOwner() == building.getOwner() && b->subBase)
	{
		b->CheckNeighbours (map);
		return b->subBase;
	}
	return nullptr;
}

uint32_t cBase::getChecksum(uint32_t crc) const
{
	for (const auto& sb : SubBases)
		crc = calcCheckSum(*sb, crc);

	return crc;
}

void cBase::addBuilding (cBuilding& building, const cMap& map)
{
	addBuilding (building, map, true);
}

void cBase::addBuilding (cBuilding& building, const cMap& map, bool signalChange)
{
	if (!building.getStaticUnitData().connectsToBase) return;
	std::vector<cSubBase*> NeighbourList;

	// find all neighbouring subbases
	if (building.getIsBig())
	{
		// big building
		if (cSubBase* SubBase = checkNeighbour (building.getPosition() + cPosition (0, -1), building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building.getPosition() + cPosition (1, -1), building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building.getPosition() + cPosition (2, 0), building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building.getPosition() + cPosition (2, 1), building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building.getPosition() + cPosition (0, 2), building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building.getPosition() + cPosition (1, 2), building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building.getPosition() + cPosition (-1, 0), building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building.getPosition() + cPosition (-1, 1), building, map)) NeighbourList.push_back (SubBase);
	}
	else
	{
		// small building
		if (cSubBase* SubBase = checkNeighbour (building.getPosition() + cPosition (0, -1), building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building.getPosition() + cPosition (1, 0), building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building.getPosition() + cPosition (0, 1), building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building.getPosition() + cPosition (-1, 0), building, map)) NeighbourList.push_back (SubBase);
	}
	building.CheckNeighbours (map);

	RemoveDuplicates (NeighbourList);

	if (NeighbourList.empty())
	{
		// no neighbours found, just generate new subbase and add the building
		SubBases.push_back (std::make_unique<cSubBase> (*this));
		SubBases.back()->addBuilding (building);

		if (signalChange) onSubbaseConfigurationChanged (std::vector<cBuilding*>{&building});
		return;
	}

	// found neighbours, so add the building to the first neighbour subbase
	cSubBase* const firstNeighbour = NeighbourList[0];
	firstNeighbour->addBuilding (building);
	NeighbourList.erase (NeighbourList.begin());

	// now merge the other neighbours to the first one, if necessary
	for (size_t i = 0; i != NeighbourList.size(); ++i)
	{
		cSubBase* const SubBase = NeighbourList[i];
		firstNeighbour->merge (*SubBase);
	}
	if (signalChange) onSubbaseConfigurationChanged (firstNeighbour->getBuildings());
}

void cBase::deleteBuilding (cBuilding& building, const cMap& map)
{
	if (!building.getStaticUnitData().connectsToBase) return;

	auto buildings = building.subBase->getBuildings();
	RemoveIf (SubBases, ByGetTo (building.subBase));

	// add all the buildings again
	for (auto b : buildings)
	{
		if (b == &building) continue;
		addBuilding (*b, map, false);
	}

	if (building.isUnitWorking() && building.getStaticUnitData().canResearch)
		building.getOwner()->stopAResearch (building.getResearchArea());
	onSubbaseConfigurationChanged (buildings);
}

bool cBase::checkTurnEnd()
{
	bool changed = false;
	for (size_t i = 0; i != SubBases.size(); ++i)
	{
		if (SubBases[i]->checkTurnEnd ())
		{
			changed = true;
		}
	}
	return changed;
}

void cBase::makeTurnStart ()
{
	for (size_t i = 0; i != SubBases.size(); ++i)
	{
		SubBases[i]->makeTurnStart ();
	}
}

void cBase::clear()
{
	SubBases.clear();
}
