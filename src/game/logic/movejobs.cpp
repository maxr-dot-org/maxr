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

#include "game/logic/movejobs.h"

#include "game/logic/attackjob.h"
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
#include "video.h"



static void setOffset (cVehicle* Vehicle, int nextDir, int offset)
{
	assert (0 <= nextDir && nextDir < 8);
	//                       N, NE, E, SE, S, SW,  W, NW
	const int offsetX[8] = { 0,  1, 1,  1, 0, -1, -1, -1};
	const int offsetY[8] = { -1, -1, 0,  1, 1,  1,  0, -1};

	auto newOffset = Vehicle->getMovementOffset();
	newOffset.x() += offsetX[nextDir] * offset;
	newOffset.y() += offsetY[nextDir] * offset;

	Vehicle->setMovementOffset (newOffset);
}

static int getDir (const cPosition& position, const cPosition& next)
{
	const cPosition diff = next - position;

	//                       N, NE, E, SE, S, SW,  W, NW
	const int offsetX[8] = { 0,  1, 1,  1, 0, -1, -1, -1};
	const int offsetY[8] = { -1, -1, 0,  1, 1,  1,  0, -1};

	for (int i = 0; i != 8; ++i)
	{
		if (diff.x() == offsetX[i] && diff.y() == offsetY[i]) return i;
	}
	assert (false);
	return -1;
}

cServerMoveJob::cServerMoveJob (cServer& server_, const cPosition& source_, const cPosition& destination_, cVehicle* vehicle) :
	server (&server_)
{
	Map = server->Map.get();
	this->Vehicle = vehicle;
	source = source_;
	destination = destination_;
	bPlane = (Vehicle->getStaticUnitData().factorAir > 0);
	bFinished = false;
	bEndForNow = false;
	iSavedSpeed = 0;
	Waypoints = nullptr;
	endAction = nullptr;

	// unset sentry status when moving vehicle
	if (Vehicle->isSentryActive())
	{
		Vehicle->getOwner()->deleteSentry (*Vehicle);
	}
	//sendUnitData (*server, *Vehicle);

	if (Vehicle->ServerMoveJob)
	{
		iSavedSpeed = Vehicle->ServerMoveJob->iSavedSpeed;
		Vehicle->ServerMoveJob->release();
		Vehicle->setMoving (false);
//		Vehicle->MoveJobActive = false;
		Vehicle->ServerMoveJob->Vehicle = nullptr;
	}
	Vehicle->ServerMoveJob = this;
}

cServerMoveJob::~cServerMoveJob()
{
	delete endAction;
}

void cServerMoveJob::stop()
{
	// an already started movement step will be finished

	// delete all waypoint of the movejob except the next one,
	// so the vehicle stops on the next field
	if (Waypoints && Waypoints->next && Waypoints->next->next)
	{
		sWaypointOld* wayPoint = Waypoints->next->next;
		Waypoints->next->next = nullptr;
		while (wayPoint)
		{
			sWaypointOld* nextWayPoint = wayPoint->next;
			delete wayPoint;
			wayPoint = nextWayPoint;
		}
	}

	// if the vehicle is not moving, it has to stop immediately
	if (!Vehicle->isUnitMoving())
	{
		release();
	}
}

void cServerMoveJob::resume()
{
	if (Vehicle && Vehicle->data.getSpeed() > 0 && !Vehicle->isUnitMoving())
	{
		// restart movejob
		calcNextDir();
		bEndForNow = false;
		source = Vehicle->getPosition();
		server->addActiveMoveJob (*this);
	}
}

void cServerMoveJob::addEndAction (int destID, eEndMoveActionType type)
{
	delete endAction;

	endAction = new cEndMoveAction (Vehicle, destID, type);
	sendEndMoveActionToClient (*server, *Vehicle, destID, type);

}

