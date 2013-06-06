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

#include <string>

#include "autosurface.h"
#include "defines.h"
#include <vector>
#include "t_2.h"

class cVehicle;
class cBuilding;
class cPlayer;
struct sUnitData;

// Resources Struktur ////////////////////////////////////////////////////////
struct sResources
{
	unsigned char value;
	unsigned char typ;
};
// Die Resorces-Typen:
const int RES_NONE  = 0;
const int RES_METAL = 1;
const int RES_OIL   = 2;
const int RES_GOLD  = 3;
const int RES_COUNT = 4;

template<typename T>
class cMapIterator
{
private:
	std::vector<T*>* list;
	int index;
public:
	explicit cMapIterator<T> (std::vector<T*>* list_);
	/** returns the number of vehicles in the List, the Iterator points to. */
	unsigned int size() const;
	//T& operator[](unsigned const int i) const;
	T* operator->() const;
	T& operator*() const;
	/** go to next vehicle on this field */
	cMapIterator operator++ (int);
	/** go to previous vehicle on this field */
	cMapIterator operator-- (int);
	operator T* () const;
	void setToEnd();
	void rewind();
	bool contains (const T& v) const;
	size_t getIndex() const;
	bool end;
	bool rend;
};

typedef cMapIterator<cVehicle> cVehicleIterator;
typedef cMapIterator<cBuilding> cBuildingIterator;

template <typename T>
cMapIterator<T>::cMapIterator (std::vector<T*>* list_)
{
	index = 0;
	list = list_;

	if (list->size() == 0)
	{
		end = true;
		rend = true;
	}
	else
	{
		end = false;
		rend = false;
	}
}

template <typename T>
unsigned int cMapIterator<T>::size() const
{
	return (unsigned int) list->size();
}

template <typename T>
T* cMapIterator<T>::operator->() const
{
	if (!end && !rend)
		return (*list) [index];
	else
		return NULL;
}

template <typename T>
T& cMapIterator<T>::operator*() const
{
	T* unit = NULL;
	if (!end && !rend) unit = (*list) [index];

	return *unit;
}

template <typename T>
cMapIterator<T> cMapIterator<T>::operator++ (int)
{
	cMapIterator<T> i = *this;
	if (end) return i;

	if (rend)
	{
		rend = false;
		index = 0;
	}
	else
	{
		index++;
	}
	if (index >= (int) list->size()) end = true;

	return i;
}

template <typename T>
cMapIterator<T> cMapIterator<T>::operator-- (int)
{
	cMapIterator<T> i = *this;
	if (rend) return i;

	if (end)
	{
		index = (int) list->size() - 1;
		end = false;
	}
	else
	{
		index--;
	}
	if (index < 0) rend = true;

	return i;
}

template <typename T>
cMapIterator<T>::operator T* () const
{
	if (end || rend) return NULL;
	return (*list) [index];
}

template <typename T>
void cMapIterator<T>::setToEnd()
{
	if (list->size() > 0)
	{
		index = list->size() - 1;
	}
	else
	{
		index = 0;
		end = true;
		rend = true;
	}
}

template <typename T>
void cMapIterator<T>::rewind()
{
	index = 0;
	if (list->size() == 0)
	{
		rend = true;
		end = true;
	}
	else
	{
		rend = false;
		end = false;
	}
}

template <typename T>
bool cMapIterator<T>::contains (const T& v) const
{
	return Contains (*list, &v);
}

template <typename T>
size_t cMapIterator<T>::getIndex() const
{
	return index;
}

/** contains all information of a map field */
class cMapField
{
private:
	friend class cMap;
	/**the list with all buildings on this field
	* the top building is always stored at fist position */
	std::vector<cBuilding*> buildings;
	/** the list with all planes on this field
	* the top plane is always stored at fist position */
	std::vector<cVehicle*> vehicles;
	/**the list with all vehicles on this field
	* the top vehicle is always stored at fist position */
	std::vector<cVehicle*> planes;

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
	/** returns a pointer to a rubble object, if there is one. */
	cBuilding* getRubble();
	/** returns a pointer to an expl. mine, if there is one */
	cBuilding* getMine();
};

struct sTerrain
{
	sTerrain();

	AutoSurface sf;      /** the scaled surface of the terrain */
	AutoSurface sf_org;  /** the original surface of the terrain */
	AutoSurface shw;     /** the scaled surface of the terrain in the fog */
	AutoSurface shw_org; /** the original surface of the terrain in the fog */
	bool water;          /** is this terrain water? */
	bool coast;          /** is this terrain a coast? */
	bool blocked;        /** is this terrain blocked? */
};

