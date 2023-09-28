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

#include "actionattack.h"

#include "game/data/map/mapview.h"
#include "game/data/model.h"
#include "utility/log.h"

//------------------------------------------------------------------------------
cActionAttack::cActionAttack (const cUnit& aggressor, cPosition targetPosition, const cUnit* targetUnit) :
	agressorId (aggressor.getId()),
	targetPosition (targetPosition),
	targetId (targetUnit ? targetUnit->getId() : 0)
{}

//------------------------------------------------------------------------------
cActionAttack::cActionAttack (cBinaryArchiveIn& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionAttack::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	//validate aggressor
	cUnit* aggressor = model.getUnitFromID (agressorId);
	if (aggressor == nullptr || !aggressor->getOwner()) return;

	if (aggressor->getOwner()->getId() != playerNr) return;
	if (aggressor->isBeeingAttacked()) return;

	//validate target
	if (!model.getMap()->isValidPosition (targetPosition)) return;

	cPosition validatedTargetPosition = targetPosition;
	if (targetId != 0)
	{
		cUnit* target = model.getUnitFromID (targetId);
		if (target == nullptr) return;

		if (!target->isABuilding() && !target->getIsBig())
		{
			if (targetPosition != target->getPosition())
			{
				NetLog.debug (" cActionAttack: Target coords changed to " + toString (target->getPosition()) + " to match current unit position");
			}
			validatedTargetPosition = target->getPosition();
		}
	}

	// check if attack is possible
	cMapView mapView (model.getMap(), nullptr);
	if (aggressor->canAttackObjectAt (validatedTargetPosition, mapView, true) == false)
	{
		NetLog.warn (" cActionAttack: Attack is not possible");
		return;
	}
	model.addAttackJob (*aggressor, validatedTargetPosition);
}
