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

#include "drawing.h"
#include "color.h"
#include "position.h"
#include "box.h"

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