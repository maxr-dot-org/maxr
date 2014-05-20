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

#ifndef utility_iequalsH
#define utility_iequalsH

#include <string>
#include <cctype>

/**
 * Case insensitive string comparison.
 * (very basic implementation; does not support Unicode encoded strings!)
 */
bool iequals (const std::string& a, const std::string& b)
{
	if (a.size () != b.size ()) return false;

	for (size_t i = 0; i < a.size (); ++i)
	{
		if (std::tolower (a[i]) != std::tolower (b[i])) return false;
	}
	return true;
}

#endif // utility_iequalsH
