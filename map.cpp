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

#include "buildings.h"
#include "clist.h"
#include "files.h"
#include "log.h"
#include "player.h"
#include "settings.h"
#include "vehicles.h"
#include "video.h"

#if 1 // TODO: [SDL2]: SDL_SetColors
inline void SDL_SetColors (SDL_Surface* surface, SDL_Color* colors, int index, int size)
{
	SDL_SetPaletteColors (surface->format->palette, colors, index, size);
}
#endif


sTerrain::sTerrain() :
	water (false),
	coast (false),
	blocked (false)
{}

cVehicle* cMapField::getVehicle()
{
	if (vehicles.empty()) return NULL;
	return vehicles[0];
}

cVehicle* cMapField::getPlane()
{
	if (planes.empty()) return NULL;
	return planes[0];
}

std::vector<cVehicle*>& cMapField::getPlanes()
{
	return planes;
}

std::vector<cBuilding*>& cMapField::getBuildings()
{
	return buildings;
}

cBuilding* cMapField::getBuilding()
{
	if (buildings.empty()) return NULL;
	return buildings[0];
}

cBuilding* cMapField::getTopBuilding()
{
	if (buildings.empty()) return NULL;
	cBuilding* building = *buildings.begin();

	if ((building->data.surfacePosition == sUnitData::SURFACE_POS_GROUND ||
		 building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) &&
		building->owner)
		return building;
	return NULL;
}

cBuilding* cMapField::getBaseBuilding()
{
	for (size_t i = 0; i != buildings.size(); ++i)
	{
		cBuilding* building = buildings[i];
		if (building->data.surfacePosition != sUnitData::SURFACE_POS_GROUND &&
			building->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE &&
			building->owner)
		{
			return building;
		}
	}
	return NULL;
}

cBuilding* cMapField::getRubble()
{
	for (size_t i = 0; i != buildings.size(); ++i)
		if (!buildings[i]->owner)
			return buildings[i];
	return NULL;
}

cBuilding* cMapField::getMine()
{
	for (size_t i = 0; i != buildings.size(); ++i)
		if (buildings[i]->data.explodesOnContact)
			return buildings[i];
	return NULL;
}

// cStaticMap //////////////////////////////////////////////////

cStaticMap::cStaticMap() : size (0), terrainCount (0), terrains (NULL)
{
}

cStaticMap::~cStaticMap()
{
	delete [] terrains;
}

const sTerrain& cStaticMap::getTerrain (int offset) const
{
	return terrains[Kacheln[offset]];
}

const sTerrain& cStaticMap::getTerrain (int x, int y) const
{
	return getTerrain (getOffset (x, y));
}

bool cStaticMap::isBlocked (int offset) const
{
	return terrains[Kacheln[offset]].blocked;
}

bool cStaticMap::isCoast (int offset) const
{
	return terrains[Kacheln[offset]].coast;
}

bool cStaticMap::isWater (int offset) const
{
	return terrains[Kacheln[offset]].water;
}

bool cStaticMap::isValidPos (int x, int y) const
{
	return 0 <= x && x < size && 0 <= y && y < size;
}

bool cStaticMap::isWater (int x, int y) const
{
	return isWater (getOffset (x, y));
}

void cStaticMap::clear()
{
	filename.clear();
	size = 0;
	delete [] terrains;
	terrains = NULL;
	terrainCount = 0;
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
	if (fpMapFile == NULL)
	{
		// now try in the user's map directory
		std::string userMapsDir = getUserMapsDir();
		if (!userMapsDir.empty())
		{
			fullFilename = userMapsDir + filename;
			fpMapFile = SDL_RWFromFile (fullFilename.c_str(), "rb");
		}
	}
	if (fpMapFile == NULL)
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
	const int iDataPos = SDL_RWtell (fpMapFile); // Map-Data
	SDL_RWseek (fpMapFile, sWidth * sHeight * 2, SEEK_CUR);
	const int iNumberOfTerrains = SDL_ReadLE16 (fpMapFile); // Read PicCount
	Log.write ("Number of terrains: " + iToStr (iNumberOfTerrains), cLog::eLOG_TYPE_DEBUG);
	const int iGraphicsPos = SDL_RWtell (fpMapFile); // Terrain Graphics
	const int iPalettePos = iGraphicsPos + iNumberOfTerrains * 64 * 64; // Palette
	const int iInfoPos = iPalettePos + 256 * 3; // Special informations

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
	terrains = new sTerrain[iNumberOfTerrains];
	terrainCount = iNumberOfTerrains;

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
		if (surface == NULL)
		{
			Log.write ("EOF while loading terrain number " + iToStr (iNum), cLog::eLOG_TYPE_WARNING);
			SDL_RWclose (fpMapFile);
			clear();
			return false;
		}
		copySrfToTerData (*surface, iNum);
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
	return true;
}

