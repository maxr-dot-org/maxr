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

#ifndef gui_game_gamemapwidgetH
#define gui_game_gamemapwidgetH

#include "../widget.h"
#include "../../maxrconfig.h"
#include "../../utility/signal/signal.h"
#include "temp/unitdrawingengine.h"

struct SDL_Surface;

class cStaticMap;
class cMap;
class cPlayer;

class cGameMapWidget : public cWidget
{
public:
	cGameMapWidget (const cBox<cPosition>& area, std::shared_ptr<const cStaticMap> staticMap);

	void setDynamicMap (const cMap* dynamicMap);
	void setPlayer (const cPlayer* player);

	void setZoomFactor (double zoomFactor);
	double getZoomFactor () const;

	/**
	 * Scrolls the map by a given offset.
	 *
	 * The scrolling is save, which means if the offset is to large
	 * the new offset position will be adjusted in such a way that the
	 * displayed area will not go out of the map range.
	 *
	 * @param offset The offset in pixels to scroll. Can be negative.
	 */
	void scroll (const cPosition& offset);
	const cPosition& getPixelOffset () const;

	void centerAt (const cPosition& position);

	void setDrawSurvey (bool drawSurvey);
	void setDrawHits (bool drawHits);
	void setDrawScan (bool drawScan);
	void setDrawStatus (bool drawStatus);
	void setDrawAmmo (bool drawAmmo);
	void setDrawGrid (bool drawGrid);
	void setDrawColor (bool drawColor);
	void setDrawRange (bool drawRange);
	void setDrawFog (bool drawFog);

	cBox<cPosition> getDisplayedMapArea () const;

	cSignal<void ()> scrolled;
	cSignal<void ()> zoomFactorChanged;

	virtual void draw () MAXR_OVERRIDE_FUNCTION;
protected:

private:
	//
	// data
	//
	std::shared_ptr<const cStaticMap> staticMap;
	const cMap* dynamicMap; // may be null
	const cPlayer* player; // may be null

	cUnitDrawingEngine unitDrawingEngine;

	//
	// drawing information data
	//
	cPosition pixelOffset;
	double internalZoomFactor; // should not be used directly! use getZoomFactor() instead!

	bool shouldDrawSurvey;
	bool shouldDrawScan;
	bool shouldDrawGrid;
	bool shouldDrawRange;
	bool shouldDrawFog;

	//
	// draw methods
	//
	void drawTerrain ();
	void drawGrid ();
	void drawEffects (bool bottom);

	void drawBaseUnits ();
	void drawTopBuildings ();
	void drawShips ();
	void drawAboveSeaBaseUnits ();
	void drawVehicles ();
	void drawConnectors ();
	void drawPlanes ();

	void drawResources ();

	//
	// position handling methods
	//
	double computeMinimalZoomFactor () const;
	cPosition computeMaximalPixelOffset () const;

	//
	// drawing helper methods
	//
	cPosition zoomSize (const cPosition& size, double zoomFactor) const;
	cPosition getZoomedTileSize () const;
	cPosition getZoomedStartTilePixelOffset () const;
	std::pair<cPosition, cPosition> computeTileDrawingRange () const;

	SDL_Rect computeTileDrawingArea (const cPosition& zoomedTileSize, const cPosition& zoomedStartTilePixelOffset, const cPosition& tileStartIndex, const cPosition& tileIndex);
};


#endif // gui_game_gamemapwidgetH
