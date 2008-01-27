/***************************************************************************
 *              Resinstaller - installs missing GFX for MAXR               *
 *              This file is part of the resinstaller project              *
 *   Copyright (C) 2007, 2008 Eiko Oltmanns                                *
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

#include <math.h>
#include <SDL.h>
#include <string>
#include "converter.h"
#include "pcx.h"
#include "SDL_flic.h"
#include <sstream>

string iToStr(int x)
{
 	stringstream strStream;
 	strStream << x;
 	return strStream.str();
}

cImage::cImage()
{
	palette = NULL;
	iImageCount = 0;

}

cImage::~cImage()
{
	if ( palette ) 
	{
		free ( palette );
	}
	
	if ( bDecoded )
	{
		for ( int i = 0; i < iImageCount; i++ )
		{
			SDL_FreeSurface( Images[i].surface );

			free ( Images[i].data );
			free ( Images[i].alpha );
		}	
		free ( Images );
	}
}

void cImage::saveFile()
{
	if( !bDecoded ) 
		return;
	for (int iNum = 0; iNum < iImageCount; iNum++ )
	{
		string sOutputname = sOutputPath;
		sOutputname += name;
		if( iImageCount > 1 )
		{
			sOutputname += "_";
			char szTmp[13];
			sprintf( szTmp, "%0.3d", iNum);
			sOutputname += szTmp;
		}
		sOutputname += ".pcx";
		
		savePCX( Images[iNum].surface, sOutputname );
		
		cout << name;
		if( iImageCount > 1 )
		{
			cout << "_" << iNum;
		}
		cout << "\n";
	}
}


void cImage::resampleFile()
{
	if( !bDecoded ) return;
	int iMaxHotX = 0, iMaxHotY = 0, iMaxRight = 0, iMaxBottom = 0;
	cImageData ImageData;
	for( int j = 0; j < iImageCount; j++ )
	{
		if ( iMaxHotX < Images[j].sUHotX )
		{
			iMaxHotX = Images[j].sUHotX;
		}
		if ( iMaxHotY < Images[j].sUHotY )
		{
			iMaxHotY = Images[j].sUHotY;
		}
		if ( iMaxRight < Images[j].sWidth - Images[j].sUHotX )
		{
			iMaxRight = Images[j].sWidth - Images[j].sUHotX;
		}
		if ( iMaxBottom < Images[j].sHeight - Images[j].sUHotY )
		{
			iMaxBottom = Images[j].sHeight - Images[j].sUHotY;
		}
	}
	sWidth = iMaxHotX + iMaxRight;
	sHeight = iMaxHotY + iMaxBottom;
	sHotX = iMaxHotX;
	sHotY = iMaxHotY;
	
	//search a free place in color table for background color
	int backgroundIndex;
	for ( backgroundIndex = 64; backgroundIndex < 256; backgroundIndex++)
	{
		if ( (palette[backgroundIndex].Blue == 215) && (palette[backgroundIndex].Green == 7) && (palette[backgroundIndex].Red == 255) )
			break;
	}

	if ( backgroundIndex > 255 )
	{
		backgroundIndex = 0;
	}
	else
	{
		palette[backgroundIndex].Blue = 255;
		palette[backgroundIndex].Green = 0;
		palette[backgroundIndex].Red = 255;
	}

	cout << "BackgroundIndex is " << iToStr(backgroundIndex) << "\n";

	cout << "Palette in cImage is:\n";
	for (int i = 0; i < 256; i++)
	{
		cout << iToStr( palette[i].Blue) << " ";
		cout << iToStr( palette[i].Green) << " ";
		cout << iToStr( palette[i].Red) << " ";
	}
	cout << "\n\n";

	for(int iF = 0; iF < iImageCount; iF++ )
	{
		//create surface
		SDL_Surface* surface = SDL_CreateRGBSurface(SDL_SWSURFACE, sWidth, sHeight,8,0,0,0,0);
		surface->pitch = surface->w;	//this seems to be an SDL-Bug...
										//sometimes the pitch of a surface has an wrong value
		

		//copy color table
		surface->format->palette->ncolors = 256;
		for (int i = 0; i < 256; i++ )
		{	
			//attention: for some reason r and b must be swapped!
			surface->format->palette->colors[i].r = palette[i].Blue;
			surface->format->palette->colors[i].g = palette[i].Green;
			surface->format->palette->colors[i].b = palette[i].Red;
		}
		if ( backgroundIndex > 0 )
		{
			//SDL_SetColorKey( surface, SDL_SRCCOLORKEY, SDL_MapRGB(surface->format, 255, 0, 255));
		}
		
		for (int iX = 0; iX < sWidth; iX++ )
		{
			for (int iY = 0; iY < sHeight; iY++ )
			{
				Uint8 *pixel = (Uint8*) surface->pixels  + (iY * sWidth + iX);

				*pixel = backgroundIndex;
				
				ImageData = Images[iF];

				if (iX < iMaxHotX - ImageData.sUHotX )
				{
					continue; // Image is right than point
				}
				if (iY < iMaxHotY - ImageData.sUHotY )
				{
					continue; // Image is bottom than point
				}
				if (iX >= ImageData.sWidth - ImageData.sUHotX + iMaxHotX )
				{
					continue; // Image is left than point
				}
				if (iY >= ImageData.sHeight - ImageData.sUHotY + iMaxHotY )
				{
					continue; // Image is top than point
				}
				if (ImageData.alpha[iX - iMaxHotX + ImageData.sUHotX + (iY - iMaxHotY + ImageData.sUHotY) * ImageData.sWidth] == 0 )
				{
					
					*pixel = ImageData.data[iX - iMaxHotX + ImageData.sUHotX + (iY - iMaxHotY + ImageData.sUHotY) * ImageData.sWidth];
				}
			}
		}
		Images[iF].surface = surface;
	}
}

void cImage::decodeFile()
{
	if( decodeSimpleImage() )
	{
		cout << "Image is SimpleImage\n";
		bDecoded = true;
	}
	else if( decodeMultiShadow() )
	{
		cout << "Image is MultiShadow\n";
		bDecoded = true;
	}
	else if( decodeMultiImage() )
	{
		cout << "Image is MultiImage\n";
		bDecoded = true;
	}
	else if( decodeBigImage() )
	{
		cout << "Image is BigImage\n";
		bDecoded = true;
	}
	else
	{
		cout << "Error, Image not decoded\n";
		bDecoded = false;
	}
}

bool cImage::decodeSimpleImage()
{
	Sint16 sLocWidth, sLocHeight, sLocHotX, sLocHotY;
	SDL_RWseek( res, lPos, SEEK_SET );
	sLocWidth = (Sint16) SDL_ReadLE16 ( res );
	sLocHeight = (Sint16) SDL_ReadLE16 ( res );
	sLocHotX = (Sint16) SDL_ReadLE16 ( res );
	sLocHotY = (Sint16) SDL_ReadLE16 ( res );

	/*SDL_RWread ( res, &sLocWidth, sizeof( short ), 1 );
	SDL_RWread ( &sLocHeight, sizeof( short ), 1, res );
	SDL_RWread ( &sLocHotX, sizeof( short ), 1, res );
	SDL_RWread ( &sLocHotY, sizeof( short ), 1, res );
	*/

	if (sLocWidth > 640 || sLocHeight > 480 )
	{
		return false;
	}
	if (sLocWidth < 1 || sLocHeight < 1 || abs( sLocHotX ) > 640 || abs( sLocHotY ) > 480 )
	{
		return false;
	}
	if ( lLenght != sLocWidth * sLocHeight + 8 )
	{
		return false;
	}
	iImageCount = 1;
	Images = ( cImageData *) malloc( sizeof ( cImageData )  * iImageCount );
	Images[0].sWidth = sLocWidth;
	Images[0].sHeight = sLocHeight;
	Images[0].sHotX = sLocHotX;
	Images[0].sHotY = sLocHotY;
	Images[0].sUHotX = 0;
	Images[0].sUHotY = 0;

	Images[0].data = ( unsigned char * ) malloc( sizeof( unsigned char ) * sLocWidth * sLocHeight );
	SDL_RWread( res, Images[0].data, sizeof( unsigned char ), sLocWidth * sLocHeight );
	Images[0].alpha = ( unsigned char * ) malloc( sizeof( unsigned char ) * sLocWidth * sLocHeight );
	memset( Images[0].alpha, 0, sLocWidth * sLocHeight );

	iImageCount = 1;
	return true;
}

