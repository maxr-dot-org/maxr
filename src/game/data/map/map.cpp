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

#include "game/data/map/map.h"

#include "game/data/units/building.h"
#include "utility/listhelpers.h"
#include "utility/mathtools.h"
#include "utility/files.h"
#include "utility/crc.h"
#include "utility/log.h"
#include "utility/string/toString.h"
#include "game/data/player/player.h"
#include "settings.h"
#include "game/data/units/vehicle.h"
#include "output/video/video.h"
#include "utility/position.h"
#include "game/data/model.h"
#include "mapdownloader/mapdownload.h"

#if 1 // TODO: [SDL2]: SDL_SetColors
inline void SDL_SetColors (SDL_Surface* surface, const SDL_Color* colors, int index, int size)
{
	SDL_SetPaletteColors (surface->format->palette, colors, index, size);
}
#endif

//------------------------------------------------------------------------------
void sGraphicTile::copySrfToTerData (SDL_Surface& surface, const SDL_Color (&palette_shw)[256])
{
	//before the surfaces are copied, the colortable of both surfaces has to be equal
	//This is needed to make sure, that the pixeldata is copied 1:1

	//copy the normal terrain
	sf_org = AutoSurface (SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0));
	SDL_SetPaletteColors (sf_org->format->palette, surface.format->palette->colors, 0, 256);
	SDL_BlitSurface (&surface, nullptr, sf_org.get(), nullptr);

	sf = AutoSurface (SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0));
	SDL_SetPaletteColors (sf->format->palette, surface.format->palette->colors, 0, 256);
	SDL_BlitSurface (&surface, nullptr, sf.get(), nullptr);

	//copy the terrains with fog
	shw_org = AutoSurface (SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0));
	SDL_SetColors (shw_org.get(), surface.format->palette->colors, 0, 256);
	SDL_BlitSurface (&surface, nullptr, shw_org.get(), nullptr);

	shw = AutoSurface (SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0));
	SDL_SetColors (shw.get(), surface.format->palette->colors, 0, 256);
	SDL_BlitSurface (&surface, nullptr, shw.get(), nullptr);

	//now set the palette for the fog terrains
	SDL_SetColors (shw_org.get(), palette_shw, 0, 256);
	SDL_SetColors (shw.get(), palette_shw, 0, 256);
}

//------------------------------------------------------------------------------
cMapField::cMapField()
{}

cVehicle* cMapField::getVehicle() const
{
	if (vehicles.empty()) return nullptr;
	return vehicles[0];
}

cVehicle* cMapField::getPlane() const
{
	if (planes.empty()) return nullptr;
	return planes[0];
}

const std::vector<cBuilding*>& cMapField::getBuildings() const
{
	return buildings;
}

const std::vector<cVehicle*>& cMapField::getVehicles() const
{
	return vehicles;
}

const std::vector<cVehicle*>& cMapField::getPlanes() const
{
	return planes;
}

void cMapField::getUnits (std::vector<cUnit*>& units) const
{
	units.clear();
	units.reserve (vehicles.size() + buildings.size() + planes.size());
	units.insert (units.end(), vehicles.begin(), vehicles.end());
	units.insert (units.end(), buildings.begin(), buildings.end());
	units.insert (units.end(), planes.begin(), planes.end());
}

cBuilding* cMapField::getBuilding() const
{
	if (buildings.empty()) return nullptr;
	return buildings[0];
}

cBuilding* cMapField::getTopBuilding() const
{
	if (buildings.empty()) return nullptr;
	cBuilding* building = buildings[0];

	if ((building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_GROUND ||
		 building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE) &&
		!building->isRubble())
		return building;
	return nullptr;
}

cBuilding* cMapField::getBaseBuilding() const
{
	for (cBuilding* building : buildings)
	{
		if (building->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_GROUND &&
			building->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE &&
			!building->isRubble())
		{
			return building;
		}
	}
	return nullptr;
}

cBuilding* cMapField::getRubble() const
{
	for (cBuilding* building : buildings)
		if (building->isRubble())
			return building;
	return nullptr;
}

cBuilding* cMapField::getMine() const
{
	for (cBuilding* building : buildings)
		if (building->getStaticUnitData().explodesOnContact)
			return building;
	return nullptr;
}

bool cMapField::hasBridgeOrPlattform() const
{
	for (cBuilding* building : buildings)
	{
		if ((building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE_SEA ||
			building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_BASE) &&
			!building->isRubble())
		{
			return true;
		}
	}
	return false;
}

void cMapField::addBuilding (cBuilding& building, size_t index)
{
	assert (index <= buildings.size());
	buildings.insert (buildings.begin() + index, &building);

	buildingsChanged();
	unitsChanged();
}
void cMapField::addVehicle (cVehicle& vehicle, size_t index)
{
	assert (index <= vehicles.size());
	vehicles.insert (vehicles.begin() + index, &vehicle);

	vehiclesChanged();
	unitsChanged();
}

