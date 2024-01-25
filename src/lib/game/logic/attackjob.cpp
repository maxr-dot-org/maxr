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

#include "game/logic/attackjob.h"

#include "game/data/map/map.h"
#include "game/data/model.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/unit.h"
#include "game/data/units/vehicle.h"
#include "game/logic/fxeffects.h"
#include "utility/crc.h"
#include "utility/listhelpers.h"
#include "utility/log.h"

#include <algorithm>
#include <cassert>
#include <memory>

//TODO: test alien attack (ground & air)

//--------------------------------------------------------------------------
cAttackJob::cAttackJob (cUnit& aggressor, const cPosition& targetPosition, const cModel& model) :
	aggressorId (aggressor.getId()),
	targetPosition (targetPosition),
	fireDir (calcFireDir (aggressor)),
	counter (10),
	state (eAJState::Rotating)
{
	NetLog.debug (" cAttackJob: Started attack, aggressor ID: " + std::to_string (aggressor.getId()) + " @" + std::to_string (model.getGameTime()));
	assert (!aggressor.isAVehicle() || !static_cast<cVehicle&> (aggressor).isUnitMoving());

	lockTarget (*model.getMap(), aggressor);

	//lock aggressor
	aggressor.setAttacking (true);

	// make the aggressor visible on all clients
	// who can see the aggressor offset
	if (aggressor.getStaticUnitData().isStealthOn != eTerrainFlag::None)
	{
		for (const auto& player : model.getPlayerList())
		{
			if (player->canSeeAnyAreaUnder (aggressor) == false) continue;
			if (aggressor.getOwner() == player.get()) continue;

			aggressor.setDetectedByPlayer (player.get());
		}
	}
}

//------------------------------------------------------------------------------
void cAttackJob::run (cModel& model)
{
	if (counter > 0) counter--;

	if (aggressorId == -1)
	{
		releaseTargets (model);
		state = eAJState::Finished;
	}
	auto* aggressor = model.getUnitFromID(aggressorId);
	assert(aggressor != nullptr);
	switch (state)
	{
		case eAJState::Rotating:
		{
			if (counter == 0)
			{
				if (aggressor->dir == fireDir)
				{
					fire (model);
					state = eAJState::Firing;
				}
				else
				{
					aggressor->rotateTo (fireDir);
					counter = 10;
				}
			}
			break;
		}
		case eAJState::Firing:
			if (counter == 0)
			{
				impact (model);
				releaseTargets (model);
				state = eAJState::Finished;
			}
			break;
		case eAJState::Finished:
		case eAJState::PlayingMuzzle:
			break;
	}
}

//------------------------------------------------------------------------------
bool cAttackJob::finished() const
{
	return state == eAJState::Finished;
}

//------------------------------------------------------------------------------
void cAttackJob::onRemoveUnit (const cUnit& unit)
{
	if (aggressorId == unit.getId())
	{
		aggressorId = -1;
	}
}

//------------------------------------------------------------------------------
uint32_t cAttackJob::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (aggressorId, crc);
	crc = calcCheckSum (targetPosition, crc);
	crc = calcCheckSum (lockedTargets, crc);
	crc = calcCheckSum (fireDir, crc);
	crc = calcCheckSum (counter, crc);
	crc = calcCheckSum (state, crc);

	return crc;
}

//------------------------------------------------------------------------------
// private functions