bool cImage::decodeMultiShadow()
{
	Sint32 iX, iY, iBlockIndex, iPicIndex;
	Sint32 lBegin, lEnd, *lBounds, *lRows;
	Sint16 sCount, sLocWidth, sLocHeight, sLocHotX, sLocHotY;
    unsigned char Opacity, Color;
	if( lLenght < 2 )
	{
		return false;
	}
	SDL_RWseek( res, lPos, SEEK_SET );
	sCount = (Sint16) SDL_ReadLE16( res );
	if( sCount < 1 )
	{
		return false;
	}
	if( lLenght < sCount * 12 + 2 )
	{
		return false;
	}
	lBounds = ( Sint32 * ) malloc ( sizeof( Sint32 ) * sCount );
	for ( iPicIndex = 0 ; iPicIndex < sCount ; iPicIndex++ )
	{
		SDL_RWseek( res, lPos + 2 + iPicIndex * 4, SEEK_SET );
		lBounds[iPicIndex] = (Sint32) SDL_ReadLE32 ( res );
		if ( lBounds[iPicIndex] > lLenght )
		{
			free( lBounds );
			return false;
		}
	}
	Images = ( cImageData * ) malloc ( sizeof( cImageData ) * sCount );

	for ( iPicIndex = 0 ; iPicIndex < sCount ; iPicIndex++ )
	{
		SDL_RWseek( res, lPos + 2 + iPicIndex * 4, SEEK_SET );
		lBegin = (Sint32) SDL_ReadLE32( res );
		lEnd = lLenght;
		for( iX = 0 ; iX < sCount ; iX++ )
		{
			if( lEnd > lBounds[iX] && lBegin < lBounds[iX] )
			{
				lEnd = lBounds[iX];
			}
		}
		lBegin += lPos;
		lEnd += lPos;

		SDL_RWseek( res, lBegin, SEEK_SET );
		sLocWidth = (Sint16) SDL_ReadLE16( res );
		sLocHeight = (Sint16) SDL_ReadLE16( res );
		sLocHotX = (Sint16) SDL_ReadLE16( res );
		sLocHotY = (Sint16) SDL_ReadLE16( res );

		if ( sLocWidth < 1 || sLocWidth > 640 || sLocHeight < 1 || sLocHeight > 480 || abs( sLocHotX ) > 640 || abs( sLocHotY ) > 480 ) 
		{
			free ( lBounds );
			free ( Images );
			return false;
		}

		Images[iPicIndex].sWidth = sLocWidth;
		Images[iPicIndex].sHeight = sLocHeight;
		Images[iPicIndex].sHotX = sLocHotX;
		Images[iPicIndex].sHotY = sLocHotY;
		Images[iPicIndex].sUHotX = sLocHotX;
		Images[iPicIndex].sUHotY = sLocHotY;

		Images[iPicIndex].data = ( unsigned char * ) malloc ( sizeof( unsigned char ) * sLocWidth * sLocHeight );
		memset( Images[iPicIndex].data, 0, sLocWidth * sLocHeight );
		Images[iPicIndex].alpha = ( unsigned char * ) malloc ( sizeof( unsigned char ) * sLocWidth * sLocHeight );
		memset( Images[iPicIndex].alpha, 255, sLocWidth * sLocHeight );
		lRows  = ( Sint32 * ) malloc ( sizeof( Sint32 ) * sLocHeight );

		if( lBegin + 8 + sLocHeight * 4 > lEnd )
		{
			//free memory here
			return false;
		}
		SDL_RWseek( res, lBegin + 8, SEEK_SET );
		for ( int i = 0; i < sLocHeight; i++ )
			lRows[i] = (Sint32) SDL_ReadLE32 ( res );

		//SDL_WRread ( res, lRows, sizeof( Sint32 ), sLocHeight, );
		for (iY = 0; iY < sLocHeight; iY++ )
		{
			Color = 0;
			iX = 0;
			Opacity = 255;
			iBlockIndex = 0;
			unsigned char CurData;

			SDL_RWseek( res, lRows[iY] + iBlockIndex + lPos, SEEK_SET );
			SDL_RWread ( res, &CurData, sizeof( char ), 1 );
			while ( lRows[iY] + iBlockIndex < lEnd - lPos && CurData != 255 )
			{
				if( iY * sLocWidth + iX + CurData > sLocWidth * sLocHeight )
				{
					//free memory here
					return false;
				}
				memset( &Images[iPicIndex].data[iY * sLocWidth + iX], Color, CurData );
				memset( &Images[iPicIndex].alpha[iY * sLocWidth + iX], Opacity, CurData );
				Color = 1 - Color;
				Opacity = 255 - Opacity;
				iX += CurData;
				iBlockIndex++;

				SDL_RWseek( res, lRows[iY] + iBlockIndex + lPos, SEEK_SET );
				SDL_RWread ( res, &CurData, sizeof( char ), 1 );
			}
		}
	}
	memset( palette, 255, 3 );
	memset( &palette[1], 0, 3 );
	iImageCount = sCount;

	free ( lBounds );
	free ( lRows );
	return true;
}

