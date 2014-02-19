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

#include "buildings.h"
#include "client.h"
#include "clientevents.h"
#include "clist.h"
#include "fxeffects.h"
#include "hud.h"
#include "log.h"
#include "netmessage.h"
#include "player.h"
#include "server.h"
#include "serverevents.h"
#include "settings.h"
#include "vehicles.h"

using namespace std;

//--------------------------------------------------------------------------
cUnit* selectTarget (int x, int y, char attackMode, cMap& map)
{
	cVehicle* targetVehicle = NULL;
	cBuilding* targetBuilding = NULL;
	const int offset = map.getOffset (x, y);
	cMapField& mapField = map[offset];

	// planes
	targetVehicle = mapField.getPlane();
	if (targetVehicle && targetVehicle->FlightHigh >  0 && ! (attackMode & TERRAIN_AIR)) targetVehicle = NULL;
	if (targetVehicle && targetVehicle->FlightHigh == 0 && ! (attackMode & TERRAIN_GROUND)) targetVehicle = NULL;

	// vehicles
	if (!targetVehicle && (attackMode & TERRAIN_GROUND))
	{
		targetVehicle = mapField.getVehicle();
		if (targetVehicle && (targetVehicle->data.isStealthOn & TERRAIN_SEA) && map.isWater (x, y) && ! (attackMode & AREA_SUB)) targetVehicle = NULL;
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
cServerAttackJob::cServerAttackJob (cServer& server_, cUnit* _unit, int targetOff, bool sentry) :
	server (&server_)
{
	unit = _unit;

	iID = iNextID;
	iNextID++;
	bMuzzlePlayed = false;
	iTargetOff = targetOff;
	damage = unit->data.damage;
	attackMode = unit->data.canAttack;
	sentryFire = sentry;

	iMuzzleType = unit->data.muzzleType;
	iAgressorOff = server->Map->getOffset (unit->PosX, unit->PosY);

	// lock targets
	if (unit->data.muzzleType == sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER)
		lockTargetCluster();
	else
		lockTarget (targetOff);

	unit->data.shotsCur--;
	unit->data.ammoCur--;
	if (unit->isAVehicle() && unit->data.canDriveAndFire == false)
		unit->data.speedCur -= (int) (((float) unit->data.speedMax) / unit->data.shotsMax);
	unit->attacking = true;

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
		unit->attacking = false;
}

//--------------------------------------------------------------------------
void cServerAttackJob::lockTarget (int offset)
{
	// make sure, that the unit data has been send to all clients
	server->checkPlayerUnits();

	cMap& map = *server->Map;
	cUnit* target = selectTarget (offset % map.getSize(), offset / map.getSize(), attackMode, map);
	if (target)
		target->isBeeingAttacked = true;

	const bool isAir = (target && target->isAVehicle() && static_cast<cVehicle*> (target)->FlightHigh > 0);

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
		std::vector<cBuilding*>& buildings = map[offset].getBuildings();
		for (std::vector<cBuilding*>::iterator it = buildings.begin(); it != buildings.end(); ++it)
		{
			(*it)->isBeeingAttacked = true;
		}
	}

	if (target == NULL)
		return;

	//change offset, to match the upper left field of big units
	if (target && target->data.isBig)
		offset = map.getOffset (target->PosX, target->PosY);

	const std::vector<cPlayer*>& playerList = server->PlayerList;
	for (unsigned int i = 0; i  < playerList.size(); i++)
	{
		const cPlayer* player = playerList[i];

		// target in sight?
		if (!player->canSeeAnyAreaUnder (*target)) continue;

		AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_ATTACKJOB_LOCK_TARGET));
		if (target->isAVehicle())
		{
			cVehicle* v = static_cast<cVehicle*> (target);
			message->pushChar (v->OffX);
			message->pushChar (v->OffY);
			message->pushInt32 (v->iID);
		}
		else
		{
			// ID 0 for 'no vehicle'
			message->pushInt32 (0);
		}
		message->pushInt32 (offset);
		message->pushBool (isAir);

		server->sendNetMessage (message, player->getNr());
	}
}

