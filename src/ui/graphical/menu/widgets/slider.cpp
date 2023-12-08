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

#include "ui/graphical/menu/widgets/slider.h"

#include "SDLutility/drawing.h"
#include "SDLutility/tosdl.h"
#include "input/mouse/mouse.h"
#include "output/video/video.h"
#include "resources/uidata.h"
#include "ui/graphical/menu/widgets/sliderhandle.h"
#include "utility/mathtools.h"

#include <algorithm>
#include <cassert>
#include <functional>

//------------------------------------------------------------------------------
cSlider::cSlider (const cBox<cPosition>& area, int minValue_, int maxValue_, eOrientationType orientation_, eSliderType sliderType) :
	cSlider (area,
             minValue_,
             maxValue_,
             orientation_,
             orientation == eOrientationType::Horizontal ? eSliderHandleType::Horizontal : eSliderHandleType::Vertical,
             sliderType)
{
}

//------------------------------------------------------------------------------
cSlider::cSlider (const cBox<cPosition>& area, int minValue_, int maxValue_, eOrientationType orientation_, eSliderHandleType handleType, eSliderType sliderType) :
	cClickableWidget (area),
	surface (nullptr),
	type (sliderType),
	currentValue (minValue_),
	minValue (minValue_),
	maxValue (maxValue_),
	orientation (orientation_)
{
	assert (minValue <= maxValue);

	createHandle (handleType);

	createSurface (sliderType);
}

//------------------------------------------------------------------------------
void cSlider::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (surface != nullptr)
	{
		auto positionRect = toSdlRect (getArea());
		SDL_BlitSurface (surface.get(), nullptr, &destination, &positionRect);
	}
	cClickableWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
void cSlider::handleMoved (const cPosition& offset)
{
	setHandleMinMaxPosition();

	cWidget::handleMoved (offset);
}

//------------------------------------------------------------------------------
void cSlider::setMinValue (int minValue_)
{
	auto oldValue = getValue();

	minValue = minValue_;

	setValue (oldValue);
}

//------------------------------------------------------------------------------
void cSlider::setMaxValue (int maxValue_)
{
	auto oldValue = getValue();

	maxValue = maxValue_;

	setValue (oldValue);
}

//------------------------------------------------------------------------------
int cSlider::getValueFromHandlePosition() const
{
	int minPosition, maxPosition;
	computeHandleMinMaxPosition (minPosition, maxPosition);

	const auto valueDiff = maxValue - minValue;
	const auto positionDiff = maxPosition - minPosition;

	const auto handlePosition = (orientation == eOrientationType::Horizontal ? handle->getPosition().x() : handle->getPosition().y());

	return positionDiff != 0 ? minValue + Round ((float) (handlePosition - minPosition) * valueDiff / positionDiff) : minValue;
}

//------------------------------------------------------------------------------
void cSlider::setValue (int value)
{
	if (settingValue) return; // prevent recursive endless loop

	// block for scoped operation
	{
		settingValue = true;
		auto reseter = makeScopedOperation ([&]() { settingValue = false; });

		value = std::max (value, minValue);
		value = std::min (value, maxValue);

		std::swap (value, currentValue);

		int minPosition, maxPosition;
		computeHandleMinMaxPosition (minPosition, maxPosition);

		const auto valueDiff = maxValue - minValue;
		const auto positionDiff = maxPosition - minPosition;

		const auto newPosition = valueDiff != 0 ? minPosition + (currentValue - minValue) * positionDiff / valueDiff : minPosition;

		cPosition position = handle->getPosition();
		(orientation == eOrientationType::Horizontal ? position.x() : position.y()) = newPosition;
		handle->moveTo (position);
	}

	if (value != currentValue) valueChanged();
}

//------------------------------------------------------------------------------
void cSlider::increase (int offset)
{
	setValue (getValue() + offset);
}

//------------------------------------------------------------------------------
void cSlider::decrease (int offset)
{
	setValue (getValue() - offset);
}

