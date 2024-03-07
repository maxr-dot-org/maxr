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

#include "SDLutility/drawing.h"
#include "output/sound/soundchannel.h"
#include "output/sound/sounddevice.h"
#include "output/video/video.h"

//------------------------------------------------------------------------------
cImage::cImage (const cPosition& position, SDL_Surface* image_, cSoundChunk* clickSound_) :
	cClickableWidget (position),
	clickSound (clickSound_)
{
	setImage (image_);
}

//------------------------------------------------------------------------------
void cImage::setImage (SDL_Surface* image_)
{
	if (image_ != nullptr)
	{
		image = UniqueSurface (SDL_CreateRGBSurface (0, image_->w, image_->h, Video.getColDepth(), 0, 0, 0, 0));

		SDL_FillRect (image.get(), nullptr, 0xFF00FF);
		SDL_SetColorKey (image.get(), SDL_TRUE, 0xFF00FF);

		SDL_BlitSurface (image_, nullptr, image.get(), nullptr);

		resize (cPosition (image->w, image->h));
	}
	else
	{
		image = nullptr;
		resize (cPosition (0, 0));
	}
}

//------------------------------------------------------------------------------
void cImage::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (image != nullptr)
	{
		blitClipped (*image, getArea(), destination, clipRect);
	}

	cClickableWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
void cImage::disableAtTransparent()
{
	disabledAtTransparent = true;
}

//------------------------------------------------------------------------------
void cImage::enableAtTransparent()
{
	disabledAtTransparent = false;
}

//------------------------------------------------------------------------------
bool cImage::isAt (const cPosition& position) const
{
	if (!cClickableWidget::isAt (position)) return false;

	if (!disabledAtTransparent) return true;

	auto color = getPixel (*image, position - getPosition());

	if (color == 0xFF00FF) return false;

	return true;
}

//------------------------------------------------------------------------------
bool cImage::handleClicked (cApplication&, cMouse&, eMouseButtonType button) /* override */
{
	if (button == eMouseButtonType::Left)
	{
		if (clickSound) cSoundDevice::getInstance().playSoundEffect (*clickSound);
		clicked();
		return true;
	}
	return false;
}
