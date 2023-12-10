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

#ifndef resources_playercolorH
#define resources_playercolorH

#include "SDLutility/uniquesurface.h"
#include "utility/color.h"

#include <SDL.h>
#include <map>

class cPlayerColor
{
public:
	static const size_t predefinedColorsCount = 8;
	static const cRgbColor predefinedColors[predefinedColorsCount];
	static size_t findClosestPredefinedColor (const cRgbColor& color);

	static SDL_Surface* getTexture (const cRgbColor&);

private:
	static std::map<cRgbColor, UniqueSurface, sLessRgbColor> textures;
};

#endif
