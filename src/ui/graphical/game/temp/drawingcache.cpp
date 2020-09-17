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

#include "game/data/units/building.h"
#include "game/logic/client.h"
#include "game/data/player/player.h"
#include "settings.h"
#include "game/data/units/vehicle.h"
#include "game/data/map/mapview.h"
#include "ui/graphical/game/animations/animationtimer.h"
#include "ui/graphical/framecounter.h"
#include "game/data/map/mapfieldview.h"
#include "utility/mathtools.h"

sDrawingCacheEntry::sDrawingCacheEntry()
{}

sDrawingCacheEntry::sDrawingCacheEntry(sDrawingCacheEntry&& other) :
	BaseN(other.BaseN),
	BaseBN(other.BaseBN),
	BaseE(other.BaseE),
	BaseBE(other.BaseBE),
	BaseS(other.BaseS),
	BaseBS(other.BaseBS),
	BaseW(other.BaseW),
	BaseBW(other.BaseBW),
	clan(other.clan),
	frame(other.frame),
	flightHigh(other.flightHigh),
	big(other.big),
	isBuilding(other.isBuilding),
	isClearing(other.isClearing),
	stealth(other.stealth),
	water(other.water),
	id(other.id),
	owner(other.owner),
	dir(other.dir),
	zoom(other.zoom),
	lastUsed(other.lastUsed),
	surface(std::move(other.surface))
{}

sDrawingCacheEntry& sDrawingCacheEntry::operator=(sDrawingCacheEntry&& other)
{
	BaseN = other.BaseN;
	BaseBN = other.BaseBN;
	BaseE = other.BaseE;
	BaseBE = other.BaseBE;
	BaseS = other.BaseS;
	BaseBS = other.BaseBS;
	BaseW = other.BaseW;
	BaseBW = other.BaseBW;
	clan = other.clan;

	frame = other.frame;
	flightHigh = other.flightHigh;
	big = other.big;
	isBuilding = other.isBuilding;
	isClearing = other.isClearing;
	stealth = other.stealth;
	water = other.water;

	id = other.id;
	owner = other.owner;
	dir = other.dir;
	zoom = other.zoom;
	lastUsed = other.lastUsed;

	surface = std::move(other.surface);

	return *this;
}

void sDrawingCacheEntry::init (const cVehicle& vehicle, const cMapView& map, const cPlayer* player, unsigned long long animationTime, double zoom_, unsigned long long frameNr)
{
	dir = vehicle.dir;
	owner = vehicle.getOwner();
	isBuilding = vehicle.isUnitBuildingABuilding();
	isClearing = vehicle.isUnitClearing();
	flightHigh = vehicle.getFlightHeight();
	big = vehicle.getIsBig();
	id = vehicle.data.getId();
	if (vehicle.uiData->animationMovement)
		frame = vehicle.WalkFrame;
	else
		frame = animationTime % 4;

	water = map.isWaterOrCoast (vehicle.getPosition()) && !map.getField (vehicle.getPosition()).getBaseBuilding();

	bool isOnWaterAndNotCoast = map.isWater (vehicle.getPosition());
	//if the vehicle can also drive on land, we have to check, whether there is a brige, platform, etc.
	//because the vehicle will drive on the bridge
	cBuilding* building = map.getField (vehicle.getPosition()).getBaseBuilding();
	if (vehicle.getStaticUnitData().factorGround > 0 && building
		&& (building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE_SEA
			|| building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_BASE
			|| building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE_BASE))
	{
		isOnWaterAndNotCoast = false;
	}
	if ((vehicle.getStaticUnitData().isStealthOn & TERRAIN_SEA) && isOnWaterAndNotCoast && !vehicle.isDetectedByAnyPlayer() && vehicle.getOwner() == player)
		stealth = true;
	else
		stealth = false;

	zoom = zoom_;
	lastUsed = frameNr;

	//determine needed size of the surface
	int height = (int) std::max (vehicle.uiData->img_org[vehicle.dir]->h * zoom, vehicle.uiData->shw_org[vehicle.dir]->h * zoom);
	int width  = (int) std::max (vehicle.uiData->img_org[vehicle.dir]->w * zoom, vehicle.uiData->shw_org[vehicle.dir]->w * zoom);
	if (vehicle.getFlightHeight() > 0)
	{
		int shwOff = ((int) (Round (vehicle.uiData->img_org[vehicle.dir]->w * zoom) * (vehicle.getFlightHeight() / 64.0f)));
		height += shwOff;
		width  += shwOff;
	}
	if (vehicle.isUnitClearing() || vehicle.isUnitBuildingABuilding())
	{
		width  = 130;
		height = 130;
	}
	surface = AutoSurface (SDL_CreateRGBSurface (0, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000));

	SDL_FillRect (surface.get(), nullptr, SDL_MapRGBA (surface->format, 0, 0, 0, 0));
}

