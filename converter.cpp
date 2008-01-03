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
		
		save_PCX( Images[iNum].surface, sOutputname );
		
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
	//don't know how to implement this in a smarter way, yet
	int backgroundIndex = 64;
	for(int iF = 0; iF < iImageCount; iF++ )
	{
		for (int iX = 0; iX < sWidth; iX++ )
		{
			for (int iY = 0; iY < sHeight; iY++ )
			{
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
					if ( ImageData.data[iX - iMaxHotX + ImageData.sUHotX + (iY - iMaxHotY + ImageData.sUHotY) * ImageData.sWidth] == backgroundIndex )
					{
						backgroundIndex++;
						iF = 0;
						iX = 0;
						iY = 0;
					}
				}
			}
		}
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
			SDL_SetColorKey( surface, SDL_SRCCOLORKEY, SDL_MapRGB(surface->format, 255, 0, 255));
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
		bDecoded = true;
	}
	else if( decodeMultiShadow() )
	{
		bDecoded = true;
	}
	else if( decodeMultiImage() )
	{
		bDecoded = true;
	}
	else if( decodeBigImage() )
	{
		bDecoded = true;
	}
	else
	{
		bDecoded = false;
	}
}

bool cImage::decodeSimpleImage()
{
	short sLocWidth, sLocHeight, sLocHotX, sLocHotY;
	fseek( res, lPos, SEEK_SET );
	fread ( &sLocWidth, sizeof( short ), 1, res );
	fread ( &sLocHeight, sizeof( short ), 1, res );
	fread ( &sLocHotX, sizeof( short ), 1, res );
	fread ( &sLocHotY, sizeof( short ), 1, res );

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
	fread( Images[0].data, sizeof( unsigned char ), sLocWidth * sLocHeight, res );
	Images[0].alpha = ( unsigned char * ) malloc( sizeof( unsigned char ) * sLocWidth * sLocHeight );
	memset( Images[0].alpha, 0, sLocWidth * sLocHeight );

	iImageCount = 1;
	return true;
}

