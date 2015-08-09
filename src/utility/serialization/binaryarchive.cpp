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
#include <cassert>

#include <SDL.h>

#include "binaryarchive.h"


cBinaryArchiveIn::cBinaryArchiveIn()
{};
//------------------------------------------------------------------------------
const unsigned char* cBinaryArchiveIn::data() const
{
	return buffer.data();
}

//------------------------------------------------------------------------------
size_t cBinaryArchiveIn::length() const
{
	return buffer.size();
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::pushValue(bool value)
{
	writeToBuffer(static_cast<Sint8>(value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::pushValue(char value)
{
	if (std::numeric_limits<char>::is_signed) writeToBuffer(static_cast<Sint8>(value));
	else writeToBuffer(static_cast<Uint8>(value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::pushValue(signed char value)
{
	writeToBuffer(static_cast<Sint8>(value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::pushValue(unsigned char value)
{
	writeToBuffer(static_cast<Uint8>(value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::pushValue(signed short value)
{
	writeToBuffer(static_cast<Sint16>(value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::pushValue(unsigned short value)
{
	writeToBuffer(static_cast<Uint16>(value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::pushValue(signed int value)
{
	writeToBuffer(static_cast<Sint32>(value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::pushValue(unsigned int value)
{
	writeToBuffer(static_cast<Uint32>(value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::pushValue(signed long value)
{
	writeToBuffer(static_cast<Sint64>(value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::pushValue(unsigned long value)
{
	writeToBuffer(static_cast<Uint64>(value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::pushValue(signed long long value)
{
	writeToBuffer(static_cast<Sint64>(value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::pushValue(unsigned long long value)
{
	writeToBuffer(static_cast<Uint64>(value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::pushValue(float value)
{
	//TODO
	assert (false);
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::pushValue(double value)
{
	//TODO
	assert(false);
}

//------------------------------------------------------------------------------
cBinaryArchiveOut::cBinaryArchiveOut(const unsigned char* data, size_t length) :
	buffer(length),
	readPosition(0)
{
	memcpy(buffer.data(), data, length); //TODO: was ist das sinnvollste hier?
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::rewind()
{
	readPosition = 0;
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::popValue(bool& value)
{
	Sint8 temp;
	readFromBuffer<1>(temp);
	value = temp != 0;
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::popValue(char& value)
{
	readFromBuffer<1>(value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::popValue(signed char& value)
{
	readFromBuffer<1>(value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::popValue(unsigned char& value)
{
	readFromBuffer<1>(value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::popValue(signed short& value)
{
	readFromBuffer<2>(value); //TODO: 4 im alzi code?!?
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::popValue(unsigned short& value)
{
	readFromBuffer<2>(value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::popValue(signed int& value)
{
	readFromBuffer<4>(value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::popValue(unsigned int& value)
{
	readFromBuffer<4>(value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::popValue(signed long& value)
{
	readFromBuffer<8>(value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::popValue(unsigned long& value)
{
	readFromBuffer<8>(value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::popValue(signed long long& value)
{
	readFromBuffer<8>(value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::popValue(unsigned long long& value)
{
	readFromBuffer<8>(value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::popValue(float& value)
{
	//TODO
	assert(false);
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::popValue(double& value)
{
	//TODO
	assert(false);
}
