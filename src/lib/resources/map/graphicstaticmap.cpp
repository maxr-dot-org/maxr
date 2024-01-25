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

#include "graphicstaticmap.h"

#include "game/data/map/map.h"
#include "output/video/video.h"
#include "utility/log.h"

#include <string>

#if 1 // TODO: [SDL2]: SDL_SetColors
//------------------------------------------------------------------------------
inline void SDL_SetColors (SDL_Surface* surface, const SDL_Color* colors, int index, int size)
{
	SDL_SetPaletteColors (surface->format->palette, colors, index, size);
}
#endif

//------------------------------------------------------------------------------
void sGraphicTile::copySrfToTerData (SDL_Surface& surface, const SDL_Color (&palette_shw)[256])
{
	//before the surfaces are copied, the colortable of both surfaces has to be equal
	//This is needed to make sure, that the pixeldata is copied 1:1

	//copy the normal terrain
	sf_org = UniqueSurface (SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0));
	SDL_SetPaletteColors (sf_org->format->palette, surface.format->palette->colors, 0, 256);
	SDL_BlitSurface (&surface, nullptr, sf_org.get(), nullptr);

	sf = UniqueSurface (SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0));
	SDL_SetPaletteColors (sf->format->palette, surface.format->palette->colors, 0, 256);
	SDL_BlitSurface (&surface, nullptr, sf.get(), nullptr);

	//copy the terrains with fog
	shw_org = UniqueSurface (SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0));
	SDL_SetColors (shw_org.get(), surface.format->palette->colors, 0, 256);
	SDL_BlitSurface (&surface, nullptr, shw_org.get(), nullptr);

	shw = UniqueSurface (SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0));
	SDL_SetColors (shw.get(), surface.format->palette->colors, 0, 256);
	SDL_BlitSurface (&surface, nullptr, shw.get(), nullptr);

	//now set the palette for the fog terrains
	SDL_SetColors (shw_org.get(), palette_shw, 0, 256);
	SDL_SetColors (shw.get(), palette_shw, 0, 256);
}

//------------------------------------------------------------------------------
void cGraphicStaticMap::loadPalette (SDL_RWops* fpMapFile, std::size_t paletteOffset, std::size_t numberOfTerrains)
{
	tiles.resize (numberOfTerrains);
	// Load Color Palette
	SDL_RWseek (fpMapFile, paletteOffset, SEEK_SET);
	for (int i = 0; i < 256; i++)
	{
		SDL_RWread (fpMapFile, palette + i, 3, 1);
	}
	//generate palette for terrains with fog
	for (int i = 0; i < 256; i++)
	{
		palette_shw[i].r = (unsigned char) (palette[i].r * 0.6f);
		palette_shw[i].g = (unsigned char) (palette[i].g * 0.6f);
		palette_shw[i].b = (unsigned char) (palette[i].b * 0.6f);
		palette[i].a = 255;
		palette_shw[i].a = 255;
	}
}

//------------------------------------------------------------------------------
/*static*/ UniqueSurface cGraphicStaticMap::loadTerrGraph (SDL_RWops* fpMapFile, Sint64 iGraphicsPos, const SDL_Color (&colors)[256], int iNum)
{
	// Create new surface and copy palette
	UniqueSurface surface (SDL_CreateRGBSurface (0, 64, 64, 8, 0, 0, 0, 0));
	surface->pitch = surface->w;

	SDL_SetPaletteColors (surface->format->palette, colors, 0, 256);

	// Go to position of filedata
	SDL_RWseek (fpMapFile, iGraphicsPos + 64 * 64 * (iNum), SEEK_SET);

	// Read pixel data and write to surface
	if (SDL_RWread (fpMapFile, surface->pixels, 1, 64 * 64) != 64 * 64) return 0;
	return surface;
}

//------------------------------------------------------------------------------
bool cGraphicStaticMap::loadTile (SDL_RWops* fpMapFile, std::size_t graphicOffset, std::size_t index)
{
	UniqueSurface surface (loadTerrGraph (fpMapFile, graphicOffset, palette, index));
	if (surface == nullptr)
	{
		Log.warn ("EOF while loading terrain number " + std::to_string (index));
		SDL_RWclose (fpMapFile);
		tiles.clear();
		return false;
	}
	tiles[index].copySrfToTerData (*surface, palette_shw);
	return true;
}

//------------------------------------------------------------------------------
void cGraphicStaticMap::generateNextAnimationFrame()
{
	//change palettes to display next frame
	SDL_Color temp = palette[127];
	memmove (palette + 97, palette + 96, 32 * sizeof (SDL_Color));
	palette[96] = palette[103];
	palette[103] = palette[110];
	palette[110] = palette[117];
	palette[117] = palette[123];
	palette[123] = temp;

	temp = palette_shw[127];
	memmove (palette_shw + 97, palette_shw + 96, 32 * sizeof (SDL_Color));
	palette_shw[96] = palette_shw[103];
	palette_shw[103] = palette_shw[110];
	palette_shw[110] = palette_shw[117];
	palette_shw[117] = palette_shw[123];
	palette_shw[123] = temp;

	//set the new palette for all terrain surfaces
	for (auto& terrain : tiles)
	{
		SDL_SetColors (terrain.sf.get(), palette + 96, 96, 127);
		//SDL_SetColors (TerrainInUse[i]->sf_org, palette + 96, 96, 127);
		SDL_SetColors (terrain.shw.get(), palette_shw + 96, 96, 127);
		//SDL_SetColors (TerrainInUse[i]->shw_org, palette_shw + 96, 96, 127);
	}
}

//------------------------------------------------------------------------------
UniqueSurface cGraphicStaticMap::createBigSurface (int sizex, int sizey) const
{
	UniqueSurface mapSurface (SDL_CreateRGBSurface (0, sizex, sizey, Video.getColDepth(), 0, 0, 0, 0));

	const auto size = map->getSize().x();
	if (SDL_MUSTLOCK (mapSurface.get())) SDL_LockSurface (mapSurface.get());
	for (int x = 0; x < mapSurface->w; ++x)
	{
		const int terrainx = std::min ((x * size) / mapSurface->w, size - 1);
		const int offsetx = ((x * size) % mapSurface->w) * 64 / mapSurface->w;

		for (int y = 0; y < mapSurface->h; y++)
		{
			const int terrainy = std::min ((y * size) / mapSurface->h, size - 1);
			const int offsety = ((y * size) % mapSurface->h) * 64 / mapSurface->h;

			const sGraphicTile& t = getTile (map->getTileIndex (cPosition (terrainx, terrainy)));
			unsigned int ColorNr = *(static_cast<const unsigned char*> (t.sf_org->pixels) + (offsetx + offsety * 64));

			unsigned char* pixel = reinterpret_cast<unsigned char*> (&static_cast<Uint32*> (mapSurface->pixels)[x + y * mapSurface->w]);
			pixel[0] = palette[ColorNr].b;
			pixel[1] = palette[ColorNr].g;
			pixel[2] = palette[ColorNr].r;
		}
	}
	if (SDL_MUSTLOCK (mapSurface.get())) SDL_UnlockSurface (mapSurface.get());
	return mapSurface;
}
