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
	sSubBase( int iNextID );
	sSubBase( const sSubBase& sb );
	~sSubBase();

	cList<cBuilding*> buildings;

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
	int calcMaxAllowedProd( int ressourceType );
	int calcMaxProd( int ressourceType );

};


// Die Base Klasse ///////////////////////////////////////////////////////////
class cBase{
public:
  cBase(cPlayer *Owner);
  ~cBase(void);

  cPlayer *owner;
  int iNextSubBaseID;
  cList<sSubBase*> SubBases;
  cMap *map;

  void AddBuilding(cBuilding *Building);
  void DeleteBuilding(cBuilding *Building);
  void AddBuildingToSubBase(cBuilding *b,sSubBase *sb);
  void AddMetal(sSubBase *sb,int value);
  void AddOil(sSubBase *sb,int value);
  void AddGold(sSubBase *sb,int value);
  void handleTurnend();
  bool OptimizeEnergy(sSubBase *sb);
  void RefreshSubbases(void);
  sSubBase *checkNeighbour ( int iOff, cBuilding *Building );
};

#endif