//------------------------------------------------------------------------------
void cSlider::createSurface (eSliderType sliderType)
{
	if (sliderType == eSliderType::Default)
	{
		assert (handle != nullptr);

		assert (orientation == eOrientationType::Horizontal); // We do not have graphics for vertical scroll bar yet!

		const auto offset = handle->getSize().x() / 2;

		auto size = getSize();

		//if (size.x() < 6)
		//{
		//	size.x() = 6;
		//}

		surface = AutoSurface (SDL_CreateRGBSurface (0, size.x(), size.y(), Video.getColDepth(), 0, 0, 0, 0));
		SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);
		SDL_FillRect (surface.get(), nullptr, 0xFF00FF);

		size.x() -= offset * 2;

		SDL_Rect sourceBegin = {201, 53, 3, 3};
		SDL_Rect sourceEnd = {259 - 3, 53, 3, 3};
		SDL_Rect sourcePart = {201 + sourceBegin.w, 53, /*259 - 201 - sourceBegin.w - sourceEnd.w*/ 10, 3};

		SDL_Rect destination = {offset, size.y() / 2 - (sourceBegin.h / 2), sourceBegin.w, sourceBegin.h};
		SDL_BlitSurface (GraphicsData.gfx_menu_stuff.get(), &sourceBegin, surface.get(), &destination);

		SDL_Rect destinationEnd = {offset + size.x() - sourceEnd.w, size.y() / 2 - (sourceEnd.h / 2), sourceEnd.w, sourceEnd.h};
		SDL_BlitSurface (GraphicsData.gfx_menu_stuff.get(), &sourceEnd, surface.get(), &destinationEnd);

		SDL_Rect destinationPart = {offset + sourceBegin.w, size.y() / 2 - (sourcePart.h / 2), sourcePart.w, sourcePart.h};
		while (destinationPart.x < destinationEnd.x)
		{
			if (destinationPart.x + sourcePart.w > destinationEnd.x) sourcePart.w = destinationEnd.x - destinationPart.x;
			SDL_BlitSurface (GraphicsData.gfx_menu_stuff.get(), &sourcePart, surface.get(), &destinationPart);
			destinationPart.x += sourcePart.w;
		}
	}
	else if (sliderType == eSliderType::DrawnBackground)
	{
		auto size = getSize();

		surface = AutoSurface (SDL_CreateRGBSurface (0, size.x(), size.y(), Video.getColDepth(), 0, 0, 0, 0));
		SDL_FillRect (surface.get(), nullptr, toSdlAlphaColor (cRgbColor::black(), *surface));

		drawLine (*surface, cPosition (0, 0), cPosition (0, size.y()), cRgbColor (140, 102, 61));
		drawLine (*surface, cPosition (size.x() - 1, 0), cPosition (size.x() - 1, size.y()), cRgbColor (140, 102, 61));
	}
}

//------------------------------------------------------------------------------
void cSlider::createHandle (eSliderHandleType handleType)
{
	if (handle) return;

	handle = emplaceChild<cSliderHandle> (getPosition(), handleType, orientation);

	setHandleMinMaxPosition();

	signalConnectionManager.connect (handle->moved, [this]() { movedHandle(); });
}

//------------------------------------------------------------------------------
void cSlider::setHandleMinMaxPosition()
{
	if (!handle) return;

	int minPosition, maxPosition;
	computeHandleMinMaxPosition (minPosition, maxPosition);

	handle->setMinMaxPosition (minPosition, maxPosition);
}

//------------------------------------------------------------------------------
bool cSlider::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (!handle) return false;

	const auto mousePosition = (orientation == eOrientationType::Horizontal ? mouse.getPosition().x() : mouse.getPosition().y());

	auto newHandlePosition = mousePosition - (orientation == eOrientationType::Horizontal ? handle->getSize().x() / 2 : handle->getSize().y() / 2);

	int minPosition, maxPosition;
	computeHandleMinMaxPosition (minPosition, maxPosition);

	newHandlePosition = std::max (std::min (newHandlePosition, maxPosition), minPosition);

	cPosition position = handle->getPosition();
	(orientation == eOrientationType::Horizontal ? position.x() : position.y()) = newHandlePosition;
	handle->moveTo (position);

	return true;
}

//------------------------------------------------------------------------------
void cSlider::computeHandleMinMaxPosition (int& minPosition, int& maxPosition) const
{
	minPosition = (orientation == eOrientationType::Horizontal ? getPosition().x() : getPosition().y());
	maxPosition = (orientation == eOrientationType::Horizontal ? getEndPosition().x() - handle->getSize().x() : getEndPosition().y() - handle->getSize().y());
	if (type != eSliderType::Default)
	{
		maxPosition += 2;
	}
}

//------------------------------------------------------------------------------
void cSlider::movedHandle()
{
	setValue (getValueFromHandlePosition());
}
