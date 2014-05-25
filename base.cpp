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
#include "base.h"

#include "buildings.h"
#include "clientevents.h"
#include "clist.h"
#include "log.h"
#include "map.h"
#include "netmessage.h"
#include "player.h"
#include "server.h"
#include "serverevents.h"
#include "game/data/report/savedreporttranslated.h"

using namespace std;

sSubBase::sSubBase (cPlayer* owner_) :
	buildings(),
	owner (owner_),
	MaxMetal(),
	MaxOil(),
	MaxGold(),
	MaxEnergyProd(),
	EnergyProd(),
	MaxEnergyNeed(),
	EnergyNeed(),
	MetalNeed(),
	OilNeed(),
	GoldNeed(),
	MaxMetalNeed(),
	MaxOilNeed(),
	MaxGoldNeed(),
	HumanProd(),
	HumanNeed(),
	MaxHumanNeed(),
	MetalProd(),
	OilProd(),
	GoldProd(),
	metal(),
	oil(),
	gold()
{}

sSubBase::sSubBase (const sSubBase& other) :
	buildings (other.buildings),
	owner (other.owner),
	MaxMetal (other.MaxMetal),
	MaxOil (other.MaxOil),
	MaxGold (other.MaxGold),
	MaxEnergyProd (other.MaxEnergyProd),
	EnergyProd (other.EnergyProd),
	MaxEnergyNeed (other.MaxEnergyNeed),
	EnergyNeed (other.EnergyNeed),
	MetalNeed (other.MetalNeed),
	OilNeed (other.OilNeed),
	GoldNeed (other.GoldNeed),
	MaxMetalNeed (other.MaxMetalNeed),
	MaxOilNeed (other.MaxOilNeed),
	MaxGoldNeed (other.MaxGoldNeed),
	HumanProd (other.HumanProd),
	HumanNeed (other.HumanNeed),
	MaxHumanNeed (other.MaxHumanNeed),
	MetalProd (other.MetalProd),
	OilProd (other.OilProd),
	GoldProd (other.GoldProd),
	metal (other.metal),
	oil (other.oil),
	gold (other.gold)
{}

int sSubBase::getMaxMetalProd() const
{
	return calcMaxProd (RES_METAL);
}

int sSubBase::getMaxGoldProd() const
{
	return calcMaxProd (RES_GOLD);
}

int sSubBase::getMaxOilProd() const
{
	return calcMaxProd (RES_OIL);
}

int sSubBase::getMaxAllowedMetalProd() const
{
	return calcMaxAllowedProd (RES_METAL);
}

int sSubBase::getMaxAllowedGoldProd() const
{
	return calcMaxAllowedProd (RES_GOLD);
}

int sSubBase::getMaxAllowedOilProd() const
{
	return calcMaxAllowedProd (RES_OIL);
}

int sSubBase::getMetalProd() const
{
	return MetalProd;
}

int sSubBase::getGoldProd() const
{
	return GoldProd;
}

int sSubBase::getOilProd() const
{
	return OilProd;
}

void sSubBase::setMetalProd (int value)
{
	const int max = getMaxAllowedMetalProd();

	value = std::max (0, value);
	value = std::min (value, max);

	MetalProd = value;
}

void sSubBase::setGoldProd (int value)
{
	const int max = getMaxAllowedGoldProd();

	value = std::max (0, value);
	value = std::min (value, max);

	GoldProd = value;
}

void sSubBase::setOilProd (int value)
{
	const int max = getMaxAllowedOilProd();

	value = std::max (0, value);
	value = std::min (value, max);

	OilProd = value;
}

int sSubBase::getMetal () const
{
	return metal;
}

void sSubBase::setMetal (int value)
{
	std::swap (metal, value);
	if (metal != value) metalChanged ();
}

int sSubBase::getOil () const
{
	return oil;
}

void sSubBase::setOil (int value)
{
	std::swap (oil, value);
	if (oil != value) oilChanged ();
}

int sSubBase::getGold () const
{
	return gold;
}

void sSubBase::setGold (int value)
{
	std::swap (gold, value);
	if (gold != value) goldChanged ();
}

void sSubBase::changeMetalProd (int value)
{
	setMetalProd (MetalProd + value);
}

void sSubBase::changeOilProd (int value)
{
	setOilProd (OilProd + value);
}

