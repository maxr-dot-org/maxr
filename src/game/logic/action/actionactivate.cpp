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

#include "actionactivate.h"

#include "game/data/model.h"

#include "utility/log.h"
#include "utility/listhelpers.h"

//------------------------------------------------------------------------------
cActionActivate::cActionActivate(const cUnit& containingUnit, const cVehicle& activatedVehicle, const cPosition& position) :
	cAction(eActiontype::ACTION_ACTIVATE),
	position(position),
	containingUnitId(containingUnit.getId()),
	activatedVehicleId(activatedVehicle.getId())
{};

//------------------------------------------------------------------------------
cActionActivate::cActionActivate(cBinaryArchiveOut& archive) :
	cAction(eActiontype::ACTION_ACTIVATE)
{
	serializeThis(archive);
}

//------------------------------------------------------------------------------
void cActionActivate::execute(cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	cUnit* containingUnit = model.getUnitFromID(containingUnitId);
	if (!containingUnit) return;

	cVehicle* activatedVehicle = model.getVehicleFromID(activatedVehicleId);
	if (!activatedVehicle) return;

	if (!model.getMap()->isValidPosition(position)) return;
	if (!containingUnit->isNextTo(position)) return;
	if (!Contains(containingUnit->storedUnits, activatedVehicle)) return;

	if (containingUnit->canExitTo(position, *model.getMap(), activatedVehicle->getStaticUnitData()))
	{
		//TODO: model.sideStepStealthUnit
		containingUnit->exitVehicleTo(*activatedVehicle, position, *model.getMap());

		if (activatedVehicle->getStaticUnitData().canSurvey)
		{
			activatedVehicle->doSurvey();
		}

		//TODO: plane takeoff animation?
		if (activatedVehicle->canLand(*model.getMap()))
		{
			activatedVehicle->setFlightHeight(0);
		}
		else
		{
			activatedVehicle->setFlightHeight(64);
		}

		//TODO: detection
		
		model.unitActivated(*containingUnit, *activatedVehicle);
	}
}
