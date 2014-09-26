
#include <algorithm>

#include "unit.h"
#include "map.h"
#include "buildings.h"
#include "vehicles.h"
#include "netmessage.h"
#include "clientevents.h"
#include "server.h"
#include "client.h"
#include "fxeffects.h"
#include "hud.h"
#include "log.h"

#include "attackJob2.h"

//TODO: unlocking bei allen gelegenheiten, wo eine Unit verschwindet/bewegt


//TODO: alien angriff luft + boden
//TODO: sentry attacks
//TODO: load/save attackjobs + isAttacking/isAttacked

/*tests:
+ angreifer außer sichtweite
+ ziel außer sichtweite
- angreifer unsichtbar
- ziel unsichtbar
- sentry
- mehrfaches Sentry auf ein Feld
- sentry einer Unit auf mehrere Ziele
- sounds nur bei sichtbaren Quellen

*/


//--------------------------------------------------------------------------
cUnit* cAttackJob::selectTarget (int x, int y, char attackMode, const cMap& map, cPlayer* owner)
{
	cVehicle* targetVehicle = NULL;
	cBuilding* targetBuilding = NULL;
	const int offset = map.getOffset (x, y);
	cMapField& mapField = map[offset];

	//planes
	//prefere enemy planes. But select own one, if there is no enemy
	auto planes = mapField.getPlanes();
	for (cVehicle* plane : planes)
	{
		if (plane->FlightHigh >  0 && !(attackMode & TERRAIN_AIR))    continue;
		if (plane->FlightHigh == 0 && !(attackMode & TERRAIN_GROUND)) continue;

		if (targetVehicle == NULL)
		{
			targetVehicle = plane;
		}
		else if (targetVehicle->owner == owner)
		{
			if (plane->owner != owner)
			{
				targetVehicle = plane;
			}
		}
	}

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

void cAttackJob::runAttackJobs(std::vector<cAttackJob*>& attackJobs, cMenu* activeMenu)
{
	for (auto &attackJob : attackJobs)
	{
		attackJob->run(activeMenu);
		if (attackJob->finished())
		{
			delete attackJob;
			attackJob = NULL;
		}
	}
	attackJobs.erase(std::remove(attackJobs.begin(), attackJobs.end(), static_cast<cAttackJob*> (NULL)), attackJobs.end());
}

//--------------------------------------------------------------------------
cAttackJob::cAttackJob(cServer* server_, cUnit* aggressor_, int targetX_, int targetY_) :
	aggressorID(aggressor_->iID),
	aggressorPlayerNr(aggressor_->owner->getNr()),
	aggressorPosX(aggressor_->PosX),
	aggressorPosY(aggressor_->PosY),
	attackMode(aggressor_->data.canAttack),
	muzzleType(aggressor_->data.muzzleType),
	attackPoints(aggressor_->data.damage),
	targetX(targetX_),
	targetY(targetY_),
	server(server_),
	client(NULL),
	state(S_ROTATING),
	destroyedTargets(NULL),
	fireDir(0)
{
	fireDir = calcFireDir();
	counter = calcTimeForRotation() + FIRE_DELAY;

	Log.write(" Server: Created AttackJob. Aggressor: " + aggressor_->getDisplayName() + " (ID: " + iToStr(aggressor_->iID) + ") at (" + iToStr(aggressorPosX) + "," + iToStr(aggressorPosY) + "). Target: (" + iToStr(targetX) + "," + iToStr(targetY) + ")." , cLog::eLOG_TYPE_NET_DEBUG);

	lockTarget();

	AutoPtr<cNetMessage> message(serialize());
	server->sendNetMessage(message);

	//lock agressor
	aggressor_->attacking = true;

	// make the aggressor visible on all clients
	// who can see the aggressor offset
	for (auto player : server->PlayerList)
	{
		if (player->canSeeAnyAreaUnder(*aggressor_) == false) continue;
		if (aggressor_->owner == player) continue;

		aggressor_->setDetectedByPlayer(*server, player);
	}
}

cAttackJob::cAttackJob(cClient* client_, cNetMessage& message) :
	client(client_),
	server(NULL)
{
	state = static_cast<cAttackJob::eAJStates> (message.popInt16());
	counter = message.popInt16();
	targetY = message.popInt16();
	targetX = message.popInt16();
	attackPoints = message.popInt16();
	muzzleType = message.popInt16();
	attackMode = message.popInt16();
	aggressorPosY = message.popInt16();
	aggressorPosX = message.popInt16();
	aggressorPlayerNr = message.popInt16();
	fireDir = message.popInt16();
	aggressorID = message.popInt32();

	cUnit* aggressor = client->getUnitFromID(aggressorID);
	if (aggressor)
		aggressor->attacking = true;

	if (aggressor)
		Log.write(" Client: Received AttackJob. Aggressor: " + aggressor->getDisplayName() + " (ID: " + iToStr(aggressor->iID) + ") at (" + iToStr(aggressorPosX) + "," + iToStr(aggressorPosY) + "). Target: (" + iToStr(targetX) + "," + iToStr(targetY) + ").", cLog::eLOG_TYPE_NET_DEBUG);
	else
		Log.write(" Client: Received AttackJob. Aggressor: instance not present on client (ID: " + iToStr(aggressorID) + ") at(" + iToStr(aggressorPosX) + ", " + iToStr(aggressorPosY) + ").Target: (" + iToStr(targetX) + ", " + iToStr(targetY) + ").", cLog::eLOG_TYPE_NET_DEBUG);

	lockTarget();

}

cNetMessage* cAttackJob::serialize() const
{
	cNetMessage* message = new cNetMessage(GAME_EV_ATTACKJOB);
	message->pushInt32(aggressorID);
	message->pushInt16(fireDir);
	message->pushInt16(aggressorPlayerNr);
	message->pushInt16(aggressorPosX);
	message->pushInt16(aggressorPosY);
	message->pushInt16(attackMode);
	message->pushInt16(muzzleType);
	message->pushInt16(attackPoints);
	message->pushInt16(targetX);
	message->pushInt16(targetY);
	message->pushInt16(counter);
	message->pushInt16(state);

	return message;
}


void cAttackJob::run(cMenu*activeMenu)
{
	if (counter > 0)
	{
		counter--;
	}

	switch (state)
	{
		case S_ROTATING:
		{
			cUnit* aggressor = getAggressor();
			if (aggressor && (counter % ROTATION_SPEED) == 0)
				aggressor->rotateTo(fireDir);

			if (counter == 0)
			{
				fire();
				state = S_FIRING;
			}
			break;
		}
		case S_FIRING:
			if (counter == 0)
			{
				bool destroyed = impact(activeMenu);
				if (destroyed)
				{
					counter = DESTROY_DELAY;
					state = S_EXPLODING;
				}
				else
				{
					state = S_FINISHED;
				}
			}
			break;
		case S_EXPLODING:
			if (counter == 0)
			{
				destroyTarget();
				state = S_FINISHED;
			}
		case S_FINISHED:
		default:
			break;
	}
}

bool cAttackJob::finished() const
{
	return state == S_FINISHED;
}

//---------------------------------------------
// private functions

int cAttackJob::calcFireDir()
{
	float dx = (float) (targetX - aggressorPosX);
	float dy = (float)-(targetY - aggressorPosY);
	float r = sqrtf(dx * dx + dy * dy);

	int fireDir = getAggressor()->dir;
	if (r <= 0.001f)
	{
		// do not rotate aggressor
	}
	else
	{
		// 360 / (2 * PI) = 57.29577951f;
		dx /= r;
		dy /= r;
		r = asinf(dx) * 57.29577951f;
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

	return fireDir;
}

int cAttackJob::calcTimeForRotation()
{
	int diff = abs(getAggressor()->dir - fireDir);
	if (diff > 4) diff = 8 - diff;
	
	return diff * ROTATION_SPEED;
}

cUnit* cAttackJob::getAggressor()
{
	if (server)
		return server->getUnitFromID(aggressorID);
	else
		return client->getUnitFromID(aggressorID);
}

void cAttackJob::lockTarget()
{
	cPlayer* player = client ? client->getPlayerFromNumber(aggressorPlayerNr) : server->getPlayerFromNumber(aggressorPlayerNr);
	cMap&    map = client ? *client->getMap() : *server->Map;

	int range = 0;
	if (muzzleType == sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER)
		range = 2; 
		
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			if (abs(x) + abs(y) <= range && map.isValidPos(targetX + x, targetY + y))
			{
				cUnit* target = selectTarget(targetX + x, targetY + y, attackMode, map, player);
				if (target)
				{
					target->isBeeingAttacked = true;
					lockedTargets.push_back(target->iID);
					Log.write(" AttackJob locked target " + target->getDisplayName() + " (ID: " + iToStr(target->iID) + ") at (" + iToStr(targetX + x) + "," + iToStr(targetY + y) + ")", cLog::eLOG_TYPE_NET_DEBUG);
				}
			}
		}
	}
}

