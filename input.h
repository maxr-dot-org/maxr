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

struct sMouseState
{
	sMouseState();
	bool leftButtonPressed;
	bool rightButtonPressed;
	bool leftButtonReleased;
	bool rightButtonReleased;
	bool wheelUp;
	bool wheelDown;
	bool isDoubleClick;
};

/**
 * @author alzi alias DoctorDeath
 * Handles keyinput from SDL_evnts
 */
class cInput
{
private:

	sMouseState MouseState;
	/**
	 * Decodes the character from UTF-16 to UTF-8 and adds him to the input string.
	 * @author alzi alias DoctorDeath
	 * @param ch character which is to be added.
	 */
	string getUTF16Char( Uint16 ch );
public:
	cInput();
	/**
	 * handles the information from a pressed key.
	 * If input is active the character will be added to the inpput string,
	 * else the information will be given to the client to handle hotkeys.
	 * @author alzi alias DoctorDeath
	 * @param keysym SDL_keysym with pressed key information.
	 */
	void inputkey ( SDL_KeyboardEvent &key );
	 
	long LastClickTicks;
	void inputMouseButton ( SDL_MouseButtonEvent &button );
	bool IsDoubleClicked (void);
};

EX cInput *InputHandler;

#endif