//--------------------------------------------------------------------------
void cServerAttackJob::lockTargetCluster()
{
	const int mapSize = server->Map->getSize();
	const int PosX = iTargetOff % mapSize;
	const int PosY = iTargetOff / mapSize;
	const int minx = std::max (PosX - 2, 0);
	const int maxx = std::min (PosX + 2, mapSize - 1);
	const int miny = std::max (PosY - 2, 0);
	const int maxy = std::min (PosY + 2, mapSize - 1);

	for (int x = minx; x <= maxx; ++x)
	{
		for (int y = miny; y <= maxy; ++y)
		{
			if (abs (PosX - x) + abs (PosY - y) > 2) continue;
			lockTarget (server->Map->getOffset (x, y));
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
	std::vector<cPlayer*>& playerList = server->PlayerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		cPlayer* player = playerList[i];

		if (player->canSeeAnyAreaUnder (*unit) == false) continue;
		if (unit->owner == player) continue;

		unit->setDetectedByPlayer (*server, player);
	}
	server->checkPlayerUnits();

	//calculate fire direction
	const int mapSize = server->Map->getSize();
	const int targetX = iTargetOff % mapSize;
	const int targetY = iTargetOff / mapSize;
	const int agressorX = iAgressorOff % mapSize;
	const int agressorY = iAgressorOff / mapSize;

	float dx = (float) (targetX - agressorX);
	float dy = (float) - (targetY - agressorY);
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
		cPlayer* player = playerList[i];

		if (player->canSeeAnyAreaUnder (*unit)
			|| (player->ScanMap[iTargetOff] && isMuzzleTypeRocket()))
		{
			sendFireCommand (player);
			executingClients.push_back (player);
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
	message->pushInt16 (unit->data.ammoCur);
	message->pushInt16 (unit->data.shotsCur);
	message->pushChar (unit->dir);
	if (isMuzzleTypeRocket())
		message->pushInt32 (iTargetOff);

	if (player->canSeeAnyAreaUnder (*unit))
		message->pushInt32 (unit->iID);
	else
	{
		// when the aggressor is out of sight,
		// send the position and muzzle type to the client
		message->pushInt32 (iAgressorOff);
		message->pushChar (unit->data.muzzleType);
		message->pushInt32 (0);
	}
	message->pushInt16 (iID);

	server->sendNetMessage (message, player->getNr());
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
			makeImpact (iTargetOff % server->Map->getSize(), iTargetOff / server->Map->getSize());
	}
}

//--------------------------------------------------------------------------
void cServerAttackJob::makeImpact (int x, int y)
{
	cMap& map = *server->Map;
	if (map.isValidPos (x, y) == false) return;
	int offset = map.getOffset (x, y);

	cUnit* target = selectTarget (x, y, attackMode, map);

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
	bool isAir = (target && target->isAVehicle() && static_cast<cVehicle*> (target)->FlightHigh > 0);

	// in the time between the first locking and the impact,
	// it is possible that a vehicle drove onto the target field
	// so relock the target, to ensure synchronity
	if (target && target->isBeeingAttacked == false)
	{
		Log.write (" Server: relocking target", cLog::eLOG_TYPE_NET_DEBUG);
		lockTarget (offset);
	}

	// if target found, make the impact
	if (target != 0)
	{
		// if taget is a stealth unit, make it visible on all clients
		if (target->data.isStealthOn != TERRAIN_NONE)
		{
			const std::vector<cPlayer*>& playerList = server->PlayerList;
			for (unsigned int i = 0; i < playerList.size(); i++)
			{
				cPlayer* player = playerList[i];
				if (target->owner == player)
					continue;
				if (!player->ScanMap[offset])
					continue;
				target->setDetectedByPlayer (*server, player);
			}
			server->checkPlayerUnits();
		}

		id = target->iID;
		target->data.hitpointsCur = target->calcHealth (damage);
		remainingHP = target->data.hitpointsCur;
		target->hasBeenAttacked = true;
		owner = target->owner;
		Log.write (" Server: unit '" + target->getDisplayName() + "' (ID: " + iToStr (target->iID) + ") hit. Remaining HP: " + iToStr (target->data.hitpointsCur), cLog::eLOG_TYPE_NET_DEBUG);
	}

	// workaround
	// make sure, the owner gets the impact message
	if (owner)
		owner->ScanMap[offset] = 1;

	sendAttackJobImpact (offset, remainingHP, id);

	// remove the destroyed units
	if (target && target->data.hitpointsCur <= 0)
	{
		if (target->isABuilding())
			server->destroyUnit (*static_cast<cBuilding*> (target));
		else
			server->destroyUnit (*static_cast<cVehicle*> (target));

		target = 0;
	}

	// attack finished. reset attacking and isBeeingAttacked flags
	if (target)
		target->isBeeingAttacked = false;

	if (isAir == false)
	{
		std::vector<cBuilding*>& buildings = map[offset].getBuildings();
		for (std::vector<cBuilding*>::iterator it = buildings.begin(); it != buildings.end(); ++it)
			(*it)->isBeeingAttacked = false;
	}
	if (unit)
		unit->attacking = false;

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
	const int mapSize = server->Map->getSize();
	const int clusterDamage = damage;
	const int PosX = iTargetOff % mapSize;
	const int PosY = iTargetOff / mapSize;

	//full damage
	makeImpact (PosX, PosY);

	// 3/4 damage
	damage = (clusterDamage * 3) / 4;
	makeImpact (PosX - 1, PosY);
	makeImpact (PosX + 1, PosY);
	makeImpact (PosX    , PosY - 1);
	makeImpact (PosX    , PosY + 1);

	// 1/2 damage
	damage = clusterDamage / 2;
	makeImpact (PosX + 1, PosY + 1);
	makeImpact (PosX + 1, PosY - 1);
	makeImpact (PosX - 1, PosY + 1);
	makeImpact (PosX - 1, PosY - 1);

	// 1/3 damage
	damage = clusterDamage / 3;
	makeImpact (PosX - 2, PosY);
	makeImpact (PosX + 2, PosY);
	makeImpact (PosX    , PosY - 2);
	makeImpact (PosX    , PosY + 2);
}

//--------------------------------------------------------------------------
void cServerAttackJob::sendAttackJobImpact (int offset, int remainingHP, int id)
{
	const std::vector<cPlayer*>& playerList = server->PlayerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		const cPlayer* player = playerList[i];

		// target in sight?
		if (player->ScanMap[offset] == 0)
			continue;

		AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_ATTACKJOB_IMPACT));
		message->pushInt32 (offset);
		message->pushInt16 (remainingHP);
		message->pushInt16 (id);

		server->sendNetMessage (message, player->getNr());
	}
}

