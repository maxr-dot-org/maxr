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

#include <cmath>

#include "attackJobs.h"

#include "game/data/units/building.h"
#include "game/logic/client.h"
#include "game/logic/clientevents.h"
#include "utility/listhelpers.h"
#include "game/logic/fxeffects.h"
#include "utility/log.h"
#include "netmessage.h"
#include "game/data/player/player.h"
#include "game/logic/server.h"
#include "game/logic/serverevents.h"
#include "settings.h"
#include "game/data/units/vehicle.h"
#include "sound.h"
#include "game/data/report/unit/savedreportattackingenemy.h"
#include "game/data/report/unit/savedreportdestroyed.h"
#include "game/data/report/unit/savedreportattacked.h"
#include "output/sound/sounddevice.h"
#include "output/sound/soundchannel.h"
#include "utility/random.h"

using namespace std;

//--------------------------------------------------------------------------
cUnit* selectTarget(const cPosition& position, char attackMode, const cMap& map)
{
	cVehicle* targetVehicle = NULL;
	cBuilding* targetBuilding = NULL;
	const cMapField& mapField = map.getField(position);

	// planes
	targetVehicle = mapField.getPlane();
	if (targetVehicle && targetVehicle->getFlightHeight () >  0 && ! (attackMode & TERRAIN_AIR)) targetVehicle = NULL;
	if (targetVehicle && targetVehicle->getFlightHeight () == 0 && !(attackMode & TERRAIN_GROUND)) targetVehicle = NULL;

	// vehicles
	if (!targetVehicle && (attackMode & TERRAIN_GROUND))
	{
		targetVehicle = mapField.getVehicle();
		if (targetVehicle && (targetVehicle->data.isStealthOn & TERRAIN_SEA) && map.isWater (position) && ! (attackMode & AREA_SUB)) targetVehicle = NULL;
	}

	// buildings
	if (!targetVehicle && (attackMode & TERRAIN_GROUND))
	{
		targetBuilding = mapField.getBuilding();
		if (targetBuilding && !targetBuilding->owner) targetBuilding = NULL;
	}

	if (targetVehicle) return targetVehicle;
	return targetBuilding;
}

//--------------------------------------------------------------------------
// cServerAttackJob Implementation
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
int cServerAttackJob::iNextID = 0;

//--------------------------------------------------------------------------
cServerAttackJob::cServerAttackJob (cServer& server_, cUnit* _unit, const cPosition& targetPosition_, bool sentry) :
	server (&server_)
{
	unit = _unit;

	iID = iNextID;
	iNextID++;
	bMuzzlePlayed = false;
	targetPosition = targetPosition_;
	damage = unit->data.getDamage();
	attackMode = unit->data.canAttack;
	sentryFire = sentry;

	iMuzzleType = unit->data.muzzleType;
	aggressorPosition = unit->getPosition();

	// lock targets
	if (unit->data.muzzleType == sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER)
		lockTargetCluster();
	else
		lockTarget(targetPosition);

	unit->data.setShots(unit->data.getShots()-1);
	unit->data.setAmmo(unit->data.getAmmo()-1);
	if (unit->isAVehicle() && unit->data.canDriveAndFire == false)
		unit->data.speedCur -= (int) (((float) unit->data.speedMax) / unit->data.shotsMax);
	unit->setAttacking(true);

	sendFireCommand();

	if (unit->isABuilding() && unit->data.explodesOnContact)
	{
		server->deleteUnit (unit, false);
		unit = 0;
	}
}

//--------------------------------------------------------------------------
cServerAttackJob::~cServerAttackJob()
{
	if (unit != 0)
		unit->setAttacking(false);
}

