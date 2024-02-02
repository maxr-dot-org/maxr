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

#include "map.h"

#include "game/data/map/mapfieldview.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "mapdownloader/mapdownload.h"
#include "settings.h"
#include "utility/crc.h"
#include "utility/listhelpers.h"
#include "utility/log.h"
#include "utility/position.h"
#include "utility/ranges.h"
#include "utility/string/toString.h"

#include <cassert>

static constexpr int MAX_PLANES_PER_FIELD = 5;

namespace
{
	//--------------------------------------------------------------------------
	std::vector<cPosition> getPositions (const cPosition& pos, bool isBig)
	{
		if (isBig)
		{
			return {pos, pos.relative (1, 0), pos.relative (0, 1), pos.relative (1, 1)};
		}
		else
		{
			return {pos};
		}
	}
} // namespace

//------------------------------------------------------------------------------
cVehicle* cMapField::getVehicle() const
{
	if (vehicles.empty()) return nullptr;
	return vehicles[0];
}

//------------------------------------------------------------------------------
cVehicle* cMapField::getPlane() const
{
	if (planes.empty()) return nullptr;
	return planes[0];
}

//------------------------------------------------------------------------------
const std::vector<cBuilding*>& cMapField::getBuildings() const
{
	return buildings;
}

//------------------------------------------------------------------------------
const std::vector<cVehicle*>& cMapField::getVehicles() const
{
	return vehicles;
}

//------------------------------------------------------------------------------
const std::vector<cVehicle*>& cMapField::getPlanes() const
{
	return planes;
}

//------------------------------------------------------------------------------
std::vector<cUnit*> cMapField::getUnits() const
{
	std::vector<cUnit*> units;
	units.reserve (vehicles.size() + buildings.size() + planes.size());
	units.insert (units.end(), vehicles.begin(), vehicles.end());
	units.insert (units.end(), buildings.begin(), buildings.end());
	units.insert (units.end(), planes.begin(), planes.end());
	return units;
}

//------------------------------------------------------------------------------
cBuilding* cMapField::getBuilding() const
{
	if (buildings.empty()) return nullptr;
	return buildings[0];
}

//------------------------------------------------------------------------------
cBuilding* cMapField::getTopBuilding() const
{
	if (buildings.empty()) return nullptr;
	cBuilding* building = buildings[0];

	if ((building->getStaticUnitData().surfacePosition == eSurfacePosition::Ground || building->getStaticUnitData().surfacePosition == eSurfacePosition::Above) && !building->isRubble())
		return building;
	return nullptr;
}

