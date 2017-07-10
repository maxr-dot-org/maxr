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
#include <cassert>
#include "game/data/base/base.h"

#include "game/data/units/building.h"
#include "game/logic/clientevents.h"
#include "utility/listhelpers.h"
#include "utility/log.h"
#include "game/data/map/map.h"
#include "netmessage.h"
#include "game/data/player/player.h"
#include "game/logic/server.h"
#include "game/logic/serverevents.h"
#include "game/data/report/savedreportsimple.h"
#include "game/data/report/special/savedreportresourcechanged.h"
#include "utility/crc.h"

using namespace std;

cSubBase::cSubBase (cBase& base) :
	buildings(),
	base (base),
	maxMetalStored(),
	maxOilStored(),
	maxGoldStored(),
	maxEnergyProd(),
	energyProd(),
	maxEnergyNeed(),
	energyNeed(),
	metalNeed(),
	oilNeed(),
	goldNeed(),
	maxMetalNeed(),
	maxOilNeed(),
	maxGoldNeed(),
	humanProd(),
	humanNeed(),
	maxHumanNeed(),
	metalProd(),
	oilProd(),
	goldProd(),
	metalStored(),
	oilStored(),
	goldStored()
{}

cSubBase::cSubBase (const cSubBase& other) :
	buildings (other.buildings),
	base (other.base),
	maxMetalStored (other.maxMetalStored),
	maxOilStored (other.maxOilStored),
	maxGoldStored (other.maxGoldStored),
	maxEnergyProd (other.maxEnergyProd),
	energyProd (other.energyProd),
	maxEnergyNeed (other.maxEnergyNeed),
	energyNeed (other.energyNeed),
	metalNeed (other.metalNeed),
	oilNeed (other.oilNeed),
	goldNeed (other.goldNeed),
	maxMetalNeed (other.maxMetalNeed),
	maxOilNeed (other.maxOilNeed),
	maxGoldNeed (other.maxGoldNeed),
	humanProd (other.humanProd),
	humanNeed (other.humanNeed),
	maxHumanNeed (other.maxHumanNeed),
	metalProd (other.metalProd),
	oilProd (other.oilProd),
	goldProd (other.goldProd),
	metalStored (other.metalStored),
	oilStored (other.oilStored),
	goldStored (other.goldStored)
{}

cSubBase::~cSubBase()
{
	destroyed();
}

int cSubBase::getMaxMetalProd() const
{
	return calcMaxProd (RES_METAL);
}

int cSubBase::getMaxGoldProd() const
{
	return calcMaxProd (RES_GOLD);
}

int cSubBase::getMaxOilProd() const
{
	return calcMaxProd (RES_OIL);
}

int cSubBase::getMaxAllowedMetalProd() const
{
	return calcMaxAllowedProd (RES_METAL);
}

int cSubBase::getMaxAllowedGoldProd() const
{
	return calcMaxAllowedProd (RES_GOLD);
}

int cSubBase::getMaxAllowedOilProd() const
{
	return calcMaxAllowedProd (RES_OIL);
}

int cSubBase::getMetalProd() const
{
	return metalProd;
}

int cSubBase::getGoldProd() const
{
	return goldProd;
}

int cSubBase::getOilProd() const
{
	return oilProd;
}

void cSubBase::setMetalProd (int value)
{
	const int max = getMaxAllowedMetalProd();

	value = std::max (0, value);
	value = std::min (value, max);

	metalProd = value;
}

void cSubBase::setGoldProd (int value)
{
	const int max = getMaxAllowedGoldProd();

	value = std::max (0, value);
	value = std::min (value, max);

	goldProd = value;
}

void cSubBase::setOilProd (int value)
{
	const int max = getMaxAllowedOilProd();

	value = std::max (0, value);
	value = std::min (value, max);

	oilProd = value;
}

int cSubBase::getMetalStored() const
{
	return metalStored;
}

void cSubBase::setMetal (int value)
{
	std::swap (metalStored, value);
	if (metalStored != value) metalChanged();
}