//--------------------------------------------------------------------------
void cServerAttackJob::lockTarget(const cPosition& position)
{
	// make sure, that the unit data has been send to all clients
	server->checkPlayerUnits();

	cMap& map = *server->Map;
	cUnit* target = selectTarget (position, attackMode, map);
	if (target)
		target->setIsBeeinAttacked(true);

	const bool isAir = (target && target->isAVehicle () && static_cast<cVehicle*> (target)->getFlightHeight () > 0);

	// if the aggressor can attack air and land units,
	// decide whether it is currently attacking air or land targets
	if ((attackMode & TERRAIN_AIR) && (attackMode & TERRAIN_GROUND))
	{
		if (isAir)
			//TODO: can alien units attack submarines?
			attackMode = TERRAIN_AIR;
		else
			attackMode = TERRAIN_GROUND;
	}

	if (!isAir)
	{
		const auto& buildings = map.getField(position).getBuildings();
		for (auto it = buildings.begin(); it != buildings.end(); ++it)
		{
			(*it)->setIsBeeinAttacked(true);
		}
	}

	if (target == NULL)
		return;

	//change position, to match the upper left field of big units
	auto targetPosition = position;
	if (target && target->data.isBig)
		targetPosition = target->getPosition();

	const auto& playerList = server->playerList;
	for (unsigned int i = 0; i  < playerList.size(); i++)
	{
		const cPlayer& player = *playerList[i];

		// target in sight?
		if (!player.canSeeAnyAreaUnder (*target)) continue;

		AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_ATTACKJOB_LOCK_TARGET));
		if (target->isAVehicle())
		{
			cVehicle* v = static_cast<cVehicle*> (target);
			message->pushPosition (v->getMovementOffset());
			message->pushInt32 (v->iID);
		}
		else
		{
			// ID 0 for 'no vehicle'
			message->pushInt32 (0);
		}
		message->pushPosition (targetPosition);
		message->pushBool (isAir);

		server->sendNetMessage (message, &player);
	}
}

//--------------------------------------------------------------------------
void cServerAttackJob::lockTargetCluster()
{
	const int minx = std::max (targetPosition.x () - 2, 0);
	const int maxx = std::min (targetPosition.x () + 2, server->Map->getSize ().x () - 1);
	const int miny = std::max (targetPosition.y () - 2, 0);
	const int maxy = std::min (targetPosition.y () + 2, server->Map->getSize ().y () - 1);

	for (int x = minx; x <= maxx; ++x)
	{
		for (int y = miny; y <= maxy; ++y)
		{
			if (abs (targetPosition.x() - x) + abs (targetPosition.y() - y) > 2) continue;
			lockTarget (cPosition(x, y));
		}
	}
}

//--------------------------------------------------------------------------
void cServerAttackJob::sendFireCommand()
{
	if (unit == 0)
		return;

	// make the aggressor visible on all clients
	// who can see the aggressor offset
	const auto& playerList = server->playerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		cPlayer& player = *playerList[i];

		if (player.canSeeAnyAreaUnder (*unit) == false) continue;
		if (unit->owner == &player) continue;

		unit->setDetectedByPlayer (*server, &player);
	}
	server->checkPlayerUnits();

	//calculate fire direction
	float dx = (float) (targetPosition.x() - aggressorPosition.x());
	float dy = (float) - (targetPosition.y() - aggressorPosition.y());
	float r = sqrtf (dx * dx + dy * dy);

	int fireDir = unit->dir;
	if (r <= 0.001f)
	{
		// do not rotate aggressor
	}
	else
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

		if (r >= 337.5f || r <= 22.5f) fireDir = 0;
		else if (r >= 22.5f && r <= 67.5f) fireDir = 1;
		else if (r >= 67.5f && r <= 112.5f) fireDir = 2;
		else if (r >= 112.5f && r <= 157.5f) fireDir = 3;
		else if (r >= 157.5f && r <= 202.5f) fireDir = 4;
		else if (r >= 202.5f && r <= 247.5f) fireDir = 5;
		else if (r >= 247.5f && r <= 292.5f) fireDir = 6;
		else if (r >= 292.5f && r <= 337.5f) fireDir = 7;
	}
	unit->dir = fireDir;

	// send the fire message to all clients who can see the attack
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		cPlayer& player = *playerList[i];

		if (player.canSeeAnyAreaUnder (*unit)
			|| (player.canSeeAt(targetPosition) && isMuzzleTypeRocket()))
		{
			sendFireCommand (&player);
			executingClients.push_back (&player);
		}
	}
}

