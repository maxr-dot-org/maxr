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

cPathDestHandler::cPathDestHandler(ePathDestinationTypes type_, const cPosition& destination_, const cVehicle* srcVehicle_, const cUnit* destUnit_) :
	type (type_),
	srcVehicle (srcVehicle_),
	destUnit (destUnit_),
	destination (destination_)
{}

bool cPathDestHandler::hasReachedDestination(const cPosition& position) const
{
	switch (type)
	{
		case PATH_DEST_TYPE_POS:
			return (destination == position);
		case PATH_DEST_TYPE_LOAD:
			return (destUnit && destUnit->isNextTo (position));
		case PATH_DEST_TYPE_ATTACK:
			return (position - destination).l2NormSquared () <= Square (srcVehicle->data.getRange ());
		default:
			return true;
	}
	return false;
}

int cPathDestHandler::heuristicCost(const cPosition& source) const
{
	switch (type)
	{
		case PATH_DEST_TYPE_POS:
		case PATH_DEST_TYPE_LOAD:
			return 0;
		case PATH_DEST_TYPE_ATTACK:
		default:
		{
			return Round ((destination - source).l2Norm());
		}
	}
}

cPathCalculator::cPathCalculator(const cPosition& source, const cPosition& destination, const cMap& Map, const cVehicle& Vehicle, const std::vector<cVehicle*>* group)
{
	destHandler = new cPathDestHandler (PATH_DEST_TYPE_POS, destination, NULL, NULL);
	init (source, Map, Vehicle, group);
}


cPathCalculator::cPathCalculator(const cPosition& source, const cUnit& destUnit, const cMap& Map, const cVehicle& Vehicle, bool load)
{
	destHandler = new cPathDestHandler (load ? PATH_DEST_TYPE_LOAD : PATH_DEST_TYPE_ATTACK, cPosition(0, 0), &Vehicle, &destUnit);
	init (source, Map, Vehicle, NULL);
}

cPathCalculator::cPathCalculator(const cPosition& source, const cMap& Map, const cVehicle& Vehicle, const cPosition& attack)
{
	destHandler = new cPathDestHandler (PATH_DEST_TYPE_ATTACK, attack, &Vehicle, NULL);
	init (source, Map, Vehicle, NULL);
}

void cPathCalculator::init(const cPosition& source, const cMap& Map, const cVehicle& Vehicle, const std::vector<cVehicle*>* group)
{
	this->source = source;
	this->Map = &Map;
	this->Vehicle = &Vehicle;
	this->group = group;
	bPlane = Vehicle.data.factorAir > 0;
	bShip = Vehicle.data.factorSea > 0 && Vehicle.data.factorGround == 0;

	Waypoints = NULL;
	MemBlocks = NULL;

	blocknum = 0;
	blocksize = 0;
	heapCount = 0;
}

cPathCalculator::~cPathCalculator()
{
	delete destHandler;
	if (MemBlocks != NULL)
	{
		for (int i = 0; i < blocknum; i++)
		{
			delete[] MemBlocks[i];
		}
		free (MemBlocks);
	}
}