int cSubBase::getOilStored() const
{
	return oilStored;
}

void cSubBase::setOil (int value)
{
	std::swap (oilStored, value);
	if (oilStored != value) oilChanged();
}

int cSubBase::getGoldStored() const
{
	return goldStored;
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

int cSubBase::getMetalNeed() const
{
	return metalNeed;
}

int cSubBase::getOilNeed() const
{
	return oilNeed;
}

int cSubBase::getGoldNeed() const
{
	return goldNeed;
}

int cSubBase::getMaxMetalNeed() const
{
	return maxMetalNeed;
}

int cSubBase::getMaxOilNeed() const
{
	return maxOilNeed;
}

int cSubBase::getMaxGoldNeed() const
{
	return maxGoldNeed;
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

const std::vector<cBuilding*>& cSubBase::getBuildings() const
{
	return buildings;
}

uint32_t cSubBase::getChecksum(uint32_t crc) const
{
	crc = calcCheckSum( maxMetalStored, crc);
	crc = calcCheckSum( maxOilStored, crc);
	crc = calcCheckSum( maxGoldStored, crc);
	crc = calcCheckSum( maxEnergyProd, crc);
	crc = calcCheckSum( energyProd, crc);
	crc = calcCheckSum( maxEnergyNeed, crc);
	crc = calcCheckSum( energyNeed, crc);
	crc = calcCheckSum( metalNeed, crc);
	crc = calcCheckSum( oilNeed, crc);
	crc = calcCheckSum( goldNeed, crc);
	crc = calcCheckSum( maxMetalNeed, crc);
	crc = calcCheckSum( maxOilNeed, crc);
	crc = calcCheckSum( maxGoldNeed, crc);
	crc = calcCheckSum( humanProd, crc);
	crc = calcCheckSum( humanNeed, crc);
	crc = calcCheckSum( maxHumanNeed, crc);
	crc = calcCheckSum( metalProd, crc);
	crc = calcCheckSum( oilProd, crc);
	crc = calcCheckSum( goldProd, crc);
	crc = calcCheckSum( metalStored, crc);
	crc = calcCheckSum( oilStored, crc);
	crc = calcCheckSum( goldStored, crc);

	return crc;
}

void cSubBase::setGold (int value)
{
	std::swap (goldStored, value);
	if (goldStored != value) goldChanged();
}

void cSubBase::changeMetalProd (int value)
{
	setMetalProd (metalProd + value);
}

void cSubBase::changeOilProd (int value)
{
	setOilProd (oilProd + value);
}

int cSubBase::getMaxMetalStored() const
{
	return maxMetalStored;
}

int cSubBase::getMaxGoldStored() const
{
	return maxGoldStored;
}

int cSubBase::getMaxOilStored() const
{
	return maxOilStored;
}

void cSubBase::changeGoldProd (int value)
{
	setGoldProd (goldProd + value);
}

int cSubBase::calcMaxProd (int ressourceType) const
{
	int maxProd = 0;
	for (size_t i = 0; i != buildings.size(); ++i)
	{
		const cBuilding& building = *buildings[i];

		if (building.getStaticUnitData().canMineMaxRes <= 0 || !building.isUnitWorking()) continue;

		maxProd += building.getMaxProd(ressourceType);
	}
	return maxProd;
}

int cSubBase::calcMaxAllowedProd (int ressourceType) const
{
	// initialize needed variables,
	// so the algorithm itself is independent from the ressouce type
	int maxAllowedProd;
	int ressourceToDistributeB;
	int ressourceToDistributeC;

	int ressourceTypeB;
	int ressourceTypeC;

	switch (ressourceType)
	{
		case RES_METAL:
			maxAllowedProd = getMaxMetalProd();
			ressourceToDistributeB = goldProd;
			ressourceToDistributeC = oilProd;
			ressourceTypeB = RES_GOLD;
			ressourceTypeC = RES_OIL;
			break;
		case RES_OIL:
			maxAllowedProd = getMaxOilProd();
			ressourceToDistributeB = metalProd;
			ressourceToDistributeC = goldProd;
			ressourceTypeB = RES_METAL;
			ressourceTypeC = RES_GOLD;
			break;
		case RES_GOLD:
			maxAllowedProd = getMaxGoldProd();
			ressourceToDistributeB = metalProd;
			ressourceToDistributeC = oilProd;
			ressourceTypeB = RES_METAL;
			ressourceTypeC = RES_OIL;
			break;
		default:
			return 0;
	}

	// when calculating the maximum allowed production for ressource A,
	// the algo tries to distribute the ressources B and C
	// so that the maximum possible production capacity is left over for A.
	// the actual production values of each mine are not saved,
	// because they are not needed.

	// step one:
	// distribute ressources,
	// that do not decrease the possible production of the others
	for (size_t i = 0; i != buildings.size(); ++i)
	{
		const cBuilding& building = *buildings[i];

		if (building.getStaticUnitData().canMineMaxRes <= 0 || !building.isUnitWorking()) continue;

		// how much of B can be produced in this mine,
		// without decreasing the possible production of A and C?
		int amount = min (building.getMaxProd(ressourceTypeB), building.getStaticUnitData().canMineMaxRes - building.getMaxProd(ressourceType) - building.getMaxProd(ressourceTypeC));
		if (amount > 0) ressourceToDistributeB -= amount;

		// how much of C can be produced in this mine,
		// without decreasing the possible production of A and B?
		amount = min(building.getMaxProd(ressourceTypeC), building.getStaticUnitData().canMineMaxRes - building.getMaxProd(ressourceType) - building.getMaxProd(ressourceTypeB));
		if (amount > 0) ressourceToDistributeC -= amount;
	}

	ressourceToDistributeB = std::max (ressourceToDistributeB, 0);
	ressourceToDistributeC = std::max (ressourceToDistributeC, 0);

	// step two:
	// distribute ressources, that do not decrease the possible production of A
	for (size_t i = 0; i != buildings.size(); ++i)
	{
		const cBuilding& building = *buildings[i];

		if (building.getStaticUnitData().canMineMaxRes <= 0 || !building.isUnitWorking()) continue;

		int freeB = min(building.getStaticUnitData().canMineMaxRes - building.getMaxProd(ressourceType), building.getMaxProd(ressourceTypeB));
		int freeC = min(building.getStaticUnitData().canMineMaxRes - building.getMaxProd(ressourceType), building.getMaxProd(ressourceTypeC));

		// subtract values from step 1
		freeB -= min(max(building.getStaticUnitData().canMineMaxRes - building.getMaxProd(ressourceType) - building.getMaxProd(ressourceTypeC), 0), building.getMaxProd(ressourceTypeB));
		freeC -= min(max(building.getStaticUnitData().canMineMaxRes - building.getMaxProd(ressourceType) - building.getMaxProd(ressourceTypeB), 0), building.getMaxProd(ressourceTypeC));

		if (ressourceToDistributeB > 0)
		{
			const int value = min (freeB, ressourceToDistributeB);
			freeC -= value;
			ressourceToDistributeB -= value;
		}
		if (ressourceToDistributeC > 0)
		{
			ressourceToDistributeC -= min (freeC, ressourceToDistributeC);
		}
	}

	// step three:
	// the remaining amount of B and C have to be subtracted
	// from the maximum allowed production of A
	maxAllowedProd -= ressourceToDistributeB + ressourceToDistributeC;

	maxAllowedProd = std::max (maxAllowedProd, 0);

	return maxAllowedProd;
}


static bool isAOnlineStation (const cBuilding& building)
{
	return building.getStaticUnitData().produceEnergy > 1 && building.isUnitWorking();
}

static bool isAOfflineStation (const cBuilding& building)
{
	return building.getStaticUnitData().produceEnergy > 1 && !building.isUnitWorking();
}

static bool isAOnlineGenerator (const cBuilding& building)
{
	return building.getStaticUnitData().produceEnergy == 1 && building.isUnitWorking();
}

static bool isAOfflineGenerator (const cBuilding& building)
{
	return building.getStaticUnitData().produceEnergy == 1 && !building.isUnitWorking();
}

template <typename T>
static std::vector<cBuilding*> getFilteredBuildings (std::vector<cBuilding*>& buildings, T filter)
{
	std::vector<cBuilding*> res;
	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];

		if (filter (building)) res.push_back (&building);
	}
	return res;
}

