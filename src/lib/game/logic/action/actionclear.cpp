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

#include "actionclear.h"

#include "game/data/model.h"
#include "game/logic/jobs/startbuildjob.h"
#include "game/logic/jobs/stealthstepwaitjob.h"

//------------------------------------------------------------------------------
cActionClear::cActionClear (const cVehicle& vehicle) :
	vehicleId (vehicle.getId())
{}

//------------------------------------------------------------------------------
cActionClear::cActionClear (cBinaryArchiveIn& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionClear::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	auto vehicle = model.getVehicleFromID (vehicleId);
	if (vehicle == nullptr) return;

	if (!vehicle->getOwner()) return;
	if (vehicle->getOwner()->getId() != playerNr) return;

	if (!vehicle->getStaticData().canClearArea) return;
	if (vehicle->isUnitMoving()) return;

	auto map = model.getMap();
	auto rubble = model.getMap()->getField (vehicle->getPosition()).getRubble();
	if (!rubble) return;

	cPosition oldPosition = vehicle->getPosition();
	if (rubble->getIsBig())
	{
		auto rubblePosition = rubble->getPosition();

		std::vector<cVehicle*> stealthVehicles;
		for (const cPosition& offset : {cPosition (0, 0), cPosition (1, 0), cPosition (0, 1), cPosition (1, 1)})
		{
			if (auto* stealthVehicle = model.sideStepStealthUnit (rubblePosition + offset, *vehicle, rubblePosition))
			{
				stealthVehicles.push_back (stealthVehicle);
			}
		}
		if (!stealthVehicles.empty())
		{
			// the clearing is started, when the stealth units have made their steps
			model.addJob (std::make_unique<cStealthStepWaitJob> (*vehicle, stealthVehicles, this));
			return;
		}

		if ((!map->possiblePlace (*vehicle, rubblePosition, false) && rubblePosition != vehicle->getPosition()) || (!map->possiblePlace (*vehicle, rubblePosition + cPosition (1, 0), false) && rubblePosition + cPosition (1, 0) != vehicle->getPosition()) || (!map->possiblePlace (*vehicle, rubblePosition + cPosition (0, 1), false) && rubblePosition + cPosition (0, 1) != vehicle->getPosition()) || (!map->possiblePlace (*vehicle, rubblePosition + cPosition (1, 1), false) && rubblePosition + cPosition (1, 1) != vehicle->getPosition()))
		{
			return;
		}

		vehicle->getOwner()->updateScan (*vehicle, rubblePosition, true);
		map->moveVehicleBig (*vehicle, rubblePosition);
	}

	vehicle->setClearing (true);
	vehicle->setClearingTurns (rubble->getIsBig() ? 4 : 1);

	model.addJob (std::make_unique<cStartBuildJob> (*vehicle, oldPosition, rubble->getIsBig()));
}