void sDrawingCacheEntry::init (const cBuilding& building, double zoom_, unsigned long long frameNr)
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
	owner = building.getOwner();
	id = building.data.getId();
	clan = building.getOwner()->getClan();

	zoom = zoom_;
	lastUsed = frameNr;

	//determine needed size of the surface
	int height = (int) std::max (building.uiData->img_org->h * zoom, building.uiData->shw_org->h * zoom);
	int width  = (int) std::max (building.uiData->img_org->w * zoom, building.uiData->shw_org->w * zoom);
	if (building.uiData->hasFrames) width = (int)(building.uiData->shw_org->w * zoom);

	surface = AutoSurface (SDL_CreateRGBSurface (0, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000));

	SDL_FillRect (surface.get(), nullptr, SDL_MapRGBA (surface->format, 0, 0, 0, 0));
}

cDrawingCache::cDrawingCache (std::shared_ptr<const cFrameCounter> frameCounter_) :
	frameCounter (frameCounter_),
	player (nullptr)
{
	//assert (animationTimer != nullptr);

	cacheHits = 0;
	cacheMisses = 0;
	notCached = 0;
	cacheSize = 0;
	const std::size_t maxCacheSize = cSettings::getInstance().getCacheSize(); //set cache size from config
	cachedImages.resize (maxCacheSize);
}

void cDrawingCache::setPlayer (const cPlayer* player_)
{
	player = player_;
}

SDL_Surface* cDrawingCache::getCachedImage (const cBuilding& building, double zoom, unsigned long long animationTime)
{
	if (!canCache (building)) return nullptr;

	for (unsigned int i = 0; i < cacheSize; i++)
	{
		sDrawingCacheEntry& entry = cachedImages[i];

		// check whether the entry's properties are equal to the building
		if (entry.id != building.data.getId()) continue;
		if (entry.owner != building.getOwner()) continue;
		if (building.subBase)
		{
			if (building.BaseN != entry.BaseN ||
				building.BaseE != entry.BaseE ||
				building.BaseS != entry.BaseS ||
				building.BaseW != entry.BaseW) continue;

			if (building.getIsBig())
			{
				if (building.BaseBN != entry.BaseBN ||
					building.BaseBE != entry.BaseBE ||
					building.BaseBS != entry.BaseBS ||
					building.BaseBW != entry.BaseBW) continue;
			}

		}
		if (building.uiData->hasFrames && !building.uiData->isAnimated)
		{
			if (entry.dir != building.dir) continue;
		}
		if (entry.zoom != zoom) continue;

		if (building.uiData->hasClanLogos && building.getOwner()->getClan() != entry.clan) continue;

		//cache hit!
		cacheHits++;
		entry.lastUsed = frameCounter->getFrame();
		return entry.surface.get();
	}

	//cache miss!
	cacheMisses++;
	return nullptr;
}

SDL_Surface* cDrawingCache::getCachedImage (const cVehicle& vehicle, double zoom, const cMapView& map, unsigned long long animationTime)
{
	if (!canCache (vehicle)) return nullptr;

	for (unsigned int i = 0; i < cacheSize; i++)
	{
		sDrawingCacheEntry& entry = cachedImages[i];

		// check whether the entry's properties are equal to the building
		if (entry.id != vehicle.data.getId()) continue;
		if (entry.owner != vehicle.getOwner()) continue;
		if (entry.big != vehicle.getIsBig()) continue;
		if (entry.isBuilding != vehicle.isUnitBuildingABuilding()) continue;
		if (entry.isClearing != vehicle.isUnitClearing()) continue;

		if (entry.flightHigh != vehicle.getFlightHeight()) continue;
		if (entry.dir != vehicle.dir) continue;

		if (vehicle.uiData->animationMovement)
		{
			if (entry.frame != vehicle.WalkFrame) continue;
		}

		if (vehicle.isUnitBuildingABuilding() || vehicle.isUnitClearing())
		{
			if (entry.frame != animationTime % 4) continue;
		}

		if (entry.zoom != zoom) continue;

		bool water = map.isWaterOrCoast (vehicle.getPosition()) && !map.getField (vehicle.getPosition()).getBaseBuilding();
		if (vehicle.isUnitBuildingABuilding())
		{
			if (water != entry.water) continue;
		}

		//check the stealth flag
		bool stealth = false;

		bool isOnWaterAndNotCoast = map.isWater (vehicle.getPosition());
		const cBuilding* building = map.getField (vehicle.getPosition()).getBaseBuilding();
		if (vehicle.getStaticUnitData().factorGround > 0 && building
			&& (building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE_SEA
				|| building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_BASE
				|| building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE_BASE))
		{
			isOnWaterAndNotCoast = false;
		}

		if ((vehicle.getStaticUnitData().isStealthOn & TERRAIN_SEA) && isOnWaterAndNotCoast && !vehicle.isDetectedByAnyPlayer() && vehicle.getOwner() == player)
			stealth = true;

		if (entry.stealth != stealth) continue;

		//cache hit!
		cacheHits++;
		entry.lastUsed = frameCounter->getFrame();
		return entry.surface.get();
	}

	//cache miss!

	cacheMisses++;
	return nullptr;
}

