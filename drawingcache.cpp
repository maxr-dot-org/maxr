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

#include "drawingcache.h"
#include "client.h"
#include "loaddata.h"
#include "settings.h"
#include "buildings.h"
#include "vehicles.h"
#include "player.h"

void sDrawingCacheEntry::init (const cGameGUI& gameGUI, const cVehicle& vehicle)
{
	dir = vehicle.dir;
	owner = vehicle.owner;
	isBuilding = vehicle.IsBuilding;
	isClearing = vehicle.IsClearing;
	flightHigh = vehicle.FlightHigh;
	big = vehicle.data.isBig;
	vehicleTyp = vehicle.typ;
	buildingTyp = NULL;
	if (vehicle.data.animationMovement)
		frame = vehicle.WalkFrame;
	else
		frame = gameGUI.getAnimationSpeed() % 4;

	const cMap& map = *gameGUI.getClient()->getMap();
	water = map.isWaterOrCoast (vehicle.PosX, vehicle.PosY) && !map.fields[map.getOffset (vehicle.PosX, vehicle.PosY)].getBaseBuilding();

	bool isOnWaterAndNotCoast = map.isWater (vehicle.PosX, vehicle.PosY);
	//if the vehicle can also drive on land, we have to check, whether there is a brige, platform, etc.
	//because the vehicle will drive on the bridge
	cBuilding* building = map.fields[map.getOffset (vehicle.PosX, vehicle.PosY)].getBaseBuilding();
	if (vehicle.data.factorGround > 0 && building
		&& (building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA
			|| building->data.surfacePosition == sUnitData::SURFACE_POS_BASE
			|| building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE))
	{
		isOnWaterAndNotCoast = false;
	}
	if ((vehicle.data.isStealthOn & TERRAIN_SEA) && isOnWaterAndNotCoast && vehicle.detectedByPlayerList.size() == 0 && vehicle.owner == gameGUI.getClient()->getActivePlayer())
		stealth = true;
	else
		stealth = false;

	zoom = gameGUI.getZoom();
	lastUsed = gameGUI.getFrame();

	//determine needed size of the surface
	int height = (int) std::max (vehicle.typ->img_org[vehicle.dir]->h * zoom, vehicle.typ->shw_org[vehicle.dir]->h * zoom);
	int width  = (int) std::max (vehicle.typ->img_org[vehicle.dir]->w * zoom, vehicle.typ->shw_org[vehicle.dir]->w * zoom);
	if (vehicle.FlightHigh > 0)
	{
		int shwOff = ( (int) (gameGUI.getTileSize() * (vehicle.FlightHigh / 64.0)));
		height += shwOff;
		width  += shwOff;
	}
	if (vehicle.IsClearing || vehicle.IsBuilding)
	{
		width  = 130;
		height = 130;
	}
	surface = SDL_CreateRGBSurface (SDL_SWSURFACE, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	SDL_FillRect (surface, NULL, SDL_MapRGBA (surface->format, 255, 0, 255, 0));
	SDL_SetColorKey (surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGBA (surface->format, 255, 0, 255, 0));
}

void sDrawingCacheEntry::init (const cGameGUI& gameGUI, const cBuilding& building)
{
	BaseN  = building.BaseN;
	BaseBN = building.BaseBN;
	BaseE  = building.BaseE;
	BaseBE = building.BaseBE;
	BaseS  = building.BaseS;
	BaseBS = building.BaseBS;
	BaseW  = building.BaseW;
	BaseBW = building.BaseBW;
	dir = building.dir;
	owner = building.owner;
	buildingTyp = building.typ;
	vehicleTyp = NULL;
	clan = building.owner->getClan();

	zoom = gameGUI.getZoom();
	lastUsed = gameGUI.getFrame();

	//determine needed size of the surface
	int height = (int) std::max (building.typ->img_org->h * zoom, building.typ->shw_org->h * zoom);
	int width  = (int) std::max (building.typ->img_org->w * zoom, building.typ->shw_org->w * zoom);
	if (building.data.hasFrames) width = (int) (building.typ->shw_org->w * zoom);

	surface = SDL_CreateRGBSurface (SDL_SWSURFACE, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	SDL_FillRect (surface, NULL, SDL_MapRGBA (surface->format, 255, 0, 255, 0));
	SDL_SetColorKey (surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGBA (surface->format, 255, 0, 255, 0));
}

cDrawingCache::cDrawingCache()
{
	cacheHits = 0;
	cacheMisses = 0;
	notCached = 0;
	cacheSize = 0;
	maxCacheSize = cSettings::getInstance().getCacheSize(); //set cache size from config
	cachedImages = new sDrawingCacheEntry[maxCacheSize];
}

cDrawingCache::~cDrawingCache()
{
	delete[] cachedImages;
}

void cDrawingCache::setGameGUI (const cGameGUI& gameGUI_)
{
	gameGUI = &gameGUI_;
}

SDL_Surface* cDrawingCache::getCachedImage (const cBuilding& building)
{
	if (!canCache (building)) return NULL;

	for (unsigned int i = 0; i < cacheSize; i++)
	{
		sDrawingCacheEntry& entry = cachedImages[i];

		//check wether the entrys properties are equal to the building
		if (entry.buildingTyp != building.typ) continue;
		if (entry.owner != building.owner) continue;
		if (building.SubBase)
		{
			if (building.BaseN != entry.BaseN ||
				building.BaseE != entry.BaseE ||
				building.BaseS != entry.BaseS ||
				building.BaseW != entry.BaseW) continue;

			if (building.data.isBig)
			{
				if (building.BaseBN != entry.BaseBN ||
					building.BaseBE != entry.BaseBE ||
					building.BaseBS != entry.BaseBS ||
					building.BaseBW != entry.BaseBW) continue;
			}

		}
		if (building.data.hasFrames && !building.data.isAnimated)
		{
			if (entry.dir != building.dir) continue;
		}
		if (entry.zoom != gameGUI->getZoom()) continue;

		if (building.data.hasClanLogos && building.owner->getClan() != entry.clan) continue;

		//cache hit!
		cacheHits++;
		entry.lastUsed = gameGUI->getFrame();
		return entry.surface;
	}

	//cache miss!
	cacheMisses++;
	return NULL;
}

SDL_Surface* cDrawingCache::getCachedImage (const cVehicle& vehicle)
{
	if (!canCache (vehicle)) return NULL;

	for (unsigned int i = 0; i < cacheSize; i++)
	{
		sDrawingCacheEntry& entry = cachedImages[i];

		//check wether the entrys properties are equal to the building
		if (entry.vehicleTyp != vehicle.typ) continue;
		if (entry.owner != vehicle.owner) continue;
		if (entry.big != vehicle.data.isBig) continue;
		if (entry.isBuilding != vehicle.IsBuilding) continue;
		if (entry.isClearing != vehicle.IsClearing) continue;

		if (entry.flightHigh != vehicle.FlightHigh) continue;
		if (entry.dir != vehicle.dir) continue;

		if (vehicle.data.animationMovement)
		{
			if (entry.frame != vehicle.WalkFrame) continue;
		}
		if (vehicle.IsBuilding || vehicle.IsClearing)
		{
			if (entry.frame != gameGUI->getAnimationSpeed() % 4) continue;
		}
		const cMap& map = *gameGUI->getClient()->getMap();

		if (entry.zoom != gameGUI->getZoom()) continue;

		bool water = map.isWaterOrCoast (vehicle.PosX, vehicle.PosY) && !map.fields[map.getOffset (vehicle.PosX, vehicle.PosY)].getBaseBuilding();
		if (vehicle.IsBuilding)
		{
			if (water != entry.water) continue;
		}

		//check the stealth flag
		bool stealth = false;

		bool isOnWaterAndNotCoast = map.isWater (vehicle.PosX, vehicle.PosY);
		const cBuilding* building = map.fields [map.getOffset (vehicle.PosX, vehicle.PosY)].getBaseBuilding();
		if (vehicle.data.factorGround > 0 && building
			&& (building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA
				|| building->data.surfacePosition == sUnitData::SURFACE_POS_BASE
				|| building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE))
		{
			isOnWaterAndNotCoast = false;
		}

		if ((vehicle.data.isStealthOn & TERRAIN_SEA) && isOnWaterAndNotCoast && vehicle.detectedByPlayerList.size() == 0 && vehicle.owner == gameGUI->getClient()->getActivePlayer())
			stealth = true;

		if (entry.stealth != stealth) continue;

		//cache hit!
		cacheHits++;
		entry.lastUsed = gameGUI->getFrame();
		return entry.surface;
	}

	//cache miss!
	cacheMisses++;
	return NULL;
}

SDL_Surface* cDrawingCache::createNewEntry (const cBuilding& building)
{
	if (!canCache (building)) return NULL;

	if (cacheSize < maxCacheSize)   //cache hasn't reached the max size, so allocate a new entry
	{
		sDrawingCacheEntry& entry = cachedImages[cacheSize];
		cacheSize++;

		//set properties of the cached image
		entry.init (*gameGUI, building);

		return entry.surface;
	}

	//try to find an old entry to reuse
	for (unsigned int i = 0; i < cacheSize; i++)
	{
		if (gameGUI->getFrame() - cachedImages[i].lastUsed < 5)  continue;
		//entry has not been used for 5 frames. Use it for the new entry.

		sDrawingCacheEntry& entry = cachedImages[i];

		//set properties of the cached image
		entry.init (*gameGUI, building);

		return entry.surface;
	}

	//there are no old entries in the cache.
	return NULL;
}

SDL_Surface* cDrawingCache::createNewEntry (const cVehicle& vehicle)
{
	if (!canCache (vehicle))
		return NULL;

	if (cacheSize < maxCacheSize)   //cache hasn't reached the max size, so allocate a new entry
	{
		sDrawingCacheEntry& entry = cachedImages[cacheSize];

		//set properties of the cached image
		entry.init (*gameGUI, vehicle);

		cacheSize++;
		return entry.surface;
	}

	//try to find an old entry to reuse
	for (unsigned int i = 0; i < cacheSize; i++)
	{
		if (gameGUI->getFrame() - cachedImages[i].lastUsed < 5)  continue;

		//entry has not been used for 5 frames. Use it for the new entry.
		sDrawingCacheEntry& entry = cachedImages[i];

		//set properties of the cached image
		entry.init (*gameGUI, vehicle);
		return entry.surface;
	}

	//there are no old entries in the cache.
	return NULL;
}

void cDrawingCache::flush()
{
	cacheSize = 0;
}

bool cDrawingCache::canCache (const cBuilding& building)
{
	if (!building.owner ||
		building.StartUp ||
		building.data.isAnimated)
	{
		notCached++;
		return false;
	}
	return true;
}

bool cDrawingCache::canCache (const cVehicle& vehicle)
{
	if ((vehicle.IsBuilding || vehicle.IsClearing) && vehicle.job)
	{
		notCached++;
		return false;
	}

	if (vehicle.StartUp)
	{
		notCached++;
		return false;
	}

	if (vehicle.FlightHigh > 0 && vehicle.FlightHigh < 64)
	{
		notCached++;
		return false;
	}

	if (vehicle.IsBuilding && vehicle.data.isBig && vehicle.BigBetonAlpha < 255)
	{
		notCached++;
		return false;
	}
	return true;
}

void cDrawingCache::resetStatistics()
{
	cacheMisses = 0;
	cacheHits = 0;
	notCached = 0;
}

int cDrawingCache::getMaxCacheSize() const
{
	return maxCacheSize;
}

void cDrawingCache::setMaxCacheSize (unsigned int newSize)
{
	delete[] cachedImages;
	cachedImages = new sDrawingCacheEntry[newSize];
	maxCacheSize = newSize;
	cacheSize = 0;

	cSettings::getInstance().setCacheSize (newSize);
}

int cDrawingCache::getCacheSize() const
{
	return cacheSize;
}

int cDrawingCache::getCacheHits() const
{
	return cacheHits;
}

int cDrawingCache::getCacheMisses() const
{
	return cacheMisses;
}

int cDrawingCache::getNotCached() const
{
	return notCached / 2;
}
