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

#ifndef ui_graphical_game_animations_animationH
#define ui_graphical_game_animations_animationH

#include "utility/signal/signal.h"

class cPosition;
template <typename>
class cBox;

/**
 * This is the base class of an animation of anything (most probable a unit)
 * on the game map.
 *
 * The class provides an interface that allows to check whether the animation
 * needs to be active at a given position on the map.
 *
 * This allows the map drawing engine to delete animations that are not visible
 * any longer.
 *
 * Further the interface provides access to information about the running state
 * of the animation.
 * This allows the drawing engine to remove animation that are finished.
 */
class cAnimation
{
public:
	/**
	 * Initializes the base class animation to be not running and not finished.
	 */
	cAnimation() = default;
	virtual ~cAnimation() = default;

	/**
	 * Should return true when the animation needs to be executed if the
	 * passed box of the map is visible.
	 *
	 * @param box The map section that is currently active.
	 * @return True if the animation needs to be executed when the given
	 *         map section is visible.
	 */
	virtual bool isLocatedIn (const cBox<cPosition>& box) const = 0;

	/**
	 * Returns true if the animation is finished and can be removed now.
	 */
	bool isFinished() const { return finished; }
	/**
	 * Returns true when the animation is currently running.
	 */
	bool isRunning() const { return running; }

protected:
	bool finished = false;
	bool running = false;
};

#endif // ui_graphical_game_animations_animationH