//--------------------------------------------------------------------------
void cServerAttackJob::sendFireCommand (cPlayer* player)
{
	if (unit == 0)
		return;

	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_ATTACKJOB_FIRE));

	message->pushBool (sentryFire);
	message->pushInt16 (unit->data.speedCur);
	message->pushInt16 (unit->data.getAmmo());
	message->pushInt16 (unit->data.getShots());
	message->pushChar (unit->dir);
	if (isMuzzleTypeRocket())
		message->pushPosition (targetPosition);

	if (player->canSeeAnyAreaUnder (*unit))
		message->pushInt32 (unit->iID);
	else
	{
		// when the aggressor is out of sight,
		// send the position and muzzle type to the client
		message->pushPosition (aggressorPosition);
		message->pushChar (unit->data.muzzleType);
		message->pushInt32 (0);
	}
	message->pushInt16 (iID);

	server->sendNetMessage (message, player);
}

//--------------------------------------------------------------------------
bool cServerAttackJob::isMuzzleTypeRocket() const
{
	return iMuzzleType == sUnitData::MUZZLE_TYPE_ROCKET
		   || iMuzzleType == sUnitData::MUZZLE_TYPE_TORPEDO
		   || iMuzzleType == sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER;
}

//--------------------------------------------------------------------------
void cServerAttackJob::clientFinished (int playerNr)
{
	for (unsigned int i = 0; i < executingClients.size(); i++)
	{
		if (executingClients[i]->getNr() == playerNr)
		{
			executingClients.erase (executingClients.begin() + i);
			break;
		}
	}

	Log.write (" Server: waiting for " + iToStr ((int) executingClients.size()) + " clients", cLog::eLOG_TYPE_NET_DEBUG);

	if (executingClients.empty())
	{
		if (unit && unit->data.muzzleType == sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER)
			makeImpactCluster();
		else
			makeImpact (targetPosition);
	}
}

//--------------------------------------------------------------------------
void cServerAttackJob::makeImpact(const cPosition& position)
{
	cMap& map = *server->Map;
	if (map.isValidPosition (position) == false) return;

	cUnit* target = selectTarget (position, attackMode, map);

	// check, whether the target is already in the target list.
	// this is needed, to make sure,
	// that a cluster attack doesn't hit the same unit multiple times
	if (Contains (targets, target))
		return;

	if (target != 0)
		targets.push_back (target);

	int remainingHP = 0;
	int id = 0;
	cPlayer* owner = 0;
	bool isAir = (target && target->isAVehicle() && static_cast<cVehicle*> (target)->getFlightHeight() > 0);

	// in the time between the first locking and the impact,
	// it is possible that a vehicle drove onto the target field
	// so relock the target, to ensure synchronity
	if (target && target->isBeeingAttacked() == false)
	{
		Log.write (" Server: relocking target", cLog::eLOG_TYPE_NET_DEBUG);
		lockTarget (position);
	}

	// if target found, make the impact
	if (target != 0)
	{
		// if taget is a stealth unit, make it visible on all clients
		if (target->data.isStealthOn != TERRAIN_NONE)
		{
			const auto& playerList = server->playerList;
			for (unsigned int i = 0; i < playerList.size(); i++)
			{
				cPlayer& player = *playerList[i];
				if (target->owner == &player)
					continue;
				if (!player.canSeeAt(position))
					continue;
				target->setDetectedByPlayer (*server, &player);
			}
			server->checkPlayerUnits();
		}

		id = target->iID;
		target->data.setHitpoints(target->calcHealth (damage));
		remainingHP = target->data.getHitpoints();
		target->setHasBeenAttacked(true);
		owner = target->owner;
		Log.write (" Server: unit '" + target->getDisplayName() + "' (ID: " + iToStr (target->iID) + ") hit. Remaining HP: " + iToStr (target->data.getHitpoints()), cLog::eLOG_TYPE_NET_DEBUG);
	}

	// workaround
	// make sure, the owner gets the impact message
	if (owner) owner->revealPosition(position);

	sendAttackJobImpact (position, remainingHP, id);

	// remove the destroyed units
	if (target && target->data.getHitpoints() <= 0)
	{
		if (target->isABuilding())
			server->destroyUnit (*static_cast<cBuilding*> (target));
		else
			server->destroyUnit (*static_cast<cVehicle*> (target));
		if (unit == target)
			unit = 0;
		target = 0;
	}

	// attack finished. reset attacking and isBeeingAttacked flags
	if (target)
		target->setIsBeeinAttacked(false);

	if (isAir == false)
	{
		const auto& buildings = map.getField(position).getBuildings();
		for (auto it = buildings.begin (); it != buildings.end (); ++it)
		{
			(*it)->setIsBeeinAttacked(false);
		}
	}
	if (unit)
		unit->setAttacking(false);

	// check whether a following sentry mode attack is possible
	if (target && target->isAVehicle())
		static_cast<cVehicle*> (target)->InSentryRange (*server);

	// check whether the aggressor itself is in sentry range
	if (unit && unit->isAVehicle())
		static_cast<cVehicle*> (unit)->InSentryRange (*server);
}

