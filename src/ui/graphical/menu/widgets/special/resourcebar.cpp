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

#include "ui/graphical/menu/widgets/special/resourcebar.h"

#include "main.h"
#include "settings.h"
#include "video.h"
#include "input/mouse/mouse.h"
#include "output/sound/sounddevice.h"
#include "output/sound/soundchannel.h"

//------------------------------------------------------------------------------
cResourceBar::cResourceBar (const cBox<cPosition>& area, int minValue_, int maxValue_, eResourceBarType type, eOrientationType orientation_, cSoundChunk* clickSound_) :
	cClickableWidget (area),
	clickSound (clickSound_),
	orientation (orientation_),
	additionalArea (orientation == eOrientationType::Horizontal ? 8 : 0, orientation == eOrientationType::Vertical ? 8 : 0),
	minValue (minValue_),
	maxValue (maxValue_),
	currentValue (maxValue_),
	fixedMinEnabled (false),
	fixedMinValue (minValue_),
	fixedMaxEnabled (false),
	fixedMaxValue (maxValue_),
	inverted (false),
	stepSize (1)
{
	assert (minValue <= maxValue);
	assert (minValue >= 0 && maxValue >= 0);

	createSurface (type);

	resize (getSize() + additionalArea * 2);
	move (additionalArea * -1);
}

//------------------------------------------------------------------------------
void cResourceBar::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (surface != nullptr && (maxValue - minValue) > 0)
	{
		const bool horizontal = orientation == eOrientationType::Horizontal;

		SDL_Rect src;
		src.h = horizontal ? surface->h : (currentValue * surface->h / maxValue);
		src.w = horizontal ? (currentValue * surface->w / maxValue) : surface->w;
		src.x = horizontal ? surface->w - src.w : 0;
		src.y = 0;

		SDL_Rect dest;
		dest.h = dest.w = 0;
		dest.x = getPosition ().x () + additionalArea.x();
		dest.y = getPosition ().y () + (horizontal ? 0 : surface->h - src.h) + additionalArea.y();

		if (inverted && horizontal)
		{
			dest.x += surface->w - src.w;
			src.x = 0;
		}

		SDL_BlitSurface (surface.get (), &src, &destination, &dest);
	}

	cClickableWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
void cResourceBar::setType (eResourceBarType type)
{
	createSurface (type);
}

//------------------------------------------------------------------------------
void cResourceBar::setStepSize (int stepSize_)
{
	stepSize = stepSize_;
	setValue (getValue ());
}

//------------------------------------------------------------------------------
int cResourceBar::getMinValue () const
{
	return minValue;
}

//------------------------------------------------------------------------------
void cResourceBar::setMinValue (int minValue_)
{
	minValue = minValue_;
	setValue (getValue ());
}

//------------------------------------------------------------------------------
int cResourceBar::getMaxValue () const
{
	return maxValue;
}

//------------------------------------------------------------------------------
void cResourceBar::setMaxValue (int maxValue_)
{
	maxValue = maxValue_;
	setValue (getValue ());
}

//------------------------------------------------------------------------------
int cResourceBar::getFixedMinValue () const
{
	return fixedMinEnabled ? fixedMinValue : minValue;
}

//------------------------------------------------------------------------------
void cResourceBar::setFixedMinValue (int fixedMinValue_)
{
	fixedMinValue = std::max (fixedMinValue_, minValue);
	fixedMinEnabled = true;
	setValue (getValue ());
}

//------------------------------------------------------------------------------
int cResourceBar::getFixedMaxValue () const
{
	return fixedMaxEnabled ? fixedMaxValue : maxValue;
}

//------------------------------------------------------------------------------
void cResourceBar::setFixedMaxValue (int fixedMaxValue_)
{
	fixedMaxValue = std::min (fixedMaxValue_, maxValue);
	fixedMaxEnabled = true;
	setValue (getValue ());
}

//------------------------------------------------------------------------------
bool cResourceBar::isInverted () const
{
	return inverted;
}

//------------------------------------------------------------------------------
void cResourceBar::setInverted (bool inverted_)
{
	inverted = inverted_;
}

//------------------------------------------------------------------------------
int cResourceBar::getValue () const
{
	return currentValue;
}

