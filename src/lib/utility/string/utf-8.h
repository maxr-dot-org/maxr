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

#ifndef utility_string_utf8H
#define utility_string_utf8H

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace utf8
{
	void decreasePos (::std::string_view, ::std::size_t& pos);
	void increasePos (::std::string_view, ::std::size_t& pos);

	::std::uint32_t decodeUnicode (::std::string_view, ::std::size_t& pos);

	void pop_back (::std::string&);
	void append_unicode (::std::string&, ::std::uint32_t);
	::std::string to_utf8 (::std::uint32_t);

	//--------------------------------------------------------------------------
	template <typename F>
	void for_each (const ::std::string_view& text, F f)
	{
		for (::std::size_t i = 0; i != text.size(); /* Empty */)
		{
			f (decodeUnicode (text, i));
		}
	}

	//--------------------------------------------------------------------------
	inline std::string to_string (const std::u8string& s)
	{
		return {s.begin(), s.end()};
	}

	//--------------------------------------------------------------------------
	inline std::string to_string (const std::filesystem::path& p)
	{
		return to_string (p.u8string());
	}

} // namespace utf8

#endif
