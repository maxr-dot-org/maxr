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

#include "game/data/units/vehicle.h"
#include "game/data/model.h"
#include "utility/crc.h"


cAirTransportLoadJob::cAirTransportLoadJob (cVehicle& loadedVehicle, cUnit& loadingUnit) :
	cJob (loadingUnit),
	vehicleToLoad (&loadedVehicle),
	landing(true)
{
	connectionManager.connect(vehicleToLoad->destroyed, [&](){finished = true; });
}

//------------------------------------------------------------------------------
void cAirTransportLoadJob::run (cModel& model)
{
	assert(unit->isAVehicle());
	cVehicle* vehicle = static_cast<cVehicle*>(unit);

	if (landing)
	{
		vehicle->setFlightHeight(std::max(0, vehicle->getFlightHeight() - 2));
		if (vehicle->getFlightHeight() <= 0)
		{
			if (vehicle->canLoad(vehicleToLoad))
			{
				vehicle->storeVehicle(*vehicleToLoad, *model.getMap());
				model.unitStored(*unit, *vehicleToLoad);
			}
			landing = false;
		}
	}
	else
	{
		vehicle->setFlightHeight(std::min(64, vehicle->getFlightHeight() + 2));
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
uint32_t cAirTransportLoadJob::getChecksum(uint32_t crc) const
{
	crc = calcCheckSum(getType(), crc);
	crc = calcCheckSum(unit, crc);
	crc = calcCheckSum(vehicleToLoad, crc);
	crc = calcCheckSum(landing, crc);

	return crc;
}
