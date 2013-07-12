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

#include "jobs.h"

#include "gametimer.h"
#include "unit.h"
#include "vehicles.h"

#include <algorithm>
#include <cassert>

cJob::cJob (cVehicle& vehicle_) :
	finished (false),
	vehicle (&vehicle_)
{}

void cJobContainer::addJob (cJob& job)
{
	//only one job per unit
	if (job.vehicle->job)
	{
		std::vector<cJob*>::iterator it = std::find (jobs.begin(), jobs.end(), &job);
		releaseJob (it);
	}

	jobs.push_back (&job);
	job.vehicle->job = &job;
}

void cJobContainer::run (cGameTimer& gameTimer)
{
	for (std::vector<cJob*>::iterator it = jobs.begin(); it != jobs.end();)
	{
		cJob* job = *it;

		if (!job->finished) job->run (gameTimer);

		if (job->finished) it = releaseJob (it);
		else ++it;
	}
}

std::vector<cJob*>::iterator cJobContainer::releaseJob (std::vector<cJob*>::iterator it)
{
	if (it == jobs.end()) return jobs.end();
	cJob* job = *it;
	assert (job->vehicle->job == job);
	job->vehicle->job = NULL;
	it = jobs.erase (it);
	delete job;
	return it;
}

void cJobContainer::onRemoveUnit (cUnit* unit)
{
	for (std::vector<cJob*>::iterator it = jobs.begin(); it != jobs.end();)
	{
		cJob* job = *it;
		if (job->vehicle == unit) it = releaseJob (it);
		else ++it;
	}
}

cStartBuildJob::cStartBuildJob (cVehicle& vehicle_, int orgX_, int orgY_, bool big_) :
	cJob (vehicle_),
	orgX (orgX_),
	orgY (orgY_),
	big (big_)
{
	vehicle_.OffX = (vehicle_.PosX < orgX ? 64 : 0);
	vehicle_.OffY = (vehicle_.PosY < orgY ? 64 : 0);
}

void cStartBuildJob::run (const cGameTimer& gameTimer)
{
	if (!vehicle->IsBuilding && !vehicle->IsClearing)
	{
		//cancel the job, if the vehicle is not building or clearing!
		finished = true;
		vehicle->OffX = 0;
		vehicle->OffY = 0;
	}

	if (big)
	{
		int deltaX = (vehicle->PosX < orgX ? -1 : 1) * MOVE_SPEED;
		int deltaY = (vehicle->PosY < orgY ? -1 : 1) * MOVE_SPEED;
		int dir;
		if (deltaX > 0 && deltaY > 0) dir = 3;
		if (deltaX > 0 && deltaY < 0) dir = 1;
		if (deltaX < 0 && deltaY > 0) dir = 5;
		if (deltaX < 0 && deltaY < 0) dir = 7;

		if (vehicle->OffX == 32)
		{
			if (!gameTimer.timer100ms) return;
			vehicle->rotateTo (0);
			if (vehicle->dir == 0)
			{
				finished = true;
				vehicle->OffX = 0;
				vehicle->OffY = 0;
			}
		}
		else if (vehicle->dir == dir)
		{
			vehicle->OffX += deltaX;
			vehicle->OffY += deltaY;

			if ( (vehicle->OffX > 32 && deltaX > 0) || (vehicle->OffX < 32 && deltaX < 0))
			{
				vehicle->OffX = 32;
				vehicle->OffY = 32;
			}
		}
		else
		{
			if (!gameTimer.timer100ms) return;
			vehicle->rotateTo (dir);
		}
	}
	else
	{
		if (!gameTimer.timer100ms) return;
		vehicle->rotateTo (0);
		if (vehicle->dir == 0)
		{
			finished = true;
		}
	}
}

cPlaneTakeoffJob::cPlaneTakeoffJob (cVehicle& vehicle_, bool takeoff_) :
	cJob (vehicle_),
	takeoff (takeoff_)
{}

void cPlaneTakeoffJob::run (const cGameTimer& gameTimer)
{
	cVehicle* plane = vehicle;
	if (takeoff)
	{
		plane->FlightHigh += 2;
		if (plane->FlightHigh < 0)
		{
			plane->FlightHigh = 0;
			finished = true;
		}
	}
	else
	{
		plane->FlightHigh -= 2;
		if (plane->FlightHigh > 64)
		{
			plane->FlightHigh = 64;
			finished = true;
		}
	}
}
