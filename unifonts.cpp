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
#include "unifonts.h"
#include "pcx.h"
#include "files.h"

cUnicodeFont::cUnicodeFont()
{
	fill <SDL_Surface**, SDL_Surface *>( charsNormal, &charsNormal[0xFFFF], NULL );
	fill <SDL_Surface**, SDL_Surface *>( charsSmall, &charsSmall[0xFFFF], NULL );
	fill <SDL_Surface**, SDL_Surface *>( charsBig, &charsBig[0xFFFF], NULL );
	fill <SDL_Surface**, SDL_Surface *>( charsBigGold, &charsBigGold[0xFFFF], NULL );

	loadChars ( CHARSET_ISO8559_ALL, FONT_LATIN_NORMAL );
	loadChars ( CHARSET_ISO8559_1, FONT_LATIN_NORMAL );
	loadChars ( CHARSET_ISO8559_2, FONT_LATIN_NORMAL );
	loadChars ( CHARSET_ISO8559_5, FONT_LATIN_NORMAL );

	loadChars ( CHARSET_ISO8559_ALL, FONT_LATIN_BIG );
	loadChars ( CHARSET_ISO8559_1, FONT_LATIN_BIG );
	loadChars ( CHARSET_ISO8559_2, FONT_LATIN_BIG );
	loadChars ( CHARSET_ISO8559_5, FONT_LATIN_BIG );

	loadChars ( CHARSET_ISO8559_ALL, FONT_LATIN_BIG_GOLD );
	loadChars ( CHARSET_ISO8559_1, FONT_LATIN_BIG_GOLD );
	loadChars ( CHARSET_ISO8559_2, FONT_LATIN_BIG_GOLD );

	loadChars ( CHARSET_ISO8559_ALL, FONT_LATIN_SMALL_WHITE );
	loadChars ( CHARSET_ISO8559_1, FONT_LATIN_SMALL_WHITE );
	loadChars ( CHARSET_ISO8559_2, FONT_LATIN_SMALL_WHITE );
	loadChars ( CHARSET_ISO8559_5, FONT_LATIN_SMALL_WHITE );
}

cUnicodeFont::~cUnicodeFont()
{
	for ( int i = 0; i < 0xFFFF; i++ )
	{
		if ( charsNormal[i] ) SDL_FreeSurface ( charsNormal[i] );
		if ( charsSmall[i] ) SDL_FreeSurface ( charsSmall[i] );
		if ( charsBig[i] ) SDL_FreeSurface ( charsBig[i] );
		if ( charsBigGold[i] ) SDL_FreeSurface ( charsBigGold[i] );
	}
}

