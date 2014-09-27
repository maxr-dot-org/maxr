
#include <algorithm>

#include "game/logic/attackjob.h"

#include "game/data/units/unit.h"
#include "game/data/map/map.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/data/player/player.h"
#include "game/data/report/unit/savedreportdestroyed.h"
#include "game/data/report/unit/savedreportattacked.h"
#include "netmessage.h"
#include "clientevents.h"
#include "server.h"
#include "client.h"
#include "fxeffects.h"
#include "utility/log.h"
#include "output/sound/sounddevice.h"
#include "output/sound/soundchannel.h"


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
cUnit* cAttackJob::selectTarget (const cPosition& position, char attackMode, const cMap& map, cPlayer* owner)
{
	cVehicle* targetVehicle = NULL;
	cBuilding* targetBuilding = NULL;
	const cMapField& mapField = map.getField(position);

	//planes
	//prefere enemy planes. But select own one, if there is no enemy
	auto planes = mapField.getPlanes();
	for (cVehicle* plane : planes)
	{
		if (plane->getFlightHeight() >  0 && !(attackMode & TERRAIN_AIR))    continue;
		if (plane->getFlightHeight () == 0 && !(attackMode & TERRAIN_GROUND)) continue;

		if (targetVehicle == NULL)
		{
			targetVehicle = plane;
		}
		else if (targetVehicle->getOwner() == owner)
		{
			if (plane->getOwner() != owner)
			{
				targetVehicle = plane;
			}
		}
	}

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
		if (targetBuilding && !targetBuilding->getOwner()) targetBuilding = NULL;
	}

	if (targetVehicle) return targetVehicle;
	return targetBuilding;
}

void cAttackJob::runAttackJobs(std::vector<cAttackJob*>& attackJobs)
{
	for (auto &attackJob : attackJobs)
	{
		attackJob->run();
		if (attackJob->finished())
		{
			delete attackJob;
			attackJob = NULL;
		}
	}
	attackJobs.erase(std::remove(attackJobs.begin(), attackJobs.end(), static_cast<cAttackJob*> (NULL)), attackJobs.end());
}

//--------------------------------------------------------------------------
cAttackJob::cAttackJob (cServer* server_, cUnit* aggressor_, const cPosition& targetPosition_) :
	aggressorID(aggressor_->iID),
	aggressorPlayerNr(aggressor_->getOwner()->getNr()),
	aggressorPosition(aggressor_->getPosition()),
	attackMode(aggressor_->data.canAttack),
	muzzleType(aggressor_->data.muzzleType),
	attackPoints(aggressor_->data.getDamage()),
	targetPosition (targetPosition_),
	server(server_),
	client(NULL),
	state(S_ROTATING),
	destroyedTargets(NULL),
	fireDir(0)
{
	fireDir = calcFireDir();
	counter = calcTimeForRotation() + FIRE_DELAY;

	Log.write (" Server: Created AttackJob. Aggressor: " + aggressor_->getDisplayName () + " (ID: " + iToStr (aggressor_->iID) + ") at (" + iToStr (aggressorPosition.x ()) + "," + iToStr (aggressorPosition.y ()) + "). Target: (" + iToStr (targetPosition.x()) + "," + iToStr (targetPosition.y()) + ").", cLog::eLOG_TYPE_NET_DEBUG);

	lockTarget();

	AutoPtr<cNetMessage> message(serialize().release());
	server->sendNetMessage(message);

	//lock agressor
	aggressor_->setAttacking(true);

	// make the aggressor visible on all clients
	// who can see the aggressor offset
	for (const auto& player : server->playerList)
	{
		if (player->canSeeAnyAreaUnder(*aggressor_) == false) continue;
		if (aggressor_->getOwner() == player.get()) continue;

		aggressor_->setDetectedByPlayer(*server, player.get());
	}
}