//--------------------------------------------------------------------------
void cServerAttackJob::makeImpactCluster()
{
	const int clusterDamage = damage;

	//full damage
	makeImpact (targetPosition);

	// 3/4 damage
	damage = (clusterDamage * 3) / 4;
	makeImpact (cPosition(targetPosition.x() - 1, targetPosition.y()));
	makeImpact (cPosition(targetPosition.x() + 1, targetPosition.y()));
	makeImpact (cPosition(targetPosition.x()    , targetPosition.y() - 1));
	makeImpact (cPosition(targetPosition.x()    , targetPosition.y() + 1));

	// 1/2 damage
	damage = clusterDamage / 2;
	makeImpact (cPosition(targetPosition.x() + 1, targetPosition.y() + 1));
	makeImpact (cPosition(targetPosition.x() + 1, targetPosition.y() - 1));
	makeImpact (cPosition(targetPosition.x() - 1, targetPosition.y() + 1));
	makeImpact (cPosition(targetPosition.x() - 1, targetPosition.y() - 1));

	// 1/3 damage
	damage = clusterDamage / 3;
	makeImpact (cPosition(targetPosition.x() - 2, targetPosition.y()));
	makeImpact (cPosition(targetPosition.x() + 2, targetPosition.y()));
	makeImpact (cPosition(targetPosition.x()    , targetPosition.y() - 2));
	makeImpact (cPosition(targetPosition.x()    , targetPosition.y() + 2));
}

//--------------------------------------------------------------------------
void cServerAttackJob::sendAttackJobImpact (const cPosition& position, int remainingHP, int id)
{
	const auto& playerList = server->playerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		const cPlayer& player = *playerList[i];

		// target in sight?
		if (!player.canSeeAt(position))
			continue;

		AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_ATTACKJOB_IMPACT));
		message->pushPosition (position);
		message->pushInt16 (remainingHP);
		message->pushInt16 (id);

		server->sendNetMessage (message, &player);
	}
}

//--------------------------------------------------------------------------
// cClientAttackJob implementation
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void cClientAttackJob::lockTarget (cClient& client, cNetMessage& message)
{
	const bool bIsAir = message.popBool();
	const auto position = message.popPosition();
	cMap& map = *client.getMap();
	const int ID = message.popInt32();
	if (ID != 0)
	{
		cVehicle* vehicle = client.getVehicleFromID (ID);
		if (vehicle == NULL)
		{
			// we are out of sync!!!
			Log.write (" Client: vehicle with ID " + iToStr (ID) + " not found", cLog::eLOG_TYPE_NET_ERROR);
			return;
		}

		vehicle->setIsBeeinAttacked(true);

		// synchonize position
        if(vehicle->getPosition() != position)
		{
            Log.write(" Client: changed vehicle position to (" + iToStr(position.x()) + ":" + iToStr(position.y()) + ")", cLog::eLOG_TYPE_NET_DEBUG);
            map.moveVehicle(*vehicle, position);
			vehicle->owner->doScan();

			vehicle->setMovementOffset (message.popPosition());
		}
	}
	if (!bIsAir)
	{
		const auto& buildings = map.getField (position).getBuildings ();
		for (auto it = buildings.begin (); it != buildings.end (); ++it)
		{
			(*it)->setIsBeeinAttacked(true);
		}
	}
}