void cAttackJob::fire()
{
	cUnit* aggressor = getAggressor();
	
	//update data
	if (aggressor)
	{
		aggressor->data.shotsCur--;
		aggressor->data.ammoCur--;
		if (aggressor->isAVehicle() && aggressor->data.canDriveAndFire == false)
			aggressor->data.speedCur -= (int) (((float) aggressor->data.speedMax) / aggressor->data.shotsMax);

		if (client)
			client->getGameGUI().checkMouseInputMode();
	}

	//set timer for next state
	cFx* muzzle = createMuzzleFx();
	if (muzzle)
		counter = muzzle->getLength() + IMPACT_DELAY;

	//play muzzle flash / fire rocket
	if (client)
	{
		if (muzzle)
			client->addFx (muzzle, aggressor != NULL);
	}
	else
	{
		delete muzzle;
	}

	//play sound
	if (aggressor && client)
	{
		if (aggressor->isAVehicle())
		{
			//TODO: play sound only for clients with active GUI
			PlayFX (static_cast<cVehicle*> (aggressor)->uiData->Attack);
		}
		else
		{
			PlayFX (static_cast<cBuilding*> (aggressor)->uiData->Attack);
		}
	}

	
}

cFx* cAttackJob::createMuzzleFx()
{
	//TODO: this shouldn't be in the attackjob class. But since
	//the attackjobs doesn't always have an instance of the unit,
	//it stays here for now

	int offx = 0, offy = 0;
	switch (muzzleType)
	{
		case sUnitData::MUZZLE_TYPE_BIG:
			switch (fireDir)
			{
				case 0:
					offy = -40;
					break;
				case 1:
					offx =  32;
					offy = -32;
					break;
				case 2:
					offx =  40;
					break;
				case 3:
					offx =  32;
					offy =  32;
					break;
				case 4:
					offy =  40;
					break;
				case 5:
					offx = -32;
					offy =  32;
					break;
				case 6:
					offx = -40;
					break;
				case 7:
					offx = -32;
					offy = -32;
					break;
			}
			return new cFxMuzzleBig (aggressorPosX * 64 + offx, aggressorPosY * 64 + offy, fireDir);

		case sUnitData::MUZZLE_TYPE_SMALL:
			return new cFxMuzzleSmall (aggressorPosX * 64, aggressorPosY * 64, fireDir);

		case sUnitData::MUZZLE_TYPE_ROCKET:
		case sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER:
			return new cFxRocket (aggressorPosX * 64 + 32, aggressorPosY * 64 + 32, targetX * 64 + 32, targetY * 64 + 32, fireDir, false);

		case sUnitData::MUZZLE_TYPE_MED:
		case sUnitData::MUZZLE_TYPE_MED_LONG:
			switch (fireDir)
			{
				case 0:
					offy = -20;
					break;
				case 1:
					offx =  12;
					offy = -12;
					break;
				case 2:
					offx =  20;
					break;
				case 3:
					offx =  12;
					offy =  12;
					break;
				case 4:
					offy =  20;
					break;
				case 5:
					offx = -12;
					offy =  12;
					break;
				case 6:
					offx = -20;
					break;
				case 7:
					offx = -12;
					offy = -12;
					break;
			}
			if (muzzleType == sUnitData::MUZZLE_TYPE_MED)
				return new cFxMuzzleMed (aggressorPosX * 64 + offx, aggressorPosY * 64 + offy, fireDir);
			else
				return new cFxMuzzleMedLong (aggressorPosX * 64 + offx, aggressorPosY * 64 + offy, fireDir);

		case sUnitData::MUZZLE_TYPE_TORPEDO:
			return new cFxRocket (aggressorPosX * 64 + 32, aggressorPosY * 64 + 32, targetX * 64 + 32, targetY * 64 + 32, fireDir, true);
		case sUnitData::MUZZLE_TYPE_SNIPER:
			//TODO: sniper hat keine animation?!?
		default:
			return NULL;
	}
}

