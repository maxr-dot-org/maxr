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


#include "endmoveaction.h"

#include "game/data/units/vehicle.h"
#include "game/data/map/mapview.h"
#include "game/data/model.h"
#include "utility/crc.h"


cEndMoveAction::cEndMoveAction (const cVehicle& vehicle, const cUnit& destUnit, eEndMoveActionType type) :
	vehicleID(vehicle.getId()),
	type(type),
	destID(destUnit.getId())
{}

//------------------------------------------------------------------------------
cEndMoveAction::cEndMoveAction  () :
	vehicleID(-1),
	type(EMAT_NONE),
	destID(-1)
{}

//------------------------------------------------------------------------------
void cEndMoveAction::execute (cModel& model)
{
	switch (type)
	{
		case EMAT_LOAD: 
			executeLoadAction (model); 
			break;
		case EMAT_GET_IN:
			executeGetInAction (model);
			break;
		case EMAT_ATTACK: 
			executeAttackAction (model); 
			break;
		default:
			break;
	}
}

//------------------------------------------------------------------------------
eEndMoveActionType cEndMoveAction::getType() const
{
	return type;
}

//------------------------------------------------------------------------------
uint32_t cEndMoveAction::getChecksum(uint32_t crc) const
{
	crc = calcCheckSum(vehicleID, crc);
	crc = calcCheckSum(type, crc);
	crc = calcCheckSum(destID, crc);

	return crc;
}

//------------------------------------------------------------------------------
void cEndMoveAction::executeLoadAction (cModel& model)
{
/*	cVehicle* destVehicle = server.getVehicleFromID (destID_);
	if (!destVehicle) return;

	if (vehicle_->canLoad (destVehicle) == false) return;

	vehicle_->storeVehicle (*destVehicle, *server.Map);
	if (destVehicle->getMoveJob()) destVehicle->getMoveJob()->stop();

	// vehicle is removed from enemy clients by cServer::checkPlayerUnits()
	sendStoreVehicle (server, vehicle_->iID, true, destVehicle->iID, *vehicle_->getOwner());
*/
}

//------------------------------------------------------------------------------
void cEndMoveAction::executeGetInAction (cModel& model)
{
/*	cVehicle* destVehicle = server.getVehicleFromID (destID_);
	cBuilding* destBuilding = server.getBuildingFromID (destID_);

	// execute the loading if possible
	if (destVehicle && destVehicle->canLoad (vehicle_))
	{
		destVehicle->storeVehicle (*vehicle_, *server.Map);
		if (destVehicle->getMoveJob()) destVehicle->getMoveJob()->stop();
		//vehicle is removed from enemy clients by cServer::checkPlayerUnits()
		sendStoreVehicle (server, destVehicle->iID, true, vehicle_->iID, *destVehicle->getOwner());
	}
	else if (destBuilding && destBuilding->canLoad (vehicle_))
	{
		destBuilding->storeVehicle (*vehicle_, *server.Map);
		if (destVehicle->getMoveJob()) destVehicle->getMoveJob()->stop();
		// vehicle is removed from enemy clients by cServer::checkPlayerUnits()
		sendStoreVehicle (server, destBuilding->iID, false, vehicle_->iID, *destBuilding->getOwner());
	}
*/
}

//------------------------------------------------------------------------------
void cEndMoveAction::executeAttackAction (cModel& model)
{
	// get the vehicle
	cUnit* vehicle = model.getUnitFromID(vehicleID);
	if (vehicle == nullptr) return;

	// get the target unit
	const cUnit* destUnit = model.getUnitFromID (destID);
	if (destUnit == nullptr) return;

	const auto& position = destUnit->getPosition();
	cMapView mapView(model.getMap(), nullptr);

	// check, whether the attack is now possible
	if (!vehicle->canAttackObjectAt (position, mapView, true, true)) return;

	// is the target in sight?
	if (!vehicle->getOwner()->canSeeUnit (*destUnit, *model.getMap())) return;

	model.addAttackJob (*vehicle, position);
}