int cAttackJob::calcFireDir (const cUnit& aggressor)
{
	auto dx = (float) (targetPosition.x() - aggressor.getPosition().x());
	auto dy = (float) -(targetPosition.y() - aggressor.getPosition().y());
	auto r = std::sqrt (dx * dx + dy * dy);

	int fireDir = aggressor.dir;
	if (r > 0.001f)
	{
		// 360 / (2 * PI) = 57.29577951f;
		dx /= r;
		dy /= r;
		r = asinf (dx) * 57.29577951f;
		if (dy >= 0)
		{
			if (r < 0)
				r += 360;
		}
		else
			r = 180 - r;

		if (r >= 337.5f || r <= 22.5f)
			fireDir = 0;
		else if (r >= 22.5f && r <= 67.5f)
			fireDir = 1;
		else if (r >= 67.5f && r <= 112.5f)
			fireDir = 2;
		else if (r >= 112.5f && r <= 157.5f)
			fireDir = 3;
		else if (r >= 157.5f && r <= 202.5f)
			fireDir = 4;
		else if (r >= 202.5f && r <= 247.5f)
			fireDir = 5;
		else if (r >= 247.5f && r <= 292.5f)
			fireDir = 6;
		else if (r >= 292.5f && r <= 337.5f)
			fireDir = 7;
	}

	return fireDir;
}

void cAttackJob::lockTarget (const cMap& map, const cUnit& aggressor)
{
	const int range = aggressor.getStaticUnitData().muzzleType == eMuzzleType::RocketCluster ? 2 : 0;

	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			if (abs (x) + abs (y) <= range && map.isValidPosition (targetPosition + cPosition (x, y)))
			{
				cUnit* target = selectTarget (targetPosition + cPosition (x, y), aggressor.getStaticUnitData().canAttack, map, aggressor.getOwner());
				if (target)
				{
					target->setIsBeeinAttacked (true);
					lockedTargets.push_back (target->iID);
					NetLog.debug (" cAttackJob: locked target ID: " + std::to_string (target->iID) + " at (" + std::to_string (targetPosition.x() + x) + "," + std::to_string (targetPosition.y() + y) + ")");
				}
			}
		}
	}
}

void cAttackJob::releaseTargets (const cModel& model)
{
	// unlock targets in case they were locked at the beginning of the attack, but are not hit by the impact
	// for example a plane flies on the target field and takes the shot in place of the original plane
	for (auto unitId : lockedTargets)
	{
		cUnit* unit = model.getUnitFromID (unitId);

		if (unit && unit->data.getHitpoints() > 0)
		{
			unit->setIsBeeinAttacked (false);
		}
	}

	lockedTargets.clear();
}

void cAttackJob::fire (cModel& model)
{
	auto* aggressor = model.getUnitFromID (aggressorId);
	assert (aggressor != nullptr);
	//update data
	aggressor->data.setShots (aggressor->data.getShots() - 1);
	aggressor->data.setAmmo (aggressor->data.getAmmo() - 1);
	if (aggressor->isAVehicle() && aggressor->getStaticUnitData().vehicleData.canDriveAndFire == false)
		aggressor->data.setSpeed (aggressor->data.getSpeed() - (int) (((float) aggressor->data.getSpeedMax()) / aggressor->data.getShotsMax()));

	auto muzzle = createMuzzleFx (*aggressor);
	if (muzzle)
	{
		//set timer for next state
		const int IMPACT_DELAY = 10;
		counter = muzzle->getLength() + IMPACT_DELAY;

		//play muzzle flash / fire rocket
		model.addFx (std::move (muzzle));
	}

	//make explosive mines explode
	if (dynamic_cast<cBuilding*> (aggressor) && aggressor->getStaticUnitData().buildingData.explodesOnContact && aggressor->getPosition() == targetPosition)
	{
		const cMap& map = *model.getMap();
		if (map.isWaterOrCoast (aggressor->getPosition()))
		{
			model.addFx (std::make_unique<cFxExploWater> (aggressor->getPosition() * 64 + cPosition (32, 32)));
		}
		else
		{
			model.addFx (std::make_unique<cFxExploSmall> (aggressor->getPosition() * 64 + cPosition (32, 32)));
		}
	}
}

