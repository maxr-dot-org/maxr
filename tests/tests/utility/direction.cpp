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

#include "utility/direction.h"

#include "utility/position.h"

#include <doctest.h>

#if 1 // TODO: move it in appropriate header and fix conflict with toString
namespace doctest
{
	template <typename T>
	struct StringMaker<std::optional<T>>
	{
		static String convert (const std::optional<T>& value)
		{
			if (value)
			{
				return toString (*value);
			}
			return "std::nullopt";
		}
	};

	template <>
	struct StringMaker<cPosition>
	{
		static String convert (const cPosition& value)
		{
			String res = "{";
			res += toString (value.x());
			res += ", ";
			res += toString (value.y());
			res += "}";
			return res;
		}
	};
} // namespace doctest
#endif

//------------------------------------------------------------------------------
TEST_CASE ("IncrementDirection")
{
	EDirection dir = EDirection::North;

	CHECK (++dir == EDirection::NorthEast);
	CHECK (++dir == EDirection::East);
	CHECK (++dir == EDirection::SouthEast);
	CHECK (++dir == EDirection::South);
	CHECK (++dir == EDirection::SouthWest);
	CHECK (++dir == EDirection::West);
	CHECK (++dir == EDirection::NorthWest);
	CHECK (++dir == EDirection::North);
}
//------------------------------------------------------------------------------
TEST_CASE ("DecrementDirection")
{
	EDirection dir = EDirection::North;

	CHECK (--dir == EDirection::NorthWest);
	CHECK (--dir == EDirection::West);
	CHECK (--dir == EDirection::SouthWest);
	CHECK (--dir == EDirection::South);
	CHECK (--dir == EDirection::SouthEast);
	CHECK (--dir == EDirection::East);
	CHECK (--dir == EDirection::NorthEast);
	CHECK (--dir == EDirection::North);
}

//------------------------------------------------------------------------------
TEST_CASE ("directionFromOffset")
{
	CHECK (directionFromOffset ({0, 0}) == std::nullopt);

	CHECK (directionFromOffset ({0, -1}) == EDirection::North);
	CHECK (directionFromOffset ({1, -1}) == EDirection::NorthEast);
	CHECK (directionFromOffset ({1, 0}) == EDirection::East);
	CHECK (directionFromOffset ({1, 1}) == EDirection::SouthEast);
	CHECK (directionFromOffset ({0, 1}) == EDirection::South);
	CHECK (directionFromOffset ({-1, 1}) == EDirection::SouthWest);
	CHECK (directionFromOffset ({-1, 0}) == EDirection::West);
	CHECK (directionFromOffset ({-1, -1}) == EDirection::NorthWest);

	CHECK (directionFromOffset ({-1, -3}) == EDirection::North);
	CHECK (directionFromOffset ({0, -3}) == EDirection::North);
	CHECK (directionFromOffset ({1, -3}) == EDirection::North);
	CHECK (directionFromOffset ({2, -3}) == EDirection::NorthEast);
	CHECK (directionFromOffset ({3, -3}) == EDirection::NorthEast);
}

//------------------------------------------------------------------------------
TEST_CASE ("offsetFromDirection")
{
	// TODO: handle `toString` conflict between doctest and our own version
	CHECK (static_cast<bool> (offsetFromDirection (EDirection::North) == cPosition (0, -1)));
	CHECK (static_cast<bool> (offsetFromDirection (EDirection::NorthEast) == cPosition (1, -1)));
	CHECK (static_cast<bool> (offsetFromDirection (EDirection::East) == cPosition (1, 0)));
	CHECK (static_cast<bool> (offsetFromDirection (EDirection::SouthEast) == cPosition (1, 1)));
	CHECK (static_cast<bool> (offsetFromDirection (EDirection::South) == cPosition (0, 1)));
	CHECK (static_cast<bool> (offsetFromDirection (EDirection::SouthWest) == cPosition (-1, 1)));
	CHECK (static_cast<bool> (offsetFromDirection (EDirection::West) == cPosition (-1, 0)));
	CHECK (static_cast<bool> (offsetFromDirection (EDirection::NorthWest) == cPosition (-1, -1)));
}