cServerMoveJob* cServerMoveJob::generateFromMessage (cServer& server, cNetMessage& message)
{

	int iVehicleID = message.popInt32();
	cVehicle* vehicle = server.getVehicleFromID (iVehicleID);
	if (vehicle == nullptr)
	{
		Log.write (" Server: Can't find vehicle with id " + iToStr (iVehicleID), cLog::eLOG_TYPE_NET_WARNING);
		return nullptr;
	}

	// TODO: is this check really needed?
	if (vehicle->isBeeingAttacked())
	{
		Log.write (" Server: cannot move a vehicle currently under attack", cLog::eLOG_TYPE_NET_DEBUG);
		return nullptr;
	}
	if (vehicle->isAttacking())
	{
		Log.write (" Server: cannot move a vehicle currently attacking", cLog::eLOG_TYPE_NET_DEBUG);
		return nullptr;
	}
	if (vehicle->isUnitBuildingABuilding() || (vehicle->BuildPath && vehicle->ServerMoveJob))
	{
		Log.write (" Server: cannot move a vehicle currently building", cLog::eLOG_TYPE_NET_DEBUG);
		return nullptr;
	}
	if (vehicle->isUnitClearing())
	{
		Log.write (" Server: cannot move a vehicle currently building", cLog::eLOG_TYPE_NET_DEBUG);
		return nullptr;
	}
	if (vehicle->isDisabled())
	{
		Log.write(" Server: cannot move a vehicle currently disabled", cLog::eLOG_TYPE_NET_DEBUG);
		return nullptr;
	}

	// reconstruct path
	sWaypointOld* path = nullptr;
	sWaypointOld* dest = nullptr;
	int iCount = 0;
	int iReceivedCount = message.popInt16();

	if (iReceivedCount == 0)
	{
		return nullptr;
	}

	while (iCount < iReceivedCount)
	{
		sWaypointOld* waypoint = new sWaypointOld;
		waypoint->position = message.popPosition();
		waypoint->Costs = message.popInt16();

		if (!dest) dest = waypoint;

		waypoint->next = path;
		path = waypoint;

		iCount++;
	}

	//is the vehicle position equal to the begin of the path?
	if (vehicle->getPosition() != path->position)
	{
		Log.write (" Server: Vehicle with id " + iToStr (iVehicleID) + " is at wrong position (" + iToStr (vehicle->getPosition().x()) + "x" + iToStr (vehicle->getPosition().y()) + ") for movejob from " +  iToStr (path->position.x()) + "x" + iToStr (path->position.y()) + " to " + iToStr (dest->position.x()) + "x" + iToStr (dest->position.y()), cLog::eLOG_TYPE_NET_WARNING);

		while (path)
		{
			sWaypointOld* waypoint = path;
			path = path->next;
			delete waypoint;
		}
		return nullptr;
	}

	//everything is ok. Construct the movejob
	Log.write (" Server: Received MoveJob: VehicleID: " + iToStr (vehicle->iID) + ", SrcX: " + iToStr (path->position.x()) + ", SrcY: " + iToStr (path->position.y()) + ", DestX: " + iToStr (dest->position.x()) + ", DestY: " + iToStr (dest->position.y()) + ", WaypointCount: " + iToStr (iReceivedCount), cLog::eLOG_TYPE_NET_DEBUG);
	cServerMoveJob* mjob = new cServerMoveJob (server, path->position, dest->position, vehicle);
	mjob->Waypoints = path;

	mjob->calcNextDir();

	return mjob;
}

bool cServerMoveJob::calcPath()
{
	if (source == destination) return false;

	cPathCalculator PathCalculator (*Vehicle, *Map, destination, false);
	Waypoints = nullptr; // PathCalculator.calcPath();
	if (Waypoints)
	{
		calcNextDir();
		return true;
	}
	return false;
}

