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

#include <SDL.h>
#include <string>
#include "converter.h"


int savePCX_8bpp(SDL_Surface* surface, string fileName)
{
	int Z_Index, S_Index;			// PCX-Größenangaben
	
	unsigned char* bild;			// Adresse des Bildspeichers (Array)
	unsigned char Pixel;
	unsigned char Anzahl;
	int i, z, s;
	long Index;						// Adresse des Pixels im Bild

	SDL_RWops* file = SDL_RWFromFile(fileName.c_str(), "wb");

	if (file == NULL)
	{
		return 0;
	}
	
	//PCX-Header erzeugen
	unsigned char PCXHeader[128];
	for ( i = 0; i < 128; i++ )
	{
		PCXHeader[i] = 0;
	}

	S_Index = surface->w - 1;
	Z_Index = surface->h - 1;

	PCXHeader[0] = 10;                           // Zsoft
	PCXHeader[1] = 5;                            // Version 5
	PCXHeader[2] = 1;                            // RLC
	PCXHeader[3] = 8;                            // Bits pro Pixel und Ebene
	PCXHeader[8] = S_Index-256*(S_Index/256);    // Xmax Low-Byte
	PCXHeader[9] = S_Index/256;                  // Xmax High-Byte
	PCXHeader[10] = Z_Index-256*(Z_Index/256);   // Ymax Low-Byte
	PCXHeader[11] = Z_Index/256;				 // Ymax High-Byte

	PCXHeader[65] = 1;                           // Anzahl der Ebenen
	PCXHeader[66] = S_Index+1-256*((S_Index+1)/256);     // Bytes/Zeile
	PCXHeader[67] = (S_Index+1)/256;
	PCXHeader[68] = 1;                           // Farbe
	
	SDL_RWwrite(file, PCXHeader, 128, 1);
	
	bild = (unsigned char*) surface->pixels;
	
	// RLC berechnen
	for (z = 0; z <= Z_Index; z++)
	{
		Index = (long)z * (long)surface->w;
		s = 0;
		while (s <= S_Index)
		{
			Pixel = bild[Index+s];      // erstes Vorkommen der Farbe
			s++;
			Anzahl=1;
			while ( (s <= S_Index) && (Pixel == bild[Index+s]) && (Anzahl < 63) )
			{
				Anzahl++;
				s++;
			}
			// Anzahl>1 oder Farbe>=192, dann RLC durchführen
			if ( (Anzahl > 1) || (Pixel >= 192) )
			{
				Anzahl = 0xC0 + Anzahl;     // RLC-Kennung
				SDL_RWwrite(file, &Anzahl, 1, 1);
				SDL_RWwrite(file, &Pixel, 1, 1);
			}
			else
			{
				SDL_RWwrite(file, &Pixel, 1, 1);    // einmaliges Pixel mit Grauwert<192
			}
		}
	}

	//write color table
	Pixel=12;					//Kennzeichen f. Bild-Ende
	SDL_RWwrite(file, &Pixel, 1, 1);
	
	for ( i = 0; i < 256 ; i++ )
	{
		SDL_Color* colors = surface->format->palette->colors;
		SDL_RWwrite(file, &colors[i].r, 1, 1);
		SDL_RWwrite(file, &colors[i].g, 1, 1);
		SDL_RWwrite(file, &colors[i].b, 1, 1);
	}

	SDL_RWclose(file);
	return 1;

}

