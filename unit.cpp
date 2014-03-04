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

#include <algorithm>

#include "unit.h"

#include "attackJobs.h"
#include "client.h"
#include "clientevents.h"
#include "map.h"
#include "player.h"

#include "buildings.h"
#include "vehicles.h"

#include "utility/position.h"
#include "utility/box.h"

using namespace std;

//------------------------------------------------------------------------------
cUnit::cUnit (const sUnitData* unitData, cPlayer* owner, unsigned int ID)
	: iID (ID)
	, PosX (0)
	, PosY (0)
	, dir (0)
	, turnsDisabled (0)
	, sentryActive (false)
	, manualFireActive (false)
	, attacking (false)
	, beeingAttacked (false)
	, markedAsDone (false)
	, beenAttacked (false)
	, owner (owner)
	, job (NULL)
	, lockerPlayer (NULL)
	, isOriginalName (true)
{
	if (unitData != 0)
		data = *unitData;

	disabledChanged.connect ([&](){ statusChanged (); });
	sentryChanged.connect ([&](){ statusChanged (); });
	manualFireChanged.connect ([&](){ statusChanged (); });
	attackingChanged.connect ([&](){ statusChanged (); });
	beeingAttackedChanged.connect ([&](){ statusChanged (); });

	layingMinesChanged.connect ([&](){ statusChanged (); });
	clearingMinesChanged.connect ([&](){ statusChanged (); });
	buildingChanged.connect ([&](){ statusChanged (); });
	clearingChanged.connect ([&](){ statusChanged (); });
	workingChanged.connect ([&](){ statusChanged (); });
}

//------------------------------------------------------------------------------
cUnit::~cUnit()
{
	deleteStoredUnits();
	if (lockerPlayer)
	{
		lockerPlayer->deleteLock (*this);
	}
}

//------------------------------------------------------------------------------
/** returns the remaining hitpoints after an attack */
//------------------------------------------------------------------------------
int cUnit::calcHealth (int damage) const
{
	damage -= data.armor;

	// minimum damage is 1
	damage = std::max (1, damage);

	const int hp = data.hitpointsCur - damage;
	return std::max (0, hp);
}

//------------------------------------------------------------------------------
/** Checks if the target is in range */
//------------------------------------------------------------------------------
bool cUnit::isInRange (int x, int y) const
{
	x -= PosX;
	y -= PosY;

	return (Square (x) + Square (y)) <= Square (data.range);
}

//------------------------------------------------------------------------------
bool cUnit::isNextTo (int x, int y) const
{
	if (x + 1 < PosX || y + 1 < PosY)
		return false;

	const int size = data.isBig ? 2 : 1;

	if (x - size > PosX || y - size > PosY)
		return false;
	return true;
}

//------------------------------------------------------------------------------
bool cUnit::isNextTo (const cPosition& position) const
{
	return isNextTo (position.x (), position.y ());
}

//------------------------------------------------------------------------------
bool cUnit::isAbove(const cPosition& position) const
{
	return getArea().withinOrTouches(position);
}

//------------------------------------------------------------------------------
cBox<cPosition> cUnit::getArea() const
{
	return cBox<cPosition>(cPosition(PosX, PosY), cPosition(data.isBig ? PosX + 1 : PosX, data.isBig ? PosY + 1 : PosY));
}

// http://rosettacode.org/wiki/Roman_numerals/Encode#C.2B.2B
static std::string to_roman (unsigned int value)
{
	struct romandata_t { unsigned int value; char const* numeral; };
	const struct romandata_t romandata[] =
	{
		//{1000, "M"}, {900, "CM"},
		//{500, "D"}, {400, "CD"},
		{100, "C"}, { 90, "XC"},
		{ 50, "L"}, { 40, "XL"},
		{ 10, "X"}, { 9, "IX"},
		{ 5, "V"}, { 4, "IV"},
		{ 1, "I"},
		{ 0, NULL} // end marker
	};

	std::string result;
	for (const romandata_t* current = romandata; current->value > 0; ++current)
	{
		while (value >= current->value)
		{
			result += current->numeral;
			value -= current->value;
		}
	}
	return result;
}

//------------------------------------------------------------------------------
/** generates the name for the unit depending on the versionnumber */
//------------------------------------------------------------------------------
string cUnit::getNamePrefix() const
{
	string rome = "MK ";
	// +1, because the numbers in the name start at 1, not at 0
	unsigned int nr = data.version + 1;

	return "MK " + to_roman (nr);
}

//------------------------------------------------------------------------------
/** Returns the name of the vehicle how it should be displayed */
//------------------------------------------------------------------------------
string cUnit::getDisplayName() const
{
	return getNamePrefix() + " " + (isNameOriginal() ? data.name : name);
}

//------------------------------------------------------------------------------
/** changes the name of the unit and indicates it as "not default" */
//------------------------------------------------------------------------------
void cUnit::changeName (const string& newName)
{
	name = newName;
	isOriginalName = false;
	renamed ();
}

//------------------------------------------------------------------------------
/** rotates the unit to the given direction */
//------------------------------------------------------------------------------
void cUnit::rotateTo (int newDir)
{
	if (newDir < 0 || newDir >= 8 || newDir == dir)
		return;

	int t = dir;
	int dest = 0;

	for (int i = 0; i < 8; ++i)
	{
		if (t == newDir)
		{
			dest = i;
			break;
		}
		++t;

		if (t > 7)
			t = 0;
	}

	if (dest < 4)
		++dir;
	else
		--dir;

	if (dir < 0)
		dir += 8;
	else
	{
		if (dir > 7)
			dir -= 8;
	}
}