bool cSubBase::increaseEnergyProd (int value)
{
	// TODO: the energy production and fuel consumption
	// of generators and stations are hardcoded in this function
	std::vector<cBuilding*> onlineStations = getFilteredBuildings (buildings, &isAOnlineStation);
	std::vector<cBuilding*> onlineGenerators = getFilteredBuildings (buildings, &isAOnlineGenerator);
	std::vector<cBuilding*> offlineStations = getFilteredBuildings (buildings, &isAOfflineStation);
	std::vector<cBuilding*> offlineGenerators = getFilteredBuildings (buildings, &isAOfflineGenerator);
	const int availableStations = onlineStations.size() + offlineStations.size();
	const int availableGenerators = onlineGenerators.size() + offlineGenerators.size();

	// calc the optimum amount of energy stations and generators
	const int energy = energyProd + value;

	int stations   = min ((energy + 3) / 6, availableStations);
	int generators = max (energy - stations * 6, 0);

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
	if (neededFuel > getOilStored() + getMaxOilProd())
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
			return getMetalStored();
		case eResourceType::Oil:
			return getOilStored();
		case eResourceType::Gold:
			return getGoldStored();
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
		const int iStartValue = value;
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

bool cSubBase::checkGoldConsumer ()
{
	if (goldNeed <= goldProd + getGoldStored()) return false;

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];
		if (!building.getStaticUnitData().convertsGold || !building.isUnitWorking()) continue;

		building.stopWork (false);

		if (goldNeed <= goldProd + getGoldStored()) break;
	}
	return true;
}