bool cImage::decodeMultiImage()
{
	Sint32 iX, iY, iBlockIndex, iPicIndex;
	Sint32 lBegin, lEnd, *lBounds, *lRows;
	Sint16 sCount, sLocWidth, sLocHeight, sLocHotX, sLocHotY;
	bool bCopyNotSkip;
    unsigned char Opacity;
	if( lLenght < 2 )
	{
		return false;
	}
	SDL_RWseek( res, lPos, SEEK_SET );
	sCount = (Sint16) SDL_ReadLE16 ( res );
	if( sCount < 1 )
	{
		return false;
	}
	if( lLenght < sCount * 12 + 2 )
	{
		return false;
	}
	lBounds = ( Sint32 * ) malloc ( sizeof( long ) *sCount );
	for ( iPicIndex = 0 ; iPicIndex < sCount ; iPicIndex++ )
	{
		SDL_RWseek( res, lPos + 2 + iPicIndex * 4, SEEK_SET );
		lBounds[iPicIndex] = (Sint32) SDL_ReadLE32 ( res );
		if ( lBounds[iPicIndex] > lLenght )
		{
			return false;
		}
	}
	Images = ( cImageData * ) malloc ( sizeof( cImageData ) * sCount );

	for ( iPicIndex = 0 ; iPicIndex < sCount ; iPicIndex++ )
	{
		SDL_RWseek( res, lPos + 2 + iPicIndex * 4, SEEK_SET );
		lBegin = (Sint32) SDL_ReadLE32 ( res );
		lEnd = lLenght;
		for( iX = 0 ; iX < sCount ; iX++ )
		{
			if( lEnd > lBounds[iX] && lBegin < lBounds[iX] )
			{
				lEnd = lBounds[iX];
			}
		}
		lBegin += lPos;
		lEnd += lPos;

		SDL_RWseek( res, lBegin, SEEK_SET );
		sLocWidth  = (Sint16) SDL_ReadLE16 ( res );
		sLocHeight = (Sint16) SDL_ReadLE16 ( res );
		sLocHotX   = (Sint16) SDL_ReadLE16 ( res );
		sLocHotY   = (Sint16) SDL_ReadLE16 ( res );
		
		if ( sLocWidth < 1 || sLocWidth > 640 || sLocHeight < 1 || sLocHeight > 480 || abs( sLocHotX ) > 640 || abs( sLocHotY ) > 480 ) 
		{
			return false;
		}

		Images[iPicIndex].sWidth = sLocWidth;
		Images[iPicIndex].sHeight = sLocHeight;
		Images[iPicIndex].sHotX = sLocHotX;
		Images[iPicIndex].sHotY = sLocHotY;
		Images[iPicIndex].sUHotX = sLocHotX;
		Images[iPicIndex].sUHotY = sLocHotY;

		Images[iPicIndex].data = ( unsigned char * ) malloc ( sizeof( unsigned char ) * sLocWidth * sLocHeight );
		memset( Images[iPicIndex].data, 0, sLocWidth * sLocHeight );
		Images[iPicIndex].alpha = ( unsigned char * ) malloc ( sizeof( unsigned char ) * sLocWidth * sLocHeight );
		memset( Images[iPicIndex].alpha, 255, sLocWidth * sLocHeight );
		lRows  = ( Sint32 * ) malloc ( sizeof( Sint32 ) * sLocHeight );

		if( lBegin + 8 + sLocHeight * 4 > lEnd )
		{
			return false;
		}
		SDL_RWseek( res, lBegin + 8, SEEK_SET );
		for ( int i = 0; i < sLocHeight; i++)
			lRows[i] = (Sint32) SDL_ReadLE32( res );

		for (iY = 0; iY < sLocHeight; iY++ )
		{
			bCopyNotSkip = false;
			iX = 0;
			Opacity = 255;
			iBlockIndex = 0;
			unsigned char CurData;

			SDL_RWseek( res, lRows[iY] + iBlockIndex + lPos, SEEK_SET );
			SDL_RWread( res, &CurData, sizeof( char ), 1 );
			while ( lRows[iY] + iBlockIndex < lEnd - lPos && CurData != 255 )
			{
				if( iY * sLocWidth + iX + CurData > sLocWidth * sLocHeight )
				{
					return false;
				}
				memset( &Images[iPicIndex].alpha[iY * sLocWidth + iX], Opacity, CurData );

				if ( bCopyNotSkip )
				{
					SDL_RWseek( res, lRows[iY] + iBlockIndex + lPos + 1, SEEK_SET );
					SDL_RWread( res, &Images[iPicIndex].data[iY * sLocWidth + iX], sizeof( char ), CurData );
					iX += CurData;
					iBlockIndex += CurData + 1;
				}
				else
				{
					memset( &Images[iPicIndex].data[iY * sLocWidth + iX], 0, CurData );
					iX += CurData;
					iBlockIndex++;
				}
				bCopyNotSkip = !bCopyNotSkip;
				Opacity = 255 - Opacity;

				SDL_RWseek( res, lRows[iY] + iBlockIndex + lPos, SEEK_SET );
				SDL_RWread( res, &CurData, sizeof( char ), 1 );
			}
		}
	}
	free ( lBounds );
	free ( lRows );

	iImageCount = sCount;
	return true;
}