//--------------------------------------------------------------------------
// cClientAttackJob implementation
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void cClientAttackJob::lockTarget (cClient& client, cNetMessage* message)
{
	const bool bIsAir = message->popBool();
	const int offset = message->popInt32();
	cMap& map = *client.getMap();
	const int x = offset % map.getSize();
	const int y = offset / map.getSize();
	const int ID = message->popInt32();
	if (ID != 0)
	{
		cVehicle* vehicle = client.getVehicleFromID (ID);
		if (vehicle == NULL)
		{
			// we are out of sync!!!
			Log.write (" Client: vehicle with ID " + iToStr (ID) + " not found", cLog::eLOG_TYPE_NET_ERROR);
			return;
		}

		vehicle->isBeeingAttacked = true;

		// synchonize position
		if (vehicle->PosX != x || vehicle->PosY != y)
		{
			Log.write (" Client: changed vehicle position to (" + iToStr (x) + ":" + iToStr (y) + ")", cLog::eLOG_TYPE_NET_DEBUG);
			map.moveVehicle (*vehicle, x, y);
			vehicle->owner->doScan();

			vehicle->OffY = message->popChar();
			vehicle->OffX = message->popChar();
		}
	}
	if (!bIsAir)
	{
		std::vector<cBuilding*>& buildings = map[offset].getBuildings();
		for (std::vector<cBuilding*>::iterator it = buildings.begin(); it != buildings.end(); ++it)
			(*it)->isBeeingAttacked = true;
	}
}