bool cSubBase::checkMetalConsumer ()
{
	if (metalNeed <= metalProd + getMetalStored()) return false;

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];
		if (!building.getStaticUnitData().needsMetal || !building.isUnitWorking()) continue;

		building.stopWork (false);

		if (metalNeed <= metalProd + getMetalStored()) break;
	}
	return true;
}

bool cSubBase::checkOil()
{
	// TODO: the energy production and fuel consumption of generators and
	// stations are hardcoded in this function
	std::vector<cBuilding*> onlineStations = getFilteredBuildings (buildings, &isAOnlineStation);
	std::vector<cBuilding*> onlineGenerators = getFilteredBuildings (buildings, &isAOnlineGenerator);
	std::vector<cBuilding*> offlineStations = getFilteredBuildings (buildings, &isAOfflineStation);
	std::vector<cBuilding*> offlineGenerators = getFilteredBuildings (buildings, &isAOfflineGenerator);
	const int availableStations = onlineStations.size() + offlineStations.size();
	const int availableGenerators = onlineGenerators.size() + offlineGenerators.size();

	// calc the optimum amount of energy stations and generators
	int stations   = min ((energyNeed + 3) / 6, availableStations);
	int generators = max (energyNeed - stations * 6, 0);

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
	const int availableOil = getMaxOilProd() + getOilStored();
	bool oilMissing = false;
	if (neededOil > availableOil)
	{
		// reduce energy production to maximum possible value
		stations = min ((availableOil) / 6, availableStations);
		generators = min ((availableOil - (stations * 6)) / 2, availableGenerators);

		oilMissing = true;
	}

	// increase oil production, if necessary
	neededOil = stations * 6 + generators * 2;
	if (neededOil > oilProd + getOilStored())
	{
		// temporary decrease gold and metal production
		const int missingOil = neededOil - oilProd - getOilStored();
		const int oldGoldProd = goldProd;
		const int oldMetalProd = metalProd;
		setMetalProd (0);
		setGoldProd (0);

		changeOilProd (missingOil);

		setGoldProd(oldGoldProd);
		setMetalProd(oldMetalProd);

		base.forcedRessouceProductionChance(RES_OIL, missingOil, true);
		if (getMetalProd() < oldMetalProd)
			base.forcedRessouceProductionChance(RES_METAL, oldMetalProd - metalProd, false);
		if (getGoldProd() < oldGoldProd)
			base.forcedRessouceProductionChance(RES_GOLD, oldGoldProd - goldProd, false);
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
		if (building.getMaxProd(RES_METAL) > 0 ||
			building.getMaxProd(RES_GOLD) > 0 ||
			building.getMaxProd(RES_OIL) > 0) continue;

		building.stopWork (false);

		if (energyNeed <= energyProd) return true;
	}

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];
		if (!building.getStaticUnitData().needsEnergy || !building.isUnitWorking()) continue;

		// do not shut down oil producers in the second run
		if (building.getMaxProd(RES_OIL) > 0) continue;

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
		|| getMetalStored() <= 0 || building.hasBeenAttacked())
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
	if (building.getStaticUnitData().canAttack && building.data.getAmmo() == 0 && getMetalStored() > 0)
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
		// so we first subtract the old value from MetalNeed and
		// then add the new one, to hold the base up to date
		metalNeed -= building.getMetalPerRound();

		auto value = buildListItem.getRemainingMetal() - building.getMetalPerRound();
		value = std::max (value, 0);
		buildListItem.setRemainingMetal (value);

		metalNeed += building.getMetalPerRound();
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
	addOil (oilProd - oilNeed);
	addMetal (metalProd - metalNeed);
	addGold (goldProd - goldNeed);

	// produce credits
	if (goldNeed)
	{
		base.owner.setCredits (base.owner.getCredits() + goldNeed);
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
	auto newMetal = std::min (this->maxMetalStored, this->getMetalStored());
	auto newOil = std::min (this->maxOilStored, this->getOilStored());
	auto newGold = std::min (this->maxGoldStored, this->getGoldStored());

	// should not happen, but to be sure:
	newMetal = std::max (newMetal, 0);
	newOil = std::max (newOil, 0);
	newGold = std::max (newGold, 0);

	setMetal (newMetal);
	setOil (newOil);
	setGold (newGold);
}

