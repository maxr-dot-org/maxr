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

#include "input/mouse/cursor/mousecursoramount.h"

#include "output/video/video.h"
#include "resources/uidata.h"
#include "utility/position.h"

//------------------------------------------------------------------------------
cMouseCursorAmount::cMouseCursorAmount (eMouseCursorAmountType type_) :
	type (type_),
	percent (-1)
{}

//------------------------------------------------------------------------------
cMouseCursorAmount::cMouseCursorAmount (eMouseCursorAmountType type_, int percent_) :
	type (type_),
	percent (percent_)
{}

//------------------------------------------------------------------------------
SDL_Surface* cMouseCursorAmount::getSurface() const
{
	if (surface == nullptr) generateSurface();
	return surface.get();
}

//------------------------------------------------------------------------------
cPosition cMouseCursorAmount::getHotPoint() const
{
	return cPosition (19, 19);
}

//------------------------------------------------------------------------------
bool cMouseCursorAmount::equal (const cMouseCursor& other) const
{
	auto other2 = dynamic_cast<const cMouseCursorAmount*> (&other);
	return other2 && other2->type == type && other2->percent == percent;
}

//------------------------------------------------------------------------------
void cMouseCursorAmount::generateSurface() const
{
	auto sourceSurface = getSourceSurface();
	surface = UniqueSurface (SDL_CreateRGBSurface (0, sourceSurface->w, sourceSurface->h, Video.getColDepth(), 0, 0, 0, 0));

	SDL_FillRect (surface.get(), nullptr, 0xFF00FF);
	SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);

	SDL_BlitSurface (sourceSurface, nullptr, surface.get(), nullptr);

	const int barWidth = 35;

	SDL_Rect rect = {1, 29, barWidth, 3};

	if (percent < 0 || percent > 100)
	{
		SDL_FillRect (sourceSurface, &rect, 0);
	}
	else
	{
		SDL_FillRect (surface.get(), &rect, 0x00FF0000);
		rect.w = static_cast<int> (percent / 100. * barWidth);
		SDL_FillRect (surface.get(), &rect, 0x0000FF00);
	}
}

//------------------------------------------------------------------------------
SDL_Surface* cMouseCursorAmount::getSourceSurface() const
{
	switch (type)
	{
		default:
		case eMouseCursorAmountType::Steal:
			return GraphicsData.gfx_Csteal.get();
		case eMouseCursorAmountType::Disable:
			return GraphicsData.gfx_Cdisable.get();
	}
}
