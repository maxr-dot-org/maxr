
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

//TODO: unit locking
//TODO: sync isAttacking and isAttacked flags in sendUnitData()
//TODO: expl mines
//TODO: cluster attack
//TODO: debug output
//TODO: uncover stealth units, when firing
//TODO: uncover stealth units, when hit
//TODO: sentry attacks
//TODO: text and voice messages
//TODO: load/save attackjobs
//TODO: resync attackjobs
//TODO: extend checksum

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
cAttackJob::cAttackJob(cServer* server_, cUnit* aggressor_, int targetX_, int targetY_) :
	aggressorID(aggressor_->iID),
	aggressor(aggressor_),
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

	fireDir(0)
{
	fireDir = calcFireDir();
	counter = calcTimeForRotation() + FIRE_DELAY;

	AutoPtr<cNetMessage> message(serialize());
	server->sendNetMessage(message);

	//TODO: lock unit
	//TODO: synchronize isAttacked and isAttacking flags by send unit data
}

cAttackJob::cAttackJob(cClient* client_, cNetMessage& message) :
	client(client_),
	server(NULL),
	aggressor(NULL)
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
				impact();
				state = S_FINISHED;
			}
			break;
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
	//TODO: cached aggressor pointer is probably not nessesary when getUnitFromID would have O(log(n))
	if (aggressor)
		return aggressor;

	if (server)
		aggressor = server->getUnitFromID(aggressorID);
	else
		aggressor = client->getUnitFromID(aggressorID);

	return aggressor;
}

void cAttackJob::fire()
{
	cUnit* aggressor = getAggressor();
	
	//update data
	if (aggressor)
	{
		//TODO: hier, oder gleich zu beinn des Jobs?
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
			client->addFx (muzzle);
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
			//TODO: play sound only for active clients
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
	//the attackjobs doesn't always has an instance of the unit,
	//it stays here for now

	//TODO: unit offsets berücksichtigen
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

void cAttackJob::impact()
{
	//select target
	cPlayer* player = client ? client->getPlayerFromNumber(aggressorPlayerNr) : server->getPlayerFromNumber(aggressorPlayerNr);
	cMap&    map    = client ? *client->getMap() : *server->Map;
	cUnit*   target = selectTarget(targetX, targetY, attackMode, map, player);

	int offX = 0, offY = 0;
	if (target->isAVehicle())
	{
		offX = static_cast<cVehicle*>(target)->OffX;
		offY = static_cast<cVehicle*>(target)->OffY;
	}

	bool destroyed = false;
	std::string name;
	sID unitID;

	//update target data
	if (target)
	{
		target->data.hitpointsCur = target->calcHealth(attackPoints);
		target->hasBeenAttacked = true;

		name = target->getDisplayName();
		unitID = target->data.ID;

		if (target->data.hitpointsCur <= 0)
		{
			destroyed = true;
			if (client)
			{
				if (target->isAVehicle())
					client->destroyUnit(*static_cast<cVehicle*>(target));
				else
					client->destroyUnit(*static_cast<cBuilding*>(target));

			}
			else
			{
				if (target->isAVehicle())
					server->destroyUnit(*static_cast<cVehicle*>(target));
				else
					server->destroyUnit(*static_cast<cBuilding*>(target));
			}
			target = NULL;
		}

	}

	
	if (!destroyed && client)
	{
		client->addFx(new cFxHit(targetX * 64 + offX + 32, targetY * 64 + offY + 32));
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


	if (client)
		client->getGameGUI().updateMouseCursor();

}