//------------------------------------------------------------------------------
/** Checks, if the unit can attack an object at the given coordinates*/
//------------------------------------------------------------------------------
bool cUnit::canAttackObjectAt (int x, int y, const cMap& map, bool forceAttack, bool checkRange) const
{
	if (data.canAttack == false) return false;
	if (data.shotsCur <= 0) return false;
	if (data.ammoCur <= 0) return false;
	if (attacking) return false;
	if (isBeeingAttacked ()) return false;
	if (isAVehicle() && static_cast<const cVehicle*> (this)->isUnitLoaded()) return false;
	if (map.isValidPos (x, y) == false) return false;
	if (checkRange && isInRange (x, y) == false) return false;
	const int off = map.getOffset (x, y);

	if (data.muzzleType == sUnitData::MUZZLE_TYPE_TORPEDO && map.isWaterOrCoast (x, y) == false)
		return false;

	const cUnit* target = selectTarget (x, y, data.canAttack, map);

	if (target && target->iID == iID)  // a unit cannot fire on itself
		return false;

	if (owner->ScanMap[off] == false && !forceAttack)
		return false;

	if (forceAttack)
		return true;

	if (target == NULL)
		return false;

	// do not fire on e.g. platforms, connectors etc.
	// see ticket #436 on bug tracker
	if (target->isABuilding() && isAVehicle() && data.factorAir == 0 && map.possiblePlace (*static_cast<const cVehicle*> (this), x, y))
		return false;

	if (target->owner == owner)
		return false;

	return true;
}

//------------------------------------------------------------------------------
void cUnit::upgradeToCurrentVersion()
{
	if (owner == NULL) return;
	const sUnitData* upgradeVersion = owner->getUnitDataCurrentVersion (data.ID);
	if (upgradeVersion == NULL) return;

	data.version = upgradeVersion->version;

	// TODO: check behaviour in original
	if (data.hitpointsCur == data.hitpointsMax)
		data.hitpointsCur = upgradeVersion->hitpointsMax;
	data.hitpointsMax = upgradeVersion->hitpointsMax;

	// don't change the current ammo-amount!
	data.ammoMax = upgradeVersion->ammoMax;

	data.speedMax = upgradeVersion->speedMax;

	data.armor = upgradeVersion->armor;
	data.scan = upgradeVersion->scan;
	data.range = upgradeVersion->range;
	// TODO: check behaviour in original
	data.shotsMax = upgradeVersion->shotsMax;
	data.damage = upgradeVersion->damage;
	data.buildCosts = upgradeVersion->buildCosts;
}

//------------------------------------------------------------------------------
void cUnit::deleteStoredUnits()
{
	for (size_t i = 0; i != storedUnits.size(); ++i)
	{
		cVehicle* vehicle = storedUnits[i];
		remove_from_intrusivelist (vehicle->owner->VehicleList, *vehicle);
		vehicle->deleteStoredUnits();

		delete vehicle;
	}
	storedUnits.clear();
}

//------------------------------------------------------------------------------
void cUnit::setDisabledTurns (int turns)
{
	std::swap (turnsDisabled, turns);
	if (turns != turnsDisabled) disabledChanged ();
}

//------------------------------------------------------------------------------
void cUnit::setSentryActive (bool value)
{
	std::swap (sentryActive, value);
	if (value != sentryActive) sentryChanged ();
}

//------------------------------------------------------------------------------
void cUnit::setManualFireActive (bool value)
{
	std::swap (manualFireActive, value);
	if (value != manualFireActive) manualFireChanged ();
}

//------------------------------------------------------------------------------
void cUnit::setAttacking (bool value)
{
	std::swap (attacking, value);
	if (value != attacking) attackingChanged ();
}

//------------------------------------------------------------------------------
void cUnit::setIsBeeinAttacked (bool value)
{
	std::swap (beeingAttacked, value);
	if (value != beeingAttacked) beeingAttackedChanged ();
}

//------------------------------------------------------------------------------
void cUnit::setMarkedAsDone (bool value)
{
	std::swap (markedAsDone, value);
	if (value != markedAsDone) markedAsDoneChanged ();
}

//------------------------------------------------------------------------------
void cUnit::setHasBeenAttacked (bool value)
{
	std::swap (beenAttacked, value);
	if (value != beenAttacked) beenAttackedChanged ();
}

//------------------------------------------------------------------------------
int cUnit::getDisabledTurns () const
{
	return turnsDisabled;
}

//------------------------------------------------------------------------------
bool cUnit::isSentryActive () const
{
	return sentryActive;
}

//------------------------------------------------------------------------------
bool cUnit::isManualFireActive () const
{
	return manualFireActive;
}

//------------------------------------------------------------------------------
bool cUnit::isAttacking () const
{
	return attacking;
}

//------------------------------------------------------------------------------
bool cUnit::isBeeingAttacked () const
{
	return beeingAttacked;
}

//------------------------------------------------------------------------------
bool cUnit::isMarkedAsDone () const
{
	return markedAsDone;
}

//------------------------------------------------------------------------------
bool cUnit::hasBeenAttacked () const
{
	return beenAttacked;
}