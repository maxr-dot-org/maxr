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
#ifndef fontsH
#define fontsH

#include <string>

#include "defines.h"
#include "SDL.h"
#include "main.h"
#include "pcx.h"

enum eBitmapFontType
{
	LATIN_NORMAL,
	LATIN_BIG,
	LATIN_BIG_GOLD,
	LATIN_SMALL_WHITE,
	LATIN_SMALL_RED,
	LATIN_SMALL_GREEN,
	LATIN_SMALL_YELLOW
};

enum eFontLangCode
{
	//BEGIN LATIN-1 iso-8559-1
	ENG,	//English iso-8559-1
	GER,	//German
	ALB,	//Alban
	DAN,	//Danish
	FAO,	//Faroese
	FIN,	//Finnish
	FRA,	//French
	FRE,	//French
	GLE,	//Irish
	ICE,	//Icelandic
	ISL,	//Icelandic
	ITA,	//Italian
	CAT,	//Catalan; Valencian
	DUT,	//Dutch; Flemish
	NLD,	//Dutch; Flemish
	NNO,	//Norwegian Nynorsk
	NOB,	//Norwegian Bokm�l
	POR,	//Portuguese
	SWE,	//Swedish
	SPA,	//Spanish
	//END LATIN-1 iso-8559-1
	//BEGIN LATIN-2 iso-8559-2
	HRV,	//Croatian
	SCR,	//Croatian
	POL,	//Polish
	RON,	//Romanian
	RUM,	//Romanian
	SLK,	//Slovak
	SLO,	//Slovak
	SLV,	//Slovenian
	CES,	//Czech
	CZE,	//Czech
	HUN,	//Hungarian
	//END LATIN-2 iso-8559-2
	//BEGIN LATIN-3 iso-8559-3
	EPO,	//Esperanto
	GLG,	//Galician
	MLT,	//Maltese
	TUR,	//Turkish
	//END LATIN-3 iso-8559-3
	//BEGIN LATIN-4 iso-8559-4
	EST,	//Estonian
	LAV,	//Latvian
	LIT,	//Lithuanian
	//END LATIN-4 iso-8559-4
	//BEGIN LATIN-5 iso-8559-5
	BUL,	//Bulgarian
	MAK,	//Macedonian
	MKD,	//Macedonian
	RUS,	//Russian
	SCC,	//Serbian
	SRP,	//Serbian
	UKR	//Ukrainian
	//END LATIN-5 iso-8559-5
};

/**
 * @author beko
 * Takes care of displaying correct ascii sign from different charactersets stored as image in fonts.<br>
 * This function is inspired by lazyfoo.net SDL tutorial "Bitmap Fonts" - Thank you, lazyfoo! (for your great howtos ;-)
*/
class cBitmapFont{
	public:
		~cBitmapFont();
		cBitmapFont();
		/**
		 * Wrapper for showText for easy use of SDL_Rects
		 * @param rdest destination to start drawing
		 * @param sText text to draw
		 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
		 * @param surface SDL_Surface to draw on. Default is buffer
		 */
		void showText(SDL_Rect rdest, std::string sText, int eBitmapFontType=LATIN_NORMAL, SDL_Surface *surface=buffer);
		/**
		 * Displays a text
		 * @param x position x to start drawing
		 * @param y position y to start drawing
		 * @param sText text to draw
		 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
		 * @param surface SDL_Surface to draw on. Default is buffer
		 */
		void showText(int x, int y, std::string sText, int eBitmapFontType=LATIN_NORMAL, SDL_Surface *surface=buffer);

 		/**
		 * Calculates the needed width for a text in pixels
		 * @param sText text to check
		 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
		 * @return needed width for text
		 */
		int getTextWide(std::string sText, int eBitmapFontType=LATIN_NORMAL);

		/**
		 * Displays a text as block.<br><br> This does <b>not</b> allow blanks in line. Linebreaks are interpreted. Unneeded blanks will be snipped.<br><br>
		 * Example: "Headline\n\n This is my text for a textblock that get's linebreaked automagically"!
		 * @param rDest SDL_Rect for position and wide of textbox. Height is not taken care of!
		 * @param sText text to draw
		 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
		 * @param surface SDL_Surface to draw on. Default is buffer
		 */
		void showTextAsBlock ( SDL_Rect rDest, std::string sText, int eBitmapFontType=LATIN_NORMAL, SDL_Surface *surface=buffer );
		
		/**
		 * Calculates the needed space for a text in pixels
		 * @param sText text to check
		 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
		 * @return SDL_Rect with needed width and height for text
		 */
		SDL_Rect getTextSize(std::string sText, int eBitmapFontType=LATIN_NORMAL);
		/**
		 * Holds information of font height
		 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
		 * @return Height of fonttype in pixels
		 */
		int getFontHeight(int eBitmapFontType=LATIN_NORMAL);
		
		
		/**
		 * Holds languagecode eFontLangCode
		 * @return eFontLangCode 
		 */
		int getLang(void);

		/**
		 * Displays a text centered on given X 
		 * @param rDest DL_Rect for position.<br>Use X for position to center on.<br>Y is not taken care of!
		 * @param sText text to draw
		 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
		 * @param surface SDL_Surface to draw on. Default is buffer
		 */
		void showTextCentered(SDL_Rect rDest, std::string sText, int eBitmapFontType=LATIN_NORMAL, SDL_Surface *surface=buffer);
		
		/**
		 * Displays a text centered on given X 
		 * @param x Use X for position to center on.<br>Y is not taken care of!
		 * @param y position y to start drawing
		 * @param sText text to draw
		 * @param eBitmapFontType enum of fonttype. LATIN_NORMAL is default
		 * @param surface SDL_Surface to draw on. Default is buffer
		 */
		void showTextCentered(int x, int y, std::string sText, int eBitmapFontType=LATIN_NORMAL, SDL_Surface *surface=buffer);

	private:
		//Surfaces to store our latin charsets
		//tmp for actual used surface during creation or drawing
		int iLangCode;
		SDL_Surface *sfTmp;
		SDL_Surface *sfLatinNormal;
		SDL_Surface *sfLatinBig;
		SDL_Surface *sfLatinBigGold;
		SDL_Surface *sfLatinSmallWhite;
		SDL_Surface *sfLatinSmallRed;
		SDL_Surface *sfLatinSmallGreen;
		SDL_Surface *sfLatinSmallYellow;
		//Rects to store carset coordinates for latin charsets
		//chars for actual used surface during creation or drawing
		SDL_Rect chars[256];
		SDL_Rect LatinNormal[256];
		SDL_Rect LatinBig[256];
		SDL_Rect LatinBigGold[256];
		SDL_Rect LatinSmall[256];
		int iLoadedCharset;
		/**
		 * IMPORTANT: Only tested with images having a colourdepth of 8!
		 * @param surface the SDL_Surface providing fonts seperated in 16x16 rows/cells on a 256x256px image
		 */
		void buildFont(SDL_Surface *surface);
		int drawWithBreakLines( SDL_Rect rDest, std::string sText, int eBitmapFontType, SDL_Surface *surface );
		int setLang(void);
		int getIsoTable(int eFontLangCode);
		void copyArray(SDL_Rect source[],SDL_Rect dest[]);
		Uint32 getPixel32(int x, int y, SDL_Surface *surface);
		Uint16 getPixel16(int x, int y, SDL_Surface *surface);
		Uint8 getPixel8(int x, int y, SDL_Surface *surface);
		void getCharset(int eBitmapFontType=LATIN_NORMAL);

};

EX cBitmapFont *font;


#endif
