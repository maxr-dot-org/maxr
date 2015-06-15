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

#ifndef serializationarchiveH
#define serializationarchiveH

#include <vector>
#include <cassert>
#include <type_traits>

class cArchiveIn
{
	//friend class cArchiveOut;
public:
	cArchiveIn();

	//explicit cArchiveIn(const cArchiveOut& other);
	//explicit cArchiveIn(cArchiveOut&& other);

	//static const bool isWriter = true;

	template<typename T>
	cArchiveIn& operator<<(const T& value);
	template<typename T>
	cArchiveIn& operator&(const T& value);

	//void clear();

	const unsigned char* data() const;
	size_t length() const;
private:
	std::vector<unsigned char> buffer;

	template <typename T>
	void writeToBuffer(const T& value);

	template<typename T>
	void pushValue(const T& value);

	//
	// push fundamental types
	//
	void pushValue(bool value);
	void pushValue(char value);
	void pushValue(signed char value);
	void pushValue(unsigned char value);
	void pushValue(signed short value);
	void pushValue(unsigned short value);
	void pushValue(signed int value);
	void pushValue(unsigned int value);
	void pushValue(signed long value);
	void pushValue(unsigned long value);
	void pushValue(signed long long value);
	void pushValue(unsigned long long value);
	void pushValue(float value);
	void pushValue(double value);

	//
	// push STL types
	//
	void pushValue(const std::string& value);
	template <typename T>
	void pushValue(const std::vector<T>& value);
	template <typename A, typename B>
	void pushValue(const std::pair<A, B>& value);
};

/**
*
*/
class cArchiveOut
{
	//friend class cArchiveIn;
public:
	//typedef unsigned char BufferElementType;
	//typedef std::vector<BufferElementType> BufferType;

	//static const bool isWriter = false;

	cArchiveOut(const unsigned char* data, size_t length);

	//explicit cArchiveOut(const cArchiveIn& other);
	//explicit cArchiveOut(cArchiveIn&& other);

	template<typename T>
	cArchiveOut& operator>>(T& value);
	template<typename T>
	cArchiveOut& operator&(T& value);

	//template<typename T>
	//T pop();

	void rewind();

	//BufferElementType* data();
	//size_t length();
private:
	std::vector<unsigned char> buffer;
	size_t readPosition;

	template<size_t SIZE, typename T1>
	void readFromBuffer(T1& value);

	template<typename T>
	void popValue(T& value);

	//
	// pop fundamental types
	//
	void popValue(bool& value);
	void popValue(char& value);
	void popValue(signed char& value);
	void popValue(unsigned char& value);
	void popValue(signed short& value);
	void popValue(unsigned short& value);
	void popValue(signed int& value);
	void popValue(unsigned int& value);
	void popValue(signed long& value);
	void popValue(unsigned long& value);
	void popValue(signed long long& value);
	void popValue(unsigned long long& value);
	void popValue(float& value);
	void popValue(double& value);
	//void popValue(long double& value);

	//
	// pop STL types
	//
	void popValue(std::string& value);
	template <typename T>
	void popValue(std::vector<T>& value);
	template <typename A, typename B>
	void popValue(std::pair<A, B>& value);
};


//------------------------------------------------------------------------------
template<typename T>
cArchiveIn& cArchiveIn::operator<<(const T& value)
{
	pushValue(value);
	return *this;
}

//------------------------------------------------------------------------------
template<typename T>
cArchiveIn& cArchiveIn::operator&(const T& value)
{
	pushValue(value);
	return *this;
}

//------------------------------------------------------------------------------
template <typename T>
void cArchiveIn::writeToBuffer(const T& value)
{
	assert(CHAR_BIT == 8); // TODO: make static assert

	buffer.resize(buffer.size() + sizeof(T));

	switch (sizeof(T))
	{
	case 1:
	{
		Sint8* dest = reinterpret_cast<Sint8*>(&buffer[buffer.size() - sizeof(T)]);
		*dest = static_cast<Sint8>(value);
		break;
	}
	case 2:
	{
		Sint16* dest = reinterpret_cast<Sint16*>(&buffer[buffer.size() - sizeof(T)]);
		*dest = SDL_SwapLE16(static_cast<Sint16>(value));
		break;
	}
	case 4:
	{
		Sint32* dest = reinterpret_cast<Sint32*>(&buffer[buffer.size() - sizeof(T)]);
		*dest = SDL_SwapLE32(static_cast<Sint32>(value));
		break;
	}
	case 8:
	{
		Sint64* dest = reinterpret_cast<Sint64*>(&buffer[buffer.size() - sizeof(T)]);
		*dest = SDL_SwapLE64(static_cast<Sint64>(value));
		break;
	}
	default:
		assert(false); // TODO: make static assert
	}
}

