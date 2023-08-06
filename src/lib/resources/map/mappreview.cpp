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

#include "mappreview.h"

#include "output/video/video.h"
#include "settings.h"

sMapPreview loadMapPreview (const std::filesystem::path& mapFilename)
{
	auto mapPath = cSettings::getInstance().getMapsPath() / mapFilename;
	// if no factory map of that name exists, try the custom user maps

	SDL_RWops* mapFile = SDL_RWFromFile (mapPath.u8string().c_str(), "rb");
	if (mapFile == nullptr && !cSettings::getInstance().getUserMapsDir().empty())
	{
		mapPath = cSettings::getInstance().getUserMapsDir() / mapFilename;
		mapFile = SDL_RWFromFile (mapPath.u8string().c_str(), "rb");
	}

	if (mapFile == nullptr) return {nullptr, {0, 0}};

	SDL_RWseek (mapFile, 5, SEEK_SET);
	const int size = SDL_ReadLE16 (mapFile);
	struct
	{
		unsigned char cBlue, cGreen, cRed;
	} Palette[256];
	SDL_RWseek (mapFile, 2 + size * size * 3, SEEK_CUR);
	short sGraphCount = SDL_ReadLE16 (mapFile);
	SDL_RWseek (mapFile, 64 * 64 * sGraphCount, SEEK_CUR);
	SDL_RWread (mapFile, &Palette, 3, 256);

	AutoSurface mapSurface (SDL_CreateRGBSurface (0, size, size, 8, 0, 0, 0, 0));
	mapSurface->pitch = mapSurface->w;

	mapSurface->format->palette->ncolors = 256;
	for (int j = 0; j < 256; j++)
	{
		mapSurface->format->palette->colors[j].r = Palette[j].cBlue;
		mapSurface->format->palette->colors[j].g = Palette[j].cGreen;
		mapSurface->format->palette->colors[j].b = Palette[j].cRed;
	}
	SDL_RWseek (mapFile, 9, SEEK_SET);
	const int byteReadCount = SDL_RWread (mapFile, mapSurface->pixels, 1, size * size);
	SDL_RWclose (mapFile);

	if (byteReadCount != size * size)
	{
		// error.
		return {nullptr, {0, 0}};
	}
	const int MAPWINSIZE = 112;
	if (mapSurface->w != MAPWINSIZE || mapSurface->h != MAPWINSIZE) // resize map
	{
		mapSurface = AutoSurface (scaleSurface (mapSurface.get(), nullptr, MAPWINSIZE, MAPWINSIZE));
	}

	return {std::move (mapSurface), {size, size}};
}