void sSubBase::changeGoldProd (int value)
{
	setGoldProd (GoldProd + value);
}

int sSubBase::calcMaxProd (int ressourceType) const
{
	int maxProd = 0;
	for (size_t i = 0; i != buildings.size(); ++i)
	{
		const cBuilding& building = *buildings[i];

		if (building.data.canMineMaxRes <= 0 || !building.isUnitWorking ()) continue;

		switch (ressourceType)
		{
			case RES_METAL: maxProd += building.MaxMetalProd; break;
			case RES_OIL: maxProd += building.MaxOilProd; break;
			case RES_GOLD: maxProd += building.MaxGoldProd; break;
		}
	}
	return maxProd;
}

int sSubBase::calcMaxAllowedProd (int ressourceType) const
{
	// initialise needed Variables and element pointers,
	// so the algorithm itself is independent from the ressouce type
	int maxAllowedProd;
	int ressourceToDistributeB;
	int ressourceToDistributeC;

	int cBuilding::* ressourceProdA;
	int cBuilding::* ressourceProdB;
	int cBuilding::* ressourceProdC;

	switch (ressourceType)
	{
		case RES_METAL:
			maxAllowedProd = getMaxMetalProd();
			ressourceToDistributeB = GoldProd;
			ressourceToDistributeC = OilProd;
			ressourceProdA = &cBuilding::MaxMetalProd;
			ressourceProdB = &cBuilding::MaxGoldProd;
			ressourceProdC = &cBuilding::MaxOilProd;
			break;
		case RES_OIL:
			maxAllowedProd = getMaxOilProd();
			ressourceToDistributeB = MetalProd;
			ressourceToDistributeC = GoldProd;
			ressourceProdA = &cBuilding::MaxOilProd;
			ressourceProdB = &cBuilding::MaxMetalProd;
			ressourceProdC = &cBuilding::MaxGoldProd;
			break;
		case RES_GOLD:
			maxAllowedProd = getMaxGoldProd();
			ressourceToDistributeB = MetalProd;
			ressourceToDistributeC = OilProd;
			ressourceProdA = &cBuilding::MaxGoldProd;
			ressourceProdB = &cBuilding::MaxMetalProd;
			ressourceProdC = &cBuilding::MaxOilProd;
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

		if (building.data.canMineMaxRes <= 0 || !building.isUnitWorking ()) continue;

		// how much of B can be produced in this mine,
		// without decreasing the possible production of A and C?
		int amount = min (building.*ressourceProdB, building.data.canMineMaxRes - building.*ressourceProdA - building.*ressourceProdC);
		if (amount > 0) ressourceToDistributeB -= amount;

		// how much of C can be produced in this mine,
		// without decreasing the possible production of A and B?
		amount = min (building.*ressourceProdC, building.data.canMineMaxRes - building.*ressourceProdA - building.*ressourceProdB);
		if (amount > 0) ressourceToDistributeC -= amount;
	}

	ressourceToDistributeB = std::max (ressourceToDistributeB, 0);
	ressourceToDistributeC = std::max (ressourceToDistributeC, 0);

	// step two:
	// distribute ressources, that do not decrease the possible production of A
	for (size_t i = 0; i != buildings.size(); ++i)
	{
		const cBuilding& building = *buildings[i];

		if (building.data.canMineMaxRes <= 0 || !building.isUnitWorking ()) continue;

		int freeB = min (building.data.canMineMaxRes - building.*ressourceProdA, building.*ressourceProdB);
		int freeC = min (building.data.canMineMaxRes - building.*ressourceProdA, building.*ressourceProdC);

		// subtract values from step 1
		freeB -= min (max (building.data.canMineMaxRes - building.*ressourceProdA - building.*ressourceProdC, 0), building.*ressourceProdB);
		freeC -= min (max (building.data.canMineMaxRes - building.*ressourceProdA - building.*ressourceProdB, 0), building.*ressourceProdC);

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
	return building.data.produceEnergy > 1 && building.isUnitWorking ();
}

static bool isAOfflineStation (const cBuilding& building)
{
	return building.data.produceEnergy > 1 && !building.isUnitWorking ();
}

static bool isAOnlineGenerator (const cBuilding& building)
{
	return building.data.produceEnergy == 1 && building.isUnitWorking ();
}

static bool isAOfflineGenerator (const cBuilding& building)
{
	return building.data.produceEnergy == 1 && !building.isUnitWorking ();
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

bool sSubBase::increaseEnergyProd (cServer& server, int value)
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
	const int energy = EnergyProd + value;

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
	if (neededFuel > getOil() + getMaxOilProd())
	{
		// not possible to produce enough fuel
		sendSavedReport (server, cSavedReportTranslated ("Text~Comp~Fuel_Insufficient", true), owner);
		return false;
	}

	// stop unneeded buildings
	for (int i = (int) onlineStations.size() - stations; i > 0; --i)
	{
		onlineStations[0]->ServerStopWork (server, true);
		onlineStations.erase (onlineStations.begin());
	}
	for (int i = (int) onlineGenerators.size() - generators; i > 0; --i)
	{
		onlineGenerators[0]->ServerStopWork (server, true);
		onlineGenerators.erase (onlineGenerators.begin());
	}

	// start needed buildings
	for (int i = stations - (int) onlineStations.size(); i > 0; --i)
	{
		offlineStations[0]->ServerStartWork (server);
		offlineStations.erase (offlineStations.begin());
	}
	for (int i = generators - (int) onlineGenerators.size(); i > 0; --i)
	{
		offlineGenerators[0]->ServerStartWork (server);
		offlineGenerators.erase (offlineGenerators.begin());
	}
	return true;
}

void sSubBase::addMetal (cServer& server, int value)
{
	addRessouce (server, sUnitData::STORE_RES_METAL, value);
}

void sSubBase::addOil (cServer& server, int value)
{
	addRessouce (server, sUnitData::STORE_RES_OIL, value);
}

void sSubBase::addGold (cServer& server, int value)
{
	addRessouce (server, sUnitData::STORE_RES_GOLD, value);
}

int sSubBase::getResource (sUnitData::eStorageResType storeResType) const
{
	switch (storeResType)
	{
	case sUnitData::STORE_RES_METAL:
		return getMetal ();
	case sUnitData::STORE_RES_OIL:
		return getOil ();
	case sUnitData::STORE_RES_GOLD:
		return getGold();
	default:
		assert (0);
	}
	return 0;
}

void sSubBase::setResource (sUnitData::eStorageResType storeResType, int value)
{
	switch (storeResType)
	{
	case sUnitData::STORE_RES_METAL:
		setMetal (value);
		break;
	case sUnitData::STORE_RES_OIL:
		setOil (value);
		break;
	case sUnitData::STORE_RES_GOLD:
		setGold (value);
		break;
	default:
		assert (0);
	}
}

void sSubBase::addRessouce (cServer& server, sUnitData::eStorageResType storeResType, int value)
{
	int storedRessources = getResource (storeResType);
	value = std::max (value, -storedRessources);
	if (value == 0) return;
	setResource (storeResType, storedRessources + value);

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& b = *buildings[i];
		if (b.data.storeResType != storeResType) continue;
		const int iStartValue = value;
		if (value < 0)
		{
			if (b.data.storageResCur > -value)
			{
				b.data.storageResCur += value;
				value = 0;
			}
			else
			{
				value += b.data.storageResCur;
				b.data.storageResCur = 0;
			}
		}
		else
		{
			if (b.data.storageResMax - b.data.storageResCur > value)
			{
				b.data.storageResCur += value;
				value = 0;
			}
			else
			{
				value -= b.data.storageResMax - b.data.storageResCur;
				b.data.storageResCur = b.data.storageResMax;
			}
		}
		if (iStartValue != value) sendUnitData (server, b, *owner);
		if (value == 0) break;
	}
	sendSubbaseValues (server, *this, *owner);
}

