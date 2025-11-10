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

#ifndef utility_versionH
#define utility_versionH

#include "utility/serialization/serialization.h"

#include <string>
#include <string_view>

/**
 * Class representing a simple 3-part version of the form
 *
 *   "<major>.<minor>.<revision>"
 *
 */
class cVersion
{
public:
	cVersion() = default;
	explicit cVersion (std::string_view string);
	explicit cVersion (int major, int minor = 0, int revision = 0);

	int getMajor() const;
	void setMajor (int value);

	int getMinor() const;
	void setMinor (int value);

	int getRevision() const;
	void setRevision (int value);

	/**
	 * Parses a version from the string.
	 *
	 * If only one or two parts are are given
	 * in the string the others will be assumed to be zero
	 *
	 * @param string The string to parse the version from.
	 */
	void parseFromString (std::string_view string);
	std::string toString() const;

	bool operator== (const cVersion&) const = default;
	auto operator<=> (const cVersion&) const = default;

	template <ArchiveInOrOut Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (major);
		archive & NVP (minor);
		archive & NVP (revision);
		// clang-format on
	}

private:
	int major = 0;
	int minor = 0;
	int revision = 0;
};

#endif // utility_versionH