void cMapField::addPlane (cVehicle& plane, size_t index)
{
	assert (index <= planes.size());
	planes.insert (planes.begin() + index, &plane);

	planesChanged();
	unitsChanged();
}

void cMapField::removeBuilding (const cBuilding& building)
{
	Remove (buildings, &building);

	buildingsChanged();
	unitsChanged();
}

void cMapField::removeVehicle (const cVehicle& vehicle)
{
	Remove (vehicles, &vehicle);

	vehiclesChanged();
	unitsChanged();
}

void cMapField::removePlane (const cVehicle& plane)
{
	Remove (planes, &plane);

	planesChanged();
	unitsChanged();
}

void cMapField::removeAll()
{
	buildings.clear();
	vehicles.clear();
	planes.clear();

	buildingsChanged();
	vehiclesChanged();
	planesChanged();
	unitsChanged();
}

// cStaticMap //////////////////////////////////////////////////

cStaticMap::cStaticMap() : crc(0), size(0), terrains()
{
}

cStaticMap::~cStaticMap()
{
}

std::size_t cStaticMap::getTileIndex (const cPosition& position) const
{
	return Kacheln[getOffset (position)];
}

const sGraphicTile& cStaticMap::getGraphicTile (const cPosition& position) const
{
	return graphics[getTileIndex (position)];
}


const sTerrain& cStaticMap::getTerrain (const cPosition& position) const
{
	return terrains[getTileIndex (position)];
}

bool cStaticMap::isBlocked (const cPosition& position) const
{
	return getTerrain (position).blocked;
}

bool cStaticMap::isCoast (const cPosition& position) const
{
	return getTerrain (position).coast;
}

bool cStaticMap::isWater (const cPosition& position) const
{
	return getTerrain (position).water;
}

bool cStaticMap::isGround(const cPosition& position) const
{
	return !getTerrain(position).water && !getTerrain(position).coast;
}

bool cStaticMap::possiblePlace(const cStaticUnitData& data, const cPosition& position) const
{
	if (!isValidPosition(position)) return false;
	if (data.isBig)
	{
		if (!isValidPosition(position + cPosition(0, 1)) ||
			!isValidPosition(position + cPosition(1, 0)) ||
			!isValidPosition(position + cPosition(1, 1)) )
		{
			return false;
		}
	}

	if (data.factorAir > 0) return true;

	if (isBlocked(position)) return false;
	if (data.isBig)
	{
		if (isBlocked(position + cPosition(0, 1)) ||
			isBlocked(position + cPosition(1, 0)) ||
			isBlocked(position + cPosition(1, 1)) )
		{
			return false;
		}
	}

	if (data.factorSea == 0)
	{
		if (isWater(position)) return false;
		if (data.isBig)
		{
			if (isWater(position + cPosition(0, 1)) ||
				isWater(position + cPosition(1, 0)) ||
				isWater(position + cPosition(1, 1)) )
			{
				return false;
			}
		}
	}

	if (data.factorCoast == 0)
	{
		if (isCoast(position)) return false;
		if (data.isBig)
		{
			if (isCoast(position + cPosition(0, 1)) ||
				isCoast(position + cPosition(1, 0)) ||
				isCoast(position + cPosition(1, 1)) )
			{
				return false;
			}
		}
	}

	if (data.factorGround == 0)
	{
		if (isGround(position)) return false;
		if (data.isBig)
		{
			if (isGround(position + cPosition(0, 1)) ||
				isGround(position + cPosition(1, 0)) ||
				isGround(position + cPosition(1, 1)) )
			{
				return false;
			}
		}
	}

	return true;
}

bool cStaticMap::isValidPosition (const cPosition& position) const
{
	return 0 <= position.x() && position.x() < size && 0 <= position.y() && position.y() < size;
}

void cStaticMap::clear()
{
	filename.clear();
	size = 0;
	crc = 0;
	terrains.clear();
	Kacheln.clear();
}

bool cStaticMap::isValid() const
{
	return !filename.empty();
}

