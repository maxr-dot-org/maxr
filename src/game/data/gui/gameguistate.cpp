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

#include "gameguistate.h"

#include "game/data/gui/unitlocklist.h"
#include "game/data/gui/unitselection.h"
#include "game/data/units/unit.h"

//------------------------------------------------------------------------------
void cGameGuiState::setSelectedUnits (const cUnitSelection& unitSelection)
{
	selectedUnitIds = ranges::Transform (unitSelection.getSelectedUnits(), [] (const cUnit* unit) { return unit->iID; });
}

//------------------------------------------------------------------------------
const std::vector<unsigned int>& cGameGuiState::getSelectedUnitIds() const
{
	return selectedUnitIds;
}

//------------------------------------------------------------------------------
void cGameGuiState::setLockedUnits (const cUnitLockList& unitLockList)
{
	lockedUnitIds.clear();
	for (size_t i = 0; i < unitLockList.getLockedUnitsCount(); ++i)
	{
		lockedUnitIds.push_back (unitLockList.getLockedUnit (i)->iID);
	}
}

//------------------------------------------------------------------------------
const std::vector<unsigned int>& cGameGuiState::getLockedUnitIds() const
{
	return lockedUnitIds;
}
