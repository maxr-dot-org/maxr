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

#ifndef utility_directionH
#define utility_directionH

#include "utility/tounderlyingtype.h"

#include <optional>

class cPosition;

enum class EDirection
{
	North = 0,
	NorthEast = 1,
	East = 2,
	SouthEast = 3,
	South = 4,
	SouthWest = 5,
	West = 6,
	NorthWest = 7
};

//-----------------------------------------------------------------------------
inline EDirection& operator++ (EDirection& dir)
{
	return dir = EDirection ((toUnderlyingType (dir) + 1) % 8);
}

//-----------------------------------------------------------------------------
inline EDirection& operator-- (EDirection& dir)
{
	return dir = EDirection ((toUnderlyingType (dir) - 1 + 8) % 8);
}

std::optional<EDirection> directionFromOffset (const cPosition& offset);

cPosition offsetFromDirection (EDirection);

#endif