/** Loads a map file */
bool cStaticMap::loadMap (const std::string& filename_)
{
	clear();
	// Open File
	filename = filename_;
	Log.write ("Loading map \"" + filename_ + "\"", cLog::eLOG_TYPE_DEBUG);

	// first try in the factory maps directory
	std::string fullFilename = cSettings::getInstance().getMapsPath() + PATH_DELIMITER + filename;
	SDL_RWops* fpMapFile = SDL_RWFromFile (fullFilename.c_str(), "rb");
	if (fpMapFile == nullptr)
	{
		// now try in the user's map directory
		std::string userMapsDir = getUserMapsDir();
		if (!userMapsDir.empty())
		{
			fullFilename = userMapsDir + filename;
			fpMapFile = SDL_RWFromFile (fullFilename.c_str(), "rb");
		}
	}
	if (fpMapFile == nullptr)
	{
		Log.write ("Cannot load map file: \"" + filename + "\"", cLog::eLOG_TYPE_WARNING);
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
		Log.write ("Wrong file format: \"" + filename + "\"", cLog::eLOG_TYPE_WARNING);
		SDL_RWclose (fpMapFile);
		clear();
		return false;
	}
	SDL_RWseek (fpMapFile, 2, SEEK_CUR);

	// Read informations and get positions from the map-file
	const short sWidth = SDL_ReadLE16 (fpMapFile);
	Log.write ("SizeX: " + iToStr (sWidth), cLog::eLOG_TYPE_DEBUG);
	const short sHeight = SDL_ReadLE16 (fpMapFile);
	Log.write ("SizeY: " + iToStr (sHeight), cLog::eLOG_TYPE_DEBUG);
	SDL_RWseek (fpMapFile, sWidth * sHeight, SEEK_CUR); // Ignore Mini-Map
	const Sint64 iDataPos = SDL_RWtell (fpMapFile); // Map-Data
	SDL_RWseek (fpMapFile, sWidth * sHeight * 2, SEEK_CUR);
	const int iNumberOfTerrains = SDL_ReadLE16 (fpMapFile); // Read PicCount
	Log.write ("Number of terrains: " + iToStr (iNumberOfTerrains), cLog::eLOG_TYPE_DEBUG);
	const Sint64 iGraphicsPos = SDL_RWtell (fpMapFile); // Terrain Graphics
	const Sint64 iPalettePos = iGraphicsPos + iNumberOfTerrains * 64 * 64; // Palette
	const Sint64 iInfoPos = iPalettePos + 256 * 3; // Special informations

	if (sWidth != sHeight)
	{
		Log.write ("Map must be quadratic!: \"" + filename + "\"", cLog::eLOG_TYPE_WARNING);
		SDL_RWclose (fpMapFile);
		clear();
		return false;
	}

	// Generate new Map
	this->size = std::max<int> (16, sWidth);
	Kacheln.resize (size * size, 0);
	terrains.resize(iNumberOfTerrains);

	// Load Color Palette
	SDL_RWseek (fpMapFile, iPalettePos, SEEK_SET);
	for (int i = 0; i < 256; i++)
	{
		SDL_RWread (fpMapFile, palette + i, 3, 1);
	}
	//generate palette for terrains with fog
	for (int i = 0; i < 256; i++)
	{
		palette_shw[i].r = (unsigned char) (palette[i].r * 0.6f);
		palette_shw[i].g = (unsigned char) (palette[i].g * 0.6f);
		palette_shw[i].b = (unsigned char) (palette[i].b * 0.6f);
		palette[i].a = 255;
		palette_shw[i].a = 255;
	}

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
				Log.write ("unknown terrain type " + iToStr (cByte) + " on tile " + iToStr (iNum) + " found. Handled as blocked!", cLog::eLOG_TYPE_WARNING);
				terrains[iNum].blocked = true;
				//SDL_RWclose (fpMapFile);
				//return false;
		}

		//load terrain graphic
		AutoSurface surface (cStaticMap::loadTerrGraph (fpMapFile, iGraphicsPos, palette, iNum));
		if (surface == nullptr)
		{
			Log.write ("EOF while loading terrain number " + iToStr (iNum), cLog::eLOG_TYPE_WARNING);
			SDL_RWclose (fpMapFile);
			clear();
			return false;
		}
		graphics[iNum].copySrfToTerData (*surface, palette_shw);
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
				Log.write ("a map field referred to a nonexisting terrain: " + iToStr (Kachel), cLog::eLOG_TYPE_WARNING);
				SDL_RWclose (fpMapFile);
				clear();
				return false;
			}
			Kacheln[iY * size + iX] = Kachel;
		}
	}
	SDL_RWclose (fpMapFile);

	//save crc, to check map file equality when loading a game
	crc = MapDownload::calculateCheckSum(filename);
	return true;
}

/*static*/AutoSurface cStaticMap::loadTerrGraph (SDL_RWops* fpMapFile, Sint64 iGraphicsPos, const SDL_Color (&colors)[256], int iNum)
{
	// Create new surface and copy palette
	AutoSurface surface (SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0));
	surface->pitch = surface->w;

	SDL_SetPaletteColors (surface->format->palette, colors, 0, 256);

	// Go to position of filedata
	SDL_RWseek (fpMapFile, iGraphicsPos + 64 * 64 * (iNum), SEEK_SET);

	// Read pixel data and write to surface
	if (SDL_RWread (fpMapFile, surface->pixels, 1, 64 * 64) != 64 * 64) return 0;
	return surface;
}

