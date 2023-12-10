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
#ifndef output_video_unifontsH
#define output_video_unifontsH

#include "SDLutility/uniquesurface.h"

#include <SDL.h>
#include <string>
#include <vector>

class cPosition;

/** different fonttypes*/
enum class eUnicodeFontType
{
	LatinNormal,
	LatinNormalRed,
	LatinBig,
	LatinBigGold,
	LatinSmallWhite,
	LatinSmallRed,
	LatinSmallGreen,
	LatinSmallYellow,
};

/** different sizes that fonttypes can have*/
enum class eUnicodeFontSize
{
	Normal,
	Big,
	Small,
};

/** different ISO-8559-X charsets*/
enum class eUnicodeFontCharset
{
	Iso8559_ALL = 0, // main part of the charsets which is the same in all charsets
	Iso8559_1,
	Iso8559_2,
	Iso8559_3,
	Iso8559_4,
	Iso8559_5,
	Iso8559_6,
	Iso8559_7,
	Iso8559_8,
	Iso8559_9,
	Iso8559_10,
	Iso8559_11,
	Iso8559_12, // doesn't exists but is just a placeholder that the enum-numbers are the same as the iso-numbers
	Iso8559_13,
	Iso8559_14,
	Iso8559_15,
	Iso8559_16,
};

/**
 * @author alzi alias DoctorDeath
 * Loads the fontbitmaps from a ISO-8859 structure to a unicode structure
 * and handles theire output to the screen
*/
class cUnicodeFont
{
public:
	cUnicodeFont();

	// Entry point for UT to works with dummy surface.
	template <typename... Args>
	cUnicodeFont (struct cUnitTestTag, Args...);

	void setTargetSurface (SDL_Surface* surface) { this->surface = surface; }
	SDL_Surface* getTargetSurface() { return surface; }

	/**
	 * Displays a text
	 * @author beko
	 * @param x position x to start drawing
	 * @param y position y to start drawing
	 * @param sText text to draw
	 * @param eBitmapFontType enum of fonttype. LatinNormal is default
	 */
	void showText (int x, int y, const std::string& sText, eUnicodeFontType fonttype = eUnicodeFontType::LatinNormal);

	void showText (const cPosition& position, const std::string& sText, eUnicodeFontType fonttype = eUnicodeFontType::LatinNormal);

	/**
	 * Displays a text as block.<br><br>
	 * This does <b>not</b> allow blanks in line. Linebreaks are interpreted.
	 * Unneeded blanks will be snipped.<br><br>
	 * Example:
	 * "Headline\n\n This is my text for a textblock that get's linebreaked automagically"!
	 * @author beko
	 * @param rDest SDL_Rect for position and wide of textbox.
	 *        Height is not taken care of!
	 * @param sText text to draw
	 * @param eBitmapFontType enum of fonttype. LatinNormal is default
	 */
	int showTextAsBlock (SDL_Rect rDest, const std::string& sText, eUnicodeFontType fonttype = eUnicodeFontType::LatinNormal);
	/**
	 * Displays a text centered on given X
	 * @author beko
	 * @param x Use X for position to center on.<br>Y is not taken care of!
	 * @param y position y to start drawing
	 * @param sText text to draw
	 * @param eBitmapFontType enum of fonttype. LatinNormal is default
	 */
	void showTextCentered (int x, int y, const std::string& sText, eUnicodeFontType fonttype = eUnicodeFontType::LatinNormal);

	void showTextCentered (const cPosition& pos, const std::string& sText, eUnicodeFontType fonttype = eUnicodeFontType::LatinNormal);
	/**
	 * Calculates the needed width for a text in pixels
	 * @author beko
	 * @param sText text to check
	 * @param eBitmapFontType enum of fonttype. LatinNormal is default
	 * @return needed width for text
	 */
	int getTextWide (const std::string& sText,
	                 eUnicodeFontType fonttype = eUnicodeFontType::LatinNormal) const;
	/**
	 * Calculates the needed space for a text in pixels
	 * @author beko
	 * @param sText text to check
	 * @param eBitmapFontType enum of fonttype. LatinNormal is default
	 * @return SDL_Rect with needed width and height for text
	 */
	SDL_Rect getTextSize (const std::string& sText,
	                      eUnicodeFontType fonttype = eUnicodeFontType::LatinNormal) const;
	/**
	 * Holds information of font height
	 * @author beko
	 * @param eBitmapFontType enum of fonttype. LatinNormal is default
	 * @return Height of fonttype in pixels
	 */
	int getFontHeight (eUnicodeFontType fonttype = eUnicodeFontType::LatinNormal) const;
	/**
	 * Holds information of font size
	 * @author alzi
	 * @param eBitmapFontType enum of fonttype.
	 * @return eUnicodeFontSize enum size of fonttype
	 */
	static eUnicodeFontSize getFontSize (eUnicodeFontType fonttype);

	std::string shortenStringToSize (const std::string& str, int size, eUnicodeFontType fonttype) const;
	std::vector<std::string> breakText (const std::string& text, int maximalWidth, eUnicodeFontType) const;

private:
	using FontTypeSurfaces = UniqueSurface[0xFFFF];

	int getUnicodeCharacterWidth (Uint16 unicodeCharacter, eUnicodeFontType fonttype) const;
	/**
	 * loads all characters of a ISO table and fonttype.
	 * @author beko
	 * @param charset the charset which should be loaded.
	 * @param fonttype the fonttype which should be loaded.
	 */
	void loadChars (eUnicodeFontCharset charset, eUnicodeFontType fonttype);
	/**
	 * returns the character array of a fonttype.
	 * @author alzi alias DoctorDeath
	 * @param fonttype the fonttype of which the character
	 *        array should be returned.
	 * @return the character array for the fonttype.
	 */
	const FontTypeSurfaces* getFontTypeSurfaces (eUnicodeFontType fonttype) const;
	FontTypeSurfaces* getFontTypeSurfaces (eUnicodeFontType fonttype);
	/**
	 * loads the ISO-8859 bitmap font surface
	 * @author alzi alias DoctorDeath
	 * @param charset the charset which bitmap should be loaded.
	 * @param fonttype the fonttype which bitmap should be loaded.
	 * @return the bitmap surface
	 */
	UniqueSurface loadCharsetSurface (eUnicodeFontCharset charset, eUnicodeFontType fonttype);
	/**
	 * returns the iso page with the unicode positions of the characters
	 * in a ISO-8859 font
	 * @author alzi alias DoctorDeath
	 * @param charset the charset for that the iso page should be returned.
	 * @return the iso page
	 */
	const unsigned short* getIsoPage (eUnicodeFontCharset charset) const;
	int drawWithBreakLines (SDL_Rect rDest, const std::string& sText, eUnicodeFontType fonttype);

private:
	// character surfaces.
	// Since SDL maximal gives us the unicodes
	// from BMP we need 0xFFFF surfaces at maximum
	UniqueSurface charsNormal[0xFFFF];
	UniqueSurface charsNormalRed[0xFFFF];
	UniqueSurface charsSmallWhite[0xFFFF];
	UniqueSurface charsSmallGreen[0xFFFF];
	UniqueSurface charsSmallRed[0xFFFF];
	UniqueSurface charsSmallYellow[0xFFFF];
	UniqueSurface charsBig[0xFFFF];
	UniqueSurface charsBigGold[0xFFFF];

	// target surface where to draw.
	SDL_Surface* surface = nullptr;

public:
	static std::unique_ptr<cUnicodeFont> font;
};

#endif
