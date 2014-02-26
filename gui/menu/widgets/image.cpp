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

#include "image.h"
#include "../../../settings.h"
#include "../../../video.h"

//------------------------------------------------------------------------------
cImage::cImage (const cPosition& position, SDL_Surface* image_, sSOUND* clickSound_) :
	cClickableWidget (position),
	clickSound (clickSound_),
	disabledAtTransparent (false)
{
	setImage (image_);
}

//------------------------------------------------------------------------------
void cImage::setImage (SDL_Surface* image_)
{
	if (image_ != nullptr)
	{
		image = SDL_CreateRGBSurface (0, image_->w, image_->h, Video.getColDepth (), 0, 0, 0, 0);

		SDL_FillRect (image, NULL, 0xFF00FF);
		SDL_SetColorKey (image, SDL_TRUE, 0xFF00FF);

		SDL_BlitSurface (image_, NULL, image, NULL);

		resize (cPosition (image->w, image->h));
	}
	else
	{
		image = nullptr;
		resize (cPosition (0, 0));
	}
}

//------------------------------------------------------------------------------
void cImage::draw ()
{
	if (image)
	{
		SDL_Rect position = getArea ().toSdlRect ();
		SDL_BlitSurface (image, NULL, cVideo::buffer, &position);
	}

	cClickableWidget::draw ();
}

//------------------------------------------------------------------------------
void cImage::disableAtTransparent ()
{
	disabledAtTransparent = true;
}

//------------------------------------------------------------------------------
void cImage::enableAtTransparent ()
{
	disabledAtTransparent = false;
}

namespace {

Uint32 getPixel (SDL_Surface *surface, const cPosition& position)
{
	int bpp = surface->format->BytesPerPixel;

	Uint8 *p = (Uint8 *)surface->pixels + position.y () * surface->pitch + position.x () * bpp;

	switch (bpp)
	{
	case 1:
		return *p;
		break;

	case 2:
		return *(Uint16 *)p;
		break;

	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		else
			return p[0] | p[1] << 8 | p[2] << 16;
		break;

	case 4:
		return *(Uint32 *)p;
		break;

	default:
		return 0;
	}
}

}

bool cImage::isAt (const cPosition& position) const
{
	if (!cClickableWidget::isAt (position)) return false;

	if (!disabledAtTransparent) return true;

	auto color = getPixel (image, position - getPosition ());

	if (color == 0xFF00FF) return false;

	return true;
}

//------------------------------------------------------------------------------
bool cImage::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left)
	{
		if (clickSound) PlayFX (clickSound);
		clicked ();
		return true;
	}
	return false;
}