void sSubBase::refresh()
{
	// copy buildings list
	const std::vector<cBuilding*> buildingsCopy = buildings;

	// reset subbase
	buildings.clear();
	MaxMetal = 0;
	setMetal(0);
	MaxOil = 0;
	setOil (0);
	MaxGold = 0;
	setGold (0);
	MaxEnergyProd = 0;
	EnergyProd = 0;
	MaxEnergyNeed = 0;
	EnergyNeed = 0;
	MetalNeed = 0;
	OilNeed = 0;
	GoldNeed = 0;
	MaxMetalNeed = 0;
	MaxOilNeed = 0;
	MaxGoldNeed = 0;
	MetalProd = 0;
	OilProd = 0;
	GoldProd = 0;
	HumanProd = 0;
	HumanNeed = 0;
	MaxHumanNeed = 0;

	// read all buildings
	for (size_t i = 0; i != buildingsCopy.size(); ++i)
	{
		addBuilding (buildingsCopy[i]);
	}
}

bool sSubBase::checkHumanConsumer (cServer& server)
{
	if (HumanNeed <= HumanProd) return false;

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];
		if (!building.data.needsHumans || !building.isUnitWorking ()) continue;

		building.ServerStopWork (server, false);

		if (HumanNeed <= HumanProd) break;
	}
	return true;
}

