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

#include <stdint.h>

#include "crc.h"

/**
* This is a wrapper class around a dynamically allocated array. The purpose 
* of this wrapper is to cache the crc value of the array, and only recalculate 
* it, when the array was modified after the last call to getChecksum(). 
*/
template<typename T>
class cArrayCrc
{
public:
	cArrayCrc();
	~cArrayCrc();

	void resize(size_t size, T initialValue);
	void fill(T value);

	const T& operator[](size_t x) const;
	size_t size() const;

	void set(size_t pos, T value);

	uint32_t getChecksum(uint32_t crc) const;

private:
	T* data;
	size_t size_;

	mutable bool crcValid;
	mutable uint32_t crcCache;
};


template<typename T>
cArrayCrc<T>::cArrayCrc() :
	data(nullptr),
	size_(0),
	crcValid(false),
	crcCache(0)
{}

template<typename T>
cArrayCrc<T>::~cArrayCrc()
{
	delete[] data;
}

template<typename T>
void cArrayCrc<T>::resize(size_t newSize, T initialValue)
{
	size_ = newSize;
	crcValid = false;

	delete[] data;
	data = new T[size_];
	for (size_t i = 0; i < size_; i++)
		data[i] = initialValue;
}

template<typename T>
void cArrayCrc<T>::fill(T value)
{
	crcValid = false;

	for (size_t i = 0; i < size_; i++)
		data[i] = value;
}

template<typename T>
const T& cArrayCrc<T>::operator[](size_t x) const
{
	return data[x];
}

template<typename T>
size_t cArrayCrc<T>::size() const
{
	return size_;
}

template<typename T>
void cArrayCrc<T>::set(size_t pos, T value)
{
	data[pos] = value;

	crcValid = false;
}

template<typename T>
uint32_t cArrayCrc<T>::getChecksum(uint32_t crc) const
{
	if (!crcValid)
	{
		crcCache = 0;
		for (size_t i = 0; i < size_; i++)
			crcCache = calcCheckSum(data[i], crcCache);

		crcValid = true;
	}

	return calcCheckSum(crcCache, crc);
}

#endif
