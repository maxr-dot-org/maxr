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
#include <array>
#include <fstream>
#include <string>

namespace
{
	//--------------------------------------------------------------------------
	std::array<unsigned char, 128> createPcxHeader (unsigned int weight, unsigned int height)
	{
		std::array<unsigned char, 128> header{};

		const unsigned int xMax = weight - 1;
		const unsigned int yMax = height - 1;

		header[0] = 10; // Zsoft
		header[1] = 5; // Version 5
		header[2] = 1; // RLC
		header[3] = 8; // Bits pro Pixel und Ebene
		header[8] = xMax & 0xFF; // Xmax Low-Byte
		header[9] = (xMax >> 8) & 0xFF; // Xmax High-Byte
		header[10] = yMax & 0xFF; // Ymax Low-Byte
		header[11] = (yMax >> 8) & 0xFF; // Ymax High-Byte

		header[65] = 1; // Anzahl der Ebenen
		header[66] = weight & 0xFF; // Bytes/Zeile
		header[67] = (weight >> 8) & 0xFF;
		header[68] = 1; // Farbe

		return header;
	}

	//--------------------------------------------------------------------------
	void savePCX_8bpp (const SDL_Surface& surface, const std::filesystem::path& fileName)
	{
		std::filesystem::create_directories (fileName.parent_path());
		std::basic_ofstream<unsigned char> file (fileName, std::ios_base::binary);

		if (!file)
		{
			throw InstallException (std::string ("Couldn't open file for writing") + TEXT_FILE_LF);
		}

		// PCX-Größenangaben
		const int S_Index = surface.w - 1;
		const int Z_Index = surface.h - 1;

		//PCX-Header erzeugen
		const auto PCXHeader = createPcxHeader (surface.w, surface.h);
		file.write (PCXHeader.data(), 128);

		// Adresse des Bildspeichers (Array)
		const unsigned char* bild = static_cast<const unsigned char*> (surface.pixels);

		// RLC berechnen
		for (int z = 0; z <= Z_Index; z++)
		{
			long Index = (long) z * (long) surface.pitch; // Adresse des Pixels im Bild
			int s = 0;
			while (s <= S_Index)
			{
				unsigned char Pixel = bild[Index + s]; // erstes Vorkommen der Farbe
				s++;
				unsigned char Anzahl = 1;
				while ((s <= S_Index) && (Pixel == bild[Index + s]) && (Anzahl < 63))
				{
					Anzahl++;
					s++;
				}
				// Anzahl>1 oder Farbe>=192, dann RLC durchführen
				if ((Anzahl > 1) || (Pixel >= 192))
				{
					Anzahl = 0xC0 + Anzahl; // RLC-Kennung
					file.write (&Anzahl, 1);
					file.write (&Pixel, 1);
				}
				else
				{
					file.write (&Pixel, 1); // einmaliges Pixel mit Grauwert<192
				}
			}
		}

		//write color table
		unsigned char Pixel = 12; //Kennzeichen f. Bild-Ende
		file.write (&Pixel, 1);

		for (int i = 0; i < 256; i++)
		{
			const SDL_Color* colors = surface.format->palette->colors;
			file.write (&colors[i].r, 1);
			file.write (&colors[i].g, 1);
			file.write (&colors[i].b, 1);
		}
	}

