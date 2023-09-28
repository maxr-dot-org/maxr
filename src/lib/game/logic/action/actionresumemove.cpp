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

#include "actionresumemove.h"

#include "game/data/model.h"
#include "game/data/units/vehicle.h"
#include "game/logic/movejob.h"

//------------------------------------------------------------------------------
cActionResumeMove::cActionResumeMove (const cVehicle& vehicle) :
	unitId (vehicle.getId())
{}

//------------------------------------------------------------------------------
cActionResumeMove::cActionResumeMove (cBinaryArchiveIn& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionResumeMove::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	if (unitId == 0) // All vehicles
	{
		const cPlayer* player = model.getPlayer (playerNr);
		if (player == nullptr) return;

		model.resumeMoveJobs (player);
	}
	else
	{
		cVehicle* vehicle = model.getVehicleFromID (unitId);
		if (vehicle == nullptr)
		{
			NetLog.warn (" Can't find vehicle with id " + std::to_string (unitId));
			return;
		}
		if (!vehicle->getOwner()) return;
		if (vehicle->getOwner()->getId() != playerNr) return;

		if (vehicle->getMoveJob())
		{
			vehicle->getMoveJob()->resume();
		}
	}
}
