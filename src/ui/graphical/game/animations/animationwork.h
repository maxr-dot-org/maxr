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

#ifndef ui_graphical_game_animations_animationworkH
#define ui_graphical_game_animations_animationworkH

#include "ui/graphical/game/animations/animation.h"
#include "utility/signal/signalconnectionmanager.h"

class cAnimationTimer;
class cBuilding;

/**
 * Animation for the alpha effect of working units.
 */
class cAnimationWork : public cAnimation
{
public:
	cAnimationWork (cAnimationTimer& animationTimer, const cBuilding& building);

	virtual bool isLocatedIn (const cBox<cPosition>& box) const MAXR_OVERRIDE_FUNCTION;
private:
	cSignalConnectionManager signalConnectionManager;
	cSignalConnectionManager animationTimerConnectionManager;
	cAnimationTimer& animationTimer;
	const cBuilding* building;

	bool incrementEffect;

	void activate();
	void run();
};

#endif // ui_graphical_game_animations_animationworkH
