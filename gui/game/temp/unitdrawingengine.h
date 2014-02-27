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

#ifndef gui_game_temp_unitdrawingengineH
#define gui_game_temp_unitdrawingengineH

#include <memory>

#include "../../../drawingcache.h"

class cBuilding;
class cVehicle;
class cUnit;
class cPlayer;
class cAnimationTimer;
//class cDrawingCache;
struct SDL_Rect;

class cUnitDrawingEngine
{
public:
	cUnitDrawingEngine ();

	// TODO: remove this function!
	void handleNewFrame ();

	void drawUnit (const cBuilding& building, SDL_Rect destination, double zoomFactor, const cPlayer* player = nullptr);
	void drawUnit (const cVehicle& vehicle, SDL_Rect destination, double zoomFactor, const cMap& map, const cPlayer* player = nullptr);

	void setDrawHits (bool drawHits);
	void setDrawStatus (bool drawStatus);
	void setDrawAmmo (bool drawAmmo);
	void setDrawColor (bool drawColor);
public:
	std::shared_ptr<cAnimationTimer> animationTimer;
	cDrawingCache drawingCache;

	bool shouldDrawHits;
	bool shouldDrawStatus;
	bool shouldDrawAmmo;
	bool shouldDrawColor;

	void drawHealthBar (const cUnit& unit, SDL_Rect destination);
	void drawMunBar (const cUnit& unit, SDL_Rect destination);
	void drawStatus (const cUnit& unit, SDL_Rect destination);
};

#endif // gui_game_temp_unitdrawingengineH
