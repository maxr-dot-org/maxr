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

#include "startbuildjob.h"

#include "game/data/model.h"
#include "game/data/units/vehicle.h"
#include "game/logic/movejob.h"

#include <cassert>

//------------------------------------------------------------------------------
cStartBuildJob::cStartBuildJob (cVehicle& vehicle_, const cPosition& org_, bool big_) :
	cJob (vehicle_),
	org (org_),
	big (big_)
{
	vehicle_.setMovementOffset (cPosition (vehicle_.getPosition().x() < org.x() ? 64 : 0, vehicle_.getPosition().y() < org.y() ? 64 : 0));
}

//------------------------------------------------------------------------------
void cStartBuildJob::postLoad (const cModel& model)
{
	auto* unit = model.getVehicleFromID (unitId);

	if (unit != nullptr)
	{
		unit->jobActive = true;
	}
}

//------------------------------------------------------------------------------
void cStartBuildJob::run (cModel& model)
{
	cVehicle* vehicle = model.getVehicleFromID (unitId);
	assert (vehicle);

	if (!vehicle->isUnitBuildingABuilding() && !vehicle->isUnitClearing())
	{
		//cancel the job, if the vehicle is not building or clearing!
		finished = true;
		vehicle->setMovementOffset (cPosition (0, 0));
	}

	if (big)
	{
		const int deltaX = (vehicle->getPosition().x() < org.x() ? -1 : 1) * MOVE_SPEED;
		const int deltaY = (vehicle->getPosition().y() < org.y() ? -1 : 1) * MOVE_SPEED;
		EDirection dir = EDirection::North;
		if (deltaX > 0 && deltaY > 0) dir = EDirection::SouthEast;
		if (deltaX > 0 && deltaY < 0) dir = EDirection::NorthEast;
		if (deltaX < 0 && deltaY > 0) dir = EDirection::SouthWest;
		if (deltaX < 0 && deltaY < 0) dir = EDirection::NorthWest;

		if (vehicle->getMovementOffset().x() == 32)
		{
			if (model.getGameTime() % 10 != 0) return;
			vehicle->rotateTo (EDirection::North);
			if (vehicle->dir == EDirection::North)
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
			if (model.getGameTime() % 10 != 0) return;
			vehicle->rotateTo (dir);
		}
	}
	else
	{
		if (model.getGameTime() % 10 != 0) return;
		vehicle->rotateTo (EDirection::North);
		if (vehicle->dir == EDirection::North)
		{
			finished = true;
		}
	}
}

//------------------------------------------------------------------------------
eJobType cStartBuildJob::getType() const
{
	return eJobType::START_BUILD;
}

//------------------------------------------------------------------------------
uint32_t cStartBuildJob::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (getType(), crc);
	crc = calcCheckSum (unitId, crc);
	crc = calcCheckSum (org, crc);
	crc = calcCheckSum (big, crc);

	return crc;
}
