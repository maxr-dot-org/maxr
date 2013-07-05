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

#include <math.h>

#include "movejobs.h"

#include "attackJobs.h"
#include "buildings.h"
#include "client.h"
#include "clientevents.h"
#include "clist.h"
#include "fxeffects.h"
#include "netmessage.h"
#include "player.h"
#include "server.h"
#include "serverevents.h"
#include "settings.h"
#include "vehicles.h"

cPathDestHandler::cPathDestHandler (ePathDestinationTypes type_, int destX_, int destY_, const cVehicle* srcVehicle_, const cUnit* destUnit_) :
	type (type_),
	srcVehicle (srcVehicle_),
	destUnit (destUnit_),
	destX (destX_),
	destY (destY_)
{}

bool cPathDestHandler::hasReachedDestination (int x, int y) const
{
	switch (type)
	{
		case PATH_DEST_TYPE_POS:
			return (x == destX && y == destY);
		case PATH_DEST_TYPE_LOAD:
			return (destUnit && destUnit->isNextTo (x, y));
		case PATH_DEST_TYPE_ATTACK:
			x -= destX;
			y -= destY;
			return Square (x) + Square (y) <= Square (srcVehicle->data.range);
		default:
			return true;
	}
	return false;
}

int cPathDestHandler::heuristicCost (int srcX, int srcY) const
{
	switch (type)
	{
		case PATH_DEST_TYPE_POS:
		case PATH_DEST_TYPE_LOAD:
			return 0;
		case PATH_DEST_TYPE_ATTACK:
		default:
		{
			int deltaX = destX - srcX;
			int deltaY = destY - srcY;

			return Round (sqrtf (static_cast<float> (Square (deltaX) + Square (deltaY))));
		}
	}
}

cPathCalculator::cPathCalculator (int ScrX, int ScrY, int DestX, int DestY, const cMap& Map, const cVehicle& Vehicle, const std::vector<cVehicle*>* group)
{
	destHandler = new cPathDestHandler (PATH_DEST_TYPE_POS, DestX, DestY, NULL, NULL);
	init (ScrX, ScrY, Map, Vehicle, group);
}


cPathCalculator::cPathCalculator (int ScrX, int ScrY, const cUnit& destUnit, const cMap& Map, const cVehicle& Vehicle, bool load)
{
	destHandler = new cPathDestHandler (load ? PATH_DEST_TYPE_LOAD : PATH_DEST_TYPE_ATTACK, 0, 0, &Vehicle, &destUnit);
	init (ScrX, ScrY, Map, Vehicle, NULL);
}

cPathCalculator::cPathCalculator (int ScrX, int ScrY, const cMap& Map, const cVehicle& Vehicle, int attackX, int attackY)
{
	destHandler = new cPathDestHandler (PATH_DEST_TYPE_ATTACK, attackX, attackY, &Vehicle, NULL);
	init (ScrX, ScrY, Map, Vehicle, NULL);
}