/*static*/SDL_Surface* cStaticMap::loadTerrGraph (SDL_RWops* fpMapFile, int iGraphicsPos, const SDL_Color (&colors)[256], int iNum)
{
	// Create new surface and copy palette
	AutoSurface surface (SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0));
	surface->pitch = surface->w;

	SDL_SetPaletteColors (surface->format->palette, colors, 0, 256);

	// Go to position of filedata
	SDL_RWseek (fpMapFile, iGraphicsPos + 64 * 64 * (iNum), SEEK_SET);

	// Read pixel data and write to surface
	if (SDL_RWread (fpMapFile, surface->pixels, 1, 64 * 64) != 64 * 64) return 0;
	return surface.Release();
}

/*static*/SDL_Surface* cStaticMap::loadMapPreview (const std::string& mapName, int* mapSize)
{
	std::string mapPath = cSettings::getInstance().getMapsPath() + PATH_DELIMITER + mapName;
	// if no factory map of that name exists, try the custom user maps

	SDL_RWops* mapFile = SDL_RWFromFile (mapPath.c_str(), "rb");
	if (mapFile == NULL && !getUserMapsDir().empty())
	{
		mapPath = getUserMapsDir() + mapName;
		mapFile = SDL_RWFromFile (mapPath.c_str(), "rb");
	}

	if (mapFile == NULL) return NULL;

	SDL_RWseek (mapFile, 5, SEEK_SET);
	int size = SDL_ReadLE16 (mapFile);
	struct { unsigned char cBlue, cGreen, cRed; } Palette[256];
	short sGraphCount;
	SDL_RWseek (mapFile, 2 + size * size * 3, SEEK_CUR);
	sGraphCount = SDL_ReadLE16 (mapFile);
	SDL_RWseek (mapFile, 64 * 64 * sGraphCount, SEEK_CUR);
	SDL_RWread (mapFile, &Palette, 3, 256);

	AutoSurface mapSurface (SDL_CreateRGBSurface (0, size, size, 8, 0, 0, 0, 0));
	mapSurface->pitch = mapSurface->w;

	mapSurface->format->palette->ncolors = 256;
	for (int j = 0; j < 256; j++)
	{
		mapSurface->format->palette->colors[j].r = Palette[j].cBlue;
		mapSurface->format->palette->colors[j].g = Palette[j].cGreen;
		mapSurface->format->palette->colors[j].b = Palette[j].cRed;
	}
	SDL_RWseek (mapFile, 9, SEEK_SET);
	const int byteReadCount = SDL_RWread (mapFile, mapSurface->pixels, 1, size * size);
	SDL_RWclose (mapFile);

	if (byteReadCount != size * size)
	{
		// error.
		return NULL;
	}
	const int MAPWINSIZE = 112;
	if (mapSurface->w != MAPWINSIZE || mapSurface->h != MAPWINSIZE) // resize map
	{
		mapSurface = scaleSurface (mapSurface.get(), NULL, MAPWINSIZE, MAPWINSIZE);
	}

	if (mapSize != NULL) *mapSize = size;
	return mapSurface.Release();
}

