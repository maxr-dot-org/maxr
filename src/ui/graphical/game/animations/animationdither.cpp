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

#include "ui/graphical/game/animations/animationdither.h"
#include "ui/graphical/game/animations/animationtimer.h"
#include "game/data/units/vehicle.h"
#include "utility/box.h"
#include "utility/random.h"

//------------------------------------------------------------------------------
cAnimationDither::cAnimationDither (cAnimationTimer& animationTimer_, const cVehicle& vehicle_) :
	animationTimer (animationTimer_),
	vehicle (&vehicle_),
	incrementEffect (false)
{
	// immediately start for planes that are in the air already
	if (vehicle->getFlightHeight() > 0)
	{
		activate ();
	}

	// register the animation for flight height changes
	signalConnectionManager.connect (vehicle->flightHeightChanged, [this]()
	{
		// the plane has landed: stop the animation
		if (vehicle->getFlightHeight () == 0 && isRunning())
		{
			vehicle->ditherX = 0;
			vehicle->ditherY = 0;

			animationTimerConnectionManager.disconnectAll ();
			running = false;
		}
		// the plane took of: restart the animation
		else if (vehicle->getFlightHeight () > 0 && !isRunning())
		{
			activate ();
		}
	});
	// make sure we stop the animation when the unit gets destroyed
	signalConnectionManager.connect (vehicle->destroyed, [this]()
	{
		signalConnectionManager.disconnectAll ();
		vehicle = nullptr;
		finished = true;
	});
}

//------------------------------------------------------------------------------
cAnimationDither::~cAnimationDither ()
{
	// make sure the vehicle is in the correct place when the animation gets deleted
	if (isRunning () && vehicle)
	{
		vehicle->ditherX = 0;
		vehicle->ditherY = 0;
	}
}

//------------------------------------------------------------------------------
bool cAnimationDither::isLocatedIn (const cBox<cPosition>& box) const
{
	return vehicle && box.intersects (vehicle->getArea ());
}

//------------------------------------------------------------------------------
void cAnimationDither::activate ()
{
	if (!vehicle) return;
	animationTimerConnectionManager.connect (animationTimer.triggered100ms, std::bind (&cAnimationDither::run, this));
	running = true;
}

//------------------------------------------------------------------------------
void cAnimationDither::run ()
{
	if (!vehicle) return;

	// do not dither while the plane is moving.
	// reset the position every now and then to make sure the plane stays around the center position
	if (vehicle->isUnitMoving () || animationTimer.getAnimationTime () % 10 == 0)
	{
		vehicle->ditherX = 0;
		vehicle->ditherY = 0;
	}
	else
	{
		vehicle->ditherX = random (2) - 1;
		vehicle->ditherY = random (2) - 1;
	}
}
