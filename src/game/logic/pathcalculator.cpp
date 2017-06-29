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


#include "pathcalculator.h"
#include "game/data/units/unit.h"
#include "game/data/units/vehicle.h"
#include "game/data/map/map.h"
#include "utility/listhelpers.h"
#include "game/data/units/building.h"
#include <forward_list>


/* Size of a memory block while pathfinding */
#define MEM_BLOCK_SIZE 10

cPathDestHandler::cPathDestHandler (ePathDestinationTypes type_, const cPosition& destination_, const cVehicle* srcVehicle_, const cUnit* destUnit_) :
	type (type_),
	srcVehicle (srcVehicle_),
	destUnit (destUnit_),
	destination (destination_)
{}

bool cPathDestHandler::hasReachedDestination (const cPosition& position) const
{
	switch (type)
	{
		case PATH_DEST_TYPE_POS:
			return (destination == position);
		case PATH_DEST_TYPE_LOAD:
			return (destUnit && destUnit->isNextTo (position));
		case PATH_DEST_TYPE_ATTACK:
			return (position - destination).l2NormSquared() <= Square (srcVehicle->data.getRange());
		default:
			return true;
	}
	return false;
}

int cPathDestHandler::heuristicCost (const cPosition& source) const
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

cPathCalculator::cPathCalculator(const cVehicle& Vehicle, const cMap& Map, const cPosition& destPosition, const std::vector<cVehicle*>* group)
{
	destHandler = new cPathDestHandler (PATH_DEST_TYPE_POS, destPosition, nullptr, nullptr);
	init (Vehicle.getPosition(), Map, Vehicle, group);
}

cPathCalculator::cPathCalculator(const cVehicle& Vehicle, const cMap& Map, const cUnit& destUnit, bool load)
{
	destHandler = new cPathDestHandler (load ? PATH_DEST_TYPE_LOAD : PATH_DEST_TYPE_ATTACK, cPosition (0, 0), &Vehicle, &destUnit);
	init (Vehicle.getPosition(), Map, Vehicle, nullptr);
}

cPathCalculator::cPathCalculator(const cVehicle& Vehicle, const cMap& Map, const cPosition& destPosition, bool attack)
{
	destHandler = new cPathDestHandler (attack ? PATH_DEST_TYPE_ATTACK : PATH_DEST_TYPE_POS, destPosition, &Vehicle, nullptr);
	init (Vehicle.getPosition(), Map, Vehicle, nullptr);
}

void cPathCalculator::init (const cPosition& source, const cMap& Map, const cVehicle& Vehicle, const std::vector<cVehicle*>* group)
{
	this->source = source;
	this->Map = &Map;
	this->Vehicle = &Vehicle;
	this->group = group;
	bPlane = Vehicle.getStaticUnitData().factorAir > 0;
	bShip = Vehicle.getStaticUnitData().factorSea > 0 && Vehicle.getStaticUnitData().factorGround == 0;

	MemBlocks = nullptr;

	blocknum = 0;
	blocksize = 0;
	heapCount = 0;
}

cPathCalculator::~cPathCalculator()
{
	delete destHandler;
	if (MemBlocks != nullptr)
	{
		for (int i = 0; i < blocknum; i++)
		{
			delete[] MemBlocks[i];
		}
		free (MemBlocks);
	}
}

std::forward_list<cPosition> cPathCalculator::calcPath()
{
	std::forward_list<cPosition> path;

	// generate open and closed list
	nodesHeap.resize (Map->getSize().x() * Map->getSize().y() + 1, nullptr);
	openList.resize (Map->getSize().x() * Map->getSize().y() + 1, nullptr);
	closedList.resize (Map->getSize().x() * Map->getSize().y() + 1, nullptr);

	// generate startnode
	sPathNode* StartNode = allocNode();
	StartNode->position = source;
	StartNode->costG = 0;
	StartNode->costH = destHandler->heuristicCost (source);
	StartNode->costF = StartNode->costG + StartNode->costH;

	StartNode->prev = nullptr;
	openList[Map->getOffset (source)] = StartNode;
	insertToHeap (StartNode, false);

	while (heapCount > 0)
	{
		// get the node with the lowest F value
		sPathNode* CurrentNode = nodesHeap[1];

		// move it from the open to the closed list
		openList[Map->getOffset (CurrentNode->position)] = nullptr;
		closedList[Map->getOffset (CurrentNode->position)] = CurrentNode;
		deleteFirstFromHeap();

		// generate waypoints when destination has been reached
		if (destHandler->hasReachedDestination (CurrentNode->position))
		{

			sPathNode* pathNode = CurrentNode;
			
			cPosition waypoint;
			while (pathNode->prev != nullptr)
			{
				waypoint = pathNode->position;
				path.push_front(waypoint);

				pathNode = pathNode->prev;
			}

			return path;
		}

		// expand node
		expandNodes (CurrentNode);
	}

	// there is no path to the destination field
	return path;
}

