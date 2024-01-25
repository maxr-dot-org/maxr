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

#include "events/eventmanager.h"

#include "events/keyboardevents.h"
#include "events/mouseevents.h"
#include "output/video/video.h"

#include <SDL.h>
#include <algorithm>

//------------------------------------------------------------------------------
cEventManager::cEventManager()
{}

//------------------------------------------------------------------------------
cEventManager& cEventManager::getInstance()
{
	static cEventManager instance;
	return instance;
}

//------------------------------------------------------------------------------
bool cEventManager::shouldExit() const
{
	return isDone;
}

//------------------------------------------------------------------------------
void cEventManager::run()
{
	SDL_Event event;
	while (SDL_PollEvent (&event))
	{
		if (!handleSdlEvent (event))
		{
			isDone = true;
			break;
		}
	}
	SDL_Delay (1);
}

//------------------------------------------------------------------------------
bool cEventManager::handleSdlEvent (const SDL_Event& event)
{
	switch (event.type)
	{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			keyboardEvent (cKeyboardEvent (event.key));
			break;
		case SDL_TEXTINPUT:
			textInputEvent (cTextInputEvent (event.text));
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			// NOTE: we do not handle all mouse buttons yet.
			if (event.button.button != SDL_BUTTON_LEFT && event.button.button != SDL_BUTTON_RIGHT && event.button.button != SDL_BUTTON_MIDDLE) break;
			mouseButtonEvent (cEventMouseButton (event.button));
			break;
		case SDL_MOUSEWHEEL:
			mouseWheelEvent (cEventMouseWheel (event.wheel));
			break;
		case SDL_QUIT:
			return false;
			break;
		case SDL_MOUSEMOTION:
			mouseMotionEvent (cEventMouseMotion (event.motion));
			break;
		case SDL_WINDOWEVENT:
			Video.draw();
			break;
		default: break;
	}
	// Everything is OK, continue the cycle
	return true;
}
