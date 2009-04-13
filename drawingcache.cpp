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

cBuildingCache::cBuildingCache()
{
	maxCacheSize = 0;
	cacheHits = 0;
	cacheMisses = 0;
};

cBuildingCache::~cBuildingCache()
{
	while ( cachedImages.Size() )
	{
		SDL_FreeSurface( cachedImages[0]->surface );
		delete cachedImages[0];
		cachedImages.Delete(0);
	}
};

SDL_Surface* cBuildingCache::getCachedImage(cBuilding* building )
{
	if ( !canCache(building) ) return NULL;

	for ( unsigned int i = 0; i < cachedImages.Size(); i++ )
	{
		sBuildingCacheEntry* entry = cachedImages[i];

		//check wether the entrys properties are equal to the building
		if ( entry->typ != building->typ ) continue;
		if ( entry->owner != building->owner ) continue;
		if ( building->SubBase )
		{
			if ( building->BaseN != entry->BaseN ||
				 building->BaseE != entry->BaseE ||
				 building->BaseS != entry->BaseS ||
				 building->BaseW != entry->BaseW ) continue;

			if ( building->data.is_big )
			{
				if ( building->BaseBN != entry->BaseBN ||
					 building->BaseBE != entry->BaseBE ||
					 building->BaseBS != entry->BaseBS ||
					 building->BaseBW != entry->BaseBW ) continue;
			}

		}
		if ( building->data.has_frames && !building->data.is_annimated )
		{
			if ( entry->dir != building->dir ) continue;
		}
		if ( entry->zoom != Client->Hud.Zoom ) continue;

		//cache hit!
		cacheHits++;
		entry->lastUsed = Client->iFrame;
		return entry->surface;

	}

	//cache miss!
	cacheMisses++;
	return NULL;
};


SDL_Surface* cBuildingCache::createNewEntry(cBuilding* building)
{
	if ( !canCache(building) ) return NULL;

	if ( cachedImages.Size() < maxCacheSize ) //cache hasn't reached the max size, so allocate a new entry
	{
		sBuildingCacheEntry* entry = new sBuildingCacheEntry();

		//set properties of the cached image
		entry->BaseN  = building->BaseN;
		entry->BaseBN = building->BaseBN;
		entry->BaseE  = building->BaseE;
		entry->BaseBE = building->BaseBE;
		entry->BaseS  = building->BaseS;
		entry->BaseBS = building->BaseBS;
		entry->BaseW  = building->BaseW;
		entry->BaseBW = building->BaseBW;
		entry->dir = building->dir;
		entry->owner = building->owner;
		entry->typ = building->typ;

		entry->zoom = Client->Hud.Zoom;
		entry->lastUsed = Client->iFrame;

		//determine needed size of the surface
		float factor = (float)(Client->Hud.Zoom/64.0);
		int height = (int) max(building->typ->img_org->h*factor, building->typ->shw_org->h*factor);
		int width  = (int) max(building->typ->img_org->w*factor, building->typ->shw_org->w*factor);
		if ( building->data.has_frames ) width = building->typ->shw_org->w*factor;
		entry->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000); 
		
		SDL_FillRect( entry->surface, NULL, SDL_MapRGBA( entry->surface->format, 255, 0, 255, 0));
		SDL_SetColorKey( entry->surface, SDL_SRCCOLORKEY, SDL_MapRGBA( entry->surface->format, 255, 0, 255, 0) );

		cachedImages.Add(entry);
		return entry->surface;
	}

	//try to find an old entry to reuse
	for ( unsigned int i = 0; i < cachedImages.Size(); i++ )
	{
		if ( Client->iFrame - cachedImages[i]->lastUsed < 5 )  continue;
		
		//entry has not been used for 5 frames. Use it for the new entry.
		sBuildingCacheEntry* entry = cachedImages[i];

		//set properties of the cached image
		entry->BaseN  = building->BaseN;
		entry->BaseBN = building->BaseBN;
		entry->BaseE  = building->BaseE;
		entry->BaseBE = building->BaseBE;
		entry->BaseS  = building->BaseS;
		entry->BaseBS = building->BaseBS;
		entry->BaseW  = building->BaseW;
		entry->BaseBW = building->BaseBW;
		entry->dir = building->dir;
		entry->owner = building->owner;
		entry->typ = building->typ;

		entry->zoom = Client->Hud.Zoom;
		entry->lastUsed = Client->iFrame;

		//check size of the surface
		float factor = (float)(Client->Hud.Zoom/64.0);
		int height = (int) max(building->typ->img_org->h*factor, building->typ->shw_org->h*factor);
		int width  = (int) max(building->typ->img_org->w*factor, building->typ->shw_org->w*factor);
		if ( building->data.has_frames ) width = building->typ->shw_org->w*factor;
		SDL_FreeSurface( entry->surface );
		entry->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000); 

		SDL_FillRect( entry->surface, NULL, SDL_MapRGBA( entry->surface->format, 255, 0, 255, 0));
		SDL_SetColorKey( entry->surface, SDL_SRCCOLORKEY, SDL_MapRGBA( entry->surface->format, 255, 0, 255, 0) );

		return entry->surface;

	}

	//there are no old entries in the cache.
	return NULL;
};

void cBuildingCache::flush()
{
	while ( cachedImages.Size() > 0 )
	{
		SDL_FreeSurface( cachedImages[0]->surface );
		delete cachedImages[0];
		cachedImages.Delete( 0 );
	}
};

bool cBuildingCache::canCache( cBuilding* building )
{
	if ( !building->owner ) return false;

	if ( building->StartUp ) return false;

	return true;
};

void cBuildingCache::resetStatistics()
{
	cacheMisses = 0;
	cacheHits = 0;
};

int cBuildingCache::getMaxCacheSize()
{
	return maxCacheSize;	
};

void cBuildingCache::setMaxCachsize( unsigned int newSize )
{
	while ( cachedImages.Size() > newSize )
	{
		SDL_FreeSurface( cachedImages[0]->surface );
		delete cachedImages[0];
		cachedImages.Delete( 0 );
	}
	maxCacheSize = newSize;	
};

int cBuildingCache::getCacheSize()
{
	return (int) cachedImages.Size();
};

int cBuildingCache::getCacheHits()
{
	return cacheHits;
};

int cBuildingCache::getCacheMisses()
{
	return cacheMisses;
};
