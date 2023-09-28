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

#ifndef serialization_binaryarchiveH
#define serialization_binaryarchiveH

#include "serialization.h"

#include <SDL_endian.h>
#include <limits.h>
#include <stdint.h>

class cBinaryArchiveOut
{
public:
	cBinaryArchiveOut (std::vector<unsigned char>& buffer);

	static const bool isWriter = true;

	template <typename T>
	cBinaryArchiveOut& operator<< (const T& value)
	{
		pushValue (value);
		return *this;
	}

	template <typename T>
	cBinaryArchiveOut& operator& (const T& value)
	{
		pushValue (value);
		return *this;
	}

private:
	std::vector<unsigned char>& buffer;

	template <typename T>
	void writeToBuffer (const T& value);

	//--------------------------------------------------------------------------
	template <typename T, std::enable_if_t<!std::is_enum<T>::value, int> = 0>
	void pushValue (const T& value)
	{
		T& valueNonConst = const_cast<T&> (value);
		serialization::serialize (*this, valueNonConst);
	}

	//
	// push fundamental types
	//
	void pushValue (bool value);
	void pushValue (char value);
	void pushValue (signed char value);
	void pushValue (unsigned char value);
	void pushValue (signed short value);
	void pushValue (unsigned short value);
	void pushValue (signed int value);
	void pushValue (unsigned int value);
	void pushValue (signed long value);
	void pushValue (unsigned long value);
	void pushValue (signed long long value);
	void pushValue (unsigned long long value);
	void pushValue (float value);
	void pushValue (double value);

	//--------------------------------------------------------------------------
	template <typename E, std::enable_if_t<std::is_enum<E>::value, int> = 0>
	void pushValue (E value)
	{
		static_assert (sizeof (E) <= sizeof (int), "!");
		int i = static_cast<int> (value);
		pushValue (i);
	}
	//--------------------------------------------------------------------------
	template <typename T>
	void pushValue (const serialization::sNameValuePair<T>& nvp)
	{
		pushValue (nvp.value);
	}

	template <typename T2, typename T1>
	void pushGenericIEEE754As (T1 value);
};

/**
*
*/
class cBinaryArchiveIn
{
public:
	static const bool isWriter = false;

	cBinaryArchiveIn (const unsigned char* data, size_t length);

	//--------------------------------------------------------------------------
	template <typename T>
	cBinaryArchiveIn& operator>> (T& value)
	{
		popValue (value);
		return *this;
	}
	//--------------------------------------------------------------------------
	template <typename T>
	cBinaryArchiveIn& operator>> (const serialization::sNameValuePair<T>& nvp)
	{
		popValue (nvp.value);
		return *this;
	}

	//--------------------------------------------------------------------------
	template <typename T>
	cBinaryArchiveIn& operator& (T& value)
	{
		popValue (value);
		return *this;
	}
	//--------------------------------------------------------------------------
	template <typename T>
	cBinaryArchiveIn& operator& (const serialization::sNameValuePair<T>& nvp)
	{
		popValue (nvp.value);
		return *this;
	}

	size_t dataLeft() const;

private:
	const unsigned char* data;
	size_t length;

	size_t readPosition;

	template <size_t SIZE, typename T1>
	void readFromBuffer (T1& value);

	//--------------------------------------------------------------------------
	template <typename T, std::enable_if_t<!std::is_enum<T>::value, int> = 0>
	void popValue (T& value)
	{
		serialization::serialize (*this, value);
	}

	//--------------------------------------------------------------------------
	template <typename E, std::enable_if_t<std::is_enum<E>::value, int> = 0>
	void popValue (E& value)
	{
		static_assert (sizeof (E) <= sizeof (int), "!");
		int i = 0;
		popValue (i);
		value = static_cast<E> (i);
	}

	//
	// pop fundamental types
	//
	void popValue (bool& value);
	void popValue (char& value);
	void popValue (signed char& value);
	void popValue (unsigned char& value);
	void popValue (signed short& value);
	void popValue (unsigned short& value);
	void popValue (signed int& value);
	void popValue (unsigned int& value);
	void popValue (signed long& value);
	void popValue (unsigned long& value);
	void popValue (signed long long& value);
	void popValue (unsigned long long& value);
	void popValue (float& value);
	void popValue (double& value);

	template <typename T2, typename T1>
	void popGenericIEEE754As (T1& value);
};

