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

#include "actionchangesentry.h"
#include "game/data/model.h"

//------------------------------------------------------------------------------
cActionChangeSentry::cActionChangeSentry(const cUnit& unit) :
	cAction(eActiontype::ACTION_CHANGE_SENTRY), 
	unitId(unit.getId())
{};

//------------------------------------------------------------------------------
cActionChangeSentry::cActionChangeSentry(cBinaryArchiveOut& archive)
	: cAction(eActiontype::ACTION_CHANGE_SENTRY)
{
	serializeThis(archive);
}

//------------------------------------------------------------------------------
void cActionChangeSentry::execute(cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	cUnit* unit = model.getUnitFromID(unitId);
	if (unit == nullptr) return;
	if (unit->getOwner()->getId() != playerNr) return;

	if (unit->isSentryActive())
	{
		unit->getOwner()->deleteSentry(*unit);
	}
	else
	{
		unit->getOwner()->addSentry(*unit);
		unit->setManualFireActive(false);
	}
}
