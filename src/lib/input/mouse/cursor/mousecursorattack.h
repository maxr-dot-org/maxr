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

#ifndef input_mouse_cursor_mousecursorattackH
#define input_mouse_cursor_mousecursorattackH

#include "SDLutility/autosurface.h"
#include "input/mouse/cursor/mousecursor.h"

class cUnit;
class cPosition;
class cMapView;

class cMouseCursorAttack : public cMouseCursor
{
public:
	cMouseCursorAttack();
	cMouseCursorAttack (const cUnit& sourceUnit, const cPosition& targetPosition, const cMapView&);
	cMouseCursorAttack (int currentHealthPercent, int newHealthPercent, bool inRange);

	SDL_Surface* getSurface() const override;

	cPosition getHotPoint() const override;

protected:
	bool equal (const cMouseCursor& other) const override;

private:
	int currentHealthPercent;
	int newHealthPercent;

	bool inRange;

	mutable UniqueSurface surface;

	void generateSurface() const;
};

#endif // input_mouse_cursor_mousecursorattackH