uint32_t cStaticMap::getChecksum(uint32_t crc)
{
	return calcCheckSum(this->crc, crc);
}

void cStaticMap::generateNextAnimationFrame()
{
	//change palettes to display next frame
	SDL_Color temp = palette[127];
	memmove (palette + 97, palette + 96, 32 * sizeof (SDL_Color));
	palette[96]  = palette[103];
	palette[103] = palette[110];
	palette[110] = palette[117];
	palette[117] = palette[123];
	palette[123] = temp;

	temp = palette_shw[127];
	memmove (palette_shw + 97, palette_shw + 96, 32 * sizeof (SDL_Color));
	palette_shw[96]  = palette_shw[103];
	palette_shw[103] = palette_shw[110];
	palette_shw[110] = palette_shw[117];
	palette_shw[117] = palette_shw[123];
	palette_shw[123] = temp;

	//set the new palette for all terrain surfaces
	for (auto& terrain : graphics)
	{
		SDL_SetColors (terrain.sf.get(), palette + 96, 96, 127);
		//SDL_SetColors (TerrainInUse[i]->sf_org, palette + 96, 96, 127);
		SDL_SetColors (terrain.shw.get(), palette_shw + 96, 96, 127);
		//SDL_SetColors (TerrainInUse[i]->shw_org, palette_shw + 96, 96, 127);
	}
}

AutoSurface cStaticMap::createBigSurface (int sizex, int sizey) const
{
	AutoSurface mapSurface (SDL_CreateRGBSurface (0, sizex, sizey, Video.getColDepth(), 0, 0, 0, 0));

	if (SDL_MUSTLOCK (mapSurface.get())) SDL_LockSurface (mapSurface.get());
	for (int x = 0; x < mapSurface->w; ++x)
	{
		const int terrainx = std::min ((x * size) / mapSurface->w, size - 1);
		const int offsetx = ((x * size) % mapSurface->w) * 64 / mapSurface->w;

		for (int y = 0; y < mapSurface->h; y++)
		{
			const int terrainy = std::min ((y * size) / mapSurface->h, size - 1);
			const int offsety = ((y * size) % mapSurface->h) * 64 / mapSurface->h;

			const sGraphicTile& t = getGraphicTile (cPosition (terrainx, terrainy));
			unsigned int ColorNr = * (static_cast<const unsigned char*> (t.sf_org->pixels) + (offsetx + offsety * 64));

			unsigned char* pixel = reinterpret_cast<unsigned char*> (&static_cast<Uint32*> (mapSurface->pixels) [x + y * mapSurface->w]);
			pixel[0] = palette[ColorNr].b;
			pixel[1] = palette[ColorNr].g;
			pixel[2] = palette[ColorNr].r;
		}
	}
	if (SDL_MUSTLOCK (mapSurface.get())) SDL_UnlockSurface (mapSurface.get());
	return mapSurface;
}

// Funktionen der Map-Klasse /////////////////////////////////////////////////
cMap::cMap (std::shared_ptr<cStaticMap> staticMap_) :
	staticMap (std::move (staticMap_)),
	fields(nullptr)
{
	init();

}

void cMap::init()
{
	const std::size_t size = staticMap->getSize().x() * staticMap->getSize().y();
	if (Resources.size() != size)
	{
		Resources.resize(size, sResources());
		delete[] fields;
		fields = new cMapField[size];
	}
}

cMap::~cMap()
{
	delete [] fields;
}

cMapField& cMap::operator[] (unsigned int offset) const
{
	return fields[offset];
}

cMapField& cMap::getField (const cPosition& position)
{
	return fields[getOffset (position)];
}

const cMapField& cMap::getField (const cPosition& position) const
{
	return fields[getOffset (position)];
}

bool cMap::isWaterOrCoast (const cPosition& position) const
{
	const sTerrain& terrainType = staticMap->getTerrain (position);
	return terrainType.water | terrainType.coast;
}

//--------------------------------------------------------------------------
std::string cMap::resourcesToString() const
{
	std::string str;
	str.reserve (4 * Resources.size() + 1);
	for (size_t i = 0; i != Resources.size(); ++i)
	{
		str += getHexValue (static_cast<int>(Resources[i].typ));
		str += getHexValue (Resources[i].value);
	}
	return str;
}

//--------------------------------------------------------------------------
void cMap::setResourcesFromString (const std::string& str)
{
	for (size_t i = 0; i != Resources.size(); ++i)
	{
		sResources res;
		res.typ   = static_cast<eResourceType>(getByteValue(str, 4 * i));
		res.value = getByteValue(str, 4 * i + 2);

		Resources.set(i, res);
	}
}

