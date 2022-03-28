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
	cRightMouseButtonScrollerWidget (std::shared_ptr<cAnimationTimer>);

	bool isScrolling() const;

	mutable cSignal<void (const cPosition&)> scroll;

	mutable cSignal<void()> mouseFocusReleased;

	mutable cSignal<void()> startedScrolling;
	mutable cSignal<void()> stoppedScrolling;

	bool handleMouseMoved (cApplication&, cMouse&, const cPosition& offset) override;
	bool handleMousePressed (cApplication&, cMouse&, eMouseButtonType) override;
	bool handleMouseReleased (cApplication&, cMouse&, eMouseButtonType) override;
	void handleLooseMouseFocus (cApplication&) override;
private:
	static const double factor;
	static const double minDistanceSquared;

	cSignalConnectionManager signalConnectionManager;
	cSignalConnectionManager animationTimerSignalConnectionManager;

	std::shared_ptr<cAnimationTimer> animationTimer;

	cImage* startIndicator = nullptr;

	cPosition startPosition;
	bool hasStartedScrolling = false;

	cPosition getCursorCenter (cMouse&) const;
};

#endif // ui_graphical_game_control_rightmousebuttonscrollerH
