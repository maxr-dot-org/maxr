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

#include "utility/string/toNumber.h"

#include <doctest.h>

//------------------------------------------------------------------------------
TEST_CASE ("parseInt")
{
	const std::pair<std::string_view, std::pair<std::optional<int>, std::size_t>> v[]{
		{"42", {42, 2u}},
		{"42.412", {42, 2u}},
		{"42e0", {42, 2u}},
		{"", {std::nullopt, 0u}},
		{"abc", {std::nullopt, 0u}},
	};
	for (auto [s, expected] : v)
	{
		CHECK (parseIntegerT<int> (s) == expected);
	}
}

//------------------------------------------------------------------------------
TEST_CASE ("toInt")
{
	const std::pair<std::string_view, std::optional<int>> v[]{
		{"42", 42},
		{"42.412", std::nullopt},
		{"42e0", std::nullopt},
		{"", std::nullopt},
		{"abc", std::nullopt},
	};
	for (const auto [s, expected] : v)
	{
		CHECK (toInt (s) == expected);
	}
}