sWaypoint* cPathCalculator::calcPath()
{
	// generate open and closed list
	nodesHeap.resize (Map->getSize ().x() * Map->getSize ().y() + 1, NULL);
	openList.resize (Map->getSize ().x() * Map->getSize ().y() + 1, NULL);
	closedList.resize (Map->getSize ().x() * Map->getSize ().y() + 1, NULL);

	// generate startnode
	sPathNode* StartNode = allocNode();
	StartNode->position = source;
	StartNode->costG = 0;
	StartNode->costH = destHandler->heuristicCost (source);
	StartNode->costF = StartNode->costG + StartNode->costH;

	StartNode->prev = NULL;
	openList[Map->getOffset (source)] = StartNode;
	insertToHeap (StartNode, false);

	while (heapCount > 0)
	{
		// get the node with the lowest F value
		sPathNode* CurrentNode = nodesHeap[1];

		// move it from the open to the closed list
		openList[Map->getOffset (CurrentNode->position)] = NULL;
		closedList[Map->getOffset(CurrentNode->position)] = CurrentNode;
		deleteFirstFromHeap();

		// generate waypoints when destination has been reached
		if(destHandler->hasReachedDestination(CurrentNode->position))
		{
			sWaypoint* NextWaypoint;
			Waypoints = new sWaypoint;

			sPathNode* NextNode = CurrentNode;
			NextNode->next = NULL;
			do
			{
				NextNode->prev->next = NextNode;
				NextNode = NextNode->prev;
			}
			while (NextNode->prev != NULL);


			NextWaypoint = Waypoints;
			NextWaypoint->position = NextNode->position;
			NextWaypoint->Costs = 0;
			do
			{
				NextNode = NextNode->next;

				NextWaypoint->next = new sWaypoint;
				NextWaypoint->next->position = NextNode->position;
				NextWaypoint->next->Costs = calcNextCost (NextNode->prev->position, NextWaypoint->next->position);
				NextWaypoint = NextWaypoint->next;
			}
			while (NextNode->next != NULL);

			NextWaypoint->next = NULL;

			return Waypoints;
		}

		// expand node
		expandNodes (CurrentNode);
	}

	// there is no path to the destination field
	Waypoints = NULL;
	return Waypoints;
}

void cPathCalculator::expandNodes (sPathNode* ParentNode)
{
	// add all nearby nodes
	const int minx = std::max(ParentNode->position.x() - 1, 0);
	const int maxx = std::min(ParentNode->position.x() + 1, Map->getSize().y() - 1);
	const int miny = std::max(ParentNode->position.y() - 1, 0);
	const int maxy = std::min(ParentNode->position.y() + 1, Map->getSize().y() - 1);

	for (int y = miny; y <= maxy; ++y)
	{
		for (int x = minx; x <= maxx; ++x)
		{
			const cPosition currentPosition(x, y);
			if (currentPosition == ParentNode->position) continue;

			if (!Map->possiblePlace (*Vehicle, currentPosition, true))
			{
				// when we have a group of units, the units will not block each other
				if (group)
				{
					// get the blocking unit
					cVehicle* blockingUnit;
					if(Vehicle->data.factorAir > 0) blockingUnit = Map->getField(currentPosition).getPlane();
					else blockingUnit = Map->getField(currentPosition).getVehicle();
					// check whether the blocking unit is the group
					bool isInGroup = Contains (*group, blockingUnit);
					if (!isInGroup) continue;
				}
				else continue;
			}
			if(closedList[Map->getOffset(currentPosition)] != NULL) continue;

			if(openList[Map->getOffset(currentPosition)] == NULL)
			{
				// generate new node
				sPathNode* NewNode = allocNode();
				NewNode->position = currentPosition;
				NewNode->costG = calcNextCost (ParentNode->position, currentPosition) + ParentNode->costG;
				NewNode->costH = destHandler->heuristicCost(currentPosition);
				NewNode->costF = NewNode->costG + NewNode->costH;
				NewNode->prev = ParentNode;
				openList[Map->getOffset(currentPosition)] = NewNode;
				insertToHeap (NewNode, false);
			}
			else
			{
				// modify existing node
				int costG, costH, costF;
				costG = calcNextCost(ParentNode->position, currentPosition) + ParentNode->costG;
				costH = destHandler->heuristicCost(currentPosition);
				costF = costG + costH;
				if(costF < openList[Map->getOffset(currentPosition)]->costF)
				{
					openList[Map->getOffset(currentPosition)]->costG = costG;
					openList[Map->getOffset(currentPosition)]->costH = costH;
					openList[Map->getOffset(currentPosition)]->costF = costF;
					openList[Map->getOffset(currentPosition)]->prev = ParentNode;
					insertToHeap(openList[Map->getOffset(currentPosition)], true);
				}
			}
		}
	}
}

