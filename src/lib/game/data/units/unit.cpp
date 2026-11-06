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

#include "game/data/units/unit.h"

#include "game/data/map/map.h"
#include "game/data/map/mapview.h"
#include "game/data/model.h"
#include "game/data/player/player.h"
#include "game/data/units/vehicle.h"
#include "game/logic/attackjob.h"
#include "utility/box.h"
#include "utility/crc.h"
#include "utility/mathtools.h"
#include "utility/position.h"
#include "utility/ranges.h"

#include <algorithm>

//------------------------------------------------------------------------------
cUnit::cUnit (const cDynamicUnitData* unitData, const cStaticUnitData* staticData, cPlayer* owner, unsigned int ID) :
	iID (ID),
	staticData (staticData),
	owner (owner)
{
	if (unitData == nullptr)
	{
		if (staticData)
		{
			data.setId (staticData->ID);
		}
	}
	else
	{
		data = *unitData;
	}

	data.setMaximumCurrentValues();

	disabledChanged.connect ([this]() { statusChanged(); });
	sentryChanged.connect ([this]() { statusChanged(); });
	manualFireChanged.connect ([this]() { statusChanged(); });
	attackingChanged.connect ([this]() { statusChanged(); });
	beingAttackedChanged.connect ([this]() { statusChanged(); });
}

//------------------------------------------------------------------------------
cUnit::~cUnit()
{
	destroyed();
}

//------------------------------------------------------------------------------
void cUnit::postLoad (cModel& model)
{
	//restore pointer to static unit data
	if (data.getId() == sID (0, 0)) // workaround for old savegame
	{
		Log.warn ("Rubble of 'Old' save game (doesn't respect isBig)");
		staticData = &model.getUnitsData()->getRubbleSmallData();
	}
	else if (data.getId() == model.getUnitsData()->getRubbleSmallData().ID)
	{
		staticData = &model.getUnitsData()->getRubbleSmallData();
	}
	else if (data.getId() == model.getUnitsData()->getRubbleBigData().ID)
	{
		staticData = &model.getUnitsData()->getRubbleBigData();
	}
	else
	{
		if (!model.getUnitsData()->isValidId (data.getId()))
		{
			NetLog.error ("Static unit data for sID " + data.getId().getText() + " not found.");
			throw std::runtime_error ("Error restoring pointer to static unitdata");
		}
		staticData = &model.getUnitsData()->getStaticUnitData (data.getId());
	}
	storedUnits = ranges::Transform (storedUnitIds, [&] (unsigned int id) { return model.getVehicleFromID (id); });
}

//------------------------------------------------------------------------------
void cUnit::setOwner (cPlayer* owner_)
{
	std::swap (owner, owner_);
	if (owner != owner_) ownerChanged();
}

//------------------------------------------------------------------------------
void cUnit::storeVehicle (cVehicle& vehicle, cMap& map)
{
	map.deleteVehicle (vehicle);
	if (vehicle.getOwner()) vehicle.getOwner()->removeFromScan (vehicle);

	if (vehicle.isSentryActive())
	{
		if (vehicle.getOwner()) vehicle.getOwner()->removeFromSentryMap (vehicle);
		vehicle.setSentryActive (false);
	}

	if (vehicle.getMoveJob()) vehicle.getMoveJob()->stop (vehicle);
	vehicle.setManualFireActive (false);

	vehicle.setLoaded (true);
	vehicle.setIsBeingAttacked (false);

	storedUnits.push_back (&vehicle);
	storedUnitsChanged();
}

//------------------------------------------------------------------------------
void cUnit::exitVehicleTo (cVehicle& vehicle, const cPosition& position, cMap& map)
{
	std::erase (storedUnits, &vehicle);
	storedUnitsChanged();
	vehicle.setLoaded (false);

	vehicle.setPosition (position);
	map.addVehicle (vehicle);

	if (vehicle.getOwner()) vehicle.getOwner()->addToScan (vehicle);
}

