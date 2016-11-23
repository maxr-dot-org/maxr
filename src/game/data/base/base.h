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

#include <vector>
#include "game/data/units/unitdata.h"

class cBuilding;
class cMap;
class cPlayer;
class cPosition;
class cBase;

class cSubBase
{
public:
	explicit cSubBase (cBase& base);
	cSubBase (const cSubBase& other);
	~cSubBase();

	/**
	* integrates all building of the given subbase in the own one
	* @author eiko
	*/
	void merge (cSubBase* sb);

	void addBuilding (cBuilding* b);

	bool startBuilding(cBuilding* b);
	bool stopBuilding(cBuilding* b, bool forced = false);

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
	bool checkTurnEnd ();

	/**
	 * Produces resources, builds units and repairs/reloads units at turn start.
	 */
	void makeTurnStart ();

	//------------------------------------
	//resource management:

	/** returns the maximum production of a resource */
	int getMaxMetalProd() const;
	int getMaxGoldProd() const;
	int getMaxOilProd() const;

	/** returns the maximum allowed production
	 * (without decreasing one of the other ones) of a resource */
	int getMaxAllowedMetalProd() const;
	int getMaxAllowedGoldProd() const;
	int getMaxAllowedOilProd() const;

	/** returns the current production of a resource */
	int getMetalProd() const;
	int getGoldProd() const;
	int getOilProd() const;

	/** sets the production of a resource.
	 * If value is bigger then maxAllowed,
	 * it will be reduced to the maximum allowed value */
	void setMetalProd (int value);
	void setGoldProd (int value);
	void setOilProd (int value);

	/** changes the production of a resource by value. */
	void changeMetalProd(int value);
	void changeGoldProd(int value);
	void changeOilProd(int value);

	int getMaxMetalStored() const;
	int getMaxGoldStored() const;
	int getMaxOilStored() const;

	int getMetalStored() const;
	int getOilStored() const;
	int getGoldStored() const;

	int getMaxEnergyProd() const;
	int getEnergyProd() const;
	int getMaxEnergyNeed() const;
	int getEnergyNeed() const;

	int getMetalNeed() const;
	int getOilNeed() const;
	int getGoldNeed() const;
	
	int getMaxMetalNeed() const;
	int getMaxOilNeed() const;
	int getMaxGoldNeed() const;

	int getHumanProd() const;
	int getHumanNeed() const;
	int getMaxHumanNeed() const;

	const std::vector<cBuilding*>& getBuildings() const;

	uint32_t getChecksum(uint32_t crc) const;

	mutable cSignal<void ()> destroyed;

	mutable cSignal<void ()> metalChanged;
	mutable cSignal<void ()> oilChanged;
	mutable cSignal<void ()> goldChanged;
private:

	/**
	* inreases the energy production of the subbase by
	* starting offline generators/stations
	* @author eiko
	*/
	bool increaseEnergyProd(int value);

	//-----------------------------------
	//turn end management:

	/**
	 * checks if consumers have to be switched off, due to a lack of resources
	 * @return returns true, if consumers have been shut down
	 * @author eiko
	 */
	bool checkGoldConsumer ();
	bool checkHumanConsumer ();
	bool checkMetalConsumer ();
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
	bool checkOil ();
	/**
	 * switches off energy consumers, if necessary
	 * @return returns true, if a energy consumers have been shut down
	 * @author eiko
	 */
	bool checkEnergy ();

	void makeTurnStartRepairs (cBuilding& building);
	void makeTurnStartReload (cBuilding& building);
	void makeTurnStartBuild (cBuilding& building);


	/**
	* calcs the maximum allowed production of a resource,
	* without decreasing the production of the other two
	* @author eiko
	*/
	int calcMaxAllowedProd (int ressourceType) const;
	/**
	* calcs the maximum possible production of a resource
	* @author eiko
	*/
	int calcMaxProd (int ressourceType) const;
	/**
	* adds/subtracts resources of the type storeResType to/from the subbase
	* @author eiko
	*/
	void addRessouce (eResourceType storeResType, int value);

	int getResource (eResourceType storeResType) const;
	void setResource (eResourceType storeResType, int value);

	void setMetal(int value);
	void setOil(int value);
	void setGold(int value);

private:
	std::vector<cBuilding*> buildings;

	int maxMetalStored;
	int maxOilStored;
	int maxGoldStored;

	int maxEnergyProd;
	int energyProd;
	int maxEnergyNeed;
	int energyNeed;
	int metalNeed;
	int oilNeed;
	int goldNeed;
	int maxMetalNeed;
	int maxOilNeed;
	int maxGoldNeed;

	int humanProd;
	int humanNeed;
	int maxHumanNeed;

	int metalProd;
	int oilProd;
	int goldProd;

	cBase& base;

	int metalStored;
	int oilStored;
	int goldStored;
};

class cBase
{
public:
	cBase(cPlayer& owner);
	~cBase();

	/**
	* adds a building to the base and updates the subbase structures
	* @param building the building, that is added to the base
	* @author eiko
	*/
	void addBuilding (cBuilding* building, const cMap& map);
	/**
	* deletes a building from the base and updates the subbase structures
	* @param building the building, that is deleted to the base
	* @author eiko
	*/
	void deleteBuilding (cBuilding* building, const cMap& map);

	bool checkTurnEnd ();

	/**
	 * Handles the turn start for all sub bases.
	 *
	 * This produces resources, builds units and repairs/reloads units.
	 */
	void makeTurnStart ();

	/**
	* recalculates the values of all subbases
	*@author eiko
	*/
	void reset();

	cSubBase* checkNeighbour (const cPosition& position, const cBuilding& Building, const cMap& map);

	uint32_t getChecksum(uint32_t crc) const;

	// report sources for the player:
	mutable cSignal<void(int resourceType, int amount, bool increase)> forcedRessouceProductionChance;
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

public:
	std::vector<cSubBase*> SubBases;
	cPlayer& owner;
};

#endif // game_data_base_baseH