void cUnicodeFont::loadChars( eUnicodeFontCharset charset, eUnicodeFontType fonttype )
{
	SDL_Surface *surface;
	SDL_Surface **chars;
	const unsigned short *iso8859_to_uni;
	int currentChar, highcount;
	int cellW, cellH;
	int pX, pY;

	surface = loadCharsetSurface ( charset, fonttype );
	if ( !surface )
	{
		// LOG: error while loading font
		return;
	}
	chars = getFontTypeSurfaces ( fonttype );
	if ( !chars )
	{
		// LOG: error while loading font
		return;
	}
	iso8859_to_uni = getIsoPage ( charset );

	if ( charset == CHARSET_ISO8559_ALL ) highcount = 16;
	else highcount = 6;

	cellW = surface->w / 16;
	cellH = surface->h / highcount;
	currentChar = 0;
	pX = 0;
	pY = 0;

	for( int rows = 0; rows < highcount; rows ++)
	{
		//go through the cols
		for( int cols = 0; cols < 16; cols ++)
		{
			SDL_Rect Rect;
			Rect.x = cellW * cols; //write each cell position and size into array
			Rect.y = cellH * rows;
			Rect.h = cellH;
			Rect.w = cellW;

			if ( currentChar == 68 )
				int stop = 0;
			//go through pixels to find offset x
			for( int pCol = 0; pCol < cellH; pCol++)
			{
				for( int pRow = 0; pRow < cellH; pRow++)
				{
					pX = (cellW * cols) + pCol;
					pY = (cellH * rows) + pRow;

					if( getPixel32(pX, pY, surface) != SDL_MapRGB(surface->format, 0xFF, 0, 0xFF) )
					{
						//offset
						Rect.x = pX;
						pCol = cellW; //break loop
						pRow = cellH;
					}
				}
			}
			//go through pixel to find offset w
			for( int pCol_w = cellW - 1; pCol_w >= 0; pCol_w--)
			{
				for ( int pRow_w = 0; pRow_w < cellH; pRow_w++)
				{
					pX = (cellW * cols ) + pCol_w;
					pY = (cellH * rows ) + pRow_w;

					if( getPixel32(pX, pY, surface) != SDL_MapRGB(surface->format, 0xFF, 0, 0xFF) )
					{
						Rect.w = (pX - Rect.x) +1;
						pCol_w = -1; //break loop
						pRow_w = cellH;
					}

				}
			}

			int charnum = 0;
			if ( iso8859_to_uni == NULL )
			{
				if ( charset == CHARSET_ISO8559_ALL ) charnum = currentChar;
				else if ( charset == CHARSET_ISO8559_1 ) charnum = currentChar + 128 + 2*16;
			}
			else charnum = iso8859_to_uni[currentChar];
			if ( chars[charnum] ) SDL_FreeSurface ( chars[charnum] );
			chars[charnum] = SDL_CreateRGBSurface ( SDL_HWSURFACE|SDL_SRCCOLORKEY,Rect.w,Rect.h,32,0,0,0,0 );
			SDL_FillRect ( chars[charnum], NULL, 0xFF00FF );
			SDL_SetColorKey ( chars[charnum], SDL_SRCCOLORKEY, 0xFF00FF );
			SDL_BlitSurface ( surface, &Rect, chars[charnum], NULL );
			//goto next character
			currentChar++;
		}
	}
	SDL_FreeSurface ( surface );
}

Uint32 cUnicodeFont::getPixel32(int x, int y, SDL_Surface *surface)
{
	//converts the Pixel to 32 bit
	Uint32 *pixels = (Uint32 *) surface->pixels;

	//get the requested pixels
	return pixels[(y*surface->w)+x];
}

SDL_Surface **cUnicodeFont::getFontTypeSurfaces ( eUnicodeFontType fonttype )
{
	switch ( fonttype )
	{
	case FONT_LATIN_NORMAL:
		return charsNormal;
		break;
	case FONT_LATIN_BIG:
		return charsBig;
		break;
	case FONT_LATIN_BIG_GOLD:
		return charsBigGold;
		break;
	case FONT_LATIN_SMALL_WHITE:
	case FONT_LATIN_SMALL_RED:
	case FONT_LATIN_SMALL_GREEN:
	case FONT_LATIN_SMALL_YELLOW:
		return charsSmall;
		break;
	}
	return NULL;
}

SDL_Surface *cUnicodeFont::loadCharsetSurface( eUnicodeFontCharset charset, eUnicodeFontType fonttype )
{
	string filename = SettingsData.sFontPath + PATH_DELIMITER + "latin_";
	switch ( fonttype )
	{
	case FONT_LATIN_NORMAL:
		filename += "normal";
		break;
	case FONT_LATIN_BIG:
		filename += "big";
		break;
	case FONT_LATIN_BIG_GOLD:
		filename += "big_gold";
		break;
	case FONT_LATIN_SMALL_WHITE:
	case FONT_LATIN_SMALL_RED:
	case FONT_LATIN_SMALL_GREEN:
	case FONT_LATIN_SMALL_YELLOW:
		filename += "small";
		break;
	}
	if ( charset != CHARSET_ISO8559_ALL )
	{
		filename += "_iso-8559-";
		filename += iToStr ( charset );
	}
	filename += ".pcx";

	if ( FileExists( filename.c_str() ) ) return LoadPCX ( filename.c_str() );
	else return NULL;
}