struct sSerializeMember
{
	template<typename T, typename A>
	static void serialize(T& object, A& archive)
	{
		object.serialize(archive);
	}
};
struct sSerializeEnum
{
	template<typename T>
	static void serialize(T& enumValue, cArchiveIn& archive)
	{
		int tmp = enumValue;
		archive & tmp;
	}
	template<typename T>
	static void serialize(T& enumValue, cArchiveOut& archive)
	{
		int tmp;
		archive & tmp;
		enumValue = (T)tmp;
	}
};

//------------------------------------------------------------------------------
template<typename T>
void cArchiveIn::pushValue(const T& value)
{
	typedef std::conditional<std::is_enum<T>::value, sSerializeEnum, sSerializeMember>::type serializeWrapper;
	T& value_nonconst = static_cast<T>(value);
	serializeWrapper::serialize(value_nonconst, *this);
}

//------------------------------------------------------------------------------
template <typename T>
void cArchiveIn::pushValue(const std::vector<T>& value)
{
	pushValue(value.size());
	for (auto c : value)
	{
		pushValue(c);
	}
}
//------------------------------------------------------------------------------
template <typename A, typename B>
void cArchiveIn::pushValue(const std::pair<A, B>& value)
{
	pushValue(value.first);
	pushValue(value.second);
}
//------------------------------------------------------------------------------
template<typename T>
cArchiveOut& cArchiveOut::operator>>(T& value)
{
	popValue(value);
	return *this;
}
//------------------------------------------------------------------------------
template<typename T>
cArchiveOut& cArchiveOut::operator&(T& value)
{
	popValue(value);
	return *this;
}
//------------------------------------------------------------------------------
template<size_t SIZE, typename T1>
void cArchiveOut::readFromBuffer(T1& value)
{
	assert(CHAR_BIT == 8); // TODO: make static assert

	if (buffer.size() - readPosition < SIZE)
	{
		throw std::runtime_error("Buffer to small");
	}

	switch (SIZE)
	{
	case 1:
	{
		Sint8 temp = *reinterpret_cast<Sint8*>(&buffer[readPosition]);
		value = static_cast<T1>(temp); //TODO: warum warning?!?
		break;
	}
	case 2:
	{
		Sint16 temp = SDL_SwapLE16(*reinterpret_cast<Sint16*>(&buffer[readPosition]));
		value = static_cast<T1>(temp);
		break;
	}
	case 4:
	{
		Sint32 temp = SDL_SwapLE32(*reinterpret_cast<Sint32*>(&buffer[readPosition]));
		value = static_cast<T1>(temp);
		break;
	}
	case 8:
	{
		Sint32 temp = SDL_SwapLE32(*reinterpret_cast<Sint32*>(&buffer[readPosition]));
		value = static_cast<T1>(temp);		
		break;
	}
	default:
		assert(false); // TODO: make static assert
	}

	readPosition += SIZE;
}
//------------------------------------------------------------------------------
template<typename T>
void cArchiveOut::popValue(T& value)
{
	typedef std::conditional<std::is_enum<T>::value, sSerializeEnum, sSerializeMember>::type serializeWrapper;

	serializeWrapper::serialize(value, *this);
}
//------------------------------------------------------------------------------
template <typename T>
void cArchiveOut::popValue(std::vector<T>& value)
{
	size_t length;
	popValue(length);
	value.resize(length);
	for (size_t i = 0; i < length; i++)
	{
		T c;
		popValue(c);
		value[i] = c;
	}
}
//------------------------------------------------------------------------------
template <typename A, typename B>
void cArchiveOut::popValue(std::pair<A, B>& value)
{
	popValue(value.first);
	popValue(value.second);
}
#endif //serializationarchiveH