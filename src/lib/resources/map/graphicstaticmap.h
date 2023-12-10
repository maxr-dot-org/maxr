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

#ifndef resources_map_graphicstaticmapH
#define resources_map_graphicstaticmapH

#include "SDLutility/uniquesurface.h"

#include <vector>

struct sGraphicTile
{
	static const int tilePixelHeight = 64;
	static const int tilePixelWidth = 64;

	void copySrfToTerData (SDL_Surface&, const SDL_Color (&palette_shw)[256]);

	UniqueSurface sf; /** the scaled surface of the terrain */
	UniqueSurface sf_org; /** the original surface of the terrain */
	UniqueSurface shw; /** the scaled surface of the terrain in the fog */
	UniqueSurface shw_org; /** the original surface of the terrain in the fog */
};

class cStaticMap;
class cGraphicStaticMap
{
public:
	cGraphicStaticMap (const cStaticMap* map) :
		map (map) {}

	cGraphicStaticMap (const cGraphicStaticMap&) = delete;
	cGraphicStaticMap& operator= (const cGraphicStaticMap&) = delete;

	void loadPalette (SDL_RWops*, std::size_t paletteOffset, std::size_t numberOfTerrains);
	bool loadTile (SDL_RWops*, std::size_t graphicOffset, std::size_t index);

	const sGraphicTile& getTile (std::size_t index) const { return tiles[index]; }

	UniqueSurface createBigSurface (int sizex, int sizey) const;
	void generateNextAnimationFrame();

private:
	static UniqueSurface loadTerrGraph (SDL_RWops*, Sint64 iGraphicsPos, const SDL_Color (&colors)[256], int iNum);

private:
	const cStaticMap* map = nullptr;
	std::vector<sGraphicTile> tiles; // The different terrain graphics.
	SDL_Color palette[256]; // Palette with all Colors for the terrain graphics
	SDL_Color palette_shw[256];
};

#endif
