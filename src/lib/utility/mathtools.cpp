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

#include "utility/mathtools.h"

#include "utility/narrow_cast.h"

#include <cmath>

namespace
{
	//--------------------------------------------------------------------------
	constexpr int pow10 (unsigned int n)
	{
		int res = 1;
		for (unsigned int i = 0; i != n; ++i)
		{
			res *= 10;
		}
		return res;
	}
} // namespace

//------------------------------------------------------------------------------
// Rounds a Number to 'iDecimalPlace' digits after the comma:
float Round (float dValueToRound, unsigned int iDecimalPlace)
{
	const auto factor = pow10 (iDecimalPlace);
	dValueToRound *= factor;
	if (dValueToRound >= 0)
		dValueToRound = floorf (dValueToRound + 0.5f);
	else
		dValueToRound = ceilf (dValueToRound - 0.5f);
	dValueToRound /= factor;
	return dValueToRound;
}

//------------------------------------------------------------------------------
int Round (float dValueToRound)
{
	return narrow_cast<int> (Round (dValueToRound, 0));
}
