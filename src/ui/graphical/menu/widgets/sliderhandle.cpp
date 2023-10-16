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
#include "output/video/video.h"
#include "resources/uidata.h"
#include "ui/widgets/application.h"

#include <algorithm>

//------------------------------------------------------------------------------
cSliderHandle::cSliderHandle (const cPosition& position, eSliderHandleType sliderHandleType, eOrientationType orientation_) :
	cWidget (position),
	surface (nullptr),
	orientation (orientation_)
{
	minPosition = maxPosition = (orientation == eOrientationType::Horizontal ? getPosition().x() : getPosition().y());

	createSurface (sliderHandleType);
}

//------------------------------------------------------------------------------
void cSliderHandle::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (surface != nullptr)
	{
		auto positionRect = toSdlRect (getArea());
		SDL_BlitSurface (surface.get(), nullptr, &destination, &positionRect);
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
void cSliderHandle::createSurface (eSliderHandleType sliderHandleType)
{
	cPosition size;
	cPosition srcPoint;
	switch (sliderHandleType)
	{
		case eSliderHandleType::Horizontal:
			srcPoint = cPosition (218, 35);
			size = cPosition (14, 17);
			break;
		case eSliderHandleType::Vertical:
			srcPoint = cPosition (201, 35);
			size = cPosition (17, 14);
			break;
		case eSliderHandleType::HudZoom:
		{
			auto rect = cGraphicsData::getRect_Slider_HudZoom();
			srcPoint = cPosition (rect.x, rect.y);
			size = cPosition (rect.w - 1, rect.h - 1);
			break;
		}
		case eSliderHandleType::ModernHorizontal:
			srcPoint = cPosition (241, 59);
			size = cPosition (8, 16);
			break;
		case eSliderHandleType::ModernVertical:
			srcPoint = cPosition (224, 91);
			size = cPosition (16, 8);
			break;
	}
	const cBox<cPosition> src (srcPoint, srcPoint + size);
	auto srcRect = toSdlRect (src);

	surface = AutoSurface (SDL_CreateRGBSurface (0, size.x(), size.y(), Video.getColDepth(), 0, 0, 0, 0));
	SDL_FillRect (surface.get(), nullptr, 0xFF00FF);

	if (sliderHandleType == eSliderHandleType::HudZoom)
		SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get(), &srcRect, surface.get(), nullptr);
	else
		SDL_BlitSurface (GraphicsData.gfx_menu_stuff.get(), &srcRect, surface.get(), nullptr);
	SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);

	resize (size);
}

//------------------------------------------------------------------------------
bool cSliderHandle::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition&)
{
	if (!application.hasMouseFocus (*this)) return false;

	cPosition offset (0, 0);
	switch (orientation)
	{
		case eOrientationType::Horizontal:
			offset.x() = std::min (std::max (mouse.getPosition().x() - grapOffset, minPosition), maxPosition) - getPosition().x();
			break;
		case eOrientationType::Vertical:
			offset.y() = std::min (std::max (mouse.getPosition().y() - grapOffset, minPosition), maxPosition) - getPosition().y();
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