//--------------------------------------------------------------------------
void cClientAttackJob::handleAttackJobs (cClient& client)
{
	for (size_t i = 0; i != client.attackJobs.size(); ++i)
	{
		cClientAttackJob* job = client.attackJobs[i];
		switch (job->state)
		{
			case FINISHED:
			{
				job->sendFinishMessage (client);
				if (job->unit) job->unit->setAttacking(false);
				delete job;
				client.attackJobs.erase (client.attackJobs.begin() + i);
				--i;
				break;
			}
			case PLAYING_MUZZLE:
			{
				job->playMuzzle (client);
				break;
			}
			case ROTATING:
			{
				job->rotate();
				break;
			}
		}
	}
}

//--------------------------------------------------------------------------
cClientAttackJob::cClientAttackJob (cClient* client, cNetMessage& message)
{
	state = ROTATING;
	wait = 0;
	length = 0;
	this->iID = message.popInt16();
	targetPosition = -1;
	unit = NULL;

	// check for duplicate jobs
	for (size_t i = 0; i != client->attackJobs.size(); ++i)
	{
		if (client->attackJobs[i]->iID == this->iID)
		{
			state = FINISHED;
			return;
		}
	}

	const int unitID = message.popInt32();

	// get muzzle type and aggressor position
	if (unitID == 0)
	{
		iMuzzleType = message.popChar();
		if (iMuzzleType != sUnitData::MUZZLE_TYPE_ROCKET && iMuzzleType != sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER && iMuzzleType != sUnitData::MUZZLE_TYPE_TORPEDO)
		{
			state = FINISHED;
			return;
		}
		aggressorPosition = message.popPosition ();
	}
	else
	{
		unit = client->getVehicleFromID (unitID);
		if (unit == NULL) unit = client->getBuildingFromID (unitID);
		if (unit == NULL)
		{
			// we are out of sync!!!
			state = FINISHED;
			Log.write (" Client: aggressor with id " + iToStr (unitID) + " not found", cLog::eLOG_TYPE_NET_ERROR);
			return;
		}

		iMuzzleType = unit->data.muzzleType;
		aggressorPosition = unit->getPosition();
	}

	if (iMuzzleType == sUnitData::MUZZLE_TYPE_ROCKET || iMuzzleType == sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER || iMuzzleType == sUnitData::MUZZLE_TYPE_TORPEDO)
	{
		targetPosition = message.popPosition ();
	}
	iFireDir = message.popChar();

	// get remaining shots, ammo and movement points
	if (unit)
	{
		unit->data.setShots(message.popInt16());
		unit->data.setAmmo(message.popInt16());
		unit->setAttacking(true);
		if (unit->isAVehicle())
			unit->data.speedCur = message.popInt16();
	}
	const bool sentryReaction = message.popBool();
	if (sentryReaction && unit && unit->owner == &client->getActivePlayer())
	{
		const string name = unit->getDisplayName();
		client->getActivePlayer ().addSavedReport (std::make_unique<cSavedReportAttackingEnemy> (*unit));
	}
}

//--------------------------------------------------------------------------
void cClientAttackJob::rotate()
{
	if (unit && unit->dir != iFireDir && !unit->data.explodesOnContact)
	{
		unit->rotateTo (iFireDir);
	}
	else
	{
		state = PLAYING_MUZZLE;
	}
}