void cServerMoveJob::release()
{
	bEndForNow = false;
	bFinished = true;
	Log.write (" Server: Released old movejob", cLog::eLOG_TYPE_NET_DEBUG);
	for (unsigned int i = 0; i < server->ActiveMJobs.size(); i++)
	{
		if (this == server->ActiveMJobs[i]) return;
	}
	server->addActiveMoveJob (*this);
	Log.write (" Server: Added released movejob to active ones", cLog::eLOG_TYPE_NET_DEBUG);
}

bool cServerMoveJob::checkMove()
{
	//bool bInSentryRange;
	if (!Vehicle || !Waypoints || !Waypoints->next)
	{
		bFinished = true;
		return false;
	}

	// not enough waypoints for this move?
	if (Vehicle->data.getSpeed() < Waypoints->next->Costs)
	{
		Log.write (" Server: Vehicle has not enough waypoints for the next move -> EndForNow: ID: " + iToStr (Vehicle->iID) + ", X: " + iToStr (Waypoints->next->position.x()) + ", Y: " + iToStr (Waypoints->next->position.y()), cLog::eLOG_TYPE_NET_DEBUG);
		iSavedSpeed += Vehicle->data.getSpeed();
		Vehicle->data.setSpeed (0);
		bEndForNow = true;
		return true;
	}

	//bInSentryRange = Vehicle->InSentryRange (*server);

	if (!Map->possiblePlace (*Vehicle, Waypoints->next->position))// && !bInSentryRange)
	{
		server->sideStepStealthUnit (Waypoints->next->position, *Vehicle);
	}

	//when the next field is still blocked, inform the client
	if (!Map->possiblePlace (*Vehicle, Waypoints->next->position))// || bInSentryRange)    //TODO: bInSentryRange?? Why?
	{
		Log.write (" Server: Next point is blocked: ID: " + iToStr (Vehicle->iID) + ", X: " + iToStr (Waypoints->next->position.x()) + ", Y: " + iToStr (Waypoints->next->position.y()), cLog::eLOG_TYPE_NET_DEBUG);
		// if the next point would be the last, finish the job here
		if (Waypoints->next->position == destination)
		{
			bFinished = true;
		}
		// else delete the movejob and inform the client that he has to find a new path
		else
		{
			sendNextMove (*server, *Vehicle, MJOB_BLOCKED);
		}
		return false;
	}

	// next step can be executed.
	// start the move and set the vehicle to the next field
	calcNextDir();
	//Vehicle->MoveJobActive = true;
	Vehicle->setMoving (true);

	Vehicle->data.setSpeed (Vehicle->data.getSpeed() + iSavedSpeed);
	iSavedSpeed = 0;
	Vehicle->DecSpeed (Waypoints->next->Costs);

	//reset detected flag, when a water stealth unit drives into the water
	if (Vehicle->getStaticUnitData().isStealthOn & TERRAIN_SEA && Vehicle->getStaticUnitData().factorGround)
	{
		bool wasOnLand = !Map->isWater (Waypoints->position);
		bool driveIntoWater = Map->isWater (Waypoints->next->position);

		if (wasOnLand && driveIntoWater)
		{
			while (Vehicle->detectedByPlayerList.size())
				Vehicle->resetDetectedByPlayer (*server, Vehicle->detectedByPlayerList[0]);
		}
	}
	// resetDetected for players, that can't _detect_ the unit anymore and if the unit was not in the detected area of that player in this turn, too
	Vehicle->tryResetOfDetectionStateAfterMove (*server);

	// send move command to all players who can see the unit
	sendNextMove (*server, *Vehicle, MJOB_OK);

	Map->moveVehicle (*Vehicle, Waypoints->next->position);
	Vehicle->getOwner()->doScan();
	Vehicle->setMovementOffset (cPosition (0, 0));
	setOffset (Vehicle, iNextDir, -64);

	return true;
}

