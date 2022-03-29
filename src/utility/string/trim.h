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

#include "utility/ranges.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <locale>

static inline std::string& trim_left (std::string& s)
{
	s.erase (s.begin(), ranges::find_if (s, std::not1 (std::ptr_fun<int, int> (std::isspace))));
	return s;
}

static inline std::string& trim_right (std::string& s)
{
	s.erase (std::find_if (s.rbegin(), s.rend(), std::not1 (std::ptr_fun<int, int> (std::isspace))).base(), s.end());
	return s;
}

static inline std::string& trim (std::string& s)
{
	return trim_left (trim_right (s));
}

static inline std::string trim_left_copy (const std::string& s)
{
	auto s2 = s;
	return trim_left (s2);
}

static inline std::string trim_right_copy (const std::string& s)
{
	auto s2 = s;
	return trim_right (s2);
}

static inline std::string trim_copy (const std::string& s)
{
	auto s2 = s;
	return trim (s2);
}

#endif // utility_string_trimH
