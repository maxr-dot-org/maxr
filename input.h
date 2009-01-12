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
#ifndef inputH
#define inputH

#include "main.h"
#include "unifonts.h"

/** time in miliseconds how long the cursor will be shown or not shown*/
#define CURSOR_BLINK_TIME	300
/** character used for the cursor*/
#define CURSOR_CHAR			"|"

/** diffrent styles how the cursor will behave during getting and displaying input*/
enum eCursorBehavior
{
	CURSOR_BLINK,
	CURSOR_SHOW,
	CURSOR_DISABLED,
	CURSOR_STANDARD		// the cursor behavior which was overgiven in setInputState() will be used
};

struct sMouseState
{
	bool leftButtonPressed;
	bool rightButtonPressed;
	bool leftButtonHold;
	bool rightButtonHold;
	bool wheelUp;
	bool wheelDown;
};

/**
 * @author alzi alias DoctorDeath
 * Handles keyinput from SDL_evnts and the input string. the string will be formated in UTF-8!
 */
class cInput
{
private:
	/** the current position of the cursor in the input string*/
	unsigned int stringpos;
	/** UTF-8 encoded string with the last input*/
	string inputStr;
	/** true when the user is inputing a string*/
	bool inputactive;
	/** the standard behavior of the cursor*/
	eCursorBehavior cursorBehavior;

	sMouseState MouseState;
	/** true when there has been input since the last check*/
	bool hasBeenInput;
	/** time since the cursor was shown last*/
	int lastShownCursorTime;
	/** true when the cursor will be shown at the moment*/
	bool showingCursor;
	/**
	 * Decodes the character from UTF-16 to UTF-8 and adds him to the input string.
	 * @author alzi alias DoctorDeath
	 * @param ch character which is to be added.
	 */
	void addUTF16Char( Uint16 ch );
public:
	cInput();
	/**
	 * handles the information from a pressed key.
	 * If input is active the character will be added to the inpput string,
	 * else the information will be given to the client to handle hotkeys.
	 * @author alzi alias DoctorDeath
	 * @param keysym SDL_keysym with pressed key information.
	 */
	void inputkey ( SDL_keysym &keysym );
	void inputMouseButton ( SDL_MouseButtonEvent &button );
	/**
	 * sets the input string.
	 * @author alzi alias DoctorDeath
	 * @param input string to which the inputstring should be set.
	 * @param decode true, when the string has to be decoeded to UTF-8. Default is false.
	 */
	void setInputStr( string input, bool decode = false );
	/**
	 * Cuts the input string to overgiven length.
	 * @author alzi alias DoctorDeath
	 * @param length length in pixels which the string is maximal allowed to take.
	 * @param fonttype fonttype in which the string will be shown. default is FONT_LATIN_NORMAL.
	 */
	void cutToLength ( int length, eUnicodeFontType fonttype = FONT_LATIN_NORMAL );
	/**
	 * returns the input string with or without cursor
	 * @author alzi alias DoctorDeath
	 * @param showcursor behavior of the cursor.
	 * @return the input string with or without cursor.
	 */
	string getInputStr( eCursorBehavior showcursor = CURSOR_STANDARD );
	/**
	 * sets the state of inputing strings.
	 * @author alzi alias DoctorDeath
	 * @param active true, when the user wants to input a string.
	 * @param curBehavior behavior of the cursor.
	 */
	void setInputState( bool active, eCursorBehavior curBehavior = CURSOR_BLINK );
	/**
	 * returns the currents input state.
	 * @author alzi alias DoctorDeath
	 */
	bool getInputState();
	/**
	 * returns whether there has been input since the last check.
	 * returns true as well when the cursor behavior is CURSOR_BLINK and the showingstate of the cursor has changed.
	 * @author alzi alias DoctorDeath
	 * @return change status.
	 */
	bool checkHasBeenInput();
};

EX cInput *InputHandler;

#endif