bool cImage::decodeBigImage()
{
	Sint16 sLocWidth, sLocHeight, sLocHotX, sLocHotY, sCnt;
	int iOutOfs, iInOfs;
	if( lLenght < 776 )
	{
		return false;
	}
	SDL_RWseek( res, lPos, SEEK_SET );
	sLocHotX   = (Sint16) SDL_ReadLE16( res );
	sLocHotY   = (Sint16) SDL_ReadLE16( res );
	sLocWidth  = (Sint16) SDL_ReadLE16( res );
	sLocHeight = (Sint16) SDL_ReadLE16( res );
	
	if ( sLocWidth > 640 || sLocHeight > 480 || sLocWidth < 1 || sLocHeight < 1 )
	{
		return false;
	}
	iInOfs = 776;
	iOutOfs = 0;
	iImageCount = 1;
	Images = ( cImageData *) malloc( sizeof ( cImageData )  * iImageCount );
	Images[0].sWidth = sLocWidth;
	Images[0].sHeight = sLocHeight;
	Images[0].sHotX = sLocHotX;
	Images[0].sHotY = sLocHotY;
	Images[0].sUHotX = 0;
	Images[0].sUHotY = 0;

	Images[0].data = ( unsigned char * ) malloc( sizeof( unsigned char ) * sLocWidth * sLocHeight );
	Images[0].alpha = ( unsigned char * ) malloc( sizeof( unsigned char ) * sLocWidth * sLocHeight );
	memset( Images[0].alpha, 0, sLocWidth * sLocHeight );

	unsigned char *Buffer = NULL;
	while ( iInOfs + 1 < lLenght && iOutOfs < sLocWidth * sLocHeight )
	{
		SDL_RWseek (res, lPos + iInOfs, SEEK_SET );
		sCnt = (Sint16) SDL_ReadLE16( res );
		iInOfs += 2;

		Buffer = (unsigned char *) malloc ( 1 );

		if( sCnt <= 0 )
		{
			sCnt = -sCnt;
			if ( iInOfs >= lLenght || sCnt + iOutOfs > sLocWidth * sLocHeight )
			{
				return false;
			}

			SDL_RWseek( res, lPos + iInOfs, SEEK_SET );
			SDL_RWread( res, Buffer, sizeof( char ), 1 );
			for(int i = 0; i < sCnt; i++ )
			{
				Images[0].data[iOutOfs+i] = Buffer[0];
			}
			iInOfs++;
			iOutOfs += sCnt;
		}
		else
		{
			if ( sCnt + iInOfs > lLenght ||  sCnt + iOutOfs > sLocWidth * sLocHeight )
			{
				return false;
			}
			
			SDL_RWseek( res, lPos + iInOfs, SEEK_SET );
			SDL_RWread( res, Buffer, sizeof( char ), 1 );
			for(int i = 0; i < sCnt; i++ )
			{
				Images[0].data[iOutOfs+i] = Buffer[0];
			}

			free ( Buffer );
			Buffer = (unsigned char *) malloc ( sCnt );
			SDL_RWseek( res, lPos + iInOfs, SEEK_SET );
			SDL_RWread( res, Buffer, sizeof( char ), sCnt );
			for(int i = 0; i < sCnt; i++ )
			{
				Images[0].data[iOutOfs+i] = Buffer[i];
			}
			
			free ( Buffer );

			iInOfs += sCnt;
			iOutOfs += sCnt;
		}
	}

	if( iInOfs != lLenght || iOutOfs != sLocWidth * sLocHeight) 
	{

		Images[0].data = NULL;
		Images[0].alpha = NULL;
		Images = NULL;
		return false;
	}
	else
	{
		SDL_RWseek( res, lPos + 8, SEEK_SET );
		SDL_RWread( res, palette, sizeof( char ), 768 );
	}
	
	iImageCount = 1;
	return true;
}