class cStaticMap
{
public:
	cStaticMap();
	~cStaticMap();

	void clear();
	bool loadMap (const std::string& filename);

	const std::string& getMapName() const { return MapName; }
	int getSize() const { return size; }
	int getOffset (int x, int y) const { return y * size + x; }

	bool isWater (int x, int y, bool not_coast) const;
	bool isBlocked(int offset) const;
	bool isCoast (int offset) const;
	bool isWater (int offset) const;

	const sTerrain& getTerrain (int offset) const;
	const sTerrain& getTerrain (int x, int y) const;

	SDL_Surface* createBigSurface (int sizex, int sizey) const;
	void generateNextAnimationFrame();
	void scaleSurfaces (int pixelSize);
private:
	static SDL_Surface* loadTerrGraph (SDL_RWops* fpMapFile, int iGraphicsPos, SDL_Color* Palette, int iNum);
	void copySrfToTerData (SDL_Surface* surface, int iNum);
private:
	std::string MapName;   // Name of the current map
	int size;
	unsigned int terrainCount;
	sTerrain* terrains;       // The different terrain type.
	std::vector<int> Kacheln; // Terrain numbers of the map fields
	SDL_Color palette[256];   // Palette with all Colors for the terrain graphics
	SDL_Color palette_shw[256];
};

// Die Map-Klasse ////////////////////////////////////////////////////////////
class cMap
{
public:
	explicit cMap (cStaticMap& staticMap_);
	~cMap();

	int getOffset (int x, int y) const { return staticMap->getOffset (x, y);}
	bool isWater (int x, int y, bool not_coast = false) const { return staticMap->isWater(x, y, not_coast); }

	void placeRessourcesAddPlayer (int x, int y, int frequency);
	void placeRessources (int Metal, int Oil, int Gold);
	/**
	* Access to a map field
	* @param the offset of the map field
	* @return an instance of cMapField, which has several methods to access the objects on the field
	*/
	cMapField& operator[] (unsigned int offset) const;

	void addBuilding (cBuilding* building, unsigned int x, unsigned int y);
	void addBuilding (cBuilding* building, unsigned int offset);
	void addVehicle (cVehicle* vehicle, unsigned int x, unsigned int y);
	void addVehicle (cVehicle* vehicle, unsigned int offset);

	/**
	* moves a vehicle to the given position
	* resets the vehicle to a single field, when it was centered on four fields
	* @param height defines the flight hight, when more then one planes on a field. 0 means top/highest.
	*/
	void moveVehicle (cVehicle* vehicle, unsigned int x, unsigned int y, int height = 0);

	/**
	* places a vehicle on the 4 fields to the right and below the given position
	*/
	void moveVehicleBig (cVehicle* vehicle, unsigned int x, unsigned int y);

	void deleteBuilding (const cBuilding* building);
	void deleteVehicle (const cVehicle* vehicle);

	int getMapLevel (const cBuilding& building) const;
	int getMapLevel (const cVehicle& vehicle) const;

	/**
	* checks, whether the given field is an allowed place for the vehicle
	* if checkPlayer is passed, the function uses the players point of view, so it does not check for units that are not in sight
	*/
	bool possiblePlace (const cVehicle& vehicle, int x, int y, bool checkPlayer = false) const;
	bool possiblePlaceVehicle (const sUnitData& vehicleData, int x, int y, const cPlayer* player, bool checkPlayer = false) const;

	/**
	* checks, whether the given field is an allowed place for the building
	* if a vehicle is passed, it will be ignored in the check, so a constructing vehicle does not block its own position
	* note, that the function can only check for map border overflows (with margin), if you pass xy coordinates instead of an offset
	*/
	bool possiblePlaceBuilding (const sUnitData& buildingData, int x, int y, const cVehicle* vehicle = NULL) const;
	bool possiblePlaceBuildingWithMargin (const sUnitData& buildingData, int x, int y, int margin, const cVehicle* vehicle = NULL) const;
	bool possiblePlaceBuilding (const sUnitData& buildingData, int offset, const cVehicle* vehicle = NULL) const;

	/**
	* removes all units from the map structure
	*/
	void reset();

public:
	cStaticMap* staticMap;
	int size;     // size of the map
	/**
	* the infomation about the fields
	*/
	cMapField* fields;
	sResources* Resources; // field with the ressource data
private:
	T_2<int>* resSpots;
	int* resSpotTypes;
	int resSpotCount;
	int resCurrentSpotCount;
};

#endif