//------------------------------------------------------------------------------
void cUnit::forEachStoredUnits (std::function<void (const cVehicle&)> func) const
{
	for (const auto* storedVehicle : storedUnits)
	{
		storedVehicle->forEachStoredUnits (func);
		func (*storedVehicle);
	}
}

//------------------------------------------------------------------------------
void cUnit::forEachStoredUnits (std::function<void (cVehicle&)> func)
{
	for (auto* storedVehicle : storedUnits)
	{
		storedVehicle->forEachStoredUnits (func);
		func (*storedVehicle);
	}
}

//------------------------------------------------------------------------------
void cUnit::setDetectedByPlayer (const cPlayer* player)
{
	int playerId = player->getId();

	if (!ranges::contains (detectedByPlayerList, playerId))
	{
		detectedByPlayerList.push_back (playerId);
		player->detectedStealthUnit (*this);
	}

	if (!ranges::contains (detectedInThisTurnByPlayerList, playerId))
		detectedInThisTurnByPlayerList.push_back (playerId);
}

//------------------------------------------------------------------------------
void cUnit::resetDetectedByPlayer (const cPlayer* player)
{
	if (ranges::contains (detectedByPlayerList, player->getId()))
	{
		std::erase (detectedByPlayerList, player->getId());
		if (!isAVehicle() || !static_cast<const cVehicle*> (this)->isUnitLoaded())
		{
			player->stealthUnitDissappeared (*this);
		}
	}
	std::erase (detectedInThisTurnByPlayerList, player->getId());
}

//------------------------------------------------------------------------------
bool cUnit::isDetectedByPlayer (const cPlayer* player) const
{
	return ranges::contains (detectedByPlayerList, player->getId());
}

//------------------------------------------------------------------------------
bool cUnit::isDetectedByAnyPlayer() const
{
	return detectedByPlayerList.size() > 0;
}

//------------------------------------------------------------------------------
void cUnit::clearDetectedInThisTurnPlayerList()
{
	detectedInThisTurnByPlayerList.clear();
}

//------------------------------------------------------------------------------
void cUnit::setPosition (cPosition position_)
{
	std::swap (position, position_);
	if (position != position_) positionChanged();
}

//------------------------------------------------------------------------------
std::vector<cPosition> cUnit::getPositions() const
{
	if (getIsBig())
	{
		return {position, position.relative (1, 0), position.relative (0, 1), position.relative (1, 1)};
	}
	else
	{
		return {position};
	}
}

//------------------------------------------------------------------------------
/** returns the remaining hitpoints after an attack */
//------------------------------------------------------------------------------
int cUnit::calcHealth (int damage) const
{
	damage -= data.getArmor();

	// minimum damage is 1
	damage = std::max (1, damage);

	const int hp = data.getHitpoints() - damage;
	return std::max (0, hp);
}

//------------------------------------------------------------------------------
/** Checks if the target is in range */
//------------------------------------------------------------------------------
bool cUnit::isInRange (const cPosition& position) const
{
	const auto distanceSquared = (position - this->position).l2NormSquared();

	return distanceSquared <= Square (data.getRange());
}

//------------------------------------------------------------------------------
bool cUnit::isNextTo (const cPosition& position) const
{
	if (position.x() + 1 < this->position.x() || position.y() + 1 < this->position.y())
		return false;

	const int size = getIsBig() ? 2 : 1;

	if (position.x() - size > this->position.x() || position.y() - size > this->position.y())
		return false;
	return true;
}

//------------------------------------------------------------------------------
bool cUnit::isAbove (const cPosition& position) const
{
	return getArea().withinOrTouches (position);
}

//------------------------------------------------------------------------------
uint32_t cUnit::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (data, crc);
	crc = calcCheckSum (iID, crc);
	crc = calcCheckSum (dir, crc);
	for (const auto& u : storedUnits)
		crc = calcCheckSum (u, crc);
	for (const auto& p : detectedByPlayerList)
		crc = calcCheckSum (p, crc);
	for (const auto& p : detectedInThisTurnByPlayerList)
		crc = calcCheckSum (p, crc);
	crc = calcCheckSum (owner, crc);
	crc = calcCheckSum (position, crc);
	crc = calcCheckSum (customName, crc);
	crc = calcCheckSum (turnsDisabled, crc);
	crc = calcCheckSum (sentryActive, crc);
	crc = calcCheckSum (manualFireActive, crc);
	crc = calcCheckSum (attacking, crc);
	crc = calcCheckSum (beingAttacked, crc);
	crc = calcCheckSum (beenAttacked, crc);
	crc = calcCheckSum (storageResCur, crc);

	return crc;
}

