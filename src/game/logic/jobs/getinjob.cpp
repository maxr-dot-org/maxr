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

#include "getinjob.h"

#include "game/data/units/vehicle.h"
#include "game/data/model.h"
#include "utility/crc.h"


cGetInJob::cGetInJob (cVehicle& loadedVehicle, cUnit& loadingUnit) :
	cJob (loadedVehicle),
	loadingUnit (&loadingUnit),
	counter(32),
	startFlightHeight(loadedVehicle.getFlightHeight())
{
	connectionManager.connect(loadingUnit.destroyed, [&](){finished = true; });
	unit->alphaEffectValue = 254;
}

//------------------------------------------------------------------------------
void cGetInJob::run (cModel& model)
{
	assert(unit->isAVehicle());
	cVehicle* vehicle = static_cast<cVehicle*>(unit);

	vehicle->setFlightHeight(std::max(0, vehicle->getFlightHeight() - 2));
	//Fixme: alphaEffect should be GUI only, and not changed from the model.
	//       Also the alphaEffect must not have any effect on the model behavior,
	//       so using a fixed counter for the delay.
	vehicle->alphaEffectValue = std::max(1, vehicle->alphaEffectValue - 8);
	counter--;

	if (counter <= 0 && vehicle->getFlightHeight() == 0)
	{
		if (loadingUnit->canLoad(vehicle))
		{
			loadingUnit->storeVehicle(*vehicle, *model.getMap());
			model.unitStored(*loadingUnit, *vehicle);
		}
		else
		{
			vehicle->setFlightHeight(startFlightHeight);
			vehicle->alphaEffectValue = 0;
		}

		finished = true;
	}
}

//------------------------------------------------------------------------------
eJobType cGetInJob::getType() const
{
	return eJobType::GET_IN;
}

//------------------------------------------------------------------------------
uint32_t cGetInJob::getChecksum(uint32_t crc) const
{
	crc = calcCheckSum(getType(), crc);
	crc = calcCheckSum(unit, crc);
	crc = calcCheckSum(loadingUnit, crc);
	crc = calcCheckSum(counter, crc);
	crc = calcCheckSum(startFlightHeight, crc);

	return crc;
}
