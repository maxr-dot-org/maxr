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

#include "defines.h"
#include "resinstaller.h"

#include <SDL.h>
#include <filesystem>

struct sPixel
{
	unsigned char Blue = 0;
	unsigned char Green = 0;
	unsigned char Red = 0;
};

struct cImageData
{
	short sWidth = 0;
	short sHeight = 0;
	short sHotX = 0;
	short sHotY = 0;
	short sUHotX = 0;
	short sUHotY = 0;

	cImageData* Images = nullptr;

	unsigned char* data = nullptr;
	unsigned char* alpha = nullptr;

	SDL_Surface* surface = nullptr;
};

class cImage
{
	bool decodeSimpleImage();
	bool decodeMultiShadow();
	bool decodeMultiImage();
	bool decodeBigImage();

public:
	static cImage* Image;

	cImage() = default;
	cImage (const cImage&) = delete;
	~cImage();
	cImage& operator= (const cImage&) = delete;

	void decodeFile();
	void resampleFile();
	void saveFile();
	SDL_Surface* getSurface (int imageNr = 0);

private:
	short sWidth = 0;
	short sHeight = 0;
	cImageData* Images = nullptr;

	int iImageCount = 0;

	short sHotX = 0;
	short sHotY = 0;

public:

	bool bDecoded = false;
	char name[9]{};
	Sint32 lPos = 0;
	Sint32 lLenght = 0;

	sPixel* palette = nullptr;
};

SDL_Surface* getImageFromRes (std::string file_name, int imageNr = 0);
void removePlayerColor (SDL_Surface* surface);
void saveAllFiles();
void copyFileFromRes (std::string src, const std::filesystem::path& dst, int number = 0);
void copyFileFromRes_rpc (std::string src, const std::filesystem::path& dst, int number = 0);
void copyImageFromFLC (const std::filesystem::path& fileName, const std::filesystem::path& dst);

/** resizes (not scales!) a surface
* @author Eiko
* @param surface the surface to resize
* @param x x and y are used for blitting the content from the old to the resized suface.
* e. g. when increasing the x dimension, x is used for the destination rect.
* @param h the new hight of the surface
* @param w the new width of the surface
* When decreasing the x dimension, x is used for the source rect.
*/
void resizeSurface (SDL_Surface*& surface, int x, int y, int h, int w);
void setColor (SDL_Surface* surface, unsigned char nr, unsigned char r, unsigned char g, unsigned char b);
void generateAnimationFrame (SDL_Surface* surface, unsigned char frame);

void updateProgressbar();
void writeLog (std::string msg);

#endif // ConvertH
