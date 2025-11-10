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

#ifndef utility_positionH
#define utility_positionH

#include "utility/fixedvector.h"

#include <cstdint>
#include <string>

/**
 * Fixed vector class for 2-dimensional integer positions.
 */
class cPosition : public cFixedVector<int, 2>
{
public:
	cPosition()
	{
		x() = 0;
		y() = 0;
	}

	template <class T>
	cPosition (const cFixedVector<T, 2>& other) :
		cFixedVector<int, 2> (other)
	{}

	cPosition (int x_, int y_)
	{
		x() = x_;
		y() = y_;
	}

	cPosition (const cPosition& other) = default;
	cPosition& operator= (const cPosition&) = default;

	cPosition& operator= (const value_type& value)
	{
		cFixedVector<int, 2>::operator= (value);
		return *this;
	}

	// Get a position, relative to current position
	cPosition relative (int x, int y) const
	{
		return cPosition (this->x() + x, this->y() + y);
	}

	uint32_t getChecksum (uint32_t crc) const;

	int x() const
	{
		return (*this)[0];
	}
	int y() const
	{
		return (*this)[1];
	}

	int& x()
	{
		return (*this)[0];
	}
	int& y()
	{
		return (*this)[1];
	}
};

/**
 * Fixed vector class for 2-dimensional float vector.
 */
class cVector2 : public cFixedVector<float, 2>
{
public:
	cVector2()
	{
		x() = 0;
		y() = 0;
	}

	template <class T>
	cVector2 (const cFixedVector<T, 2>& other) :
		cFixedVector<float, 2> (other)
	{}

	cVector2 (float x_, float y_)
	{
		x() = x_;
		y() = y_;
	}

	cVector2 (const cVector2& other) = default;
	cVector2& operator= (const cVector2&) = default;

	cVector2& operator= (const value_type& value)
	{
		cFixedVector<float, 2>::operator= (value);
		return *this;
	}

	// Get a position, relative to current position
	cVector2 relative (float x, float y) const
	{
		return cVector2 (this->x() + x, this->y() + y);
	}

	uint32_t getChecksum (uint32_t crc) const;

	float x() const
	{
		return (*this)[0];
	}
	float y() const
	{
		return (*this)[1];
	}

	float& x()
	{
		return (*this)[0];
	}

	float& y()
	{
		return (*this)[1];
	}
};

inline std::string toString (const cPosition& pos)
{
	return "(" + std::to_string (pos.x()) + ", " + std::to_string (pos.y()) + ")";
}

#endif // utility_positionH
