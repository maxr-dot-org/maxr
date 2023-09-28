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

#include "actionload.h"

#include "game/data/model.h"
#include "game/logic/jobs/airtransportloadjob.h"
#include "game/logic/jobs/getinjob.h"
#include "utility/log.h"

//------------------------------------------------------------------------------
cActionLoad::cActionLoad (const cUnit& loadingUnit, const cVehicle& loadedVehicle) :
	loadingUnitId (loadingUnit.getId()),
	loadedVehicleId (loadedVehicle.getId())
{}

//------------------------------------------------------------------------------
cActionLoad::cActionLoad (cBinaryArchiveIn& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionLoad::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	auto loadingUnit = model.getUnitFromID (loadingUnitId);
	if (loadingUnit == nullptr) return;

	auto loadedVehicle = model.getVehicleFromID (loadedVehicleId);
	if (loadedVehicle == nullptr) return;

	if (!loadingUnit->canLoad (loadedVehicle)) return;

	if (loadingUnit->getStaticUnitData().factorAir > 0)
	{
		model.addJob (std::make_unique<cAirTransportLoadJob> (*loadedVehicle, *loadingUnit));
	}
	else
	{
		model.addJob (std::make_unique<cGetInJob> (*loadedVehicle, *loadingUnit));
	}
}