void cServerMoveJob::moveVehicle()
{
	int iSpeed;
	if (!Vehicle)
		return;
	if (Vehicle->uiData->animationMovement)
		iSpeed = MOVE_SPEED / 2;
	else if (!(Vehicle->getStaticUnitData().factorAir > 0) && !(Vehicle->getStaticUnitData().factorSea > 0 && Vehicle->getStaticUnitData().factorGround == 0))
	{
		iSpeed = MOVE_SPEED;
		cBuilding* building = Map->getField (Waypoints->next->position).getBaseBuilding();
		if (building && building->getStaticUnitData().modifiesSpeed)
			iSpeed = (int)(iSpeed / building->getStaticUnitData().modifiesSpeed);
	}
	else if (Vehicle->getStaticUnitData().factorAir > 0)
		iSpeed = MOVE_SPEED * 2;
	else
		iSpeed = MOVE_SPEED;

	setOffset (Vehicle, iNextDir, iSpeed);

	// check whether the point has been reached:
	if (abs (Vehicle->getMovementOffset().x()) < iSpeed && abs (Vehicle->getMovementOffset().y()) < iSpeed)
		doEndMoveVehicle();
}

void cServerMoveJob::doEndMoveVehicle()
{
	Log.write (" Server: Vehicle reached the next field: ID: " + iToStr (Vehicle->iID) + ", X: " + iToStr (Waypoints->next->position.x()) + ", Y: " + iToStr (Waypoints->next->position.y()), cLog::eLOG_TYPE_NET_DEBUG);

	sWaypointOld* Waypoint;
	Waypoint = Waypoints->next;
	delete Waypoints;
	Waypoints = Waypoint;

	Vehicle->setMovementOffset (cPosition (0, 0));

	if (Waypoints->next == nullptr)
	{
		bFinished = true;
	}

	// check for results of the move

	// make mines explode if necessary
	cBuilding* mine = Map->getField (Vehicle->getPosition()).getMine();
	if (Vehicle->getStaticUnitData().factorAir == 0 && mine && mine->getOwner() != Vehicle->getOwner())
	{
		server->addAttackJob (mine, Vehicle->getPosition());
		bEndForNow = true;
	}

	// search for resources if necessary
	if (Vehicle->getStaticUnitData().canSurvey)
	{
		sendVehicleResources (*server, *Vehicle);
		Vehicle->doSurvey ();
	}

	//handle detection
	Vehicle->makeDetection (*server);

	// let other units fire on this one
	Vehicle->InSentryRange (*server);

	// lay/clear mines if necessary
	if (Vehicle->getStaticUnitData().canPlaceMines)
	{
		bool bResult = false;
		if (Vehicle->isUnitLayingMines()) bResult = Vehicle->layMine (*server);
		else if (Vehicle->isUnitClearingMines()) bResult = Vehicle->clearMine (*server);
		if (bResult)
		{
			// send new unit values
			//sendUnitData (*server, *Vehicle);
		}
	}

	Vehicle->setMoving (false);
	calcNextDir();

	if (Vehicle->canLand (*server->Map))
	{
		Vehicle->setFlightHeight (0);
	}
	else
	{
		Vehicle->setFlightHeight (64);
	}
}

void cServerMoveJob::calcNextDir()
{
	if (!Waypoints || !Waypoints->next) return;
	iNextDir = getDir (Waypoints->position, Waypoints->next->position);
}

cEndMoveAction::cEndMoveAction (cVehicle* vehicle, int destID, eEndMoveActionType type)
{
	destID_ = destID;
	type_ = type;
	vehicle_ = vehicle;
}

void cEndMoveAction::execute (cServer& server)
{
	switch (type_)
	{
		case EMAT_LOAD: executeLoadAction (server); break;
		case EMAT_GET_IN: executeGetInAction (server); break;
		case EMAT_ATTACK: executeAttackAction (server); break;
	}
}

