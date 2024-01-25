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

#include "game/data/gamesettings.h"
#include "game/data/resourcetype.h"
#include "resources/map/graphicstaticmap.h"
#include "utility/arraycrc.h"
#include "utility/log.h"
#include "utility/position.h"
#include "utility/signal/signal.h"
#include "utility/t_2.h"

#include <cassert>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class cUnit;
class cVehicle;
class cBuilding;
class cPlayer;
class cPosition;
class cStaticUnitData;

// Resources Struktur ////////////////////////////////////////////////////////
struct sResources
{
public:
	sResources() = default;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (value);
		archive & NVP (typ);
		// clang-format on
	}
	uint32_t getChecksum (uint32_t crc) const;

public:
	unsigned char value = 0;
	eResourceType typ = eResourceType::None;
};

/** contains all information of a map field */
class cMapField
{
public:
	cMapField() = default;

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
	std::vector<cUnit*> getUnits() const;

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

	/** checks if there is a building that allows gorund units on water fields */
	bool hasBridgeOrPlattform() const;

	/** Adds the passed building before the given index to the building list of the field */
	void addBuilding (cBuilding&, size_t index);
	/** Adds the passed vehicle before the given index to the vehicle list of the field */
	void addVehicle (cVehicle&, size_t index);
	/** Adds the passed plane before the given index to the plane list of the field */
	void addPlane (cVehicle& plane, size_t index);

	/** Removes the passed building from the field's building list */
	void removeBuilding (const cBuilding&);
	/** Removes the passed vehicle from the field's vehicle list */
	void removeVehicle (const cVehicle&);
	/** Removes the passed plane from the field's plane list */
	void removePlane (const cVehicle& plane);

	/** Removed all units from the field */
	void removeAll();

	/** Triggered when any unit (building, vehicle or plane) has been added or removed to/from the field */
	mutable cSignal<void()> unitsChanged;

private:
	cMapField (const cMapField&) = delete;
	cMapField& operator= (const cMapField&) = delete;

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
	bool water = false; /** is this terrain water? */
	bool coast = false; /** is this terrain a coast? */
	bool blocked = false; /** is this terrain blocked? */
};

class cStaticMap
{
public:
	cStaticMap();
	~cStaticMap();

	void clear();
	bool loadMap (const std::filesystem::path& filename);
	bool isValid() const;

	const std::filesystem::path& getFilename() const { return filename; }
	cPosition getSize() const { return cPosition (size, size); }
	int getOffset (const cPosition& pos) const
	{
		assert (isValidPosition (pos));
		return pos.y() * size + pos.x();
	}

	bool isValidPosition (const cPosition&) const;

	bool isBlocked (const cPosition&) const;
	bool isCoast (const cPosition&) const;
	bool isWater (const cPosition&) const;
	bool isGround (const cPosition&) const;

	bool possiblePlace (const cStaticUnitData&, const cPosition&) const;

	std::size_t getTileIndex (const cPosition&) const;
	const sTerrain& getTerrain (const cPosition&) const;

	cGraphicStaticMap& getGraphic() const { return graphic; }

	uint32_t getChecksum (uint32_t crc) const;

	template <typename Archive>
	void save (Archive& archive) const
	{
		archive << NVP (filename);
		archive << NVP (crc);
	}
	template <typename Archive>
	void load (Archive& archive)
	{
		std::filesystem::path fileToLoad;
		archive >> serialization::makeNvp ("filename", fileToLoad);
		uint32_t crcFromSave;
		archive >> serialization::makeNvp ("crc", crcFromSave);

		if (filename == fileToLoad && crc == crcFromSave)
		{
			NetLog.debug ("Static map already loaded. Skipped...");
			return;
		}
		if (!loadMap (fileToLoad))
			throw std::runtime_error ("Loading map failed.");

		if (crc != crcFromSave && crcFromSave != 0)
			throw std::runtime_error ("CRC error while loading map. The loaded map file is not equal to the one the game was started with.");
	}

