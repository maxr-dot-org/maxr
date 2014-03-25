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
#include <memory>

#include "autosurface.h"
#include "defines.h"
#include <vector>
#include "t_2.h"
#include "utility/position.h"
#include "utility/signal/signal.h"
#include "gui/menu/windows/windowgamesettings/gamesettings.h"

class cUnit;
class cVehicle;
class cBuilding;
class cPlayer;
class cPosition;
struct sUnitData;

// Resources Struktur ////////////////////////////////////////////////////////
struct sResources
{
public:
	sResources() : value (0), typ (0) {}
public:
	unsigned char value;
	unsigned char typ;
};
// Die Resorces-Typen:
const int RES_NONE  = 0;
const int RES_METAL = 1;
const int RES_OIL   = 2;
const int RES_GOLD  = 3;
const int RES_COUNT = 4;

/** contains all information of a map field */
class cMapField
{
private:
	friend class cMap;
	/**the list with all buildings on this field
	* the top building is always stored at first position */
	std::vector<cBuilding*> buildings;
	/** the list with all planes on this field
	* the top plane is always stored at first position */
	std::vector<cVehicle*> vehicles;
	/**the list with all vehicles on this field
	* the top vehicle is always stored at first position */
	std::vector<cVehicle*> planes;

public:

	/** returns the top vehicle on this field */
	cVehicle* getVehicle() const;

	/** returns a Iterator for the planes on this field */
	cVehicle* getPlane() const;
	/** returns the planes on this field */
	const std::vector<cVehicle*>& getPlanes () const;
	/** returns the buildings on this field */
	const std::vector<cBuilding*>& getBuildings ()const;

	/** returns a pointer for the buildings on this field */
	cBuilding* getBuilding() const;
	/** returns a pointer to the top building or NULL if the first building is a base type */
	cBuilding* getTopBuilding() const;
	/** returns a pointer to the first base building or NULL if there is no base building */
	cBuilding* getBaseBuilding() const;
	/** returns a pointer to a rubble object, if there is one. */
	cBuilding* getRubble() const;
	/** returns a pointer to an expl. mine, if there is one */
	cBuilding* getMine() const;
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
	static const int tilePixelHeight = 64;
	static const int tilePixelWidth = 64;

	cStaticMap();
	~cStaticMap();

	void clear();
	bool loadMap (const std::string& filename);
	bool isValid() const;

	const std::string& getName() const { return filename; }
	int getSize () const { return size; }
	cPosition getSizeNew () const { return cPosition (size, size); }
	int getOffset (int x, int y) const { return y * size + x; }
	int getOffset (const cPosition& pos) const { return getOffset (pos.x (), pos.y ()); }

	bool isValidPos (int x, int y) const;

	bool isWater (int x, int y) const;
	bool isBlocked (int offset) const;
	bool isCoast (int offset) const;
	bool isWater (int offset) const;

	const sTerrain& getTerrain (int offset) const;
	const sTerrain& getTerrain (int x, int y) const;
	const sTerrain& getTerrain (const cPosition& position) const;

	SDL_Surface* createBigSurface (int sizex, int sizey) const;
	void generateNextAnimationFrame();
	void scaleSurfaces (int pixelSize);
	static SDL_Surface* loadMapPreview (const std::string& mapPath, int* mapSize = NULL);
private:
	static SDL_Surface* loadTerrGraph (SDL_RWops* fpMapFile, int iGraphicsPos, const SDL_Color (&colors)[256], int iNum);
	void copySrfToTerData (SDL_Surface* surface, int iNum);
private:
	std::string filename;   // Name of the current map
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
	explicit cMap (std::shared_ptr<cStaticMap> staticMap_);
	~cMap();

	const std::string& getName() const { return staticMap->getName(); }
	int getSize() const { return staticMap->getSize(); }
	int getOffset (int x, int y) const { return staticMap->getOffset (x, y); }
	int getOffset (const cPosition& pos) const { return staticMap->getOffset (pos); }
	bool isValidPos (int x, int y) const { return staticMap->isValidPos (x, y); }
	bool isValidOffset (int offset) const;

	bool isBlocked (int offset) const { return staticMap->isBlocked (offset); }
	bool isBlocked (const cPosition& pos) const { return staticMap->isBlocked (getOffset(pos)); }
	bool isCoast (int offset) const { return staticMap->isCoast (offset); }
	bool isWater (int offset) const { return staticMap->isWater (offset); }
	bool isCoast (int x, int y) const { return staticMap->isCoast (getOffset (x, y)); }
	bool isWater (int x, int y) const { return staticMap->isWater (x, y); }
	bool isWaterOrCoast (int x, int y) const;

	const sResources& getResource (int offset) const { return Resources[offset]; }
	const sResources& getResource (int x, int y) const { return Resources[getOffset (x, y)]; }
	const sResources& getResource (const cPosition& position) const { return getResource (position.x (), position.y ()); }
	sResources& getResource (int offset) { return Resources[offset]; }
	void assignRessources (const cMap& rhs);

	/**
	* converts the resource data to a string in HEX format
	*@author alzi alias DoctorDeath
	*/
	std::string resourcesToString() const;
	/**
	* converts from HEX-string to the resources
	*@author alzi alias DoctorDeath
	*/
	void setResourcesFromString (const std::string& str);

	void placeRessourcesAddPlayer (int x, int y, eGameSettingsResourceDensity desity);
	void placeRessources (eGameSettingsResourceAmount metal, eGameSettingsResourceAmount oil, eGameSettingsResourceAmount gold);

	int getResourceDensityFactor (eGameSettingsResourceDensity desity) const;
	int getResourceAmountFactor (eGameSettingsResourceAmount amount) const;
	/**
	* Access to a map field
	* @param the offset of the map field
	* @return an instance of cMapField, which has several methods to access the objects on the field
	*/
	cMapField& operator[] (unsigned int offset) const;

	cMapField& getField (const cPosition& position);
	const cMapField& getField (const cPosition& position) const;

	void addBuilding (cBuilding& building, unsigned int x, unsigned int y);
	void addBuilding (cBuilding& building, unsigned int offset);
	void addVehicle (cVehicle& vehicle, unsigned int x, unsigned int y);
	void addVehicle (cVehicle& vehicle, unsigned int offset);

	/**
	* moves a vehicle to the given position
	* resets the vehicle to a single field, when it was centered on four fields
	* @param height defines the flight hight, when more then one planes on a field. 0 means top/highest.
	*/
	void moveVehicle (cVehicle& vehicle, unsigned int x, unsigned int y, int height = 0);

	/**
	* places a vehicle on the 4 fields to the right and below the given position
	*/
	void moveVehicleBig (cVehicle& vehicle, unsigned int x, unsigned int y);

	void deleteBuilding (const cBuilding& building);
	void deleteVehicle (const cVehicle& vehicle);

	static int getMapLevel (const cBuilding& building);
	static int getMapLevel (const cVehicle& vehicle);

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

	mutable cSignal<void (const cUnit&)> addedUnit;
	mutable cSignal<void (const cUnit&)> removedUnit;
	mutable cSignal<void (const cVehicle&)> movedVehicle;
public:
	std::shared_ptr<cStaticMap> staticMap;
	/**
	* the information about the fields
	*/
	cMapField* fields;
private:
	std::vector<sResources> Resources; // field with the ressource data
	T_2<int>* resSpots;
	int* resSpotTypes;
	int resSpotCount;
	int resCurrentSpotCount;
};

#endif
