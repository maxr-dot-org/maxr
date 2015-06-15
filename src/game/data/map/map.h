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

#ifndef game_data_map_mapH
#define game_data_map_mapH

#include <string>
#include <memory>
#include <vector>

#include "utility/autosurface.h"
#include "defines.h"
#include "t_2.h"
#include "utility/position.h"
#include "utility/signal/signal.h"
#include "game/data/gamesettings.h"

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
public:
	cMapField();

	/** returns the top vehicle on this field */
	cVehicle* getVehicle() const;

	/** returns a Iterator for the planes on this field */
	cVehicle* getPlane() const;

	/** returns the buildings on this field */
	const std::vector<cBuilding*>& getBuildings() const;
	/** returns the vehicles on this field */
	const std::vector<cVehicle*>& getVehicles() const;
	/** returns the planes on this field */
	const std::vector<cVehicle*>& getPlanes() const;

	/** returns all units on this field */
	void getUnits (std::vector<cUnit*>& units) const;

	/** returns a pointer for the buildings on this field */
	cBuilding* getBuilding() const;
	/** returns a pointer to the top building or nullptr if the first building is a base type */
	cBuilding* getTopBuilding() const;
	/** returns a pointer to the first base building or nullptr if there is no base building */
	cBuilding* getBaseBuilding() const;
	/** returns a pointer to a rubble object, if there is one. */
	cBuilding* getRubble() const;
	/** returns a pointer to an expl. mine, if there is one */
	cBuilding* getMine() const;

	/** Adds the passed building before the given index to the building list of the field */
	void addBuilding (cBuilding& building, size_t index);
	/** Adds the passed vehicle before the given index to the vehicle list of the field */
	void addVehicle (cVehicle& vehicle, size_t index);
	/** Adds the passed plane before the given index to the plane list of the field */
	void addPlane (cVehicle& plane, size_t index);

	/** Removes the passed building from the field's building list */
	void removeBuilding (const cBuilding& building);
	/** Removes the passed vehicle from the field's vehicle list */
	void removeVehicle (const cVehicle& vehicle);
	/** Removes the passed plane from the field's plane list */
	void removePlane (const cVehicle& plane);

	/** Removed all units from the field */
	void removeAll();

	/** Triggered when a building has been added or removed to/from the field */
	mutable cSignal<void ()> buildingsChanged;
	/** Triggered when a vehicle has been added or removed to/from the field */
	mutable cSignal<void ()> vehiclesChanged;
	/** Triggered when a plane has been added or removed to/from the field */
	mutable cSignal<void ()> planesChanged;
	/** Triggered when any unit (building, vehicle or plane) has been added or removed to/from the field */
	mutable cSignal<void ()> unitsChanged;

private:
	cMapField (const cMapField& other) MAXR_DELETE_FUNCTION;
	cMapField& operator= (const cMapField& other) MAXR_DELETE_FUNCTION;

	/**the list with all buildings on this field
	* the top building is always stored at first position */
	std::vector<cBuilding*> buildings;
	/** the list with all planes on this field
	* the top plane is always stored at first position */
	std::vector<cVehicle*> vehicles;
	/**the list with all vehicles on this field
	* the top vehicle is always stored at first position */
	std::vector<cVehicle*> planes;

};

struct sTerrain
{
	sTerrain();
	sTerrain(sTerrain&& other);
	sTerrain& operator=(sTerrain&& other);

	AutoSurface sf;      /** the scaled surface of the terrain */
	AutoSurface sf_org;  /** the original surface of the terrain */
	AutoSurface shw;     /** the scaled surface of the terrain in the fog */
	AutoSurface shw_org; /** the original surface of the terrain in the fog */
	bool water;          /** is this terrain water? */
	bool coast;          /** is this terrain a coast? */
	bool blocked;        /** is this terrain blocked? */

private:
	sTerrain(const sTerrain& other) MAXR_DELETE_FUNCTION;
	sTerrain& operator=(const sTerrain& other) MAXR_DELETE_FUNCTION;
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
	cPosition getSize() const { return cPosition (size, size); }
	int getOffset (const cPosition& pos) const { assert (isValidPosition (pos));  return pos.y() * size + pos.x(); }

	bool isValidPosition (const cPosition& position) const;

	bool isBlocked (const cPosition& position) const;
	bool isCoast (const cPosition& position) const;
	bool isWater (const cPosition& position) const;

