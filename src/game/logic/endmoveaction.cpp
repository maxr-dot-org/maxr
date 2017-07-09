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
#include "server.h"
#include "serverevents.h"


cEndMoveAction::cEndMoveAction (cVehicle* vehicle, int destID, eEndMoveActionType type)
{
	destID_ = destID;
	type_ = type;
	vehicle_ = vehicle;
}

void cEndMoveAction::execute (cServer& server)
{
	switch (type_)
	{
		case EMAT_LOAD: executeLoadAction (server); break;
		case EMAT_GET_IN: executeGetInAction (server); break;
		case EMAT_ATTACK: executeAttackAction (server); break;
	}
}

void cEndMoveAction::executeLoadAction (cServer& server)
{
	cVehicle* destVehicle = server.getVehicleFromID (destID_);
	if (!destVehicle) return;

	if (vehicle_->canLoad (destVehicle) == false) return;

	vehicle_->storeVehicle (*destVehicle, *server.Map);
	if (destVehicle->getMoveJob()) destVehicle->getMoveJob()->stop();

	// vehicle is removed from enemy clients by cServer::checkPlayerUnits()
	sendStoreVehicle (server, vehicle_->iID, true, destVehicle->iID, *vehicle_->getOwner());
}

void cEndMoveAction::executeGetInAction (cServer& server)
{
	cVehicle* destVehicle = server.getVehicleFromID (destID_);
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
}

void cEndMoveAction::executeAttackAction (cServer& server)
{
	// get the target unit
	const cUnit* destUnit = server.getUnitFromID (destID_);
	if (destUnit == nullptr) return;

	const auto& position = destUnit->getPosition();
	cMap& map = *server.Map;

	// check, whether the attack is now possible
	if (!vehicle_->canAttackObjectAt (position, map, true, true)) return;

	// is the target in sight?
	if (!vehicle_->getOwner()->canSeeAnyAreaUnder (*destUnit)) return;

	server.addAttackJob (vehicle_, position);
}