bool cAttackJob::impact(cMenu* activeMenu)
{
	if (muzzleType == sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER)
		return impactCluster(activeMenu);
	else
		return impactSingle(activeMenu, targetX, targetY);

	// unlock targets in case they were locked at the beginning of the attack, but are not hit by the impact
	// for example a plane flies on the target field and takes the shot in place of the original plane
	for (auto unitId : lockedTargets)
	{
		cUnit* unit;
		if (server)
			unit = server->getUnitFromID(unitId);
		else
			unit = client->getUnitFromID(unitId);
		
		if (unit)
			unit->isBeeingAttacked = false;
	}
}

bool cAttackJob::impactCluster(cMenu* activeMenu)
{
	const int clusterDamage = attackPoints;
	bool destroyed = false;
	std::vector<cUnit*> targets;

	//full damage
	destroyed = destroyed || impactSingle(activeMenu, targetX, targetY, &targets);

	// 3/4 damage
	attackPoints = (clusterDamage * 3) / 4;
	destroyed = destroyed || impactSingle(activeMenu, targetX - 1, targetY,     &targets);
	destroyed = destroyed || impactSingle(activeMenu, targetX + 1, targetY,     &targets);
	destroyed = destroyed || impactSingle(activeMenu, targetX,     targetY - 1, &targets);
	destroyed = destroyed || impactSingle(activeMenu, targetX,     targetY + 1, &targets);
	
	// 1/2 damage
	attackPoints = clusterDamage / 2;
	destroyed = destroyed || impactSingle(activeMenu, targetX + 1, targetY + 1, &targets);
	destroyed = destroyed || impactSingle(activeMenu, targetX + 1, targetY - 1, &targets);
	destroyed = destroyed || impactSingle(activeMenu, targetX - 1, targetY + 1, &targets);
	destroyed = destroyed || impactSingle(activeMenu, targetX - 1, targetY - 1, &targets);

	// 1/3 damage
	attackPoints = clusterDamage / 3;
	destroyed = destroyed || impactSingle(activeMenu, targetX - 2, targetY    , &targets);
	destroyed = destroyed || impactSingle(activeMenu, targetX + 2, targetY    , &targets);
	destroyed = destroyed || impactSingle(activeMenu, targetX    , targetY - 2, &targets);
	destroyed = destroyed || impactSingle(activeMenu, targetX    , targetY + 2, &targets);

	return destroyed;
}

