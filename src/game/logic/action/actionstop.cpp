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
cActionStop::cActionStop(const cUnit& unit) :
	cAction(eActiontype::ACTION_STOP), 
	unitId(unit.getId())
{};

//------------------------------------------------------------------------------
cActionStop::cActionStop(cBinaryArchiveOut& archive)
	: cAction(eActiontype::ACTION_STOP)
{
	serializeThis(archive);
}

//------------------------------------------------------------------------------
void cActionStop::execute(cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	cUnit* unit = model.getUnitFromID(unitId);
	if (unit == nullptr) return;
	if (unit->getOwner()->getId() != playerNr) return;

	if (unit->isABuilding())
	{
		auto b = static_cast<cBuilding*>(unit);

		b->stopWork();
	}
	else
	{
		auto vehicle = static_cast<cVehicle*>(unit);

		if (vehicle->getMoveJob())
		{
			vehicle->getMoveJob()->stop();
		}
		else if (vehicle->isUnitBuildingABuilding())
		{
			if (vehicle->getBuildTurns() == 0) return; 

			vehicle->setBuildingABuilding(false);
			vehicle->BuildPath = false;

			if (vehicle->getIsBig())
			{
				model.getMap()->moveVehicle(*vehicle, vehicle->buildBigSavedPosition);
				vehicle->getOwner()->doScan();
			}
		}
	}
}