void cSubBase::merge (cSubBase* sb)
{
	// merge buildings
	for (size_t i = 0; i != sb->buildings.size(); ++i)
	{
		cBuilding* building = sb->buildings[i];
		addBuilding (building);
	}
	sb->buildings.clear();

	// delete the subbase from the subbase list
	Remove (base.SubBases, sb);
}

void cSubBase::addBuilding (cBuilding* b)
{
	buildings.push_back (b);
	b->subBase = this;

	// calculate storage level
	switch (b->getStaticUnitData().storeResType)
	{
		case eResourceType::Metal:
			maxMetalStored += b->getStaticUnitData().storageResMax;
			setMetal (getMetalStored() + b->getStoredResources());
			break;
		case eResourceType::Oil:
			maxOilStored += b->getStaticUnitData().storageResMax;
			setOil (getOilStored() + b->getStoredResources());
			break;
		case eResourceType::Gold:
			maxGoldStored += b->getStaticUnitData().storageResMax;
			setGold (getGoldStored() + b->getStoredResources());
			break;
		case eResourceType::None:
			break;
	}
	// calculate energy
	if (b->getStaticUnitData().produceEnergy)
	{
		maxEnergyProd += b->getStaticUnitData().produceEnergy;
		maxOilNeed += b->getStaticUnitData().needsOil;
		if (b->isUnitWorking())
		{
			energyProd += b->getStaticUnitData().produceEnergy;
			oilNeed += b->getStaticUnitData().needsOil;
		}
	}
	else if (b->getStaticUnitData().needsEnergy)
	{
		maxEnergyNeed += b->getStaticUnitData().needsEnergy;
		if (b->isUnitWorking())
		{
			energyNeed += b->getStaticUnitData().needsEnergy;
		}
	}
	// calculate ressource consumption
	if (b->getStaticUnitData().needsMetal)
	{
		maxMetalNeed += b->getStaticUnitData().needsMetal * 12;
		if (b->isUnitWorking())
		{
			metalNeed += min (b->getMetalPerRound(), b->getBuildListItem (0).getRemainingMetal());
		}
	}
	// calculate gold
	if (b->getStaticUnitData().convertsGold)
	{
		maxGoldNeed += b->getStaticUnitData().convertsGold;
		if (b->isUnitWorking())
		{
			goldNeed += b->getStaticUnitData().convertsGold;
		}
	}
	// calculate ressource production
	if (b->getStaticUnitData().canMineMaxRes > 0 && b->isUnitWorking())
	{
		metalProd += b->metalProd;
		oilProd += b->oilProd;
		goldProd += b->goldProd;
	}
	// calculate humans
	if (b->getStaticUnitData().produceHumans)
	{
		humanProd += b->getStaticUnitData().produceHumans;
	}
	if (b->getStaticUnitData().needsHumans)
	{
		maxHumanNeed += b->getStaticUnitData().needsHumans;
		if (b->isUnitWorking())
		{
			humanNeed += b->getStaticUnitData().needsHumans;
		}
	}
}