std::unique_ptr<cFx> cAttackJob::createMuzzleFx (const cUnit& aggressor)
{
	//TODO: this shouldn't be in the attackjob class.

	const sID id = aggressor.data.getId();
	const cPosition aggressorPosition = aggressor.getPosition();

	cPosition offset (0, 0);
	switch (aggressor.getStaticUnitData().muzzleType)
	{
		case eMuzzleType::Big:
			switch (fireDir)
			{
				case 0:
					offset.y() = -40;
					break;
				case 1:
					offset.x() = 32;
					offset.y() = -32;
					break;
				case 2:
					offset.x() = 40;
					break;
				case 3:
					offset.x() = 32;
					offset.y() = 32;
					break;
				case 4:
					offset.y() = 40;
					break;
				case 5:
					offset.x() = -32;
					offset.y() = 32;
					break;
				case 6:
					offset.x() = -40;
					break;
				case 7:
					offset.x() = -32;
					offset.y() = -32;
					break;
			}
			return std::make_unique<cFxMuzzleBig> (aggressorPosition * 64 + offset, fireDir, id);

		case eMuzzleType::Small:
			return std::make_unique<cFxMuzzleSmall> (aggressorPosition * 64, fireDir, id);

		case eMuzzleType::Rocket:
		case eMuzzleType::RocketCluster:
			return std::make_unique<cFxRocket> (aggressorPosition * 64 + cPosition (32, 32), targetPosition * 64 + cPosition (32, 32), fireDir, false, id);

		case eMuzzleType::Med:
		case eMuzzleType::MedLong:
			switch (fireDir)
			{
				case 0:
					offset.y() = -20;
					break;
				case 1:
					offset.x() = 12;
					offset.y() = -12;
					break;
				case 2:
					offset.x() = 20;
					break;
				case 3:
					offset.x() = 12;
					offset.y() = 12;
					break;
				case 4:
					offset.y() = 20;
					break;
				case 5:
					offset.x() = -12;
					offset.y() = 12;
					break;
				case 6:
					offset.x() = -20;
					break;
				case 7:
					offset.x() = -12;
					offset.y() = -12;
					break;
			}
			if (aggressor.getStaticUnitData().muzzleType == eMuzzleType::Med)
				return std::make_unique<cFxMuzzleMed> (aggressorPosition * 64 + offset, fireDir, id);
			else
				return std::make_unique<cFxMuzzleMedLong> (aggressorPosition * 64 + offset, fireDir, id);

		case eMuzzleType::Torpedo:
			return std::make_unique<cFxRocket> (aggressorPosition * 64 + cPosition (32, 32), targetPosition * 64 + cPosition (32, 32), fireDir, true, id);
		case eMuzzleType::Sniper:
		//TODO: sniper has no animation?!?
		default:
			return nullptr;
	}
}

void cAttackJob::impact (cModel& model)
{
	auto* aggressor = model.getUnitFromID (aggressorId);
	assert (aggressor != nullptr);

	if (aggressor->getStaticUnitData().muzzleType == eMuzzleType::RocketCluster)
		impactCluster (model);
	else
		impactSingle (targetPosition, aggressor->data.getDamage(), model);
}

void cAttackJob::impactCluster (cModel& model)
{
	std::vector<cUnit*> targets;

	auto* aggressor = model.getUnitFromID (aggressorId);
	assert (aggressor != nullptr);
	//full damage
	int clusterDamage = aggressor->data.getDamage();
	impactSingle (targetPosition, clusterDamage, model, &targets);

	// 3/4 damage
	clusterDamage = (aggressor->data.getDamage() * 3) / 4;
	impactSingle (targetPosition + cPosition (-1, 0), clusterDamage, model, &targets);
	impactSingle (targetPosition + cPosition (+1, 0), clusterDamage, model, &targets);
	impactSingle (targetPosition + cPosition (0, -1), clusterDamage, model, &targets);
	impactSingle (targetPosition + cPosition (0, +1), clusterDamage, model, &targets);

	// 1/2 damage
	clusterDamage = aggressor->data.getDamage() / 2;
	impactSingle (targetPosition + cPosition (+1, +1), clusterDamage, model, &targets);
	impactSingle (targetPosition + cPosition (+1, -1), clusterDamage, model, &targets);
	impactSingle (targetPosition + cPosition (-1, +1), clusterDamage, model, &targets);
	impactSingle (targetPosition + cPosition (-1, -1), clusterDamage, model, &targets);

	// 1/3 damage
	clusterDamage = aggressor->data.getDamage() / 3;
	impactSingle (targetPosition + cPosition (-2, 0), clusterDamage, model, &targets);
	impactSingle (targetPosition + cPosition (+2, 0), clusterDamage, model, &targets);
	impactSingle (targetPosition + cPosition (0, -2), clusterDamage, model, &targets);
	impactSingle (targetPosition + cPosition (0, +2), clusterDamage, model, &targets);
}

