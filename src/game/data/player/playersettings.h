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
#ifndef game_data_player_playersettingsH
#define game_data_player_playersettingsH

#include "game/serialization/serialization.h"
#include "utility/color.h"
#include "utility/crc.h"

struct sPlayerSettings
{
	template <typename Archive>
	void serialize (Archive& archive)
	{
		archive & NVP (name);
		archive & NVP (color);
	}

	//------------------------------------------------------------------------------
	[[nodiscard]] uint32_t getCheckSum (uint32_t crc) const
	{
		crc = calcCheckSum (name, crc);
		crc = calcCheckSum (color, crc);
		return crc;
	}

	bool operator == (const sPlayerSettings& rhs) const { return name == rhs.name && color == rhs.color; }

	std::string name;
	cRgbColor color;
};

#endif
