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

#ifndef game_data_miningresourceH
#define game_data_miningresourceH

#include "game/data/resourcetype.h"
#include "utility/serialization/serialization.h"

#include <cstdint>

struct sMiningResource
{
	int get (eResourceType) const;
	int& get (eResourceType);

	int total() const;

	friend bool operator== (const sMiningResource&, const sMiningResource&) = default;

	sMiningResource& operator+= (const sMiningResource&);
	sMiningResource& operator-= (const sMiningResource&);

	template <ArchiveInOrOut Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (metal);
		archive & NVP (oil);
		archive & NVP (gold);
		// clang-format on
	}

	int metal = 0;
	int oil = 0;
	int gold = 0;
};

[[nodiscard]] uint32_t calcCheckSum (const sMiningResource&, uint32_t crc);

#endif
