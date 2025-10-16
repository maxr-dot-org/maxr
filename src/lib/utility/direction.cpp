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

#include "direction.h"

#include "utility/position.h"

#include <numbers>

namespace
{

	//--------------------------------------------------------------------------
	EDirection degreeToDirection (double angle)
	{
		if (angle < 0)
		{
			angle = 360. + fmod (angle, 360.);
		}
		if (angle >= 360.)
		{
			angle = 360. + fmod (angle, 360.);
		}
		if (angle <= 22.5)
			return EDirection::North;
		else if (angle <= 67.5)
			return EDirection::NorthEast;
		else if (angle <= 112.5)
			return EDirection::East;
		else if (angle <= 157.5)
			return EDirection::SouthEast;
		else if (angle <= 202.5)
			return EDirection::South;
		else if (angle <= 247.5)
			return EDirection::SouthWest;
		else if (angle <= 292.5)
			return EDirection::West;
		else if (angle <= 337.5)
			return EDirection::NorthWest;
		else
			return EDirection::North;
	}

	//--------------------------------------------------------------------------
	EDirection radianToDirection (double angle)
	{
		constexpr auto degree_by_radian = 360. / (2 * std::numbers::pi); // 57.29577951f;
		return degreeToDirection (angle * degree_by_radian);
	}

} // namespace

//------------------------------------------------------------------------------
std::optional<EDirection> directionFromOffset (const cPosition& offset)
{
	if (offset == cPosition{0, 0}) { return std::nullopt; }

	return radianToDirection (atan2 ((double) offset.x(), (double) -offset.y()));
}

//------------------------------------------------------------------------------
cPosition offsetFromDirection (EDirection direction)
{
	static const cPosition offsets[8] = {
		{0, -1},
		{1, -1},
		{1, 0},
		{1, 1},
		{0, 1},
		{-1, 1},
		{-1, 0},
		{-1, -1}};

	return offsets[toUnderlyingType (direction)];
}