SDL_Surface* cImage::getSurface(int imageNr)
{
	if ( imageNr > iImageCount - 1) 
		return NULL;

	Images[imageNr].surface->refcount++;

	return Images[imageNr].surface;
}

//TODO: for every call of getImage, all picutes with the given name (and different imageNr) are decoded too
//this wastes much time
//I should really think about my interface to the res hacker...
SDL_Surface* getImage(string file_name, int imageNr)
{
	Uint32 lPosOfFile = lPosBegin;

	//search desired file in max.res
	while( lPosOfFile < lEndOfFile )
	{
		char name[9];

		SDL_RWseek( res, lPosOfFile, SEEK_SET );

		SDL_RWread ( res, name, sizeof( char ), 8 );
		name[8] = '\0';

		if ( file_name.compare(name) == 0)
			break;

		lPosOfFile += 16;
	}

	if (lPosOfFile >= lEndOfFile)
	{
		return NULL;
	}

	cImage Image;

	//read pos and length of image in max.res
	Image.lPos = (Sint32) SDL_ReadLE32 ( res );
	Image.lLenght = (Sint32) SDL_ReadLE32 ( res );
	
	cout << "Palette loaded from disk is:\n";
	for ( int i = 0; i < 3*256; i++)
		cout << iToStr(orig_palette[i]) << " ";
	cout << "\n\n";

	//copy color table
	Image.palette = (sPixel*) malloc ( sizeof( sPixel ) * 256 );
	for ( int i = 0; i < 256; i++ )
	{
		Image.palette[i].Blue  = orig_palette[3*i];
		Image.palette[i].Green = orig_palette[3*i + 1];
		Image.palette[i].Red   = orig_palette[3*i + 2];
	}

	//extract image
	Image.decodeFile();
	Image.resampleFile();
	
	return Image.getSurface(imageNr);
	
}