void cEndMoveAction::executeLoadAction (cServer& server)
{
	cVehicle* destVehicle = server.getVehicleFromID (destID_);
	if (!destVehicle) return;

	if (vehicle_->canLoad (destVehicle) == false) return;

	vehicle_->storeVehicle (*destVehicle, *server.Map);
	if (destVehicle->ServerMoveJob) destVehicle->ServerMoveJob->release();

	// vehicle is removed from enemy clients by cServer::checkPlayerUnits()
	sendStoreVehicle (server, vehicle_->iID, true, destVehicle->iID, *vehicle_->getOwner());
}

void cEndMoveAction::executeGetInAction (cServer& server)
{
	cVehicle* destVehicle = server.getVehicleFromID (destID_);
	cBuilding* destBuilding = server.getBuildingFromID (destID_);

	// execute the loading if possible
	if (destVehicle && destVehicle->canLoad (vehicle_))
	{
		destVehicle->storeVehicle (*vehicle_, *server.Map);
		if (vehicle_->ServerMoveJob) vehicle_->ServerMoveJob->release();
		//vehicle is removed from enemy clients by cServer::checkPlayerUnits()
		sendStoreVehicle (server, destVehicle->iID, true, vehicle_->iID, *destVehicle->getOwner());
	}
	else if (destBuilding && destBuilding->canLoad (vehicle_))
	{
		destBuilding->storeVehicle (*vehicle_, *server.Map);
		if (vehicle_->ServerMoveJob) vehicle_->ServerMoveJob->release();
		// vehicle is removed from enemy clients by cServer::checkPlayerUnits()
		sendStoreVehicle (server, destBuilding->iID, false, vehicle_->iID, *destBuilding->getOwner());
	}
}

void cEndMoveAction::executeAttackAction (cServer& server)
{
	// get the target unit
	const cUnit* destUnit = server.getUnitFromID (destID_);
	if (destUnit == nullptr) return;

	const auto& position = destUnit->getPosition();
	cMap& map = *server.Map;

	// check, whether the attack is now possible
	if (!vehicle_->canAttackObjectAt (position, map, true, true)) return;

	// is the target in sight?
	if (!vehicle_->getOwner()->canSeeAnyAreaUnder (*destUnit)) return;

	server.addAttackJob (vehicle_, position);
}

cClientMoveJob::cClientMoveJob (cClient& client_, const cPosition& source_, const cPosition& destination_, cVehicle* Vehicle) :
	client (&client_),
	endMoveAction (nullptr),
	destination (destination_),
	Waypoints (nullptr)
{
	init (source_, Vehicle);
}

void cClientMoveJob::init (const cPosition& source_, cVehicle* Vehicle)
{
	//Map = client->getMap().get();
	this->Vehicle = Vehicle;
	source = source_;
	this->bPlane = (Vehicle->getStaticUnitData().factorAir > 0);
	bFinished = false;
	bEndForNow = false;
	iSavedSpeed = 0;
	bSuspended = false;

/*	if (Vehicle->getClientMoveJob())
	{
		Vehicle->getClientMoveJob()->release();
		Vehicle->setMoving (false);
		Vehicle->MoveJobActive = false;
	}
	Vehicle->setClientMoveJob (this);*/
	endMoveAction = nullptr;
}

cClientMoveJob::~cClientMoveJob()
{
	sWaypointOld* NextWaypoint;
	while (Waypoints)
	{
		NextWaypoint = Waypoints->next;
		delete Waypoints;
		Waypoints = NextWaypoint;
	}
	Remove (client->ActiveMJobs, this);
	delete endMoveAction;
}

