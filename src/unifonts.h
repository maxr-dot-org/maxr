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
#ifndef unifontsH
#define unifontsH

#include "utility/autosurface.h"
#include <SDL.h>
#include <string>
#include "defines.h"

class cPosition;

/** different fonttypes*/
enum eUnicodeFontType
{
	FONT_LATIN_NORMAL,
	FONT_LATIN_NORMAL_RED,
	FONT_LATIN_BIG,
	FONT_LATIN_BIG_GOLD,
	FONT_LATIN_SMALL_WHITE,
	FONT_LATIN_SMALL_RED,
	FONT_LATIN_SMALL_GREEN,
	FONT_LATIN_SMALL_YELLOW,
};

/** different sizes that fonttypes can have*/
enum eUnicodeFontSize
{
	FONT_SIZE_NORMAL,
	FONT_SIZE_BIG,
	FONT_SIZE_SMALL,
};

/** different ISO-8559-X charsets*/
enum eUnicodeFontCharset
{
	CHARSET_ISO8559_ALL, // main part of the charsets which is the same in all charsets
	CHARSET_ISO8559_1,
	CHARSET_ISO8559_2,
	CHARSET_ISO8559_3,
	CHARSET_ISO8559_4,
	CHARSET_ISO8559_5,
	CHARSET_ISO8559_6,
	CHARSET_ISO8559_7,
	CHARSET_ISO8559_8,
	CHARSET_ISO8559_9,
	CHARSET_ISO8559_10,
	CHARSET_ISO8559_11,
	CHARSET_ISO8559_12, // doesn't exists but is just a placeholder that the enum-numbers are the same as the iso-numbers
	CHARSET_ISO8559_13,
	CHARSET_ISO8559_14,
	CHARSET_ISO8559_15,
	CHARSET_ISO8559_16,
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

	/**
	 * Wrapper for showText for easy use of SDL_Rects
	 * @author beko
	 * @param rdest destination to start drawing
	 * @param sText text to draw
	 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
	 */
	void showText (SDL_Rect rDest, const std::string& sText,
				   eUnicodeFontType fonttype = FONT_LATIN_NORMAL);
	/**
	 * Displays a text
	 * @author beko
	 * @param x position x to start drawing
	 * @param y position y to start drawing
	 * @param sText text to draw
	 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
	 */
	void showText (int x, int y, const std::string& sText,
				   eUnicodeFontType fonttype = FONT_LATIN_NORMAL);

	void showText (const cPosition& position, const std::string& sText,
				   eUnicodeFontType fonttype = FONT_LATIN_NORMAL);

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
	 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
	 */
	int showTextAsBlock (SDL_Rect rDest, const std::string& sText,
						 eUnicodeFontType fonttype = FONT_LATIN_NORMAL);
	/**
	 * Displays a text centered on given X
	 * @author beko
	 * @param rDest DL_Rect for position.<br>Use X for position to center on.
	 *              <br>Y is not taken care of!
	 * @param sText text to draw
	 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
	 */
	void showTextCentered (SDL_Rect rDest, const std::string& sText,
						   eUnicodeFontType fonttype = FONT_LATIN_NORMAL);
	/**
	 * Displays a text centered on given X
	 * @author beko
	 * @param x Use X for position to center on.<br>Y is not taken care of!
	 * @param y position y to start drawing
	 * @param sText text to draw
	 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
	 */
	void showTextCentered (int x, int y, const std::string& sText,
						   eUnicodeFontType fonttype = FONT_LATIN_NORMAL);

	void showTextCentered (const cPosition& pos, const std::string& sText,
						   eUnicodeFontType fonttype = FONT_LATIN_NORMAL);
	/**
	 * Calculates the needed width for a text in pixels
	 * @author beko
	 * @param sText text to check
	 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
	 * @return needed width for text
	 */
	int getTextWide (const std::string& sText,
					 eUnicodeFontType fonttype = FONT_LATIN_NORMAL);
	/**
	 * Calculates the needed space for a text in pixels
	 * @author beko
	 * @param sText text to check
	 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
	 * @return SDL_Rect with needed width and height for text
	 */
	SDL_Rect getTextSize (const std::string& sText,
						  eUnicodeFontType fonttype = FONT_LATIN_NORMAL);
	/**
	 * Holds information of font height
	 * @author beko
	 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
	 * @return Height of fonttype in pixels
	 */
	int getFontHeight (eUnicodeFontType fonttype = FONT_LATIN_NORMAL);
	/**
	 * Holds information of font size
	 * @author alzi
	 * @param eBitmapFontType enum of fonttype.
	 * @return eUnicodeFontSize enum size of fonttype
	 */
	eUnicodeFontSize getFontSize (eUnicodeFontType fonttype) const;

	std::string shortenStringToSize (const std::string& str, int size,
									 eUnicodeFontType fonttype);

	void setTargetSurface (SDL_Surface* surface) { this->surface = surface; }
	SDL_Surface* getTargetSurface () { return surface; }
	
	/**
	 * encodes a UTF-8 character to its unicode position
	 * @author alzi alias DoctorDeath
	 * @param pch pointer to the character string
	 * @param increase number which will be changed to the value
	 *        how much bytes the character has taken in UTF-8
	 * @return unicode position
	 */
	Uint16 encodeUTF8Char (const char* pch, int& increase) const;

	bool isUtf8Space (const char* pch) const;

	int getUnicodeCharacterWidth (Uint16 unicodeCharacter, eUnicodeFontType fonttype) /*const*/;
private:
	typedef AutoSurface FontTypeSurfaces[0xFFFF];
	// character surfaces.
	// Since SDL maximal gives us the unicodes
	// from BMP we need 0xFFFF surfaces at maximum
	AutoSurface charsNormal[0xFFFF];
	AutoSurface charsNormalRed[0xFFFF];
	AutoSurface charsSmallWhite[0xFFFF];
	AutoSurface charsSmallGreen[0xFFFF];
	AutoSurface charsSmallRed[0xFFFF];
	AutoSurface charsSmallYellow[0xFFFF];
	AutoSurface charsBig[0xFFFF];
	AutoSurface charsBigGold[0xFFFF];

	// target surface where to draw.
	SDL_Surface* surface;

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
	FontTypeSurfaces* getFontTypeSurfaces (eUnicodeFontType fonttype);
	/**
	 * loads the ISO-8859 bitmap font surface
	 * @author alzi alias DoctorDeath
	 * @param charset the charset which bitmap should be loaded.
	 * @param fonttype the fonttype which bitmap should be loaded.
	 * @return the bitmap surface
	 */
	AutoSurface loadCharsetSurface (eUnicodeFontCharset charset, eUnicodeFontType fonttype);
	/**
	 * returns the iso page with the unicode positions of the characters
	 * in a ISO-8859 font
	 * @author alzi alias DoctorDeath
	 * @param charset the charset for that the iso page should be returned.
	 * @return the iso page
	 */
	const unsigned short* getIsoPage (eUnicodeFontCharset charset);
	int drawWithBreakLines (SDL_Rect rDest, const std::string& sText,
							eUnicodeFontType fonttype);
};

EX cUnicodeFont* font;

#endif
