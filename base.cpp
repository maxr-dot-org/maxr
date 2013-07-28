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
#include "player.h"
#include "server.h"
#include "serverevents.h"

using namespace std;

sSubBase::sSubBase (cPlayer* owner_) :
	buildings(),
	owner (owner_),
	MaxMetal(),
	Metal(),
	MaxOil(),
	Oil(),
	MaxGold(),
	Gold(),
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
	GoldProd()
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

		if (building.data.canMineMaxRes <= 0 || !building.IsWorking) continue;

		switch (ressourceType)
		{
			case RES_METAL:
				maxProd += building.MaxMetalProd;
				break;
			case RES_OIL:
				maxProd += building.MaxOilProd;
				break;
			case RES_GOLD:
				maxProd += building.MaxGoldProd;
				break;
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

		if (building.data.canMineMaxRes <= 0 || !building.IsWorking) continue;

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

		if (building.data.canMineMaxRes <= 0 || !building.IsWorking) continue;

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

bool sSubBase::increaseEnergyProd (cServer& server, int value)
{
	// TODO: the energy production and fuel consumption
	// of generators and stations are hardcoded in this function
	std::vector<cBuilding*> onlineStations;
	std::vector<cBuilding*> onlineGenerators;
	std::vector<cBuilding*> offlineStations;
	std::vector<cBuilding*> offlineGenerators;
	int availableStations = 0;
	int availableGenerators = 0;

	// generate lists with energy producers
	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& b = *buildings[i];

		if (!b.data.produceEnergy) continue;

		if (b.data.produceEnergy == 1)
		{
			availableGenerators++;

			if (b.IsWorking)
				onlineGenerators.push_back (&b);
			else
				offlineGenerators.push_back (&b);
		}
		else
		{
			availableStations++;

			if (b.IsWorking)
				onlineStations.push_back (&b);
			else
				offlineStations.push_back (&b);
		}
	}

	// calc the optimum amount of energy stations and generators
	const int energy = EnergyProd + value;

	int stations   = min ( (energy + 3) / 6, availableStations);
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
	if (neededFuel > Oil + getMaxOilProd())
	{
		// not possible to produce enough fuel
		sendChatMessageToClient (server, "Text~Comp~Fuel_Insufficient", SERVER_ERROR_MESSAGE, owner->getNr());
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

void sSubBase::addRessouce (cServer& server, sUnitData::eStorageResType storeResType, int value)
{
	int* storedRessources = NULL;
	switch (storeResType)
	{
		case sUnitData::STORE_RES_METAL:
			storedRessources = &Metal;
			break;
		case sUnitData::STORE_RES_OIL:
			storedRessources = &Oil;
			break;
		case sUnitData::STORE_RES_GOLD:
			storedRessources = &Gold;
			break;
		case sUnitData::STORE_RES_NONE:
			assert (0);
			break;
	}

	value = std::max (value, -*storedRessources);
	if (!value) return;

	*storedRessources += value;

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
		if (iStartValue != value) sendUnitData (server, b, owner->getNr());
		if (value == 0) break;
	}
	sendSubbaseValues (server, *this, owner->getNr());
}

void sSubBase::refresh()
{
	// copy buildings list
	const std::vector<cBuilding*> buildingsCopy = buildings;

	// reset subbase
	buildings.clear();
	MaxMetal = 0;
	Metal = 0;
	MaxOil = 0;
	Oil = 0;
	MaxGold = 0;
	Gold = 0;
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

	// readd all buildings
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
		if (!building.data.needsHumans || !building.IsWorking) continue;

		building.ServerStopWork (server, false);

		if (HumanNeed <= HumanProd) break;
	}
	return true;
}

bool sSubBase::checkGoldConsumer (cServer& server)
{
	if (GoldNeed <= GoldProd + Gold) return false;

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];
		if (!building.data.convertsGold || !building.IsWorking) continue;

		building.ServerStopWork (server, false);

		if (GoldNeed <= GoldProd + Gold) break;
	}
	return true;
}

