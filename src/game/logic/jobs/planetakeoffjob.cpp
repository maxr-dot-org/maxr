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

#include "planetakeoffjob.h"

#include "game/data/units/vehicle.h"
#include "utility/crc.h"
#include "game/data/model.h"


cPlaneTakeoffJob::cPlaneTakeoffJob (cVehicle& vehicle) :
	cJob (vehicle)
{
	connectionManager.connect (vehicle.destroyed, [&](){finished = true; });
}

//------------------------------------------------------------------------------
void cPlaneTakeoffJob::run (cModel& model)
{
	assert (unit->isAVehicle());
	cVehicle* plane = static_cast<cVehicle*> (unit);

	if (plane->canLand (*model.getMap()))
	{
		if (plane->getFlightHeight() == MAX_FLIGHT_HEIGHT)
		{
			model.planeLanding (*plane);
		}

		plane->setFlightHeight (plane->getFlightHeight() - 2);
		if (plane->getFlightHeight() <= 0)
		{
			finished = true;
		}
	}
	else
	{
		if (plane->getFlightHeight() < 2)
		{
			model.planeTakeoff (*plane);
		}

		plane->setFlightHeight (plane->getFlightHeight() + 2);
		if (plane->getFlightHeight() >= MAX_FLIGHT_HEIGHT)
		{
			finished = true;
		}
	}
}

//------------------------------------------------------------------------------
eJobType cPlaneTakeoffJob::getType() const
{
	return eJobType::PLANE_TAKEOFF;
}

//------------------------------------------------------------------------------
uint32_t cPlaneTakeoffJob::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (getType(), crc);
	crc = calcCheckSum (unit ? unit->getId() : 0, crc);

	return crc;
}