void cStaticMap::copySrfToTerData (SDL_Surface& surface, int iNum)
{
	//before the surfaces are copied, the colortable of both surfaces has to be equal
	//This is needed to make sure, that the pixeldata is copied 1:1

	//copy the normal terrains
	terrains[iNum].sf_org = SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0);
	SDL_SetPaletteColors (terrains[iNum].sf_org->format->palette, surface.format->palette->colors, 0, 256);
	SDL_BlitSurface (&surface, NULL, terrains[iNum].sf_org.get(), NULL);

	terrains[iNum].sf = SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0);
	SDL_SetPaletteColors (terrains[iNum].sf->format->palette, surface.format->palette->colors, 0, 256);
	SDL_BlitSurface (&surface, NULL, terrains[iNum].sf.get(), NULL);

	//copy the terrains with fog
	terrains[iNum].shw_org = SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0);
	SDL_SetColors (terrains[iNum].shw_org.get(), surface.format->palette->colors, 0, 256);
	SDL_BlitSurface (&surface, NULL, terrains[iNum].shw_org.get(), NULL);

	terrains[iNum].shw = SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0);
	SDL_SetColors (terrains[iNum].shw.get(), surface.format->palette->colors, 0, 256);
	SDL_BlitSurface (&surface, NULL, terrains[iNum].shw.get(), NULL);

	//now set the palette for the fog terrains
	SDL_SetColors (terrains[iNum].shw_org.get(), palette_shw, 0, 256);
	SDL_SetColors (terrains[iNum].shw.get(), palette_shw, 0, 256);
}

void cStaticMap::scaleSurfaces (int pixelSize)
{
	for (size_t i = 0; i != terrainCount; ++i)
	{
		sTerrain& t = terrains[i];
		scaleSurface (t.sf_org.get(), t.sf.get(), pixelSize, pixelSize);
		scaleSurface (t.shw_org.get(), t.shw.get(), pixelSize, pixelSize);
	}
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
	for (size_t i = 0; i != terrainCount; ++i)
	{
		SDL_SetColors (terrains[i].sf.get(), palette + 96, 96, 127);
		//SDL_SetColors (TerrainInUse[i]->sf_org.get(), palette + 96, 96, 127);
		SDL_SetColors (terrains[i].shw.get(), palette_shw + 96, 96, 127);
		//SDL_SetColors (TerrainInUse[i]->shw_org.get(), palette_shw + 96, 96, 127);
	}
}

SDL_Surface* cStaticMap::createBigSurface (int sizex, int sizey) const
{
	SDL_Surface* mapSurface = SDL_CreateRGBSurface (0, sizex, sizey, Video.getColDepth(), 0, 0, 0, 0);

	if (SDL_MUSTLOCK (mapSurface)) SDL_LockSurface (mapSurface);
	for (int x = 0; x < mapSurface->w; ++x)
	{
		const int terrainx = std::min ((x * size) / mapSurface->w, size - 1);
		const int offsetx = ((x * size) % mapSurface->w) * 64 / mapSurface->w;

		for (int y = 0; y < mapSurface->h; y++)
		{
			const int terrainy = std::min ((y * size) / mapSurface->h, size - 1);
			const int offsety = ((y * size) % mapSurface->h) * 64 / mapSurface->h;

			const sTerrain& t = this->getTerrain (terrainx, terrainy);
			unsigned int ColorNr = * (static_cast<const unsigned char*> (t.sf_org->pixels) + (offsetx + offsety * 64));

			unsigned char* pixel = reinterpret_cast<unsigned char*> (&static_cast<Uint32*> (mapSurface->pixels) [x + y * mapSurface->w]);
			pixel[0] = palette[ColorNr].b;
			pixel[1] = palette[ColorNr].g;
			pixel[2] = palette[ColorNr].r;
		}
	}
	if (SDL_MUSTLOCK (mapSurface)) SDL_UnlockSurface (mapSurface);
	return mapSurface;
}

// Funktionen der Map-Klasse /////////////////////////////////////////////////
cMap::cMap (cStaticMap& staticMap_) :
	staticMap (&staticMap_)
{
	const int size = staticMap->getSize();
	fields = new cMapField[size * size];
	Resources.resize (size * size);

	resSpots = NULL;
	resSpotTypes = NULL;
	resSpotCount = 0;
	resCurrentSpotCount = 0;
}

cMap::~cMap()
{
	delete [] fields;
}

cMapField& cMap::operator[] (unsigned int offset) const
{
	return fields[offset];
}

bool cMap::isWaterOrCoast (int x, int y) const
{
	const sTerrain& terrainType = staticMap->getTerrain (x, y);
	return terrainType.water | terrainType.coast;
}