//sets the player colors in the color table to white
//note that the information about the player colors are lost, when blitting the surface
void removePlayerColor( SDL_Surface *surface)
{
	for ( int i = 32; i < 39 ; i++ )
	{
		surface->format->palette->colors[i].r = 255;
		surface->format->palette->colors[i].g = 255;
		surface->format->palette->colors[i].b = 255;
	}
}

int saveAllFiles()
{
	Uint32 lPosOfFile = lPosBegin;
	while( lPosOfFile < lEndOfFile )
	{
		cImage Image;

		SDL_RWseek( res, lPosOfFile, SEEK_SET );

		SDL_RWread ( res, &Image.name, sizeof( char ), 8 );
		Image.name[8] = '\0';


		//read pos and length of image in max.res
		Image.lPos = (Sint32) SDL_ReadLE32 ( res );
		Image.lLenght = (Sint32) SDL_ReadLE32 ( res );
		
		//copy color table
		Image.palette = (sPixel*) malloc ( sizeof( sPixel ) * 256 );
		for ( int i = 0; i < 256; i++ )
		{
			Image.palette[i].Blue  = orig_palette[i];
			Image.palette[i].Green = orig_palette[i + 1];
			Image.palette[i].Red   = orig_palette[i + 2];
		}	

		//extract image
		Image.decodeFile();
		Image.resampleFile();
		
		Image.saveFile();

		lPosOfFile += 16;
	}
	return 1;
}

int copyFileFromRes ( string src, string dst, int number )
{
	SDL_Surface *surface;
	surface = getImage(src, number);
	savePCX ( surface, dst );
	SDL_FreeSurface ( surface );

	return 1;
}