bool cClientMoveJob::generateFromMessage (cNetMessage& message)
{
//	if (message.iType != GAME_EV_MOVE_JOB_SERVER) return false;
	int iCount = 0;
	int iReceivedCount = message.popInt16();

	Log.write (" Client: Received MoveJob: VehicleID: " + iToStr (Vehicle->iID) + ", SrcX: " + iToStr (source.x()) + ", SrcY: " + iToStr (source.y()) + ", DestX: " + iToStr (destination.x()) + ", DestY: " + iToStr (destination.y()) + ", WaypointCount: " + iToStr (iReceivedCount), cLog::eLOG_TYPE_NET_DEBUG);

	// Add the waypoints
	while (iCount < iReceivedCount)
	{
		sWaypointOld* waypoint = new sWaypointOld;
		waypoint->position = message.popPosition();
		waypoint->Costs = message.popInt16();

		waypoint->next = Waypoints;
		Waypoints = waypoint;

		iCount++;
	}

	calcNextDir();
	return true;
}

std::forward_list<cPosition> cClientMoveJob::calcPath (const cMap& map, const cPosition& source, const cPosition& destination, const cVehicle& vehicle, const std::vector<cVehicle*>* group)
{
	//if (source == destination) return 0;

	cPathCalculator PathCalculator (vehicle, map, destination, group);
	const auto waypoints = PathCalculator.calcPath();

	return waypoints;
}

void cClientMoveJob::release()
{
	bEndForNow = false;
	bFinished = true;
	Log.write (" Client: Released old movejob", cLog::eLOG_TYPE_NET_DEBUG);
	client->addActiveMoveJob (*this);
	Log.write (" Client: Added released movejob to active ones", cLog::eLOG_TYPE_NET_DEBUG);
}

void cClientMoveJob::handleNextMove (int iType, int iSavedSpeed)
{
	// the client is faster than the server and has already
	// reached the last field or the next will be the last,
	// then stop the vehicle
	if (Waypoints == nullptr || Waypoints->next == nullptr)
	{
		Log.write (" Client: Client has already reached the last field", cLog::eLOG_TYPE_NET_DEBUG);
		bEndForNow = true;
		Vehicle->setMovementOffset (cPosition (0, 0));
		return;
	}

	switch (iType)
	{
		case MJOB_OK:
		{
/*			if (!Vehicle->MoveJobActive)
			{
				client->addActiveMoveJob (*this);
				activated (*Vehicle);
			}
			if (bEndForNow)
			{
				bEndForNow = false;
				//client->addActiveMoveJob (*Vehicle->getClientMoveJob());
				Log.write (" Client: reactivated movejob; Vehicle-ID: " + iToStr (Vehicle->iID), cLog::eLOG_TYPE_NET_DEBUG);
			}
			Vehicle->MoveJobActive = true;
			if (Vehicle->isUnitMoving()) doEndMoveVehicle();
*/
			Vehicle->setMoving (true);
			Map->moveVehicle (*Vehicle, Waypoints->next->position);

			Vehicle->data.setSpeed (Vehicle->data.getSpeed() + this->iSavedSpeed);
			this->iSavedSpeed = 0;
			Vehicle->DecSpeed (Waypoints->next->Costs);

			//Vehicle->owner->doScan();
			Vehicle->setMovementOffset (cPosition (0, 0));
			setOffset (Vehicle, iNextDir, -64);

			if (Vehicle->getStaticUnitData().factorAir > 0 && Vehicle->getFlightHeight() < 64)
			{
				//client->addJob (new cPlaneTakeoffJob (*Vehicle, true));
			}

			moved (*Vehicle);
		}
		break;
		case MJOB_STOP:
		{
			Log.write (" Client: The movejob will end for now", cLog::eLOG_TYPE_NET_DEBUG);
			if (Vehicle->isUnitMoving()) doEndMoveVehicle();
			if (bEndForNow) client->addActiveMoveJob (*this);
			this->iSavedSpeed = iSavedSpeed;
			Vehicle->data.setSpeed (0);
			bSuspended = true;
			bEndForNow = true;
		}
		break;
		case MJOB_FINISHED:
		{
			Log.write (" Client: The movejob is finished", cLog::eLOG_TYPE_NET_DEBUG);
			if (Vehicle->isUnitMoving()) doEndMoveVehicle();
			release();
		}
		break;
		case MJOB_BLOCKED:
		{
			if (Vehicle->isUnitMoving()) doEndMoveVehicle();
			Log.write (" Client: next field is blocked: DestX: " + iToStr (Waypoints->next->position.x()) + ", DestY: " + iToStr (Waypoints->next->position.y()), cLog::eLOG_TYPE_NET_DEBUG);

			if (Vehicle->getOwner() != &client->getActivePlayer())
			{
				bFinished = true;
				break;
			}

			bEndForNow = true;
			cPosition* path;// = calcPath(*client->getMap(), Vehicle->getPosition(), destination, *Vehicle);
			if (path)
			{
				//sendMoveJob (*client, path, Vehicle->iID);
				if (endMoveAction) sendEndMoveAction (*client, Vehicle->iID, endMoveAction->destID_, endMoveAction->type_);
			}
			else
			{
				bFinished = true;

				blocked (*Vehicle);
			}
		}
		break;
	}
}

