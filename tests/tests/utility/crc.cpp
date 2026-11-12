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

#include "utility/crc.h"

#include "game/data/player/playersettings.h"
#include "utility/color.h"

#include <cstdint>
#include <doctest.h>
#include <iterator>
#include <random>
#include <string>
#include <vector>

namespace
{
	//--------------------------------------------------------------------------
	// the checksum of the bytes of all values, low byte first
	uint32_t calcCheckSumByteByByte (const std::vector<uint16_t>& data, uint32_t checksum)
	{
		std::vector<char> bytes;
		for (const uint16_t value : data)
		{
			bytes.push_back (static_cast<char> (value & 0xFF));
			bytes.push_back (static_cast<char> (value >> 8));
		}
		return calcCheckSum (bytes.data(), bytes.size(), checksum);
	}

	//--------------------------------------------------------------------------
	// the generic calcCheckSum (const std::vector<T>&, uint32_t)
	uint32_t calcCheckSumValueByValue (const std::vector<uint16_t>& data, uint32_t checksum)
	{
		for (const uint16_t value : data)
		{
			checksum = calcCheckSum (value, checksum);
		}
		return checksum;
	}
} // namespace

//------------------------------------------------------------------------------
TEST_CASE ("CalcCheckSumUint16VectorKeepsTheValues")
{
	// values of the generic code before the overload
	// (bytes below 128: the same with a signed and an unsigned char)
	const uint32_t start = 0x12345678;

	CHECK (calcCheckSum (std::vector<uint16_t>{}, 0) == 0);
	CHECK (calcCheckSum (std::vector<uint16_t>{}, start) == start);

	const uint32_t zeros[] = {0x48d159e0u, 0x23456781u, 0x8d159e04u, 0x34567812u, 0xd159e048u, 0x45678123u, 0x159e048du, 0x56781234u, 0x59e048d1u};
	for (std::size_t n = 1; n <= 9; ++n)
	{
		CAPTURE (n);
		CHECK (calcCheckSum (std::vector<uint16_t> (n, 0), start) == zeros[n - 1]);
	}

	const std::vector<uint16_t> small{1, 0, 0, 0, 0, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0};
	CHECK (calcCheckSum (small, 0) == 0x160000a0u);
	CHECK (calcCheckSum (small, start) == 0xa3159ea4u);

	const std::vector<uint16_t> mixed{0, 127, 256, 300, 0x7F7F, 0, 0, 0, 0, 0x0100, 5};
	CHECK (calcCheckSum (mixed, 0) == 0x0427100eu);
	CHECK (calcCheckSum (mixed, start) == 0xa22b9d23u);
}

//------------------------------------------------------------------------------
TEST_CASE ("CalcCheckSumUint16VectorEqualsByteByByte")
{
	std::mt19937 random (12345);
	const uint16_t values[] = {1, 2, 127, 128, 255, 256, 300, 32767, 32768, 65535};

	for (int test = 0; test != 2000; ++test)
	{
		std::vector<uint16_t> data (random() % 70);
		const unsigned int zeroPercent = random() % 101;
		for (auto& value : data)
		{
			if (random() % 100 < zeroPercent)
				value = 0;
			else if (random() % 2 == 0)
				value = values[random() % std::size (values)];
			else
				value = static_cast<uint16_t> (random());
		}
		const uint32_t start = test % 3 == 0 ? 0 : static_cast<uint32_t> (random());

		CAPTURE (test);
		CHECK (calcCheckSum (data, start) == calcCheckSumByteByByte (data, start));
		CHECK (calcCheckSum (data, start) == calcCheckSumValueByValue (data, start));
	}
}

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

	// the overload for a vector of uint16_t (range maps)
	CHECK (calcCheckSum (std::vector<uint16_t>{200, 0, 0xFF80, 0x80FF}, 0) == 0xffffdff9u);
	CHECK (calcCheckSum (std::vector<uint16_t>{200, 0, 0xFF80, 0x80FF}, 0x12345678) == 0x3456578cu);
	CHECK (calcCheckSum (std::vector<uint16_t>{0, 0, 0, 0, 0x8000, 0, 0, 0, 0, 0, 0x00FF, 0xFFFF}, 0) == 0xffe03ff4u);

	// player names in UTF-8: "Jürgen", "Владыка"
	CHECK (calcCheckSum (std::string ("J\xC3\xBCrgen"), 0) == 0x00000d04u);
	CHECK (calcCheckSum (std::string ("\xD0\x92\xD0\xBB\xD0\xB0\xD0\xB4\xD1\x8B\xD0\xBA\xD0\xB0"), 0) == 0xffefc087u);

	// part of the model checksum (cPlayer::getChecksum()): the alpha of every color is 255
	CHECK (cRgbColor::red().getChecksum (0) == 0xfffffffeu);
	CHECK (sPlayerSettings{"Player", cRgbColor::red()}.getCheckSum (0) == 0x00016e77u);
}