void cAttackJob::impactSingle (const cPosition& position, int attackPoints, cModel& model, std::vector<cUnit*>* avoidTargets)
{
	const cMap& map = *model.getMap();

	if (!map.isValidPosition (position))
		return;
	auto* aggressor = model.getUnitFromID (aggressorId);
	assert (aggressor != nullptr);

	cUnit* target = selectTarget (position, aggressor->getStaticUnitData().canAttack, map, aggressor->getOwner());

	//check list of units that will be ignored as target.
	//Used to prevent, that cluster attacks hit the same unit multiple times
	if (avoidTargets)
	{
		if (ranges::contains (*avoidTargets, target))
		{
			return;
		}
		avoidTargets->push_back (target);
	}

	NetLog.debug (" cAttackJob: Impact at " + toString (position) + " @" + std::to_string (model.getGameTime()));

	// if target is a stealth unit, make it visible on all clients
	if (target && target->getStaticUnitData().isStealthOn != eTerrainFlag::None)
	{
		for (const auto& player : model.getPlayerList())
		{
			if (target->getOwner() == player.get()) continue;
			if (!player->canSeeAnyAreaUnder (*target)) continue;

			target->setDetectedByPlayer (player.get());
		}
	}

	//make impact on target
	bool destroyed = false;
	if (target)
	{
		int remainingHp = target->calcHealth (attackPoints);
		target->data.setHitpoints (remainingHp);
		target->setHasBeenAttacked (true);

		NetLog.debug (" cAttackJob: target hit ID: " + std::to_string (target->getId()) + ", remaining hp: " + std::to_string (remainingHp) + " @" + std::to_string (model.getGameTime()));

		if (remainingHp <= 0)
		{
			destroyed = true;
		}
	}

	if (!destroyed)
	{
		bool targetHit = target != nullptr;
		bool bigTarget = false;
		cPosition offset;
		if (target)
		{
			bigTarget = target->getIsBig();
			offset = target->getMovementOffset();
		}
		model.addFx (std::make_unique<cFxHit> (position * 64 + offset + cPosition (32, 32), targetHit, bigTarget));
	}

	aggressor->setAttacking (false);

	//make message
	if (target && target->getOwner())
	{
		if (destroyed)
		{
			target->getOwner()->unitDestroyed (*target);
		}
		else
		{
			target->getOwner()->unitAttacked (*target);
		}
	}

	if (dynamic_cast<cBuilding*> (aggressor) && aggressor->getStaticUnitData().buildingData.explodesOnContact)
	{
		model.deleteUnit (aggressor);
		aggressor = nullptr;
	}

	// check whether a following sentry mode attack is possible
	if (target && target->isAVehicle() && !destroyed)
		static_cast<cVehicle*> (target)->inSentryRange (model);

	// check whether the aggressor is in sentry range
	if (aggressor && aggressor->isAVehicle())
		static_cast<cVehicle*> (aggressor)->inSentryRange (model);

	// remove destroyed unit
	if (target && destroyed)
	{
		model.destroyUnit (*target);
		target = nullptr;
	}
}
