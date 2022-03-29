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

#include "crossplattformrandom.h"

//------------------------------------------------------------------------------
void cCrossPlattformRandom::seed (uint64_t seed)
{
	stateW = static_cast<uint32_t> (seed); /* must not be zero, nor 0x464fffff */
	if (stateW == 0 || stateW == 0x464fffff)
		stateW++;

	stateZ = static_cast<uint32_t> (seed >> 32); /* must not be zero, nor 0x9068ffff */
	if (stateZ == 0 || stateZ == 0x9068ffff)
		stateZ++;
}

//------------------------------------------------------------------------------
uint32_t cCrossPlattformRandom::get()
{
	stateZ = 36969 * (stateZ & 65535) + (stateZ >> 16);
	stateW = 18000 * (stateW & 65535) + (stateW >> 16);
	return (stateZ << 16) + stateW;
}

//------------------------------------------------------------------------------
uint32_t cCrossPlattformRandom::get (uint32_t interval)
{
	uint32_t r;
	const unsigned int buckets = UINT32_MAX / interval;
	const unsigned int limit = buckets * interval;

	// create random numbers,
	// with a uniform distribution in the specified interval
	do
	{
		r = get();
	} while (r >= limit);

	return r / buckets;
}
