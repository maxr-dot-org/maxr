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

#include "converter.h"

#include "3rd/SDL_flic/SDL_flic.h"
#include "file.h"
#include "palette.h"
#include "pcx.h"

#include <SDL.h>
#include <math.h>
#include <sstream>
#include <string>

#if MAC
# include "mac/sources/resinstallerGUI.h"
#endif

cImage* cImage::Image = nullptr;

cImage::cImage()
{
	palette = nullptr;
	iImageCount = 0;
}

cImage::~cImage()
{
	if (palette)
	{
		free (palette);
	}

	if (bDecoded)
	{
		for (int i = 0; i < iImageCount; i++)
		{
			SDL_FreeSurface (Images[i].surface);

			free (Images[i].data);
			free (Images[i].alpha);
		}
		free (Images);
	}
}

void cImage::saveFile()
{
	if (!bDecoded)
		return;
	for (int iNum = 0; iNum < iImageCount; iNum++)
	{
		std::string sOutputname = name;
		if (iImageCount > 1)
		{
			sOutputname += "_";
			char szTmp[13];
			sprintf (szTmp, "%0.3d", iNum);
			sOutputname += szTmp;
		}
		sOutputname += ".pcx";

		savePCX (Images[iNum].surface, sOutputPath / sOutputname);

		std::cout << name;
		if (iImageCount > 1)
		{
			std::cout << "_" << iNum;
		}
		std::cout << "\n";
	}
}