int savePCX_32bpp(SDL_Surface* surface, string fileName)
{
	int Z_Index, S_Index;			// PCX-Größenangaben
	
	unsigned char* bild;			// Adresse des Bildspeichers (Array)
	unsigned char Pixel;
	unsigned char Anzahl;
	int i, j, z, s;
	long Index;						// Adresse des Pixels im Bild

	SDL_RWops* file = SDL_RWFromFile(fileName.c_str(), "wb");

	if (file == NULL)
	{
		return 0;
	}

	//PCX-Header erzeugen
	unsigned char PCXHeader[128];
	for ( i = 0; i < 128; i++ )
	{
		PCXHeader[i] = 0;
	}

	S_Index = surface->w - 1;
	Z_Index = surface->h - 1;

	PCXHeader[0] = 10;                           // Zsoft
	PCXHeader[1] = 5;                            // Version 5
	PCXHeader[2] = 1;                            // RLC
	PCXHeader[3] = 8;                            // Bits pro Pixel und Ebene
	PCXHeader[8] = S_Index-256*(S_Index/256);    // Xmax Low-Byte
	PCXHeader[9] = S_Index/256;                  // Xmax High-Byte
	PCXHeader[10] = Z_Index-256*(Z_Index/256);   // Ymax Low-Byte
	PCXHeader[11] = Z_Index/256;				 // Ymax High-Byte

	PCXHeader[65] = 1;                           // Anzahl der Ebenen
	PCXHeader[66] = S_Index+1-256*((S_Index+1)/256);     // Bytes/Zeile
	PCXHeader[67] = (S_Index+1)/256;
	PCXHeader[68] = 1;                           // Farbe

	
	SDL_RWwrite(file,PCXHeader, 128, 1);
	
	//build color table
	bild = (unsigned char*) malloc (surface->w * surface->h - 1);
	Uint32* surface_data = (Uint32*) surface->pixels;
	Uint32 colors[256];
	for ( i = 0; i < 256; i++)
	{
		colors[i] = 0;
	}
	int NrColors = 0;

	for ( Index = 0; Index < surface->h * surface->w - 1; Index++ )
	{
		//search color in table
		for ( j = 0; j < NrColors; j++ )
		{
			if (colors[j] == surface_data[Index])
			{
				bild[Index] = j;
				break;
			}
		}

		//add color, if its not in the table
		if ( j == NrColors )
		{
			if ( NrColors > 255 ) 
				return 0;			//to many colors, table full
			colors[NrColors] = surface_data[Index];
			bild[Index] = NrColors;
			NrColors++;
		}
	}

	// RLC berechnen
	for (z = 0; z <= Z_Index; z++)
	{
		Index = (long)z * (long)surface->w;
		s = 0;
		while (s <= S_Index)
		{
			Pixel = bild[Index+s];      // erstes Vorkommen der Farbe
			s++;
			Anzahl=1;
			while ( (s <= S_Index) && (Pixel == bild[Index+s]) && (Anzahl < 63) )
			{
				Anzahl++;
				s++;
			}
			// Anzahl>1 oder Farbe>=192, dann RLC durchführen
			if ( (Anzahl > 1) || (Pixel >= 192) )
			{
				Anzahl = 0xC0 + Anzahl;     // RLC-Kennung
				SDL_RWwrite(file, &Anzahl, 1, 1);
				SDL_RWwrite(file, &Pixel, 1, 1);
			}
			else
			{
				SDL_RWwrite(file, &Pixel, 1, 1);    // einmaliges Pixel mit Grauwert<192
			}
		}
	}

	//write color table
	Pixel=12;					//Kennzeichen f. Bild-Ende
	SDL_RWwrite(file, &Pixel, 1, 1);
	
	unsigned char temp;
	for ( i = 0; i < 256 ; i++ )
	{
		temp = (unsigned char) (colors[i]/65536);
		SDL_RWwrite(file, &temp, 1, 1);
		temp = (unsigned char) (colors[i]/256);
		SDL_RWwrite(file, &temp, 1, 1);
		temp = (unsigned char) colors[i];
		SDL_RWwrite(file, &temp, 1, 1);
	}
	
	SDL_RWclose(file);

	free ( bild );
	return 1;

}

int savePCX ( SDL_Surface* surface, string fileName )
{
	if ( !surface ) 
		return 0;

	if ( surface->format->BitsPerPixel == 8 )
	{
		cout << "saveing Image as 8 bpp\n";
		return savePCX_8bpp( surface, fileName );
	}
	else if ( surface->format->BitsPerPixel == 32 )
	{
		cout << "saveing Image as 32 bpp\n";
		return savePCX_32bpp( surface, fileName );
	}

	return 1;
}
