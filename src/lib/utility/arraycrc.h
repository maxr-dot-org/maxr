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

#ifndef utility_arraycrcH
#define utility_arraycrcH

#include "crc.h"

#include <cstdint>
#include <optional>
#include <vector>

/**
* This is a wrapper class around a dynamically allocated array.
* The purpose of this wrapper is to cache the crc value of the array,
* and only recalculate it,
* when the array was modified after the last call to getChecksum().
*/
template <typename T>
class cArrayCrc
{
public:
	cArrayCrc() = default;

	void resize (size_t size, T initialValue);
	void fill (T value);

	const T& operator[] (size_t x) const { return data[x]; }
	size_t size() const { return data.size(); }

	void set (size_t pos, T value);

	uint32_t getChecksum (uint32_t crc) const;

private:
	std::vector<T> data;

	mutable std::optional<uint32_t> crcCache;
};

//------------------------------------------------------------------------------
template <typename T>
void cArrayCrc<T>::resize (size_t newSize, T initialValue)
{
	data.clear();
	data.resize (newSize, initialValue);

	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
template <typename T>
void cArrayCrc<T>::fill (T value)
{
	crcCache = std::nullopt;

	for (auto& e : data)
		e = value;
}

//------------------------------------------------------------------------------
template <typename T>
void cArrayCrc<T>::set (size_t pos, T value)
{
	data[pos] = value;

	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
template <typename T>
uint32_t cArrayCrc<T>::getChecksum (uint32_t crc) const
{
	if (!crcCache)
	{
		crcCache = 0;
		for (const auto& e : data)
			*crcCache = calcCheckSum (e, *crcCache);
	}

	return calcCheckSum (*crcCache, crc);
}

#endif