//--------------------------------------------------------------------------
void cClientAttackJob::playMuzzle (cClient& client)
{
	cPosition offset(0, 0);
	const cMap& map = *client.getMap();

	if (unit && unit->isABuilding() && unit->data.explodesOnContact)
	{
		cBuilding* building = static_cast<cBuilding*> (unit);
		state = FINISHED;
		//FIXME: do not play sound at this place! Do play in gameGui! Do play through SoundManager!
		cSoundDevice::getInstance ().getFreeSoundEffectChannel ().play (building->uiData->Attack);
		if (map.isWaterOrCoast (unit->getPosition()))
		{
			client.addFx (std::make_shared<cFxExploWater> (unit->getPosition() * 64 + 32));
		}
		else
		{
			client.addFx (std::make_shared<cFxExploSmall> (unit->getPosition() * 64 + 32));
		}
		client.deleteUnit (building);
		return;
	}

	switch (iMuzzleType)
	{
		case sUnitData::MUZZLE_TYPE_BIG:
			if (wait++ != 0)
			{
				if (wait > 2) state = FINISHED;
				return;
			}
			switch (iFireDir)
			{
				case 0:
					offset.y() -= 40;
					break;
				case 1:
					offset.x() += 32;
					offset.y() -= 32;
					break;
				case 2:
					offset.x() += 40;
					break;
				case 3:
					offset.x() += 32;
					offset.y() += 32;
					break;
				case 4:
					offset.y() += 40;
					break;
				case 5:
					offset.x() -= 32;
					offset.y() += 32;
					break;
				case 6:
					offset.x() -= 40;
					break;
				case 7:
					offset.x() -= 32;
					offset.y() -= 32;
					break;
			}
			if (unit)
			{
				client.addFx (std::make_shared<cFxMuzzleBig> (unit->getPosition() * 64 + offset, iFireDir));
			}
			break;
		case sUnitData::MUZZLE_TYPE_SMALL:
			if (wait++ != 0)
			{
				if (wait > 2) state = FINISHED;
				return;
			}
			if (unit)
			{
				client.addFx(std::make_shared<cFxMuzzleSmall>(unit->getPosition() * 64, iFireDir));
			}
			break;
		case sUnitData::MUZZLE_TYPE_ROCKET:
		case sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER:
		{
			if (wait++ != 0)
			{
				if (wait > length) state = FINISHED;

				return;
			}

			auto rocket = std::make_shared<cFxRocket>(aggressorPosition * 64 + 32, targetPosition * 64 + 32, iFireDir, false);
			length = rocket->getLength() / 5;
			client.addFx (std::move(rocket));
			break;

		}
		case sUnitData::MUZZLE_TYPE_MED:
		case sUnitData::MUZZLE_TYPE_MED_LONG:
			if (wait++ != 0)
			{
				if (wait > 2) state = FINISHED;
				return;
			}
			switch (iFireDir)
			{
				case 0:
					offset.y() -= 20;
					break;
				case 1:
					offset.x() += 12;
					offset.y() -= 12;
					break;
				case 2:
					offset.x() += 20;
					break;
				case 3:
					offset.x() += 12;
					offset.y() += 12;
					break;
				case 4:
					offset.y() += 20;
					break;
				case 5:
					offset.x() -= 12;
					offset.y() += 12;
					break;
				case 6:
					offset.x() -= 20;
					break;
				case 7:
					offset.x() -= 12;
					offset.y() -= 12;
					break;
			}
			if (iMuzzleType == sUnitData::MUZZLE_TYPE_MED)
			{
				if (unit)
				{
					client.addFx (std::make_shared<cFxMuzzleMed> (unit->getPosition() * 64 + offset, iFireDir));
				}
			}
			else
			{
				if (unit)
				{
					client.addFx(std::make_shared<cFxMuzzleMedLong>(unit->getPosition() * 64 + offset, iFireDir));
				}
			}
			break;
		case sUnitData::MUZZLE_TYPE_TORPEDO:
		{
			if (wait++ != 0)
			{
				if (wait > length) state = FINISHED;

				return;
			}

			auto rocket = std::make_shared<cFxRocket> (aggressorPosition * 64 + 32, targetPosition * 64 + 32, iFireDir, false);
			length = rocket->getLength() / 5;
			client.addFx (std::move(rocket));

			break;
		}
		case sUnitData::MUZZLE_TYPE_SNIPER:
			state = FINISHED;
			break;
	}
	if (unit && unit->isAVehicle())
	{
		//FIXME: do not play sound at this place! Do play in gameGui! Do play through SoundManager!
		cSoundDevice::getInstance ().getFreeSoundEffectChannel ().play (static_cast<cVehicle*> (unit)->uiData->Attack);
	}
	else if (unit && unit->isABuilding())
	{
		//FIXME: do not play sound at this place! Do play in gameGui! Do play through SoundManager!
		cSoundDevice::getInstance ().getFreeSoundEffectChannel ().play (static_cast<cBuilding*> (unit)->uiData->Attack);
	}
}

