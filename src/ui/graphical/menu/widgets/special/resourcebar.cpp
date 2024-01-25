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

#include "input/mouse/mouse.h"
#include "output/sound/soundchannel.h"
#include "output/sound/sounddevice.h"
#include "output/video/video.h"
#include "resources/uidata.h"
#include "utility/mathtools.h"

#include <cassert>

namespace
{
	//--------------------------------------------------------------------------
	SDL_Surface* getResourceHorizontalBarSurface(eResourceBarType type)
	{
		switch (type)
		{
			default:
			case eResourceBarType::Metal:
				return GraphicsData.gfx_horizontal_bar_metal.get();
			case eResourceBarType::Oil:
				return GraphicsData.gfx_horizontal_bar_oil.get();
			case eResourceBarType::Gold:
				return GraphicsData.gfx_horizontal_bar_gold.get();
			case eResourceBarType::Blocked:
				return GraphicsData.gfx_horizontal_bar_blocked.get();
			case eResourceBarType::MetalSlim:
				return GraphicsData.gfx_horizontal_bar_slim_metal.get();
			case eResourceBarType::OilSlim:
				return GraphicsData.gfx_horizontal_bar_slim_oil.get();
			case eResourceBarType::GoldSlim:
				return GraphicsData.gfx_horizontal_bar_slim_gold.get();
		}
	}

	//--------------------------------------------------------------------------
	SDL_Surface* getResourceVerticalBarSurface (eResourceBarType type)
	{
		switch (type)
		{
			default:
			case eResourceBarType::Metal:
				return GraphicsData.gfx_vertical_bar_slim_metal.get();
			case eResourceBarType::Oil:
				return GraphicsData.gfx_vertical_bar_slim_oil.get();
			case eResourceBarType::Gold:
				return GraphicsData.gfx_vertical_bar_slim_gold.get();
		}
	}

	//--------------------------------------------------------------------------
	SDL_Surface* getResourceBarSurface (eOrientationType orientation, eResourceBarType type)
	{
		switch (orientation)
		{
			default:
			case eOrientationType::Horizontal: return getResourceHorizontalBarSurface (type);
			case eOrientationType::Vertical: return getResourceVerticalBarSurface (type);
		}
	}

} // namespace

//------------------------------------------------------------------------------
cResourceBar::cResourceBar (const cBox<cPosition>& area, int minValue_, int maxValue_, eResourceBarType type, eOrientationType orientation_, cSoundChunk* clickSound_) :
	cClickableWidget (area),
	surface (getResourceBarSurface(orientation_, type)),
	clickSound (clickSound_),
	orientation (orientation_),
	additionalArea (orientation == eOrientationType::Horizontal ? 8 : 0, orientation == eOrientationType::Vertical ? 8 : 0),
	minValue (minValue_),
	maxValue (maxValue_),
	currentValue (maxValue_),
	fixedMinValue (minValue_),
	fixedMaxValue (maxValue_)
{
	assert (minValue <= maxValue);
	assert (minValue >= 0 && maxValue >= 0);

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
		dest.x = getPosition().x() + additionalArea.x();
		dest.y = getPosition().y() + (horizontal ? 0 : surface->h - src.h) + additionalArea.y();

		if (inverted && horizontal)
		{
			dest.x += surface->w - src.w;
			src.x = 0;
		}

		SDL_BlitSurface (surface, &src, &destination, &dest);
	}

	cClickableWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
void cResourceBar::setType (eResourceBarType type)
{
	surface = getResourceBarSurface (orientation, type);
}

//------------------------------------------------------------------------------
void cResourceBar::setStepSize (int stepSize_)
{
	stepSize = stepSize_;
	setValue (getValue());
}

//------------------------------------------------------------------------------
int cResourceBar::getMinValue() const
{
	return minValue;
}

//------------------------------------------------------------------------------
void cResourceBar::setMinValue (int minValue_)
{
	minValue = minValue_;
	setValue (getValue());
}

//------------------------------------------------------------------------------
int cResourceBar::getMaxValue() const
{
	return maxValue;
}

//------------------------------------------------------------------------------
void cResourceBar::setMaxValue (int maxValue_)
{
	maxValue = maxValue_;
	setValue (getValue());
}

//------------------------------------------------------------------------------
int cResourceBar::getFixedMinValue() const
{
	return fixedMinEnabled ? fixedMinValue : minValue;
}

//------------------------------------------------------------------------------
void cResourceBar::setFixedMinValue (int fixedMinValue_)
{
	fixedMinValue = std::max (fixedMinValue_, minValue);
	fixedMinEnabled = true;
	setValue (getValue());
}

//------------------------------------------------------------------------------
int cResourceBar::getFixedMaxValue() const
{
	return fixedMaxEnabled ? fixedMaxValue : maxValue;
}

//------------------------------------------------------------------------------
void cResourceBar::setFixedMaxValue (int fixedMaxValue_)
{
	fixedMaxValue = std::min (fixedMaxValue_, maxValue);
	fixedMaxEnabled = true;
	setValue (getValue());
}

//------------------------------------------------------------------------------
bool cResourceBar::isInverted() const
{
	return inverted;
}

//------------------------------------------------------------------------------
void cResourceBar::setInverted (bool inverted_)
{
	inverted = inverted_;
}

//------------------------------------------------------------------------------
int cResourceBar::getValue() const
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
		value = Round ((float) value / stepSize) * stepSize;
	}

	value = std::max (getFixedMinValue(), value);
	value = std::min (getFixedMaxValue(), value);

	if (value != currentValue)
	{
		currentValue = value;
		valueChanged();
	}
}

//------------------------------------------------------------------------------
void cResourceBar::increase (int offset)
{
	setValue (getValue() + offset);
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
		if (clickSound) cSoundDevice::getInstance().playSoundEffect (*clickSound);

		const auto valueRange = maxValue - minValue;
		switch (orientation)
		{
			default:
			case eOrientationType::Horizontal:
				setValue (minValue + (mouse.getPosition().x() - (getPosition().x() + additionalArea.x())) * valueRange / (getSize().x() - additionalArea.x() * 2));
				break;
			case eOrientationType::Vertical:
				setValue (minValue + valueRange - (mouse.getPosition().y() - (getPosition().y() + additionalArea.y())) * valueRange / (getSize().y() - additionalArea.y() * 2));
				break;
		}

		return true;
	}
	return false;
}