void cClientMoveJob::moveVehicle()
{
	//if (Vehicle == nullptr || Vehicle->getClientMoveJob() != this) return;

	// do not move the vehicle, if the movejob hasn't got any more waypoints
	if (Waypoints == nullptr || Waypoints->next == nullptr)
	{
		stopped (*Vehicle);
		return;
	}

	if (!Vehicle->isUnitMoving())
	{
		//check remaining speed
		if (Vehicle->data.getSpeed() < Waypoints->next->Costs)
		{
			bSuspended = true;
			bEndForNow = true;
			stopped (*Vehicle);
			return;
		}

		Vehicle->data.setSpeed (Vehicle->data.getSpeed() + iSavedSpeed);
		iSavedSpeed = 0;
		Vehicle->DecSpeed (Waypoints->next->Costs);

		Map->moveVehicle (*Vehicle, Waypoints->next->position);
		Vehicle->getOwner()->doScan();
		Vehicle->setMovementOffset (cPosition (0, 0));
		setOffset (Vehicle, iNextDir, -64);
		Vehicle->setMoving (true);

		if (Vehicle->getStaticUnitData().factorAir > 0 && Vehicle->getFlightHeight() < 64)
		{
			//client->addJob (new cPlaneTakeoffJob (*Vehicle, true));
		}

		moved (*Vehicle);
	}

	int iSpeed;
	if (Vehicle->uiData->animationMovement)
	{
		//if (client->getGameTimer()->timer50ms)
			Vehicle->WalkFrame++;
		if (Vehicle->WalkFrame >= 13) Vehicle->WalkFrame = 1;
		iSpeed = MOVE_SPEED / 2;
	}
	else if (!(Vehicle->getStaticUnitData().factorAir > 0) && !(Vehicle->getStaticUnitData().factorSea > 0 && Vehicle->getStaticUnitData().factorGround == 0))
	{
		iSpeed = MOVE_SPEED;
		cBuilding* building = Map->getField (Waypoints->next->position).getBaseBuilding();
		if (Waypoints && Waypoints->next && building && building->getStaticUnitData().modifiesSpeed) iSpeed = (int)(iSpeed / building->getStaticUnitData().modifiesSpeed);
	}
	else if (Vehicle->getStaticUnitData().factorAir > 0) iSpeed = MOVE_SPEED * 2;
	else iSpeed = MOVE_SPEED;

	// Ggf Tracks malen:
	if (cSettings::getInstance().isMakeTracks() && Vehicle->uiData->makeTracks && !Map->isWaterOrCoast(Vehicle->getPosition()) && !
		(Waypoints && Waypoints->next && Map->isWater (Waypoints->next->position)) &&
		(Vehicle->getOwner() == &client->getActivePlayer() || client->getActivePlayer().canSeeAnyAreaUnder (*Vehicle)))
	{
		if (abs (Vehicle->getMovementOffset().x()) == 64 || abs (Vehicle->getMovementOffset().y()) == 64)
		{
			switch (Vehicle->dir)
			{
				case 0:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() - cPosition (0, 10), 0));
					break;
				case 4:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() + cPosition (0, 10), 0));
					break;
				case 2:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() + cPosition (10, 0), 2));
					break;
				case 6:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() - cPosition (10, 0), 2));
					break;
				case 1:
				case 5:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset(), 1));
					break;
				case 3:
				case 7:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset(), 3));
					break;
			}
		}
		else if (abs (Vehicle->getMovementOffset().x()) == 64 - (iSpeed * 2) || abs (Vehicle->getMovementOffset().y()) == 64 - (iSpeed * 2))
		{
			switch (Vehicle->dir)
			{
				case 1:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() + cPosition (26, -26), 1));
					break;
				case 5:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() + cPosition (-26, 26), 1));
					break;
				case 3:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() + cPosition (26, 26), 3));
					break;
				case 7:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() + cPosition (-26, -26), 3));
					break;
			}
		}
	}

	setOffset (Vehicle, iNextDir, iSpeed);

	if (abs (Vehicle->getMovementOffset().x()) > 70 || abs (Vehicle->getMovementOffset().y()) > 70)
	{
		Log.write (" Client: Flying dutchmen detected! Unit ID: " + iToStr (Vehicle->iID) + " at position (" + iToStr (Vehicle->getPosition().x()) + ":" + iToStr (Vehicle->getPosition().y()) + ")", cLog::eLOG_TYPE_NET_DEBUG);
	}
}

