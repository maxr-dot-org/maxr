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

#include "output/video/unifonts.h"
#include "unittesttag.h"

#include <doctest.h>
#include <regex>

static constexpr const std::size_t charWidth = 2;
static constexpr const std::size_t charHeight = 2;

//------------------------------------------------------------------------------
template <typename... Args>
cUnicodeFont::cUnicodeFont (cUnitTestTag, Args...)
{
	const int w = charWidth;
	const int h = charHeight;

	for (auto& surface : charsNormal)
	{
		surface = UniqueSurface (SDL_CreateRGBSurface (0, w, h, 32, 0, 0, 0, 0));
	}
}

namespace
{
	//--------------------------------------------------------------------------
	std::string concatenateLines (const std::vector<std::string>& lines, const char* newLine = "\n")
	{
		const char* sep = "";
		std::string res;

		for (const auto& line : lines)
		{
			res += sep;
			res += line;
			sep = newLine;
		}
		return res;
	}

	//--------------------------------------------------------------------------
	void TestBreakLine (const std::vector<std::string> expected)
	{
		const auto font = std::make_unique<cUnicodeFont> (cUnitTestTag{});
		const auto text = concatenateLines (expected);

		auto res = font->breakText (text, text.size() * charWidth, eUnicodeFontType::LatinNormal);

		CHECK (std::regex_replace (text, std::regex ("\n"), "|") == std::regex_replace (concatenateLines (res, "|"), std::regex ("\n"), "\\n"));
#if 0 // similar check, but by part
		REQUIRE (expected.size() == res.size());
		for (std::size_t i = 0; i != expected.size(); ++i)
		{
			CHECK (expected[i] == res[i]);
		}
#endif
	}
} // namespace

//------------------------------------------------------------------------------
TEST_CASE ("BreakText_newline_simple")
{
	TestBreakLine ({"a", "b", "c", "d"});
}

//------------------------------------------------------------------------------
TEST_CASE ("BreakText_newline_prefix")
{
	TestBreakLine ({" a", " b", " c", " d"});
}

//------------------------------------------------------------------------------
TEST_CASE ("BreakText_newline_emptyline")
{
	TestBreakLine ({"a", "", "c", ""});
}

namespace
{
	//--------------------------------------------------------------------------
	void TestBreakSpace (const std::string& expected, const std::string& text, std::size_t nbPixels)
	{
		const auto font = std::make_unique<cUnicodeFont> (cUnitTestTag{});

		auto res = font->breakText (text, nbPixels, eUnicodeFontType::LatinNormal);

		CHECK (expected == concatenateLines (res, "|"));
	}
} // namespace

//------------------------------------------------------------------------------
TEST_CASE ("BreakText_space_smallword")
{
	TestBreakSpace ("a|bb|c d|e", "a bb c d e", 3 * charWidth + 1);
}

//------------------------------------------------------------------------------
TEST_CASE ("BreakText_space_longword")
{
	TestBreakSpace ("aa|bbbb|ccc", "aa bbbb ccc", 2 * charWidth + 1);
}

//------------------------------------------------------------------------------
TEST_CASE ("BreakText_space_manyspaces")
{
	TestBreakSpace ("a|bb|cc|dd", "a     bb     cc   dd  ", 2 * charWidth + 1);
}

//------------------------------------------------------------------------------
TEST_CASE ("BreakText_space_eolandspace")
{
	TestBreakSpace ("a|bb|cc|dd", "a  \nbb\n     cc  dd", 2 * charWidth + 1);
}
