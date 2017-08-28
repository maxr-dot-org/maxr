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

#include "actionfinishbuild.h"

#include "game/data/model.h"

//------------------------------------------------------------------------------
cActionFinishBuild::cActionFinishBuild(const cVehicle& vehicle, const cPosition& escapePosition) :
	cAction(eActiontype::ACTION_FINISH_BUILD), 
	vehicleId(vehicle.getId()),
	escapePosition(escapePosition)
{};

//------------------------------------------------------------------------------
cActionFinishBuild::cActionFinishBuild(cBinaryArchiveOut& archive)
	: cAction(eActiontype::ACTION_FINISH_BUILD)
{
	serializeThis(archive);
}

//------------------------------------------------------------------------------
void cActionFinishBuild::execute(cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	auto map = model.getMap();

	cVehicle* vehicle = model.getVehicleFromID(vehicleId);
	if (vehicle == nullptr) return;
	if (vehicle->getOwner()->getId() != playerNr) return;

	if (!vehicle->isUnitBuildingABuilding() || vehicle->getBuildTurns() > 0) return;
	if (!map->isValidPosition(escapePosition)) return;
	if (!vehicle->isNextTo(escapePosition)) return;
	
	if (!map->possiblePlace(*vehicle, escapePosition))
	{
		//model.sideStepStealthUnit(escapePosition, *vehicle);
	}
	if (!map->possiblePlace(*vehicle, escapePosition)) return;

	model.addBuilding (vehicle->getPosition(), vehicle->getBuildingType(), vehicle->getOwner());

	// end building
	vehicle->setBuildingABuilding(false);
	vehicle->BuildPath = false;

	// set the vehicle to the border
	if (vehicle->getIsBig())
	{
		int x = vehicle->getPosition().x();
		int y = vehicle->getPosition().y();
		if (escapePosition.x() > vehicle->getPosition().x()) x++;
		if (escapePosition.y() > vehicle->getPosition().y()) y++;
		map->moveVehicle(*vehicle, cPosition(x, y));
	}

	// drive away from the building lot
	model.addMoveJob (*vehicle, escapePosition);
}
