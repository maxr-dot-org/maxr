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
#include "input.h"
#include "menus.h"

sMouseState::sMouseState()
{
	leftButtonPressed = false;
	rightButtonPressed = false;
	leftButtonReleased = false;
	rightButtonReleased = false;
	wheelUp = false;
	wheelDown = false;
	isDoubleClick = false;
	x = 0;
	y = 0;
}

cInput::cInput() :
	LastClickTicks (0)
{
}

bool cInput::IsDoubleClicked()
{
	const long CurrentClickTicks = SDL_GetTicks();

	// First time this function is called,
	// LastClickTicks has not been initialised yet.
	if (! LastClickTicks)
	{
		LastClickTicks = CurrentClickTicks;
		return false;
	}

	// If the period between the two clicks is smaller
	// or equal to a pre-defined number,
	// we report a DoubleClick event.
	if (CurrentClickTicks - LastClickTicks <= 500)
	{
		// Update LastClickTicks and signal a DoubleClick.

		LastClickTicks = CurrentClickTicks;
		return true;
	}

	// Update LastClickTicks and signal a SingleClick.
	LastClickTicks = CurrentClickTicks;
	return false;
}

void cInput::inputMouseButton (cMenu& activeMenu, const SDL_MouseButtonEvent& button)
{
	MouseState.wheelUp = false;
	MouseState.wheelDown = false;
	MouseState.x = button.x;
	MouseState.y = button.y;

	if (button.state == SDL_PRESSED)
	{
		if (button.button == SDL_BUTTON_LEFT)
		{
			MouseState.leftButtonPressed = true;
			MouseState.leftButtonReleased = false;
			MouseState.rightButtonReleased = false;
		}
		else if (button.button == SDL_BUTTON_RIGHT)
		{
			MouseState.rightButtonPressed = true;
			MouseState.rightButtonReleased = false;
			MouseState.leftButtonReleased = false;
		}
		MouseState.isDoubleClick = IsDoubleClicked();
	}
	else if (button.state == SDL_RELEASED)
	{
		if (button.button == SDL_BUTTON_LEFT)
		{
			MouseState.leftButtonPressed = false;
			MouseState.leftButtonReleased = true;
			MouseState.rightButtonReleased = false;
		}
		else if (button.button == SDL_BUTTON_RIGHT)
		{
			MouseState.rightButtonPressed = false;
			MouseState.rightButtonReleased = true;
			MouseState.leftButtonReleased = false;
		}
	}

	// do not send events to a menu, after an event triggered the termination
	// the user wouldn't expects the menu to execute further events
	// after clicking the exit button
	if (!activeMenu.exiting())
	{
		activeMenu.handleMouseInput (MouseState);
	}
}

void cInput::inputMouseButton (cMenu& activeMenu, const SDL_MouseWheelEvent& wheel)
{
	MouseState.leftButtonReleased = false;
	MouseState.rightButtonReleased = false;
	MouseState.wheelUp = wheel.y > 0;
	MouseState.wheelDown = wheel.y < 0;

	// do not send events to a menu, after an event triggered the termination
	// the user wouldn't expects the menu to execute further events
	// after clicking the exit button
	if (!activeMenu.exiting())
	{
		activeMenu.handleMouseInput (MouseState);
	}
}
