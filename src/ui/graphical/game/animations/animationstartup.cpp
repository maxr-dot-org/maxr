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

#include "ui/graphical/game/animations/animationstartup.h"
#include "ui/graphical/game/animations/animationtimer.h"
#include "game/data/units/unit.h"
#include "utility/box.h"

//------------------------------------------------------------------------------
cAnimationStartUp::cAnimationStartUp (cAnimationTimer& animationTimer_, const cUnit& unit_) :
	animationTimer (animationTimer_),
	unit (&unit_)
{
	unit->alphaEffectValue = 10;

	running = true;
	animationTimerConnectionManager.connect (animationTimer.triggered100ms, std::bind (&cAnimationStartUp::run, this));

	signalConnectionManager.connect (unit->destroyed, [this]()
	{
		animationTimerConnectionManager.disconnectAll ();
		unit = nullptr;
		finished = true;
	});
}

//------------------------------------------------------------------------------
cAnimationStartUp::~cAnimationStartUp ()
{
	if (isRunning () && unit)
	{
		unit->alphaEffectValue = 0;
	}
}

//------------------------------------------------------------------------------
bool cAnimationStartUp::isLocatedIn (const cBox<cPosition>& box) const
{
	return unit && box.intersects (unit->getArea ());
}

//------------------------------------------------------------------------------
void cAnimationStartUp::run ()
{
	if (!unit) return;

	unit->alphaEffectValue += 25;

	if (unit->alphaEffectValue >= 255)
	{
		unit->alphaEffectValue = 0;
		animationTimerConnectionManager.disconnectAll ();
		finished = true;
		running = false;
	}
}
