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
 
#ifndef ConverterH
#define ConverterH
#include <SDL.h>
#include "defines.h"
#include "resinstaller.h"

struct sPixel
{
	unsigned char Blue, Green, Red;
};

struct cImageData
{
	short sWidth;
	short sHeight;
	short sHotX;
	short sHotY;
	short sUHotX;
	short sUHotY;

	cImageData *Images;

	unsigned char *data;
	unsigned char *alpha;

	SDL_Surface* surface;
};

class cImage
{
	bool bDecoded;
	short sWidth;
	short sHeight;
	cImageData *Images;

	int iImageCount;

	short sHotX;
	short sHotY;

	bool decodeSimpleImage();
	bool decodeMultiShadow();
	bool decodeMultiImage();
	bool decodeBigImage();

public:
	
	cImage();
	~cImage();

	char name[9];
	Sint32 lPos;
	Sint32 lLenght;

	sPixel *palette;

	void decodeFile();
	void resampleFile();
	void saveFile();
	SDL_Surface* getSurface(int imageNr = 0);
};

SDL_Surface* getImage(string file_name, int imageNr = 0);
void removePlayerColor( SDL_Surface *surface);
int saveAllFiles();
int copyFileFromRes ( string src, string dst, int number = 0 );
int copyFileFromRes_rpc(string src, string dst, int number = 0 );
int copyImageFromFLC(string fileName, string dst);

/** resizes (not scales!) a surface
* @author Eiko
* @param surface the surface to resize
* @param h the new hight of the surface
* @param w the new width of the surface
* @param x x and y are used for blitting the content from the old to the resized suface.
* e. g. when increaseing the x dimension, x is used for the destination rect.
* When decreaseing the x dimention, x is used for the source rect.
*/
void resizeSurface ( SDL_Surface*& surface, int x, int y, int h, int w );

void generateAnimationFrame( SDL_Surface* surface, unsigned char frame);


#endif // ConvertH