bool sSubBase::checkMetalConsumer (cServer& server)
{
	if (MetalNeed <= MetalProd + Metal) return false;

	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& building = *buildings[i];
		if (!building.data.needsMetal || !building.IsWorking) continue;

		building.ServerStopWork (server, false);

		if (MetalNeed <= MetalProd + Metal) break;
	}
	return true;
}

bool sSubBase::checkOil (cServer& server)
{
	// TODO: the energy production and fuel consumption of generators and
	// stations are hardcoded in this function
	std::vector<cBuilding*> onlineStations;
	std::vector<cBuilding*> onlineGenerators;
	std::vector<cBuilding*> offlineStations;
	std::vector<cBuilding*> offlineGenerators;
	int availableStations = 0;
	int availableGenerators = 0;

	// generate lists with energy producers
	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& b = *buildings[i];

		if (!b.data.produceEnergy) continue;

		if (b.data.produceEnergy == 1)
		{
			availableGenerators++;

			if (b.IsWorking)
				onlineGenerators.push_back (&b);
			else
				offlineGenerators.push_back (&b);
		}
		else
		{
			availableStations++;

			if (b.IsWorking)
				onlineStations.push_back (&b);
			else
				offlineStations.push_back (&b);
		}
	}

	// calc the optimum amount of energy stations and generators
	int stations   = min ( (EnergyNeed + 3) / 6, availableStations);
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
	const int availableOil = getMaxOilProd() + Oil;
	bool oilMissing = false;
	if (neededOil > availableOil)
	{
		// reduce energy production to maximum possible value
		stations = min ( (availableOil) / 6, availableStations);
		generators = min ( (availableOil - (stations * 6)) / 2, availableGenerators);

		oilMissing = true;
	}

	// increase oil production, if necessary
	neededOil = stations * 6 + generators * 2;
	if (neededOil > OilProd + Oil)
	{
		// temporary decrease gold and metal production
		const int missingOil = neededOil - OilProd - Oil;
		const int gold = GoldProd;
		const int metal = MetalProd;
		setMetalProd (0);
		setGoldProd (0);

		changeOilProd (missingOil);

		setGoldProd (gold);
		setMetalProd (metal);

		sendChatMessageToClient (server, "Text~Comp~Adjustments_Fuel_Increased", SERVER_INFO_MESSAGE, owner->getNr(), iToStr (missingOil));
		if (getMetalProd() < metal)
			sendChatMessageToClient (server, "Text~Comp~Adjustments_Metal_Decreased", SERVER_INFO_MESSAGE, owner->getNr(), iToStr (metal - MetalProd));
		if (getGoldProd() < gold)
			sendChatMessageToClient (server, "Text~Comp~Adjustments_Gold_Decreased", SERVER_INFO_MESSAGE, owner->getNr(), iToStr (gold - GoldProd));
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
	if (getGoldProd() < getMaxAllowedGoldProd() ||
		getMetalProd() < getMaxAllowedMetalProd() ||
		getOilProd() < getMaxAllowedOilProd())
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
		if (!building.data.needsEnergy || !building.IsWorking) continue;

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
		if (!building.data.needsEnergy || !building.IsWorking) continue;

		// do not shut down oil producers in the second run
		if (building.MaxOilProd > 0) continue;

		building.ServerStopWork (server, false);

		if (EnergyNeed <= EnergyProd) return true;
	}

	// if energy is still missing, shut down also oil producers
	for (size_t i = 0; i < buildings.size(); i++)
	{
		cBuilding& building = *buildings[i];
		if (!building.data.needsEnergy || !building.IsWorking) continue;

		building.ServerStopWork (server, false);

		if (EnergyNeed <= EnergyProd) return true;
	}
	return true;
}