bool sSubBase::checkGoldConsumer (cServer& server)
{
	if (GoldNeed <= GoldProd + getGold()) return false;

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];
		if (!building.data.convertsGold || !building.isUnitWorking ()) continue;

		building.ServerStopWork (server, false);

		if (GoldNeed <= GoldProd + getGold()) break;
	}
	return true;
}

bool sSubBase::checkMetalConsumer (cServer& server)
{
	if (MetalNeed <= MetalProd + getMetal ()) return false;

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];
		if (!building.data.needsMetal || !building.isUnitWorking ()) continue;

		building.ServerStopWork (server, false);

		if (MetalNeed <= MetalProd + getMetal ()) break;
	}
	return true;
}

bool sSubBase::checkOil (cServer& server)
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
	int stations   = min ((EnergyNeed + 3) / 6, availableStations);
	int generators = max (EnergyNeed - stations * 6, 0);

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
	const int availableOil = getMaxOilProd () + getOil ();
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
	if (neededOil > OilProd + getOil ())
	{
		// temporary decrease gold and metal production
		const int missingOil = neededOil - OilProd - getOil ();
		const int gold = GoldProd;
		const int metal = MetalProd;
		setMetalProd (0);
		setGoldProd (0);

		changeOilProd (missingOil);

		setGoldProd (gold);
		setMetalProd (metal);

		sendSavedReport (server, cSavedReportTranslated ("Text~Comp~Adjustments_Fuel_Increased", iToStr (missingOil)), owner);
		if (getMetalProd () < metal)
			sendSavedReport (server, cSavedReportTranslated ("Text~Comp~Adjustments_Metal_Decreased", iToStr (metal - MetalProd)), owner);
		if (getGoldProd() < gold)
			sendSavedReport (server, cSavedReportTranslated ("Text~Comp~Adjustments_Gold_Decreased", iToStr (gold - GoldProd)), owner);
	}

	// stop unneeded buildings
	for (int i = (int) onlineStations.size() - stations; i > 0; i--)
	{
		onlineStations[0]->ServerStopWork (server, true);
		onlineStations.erase (onlineStations.begin());
	}
	for (int i = (int) onlineGenerators.size() - generators; i > 0; i--)
	{
		onlineGenerators[0]->ServerStopWork (server, true);
		onlineGenerators.erase (onlineGenerators.begin());
	}

	// start needed buildings
	for (int i = stations - (int) onlineStations.size(); i > 0; i--)
	{
		offlineStations[0]->ServerStartWork (server);
		offlineStations.erase (offlineStations.begin());
	}
	for (int i = generators - (int) onlineGenerators.size(); i > 0; i--)
	{
		offlineGenerators[0]->ServerStartWork (server);
		offlineGenerators.erase (offlineGenerators.begin());
	}

	// temporary debug check
	if (isDitributionMaximized() == false)
	{
		Log.write (" Server: Mine distribution values are not a maximum", cLog::eLOG_TYPE_NET_WARNING);
	}

	return oilMissing;
}

