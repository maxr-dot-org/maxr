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

#ifndef ui_graphical_game_temp_unitdrawingengineH
#define ui_graphical_game_temp_unitdrawingengineH

#include <memory>

#include "ui/graphical/game/temp/drawingcache.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/color.h"

class cBuilding;
class cVehicle;
class cUnit;
class cPlayer;
class cAnimationTimer;
class cUnitSelection;
struct SDL_Rect;
class cFrameCounter;

// TODO: remove this temporary class!

class cUnitDrawingEngine
{
public:
	cUnitDrawingEngine (std::shared_ptr<cAnimationTimer> animationTimer, std::shared_ptr<const cFrameCounter> frameCounter);

	void drawUnit (const cBuilding& building, SDL_Rect destination, float zoomFactor, const cUnitSelection* unitSelection = nullptr, const cPlayer* player = nullptr);
	void drawUnit (const cVehicle& vehicle, SDL_Rect destination, float zoomFactor, const cMapView& map, const cUnitSelection* unitSelection = nullptr, const cPlayer* player = nullptr);

	void setDrawHits (bool drawHits);
	void setDrawStatus (bool drawStatus);
	void setDrawAmmo (bool drawAmmo);
	void setDrawColor (bool drawColor);

	void drawPath (const cVehicle& vehicle);
public:
	std::shared_ptr<cAnimationTimer> animationTimer;
	cDrawingCache drawingCache;

	cSignalConnectionManager signalConnectionManager;

	cRgbColor blinkColor;

	bool shouldDrawHits;
	bool shouldDrawStatus;
	bool shouldDrawAmmo;
	bool shouldDrawColor;

	void drawHealthBar (const cUnit& unit, SDL_Rect destination);
	void drawMunBar (const cUnit& unit, SDL_Rect destination);
	void drawStatus (const cUnit& unit, SDL_Rect destination);

	void rotateBlinkColor();
};

#endif // ui_graphical_game_temp_unitdrawingengineH
