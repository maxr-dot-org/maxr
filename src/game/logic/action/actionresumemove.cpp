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
#include "game/data/units/vehicle.h"
#include "game/data/model.h"
#include "utility/string/toString.h"
#include "game/logic/movejob.h"

//------------------------------------------------------------------------------
cActionResumeMove::cActionResumeMove (cBinaryArchiveOut& archive) :
	cAction (eActiontype::ACTION_RESUME_MOVE)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
cActionResumeMove::cActionResumeMove (const cVehicle& vehicle) :
	cAction (eActiontype::ACTION_RESUME_MOVE),
	unitId (vehicle.getId())
{}

//------------------------------------------------------------------------------
cActionResumeMove::cActionResumeMove () :
	cAction (eActiontype::ACTION_RESUME_MOVE),
	unitId (0)
{}

//------------------------------------------------------------------------------
void cActionResumeMove::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	if (unitId == 0) // All vehicles
	{
		const cPlayer* player = model.getPlayer(playerNr);
		if (player == nullptr) return;
	
		for (const auto& vehicle : player->getVehicles())
		{
			if (vehicle->getMoveJob())
			{
				vehicle->getMoveJob()->resume();
			}
		}
	}
	else
	{
		cVehicle* vehicle = model.getVehicleFromID(unitId);
		if (vehicle == nullptr)
		{
			Log.write(" Can't find vehicle with id " + toString(unitId), cLog::eLOG_TYPE_NET_WARNING);
			return;
		}

		if (vehicle->getOwner()->getId() != playerNr) return;

		if (vehicle->getMoveJob())
		{
			vehicle->getMoveJob()->resume();
		}
	}
}
