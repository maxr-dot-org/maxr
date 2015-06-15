
#include <SDL.h>

#include "serializationarchive.h"


cArchiveIn::cArchiveIn()
{};
//------------------------------------------------------------------------------
const unsigned char* cArchiveIn::data() const
{
	return buffer.data();
}

//------------------------------------------------------------------------------
size_t cArchiveIn::length() const
{
	return buffer.size();
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(bool value)
{
	writeToBuffer(static_cast<Sint8>(value));
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(char value)
{
	if (std::numeric_limits<char>::is_signed) writeToBuffer(static_cast<Sint8>(value));
	else writeToBuffer(static_cast<Uint8>(value));
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(signed char value)
{
	writeToBuffer(static_cast<Sint8>(value));
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(unsigned char value)
{
	writeToBuffer(static_cast<Uint8>(value));
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(signed short value)
{
	writeToBuffer(static_cast<Sint16>(value));
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(unsigned short value)
{
	writeToBuffer(static_cast<Uint16>(value));
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(signed int value)
{
	writeToBuffer(static_cast<Sint32>(value));
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(unsigned int value)
{
	writeToBuffer(static_cast<Uint32>(value));
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(signed long value)
{
	writeToBuffer(static_cast<Sint64>(value));
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(unsigned long value)
{
	writeToBuffer(static_cast<Uint64>(value));
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(signed long long value)
{
	writeToBuffer(static_cast<Sint64>(value));
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(unsigned long long value)
{
	writeToBuffer(static_cast<Uint64>(value));
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(float value)
{
	//TODO
	assert (false);
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(double value)
{
	//TODO
	assert(false);
}
//------------------------------------------------------------------------------
void cArchiveIn::pushValue(const std::string& value)
{
	pushValue(value.length());
	for (auto c : value)
	{
		pushValue(c);
	}
}
//------------------------------------------------------------------------------
cArchiveOut::cArchiveOut(const unsigned char* data, size_t length) :
	buffer(length),
	readPosition(0)
{
	memcpy(buffer.data(), data, length); //TODO: was ist das sinnvollste hier?
}
//------------------------------------------------------------------------------
void cArchiveOut::rewind()
{
	readPosition = 0;
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(bool& value)
{
	Sint8 temp;
	readFromBuffer<1>(temp);
	value = temp != 0;
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(char& value)
{
	readFromBuffer<1>(value);
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(signed char& value)
{
	readFromBuffer<1>(value);
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(unsigned char& value)
{
	readFromBuffer<1>(value);
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(signed short& value)
{
	readFromBuffer<2>(value); //TODO: 4 im alzi code?!?
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(unsigned short& value)
{
	readFromBuffer<2>(value);
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(signed int& value)
{
	readFromBuffer<4>(value);
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(unsigned int& value)
{
	readFromBuffer<4>(value);
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(signed long& value)
{
	readFromBuffer<8>(value);
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(unsigned long& value)
{
	readFromBuffer<8>(value);
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(signed long long& value)
{
	readFromBuffer<8>(value);
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(unsigned long long& value)
{
	readFromBuffer<8>(value);
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(float& value)
{
	//TODO
	assert(false);
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(double& value)
{
	//TODO
	assert(false);
}
//------------------------------------------------------------------------------
void cArchiveOut::popValue(std::string& value)
{
	size_t length;
	popValue(length);
	value.reserve(length);
	for (size_t i = 0; i < length; i++)
	{
		char c;
		popValue(c);
		value.push_back(c);
	}
}
