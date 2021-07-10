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

#include "textarchive.h"



cTextArchiveIn::cTextArchiveIn() :
	nextCommaNeeded (false)
{
	buffer.imbue (std::locale ("C"));
}
//------------------------------------------------------------------------------
const std::string cTextArchiveIn::data() const
{
	return buffer.str();
}
//------------------------------------------------------------------------------
void cTextArchiveIn::openBracket()
{
	addComma();
	buffer << "(";
	nextCommaNeeded = false;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::closeBracket()
{
	buffer << ")";
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::addComma()
{
	if (nextCommaNeeded)
		buffer << ", ";

	nextCommaNeeded = false;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (bool value)
{
	addComma();
	buffer << value;
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (char value)
{
	addComma();
	buffer << static_cast<int> (value);
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (signed char value)
{
	addComma();
	buffer << static_cast<int> (value);
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (unsigned char value)
{
	addComma();
	buffer << static_cast<int> (value);
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (signed short value)
{
	addComma();
	buffer << value;
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (unsigned short value)
{
	addComma();
	buffer << value;
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (signed int value)
{
	addComma();
	buffer << value;
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (unsigned int value)
{
	addComma();
	buffer << value;
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (signed long value)
{
	addComma();
	buffer << value;
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (unsigned long value)
{
	addComma();
	buffer << value;
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (signed long long value)
{
	addComma();
	buffer << value;
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (unsigned long long value)
{
	addComma();
	buffer << value;
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (float value)
{
	addComma();
	buffer << value;
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (double value)
{
	addComma();
	buffer << value;
	nextCommaNeeded = true;
}
//------------------------------------------------------------------------------
void cTextArchiveIn::pushValue (const std::string& value)
{
	std::string s = value;

	//replace line breaks in string by "\n"
	size_t pos = 0;
	while ((pos = s.find ('\n', pos)) != std::string::npos)
	{
		s.replace (pos, 1, "\\n");
		pos += 2;
	}

	addComma();
	buffer << "\"" << s << "\"";
	nextCommaNeeded = true;
}
