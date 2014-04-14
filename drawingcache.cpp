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

#include <algorithm>

#include "drawingcache.h"

#include "buildings.h"
#include "client.h"
#include "hud.h"
#include "loaddata.h"
#include "player.h"
#include "settings.h"
#include "vehicles.h"
#include "map.h"
#include "gui/game/temp/animationtimer.h"

void sDrawingCacheEntry::init (const cVehicle& vehicle, const cMap& map, const cPlayer* player, unsigned long long animationTime, double zoom_)
{
	dir = vehicle.dir;
	owner = vehicle.owner;
	isBuilding = vehicle.isUnitBuildingABuilding ();
	isClearing = vehicle.isUnitClearing ();
	flightHigh = vehicle.FlightHigh;
	big = vehicle.data.isBig;
	id = vehicle.data.ID;
	if (vehicle.data.animationMovement)
		frame = vehicle.WalkFrame;
	else
		frame = animationTime % 4;

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
	if ((vehicle.data.isStealthOn & TERRAIN_SEA) && isOnWaterAndNotCoast && vehicle.detectedByPlayerList.empty () && vehicle.owner == player)
		stealth = true;
	else
		stealth = false;

	zoom = zoom_;
	//lastUsed = gameGUI.getFrame();

	//determine needed size of the surface
	int height = (int) std::max (vehicle.uiData->img_org[vehicle.dir]->h * zoom, vehicle.uiData->shw_org[vehicle.dir]->h * zoom);
	int width  = (int) std::max (vehicle.uiData->img_org[vehicle.dir]->w * zoom, vehicle.uiData->shw_org[vehicle.dir]->w * zoom);
	if (vehicle.FlightHigh > 0)
	{
		//int shwOff = ((int) (gameGUI.getTileSize() * (vehicle.FlightHigh / 64.0f)));
		//height += shwOff;
		//width  += shwOff;
	}
	if (vehicle.isUnitClearing () || vehicle.isUnitBuildingABuilding ())
	{
		width  = 130;
		height = 130;
	}
	surface = SDL_CreateRGBSurface (0, width, height, 32,
									0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	SDL_FillRect (surface.get (), NULL, SDL_MapRGBA (surface->format, 0, 0, 0, 0));
}

void sDrawingCacheEntry::init (const cBuilding& building, double zoom_)
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
	id = building.data.ID;
	clan = building.owner->getClan();

	zoom = zoom_;
	//lastUsed = gameGUI.getFrame();

	//determine needed size of the surface
	int height = (int) std::max (building.uiData->img_org->h * zoom, building.uiData->shw_org->h * zoom);
	int width  = (int) std::max (building.uiData->img_org->w * zoom, building.uiData->shw_org->w * zoom);
	if (building.data.hasFrames) width = (int) (building.uiData->shw_org->w * zoom);

	surface = SDL_CreateRGBSurface (0, width, height, 32,
									0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	SDL_FillRect (surface.get (), NULL, SDL_MapRGBA (surface->format, 0, 0, 0, 0));
}

cDrawingCache::cDrawingCache (std::shared_ptr<cAnimationTimer> animationTimer_) :
	animationTimer (animationTimer_),
	player (nullptr)
{
	//assert (animationTimer != nullptr);

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

void cDrawingCache::setPlayer (const cPlayer* player_)
{
	player = player_;
}

SDL_Surface* cDrawingCache::getCachedImage (const cBuilding& building, double zoom)
{
	if (!canCache (building)) return NULL;

	for (unsigned int i = 0; i < cacheSize; i++)
	{
		sDrawingCacheEntry& entry = cachedImages[i];

		// check whether the entry's properties are equal to the building
		if (entry.id != building.data.ID) continue;
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
		if (entry.zoom != zoom) continue;

		if (building.data.hasClanLogos && building.owner->getClan() != entry.clan) continue;

		//cache hit!
		cacheHits++;
		//entry.lastUsed = gameGUI->getFrame();
		return entry.surface.get ();
	}

	//cache miss!
	cacheMisses++;
	return NULL;
}

SDL_Surface* cDrawingCache::getCachedImage (const cVehicle& vehicle, double zoom, const cMap& map)
{
	if (!canCache (vehicle)) return NULL;

	for (unsigned int i = 0; i < cacheSize; i++)
	{
		sDrawingCacheEntry& entry = cachedImages[i];

		// check whether the entry's properties are equal to the building
		if (entry.id != vehicle.data.ID) continue;
		if (entry.owner != vehicle.owner) continue;
		if (entry.big != vehicle.data.isBig) continue;
		if (entry.isBuilding != vehicle.isUnitBuildingABuilding ()) continue;
		if (entry.isClearing != vehicle.isUnitClearing ()) continue;

		if (entry.flightHigh != vehicle.FlightHigh) continue;
		if (entry.dir != vehicle.dir) continue;

		if (vehicle.data.animationMovement)
		{
			if (entry.frame != vehicle.WalkFrame) continue;
		}
		if (vehicle.isUnitBuildingABuilding () || vehicle.isUnitClearing ())
		{
			if (entry.frame != animationTimer->getAnimationTime() % 4) continue;
		}

		if (entry.zoom != zoom) continue;

		bool water = map.isWaterOrCoast (vehicle.PosX, vehicle.PosY) && !map.fields[map.getOffset (vehicle.PosX, vehicle.PosY)].getBaseBuilding();
		if (vehicle.isUnitBuildingABuilding ())
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

		if ((vehicle.data.isStealthOn & TERRAIN_SEA) && isOnWaterAndNotCoast && vehicle.detectedByPlayerList.empty () && vehicle.owner == player)
			stealth = true;

		if (entry.stealth != stealth) continue;

		//cache hit!
		cacheHits++;
		//entry.lastUsed = gameGUI->getFrame();
		return entry.surface.get ();
	}

	//cache miss!
	cacheMisses++;
	return NULL;
}

SDL_Surface* cDrawingCache::createNewEntry (const cBuilding& building, double zoom)
{
	if (!canCache (building)) return NULL;

	if (cacheSize < maxCacheSize)   //cache hasn't reached the max size, so allocate a new entry
	{
		sDrawingCacheEntry& entry = cachedImages[cacheSize];
		cacheSize++;

		//set properties of the cached image
		entry.init (building, zoom);

		return entry.surface.get ();
	}

	//try to find an old entry to reuse
	for (unsigned int i = 0; i < cacheSize; i++)
	{
		//if (gameGUI->getFrame() - cachedImages[i].lastUsed < 5)  continue;
		//entry has not been used for 5 frames. Use it for the new entry.

		sDrawingCacheEntry& entry = cachedImages[i];

		//set properties of the cached image
		entry.init (building, zoom);

		return entry.surface.get ();
	}

	//there are no old entries in the cache.
	return NULL;
}

SDL_Surface* cDrawingCache::createNewEntry (const cVehicle& vehicle, double zoom, const cMap& map)
{
	if (!canCache (vehicle))
		return NULL;

	if (cacheSize < maxCacheSize)   //cache hasn't reached the max size, so allocate a new entry
	{
		sDrawingCacheEntry& entry = cachedImages[cacheSize];

		//set properties of the cached image
		entry.init (vehicle, map, player, animationTimer->getAnimationTime (), zoom);

		cacheSize++;
		return entry.surface.get ();
	}

	//try to find an old entry to reuse
	for (unsigned int i = 0; i < cacheSize; i++)
	{
		//if (gameGUI->getFrame() - cachedImages[i].lastUsed < 5)  continue;

		//entry has not been used for 5 frames. Use it for the new entry.
		sDrawingCacheEntry& entry = cachedImages[i];

		//set properties of the cached image
		entry.init (vehicle, map, player, animationTimer->getAnimationTime (), zoom);
		return entry.surface.get ();
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
	if ((vehicle.isUnitBuildingABuilding () || vehicle.isUnitClearing ()) && vehicle.job)
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

	if (vehicle.isUnitBuildingABuilding () && vehicle.data.isBig && vehicle.BigBetonAlpha < 254u)
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
