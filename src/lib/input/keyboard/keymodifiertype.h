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

#ifndef input_keyboard_keymodifiertypeH
#define input_keyboard_keymodifiertypeH

#include "utility/enumflag.h"

enum class eKeyModifierType
{
	None = 0,
	ShiftLeft = (1 << 0),
	ShiftRight = (1 << 1),
	Shift = (1 << 0) | (1 << 1),
	CtrlLeft = (1 << 2),
	CtrlRight = (1 << 3),
	Ctrl = (1 << 2) | (1 << 3),
	AltLeft = (1 << 4),
	AltRight = (1 << 5),
	Alt = (1 << 4) | (1 << 5),
	GuiLeft = (1 << 6),
	GuiRight = (1 << 7),
	Gui = (1 << 6) | (1 << 7),
	Num = (1 << 8),
	Caps = (1 << 9),
	Mode = (1 << 10)
};

using KeyModifierFlags = cEnumFlag<eKeyModifierType>;

#endif // input_keyboard_keymodifiertypeH
