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

#include "game/data/map/mapview.h"
#include "game/data/model.h"
#include "game/data/units/vehicle.h"
#include "jobs/airtransportloadjob.h"
#include "jobs/getinjob.h"
#include "utility/crc.h"

//------------------------------------------------------------------------------
/* static */ cEndMoveAction cEndMoveAction::None()
{
	return {};
}
//------------------------------------------------------------------------------
/* static */ cEndMoveAction cEndMoveAction::Attacking (const cUnit& destUnit)
{
	return {destUnit, eEndMoveActionType::Attack};
}

//------------------------------------------------------------------------------
/* static */ cEndMoveAction cEndMoveAction::GetIn (const cUnit& destUnit)
{
	return {destUnit, eEndMoveActionType::GetIn};
}

//------------------------------------------------------------------------------
/* static */ cEndMoveAction cEndMoveAction::Load (const cUnit& destUnit)
{
	return {destUnit, eEndMoveActionType::Load};
}

//------------------------------------------------------------------------------
cEndMoveAction::cEndMoveAction (const cUnit& destUnit, eEndMoveActionType type) :
	endMoveAction (type),
	destID (destUnit.getId())
{}

//------------------------------------------------------------------------------
cEndMoveAction::cEndMoveAction() :
	endMoveAction (eEndMoveActionType::None)
{}

//------------------------------------------------------------------------------
void cEndMoveAction::execute (cModel& model, cVehicle& vehicle)
{
	switch (endMoveAction)
	{
		case eEndMoveActionType::GetIn:
			executeGetInAction (model, vehicle);
			break;
		case eEndMoveActionType::Load:
			executeLoadAction (model, vehicle);
			break;
		case eEndMoveActionType::Attack:
			executeAttackAction (model, vehicle);
			break;
		default:
			break;
	}
}

//------------------------------------------------------------------------------
uint32_t cEndMoveAction::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (endMoveAction, crc);
	crc = calcCheckSum (destID, crc);

	return crc;
}

//------------------------------------------------------------------------------
void cEndMoveAction::executeGetInAction (cModel& model, cVehicle& vehicle)
{
	// get the target unit
	cUnit* transporter = model.getUnitFromID (destID);
	if (transporter == nullptr) return;

	if (!transporter->canLoad (&vehicle)) return;

	if (transporter->getStaticUnitData().factorAir > 0)
	{
		model.addJob (std::make_unique<cAirTransportLoadJob> (vehicle, *transporter));
	}
	else
	{
		model.addJob (std::make_unique<cGetInJob> (vehicle, *transporter));
	}
}

//------------------------------------------------------------------------------
void cEndMoveAction::executeLoadAction (cModel& model, cVehicle& vehicle)
{
	// get the target unit
	cVehicle* cargo = model.getVehicleFromID (destID);
	if (cargo == nullptr) return;

	if (!vehicle.canLoad (cargo)) return;

	if (vehicle.getStaticUnitData().factorAir > 0)
	{
		model.addJob (std::make_unique<cAirTransportLoadJob> (*cargo, vehicle));
	}
	else
	{
		model.addJob (std::make_unique<cGetInJob> (*cargo, vehicle));
	}
}

//------------------------------------------------------------------------------
void cEndMoveAction::executeAttackAction (cModel& model, cVehicle& vehicle)
{
	if (!vehicle.getOwner()) return;

	// get the target unit
	const cUnit* destUnit = model.getUnitFromID (destID);
	if (destUnit == nullptr) return;

	const auto& position = destUnit->getPosition();
	cMapView mapView (model.getMap(), nullptr);

	// check, whether the attack is now possible
	if (!vehicle.canAttackObjectAt (position, mapView, true, true)) return;

	// is the target in sight?
	if (!vehicle.getOwner()->canSeeUnit (*destUnit, *model.getMap())) return;

	model.addAttackJob (vehicle, position);
}
