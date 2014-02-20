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

#include "resourcebar.h"

#include "../../../../main.h"
#include "../../../../settings.h"
#include "../../../../video.h"
#include "../../../../input/mouse/mouse.h"

//------------------------------------------------------------------------------
cResourceBar::cResourceBar (const cBox<cPosition>& area, int minValue_, int maxValue_, eResourceBarType type, eOrientationType orientation_, sSOUND* clickSound_) :
	cClickableWidget (area),
	clickSound (clickSound_),
	orientation (orientation_),
	additionalArea (orientation == eOrientationType::Horizontal ? 8 : 0, orientation == eOrientationType::Vertical ? 8 : 0),
	minValue (minValue_),
	maxValue (maxValue_),
	currentValue (maxValue_),
	stepSize (1)
{
	assert (minValue <= maxValue);
	assert (minValue >= 0 && maxValue >= 0);

	createSurface (type);

	resize (getSize() + additionalArea * 2);
	move (additionalArea * -1);
}

//------------------------------------------------------------------------------
void cResourceBar::draw ()
{
	if (surface)
	{
		const bool horizontal = orientation == eOrientationType::Horizontal;

		SDL_Rect src;
		src.h = horizontal ? surface->h : (int)((float)currentValue / maxValue * surface->h);
		src.w = horizontal ? (int)((float)currentValue / maxValue * surface->w) : surface->w;
		src.x = horizontal ? surface->w - src.w : 0;
		src.y = 0;

		SDL_Rect dest;
		dest.h = dest.w = 0;
		dest.x = getPosition ().x () + additionalArea.x();
		dest.y = getPosition ().y () + (horizontal ? 0 : surface->h - src.h) + additionalArea.y();

		if (/*inverted*/false && horizontal)
		{
			dest.x += surface->w - src.w;
			src.x = 0;
		}

		SDL_BlitSurface (surface, &src, cVideo::buffer, &dest);
	}

	cClickableWidget::draw ();
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
int cResourceBar::getValue () const
{
	return currentValue;
}

//------------------------------------------------------------------------------
void cResourceBar::setValue (int value)
{
	value = std::max (minValue, value);
	value = std::min (maxValue, value);

	if (value % stepSize != 0)
	{
		value = Round ((float)value / stepSize) * stepSize;
	}

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
		if (clickSound) PlayFX (clickSound);

		double factor;
		switch (orientation)
		{
		default:
		case eOrientationType::Horizontal:
			factor = (double)(mouse.getPosition ().x () - (getPosition ().x () + additionalArea.x ())) / (getSize ().x () - additionalArea.x () * 2);
			break;
		case eOrientationType::Vertical:
			factor = 1. - (double)(mouse.getPosition ().y () - (getPosition ().y () + additionalArea.y ())) / (getSize ().y () - additionalArea.y () * 2);
			break;
		}

		const auto valueRange = maxValue - minValue;
		setValue (minValue + (int)(factor * valueRange));

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

	surface = SDL_CreateRGBSurface (0, size.x (), size.y (), Video.getColDepth (), 0, 0, 0, 0);

	SDL_SetColorKey (surface, SDL_TRUE, 0xFF00FF);
	SDL_FillRect (surface, NULL, 0xFF00FF);

	SDL_Rect src = {srcPosition.x (), srcPosition.y (), size.x (), size.y ()};
	SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &src, surface, NULL);
}