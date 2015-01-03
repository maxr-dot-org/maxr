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

#include "ui/graphical/game/animations/animationstartupbuildingsite.h"
#include "ui/graphical/game/animations/animationtimer.h"
#include "game/data/units/vehicle.h"
#include "utility/box.h"

//------------------------------------------------------------------------------
cAnimationStartUpBuildingSite::cAnimationStartUpBuildingSite (cAnimationTimer& animationTimer_, const cVehicle& vehicle_) :
	animationTimer (animationTimer_),
	vehicle (&vehicle_)
{
	// register the animation to build status changes
	signalConnectionManager.connect (vehicle->buildingChanged, [this]()
	{
		if (vehicle->isUnitBuildingABuilding())
		{
			activate();
		}
		else if (isRunning())
		{
			animationTimerConnectionManager.disconnectAll();
			running = false;
		}
	});

	signalConnectionManager.connect (vehicle->destroyed, [this]()
	{
		animationTimerConnectionManager.disconnectAll();
		vehicle = nullptr;
		finished = true;
	});
}

//------------------------------------------------------------------------------
cAnimationStartUpBuildingSite::~cAnimationStartUpBuildingSite()
{
	if (isRunning() && vehicle)
	{
		vehicle->bigBetonAlpha = 254;
	}
}

//------------------------------------------------------------------------------
void cAnimationStartUpBuildingSite::activate()
{
	if (!vehicle) return;

	vehicle->bigBetonAlpha = 0;

	running = true;
	animationTimerConnectionManager.connect (animationTimer.triggered50ms, std::bind (&cAnimationStartUpBuildingSite::run, this));
}

//------------------------------------------------------------------------------
bool cAnimationStartUpBuildingSite::isLocatedIn (const cBox<cPosition>& box) const
{
	return vehicle && box.intersects (vehicle->getArea());
}

//------------------------------------------------------------------------------
void cAnimationStartUpBuildingSite::run()
{
	if (!vehicle) return;

	vehicle->bigBetonAlpha += 25;

	if (vehicle->bigBetonAlpha >= 254)
	{
		vehicle->bigBetonAlpha = 254;
		animationTimerConnectionManager.disconnectAll();
		running = false;
	}
}