void cClientMoveJob::doEndMoveVehicle()
{
	//if (Vehicle == nullptr || Vehicle->getClientMoveJob() != this) return;

	if (Waypoints->next == nullptr)
	{
		// this is just to avoid errors, this should normaly never happen.
		bFinished = true;
		return;
	}

	Vehicle->WalkFrame = 0;

	sWaypointOld* Waypoint = Waypoints;
	Waypoints = Waypoints->next;
	delete Waypoint;

	Vehicle->setMoving (false);

	Vehicle->setMovementOffset (cPosition (0, 0));

	Vehicle->getOwner()->doScan();

	calcNextDir();

/*	if (Vehicle->canLand (*client->getMap()) && Vehicle->getFlightHeight() > 0)
	{
		client->addJob (new cPlaneTakeoffJob (*Vehicle, false));
	}*/
}

void cClientMoveJob::calcNextDir()
{
	if (!Waypoints || !Waypoints->next) return;
	iNextDir = getDir (Waypoints->position, Waypoints->next->position);
}

void cClientMoveJob::drawArrow (SDL_Rect Dest, SDL_Rect* LastDest, bool bSpezial) const
{
	int iIndex = -1;
#define TESTXY_DP(a, b) if (Dest.x a LastDest->x && Dest.y b LastDest->y)
	TESTXY_DP ( > , <) iIndex = 0;
	else TESTXY_DP ( == , <) iIndex = 1;
	else TESTXY_DP ( < , <) iIndex = 2;
	else TESTXY_DP ( > , ==) iIndex = 3;
	else TESTXY_DP ( < , ==) iIndex = 4;
	else TESTXY_DP ( > , >) iIndex = 5;
	else TESTXY_DP ( == , >) iIndex = 6;
	else TESTXY_DP (<,>) iIndex = 7;

	if (iIndex == -1) return;

	if (bSpezial)
	{
		SDL_BlitSurface (OtherData.WayPointPfeileSpecial[iIndex][64 - Dest.w].get(), nullptr, cVideo::buffer, &Dest);
	}
	else
	{
		SDL_BlitSurface (OtherData.WayPointPfeile[iIndex][64 - Dest.w].get(), nullptr, cVideo::buffer, &Dest);
	}
}
