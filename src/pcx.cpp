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

#include "pcx.h"

#include "converter.h"

#include <SDL.h>
#include <string>

int savePCX_8bpp (SDL_Surface* surface, const std::filesystem::path& fileName)
{
	int Z_Index, S_Index; // PCX-Größenangaben

	unsigned char* bild; // Adresse des Bildspeichers (Array)
	unsigned char Pixel;
	unsigned char Anzahl;
	int i, z, s;
	long Index; // Adresse des Pixels im Bild

	std::filesystem::create_directories (fileName.parent_path());
	SDL_RWops* file = SDL_RWFromFile (fileName.string().c_str(), "wb");

	if (file == nullptr)
	{
		throw InstallException (std::string ("Couldn't open file for writing") + TEXT_FILE_LF);
	}

	//PCX-Header erzeugen
	unsigned char PCXHeader[128];
	for (i = 0; i < 128; i++)
	{
		PCXHeader[i] = 0;
	}

	S_Index = surface->w - 1;
	Z_Index = surface->h - 1;

	PCXHeader[0] = 10; // Zsoft
	PCXHeader[1] = 5; // Version 5
	PCXHeader[2] = 1; // RLC
	PCXHeader[3] = 8; // Bits pro Pixel und Ebene
	PCXHeader[8] = S_Index - 256 * (S_Index / 256); // Xmax Low-Byte
	PCXHeader[9] = S_Index / 256; // Xmax High-Byte
	PCXHeader[10] = Z_Index - 256 * (Z_Index / 256); // Ymax Low-Byte
	PCXHeader[11] = Z_Index / 256; // Ymax High-Byte

	PCXHeader[65] = 1; // Anzahl der Ebenen
	PCXHeader[66] = S_Index + 1 - 256 * ((S_Index + 1) / 256); // Bytes/Zeile
	PCXHeader[67] = (S_Index + 1) / 256;
	PCXHeader[68] = 1; // Farbe

	SDL_RWwrite (file, PCXHeader, 128, 1);

	bild = (unsigned char*) surface->pixels;

	// RLC berechnen
	for (z = 0; z <= Z_Index; z++)
	{
		Index = (long) z * (long) surface->pitch;
		s = 0;
		while (s <= S_Index)
		{
			Pixel = bild[Index + s]; // erstes Vorkommen der Farbe
			s++;
			Anzahl = 1;
			while ((s <= S_Index) && (Pixel == bild[Index + s]) && (Anzahl < 63))
			{
				Anzahl++;
				s++;
			}
			// Anzahl>1 oder Farbe>=192, dann RLC durchführen
			if ((Anzahl > 1) || (Pixel >= 192))
			{
				Anzahl = 0xC0 + Anzahl; // RLC-Kennung
				SDL_RWwrite (file, &Anzahl, 1, 1);
				SDL_RWwrite (file, &Pixel, 1, 1);
			}
			else
			{
				SDL_RWwrite (file, &Pixel, 1, 1); // einmaliges Pixel mit Grauwert<192
			}
		}
	}

	//write color table
	Pixel = 12; //Kennzeichen f. Bild-Ende
	SDL_RWwrite (file, &Pixel, 1, 1);

	for (i = 0; i < 256; i++)
	{
		SDL_Color* colors = surface->format->palette->colors;
		SDL_RWwrite (file, &colors[i].r, 1, 1);
		SDL_RWwrite (file, &colors[i].g, 1, 1);
		SDL_RWwrite (file, &colors[i].b, 1, 1);
	}

	SDL_RWclose (file);
	return 1;
}

