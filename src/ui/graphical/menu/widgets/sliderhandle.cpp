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

#include "ui/graphical/menu/widgets/sliderhandle.h"

#include "SDLutility/tosdl.h"
#include "input/mouse/mouse.h"
#include "resources/uidata.h"
#include "ui/widgets/application.h"

#include <algorithm>

namespace
{
	//--------------------------------------------------------------------------
	sPartialSurface getPartialSurface (eSliderHandleType sliderHandleType)
	{
		switch (sliderHandleType)
		{
			case eSliderHandleType::Horizontal: return {GraphicsData.gfx_menu_stuff.get(), {218, 35, 14, 17}};
			case eSliderHandleType::Vertical: return {GraphicsData.gfx_menu_stuff.get(), {201, 35, 17, 14}};
			case eSliderHandleType::HudZoom: return {GraphicsData.gfx_hud_stuff.get(), cGraphicsData::getRect_Slider_HudZoom()};
			case eSliderHandleType::ModernHorizontal: return {GraphicsData.gfx_menu_stuff.get(), {241, 59, 8, 16}};
			case eSliderHandleType::ModernVertical: return {GraphicsData.gfx_menu_stuff.get(), {224, 91, 16, 8}};
		}
		throw std::runtime_error ("Unknown enum eSliderHandleType: " + std::to_string (static_cast<int>(sliderHandleType)));
	}

}

//------------------------------------------------------------------------------
cSliderHandle::cSliderHandle (const cPosition& position, eSliderHandleType sliderHandleType, eOrientationType orientation_) :
	cWidget (position),
	partialSurface (getPartialSurface (sliderHandleType)),
	orientation (orientation_)
{
	minPosition = maxPosition = (orientation == eOrientationType::Horizontal ? getPosition().x() : getPosition().y());

	resize (cPosition (partialSurface.rect.w, partialSurface.rect.h));
}

//------------------------------------------------------------------------------
void cSliderHandle::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (partialSurface.surface != nullptr)
	{
		auto positionRect = toSdlRect (getArea());
		SDL_BlitSurface (partialSurface.surface, &partialSurface.rect, &destination, &positionRect);
	}

	cWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
void cSliderHandle::setMinMaxPosition (int minPosition_, int maxPosition_)
{
	minPosition = minPosition_;
	maxPosition = maxPosition_;
}

//------------------------------------------------------------------------------
bool cSliderHandle::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition&)
{
	if (!application.hasMouseFocus (*this)) return false;

	cPosition offset (0, 0);
	switch (orientation)
	{
		case eOrientationType::Horizontal:
			offset.x() = std::clamp (mouse.getPosition().x() - grapOffset, minPosition, maxPosition) - getPosition().x();
			break;
		case eOrientationType::Vertical:
			offset.y() = std::clamp (mouse.getPosition().y() - grapOffset, minPosition, maxPosition) - getPosition().y();
			break;
	}
	if (offset != 0)
	{
		move (offset);
	}
	return true;
}

//------------------------------------------------------------------------------
bool cSliderHandle::handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left)
	{
		switch (orientation)
		{
			case eOrientationType::Horizontal:
				grapOffset = mouse.getPosition().x() - getPosition().x();
				break;
			case eOrientationType::Vertical:
				grapOffset = mouse.getPosition().y() - getPosition().y();
				break;
		}
		application.grapMouseFocus (*this);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cSliderHandle::handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left && application.hasMouseFocus (*this))
	{
		application.releaseMouseFocus (*this);
	}
	return false;
}

//------------------------------------------------------------------------------
void cSliderHandle::handleMoved (const cPosition& offset)
{
	moved();
	cWidget::handleMoved (offset);
}
