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

#include "game/data/player/playersettings.h"
#include "utility/color.h"
#include "utility/crc.h"

#include <cstdint>
#include <doctest.h>
#include <string>

//------------------------------------------------------------------------------
// The bytes from 128 to 255 give the same checksum on every platform,
// whether char is signed (x86, Windows, macOS) or unsigned (Linux on ARM).
// The expected values are the ones of x86.
TEST_CASE ("CalcCheckSumOfBytesFrom128IsTheSameOnEveryPlatform")
{
	const char bytes[] = {'\x80', '\xFF', '\x7F', '\x00', '\xC8'};
	CHECK (calcCheckSum (bytes, sizeof (bytes), 0) == 0xfffff9cbu);

	CHECK (calcCheckSum (uint8_t (255), 0) == 0xffffffffu);
	CHECK (calcCheckSum (uint16_t (200), 0) == 0xffffff91u);
	CHECK (calcCheckSum (-1, 0) == 0xfffffff8u);
	CHECK (calcCheckSum (1000, 0) == 0xffffff53u);

	// player names in UTF-8: "Jürgen", "Владыка"
	CHECK (calcCheckSum (std::string ("J\xC3\xBCrgen"), 0) == 0x00000d04u);
	CHECK (calcCheckSum (std::string ("\xD0\x92\xD0\xBB\xD0\xB0\xD0\xB4\xD1\x8B\xD0\xBA\xD0\xB0"), 0) == 0xffefc087u);

	// part of the model checksum (cPlayer::getChecksum()): the alpha of every color is 255
	CHECK (cRgbColor::red().getChecksum (0) == 0xfffffffeu);
	CHECK (sPlayerSettings{"Player", cRgbColor::red()}.getCheckSum (0) == 0x00016e77u);
}