bool sSubBase::checkEnergy (cServer& server)
{
	if (EnergyNeed <= EnergyProd) return false;

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];
		if (!building.data.needsEnergy || !building.isUnitWorking ()) continue;

		// do not shut down ressource producers in the first run
		if (building.MaxOilProd > 0 ||
			building.MaxGoldProd > 0 ||
			building.MaxMetalProd > 0) continue;

		building.ServerStopWork (server, false);

		if (EnergyNeed <= EnergyProd) return true;
	}

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];
		if (!building.data.needsEnergy || !building.isUnitWorking ()) continue;

		// do not shut down oil producers in the second run
		if (building.MaxOilProd > 0) continue;

		building.ServerStopWork (server, false);

		if (EnergyNeed <= EnergyProd) return true;
	}

	// if energy is still missing, shut down also oil producers
	for (size_t i = 0; i < buildings.size(); i++)
	{
		cBuilding& building = *buildings[i];
		if (!building.data.needsEnergy || !building.isUnitWorking ()) continue;

		building.ServerStopWork (server, false);

		if (EnergyNeed <= EnergyProd) return true;
	}
	return true;
}

void sSubBase::prepareTurnend (cServer& server)
{
	if (checkMetalConsumer (server))
		sendSavedReport (server, cSavedReportTranslated ("Text~Comp~Metal_Low"), owner);

	if (checkHumanConsumer (server))
		sendSavedReport (server, cSavedReportTranslated ("Text~Comp~Team_Low"), owner);

	if (checkGoldConsumer (server))
		sendSavedReport (server, cSavedReportTranslated ("Text~Comp~Gold_Low"), owner);

	// there is a loop around checkOil/checkEnergy,
	// because a lack of energy can lead to less fuel,
	// that can lead to less energy, etc...
	bool oilMissing = false;
	bool energyMissing = false;
	bool changed = true;
	while (changed)
	{
		changed = false;
		if (checkOil (server))
		{
			changed = true;
			oilMissing = true;
		}

		if (checkEnergy (server))
		{
			changed = true;
			energyMissing = true;
		}
	}
	if (oilMissing)
		sendSavedReport (server, cSavedReportTranslated ("Text~Comp~Fuel_Low"), owner);
	if (energyMissing)
		sendSavedReport (server, cSavedReportTranslated ("Text~Comp~Energy_Low"), owner);

	// recheck metal and gold,
	// because metal and gold producers could have been shut down,
	// due to a lack of energy
	if (checkMetalConsumer (server))
		sendSavedReport (server, cSavedReportTranslated ("Text~Comp~Metal_Low"), owner);

	if (checkGoldConsumer (server))
		sendSavedReport (server, cSavedReportTranslated ("Text~Comp~Gold_Low"), owner);
}

void sSubBase::makeTurnend_reparation (cServer& server, cBuilding& building)
{
	// repair (do not repair buildings that have been attacked in this turn):
	if (building.data.getHitpoints () >= building.data.hitpointsMax
		|| getMetal () <= 0 || building.hasBeenAttacked ())
	{
		return;
	}
	// calc new hitpoints
	const auto newHitPoints = building.data.getHitpoints() + Round (((float) building.data.hitpointsMax / building.data.buildCosts) * 4);
	building.data.setHitpoints(std::min (building.data.hitpointsMax, newHitPoints));
	addMetal (server, -1);
	sendUnitData (server, building, *owner);
	for (size_t j = 0; j != building.seenByPlayerList.size(); ++j)
	{
		sendUnitData (server, building, *building.seenByPlayerList[j]);
	}
}

void sSubBase::makeTurnend_reload (cServer& server, cBuilding& building)
{
	// reload:
	if (building.data.canAttack && building.data.getAmmo () == 0 && getMetal () > 0)
	{
		building.data.setAmmo(building.data.ammoMax);
		addMetal (server, -1);
		// ammo is not visible to enemies. So only send to the owner
		sendUnitData (server, building, *owner);
	}
}

void sSubBase::makeTurnend_build (cServer& server, cBuilding& building)
{
	// build:
	if (!building.isUnitWorking () || building.data.canBuild.empty () || building.BuildList.empty ())
	{
		return;
	}

	sBuildList& buildListItem = building.BuildList[0];
	if (buildListItem.metall_remaining > 0)
	{
		// in this block the metal consumption of the factory
		// in the next round can change
		// so we first subtract the old value from MetalNeed and
		// then add the new one, to hold the base up to date
		MetalNeed -= min (building.MetalPerRound, buildListItem.metall_remaining);

		buildListItem.metall_remaining -= min (building.MetalPerRound, buildListItem.metall_remaining);
		buildListItem.metall_remaining = std::max (buildListItem.metall_remaining, 0);

		MetalNeed += min (building.MetalPerRound, buildListItem.metall_remaining);
		sendBuildList (server, building);
		sendSubbaseValues (server, *this, *owner);
	}
	if (buildListItem.metall_remaining <= 0)
	{
		server.addReport (buildListItem.type, owner->getNr());
		building.ServerStopWork (server, false);
	}
}

