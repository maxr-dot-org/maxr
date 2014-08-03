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

#include "ui/graphical/game/animations/animationwork.h"
#include "ui/graphical/game/animations/animationtimer.h"
#include "buildings.h"
#include "utility/box.h"

//------------------------------------------------------------------------------
cAnimationWork::cAnimationWork (cAnimationTimer& animationTimer_, const cBuilding& building_) :
	animationTimer (animationTimer_),
	building (&building_),
	incrementEffect (false)
{
	if (building->isUnitWorking () || building->data.powerOnGraphic)
	{
		activate ();
	}

	if (!building->data.powerOnGraphic)
	{
		signalConnectionManager.connect (building->workingChanged, [this]()
		{
			if (!building->isUnitWorking ())
			{
				running = false;
				animationTimerConnectionManager.disconnectAll ();
			}
			else
			{
				activate ();
			}
		});
	}
	signalConnectionManager.connect (building->destroyed, [this]()
	{
		signalConnectionManager.disconnectAll ();
		building = nullptr;
		finished = true;
	});
}

//------------------------------------------------------------------------------
bool cAnimationWork::isLocatedIn (const cBox<cPosition>& box) const
{
	return building && box.intersects (building->getArea ());
}

//------------------------------------------------------------------------------
void cAnimationWork::activate ()
{
	if (!building) return;
	building->effectAlpha = 0;
	running = true;
	animationTimerConnectionManager.connect (animationTimer.triggered100ms, std::bind (&cAnimationWork::run, this));
}

//------------------------------------------------------------------------------
void cAnimationWork::run ()
{
	if (!building) return;

	if (incrementEffect)
	{
		building->effectAlpha += 30;

		if (building->effectAlpha > 220)
		{
			building->effectAlpha = 254;
			incrementEffect = false;
		}
	}
	else
	{
		building->effectAlpha -= 30;

		if (building->effectAlpha < 30)
		{
			building->effectAlpha = 0;
			incrementEffect = true;
		}
	}
}
