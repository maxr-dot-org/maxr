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
#include "client.h"

cInput::cInput()
{
	// enables that SDL puts the unicode values to the keyevents.
	SDL_EnableUNICODE ( 1 );
	// enables keyrepetition
	SDL_EnableKeyRepeat ( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL );

	stringpos = 0;
	inputactive = false;
	hasBeenInput = false;
	lastShownCursorTime = 0;

	MouseState.leftButtonPressed = false;
	MouseState.rightButtonPressed = false;
	MouseState.leftButtonHold = false;
	MouseState.rightButtonHold = false;
	MouseState.wheelUp = false;
	MouseState.wheelDown = false;
}

void cInput::inputkey ( SDL_keysym &keysym )
{
	// if input is active write the characters to the inputstring
	if ( inputactive )
	{
		if ( keysym.unicode >= 32 )
		{
			// write to inputstr when it is a normal character
			addUTF16Char ( keysym.unicode );
		}
		else
		{
			// special behaviour for other keys
			switch ( keysym.sym )
			{
			case SDLK_RETURN:
				// return will be handled from the client like a hotkey
				if ( Client ) Client->handleHotKey ( keysym );
				break;
			case SDLK_LEFT:
				// makes the cursor go left
				if ( stringpos > 0 )
				{
					unsigned char ch = ((unsigned char*)(inputStr.c_str()))[stringpos-1];
					while ( ((ch & 0xE0) != 0xE0) && ((ch & 0xC0) != 0xC0) && ((ch & 0x80) == 0x80) )
					{
						stringpos--;
						ch = ((unsigned char*)(inputStr.c_str()))[stringpos-1];
					}
					stringpos--;
				}
				break;
			case SDLK_RIGHT:
				// makes the cursor go right
				if ( stringpos < (int)inputStr.length() )
				{
					unsigned char ch = ((unsigned char*)(inputStr.c_str()))[stringpos];
					if ( (ch & 0xE0) == 0xE0 ) stringpos += 3;
					else if ( (ch & 0xC0) == 0xC0 ) stringpos += 2;
					else  stringpos += 1;
				}
				break;
			case SDLK_BACKSPACE:
				// deletes the first character left from the cursor
				if ( stringpos > 0 )
				{
					unsigned char ch = ((unsigned char*)(inputStr.c_str()))[stringpos-1];
					while ( ((ch & 0xE0) != 0xE0) && ((ch & 0xC0) != 0xC0) && ((ch & 0x80) == 0x80) )
					{
						inputStr.erase ( stringpos-1, 1 );
						stringpos--;
						ch = ((unsigned char*)(inputStr.c_str()))[stringpos-1];
					}
					inputStr.erase ( stringpos-1, 1 );
					stringpos--;
				}
				break;
			case SDLK_DELETE:
				// deletes the first character right from the cursor
				if ( stringpos < (int)inputStr.length() )
				{
					unsigned char ch = ((unsigned char*)(inputStr.c_str()))[stringpos];
					if ( (ch & 0xE0) == 0xE0 )inputStr.erase ( stringpos, 3 );
					else if ( (ch & 0xC0) == 0xC0 )inputStr.erase ( stringpos, 2 );
					else inputStr.erase ( stringpos, 1 );
				}
				break;
			}
		}
		hasBeenInput = true;
	}
	else
	{
		// when input isn't active the client will handle the input as hotkey
		if ( Client ) Client->handleHotKey ( keysym );
	}
}

void cInput::inputMouseButton ( SDL_MouseButtonEvent &button )
{
	if ( button.state == SDL_PRESSED )
	{
		if ( button.button == SDL_BUTTON_LEFT )
		{
			MouseState.leftButtonPressed = true;
			if ( MouseState.rightButtonPressed )
			{
				MouseState.rightButtonHold = true;
				MouseState.rightButtonPressed = false;
			}
		}
		else if ( button.button == SDL_BUTTON_RIGHT )
		{
			MouseState.rightButtonPressed = true;
			if ( MouseState.leftButtonPressed )
			{
				MouseState.leftButtonHold = true;
				MouseState.leftButtonPressed = false;
			}
		}
		else if ( button.button == SDL_BUTTON_WHEELUP ) MouseState.wheelUp = true;
		else if ( button.button == SDL_BUTTON_WHEELDOWN ) MouseState.wheelDown = true;
	}
	else if ( button.state == SDL_RELEASED )
	{
		if ( button.button == SDL_BUTTON_LEFT )
		{
			MouseState.leftButtonPressed = false;
			MouseState.leftButtonHold = false;
		}
		else if ( button.button == SDL_BUTTON_RIGHT )
		{
			MouseState.rightButtonPressed = false;
			MouseState.rightButtonHold = false;
		}
		else if ( button.button == SDL_BUTTON_WHEELUP ) MouseState.wheelUp = false;
		else if ( button.button == SDL_BUTTON_WHEELDOWN ) MouseState.wheelDown = false;
	}
	if ( Client ) Client->handleMouseInput ( MouseState );
}

