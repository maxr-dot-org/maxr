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

#include "ui/graphical/menu/widgets/scrollbar.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/slider.h"

//------------------------------------------------------------------------------
cScrollBar::cScrollBar (const cPosition& position, int width, eScrollBarStyle style_, eOrientationType orientation_) :
	cWidget (position),
	style (style_),
	orientation (orientation_)
{
	assert (width >= 0);

	ePushButtonType backButtonType;
	ePushButtonType forwardButtonType;
	eSliderType sliderType;
	eSliderHandleType sliderHandleType;
	switch (style)
	{
	case eScrollBarStyle::Classic:
		backButtonType = orientation == eOrientationType::Horizontal ? ePushButtonType::ArrowLeftSmall : ePushButtonType::ArrowUpSmall;
		forwardButtonType = orientation == eOrientationType::Horizontal ? ePushButtonType::ArrowRightSmall : ePushButtonType::ArrowDownSmall;
		sliderType = eSliderType::DrawnBackground;
		sliderHandleType = orientation == eOrientationType::Horizontal ? eSliderHandleType::Horizontal : eSliderHandleType::Vertical;
		break;
	case eScrollBarStyle::Modern:
		backButtonType = orientation == eOrientationType::Horizontal ? ePushButtonType::ArrowLeftSmallModern : ePushButtonType::ArrowUpSmallModern;
		forwardButtonType = orientation == eOrientationType::Horizontal ? ePushButtonType::ArrowRightSmallModern : ePushButtonType::ArrowDownSmallModern;
		sliderType = eSliderType::Invisible;
		sliderHandleType = orientation == eOrientationType::Horizontal ? eSliderHandleType::ModernHorizontal : eSliderHandleType::ModernVertical;
		break;
	}

	backButton = addChild (std::make_unique<cPushButton> (getPosition(), backButtonType));
	
	forwardButton = addChild (std::make_unique<cPushButton> (getPosition (), forwardButtonType));
	auto forwardButtonOffset = orientation == eOrientationType::Horizontal ? cPosition (width-forwardButton->getSize ().x (), 0) : cPosition (0, width-forwardButton->getSize ().y ());
	forwardButtonOffset.x () = std::max (forwardButtonOffset.x (), 0);
	forwardButtonOffset.y () = std::max (forwardButtonOffset.y (), 0);
	forwardButton->move (forwardButtonOffset);

	cPosition sliderStartPos = backButton->getPosition () + (orientation == eOrientationType::Horizontal ? cPosition (backButton->getSize ().x (), 0) : cPosition (0, backButton->getSize ().y ()));
	cPosition sliderEndPos = forwardButton->getPosition () + (orientation == eOrientationType::Horizontal ? cPosition (0, forwardButton->getSize ().y ()) : cPosition (forwardButton->getSize ().x (), 0));
	if (style == eScrollBarStyle::Classic)
	{
		if (orientation == eOrientationType::Horizontal)
		{
			sliderStartPos += cPosition (0, 1);
			sliderEndPos += cPosition (0, -2);
		}
		else
		{
			sliderStartPos += cPosition (1, 0);
			sliderEndPos += cPosition (-2, 0);
		}
	}

	slider = addChild (std::make_unique<cSlider> (cBox<cPosition> (sliderStartPos, sliderEndPos), 0, 0, orientation, sliderHandleType, sliderType));
	slider->setValue (5);
	fitToChildren ();

	signalConnectionManager.connect (backButton->clicked, [this](){ backClicked (); });
	signalConnectionManager.connect (forwardButton->clicked, [this](){ forwardClicked (); });
	signalConnectionManager.connect (slider->valueChanged, [this](){ offsetChanged (); });
}

//------------------------------------------------------------------------------
void cScrollBar::handleResized (const cPosition& oldSize)
{
	const cPosition forwardButtonPosition = getPosition () + (orientation == eOrientationType::Horizontal ? cPosition (getSize ().x () - forwardButton->getSize ().x (), 0) : cPosition (0, getSize ().y () - forwardButton->getSize ().y ()));
	forwardButton->moveTo (forwardButtonPosition);

	cPosition sliderNewSize = (orientation == eOrientationType::Horizontal ? cPosition (getSize ().x () - backButton->getSize ().x () - forwardButton->getSize ().x (), slider->getSize ().y ()) : cPosition (slider->getSize ().x (), getSize ().y () - backButton->getSize ().y () - forwardButton->getSize ().y ()));
	slider->resize (sliderNewSize);

	cWidget::handleResized (oldSize);
}

//------------------------------------------------------------------------------
void cScrollBar::setRange (int range)
{
	slider->setMaxValue (range);
}

//------------------------------------------------------------------------------
void cScrollBar::setOffset (int offset)
{
	slider->setValue (offset);
}

//------------------------------------------------------------------------------
int cScrollBar::getOffset ()
{
	return slider->getValue();
}