bool cAttackJob::impactSingle(cMenu* activeMenu, int x, int y, std::vector<cUnit*>* avoidTargets)
{
	//select target
	cPlayer* player = client ? client->getPlayerFromNumber(aggressorPlayerNr) : server->getPlayerFromNumber(aggressorPlayerNr);
	cMap&    map    = client ? *client->getMap() : *server->Map;

	if (!map.isValidPos(x, y))
		return false;
	
	cUnit* target = selectTarget(x, y, attackMode, map, player);

	//check list of units that will be ignored as target.
	//Used to prevent, that cluster attacks hit the same unit multible times
	if (avoidTargets)
	{
		for (auto unit : *avoidTargets)
		{
			if (unit == target)
				return false;
		}
		avoidTargets->push_back(target);
	}

	int offX = 0, offY = 0;
	if (target && target->isAVehicle())
	{
		offX = static_cast<cVehicle*>(target)->OffX;
		offY = static_cast<cVehicle*>(target)->OffY;
	}

	bool destroyed = false;
	std::string name;
	sID unitID;

	// if taget is a stealth unit, make it visible on all clients
	if (server && target && target->data.isStealthOn != TERRAIN_NONE)
	{
		for (auto player : server->PlayerList)
		{
			if (target->owner == player) continue;
			if (!player->canSeeAnyAreaUnder(*target)) continue;

			target->setDetectedByPlayer(*server, player);
		}
	}

	//make impact on target
	if (target)
	{
		target->data.hitpointsCur = target->calcHealth(attackPoints);
		target->hasBeenAttacked = true;
		target->isBeeingAttacked = false;

		name = target->getDisplayName();
		unitID = target->data.ID;

		if (target->data.hitpointsCur <= 0)
		{
			target->isBeeingAttacked = true;
			destroyed = true;
			destroyedTargets.push_back(target->iID);
			if (client)
			{
				if (target->isAVehicle())
					client->addDestroyFx (*static_cast<cVehicle*>(target));
				else
					client->addDestroyFx (*static_cast<cBuilding*>(target));
			}
		}
	}

	auto aggressor = getAggressor();
	if (aggressor && aggressor->data.explodesOnContact && aggressorPosX == x && aggressorPosY == y)
	{
		if (client)
		{
			cBuilding& b = *static_cast<cBuilding*> (aggressor);

			PlayFX(b.uiData->Attack);
			if (map.isWaterOrCoast(b.PosX, b.PosY))
			{
				client->addFx(new cFxExploWater(b.PosX * 64 + 32, b.PosY * 64 + 32));
			}
			else
			{
				client->addFx(new cFxExploSmall(b.PosX * 64 + 32, b.PosY * 64 + 32));
			}
			client->deleteUnit(aggressor, activeMenu);
		}
		else
		{
			// delete unit is only called on server, because it sends 
			// all nessesary net messages to update the client
			server->deleteUnit(aggressor, false);
		}
		aggressor = NULL;
	}
	else if (!destroyed && client)
	{
		// TODO:  PlayFX (SoundData.hit); in cFxHit!
		client->addFx(new cFxHit(x * 64 + offX + 32, y * 64 + offY + 32), target != NULL);
	}

	//make message
	if (client && target && target->owner->getNr() == client->getActivePlayer().getNr())
	{
		std::string message;

		if (destroyed)
		{
			message = name + " " + lngPack.i18n("Text~Comp~Destroyed");
			PlayRandomVoice(VoiceData.VOIDestroyedUs);
		}
		else
		{
			message = name + " " + lngPack.i18n("Text~Comp~Attacked");
			PlayRandomVoice(VoiceData.VOIAttackingUs);
		}
		const sSavedReportMessage& report = client->getActivePlayer().addSavedReport(message, sSavedReportMessage::REPORT_TYPE_UNIT, unitID, targetX, targetY);
		client->getGameGUI().addCoords(report);
	}

	if (aggressor)
		aggressor->attacking = false;

	if (client)
		client->getGameGUI().updateMouseCursor();

	if (target)
		Log.write(std::string(server ? " Server: " : " Client: ") + "AttackJob Impact. Target: " + target->getDisplayName() + " (ID: " + iToStr(target->iID) + ") at (" + iToStr(targetX) + "," + iToStr(targetY) + "), Remaining HP: " + iToStr(target->data.hitpointsCur), cLog::eLOG_TYPE_NET_DEBUG);
	else
		Log.write(std::string(server ? " Server: " : " Client: ") + " AttackJob Impact. Target: none (" + iToStr(targetX) + "," + iToStr(targetY) + ")", cLog::eLOG_TYPE_NET_DEBUG);


	return destroyed;
}

void cAttackJob::destroyTarget()
{
	// destroy unit is only called on server, because it sends 
	// all nessesary net messages to update the client
	if (server)
	{
		for (auto targetId : destroyedTargets)
		{
			cUnit* unit = server->getUnitFromID(targetId);
			if (unit)
			{
				Log.write(" Server: AttackJob destroyed unit " + unit->getDisplayName() + " (ID: " + iToStr(unit->iID) + ") at (" + iToStr(unit->PosX) + "," + iToStr(unit->PosY) + ")", cLog::eLOG_TYPE_NET_DEBUG);
				server->destroyUnit(*unit);
			}
		}
	}
}
