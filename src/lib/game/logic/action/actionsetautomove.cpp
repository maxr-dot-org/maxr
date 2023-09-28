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

#include "actionsetautomove.h"

#include "game/data/model.h"

//------------------------------------------------------------------------------
cActionSetAutoMove::cActionSetAutoMove (const cVehicle& vehicle, bool autoMoveActive) :
	vehicleId (vehicle.getId()),
	autoMoveActive (autoMoveActive)
{}

//------------------------------------------------------------------------------
cActionSetAutoMove::cActionSetAutoMove (cBinaryArchiveIn& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionSetAutoMove::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	cVehicle* surveyor = model.getVehicleFromID (vehicleId);
	if (!surveyor || !surveyor->getOwner()) return;

	if (surveyor->getOwner()->getId() != playerNr) return;
	if (!surveyor->getStaticData().canSurvey) return;

	surveyor->setSurveyorAutoMoveActive (autoMoveActive);
}
