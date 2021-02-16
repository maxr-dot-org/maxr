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

#ifndef ui_graphical_game_control_rightmousebuttonscrollerH
#define ui_graphical_game_control_rightmousebuttonscrollerH

#include <memory>

#include "ui/graphical/widget.h"
#include "utility/position.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/signal/signal.h"

class cMouse;
class cAnimationTimer;
class cImage;

class cRightMouseButtonScrollerWidget : public cWidget
{
public:
	cRightMouseButtonScrollerWidget (std::shared_ptr<cAnimationTimer> animationTimer);

	bool isScrolling() const;

	mutable cSignal<void (const cPosition&)> scroll;

	mutable cSignal<void ()> mouseFocusReleased;

	mutable cSignal<void ()> startedScrolling;
	mutable cSignal<void ()> stoppedScrolling;

	bool handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset) override;
	bool handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button) override;
	bool handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button) override;
	void handleLooseMouseFocus (cApplication& application) override;
private:
	static const double factor;
	static const double minDistanceSquared;

	cSignalConnectionManager signalConnectionManager;
	cSignalConnectionManager animationTimerSignalConnectionManager;

	std::shared_ptr<cAnimationTimer> animationTimer;

	cImage* startIndicator;

	cPosition startPosition;
	bool hasStartedScrolling;

	cPosition getCursorCenter (cMouse& mouse) const;
};

#endif // ui_graphical_game_control_rightmousebuttonscrollerH