cAttackJob::cAttackJob(cClient* client_, cNetMessage& message) :
	client(client_),
	server(NULL)
{
	state = static_cast<cAttackJob::eAJStates> (message.popInt16());
	counter = message.popInt16();
	targetPosition = message.popPosition ();
	attackPoints = message.popInt16();
	muzzleType = message.popInt16();
	attackMode = message.popInt16();
	aggressorPosition = message.popPosition ();
	aggressorPlayerNr = message.popInt16();
	fireDir = message.popInt16();
	aggressorID = message.popInt32();

	cUnit* aggressor = client->getUnitFromID(aggressorID);
	if (aggressor)
		aggressor->setAttacking(true);

	if (aggressor)
		Log.write (" Client: Received AttackJob. Aggressor: " + aggressor->getDisplayName () + " (ID: " + iToStr (aggressor->iID) + ") at (" + iToStr (aggressorPosition.x ()) + "," + iToStr (aggressorPosition.y ()) + "). Target: (" + iToStr (targetPosition.x ()) + "," + iToStr (targetPosition.y ()) + ").", cLog::eLOG_TYPE_NET_DEBUG);
	else
		Log.write (" Client: Received AttackJob. Aggressor: instance not present on client (ID: " + iToStr (aggressorID) + ") at(" + iToStr (aggressorPosition.x ()) + ", " + iToStr (aggressorPosition.y ()) + ").Target: (" + iToStr (targetPosition.x ()) + ", " + iToStr (targetPosition.y ()) + ").", cLog::eLOG_TYPE_NET_DEBUG);

	lockTarget();

}

std::unique_ptr<cNetMessage> cAttackJob::serialize () const
{
	auto message = std::make_unique<cNetMessage>(GAME_EV_ATTACKJOB);
	message->pushInt32(aggressorID);
	message->pushInt16(fireDir);
	message->pushInt16(aggressorPlayerNr);
	message->pushPosition (aggressorPosition);
	message->pushInt16(attackMode);
	message->pushInt16(muzzleType);
	message->pushInt16(attackPoints);
	message->pushPosition (targetPosition);
	message->pushInt16(counter);
	message->pushInt16(state);

	return message;
}


void cAttackJob::run()
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
				bool destroyed = impact();
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
	auto dx = (float) (targetPosition.x () - aggressorPosition.x ());
	auto dy = (float)-(targetPosition.y () - aggressorPosition.y ());
	auto r = std::sqrt(dx * dx + dy * dy);

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
	cPlayer* player = client ? client->getPlayerFromNumber(aggressorPlayerNr) : &server->getPlayerFromNumber(aggressorPlayerNr);
	cMap&    map = client ? *client->getMap() : *server->Map;

	int range = 0;
	if (muzzleType == sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER)
		range = 2; 
		
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			if (abs(x) + abs(y) <= range && map.isValidPosition(targetPosition + cPosition(x, y)))
			{
				cUnit* target = selectTarget (targetPosition + cPosition (x, y), attackMode, map, player);
				if (target)
				{
					target->setIsBeeinAttacked (true);
					lockedTargets.push_back(target->iID);
					Log.write(" AttackJob locked target " + target->getDisplayName() + " (ID: " + iToStr(target->iID) + ") at (" + iToStr(targetPosition.x() + x) + "," + iToStr(targetPosition.y() + y) + ")", cLog::eLOG_TYPE_NET_DEBUG);
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
		aggressor->data.setShots(aggressor->data.getShots() - 1);
		aggressor->data.setAmmo(aggressor->data.getAmmo() - 1);
		if (aggressor->isAVehicle() && aggressor->data.canDriveAndFire == false)
			aggressor->data.speedCur -= (int) (((float) aggressor->data.speedMax) / aggressor->data.shotsMax);
	}

	//set timer for next state
	auto muzzle = createMuzzleFx();
	if (muzzle)
		counter = muzzle->getLength() + IMPACT_DELAY;

	//play muzzle flash / fire rocket
	if (client)
	{
		if (muzzle)
			client->addFx (std::move(muzzle), aggressor != NULL);
	}

	//play sound
	if (aggressor && client)
	{
		if (aggressor->isAVehicle())
		{
			//TODO: play sound only for clients with active GUI
			cSoundDevice::getInstance ().getFreeSoundEffectChannel ().play (static_cast<cVehicle*> (aggressor)->uiData->Attack);
		}
		else
		{
			cSoundDevice::getInstance ().getFreeSoundEffectChannel ().play (static_cast<cBuilding*> (aggressor)->uiData->Attack);
		}
	}

	
}