	const sTerrain& getTerrain (const cPosition& position) const;

	AutoSurface createBigSurface (int sizex, int sizey) const;
	void generateNextAnimationFrame();
	void scaleSurfaces (int pixelSize);
	static AutoSurface loadMapPreview (const std::string& mapPath, int* mapSize = nullptr);
private:
	static AutoSurface loadTerrGraph (SDL_RWops* fpMapFile, Sint64 iGraphicsPos, const SDL_Color (&colors)[256], int iNum);
	void copySrfToTerData (SDL_Surface& surface, int iNum);

	std::string filename;   // Name of the current map
	int size;
	std::vector<sTerrain> terrains; // The different terrain type.
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
	cPosition getSize() const { return staticMap->getSize(); }
	int getOffset (const cPosition& pos) const { return staticMap->getOffset (pos); }
	bool isValidPosition (const cPosition& pos) const { return staticMap->isValidPosition (pos); }

	bool isBlocked (const cPosition& position) const { return staticMap->isBlocked (position); }
	bool isCoast (const cPosition& position) const { return staticMap->isCoast (position); }
	bool isWater (const cPosition& position) const { return staticMap->isWater (position); }

	bool isWaterOrCoast (const cPosition& position) const;

	const sResources& getResource (const cPosition& position) const { return Resources[getOffset (position)]; }
	sResources& getResource (const cPosition& position) { return Resources[getOffset (position)]; }
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

	void placeRessources (const std::vector<cPosition>& landingPositions, const cGameSettings& gameSetting);

	/**
	* Access to a map field
	* @param the offset of the map field
	* @return an instance of cMapField, which has several methods to access the objects on the field
	*/
	cMapField& operator[] (unsigned int offset) const;

	cMapField& getField (const cPosition& position);
	const cMapField& getField (const cPosition& position) const;

	void addBuilding (cBuilding& building, const cPosition& position);
	void addVehicle (cVehicle& vehicle, const cPosition& position);

	/**
	* moves a vehicle to the given position
	* resets the vehicle to a single field, when it was centered on four fields
	* @param height defines the flight hight, when more then one planes on a field. 0 means top/highest.
	*/
	void moveVehicle (cVehicle& vehicle, const cPosition& position, int height = 0);

	/**
	* places a vehicle on the 4 fields to the right and below the given position
	*/
	void moveVehicleBig (cVehicle& vehicle, const cPosition& position);

	void deleteBuilding (const cBuilding& building);
	void deleteVehicle (const cVehicle& vehicle);
	void deleteUnit (const cUnit& unit);

	/**
	* checks, whether the given field is an allowed place for the vehicle
	* if checkPlayer is passed, the function uses the players point of view, so it does not check for units that are not in sight
	*/
	bool possiblePlace (const cVehicle& vehicle, const cPosition& position, bool checkPlayer = false) const;
	bool possiblePlaceVehicle (const sUnitData& vehicleData, const cPosition& position, const cPlayer* player, bool checkPlayer = false) const;

	/**
	* checks, whether the given field is an allowed place for the building
	* if a vehicle is passed, it will be ignored in the check, so a constructing vehicle does not block its own position
	* Note: that the function can check for map border overflows (with margin).
	*/
	bool possiblePlaceBuilding (const sUnitData& buildingData, const cPosition& position, const cVehicle* vehicle = nullptr) const;
	bool possiblePlaceBuildingWithMargin (const sUnitData& buildingData, const cPosition& position, int margin, const cVehicle* vehicle = nullptr) const;

	/**
	* removes all units from the map structure
	*/
	void reset();

private:
	static int getMapLevel (const cBuilding& building);
	static int getMapLevel (const cVehicle& vehicle);
	static int getResourceDensityFactor (eGameSettingsResourceDensity density);
	static int getResourceAmountFactor (eGameSettingsResourceAmount amount);

public:
	mutable cSignal<void (const cUnit&)> addedUnit;
	mutable cSignal<void (const cUnit&)> removedUnit;
	mutable cSignal<void (const cVehicle&, const cPosition&)> movedVehicle;

	std::shared_ptr<cStaticMap> staticMap;
private:
	/**
	* the information about the fields
	*/
	cMapField* fields;
	std::vector<sResources> Resources; // field with the ressource data
};

#endif // game_data_map_mapH
