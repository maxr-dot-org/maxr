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

#ifndef utility_crossplatformrandomH
#define utility_crossplatformrandomH

#include "utility/crc.h"
#include "utility/serialization/serialization.h"

#include <cstdint>

class cCrossPlattformRandom
{
private:
	uint32_t stateW = 0;
	uint32_t stateZ = 0;

public:
	cCrossPlattformRandom() = default;

	void seed (uint64_t seed);

	/** returns a random number in the interval [0..UINT32_MAX] */
	uint32_t get();

	/** returns a random number in the interval [0..interval) */
	uint32_t get (uint32_t interval);

	template <ArchiveInOrOut Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (stateW);
		archive & NVP (stateZ);
		// clang-format on
	}

	[[nodiscard]] friend std::uint32_t calcCheckSum (const cCrossPlattformRandom& self, std::uint32_t crc)
	{
		crc = calcCheckSum (self.stateW, crc);
		crc = calcCheckSum (self.stateZ, crc);
		return crc;
	}
};

#endif
