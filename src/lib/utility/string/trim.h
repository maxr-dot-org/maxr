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

#ifndef utility_string_trimH
#define utility_string_trimH

#include <algorithm>
#include <cctype>
#include <iterator>
#include <locale>
#include <string>
#include <string_view>

static inline std::string& trim_left (std::string& s)
{
	s.erase (s.begin(), std::ranges::find_if (s, [] (unsigned char c) { return !std::isspace (c); }));
	return s;
}

static inline std::string& trim_right (std::string& s)
{
	s.erase (std::find_if (s.rbegin(), s.rend(), [] (unsigned char c) { return !std::isspace (c); }).base(), s.end());
	return s;
}

static inline std::string& trim (std::string& s)
{
	return trim_left (trim_right (s));
}

[[nodiscard]] inline std::string_view trim_left (std::string_view s)
{
	return s.substr (std::distance (s.begin(), std::ranges::find_if (s, [] (unsigned char c) { return !std::isspace (c); })));
}

[[nodiscard]] inline std::string_view trim_right (std::string_view s)
{
	return s.substr (0, std::distance (s.begin(), std::find_if (s.rbegin(), s.rend(), [] (unsigned char c) { return !std::isspace (c); }).base()));
}

[[nodiscard]] inline std::string_view trim (std::string_view s)
{
	return trim_left (trim_right (s));
}

#endif // utility_string_trimH
