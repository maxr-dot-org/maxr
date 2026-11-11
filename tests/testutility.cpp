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

#include "testutility.h"

#include "game/data/gamesettings.h"
#include "game/data/map/map.h"
#include "game/data/model.h"
#include "game/data/player/playerbasicdata.h"
#include "utility/color.h"

#include <cstddef>
#include <fstream>
#include <random>
#include <system_error>
#include <utility>

namespace
{
	//--------------------------------------------------------------------------
	void appendLE16 (std::vector<unsigned char>& bytes, std::uint16_t value)
	{
		bytes.push_back (static_cast<unsigned char> (value & 0xFF));
		bytes.push_back (static_cast<unsigned char> (value >> 8));
	}
} // namespace

//------------------------------------------------------------------------------
cTemporaryFile::cTemporaryFile (const std::string& extension) :
	path (std::filesystem::temp_directory_path() / ("maxr_tests_" + std::to_string (std::random_device{}()) + extension))
{}

//------------------------------------------------------------------------------
cTemporaryFile::~cTemporaryFile()
{
	std::error_code error; // a destructor does not throw
	std::filesystem::remove (path, error);
}

//------------------------------------------------------------------------------
void cTemporaryFile::write (const std::vector<unsigned char>& bytes) const
{
	std::ofstream file (path, std::ios::binary | std::ios::trunc);
	file.write (reinterpret_cast<const char*> (bytes.data()), static_cast<std::streamsize> (bytes.size()));
}

//------------------------------------------------------------------------------
std::vector<unsigned char> makeGroundMapFile (std::uint16_t size)
{
	const std::size_t fieldCount = std::size_t (size) * size;

	std::vector<unsigned char> bytes{'W', 'R', 'L'};
	appendLE16 (bytes, 1);
	appendLE16 (bytes, size); // width
	appendLE16 (bytes, size); // height
	bytes.insert (bytes.end(), fieldCount, 0); // mini map
	bytes.insert (bytes.end(), fieldCount * 2, 0); // fields: terrain 0
	appendLE16 (bytes, 1); // number of terrains
	bytes.insert (bytes.end(), 64 * 64, 0); // graphic of terrain 0
	bytes.insert (bytes.end(), 256 * 3, 0); // palette
	bytes.push_back (0); // terrain 0: ground
	return bytes;
}

//------------------------------------------------------------------------------
std::shared_ptr<cStaticMap> loadGroundMap (std::uint16_t size)
{
	const cTemporaryFile file (".wrl");
	file.write (makeGroundMapFile (size));

	auto staticMap = std::make_shared<cStaticMap>();
	if (!staticMap->loadMap (file.getPath())) return nullptr;
	return staticMap;
}

//------------------------------------------------------------------------------
bool initModel (cModel& model, std::shared_ptr<cUnitsData> unitsData)
{
	auto staticMap = loadGroundMap (16);
	if (!staticMap) return false;

	model.setUnitsData (std::move (unitsData));
	model.setGameSettings (cGameSettings{});
	model.setMap (staticMap);
	model.setPlayerList ({cPlayerBasicData ({"Player 0", cRgbColor::red()}, 0, false), cPlayerBasicData ({"Player 1", cRgbColor::blue()}, 1, false)});
	return true;
}