void cMap::placeResources(cModel& model)
{
	const auto& playerList = model.getPlayerList();
	const auto& gameSettings = *model.getGameSettings();

	Resources.fill(sResources());

	std::vector<eResourceType> resSpotTypes(playerList.size(), eResourceType::Metal);
	std::vector<T_2<int>> resSpots;
	for (const auto& player : playerList)
	{
		const auto& position = player->getLandingPos();
		resSpots.push_back(T_2<int> ((position.x() & ~1) + 1, position.y() & ~1));
	}

	const eGameSettingsResourceDensity density = gameSettings.getResourceDensity();
	std::map<eResourceType, eGameSettingsResourceAmount> frequencies;

	frequencies[eResourceType::Metal] = gameSettings.getMetalAmount();
	frequencies[eResourceType::Oil] = gameSettings.getOilAmount();
	frequencies[eResourceType::Gold] = gameSettings.getGoldAmount();

	const std::size_t resSpotCount = (std::size_t) (getSize().x() * getSize().y() * 0.003f * (1.5f + getResourceDensityFactor (density)));
	const std::size_t playerCount = playerList.size();
	// create remaining resource positions
	while (resSpots.size() < resSpotCount)
	{
		T_2<int> pos;

		pos.x = 2 + model.randomGenerator.get (getSize().x() - 4);
		pos.y = 2 + model.randomGenerator.get (getSize().y() - 4);
		resSpots.push_back(pos);
	}
	resSpotTypes.resize (resSpotCount);
	// Resourcen gleichmässiger verteilen
	for (std::size_t j = 0; j < 3; j++)
	{
		for (std::size_t i = playerCount; i < resSpotCount; i++)
		{
			T_2<float> d;
			for (std::size_t j = 0; j < resSpotCount; j++)
			{
				if (i == j) continue;

				int diffx1 = resSpots[i].x - resSpots[j].x;
				int diffx2 = diffx1 + (getSize().x() - 4);
				int diffx3 = diffx1 - (getSize().x() - 4);
				int diffy1 = resSpots[i].y - resSpots[j].y;
				int diffy2 = diffy1 + (getSize().y() - 4);
				int diffy3 = diffy1 - (getSize().y() - 4);
				if (abs (diffx2) < abs (diffx1)) diffx1 = diffx2;
				if (abs (diffx3) < abs (diffx1)) diffx1 = diffx3;
				if (abs (diffy2) < abs (diffy1)) diffy1 = diffy2;
				if (abs (diffy3) < abs (diffy1)) diffy1 = diffy3;
				T_2<float> diff (diffx1, diffy1);
				if (diff == T_2<float>::Zero)
				{
					diff.x += 1;
				}
				const float dist = diff.dist();
				d += diff * (10.f / (dist * dist));

			}
			resSpots[i] += T_2<int> (Round (d.x), Round (d.y));
			if (resSpots[i].x < 2) resSpots[i].x += getSize().x() - 4;
			if (resSpots[i].y < 2) resSpots[i].y += getSize().y() - 4;
			if (resSpots[i].x > getSize().x() - 3) resSpots[i].x -= getSize().x() - 4;
			if (resSpots[i].y > getSize().y() - 3) resSpots[i].y -= getSize().y() - 4;

		}
	}
	// Resourcen Typ bestimmen
	for (std::size_t i = playerCount; i < resSpotCount; i++)
	{
		std::map<eResourceType, double> amount;
		for (std::size_t j = 0; j < i; j++)
		{
			const float maxDist = 40.f;
			float dist = sqrtf (resSpots[i].distSqr (resSpots[j]));
			if (dist < maxDist) amount[resSpotTypes[j]] += 1 - sqrtf (dist / maxDist);
		}

		amount[eResourceType::Metal] /= 1.0f;
		amount[eResourceType::Oil] /= 0.8f;
		amount[eResourceType::Gold] /= 0.4f;

		eResourceType type = eResourceType::Metal;
		if (amount[eResourceType::Oil] < amount[type]) type = eResourceType::Oil;
		if (amount[eResourceType::Gold] < amount[type]) type = eResourceType::Gold;

		resSpots[i].x &= ~1;
		resSpots[i].y &= ~1;
		resSpots[i].x += static_cast<int>(type) % 2;
		resSpots[i].y += (static_cast<int>(type) / 2) % 2;

		resSpotTypes[i] = static_cast<eResourceType>(((resSpots[i].y % 2) * 2) + (resSpots[i].x % 2));
	}
	// Resourcen platzieren
	for (std::size_t i = 0; i < resSpotCount; i++)
	{
		T_2<int> pos = resSpots[i];
		T_2<int> p;
		bool hasGold = model.randomGenerator.get (100) < 40;
		const int minx = std::max (pos.x - 1, 0);
		const int maxx = std::min (pos.x + 1, getSize().x() - 1);
		const int miny = std::max (pos.y - 1, 0);
		const int maxy = std::min (pos.y + 1, getSize().y() - 1);
		for (p.y = miny; p.y <= maxy; ++p.y)
		{
			for (p.x = minx; p.x <= maxx; ++p.x)
			{
				T_2<int> absPos = p;
				eResourceType type = static_cast<eResourceType>((absPos.y % 2) * 2 + (absPos.x % 2));

				int index = getOffset (cPosition (absPos.x, absPos.y));
				if (type != eResourceType::None &&
					((hasGold && i >= playerCount) || resSpotTypes[i] == eResourceType::Gold || type != eResourceType::Gold) &&
					!isBlocked (cPosition (absPos.x, absPos.y)))
				{
					sResources res;
					res.typ = type;
					if (i >= playerCount)
					{
						res.value = 1 + model.randomGenerator.get (2 + getResourceAmountFactor (frequencies[type]) * 2);
						if (p == pos) res.value += 3 + model.randomGenerator.get(4 + getResourceAmountFactor(frequencies[type]) * 2);
					}
					else
					{
						res.value = 1 + 4 + getResourceAmountFactor (frequencies[type]);
						if (p == pos) res.value += 3 + 2 + getResourceAmountFactor (frequencies[type]);
					}
					res.value = std::min<unsigned char> (16, res.value);
					Resources.set(index, res);
				}
			}
		}
	}
}

