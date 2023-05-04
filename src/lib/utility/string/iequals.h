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

#ifndef utility_string_iequalsH
#define utility_string_iequalsH

#include <cctype>
#include <string_view>

/**
 * Case insensitive string comparison.
 * (very basic implementation; does not support Unicode encoded strings!)
 */
static inline bool iequals (const std::string_view& a, const std::string_view& b)
{
	if (a.size() != b.size()) return false;

	for (size_t i = 0; i < a.size(); ++i)
	{
		if (std::tolower (static_cast<unsigned char> (a[i])) != std::tolower (static_cast<unsigned char> (b[i]))) return false;
	}
	return true;
}

#endif // utility_string_iequalsH