const unsigned short *cUnicodeFont::getIsoPage ( eUnicodeFontCharset charset )
{
	switch ( charset )
	{
	case CHARSET_ISO8559_ALL:
		return NULL;
	case CHARSET_ISO8559_1:
		return NULL;
	case CHARSET_ISO8559_2:
		return iso8859_2_2uni;
	case CHARSET_ISO8559_3:
		return iso8859_3_2uni;
	case CHARSET_ISO8559_4:
		return iso8859_4_2uni;
	case CHARSET_ISO8559_5:
		return iso8859_5_2uni;
	case CHARSET_ISO8559_6:
		return iso8859_6_2uni;
	case CHARSET_ISO8559_7:
		return iso8859_7_2uni;
	case CHARSET_ISO8559_8:
		return iso8859_8_2uni;
	case CHARSET_ISO8559_9:
		return iso8859_9_2uni;
	case CHARSET_ISO8559_10:
		return iso8859_10_2uni;
	case CHARSET_ISO8559_11:
		return NULL;
	case CHARSET_ISO8559_13:
		return iso8859_13_2uni;
	case CHARSET_ISO8559_14:
		return iso8859_14_2uni;
	case CHARSET_ISO8559_15:
		return iso8859_15_2uni;
	case CHARSET_ISO8559_16:
		return iso8859_16_2uni;
	default:
		//LOG: unknown iso format
		break;
	}
	return NULL;
}

void cUnicodeFont::showText( SDL_Rect rDest, string sText, eUnicodeFontType fonttype, SDL_Surface *surface, bool encode )
{
	showText ( rDest.x, rDest.y, sText, fonttype, surface, encode );
}

void cUnicodeFont::showText( int x, int y, string sText, eUnicodeFontType fonttype, SDL_Surface *surface, bool encode )
{
	int offX = x;
	int offY = y;
	int iSpace = 0;
	SDL_Surface **chars = getFontTypeSurfaces ( fonttype );

	//make sure only upper characters are read for the small fonts
	// since we don't support lower chars on the small fonts
	switch( fonttype )
	{
		case FONT_LATIN_SMALL_GREEN:
		case FONT_LATIN_SMALL_RED:
		case FONT_LATIN_SMALL_WHITE:
		case FONT_LATIN_SMALL_YELLOW:
			for( int i=0; i < (int)sText.size(); i++ ) sText[i] = toupper(sText[i]);
			iSpace = 1;
			break;
	}

	// decode the UTF-8 String:
	char *p = (char*)sText.c_str();
	char *now = &p[sText.length()];
	while ( p < now )
	{
		//is space?
		if( *p == ' ' )
		{
			if ( chars[97] ) offX += chars[97]->w;
			p++;
		} //is new line?
		else if( *p == '\n' )
		{
			offY += getFontHeight ( fonttype );
			offX = x;
			p++;
		}
		else if( 13 == *(unsigned char *)p )
		{
			p++;
			//ignore - is breakline in file
		}
		else
		{
			Uint16 uni;
			if ( encode )
			{
				int increase;
				uni = encodeUTF8Char ( (unsigned char *)p, &increase );
				p += increase;
			}
			else
			{
				uni = *(unsigned char *)p;
				p++;
			}
			if ( chars[uni] != NULL )
			{
				SDL_Rect rTmp = {offX, offY, 16, 16};
				SDL_BlitSurface( chars[uni], NULL, surface, &rTmp);

				//move one px forward for space between signs
				offX += chars[uni]->w + iSpace;
			}
		}
	}
}

