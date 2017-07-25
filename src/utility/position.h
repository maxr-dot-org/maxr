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

#include <stdint.h>

#include "utility/fixedvector.h"
#include "serialization/serialization.h"

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

	cPosition (const cPosition& other) :
		cFixedVector<int, 2> (other)
	{}
	cPosition (const cFixedVector<int, 2>& other) :
		cFixedVector<int, 2> (other)
	{}
	cPosition (int x_, int y_)
	{
		x() = x_;
		y() = y_;
	}

	uint32_t getChecksum(uint32_t crc) const;

	template<typename T>
	void serialize(T& archive)
	{
		archive & serialization::makeNvp("X", (*this)[0]);
		archive & serialization::makeNvp("Y", (*this)[1]);
	}
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

	cPosition& operator= (const value_type& value)
	{
		cFixedVector<int, 2>::operator= (value);
		return *this;
	}
};

#endif // utility_positionH