//--------------------------------------------------------------------------
void cClientAttackJob::sendFinishMessage (cClient& client)
{
	cNetMessage* message = new cNetMessage (GAME_EV_ATTACKJOB_FINISHED);
	message->pushInt16 (iID);
	client.sendNetMessage (message);
}

//--------------------------------------------------------------------------
void cClientAttackJob::makeImpact (cClient& client, const cPosition& position, int remainingHP, int id)
{
	cMap& map = *client.getMap();
	if (map.isValidPosition (position) == false)
	{
		Log.write (" Client: Invalid offset", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}

	cVehicle* targetVehicle = client.getVehicleFromID (id);
	cBuilding* targetBuilding = client.getBuildingFromID (id);

	cUnit* targetUnit = nullptr;

	bool playImpact = false;
	bool ownUnit = false;
	bool destroyed = false;
	bool isAir = false;
	int offX = 0;
	int offY = 0;

	// no target found
	if (!targetBuilding && !targetVehicle)
	{
		playImpact = true;
	}
	else
	{
		if (targetVehicle)
		{
			targetUnit = targetVehicle;
			isAir = (targetVehicle->data.factorAir > 0);
			targetVehicle->data.setHitpoints(remainingHP);

			Log.write (" Client: vehicle '" + targetVehicle->getDisplayName () + "' (ID: " + iToStr (targetVehicle->iID) + ") hit. Remaining HP: " + iToStr (targetVehicle->data.getHitpoints ()), cLog::eLOG_TYPE_NET_DEBUG);

			if (targetVehicle->owner == &client.getActivePlayer()) ownUnit = true;

			if (targetVehicle->data.getHitpoints() <= 0)
			{
				client.destroyUnit (*targetVehicle);
				targetVehicle = NULL;
				destroyed = true;
			}
			else
			{
				playImpact = true;
				offX = targetVehicle->getMovementOffset().x();
				offY = targetVehicle->getMovementOffset().y();
			}
		}
		else
		{
			targetUnit = targetBuilding;
			targetBuilding->data.setHitpoints(remainingHP);

			Log.write (" Client: building '" + targetBuilding->getDisplayName () + "' (ID: " + iToStr (targetBuilding->iID) + ") hit. Remaining HP: " + iToStr (targetBuilding->data.getHitpoints ()), cLog::eLOG_TYPE_NET_DEBUG);

			if (targetBuilding->owner == &client.getActivePlayer()) ownUnit = true;

			if (targetBuilding->data.getHitpoints () <= 0)
			{
				client.destroyUnit (*targetBuilding);
				targetBuilding = NULL;
				destroyed = true;
			}
			else
			{
				playImpact = true;
			}
		}
	}

	if (playImpact && cSettings::getInstance().isAlphaEffects())
	{
		client.addFx (std::make_shared<cFxHit> (position * 64 + 32));
	}

	if (ownUnit)
	{
		string message;
		assert (targetUnit != nullptr);

		if (destroyed)
		{
			client.getActivePlayer ().addSavedReport (std::make_unique<cSavedReportDestroyed> (*targetUnit));
		}
		else
		{
			client.getActivePlayer ().addSavedReport (std::make_unique<cSavedReportAttacked> (*targetUnit));
		}
	}

	// clean up
	if (targetVehicle) targetVehicle->setIsBeeinAttacked(false);

	if (!isAir)
	{
		const auto& buildings = map.getField(position).getBuildings();
		for (auto it = buildings.begin (); it != buildings.end (); ++it)
		{
			(*it)->setIsBeeinAttacked(false);
		}
	}
}
