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

#ifndef events_mouseeventsH
#define events_mouseeventsH

#include <SDL.h>

#include "../utility/position.h"
#include "../input/mouse/mousebuttontype.h"

/**
 * Mouse event for mouse motion.
 */
class cEventMouseMotion
{
public:
	cEventMouseMotion(const SDL_MouseMotionEvent& sdlEvent);

	cPosition getNewPosition() const;
private:
	SDL_MouseMotionEvent sdlEvent;
};

/**
 * Mouse event for mouse button clicks.
 */
class cEventMouseButton
{
public:
	enum eType
	{
		Down,
		Up
	};

	cEventMouseButton(const SDL_MouseButtonEvent& sdlEvent);

	eMouseButtonType getButton() const;
	eType getType() const;
private:
	SDL_MouseButtonEvent sdlEvent;
};

/**
 * Mouse event for mouse wheel action.
 */
class cEventMouseWheel
{
public:
	cEventMouseWheel(const SDL_MouseWheelEvent& sdlEvent);

	cPosition getAmount() const;
private:
	SDL_MouseWheelEvent sdlEvent;
};

#endif // events_mouseeventsH