void cImage::resampleFile()
{
	if (!bDecoded) return;
	int iMaxHotX = 0, iMaxHotY = 0, iMaxRight = 0, iMaxBottom = 0;
	cImageData ImageData;
	for (int j = 0; j < iImageCount; j++)
	{
		if (iMaxHotX < Images[j].sUHotX)
		{
			iMaxHotX = Images[j].sUHotX;
		}
		if (iMaxHotY < Images[j].sUHotY)
		{
			iMaxHotY = Images[j].sUHotY;
		}
		if (iMaxRight < Images[j].sWidth - Images[j].sUHotX)
		{
			iMaxRight = Images[j].sWidth - Images[j].sUHotX;
		}
		if (iMaxBottom < Images[j].sHeight - Images[j].sUHotY)
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
	for (backgroundIndex = 64; backgroundIndex < 256; backgroundIndex++)
	{
		if ((palette[backgroundIndex].Blue == 215) && (palette[backgroundIndex].Green == 7) && (palette[backgroundIndex].Red == 255))
			break;
	}

	if (backgroundIndex > 255)
	{
		backgroundIndex = 0;
	}
	else
	{
		palette[backgroundIndex].Blue = 255;
		palette[backgroundIndex].Green = 0;
		palette[backgroundIndex].Red = 255;
	}

	for (int iF = 0; iF < iImageCount; iF++)
	{
		//create surface
		SDL_Surface* surface = SDL_CreateRGBSurface (SDL_SWSURFACE, sWidth, sHeight, 8, 0, 0, 0, 0);
		if (surface == nullptr)
		{
			std::cout << "Out of memory\n";
			exit (-1);
		}

		//copy color table
		surface->format->palette->ncolors = 256;
		for (int i = 0; i < 256; i++)
		{
			//attention: for some reason r and b must be swapped!
			surface->format->palette->colors[i].r = palette[i].Blue;
			surface->format->palette->colors[i].g = palette[i].Green;
			surface->format->palette->colors[i].b = palette[i].Red;
		}
		if (backgroundIndex > 0)
		{
			SDL_SetColorKey (surface, SDL_TRUE, SDL_MapRGB (surface->format, 255, 0, 255));
		}

		for (int iX = 0; iX < sWidth; iX++)
		{
			for (int iY = 0; iY < sHeight; iY++)
			{
				Uint8* pixel = (Uint8*) surface->pixels + (iY * surface->pitch + iX);

				*pixel = backgroundIndex;

				ImageData = Images[iF];

				if (iX < iMaxHotX - ImageData.sUHotX)
				{
					continue; // Image is right than point
				}
				if (iY < iMaxHotY - ImageData.sUHotY)
				{
					continue; // Image is bottom than point
				}
				if (iX >= ImageData.sWidth - ImageData.sUHotX + iMaxHotX)
				{
					continue; // Image is left than point
				}
				if (iY >= ImageData.sHeight - ImageData.sUHotY + iMaxHotY)
				{
					continue; // Image is top than point
				}
				if (ImageData.alpha[iX - iMaxHotX + ImageData.sUHotX + (iY - iMaxHotY + ImageData.sUHotY) * ImageData.sWidth] == 0)
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
	if (decodeSimpleImage())
	{
		bDecoded = true;
	}
	else if (decodeMultiShadow())
	{
		bDecoded = true;
	}
	else if (decodeMultiImage())
	{
		bDecoded = true;
	}
	else if (decodeBigImage())
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
	Sint16 sLocWidth, sLocHeight, sLocHotX, sLocHotY;
	SDL_RWseek (res, lPos, SEEK_SET);
	sLocWidth = (Sint16) SDL_ReadLE16 (res);
	sLocHeight = (Sint16) SDL_ReadLE16 (res);
	sLocHotX = (Sint16) SDL_ReadLE16 (res);
	sLocHotY = (Sint16) SDL_ReadLE16 (res);

	if (sLocWidth > 640 || sLocHeight > 480)
	{
		return false;
	}
	if (sLocWidth < 1 || sLocHeight < 1 || abs (sLocHotX) > 640 || abs (sLocHotY) > 480)
	{
		return false;
	}
	if (lLenght != sLocWidth * sLocHeight + 8)
	{
		return false;
	}
	iImageCount = 1;
	Images = (cImageData*) malloc (sizeof (cImageData) * iImageCount);
	Images[0].sWidth = sLocWidth;
	Images[0].sHeight = sLocHeight;
	Images[0].sHotX = sLocHotX;
	Images[0].sHotY = sLocHotY;
	Images[0].sUHotX = 0;
	Images[0].sUHotY = 0;

	Images[0].data = (unsigned char*) malloc (sizeof (unsigned char) * sLocWidth * sLocHeight);
	SDL_RWread (res, Images[0].data, sizeof (unsigned char), sLocWidth * sLocHeight);
	Images[0].alpha = (unsigned char*) malloc (sizeof (unsigned char) * sLocWidth * sLocHeight);
	memset (Images[0].alpha, 0, sLocWidth * sLocHeight);

	iImageCount = 1;
	return true;
}

bool cImage::decodeMultiShadow()
{
	Sint32 iX, iY, iBlockIndex, iPicIndex;
	Sint32 lBegin, lEnd, *lBounds, *lRows;
	Sint16 sCount, sLocWidth, sLocHeight, sLocHotX, sLocHotY;
	unsigned char Opacity, Color;

	if (lLenght < 2)
	{
		return false;
	}
	SDL_RWseek (res, lPos, SEEK_SET);
	sCount = (Sint16) SDL_ReadLE16 (res);
	if (sCount < 1)
	{
		return false;
	}
	if (lLenght < sCount * 12 + 2)
	{
		return false;
	}
	lBounds = (Sint32*) malloc (sizeof (Sint32) * sCount);
	for (iPicIndex = 0; iPicIndex < sCount; iPicIndex++)
	{
		SDL_RWseek (res, lPos + 2 + iPicIndex * 4, SEEK_SET);
		lBounds[iPicIndex] = (Sint32) SDL_ReadLE32 (res);
		if (lBounds[iPicIndex] > lLenght)
		{
			free (lBounds);
			return false;
		}
	}
	Images = (cImageData*) malloc (sizeof (cImageData) * sCount);

	for (iPicIndex = 0; iPicIndex < sCount; iPicIndex++)
	{
		SDL_RWseek (res, lPos + 2 + iPicIndex * 4, SEEK_SET);
		lBegin = (Sint32) SDL_ReadLE32 (res);
		lEnd = lLenght;
		for (iX = 0; iX < sCount; iX++)
		{
			if (lEnd > lBounds[iX] && lBegin < lBounds[iX])
			{
				lEnd = lBounds[iX];
			}
		}
		lBegin += lPos;
		lEnd += lPos;

		SDL_RWseek (res, lBegin, SEEK_SET);
		sLocWidth = (Sint16) SDL_ReadLE16 (res);
		sLocHeight = (Sint16) SDL_ReadLE16 (res);
		sLocHotX = (Sint16) SDL_ReadLE16 (res);
		sLocHotY = (Sint16) SDL_ReadLE16 (res);

		if (sLocWidth < 1 || sLocWidth > 640 || sLocHeight < 1 || sLocHeight > 480 || abs (sLocHotX) > 640 || abs (sLocHotY) > 480)
		{
			free (lBounds);
			free (Images);
			return false;
		}

		Images[iPicIndex].sWidth = sLocWidth;
		Images[iPicIndex].sHeight = sLocHeight;
		Images[iPicIndex].sHotX = sLocHotX;
		Images[iPicIndex].sHotY = sLocHotY;
		Images[iPicIndex].sUHotX = sLocHotX;
		Images[iPicIndex].sUHotY = sLocHotY;

		Images[iPicIndex].data = (unsigned char*) malloc (sizeof (unsigned char) * sLocWidth * sLocHeight);
		memset (Images[iPicIndex].data, 0, sLocWidth * sLocHeight);
		Images[iPicIndex].alpha = (unsigned char*) malloc (sizeof (unsigned char) * sLocWidth * sLocHeight);
		memset (Images[iPicIndex].alpha, 255, sLocWidth * sLocHeight);
		lRows = (Sint32*) malloc (sizeof (Sint32) * sLocHeight);

		if (lBegin + 8 + sLocHeight * 4 > lEnd)
		{
			//free memory here
			return false;
		}
		SDL_RWseek (res, lBegin + 8, SEEK_SET);
		for (int i = 0; i < sLocHeight; i++)
			lRows[i] = (Sint32) SDL_ReadLE32 (res);

		//SDL_WRread ( res, lRows, sizeof( Sint32 ), sLocHeight, );
		for (iY = 0; iY < sLocHeight; iY++)
		{
			Color = 0;
			iX = 0;
			Opacity = 255;
			iBlockIndex = 0;
			unsigned char CurData;

			SDL_RWseek (res, lRows[iY] + iBlockIndex + lPos, SEEK_SET);
			SDL_RWread (res, &CurData, sizeof (char), 1);
			while (lRows[iY] + iBlockIndex < lEnd - lPos && CurData != 255)
			{
				if (iY * sLocWidth + iX + CurData > sLocWidth * sLocHeight)
				{
					//free memory here
					return false;
				}
				memset (&Images[iPicIndex].data[iY * sLocWidth + iX], Color, CurData);
				memset (&Images[iPicIndex].alpha[iY * sLocWidth + iX], Opacity, CurData);
				Color = 1 - Color;
				Opacity = 255 - Opacity;
				iX += CurData;
				iBlockIndex++;

				SDL_RWseek (res, lRows[iY] + iBlockIndex + lPos, SEEK_SET);
				SDL_RWread (res, &CurData, sizeof (char), 1);
			}
		}
	}
	memset (palette, 255, 3);
	memset (&palette[1], 0, 3);
	iImageCount = sCount;

	free (lBounds);
	free (lRows);
	return true;
}

bool cImage::decodeMultiImage()
{
	Sint32 iX, iY, iBlockIndex, iPicIndex;
	Sint32 lBegin, lEnd, *lBounds, *lRows;
	Sint16 sCount, sLocWidth, sLocHeight, sLocHotX, sLocHotY;
	bool bCopyNotSkip;
	unsigned char Opacity;

	if (lLenght < 2)
	{
		return false;
	}
	SDL_RWseek (res, lPos, SEEK_SET);
	sCount = (Sint16) SDL_ReadLE16 (res);
	if (sCount < 1)
	{
		return false;
	}
	if (lLenght < sCount * 12 + 2)
	{
		return false;
	}
	lBounds = (Sint32*) malloc (sizeof (long) * sCount);
	for (iPicIndex = 0; iPicIndex < sCount; iPicIndex++)
	{
		SDL_RWseek (res, lPos + 2 + iPicIndex * 4, SEEK_SET);
		lBounds[iPicIndex] = (Sint32) SDL_ReadLE32 (res);
		if (lBounds[iPicIndex] > lLenght)
		{
			return false;
		}
	}
	Images = (cImageData*) malloc (sizeof (cImageData) * sCount);

	for (iPicIndex = 0; iPicIndex < sCount; iPicIndex++)
	{
		SDL_RWseek (res, lPos + 2 + iPicIndex * 4, SEEK_SET);
		lBegin = (Sint32) SDL_ReadLE32 (res);
		lEnd = lLenght;
		for (iX = 0; iX < sCount; iX++)
		{
			if (lEnd > lBounds[iX] && lBegin < lBounds[iX])
			{
				lEnd = lBounds[iX];
			}
		}
		lBegin += lPos;
		lEnd += lPos;

		SDL_RWseek (res, lBegin, SEEK_SET);
		sLocWidth = (Sint16) SDL_ReadLE16 (res);
		sLocHeight = (Sint16) SDL_ReadLE16 (res);
		sLocHotX = (Sint16) SDL_ReadLE16 (res);
		sLocHotY = (Sint16) SDL_ReadLE16 (res);

		if (sLocWidth < 1 || sLocWidth > 640 || sLocHeight < 1 || sLocHeight > 480 || abs (sLocHotX) > 640 || abs (sLocHotY) > 480)
		{
			return false;
		}

		Images[iPicIndex].sWidth = sLocWidth;
		Images[iPicIndex].sHeight = sLocHeight;
		Images[iPicIndex].sHotX = sLocHotX;
		Images[iPicIndex].sHotY = sLocHotY;
		Images[iPicIndex].sUHotX = sLocHotX;
		Images[iPicIndex].sUHotY = sLocHotY;

		Images[iPicIndex].data = (unsigned char*) malloc (sizeof (unsigned char) * sLocWidth * sLocHeight);
		memset (Images[iPicIndex].data, 0, sLocWidth * sLocHeight);
		Images[iPicIndex].alpha = (unsigned char*) malloc (sizeof (unsigned char) * sLocWidth * sLocHeight);
		memset (Images[iPicIndex].alpha, 255, sLocWidth * sLocHeight);
		lRows = (Sint32*) malloc (sizeof (Sint32) * sLocHeight);

		if (lBegin + 8 + sLocHeight * 4 > lEnd)
		{
			return false;
		}
		SDL_RWseek (res, lBegin + 8, SEEK_SET);
		for (int i = 0; i < sLocHeight; i++)
			lRows[i] = (Sint32) SDL_ReadLE32 (res);

		for (iY = 0; iY < sLocHeight; iY++)
		{
			bCopyNotSkip = false;
			iX = 0;
			Opacity = 255;
			iBlockIndex = 0;
			unsigned char CurData;

			SDL_RWseek (res, lRows[iY] + iBlockIndex + lPos, SEEK_SET);
			SDL_RWread (res, &CurData, sizeof (char), 1);
			while (lRows[iY] + iBlockIndex < lEnd - lPos && CurData != 255)
			{
				if (iY * sLocWidth + iX + CurData > sLocWidth * sLocHeight)
				{
					return false;
				}
				memset (&Images[iPicIndex].alpha[iY * sLocWidth + iX], Opacity, CurData);

				if (bCopyNotSkip)
				{
					SDL_RWseek (res, lRows[iY] + iBlockIndex + lPos + 1, SEEK_SET);
					SDL_RWread (res, &Images[iPicIndex].data[iY * sLocWidth + iX], sizeof (char), CurData);
					iX += CurData;
					iBlockIndex += CurData + 1;
				}
				else
				{
					memset (&Images[iPicIndex].data[iY * sLocWidth + iX], 0, CurData);
					iX += CurData;
					iBlockIndex++;
				}
				bCopyNotSkip = !bCopyNotSkip;
				Opacity = 255 - Opacity;

				SDL_RWseek (res, lRows[iY] + iBlockIndex + lPos, SEEK_SET);
				SDL_RWread (res, &CurData, sizeof (char), 1);
			}
		}
	}
	free (lBounds);
	free (lRows);

	iImageCount = sCount;
	return true;
}

bool cImage::decodeBigImage()
{
	Sint16 sLocWidth, sLocHeight, sLocHotX, sLocHotY, sCnt;
	int iOutOfs, iInOfs;

	if (lLenght < 776)
	{
		return false;
	}
	SDL_RWseek (res, lPos, SEEK_SET);
	sLocHotX = (Sint16) SDL_ReadLE16 (res);
	sLocHotY = (Sint16) SDL_ReadLE16 (res);
	sLocWidth = (Sint16) SDL_ReadLE16 (res);
	sLocHeight = (Sint16) SDL_ReadLE16 (res);

	if (sLocWidth > 640 || sLocHeight > 480 || sLocWidth < 1 || sLocHeight < 1)
	{
		return false;
	}
	iInOfs = 776;
	iOutOfs = 0;
	iImageCount = 1;
	Images = (cImageData*) malloc (sizeof (cImageData) * iImageCount);
	Images[0].sWidth = sLocWidth;
	Images[0].sHeight = sLocHeight;
	Images[0].sHotX = sLocHotX;
	Images[0].sHotY = sLocHotY;
	Images[0].sUHotX = 0;
	Images[0].sUHotY = 0;

	Images[0].data = (unsigned char*) malloc (sizeof (unsigned char) * sLocWidth * sLocHeight);
	Images[0].alpha = (unsigned char*) malloc (sizeof (unsigned char) * sLocWidth * sLocHeight);
	memset (Images[0].alpha, 0, sLocWidth * sLocHeight);

	unsigned char* Buffer = nullptr;
	while (iInOfs + 1 < lLenght && iOutOfs < sLocWidth * sLocHeight)
	{
		SDL_RWseek (res, lPos + iInOfs, SEEK_SET);
		sCnt = (Sint16) SDL_ReadLE16 (res);
		iInOfs += 2;

		Buffer = (unsigned char*) malloc (1);

		if (sCnt <= 0)
		{
			sCnt = -sCnt;
			if (iInOfs >= lLenght || sCnt + iOutOfs > sLocWidth * sLocHeight)
			{
				return false;
			}

			SDL_RWseek (res, lPos + iInOfs, SEEK_SET);
			SDL_RWread (res, Buffer, sizeof (char), 1);
			for (int i = 0; i < sCnt; i++)
			{
				Images[0].data[iOutOfs + i] = Buffer[0];
			}
			iInOfs++;
			iOutOfs += sCnt;
		}
		else
		{
			if (sCnt + iInOfs > lLenght || sCnt + iOutOfs > sLocWidth * sLocHeight)
			{
				return false;
			}

			SDL_RWseek (res, lPos + iInOfs, SEEK_SET);
			SDL_RWread (res, Buffer, sizeof (char), 1);
			for (int i = 0; i < sCnt; i++)
			{
				Images[0].data[iOutOfs + i] = Buffer[0];
			}

			free (Buffer);
			Buffer = (unsigned char*) malloc (sCnt);
			SDL_RWseek (res, lPos + iInOfs, SEEK_SET);
			SDL_RWread (res, Buffer, sizeof (char), sCnt);
			for (int i = 0; i < sCnt; i++)
			{
				Images[0].data[iOutOfs + i] = Buffer[i];
			}

			free (Buffer);

			iInOfs += sCnt;
			iOutOfs += sCnt;
		}
	}

	if (iInOfs != lLenght || iOutOfs != sLocWidth * sLocHeight)
	{
		Images[0].data = nullptr;
		Images[0].alpha = nullptr;
		Images = nullptr;
		return false;
	}
	else
	{
		SDL_RWseek (res, lPos + 8, SEEK_SET);
		SDL_RWread (res, palette, sizeof (char), 768);
	}

	iImageCount = 1;
	return true;
}

SDL_Surface* cImage::getSurface (int imageNr)
{
	if (imageNr > iImageCount - 1)
	{
		throw InstallException (std::string ("Image '") + name + "' number " + std::to_string (imageNr) + " not found in max.res");
	}

	Images[imageNr].surface->refcount++;

	return Images[imageNr].surface;
}

SDL_Surface* getImageFromRes (std::string file_name, int imageNr)
{
	if (res == nullptr)
	{
		throw InstallException (std::string ("max.res was not opened") + TEXT_FILE_LF);
	}
	cImage* Image = cImage::Image;
	if (Image == nullptr || file_name.compare (Image->name) != 0)
	{
		Uint32 lPosOfFile = lPosBegin;

		Image = new cImage();

		//search desired file in max.res
		while (lPosOfFile < lEndOfFile)
		{
			SDL_RWseek (res, lPosOfFile, SEEK_SET);

			SDL_RWread (res, Image->name, sizeof (char), 8);
			Image->name[8] = '\0';

			if (file_name.compare (Image->name) == 0)
				break;

			lPosOfFile += 16;
		}

		if (lPosOfFile >= lEndOfFile)
		{
			throw InstallException ("Image '" + file_name + "' not found in max.res" + TEXT_FILE_LF);
		}

		//read pos and length of image in max.res
		Image->lPos = (Sint32) SDL_ReadLE32 (res);
		Image->lLenght = (Sint32) SDL_ReadLE32 (res);

		//copy color table
		Image->palette = (sPixel*) malloc (sizeof (sPixel) * 256);
		for (int i = 0; i < 256; i++)
		{
			Image->palette[i].Blue = orig_palette[3 * i];
			Image->palette[i].Green = orig_palette[3 * i + 1];
			Image->palette[i].Red = orig_palette[3 * i + 2];
		}

		//extract image
		Image->decodeFile();
		Image->resampleFile();

		if (cImage::Image != nullptr)
		{
			delete cImage::Image;
		}
		cImage::Image = Image;
	}

	if (!Image->bDecoded)
	{
		throw InstallException ("Couldn't decode image '" + file_name + "' from max.res" + TEXT_FILE_LF);
	}

	return Image->getSurface (imageNr);
}

//sets the player colors in the color table to white
//note that the information about the player colors are lost, when blitting the surface
void removePlayerColor (SDL_Surface* surface)
{
	SDL_Color color = {255, 255, 255, 0};
	for (int i = 32; i < 39; i++)
	{
		SDL_SetPaletteColors (surface->format->palette, &color, i, 1);
	}
}

int saveAllFiles()
{
	Uint32 lPosOfFile = lPosBegin;
	while (lPosOfFile < lEndOfFile)
	{
		cImage Image;

		SDL_RWseek (res, lPosOfFile, SEEK_SET);

		SDL_RWread (res, &Image.name, sizeof (char), 8);
		Image.name[8] = '\0';

		//read pos and length of image in max.res
		Image.lPos = (Sint32) SDL_ReadLE32 (res);
		Image.lLenght = (Sint32) SDL_ReadLE32 (res);

		//copy color table
		Image.palette = (sPixel*) malloc (sizeof (sPixel) * 256);
		for (int i = 0; i < 256; i++)
		{
			Image.palette[i].Blue = orig_palette[i];
			Image.palette[i].Green = orig_palette[i + 1];
			Image.palette[i].Red = orig_palette[i + 2];
		}

		//extract image
		Image.decodeFile();
		Image.resampleFile();

		Image.saveFile();

		lPosOfFile += 16;
	}
	return 1;
}

void copyFileFromRes (std::string src, const std::filesystem::path& dst, int number)
{
	SDL_Surface* surface = nullptr;
	try
	{
		surface = getImageFromRes (src, number);
		savePCX (surface, dst);
		SDL_FreeSurface (surface);
	}
	END_INSTALL_FILE (dst)
}

//rpc stands for "remove player color"
void copyFileFromRes_rpc (std::string src, const std::filesystem::path& dst, int number)
{
	SDL_Surface* surface = nullptr;
	try
	{
		surface = getImageFromRes (src, number);
		removePlayerColor (surface);
		savePCX (surface, dst);
		SDL_FreeSurface (surface);
	}
	END_INSTALL_FILE (dst)
}

void copyImageFromFLC (const std::filesystem::path& fileName, const std::filesystem::path& dst)
{
	try
	{
		SDL_RWops* file = openFile (fileName, "rb");
		int error;
		FLI_Animation* animation;

		if (file == nullptr)
		{
			throw InstallException ("FLC-File '" + fileName.u8string() + "' not found" + TEXT_FILE_LF);
		}

		animation = FLI_Open (file, &error);
		if (error != 0)
		{
			throw InstallException ("FLC-File '" + fileName.u8string() + "' may be corrupted" + TEXT_FILE_LF);
		}

		error = FLI_NextFrame (animation);
		if (error != 0)
		{
			FLI_Close (animation);
			throw InstallException ("FLC-File '" + fileName.u8string() + "' may be corrupted" + TEXT_FILE_LF);
		}

		savePCX (animation->surface, dst);
	}
	END_INSTALL_FILE (dst)
}

void resizeSurface (SDL_Surface*& surface, int x, int y, int h, int w)
{
	SDL_Rect dst_rect, src_rect;
	SDL_Surface* resizedSurface;

	if (surface->format->BitsPerPixel != 8)
		return;

	if (surface->h > h)
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

	if (surface->w > w)
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

	resizedSurface = SDL_CreateRGBSurface (SDL_SWSURFACE, h, w, 8, 0, 0, 0, 0);
	if (resizedSurface == nullptr)
	{
		std::cout << "Out of memory";
		exit (-1);
	}
	SDL_SetPaletteColors (resizedSurface->format->palette, surface->format->palette->colors, 0, 256);

	Uint32 key;
	if (SDL_GetColorKey (surface, &key) == 0)
		SDL_SetColorKey (resizedSurface, SDL_TRUE, key);

	SDL_FillRect (resizedSurface, nullptr, SDL_MapRGB (resizedSurface->format, 255, 0, 255));

	SDL_BlitSurface (surface, &src_rect, resizedSurface, &dst_rect);

	SDL_FreeSurface (surface);

	surface = resizedSurface;
}

void setColor (SDL_Surface* surface, unsigned char nr, unsigned char r, unsigned char g, unsigned char b)
{
	SDL_Color color;
	color.r = r;
	color.g = g;
	color.b = b;
	SDL_SetPaletteColors (surface->format->palette, &color, nr, 1);
}

void setAnimationColor (SDL_Surface* surface, unsigned char index, unsigned char frame)
{
	switch (index)
	{
		case 7:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 255, 255, 147);
					break;
				case 1:
					setColor (surface, index, 255, 71, 0);
					break;
				case 2:
					setColor (surface, index, 131, 131, 163);
					break;
				case 3:
					setColor (surface, index, 255, 171, 0);
					break;
			}
			break;
		case 9:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 203, 203, 255);
					break;
				case 1:
					setColor (surface, index, 171, 171, 227);
					break;
				case 2:
					setColor (surface, index, 99, 91, 223);
					break;
				case 3:
					setColor (surface, index, 171, 171, 227);
					break;
			}
			break;
		case 10:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 171, 171, 227);
					break;
				case 1:
					setColor (surface, index, 203, 203, 255);
					break;
				case 2:
					setColor (surface, index, 171, 171, 227);
					break;
				case 3:
					setColor (surface, index, 99, 91, 223);
					break;
			}
			break;
		case 11:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 99, 91, 223);
					break;
				case 1:
					setColor (surface, index, 171, 171, 227);
					break;
				case 2:
					setColor (surface, index, 203, 203, 255);
					break;
				case 3:
					setColor (surface, index, 171, 171, 227);
					break;
			}
			break;
		case 12:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 171, 171, 227);
					break;
				case 1:
					setColor (surface, index, 99, 91, 223);
					break;
				case 2:
					setColor (surface, index, 171, 171, 227);
					break;
				case 3:
					setColor (surface, index, 203, 203, 255);
					break;
			}
			break;
		case 13:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 255, 255, 159);
					break;
				case 1:
					setColor (surface, index, 243, 171, 103);
					break;
				case 2:
					setColor (surface, index, 235, 51, 51);
					break;
				case 3:
					setColor (surface, index, 243, 171, 103);
					break;
			}
			break;
		case 14:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 243, 171, 103);
					break;
				case 1:
					setColor (surface, index, 255, 255, 159);
					break;
				case 2:
					setColor (surface, index, 243, 171, 103);
					break;
				case 3:
					setColor (surface, index, 235, 51, 51);
					break;
			}
			break;
		case 15:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 235, 51, 51);
					break;
				case 1:
					setColor (surface, index, 243, 171, 103);
					break;
				case 2:
					setColor (surface, index, 255, 255, 159);
					break;
				case 3:
					setColor (surface, index, 243, 171, 103);
					break;
			}
			break;
		case 16:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 243, 171, 103);
					break;
				case 1:
					setColor (surface, index, 235, 51, 51);
					break;
				case 2:
					setColor (surface, index, 243, 171, 103);
					break;
				case 3:
					setColor (surface, index, 255, 255, 159);
					break;
			}
			break;
		case 17:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 23, 99, 135);
					break;
				case 1:
					setColor (surface, index, 43, 63, 75);
					break;
				case 2:
					setColor (surface, index, 15, 15, 15);
					break;
				case 3:
					setColor (surface, index, 43, 63, 75);
					break;
			}
			break;
		case 19:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 15, 15, 15);
					break;
				case 1:
					setColor (surface, index, 43, 63, 75);
					break;
				case 2:
					setColor (surface, index, 23, 99, 135);
					break;
				case 3:
					setColor (surface, index, 43, 63, 75);
					break;
			}
			break;
		case 21:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 183, 103, 0);
					break;
				case 1:
					setColor (surface, index, 74, 57, 33);
					break;
				case 2:
					setColor (surface, index, 15, 15, 15);
					break;
				case 3:
					setColor (surface, index, 82, 57, 41);
					break;
			}
			break;
		case 22:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 75, 59, 39);
					break;
				case 1:
					setColor (surface, index, 181, 99, 0);
					break;
				case 2:
					setColor (surface, index, 75, 59, 39);
					break;
				case 3:
					setColor (surface, index, 8, 8, 8);
					break;
			}
			break;
		case 23:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 15, 15, 15);
					break;
				case 1:
					setColor (surface, index, 74, 57, 33);
					break;
				case 2:
					setColor (surface, index, 181, 99, 0);
					break;
				case 3:
					setColor (surface, index, 74, 57, 33);
					break;
			}
			break;

		case 24:
			switch (frame)
			{
				case 0:
					setColor (surface, index, 75, 59, 39);
					break;
				case 1:
					setColor (surface, index, 15, 15, 15);
					break;
				case 2:
					setColor (surface, index, 75, 59, 39);
					break;
				case 3:
					setColor (surface, index, 181, 99, 0);
					break;
			}
			break;
	}
}