SDL_Surface* cDrawingCache::createNewEntry (const cBuilding& building, double zoom, unsigned long long animationTime)
{
	if (!canCache (building))
		return nullptr;

	if (cachedImages.size() == 0)
		return nullptr;

	if (cacheSize < cachedImages.size())   //cache hasn't reached the max size, so allocate a new entry
	{
		sDrawingCacheEntry& entry = cachedImages[cacheSize];
		cacheSize++;

		//set properties of the cached image
		entry.init (building, zoom, frameCounter->getFrame());

		return entry.surface.get();
	}

	//try to find an old entry to reuse
	int oldest = 0;
	for (unsigned int i = 0; i < cacheSize; i++)
	{
		if (cachedImages[i].lastUsed < cachedImages[oldest].lastUsed)
			oldest = i;
	}

	if (frameCounter->getFrame() - cachedImages[oldest].lastUsed > 5)
	{

		//entry has not been used for 5 frames. Use it for the new entry.
		sDrawingCacheEntry& entry = cachedImages[oldest];

		//set properties of the cached image
		entry.init (building, zoom, frameCounter->getFrame());

		return entry.surface.get();
	}

	//there are no old entries in the cache.
	return nullptr;
}

SDL_Surface* cDrawingCache::createNewEntry (const cVehicle& vehicle, double zoom, const cMapView& map, unsigned long long animationTime)
{
	if (!canCache (vehicle))
		return nullptr;

	if (cachedImages.size() == 0)
		return nullptr;

	if (cacheSize < cachedImages.size())   //cache hasn't reached the max size, so allocate a new entry
	{
		sDrawingCacheEntry& entry = cachedImages[cacheSize];

		//set properties of the cached image
		entry.init (vehicle, map, player, animationTime, zoom, frameCounter->getFrame());

		cacheSize++;
		return entry.surface.get();
	}

	//try to find an old entry to reuse
	int oldest = 0;
	for (unsigned int i = 0; i < cacheSize; i++)
	{
		if (cachedImages[i].lastUsed < cachedImages[oldest].lastUsed)
			oldest = i;
	}

	if (frameCounter->getFrame() - cachedImages[oldest].lastUsed > 5)
	{

		//entry has not been used for 5 frames. Use it for the new entry.
		sDrawingCacheEntry& entry = cachedImages[oldest];

		//set properties of the cached image
		entry.init (vehicle, map, player, animationTime, zoom, frameCounter->getFrame());
		return entry.surface.get();
	}

	//there are no old entries in the cache.
	return nullptr;
}

void cDrawingCache::flush()
{
	cacheSize = 0;
}

bool cDrawingCache::canCache (const cBuilding& building)
{
	if (!building.getOwner() ||
		building.alphaEffectValue ||
		building.uiData->isAnimated)
	{
		notCached++;
		return false;
	}
	return true;
}

bool cDrawingCache::canCache (const cVehicle& vehicle)
{
	if ((vehicle.isUnitBuildingABuilding() || vehicle.isUnitClearing()) && vehicle.jobActive)
	{
		notCached++;
		return false;
	}

	if (vehicle.alphaEffectValue)
	{
		notCached++;
		return false;
	}

	if (vehicle.getFlightHeight() > 0 && vehicle.getFlightHeight() < 64)
	{
		notCached++;
		return false;
	}

	if (vehicle.isUnitBuildingABuilding() && vehicle.getIsBig() && vehicle.bigBetonAlpha < 254)
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
	return cachedImages.size();
}

void cDrawingCache::setMaxCacheSize (unsigned int newSize)
{
	cachedImages.clear();
	cachedImages.resize (newSize);
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