void sSubBase::prepareTurnend (cServer& server)
{
	if (checkMetalConsumer (server))
		sendChatMessageToClient (server, "Text~Comp~Metal_Low", SERVER_INFO_MESSAGE, owner->getNr());

	if (checkHumanConsumer (server))
		sendChatMessageToClient (server, "Text~Comp~Team_Low", SERVER_INFO_MESSAGE, owner->getNr());

	if (checkGoldConsumer (server))
		sendChatMessageToClient (server, "Text~Comp~Gold_Low", SERVER_INFO_MESSAGE, owner->getNr());

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
	if (oilMissing) sendChatMessageToClient (server, "Text~Comp~Fuel_Low", SERVER_INFO_MESSAGE, owner->getNr());
	if (energyMissing) sendChatMessageToClient (server, "Text~Comp~Energy_Low", SERVER_INFO_MESSAGE, owner->getNr());

	// recheck metal and gold,
	// because metal and gold producers could have been shut down,
	// due to a lack of energy
	if (checkMetalConsumer (server))
		sendChatMessageToClient (server, "Text~Comp~Metal_Low", SERVER_INFO_MESSAGE, owner->getNr());

	if (checkGoldConsumer (server))
		sendChatMessageToClient (server, "Text~Comp~Gold_Low", SERVER_INFO_MESSAGE, owner->getNr());
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
		sendCredits (server, owner->Credits, owner->getNr());
	}

	// make repairs/build/reload
	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding& Building = *buildings[i];

		// repair (do not repair buildings that have been attacked in this turn):
		if (Building.data.hitpointsCur < Building.data.hitpointsMax && Metal > 0 && !Building.hasBeenAttacked)
		{
			// calc new hitpoints
			Building.data.hitpointsCur += Round ( ( (float) Building.data.hitpointsMax / Building.data.buildCosts) * 4);
			Building.data.hitpointsCur = std::min (Building.data.hitpointsMax, Building.data.hitpointsCur);
			addMetal (server, -1);
			sendUnitData (server, Building, owner->getNr());
			for (size_t j = 0; j != Building.seenByPlayerList.size(); ++j)
			{
				sendUnitData (server, Building, Building.seenByPlayerList[j]->getNr());
			}
		}
		Building.hasBeenAttacked = false;

		// reload:
		if (Building.data.canAttack && Building.data.ammoCur == 0 && Metal > 0)
		{
			Building.data.ammoCur = Building.data.ammoMax;
			addMetal (server, -1);
			// ammo is not visible to enemies. So only send to the owner
			sendUnitData (server, Building, owner->getNr());
		}

		// build:
		if (Building.IsWorking && !Building.data.canBuild.empty() && !Building.BuildList.empty())
		{
			sBuildList& BuildListItem = Building.BuildList[0];
			if (BuildListItem.metall_remaining > 0)
			{
				// in this block the metal consumption of the factory
				// in the next round can change
				// so we first subtract the old value from MetalNeed and
				// then add the new one, to hold the base up to date
				MetalNeed -= min (Building.MetalPerRound, BuildListItem.metall_remaining);

				BuildListItem.metall_remaining -= min (Building.MetalPerRound, BuildListItem.metall_remaining);
				BuildListItem.metall_remaining = std::max (BuildListItem.metall_remaining, 0);

				MetalNeed += min (Building.MetalPerRound, BuildListItem.metall_remaining);
				sendBuildList (server, Building);
				sendSubbaseValues (server, *this, owner->getNr());
			}
			if (BuildListItem.metall_remaining <= 0)
			{
				server.addReport (BuildListItem.type, owner->getNr());
				Building.ServerStopWork (server, false);
			}
		}
	}

	// check maximum storage limits
	Metal = std::min (this->MaxMetal, this->Metal);
	Oil = std::min (this->MaxOil, this->Oil);
	Gold = std::min (this->MaxGold, this->Gold);

	// should not happen, but to be sure:
	Metal = std::max (this->Metal, 0);
	Oil = std::max (this->Oil, 0);
	Gold = std::max (this->Gold, 0);

	sendSubbaseValues (server, *this, owner->getNr());
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
	assert (buildings.size());

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
			Metal += b->data.storageResCur;
			break;
		case sUnitData::STORE_RES_OIL:
			MaxOil += b->data.storageResMax;
			Oil += b->data.storageResCur;
			break;
		case sUnitData::STORE_RES_GOLD:
			MaxGold += b->data.storageResMax;
			Gold += b->data.storageResCur;
			break;
		case sUnitData::STORE_RES_NONE:
			break;
	}
	// calculate energy
	if (b->data.produceEnergy)
	{
		MaxEnergyProd += b->data.produceEnergy;
		MaxOilNeed += b->data.needsOil;
		if (b->IsWorking)
		{
			EnergyProd += b->data.produceEnergy;
			OilNeed += b->data.needsOil;
		}
	}
	else if (b->data.needsEnergy)
	{
		MaxEnergyNeed += b->data.needsEnergy;
		if (b->IsWorking)
		{
			EnergyNeed += b->data.needsEnergy;
		}
	}
	// calculate ressource consumption
	if (b->data.needsMetal)
	{
		MaxMetalNeed += b->data.needsMetal * 12;
		if (b->IsWorking)
		{
			MetalNeed += min (b->MetalPerRound, b->BuildList[0].metall_remaining);
		}
	}
	// calculate gold
	if (b->data.convertsGold)
	{
		MaxGoldNeed += b->data.convertsGold;
		if (b->IsWorking)
		{
			GoldNeed += b->data.convertsGold;
		}
	}
	// calculate ressource production
	if (b->data.canMineMaxRes > 0 && b->IsWorking)
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
		if (b->IsWorking)
		{
			HumanNeed += b->data.needsHumans;
		}
	}

	// temporary debug check
	if (getGoldProd() < getMaxAllowedGoldProd() ||
		getMetalProd() < getMaxAllowedMetalProd() ||
		getOilProd() < getMaxAllowedOilProd())
	{
		Log.write (" Server: Mine distribution values are not a maximum", cLog::eLOG_TYPE_NET_WARNING);
	}
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