/* static */ int cMap::getMapLevel (const cBuilding& building)
{
	const cStaticUnitData& data = building.getStaticUnitData();

	if (building.isRubble()) return 4;  // rubble

	if (data.surfacePosition == cStaticUnitData::SURFACE_POS_BENEATH_SEA) return 9; // seamine
	if (data.surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE_SEA) return 7; // bridge
	if (data.surfacePosition == cStaticUnitData::SURFACE_POS_BASE && data.canBeOverbuild) return 6; // platform
	if (data.surfacePosition == cStaticUnitData::SURFACE_POS_BASE) return 5; // road

	if (data.surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE_BASE) return 3; // landmine

	return 1; // other buildings
}

/* static */ int cMap::getMapLevel (const cVehicle& vehicle)
{
	if (vehicle.getStaticUnitData().factorSea > 0 && vehicle.getStaticUnitData().factorGround == 0) return 8; // ships
	if (vehicle.getStaticUnitData().factorAir > 0) return 0; // planes

	return 2; // other vehicles
}

void cMap::addBuilding (cBuilding& building, const cPosition& position)
{
	//big base building are not implemented
	if (building.getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_GROUND && building.getIsBig() && !building.isRubble()) return;

	const int mapLevel = cMap::getMapLevel (building);
	size_t i = 0;

	if (building.getIsBig())
	{
		auto& field = getField (position);
		i = 0;
		while (i < field.getBuildings().size() && cMap::getMapLevel (*field.getBuildings()[i]) < mapLevel) i++;
		field.addBuilding (building, i);

		auto& fieldEast = getField (position + cPosition (1, 0));
		i = 0;
		while (i < fieldEast.getBuildings().size() && cMap::getMapLevel (*fieldEast.getBuildings()[i]) < mapLevel) i++;
		fieldEast.addBuilding (building, i);

		auto& fieldSouth = getField (position + cPosition (0, 1));
		i = 0;
		while (i < fieldSouth.getBuildings().size() && cMap::getMapLevel (*fieldSouth.getBuildings()[i]) < mapLevel) i++;
		fieldSouth.addBuilding (building, i);

		auto& fieldSouthEast = getField (position + cPosition (1, 1));
		i = 0;
		while (i < fieldSouthEast.getBuildings().size() && cMap::getMapLevel (*fieldSouthEast.getBuildings()[i]) < mapLevel) i++;
		fieldSouthEast.addBuilding (building, i);
	}
	else
	{
		auto& field = getField (position);

		while (i < field.getBuildings().size() && cMap::getMapLevel (*field.getBuildings()[i]) < mapLevel) i++;
		field.addBuilding (building, i);
	}
	addedUnit (building);
}

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
		vehicle.setIsBig (false);
		moveVehicleBig(vehicle, position);
	}
	addedUnit (vehicle);
}

void cMap::deleteBuilding (const cBuilding& building)
{
	getField (building.getPosition()).removeBuilding (building);

	if (building.getIsBig())
	{
		getField (building.getPosition() + cPosition (1, 0)).removeBuilding (building);
		getField (building.getPosition() + cPosition (1, 1)).removeBuilding (building);
		getField (building.getPosition() + cPosition (0, 1)).removeBuilding (building);
	}
	removedUnit (building);
}

