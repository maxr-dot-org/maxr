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

#include "ui/graphical/game/control/rightmousebuttonscroller.h"

#include "SDLutility/drawing.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "input/mouse/mouse.h"
#include "output/video/video.h"
#include "ui/graphical/game/animations/animationtimer.h"
#include "ui/widgets/application.h"
#include "ui/widgets/image.h"
#include "utility/color.h"

const double cRightMouseButtonScrollerWidget::factor = 0.05;
const double cRightMouseButtonScrollerWidget::minDistanceSquared = 15 * 15;

//------------------------------------------------------------------------------
cRightMouseButtonScrollerWidget::cRightMouseButtonScrollerWidget (std::shared_ptr<cAnimationTimer> animationTimer_) :
	cWidget (cBox<cPosition> (cPosition (0, 0), cPosition (Video.getResolutionX(), Video.getResolutionY()))),
	animationTimer (animationTimer_)
{
	signalConnectionManager.connect (Video.resolutionChanged, [this]() {
		resize (cPosition (Video.getResolutionX(), Video.getResolutionY()));
	});

	// TODO: read nice image from file
	UniqueSurface image (SDL_CreateRGBSurface (0, 18, 18, Video.getColDepth(), 0, 0, 0, 0));
	SDL_FillRect (image.get(), nullptr, 0xFF00FF);
	SDL_SetColorKey (image.get(), SDL_TRUE, 0xFF00FF);
	const cPosition middle (image->w / 2, image->h / 2);
	for (int x = 0; x < image->w; ++x)
	{
		for (int y = 0; y < image->h; ++y)
		{
			const cPosition p (x, y);
			const auto distance = (p - middle).l2NormSquared();
			if (distance < (9 * 9) && distance > (3 * 3))
			{
				const auto colorValue = 30 + std::abs (distance - (6 * 6));
				drawPoint (*image, p, cRgbColor (colorValue, colorValue, colorValue));
			}
		}
	}

	startIndicator = emplaceChild<cImage> (cPosition (0, 0), image.get());
	startIndicator->disable();
}

//------------------------------------------------------------------------------
bool cRightMouseButtonScrollerWidget::isScrolling() const
{
	return hasStartedScrolling;
}

//------------------------------------------------------------------------------
bool cRightMouseButtonScrollerWidget::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	if (hasStartedScrolling)
	{
		const cPosition offset = getCursorCenter (mouse) - startPosition;

		const cPosition scaledOffset (static_cast<int> (offset.x() * factor), static_cast<int> (offset.y() * factor));

		auto degrees = std::atan2 (offset.y(), offset.x());
		if (degrees < 0) degrees += M_PI * 2.0;
		degrees = (degrees * 180) / M_PI;

		if ((degrees >= 0 && degrees <= 20) || degrees > 350)
			mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowRight));
		else if (degrees > 160 && degrees <= 200)
			mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowLeft));
		else if (degrees > 250 && degrees <= 290)
			mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowUp));
		else if (degrees > 70 && degrees <= 110)
			mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowDown));
		else if (degrees > 290 && degrees <= 350)
			mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowRightUp));
		else if (degrees > 20 && degrees <= 70)
			mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowRightDown));
		else if (degrees > 200 && degrees <= 250)
			mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowLeftUp));
		else if (degrees > 110 && degrees <= 160)
			mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowLeftDown));

		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cRightMouseButtonScrollerWidget::handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Right && !mouse.isButtonPressed (eMouseButtonType::Left) && !hasStartedScrolling)
	{
		startPosition = getCursorCenter (mouse);
		animationTimerSignalConnectionManager.connect (animationTimer->triggered10msCatchUp, [this, &mouse, &application]() {
			if (!mouse.isButtonPressed (eMouseButtonType::Right) || mouse.isButtonPressed (eMouseButtonType::Left))
			{
				animationTimerSignalConnectionManager.disconnectAll();
			}

			const cPosition offset = getCursorCenter (mouse) - startPosition;

			if (offset.l2NormSquared() > minDistanceSquared)
			{
				if (!hasStartedScrolling)
				{
					hasStartedScrolling = true;
					application.grapMouseFocus (*this);
					startIndicator->moveTo (startPosition - startIndicator->getSize() / 2);
					startIndicator->show();
					startedScrolling();
				}

				if (hasStartedScrolling)
				{
					const cPosition scaledOffset (static_cast<int> (offset.x() * factor), static_cast<int> (offset.y() * factor));

					scroll (scaledOffset);
				}
			}
		});
	}
	return false;
}

//------------------------------------------------------------------------------
bool cRightMouseButtonScrollerWidget::handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	animationTimerSignalConnectionManager.disconnectAll();
	if (hasStartedScrolling && button == eMouseButtonType::Right && !mouse.isButtonPressed (eMouseButtonType::Left))
	{
		application.releaseMouseFocus (*this);
		hasStartedScrolling = false;
		startIndicator->hide();
		stoppedScrolling();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void cRightMouseButtonScrollerWidget::handleLooseMouseFocus (cApplication& application)
{
	mouseFocusReleased();
}

//------------------------------------------------------------------------------
cPosition cRightMouseButtonScrollerWidget::getCursorCenter (cMouse& mouse) const
{
	auto position = mouse.getPosition();
	const auto cursor = mouse.getCursor();
	if (cursor)
	{
		position -= cursor->getHotPoint();
		const auto cursorSurface = cursor->getSurface();
		if (cursorSurface)
		{
			position += cPosition (cursorSurface->w / 2, cursorSurface->h / 2);
		}
	}
	return position;
}