//------------------------------------------------------------------------------
cBuilding* cMapField::getBaseBuilding() const
{
	for (cBuilding* building : buildings)
	{
		if (building->getStaticUnitData().surfacePosition != eSurfacePosition::Ground && building->getStaticUnitData().surfacePosition != eSurfacePosition::Above && !building->isRubble())
		{
			return building;
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
cBuilding* cMapField::getRubble() const
{
	for (cBuilding* building : buildings)
		if (building->isRubble())
			return building;
	return nullptr;
}

//------------------------------------------------------------------------------
cBuilding* cMapField::getMine() const
{
	for (cBuilding* building : buildings)
		if (building->getStaticData().explodesOnContact)
			return building;
	return nullptr;
}

//------------------------------------------------------------------------------
bool cMapField::hasBridgeOrPlattform() const
{
	for (cBuilding* building : buildings)
	{
		if ((building->getStaticUnitData().surfacePosition == eSurfacePosition::AboveSea || building->getStaticUnitData().surfacePosition == eSurfacePosition::Base) && !building->isRubble())
		{
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------
void cMapField::addBuilding (cBuilding& building, size_t index)
{
	assert (index <= buildings.size());
	buildings.insert (buildings.begin() + index, &building);

	unitsChanged();
}

//------------------------------------------------------------------------------
void cMapField::addVehicle (cVehicle& vehicle, size_t index)
{
	assert (index <= vehicles.size());
	vehicles.insert (vehicles.begin() + index, &vehicle);

	unitsChanged();
}

//------------------------------------------------------------------------------
void cMapField::addPlane (cVehicle& plane, size_t index)
{
	assert (index <= planes.size());
	planes.insert (planes.begin() + index, &plane);

	unitsChanged();
}

//------------------------------------------------------------------------------
void cMapField::removeBuilding (const cBuilding& building)
{
	Remove (buildings, &building);

	unitsChanged();
}

//------------------------------------------------------------------------------
void cMapField::removeVehicle (const cVehicle& vehicle)
{
	Remove (vehicles, &vehicle);

	unitsChanged();
}

//------------------------------------------------------------------------------
void cMapField::removePlane (const cVehicle& plane)
{
	Remove (planes, &plane);

	unitsChanged();
}

//------------------------------------------------------------------------------
void cMapField::removeAll()
{
	buildings.clear();
	vehicles.clear();
	planes.clear();

	unitsChanged();
}

// cStaticMap //////////////////////////////////////////////////

//------------------------------------------------------------------------------
cStaticMap::cStaticMap() :
	graphic (this)
{
}

//------------------------------------------------------------------------------
cStaticMap::~cStaticMap()
{
}

//------------------------------------------------------------------------------
std::size_t cStaticMap::getTileIndex (const cPosition& position) const
{
	return Kacheln[getOffset (position)];
}

//------------------------------------------------------------------------------
const sTerrain& cStaticMap::getTerrain (const cPosition& position) const
{
	return terrains[getTileIndex (position)];
}

//------------------------------------------------------------------------------
bool cStaticMap::isBlocked (const cPosition& position) const
{
	return getTerrain (position).blocked;
}

//------------------------------------------------------------------------------
bool cStaticMap::isCoast (const cPosition& position) const
{
	return getTerrain (position).coast;
}

//------------------------------------------------------------------------------
bool cStaticMap::isWater (const cPosition& position) const
{
	return getTerrain (position).water;
}

//------------------------------------------------------------------------------
bool cStaticMap::isGround (const cPosition& position) const
{
	return !getTerrain (position).water && !getTerrain (position).coast;
}

//------------------------------------------------------------------------------
bool cStaticMap::possiblePlace (const cStaticUnitData& data, const cPosition& position) const
{
	const auto positions = getPositions (position, data.ID.isABuilding() && data.buildingData.isBig);

	if (!ranges::all_of (positions, [this] (const auto& pos) { return isValidPosition (pos); })) return false;
	if (data.factorAir > 0) return true;

	if (ranges::any_of (positions, [this] (const auto& pos) { return isBlocked (pos); })) return false;

	if (data.factorSea == 0 && ranges::any_of (positions, [this] (const auto& pos) { return isWater (pos); })) return false;
	if (data.factorCoast == 0 && ranges::any_of (positions, [this] (const auto& pos) { return isCoast (pos); })) return false;
	if (data.factorGround == 0 && ranges::any_of (positions, [this] (const auto& pos) { return isGround (pos); })) return false;

	return true;
}

//------------------------------------------------------------------------------
bool cStaticMap::isValidPosition (const cPosition& position) const
{
	return 0 <= position.x() && position.x() < size && 0 <= position.y() && position.y() < size;
}

//------------------------------------------------------------------------------
void cStaticMap::clear()
{
	filename.clear();
	size = 0;
	crc = 0;
	terrains.clear();
	Kacheln.clear();
}

//------------------------------------------------------------------------------
bool cStaticMap::isValid() const
{
	return !filename.empty();
}

//------------------------------------------------------------------------------
/** Loads a map file */
bool cStaticMap::loadMap (const std::filesystem::path& filename_)
{
	clear();
	// Open File
	filename = filename_;
	Log.debug ("Loading map \"" + filename_.u8string() + "\"");

	// first try in the factory maps directory
	auto fullFilename = cSettings::getInstance().getMapsPath() / filename;
	SDL_RWops* fpMapFile = SDL_RWFromFile (fullFilename.u8string().c_str(), "rb");
	if (fpMapFile == nullptr)
	{
		// now try in the user's map directory
		auto userMapsDir = cSettings::getInstance().getUserMapsDir();
		if (!userMapsDir.empty())
		{
			fullFilename = userMapsDir / filename;
			fpMapFile = SDL_RWFromFile (fullFilename.u8string().c_str(), "rb");
		}
	}
	if (fpMapFile == nullptr)
	{
		Log.warn ("Cannot load map file: \"" + filename.u8string() + "\"");
		clear();
		return false;
	}
	char szFileTyp[4];

	// check for typ
	SDL_RWread (fpMapFile, &szFileTyp, 1, 3);
	szFileTyp[3] = '\0';
	// WRL - interplays original mapformat
	// WRX - mapformat from russian mapeditor
	// DMO - for some reason some original maps have this filetype
	if (strcmp (szFileTyp, "WRL") != 0 && strcmp (szFileTyp, "WRX") != 0 && strcmp (szFileTyp, "DMO") != 0)
	{
		Log.warn ("Wrong file format: \"" + filename.u8string() + "\"");
		SDL_RWclose (fpMapFile);
		clear();
		return false;
	}
	SDL_RWseek (fpMapFile, 2, SEEK_CUR);

	// Read informations and get positions from the map-file
	const short sWidth = SDL_ReadLE16 (fpMapFile);
	Log.debug ("SizeX: " + std::to_string (sWidth));
	const short sHeight = SDL_ReadLE16 (fpMapFile);
	Log.debug ("SizeY: " + std::to_string (sHeight));
	SDL_RWseek (fpMapFile, sWidth * sHeight, SEEK_CUR); // Ignore Mini-Map
	const Sint64 iDataPos = SDL_RWtell (fpMapFile); // Map-Data
	SDL_RWseek (fpMapFile, sWidth * sHeight * 2, SEEK_CUR);
	const int iNumberOfTerrains = SDL_ReadLE16 (fpMapFile); // Read PicCount
	Log.debug ("Number of terrains: " + std::to_string (iNumberOfTerrains));
	const Sint64 iGraphicsPos = SDL_RWtell (fpMapFile); // Terrain Graphics
	const Sint64 iPalettePos = iGraphicsPos + iNumberOfTerrains * 64 * 64; // Palette
	const Sint64 iInfoPos = iPalettePos + 256 * 3; // Special informations

	if (sWidth != sHeight)
	{
		Log.warn ("Map must be quadratic!: \"" + filename.u8string() + "\"");
		SDL_RWclose (fpMapFile);
		clear();
		return false;
	}

	// Generate new Map
	this->size = std::max<int> (16, sWidth);
	Kacheln.resize (size * size, 0);
	terrains.resize (iNumberOfTerrains);

	graphic.loadPalette (fpMapFile, iPalettePos, iNumberOfTerrains);
	// Load necessary Terrain Graphics
	for (int iNum = 0; iNum < iNumberOfTerrains; iNum++)
	{
		unsigned char cByte; // one Byte
		// load terrain type info
		SDL_RWseek (fpMapFile, iInfoPos + iNum, SEEK_SET);
		SDL_RWread (fpMapFile, &cByte, 1, 1);

		switch (cByte)
		{
			case 0:
				//normal terrain without special property
				break;
			case 1:
				terrains[iNum].water = true;
				break;
			case 2:
				terrains[iNum].coast = true;
				break;
			case 3:
				terrains[iNum].blocked = true;
				break;
			default:
				Log.warn ("unknown terrain type " + std::to_string (cByte) + " on tile " + std::to_string (iNum) + " found. Handled as blocked!");
				terrains[iNum].blocked = true;
				//SDL_RWclose (fpMapFile);
				//return false;
		}
		if (!graphic.loadTile (fpMapFile, iGraphicsPos, iNum))
		{
			clear();
			return false;
		}
	}

	// Load map data
	SDL_RWseek (fpMapFile, iDataPos, SEEK_SET);
	for (int iY = 0; iY < size; iY++)
	{
		for (int iX = 0; iX < size; iX++)
		{
			int Kachel = SDL_ReadLE16 (fpMapFile);
			if (Kachel >= iNumberOfTerrains)
			{
				Log.warn ("a map field referred to a nonexisting terrain: " + std::to_string (Kachel));
				SDL_RWclose (fpMapFile);
				clear();
				return false;
			}
			Kacheln[iY * size + iX] = Kachel;
		}
	}
	SDL_RWclose (fpMapFile);

	//save crc, to check map file equality when loading a game
	crc = MapDownload::calculateCheckSum (filename);
	return true;
}

//------------------------------------------------------------------------------
uint32_t cStaticMap::getChecksum (uint32_t crc) const
{
	return calcCheckSum (this->crc, crc);
}

// Funktionen der Map-Klasse /////////////////////////////////////////////////
//------------------------------------------------------------------------------
cMap::cMap (std::shared_ptr<cStaticMap> staticMap_) :
	staticMap (std::move (staticMap_))
{
	init();
}

//------------------------------------------------------------------------------
void cMap::init()
{
	const std::size_t size = staticMap->getSize().x() * staticMap->getSize().y();
	if (Resources.size() != size)
	{
		Resources.resize (size, sResources());
		fields = std::vector<cMapField> (size);
	}
}

//------------------------------------------------------------------------------
cMap::~cMap()
{
}

//------------------------------------------------------------------------------
cMapField& cMap::getField (const cPosition& position)
{
	return fields[getOffset (position)];
}

//------------------------------------------------------------------------------
const cMapField& cMap::getField (const cPosition& position) const
{
	return fields[getOffset (position)];
}

//------------------------------------------------------------------------------
bool cMap::isWaterOrCoast (const cPosition& position) const
{
	const sTerrain& terrainType = staticMap->getTerrain (position);
	return terrainType.water || terrainType.coast;
}

//------------------------------------------------------------------------------
std::string cMap::resourcesToString() const
{
	std::string str;
	str.reserve (4 * Resources.size() + 1);
	for (size_t i = 0; i != Resources.size(); ++i)
	{
		str += getHexValue (static_cast<int> (Resources[i].typ));
		str += getHexValue (Resources[i].value);
	}
	return str;
}

//------------------------------------------------------------------------------
void cMap::setResourcesFromString (const std::string& str)
{
	for (size_t i = 0; i != Resources.size(); ++i)
	{
		sResources res;
		res.typ = static_cast<eResourceType> (getByteValue (str, 4 * i));
		res.value = getByteValue (str, 4 * i + 2);

		Resources.set (i, res);
	}
}

//------------------------------------------------------------------------------
/* static */ int cMap::getMapLevel (const cBuilding& building)
{
	const cStaticUnitData& data = building.getStaticUnitData();

	if (building.isRubble()) return 4; // rubble

	if (data.surfacePosition == eSurfacePosition::BeneathSea) return 9; // seamine
	if (data.surfacePosition == eSurfacePosition::AboveSea) return 7; // bridge
	if (data.surfacePosition == eSurfacePosition::Base && building.getStaticData().canBeOverbuild != eOverbuildType::No) return 6; // platform
	if (data.surfacePosition == eSurfacePosition::Base) return 5; // road

	if (data.surfacePosition == eSurfacePosition::AboveBase) return 3; // landmine

	return 1; // other buildings
}

//------------------------------------------------------------------------------
/* static */ int cMap::getMapLevel (const cVehicle& vehicle)
{
	if (vehicle.getStaticUnitData().factorSea > 0 && vehicle.getStaticUnitData().factorGround == 0) return 8; // ships
	if (vehicle.getStaticUnitData().factorAir > 0) return 0; // planes

	return 2; // other vehicles
}

//------------------------------------------------------------------------------
void cMap::addBuilding (cBuilding& building, const cPosition& position)
{
	//big base building are not implemented
	if (building.getStaticUnitData().surfacePosition != eSurfacePosition::Ground && building.getIsBig() && !building.isRubble()) return;

	const int mapLevel = cMap::getMapLevel (building);

	for (const auto& pos : getPositions (position, building.getIsBig()))
	{
		auto& field = getField (pos);
		std::size_t i = 0;
		while (i < field.getBuildings().size() && cMap::getMapLevel (*field.getBuildings()[i]) < mapLevel)
		{
			i++;
		}
		field.addBuilding (building, i);
	}
	addedUnit (building);
}

//------------------------------------------------------------------------------
void cMap::addVehicle (cVehicle& vehicle, const cPosition& position)
{
	auto& field = getField (position);
	if (vehicle.getStaticUnitData().factorAir > 0)
	{
		field.addPlane (vehicle, 0);
	}
	else
	{
		field.addVehicle (vehicle, 0);
	}

	if (vehicle.getIsBig())
	{
		vehicle.buildBigSavedPosition.reset();
		moveVehicleBig (vehicle, position);
	}
	addedUnit (vehicle);
}

//------------------------------------------------------------------------------
void cMap::deleteBuilding (const cBuilding& building)
{
	for (const auto& position : building.getPositions())
	{
		getField (position).removeBuilding (building);
	}
	removedUnit (building);
}

//------------------------------------------------------------------------------
void cMap::deleteVehicle (const cVehicle& vehicle)
{
	if (vehicle.getStaticUnitData().factorAir > 0)
	{
		getField (vehicle.getPosition()).removePlane (vehicle);
	}
	else
	{
		for (const auto& position : vehicle.getPositions())
		{
			getField (position).removeVehicle (vehicle);
		}
	}
	removedUnit (vehicle);
}

//------------------------------------------------------------------------------
void cMap::deleteUnit (const cUnit& unit)
{
	if (const auto* building = dynamic_cast<const cBuilding*> (&unit))
	{
		deleteBuilding (*building);
	}
	else if (const auto* vehicle = dynamic_cast<const cVehicle*> (&unit))
	{
		deleteVehicle (*vehicle);
	}
}

//------------------------------------------------------------------------------
void cMap::moveVehicle (cVehicle& vehicle, const cPosition& position, int height)
{
	const auto oldPosition = vehicle.getPosition();

	vehicle.setPosition (position);

	if (vehicle.getStaticUnitData().factorAir > 0)
	{
		getField (oldPosition).removePlane (vehicle);
		height = std::min<int> (getField (position).getPlanes().size(), height);
		getField (position).addPlane (vehicle, height);
	}
	else
	{
		for (const auto& pos : getPositions (oldPosition, vehicle.getIsBig()))
		{
			getField (pos).removeVehicle (vehicle);
		}
		vehicle.buildBigSavedPosition.reset();
		getField (position).addVehicle (vehicle, 0);
	}
	movedVehicle (vehicle, oldPosition);
}

//------------------------------------------------------------------------------
void cMap::moveVehicleBig (cVehicle& vehicle, const cPosition& position)
{
	if (vehicle.getIsBig())
	{
		NetLog.error ("Calling moveVehicleBig on a big vehicle");
		//calling this function twice is always an error.
		//nevertheless try to proceed by resetting the data.isBig flag
		moveVehicle (vehicle, position);
	}

	const auto oldPosition = vehicle.getPosition();

	getField (oldPosition).removeVehicle (vehicle);

	vehicle.setPosition (position);

	getField (position).addVehicle (vehicle, 0);
	getField (position + cPosition (1, 0)).addVehicle (vehicle, 0);
	getField (position + cPosition (1, 1)).addVehicle (vehicle, 0);
	getField (position + cPosition (0, 1)).addVehicle (vehicle, 0);

	vehicle.buildBigSavedPosition = oldPosition;

	movedVehicle (vehicle, oldPosition);
}

//------------------------------------------------------------------------------
bool cMap::possiblePlace (const cVehicle& vehicle, const cPosition& position, bool checkPlayer, bool ignoreMovingVehicles) const
{
	return possiblePlaceVehicle (vehicle.getStaticUnitData(), position, checkPlayer ? vehicle.getOwner() : nullptr, ignoreMovingVehicles);
}

//------------------------------------------------------------------------------
bool cMap::possiblePlaceVehicle (const cStaticUnitData& vehicleData, const cPosition& position, const cPlayer* player, bool ignoreMovingVehicles) const
{
	if (isValidPosition (position) == false) return false;
	const auto field = cMapFieldView (getField (position), staticMap->getTerrain (position), player);

	const std::vector<cBuilding*> buildings = field.getBuildings();
	std::vector<cBuilding*>::const_iterator b_it = buildings.begin();
	std::vector<cBuilding*>::const_iterator b_end = buildings.end();

	//search first building, that is not a connector
	if (b_it != b_end && (*b_it)->getStaticUnitData().surfacePosition == eSurfacePosition::Above) ++b_it;

	if (vehicleData.factorAir > 0)
	{
		if (player && !player->canSeeAt (position)) return true;

		const auto& planes = field.getPlanes();
		if (!ignoreMovingVehicles)
		{
			if (planes.size() >= MAX_PLANES_PER_FIELD) return false;
		}
		else
		{
			const int notMovingPlanes = ranges::count_if (planes, [](const auto* plane) { return !plane->isUnitMoving(); });
			if (notMovingPlanes >= MAX_PLANES_PER_FIELD) return false;
		}
	}
	if (vehicleData.factorGround > 0)
	{
		if (isBlocked (position)) return false;

		if ((isWater (position) && vehicleData.factorSea == 0) || (isCoast (position) && vehicleData.factorCoast == 0))
		{
			if (player && !player->canSeeAt (position)) return false;

			//vehicle can drive on water, if there is a bridge, platform or road
			if (b_it == b_end) return false;
			if ((*b_it)->getStaticUnitData().surfacePosition != eSurfacePosition::AboveSea && (*b_it)->getStaticUnitData().surfacePosition != eSurfacePosition::Base && (*b_it)->getStaticUnitData().surfacePosition != eSurfacePosition::AboveBase) return false;
		}
		//check for enemy mines
		if (player && b_it != b_end && (*b_it)->getOwner() != player && (*b_it)->getStaticData().explodesOnContact && (*b_it)->isDetectedByPlayer (player))
		{
			return false;
		}

		if (player && !player->canSeeAt (position)) return true;

		if (field.getVehicle() && (!ignoreMovingVehicles || !field.getVehicle()->isUnitMoving()))
		{
			return false;
		}
		if (field.getPlane() && field.getPlane()->getFlightHeight() == 0)
		{
			return false;
		}
		if (b_it != b_end)
		{
			// only base buildings and rubble is allowed on the same field with a vehicle
			// (connectors have been skiped, so doesn't matter here)
			if ((*b_it)->getStaticUnitData().surfacePosition != eSurfacePosition::AboveSea && (*b_it)->getStaticUnitData().surfacePosition != eSurfacePosition::Base && (*b_it)->getStaticUnitData().surfacePosition != eSurfacePosition::AboveBase && (*b_it)->getStaticUnitData().surfacePosition != eSurfacePosition::BeneathSea && !(*b_it)->isRubble()) return false;
		}
	}
	else if (vehicleData.factorSea > 0)
	{
		if (isBlocked (position)) return false;

		if (!isWater (position) && (!isCoast (position) || vehicleData.factorCoast == 0)) return false;

		//check for enemy mines
		if (player && b_it != b_end && (*b_it)->getOwner() != player && (*b_it)->getStaticData().explodesOnContact && (*b_it)->isDetectedByPlayer (player))
		{
			return false;
		}

		if (player && !player->canSeeAt (position)) return true;

		if (field.getVehicle() && (!ignoreMovingVehicles || !field.getVehicle()->isUnitMoving()))
		{
			return false;
		}

		//only bridge and sea mine are allowed on the same field with a ship (connectors have been skiped, so doesn't matter here)
		if (b_it != b_end && (*b_it)->getStaticUnitData().surfacePosition != eSurfacePosition::AboveSea && (*b_it)->getStaticUnitData().surfacePosition != eSurfacePosition::BeneathSea)
		{
			// if the building is a landmine, we have to check whether it's on a bridge or not
			if ((*b_it)->getStaticUnitData().surfacePosition == eSurfacePosition::AboveBase)
			{
				++b_it;
				if (b_it == b_end || (*b_it)->getStaticUnitData().surfacePosition != eSurfacePosition::AboveSea) return false;
			}
			else
				return false;
		}
	}
	return true;
}

//------------------------------------------------------------------------------
bool cMap::possiblePlaceBuilding (const cStaticUnitData& buildingData, const cPosition& position, const cPlayer* player, const cVehicle* vehicle) const
{
	if (!isValidPosition (position)) return false;
	if (isBlocked (position)) return false;
	const auto field = cMapFieldView (getField (position), staticMap->getTerrain (position), player);

	// Check all buildings in this field for a building of the same type. This
	// will prevent roads, connectors and water platforms from building on top
	// of themselves.
	const std::vector<cBuilding*>& buildings = field.getBuildings();
	for (const cBuilding* building : buildings)
	{
		if (building->getStaticUnitData().ID == buildingData.ID)
		{
			return false;
		}
	}

	// Determine terrain type
	bool water = isWater (position);
	bool coast = isCoast (position);
	bool ground = !water && !coast;

	for (const cBuilding* building : buildings)
	{
		if (buildingData.surfacePosition == building->getStaticUnitData().surfacePosition && building->getStaticData().canBeOverbuild == eOverbuildType::No) return false;
		switch (building->getStaticUnitData().surfacePosition)
		{
			case eSurfacePosition::Ground:
			case eSurfacePosition::AboveSea: // bridge
				if (buildingData.surfacePosition != eSurfacePosition::Above && buildingData.surfacePosition != eSurfacePosition::Base && // mine can be placed on bridge
				    buildingData.surfacePosition != eSurfacePosition::BeneathSea && // seamine can be placed under bridge
				    building->getStaticData().canBeOverbuild == eOverbuildType::No) return false;
				break;
			case eSurfacePosition::BeneathSea: // seamine
			case eSurfacePosition::AboveBase: // landmine
				// mine must be removed first
				if (buildingData.surfacePosition != eSurfacePosition::Above) return false;
				break;
			case eSurfacePosition::Base: // platform, road
				water = coast = false;
				ground = true;
				break;
			case eSurfacePosition::Above: // connector
				break;
		}
	}
	if ((water && buildingData.factorSea == 0) || (coast && buildingData.factorCoast == 0) || (ground && buildingData.factorGround == 0)) return false;

	//can not build on rubble
	if (field.getRubble() && buildingData.surfacePosition != eSurfacePosition::Above && buildingData.surfacePosition != eSurfacePosition::AboveBase) return false;

	if (field.getVehicle())
	{
		if (!vehicle) return false;
		if (vehicle != field.getVehicle()) return false;
	}
	return true;
}

//------------------------------------------------------------------------------
void cMap::reset()
{
	for (int i = 0; i < getSize().x() * getSize().y(); i++)
	{
		fields[i].removeAll();
	}
}

//------------------------------------------------------------------------------
uint32_t cMap::getChecksum (uint32_t crc) const
{
	crc = staticMap->getChecksum (crc);
	//cMapField* fields;
	crc = calcCheckSum (Resources, crc);

	return crc;
}

//------------------------------------------------------------------------------
uint32_t sResources::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (value, crc);
	crc = calcCheckSum (typ, crc);

	return crc;
}
