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

#include "crc.h"

#include <SDL_endian.h>
#include <climits>

//------------------------------------------------------------------------------
uint32_t calcCheckSum (const char* data, size_t dataSize, uint32_t checksum)
{
	for (const char* i = data; i != data + dataSize; ++i)
	{
		checksum = checksum << 1 | checksum >> 31; // Rotate left by one.
		checksum += *i;
	}
	return checksum;
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (bool data, uint32_t checksum)
{
	uint8_t x = static_cast<uint8_t> (data);
	return calcCheckSum (reinterpret_cast<char*> (&x), sizeof (x), checksum);
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (char data, uint32_t checksum)
{
	uint8_t x = static_cast<uint8_t> (data);
	return calcCheckSum (reinterpret_cast<char*> (&x), sizeof (x), checksum);
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (signed char data, uint32_t checksum)
{
	uint8_t x = static_cast<uint8_t> (data);
	return calcCheckSum (reinterpret_cast<char*> (&x), sizeof (x), checksum);
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (unsigned char data, uint32_t checksum)
{
	uint8_t x = static_cast<uint8_t> (data);
	return calcCheckSum (reinterpret_cast<char*> (&x), sizeof (x), checksum);
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (signed short data, uint32_t checksum)
{
	uint16_t x = SDL_SwapLE16 (static_cast<uint16_t> (data));
	return calcCheckSum (reinterpret_cast<char*> (&x), sizeof (x), checksum);
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (unsigned short data, uint32_t checksum)
{
	uint16_t x = SDL_SwapLE16 (static_cast<uint16_t> (data));
	return calcCheckSum (reinterpret_cast<char*> (&x), sizeof (x), checksum);
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (signed int data, uint32_t checksum)
{
	uint32_t x = SDL_SwapLE32 (static_cast<uint32_t> (data));
	return calcCheckSum (reinterpret_cast<char*> (&x), sizeof (x), checksum);
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (unsigned int data, uint32_t checksum)
{
	uint32_t x = SDL_SwapLE32 (static_cast<uint32_t> (data));
	return calcCheckSum (reinterpret_cast<char*> (&x), sizeof (x), checksum);
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (signed long data, uint32_t checksum)
{
	uint64_t x = SDL_SwapLE64 (static_cast<uint64_t> (data));
	return calcCheckSum (reinterpret_cast<char*> (&x), sizeof (x), checksum);
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (unsigned long data, uint32_t checksum)
{
	uint64_t x = SDL_SwapLE64 (static_cast<uint64_t> (data));
	return calcCheckSum (reinterpret_cast<char*> (&x), sizeof (x), checksum);
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (signed long long data, uint32_t checksum)
{
	uint64_t x = SDL_SwapLE64 (static_cast<uint64_t> (data));
	return calcCheckSum (reinterpret_cast<char*> (&x), sizeof (x), checksum);
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (unsigned long long data, uint32_t checksum)
{
	uint64_t x = SDL_SwapLE64 (static_cast<uint64_t> (data));
	return calcCheckSum (reinterpret_cast<char*> (&x), sizeof (x), checksum);
}

//------------------------------------------------------------------------------
template <typename T2, typename T1>
uint32_t calcCheckSumGenericIEEE754 (T1 value, uint32_t checksum)
{
	static_assert (sizeof (T1) == 4 || sizeof (T1) == 8, "!");
	static_assert (sizeof (T1) == sizeof (T2), "!");

	const unsigned int BITS = sizeof (T1) * CHAR_BIT;
	const unsigned int EXPBITS = sizeof (T1) == 4 ? 8 : 11;
	const unsigned int SIGNIFICANTBITS = BITS - EXPBITS - 1; // -1 for sign bit

	if (value == 0.0)
	{
		return calcCheckSum (T2 (0), checksum);
	}

	T1 norm;
	T2 sign;
	// check sign and begin normalization
	if (value < 0)
	{
		sign = 1;
		norm = -value;
	}
	else
	{
		sign = 0;
		norm = value;
	}

	// get the normalized form of f and track the exponent
	T2 shift = 0;
	while (norm >= 2.0)
	{
		norm /= 2.0;
		shift++;
	}
	while (norm < 1.0)
	{
		norm *= 2.0;
		shift--;
	}
	norm -= 1.0;

	// calculate the binary form (non-float) of the significand data
	const T2 significand = T2 (norm * ((1LL << SIGNIFICANTBITS) + 0.5f));

	// get the biased exponent
	const T2 exp = shift + ((1 << (EXPBITS - 1)) - 1); // shift + bias

	return calcCheckSum (T2 ((sign << (BITS - 1)) | (exp << (BITS - EXPBITS - 1)) | significand), checksum);
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (float data, uint32_t checksum)
{
	return calcCheckSumGenericIEEE754<int32_t> (data, checksum);
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (double data, uint32_t checksum)
{
	return calcCheckSumGenericIEEE754<int64_t> (data, checksum);
}

//------------------------------------------------------------------------------
uint32_t calcCheckSum (const std::string& data, uint32_t checksum)
{
	return calcCheckSum (data.c_str(), data.length(), checksum);
}