//------------------------------------------------------------------------------
template <typename T>
void cBinaryArchiveOut::writeToBuffer (const T& value)
{
	static_assert (CHAR_BIT == 8, "!");

	buffer.resize (buffer.size() + sizeof (T));

	switch (sizeof (T))
	{
		case 1:
		{
			int8_t* dest = reinterpret_cast<int8_t*> (&buffer[buffer.size() - sizeof (T)]);
			*dest = static_cast<int8_t> (value);
			break;
		}
		case 2:
		{
			int16_t* dest = reinterpret_cast<int16_t*> (&buffer[buffer.size() - sizeof (T)]);
			*dest = SDL_SwapLE16 (static_cast<int16_t> (value));
			break;
		}
		case 4:
		{
			int32_t* dest = reinterpret_cast<int32_t*> (&buffer[buffer.size() - sizeof (T)]);
			*dest = SDL_SwapLE32 (static_cast<int32_t> (value));
			break;
		}
		case 8:
		{
			int64_t* dest = reinterpret_cast<int64_t*> (&buffer[buffer.size() - sizeof (T)]);
			*dest = SDL_SwapLE64 (static_cast<int64_t> (value));
			break;
		}
		default:
			static_assert (sizeof (T) == 1 || sizeof (T) == 2 || sizeof (T) == 4 || sizeof (T) == 8, "!");
	}
}

//------------------------------------------------------------------------------
template <typename T2, typename T1>
void cBinaryArchiveOut::pushGenericIEEE754As (T1 value)
{
	static_assert (sizeof (T1) == 4 || sizeof (T1) == 8, "!");
	static_assert (sizeof (T1) == sizeof (T2), "!");

	const unsigned int BITS = sizeof (T1) * CHAR_BIT;
	const unsigned int EXPBITS = sizeof (T1) == 4 ? 8 : 11;
	const unsigned int SIGNIFICANTBITS = BITS - EXPBITS - 1; // -1 for sign bit

	if (value == 0.0)
	{
		writeToBuffer (T2 (0));
		return;
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

	writeToBuffer (T2 ((sign << (BITS - 1)) | (exp << (BITS - EXPBITS - 1)) | significand));
}

//------------------------------------------------------------------------------
template <size_t SIZE, typename T1>
void cBinaryArchiveIn::readFromBuffer (T1& value)
{
	static_assert (CHAR_BIT == 8, "!");

	if (length - readPosition < SIZE)
	{
		throw std::runtime_error ("cBinaryArchiveIn: Buffer underrun");
	}

	switch (SIZE)
	{
		case 1:
		{
			int8_t temp = *reinterpret_cast<const int8_t*> (&data[readPosition]);
			value = static_cast<T1> (temp);
			break;
		}
		case 2:
		{
			int16_t temp = SDL_SwapLE16 (*reinterpret_cast<const int16_t*> (&data[readPosition]));
			value = static_cast<T1> (temp);
			break;
		}
		case 4:
		{
			int32_t temp = SDL_SwapLE32 (*reinterpret_cast<const int32_t*> (&data[readPosition]));
			value = static_cast<T1> (temp);
			break;
		}
		case 8:
		{
			int64_t temp = SDL_SwapLE64 (*reinterpret_cast<const int64_t*> (&data[readPosition]));
			value = static_cast<T1> (temp);
			break;
		}
		default:
			static_assert (SIZE == 1 || SIZE == 2 || SIZE == 4 || SIZE == 8, "!");
	}

	readPosition += SIZE;
}

//------------------------------------------------------------------------------
template <typename T2, typename T1>
void cBinaryArchiveIn::popGenericIEEE754As (T1& value)
{
	static_assert (sizeof (T1) == 4 || sizeof (T1) == 8, "!");
	static_assert (sizeof (T1) == sizeof (T2), "!");

	const unsigned int BITS = sizeof (T1) * CHAR_BIT;
	const unsigned int EXPBITS = sizeof (T1) == 4 ? 8 : 11;
	const unsigned int SIGNIFICANTBITS = BITS - EXPBITS - 1; // -1 for sign bit

	// get data
	T2 i;
	readFromBuffer<sizeof (T2)> (i);

	if (i == 0)
	{
		value = 0;
		return;
	}

	// pull the significand
	value = T1 (i & ((1LL << SIGNIFICANTBITS) - 1)); // mask
	value /= (1LL << SIGNIFICANTBITS); // convert back to float
	value += 1.0; // add the one back on

	// deal with the exponent
	unsigned int bias = (1 << (EXPBITS - 1)) - 1;
	long long shift = ((i >> SIGNIFICANTBITS) & ((1LL << EXPBITS) - 1)) - bias;
	while (shift > 0)
	{
		value *= 2.0;
		shift--;
	}
	while (shift < 0)
	{
		value /= 2.0;
		shift++;
	}

	// sign it
	value *= T1 (((i >> (BITS - 1)) & 1) ? -1.0 : 1.0);
}
#endif //serialization_binaryarchiveH
