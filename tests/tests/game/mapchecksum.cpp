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

#include "mapdownloader/mapdownload.h"
#include "testutility.h"
#include "utility/crc.h"

#include <cstddef>
#include <cstdint>
#include <doctest.h>
#include <random>
#include <vector>

//------------------------------------------------------------------------------
// The size of a map is read as unsigned: on x86 (signed char) a map of 128 to
// 255 fields was summed only over its first (256 - size)^2 * 3 bytes, so two
// versions of such a map could have the same checksum.
TEST_CASE ("MapCheckSumCoversAllFieldsOfMapsFrom128To255Fields")
{
	const cTemporaryFile file (".wrl");
	const std::uint16_t sizes[] = {112, 128, 160, 250, 255};

	for (const std::uint16_t size : sizes)
	{
		CAPTURE (size);
		// header, mini map and fields (pseudo-random bytes), graphics
		auto bytes = makeGroundMapFile (size);
		const std::size_t relevantSize = 9 + std::size_t (size) * size * 3;
		std::minstd_rand generator (size);
		for (std::size_t i = 9; i != relevantSize; ++i)
			bytes[i] = static_cast<unsigned char> (generator() & 0xFF);

		file.write (bytes);
		const auto checksum = MapDownload::calculateCheckSum (file.getPath());
		CHECK (checksum == calcCheckSum (reinterpret_cast<const char*> (bytes.data()), relevantSize, 0));

		// the last byte of the fields is in the checksum
		bytes[relevantSize - 1] ^= 0x55;
		file.write (bytes);
		CHECK (MapDownload::calculateCheckSum (file.getPath()) != checksum);
		bytes[relevantSize - 1] ^= 0x55;

		// the graphics after it are not
		bytes[relevantSize + 2] ^= 0x55;
		file.write (bytes);
		CHECK (MapDownload::calculateCheckSum (file.getPath()) == checksum);
	}
}

//------------------------------------------------------------------------------
// A map file shorter than its header (9 bytes) has no checksum, like a missing
// file. The size of the map was read from the bytes 5 to 8 of a buffer of the
// size of the file: after its end, an empty file crashed.
TEST_CASE ("MapCheckSumOfFileShorterThanHeaderIsZero")
{
	const cTemporaryFile file (".wrl");
	const auto bytes = makeGroundMapFile (16);

	for (int length = 0; length != 9; ++length)
	{
		CAPTURE (length);
		file.write (std::vector<unsigned char> (bytes.begin(), bytes.begin() + length));
		CHECK (MapDownload::calculateCheckSum (file.getPath()) == 0);
	}
}

//------------------------------------------------------------------------------
// A map file, whose header promises more fields than the file has, has no
// checksum. 3 * width * height was an int: from 26755 fields it overflowed, and
// for 37838 and 53510 fields it wrapped to 175436 and 25708, so a part of the
// file got a checksum.
TEST_CASE ("MapCheckSumOfFileShorterThanItsFieldsIsZero")
{
	const cTemporaryFile file (".wrl");
	auto bytes = makeGroundMapFile (255); // 199951 bytes
	const std::uint16_t sizes[] = {300, 26755, 37838, 53510, 65535};

	for (const std::uint16_t size : sizes)
	{
		CAPTURE (size);
		bytes[5] = bytes[7] = static_cast<unsigned char> (size & 0xFF);
		bytes[6] = bytes[8] = static_cast<unsigned char> (size >> 8);
		file.write (bytes);
		CHECK (MapDownload::calculateCheckSum (file.getPath()) == 0);
	}

	// a file, that ends right after the fields, has a checksum; one byte less has none
	bytes = makeGroundMapFile (16);
	const std::size_t relevantSize = 9 + std::size_t (16) * 16 * 3;
	bytes.resize (relevantSize);
	file.write (bytes);
	CHECK (MapDownload::calculateCheckSum (file.getPath()) == calcCheckSum (reinterpret_cast<const char*> (bytes.data()), relevantSize, 0));

	bytes.resize (relevantSize - 1);
	file.write (bytes);
	CHECK (MapDownload::calculateCheckSum (file.getPath()) == 0);
}