void cPathCalculator::init (int ScrX, int ScrY, const cMap& Map, const cVehicle& Vehicle, const std::vector<cVehicle*>* group)
{
	this->ScrX = ScrX;
	this->ScrY = ScrY;
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
	nodesHeap.resize (Map->getSize() * Map->getSize() + 1, NULL);
	openList.resize (Map->getSize() * Map->getSize() + 1, NULL);
	closedList.resize (Map->getSize() * Map->getSize() + 1, NULL);

	// generate startnode
	sPathNode* StartNode = allocNode();
	StartNode->x = ScrX;
	StartNode->y = ScrY;
	StartNode->costG = 0;
	StartNode->costH = destHandler->heuristicCost (ScrX, ScrY);
	StartNode->costF = StartNode->costG + StartNode->costH;

	StartNode->prev = NULL;
	openList[Map->getOffset (ScrX, ScrY)] = StartNode;
	insertToHeap (StartNode, false);

	while (heapCount > 0)
	{
		// get the node with the lowest F value
		sPathNode* CurrentNode = nodesHeap[1];

		// move it from the open to the closed list
		openList[Map->getOffset (CurrentNode->x, CurrentNode->y)] = NULL;
		closedList[Map->getOffset (CurrentNode->x, CurrentNode->y)] = CurrentNode;
		deleteFirstFromHeap();

		// generate waypoints when destination has been reached
		if (destHandler->hasReachedDestination (CurrentNode->x, CurrentNode->y))
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
			NextWaypoint->X = NextNode->x;
			NextWaypoint->Y = NextNode->y;
			NextWaypoint->Costs = 0;
			do
			{
				NextNode = NextNode->next;

				NextWaypoint->next = new sWaypoint;
				NextWaypoint->next->X = NextNode->x;
				NextWaypoint->next->Y = NextNode->y;
				NextWaypoint->next->Costs = calcNextCost (NextNode->prev->x, NextNode->prev->y, NextWaypoint->next->X, NextWaypoint->next->Y);
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
	const int minx = std::max (ParentNode->x - 1, 0);
	const int maxx = std::min (ParentNode->x + 1, Map->getSize() - 1);
	const int miny = std::max (ParentNode->y - 1, 0);
	const int maxy = std::min (ParentNode->y + 1, Map->getSize() - 1);

	for (int y = miny; y <= maxy; ++y)
	{
		for (int x = minx; x <= maxx; ++x)
		{
			if (x == ParentNode->x && y == ParentNode->y) continue;

			if (!Map->possiblePlace (*Vehicle, x, y, true))
			{
				// when we have a group of units, the units will not block each other
				if (group)
				{
					// get the blocking unit
					cVehicle* blockingUnit;
					if (Vehicle->data.factorAir > 0) blockingUnit = (*Map) [Map->getOffset (x, y)].getPlane();
					else blockingUnit = (*Map) [Map->getOffset (x, y)].getVehicle();
					// check whether the blocking unit is the group
					bool isInGroup = Contains (*group, blockingUnit);
					if (!isInGroup) continue;
				}
				else continue;
			}
			if (closedList[Map->getOffset (x, y)] != NULL) continue;

			if (openList[Map->getOffset (x, y)] == NULL)
			{
				// generate new node
				sPathNode* NewNode = allocNode();
				NewNode->x = x;
				NewNode->y = y;
				NewNode->costG = calcNextCost (ParentNode->x, ParentNode->y, x, y) + ParentNode->costG;
				NewNode->costH = destHandler->heuristicCost (x, y);
				NewNode->costF = NewNode->costG + NewNode->costH;
				NewNode->prev = ParentNode;
				openList[Map->getOffset (x, y)] = NewNode;
				insertToHeap (NewNode, false);
			}
			else
			{
				// modify existing node
				int costG, costH, costF;
				costG = calcNextCost (ParentNode->x, ParentNode->y, x, y) + ParentNode->costG;
				costH = destHandler->heuristicCost (x, y);
				costF = costG + costH;
				if (costF < openList[Map->getOffset (x, y)]->costF)
				{
					openList[Map->getOffset (x, y)]->costG = costG;
					openList[Map->getOffset (x, y)]->costH = costH;
					openList[Map->getOffset (x, y)]->costF = costF;
					openList[Map->getOffset (x, y)]->prev = ParentNode;
					insertToHeap (openList[Map->getOffset (x, y)], true);
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

int cPathCalculator::calcNextCost (int srcX, int srcY, int destX, int destY) const
{
	int costs;
	// first we check whether the unit can fly
	if (Vehicle->data.factorAir > 0)
	{
		if (srcX != destX && srcY != destY) return (int) (4 * 1.5f * Vehicle->data.factorAir);
		else return (int) (4 * Vehicle->data.factorAir);
	}
	const int offset = Map->getOffset (destX, destY);
	const cBuilding* building = Map->fields[offset].getBaseBuilding();
	// moving on water will cost more
	if (Map->isWater (offset) && (!building || (building->data.surfacePosition == sUnitData::SURFACE_POS_BENEATH_SEA || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE)) && Vehicle->data.factorSea > 0) costs = (int) (4 * Vehicle->data.factorSea);
	else if (Map->isCoast (offset) && !building && Vehicle->data.factorCoast > 0) costs = (int) (4 * Vehicle->data.factorCoast);
	else if (Vehicle->data.factorGround > 0) costs = (int) (4 * Vehicle->data.factorGround);
	else
	{
		Log.write ("Where can this unit move? " + iToStr (Vehicle->iID), cLog::eLOG_TYPE_NET_WARNING);
		costs = 4;
	}
	// moving on a road is cheaper
	if (building && building->data.modifiesSpeed != 0) costs = (int) (costs * building->data.modifiesSpeed);

	// multiplicate with the factor 1.5 for diagonal movements
	if (srcX != destX && srcY != destY) costs = (int) (costs * 1.5f);
	return costs;
}

static void setOffset (cVehicle* Vehicle, int nextDir, int offset)
{
	assert (0 <= nextDir && nextDir < 8);
	//                       N, NE, E, SE, S, SW,  W, NW
	const int offsetX[8] = { 0,  1, 1,  1, 0, -1, -1, -1};
	const int offsetY[8] = {-1, -1, 0,  1, 1,  1,  0, -1};

	Vehicle->OffX += offsetX[nextDir] * offset;
	Vehicle->OffY += offsetY[nextDir] * offset;
}

static int getDir (int x, int y, int nextX, int nextY)
{
	const int diffX = nextX - x;
	const int diffY = nextY - y;
	//                       N, NE, E, SE, S, SW,  W, NW
	const int offsetX[8] = { 0,  1, 1,  1, 0, -1, -1, -1};
	const int offsetY[8] = {-1, -1, 0,  1, 1,  1,  0, -1};

	for (int i = 0; i != 8; ++i) {
		if (diffX == offsetX[i] && diffY == offsetY[i]) return i;
	}
	assert (false);
	return -1;
}

cServerMoveJob::cServerMoveJob (cServer& server_, int srcX_, int srcY_, int destX_, int destY_, cVehicle* vehicle) :
	server (&server_)
{
	Map = server->Map;
	this->Vehicle = vehicle;
	SrcX = srcX_;
	SrcY = srcY_;
	DestX = destX_;
	DestY = destY_;
	bPlane = (Vehicle->data.factorAir > 0);
	bFinished = false;
	bEndForNow = false;
	iSavedSpeed = 0;
	Waypoints = NULL;
	endAction = NULL;

	// unset sentry status when moving vehicle
	if (Vehicle->sentryActive)
	{
		Vehicle->owner->deleteSentry (Vehicle);
	}
	sendUnitData (*server, *Vehicle, Vehicle->owner->getNr());
	for (unsigned int i = 0; i < Vehicle->seenByPlayerList.size(); i++)
	{
		sendUnitData (*server, *Vehicle, Vehicle->seenByPlayerList[i]->getNr());
	}

	if (Vehicle->ServerMoveJob)
	{
		iSavedSpeed = Vehicle->ServerMoveJob->iSavedSpeed;
		Vehicle->ServerMoveJob->release();
		Vehicle->moving = false;
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
	if (!Vehicle->moving)
	{
		release();
	}
}

void cServerMoveJob::resume()
{
	if (Vehicle && Vehicle->data.speedCur > 0 && !Vehicle->moving)
	{
		// restart movejob
		calcNextDir();
		bEndForNow = false;
		SrcX = Vehicle->PosX;
		SrcY = Vehicle->PosY;
		server->addActiveMoveJob (this);
	}
}

void cServerMoveJob::addEndAction (int destID, eEndMoveActionType type)
{
	delete endAction;

	endAction = new cEndMoveAction (Vehicle, destID, type);
	sendEndMoveActionToClient (*server, *Vehicle, destID, type);

}

cServerMoveJob* cServerMoveJob::generateFromMessage (cServer& server, cNetMessage* message)
{
	if (message->iType != GAME_EV_MOVE_JOB_CLIENT) return NULL;

	int iVehicleID = message->popInt32();
	cVehicle* vehicle = server.getVehicleFromID (iVehicleID);
	if (vehicle == NULL)
	{
		Log.write (" Server: Can't find vehicle with id " + iToStr (iVehicleID), cLog::eLOG_TYPE_NET_WARNING);
		return NULL;
	}

	// TODO: is this check really needed?
	if (vehicle->isBeeingAttacked)
	{
		Log.write (" Server: cannot move a vehicle currently under attack", cLog::eLOG_TYPE_NET_DEBUG);
		return NULL;
	}
	if (vehicle->attacking)
	{
		Log.write (" Server: cannot move a vehicle currently attacking", cLog::eLOG_TYPE_NET_DEBUG);
		return NULL;
	}
	if (vehicle->IsBuilding || (vehicle->BuildPath && vehicle->ServerMoveJob))
	{
		Log.write (" Server: cannot move a vehicle currently building", cLog::eLOG_TYPE_NET_DEBUG);
		return NULL;
	}
	if (vehicle->IsClearing)
	{
		Log.write (" Server: cannot move a vehicle currently building", cLog::eLOG_TYPE_NET_DEBUG);
		return NULL;
	}

	// reconstruct path
	sWaypoint* path = NULL;
	sWaypoint* dest = NULL;
	int iCount = 0;
	int iReceivedCount = message->popInt16();

	if (iReceivedCount == 0)
	{
		return NULL;
	}

	while (iCount < iReceivedCount)
	{
		sWaypoint* waypoint = new sWaypoint;
		waypoint->Y = message->popInt16();
		waypoint->X = message->popInt16();
		waypoint->Costs = message->popInt16();

		if (!dest) dest = waypoint;

		waypoint->next = path;
		path = waypoint;

		iCount++;
	}

	//is the vehicle position equal to the begin of the path?
	if (vehicle->PosX != path->X || vehicle->PosY != path->Y)
	{
		Log.write (" Server: Vehicle with id " + iToStr (iVehicleID) + " is at wrong position (" + iToStr (vehicle->PosX) + "x" + iToStr (vehicle->PosY) + ") for movejob from " +  iToStr (path->X) + "x" + iToStr (path->Y) + " to " + iToStr (dest->X) + "x" + iToStr (dest->Y), cLog::eLOG_TYPE_NET_WARNING);

		while (path)
		{
			sWaypoint* waypoint = path;
			path = path->next;
			delete waypoint;
		}
		return NULL;
	}

	//everything is ok. Construct the movejob
	Log.write (" Server: Received MoveJob: VehicleID: " + iToStr (vehicle->iID) + ", SrcX: " + iToStr (path->X) + ", SrcY: " + iToStr (path->Y) + ", DestX: " + iToStr (dest->X) + ", DestY: " + iToStr (dest->Y) + ", WaypointCount: " + iToStr (iReceivedCount), cLog::eLOG_TYPE_NET_DEBUG);
	cServerMoveJob* mjob = new cServerMoveJob (server, path->X, path->Y, dest->X, dest->Y, vehicle);
	mjob->Waypoints = path;

	mjob->calcNextDir();

	return mjob;
}

bool cServerMoveJob::calcPath()
{
	if (SrcX == DestX && SrcY == DestY) return false;

	cPathCalculator PathCalculator (SrcX, SrcY, DestX, DestY, *Map, *Vehicle);
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
	server->addActiveMoveJob (this);
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
	if (Vehicle->data.speedCur < Waypoints->next->Costs)
	{
		Log.write (" Server: Vehicle has not enough waypoints for the next move -> EndForNow: ID: " + iToStr (Vehicle->iID) + ", X: " + iToStr (Waypoints->next->X) + ", Y: " + iToStr (Waypoints->next->Y), LOG_TYPE_NET_DEBUG);
		iSavedSpeed += Vehicle->data.speedCur;
		Vehicle->data.speedCur = 0;
		bEndForNow = true;
		return true;
	}

	bInSentryRange = Vehicle->InSentryRange (*server);

	if (!Map->possiblePlace (*Vehicle, Waypoints->next->X, Waypoints->next->Y) && !bInSentryRange)
	{
		server->sideStepStealthUnit (Waypoints->next->X, Waypoints->next->Y, Vehicle);
	}

	//when the next field is still blocked, inform the client
	if (!Map->possiblePlace (*Vehicle, Waypoints->next->X, Waypoints->next->Y) || bInSentryRange)    //TODO: bInSentryRange?? Why?
	{
		Log.write (" Server: Next point is blocked: ID: " + iToStr (Vehicle->iID) + ", X: " + iToStr (Waypoints->next->X) + ", Y: " + iToStr (Waypoints->next->Y), LOG_TYPE_NET_DEBUG);
		// if the next point would be the last, finish the job here
		if (Waypoints->next->X == DestX && Waypoints->next->Y == DestY)
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
	Vehicle->moving = true;

	Vehicle->data.speedCur += iSavedSpeed;
	iSavedSpeed = 0;
	Vehicle->DecSpeed (Waypoints->next->Costs);

	//reset detected flag, when a water stealth unit drives into the water
	if (Vehicle->data.isStealthOn & TERRAIN_SEA && Vehicle->data.factorGround)
	{
		bool wasOnLand = !Map->isWater (Waypoints->X, Waypoints->Y);
		bool driveIntoWater = Map->isWater (Waypoints->next->X, Waypoints->next->Y);

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

	Map->moveVehicle (*Vehicle, Waypoints->next->X, Waypoints->next->Y);
	Vehicle->owner->doScan();
	Vehicle->OffX = 0;
	Vehicle->OffY = 0;
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
		cBuilding* building = Map->fields[Map->getOffset (Waypoints->next->X, Waypoints->next->Y)].getBaseBuilding();
		if (building && building->data.modifiesSpeed)
			iSpeed = (int) (iSpeed / building->data.modifiesSpeed);
	}
	else if (Vehicle->data.factorAir > 0)
		iSpeed = MOVE_SPEED * 2;
	else
		iSpeed = MOVE_SPEED;

	setOffset (Vehicle, iNextDir, iSpeed);

	// check whether the point has been reached:
	if (abs (Vehicle->OffX) < iSpeed && abs (Vehicle->OffY) < iSpeed)
		doEndMoveVehicle();
}

void cServerMoveJob::doEndMoveVehicle()
{
	Log.write (" Server: Vehicle reached the next field: ID: " + iToStr (Vehicle->iID) + ", X: " + iToStr (Waypoints->next->X) + ", Y: " + iToStr (Waypoints->next->Y), cLog::eLOG_TYPE_NET_DEBUG);

	sWaypoint* Waypoint;
	Waypoint = Waypoints->next;
	delete Waypoints;
	Waypoints = Waypoint;

	Vehicle->OffX = 0;
	Vehicle->OffY = 0;

	if (Waypoints->next == NULL)
	{
		bFinished = true;
	}

	// check for results of the move

	// make mines explode if necessary
	cBuilding* mine = Map->fields[Map->getOffset (Vehicle->PosX, Vehicle->PosY)].getMine();
	if (Vehicle->data.factorAir == 0 && mine && mine->owner != Vehicle->owner)
	{
		server->AJobs.push_back (new cServerAttackJob (*server, mine, Map->getOffset (Vehicle->PosX, Vehicle->PosY), false));
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
		if (Vehicle->LayMines) bResult = Vehicle->layMine (*server);
		else if (Vehicle->ClearMines) bResult = Vehicle->clearMine (*server);
		if (bResult)
		{
			// send new unit values
			sendUnitData (*server, *Vehicle, Vehicle->owner->getNr());
			for (unsigned int i = 0; i < Vehicle->seenByPlayerList.size(); i++)
			{
				sendUnitData (*server, *Vehicle, Vehicle->seenByPlayerList[i]->getNr());
			}
		}
	}

	Vehicle->moving = false;
	calcNextDir();

	if (Vehicle->canLand (*server->Map))
	{
		Vehicle->FlightHigh = 0;
	}
	else
	{
		Vehicle->FlightHigh = 64;
	}
}

void cServerMoveJob::calcNextDir()
{
	if (!Waypoints || !Waypoints->next) return;
	iNextDir = getDir (Waypoints->X, Waypoints->Y, Waypoints->next->X, Waypoints->next->Y);
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

	vehicle_->storeVehicle (destVehicle, server.Map);
	if (destVehicle->ServerMoveJob) destVehicle->ServerMoveJob->release();

	// vehicle is removed from enemy clients by cServer::checkPlayerUnits()
	sendStoreVehicle (server, vehicle_->iID, true, destVehicle->iID, vehicle_->owner->getNr());
}

void cEndMoveAction::executeGetInAction (cServer& server)
{
	cVehicle* destVehicle = server.getVehicleFromID (destID_);
	cBuilding* destBuilding = server.getBuildingFromID (destID_);

	// execute the loading if possible
	if (destVehicle && destVehicle->canLoad (vehicle_))
	{
		destVehicle->storeVehicle (vehicle_, server.Map);
		if (vehicle_->ServerMoveJob) vehicle_->ServerMoveJob->release();
		//vehicle is removed from enemy clients by cServer::checkPlayerUnits()
		sendStoreVehicle (server, destVehicle->iID, true, vehicle_->iID, destVehicle->owner->getNr());
	}
	else if (destBuilding && destBuilding->canLoad (vehicle_))
	{
		destBuilding->storeVehicle (vehicle_, server.Map);
		if (vehicle_->ServerMoveJob) vehicle_->ServerMoveJob->release();
		// vehicle is removed from enemy clients by cServer::checkPlayerUnits()
		sendStoreVehicle (server, destBuilding->iID, false, vehicle_->iID, destBuilding->owner->getNr());
	}
}

void cEndMoveAction::executeAttackAction (cServer& server)
{
	// get the target unit
	const cUnit* destUnit = server.getVehicleFromID (destID_);
	if (destUnit == NULL) destUnit = server.getBuildingFromID (destID_);
	if (destUnit == NULL) return;

	int x = destUnit->PosX;
	int y = destUnit->PosY;
	cMap& map = *server.Map;
	const int offset = map.getOffset (x, y);

	// check, whether the attack is now possible
	if (!vehicle_->canAttackObjectAt (x, y, &map, true, true)) return;

	// is the target in sight?
	if (!vehicle_->owner->canSeeAnyAreaUnder (*destUnit)) return;

	server.AJobs.push_back (new cServerAttackJob (server, vehicle_, offset, false));
}

cClientMoveJob::cClientMoveJob (cClient& client_, int iSrcOff, int iDestOff, cVehicle* Vehicle) :
	client (&client_),
	Waypoints (NULL)
{
	DestX = iDestOff % client->getMap()->getSize();
	DestY = iDestOff / client->getMap()->getSize();
	init (iSrcOff, Vehicle);
}

void cClientMoveJob::init (int iSrcOff, cVehicle* Vehicle)
{
	Map = client->getMap();
	this->Vehicle = Vehicle;
	ScrX = iSrcOff % Map->getSize();
	ScrY = iSrcOff / Map->getSize();
	this->bPlane = (Vehicle->data.factorAir > 0);
	bFinished = false;
	bEndForNow = false;
	bSoundRunning = false;
	iSavedSpeed = 0;
	bSuspended = false;

	if (Vehicle->ClientMoveJob)
	{
		Vehicle->ClientMoveJob->release();
		Vehicle->moving = false;
		Vehicle->MoveJobActive = false;
	}
	Vehicle->ClientMoveJob = this;
	endMoveAction = NULL;
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

bool cClientMoveJob::generateFromMessage (cNetMessage* message)
{
	if (message->iType != GAME_EV_MOVE_JOB_SERVER) return false;
	int iCount = 0;
	int iReceivedCount = message->popInt16();

	Log.write (" Client: Received MoveJob: VehicleID: " + iToStr (Vehicle->iID) + ", SrcX: " + iToStr (ScrX) + ", SrcY: " + iToStr (ScrY) + ", DestX: " + iToStr (DestX) + ", DestY: " + iToStr (DestY) + ", WaypointCount: " + iToStr (iReceivedCount), cLog::eLOG_TYPE_NET_DEBUG);

	// Add the waypoints
	while (iCount < iReceivedCount)
	{
		sWaypoint* waypoint = new sWaypoint;
		waypoint->Y = message->popInt16();
		waypoint->X = message->popInt16();
		waypoint->Costs = message->popInt16();

		waypoint->next = Waypoints;
		Waypoints = waypoint;

		iCount++;
	}

	calcNextDir();
	return true;
}

sWaypoint* cClientMoveJob::calcPath (const cMap& map, int SrcX, int SrcY, int DestX, int DestY, const cVehicle& vehicle, const std::vector<cVehicle*>* group)
{
	if (SrcX == DestX && SrcY == DestY) return 0;

	cPathCalculator PathCalculator (SrcX, SrcY, DestX, DestY, map, vehicle, group);
	sWaypoint* waypoints = PathCalculator.calcPath();

	return waypoints;
}

void cClientMoveJob::release()
{
	bEndForNow = false;
	bFinished = true;
	Log.write (" Client: Released old movejob", cLog::eLOG_TYPE_NET_DEBUG);
	client->addActiveMoveJob (this);
	Log.write (" Client: Added released movejob to avtive ones", cLog::eLOG_TYPE_NET_DEBUG);
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
		Vehicle->OffX = Vehicle->OffY = 0;
		return;
	}

	switch (iType)
	{
		case MJOB_OK:
		{
			if (!Vehicle->MoveJobActive)
			{
				startMoveSound();
				client->addActiveMoveJob (this);
				if (client->gameGUI.getSelectedUnit() == Vehicle) client->gameGUI.unitMenuActive = false;
			}
			if (bEndForNow)
			{
				bEndForNow = false;
				client->addActiveMoveJob (Vehicle->ClientMoveJob);
				Log.write (" Client: reactivated movejob; Vehicle-ID: " + iToStr (Vehicle->iID), cLog::eLOG_TYPE_NET_DEBUG);
			}
			Vehicle->MoveJobActive = true;
			if (Vehicle->moving) doEndMoveVehicle();

			Vehicle->moving = true;
			Map->moveVehicle (*Vehicle, Waypoints->next->X, Waypoints->next->Y);
			//Vehicle->owner->doScan();
			Vehicle->OffX = 0;
			Vehicle->OffY = 0;
			setOffset (Vehicle, iNextDir, -64);
		}
		break;
		case MJOB_STOP:
		{
			Log.write (" Client: The movejob will end for now", cLog::eLOG_TYPE_NET_DEBUG);
			if (Vehicle->moving) doEndMoveVehicle();
			if (bEndForNow) client->addActiveMoveJob (this);
			this->iSavedSpeed = iSavedSpeed;
			Vehicle->data.speedCur = 0;
			bSuspended = true;
			bEndForNow = true;
		}
		break;
		case MJOB_FINISHED:
		{
			Log.write (" Client: The movejob is finished", cLog::eLOG_TYPE_NET_DEBUG);
			if (Vehicle->moving) doEndMoveVehicle();
			release();
		}
		break;
		case MJOB_BLOCKED:
		{
			if (Vehicle->moving) doEndMoveVehicle();
			Log.write (" Client: next field is blocked: DestX: " + iToStr (Waypoints->next->X) + ", DestY: " + iToStr (Waypoints->next->Y), cLog::eLOG_TYPE_NET_DEBUG);

			if (Vehicle->owner != client->getActivePlayer())
			{
				bFinished = true;
				break;
			}

			bEndForNow = true;
			sWaypoint* path = calcPath (*client->getMap(), Vehicle->PosX, Vehicle->PosY, DestX, DestY, *Vehicle);
			if (path)
			{
				sendMoveJob (*client, path, Vehicle->iID);
				if (endMoveAction) sendEndMoveAction (*client, Vehicle->iID, endMoveAction->destID_, endMoveAction->type_);
			}
			else
			{
				bFinished = true;

				if (Vehicle == client->gameGUI.getSelectedUnit())
				{
					PlayRandomVoice (VoiceData.VOINoPath);
				}
			}
		}
		break;
	}
}

void cClientMoveJob::moveVehicle()
{
	if (Vehicle == NULL || Vehicle->ClientMoveJob != this) return;

	// do not move the vehicle, if the movejob hasn't got any more waypoints
	if (Waypoints == NULL || Waypoints->next == NULL)
	{
		stopMoveSound();
		return;
	}

	if (!Vehicle->moving)
	{
		//check remaining speed
		if (Vehicle->data.speedCur < Waypoints->next->Costs)
		{
			bSuspended = true;
			bEndForNow = true;
			stopMoveSound();
			return;
		}

		Map->moveVehicle (*Vehicle, Waypoints->next->X, Waypoints->next->Y);
		Vehicle->owner->doScan();
		Vehicle->OffX = 0;
		Vehicle->OffY = 0;
		setOffset (Vehicle, iNextDir, -64);
		Vehicle->moving = true;

		//restart movesound, when drinving into or out of water
		if (Vehicle == client->gameGUI.getSelectedUnit())
		{
			bool wasWater = Map->isWater (Waypoints->X, Waypoints->Y);
			bool water = Map->isWater (Waypoints->next->X, Waypoints->next->Y);

			if (wasWater != water)
			{
				Vehicle->StartMoveSound (client->gameGUI);
			}
		}
	}

	int iSpeed;
	if (Vehicle->data.animationMovement)
	{
		if (client->gameTimer.timer50ms)
			Vehicle->WalkFrame++;
		if (Vehicle->WalkFrame >= 13) Vehicle->WalkFrame = 1;
		iSpeed = MOVE_SPEED / 2;
	}
	else if (! (Vehicle->data.factorAir > 0) && ! (Vehicle->data.factorSea > 0 && Vehicle->data.factorGround == 0))
	{
		iSpeed = MOVE_SPEED;
		cBuilding* building = Map->fields[Map->getOffset (Waypoints->next->X, Waypoints->next->Y)].getBaseBuilding();
		if (Waypoints && Waypoints->next && building && building->data.modifiesSpeed) iSpeed = (int) (iSpeed / building->data.modifiesSpeed);
	}
	else if (Vehicle->data.factorAir > 0) iSpeed = MOVE_SPEED * 2;
	else iSpeed = MOVE_SPEED;

	// Ggf Tracks malen:
	if (cSettings::getInstance().isMakeTracks() && Vehicle->data.makeTracks && !Map->isWaterOrCoast (Vehicle->PosX, Vehicle->PosY) && !
		(Waypoints && Waypoints->next && Map->isWater (Waypoints->next->X, Waypoints->next->Y)) &&
		(Vehicle->owner == client->getActivePlayer() || client->getActivePlayer()->canSeeAnyAreaUnder (*Vehicle)))
	{
		if (abs (Vehicle->OffX) == 64 || abs (Vehicle->OffY) == 64)
		{
			switch (Vehicle->dir)
			{
				case 0:
					client->addFx (new cFxTracks (Vehicle->PosX * 64 + Vehicle->OffX, Vehicle->PosY * 64 - 10 + Vehicle->OffY, 0));
					break;
				case 4:
					client->addFx (new cFxTracks (Vehicle->PosX * 64 + Vehicle->OffX, Vehicle->PosY * 64 + 10 + Vehicle->OffY, 0));
					break;
				case 2:
					client->addFx (new cFxTracks (Vehicle->PosX * 64 + 10 + Vehicle->OffX, Vehicle->PosY * 64 + Vehicle->OffY, 2));
					break;
				case 6:
					client->addFx (new cFxTracks (Vehicle->PosX * 64 - 10 + Vehicle->OffX, Vehicle->PosY * 64 + Vehicle->OffY, 2));
					break;
				case 1:
				case 5:
					client->addFx (new cFxTracks (Vehicle->PosX * 64 + Vehicle->OffX, Vehicle->PosY * 64 + Vehicle->OffY, 1));
					break;
				case 3:
				case 7:
					client->addFx (new cFxTracks (Vehicle->PosX * 64 + Vehicle->OffX, Vehicle->PosY * 64 + Vehicle->OffY, 3));
					break;
			}
		}
		else if (abs (Vehicle->OffX) == 64 - (iSpeed * 2) || abs (Vehicle->OffY) == 64 - (iSpeed * 2))
		{
			switch (Vehicle->dir)
			{
				case 1:
					client->addFx (new cFxTracks (Vehicle->PosX * 64 + 26 + Vehicle->OffX, Vehicle->PosY * 64 - 26 + Vehicle->OffY, 1));
					break;
				case 5:
					client->addFx (new cFxTracks (Vehicle->PosX * 64 - 26 + Vehicle->OffX, Vehicle->PosY * 64 + 26 + Vehicle->OffY, 1));
					break;
				case 3:
					client->addFx (new cFxTracks (Vehicle->PosX * 64 + 26 + Vehicle->OffX, Vehicle->PosY * 64 + 26 + Vehicle->OffY, 3));
					break;
				case 7:
					client->addFx (new cFxTracks (Vehicle->PosX * 64 - 26 + Vehicle->OffX, Vehicle->PosY * 64 - 26 + Vehicle->OffY, 3));
					break;
			}
		}
	}

	setOffset (Vehicle, iNextDir, iSpeed);

	if (abs (Vehicle->OffX) > 70 || abs (Vehicle->OffY) > 70)
	{
		Log.write (" Client: Flying dutchmen detected! Unit ID: " + iToStr (Vehicle->iID) + " at position (" + iToStr (Vehicle->PosX) + ":" + iToStr (Vehicle->PosY) + ")", cLog::eLOG_TYPE_NET_DEBUG);
	}
}

void cClientMoveJob::doEndMoveVehicle()
{
	if (Vehicle == NULL || Vehicle->ClientMoveJob != this) return;

	if (Waypoints->next == NULL)
	{
		// this is just to avoid errors, this should normaly never happen.
		bFinished = true;
		return;
	}

	Vehicle->data.speedCur += iSavedSpeed;
	iSavedSpeed = 0;
	Vehicle->DecSpeed (Waypoints->next->Costs);
	Vehicle->WalkFrame = 0;

	sWaypoint* Waypoint = Waypoints;
	Waypoints = Waypoints->next;
	delete Waypoint;

	Vehicle->moving = false;

	Vehicle->OffX = 0;
	Vehicle->OffY = 0;

	client->gameGUI.callMiniMapDraw();
	client->gameGUI.updateMouseCursor();
	Vehicle->owner->doScan();

	calcNextDir();
}

void cClientMoveJob::calcNextDir()
{
	if (!Waypoints || !Waypoints->next) return;
	iNextDir = getDir (Waypoints->X, Waypoints->Y, Waypoints->next->X, Waypoints->next->Y);
}

void cClientMoveJob::drawArrow (SDL_Rect Dest, SDL_Rect* LastDest, bool bSpezial)
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
		SDL_BlitSurface (OtherData.WayPointPfeileSpecial[iIndex][64 - client->gameGUI.getTileSize()], NULL, buffer, &Dest);
	}
	else
	{
		SDL_BlitSurface (OtherData.WayPointPfeile[iIndex][64 - client->gameGUI.getTileSize()], NULL, buffer, &Dest);
	}
}

void cClientMoveJob::startMoveSound()
{
	cGameGUI& gameGUI = client->gameGUI;
	if (Vehicle == gameGUI.getSelectedUnit()) Vehicle->StartMoveSound (gameGUI);
	bSoundRunning = true;
}

void cClientMoveJob::stopMoveSound()
{
	if (!bSoundRunning) return;

	bSoundRunning = false;
	cGameGUI& gameGUI = client->gameGUI;

	if (Vehicle != gameGUI.getSelectedUnit()) return;

	cBuilding* building = Map->fields[Map->getOffset (Vehicle->PosX, Vehicle->PosY)].getBaseBuilding();
	bool water = Map->isWater (Vehicle->PosX, Vehicle->PosY);
	if (Vehicle->data.factorGround > 0 && building && (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)) water = false;

	StopFXLoop (gameGUI.iObjectStream);
	if (water && Vehicle->data.factorSea > 0) PlayFX (Vehicle->typ->StopWater);
	else PlayFX (Vehicle->typ->Stop);

	gameGUI.iObjectStream = Vehicle->playStream (gameGUI);
}