void sSubBase::makeTurnend (cServer& server)
{
	prepareTurnend (server);

	// produce ressources
	addOil (server, OilProd - OilNeed);
	addMetal (server, MetalProd - MetalNeed);
	addGold (server, GoldProd - GoldNeed);

	// produce credits
	if (GoldNeed)
	{
		owner->Credits += GoldNeed;
		sendCredits (server, owner->Credits, *owner);
	}

	// make repairs/build/reload
	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];

		makeTurnend_reparation (server, building);
		building.setHasBeenAttacked(false);
		makeTurnend_reload (server, building);
		makeTurnend_build (server, building);
	}

	// check maximum storage limits
	auto newMetal = std::min (this->MaxMetal, this->getMetal());
	auto newOil = std::min (this->MaxOil, this->getOil());
	auto newGold = std::min (this->MaxGold, this->getGold());

	// should not happen, but to be sure:
	newMetal = std::max (newMetal, 0);
	newOil = std::max (newOil, 0);
	newGold = std::max (newGold, 0);

	setMetal (newMetal);
	setOil (newOil);
	setGold (newGold);

	sendSubbaseValues (server, *this, *owner);
}

void sSubBase::merge (sSubBase* sb)
{
	// merge ressource allocation
	int metal = MetalProd;
	int oil = OilProd;
	int gold = GoldProd;

	metal += sb->getMetalProd();
	gold  += sb->getGoldProd();
	oil   += sb->getOilProd();

	// merge buildings
	for (size_t i = 0; i != sb->buildings.size(); ++i)
	{
		cBuilding* building = sb->buildings[i];
		addBuilding (building);
		building->SubBase = this;
	}
	sb->buildings.clear();

	// set ressource allocation
	setMetalProd (0);
	setOilProd (0);
	setGoldProd (0);

	setMetalProd (metal);
	setGoldProd (gold);
	setOilProd (oil);

	// delete the subbase from the subbase list
	Remove (owner->base.SubBases, sb);
}

int sSubBase::getID() const
{
	assert (!buildings.empty());

	return buildings[0]->iID;
}

void sSubBase::addBuilding (cBuilding* b)
{
	buildings.push_back (b);
	// calculate storage level
	switch (b->data.storeResType)
	{
		case sUnitData::STORE_RES_METAL:
			MaxMetal += b->data.storageResMax;
			setMetal (getMetal () + b->data.storageResCur);
			break;
		case sUnitData::STORE_RES_OIL:
			MaxOil += b->data.storageResMax;
			setOil (getOil () + b->data.storageResCur);
			break;
		case sUnitData::STORE_RES_GOLD:
			MaxGold += b->data.storageResMax;
			setGold (getGold () + b->data.storageResCur);
			break;
		case sUnitData::STORE_RES_NONE:
			break;
	}
	// calculate energy
	if (b->data.produceEnergy)
	{
		MaxEnergyProd += b->data.produceEnergy;
		MaxOilNeed += b->data.needsOil;
		if (b->isUnitWorking ())
		{
			EnergyProd += b->data.produceEnergy;
			OilNeed += b->data.needsOil;
		}
	}
	else if (b->data.needsEnergy)
	{
		MaxEnergyNeed += b->data.needsEnergy;
		if (b->isUnitWorking ())
		{
			EnergyNeed += b->data.needsEnergy;
		}
	}
	// calculate ressource consumption
	if (b->data.needsMetal)
	{
		MaxMetalNeed += b->data.needsMetal * 12;
		if (b->isUnitWorking ())
		{
			MetalNeed += min (b->MetalPerRound, b->BuildList[0].metall_remaining);
		}
	}
	// calculate gold
	if (b->data.convertsGold)
	{
		MaxGoldNeed += b->data.convertsGold;
		if (b->isUnitWorking ())
		{
			GoldNeed += b->data.convertsGold;
		}
	}
	// calculate ressource production
	if (b->data.canMineMaxRes > 0 && b->isUnitWorking ())
	{
		int mineFree = b->data.canMineMaxRes;
		changeMetalProd (b->MaxMetalProd);
		mineFree -= b->MaxMetalProd;

		changeOilProd (min (b->MaxOilProd, mineFree));
		mineFree -= min (b->MaxOilProd, mineFree);

		changeGoldProd (min (b->MaxGoldProd, mineFree));
	}
	// calculate humans
	if (b->data.produceHumans)
	{
		HumanProd += b->data.produceHumans;
	}
	if (b->data.needsHumans)
	{
		MaxHumanNeed += b->data.needsHumans;
		if (b->isUnitWorking ())
		{
			HumanNeed += b->data.needsHumans;
		}
	}

	// temporary debug check
	if (isDitributionMaximized() == false)
	{
		Log.write (" Server: Mine distribution values are not a maximum", cLog::eLOG_TYPE_NET_WARNING);
	}
}

