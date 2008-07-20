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
#ifndef mapH
#define mapH
#include "defines.h"
#include "main.h"
#include "vehicles.h"
#include "buildings.h"

// GameObjects Struktur //////////////////////////////////////////////////////
struct sGameObjects{
  cVehicle *vehicle;		// Vehicles on the ground
  cVehicle *plane;			// Vehicles in the air
  cBuilding *base;			// Buildings on the ground whitch are under vehciles and top-buildings
  cBuilding *subbase;		// Normaley base-buildings but in this special case they are under an other base-building (for examble plattform under street)
  cBuilding *top;			// Normal buildings on the ground
  bool reserviert;
  bool air_reserviert;
};

// Resources Struktur ////////////////////////////////////////////////////////
struct sResources{
  unsigned char value;
  unsigned char typ;
};
// Die Resorces-Typen:
#define RES_NONE  0
#define RES_METAL 1
#define RES_OIL   2
#define RES_GOLD  3

struct sColor
{
	unsigned char cBlue, cGreen, cRed;
};

struct sVehicleList
{
	sVehicleList* next;
	sVehicleList* prev;
	cVehicle* vehicle;
};

class cVehicleIterator
{
private:
	sVehicleList* vehicleListItem;
public:
	cVehicleIterator(sVehicleList* vli);
	/** returns the number of vehicles in the List, the Iterator points to. */
	unsigned int size();
//	cVehicle& operator[](unsigned const int i) const;
	cVehicle* operator->() const;
	cVehicle& operator*() const;
	/** go to next vehicle on this field */
	cVehicleIterator operator++(int);
	/** go to previous vehicle on this field */
	cVehicleIterator operator--(int);
	bool operator==(cVehicle* v) const;
	operator cVehicle*() const;
	operator bool() const;
};

struct sBuildingList
{
	sBuildingList* next;
	sBuildingList* prev;
	cBuilding* building;
};

class cBuildingIterator
{
private:
	sBuildingList* buildingListItem;
public:
	cBuildingIterator( sBuildingList* bli );
	/** returns the number of buildings in the List, the Iterator points to. */
	unsigned int size();
//	cBuilding& operator[](unsigned const int i) const;
	cBuilding* operator->() const;
	cBuilding& operator*() const;
	/** go to next building on this field */
	cBuildingIterator operator++(int);
	/** go to next building on this field */
	cBuildingIterator operator--(int);
	bool operator==(cBuilding* b) const;
	operator cBuilding*() const;
	operator bool() const;
};

/** contains all information of a map field */
class cMapField
{
private:
	friend class cMap;
	/**
	* list of all vehicles on this field
	* the top vehicle is always stored at fist position */
	sVehicleList* vehicleList;
	/**
	* list of all planes on this field
	* the top plane is always stored at fist position */
	sVehicleList* planeList;
	/**
	* list of all buildings on this field
	* the top building is always stored at fist position */
	sBuildingList* buildingList;
public:
	/* returns a Iterator for the vehicles on this field */
	cVehicleIterator getVehicles() const;
	/* returns a Iterator for the planes on this field */
	cVehicleIterator getPlanes() const;
	/* returns a Iterator for the buildings on this field */
	cBuildingIterator getBuildings() const;

	/* returns a pointer to the top building of NULL if the first building is a base type */
	cBuilding* getTopBuilding() const;
	/* returns a pointer to the first base building or NULL if there is no base building */
	cBuilding* getBaseBuilding() const;

	cMapField();
	~cMapField();
};

// Die Map-Klasse ////////////////////////////////////////////////////////////
class cMap{
public:
  cMap(void);
  ~cMap(void);

  int size;     // Größe der Karte.
  int *Kacheln; // Map mit den Kacheln.
  int DefaultWater; // Nummer des Wassers für Küsten
  sGameObjects *GO; // Feld mit den Gameobjects für einen schnelleren Zugriff.
  sResources *Resources; // Feld mit den Resourcen.
  cList<sTerrain*> TerrainInUse; // Liste mit Zeigern auf die terrains, die benutzt werden.
  string ErrorStr; // Der String mit der Fehlermeldung fürs Laden der Maps.
  string MapName;  // Name der aktuellen Map.
  /** a list of all rubble objects */
  cBuilding* rubbleList;

  sTerrain *terrain; // Terrain graphics

  bool IsWater(int off,bool not_coast=false,bool is_ship=false);
  void NewMap(int size, int iTerrainGrphCount );
  void DeleteMap(void);
  bool SaveMap(string filename,SDL_Surface *preview);
  bool LoadMap(string filename);
  void UseAllTerrain(void);
  void PlaceRessources(int Metal,int Oil,int Gold,int Dichte);
  /**
  * places rubble on the map
  *@param offset place where the rubble should be added
  *@param value the ressource value of the rubble
  *@param big true if the rubble is 4 fields big
  */
  void addRubble( int offset, int value, bool big );

  /**
  * removes rubble from the map
  *@param rubble pointer to the rubble
  */
  void deleteRubble( cBuilding* rubble );
  /**
  * Access to a map field
  * @param the offset of the map field
  * @return an instance of cMapField, which has several methods to access the objects on the field
  */
  cMapField& operator[]( unsigned int offset ) const;
private:
	/**
	* the infomation about the fields
	*/
	cMapField* fields;

	SDL_Surface *LoadTerrGraph ( SDL_RWops *fpMapFile, int iGraphicsPos, sColor Palette[256], int iNum, bool bWater, bool &overlay );
	void CopySrfToTerData ( SDL_Surface *surface, int iNum, int iSizeX  );
};

#endif
