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

#include <algorithm>
#include <functional>

#include "slider.h"
#include "sliderhandle.h"

#include "../../../settings.h"
#include "../../../video.h"
#include "../../../main.h"
#include "../../../input/mouse/mouse.h"

//------------------------------------------------------------------------------
cSlider::cSlider (const cBox<cPosition>& area, int minValue_, int maxValue_, eOrientationType orientation_) :
	cClickableWidget (area),
	surface (nullptr),
	minValue (minValue_),
	maxValue (maxValue_),
	orientation (orientation_),
	handle (nullptr)
{
	assert (minValue <= maxValue);

	createHandle (orientation == eOrientationType::Horizontal ? eSliderHandleType::Horizontal : eSliderHandleType::Vertical);

	createSurface ();
}

//------------------------------------------------------------------------------
cSlider::cSlider (const cBox<cPosition>& area, int minValue_, int maxValue_, eOrientationType orientation_, eSliderHandleType handleType) :
	cClickableWidget (area),
	surface (nullptr),
	minValue (minValue_),
	maxValue (maxValue_),
	orientation (orientation_),
	handle (nullptr)
{
	assert (minValue <= maxValue);

	createHandle (handleType);

	createSurface ();
}

//------------------------------------------------------------------------------
void cSlider::draw ()
{
	if (surface)
	{
		auto positionRect = getArea ().toSdlRect ();
		SDL_BlitSurface (surface, NULL, cVideo::buffer, &positionRect);
	}
	cClickableWidget::draw ();
}

//------------------------------------------------------------------------------
void cSlider::handleMoved (const cPosition& offset)
{
	setHandleMinMaxPosition ();

	cWidget::handleMoved (offset);
}

//------------------------------------------------------------------------------
int cSlider::getValue ()
{
	int minPosition, maxPosition;
	computeHandleMinMaxPosition (minPosition, maxPosition);

	const auto valueDiff = maxValue - minValue;
	const auto positionDiff = maxPosition - minPosition;

	const auto handlePosition = (orientation == eOrientationType::Horizontal ? handle->getPosition ().x () : handle->getPosition ().y ());

	const auto factor = (double)(handlePosition - minPosition) / positionDiff;

	return minValue + (int)(factor * valueDiff);
}

//------------------------------------------------------------------------------
void cSlider::setValue (int value)
{
	value = std::max (value, minValue);
	value = std::min (value, maxValue);

	int minPosition, maxPosition;
	computeHandleMinMaxPosition (minPosition, maxPosition);

	const auto valueDiff = maxValue - minValue;
	const auto positionDiff = maxPosition - minPosition;

	const auto factor = (double)(value - minValue) / valueDiff;
	const auto newPosition = minPosition + (int)(factor * positionDiff);

	cPosition position = handle->getPosition ();
	(orientation == eOrientationType::Horizontal ? position.x () : position.y ()) = newPosition;
	handle->moveTo (position);
}

//------------------------------------------------------------------------------
void cSlider::createSurface ()
{
	assert (handle != nullptr);

	assert (orientation == eOrientationType::Horizontal); // We do not have graphics for vertical scroll bar yet!
	
	const auto offset = handle->getSize ().x () / 2;

	auto size = getSize ();

	//if (size.x () < 6)
	//{
	//	size.x () = 6;
	//}

	surface = SDL_CreateRGBSurface (0, size.x (), size.y (), Video.getColDepth (), 0, 0, 0, 0);
	SDL_SetColorKey (surface, SDL_TRUE, 0xFF00FF);
	SDL_FillRect (surface, NULL, 0xFF00FF);

	size.x () -= offset * 2;

	SDL_Rect sourceBegin = {201, 53, 3, 3};
	SDL_Rect sourceEnd = {259 - 3, 53, 3, 3};
	SDL_Rect sourcePart = {201 + sourceBegin.w, 53, /*259 - 201 - sourceBegin.w - sourceEnd.w*/ 10, 3};

	SDL_Rect destination = {offset, size.y () / 2 - (sourceBegin.h / 2), sourceBegin.w, sourceBegin.h};
	SDL_BlitSurface (GraphicsData.gfx_menu_stuff, &sourceBegin, surface, &destination);

	SDL_Rect destinationEnd = {offset + size.x () - sourceEnd.w, size.y () / 2 - (sourceEnd.h / 2), sourceEnd.w, sourceEnd.h};
	SDL_BlitSurface (GraphicsData.gfx_menu_stuff, &sourceEnd, surface, &destinationEnd);

	SDL_Rect destinationPart = {offset + sourceBegin.w, size.y () / 2 - (sourcePart.h / 2), sourcePart.w, sourcePart.h};
	while (destinationPart.x < destinationEnd.x)
	{
		if (destinationPart.x + sourcePart.w > destinationEnd.x) sourcePart.w = destinationEnd.x - destinationPart.x;
		SDL_BlitSurface (GraphicsData.gfx_menu_stuff, &sourcePart, surface, &destinationPart);
		destinationPart.x += sourcePart.w;
	}
}

//------------------------------------------------------------------------------
void cSlider::createHandle (eSliderHandleType handleType)
{
	if (handle) return;

	handle = addChild (std::make_unique<cSliderHandle> (getPosition (), handleType, orientation));

	setHandleMinMaxPosition ();

	signalConnectionManager.connect (handle->moved, std::bind (&cSlider::movedHandle, this));
}

//------------------------------------------------------------------------------
void cSlider::setHandleMinMaxPosition ()
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

	const auto mousePosition = (orientation == eOrientationType::Horizontal ? mouse.getPosition ().x () : mouse.getPosition ().y ());
	
	auto newHandlePosition = mousePosition - (orientation == eOrientationType::Horizontal ? handle->getSize ().x () / 2 : handle->getSize ().y () / 2);

	int minPosition, maxPosition;
	computeHandleMinMaxPosition (minPosition, maxPosition);

	newHandlePosition = std::max (std::min (newHandlePosition, maxPosition), minPosition);

	cPosition position = handle->getPosition();
	(orientation == eOrientationType::Horizontal ? position.x () : position.y ()) = newHandlePosition;
	handle->moveTo (position);

	return true;
}

//------------------------------------------------------------------------------
void cSlider::computeHandleMinMaxPosition (int& minPosition, int& maxPosition)
{
	minPosition = (orientation == eOrientationType::Horizontal ? getPosition ().x () : getPosition ().y ());
	maxPosition = (orientation == eOrientationType::Horizontal ? getEndPosition ().x () - handle->getSize ().x () : getEndPosition ().y () - handle->getSize ().y ());
}

//------------------------------------------------------------------------------
void cSlider::movedHandle ()
{
	valueChanged ();
}
