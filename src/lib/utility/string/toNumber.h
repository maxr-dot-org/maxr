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

#ifndef utility_string_toNumberH
#define utility_string_toNumberH

#include <charconv>
#include <optional>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

template <typename T>
std::pair<std::optional<T>, std::size_t> parseIntegerT (std::string_view s)
{
	static_assert (std::is_integral_v<T>);
	T n;
	const auto [ptr, ec] = std::from_chars (s.data(), s.data() + s.size(), n);
	if (ec != std::errc{})
	{
		return {std::nullopt, ptr - s.data()};
	}
	return {n, ptr - s.data()};
}

//------------------------------------------------------------------------------
template <typename T>
std::optional<T> toIntegerT (std::string_view s)
{
	const auto [res, offset] = parseIntegerT<T> (s);
	if (offset != s.size())
	{
		return std::nullopt;
	}
	return res;
}

//------------------------------------------------------------------------------
inline std::optional<int> toInt (std::string_view s)
{
	return toIntegerT<int> (s);
}

//------------------------------------------------------------------------------
inline std::optional<long long> toLongLong (std::string_view s)
{
	return toIntegerT<long long> (s);
}

#endif // utility_string_toNumberH