std::unique_ptr<cFx> cAttackJob::createMuzzleFx()
{
	//TODO: this shouldn't be in the attackjob class. But since
	//the attackjobs doesn't always have an instance of the unit,
	//it stays here for now

	cPosition offset(0, 0);
	switch (muzzleType)
	{
		case sUnitData::MUZZLE_TYPE_BIG:
			switch (fireDir)
			{
				case 0:
					offset.y () = -40;
					break;
				case 1:
					offset.x () = 32;
					offset.y () = -32;
					break;
				case 2:
					offset.x () = 40;
					break;
				case 3:
					offset.x () = 32;
					offset.y () = 32;
					break;
				case 4:
					offset.y () = 40;
					break;
				case 5:
					offset.x () = -32;
					offset.y () = 32;
					break;
				case 6:
					offset.x () = -40;
					break;
				case 7:
					offset.x () = -32;
					offset.y () = -32;
					break;
			}
			return std::make_unique<cFxMuzzleBig> (aggressorPosition * 64 + offset, fireDir);

		case sUnitData::MUZZLE_TYPE_SMALL:
			return std::make_unique<cFxMuzzleSmall> (aggressorPosition * 64, fireDir);

		case sUnitData::MUZZLE_TYPE_ROCKET:
		case sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER:
			return std::make_unique<cFxRocket> (aggressorPosition * 64 + cPosition (32, 32), targetPosition * 64 + cPosition (32, 32), fireDir, false);

		case sUnitData::MUZZLE_TYPE_MED:
		case sUnitData::MUZZLE_TYPE_MED_LONG:
			switch (fireDir)
			{
				case 0:
					offset.y () = -20;
					break;
				case 1:
					offset.x () = 12;
					offset.y () = -12;
					break;
				case 2:
					offset.x () = 20;
					break;
				case 3:
					offset.x () = 12;
					offset.y () = 12;
					break;
				case 4:
					offset.y () = 20;
					break;
				case 5:
					offset.x () = -12;
					offset.y () = 12;
					break;
				case 6:
					offset.x () = -20;
					break;
				case 7:
					offset.x () = -12;
					offset.y () = -12;
					break;
			}
			if (muzzleType == sUnitData::MUZZLE_TYPE_MED)
				return std::make_unique<cFxMuzzleMed> (aggressorPosition * 64 + offset, fireDir);
			else
				return std::make_unique<cFxMuzzleMedLong> (aggressorPosition * 64 + offset, fireDir);

		case sUnitData::MUZZLE_TYPE_TORPEDO:
			return std::make_unique<cFxRocket> (aggressorPosition * 64 + cPosition(32, 32), targetPosition * 64 + cPosition(32, 32), fireDir, true);
		case sUnitData::MUZZLE_TYPE_SNIPER:
			//TODO: sniper hat keine animation?!?
		default:
			return NULL;
	}
}

bool cAttackJob::impact()
{
	if (muzzleType == sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER)
		return impactCluster();
	else
		return impactSingle(targetPosition);

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
			unit->setIsBeeinAttacked (false);
	}
}

bool cAttackJob::impactCluster()
{
	const int clusterDamage = attackPoints;
	bool destroyed = false;
	std::vector<cUnit*> targets;

	//full damage
	destroyed = destroyed || impactSingle(targetPosition, &targets);

	// 3/4 damage
	attackPoints = (clusterDamage * 3) / 4;
	destroyed = destroyed || impactSingle (targetPosition + cPosition (-1, 0), &targets);
	destroyed = destroyed || impactSingle (targetPosition + cPosition (+1, 0), &targets);
	destroyed = destroyed || impactSingle (targetPosition + cPosition (0, -1), &targets);
	destroyed = destroyed || impactSingle (targetPosition + cPosition (0, +1), &targets);
	
	// 1/2 damage
	attackPoints = clusterDamage / 2;
	destroyed = destroyed || impactSingle (targetPosition + cPosition (+1, +1), &targets);
	destroyed = destroyed || impactSingle (targetPosition + cPosition (+1, -1), &targets);
	destroyed = destroyed || impactSingle (targetPosition + cPosition (-1, +1), &targets);
	destroyed = destroyed || impactSingle (targetPosition + cPosition (-1, -1), &targets);

	// 1/3 damage
	attackPoints = clusterDamage / 3;
	destroyed = destroyed || impactSingle (targetPosition + cPosition (-2, 0), &targets);
	destroyed = destroyed || impactSingle (targetPosition + cPosition (+2, 0), &targets);
	destroyed = destroyed || impactSingle (targetPosition + cPosition (0, -2), &targets);
	destroyed = destroyed || impactSingle (targetPosition + cPosition (0, +2), &targets);

	return destroyed;
}