int savePCX_32bpp (SDL_Surface* surface, const std::filesystem::path& fileName)
{
	int Z_Index, S_Index; // PCX-Größenangaben

	unsigned char* bild; // Adresse des Bildspeichers (Array)
	unsigned char Pixel;
	unsigned char Anzahl;
	int i, j, z, s;
	long Index; // Adresse des Pixels im Bild

	std::filesystem::create_directories (fileName.parent_path());
	SDL_RWops* file = SDL_RWFromFile (fileName.string().c_str(), "wb");

	if (file == nullptr)
	{
		throw InstallException (std::string ("Couldn't open file for writing") + TEXT_FILE_LF);
	}

	//PCX-Header erzeugen
	unsigned char PCXHeader[128];
	for (i = 0; i < 128; i++)
	{
		PCXHeader[i] = 0;
	}

	S_Index = surface->w - 1;
	Z_Index = surface->h - 1;

	PCXHeader[0] = 10; // Zsoft
	PCXHeader[1] = 5; // Version 5
	PCXHeader[2] = 1; // RLC
	PCXHeader[3] = 8; // Bits pro Pixel und Ebene
	PCXHeader[8] = S_Index - 256 * (S_Index / 256); // Xmax Low-Byte
	PCXHeader[9] = S_Index / 256; // Xmax High-Byte
	PCXHeader[10] = Z_Index - 256 * (Z_Index / 256); // Ymax Low-Byte
	PCXHeader[11] = Z_Index / 256; // Ymax High-Byte

	PCXHeader[65] = 1; // Anzahl der Ebenen
	PCXHeader[66] = S_Index + 1 - 256 * ((S_Index + 1) / 256); // Bytes/Zeile
	PCXHeader[67] = (S_Index + 1) / 256;
	PCXHeader[68] = 1; // Farbe

	SDL_RWwrite (file, PCXHeader, 128, 1);

	//build color table
	bild = (unsigned char*) malloc (surface->w * surface->h - 1);
	Uint32* surface_data = (Uint32*) surface->pixels;
	Uint32 colors[256];
	for (i = 0; i < 256; i++)
	{
		colors[i] = 0;
	}
	int NrColors = 0;

	for (Index = 0; Index < surface->h * surface->w - 1; Index++)
	{
		Uint32 sourceIndex = (Index / surface->w) * (surface->pitch / 4) + (Index % surface->w);
		//search color in table
		for (j = 0; j < NrColors; j++)
		{
			if (colors[j] == surface_data[sourceIndex])
			{
				bild[Index] = j;
				break;
			}
		}

		//add color, if its not in the table
		if (j == NrColors)
		{
			if (NrColors > 255)
			{
				//to many colors, table full
				throw InstallException (std::string ("Couldn't convert image to 8 bpp, color table full") + TEXT_FILE_LF);
			}
			colors[NrColors] = surface_data[sourceIndex];
			bild[Index] = NrColors;
			NrColors++;
		}
	}

	// RLC berechnen
	for (z = 0; z <= Z_Index; z++)
	{
		Index = (long) z * (long) surface->w;
		s = 0;
		while (s <= S_Index)
		{
			Pixel = bild[Index + s]; // erstes Vorkommen der Farbe
			s++;
			Anzahl = 1;
			while ((s <= S_Index) && (Pixel == bild[Index + s]) && (Anzahl < 63))
			{
				Anzahl++;
				s++;
			}
			// Anzahl>1 oder Farbe>=192, dann RLC durchführen
			if ((Anzahl > 1) || (Pixel >= 192))
			{
				Anzahl = 0xC0 + Anzahl; // RLC-Kennung
				SDL_RWwrite (file, &Anzahl, 1, 1);
				SDL_RWwrite (file, &Pixel, 1, 1);
			}
			else
			{
				SDL_RWwrite (file, &Pixel, 1, 1); // einmaliges Pixel mit Grauwert<192
			}
		}
	}

	//write color table
	Pixel = 12; //Kennzeichen f. Bild-Ende
	SDL_RWwrite (file, &Pixel, 1, 1);

	unsigned char temp;
	for (i = 0; i < 256; i++)
	{
		temp = (unsigned char) (colors[i] / 65536);
		SDL_RWwrite (file, &temp, 1, 1);
		temp = (unsigned char) (colors[i] / 256);
		SDL_RWwrite (file, &temp, 1, 1);
		temp = (unsigned char) colors[i];
		SDL_RWwrite (file, &temp, 1, 1);
	}

	SDL_RWclose (file);

	free (bild);
	return 1;
}

int savePCX (SDL_Surface* surface, const std::filesystem::path& fileName)
{
	if (!surface)
		return 0;

	if (surface->format->BitsPerPixel == 8)
	{
		return savePCX_8bpp (surface, fileName);
	}
	else if (surface->format->BitsPerPixel == 32)
	{
		return savePCX_32bpp (surface, fileName);
	}

	return 0;
}

SDL_Surface* loadPCX (const std::filesystem::path& name)
{
	Uint8* _ptr;
	Uint8 byte;
	Sint32 k = 0, i = 0, z, j;
	SDL_Surface* sf;
	Sint16 x, y;
	SDL_RWops* file;

	//open file
	file = SDL_RWFromFile (name.string().c_str(), "rb");

	if (file == nullptr)
	{
		throw InstallException (std::string ("Couldn't open file") + name.string() + TEXT_FILE_LF);
	}

	//load data
	SDL_RWseek (file, 8, SEEK_SET);
	x = SDL_ReadLE16 (file);
	y = SDL_ReadLE16 (file);
	x++;
	y++;
	sf = SDL_CreateRGBSurface (SDL_SWSURFACE, x, y, 8, 0, 0, 0, 0);
	SDL_SetColorKey (sf, SDL_TRUE, 0xFF00FF);

	_ptr = (Uint8*) sf->pixels;
	SDL_RWseek (file, 128, SEEK_SET);
	do
	{
		SDL_RWread (file, &byte, 1, 1);
		if (byte > 191)
		{
			z = byte - 192;
			if (z + k > x)
				z = x - k;
			SDL_RWread (file, &byte, 1, 1);
			for (j = 0; j < z; j++)
			{
				_ptr[k + i * sf->pitch] = byte;
				k++;
				if (k == x) break;
			}
		}
		else
		{
			_ptr[k + i * sf->pitch] = byte;
			k++;
		}
		if (k == x)
		{
			k = 0;
			i++;
		}
	} while (i != y);

	//load color table
	SDL_RWseek (file, -768, SEEK_END);

	SDL_Color colors[256];
	for (i = 0; i < 256; i++)
	{
		SDL_RWread (file, &colors[i].r, 1, 1);
		;
		SDL_RWread (file, &colors[i].g, 1, 1);
		SDL_RWread (file, &colors[i].b, 1, 1);
	}
	SDL_SetPaletteColors (sf->format->palette, colors, 0, 256);

	SDL_RWclose (file);
	return sf;
}