//--------------------------------------------------------------------------
void cClientAttackJob::handleAttackJobs (cClient& client, cMenu* activeMenu)
{
	for (size_t i = 0; i != client.attackJobs.size(); ++i)
	{
		cClientAttackJob* job = client.attackJobs[i];
		switch (job->state)
		{
			case FINISHED:
			{
				job->sendFinishMessage (client);
				if (job->unit) job->unit->attacking = false;
				delete job;
				client.attackJobs.erase (client.attackJobs.begin() + i);
				--i;
				break;
			}
			case PLAYING_MUZZLE:
			{
				job->playMuzzle (client, activeMenu);
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
cClientAttackJob::cClientAttackJob (cClient* client, cNetMessage* message)
{
	state = ROTATING;
	wait = 0;
	length = 0;
	this->iID = message->popInt16();
	iTargetOffset = -1;
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

	const int unitID = message->popInt32();

	// get muzzle type and aggressor position
	if (unitID == 0)
	{
		iMuzzleType = message->popChar();
		if (iMuzzleType != sUnitData::MUZZLE_TYPE_ROCKET && iMuzzleType != sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER && iMuzzleType != sUnitData::MUZZLE_TYPE_TORPEDO)
		{
			state = FINISHED;
			return;
		}
		iAgressorOffset = message->popInt32();
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
		iAgressorOffset = client->getMap()->getOffset (unit->PosX, unit->PosY);
	}

	if (iMuzzleType == sUnitData::MUZZLE_TYPE_ROCKET || iMuzzleType == sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER || iMuzzleType == sUnitData::MUZZLE_TYPE_TORPEDO)
	{
		iTargetOffset = message->popInt32();
	}
	iFireDir = message->popChar();

	// get remaining shots, ammo and movement points
	if (unit)
	{
		unit->data.shotsCur = message->popInt16();
		unit->data.ammoCur = message->popInt16();
		unit->attacking = true;
		if (unit->isAVehicle())
			unit->data.speedCur = message->popInt16();
	}
	const bool sentryReaction = message->popBool();
	if (sentryReaction && unit && unit->owner == &client->getActivePlayer())
	{
		const int x = unit->PosX;
		const int y = unit->PosY;
		const string name = unit->getDisplayName();
		const sID id = unit->data.ID;
		const sSavedReportMessage& report = client->getActivePlayer().addSavedReport (lngPack.i18n ("Text~Comp~AttackingEnemy", name), sSavedReportMessage::REPORT_TYPE_UNIT, id, x, y);
		client->getGameGUI().addCoords (report);
		PlayRandomVoice (VoiceData.VOIAttackingEnemy);
	}
	client->getGameGUI().checkMouseInputMode();
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
void cClientAttackJob::playMuzzle (cClient& client, cMenu* activeMenu)
{
	int offx = 0, offy = 0;
	const cMap& map = *client.getMap();

	if (unit && unit->isABuilding() && unit->data.explodesOnContact)
	{
		cBuilding* building = static_cast<cBuilding*> (unit);
		state = FINISHED;
		PlayFX (building->uiData->Attack);
		if (map.isWaterOrCoast (unit->PosX, unit->PosY))
		{
			client.addFx (new cFxExploWater (unit->PosX * 64 + 32, unit->PosY * 64 + 32));
		}
		else
		{
			client.addFx (new cFxExploSmall (unit->PosX * 64 + 32, unit->PosY * 64 + 32));
		}
		client.deleteUnit (building, activeMenu);
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
					offy -= 40;
					break;
				case 1:
					offx += 32;
					offy -= 32;
					break;
				case 2:
					offx += 40;
					break;
				case 3:
					offx += 32;
					offy += 32;
					break;
				case 4:
					offy += 40;
					break;
				case 5:
					offx -= 32;
					offy += 32;
					break;
				case 6:
					offx -= 40;
					break;
				case 7:
					offx -= 32;
					offy -= 32;
					break;
			}
			if (unit)
			{
				client.addFx (new cFxMuzzleBig (unit->PosX * 64 + offx, unit->PosY * 64 + offy, iFireDir));
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
				client.addFx (new cFxMuzzleSmall (unit->PosX * 64, unit->PosY * 64, iFireDir));
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

			int PosX = iAgressorOffset % map.getSize();
			int PosY = iAgressorOffset / map.getSize();
			int endX = iTargetOffset % map.getSize();
			int endY = iTargetOffset / map.getSize();
			cFx* rocket = new cFxRocket (PosX * 64 + 32, PosY * 64 + 32, endX * 64 + 32, endY * 64 + 32, iFireDir, false);
			length = rocket->getLength() / 5;
			client.addFx (rocket);
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
					offy -= 20;
					break;
				case 1:
					offx += 12;
					offy -= 12;
					break;
				case 2:
					offx += 20;
					break;
				case 3:
					offx += 12;
					offy += 12;
					break;
				case 4:
					offy += 20;
					break;
				case 5:
					offx -= 12;
					offy += 12;
					break;
				case 6:
					offx -= 20;
					break;
				case 7:
					offx -= 12;
					offy -= 12;
					break;
			}
			if (iMuzzleType == sUnitData::MUZZLE_TYPE_MED)
			{
				if (unit)
				{
					client.addFx (new cFxMuzzleMed (unit->PosX * 64 + offx, unit->PosY * 64 + offy, iFireDir));
				}
			}
			else
			{
				if (unit)
				{
					client.addFx (new cFxMuzzleMedLong (unit->PosX * 64 + offx, unit->PosY * 64 + offy, iFireDir));
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

			int PosX = iAgressorOffset % map.getSize();
			int PosY = iAgressorOffset / map.getSize();
			int endX = iTargetOffset % map.getSize();
			int endY = iTargetOffset / map.getSize();
			cFx* rocket = new cFxRocket (PosX * 64 + 32, PosY * 64 + 32, endX * 64 + 32, endY * 64 + 32, iFireDir, true);
			length = rocket->getLength() / 5;
			client.addFx (rocket);

			break;
		}
		case sUnitData::MUZZLE_TYPE_SNIPER:
			state = FINISHED;
			break;
	}
	if (unit && unit->isAVehicle())
	{
		PlayFX (static_cast<cVehicle*> (unit)->uiData->Attack);
	}
	else if (unit && unit->isABuilding())
	{
		PlayFX (static_cast<cBuilding*> (unit)->uiData->Attack);
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
void cClientAttackJob::makeImpact (cClient& client, int offset, int remainingHP, int id)
{
	cMap& map = *client.getMap();
	if (map.isValidOffset (offset) == false)
	{
		Log.write (" Client: Invalid offset", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}

	cVehicle* targetVehicle = client.getVehicleFromID (id);
	cBuilding* targetBuilding = client.getBuildingFromID (id);

	bool playImpact = false;
	bool ownUnit = false;
	bool destroyed = false;
	bool isAir = false;
	string name;
	int offX = 0;
	int offY = 0;
	sID unitID;

	// no target found
	if (!targetBuilding && !targetVehicle)
	{
		playImpact = true;
	}
	else
	{

		if (targetVehicle)
		{
			unitID = targetVehicle->data.ID;
			isAir = (targetVehicle->data.factorAir > 0);
			targetVehicle->data.hitpointsCur = remainingHP;

			Log.write (" Client: vehicle '" + targetVehicle->getDisplayName() + "' (ID: " + iToStr (targetVehicle->iID) + ") hit. Remaining HP: " + iToStr (targetVehicle->data.hitpointsCur), cLog::eLOG_TYPE_NET_DEBUG);

			name = targetVehicle->getDisplayName();
			if (targetVehicle->owner == &client.getActivePlayer()) ownUnit = true;

			if (targetVehicle->data.hitpointsCur <= 0)
			{
				client.destroyUnit (*targetVehicle);
				targetVehicle = NULL;
				destroyed = true;
			}
			else
			{
				playImpact = true;
				offX = targetVehicle->OffX;
				offY = targetVehicle->OffY;
			}
		}
		else
		{
			unitID = targetBuilding->data.ID;
			targetBuilding->data.hitpointsCur = remainingHP;

			Log.write (" Client: building '" + targetBuilding->getDisplayName() + "' (ID: " + iToStr (targetBuilding->iID) + ") hit. Remaining HP: " + iToStr (targetBuilding->data.hitpointsCur), cLog::eLOG_TYPE_NET_DEBUG);

			name = targetBuilding->getDisplayName();
			if (targetBuilding->owner == &client.getActivePlayer()) ownUnit = true;

			if (targetBuilding->data.hitpointsCur <= 0)
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
		client.getGameGUI().updateMouseCursor();
	}

	const int x = offset % map.getSize();
	const int y = offset / map.getSize();

	if (playImpact && cSettings::getInstance().isAlphaEffects())
	{
		// TODO:  PlayFX (SoundData.hit);
		client.addFx (new cFxHit (x * 64 + offX + 32, y * 64 + offY + 32));
	}

	if (ownUnit)
	{
		string message;

		if (destroyed)
		{
			message = name + " " + lngPack.i18n ("Text~Comp~Destroyed");
			PlayRandomVoice (VoiceData.VOIDestroyedUs);
		}
		else
		{
			message = name + " " + lngPack.i18n ("Text~Comp~Attacked");
			PlayRandomVoice (VoiceData.VOIAttackingUs);
		}
		const sSavedReportMessage& report = client.getActivePlayer().addSavedReport (message, sSavedReportMessage::REPORT_TYPE_UNIT, unitID, x, y);
		client.getGameGUI().addCoords (report);
	}

	// clean up
	if (targetVehicle) targetVehicle->isBeeingAttacked = false;

	if (!isAir)
	{
		std::vector<cBuilding*>& buildings = map[offset].getBuildings();
		for (std::vector<cBuilding*>::iterator it = buildings.begin(); it != buildings.end(); ++it)
			(*it)->isBeeingAttacked = false;
	}
}