	SERIALIZATION_SPLIT_MEMBER()
private:
	std::filesystem::path filename; // Name of the current map
	uint32_t crc = 0;
	int size = 0;
	std::vector<int> Kacheln; // Terrain numbers of the map fields
	std::vector<sTerrain> terrains; // The different terrain type.
	mutable cGraphicStaticMap graphic;
};

class cMap
{
public:
	explicit cMap (std::shared_ptr<cStaticMap>);

	~cMap();

	const std::filesystem::path& getFilename() const { return staticMap->getFilename(); }
	cPosition getSize() const { return staticMap->getSize(); }
	int getOffset (const cPosition& pos) const { return staticMap->getOffset (pos); }
	bool isValidPosition (const cPosition& pos) const { return staticMap->isValidPosition (pos); }

	bool isBlocked (const cPosition& position) const { return staticMap->isBlocked (position); }
	bool isCoast (const cPosition& position) const { return staticMap->isCoast (position); }
	bool isWater (const cPosition& position) const { return staticMap->isWater (position); }

	bool isWaterOrCoast (const cPosition&) const;

	const sResources& getResource (const cPosition& position) const { return Resources[getOffset (position)]; }
	void setResource (const cPosition& position, const sResources& res) { return Resources.set (getOffset (position), res); }

	cMapField& getField (const cPosition&);
	const cMapField& getField (const cPosition&) const;

	void addBuilding (cBuilding&, const cPosition&);
	void addVehicle (cVehicle&, const cPosition&);

	/**
	* moves a vehicle to the given position
	* resets the vehicle to a single field, when it was centered on four fields
	* @param height defines the flight hight, when more than one plane on a field. 0 means top/highest.
	*/
	void moveVehicle (cVehicle&, const cPosition&, int height = 0);

	/**
	* places a vehicle on the 4 fields to the right and below the given position
	*/
	void moveVehicleBig (cVehicle&, const cPosition&);

	void deleteBuilding (const cBuilding&);
	void deleteVehicle (const cVehicle&);
	void deleteUnit (const cUnit&);

	/**
	* checks, whether the given field is an allowed place for the vehicle
	* if checkPlayer is passed, the function uses the players point of view, so it does not check for units that are not in sight
	*/
	bool possiblePlace (const cVehicle&, const cPosition&, bool checkPlayer, bool ignoreMovingVehicles = false) const;
	bool possiblePlaceVehicle (const cStaticUnitData& vehicleData, const cPosition&, const cPlayer*, bool ignoreMovingVehicles = false) const;

	/**
	* checks, whether the given field is an allowed place for the building
	* if a vehicle is passed, it will be ignored in the check, so a constructing vehicle does not block its own position
	*/
	bool possiblePlaceBuilding (const cStaticUnitData& buildingData, const cPosition&, const cPlayer*, const cVehicle* = nullptr) const;

	/**
	* removes all units from the map structure
	*/
	void reset();

	uint32_t getChecksum (uint32_t crc) const;

	template <typename Archive>
	void save (Archive& archive) const
	{
		archive << serialization::makeNvp ("mapFile", *staticMap);
		const std::string resources = resourcesToString();
		archive << NVP (resources);
	}
	template <typename Archive>
	void load (Archive& archive)
	{
		assert (staticMap != nullptr);
		archive >> serialization::makeNvp ("mapFile", *staticMap);
		init();

		std::string resources;
		archive >> NVP (resources);
		setResourcesFromString (resources);
	}
	SERIALIZATION_SPLIT_MEMBER()

private:
	void init();
	std::string resourcesToString() const;
	void setResourcesFromString (const std::string&);

	static int getMapLevel (const cBuilding&);
	static int getMapLevel (const cVehicle&);

public:
	mutable cSignal<void (const cUnit&)> addedUnit;
	mutable cSignal<void (const cUnit&)> removedUnit;
	mutable cSignal<void (const cVehicle&, const cPosition&)> movedVehicle;

	std::shared_ptr<cStaticMap> staticMap;

private:
	/**
	* the information about the fields
	*/
	std::vector<cMapField> fields;
	cArrayCrc<sResources> Resources; // field with the resource data
};

#endif // game_data_map_mapH
