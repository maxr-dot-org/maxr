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

#ifndef drawingcacheH
#define drawingcacheH

#include "main.h"

/**
* Stores all properties, which determine the look of the unit.
* Overlays and alpha effects are not cached!
*/
struct sDrawingCacheEntry
{
	//building proberties
	bool BaseN;
	bool BaseBN;
	bool BaseE;
	bool BaseBE;
	bool BaseS;
	bool BaseBS;
	bool BaseW;
	bool BaseBW;
	sBuilding* buildingTyp;

	//vehicle properties
	sVehicle* vehicleTyp;
	int frame;
	int flightHigh;
	bool big;
	bool isBuilding;
	bool isClearing;
	bool stealth;

	//common properties
	cPlayer* owner;
	int dir;
	int zoom;

	int lastUsed;
	SDL_Surface* surface;

	sDrawingCacheEntry();
	~sDrawingCacheEntry();
	/**
	* sets all properties and initialises the surface.
	*/
	void init( cVehicle* vehicle);
	void init( cBuilding* building);
};

class cDrawingCache
{
public:
	cDrawingCache();
	~cDrawingCache();

	/**
	* This method looks for a cached image, that matches the properties of the passed building.
	* @return a pointer to a surface, which contains the already rendered image of the building or NULL when no matchong cache entry exists.
	*/  
	SDL_Surface* getCachedImage(cBuilding* building );
	SDL_Surface* getCachedImage(cVehicle* vehicle );
	/**
	* This method creates a new chace entry, when there is space in the cache.
	* When there is no free space, an old entry is reused.
	* When there is no free space and no old entries, NULL is returned.
	* @return a surface to which the building has to be drawn, after calling this function. Returns NULL when the cache is full.
	*/
	SDL_Surface* createNewEntry(cBuilding* building);
	SDL_Surface* createNewEntry(cVehicle* vehicle);
	/**
	* Deletes all cache entries.
	*/
	void flush();
	
	void resetStatistics();
	int getMaxCacheSize();
	void setMaxCacheSize( unsigned int newSize );
	int getCacheSize();
	int getCacheHits();
	int getCacheMisses();
	int getNotCached();

private:
	unsigned int maxCacheSize;
	unsigned int cacheSize;
	sDrawingCacheEntry* cachedImages;
	bool canCache( cBuilding* building );
	bool canCache( cVehicle* vehicle );

	//statistics
	int cacheHits;
	int cacheMisses;
	int notCached;

};

#endif