sPathNode* cPathCalculator::allocNode()
{
	// alloced new memory block if necessary
	if (blocksize <= 0)
	{
		MemBlocks = static_cast<sPathNode**> (realloc (MemBlocks, (blocknum + 1) * sizeof (sPathNode*)));
		MemBlocks[blocknum] = new sPathNode[10];
		blocksize = MEM_BLOCK_SIZE;
		blocknum++;
	}
	blocksize--;
	return &MemBlocks[blocknum - 1][blocksize];
}

void cPathCalculator::insertToHeap (sPathNode* Node, bool exists)
{
	int i = 0;
	if (exists)
	{
		// get the number of the existing node
		for (int j = 1; j <= heapCount; j++)
		{
			if (nodesHeap[j] == Node)
			{
				i = j;
				break;
			}
		}
	}
	else
	{
		// add the new node in the end
		heapCount++;
		nodesHeap[heapCount] = Node;
		i = heapCount;
	}
	// resort the nodes
	while (i > 1)
	{
		assert (nodesHeap[i] == Node);
		if (Node->costF < nodesHeap[i / 2]->costF)
		{
			std::swap (nodesHeap[i / 2], nodesHeap[i]);
			i = i / 2;
		}
		else break;
	}
}

void cPathCalculator::deleteFirstFromHeap()
{
	// overwrite the first node by the last one
	nodesHeap[1] = nodesHeap[heapCount];
	nodesHeap[heapCount] = NULL;
	heapCount--;
	int v = 1;
	while (true)
	{
		int u = v;
		if (2 * u + 1 <= heapCount) // both children in the heap exists
		{
			if (nodesHeap[u]->costF >= nodesHeap[u * 2]->costF) v = 2 * u;
			if (nodesHeap[v]->costF >= nodesHeap[u * 2 + 1]->costF) v = 2 * u + 1;
		}
		else if (2 * u <= heapCount) // only one children exists
		{
			if (nodesHeap[u]->costF >= nodesHeap[u * 2]->costF) v = 2 * u;
		}
		// do the resort
		if (u != v)
		{
			std::swap (nodesHeap[u], nodesHeap[v]);
		}
		else break;
	}
}

int cPathCalculator::calcNextCost (const cPosition& source, const cPosition& destination) const
{
	int costs;
	// first we check whether the unit can fly
	if (Vehicle->data.factorAir > 0)
	{
		if (source.x() != destination.x() && source.y() != destination.y()) return (int) (4 * 1.5f * Vehicle->data.factorAir);
		else return (int) (4 * Vehicle->data.factorAir);
	}
	const cBuilding* building = Map->getField(destination).getBaseBuilding();
	// moving on water will cost more
	if (Map->isWater (destination) && (!building || (building->data.surfacePosition == sUnitData::SURFACE_POS_BENEATH_SEA || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE)) && Vehicle->data.factorSea > 0) costs = (int) (4 * Vehicle->data.factorSea);
	else if (Map->isCoast (destination) && !building && Vehicle->data.factorCoast > 0) costs = (int) (4 * Vehicle->data.factorCoast);
	else if (Vehicle->data.factorGround > 0) costs = (int) (4 * Vehicle->data.factorGround);
	else
	{
		Log.write ("Where can this unit move? " + iToStr (Vehicle->iID), cLog::eLOG_TYPE_NET_WARNING);
		costs = 4;
	}
	// moving on a road is cheaper
	if (building && building->data.modifiesSpeed != 0) costs = (int) (costs * building->data.modifiesSpeed);

	// multiplicate with the factor 1.5 for diagonal movements
	if (source.x() != destination.x() && source.y() != destination.y()) costs = (int)(costs * 1.5f);
	return costs;
}

static void setOffset (cVehicle* Vehicle, int nextDir, int offset)
{
	assert (0 <= nextDir && nextDir < 8);
	//                       N, NE, E, SE, S, SW,  W, NW
	const int offsetX[8] = { 0,  1, 1,  1, 0, -1, -1, -1};
	const int offsetY[8] = { -1, -1, 0,  1, 1,  1,  0, -1};

	auto newOffset = Vehicle->getMovementOffset();
	newOffset.x() += offsetX[nextDir] * offset;
	newOffset.y() += offsetY[nextDir] * offset;

	Vehicle->setMovementOffset(newOffset);
}

