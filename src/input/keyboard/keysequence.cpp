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

#include "input/keyboard/keysequence.h"

//------------------------------------------------------------------------------
cKeySequence::cKeySequence()
{}

//------------------------------------------------------------------------------
cKeySequence::cKeySequence (const std::string& sequence)
{
	// FIXME: make the parser more intelligent:
	//        e.g. it should be able to parse the following commands successfully:
	//         - "Ctrl++" meaning the combination of the control and the plus key at the same time
	//         - "a,," meaning first the a key and then the colon key.
	std::string::size_type start = 0;
	while (true)
	{
		auto end = sequence.find (',', start);

		addKeyCombination (cKeyCombination (sequence.substr (start, end - start)));

		if (end == std::string::npos) break;

		start += end + 1;
	}
}

//------------------------------------------------------------------------------
cKeySequence::cKeySequence (cKeyCombination keyCombination)
{
	addKeyCombination (keyCombination);
}

//------------------------------------------------------------------------------
cKeySequence::cKeySequence (cKeyCombination keyCombination1, cKeyCombination keyCombination2)
{
	addKeyCombination (keyCombination1);
	addKeyCombination (keyCombination2);
}

//------------------------------------------------------------------------------
cKeySequence::cKeySequence (cKeyCombination keyCombination1, cKeyCombination keyCombination2, cKeyCombination keyCombination3)
{
	addKeyCombination (keyCombination1);
	addKeyCombination (keyCombination2);
	addKeyCombination (keyCombination3);
}

//------------------------------------------------------------------------------
bool cKeySequence::operator== (const cKeySequence& other) const
{
	if (keySequence.size() != other.keySequence.size()) return false;

	for (size_t i = 0; i < keySequence.size(); ++i)
	{
		if (keySequence[i] != other.keySequence[i]) return false;
	}
	return true;
}

//------------------------------------------------------------------------------
bool cKeySequence::operator!= (const cKeySequence& other) const
{
	return ! (*this == other);
}

//------------------------------------------------------------------------------
void cKeySequence::addKeyCombination (cKeyCombination keyCombination)
{
	keySequence.push_back (std::move (keyCombination));
}

//------------------------------------------------------------------------------
void cKeySequence::removeFirst()
{
	keySequence.erase (keySequence.begin());
}

//------------------------------------------------------------------------------
size_t cKeySequence::length() const
{
	return keySequence.size();
}

//------------------------------------------------------------------------------
const cKeyCombination& cKeySequence::operator[] (size_t index) const
{
	return keySequence[index];
}

//------------------------------------------------------------------------------
void cKeySequence::reset()
{
	keySequence.clear();
}

//------------------------------------------------------------------------------
std::string cKeySequence::toString() const
{
	std::string result;
	for (size_t i = 0; i < keySequence.size(); ++i)
	{
		if (i > 0) result += ",";
		result += keySequence[i].toString();
	}
	return result;
}