bool sSubBase::isDitributionMaximized() const
{
	return (getGoldProd() == getMaxAllowedGoldProd() &&
			getMetalProd() == getMaxAllowedMetalProd() &&
			getOilProd() == getMaxAllowedOilProd());
}

void sSubBase::pushInto (cNetMessage& message) const
{
	message.pushInt16 (EnergyProd);
	message.pushInt16 (EnergyNeed);
	message.pushInt16 (MaxEnergyProd);
	message.pushInt16 (MaxEnergyNeed);
	message.pushInt16 (getMetal ());
	message.pushInt16 (MaxMetal);
	message.pushInt16 (MetalNeed);
	message.pushInt16 (MaxMetalNeed);
	message.pushInt16 (getMetalProd());
	message.pushInt16 (getGold());
	message.pushInt16 (MaxGold);
	message.pushInt16 (GoldNeed);
	message.pushInt16 (MaxGoldNeed);
	message.pushInt16 (getGoldProd());
	message.pushInt16 (getOil ());
	message.pushInt16 (MaxOil);
	message.pushInt16 (OilNeed);
	message.pushInt16 (MaxOilNeed);
	message.pushInt16 (getOilProd());
	message.pushInt16 (HumanNeed);
	message.pushInt16 (MaxHumanNeed);
	message.pushInt16 (HumanProd);
}

void sSubBase::popFrom (cNetMessage& message)
{
	HumanProd = message.popInt16();
	MaxHumanNeed = message.popInt16();
	HumanNeed = message.popInt16();
	OilProd = message.popInt16();
	MaxOilNeed = message.popInt16();
	OilNeed = message.popInt16();
	MaxOil = message.popInt16();
	setOil(message.popInt16());
	GoldProd = message.popInt16();
	MaxGoldNeed = message.popInt16();
	GoldNeed = message.popInt16();
	MaxGold = message.popInt16();
	setGold(message.popInt16());
	MetalProd = message.popInt16();
	MaxMetalNeed = message.popInt16();
	MetalNeed = message.popInt16();
	MaxMetal = message.popInt16();
	setMetal(message.popInt16());
	MaxEnergyNeed = message.popInt16();
	MaxEnergyProd = message.popInt16();
	EnergyNeed = message.popInt16();
	EnergyProd = message.popInt16();
}

cBase::cBase() : map()
{}

cBase::~cBase()
{
	for (size_t i = 0; i != SubBases.size(); ++i)
	{
		delete SubBases[i];
	}
}

sSubBase* cBase::checkNeighbour (const cPosition& position, const cBuilding& building)
{
	if (map->isValidPosition (position) == false) return NULL;
	cBuilding* b = map->getField (position).getBuilding ();

	if (b && b->owner == building.owner && b->SubBase)
	{
		b->CheckNeighbours (*map);
		return b->SubBase;
	}
	return NULL;
}

