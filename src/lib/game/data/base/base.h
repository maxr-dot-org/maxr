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

#ifndef game_data_base_baseH
#define game_data_base_baseH

#include "game/data/miningresource.h"
#include "utility/signal/signal.h"

#include <memory>
#include <vector>

class cBuilding;
class cMap;
class cPlayer;
class cPosition;
class cBase;

struct sNewTurnPlayerReport;

class cSubBase
{
public:
	explicit cSubBase (cBase&);
	cSubBase (const cSubBase& other);
	~cSubBase();

	/**
	* integrates all building of the given subbase in the own one
	* @author eiko
	*/
	void merge (cSubBase&);

	void addBuilding (cBuilding&);

	bool startBuilding (cBuilding&);
	bool stopBuilding (cBuilding&, bool forced = false);

	/**
	* transfers a resource to/from the subbase
	* @author eiko
	*/
	void addMetal (int value);
	void addOil (int value);
	void addGold (int value);

	/**
	 * Checks, if there are consumers, that have to be shut down,
	 * due to a lack of a resources
	 */
	bool checkTurnEnd();

	/**
	 * Produces resources, builds units and repairs/reloads units at turn start.
	 */
	void makeTurnStart (sNewTurnPlayerReport&);

	//------------------------------------
	//resource management:

	/** returns the maximum production */
	sMiningResource getMaxProd() const;

	/** returns the current production */
	const sMiningResource& getProd() const;

	void setProduction (const sMiningResource&);

	const sMiningResource& getResourcesNeeded() const { return needed; }
	const sMiningResource& getMaxResourcesNeeded() const { return maxNeeded; }

	const sMiningResource& getResourcesStored() const { return stored; }
	const sMiningResource& getMaxResourcesStored() const { return maxStored; }

	int getMaxEnergyProd() const;
	int getEnergyProd() const;
	int getMaxEnergyNeed() const;
	int getEnergyNeed() const;

	int getHumanProd() const;
	int getHumanNeed() const;
	int getMaxHumanNeed() const;

	const std::vector<cBuilding*>& getBuildings() const { return buildings; }

	uint32_t getChecksum (uint32_t crc) const;

	mutable cSignal<void()> metalChanged;
	mutable cSignal<void()> oilChanged;
	mutable cSignal<void()> goldChanged;

private:
	/**
	* increases the energy production of the subbase by
	* starting offline generators/stations
	* @author eiko
	*/
	bool increaseEnergyProd (int value);

	void increaseOilProd (int value);
	//-----------------------------------
	//turn end management:

	/**
	 * checks if consumers have to be switched off, due to a lack of resources
	 * @return returns true, if consumers have been shut down
	 * @author eiko
	 */
	bool checkGoldConsumer();
	bool checkHumanConsumer();
	bool checkMetalConsumer();
	/**
	 * - switches off unneeded fuel consumers(=energy producers)
	 * - sets the optimal amount of generators and stations
	 *   to minimize fuel consumption
	 * - increases oil production, if necessary
	 * - switches off oil consumers, if too few oil is available
	 * @return: returns true, if oil consumers have been shut down,
	 *          due to a lack of oil
	 * @author eiko
	 */
	bool checkOil();
	/**
	 * switches off energy consumers, if necessary
	 * @return returns true, if a energy consumers have been shut down
	 * @author eiko
	 */
	bool checkEnergy();

	void makeTurnStartRepairs (cBuilding&);
	void makeTurnStartReload (cBuilding&);
	void makeTurnStartBuild (cBuilding&, sNewTurnPlayerReport&);

	/**
	* adds/subtracts resources of the type storeResType to/from the subbase
	* @author eiko
	*/
	void addResource (eResourceType, int value);

	int getResource (eResourceType) const;
	void setResource (eResourceType, int value);

	void setMetal (int value);
	void setOil (int value);
	void setGold (int value);

private:
	std::vector<cBuilding*> buildings;

	sMiningResource stored;
	sMiningResource maxStored;
	sMiningResource needed;
	sMiningResource maxNeeded;
	sMiningResource prod;

	int maxEnergyProd = 0;
	int energyProd = 0;
	int maxEnergyNeed = 0;
	int energyNeed = 0;

	int humanProd = 0;
	int humanNeed = 0;
	int maxHumanNeed = 0;

	cBase& base;
};

class cBase
{
public:
	explicit cBase (cPlayer& owner);
	~cBase();

	/**
	* adds a building to the base and updates the subbase structures
	* @author eiko
	*/
	void addBuilding (cBuilding&, const cMap&);
	/**
	* deletes a building from the base and updates the subbase structures
	* @author eiko
	*/
	void deleteBuilding (cBuilding&, const cMap&);

	bool checkTurnEnd();

	/**
	 * Handles the turn start for all sub bases.
	 *
	 * This produces resources, builds units and repairs/reloads units.
	 */
	void makeTurnStart (sNewTurnPlayerReport&);

	/**
	* recalculates the values of all subbases
	*@author eiko
	*/
	void clear();

	cSubBase* checkNeighbour (const cPosition&, const cBuilding&, const cMap&);

	uint32_t getChecksum (uint32_t crc) const;

	cSignal<void (const std::vector<cBuilding*>&)> onSubbaseConfigurationChanged;

	// report sources for the player:
	mutable cSignal<void (eResourceType, int amount, bool increase)> forcedResourceProductionChance;
	mutable cSignal<void()> teamLow;
	mutable cSignal<void()> metalLow;
	mutable cSignal<void()> goldLow;
	mutable cSignal<void()> fuelLow;
	mutable cSignal<void()> energyLow;

	mutable cSignal<void()> fuelInsufficient;
	mutable cSignal<void()> teamInsufficient;
	mutable cSignal<void()> goldInsufficient;
	mutable cSignal<void()> metalInsufficient;
	mutable cSignal<void()> energyInsufficient;

	mutable cSignal<void()> energyToLow;
	mutable cSignal<void()> energyIsNeeded;

private:
	void addBuilding (cBuilding&, const cMap&, bool signalChange);

public:
	std::vector<std::unique_ptr<cSubBase>> SubBases;
	cPlayer& owner;
};

#endif
