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

#include "utility/drawing.h"
#include "utility/color.h"
#include "utility/position.h"
#include "utility/box.h"
#include "autosurface.h"

//------------------------------------------------------------------------------
void drawPoint (SDL_Surface* surface, const cPosition& position, const cColor& color)
{
	SDL_Rect rect = {Sint16 (position.x ()), Sint16 (position.y ()), 1, 1};
	SDL_FillRect (surface, &rect, color.toMappedSdlRGBAColor (surface->format));
}

//------------------------------------------------------------------------------
void drawLine (SDL_Surface* s, const cPosition& start, const cPosition& end, const cColor& color)
{
	auto x0 = start.x ();
	auto x1 = end.x ();
	auto y0 = start.y ();
	auto y1 = end.y ();

	bool steep = abs (y1 - y0) > abs (x1 - x0);
	if (steep)
	{
		std::swap (x0, y0);
		std::swap (x1, y1);
	}
	if (x0 > x1)
	{
		std::swap (x0, x1);
		std::swap (y0, y1);
	}

	int dx = x1 - x0;
	int dy = abs (y1 - y0);
	int er = dx / 2;
	int ys = y0 < y1 ? 1 : -1;
	int y = y0;

	for (int x = x0; x < x1; x++)
	{
		if (steep) drawPoint (s, cPosition (y, x), color);
		else drawPoint (s, cPosition (x, y), color);
		er -= dy;
		if (er < 0)
		{
			y += ys;
			er += dx;
		}
	}
}

//------------------------------------------------------------------------------
void drawRectangle (SDL_Surface* surface, const cBox<cPosition>& rectangle, const cColor& color)
{
	const cPosition size = rectangle.getMaxCorner () - rectangle.getMinCorner ();

	SDL_Rect line_h = {rectangle.getMinCorner ().x (), rectangle.getMinCorner ().y (), size.x (), 1};

	const auto sdlColor = color.toMappedSdlRGBAColor (surface->format);

	SDL_FillRect (surface, &line_h, sdlColor);
	line_h.y += size.y () - 1;
	SDL_FillRect (surface, &line_h, sdlColor);
	SDL_Rect line_v = {rectangle.getMinCorner ().x (), rectangle.getMinCorner ().y (), 1, size.y ()};
	SDL_FillRect (surface, &line_v, sdlColor);
	line_v.x += size.x () - 1;
	SDL_FillRect (surface, &line_v, sdlColor);
}

//------------------------------------------------------------------------------
Uint32 getPixel (const SDL_Surface& surface, const cPosition& position)
{
	int bpp = surface.format->BytesPerPixel;

	Uint8* p = (Uint8*)surface.pixels + position.y () * surface.pitch + position.x () * bpp;

	switch (bpp)
	{
	case 1:
		return *p;
		break;

	case 2:
		return *(Uint16*)p;
		break;

	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		else
			return p[0] | p[1] << 8 | p[2] << 16;
		break;

	case 4:
		return *(Uint32*)p;
		break;

	default:
		return 0;
	}
}

//------------------------------------------------------------------------------
void putPixel (SDL_Surface& surface, const cPosition& position, Uint32 pixel)
{
	int bpp = surface.format->BytesPerPixel;
	Uint8* p = (Uint8*)surface.pixels + position.y () * surface.pitch + position.x () * bpp;

	switch (bpp)
	{
	case 1:
		*p = pixel;
		break;

	case 2:
		*(Uint16*)p = pixel;
		break;

	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		{
			p[0] = (pixel >> 16) & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = pixel & 0xff;
		}
		else
		{
			p[0] = pixel & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = (pixel >> 16) & 0xff;
		}
		break;

	case 4:
		*(Uint32*)p = pixel;
		break;
	}
}

//------------------------------------------------------------------------------
void replaceColor (SDL_Surface& surface, const cColor& sourceColor, const cColor& destinationColor)
{
	const auto srcMapped = sourceColor.toMappedSdlRGBAColor (surface.format);
	const auto destMapped = destinationColor.toMappedSdlRGBAColor (surface.format);

	Uint32 key;
	const auto hadKey = SDL_GetColorKey (&surface, &key) == 0;

	AutoSurface temp (SDL_ConvertSurface (&surface, surface.format, surface.flags));

	SDL_SetColorKey (temp.get(), SDL_TRUE, srcMapped);
	SDL_FillRect (&surface, nullptr, destMapped);
	SDL_BlitSurface (temp.get (), nullptr, &surface, nullptr);

	if (hadKey) SDL_SetColorKey (&surface, SDL_TRUE, key);
	else SDL_SetColorKey (&surface, SDL_FALSE, 0);

	// The following version is to slow...
	//
	//SDL_LockSurface (&surface);
	//for (int y = 0; y < surface.h; y++)
	//{
	//	for (int x = 0; x < surface.w; x++)
	//	{
	//		const cPosition position (x, y);
	//		if (getPixel (surface, position) == srcMapped)
	//		{
	//			putPixel (surface, position, destMapped);
	//		}
	//	}
	//}
	//SDL_UnlockSurface (&surface);
}