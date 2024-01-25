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

#include "binaryarchive.h"

//------------------------------------------------------------------------------
cBinaryArchiveOut::cBinaryArchiveOut (std::vector<unsigned char>& buffer) :
	buffer (buffer)
{}

//------------------------------------------------------------------------------
void cBinaryArchiveOut::pushValue (bool value)
{
	writeToBuffer (static_cast<Sint8> (value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::pushValue (char value)
{
	if (std::numeric_limits<char>::is_signed)
		writeToBuffer (static_cast<Sint8> (value));
	else
		writeToBuffer (static_cast<Uint8> (value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::pushValue (signed char value)
{
	writeToBuffer (static_cast<Sint8> (value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::pushValue (unsigned char value)
{
	writeToBuffer (static_cast<Uint8> (value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::pushValue (signed short value)
{
	writeToBuffer (static_cast<Sint16> (value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::pushValue (unsigned short value)
{
	writeToBuffer (static_cast<Uint16> (value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::pushValue (signed int value)
{
	writeToBuffer (static_cast<Sint32> (value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::pushValue (unsigned int value)
{
	writeToBuffer (static_cast<Uint32> (value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::pushValue (signed long value)
{
	writeToBuffer (static_cast<Sint64> (value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::pushValue (unsigned long value)
{
	writeToBuffer (static_cast<Uint64> (value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::pushValue (signed long long value)
{
	writeToBuffer (static_cast<Sint64> (value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::pushValue (unsigned long long value)
{
	writeToBuffer (static_cast<Uint64> (value));
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::pushValue (float value)
{
	pushGenericIEEE754As<Sint32> (value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveOut::pushValue (double value)
{
	pushGenericIEEE754As<Sint64> (value);
}

//------------------------------------------------------------------------------
cBinaryArchiveIn::cBinaryArchiveIn (const unsigned char* data, size_t length) :
	data (data),
	length (length)
{}
//------------------------------------------------------------------------------
size_t cBinaryArchiveIn::dataLeft() const
{
	return length - readPosition;
}

//------------------------------------------------------------------------------
void cBinaryArchiveIn::popValue (bool& value)
{
	Sint8 temp;
	readFromBuffer<1> (temp);
	value = temp != 0;
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::popValue (char& value)
{
	readFromBuffer<1> (value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::popValue (signed char& value)
{
	readFromBuffer<1> (value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::popValue (unsigned char& value)
{
	readFromBuffer<1> (value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::popValue (signed short& value)
{
	readFromBuffer<2> (value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::popValue (unsigned short& value)
{
	readFromBuffer<2> (value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::popValue (signed int& value)
{
	readFromBuffer<4> (value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::popValue (unsigned int& value)
{
	readFromBuffer<4> (value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::popValue (signed long& value)
{
	readFromBuffer<8> (value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::popValue (unsigned long& value)
{
	readFromBuffer<8> (value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::popValue (signed long long& value)
{
	readFromBuffer<8> (value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::popValue (unsigned long long& value)
{
	readFromBuffer<8> (value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::popValue (float& value)
{
	popGenericIEEE754As<Sint32> (value);
}
//------------------------------------------------------------------------------
void cBinaryArchiveIn::popValue (double& value)
{
	popGenericIEEE754As<Sint64> (value);
}