//------------------------------------------------------------------------------
cBox<cPosition> cUnit::getArea (int range) const
{
	const auto offset = getIsBig() ? 1 + range : range;
	return {position.relative (-range, -range), position.relative (offset, offset)};
}

//------------------------------------------------------------------------------
std::optional<std::string> cUnit::getCustomName() const
{
	if (!customName.empty()) return customName;
	return std::nullopt;
}

//------------------------------------------------------------------------------
/** changes the name of the unit and indicates it as "not default" */
//------------------------------------------------------------------------------
void cUnit::changeName (std::string&& newName)
{
	customName = std::move (newName);
	renamed();
}

//------------------------------------------------------------------------------
/** rotates the unit to the given direction */
//------------------------------------------------------------------------------
void cUnit::rotateTo (EDirection newDir)
{
	if (newDir == dir)
		return;

	EDirection t = dir;
	int dest = 0;

	for (int i = 0; i < 8; ++i)
	{
		if (t == newDir)
		{
			dest = i;
			break;
		}
		++t;
	}

	if (dest < 4)
		++dir;
	else
		--dir;
}

//------------------------------------------------------------------------------
/** Checks, if the unit can attack an object at the given coordinates*/
//------------------------------------------------------------------------------
bool cUnit::canAttackObjectAt (const cPosition& position, const cMapView& map, bool forceAttack, bool checkRange) const
{
	if (staticData->canAttack == eTerrainFlag::None) return false;
	if (data.getShots() <= 0) return false;
	if (data.getAmmo() <= 0) return false;
	if (attacking) return false;
	// a unit disabled by an infiltrator does not fire (sentry, reaction fire and orders)
	if (isDisabled()) return false;
	if (isAVehicle() && static_cast<const cVehicle*> (this)->isUnitMoving()) return false;
	if (isBeingAttacked()) return false;
	if (isAVehicle() && static_cast<const cVehicle*> (this)->isUnitLoaded()) return false;
	if (map.isValidPosition (position) == false) return false;
	if (checkRange && isInRange (position) == false) return false;

	if (staticData->muzzleType == eMuzzleType::Torpedo && map.isWaterOrCoast (position) == false)
		return false;

	const cUnit* target = cAttackJob::selectTarget (position, staticData->canAttack, map, owner);

	if (target && target->iID == iID) // a unit cannot fire on itself
		return false;

	if (!owner->canSeeAt (position) && !forceAttack)
		return false;

	if (forceAttack)
		return true;

	if (target == nullptr)
		return false;

	// do not fire on e.g. platforms, connectors etc.
	// see ticket #253 on bug tracker
	if (target->isABuilding() && isAVehicle() && staticData->factorAir == 0 && map.possiblePlace (*static_cast<const cVehicle*> (this), position))
		return false;

	if (target->owner == owner)
		return false;

	return true;
}

//------------------------------------------------------------------------------
void cUnit::upgradeToCurrentVersion()
{
	if (owner == nullptr) return;
	cDynamicUnitData* upgradeVersion = owner->getLastUnitData (data.getId());
	if (upgradeVersion == nullptr) return;
	upgradeVersion->markLastVersionUsed();

	data.setVersion (upgradeVersion->getVersion());

	// keep difference between max and current hitpoints
	int missingHitpoints = data.getHitpointsMax() - data.getHitpoints();
	data.setHitpoints (upgradeVersion->getHitpointsMax() - missingHitpoints);

	data.setHitpointsMax (upgradeVersion->getHitpointsMax());

	// don't change the current ammo-amount!
	data.setAmmoMax (upgradeVersion->getAmmoMax());

	// don't change the current speed-amount!
	data.setSpeedMax (upgradeVersion->getSpeedMax());

	data.setArmor (upgradeVersion->getArmor());
	data.setScan (upgradeVersion->getScan());
	data.setRange (upgradeVersion->getRange());
	// don't change the current shot-amount!
	data.setShotsMax (upgradeVersion->getShotsMax());
	data.setDamage (upgradeVersion->getDamage());
	data.setBuildCost (upgradeVersion->getBuildCost());
	statusChanged();
}

