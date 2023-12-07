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

#include "utf-8.h"

#include "utility/log.h"

#include <algorithm>
#include <stdexcept>

namespace utf8
{

	//--------------------------------------------------------------------------
	void decreasePos (const ::std::string& text, ::std::size_t& pos)
	{
		if (pos == 0)
		{
			return;
		}
		unsigned char c = text[pos - 1];
		while ((c & 0xC0) == 0x80)
		{
			if (pos <= 1)
			{
				Log.warn ("Invalid UTF-8 string in text: '" + text + "'");
				break;
			}

			pos--;
			c = text[pos - 1];
		}
		pos--;
	}

	//--------------------------------------------------------------------------
	void increasePos (const ::std::string& text, ::std::size_t& pos)
	{
		if (text.length() <= pos)
		{
			return;
		}
		const unsigned char c = text[pos];

		if ((c & 0xF8) == 0xF0)
		{
			pos += 4;
		}
		if ((c & 0xF0) == 0xE0)
		{
			pos += 3;
		}
		else if ((c & 0xE0) == 0xC0)
		{
			pos += 2;
		}
		else
		{
			pos += 1;
		}

		if (text.length() < pos)
		{
			pos = text.length();
			Log.warn ("Invalid UTF-8 string in text: '" + text + "'");
		}
	}

	//--------------------------------------------------------------------------
	void pop_back (::std::string& s)
	{
		::std::size_t size = s.size();
		decreasePos (s, size);
		s.resize (size);
	}

	//--------------------------------------------------------------------------
	void append_unicode (::std::string& s, ::std::uint32_t unicode)
	{
		if (unicode < 0x80)
		{
			s.push_back (unicode & 0x7F);
		}
		else if (unicode < 0x08'00)
		{
			s.push_back (((unicode >> 6) & 0x1F) | 0xC0);
			s.push_back ((unicode & 0x3F) | 0x80);
		}
		else if (unicode < 0x00'01'00'00)
		{
			s.push_back (((unicode >> 12) & 0x0F) | 0xE0);
			s.push_back (((unicode >> 6) & 0x3F) | 0x80);
			s.push_back ((unicode & 0x3F) | 0x80);
		}
		else
		{
			s.push_back (((unicode >> 18) & 0x07) | 0xF0);
			s.push_back (((unicode >> 12) & 0x3F) | 0x80);
			s.push_back (((unicode >> 6) & 0x3F) | 0x80);
			s.push_back ((unicode & 0x3F) | 0x80);
		}
	}

	//--------------------------------------------------------------------------
	::std::string to_utf8 (::std::uint32_t unicode)
	{
		::std::string res;
		append_unicode (res, unicode);
		return res;
	}

	//--------------------------------------------------------------------------
	::std::uint32_t decodeUnicode (const ::std::string& text, ::std::size_t& pos)
	{
		if (text.size() <= pos)
		{
			throw ::std::out_of_range ("invalid position for decodeUnicode");
		}
		const ::std::uint32_t c0 = static_cast<unsigned char> (text[pos]);
		const auto is_extra_char = [] (char c) { return (c & 0b1100'0000) == 0b1000'0000; };
		const auto check_extra_size = [&] (int size) {
			if (text.size() <= pos + size || !std::all_of (&text[pos + 1], &text.c_str()[pos + 1 + size], is_extra_char))
			{
				Log.warn ("Invalid UTF-8 string in text: '" + text + "' at pos " + ::std::to_string (pos));
				throw ::std::out_of_range ("invalid position for decodeUnicode");
			}
		};
		if ((c0 & 0b1111'1000) == 0b1111'0000)
		{
			check_extra_size (3);
			const unsigned char c1 = text[pos + 1] & 0b0011'1111;
			const unsigned char c2 = text[pos + 2] & 0b0011'1111;
			const unsigned char c3 = text[pos + 3] & 0b0011'1111;

			pos += 4;
			return ((c0 & 0b000'0111) << 18) | (c1 << 12) | (c2 << 6) | c3;
		}
		else if ((c0 & 0b1111'0000) == 0b1110'0000)
		{
			check_extra_size (2);
			const unsigned char c1 = text[pos + 1] & 0b0011'1111;
			const unsigned char c2 = text[pos + 2] & 0b0011'1111;

			pos += 3;
			return ((c0 & 0b0000'1111) << 12) | (c1 << 6) | c2;
		}
		else if ((c0 & 0b1110'0000) == 0b1100'0000)
		{
			check_extra_size (1);
			const unsigned char c1 = text[pos + 1] & 0b0011'1111;

			pos += 2;
			return ((c0 & 0b0001'1111) << 6) | c1;
		}
		else
		{
			if ((c0 & 0b1000'0000) != 0)
			{
				Log.warn ("Invalid UTF-8 string in text: '" + text + "' at pos " + ::std::to_string (pos));
				throw ::std::runtime_error ("Invalid utf8 character for decodeUnicode");
			}
			pos += 1;
			return (c0 & 0b0111'1111);
		}
	}

} // namespace utf8