int cUnicodeFont::drawWithBreakLines( SDL_Rect rDest, string sText, eUnicodeFontType fonttype, SDL_Surface *surface, bool encode )
{
	int k;
	int lastK = 0;

	SDL_Rect rLenght = {0,0,0,0};
	string sTmp = sText;
	string sTextShortened;

	rLenght = getTextSize ( sText, fonttype, encode );

	if ( rLenght.w > rDest.w ) //text is longer than dest-width - let's snip it
	{
		do
		{
			k = ( int ) sTmp.find ( " " ); //search spaces/blanks

			if ( k == string::npos ) //reached the end but leftovers might be to long
			{
				rLenght = getTextSize ( sText, fonttype, encode ); //test new string lenght
				if ( rLenght.w > rDest.w ) //if leftover is to long snip it too
				{
					sTextShortened = sText; //get total leftover again

					if ( lastK > 0 )
					{
						sTextShortened.erase ( lastK, sTextShortened.size() ); //erase everything longer than line
						sText.erase ( 0, lastK + 1 ); //erase txt from original that we just copied to tmp including leading blank
					}
					else
					{
						sTextShortened.erase ( sText.size() / 2 - 1 , sTextShortened.size() ); //erase everything longer than line
						sTextShortened += "-";
						sText.erase ( 0, sText.size() / 2 - 1 ); //erase txt from original that we just copied to tmp
					}

					showText ( rDest, sTextShortened, fonttype, surface, encode ); //blit part of text
					rDest.y += getFontHeight ( fonttype ); //and add a linebreak
				}

				showText ( rDest, sText, fonttype, surface, encode ); //draw last part of
				rDest.y += getFontHeight ( fonttype ); //and add a linebreak text
			}

			if ( k != string::npos )
			{
				sTmp.erase ( k, 1 ); //replace spaces with # so we don't find them again next search
				sTmp.insert ( k, "#" );
				sTextShortened = sText; //copy clean text to tmp string
				sTextShortened.erase ( k, sTextShortened.size() ); //erase everything longer than line

				rLenght = getTextSize ( sTextShortened, fonttype, encode ); //test new string lenght
				if ( rLenght.w > rDest.w )
				{
					//found important lastK to snip text since text is now to long
					sTextShortened = sText; //copy text to tmp string

					if ( lastK > 0 )
					{
						sTextShortened.erase ( lastK, sTextShortened.size() ); //erase everything longer than line
						sText.erase ( 0, lastK + 1 ); //erase txt from original that we just copied to tmp including leading blank
					}
					else
					{
						sTextShortened.erase ( sText.size() / 2 - 1, sTextShortened.size() ); //erase everything longer than line
						sTextShortened += "-";
						sText.erase ( 0, sText.size() / 2 - 1 ); //erase txt from original that we just copied to tmp

					}

					sTmp = sText; //copy snipped original sText to sTmp to start searching again
					showText ( rDest, sTextShortened, fonttype, surface, encode ); //blit part of text
					rDest.y += getFontHeight ( fonttype ); //and add a linebreak
				}
				else
				{
					lastK = k; //seek more, couldn't find korrekt lastK
				}
			}
		}
		while ( k != string::npos );
	}
	else
	{
		showText ( rDest, sText, fonttype, surface, encode ); //nothing to shorten, just blit text
		rDest.y += getFontHeight ( fonttype ); //and add a linebreak
	}
	return rDest.y;
}

int cUnicodeFont::showTextAsBlock ( SDL_Rect rDest, string sText, eUnicodeFontType fonttype, SDL_Surface *surface, bool encode )
{
	string sTmp;

	int k;

	do
	{
		//erase all invalid formatted breaklines like we may get them from translation XMLs
		k = ( int ) sText.find ( "\\n" );

		if ( k != string::npos )
		{
			sText.erase ( k, 2 );
			sText.insert (k, "\n");
		}
	}
	while ( k != string::npos );

	do
	{
		//erase all blanks > 2
		k = ( int ) sText.find ( "  " ); //IMPORTANT: _two_ blanks! don't change this or this will become an endless loop

		if ( k != string::npos )
		{
			sText.erase ( k, 1 );
		}
	}
	while ( k != string::npos );

	do //support of linebreaks: snip text at linebreaks, do the auto linebreak for first part and proceed with second part
	{
		//search and replace \n since we want a blocktext - no manual breaklines allowed
		k = ( int ) sText.find ( "\n" );

		if ( k != string::npos )
		{

			sTmp=sText;

			sText.erase ( 0, k+1 ); //delete everything before and including linebreak \n
			sTmp.erase (k, sTmp.size()); //delete everything after \n

			rDest.y = drawWithBreakLines(rDest, sTmp, fonttype, surface, encode); //draw first part of text and proceed searching for breaklines
			// += font->getFontHeight(eBitmapFontType); //add newline for each breakline
		}
	}
	while ( k != string::npos );

	return drawWithBreakLines(rDest, sText, fonttype, surface, encode); //draw rest of text
}