bool cSubBase::startBuilding(cBuilding* b)
{
	const cStaticUnitData& staticData = b->getStaticUnitData();
	
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
		if (goldNeed + staticData.convertsGold > goldProd + goldStored)
		{
			base.goldInsufficient();
			return false;
		}
	}

	// needs raw material:
	if (staticData.needsMetal)
	{
		if (metalNeed + b->getMetalPerRound() > metalProd + metalStored)
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
		if (staticData.needsOil + oilNeed > oilStored + getMaxOilProd())
		{
			base.fuelInsufficient();
			return false;
		}
		else if (staticData.needsOil + oilNeed > oilStored + oilProd)
		{
			// increase oil production
			int missingOil = staticData.needsOil + oilNeed - (oilStored + oilProd);

			int oldMetalProd = metalProd;
			int oldGoldProd = goldProd;

			// temporay decrease metal and gold production
			setMetalProd(0);
			setGoldProd(0);

			changeOilProd(missingOil);

			// restore as much production as possible
			setGoldProd(oldGoldProd);
			setMetalProd(oldMetalProd);

			base.forcedRessouceProductionChance(RES_OIL, missingOil, true);
			if (metalProd < oldMetalProd)
				base.forcedRessouceProductionChance(RES_METAL, oldMetalProd - metalProd, false);
			if (goldProd < oldGoldProd)
				base.forcedRessouceProductionChance(RES_GOLD, oldGoldProd - goldProd, false);
		}
	}

	// IsWorking is set to true before checking the energy production.
	// So if an energy generator has to be started,
	// it can use the fuel production of this building
	// (when this building is a mine).
	b->setWorking(true);

	// set mine values. This has to be undone, if the energy is insufficient
	if (staticData.canMineMaxRes > 0)
	{
		changeMetalProd(b->metalProd);
		changeGoldProd(b->goldProd);
		changeOilProd(b->oilProd);
	}

	// Energy consumers:
	if (staticData.needsEnergy)
	{
		if (staticData.needsEnergy + energyNeed > energyProd)
		{
			// try to increase energy production
			if (!increaseEnergyProd(staticData.needsEnergy + energyNeed - energyProd))
			{
				b->setWorking(false);

				// reset mine values
				if (staticData.canMineMaxRes > 0)
				{
					changeMetalProd(- b->metalProd);
					changeGoldProd(- b->goldProd);
					changeOilProd(- b->oilProd);
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

	oilNeed += staticData.needsOil;

	// raw material consumer:
	if (staticData.needsMetal)
		metalNeed += b->getMetalPerRound();

	// gold consumer:
	goldNeed += staticData.convertsGold;

	return true;
}

bool cSubBase::stopBuilding(cBuilding* b, bool forced /*= false*/)
{
	const cStaticUnitData& staticData = b->getStaticUnitData();

	// energy generators
	if (staticData.produceEnergy)
	{
		if (energyNeed > energyProd - staticData.produceEnergy && !forced)
		{
			base.energyIsNeeded();
			return false;
		}

		energyProd -= staticData.produceEnergy;
		oilNeed -= staticData.needsOil;
	}

	b->setWorking(false);

	// Energy consumers:
	energyNeed -= staticData.needsEnergy;

	// raw material consumer:
	if (staticData.needsMetal)
		metalNeed -= b->getMetalPerRound();

	// gold consumer
	goldNeed -= staticData.convertsGold;

	// human consumer
	humanNeed -= staticData.needsHumans;

	// Minen:
	if (staticData.canMineMaxRes > 0)
	{
		metalProd -= b->metalProd;
		oilProd -= b->oilProd;
		goldProd -= b->goldProd;
	}

	return true;
}

cBase::cBase(cPlayer& owner) : 
	owner(owner)
{}

cBase::~cBase()
{
	for (size_t i = 0; i != SubBases.size(); ++i)
	{
		delete SubBases[i];
	}
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

void cBase::addBuilding (cBuilding* building, const cMap& map)
{
	if (!building->getStaticUnitData().connectsToBase) return;
	std::vector<cSubBase*> NeighbourList;

	// find all neighbouring subbases
	if (building->getIsBig())
	{
		// big building
		if (cSubBase* SubBase = checkNeighbour (building->getPosition() + cPosition (0, -1), *building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building->getPosition() + cPosition (1, -1), *building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building->getPosition() + cPosition (2, 0), *building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building->getPosition() + cPosition (2, 1), *building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building->getPosition() + cPosition (0, 2), *building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building->getPosition() + cPosition (1, 2), *building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building->getPosition() + cPosition (-1, 0), *building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building->getPosition() + cPosition (-1, 1), *building, map)) NeighbourList.push_back (SubBase);
	}
	else
	{
		// small building
		if (cSubBase* SubBase = checkNeighbour (building->getPosition() + cPosition (0, -1), *building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building->getPosition() + cPosition (1, 0), *building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building->getPosition() + cPosition (0, 1), *building, map)) NeighbourList.push_back (SubBase);
		if (cSubBase* SubBase = checkNeighbour (building->getPosition() + cPosition (-1, 0), *building, map)) NeighbourList.push_back (SubBase);
	}
	building->CheckNeighbours (map);

	RemoveDuplicates (NeighbourList);

	if (NeighbourList.empty())
	{
		// no neighbours found, just generate new subbase and add the building
		cSubBase* NewSubBase = new cSubBase (*this);
		NewSubBase->addBuilding (building);
		SubBases.push_back (NewSubBase);

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
		firstNeighbour->merge (SubBase);

		delete SubBase;
	}
	NeighbourList.clear();
}

void cBase::deleteBuilding (cBuilding* building, const cMap& map)
{
	if (!building->getStaticUnitData().connectsToBase) return;
	cSubBase* sb = building->subBase;

	// remove the current subbase
	for (auto b : sb->getBuildings())
	{
		b->subBase = nullptr;
	}
	Remove (SubBases, sb);

	// add all the buildings again
	for (auto b : sb->getBuildings())
	{
		if (b == building) continue;
		addBuilding (b, map);
	}

	if (building->isUnitWorking() && building->getStaticUnitData().canResearch)
		building->getOwner()->stopAResearch (building->getResearchArea());

	delete sb;
}

bool cBase::checkTurnEnd ()
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

void cBase::reset()
{
	for (cSubBase* sb : SubBases)
	{
		delete sb;
	}
	SubBases.resize(0);
}