static int getDir(const cPosition& position, const cPosition& next)
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
	bPlane = (Vehicle->data.factorAir > 0);
	bFinished = false;
	bEndForNow = false;
	iSavedSpeed = 0;
	Waypoints = NULL;
	endAction = NULL;

	// unset sentry status when moving vehicle
	if (Vehicle->isSentryActive())
	{
		Vehicle->getOwner ()->deleteSentry (*Vehicle);
	}
	sendUnitData (*server, *Vehicle);

	if (Vehicle->ServerMoveJob)
	{
		iSavedSpeed = Vehicle->ServerMoveJob->iSavedSpeed;
		Vehicle->ServerMoveJob->release();
		Vehicle->setMoving (false);
		Vehicle->MoveJobActive = false;
		Vehicle->ServerMoveJob->Vehicle = NULL;
	}
	Vehicle->ServerMoveJob = this;
}

cServerMoveJob::~cServerMoveJob()
{
	sWaypoint* NextWaypoint;
	while (Waypoints)
	{
		NextWaypoint = Waypoints->next;
		delete Waypoints;
		Waypoints = NextWaypoint;
	}
	Waypoints = NULL;

	delete endAction;
}

void cServerMoveJob::stop()
{
	// an already started movement step will be finished

	// delete all waypoint of the movejob except the next one,
	// so the vehicle stops on the next field
	if (Waypoints && Waypoints->next && Waypoints->next->next)
	{
		sWaypoint* wayPoint = Waypoints->next->next;
		Waypoints->next->next = NULL;
		while (wayPoint)
		{
			sWaypoint* nextWayPoint = wayPoint->next;
			delete wayPoint;
			wayPoint = nextWayPoint;
		}
	}

	// if the vehicle is not moving, it has to stop immediately
	if (!Vehicle->isUnitMoving ())
	{
		release();
	}
}

