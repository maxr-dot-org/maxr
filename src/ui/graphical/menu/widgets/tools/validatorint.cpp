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

#include <limits>
#include <algorithm>
#include <cctype>
#include <cstdlib>

#include "ui/graphical/menu/widgets/tools/validatorint.h"
#include "main.h"

cValidatorInt::cValidatorInt () :
	minValue (std::numeric_limits<int>::min	()),
	maxValue (std::numeric_limits<int>::max ())
{}

cValidatorInt::cValidatorInt (int minValue_, int maxValue_) :
	minValue (minValue_),
	maxValue (maxValue_)
{}

eValidatorState cValidatorInt::validate (const std::string& text) const
{
	if (text.empty ()) return eValidatorState::Intermediate;

	for (size_t i = 0; i < text.size (); ++i)
	{
		if (!std::isdigit (text[i])) return eValidatorState::Invalid;
	}

	int value = std::atoi (text.c_str ());

	if (value < minValue || value > maxValue) return eValidatorState::Intermediate;

	return eValidatorState::Valid;
}

void cValidatorInt::fixup (std::string& text) const
{
	// remove all non digits
	auto newEnd = std::remove_if (text.begin (), text.end (), [](char c) { return !std::isdigit (c); });
	text.erase (newEnd, text.end ());

	if (text.empty ())
	{
		text = iToStr (std::max (std::min (0, maxValue), minValue));
	}
	else
	{
		int value = std::atoi (text.c_str ());

		if (value < minValue)
		{
			text = iToStr (minValue);
		}
		else if (value > maxValue)
		{
			text = iToStr (maxValue);
		}
	}
}
