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

#include "actionstop.h"

#include "game/data/model.h"

//------------------------------------------------------------------------------
cActionStop::cActionStop (const cUnit& unit) :
	unitId (unit.getId())
{}

//------------------------------------------------------------------------------
cActionStop::cActionStop (cBinaryArchiveIn& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionStop::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	cUnit* unit = model.getUnitFromID (unitId);
	if (unit == nullptr || !unit->getOwner()) return;
	if (unit->getOwner()->getId() != playerNr) return;

	if (unit->isABuilding())
	{
		auto b = static_cast<cBuilding*> (unit);

		b->stopWork();
	}
	else
	{
		auto vehicle = static_cast<cVehicle*> (unit);

		if (vehicle->getMoveJob())
		{
			vehicle->getMoveJob()->stop (*vehicle);
		}
		else if (vehicle->isUnitBuildingABuilding())
		{
			if (vehicle->getBuildTurns() == 0) return;

			vehicle->setBuildingABuilding (false);
			vehicle->bandPosition.reset();

			if (vehicle->buildBigSavedPosition)
			{
				vehicle->getOwner()->updateScan (*vehicle, *vehicle->buildBigSavedPosition);
				model.getMap()->moveVehicle (*vehicle, *vehicle->buildBigSavedPosition);
			}
		}
		else if (vehicle->isUnitClearing())
		{
			vehicle->setClearing (false);
			vehicle->setClearingTurns (0);

			if (vehicle->buildBigSavedPosition)
			{
				vehicle->getOwner()->updateScan (*vehicle, *vehicle->buildBigSavedPosition);
				model.getMap()->moveVehicle (*vehicle, *vehicle->buildBigSavedPosition);
			}
		}
	}
}
