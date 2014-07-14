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

#ifndef utility_drawingH
#define utility_drawingH

#include <SDL.h>

class cColor;
class cPosition;
template<typename> class cBox;

void drawPoint (SDL_Surface* surface, const cPosition& position, const cColor& color);

void drawLine (SDL_Surface* surface, const cPosition& start, const cPosition& end, const cColor& color);

void drawRectangle (SDL_Surface* surface, const cBox<cPosition>& rectangle, const cColor& color);

Uint32 getPixel (const SDL_Surface& surface, const cPosition& position);

void putPixel (SDL_Surface& surface, const cPosition& position, Uint32 pixel);

void replaceColor (SDL_Surface& surface, const cColor& sourceColor, const cColor& destinationColor);

#endif // utility_drawingH