void cMap::deleteVehicle (const cVehicle& vehicle)
{
	if (vehicle.getStaticUnitData().factorAir > 0)
	{
		getField (vehicle.getPosition()).removePlane (vehicle);
	}
	else
	{
		getField (vehicle.getPosition()).removeVehicle (vehicle);

		if (vehicle.getIsBig())
		{
			getField (vehicle.getPosition() + cPosition (1, 0)).removeVehicle (vehicle);
			getField (vehicle.getPosition() + cPosition (1, 1)).removeVehicle (vehicle);
			getField (vehicle.getPosition() + cPosition (0, 1)).removeVehicle (vehicle);
		}
	}
	removedUnit (vehicle);
}

void cMap::deleteUnit (const cUnit& unit)
{
	if (unit.isABuilding())
	{
		deleteBuilding (static_cast<const cBuilding&> (unit));
	}
	else
	{
		assert (unit.isAVehicle());
		deleteVehicle (static_cast<const cVehicle&> (unit));
	}
}

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
		getField (oldPosition).removeVehicle (vehicle);

		//check, whether the vehicle is centered on 4 map fields
		if (vehicle.getIsBig())
		{
			getField (oldPosition + cPosition (1, 0)).removeVehicle (vehicle);
			getField (oldPosition + cPosition (1, 1)).removeVehicle (vehicle);
			getField (oldPosition + cPosition (0, 1)).removeVehicle (vehicle);

			vehicle.setIsBig(false);
		}

		getField (position).addVehicle (vehicle, 0);
	}
	movedVehicle (vehicle, oldPosition);
}

void cMap::moveVehicleBig (cVehicle& vehicle, const cPosition& position)
{
	if (vehicle.getIsBig())
	{
		Log.write ("Calling moveVehicleBig on a big vehicle", cLog::eLOG_TYPE_NET_ERROR);
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

	vehicle.setIsBig(true);

	movedVehicle (vehicle, oldPosition);
}

bool cMap::possiblePlace (const cVehicle& vehicle, const cPosition& position, bool checkPlayer, bool ignoreMovingVehicles) const
{
	return possiblePlaceVehicle (vehicle.getStaticUnitData(), position, checkPlayer ? vehicle.getOwner() : nullptr, ignoreMovingVehicles);
}

bool cMap::possiblePlaceVehicle (const cStaticUnitData& vehicleData, const cPosition& position, const cPlayer* player, bool ignoreMovingVehicles) const
{
	if (isValidPosition (position) == false) return false;
	const auto field = cMapFieldView(getField(position), staticMap->getTerrain(position), player);

	const std::vector<cBuilding*> buildings = field.getBuildings();
	std::vector<cBuilding*>::const_iterator b_it = buildings.begin();
	std::vector<cBuilding*>::const_iterator b_end = buildings.end();

	//search first building, that is not a connector
	if (b_it != b_end && (*b_it)->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE) ++b_it;

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
			int notMovingPlanes = 0;
			for (const auto& plane : planes)
			{
				if (!plane->isUnitMoving())
				{
					notMovingPlanes++;
				}
			}
			if (notMovingPlanes >= MAX_PLANES_PER_FIELD) return false;
		}
	}
	if (vehicleData.factorGround > 0)
	{
		if (isBlocked (position)) return false;

		if ((isWater (position) && vehicleData.factorSea == 0) ||
			(isCoast (position) && vehicleData.factorCoast == 0))
		{
			if (player && !player->canSeeAt (position)) return false;

			//vehicle can drive on water, if there is a bridge, platform or road
			if (b_it == b_end) return false;
			if ((*b_it)->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE_SEA &&
				(*b_it)->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_BASE &&
				(*b_it)->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE_BASE) return false;
		}
		//check for enemy mines
		if (player &&
			b_it != b_end &&
			(*b_it)->getOwner() != player &&
			(*b_it)->getStaticUnitData().explodesOnContact &&
			(*b_it)->isDetectedByPlayer(player))
		{
			return false;
		}

		if (player && !player->canSeeAt (position)) return true;

		if (field.getVehicle() && (!ignoreMovingVehicles || !field.getVehicle()->isUnitMoving()))
		{
			return false;
		}
		if (b_it != b_end)
		{
			// only base buildings and rubble is allowed on the same field with a vehicle
			// (connectors have been skiped, so doesn't matter here)
			if ((*b_it)->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE_SEA &&
				(*b_it)->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_BASE &&
				(*b_it)->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE_BASE &&
				(*b_it)->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_BENEATH_SEA &&
				!(*b_it)->isRubble()) return false;
		}
	}
	else if (vehicleData.factorSea > 0)
	{
		if (isBlocked (position)) return false;

		if (!isWater (position) &&
			(!isCoast (position) || vehicleData.factorCoast == 0)) return false;

		//check for enemy mines
		if (player &&
			b_it != b_end &&
			(*b_it)->getOwner() != player &&
			(*b_it)->getStaticUnitData().explodesOnContact &&
			(*b_it)->isDetectedByPlayer(player))
		{
			return false;
		}

		if (player && !player->canSeeAt (position)) return true;

		if (field.getVehicle() && (!ignoreMovingVehicles || !field.getVehicle()->isUnitMoving()))
		{
			return false;
		}

		//only bridge and sea mine are allowed on the same field with a ship (connectors have been skiped, so doesn't matter here)
		if (b_it != b_end &&
			(*b_it)->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE_SEA &&
			(*b_it)->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_BENEATH_SEA)
		{
			// if the building is a landmine, we have to check whether it's on a bridge or not
			if ((*b_it)->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE_BASE)
			{
				++b_it;
				if (b_it == b_end || (*b_it)->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE_SEA) return false;
			}
			else return false;
		}
	}
	return true;
}