void cInput::addUTF16Char( Uint16 ch )
{
	int count;
	Uint32 bitmask;

	// convert from UTF-16 to UTF-8
	count = 1;
	if( ch >= 0x80 ) count++;

	bitmask = 0x800;
	for( int i = 0; i < 5; i++ )
	{
		if( (Uint32)ch >= bitmask ) count++;
		bitmask <<= 5;
	}

	if( count == 1 )
	{
		string str;
		str += (char)ch;
		inputStr.insert( stringpos, str );
		stringpos++;
	}
	else
	{
		for( int i = count-1; i >= 0; i-- )
		{
			unsigned char c = (ch >> (6*i)) & 0x3f;
			c |= 0x80;
			if( i == count-1 ) c |= 0xff << (8-count);
			string str;
			str += c;
			inputStr.insert( stringpos, str );
			stringpos++;
		}
	}
}

void cInput::setInputStr( string input, bool decode )
{
	if ( decode )
	{
		// decode the string
		wstring wstr ( input.begin(), input.end() );
		inputStr = "";
		stringpos = 0;
		for ( unsigned int i = 0; i < wstr.length(); i++ )
		{
			addUTF16Char ( (Uint16)wstr[i] );
		}
	}
	else inputStr = input;
	stringpos = (unsigned int)inputStr.length();
}

void cInput::cutToLength ( int length, eUnicodeFontType fonttype )
{
	if ( length < 0 ) return;
	while ( font->getTextWide( inputStr, fonttype ) > length )
	{
		// erase the last character
		unsigned char ch = ((unsigned char*)(inputStr.c_str()))[inputStr.length()-1];
		while ( ((ch & 0xE0) != 0xE0) && ((ch & 0xC0) != 0xC0) && ((ch & 0x80) == 0x80) )
		{
			inputStr.erase ( inputStr.length()-1, 1 );
			stringpos--;
			ch = ((unsigned char*)(inputStr.c_str()))[inputStr.length()-1];
		}
		inputStr.erase ( inputStr.length()-1, 1 );
		stringpos--;
	}
}

string cInput::getInputStr( eCursorBehavior showcursor )
{
	string returnstring = inputStr;
	// use standard cursor behavior when standard was overgiven
	if ( showcursor == CURSOR_STANDARD ) showcursor = cursorBehavior;
	switch ( showcursor )
	{
	case CURSOR_BLINK:
		// handles the blinkstate and adds the cursor if necessary
		if ( SDL_GetTicks()-lastShownCursorTime > CURSOR_BLINK_TIME || showingCursor )
		{
			if ( !showingCursor )
				showingCursor = true;
			else if ( SDL_GetTicks()-lastShownCursorTime > CURSOR_BLINK_TIME )
				showingCursor = false;
			if ( showingCursor ) returnstring.insert( stringpos, CURSOR_CHAR );
			if ( SDL_GetTicks()-lastShownCursorTime > CURSOR_BLINK_TIME )
				lastShownCursorTime = SDL_GetTicks();
		}
		break;
	case CURSOR_SHOW:
		// adds the cursor
		returnstring.insert( stringpos, CURSOR_CHAR );
		break;
	}
	return returnstring;
}

void cInput::setInputState( bool active, eCursorBehavior curBehavior )
{
	inputactive = active;
	if ( !active )
	{
		// disable cursor when deactivating input
		hasBeenInput = true;
		cursorBehavior = CURSOR_DISABLED;
	}
	else
	{
		hasBeenInput = false;
		cursorBehavior = curBehavior;
	}
}

bool cInput::getInputState( )
{
	return inputactive;
}

bool cInput::checkHasBeenInput()
{
	if ( hasBeenInput || ( SDL_GetTicks()-lastShownCursorTime > CURSOR_BLINK_TIME && inputactive ) )
	{
		hasBeenInput = false;
		return true;
	}
	return false;
}
