#include "jobs.h"
#include "unit.h"
#include "vehicles.h"
#include "gametimer.h"

cJob::cJob (cUnit* unit_) :
	finished (false),
	unit (unit_)
{}


cStartBuildJob::cStartBuildJob (cUnit* unit_, int orgX_, int orgY_, bool big_) : 
	cJob (unit_),
	orgX (orgX_),
	orgY (orgY_),
	big (big_)
{
	cVehicle* vehicle = dynamic_cast<cVehicle*> (unit);
	vehicle->OffX = (vehicle->PosX < orgX ? 64 : 0);
	vehicle->OffY = (vehicle->PosY < orgY ? 64 : 0);
}

void cStartBuildJob::run (const cGameTimer &gameTimer)
{
	cVehicle* vehicle = dynamic_cast<cVehicle*> (unit);

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
			vehicle->rotateTo(0);
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

			if ( (vehicle->OffX > 32 && deltaX > 0) || (vehicle->OffX < 32 && deltaX < 0) )
			{
				vehicle->OffX = 32;
				vehicle->OffY = 32;
			}
		}
		else
		{
			if (!gameTimer.timer100ms) return;
			vehicle->rotateTo(dir);
		}
	}
	else
	{
		if (!gameTimer.timer100ms) return;
		vehicle->rotateTo(0);
		if (vehicle->dir == 0)
		{
			finished = true;
		}
	}
}

cPlaneTakeoffJob::cPlaneTakeoffJob(cUnit* unit_, bool takeoff_) : 
	cJob (unit_),
	takeoff (takeoff_)
{}

void cPlaneTakeoffJob::run (const cGameTimer &gameTimer)
{
	cVehicle* plane = dynamic_cast<cVehicle*> (unit);
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