void cBase::addBuilding (cBuilding* building, cServer* server)
{
	if (!building->data.connectsToBase) return;
	std::vector<sSubBase*> NeighbourList;

	// check for neighbours
	if (building->data.isBig)
	{
		// big building
		if (sSubBase* SubBase = checkNeighbour (building->getPosition () + cPosition (0, -1), *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (building->getPosition () + cPosition (1, -1), *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (building->getPosition () + cPosition (2, 0), *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (building->getPosition () + cPosition (2, 1), *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (building->getPosition () + cPosition (0, 2), *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (building->getPosition () + cPosition (1, 2), *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (building->getPosition () + cPosition (-1, 0), *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (building->getPosition () + cPosition (-1, 1), *building)) NeighbourList.push_back (SubBase);
	}
	else
	{
		// small building
		if (sSubBase* SubBase = checkNeighbour (building->getPosition () + cPosition (0, -1), *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (building->getPosition () + cPosition (1, 0), *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (building->getPosition () + cPosition (0, 1), *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (building->getPosition () + cPosition (-1, 0), *building)) NeighbourList.push_back (SubBase);
	}
	building->CheckNeighbours (*map);

	RemoveDuplicates (NeighbourList);

	if (NeighbourList.empty())
	{
		// no neighbours found, just generate new subbase and add the building
		sSubBase* NewSubBase = new sSubBase (building->owner);
		building->SubBase = NewSubBase;
		NewSubBase->addBuilding (building);
		SubBases.push_back (NewSubBase);

		if (server) sendSubbaseValues (*server, *NewSubBase, *NewSubBase->owner);

		return;
	}

	// found neighbours, so add the building to the first neighbour subbase
	sSubBase* const firstNeighbour = NeighbourList[0];
	firstNeighbour->addBuilding (building);
	building->SubBase = firstNeighbour;
	NeighbourList.erase (NeighbourList.begin());

	// now merge the other neighbours to the first one, if necessary
	for (size_t i = 0; i != NeighbourList.size(); ++i)
	{
		sSubBase* const SubBase = NeighbourList[i];
		firstNeighbour->merge (SubBase);

		delete SubBase;
	}
	NeighbourList.clear();
	if (server) sendSubbaseValues (*server, *firstNeighbour, *building->owner);
}

void cBase::deleteBuilding (cBuilding* building, cServer* server)
{
	if (!building->data.connectsToBase) return;
	sSubBase* sb = building->SubBase;

	// remove the current subbase
	for (size_t i = 0; i != sb->buildings.size(); ++i)
	{
		sb->buildings[i]->SubBase = NULL;
	}
	Remove (SubBases, sb);

	// save ressource allocation
	int metal = sb->getMetalProd();
	int gold = sb->getGoldProd();
	int oil = sb->getOilProd();

	// add all the buildings again
	for (size_t i = 0; i != sb->buildings.size(); ++i)
	{
		cBuilding* n = sb->buildings[i];
		if (n == building) continue;
		addBuilding (n, NULL);
	}

	//generate list, with the new subbases
	std::vector<sSubBase*> newSubBases;
	for (size_t i = 0; i != sb->buildings.size(); ++i)
	{
		cBuilding* n = sb->buildings[i];
		if (n == building) continue;
		newSubBases.push_back (n->SubBase);
	}
	RemoveDuplicates (newSubBases);

	// try to restore ressource allocation
	for (size_t i = 0; i != newSubBases.size(); ++i)
	{
		sSubBase& subBase = *newSubBases[i];

		subBase.setMetalProd (metal);
		subBase.setGoldProd (gold);
		subBase.setOilProd (oil);

		metal -= subBase.getMetalProd();
		gold  -= subBase.getGoldProd();
		oil   -= subBase.getOilProd();

		subBase.setMetalProd (subBase.getMaxAllowedMetalProd());
		subBase.setGoldProd (subBase.getMaxAllowedGoldProd());
		subBase.setOilProd (subBase.getMaxAllowedOilProd());
	}

	if (building->isUnitWorking () && building->data.canResearch)
		building->owner->stopAResearch (building->researchArea);

	if (server)
	{
		// send subbase values to client
		for (size_t i = 0; i != newSubBases.size(); ++i)
		{
			sendSubbaseValues (*server, *newSubBases[i], *building->owner);
		}
	}

	delete sb;
}

void cBase::handleTurnend (cServer& server)
{
	for (size_t i = 0; i != SubBases.size(); ++i)
	{
		SubBases[i]->makeTurnend (server);
	}
}

// recalculates all subbase values (after a load)
void cBase::refreshSubbases()
{
	for (size_t i = 0; i != SubBases.size(); ++i)
	{
		SubBases[i]->refresh();
	}
}