//rpc stands for "remove player color"
int copyFileFromRes_rpc(string src, string dst, int number )
{
	SDL_Surface *surface;
	surface = getImage(src, number);
	
	cout << "bpp: " << iToStr(surface->format->BitsPerPixel) << "\n";
	cout << "ncolors: " << iToStr(surface->format->palette->ncolors) << "\n";
	//itoa( *surface->pixels, szTemp, 10);
	//cout << "ncolors: " << szTemp << "\n";
	cout << "Palette after decoding image:\n";
	for ( int i = 0; i < 256; i++)
	{	
		cout << iToStr( surface->format->palette->colors[i].r) << " ";
		cout << iToStr( surface->format->palette->colors[i].g) << " ";
		cout << iToStr( surface->format->palette->colors[i].b) << " ";
		
	}
	cout << "\n";

	/*SDL_Init(SDL_INIT_VIDEO);
	SDL_Surface* screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
	SDL_BlitSurface(surface, NULL, screen, NULL);
	SDL_UpdateRect(screen, 0, 0, 0, 0); */
	
	
	/*surface->format->palette->ncolors = 256;
	for ( int i = 0; i < 256; i++ )
	{
		surface->format->palette->colors[i].r = orig_palette[i*3];
		surface->format->palette->colors[i].g = orig_palette[i*3+1];
		surface->format->palette->colors[i].b = orig_palette[i*3+2];
	}

	surface->format->palette->colors[64].r = 255;
	surface->format->palette->colors[64].g = 0;
	surface->format->palette->colors[64].b = 255;
	*/
	//removePlayerColor( surface );
	savePCX( surface, dst);
	SDL_FreeSurface( surface );

	//while (1)
	{
		//SDL_UpdateRect ( screen,0,0,0,0 );
	};

	return 1;
}
	
int copyImageFromFLC(string fileName, string dst)
{
	SDL_RWops* file = SDL_RWFromFile(fileName.c_str(), "rb");
	if (!file)
		return 0;

	int error;
	FLI_Animation* animation;

	animation = FLI_Open( file, &error);
	if ( error != 0 )
		return 0;

	error = FLI_NextFrame( animation );
	if (  error!=0 )
		return 0;
	
	savePCX(animation->surface, dst);

	return 1;
}

void resizeSurface ( SDL_Surface*& surface, int x, int y, int h, int w )
{
	SDL_Rect dst_rect, src_rect;
	SDL_Surface* resizedSurface;

	if ( surface->format->BitsPerPixel != 8 )
		return;
	
	if ( surface->h > h )
	{
		dst_rect.y = 0;
		src_rect.y = y;
		src_rect.h = h;
	}
	else 
	{
		dst_rect.y = y;
		src_rect.y = 0;
		src_rect.h = surface->h;
	}

	if ( surface->w > w )
	{
		dst_rect.x = 0;
		src_rect.x = x;
		src_rect.w = w;
	}
	else 
	{
		dst_rect.x = x;
		src_rect.x = 0;
		src_rect.w = surface->w;
	}

	resizedSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, h, w, 8,0,0,0,0);
	resizedSurface->pitch = resizedSurface->w;	//this seems to be an SDL-Bug...
												//sometimes the pitch of a surface has an wrong value
	SDL_SetColors(resizedSurface, surface->format->palette->colors, 0, 256);
	SDL_FillRect( resizedSurface, 0, SDL_MapRGB( resizedSurface->format, 255, 0, 255));
	resizedSurface->format->colorkey = surface->format->colorkey;

	SDL_BlitSurface( surface, &src_rect, resizedSurface, &dst_rect);

	SDL_FreeSurface( surface );

	surface = resizedSurface;
}

void setColor( SDL_Surface* surface, unsigned char nr, unsigned char r, unsigned char g, unsigned char b )
{
	surface->format->palette->colors[nr].r = r;
	surface->format->palette->colors[nr].g = g;
	surface->format->palette->colors[nr].b = b;
}

