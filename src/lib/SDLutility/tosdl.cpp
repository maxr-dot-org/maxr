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

#include "tosdl.h"

#include "utility/box.h"
#include "utility/color.h"
#include "utility/position.h"

//------------------------------------------------------------------------------
SDL_Rect toSdlRect (const cBox<cPosition>& box)
{
	const auto& diff = box.getSize();

	return {box.getMinCorner()[0], box.getMinCorner()[1], diff[0], diff[1]};
}

//------------------------------------------------------------------------------
Uint32 toSdlColor (const cRgbColor& color, const SDL_PixelFormat* format)
{
	return SDL_MapRGB (format, color.r, color.g, color.b);
}

//------------------------------------------------------------------------------
Uint32 toSdlColor (const cRgbColor& color, const SDL_Surface& surface)
{
	return toSdlColor (color, surface.format);
}

//------------------------------------------------------------------------------
Uint32 toSdlAlphaColor (const cRgbColor& color, const SDL_PixelFormat* format)
{
	return SDL_MapRGBA (format, color.r, color.g, color.b, color.a);
}
//------------------------------------------------------------------------------
Uint32 toSdlAlphaColor (const cRgbColor& color, const SDL_Surface& surface)
{
	return toSdlAlphaColor (color, surface.format);
}
