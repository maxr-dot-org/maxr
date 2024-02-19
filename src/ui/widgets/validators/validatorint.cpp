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

#include "validatorint.h"

#include "utility/listhelpers.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <limits>

//------------------------------------------------------------------------------
cValidatorInt::cValidatorInt() :
	minValue (std::numeric_limits<int>::min()),
	maxValue (std::numeric_limits<int>::max())
{}

//------------------------------------------------------------------------------
cValidatorInt::cValidatorInt (int minValue_, int maxValue_) :
	minValue (minValue_),
	maxValue (maxValue_)
{}

//------------------------------------------------------------------------------
eValidatorState cValidatorInt::validate (const std::string& text) const
{
	if (text.empty()) return eValidatorState::Intermediate;

	if (ranges::any_of (text, [](int c) {return !std::isdigit (c);}))
	{
		return eValidatorState::Invalid;
	}

	int value = std::atoi (text.c_str());

	if (value < minValue || value > maxValue) return eValidatorState::Intermediate;

	return eValidatorState::Valid;
}

//------------------------------------------------------------------------------
void cValidatorInt::fixup (std::string& text) const
{
	EraseIf (text, [] (char c) { return !std::isdigit (c); });

	if (text.empty())
	{
		text = std::to_string (std::clamp (0, minValue, maxValue));
	}
	else
	{
		int value = std::atoi (text.c_str());

		if (value < minValue)
		{
			text = std::to_string (minValue);
		}
		else if (value > maxValue)
		{
			text = std::to_string (maxValue);
		}
	}
}