void setAnimationColor( SDL_Surface* surface, unsigned char index, unsigned char frame )
{

	switch ( index )
	{
		case 9:
			switch ( frame )
			{
				case 0:
					setColor( surface, index, 203, 203, 255 );
					break;
				case 1:
					setColor( surface, index, 171, 171, 227);
					break;
				case 2:
					setColor( surface, index, 99, 91, 223);
					break;
				case 3:
					setColor( surface, index, 171, 171, 227);
					break;
			}
			break;
		case 11:
			switch ( frame )
			{
				case 0:
					setColor( surface, index, 99, 91, 223);
					break;
				case 1:
					setColor( surface, index, 171, 171, 227);
					break;
				case 2:
					setColor( surface, index, 203, 203, 255);
					break;
				case 3:
					setColor( surface, index, 171, 171, 227);
					break;
			}
			break;
		case 13:
			switch ( frame )
			{
				case 0:
					setColor( surface, index, 255, 255, 159);
					break;
				case 1:
					setColor( surface, index, 243, 171, 103);
					break;
				case 2:
					setColor( surface, index, 235,  51,  51);
					break;
				case 3:
					setColor( surface, index, 243, 171, 103);
					break;
			}
			break;
		case 14:
			switch ( frame )
			{
				case 0:
					setColor( surface, index, 243, 171, 103);
					break;
				case 1:
					setColor( surface, index, 255, 255, 159);
					break;
				case 2:
					setColor( surface, index, 243, 171, 103);
					break;
				case 3:
					setColor( surface, index, 235,  51,  51);
					break;
			}
			break;
		case 15:
			switch ( frame )
			{
				case 0:
					setColor( surface, index, 235,  51,  51);
					break;
				case 1:
					setColor( surface, index, 243, 171, 103);
					break;
				case 2:
					setColor( surface, index, 255, 255, 159);
					break;
				case 3:
					setColor( surface, index, 243, 171, 103);
					break;
			}
			break;
		case 16:
			switch ( frame )
			{
				case 0:
					setColor( surface, index, 243, 171, 103);
					break;
				case 1:
					setColor( surface, index, 235,  51,  51);
					break;
				case 2:
					setColor( surface, index, 243, 171, 103);
					break;
				case 3:
					setColor( surface, index, 255, 255, 159);
					break;
			}
			break;
		case 17:
			switch ( frame )
			{
				case 0:
					setColor( surface, index, 23, 99, 135);
					break;
				case 1:
					setColor( surface, index, 43, 63, 75);
					break;
				case 2:
					setColor( surface, index, 15, 15, 15);
					break;
				case 3:
					setColor( surface, index, 43, 63, 75);
					break;
			}
			break;
		case 19:
			switch ( frame )
			{
				case 0:
					setColor( surface, index, 15, 15, 15);
					break;
				case 1:
					setColor( surface, index, 43, 63, 75);
					break;
				case 2:
					setColor( surface, index, 23, 99, 135);
					break;
				case 3:
					setColor( surface, index, 43, 63, 75);
					break;
			}
			break;
		case 21:
			switch ( frame )
			{
				case 0:
					setColor( surface, index, 183, 103, 0);
					break;
				case 1:
					setColor( surface, index, 74, 57, 33);
					break;
				case 2:
					setColor( surface, index, 15, 15, 15);
					break;
				case 3:
					setColor( surface, index, 82, 57, 41);
					break;
			}
			break;
		case 22:
			switch ( frame )
			{
				case 0:
					setColor( surface, index, 75, 59, 39);
					break;
				case 1:
					setColor( surface, index, 181, 99, 0);
					break;
				case 2:
					setColor( surface, index, 75, 59, 39);
					break;
				case 3:
					setColor( surface, index, 8, 8, 8);
					break;
			}
			break;
		case 23:
			switch ( frame )
			{
				case 0:
					setColor( surface, index, 15, 15, 15);
					break;
				case 1:
					setColor( surface, index, 74, 57, 33);
					break;
				case 2:
					setColor( surface, index, 181, 99, 0);
					break;
				case 3:
					setColor( surface, index, 74, 57, 33);
					break;
			}
			break;

		case 24:
			switch ( frame )
			{
				case 0:
					setColor( surface, index, 75, 59, 39);
					break;
				case 1:
					setColor( surface, index, 15, 15, 15);
					break;
				case 2:
					setColor( surface, index, 75, 59, 39);
					break;
				case 3:
					setColor( surface, index, 181, 99, 0);
					break;
			}
			break;
	}	
}

//change palette of surface to generate an animation frame
void generateAnimationFrame( SDL_Surface *surface, unsigned char frame)
{
	setAnimationColor( surface,  9, frame);
	setAnimationColor( surface, 11, frame);
	setAnimationColor( surface, 13, frame);
	setAnimationColor( surface, 14, frame);
	setAnimationColor( surface, 15, frame);
	setAnimationColor( surface, 16, frame);
	setAnimationColor( surface, 17, frame);
	setAnimationColor( surface, 19, frame);
	setAnimationColor( surface, 21, frame);
	setAnimationColor( surface, 22, frame);
	setAnimationColor( surface, 23, frame);
	setAnimationColor( surface, 24, frame);
}
