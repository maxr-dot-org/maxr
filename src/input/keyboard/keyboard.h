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

#ifndef input_keyboard_keyboardH
#define input_keyboard_keyboardH

#include <SDL.h>

#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"
#include "input/keyboard/keymodifiertype.h"

class cKeyboardEvent;
class cTextInputEvent;

/**
 * A class to handle a keyboard device.
 *
 * The keyboard provides signals for key pressing/releasing and text input.
 */
class cKeyboard
{
public:
	/**
	 * Creates a new keyboard object and connects the keyboard to the global
	 * event manager to get the system events the keyboard needs.
	 *
	 * Since currently we have a single global keyboard instance, I see no reason why
	 * we should ever create another keyboard instance.
	 */
	cKeyboard();

	cKeyboard (const cKeyboard&) = delete;
	cKeyboard& operator= (const cKeyboard&) = delete;
	/**
	 * Returns the global instance of the keyboard.
	 *
	 * Note: This may should be removed and everywhere where the mouse needs to be accessed an instance
	 *       of the a mouse object should be passed to.
	 *
	 * @return The one global instance of the mouse.
	 */
	static cKeyboard& getInstance();

	cSignal<void (cKeyboard&, SDL_Keycode)> keyPressed;
	cSignal<void (cKeyboard&, SDL_Keycode)> keyReleased;
	cSignal<void()> modifierChanged;
	cSignal<void (cKeyboard&, const char*)> textEntered;

	/**
	 * Returns the flags of all the modifiers that are currently active
	 * on the keyboard.
	 *
	 * @return The currently active modifiers.
	 */
	KeyModifierFlags getCurrentModifiers() const;

	/**
	 * Checks whether any of the set modifier flags is currently active.
	 *
	 * @param flags Flags to check against.
	 * @return True if any of the passed modifier flags is active.
	 */
	bool isAnyModifierActive (KeyModifierFlags flags) const;

	/**
	 * Checks whether all of the set modifier flags are currently active.
	 *
	 * @param flags Flags to check against.
	 * @return True if all of the passed modifier flags are active.
	 */
	bool isAllModifiersActive (KeyModifierFlags flags) const;
private:

	cSignalConnectionManager signalConnectionManager;

	void updateModifiersFromKeyPress (const cKeyboardEvent&);
	void handleKeyboardEvent (const cKeyboardEvent&);
	void handleTextInputEvent (const cTextInputEvent&);

	KeyModifierFlags currentModifiers;
};

#endif // input_keyboard_keyboardH