bool cMap::possiblePlaceBuilding (const cStaticUnitData& buildingData, const cPosition& position, const cPlayer* player, const cVehicle* vehicle) const
{
	if (!isValidPosition (position)) return false;
	if (isBlocked (position)) return false;
	const auto field = cMapFieldView(getField(position), staticMap->getTerrain(position), player);

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
		if (buildingData.surfacePosition == building->getStaticUnitData().surfacePosition &&
			building->getStaticUnitData().canBeOverbuild == cStaticUnitData::OVERBUILD_TYPE_NO) return false;
		switch (building->getStaticUnitData().surfacePosition)
		{
			case cStaticUnitData::SURFACE_POS_GROUND:
			case cStaticUnitData::SURFACE_POS_ABOVE_SEA: // bridge
				if (buildingData.surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE &&
					buildingData.surfacePosition != cStaticUnitData::SURFACE_POS_BASE && // mine can be placed on bridge
					buildingData.surfacePosition != cStaticUnitData::SURFACE_POS_BENEATH_SEA && // seamine can be placed under bridge
					building->getStaticUnitData().canBeOverbuild == cStaticUnitData::OVERBUILD_TYPE_NO) return false;
				break;
			case cStaticUnitData::SURFACE_POS_BENEATH_SEA: // seamine
			case cStaticUnitData::SURFACE_POS_ABOVE_BASE:  // landmine
				// mine must be removed first
				if (buildingData.surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE) return false;
				break;
			case cStaticUnitData::SURFACE_POS_BASE: // platform, road
				water = coast = false;
				ground = true;
				break;
			case cStaticUnitData::SURFACE_POS_ABOVE: // connector
				break;
		}
	}
	if ((water && buildingData.factorSea == 0) ||
		(coast && buildingData.factorCoast == 0) ||
		(ground && buildingData.factorGround == 0)) return false;

	//can not build on rubble
	if (field.getRubble() &&
		buildingData.surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE &&
		buildingData.surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE_BASE) return false;

	if (field.getVehicle())
	{
		if (!vehicle) return false;
		if (vehicle != field.getVehicle()) return false;
	}
	return true;
}
void cMap::reset()
{
	for (int i = 0; i < getSize().x() * getSize().y(); i++)
	{
		fields[i].removeAll();
	}
}

uint32_t cMap::getChecksum(uint32_t crc) const
{
	crc = staticMap->getChecksum(crc);
	//cMapField* fields;
	crc = calcCheckSum(Resources, crc);

	return crc;
}

/*static*/ int cMap::getResourceDensityFactor (eGameSettingsResourceDensity density)
{
	switch (density)
	{
		case eGameSettingsResourceDensity::Sparse:
			return 0;
		case eGameSettingsResourceDensity::Normal:
			return 1;
		case eGameSettingsResourceDensity::Dense:
			return 2;
		case eGameSettingsResourceDensity::TooMuch:
			return 3;
	}
	assert (false);
	return 0;
}

/*static*/int cMap::getResourceAmountFactor (eGameSettingsResourceAmount amount)
{
	switch (amount)
	{
		case eGameSettingsResourceAmount::Limited:
			return 0;
		case eGameSettingsResourceAmount::Normal:
			return 1;
		case eGameSettingsResourceAmount::High:
			return 2;
		case eGameSettingsResourceAmount::TooMuch:
			return 3;
	}
	assert (false);
	return 0;
}

uint32_t sResources::getChecksum(uint32_t crc) const
{
	crc = calcCheckSum(value, crc);
	crc = calcCheckSum(typ, crc);

	return crc;
}