//------------------------------------------------------------------------------
void cUnit::setDisabledTurns (int turns)
{
	std::swap (turnsDisabled, turns);
	if (turns != turnsDisabled) disabledChanged();
}

//------------------------------------------------------------------------------
void cUnit::setSentryActive (bool value)
{
	std::swap (sentryActive, value);
	if (value != sentryActive) sentryChanged();
}

//------------------------------------------------------------------------------
void cUnit::setManualFireActive (bool value)
{
	std::swap (manualFireActive, value);
	if (value != manualFireActive) manualFireChanged();
}

//------------------------------------------------------------------------------
void cUnit::setAttacking (bool value)
{
	std::swap (attacking, value);
	if (value != attacking) attackingChanged();
}

//------------------------------------------------------------------------------
void cUnit::setIsBeingAttacked (bool value)
{
	std::swap (beingAttacked, value);
	if (value != beingAttacked) beingAttackedChanged();
}

//------------------------------------------------------------------------------
void cUnit::setHasBeenAttacked (bool value)
{
	std::swap (beenAttacked, value);
	if (value != beenAttacked) beenAttackedChanged();
}

//------------------------------------------------------------------------------
void cUnit::setStoredResources (int value)
{
	value = std::clamp (value, 0, staticData->storageResMax);
	std::swap (storageResCur, value);
	if (storageResCur != value) storedResourcesChanged();
}

//------------------------------------------------------------------------------
bool cUnit::isStealthOnCurrentTerrain (const cMapField& field, const sTerrain& terrain) const
{
	if (staticData->isStealthOn & eTerrainFlag::AreaExpMine)
	{
		return true;
	}
	else if (staticData->factorAir > 0 && isAVehicle() && static_cast<const cVehicle*> (this)->getFlightHeight() > 0)
	{
		return (staticData->isStealthOn & eTerrainFlag::Air) != 0;
	}
	else if ((field.hasBridgeOrPlatform() && staticData->factorGround > 0) || (!terrain.coast && !terrain.water))
	{
		return (staticData->isStealthOn & eTerrainFlag::Ground) != 0;
	}
	else if (terrain.coast)
	{
		return (staticData->isStealthOn & eTerrainFlag::Coast) != 0;
	}
	else if (terrain.water)
	{
		return (staticData->isStealthOn & eTerrainFlag::Sea) != 0;
	}

	return false;
}

//------------------------------------------------------------------------------
void cUnit::detectThisUnit (const cMap& map, const std::vector<std::shared_ptr<cPlayer>>& playerList)
{
	if (staticData->isStealthOn == eTerrainFlag::None) return;

	for (const auto& player : playerList)
	{
		if (checkDetectedByPlayer (*player, map))
		{
			setDetectedByPlayer (player.get());
		}
	}
}

//------------------------------------------------------------------------------
void cUnit::detectOtherUnits (const cMap& map) const
{
	if (!owner || staticData->canDetectStealthOn == eTerrainFlag::None) return;
	// a unit disabled by an infiltrator does not search
	if (isDisabled()) return;

	for (const cPosition& pos : map.staticMap->collectPositions (getArea (data.getScan())))
	{
		const int scanSquared = data.getScan() * data.getScan();
		if ((getPosition() - pos).l2NormSquared() > scanSquared) continue;

		// the unit searches by itself: only with its own detection and range,
		// not with the detection of the other units of its owner
		const auto& vehicles = map.getField (pos).getVehicles();
		for (const auto& vehicle : vehicles)
		{
			if (vehicle->checkDetectedByPlayer (*owner, map, this))
			{
				vehicle->setDetectedByPlayer (owner);
			}
		}
		const auto& buildings = map.getField (pos).getBuildings();
		for (const auto& building : buildings)
		{
			if (building->checkDetectedByPlayer (*owner, map, this))
			{
				building->setDetectedByPlayer (owner);
			}
		}
	}
}