bool cAttackJob::impactSingle (const cPosition& position, std::vector<cUnit*>* avoidTargets)
{
	//select target
	cPlayer* player = client ? client->getPlayerFromNumber(aggressorPlayerNr) : &server->getPlayerFromNumber(aggressorPlayerNr);
	cMap&    map    = client ? *client->getMap() : *server->Map;

	if (!map.isValidPosition(position))
		return false;
	
	cUnit* target = selectTarget(position, attackMode, map, player);

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

	cPosition offset(0, 0);
	if (target && target->isAVehicle())
	{
		offset = static_cast<cVehicle*>(target)->getMovementOffset ();
	}

	bool destroyed = false;
	std::string name;
	sID unitID;

	// if taget is a stealth unit, make it visible on all clients
	if (server && target && target->data.isStealthOn != TERRAIN_NONE)
	{
		for (const auto& player : server->playerList)
		{
			if (target->getOwner() == player.get()) continue;
			if (!player->canSeeAnyAreaUnder(*target)) continue;

			target->setDetectedByPlayer(*server, player.get());
		}
	}

	//make impact on target
	if (target)
	{
		target->data.setHitpoints(target->calcHealth(attackPoints));
		target->setHasBeenAttacked (true);
		target->setIsBeeinAttacked (false);

		name = target->getDisplayName();
		unitID = target->data.ID;

		if (target->data.getHitpoints() <= 0)
		{
			target->setIsBeeinAttacked (true);
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
	if (aggressor && aggressor->data.explodesOnContact && aggressorPosition == position)
	{
		if (client)
		{
			cBuilding& b = *static_cast<cBuilding*> (aggressor);

			cSoundDevice::getInstance ().getFreeSoundEffectChannel ().play (b.uiData->Attack);
			if (map.isWaterOrCoast(b.getPosition()))
			{
				client->addFx(std::make_unique<cFxExploWater>(b.getPosition() * 64 + cPosition(32, 32)));
			}
			else
			{
				client->addFx (std::make_unique<cFxExploSmall> (b.getPosition () * 64 + cPosition (32, 32)));
			}
			client->deleteUnit(aggressor);
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
		client->addFx(std::make_unique<cFxHit>(position * 64 + offset + cPosition(32, 32)), target != nullptr);
	}

	//make message
	if (client && target && target->getOwner()->getNr() == client->getActivePlayer().getNr())
	{
		std::string message;
		if (destroyed)
		{
			client->getActivePlayer ().addSavedReport (std::make_unique<cSavedReportDestroyed> (*target));
		}
		else
		{
			client->getActivePlayer ().addSavedReport (std::make_unique<cSavedReportAttacked> (*target));
		}
	}

	if (aggressor)
		aggressor->setAttacking(false);

	if (target)
		Log.write (std::string (server ? " Server: " : " Client: ") + "AttackJob Impact. Target: " + target->getDisplayName () + " (ID: " + iToStr (target->iID) + ") at (" + iToStr (targetPosition.x ()) + "," + iToStr (targetPosition.y ()) + "), Remaining HP: " + iToStr (target->data.getHitpoints ()), cLog::eLOG_TYPE_NET_DEBUG);
	else
		Log.write (std::string (server ? " Server: " : " Client: ") + " AttackJob Impact. Target: none (" + iToStr (targetPosition.x ()) + "," + iToStr (targetPosition.y ()) + ")", cLog::eLOG_TYPE_NET_DEBUG);


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
				Log.write(" Server: AttackJob destroyed unit " + unit->getDisplayName() + " (ID: " + iToStr(unit->iID) + ") at (" + iToStr(unit->getPosition().x()) + "," + iToStr(unit->getPosition().y()) + ")", cLog::eLOG_TYPE_NET_DEBUG);
				server->destroyUnit(*unit);
			}
		}
	}
}