bool cImage::decodeMultiShadow()
{
	int iX, iY, iBlockIndex, iPicIndex;
	long lBegin, lEnd, *lBounds, *lRows;
	short sCount, sLocWidth, sLocHeight, sLocHotX, sLocHotY;
    unsigned char Opacity, Color;
	if( lLenght < 2 )
	{
		return false;
	}
	fseek( res, lPos, SEEK_SET );
	fread ( &sCount, sizeof( short ), 1, res );
	if( sCount < 1 )
	{
		return false;
	}
	if( lLenght < sCount * 12 + 2 )
	{
		return false;
	}
	lBounds = ( long * ) malloc ( sizeof( long ) *sCount );
	for ( iPicIndex = 0 ; iPicIndex < sCount ; iPicIndex++ )
	{
		fseek( res, lPos + 2 + iPicIndex * 4, SEEK_SET );
		fread ( &lBounds[iPicIndex], sizeof( long ), 1, res );
		if ( lBounds[iPicIndex] > lLenght )
		{
			return false;
		}
	}
	Images = ( cImageData * ) malloc ( sizeof( cImageData ) * sCount );

	for ( iPicIndex = 0 ; iPicIndex < sCount ; iPicIndex++ )
	{
		fseek( res, lPos + 2 + iPicIndex * 4, SEEK_SET );
		fread ( &lBegin, sizeof( long ), 1, res );
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

		fseek( res, lBegin, SEEK_SET );
		fread ( &sLocWidth, sizeof( short ), 1, res );
		fread ( &sLocHeight, sizeof( short ), 1, res );
		fread ( &sLocHotX, sizeof( short ), 1, res );
		fread ( &sLocHotY, sizeof( short ), 1, res );

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
		lRows  = ( long * ) malloc ( sizeof( long ) * sLocHeight );

		if( lBegin + 8 + sLocHeight * 4 > lEnd )
		{
			return false;
		}
		fseek( res, lBegin + 8, SEEK_SET );
		fread ( lRows, sizeof( long ), sLocHeight, res );
		for (iY = 0; iY < sLocHeight; iY++ )
		{
			Color = 0;
			iX = 0;
			Opacity = 255;
			iBlockIndex = 0;
			unsigned char CurData;

			fseek( res, lRows[iY] + iBlockIndex + lPos, SEEK_SET );
			fread ( &CurData, sizeof( char ), 1, res );
			while ( lRows[iY] + iBlockIndex < lEnd - lPos && CurData != 255 )
			{
				if( iY * sLocWidth + iX + CurData > sLocWidth * sLocHeight )
				{
					return false;
				}
				memset( &Images[iPicIndex].data[iY * sLocWidth + iX], Color, CurData );
				memset( &Images[iPicIndex].alpha[iY * sLocWidth + iX], Opacity, CurData );
				Color = 1 - Color;
				Opacity = 255 - Opacity;
				iX += CurData;
				iBlockIndex++;

				fseek( res, lRows[iY] + iBlockIndex + lPos, SEEK_SET );
				fread ( &CurData, sizeof( char ), 1, res );
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
	int iX, iY, iBlockIndex, iPicIndex;
	long lBegin, lEnd, *lBounds, *lRows;
	short sCount, sLocWidth, sLocHeight, sLocHotX, sLocHotY;
	bool bCopyNotSkip;
    unsigned char Opacity;
	if( lLenght < 2 )
	{
		return false;
	}
	fseek( res, lPos, SEEK_SET );
	fread ( &sCount, sizeof( short ), 1, res );
	if( sCount < 1 )
	{
		return false;
	}
	if( lLenght < sCount * 12 + 2 )
	{
		return false;
	}
	lBounds = ( long * ) malloc ( sizeof( long ) *sCount );
	for ( iPicIndex = 0 ; iPicIndex < sCount ; iPicIndex++ )
	{
		fseek( res, lPos + 2 + iPicIndex * 4, SEEK_SET );
		fread ( &lBounds[iPicIndex], sizeof( long ), 1, res );
		if ( lBounds[iPicIndex] > lLenght )
		{
			return false;
		}
	}
	Images = ( cImageData * ) malloc ( sizeof( cImageData ) * sCount );

	for ( iPicIndex = 0 ; iPicIndex < sCount ; iPicIndex++ )
	{
		fseek( res, lPos + 2 + iPicIndex * 4, SEEK_SET );
		fread ( &lBegin, sizeof( long ), 1, res );
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

		fseek( res, lBegin, SEEK_SET );
		fread ( &sLocWidth, sizeof( short ), 1, res );
		fread ( &sLocHeight, sizeof( short ), 1, res );
		fread ( &sLocHotX, sizeof( short ), 1, res );
		fread ( &sLocHotY, sizeof( short ), 1, res );

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
		lRows  = ( long * ) malloc ( sizeof( long ) * sLocHeight );

		if( lBegin + 8 + sLocHeight * 4 > lEnd )
		{
			return false;
		}
		fseek( res, lBegin + 8, SEEK_SET );
		fread ( lRows, sizeof( long ), sLocHeight, res );
		for (iY = 0; iY < sLocHeight; iY++ )
		{
			bCopyNotSkip = false;
			iX = 0;
			Opacity = 255;
			iBlockIndex = 0;
			unsigned char CurData;

			fseek( res, lRows[iY] + iBlockIndex + lPos, SEEK_SET );
			fread ( &CurData, sizeof( char ), 1, res );
			while ( lRows[iY] + iBlockIndex < lEnd - lPos && CurData != 255 )
			{
				if( iY * sLocWidth + iX + CurData > sLocWidth * sLocHeight )
				{
					return false;
				}
				memset( &Images[iPicIndex].alpha[iY * sLocWidth + iX], Opacity, CurData );

				if ( bCopyNotSkip )
				{
					fseek( res, lRows[iY] + iBlockIndex + lPos + 1, SEEK_SET );
					fread ( &Images[iPicIndex].data[iY * sLocWidth + iX], sizeof( char ), CurData, res );
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

				fseek( res, lRows[iY] + iBlockIndex + lPos, SEEK_SET );
				fread ( &CurData, sizeof( char ), 1, res );
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
	short sLocWidth, sLocHeight, sLocHotX, sLocHotY, sCnt;
	int iOutOfs, iInOfs;
	if( lLenght < 776 )
	{
		return false;
	}
	fseek( res, lPos, SEEK_SET );
	fread ( &sLocHotX, sizeof( short ), 1, res );
	fread ( &sLocHotY, sizeof( short ), 1, res );
	fread ( &sLocWidth, sizeof( short ), 1, res );
	fread ( &sLocHeight, sizeof( short ), 1, res );
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
		fseek (res, lPos + iInOfs, SEEK_SET );
		fread ( &sCnt, sizeof( short ), 1, res );
		iInOfs += 2;

		Buffer = (unsigned char *) malloc ( 1 );

		if( sCnt <= 0 )
		{
			sCnt = -sCnt;
			if ( iInOfs >= lLenght || sCnt + iOutOfs > sLocWidth * sLocHeight )
			{
				return false;
			}

			fseek( res, lPos + iInOfs, SEEK_SET );
			fread( Buffer, sizeof( char ), 1, res);
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
			
			fseek( res, lPos + iInOfs, SEEK_SET );
			fread( Buffer, sizeof( char ), 1, res);
			for(int i = 0; i < sCnt; i++ )
			{
				Images[0].data[iOutOfs+i] = Buffer[0];
			}

			free ( Buffer );
			Buffer = (unsigned char *) malloc ( sCnt );
			fseek( res, lPos + iInOfs, SEEK_SET );
			fread( Buffer, sizeof( char ), sCnt, res);
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
		fseek( res, lPos + 8, SEEK_SET );
		fread( palette, sizeof( char ), 768, res);
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

SDL_Surface* getImage(string file_name, int imageNr)
{
	long lPosOfFile = lPosBegin;

	//search desired file in max.res
	while( lPosOfFile < lEndOfFile )
	{
		char name[9];

		fseek( res, lPosOfFile, SEEK_SET );

		fread ( name, sizeof( char ), 8, res );
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
	fread ( &Image.lPos, sizeof( long ), 1, res );
	fread ( &Image.lLenght, sizeof( long ), 1, res );
	
	//copy color table
	Image.palette = (sPixel*) malloc ( 768 );
	memcpy (Image.palette, orig_palette, 768);

	//extract image
	Image.decodeFile();
	Image.resampleFile();
	
	return Image.getSurface(imageNr);
	
}

//sets the player colors in the color table to white
void removePlayerColor( SDL_Surface *surface)
{
	for ( int i = 32; i < 40 ; i++ )
	{
		surface->format->palette->colors[i].r = 255;
		surface->format->palette->colors[i].g = 255;
		surface->format->palette->colors[i].b = 255;
	}
}

int saveAllFiles()
{
	long lPosOfFile = lPosBegin;
	while( lPosOfFile < lEndOfFile )
	{
		cImage Image;

		fseek( res, lPosOfFile, SEEK_SET );

		fread ( &Image.name, sizeof( char ), 8, res );
		Image.name[8] = '\0';


		//read pos and length of image in max.res
		fread ( &Image.lPos, sizeof( long ), 1, res );
		fread ( &Image.lLenght, sizeof( long ), 1, res );

		
		//copy color table
		Image.palette = (sPixel*) malloc ( 768 );
		memcpy (Image.palette, orig_palette, 768);

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
	save_PCX ( surface, dst );
	SDL_FreeSurface ( surface );

	return 1;
}

//rpc stands for "remove player color"
int copyFileFromRes_rpc(string src, string dst, int number )
{
	SDL_Surface *surface;
	surface = getImage(src, number);
	removePlayerColor( surface );
	save_PCX( surface, dst);
	SDL_FreeSurface( surface );

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
	
	save_PCX(animation->surface, dst);

	return 1;
}

