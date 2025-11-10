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

#include "unitlocklist.h"

#include "game/data/map/mapfieldview.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"

#include <algorithm>

//------------------------------------------------------------------------------
void cUnitLockList::setPlayer (const cPlayer* player_)
{
	player = player_;
}

//------------------------------------------------------------------------------
void cUnitLockList::toggleLockAt (const cMapFieldView& field)
{
	const cUnit* unit = nullptr;
	if (field.getBaseBuilding() && (!player || field.getBaseBuilding()->getOwner() != player))
	{
		unit = field.getBaseBuilding();
	}
	else if (field.getTopBuilding() && (!player || field.getTopBuilding()->getOwner() != player))
	{
		unit = field.getTopBuilding();
	}
	if (field.getVehicle() && (!player || field.getVehicle()->getOwner() != player))
	{
		unit = field.getVehicle();
	}
	if (field.getPlane() && (!player || field.getPlane()->getOwner() != player))
	{
		unit = field.getPlane();
	}
	if (unit == nullptr) return;

	auto iter = std::ranges::find_if (lockedUnits, [unit] (const std::pair<const cUnit*, cSignalConnectionManager>& entry) { return entry.first == unit; });
	if (iter == lockedUnits.end())
	{
		lockedUnits.push_back (std::make_pair (unit, cSignalConnectionManager()));
		lockedUnits.back().second.connect (unit->destroyed, [this, unit]() {
			auto iter = std::ranges::find_if (lockedUnits, [unit] (const std::pair<const cUnit*, cSignalConnectionManager>& entry) { return entry.first == unit; });
			if (iter != lockedUnits.end())
			{
				lockedUnits.erase (iter);
			}
		});
	}
	else
	{
		lockedUnits.erase (iter);
	}
}

//------------------------------------------------------------------------------
size_t cUnitLockList::getLockedUnitsCount() const
{
	return lockedUnits.size();
}

//------------------------------------------------------------------------------
const cUnit* cUnitLockList::getLockedUnit (size_t index) const
{
	return lockedUnits[index].first;
}

//------------------------------------------------------------------------------
void cUnitLockList::unlockAll()
{
	lockedUnits.clear();
}

//------------------------------------------------------------------------------
void cUnitLockList::lockUnit (const cUnit& unit)
{
	auto iter = std::ranges::find_if (lockedUnits, [&unit] (const std::pair<const cUnit*, cSignalConnectionManager>& entry) { return entry.first == &unit; });
	if (iter == lockedUnits.end())
	{
		lockedUnits.push_back (std::make_pair (&unit, cSignalConnectionManager()));
		lockedUnits.back().second.connect (unit.destroyed, [this, &unit]() {
			auto iter = std::ranges::find_if (lockedUnits, [&unit] (const std::pair<const cUnit*, cSignalConnectionManager>& entry) { return entry.first == &unit; });
			if (iter != lockedUnits.end())
			{
				lockedUnits.erase (iter);
			}
		});
	}
}