bool cMap::isValidOffset (int offset) const
{
	return 0 <= offset && offset < getSize() * getSize();
}

// Platziert die Ressourcen für einen Spieler.
void cMap::placeRessourcesAddPlayer (int x, int y, int frequency)
{
	if (resSpots == NULL)
	{
		resSpotCount = (int) (getSize() * getSize() * 0.003f * (1.5f + frequency));
		resCurrentSpotCount = 0;
		resSpots = new T_2<int>[resSpotCount];
		resSpotTypes = new int[resSpotCount];
	}
	resSpotTypes[resCurrentSpotCount] = RES_METAL;
	resSpots[resCurrentSpotCount] = T_2<int> ((x & ~1) + (RES_METAL % 2), (y & ~1) + ((RES_METAL / 2) % 2));
	resCurrentSpotCount++;
}

void cMap::assignRessources (const cMap& rhs)
{
	Resources = rhs.Resources;
}

//--------------------------------------------------------------------------
static std::string getHexValue (unsigned char byte)
{
	std::string str = "";
	const char hexChars[] = "0123456789ABCDEF";
	const unsigned char high = (byte >> 4) & 0x0F;
	const unsigned char low = byte & 0x0F;

	str += hexChars[high];
	str += hexChars[low];
	return str;
}

//--------------------------------------------------------------------------
std::string cMap::resourcesToString() const
{
	std::string str;
	str.reserve (4 * Resources.size() + 1);
	for (size_t i = 0; i != Resources.size(); ++i)
	{
		str += getHexValue (Resources[i].typ);
		str += getHexValue (Resources[i].value);
	}
	return str;
}

//--------------------------------------------------------------------------
static unsigned char getByteValue (const std::string& str, int index)
{
	unsigned char first = str[index + 0] - '0';
	unsigned char second = str[index + 1] - '0';

	if (first >= 'A' - '0') first -= 'A' - '0' - 10;
	if (second >= 'A' - '0') second -= 'A' - '0' - 10;
	return (first * 16 + second);
}

//--------------------------------------------------------------------------
void cMap::setResourcesFromString (const std::string& str)
{
	for (size_t i = 0; i != Resources.size(); ++i)
	{
		Resources[i].typ = getByteValue (str, 4 * i);
		Resources[i].value = getByteValue (str, 4 * i + 2);
	}
}

