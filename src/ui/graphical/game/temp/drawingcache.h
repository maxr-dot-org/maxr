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

#ifndef ui_graphical_game_temp_drawingcacheH
#define ui_graphical_game_temp_drawingcacheH

#include "SDLutility/uniquesurface.h"
#include "game/data/units/unitdata.h"

#include <memory>

class cPlayer;
class cVehicle;
class cBuilding;
class cAnimationTimer;
class cMapView;
class cFrameCounter;

/**
* Stores all properties, which determine the look of the unit.
* Overlays and alpha effects are not cached!
*/
struct sDrawingCacheEntry
{
	sDrawingCacheEntry();
	sDrawingCacheEntry (sDrawingCacheEntry&&) = default;
	sDrawingCacheEntry& operator= (sDrawingCacheEntry&&) = default;
	sDrawingCacheEntry (const sDrawingCacheEntry&) = delete;
	sDrawingCacheEntry& operator= (const sDrawingCacheEntry&) = delete;

	/**
	* sets all properties and initialises the surface.
	*/
	void init (const cVehicle&, const cMapView&, const cPlayer*, unsigned long long animationTime, double zoom, unsigned long long frameNr);
	void init (const cBuilding&, double zoom, unsigned long long frameNr);

	//building properties
	bool BaseN;
	bool BaseBN;
	bool BaseE;
	bool BaseBE;
	bool BaseS;
	bool BaseBS;
	bool BaseW;
	bool BaseBW;
	int clan;

	//vehicle properties
	unsigned int frame;
	int flightHigh;
	bool big;
	bool isBuilding;
	bool isClearing;
	bool stealth;
	bool water;

	//common properties
	sID id;
	cPlayer* owner = nullptr;
	int dir;
	double zoom;
	unsigned long long lastUsed;

	UniqueSurface surface;
};

class cDrawingCache
{
public:
	cDrawingCache (std::shared_ptr<const cFrameCounter> frameCounter);

	void setPlayer (const cPlayer* player);

	/**
	* This method looks for a cached image, that matches the properties of the passed building.
	* @return a pointer to a surface, which contains the already rendered image of the building or nullptr when no matching cache entry exists.
	*/
	SDL_Surface* getCachedImage (const cBuilding& building, double zoom, unsigned long long animationTime);
	SDL_Surface* getCachedImage (const cVehicle& vehicle, double zoom, const cMapView& map, unsigned long long animationTime);
	/**
	* This method creates a new chace entry, when there is space in the cache.
	* When there is no free space, an old entry is reused.
	* When there is no free space and no old entries, nullptr is returned.
	* @return a surface to which the building has to be drawn, after calling this function. Returns nullptr when the cache is full.
	*/
	SDL_Surface* createNewEntry (const cBuilding& building, double zoom, unsigned long long animationTime);
	SDL_Surface* createNewEntry (const cVehicle& vehicle, double zoom, const cMapView& map, unsigned long long animationTime);
	/**
	* Deletes all cache entries.
	*/
	void flush();

	void resetStatistics();
	int getMaxCacheSize() const;
	void setMaxCacheSize (unsigned int newSize);
	int getCacheSize() const;
	int getCacheHits() const;
	int getCacheMisses() const;
	int getNotCached() const;

private:
	std::shared_ptr<const cFrameCounter> frameCounter;
	const cPlayer* player;

	unsigned int cacheSize;
	std::vector<sDrawingCacheEntry> cachedImages;
	bool canCache (const cBuilding& building);
	bool canCache (const cVehicle& vehicle);

	//statistics
	int cacheHits;
	int cacheMisses;
	int notCached;
};

#endif
