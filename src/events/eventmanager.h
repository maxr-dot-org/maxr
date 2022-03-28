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

#ifndef events_eventmanagerH
#define events_eventmanagerH

#include <SDL.h>

#include "utility/signal/signal.h"

class cEventMouseMotion;
class cEventMouseButton;
class cEventMouseWheel;

class cKeyboardEvent;
class cTextInputEvent;

/**
 * Global event manager.
 *
 * This event manager is the bridge between SDL and the game itself.
 *
 * In all game loops of the main thread the @ref run method of
 * the singleton instance of this class has to be called to make sure
 * all the external events (as for the devices like mouse and keyboard)
 * can be handles correctly.
 */
class cEventManager
{
public:
	/**
	 * Returns the one and only instance of this class.
	 * @return The singleton instance.
	 */
	static cEventManager& getInstance();

	/**
	 * Polls the SDL event cue and handles all incoming events.
	 */
	void run();

	cSignal<void (const cEventMouseMotion&)> mouseMotionEvent;
	cSignal<void (const cEventMouseButton&)> mouseButtonEvent;
	cSignal<void (const cEventMouseWheel&)> mouseWheelEvent;

	cSignal<void (const cKeyboardEvent&)> keyboardEvent;
	cSignal<void (const cTextInputEvent&)> textInputEvent;

	bool shouldExit() const;
private:
	cEventManager();
	cEventManager (const cEventManager&) = delete;
	cEventManager& operator= (const cEventManager&) = delete;

	bool handleSdlEvent (const SDL_Event& event);

	bool isDone = false;
};

#endif // events_eventmanagerH
