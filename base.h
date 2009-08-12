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
#ifndef baseH
#define baseH
#include "defines.h"
#include <SDL.h>
#include "buildings.h"

class cPlayer;
class cBuilding;
class cMap;

// Die SubBase Struktur //////////////////////////////////////////////////////
struct sSubBase{
public:
	sSubBase( int iNextID, cPlayer* owner_ );
	sSubBase( const sSubBase& sb );
	~sSubBase();

	cList<cBuilding*> buildings;
	cPlayer* owner;

	int iID;
	int MaxMetal;
	int Metal;
	int MaxOil;
	int Oil;
	int MaxGold;
	int Gold;

	int MaxEnergyProd;
	int EnergyProd;
	int MaxEnergyNeed;
	int EnergyNeed;
	int MetalNeed;
	int OilNeed;
	int GoldNeed;
	int MaxMetalNeed;
	int MaxOilNeed;
	int MaxGoldNeed;

 
	int HumanProd;
	int HumanNeed;
	int MaxHumanNeed;

	void addBuilding( cBuilding *b );

	/*
	* adds/substracts a ressource to/from the subbase
	* @author eiko
	*/
	void addMetal( int i );
	void addOil( int i );
	void addGold( int i );


	/**
	* recalculates the values of all subbases
	* @author eiko
	*/
	void refresh();

	/**
	* inreases the energy production of the subbase by starting offline generators/stations
	* @author eiko	
	*/
	bool increaseEnergyProd( int i );

	//-----------------------------------
	//turn end manangement:
	
	/**
	* checks if consumers have to be switched off, due to a lack of ressources
	* @return returns true, if consumers have been shut down
	* @author eiko
	*/
	bool checkGoldConsumer();
	bool checkHumanConsumer();
	bool checkMetalConsumer();
	/**
	* - switch off unneeded fuel consumers(=energy producers)
	* - sets the optimal amount of generators and stations to minimize fuel consumption
	* - increases oil production, if nessesary
	* - switches off oil consumers, if to few oil is available
	* @return: returns true, if oil consumers have been shut down, due to a lack of oil
	* @author eiko
	*/
	bool checkOil();
	/**
	* switch off energy consumers, if nessesary
	* @return returns true, if a energy consumers have been shut down
	* @author eiko
	*/
	bool checkEnergy();
	/**
	* checks, if there a consumers, that havt to be shut down, due to a lack of a ressource
	* @author eiko
	*/
	void prepareTurnend();
	/**
	* produce ressources, rapair/reload buildings etc.
	* @author eiko
	*/
	void makeTurnend();

	//------------------------------------
	//ressource management:

	/** returns the maximum production of a ressource */
	int getMaxMetalProd();
	int getMaxGoldProd();
	int getMaxOilProd();

	/** returns the maximum allowed production (without dereasing one of the other ones) of a ressource */
	int getMaxAllowedMetalProd();
	int getMaxAllowedGoldProd();
	int getMaxAllowedOilProd();

	/** returns the current production of a ressource */
	int getMetalProd();
	int getGoldProd();
	int getOilProd();

	/** sets the production of a ressource. If i is bigger then maxAllowed, it will be reduced to the maximum allowed value */
	void setMetalProd( int i );
	void setGoldProd( int i );
	void setOilProd( int i );

	/** changes the production of a ressource by i. */
	void changeMetalProd( int i );
	void changeGoldProd( int i );
	void changeOilProd( int i );

//TODO: private:
	int MetalProd;
	int OilProd;
	int GoldProd;
private:
	/**
	* calcs the maximum allowed production of a ressource, without decreasing the production of the other two
	* @author eiko
	*/
	int calcMaxAllowedProd( int ressourceType );
	/**
	* calcs the maximum possible production of a ressource
	* @author eiko
	*/
	int calcMaxProd( int ressourceType );
	/**
	* adds/substracts ressourcec of the type storeResType  to/from the subbase
	* @author eiko
	*/
	void addRessouce( sUnitData::eStorageResType storeResType, int value );

};

class cBase
{
public:
	cBase(cPlayer *Owner);
	~cBase(void);

	int iNextSubBaseID;
	cList<sSubBase*> SubBases;
	cMap *map;

	void AddBuilding(cBuilding *Building);
	void DeleteBuilding(cBuilding *Building);
	void handleTurnend();
	/**
	* recalculates the values of all subbases
	*@author eiko
	*/
	void refreshSubbases();
	sSubBase *checkNeighbour ( int iOff, cBuilding *Building );
};

#endif
