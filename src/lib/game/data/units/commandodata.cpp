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

#include "commandodata.h"

#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/units/building.h"
#include "game/data/units/unit.h"
#include "game/data/units/vehicle.h"
#include "utility/crc.h"
#include "utility/position.h"

//-----------------------------------------------------------------------------
/** Checks if the target is on a neighbor field and if it can be stolen or disabled */
//------------------------------------------------------------------------------
/*static*/ bool cCommandoData::canDoAction (const cVehicle& commando, const cPosition& position, const cMapView& map, bool steal)
{
	const auto& field = map.getField (position);

	const cUnit* unit = field.getPlane();
	if (canDoAction (commando, unit, steal)) return true;

	unit = field.getVehicle();
	if (canDoAction (commando, unit, steal)) return true;

	unit = field.getBuilding();
	if (canDoAction (commando, unit, steal)) return true;

	return false;
}

/*static*/ bool cCommandoData::canDoAction (const cVehicle& commando, const cUnit* unit, bool steal)
{
	if (unit == nullptr) return false;

	if (commando.data.getShots() == 0) return false;
	if (unit->isNextTo (commando.getPosition()) == false) return false;
	if (unit->isABuilding() && static_cast<const cBuilding*> (unit)->isRubble()) return false;
	if (unit->getOwner() == commando.getOwner()) return false;
	if (unit->isAVehicle() && unit->getStaticUnitData().factorAir > 0 && static_cast<const cVehicle*> (unit)->getFlightHeight() > 0) return false;

	if (steal)
	{
		if (!unit->storedUnits.empty()) return false;
		return commando.getStaticData().canCapture && unit->getStaticUnitData().canBeCaptured;
	}
	else
	{
		if (unit->isDisabled()) return false;
		return commando.getStaticData().canDisable && unit->getStaticUnitData().canBeDisabled;
	}
}

//------------------------------------------------------------------------------
/*static*/ void cCommandoData::increaseXp (cVehicle& commando)
{
	++commando.getCommandoData().successCount;
	commando.statusChanged();
}

//------------------------------------------------------------------------------
/*static*/ int cCommandoData::getLevel (std::uint32_t numberOfSuccess)
{
	auto rank = 0.f;

	// The higher his level is the slower he gains exp.
	// Every 5 rankings he needs one successful disabling more,
	// to get to the next ranking
	for (std::uint32_t i = 0; i != numberOfSuccess; ++i)
	{
		rank = rank + 1.f / (((int) rank + 5) / 5);
	}
	return rank;
}

//------------------------------------------------------------------------------
int cCommandoData::computeChance (const cUnit* destUnit, bool steal) const
{
	if (destUnit == nullptr)
		return 0;

	// TODO: include cost research and clan modifications ?
	// Or should always the basic version without clanmods be used ?
	// TODO: Bug for buildings ? /3? or correctly /2,
	// because constructing buildings takes two resources per turn ?
	const int destTurn = destUnit->data.getBuildCost() / 3;

	const int factor = steal ? 1 : 4;
	const int srcLevel = getLevel (successCount) + 7;

	// The chance to disable or steal a unit depends on
	// the infiltrator ranking and the buildcosts
	// (or 'turns' in the original game) of the target unit.
	// The chance rises linearly with a higher ranking of the infiltrator.
	// The chance of a inexperienced infiltrator will be calculated like
	// he has the ranking 7.
	// Disabling has a 4 times higher chance than stealing.
	int chance = 100 * factor * (8 * srcLevel) / (35 * destTurn);
	chance = std::min (90, chance);

	return chance;
}

//------------------------------------------------------------------------------
int cCommandoData::computeDisabledTurnCount (const cUnit& destUnit) const
{
	int destTurn, srcLevel;

	if (destUnit.isAVehicle())
	{
		const int vehiclesTable[13] = {0, 0, 0, 5, 8, 3, 3, 0, 0, 0, 1, 0, -4};
		destTurn = destUnit.data.getBuildCost() / 3;
		srcLevel = getLevel (successCount);
		if (destTurn > 0 && destTurn < 13)
			srcLevel += vehiclesTable[destTurn];
	}
	else
	{
		destTurn = destUnit.data.getBuildCost() / 2;
		srcLevel = getLevel (successCount) + 8;
	}

	int turns = srcLevel / destTurn;
	turns = std::max (turns, 1);
	return turns;
}

//------------------------------------------------------------------------------
std::string cCommandoData::getDebugString() const
{
	return "commando_success: " + std::to_string (successCount);
}

//------------------------------------------------------------------------------
std::uint32_t cCommandoData::calcCheckSum (std::uint32_t crc) const
{
	return ::calcCheckSum (successCount, crc);
}