	//--------------------------------------------------------------------------
	void savePCX_32bpp (const SDL_Surface& surface, const std::filesystem::path& fileName)
	{
		std::filesystem::create_directories (fileName.parent_path());
		std::basic_ofstream<unsigned char> file (fileName, std::ios_base::binary);

		if (!file)
		{
			throw InstallException (std::string ("Couldn't open file for writing") + TEXT_FILE_LF);
		}

		//PCX-Header erzeugen
		const auto PCXHeader = createPcxHeader (surface.w, surface.h);

		// PCX-Größenangaben
		const int S_Index = surface.w - 1;
		const int Z_Index = surface.h - 1;

		file.write (PCXHeader.data(), 128);

		//build color table
		std::vector<unsigned char> bild (surface.w * surface.h - 1); // Adresse des Bildspeichers
		const Uint32* surface_data = static_cast<const Uint32*> (surface.pixels);
		Uint32 colors[256]{};
		int NrColors = 0;

		long Index; // Adresse des Pixels im Bild
		for (Index = 0; Index < surface.h * surface.w - 1; Index++)
		{
			Uint32 sourceIndex = (Index / surface.w) * (surface.pitch / 4) + (Index % surface.w);
			//search color in table
			const auto j = std::distance (std::begin (colors), std::find (std::begin (colors), std::begin (colors) + NrColors, surface_data[sourceIndex]));

			//add color, if its not in the table
			if (j == NrColors)
			{
				if (NrColors > 255)
				{
					// too many colors, table full
					throw InstallException (std::string ("Couldn't convert image to 8 bpp, color table full") + TEXT_FILE_LF);
				}
				colors[NrColors] = surface_data[sourceIndex];
				NrColors++;
			}
			bild[Index] = j;
		}

		// RLC berechnen
		for (int z = 0; z <= Z_Index; z++)
		{
			Index = (long) z * (long) surface.w;
			int s = 0;
			while (s <= S_Index)
			{
				unsigned char Pixel = bild[Index + s]; // erstes Vorkommen der Farbe
				s++;
				unsigned char Anzahl = 1;
				while ((s <= S_Index) && (Pixel == bild[Index + s]) && (Anzahl < 63))
				{
					Anzahl++;
					s++;
				}
				// Anzahl>1 oder Farbe>=192, dann RLC durchführen
				if ((Anzahl > 1) || (Pixel >= 192))
				{
					Anzahl = 0xC0 + Anzahl; // RLC-Kennung
					file.write (&Anzahl, 1);
					file.write (&Pixel, 1);
				}
				else
				{
					file.write (&Pixel, 1); // einmaliges Pixel mit Grauwert<192
				}
			}
		}

		//write color table
		unsigned char Pixel = 12; //Kennzeichen f. Bild-Ende
		file.write (&Pixel, 1);

		for (auto color : colors)
		{
			const unsigned char r = (color >> 16) & 0xFF;
			const unsigned char g = (color >> 8) & 0xFF;
			const unsigned char b = (color >> 0) & 0xFF;
			file.write (&r, 1);
			file.write (&g, 1);
			file.write (&b, 1);
		}
	}
} // namespace

int savePCX (const SDL_Surface* surface, const std::filesystem::path& fileName)
{
	if (!surface)
		return 0;

	if (surface->format->BitsPerPixel == 8) // palette color
	{
		savePCX_8bpp (*surface, fileName);
		return 1;
	}
	else if (surface->format->BitsPerPixel == 32) // rgb
	{
		savePCX_32bpp (*surface, fileName);
		return 1;
	}

	return 0;
}

SDL_Surface* loadPCX (const std::filesystem::path& name)
{
	//open file
	SDL_RWops* file = SDL_RWFromFile (name.string().c_str(), "rb");

	if (file == nullptr)
	{
		throw InstallException (std::string ("Couldn't open file ") + name.string() + TEXT_FILE_LF);
	}

	//load data
	SDL_RWseek (file, 8, SEEK_SET);
	const Sint16 width = SDL_ReadLE16 (file) + 1;
	const Sint16 height = SDL_ReadLE16 (file) + 1;

	SDL_Surface* sf = SDL_CreateRGBSurface (SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0);
	SDL_SetColorKey (sf, SDL_TRUE, 0xFF00FF);

	Uint8* _ptr = static_cast<Uint8*> (sf->pixels);
	SDL_RWseek (file, 128, SEEK_SET);

	Sint32 x = 0, y = 0;
	do
	{
		Uint8 byte;
		SDL_RWread (file, &byte, 1, 1);
		if (byte > 191)
		{
			const Sint32 z = std::min<Sint32> (byte - 192, width - x);
			SDL_RWread (file, &byte, 1, 1);
			for (Sint32 j = 0; j < z; j++)
			{
				_ptr[x + y * sf->pitch] = byte;
				x++;
				if (x == width) break;
			}
		}
		else
		{
			_ptr[x + y * sf->pitch] = byte;
			x++;
		}
		if (x == width)
		{
			x = 0;
			y++;
		}
	} while (y != height);

	//load color table
	SDL_RWseek (file, -768, SEEK_END);

	SDL_Color colors[256];
	for (auto& color : colors)
	{
		SDL_RWread (file, &color.r, 1, 1);
		SDL_RWread (file, &color.g, 1, 1);
		SDL_RWread (file, &color.b, 1, 1);
	}
	SDL_SetPaletteColors (sf->format->palette, colors, 0, 256);

	SDL_RWclose (file);
	return sf;
}
