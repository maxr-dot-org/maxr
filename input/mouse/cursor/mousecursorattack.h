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

#include "mousecursor.h"
#include "../../../maxrconfig.h"
#include "../../../autosurface.h"

class cMouseCursorAttack : public cMouseCursor
{
public:
	cMouseCursorAttack ();
	cMouseCursorAttack (int currentHealthPercent_, int newHealthPercent_);

	virtual SDL_Surface* getSurface () const MAXR_OVERRIDE_FUNCTION;

	virtual cPosition getHotPoint () const MAXR_OVERRIDE_FUNCTION;

protected:
	virtual bool equal (const cMouseCursor& other) const MAXR_OVERRIDE_FUNCTION;

private:
	int currentHealthPercent;
	int newHealthPercent;

	mutable AutoSurface surface;

	void generateSurface () const;
};

#endif // input_mouse_cursor_mousecursorattackH
