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

#include "events/mouseevents.h"

//------------------------------------------------------------------------------
cEventMouseMotion::cEventMouseMotion (const SDL_MouseMotionEvent& sdlEvent_) :
	sdlEvent (sdlEvent_)
{}

//------------------------------------------------------------------------------
cPosition cEventMouseMotion::getNewPosition() const
{
	return cPosition (sdlEvent.x, sdlEvent.y);
}

//------------------------------------------------------------------------------
cPosition cEventMouseMotion::getOffset() const
{
	return cPosition (sdlEvent.xrel, sdlEvent.yrel);
}

//------------------------------------------------------------------------------
cEventMouseButton::cEventMouseButton (const SDL_MouseButtonEvent& sdlEvent_) :
	sdlEvent (sdlEvent_)
{}

//------------------------------------------------------------------------------
eMouseButtonType cEventMouseButton::getButton() const
{
	if (sdlEvent.button == SDL_BUTTON_RIGHT)
	{
		return eMouseButtonType::Right;
	}
	else if (sdlEvent.button == SDL_BUTTON_MIDDLE)
	{
		return eMouseButtonType::Middle;
	}
	else
	{
		return eMouseButtonType::Left;
	}
}

//------------------------------------------------------------------------------
cEventMouseButton::eType cEventMouseButton::getType() const
{
	return sdlEvent.type == SDL_MOUSEBUTTONUP ? eType::Up : eType::Down;
}

//------------------------------------------------------------------------------
cEventMouseWheel::cEventMouseWheel (const SDL_MouseWheelEvent& sdlEvent_) :
	sdlEvent (sdlEvent_)
{}

//------------------------------------------------------------------------------
cPosition cEventMouseWheel::getAmount() const
{
	return cPosition (sdlEvent.x, sdlEvent.y);
}
