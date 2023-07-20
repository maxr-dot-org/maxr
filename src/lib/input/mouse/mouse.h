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

#ifndef input_mouse_mouseH
#define input_mouse_mouseH

#include "input/mouse/mousebuttontype.h"
#include "input/mouse/mousecursortype.h"
#include "utility/position.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <SDL.h>
#include <chrono>
#include <map>
#include <memory>

class cEventMouseMotion;
class cEventMouseButton;
class cEventMouseWheel;

class cMouseCursor;

/**
 * A class to handle a mouse device.
 *
 * The mouse provides signals for button clicks, moving and mouse wheel events.
 */
class cMouse
{
public:
	/**
	 * Creates a new mouse object and connects the mouse to the global
	 * event manager to get the system events the mouse needs.
	 *
	 * Since currently we have a single global mouse instance, I see no reason why
	 * we should ever create another mouse instance.
	 */
	cMouse();
	cMouse (const cMouse&) = delete;
	cMouse& operator= (const cMouse&) = delete;

	~cMouse();

	/**
	 * Returns the global instance of the mouse.
	 *
	 * Note: This may should be removed and everywhere where the mouse needs to be accessed an instance
	 *       of the a mouse object should be passed to.
	 *
	 * @return The one global instance of the mouse.
	 */
	static cMouse& getInstance();

	/**
	 * Signal that will be triggered when a mouse button gets pressed.
	 */
	cSignal<void (cMouse&, eMouseButtonType)> pressed;

	/**
	 * Signal that will be triggered when a mouse button gets released.
	 */
	cSignal<void (cMouse&, eMouseButtonType)> released;

	/**
	 * Signal that will be triggered when the mouse wheel is moved.
	 */
	cSignal<void (cMouse&, const cPosition& amount)> wheelMoved;

	/**
	 * Signal that will be triggered when the mouse is moved.
	 */
	cSignal<void (cMouse&, const cPosition& offset)> moved;

	/**
	 * The current position of the mouse.
	 * @return The position on the screen (in pixels).
	 */
	const cPosition& getPosition() const;

	/**
	 * Sets the cursor of the mouse.
	 *
	 * @param cursor The new cursor to set.
	 * @param force Force setting the cursor, even if the mouse internally thinks
	 *              that the same cursor is currently set.
	 * @return True if the cursor has been reset.
	 *         False if the same cursor has been set already and force has been false.
	 */
	bool setCursor (std::unique_ptr<cMouseCursor> cursor, bool force = false);

	/**
	 * Returns the current cursor.
	 * @return The cursor.
	 */
	const cMouseCursor* getCursor() const;

	/**
	 * Returns whether a mouse button is currently pressed.
	 *
	 * @param button The button to check.
	 * @return True if the passed button is pressed. Else false.
	 */
	bool isButtonPressed (eMouseButtonType button) const;

	/**
	 * Returns how often a button has been clicked consecutively in a short defined time
	 * period.
	 *
	 * The value returned by this function is only reliable during the execution of
	 * functions connected to the @ref pressed signal.
	 *
	 * @param button The button to check the click count for.
	 * @return The number of times the button has been clicked consecutively.
	 */
	unsigned int getButtonClickCount (eMouseButtonType button) const;

	/**
	 * Enables showing the mouse cursor on the display.
	 */
	void show();

	/**
	 * Hides the mouse cursor from the display.
	 */
	void hide();

private:
	void handleMouseMotionEvent (const cEventMouseMotion&);
	void handleMouseButtonEvent (const cEventMouseButton&);
	void handleMouseWheelEvent (const cEventMouseWheel&);

private:
	struct SdlCursorDeleter
	{
		void operator() (SDL_Cursor* cursor) const { SDL_FreeCursor (cursor); }
	};
	using SdlCursorPtrType = std::unique_ptr<SDL_Cursor, SdlCursorDeleter>;

	cSignalConnectionManager signalConnectionManager;

	cPosition position;
	std::unique_ptr<cMouseCursor> cursor;

	std::map<eMouseButtonType, bool> buttonPressedState;
	std::map<eMouseButtonType, unsigned int> buttonClickCount;
	std::map<eMouseButtonType, std::chrono::steady_clock::time_point> lastClickTimes;

	SdlCursorPtrType sdlCursor;
};

#endif // input_mouse_mouseH
