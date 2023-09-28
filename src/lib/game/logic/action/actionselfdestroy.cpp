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

#include "actionselfdestroy.h"

#include "game/data/model.h"
#include "utility/log.h"

//------------------------------------------------------------------------------
cActionSelfDestroy::cActionSelfDestroy (const cBuilding& unit) :
	unitId (unit.getId())
{}

//------------------------------------------------------------------------------
cActionSelfDestroy::cActionSelfDestroy (cBinaryArchiveIn& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionSelfDestroy::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	cBuilding* b = model.getBuildingFromID (unitId);
	if (b == nullptr || !b->getOwner()) return;
	if (b->getOwner()->getId() != playerNr) return;

	if (b->isBeeingAttacked()) return;

	// special case: when a land/sea mine explodes, it makes damage according to
	// its attack points. So start an attack job instead of just destoying it.
	if (b->getStaticData().explodesOnContact)
	{
		model.addAttackJob (*b, b->getPosition());
	}
	else
	{
		b->getOwner()->getGameOverStat().lostBuildingsCount++;
		model.destroyUnit (*b);
	}
}
