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
#include "clist.h"

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

class cVehicleIterator
{
private:
	cList<cVehicle*>* vehicleList;
	int index;
public:
	cVehicleIterator(cList<cVehicle*>* list);
	/** returns the number of vehicles in the List, the Iterator points to. */
	unsigned int size();
	cVehicle& operator[](unsigned const int i) const;
	cVehicle* operator->() const;
	cVehicle& operator*() const;
	/** go to next vehicle on this field */
	cVehicleIterator operator++(int);
	/** go to previous vehicle on this field */
	cVehicleIterator operator--(int);
	bool operator==(cVehicle* v) const;
	operator cVehicle*() const;
	bool end;
	bool rend;
};

class cBuildingIterator
{
private:
	cList<cBuilding*>* buildingList;
	int index;
public:
	cBuildingIterator(cList<cBuilding*>* list);
	/** returns the number of buildings in the List, the Iterator points to. */
	unsigned int size();
	cVehicle& operator[](unsigned const int i) const;
	cBuilding* operator->() const;
	cBuilding& operator*() const;
	/** go to next building on this field */
	cBuildingIterator operator++(int);
	/** go to previous building on this field */
	cBuildingIterator operator--(int);
	bool operator==(cBuilding* b) const;
	operator cBuilding*() const;
	bool end;
	bool rend;
};

/** contains all information of a map field */
class cMapField
{
private:
	friend class cMap;
	/**the list with all buildings on this field
	* the top building is always stored at fist position */
	cList<cBuilding*> buildings;
	/** the list with all planes on this field
	* the top plane is always stored at fist position */
	cList<cVehicle*> vehicles;
	/**the list with all vehicles on this field
	* the top vehicle is always stored at fist position */
	cList<cVehicle*> planes;
	
	
public:
	/** returns a Iterator for the vehicles on this field */
	cVehicleIterator getVehicles();
	/** returns a Iterator for the planes on this field */
	cVehicleIterator getPlanes();
	/** returns a Iterator for the buildings on this field */
	cBuildingIterator getBuildings();

	/** returns a pointer to the top building or NULL if the first building is a base type */
	cBuilding* getTopBuilding();
	/** returns a pointer to the first base building or NULL if there is no base building */
	cBuilding* getBaseBuilding();

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

	sTerrain *terrain; // Terrain graphics

	bool IsWater(int off,bool not_coast=false,bool is_ship=false);
	void NewMap(int size, int iTerrainGrphCount );
	void DeleteMap(void);
	bool SaveMap(string filename,SDL_Surface *preview);
	bool LoadMap(string filename);
	void PlaceRessources(int Metal,int Oil,int Gold,int Dichte);
	/**
	* Access to a map field
	* @param the offset of the map field
	* @return an instance of cMapField, which has several methods to access the objects on the field
	*/
	cMapField& operator[]( unsigned int offset ) const;

	void addBuilding( cBuilding* building, unsigned int x, unsigned int y );
	void addBuilding( cBuilding* building, unsigned int offset );
	void addVehicle( cVehicle* vehicle, unsigned int x, unsigned int y );
	void addVehicle( cVehicle* vehicle, unsigned int offset );
	
	/**
	* moves a vehicle to the given position
	* resets the vehicle to a single field, when it was centered on four fields
	*/
	void moveVehicle( cVehicle* vehicle, unsigned int x, unsigned int y );
	void moveVehicle( cVehicle* vehicle, unsigned int offset );

	/**
	* places a vehicle on the 4 fields to the right and below the given position
	*/
	void moveVehicleBig( cVehicle* vehicle, unsigned int x, unsigned int y);
	void moveVehicleBig( cVehicle* vehicle, unsigned int offset );

	void deleteBuilding( cBuilding* building );
	void deleteVehicle( cVehicle* vehicle );

	int getMapLevel( cBuilding* building ) const;
	int getMapLevel( cVehicle* vehicle ) const;

	/** 
	* checks, whether the given field is an allowed place for the vehicle
	* if a player is passed, the function uses the players point of view, so it does not check for units that are not in sight
	* note, that the function can only check for map border overflows, if you pass xy coordinates instead of an offset
	*/
	bool possiblePlace( const cVehicle* vehicle, int x, int y, const cPlayer* player = NULL ) const;
	bool possiblePlace( const cVehicle* vehicle, int offset, const cPlayer* player = NULL ) const;
	bool possiblePlaceVehicle( const sUnitData& vehicleData, int x, int y, const cPlayer* player = NULL ) const;
	bool possiblePlaceVehicle( const sUnitData& vehicleData, int offset, const cPlayer* player = NULL ) const;

	/**
	* checks, whether the given field is an allowed place for the building
	* if a vehicle is passed, it will be ignored in the check, so a constructing vehicle does not block its own position
	* note, that the function can only check for map border overflows, if you pass xy coordinates instead of an offset
	*/
	bool possiblePlaceBuilding( const sUnitData& buildingData, int x, int y, cVehicle* vehicle = NULL ) const;
	bool possiblePlaceBuilding( const sUnitData& buildingData, int offset, cVehicle* vehicle = NULL ) const;


private:
	/**
	* the infomation about the fields
	*/
	cMapField* fields;

	SDL_Surface *LoadTerrGraph ( SDL_RWops *fpMapFile, int iGraphicsPos, sColor Palette[256], int iNum, bool bWater, bool &overlay );
	void CopySrfToTerData ( SDL_Surface *surface, int iNum, int iSizeX  );
};

#endif