void cServerMoveJob::resume()
{
	if (Vehicle && Vehicle->data.getSpeed() > 0 && !Vehicle->isUnitMoving ())
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
	if (message.iType != GAME_EV_MOVE_JOB_CLIENT) return NULL;

	int iVehicleID = message.popInt32();
	cVehicle* vehicle = server.getVehicleFromID (iVehicleID);
	if (vehicle == NULL)
	{
		Log.write (" Server: Can't find vehicle with id " + iToStr (iVehicleID), cLog::eLOG_TYPE_NET_WARNING);
		return NULL;
	}

	// TODO: is this check really needed?
	if (vehicle->isBeeingAttacked ())
	{
		Log.write (" Server: cannot move a vehicle currently under attack", cLog::eLOG_TYPE_NET_DEBUG);
		return NULL;
	}
	if (vehicle->isAttacking())
	{
		Log.write (" Server: cannot move a vehicle currently attacking", cLog::eLOG_TYPE_NET_DEBUG);
		return NULL;
	}
	if (vehicle->isUnitBuildingABuilding () || (vehicle->BuildPath && vehicle->ServerMoveJob))
	{
		Log.write (" Server: cannot move a vehicle currently building", cLog::eLOG_TYPE_NET_DEBUG);
		return NULL;
	}
	if (vehicle->isUnitClearing ())
	{
		Log.write (" Server: cannot move a vehicle currently building", cLog::eLOG_TYPE_NET_DEBUG);
		return NULL;
	}

	// reconstruct path
	sWaypoint* path = NULL;
	sWaypoint* dest = NULL;
	int iCount = 0;
	int iReceivedCount = message.popInt16();

	if (iReceivedCount == 0)
	{
		return NULL;
	}

	while (iCount < iReceivedCount)
	{
		sWaypoint* waypoint = new sWaypoint;
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
			sWaypoint* waypoint = path;
			path = path->next;
			delete waypoint;
		}
		return NULL;
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

	cPathCalculator PathCalculator (source, destination, *Map, *Vehicle);
	Waypoints = PathCalculator.calcPath();
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
	bool bInSentryRange;
	if (!Vehicle || !Waypoints || !Waypoints->next)
	{
		bFinished = true;
		return false;
	}

	// not enough waypoints for this move?
	if (Vehicle->data.getSpeed() < Waypoints->next->Costs)
	{
		Log.write (" Server: Vehicle has not enough waypoints for the next move -> EndForNow: ID: " + iToStr (Vehicle->iID) + ", X: " + iToStr (Waypoints->next->position.x()) + ", Y: " + iToStr (Waypoints->next->position.y()), LOG_TYPE_NET_DEBUG);
		iSavedSpeed += Vehicle->data.getSpeed();
		Vehicle->data.setSpeed(0);
		bEndForNow = true;
		return true;
	}

	bInSentryRange = Vehicle->InSentryRange (*server);

	if (!Map->possiblePlace (*Vehicle, Waypoints->next->position) && !bInSentryRange)
	{
		server->sideStepStealthUnit (Waypoints->next->position, *Vehicle);
	}

	//when the next field is still blocked, inform the client
	if (!Map->possiblePlace (*Vehicle, Waypoints->next->position) || bInSentryRange)    //TODO: bInSentryRange?? Why?
	{
		Log.write (" Server: Next point is blocked: ID: " + iToStr (Vehicle->iID) + ", X: " + iToStr (Waypoints->next->position.x()) + ", Y: " + iToStr (Waypoints->next->position.y()), LOG_TYPE_NET_DEBUG);
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
	Vehicle->MoveJobActive = true;
	Vehicle->setMoving (true);

	Vehicle->data.setSpeed(Vehicle->data.getSpeed() + iSavedSpeed);
	iSavedSpeed = 0;
	Vehicle->DecSpeed (Waypoints->next->Costs);

	//reset detected flag, when a water stealth unit drives into the water
	if (Vehicle->data.isStealthOn & TERRAIN_SEA && Vehicle->data.factorGround)
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
	Vehicle->getOwner ()->doScan ();
	Vehicle->setMovementOffset(cPosition(0, 0));
	setOffset (Vehicle, iNextDir, -64);

	return true;
}

void cServerMoveJob::moveVehicle()
{
	int iSpeed;
	if (!Vehicle)
		return;
	if (Vehicle->data.animationMovement)
		iSpeed = MOVE_SPEED / 2;
	else if (! (Vehicle->data.factorAir > 0) && ! (Vehicle->data.factorSea > 0 && Vehicle->data.factorGround == 0))
	{
		iSpeed = MOVE_SPEED;
		cBuilding* building = Map->getField(Waypoints->next->position).getBaseBuilding();
		if (building && building->data.modifiesSpeed)
			iSpeed = (int) (iSpeed / building->data.modifiesSpeed);
	}
	else if (Vehicle->data.factorAir > 0)
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

	sWaypoint* Waypoint;
	Waypoint = Waypoints->next;
	delete Waypoints;
	Waypoints = Waypoint;

	Vehicle->setMovementOffset(cPosition(0, 0));

	if (Waypoints->next == NULL)
	{
		bFinished = true;
	}

	// check for results of the move

	// make mines explode if necessary
	cBuilding* mine = Map->getField(Vehicle->getPosition()).getMine();
	if (Vehicle->data.factorAir == 0 && mine && mine->getOwner () != Vehicle->getOwner ())
	{
		server->addAttackJob (mine, Vehicle->getPosition());
		bEndForNow = true;
	}

	// search for resources if necessary
	if (Vehicle->data.canSurvey)
	{
		sendVehicleResources (*server, *Vehicle);
		Vehicle->doSurvey (*server);
	}

	//handle detection
	Vehicle->makeDetection (*server);

	// let other units fire on this one
	Vehicle->InSentryRange (*server);

	// lay/clear mines if necessary
	if (Vehicle->data.canPlaceMines)
	{
		bool bResult = false;
		if (Vehicle->isUnitLayingMines ()) bResult = Vehicle->layMine (*server);
		else if (Vehicle->isUnitClearingMines ()) bResult = Vehicle->clearMine (*server);
		if (bResult)
		{
			// send new unit values
			sendUnitData (*server, *Vehicle);
		}
	}

	Vehicle->setMoving (false);
	calcNextDir();

	if (Vehicle->canLand (*server->Map))
	{
		Vehicle->setFlightHeight(0);
	}
	else
	{
		Vehicle->setFlightHeight(64);
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
	sendStoreVehicle (server, vehicle_->iID, true, destVehicle->iID, *vehicle_->getOwner ());
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
		sendStoreVehicle (server, destVehicle->iID, true, vehicle_->iID, *destVehicle->getOwner ());
	}
	else if (destBuilding && destBuilding->canLoad (vehicle_))
	{
		destBuilding->storeVehicle (*vehicle_, *server.Map);
		if (vehicle_->ServerMoveJob) vehicle_->ServerMoveJob->release();
		// vehicle is removed from enemy clients by cServer::checkPlayerUnits()
		sendStoreVehicle (server, destBuilding->iID, false, vehicle_->iID, *destBuilding->getOwner ());
	}
}

void cEndMoveAction::executeAttackAction (cServer& server)
{
	// get the target unit
	const cUnit* destUnit = server.getUnitFromID (destID_);

	const auto& position = destUnit->getPosition();
	cMap& map = *server.Map;

	// check, whether the attack is now possible
	if (!vehicle_->canAttackObjectAt (position, map, true, true)) return;

	// is the target in sight?
	if (!vehicle_->getOwner ()->canSeeAnyAreaUnder (*destUnit)) return;

	server.addAttackJob (vehicle_, position);
}

cClientMoveJob::cClientMoveJob (cClient& client_, const cPosition& source_, const cPosition& destination_, cVehicle* Vehicle) :
	client (&client_),
	endMoveAction (nullptr),
	destination(destination_),
	Waypoints (nullptr)
{
	init (source_, Vehicle);
}

void cClientMoveJob::init(const cPosition& source_, cVehicle* Vehicle)
{
	Map = client->getMap().get();
	this->Vehicle = Vehicle;
	source = source_;
	this->bPlane = (Vehicle->data.factorAir > 0);
	bFinished = false;
	bEndForNow = false;
	iSavedSpeed = 0;
	bSuspended = false;

	if (Vehicle->getClientMoveJob ())
	{
		Vehicle->getClientMoveJob ()->release ();
		Vehicle->setMoving (false);
		Vehicle->MoveJobActive = false;
	}
	Vehicle->setClientMoveJob(this);
	endMoveAction = nullptr;
}

cClientMoveJob::~cClientMoveJob()
{
	sWaypoint* NextWaypoint;
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
	if (message.iType != GAME_EV_MOVE_JOB_SERVER) return false;
	int iCount = 0;
	int iReceivedCount = message.popInt16();

	Log.write (" Client: Received MoveJob: VehicleID: " + iToStr (Vehicle->iID) + ", SrcX: " + iToStr (source.x()) + ", SrcY: " + iToStr (source.y()) + ", DestX: " + iToStr (destination.x()) + ", DestY: " + iToStr (destination.y()) + ", WaypointCount: " + iToStr (iReceivedCount), cLog::eLOG_TYPE_NET_DEBUG);

	// Add the waypoints
	while (iCount < iReceivedCount)
	{
		sWaypoint* waypoint = new sWaypoint;
		waypoint->position = message.popPosition();
		waypoint->Costs = message.popInt16();

		waypoint->next = Waypoints;
		Waypoints = waypoint;

		iCount++;
	}

	calcNextDir();
	return true;
}

sWaypoint* cClientMoveJob::calcPath(const cMap& map, const cPosition& source, const cPosition& destination, const cVehicle& vehicle, const std::vector<cVehicle*>* group)
{
	if (source == destination) return 0;

	cPathCalculator PathCalculator (source, destination, map, vehicle, group);
	sWaypoint* waypoints = PathCalculator.calcPath();

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
	if (Waypoints == NULL || Waypoints->next == NULL)
	{
		Log.write (" Client: Client has already reached the last field", cLog::eLOG_TYPE_NET_DEBUG);
		bEndForNow = true;
		Vehicle->setMovementOffset(cPosition(0, 0));
		return;
	}

	switch (iType)
	{
		case MJOB_OK:
		{
			if (!Vehicle->MoveJobActive)
			{
				client->addActiveMoveJob (*this);
				activated (*Vehicle);
			}
			if (bEndForNow)
			{
				bEndForNow = false;
				client->addActiveMoveJob (*Vehicle->getClientMoveJob ());
				Log.write (" Client: reactivated movejob; Vehicle-ID: " + iToStr (Vehicle->iID), cLog::eLOG_TYPE_NET_DEBUG);
			}
			Vehicle->MoveJobActive = true;
			if (Vehicle->isUnitMoving ()) doEndMoveVehicle ();

			Vehicle->setMoving (true);
			Map->moveVehicle (*Vehicle, Waypoints->next->position);

			Vehicle->data.setSpeed(Vehicle->data.getSpeed() + this->iSavedSpeed);
			this->iSavedSpeed = 0;
			Vehicle->DecSpeed (Waypoints->next->Costs);

			//Vehicle->owner->doScan();
			Vehicle->setMovementOffset(cPosition(0, 0));
			setOffset (Vehicle, iNextDir, -64);

			if (Vehicle->data.factorAir > 0 && Vehicle->getFlightHeight () < 64)
			{
				client->addJob (new cPlaneTakeoffJob (*Vehicle, true));
			}

			moved (*Vehicle);
		}
		break;
		case MJOB_STOP:
		{
			Log.write (" Client: The movejob will end for now", cLog::eLOG_TYPE_NET_DEBUG);
			if (Vehicle->isUnitMoving ()) doEndMoveVehicle ();
			if (bEndForNow) client->addActiveMoveJob (*this);
			this->iSavedSpeed = iSavedSpeed;
			Vehicle->data.setSpeed(0);
			bSuspended = true;
			bEndForNow = true;
		}
		break;
		case MJOB_FINISHED:
		{
			Log.write (" Client: The movejob is finished", cLog::eLOG_TYPE_NET_DEBUG);
			if (Vehicle->isUnitMoving ()) doEndMoveVehicle ();
			release();
		}
		break;
		case MJOB_BLOCKED:
		{
			if (Vehicle->isUnitMoving ()) doEndMoveVehicle ();
			Log.write (" Client: next field is blocked: DestX: " + iToStr (Waypoints->next->position.x()) + ", DestY: " + iToStr (Waypoints->next->position.y()), cLog::eLOG_TYPE_NET_DEBUG);

			if (Vehicle->getOwner () != &client->getActivePlayer ())
			{
				bFinished = true;
				break;
			}

			bEndForNow = true;
			sWaypoint* path = calcPath (*client->getMap(), Vehicle->getPosition(), destination, *Vehicle);
			if (path)
			{
				sendMoveJob (*client, path, Vehicle->iID);
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
	if (Vehicle == NULL || Vehicle->getClientMoveJob () != this) return;

	// do not move the vehicle, if the movejob hasn't got any more waypoints
	if (Waypoints == NULL || Waypoints->next == NULL)
	{
		stopped (*Vehicle);
		return;
	}

	if (!Vehicle->isUnitMoving ())
	{
		//check remaining speed
		if (Vehicle->data.getSpeed() < Waypoints->next->Costs)
		{
			bSuspended = true;
			bEndForNow = true;
			stopped (*Vehicle);
			return;
		}

		Vehicle->data.setSpeed(Vehicle->data.getSpeed() + iSavedSpeed);
		iSavedSpeed = 0;
		Vehicle->DecSpeed (Waypoints->next->Costs);

		Map->moveVehicle (*Vehicle, Waypoints->next->position);
		Vehicle->getOwner ()->doScan ();
		Vehicle->setMovementOffset(cPosition(0, 0));
		setOffset (Vehicle, iNextDir, -64);
		Vehicle->setMoving (true);

		if (Vehicle->data.factorAir > 0 && Vehicle->getFlightHeight () < 64)
		{
			client->addJob (new cPlaneTakeoffJob (*Vehicle, true));
		}

		moved (*Vehicle);
	}

	int iSpeed;
	if (Vehicle->data.animationMovement)
	{
		if (client->getGameTimer ()->timer50ms)
			Vehicle->WalkFrame++;
		if (Vehicle->WalkFrame >= 13) Vehicle->WalkFrame = 1;
		iSpeed = MOVE_SPEED / 2;
	}
	else if (! (Vehicle->data.factorAir > 0) && ! (Vehicle->data.factorSea > 0 && Vehicle->data.factorGround == 0))
	{
		iSpeed = MOVE_SPEED;
		cBuilding* building = Map->getField(Waypoints->next->position).getBaseBuilding();
		if (Waypoints && Waypoints->next && building && building->data.modifiesSpeed) iSpeed = (int) (iSpeed / building->data.modifiesSpeed);
	}
	else if (Vehicle->data.factorAir > 0) iSpeed = MOVE_SPEED * 2;
	else iSpeed = MOVE_SPEED;

	// Ggf Tracks malen:
	if (cSettings::getInstance().isMakeTracks() && Vehicle->data.makeTracks && !Map->isWaterOrCoast (Vehicle->getPosition()) && !
		(Waypoints && Waypoints->next && Map->isWater (Waypoints->next->position)) &&
		(Vehicle->getOwner () == &client->getActivePlayer () || client->getActivePlayer ().canSeeAnyAreaUnder (*Vehicle)))
	{
		if (abs (Vehicle->getMovementOffset().x()) == 64 || abs (Vehicle->getMovementOffset().y()) == 64)
		{
			switch (Vehicle->dir)
			{
				case 0:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() - cPosition(0, 10), 0));
					break;
				case 4:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() + cPosition(0, 10), 0));
					break;
				case 2:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() + cPosition(10, 0), 2));
					break;
				case 6:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() - cPosition(10, 0), 2));
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
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() + cPosition(26, -26), 1));
					break;
				case 5:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() + cPosition(-26, 26), 1));
					break;
				case 3:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() + cPosition(26, 26), 3));
					break;
				case 7:
					client->addFx (std::make_shared<cFxTracks> (Vehicle->getPosition() * 64 + Vehicle->getMovementOffset() + cPosition(-26, -26), 3));
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
	if (Vehicle == NULL || Vehicle->getClientMoveJob () != this) return;

	if (Waypoints->next == NULL)
	{
		// this is just to avoid errors, this should normaly never happen.
		bFinished = true;
		return;
	}

	Vehicle->WalkFrame = 0;

	sWaypoint* Waypoint = Waypoints;
	Waypoints = Waypoints->next;
	delete Waypoint;

	Vehicle->setMoving (false);

	Vehicle->setMovementOffset(cPosition(0, 0));

	Vehicle->getOwner ()->doScan ();

	calcNextDir();

	if (Vehicle->canLand (*client->getMap ()) && Vehicle->getFlightHeight() > 0)
	{
		client->addJob (new cPlaneTakeoffJob (*Vehicle, false));
	}
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
		SDL_BlitSurface (OtherData.WayPointPfeileSpecial[iIndex][64 - Dest.w].get (), NULL, cVideo::buffer, &Dest);
	}
	else
	{
		SDL_BlitSurface (OtherData.WayPointPfeile[iIndex][64 - Dest.w].get (), NULL, cVideo::buffer, &Dest);
	}
}