//change palette of surface to generate an animation frame
void generateAnimationFrame (SDL_Surface* surface, unsigned char frame)
{
	setAnimationColor (surface, 7, frame);
	setAnimationColor (surface, 9, frame);
	setAnimationColor (surface, 10, frame);
	setAnimationColor (surface, 11, frame);
	setAnimationColor (surface, 12, frame);
	setAnimationColor (surface, 13, frame);
	setAnimationColor (surface, 14, frame);
	setAnimationColor (surface, 15, frame);
	setAnimationColor (surface, 16, frame);
	setAnimationColor (surface, 17, frame);
	setAnimationColor (surface, 19, frame);
	setAnimationColor (surface, 21, frame);
	setAnimationColor (surface, 22, frame);
	setAnimationColor (surface, 23, frame);
	setAnimationColor (surface, 24, frame);
}

void updateProgressbar()
{
	static int value = 0;

	if (iInstalledFiles == 1) //we are drawing a new progress bar
	{
		value = 0;
	}

	int newValue = (int) ((float) iInstalledFiles * 72 / iTotalFiles);

	for (int i = value; i < newValue; i++)
		std::cout << ".";

	value = newValue;

#if MAC
	updateProgressWindow ("", iTotalFiles, iInstalledFiles);
#endif
}

void writeLog (std::string msg)
{
	if (logFile != nullptr)
	{
		SDL_RWwrite (logFile, msg.c_str(), msg.length(), 1);
	}
	else
	{
		std::cout << msg;
	}
}