void cUnicodeFont::showTextCentered( SDL_Rect rDest, string sText, eUnicodeFontType fonttype, SDL_Surface *surface, bool encode )
{
	showTextCentered ( rDest.x, rDest.y, sText, fonttype, surface, encode );
}

void cUnicodeFont::showTextCentered( int x, int y, string sText, eUnicodeFontType fonttype, SDL_Surface *surface, bool encode )
{
	SDL_Rect rTmp = getTextSize ( sText, fonttype, encode );
	showText ( x - rTmp.w / 2, y, sText, fonttype, surface, encode );
}

int cUnicodeFont::getTextWide( string sText, eUnicodeFontType fonttype, bool encode )
{
	SDL_Rect rTmp = getTextSize( sText, fonttype, encode );
	return rTmp.w;
}

SDL_Rect cUnicodeFont::getTextSize( string sText, eUnicodeFontType fonttype, bool encode )
{
	int iSpace = 0;
	SDL_Surface **chars = getFontTypeSurfaces ( fonttype );
	SDL_Rect rTmp = {0, 0, 0, 0};

	//make sure only upper characters are read for the small fonts
	// since we don't support lower chars on the small fonts
	switch( fonttype )
	{
		case FONT_LATIN_SMALL_GREEN:
		case FONT_LATIN_SMALL_RED:
		case FONT_LATIN_SMALL_WHITE:
		case FONT_LATIN_SMALL_YELLOW:
			for( int i=0; i < (int)sText.size(); i++ ) sText[i] = toupper(sText[i]);
			iSpace = 1;
			break;
	}

	// decode the UTF-8 String:
	char *p = (char*)sText.c_str();
	char *now = &p[sText.length()];
	while ( p < now )
	{
		//is space?
		if( *p == ' ' )
		{
			if ( chars[97] ) rTmp.w += chars[97]->w;
			p++;
		} //is new line?
		else if( *p == '\n' )
		{
			rTmp.h += getFontHeight ( fonttype );
			p++;
		}
		else if( 13 == *(unsigned char *)p )
		{
			p++;
			//ignore - is breakline in file
		}
		else
		{
			Uint16 uni;
			if ( encode )
			{
				int increase;
				uni = encodeUTF8Char ( (unsigned char *)p, &increase );
				p += increase;
			}
			else
			{
				uni = *(unsigned char *)p;
				p++;
			}
			if ( chars[uni] != NULL )
			{
				rTmp.w += chars[uni]->w + iSpace;
				rTmp.h = chars[uni]->h;
			}
		}
	}
	return rTmp;
}

int cUnicodeFont::getFontHeight( eUnicodeFontType fonttype )
{
	SDL_Surface **chars = getFontTypeSurfaces ( fonttype );
	for ( int i = 0; i < 0xFFFF; i++ )
	{
		if ( chars[i] != NULL ) return chars[i]->h;
	}
	return 0;
}

Uint16 cUnicodeFont::encodeUTF8Char ( unsigned char *pch, int *increase )
{
	Uint16 uni = 0;
	unsigned char ch = *pch;
	if ( (ch & 0xE0) == 0xE0 )
	{
		uni |= (ch & 0x0F) << 12;
		uni |= (*(unsigned char*)(pch+1) & 0x3F) << 6;
		uni |= (*(unsigned char*)(pch+2) & 0x3F);
		*increase = 3;
	}
	else if ( (ch & 0xC0) == 0xC0 )
	{
		uni |= (ch & 0x1F) << 6;
		uni |= (*(unsigned char*)(pch+1) & 0x3F);
		*increase = 2;
	}
	else
	{
		uni |= (ch & 0x7F);
		*increase = 1;
	}
	return uni;
}
