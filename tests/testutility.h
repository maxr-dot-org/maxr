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

#ifndef tests_testutilityH
#define tests_testutilityH

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class cModel;
class cStaticMap;
class cUnitsData;

/**
 * A file with a unique name in the temp directory,
 * that is removed when the object is destroyed.
 */
class cTemporaryFile
{
public:
	explicit cTemporaryFile (const std::string& extension);
	cTemporaryFile (const cTemporaryFile&) = delete;
	cTemporaryFile& operator= (const cTemporaryFile&) = delete;
	~cTemporaryFile();

	const std::filesystem::path& getPath() const { return path; }
	void write (const std::vector<unsigned char>& bytes) const;

private:
	std::filesystem::path path;
};

/**
 * The bytes of a map file (WRL format) of size x size fields of ground:
 * the header, the mini map, the fields (all of terrain 0)
 * and one terrain (graphic, palette, terrain type).
 */
std::vector<unsigned char> makeGroundMapFile (std::uint16_t size);

/**
 * A map of size x size fields of ground, loaded from a temporary map file.
 * Loading a map initializes cSettings, as in the game.
 */
std::shared_ptr<cStaticMap> loadGroundMap (std::uint16_t size);

/**
 * Prepares the model of a test: a map of 16x16 fields of ground,
 * the unit data, the default game settings and two players.
 */
[[nodiscard]] bool initModel (cModel&, std::shared_ptr<cUnitsData>);

#endif
