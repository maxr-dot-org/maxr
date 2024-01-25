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

#include "airtransportloadjob.h"

#include "game/data/model.h"
#include "game/data/units/vehicle.h"
#include "utility/crc.h"

#include <cassert>

cAirTransportLoadJob::cAirTransportLoadJob (cVehicle& loadedVehicle, cUnit& loadingUnit) :
	cJob (loadingUnit),
	vehicleToLoadId (loadedVehicle.getId())
{
	connectionManager.connect (loadedVehicle.destroyed, [this]() { finished = true; });
}

//------------------------------------------------------------------------------
void cAirTransportLoadJob::postLoad (const cModel& model)
{
	auto* unit = model.getUnitFromID (unitId);
	auto* vehicleToLoad = model.getVehicleFromID (vehicleToLoadId);

	if (!unit || !vehicleToLoad)
	{
		finished = true;
		return;
	}
	connectionManager.connect (vehicleToLoad->destroyed, [this]() { finished = true; });
	unit->jobActive = true;
}

//------------------------------------------------------------------------------
void cAirTransportLoadJob::run (cModel& model)
{
	auto* unit = model.getUnitFromID (unitId);
	auto* vehicleToLoad = model.getVehicleFromID (vehicleToLoadId);

	assert (unit->isAVehicle());
	cVehicle* vehicle = static_cast<cVehicle*> (unit);

	if (landing)
	{
		if (vehicle->getFlightHeight() == MAX_FLIGHT_HEIGHT)
		{
			model.planeLanding (*vehicle);
		}

		vehicle->setFlightHeight (std::max (0, vehicle->getFlightHeight() - 2));
		if (vehicle->getFlightHeight() <= 0)
		{
			if (vehicle->canLoad (vehicleToLoad))
			{
				vehicle->storeVehicle (*vehicleToLoad, *model.getMap());
				model.unitStored (*unit, *vehicleToLoad);
			}
			landing = false;
			model.planeTakeoff (*vehicle);
		}
	}
	else
	{
		vehicle->setFlightHeight (std::min (64, vehicle->getFlightHeight() + 2));
		if (vehicle->getFlightHeight() >= 64)
		{
			finished = true;
		}
	}
}

//------------------------------------------------------------------------------
eJobType cAirTransportLoadJob::getType() const
{
	return eJobType::AIR_TRANSPORT_LOAD;
}

//------------------------------------------------------------------------------
uint32_t cAirTransportLoadJob::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (getType(), crc);
	crc = calcCheckSum (unitId, crc);
	crc = calcCheckSum (vehicleToLoadId, crc);
	crc = calcCheckSum (landing, crc);

	return crc;
}