void cPathCalculator::expandNodes (sPathNode* ParentNode)
{
	// add all nearby nodes
	const int minx = std::max (ParentNode->position.x() - 1, 0);
	const int maxx = std::min (ParentNode->position.x() + 1, Map->getSize().y() - 1);
	const int miny = std::max (ParentNode->position.y() - 1, 0);
	const int maxy = std::min (ParentNode->position.y() + 1, Map->getSize().y() - 1);

	for (int y = miny; y <= maxy; ++y)
	{
		for (int x = minx; x <= maxx; ++x)
		{
			const cPosition currentPosition (x, y);
			if (currentPosition == ParentNode->position) continue;

			if (!Map->possiblePlace (*Vehicle, currentPosition, true))
			{
				// when we have a group of units, the units will not block each other
				if (group)
				{
					// get the blocking unit
					cVehicle* blockingUnit;
					if (Vehicle->getStaticUnitData().factorAir > 0) blockingUnit = Map->getField(currentPosition).getPlane();
					else blockingUnit = Map->getField (currentPosition).getVehicle();
					// check whether the blocking unit is the group
					bool isInGroup = Contains (*group, blockingUnit);
					if (!isInGroup) continue;
				}
				else continue;
			}
			if (closedList[Map->getOffset (currentPosition)] != nullptr) continue;

			if (openList[Map->getOffset (currentPosition)] == nullptr)
			{
				// generate new node
				sPathNode* NewNode = allocNode();
				NewNode->position = currentPosition;
				NewNode->costG = calcNextCost (ParentNode->position, currentPosition, Vehicle, Map) + ParentNode->costG;
				NewNode->costH = destHandler->heuristicCost (currentPosition);
				NewNode->costF = NewNode->costG + NewNode->costH;
				NewNode->prev = ParentNode;
				openList[Map->getOffset (currentPosition)] = NewNode;
				insertToHeap (NewNode, false);
			}
			else
			{
				// modify existing node
				int costG, costH, costF;
				costG = calcNextCost (ParentNode->position, currentPosition, Vehicle, Map) + ParentNode->costG;
				costH = destHandler->heuristicCost (currentPosition);
				costF = costG + costH;
				if (costF < openList[Map->getOffset (currentPosition)]->costF)
				{
					openList[Map->getOffset (currentPosition)]->costG = costG;
					openList[Map->getOffset (currentPosition)]->costH = costH;
					openList[Map->getOffset (currentPosition)]->costF = costF;
					openList[Map->getOffset (currentPosition)]->prev = ParentNode;
					insertToHeap (openList[Map->getOffset (currentPosition)], true);
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
	nodesHeap[heapCount] = nullptr;
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

int cPathCalculator::calcNextCost(const cPosition& source, const cPosition& destination, const cVehicle* vehicle, const cMap* map)
{
	int costs;
	// first we check whether the unit can fly
	if (vehicle->getStaticUnitData().factorAir > 0)
	{
		if (source.x() != destination.x() && source.y() != destination.y()) return (int) (4 * 1.5f * vehicle->getStaticUnitData().factorAir);
		else return (int) (4 * vehicle->getStaticUnitData().factorAir);
	}
	const cBuilding* building = map->getField (destination).getBaseBuilding();
	// moving on water will cost more
	if (map->isWater (destination) && (!building || (building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_BENEATH_SEA || building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE)) && vehicle->getStaticUnitData().factorSea > 0) costs = (int) (4 * vehicle->getStaticUnitData().factorSea);
	else if (map->isCoast (destination) && !building && vehicle->getStaticUnitData().factorCoast > 0) costs = (int) (4 * vehicle->getStaticUnitData().factorCoast);
	else if (vehicle->getStaticUnitData().factorGround > 0) costs = (int)(4 * vehicle->getStaticUnitData().factorGround);
	else
	{
		Log.write ("Where can this unit move? " + iToStr (vehicle->iID), cLog::eLOG_TYPE_NET_WARNING);
		costs = 4;
	}
	// moving on a road is cheaper
	if (building && building->getStaticUnitData().modifiesSpeed != 0) costs = (int)(costs * building->getStaticUnitData().modifiesSpeed);

	// multiplicate with the factor 1.5 for diagonal movements
	if (source.x() != destination.x() && source.y() != destination.y()) costs = (int) (costs * 1.5f);
	return costs;
}

