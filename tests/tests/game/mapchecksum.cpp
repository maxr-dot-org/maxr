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
#include "utility/crc.h"

#include <cstdint>
#include <doctest.h>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

namespace
{
	//--------------------------------------------------------------------------
	void appendLE16 (std::vector<char>& bytes, int value)
	{
		bytes.push_back (static_cast<char> (value & 0xFF));
		bytes.push_back (static_cast<char> ((value >> 8) & 0xFF));
	}

	//--------------------------------------------------------------------------
	// a map file (WRL format) of size x size fields: header, mini map and
	// fields (pseudo-random bytes), one terrain graphic, palette
	std::vector<char> makeMapFile (int size)
	{
		std::vector<char> bytes{'W', 'R', 'L'};
		appendLE16 (bytes, 1);
		appendLE16 (bytes, size);
		appendLE16 (bytes, size);
		std::minstd_rand generator (size);
		for (int i = 0; i != size * size * 3; ++i) // mini map, fields
			bytes.push_back (static_cast<char> (generator() & 0xFF));
		appendLE16 (bytes, 1); // number of terrains
		bytes.insert (bytes.end(), 64 * 64 + 256 * 3 + 1, '\0'); // terrain graphic, palette, terrain type
		return bytes;
	}

	//--------------------------------------------------------------------------
	void writeFile (const std::filesystem::path& filename, const std::vector<char>& bytes)
	{
		std::ofstream file (filename, std::ios::binary | std::ios::trunc);
		file.write (bytes.data(), bytes.size());
	}
} // namespace

//------------------------------------------------------------------------------
// The size of a map is read as unsigned: on x86 (signed char) a map of 128 to
// 255 fields was summed only over its first (256 - size)^2 * 3 bytes, so two
// versions of such a map could have the same checksum.
TEST_CASE ("MapCheckSumCoversAllFieldsOfMapsFrom128To255Fields")
{
	const auto filename = std::filesystem::temp_directory_path() / ("maxr_tests_" + std::to_string (std::random_device{}()) + ".wrl");

	for (int size : {112, 128, 160, 250, 255})
	{
		CAPTURE (size);
		auto bytes = makeMapFile (size);
		const std::size_t relevantSize = 9 + size * size * 3; // header, mini map, fields

		writeFile (filename, bytes);
		const auto checksum = MapDownload::calculateCheckSum (filename);
		CHECK (checksum == calcCheckSum (bytes.data(), relevantSize, 0));

		// the last byte of the fields is in the checksum
		bytes[relevantSize - 1] ^= 0x55;
		writeFile (filename, bytes);
		CHECK (MapDownload::calculateCheckSum (filename) != checksum);
		bytes[relevantSize - 1] ^= 0x55;

		// the graphics after it are not
		bytes[relevantSize + 2] ^= 0x55;
		writeFile (filename, bytes);
		CHECK (MapDownload::calculateCheckSum (filename) == checksum);
	}
	std::filesystem::remove (filename);
}
