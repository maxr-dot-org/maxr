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

#include "game/logic/jobs.h"

#include "game/logic/gametimer.h"
#include "game/data/units/unit.h"
#include "game/data/units/vehicle.h"
#include "game/logic/movejob.h"

#include <algorithm>
#include <cassert>

cJob::cJob (cVehicle& vehicle_, unsigned int id) :
	finished (false),
	vehicle (&vehicle_),
	id(id)
{}

void cJobContainer::addJob (cJob& job)
{
	//only one job per unit
	if (job.vehicle->job)
	{
		std::vector<cJob*>::iterator it = std::find (jobs.begin(), jobs.end(), job.vehicle->job);
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

void cJobContainer::clear()
{
	for (int i = 0; i < jobs.size(); i++)
	{
		cJob* job = jobs[i];
		assert(job->vehicle->job == job);
		job->vehicle->job = nullptr;
		delete job;
	}
	jobs.clear();
}

std::vector<cJob*>::iterator cJobContainer::releaseJob (std::vector<cJob*>::iterator it)
{
	if (it == jobs.end()) return jobs.end();
	cJob* job = *it;
	assert (job->vehicle->job == job);
	job->vehicle->job = nullptr;
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

cStartBuildJob::cStartBuildJob (cVehicle& vehicle_, const cPosition& org_, bool big_, unsigned int id) :
	cJob (vehicle_, id),
	org (org_),
	big (big_)
{
	vehicle_.setMovementOffset (cPosition (vehicle_.getPosition().x() < org.x() ? 64 : 0, vehicle_.getPosition().y() < org.y() ? 64 : 0));
}

void cStartBuildJob::run (const cGameTimer& gameTimer)
{
	if (!vehicle->isUnitBuildingABuilding() && !vehicle->isUnitClearing())
	{
		//cancel the job, if the vehicle is not building or clearing!
		finished = true;
		vehicle->setMovementOffset (cPosition (0, 0));
	}

	if (big)
	{
		int deltaX = (vehicle->getPosition().x() < org.x() ? -1 : 1) * MOVE_SPEED;
		int deltaY = (vehicle->getPosition().y() < org.y() ? -1 : 1) * MOVE_SPEED;
		int dir = 0;
		if (deltaX > 0 && deltaY > 0) dir = 3;
		if (deltaX > 0 && deltaY < 0) dir = 1;
		if (deltaX < 0 && deltaY > 0) dir = 5;
		if (deltaX < 0 && deltaY < 0) dir = 7;

		if (vehicle->getMovementOffset().x() == 32)
		{
			//if (!gameTimer.timer100ms) return;
			vehicle->rotateTo (0);
			if (vehicle->dir == 0)
			{
				finished = true;
				vehicle->setMovementOffset (cPosition (0, 0));
			}
		}
		else if (vehicle->dir == dir)
		{
			cPosition newOffset (vehicle->getMovementOffset());
			newOffset.x() += deltaX;
			newOffset.y() += deltaY;
			vehicle->setMovementOffset (newOffset);

			if ((vehicle->getMovementOffset().x() > 32 && deltaX > 0) || (vehicle->getMovementOffset().y() < 32 && deltaX < 0))
			{
				vehicle->setMovementOffset (cPosition (32, 32));
			}
		}
		else
		{
			//if (!gameTimer.timer100ms) return;
			vehicle->rotateTo (dir);
		}
	}
	else
	{
		//if (!gameTimer.timer100ms) return;
		vehicle->rotateTo (0);
		if (vehicle->dir == 0)
		{
			finished = true;
		}
	}
}

cPlaneTakeoffJob::cPlaneTakeoffJob (cVehicle& vehicle_, bool takeoff_, unsigned int id) :
	cJob (vehicle_, id),
	takeoff (takeoff_)
{}

void cPlaneTakeoffJob::run (const cGameTimer& gameTimer)
{
	// TODO add sound #708
	cVehicle* plane = vehicle;
	if (takeoff)
	{
		plane->setFlightHeight (plane->getFlightHeight() + 2);
		if (plane->getFlightHeight() == 64)
		{
			finished = true;
		}
	}
	else
	{
		plane->setFlightHeight (plane->getFlightHeight() - 2);
		if (plane->getFlightHeight() == 0)
		{
			finished = true;
		}
	}
}