//------------------------------------------------------------------------------
void cResourceBar::setValue (int value)
{
	value = std::max (minValue, value);
	value = std::min (maxValue, value);

	if (value < (maxValue / stepSize) * stepSize && value % stepSize != 0)
	{
		value = Round ((float)value / stepSize) * stepSize;
	}

	value = std::max (getFixedMinValue (), value);
	value = std::min (getFixedMaxValue (), value);

	if (value != currentValue)
	{
		currentValue = value;
		valueChanged ();
	}
}

//------------------------------------------------------------------------------
void cResourceBar::increase (int offset)
{
	setValue (getValue () + offset);
}

//------------------------------------------------------------------------------
void cResourceBar::decrease (int offset)
{
	increase (-offset);
}

//------------------------------------------------------------------------------
bool cResourceBar::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left)
	{
        if (clickSound) cSoundDevice::getInstance ().playSoundEffect (*clickSound);

		const auto valueRange = maxValue - minValue;
		switch (orientation)
		{
		default:
		case eOrientationType::Horizontal:
			setValue (minValue + (mouse.getPosition ().x () - (getPosition ().x () + additionalArea.x ())) * valueRange / (getSize ().x () - additionalArea.x () * 2));
			break;
		case eOrientationType::Vertical:
			setValue (minValue + valueRange - (mouse.getPosition ().y () - (getPosition ().y () + additionalArea.y ())) * valueRange / (getSize ().y () - additionalArea.y () * 2));
			break;
		}

		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void cResourceBar::createSurface (eResourceBarType type)
{
	cPosition srcPosition;
	switch (orientation)
	{
	default:
	case eOrientationType::Horizontal:
		switch (type)
		{
		default:
		case eResourceBarType::Metal:
			srcPosition.x () = 156;
			srcPosition.y () = 339;
			break;
		case eResourceBarType::Oil:
			srcPosition.x () = 156;
			srcPosition.y () = 369;
			break;
		case eResourceBarType::Gold:
			srcPosition.x () = 156;
			srcPosition.y () = 400;
			break;
		case eResourceBarType::Blocked:
			srcPosition.x () = 156;
			srcPosition.y () = 307;
			break;
		case eResourceBarType::MetalSlim:
			srcPosition.x () = 156;
			srcPosition.y () = 256;
			break;
		case eResourceBarType::OilSlim:
			srcPosition.x () = 156;
			srcPosition.y () = 273;
			break;
		case eResourceBarType::GoldSlim:
			srcPosition.x () = 156;
			srcPosition.y () = 290;
			break;
		}
		break;
	case eOrientationType::Vertical:
		switch (type)
		{
		default:
		case eResourceBarType::Metal:
			srcPosition.x () = 135;
			srcPosition.y () = 336;
			break;
		case eResourceBarType::Oil:
			srcPosition.x () = 400;
			srcPosition.y () = 348;
			break;
		case eResourceBarType::Gold:
			srcPosition.x () = 114;
			srcPosition.y () = 336;
			break;
		}
		break;
	}

	cPosition size;
	switch (orientation)
	{
	default:
	case eOrientationType::Horizontal:
		switch (type)
		{
		default:
		case eResourceBarType::Metal:
		case eResourceBarType::Oil:
		case eResourceBarType::Gold:
		case eResourceBarType::Blocked:
			size.x () = 240;
			size.y () = 30;
			break;
		case eResourceBarType::MetalSlim:
		case eResourceBarType::OilSlim:
		case eResourceBarType::GoldSlim:
			size.x () = 223;
			size.y () = 16;
			break;
		}
		break;
	case eOrientationType::Vertical:
		size.x () = 20;
		size.y () = 115;
		break;
	}

    surface = AutoSurface (SDL_CreateRGBSurface (0, size.x (), size.y (), Video.getColDepth (), 0, 0, 0, 0));

	SDL_SetColorKey (surface.get (), SDL_TRUE, 0xFF00FF);
	SDL_FillRect (surface.get (), NULL, 0xFF00FF);

	SDL_Rect src = {srcPosition.x (), srcPosition.y (), size.x (), size.y ()};
	SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get (), &src, surface.get (), NULL);
}
