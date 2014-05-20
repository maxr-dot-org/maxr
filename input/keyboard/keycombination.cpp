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

#include "keycombination.h"
#include "../../utility/trim.h"
#include "../../utility/iequals.h"

static const struct
{
	SDL_Keycode key;
	const char* name;
} keyNames[] = {

	{SDLK_RETURN, "Return"},
	{SDLK_ESCAPE, "Esc"},
	{SDLK_BACKSPACE, "Backspace"},
	{SDLK_TAB, "Tab"},
	{SDLK_SPACE, "Space"},

	{SDLK_EXCLAIM, "!"},
	{SDLK_QUOTEDBL, "\""},
	{SDLK_HASH, "#"},
	{SDLK_PERCENT, "%"},
	{SDLK_DOLLAR, "$"},
	{SDLK_AMPERSAND, "&"},
	{SDLK_QUOTE, "'"},
	{SDLK_LEFTPAREN, "("},
	{SDLK_RIGHTPAREN, ")"},
	{SDLK_ASTERISK, "*"},
	{SDLK_PLUS, "+"},
	{SDLK_COMMA, ","},
	{SDLK_MINUS, "-"},
	{SDLK_PERIOD, "."},
	{SDLK_SLASH, "/"},
	{SDLK_COLON, ":"},
	{SDLK_SEMICOLON, ";"},
	{SDLK_LESS, "<"},
	{SDLK_EQUALS, "="},
	{SDLK_GREATER, ">"},
	{SDLK_QUESTION, "?"},
	{SDLK_AT, "@"},
	{SDLK_LEFTBRACKET, "["},
	{SDLK_BACKSLASH, "\\"},
	{SDLK_RIGHTBRACKET, "]"},
	{SDLK_CARET, "^"},
	{SDLK_UNDERSCORE, "_"},
	{SDLK_BACKQUOTE, "`"},

	{SDLK_0, "0"},
	{SDLK_1, "1"},
	{SDLK_2, "2"},
	{SDLK_3, "3"},
	{SDLK_4, "4"},
	{SDLK_5, "5"},
	{SDLK_6, "6"},
	{SDLK_7, "7"},
	{SDLK_8, "8"},
	{SDLK_9, "9"},

	{SDLK_a, "a"},
	{SDLK_b, "b"},
	{SDLK_c, "c"},
	{SDLK_d, "d"},
	{SDLK_e, "e"},
	{SDLK_f, "f"},
	{SDLK_g, "g"},
	{SDLK_h, "h"},
	{SDLK_i, "i"},
	{SDLK_j, "j"},
	{SDLK_k, "k"},
	{SDLK_l, "l"},
	{SDLK_m, "m"},
	{SDLK_n, "n"},
	{SDLK_o, "o"},
	{SDLK_p, "p"},
	{SDLK_q, "q"},
	{SDLK_r, "r"},
	{SDLK_s, "s"},
	{SDLK_t, "t"},
	{SDLK_u, "u"},
	{SDLK_v, "v"},
	{SDLK_w, "w"},
	{SDLK_x, "x"},
	{SDLK_y, "y"},
	{SDLK_z, "z"},

	{SDLK_F1, "F1"},
	{SDLK_F2, "F2"},
	{SDLK_F3, "F3"},
	{SDLK_F4, "F4"},
	{SDLK_F5, "F5"},
	{SDLK_F6, "F6"},
	{SDLK_F7, "F7"},
	{SDLK_F8, "F8"},
	{SDLK_F9, "F9"},
	{SDLK_F10, "F10"},
	{SDLK_F11, "F11"},
	{SDLK_F12, "F12"},

	{SDLK_PRINTSCREEN, "Print"},
	{SDLK_SCROLLLOCK, "ScrollLock"},
	{SDLK_PAUSE, "Pause"},
	{SDLK_INSERT, "Ins"},
	{SDLK_HOME, "Home"},
	{SDLK_PAGEUP, "PgUp"},
	{SDLK_DELETE, "Del"},
	{SDLK_END, "End"},
	{SDLK_PAGEDOWN, "PgDown"},
	{SDLK_RIGHT, "Right"},
	{SDLK_LEFT, "Left"},
	{SDLK_DOWN, "Down"},
	{SDLK_UP, "Up"},
};

//------------------------------------------------------------------------------
bool cKeyCombination::isRepresentableKey (SDL_Keycode key)
{
	for (size_t i = 0; i < sizeof(keyNames) / sizeof(keyNames[0]); ++i)
	{
		if (keyNames[i].key == key) return true;
	}
	return false;
}

//------------------------------------------------------------------------------
cKeyCombination::cKeyCombination (const std::string& sequence)
{
	std::string::size_type start = 0;
	while (true)
	{
		auto end = sequence.find ('+', start);

		addKey(sequence.substr (start, end - start));

		if (end == std::string::npos) break;

		start += end + 1;
	}
}

//------------------------------------------------------------------------------
cKeyCombination::cKeyCombination (KeyModifierFlags modifiers_, SDL_Keycode key_) :
	modifiers (modifiers_),
	key (key_)
{}

//------------------------------------------------------------------------------
void cKeyCombination::addKey (const std::string& sequence)
{
	auto trimmed = trim_copy (sequence);

	if (iequals (trimmed, "Ctrl"))
	{
		modifiers |= eKeyModifierType::CtrlLeft;
		modifiers |= eKeyModifierType::CtrlRight;
		return;
	}
	else if (iequals (trimmed, "Alt"))
	{
		modifiers |= eKeyModifierType::AltLeft;
		modifiers |= eKeyModifierType::AltRight;
		return;
	}
	else if (iequals (trimmed, "Shift"))
	{
		modifiers |= eKeyModifierType::ShiftLeft;
		modifiers |= eKeyModifierType::ShiftRight;
		return;
	}
	else if (iequals (trimmed, "Num"))
	{
		modifiers |= eKeyModifierType::Num;
		return;
	}
	else
	{
		for (size_t i = 0; i < sizeof(keyNames) / sizeof(keyNames[0]); ++i)
		{
			if (iequals (trimmed, keyNames[i].name))
			{
				key = keyNames[i].key;
				return;
			}
		}
	}

	throw std::runtime_error ("Unknown key name '" + sequence + "'");
}

//------------------------------------------------------------------------------
std::string cKeyCombination::toString () const
{
	std::string result;
	if (modifiers & eKeyModifierType::CtrlLeft ||
		modifiers & eKeyModifierType::CtrlRight)
	{
		result += "Ctrl";
	}

	if (modifiers & eKeyModifierType::AltLeft ||
		modifiers & eKeyModifierType::AltRight)
	{
		if (!result.empty ()) result += "+";
		result += "Alt";
	}

	if (modifiers & eKeyModifierType::ShiftLeft ||
		modifiers & eKeyModifierType::ShiftRight)
	{
		if (!result.empty ()) result += "+";
		result += "Shift";
	}

	if (modifiers & eKeyModifierType::Num)
	{
		if (!result.empty ()) result += "+";
		result += "Num";
	}

	for (size_t i = 0; i < sizeof(keyNames) / sizeof(keyNames[0]); ++i)
	{
		if (key == keyNames[i].key)
		{
			if (!result.empty ()) result += "+";
			result += keyNames[i].name;
			break;
		}
	}

	return result;
}

//------------------------------------------------------------------------------
bool cKeyCombination::operator==(const cKeyCombination& other) const
{
	return modifiers == other.modifiers && key == other.key;
}

//------------------------------------------------------------------------------
bool cKeyCombination::operator!=(const cKeyCombination& other) const
{
	return !(*this == other);
}