//------------------------------------------------------------------------------
const cStaticUnitData& cUnit::getStaticUnitData() const
{
	return *staticData;
}

namespace
{
	//--------------------------------------------------------------------------
	/** the position is in the range of the unit: the same circle (or square) as in the range maps of the players (cRangeMap) */
	bool isInRangeOfUnit (const cUnit& unit, const cPosition& position, int range, bool square)
	{
		const cPosition delta2x = (position - unit.getPosition()) * 2 - (unit.getIsBig() ? 2 : 1) + 1;
		if (square) return delta2x.abs().max() <= 2 * range;
		return delta2x.l2NormSquared() <= 4 * range * range;
	}
} // namespace

//------------------------------------------------------------------------------
bool cUnit::checkDetectedByPlayer (const cPlayer& player, const cMap& map, const cUnit* detector) const
{
	//big stealth units not implemented yet
	if (getIsBig())
		return false;

	if (&player == owner)
		return false;

	if (staticData->isStealthOn == eTerrainFlag::None)
		return false;

	if (isAVehicle() && static_cast<const cVehicle*> (this)->isUnitLoaded())
		return false;

	// get current terrain
	bool isOnWater = map.isWater (position);
	bool isOnCoast = map.isCoast (position);

	if (staticData->factorGround > 0 && map.getField (position).hasBridgeOrPlatform())
	{
		isOnWater = false;
		isOnCoast = false;
	}

	// the sight and the detection of the player (all his units),
	// or of the detector alone (a unit, that searches by itself, see detectOtherUnits)
	const auto canSee = [&]() {
		if (detector == nullptr) return player.canSeeAnyAreaUnder (*this);
		return isInRangeOfUnit (*detector, position, detector->data.getScan(), false);
	};
	const auto hasDetection = [&] (eTerrainFlag type) {
		if (detector == nullptr)
		{
			if (type == eTerrainFlag::Ground) return player.hasLandDetection (position);
			if (type == eTerrainFlag::Sea) return player.hasSeaDetection (position);
			return player.hasMineDetection (position);
		}
		if (!(detector->getStaticUnitData().canDetectStealthOn & type)) return false;
		// mines can only be detected on directly adjacent fields (see cPlayer::addToScan)
		if (type == eTerrainFlag::AreaExpMine) return isInRangeOfUnit (*detector, position, 1, true);
		return isInRangeOfUnit (*detector, position, detector->data.getScan(), false);
	};

	// check stealth status
	if (!isStealthOnCurrentTerrain (map.getField (position), map.staticMap->getTerrain (position)) && canSee())
	{
		return true;
	}
	else if ((staticData->isStealthOn & eTerrainFlag::Ground) && hasDetection (eTerrainFlag::Ground) && !isOnCoast && !isOnWater)
	{
		return true;
	}
	else if ((staticData->isStealthOn & eTerrainFlag::Sea) && hasDetection (eTerrainFlag::Sea) && isOnWater)
	{
		return true;
	}
	else if ((staticData->isStealthOn & eTerrainFlag::Coast) && hasDetection (eTerrainFlag::Ground) && isOnCoast && staticData->factorGround > 0)
	{
		return true;
	}
	else if ((staticData->isStealthOn & eTerrainFlag::Coast) && hasDetection (eTerrainFlag::Sea) && isOnCoast && staticData->factorSea > 0)
	{
		return true;
	}
	else if ((staticData->isStealthOn & eTerrainFlag::AreaExpMine) && hasDetection (eTerrainFlag::AreaExpMine))
	{
		return true;
	}
	//TODO: isStealthOn & eTerrainFlag::Air

	return false;
}