sSubBase* cBase::checkNeighbour (int iOff, const cBuilding& building)
{
	if (map->isValidOffset (iOff) == false) return NULL;
	cBuilding* b = map->fields[iOff].getBuilding();

	if (b && b->owner == building.owner && b->SubBase)
	{
		b->CheckNeighbours (map);
		return b->SubBase;
	}
	else return NULL;
}

void cBase::addBuilding (cBuilding* building, cServer* server)
{
	if (!building->data.connectsToBase) return;
	int pos = map->getOffset (building->PosX, building->PosY);
	std::vector<sSubBase*> NeighbourList;

	// check for neighbours
	if (!building->data.isBig)
	{
		// small building
		if (sSubBase* const SubBase = checkNeighbour (pos - map->getSize(), *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* const SubBase = checkNeighbour (pos + 1             , *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* const SubBase = checkNeighbour (pos + map->getSize(), *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* const SubBase = checkNeighbour (pos - 1             , *building)) NeighbourList.push_back (SubBase);
	}
	else
	{
		// big building
		if (sSubBase* SubBase = checkNeighbour (pos - map->getSize(),         *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (pos - map->getSize() + 1,     *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (pos + 2,                      *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (pos + 2 + map->getSize(),     *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (pos + map->getSize() * 2,     *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (pos + map->getSize() * 2 + 1, *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (pos - 1,                      *building)) NeighbourList.push_back (SubBase);
		if (sSubBase* SubBase = checkNeighbour (pos - 1 + map->getSize(),     *building)) NeighbourList.push_back (SubBase);
	}
	building->CheckNeighbours (map);

	RemoveDuplicates (NeighbourList);

	if (NeighbourList.empty())
	{
		// no neighbours found, just generate new subbase and add the building
		sSubBase* NewSubBase = new sSubBase (building->owner);
		building->SubBase = NewSubBase;
		NewSubBase->addBuilding (building);
		SubBases.push_back (NewSubBase);

		if (server) sendSubbaseValues (*server, *NewSubBase, NewSubBase->owner->getNr());

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
	if (server) sendSubbaseValues (*server, *firstNeighbour, building->owner->getNr());
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

	if (building->IsWorking && building->data.canResearch)
		building->owner->stopAResearch (building->researchArea);

	if (server)
	{
		// send subbase values to client
		for (size_t i = 0; i != newSubBases.size(); ++i)
		{
			sendSubbaseValues (*server, *newSubBases[i], building->owner->getNr());
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
