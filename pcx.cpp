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
#include "pcx.h"
#include "log.h"
#include "files.h"
#include "settings.h"
#include "main.h"

SDL_Surface* LoadPCX(std::string name)
{
	// Open the file.
	if (!FileExists(name.c_str()))
	{ // File not found, create empty surface.
		SDL_Surface* const s = SDL_CreateRGBSurface(OtherData.iSurface, 100, 20, SettingsData.iColourDepth, 0, 0, 0, 0);
		return s;
	}

	SDL_RWops* const f = SDL_RWFromFile(name.c_str(), "rb");
	if (!f)
	{
		Log.write(SDL_GetError(), cLog::eLOG_TYPE_WARNING); // Image corrupted, create empty surface.
		SDL_Surface* const s = SDL_CreateRGBSurface(OtherData.iSurface, 100, 20, SettingsData.iColourDepth, 0, 0, 0, 0);
		return s;
	}

	// Load the image.
	SDL_RWseek(f, 8, SEEK_SET);
	Uint16       const x = SDL_ReadLE16(f) + 1;
	Uint16       const y = SDL_ReadLE16(f) + 1;
	SDL_Surface* const s = SDL_CreateRGBSurface(OtherData.iSurface | SDL_SRCCOLORKEY, x, y, 32, 0, 0, 0, 0);
	if (!s)
	{
		Log.write(SDL_GetError(), cLog::eLOG_TYPE_ERROR);
		SDL_RWclose(f);
		return NULL; //app will crash using this
	}
	SDL_SetColorKey(s, SDL_SRCCOLORKEY, 0xFF00FF);

	Uint32* const buf = static_cast<Uint32*>(s->pixels);
	SDL_RWseek(f, 128, RW_SEEK_SET);
	int k = 0;
	int i = 0;
	do
	{
		char tmp[2];
		SDL_RWread(f, tmp, 1, 1);
		unsigned char const temp = tmp[0];
		if (temp >= 0xC0)
		{
			int z = temp - 192;
			if (z + k > x)
				z = x - k;
			SDL_RWread(f, tmp, 1, 1);
			unsigned char const temp = tmp[0];
			for (int j = 0; j < z; ++j)
			{
				buf[k + i * x] = temp;
				++k;
				if (k == x) break;
			}
		}
		else
		{
			buf[k + i * x] = temp;
			++k;
		}
		if (k == x)
		{
			k = 0;
			++i;
		}
	}
	while (i != y);

	// Convert from palette to true colour.
	int colors[256];
	SDL_RWseek(f, -768, SEEK_END);
	for (int i = 0; i != 256; ++i)
	{
		Uint8 rgb[3];
		SDL_RWread(f, rgb, sizeof(rgb), 1);
		colors[i] = SDL_MapRGB(s->format, rgb[0], rgb[1], rgb[2]);
	}
	for (int i = 0; i != x * y; ++i)
	{
		buf[i] = colors[buf[i]];
	}
	SDL_RWclose(f);
	return s;
}
