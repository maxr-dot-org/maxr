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
#include "unifonts.h"
#include "client.h"

cInput::cInput()
{
	SDL_EnableUNICODE ( 1 );
	SDL_EnableKeyRepeat ( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL );

	stringpos = 0;
	inputactive = false;
	hasBeenInput = false;
	lastShownCursorTime = 0;
}

void cInput::inputkey ( SDL_keysym &keysym )
{
	if ( inputactive )
	{
		// write to inputstr when it is a normal character
		if ( keysym.unicode >= 32 )
		{
			addUTF16Char ( keysym.unicode );
		}
		else
		{
			// special behaviour
			switch ( keysym.sym )
			{
			case SDLK_RETURN:
				if ( Client ) Client->handleHotKey ( keysym );
				break;
			case SDLK_LEFT:
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
				if ( stringpos < (int)inputStr.length() )
				{
					unsigned char ch = ((unsigned char*)(inputStr.c_str()))[stringpos];
					if ( (ch & 0xE0) == 0xE0 ) stringpos += 3;
					else if ( (ch & 0xC0) == 0xC0 ) stringpos += 2;
					else  stringpos += 1;
				}
				break;
			case SDLK_BACKSPACE:
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
		if ( Client ) Client->handleHotKey ( keysym );
	}
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

void cInput::cutToLength ( int length )
{
	if ( length < 0 ) return;
	while ( font->getTextWide( inputStr ) > length )
	{
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
	if ( showcursor == CURSOR_STANDARD ) showcursor = cursorBehavior;
	switch ( showcursor )
	{
	case CURSOR_BLINK:
		if ( SDL_GetTicks()-lastShownCursorTime > CURSOR_BLINK_TIME || showingCursor )
		{
			if ( !showingCursor )
				showingCursor = true;
			else if ( SDL_GetTicks()-lastShownCursorTime > CURSOR_BLINK_TIME )
				showingCursor = false;
			if ( showingCursor ) returnstring.insert( stringpos, "|" );
			if ( SDL_GetTicks()-lastShownCursorTime > CURSOR_BLINK_TIME )
				lastShownCursorTime = SDL_GetTicks();
		}
		break;
	case CURSOR_SHOW:
		returnstring.insert( stringpos, "|" );
		break;
	}
	return returnstring;
}

void cInput::setInputState( bool active, eCursorBehavior curBehavior )
{
	inputactive = active;
	if ( !active )
	{
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