// Platziert die Ressourcen (0-wenig,3-viel):
void cMap::placeRessources (int metal, int oil, int gold)
{
	std::fill (Resources.begin(), Resources.end(), sResources());

	int frequencies[RES_COUNT];

	frequencies[RES_METAL] = metal;
	frequencies[RES_OIL] = oil;
	frequencies[RES_GOLD] = gold;

	int playerCount = resCurrentSpotCount;
	// create remaining resource positions
	while (resCurrentSpotCount < resSpotCount)
	{
		T_2<int> pos;

		pos.x = 2 + random (getSize() - 4);
		pos.y = 2 + random (getSize() - 4);
		resSpots[resCurrentSpotCount] = pos;
		resCurrentSpotCount++;
	}
	// Resourcen gleichmässiger verteilen
	for (int j = 0; j < 3; j++)
	{
		for (int i = playerCount; i < resSpotCount; i++)
		{
			T_2<float> d;
			for (int j = 0; j < resSpotCount; j++)
			{
				if (i == j) continue;

				int diffx1 = resSpots[i].x - resSpots[j].x;
				int diffx2 = diffx1 + (getSize() - 4);
				int diffx3 = diffx1 - (getSize() - 4);
				int diffy1 = resSpots[i].y - resSpots[j].y;
				int diffy2 = diffy1 + (getSize() - 4);
				int diffy3 = diffy1 - (getSize() - 4);
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
			if (resSpots[i].x < 2) resSpots[i].x += getSize() - 4;
			if (resSpots[i].y < 2) resSpots[i].y += getSize() - 4;
			if (resSpots[i].x > getSize() - 3) resSpots[i].x -= getSize() - 4;
			if (resSpots[i].y > getSize() - 3) resSpots[i].y -= getSize() - 4;

		}
	}
	// Resourcen Typ bestimmen
	for (int i = playerCount; i < resSpotCount; i++)
	{
		float amount[RES_COUNT] = {0.f, 0.f, 0.f, 0.f};
		for (int j = 0; j < i; j++)
		{
			const float maxDist = 40.f;
			float dist = sqrtf (resSpots[i].distSqr (resSpots[j]));
			if (dist < maxDist) amount[resSpotTypes[j]] += 1 - sqrtf (dist / maxDist);
		}

		amount[RES_METAL] /= 1.0f;
		amount[RES_OIL] /= 0.8f;
		amount[RES_GOLD] /= 0.4f;

		int type = RES_METAL;
		if (amount[RES_OIL] < amount[type]) type = RES_OIL;
		if (amount[RES_GOLD] < amount[type]) type = RES_GOLD;

		resSpots[i].x &= ~1;
		resSpots[i].y &= ~1;
		resSpots[i].x += type % 2;
		resSpots[i].y += (type / 2) % 2;

		resSpotTypes[i] = ((resSpots[i].y % 2) * 2) + (resSpots[i].x % 2);
	}
	// Resourcen platzieren
	for (int i = 0; i < resSpotCount; i++)
	{
		T_2<int> pos = resSpots[i];
		T_2<int> p;
		bool hasGold = random (100) < 40;
		const int minx = std::max (pos.x - 1, 0);
		const int maxx = std::min (pos.x + 1, getSize() - 1);
		const int miny = std::max (pos.y - 1, 0);
		const int maxy = std::min (pos.y + 1, getSize() - 1);
		for (p.y = miny; p.y <= maxy; ++p.y)
		{
			for (p.x = minx; p.x <= maxx; ++p.x)
			{
				T_2<int> absPos = p;
				int type = (absPos.y % 2) * 2 + (absPos.x % 2);

				int index = getOffset (absPos.x, absPos.y);
				if (type != RES_NONE &&
					((hasGold && i >= playerCount) || resSpotTypes[i] == RES_GOLD || type != RES_GOLD) &&
					!isBlocked (index))
				{
					Resources[index].typ = type;
					if (i >= playerCount)
					{
						Resources[index].value = 1 + random (2 + frequencies[type] * 2);
						if (p == pos) Resources[index].value += 3 + random (4 + frequencies[type] * 2);
					}
					else
					{
						Resources[index].value = 1 + 4 + frequencies[type];
						if (p == pos) Resources[index].value += 3 + 2 + frequencies[type];
					}

					Resources[index].value = std::min<unsigned char> (16, Resources[index].value);
				}
			}
		}
	}
	delete[] resSpots;
	delete[] resSpotTypes;
	resSpots = NULL;
}

/* static */ int cMap::getMapLevel (const cBuilding& building)
{
	const sUnitData& data = building.data;

	if (data.surfacePosition == sUnitData::SURFACE_POS_BENEATH_SEA) return 9; // seamine
	if (data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA) return 7; // bridge
	if (data.surfacePosition == sUnitData::SURFACE_POS_BASE && data.canBeOverbuild) return 6; // platform
	if (data.surfacePosition == sUnitData::SURFACE_POS_BASE) return 5; // road
	if (!building.owner) return 4; // rubble
	if (data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE) return 3; // landmine

	return 1; // other buildings
}

/* static */ int cMap::getMapLevel (const cVehicle& vehicle)
{
	if (vehicle.data.factorSea > 0 && vehicle.data.factorGround == 0) return 8; // ships
	if (vehicle.data.factorAir > 0) return 0; // planes

	return 2; // other vehicles
}

void cMap::addBuilding (cBuilding& building, unsigned int x, unsigned int y)
{
	addBuilding (building, getOffset (x, y));
}

void cMap::addBuilding (cBuilding& building, unsigned int offset)
{
	//big base building are not implemented
	if (building.data.surfacePosition != sUnitData::SURFACE_POS_GROUND && building.data.isBig && building.owner) return;

	const int mapLevel = cMap::getMapLevel (building);
	unsigned int i = 0;

	if (building.data.isBig)
	{
		i = 0;
		while (i < fields[offset].buildings.size() && cMap::getMapLevel (*fields[offset].buildings[i]) < mapLevel) i++;
		fields[offset].buildings.insert (fields[offset].buildings.begin() + i, &building);

		offset += 1;
		i = 0;
		while (i < fields[offset].buildings.size() && cMap::getMapLevel (*fields[offset].buildings[i]) < mapLevel) i++;
		fields[offset].buildings.insert (fields[offset].buildings.begin() + i, &building);

		offset += getSize();
		i = 0;
		while (i < fields[offset].buildings.size() && cMap::getMapLevel (*fields[offset].buildings[i]) < mapLevel) i++;
		fields[offset].buildings.insert (fields[offset].buildings.begin() + i, &building);

		offset -= 1;
		i = 0;
		while (i < fields[offset].buildings.size() && cMap::getMapLevel (*fields[offset].buildings[i]) < mapLevel) i++;
		fields[offset].buildings.insert (fields[offset].buildings.begin() + i, &building);
	}
	else
	{
		while (i < fields[offset].buildings.size() && cMap::getMapLevel (*fields[offset].buildings[i]) < mapLevel) i++;
		fields[offset].buildings.insert (fields[offset].buildings.begin() + i, &building);
	}
}

void cMap::addVehicle (cVehicle& vehicle, unsigned int x, unsigned int y)
{
	addVehicle (vehicle, getOffset (x, y));
}

void cMap::addVehicle (cVehicle& vehicle, unsigned int offset)
{
	if (vehicle.data.factorAir > 0)
	{
		fields[offset].planes.insert (fields[offset].planes.begin(), &vehicle);
	}
	else
	{
		fields[offset].vehicles.insert (fields[offset].vehicles.begin(), &vehicle);
	}
}

void cMap::deleteBuilding (const cBuilding& building)
{
	int offset = getOffset (building.PosX, building.PosY);

	std::vector<cBuilding*>* buildings = &fields[offset].buildings;
	Remove (*buildings, &building);

	if (building.data.isBig)
	{
		offset++;
		buildings = &fields[offset].buildings;
		Remove (*buildings, &building);

		offset += getSize();
		buildings = &fields[offset].buildings;
		Remove (*buildings, &building);

		offset--;
		buildings = &fields[offset].buildings;
		Remove (*buildings, &building);
	}
}

void cMap::deleteVehicle (const cVehicle& vehicle)
{
	int offset = getOffset (vehicle.PosX, vehicle.PosY);

	if (vehicle.data.factorAir > 0)
	{
		std::vector<cVehicle*>& planes = fields[offset].planes;
		Remove (planes, &vehicle);
	}
	else
	{
		std::vector<cVehicle*>* vehicles = &fields[offset].vehicles;
		Remove (*vehicles, &vehicle);

		if (vehicle.data.isBig)
		{
			offset++;
			vehicles = &fields[offset].vehicles;
			Remove (*vehicles, &vehicle);

			offset += getSize();
			vehicles = &fields[offset].vehicles;
			Remove (*vehicles, &vehicle);

			offset--;
			vehicles = &fields[offset].vehicles;
			Remove (*vehicles, &vehicle);
		}
	}
}

void cMap::moveVehicle (cVehicle& vehicle, unsigned int x, unsigned int y, int height)
{
	int oldOffset = getOffset (vehicle.PosX, vehicle.PosY);
	int newOffset = getOffset (x, y);

	vehicle.PosX = x;
	vehicle.PosY = y;

	if (vehicle.data.factorAir > 0)
	{
		std::vector<cVehicle*>& planes = fields[oldOffset].planes;
		Remove (planes, &vehicle);
		height = std::min<int> (this->fields[newOffset].planes.size(), height);
		fields[newOffset].planes.insert (fields[newOffset].planes.begin() + height, &vehicle);
	}
	else
	{
		std::vector<cVehicle*>& vehicles = fields[oldOffset].vehicles;
		Remove (vehicles, &vehicle);

		//check, whether the vehicle is centered on 4 map fields
		if (vehicle.data.isBig)
		{
			oldOffset++;
			fields[oldOffset].vehicles.erase (fields[oldOffset].vehicles.begin());
			oldOffset += getSize();
			fields[oldOffset].vehicles.erase (fields[oldOffset].vehicles.begin());
			oldOffset--;
			fields[oldOffset].vehicles.erase (fields[oldOffset].vehicles.begin());

			vehicle.data.isBig = false;
		}

		fields[newOffset].vehicles.insert (fields[newOffset].vehicles.begin(), &vehicle);
	}
}

void cMap::moveVehicleBig (cVehicle& vehicle, unsigned int x, unsigned int y)
{
	if (vehicle.data.isBig)
	{
		Log.write ("Calling moveVehicleBig on a big vehicle", cLog::eLOG_TYPE_NET_ERROR);
		//calling this function twice is always an error.
		//nevertheless try to proceed by resetting the data.isBig flag
		moveVehicle (vehicle, x, y);
	}

	int oldOffset = getOffset (vehicle.PosX, vehicle.PosY);
	int newOffset = getOffset (x, y);

	fields[oldOffset].vehicles.erase (fields[oldOffset].vehicles.begin());

	vehicle.PosX = x;
	vehicle.PosY = y;

	fields[newOffset].vehicles.insert (fields[newOffset].vehicles.begin(), &vehicle);
	newOffset++;
	fields[newOffset].vehicles.insert (fields[newOffset].vehicles.begin(), &vehicle);
	newOffset += getSize();
	fields[newOffset].vehicles.insert (fields[newOffset].vehicles.begin(), &vehicle);
	newOffset--;
	fields[newOffset].vehicles.insert (fields[newOffset].vehicles.begin(), &vehicle);

	vehicle.data.isBig = true;
}

bool cMap::possiblePlace (const cVehicle& vehicle, int x, int y, bool checkPlayer) const
{
	return possiblePlaceVehicle (vehicle.data, x, y, vehicle.owner, checkPlayer);
}

bool cMap::possiblePlaceVehicle (const sUnitData& vehicleData, int x, int y, const cPlayer* player, bool checkPlayer) const
{
	if (isValidPos (x, y) == false) return false;
	int offset = getOffset (x, y);

	const std::vector<cBuilding*>& buildings = fields[offset].getBuildings();
	std::vector<cBuilding*>::const_iterator b_it = buildings.begin();
	std::vector<cBuilding*>::const_iterator b_end = buildings.end();

	//search first building, that is not a connector
	if (b_it != b_end && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE) ++b_it;

	if (vehicleData.factorAir > 0)
	{
		if (checkPlayer && player && !player->ScanMap[offset]) return true;
		//only one plane per field for now
		if (fields[offset].planes.size() >= MAX_PLANES_PER_FIELD) return false;
	}
	if (vehicleData.factorGround > 0)
	{
		if (isBlocked (offset)) return false;

		if ((isWater (offset) && vehicleData.factorSea == 0) ||
			(isCoast (offset) && vehicleData.factorCoast == 0))
		{
			if (checkPlayer && player && !player->ScanMap[offset]) return false;

			//vehicle can drive on water, if there is a bridge, platform or road
			if (b_it == b_end) return false;
			if ((*b_it)->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE_SEA &&
				(*b_it)->data.surfacePosition != sUnitData::SURFACE_POS_BASE &&
				(*b_it)->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE_BASE) return false;
		}
		//check for enemy mines
		if (player && b_it != b_end && (*b_it)->owner != player &&
			(*b_it)->data.explodesOnContact &&
			((*b_it)->isDetectedByPlayer (player) || checkPlayer))
			return false;

		if (checkPlayer && player && !player->ScanMap[offset]) return true;

		if (fields[offset].vehicles.empty() == false) return false;
		if (b_it != b_end)
		{
			// only base buildings and rubble is allowed on the same field with a vehicle
			// (connectors have been skiped, so doesn't matter here)
			if ((*b_it)->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE_SEA &&
				(*b_it)->data.surfacePosition != sUnitData::SURFACE_POS_BASE &&
				(*b_it)->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE_BASE &&
				(*b_it)->data.surfacePosition != sUnitData::SURFACE_POS_BENEATH_SEA &&
				(*b_it)->owner) return false;
		}
	}
	else if (vehicleData.factorSea > 0)
	{
		if (isBlocked (offset)) return false;

		if (!isWater (offset) &&
			(!isCoast (offset) || vehicleData.factorCoast == 0)) return false;

		//check for enemy mines
		if (player && b_it != b_end && (*b_it)->owner != player &&
			(*b_it)->data.explodesOnContact && (*b_it)->isDetectedByPlayer (player))
			return false;

		if (checkPlayer && player && !player->ScanMap[offset]) return true;

		if (fields[offset].vehicles.empty() == false) return false;

		//only bridge and sea mine are allowed on the same field with a ship (connectors have been skiped, so doesn't matter here)
		if (b_it != b_end &&
			(*b_it)->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE_SEA &&
			(*b_it)->data.surfacePosition != sUnitData::SURFACE_POS_BENEATH_SEA)
		{
			// if the building is a landmine, we have to check whether it's on a bridge or not
			if ((*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)
			{
				++b_it;
				if (b_it == b_end || (*b_it)->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE_SEA) return false;
			}
			else return false;
		}
	}
	return true;
}

bool cMap::possiblePlaceBuilding (const sUnitData& buildingData, int x, int y, const cVehicle* vehicle) const
{
	return possiblePlaceBuildingWithMargin (buildingData, x, y, 0, vehicle);
}

// can't place it too near to the map border
bool cMap::possiblePlaceBuildingWithMargin (const sUnitData& buildingData, int x, int y, int margin, const cVehicle* vehicle) const
{
	if (x < margin || x >= getSize() - margin || y < margin || y >= getSize() - margin) return false;
	return possiblePlaceBuilding (buildingData, getOffset (x, y), vehicle);
}

bool cMap::possiblePlaceBuilding (const sUnitData& buildingData, int offset, const cVehicle* vehicle) const
{
	if (isValidOffset (offset) == false) return false;
	if (isBlocked (offset)) return false;
	cMapField& field = fields[offset];

	// Check all buildings in this field for a building of the same type. This
	// will prevent roads, connectors and water platforms from building on top
	// of themselves.
	const std::vector<cBuilding*>& buildings = field.getBuildings();
	for (std::vector<cBuilding*>::const_iterator it = buildings.begin(); it != buildings.end(); ++it)
	{
		if ((*it)->data.ID == buildingData.ID)
		{
			return false;
		}
	}

	// Determine terrain type
	bool water = isWater (offset);
	bool coast = isCoast (offset);
	bool ground = !water && !coast;

	for (std::vector<cBuilding*>::const_iterator it = buildings.begin(); it != buildings.end(); ++it)
	{
		if (buildingData.surfacePosition == (*it)->data.surfacePosition &&
			(*it)->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_NO) return false;
		switch ((*it)->data.surfacePosition)
		{
			case sUnitData::SURFACE_POS_GROUND:
			case sUnitData::SURFACE_POS_ABOVE_SEA: // bridge
				if (buildingData.surfacePosition != sUnitData::SURFACE_POS_ABOVE &&
					buildingData.surfacePosition != sUnitData::SURFACE_POS_BASE && // mine can be placed on bridge
					buildingData.surfacePosition != sUnitData::SURFACE_POS_BENEATH_SEA && // seamine can be placed under bridge
					(*it)->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_NO) return false;
				break;
			case sUnitData::SURFACE_POS_BENEATH_SEA: // seamine
			case sUnitData::SURFACE_POS_ABOVE_BASE:  // landmine
				// mine must be removed first
				if (buildingData.surfacePosition != sUnitData::SURFACE_POS_ABOVE) return false;
				break;
			case sUnitData::SURFACE_POS_BASE: // platform, road
				water = coast = false;
				ground = true;
				break;
			case sUnitData::SURFACE_POS_ABOVE: // connector
				break;
		}
	}
	if ((water && buildingData.factorSea == 0) ||
		(coast && buildingData.factorCoast == 0) ||
		(ground && buildingData.factorGround == 0)) return false;

	//can not build on rubble
	if (field.getRubble() &&
		buildingData.surfacePosition != sUnitData::SURFACE_POS_ABOVE &&
		buildingData.surfacePosition != sUnitData::SURFACE_POS_ABOVE_BASE) return false;

	if (field.vehicles.empty() == false)
	{
		if (!vehicle) return false;
		if (vehicle != field.vehicles[0]) return false;
	}
	return true;
}
void cMap::reset()
{
	for (int i = 0; i < getSize() * getSize(); i++)
	{
		fields[i].buildings.clear();
		fields[i].vehicles.clear();
		fields[i].planes.clear();
	}
}
