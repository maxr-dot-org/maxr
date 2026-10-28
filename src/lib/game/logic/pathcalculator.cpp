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

#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/units/unit.h"
#include "game/data/units/unitdata.h"
#include "game/data/units/vehicle.h"
#include "utility/mathtools.h"
#include "utility/narrow_cast.h"
#include "utility/ranges.h"

#include <cassert>
#include <forward_list>

/* Size of a memory block while pathfinding */
#define MEM_BLOCK_SIZE 10

//------------------------------------------------------------------------------
cPathDestHandler::cPathDestHandler (ePathDestinationType type_, const cPosition& destination_, const cVehicle* srcVehicle_, const cUnit* destUnit_) :
	type (type_),
	srcVehicle (srcVehicle_),
	destUnit (destUnit_),
	destination (destination_)
{}

//------------------------------------------------------------------------------
bool cPathDestHandler::hasReachedDestination (const cPosition& position) const
{
	switch (type)
	{
		case ePathDestinationType::Pos:
			return (destination == position);
		case ePathDestinationType::Load:
			return (destUnit && destUnit->isNextTo (position));
		case ePathDestinationType::Attack:
			return (position - destination).l2NormSquared() <= Square (srcVehicle->data.getRange());
		default:
			return true;
	}
}

//------------------------------------------------------------------------------
std::optional<cPosition> cPathDestHandler::getExactDestination() const
{
	if (type == ePathDestinationType::Pos) return destination;
	return std::nullopt;
}

//------------------------------------------------------------------------------
int cPathDestHandler::heuristicCost (const cPosition& source) const
{
	switch (type)
	{
		case ePathDestinationType::Pos:
		case ePathDestinationType::Load:
			return 0;
		case ePathDestinationType::Attack:
		default:
		{
			return Round (narrow_cast<float> ((destination - source).l2Norm()));
		}
	}
}

//------------------------------------------------------------------------------
cPathCalculator::cPathCalculator (const cVehicle& Vehicle, const cMapView& Map, const cPosition& destPosition, const std::vector<cVehicle*>* group)
{
	destHandler = std::make_unique<cPathDestHandler> (ePathDestinationType::Pos, destPosition, nullptr, nullptr);
	init (Vehicle.getPosition(), Map, Vehicle, group);
}

//------------------------------------------------------------------------------
cPathCalculator::cPathCalculator (const cVehicle& Vehicle, const cMapView& Map, const cUnit& destUnit, bool load)
{
	destHandler = std::make_unique<cPathDestHandler> (load ? ePathDestinationType::Load : ePathDestinationType::Attack, cPosition (0, 0), &Vehicle, &destUnit);
	init (Vehicle.getPosition(), Map, Vehicle, nullptr);
}

//------------------------------------------------------------------------------
cPathCalculator::cPathCalculator (const cVehicle& Vehicle, const cMapView& Map, const cPosition& destPosition, bool attack)
{
	destHandler = std::make_unique<cPathDestHandler> (attack ? ePathDestinationType::Attack : ePathDestinationType::Pos, destPosition, &Vehicle, nullptr);
	init (Vehicle.getPosition(), Map, Vehicle, nullptr);
}

//------------------------------------------------------------------------------
void cPathCalculator::init (const cPosition& source, const cMapView& Map, const cVehicle& Vehicle, const std::vector<cVehicle*>* group)
{
	this->source = source;
	this->Map = &Map;
	this->Vehicle = &Vehicle;
	this->group = group;
	bPlane = Vehicle.getStaticUnitData().factorAir > 0;
	bShip = Vehicle.getStaticUnitData().factorSea > 0 && Vehicle.getStaticUnitData().factorGround == 0;

	MemBlocks.clear();

	blocknum = 0;
	blocksize = 0;
	heapCount = 0;
}

//------------------------------------------------------------------------------
cPathCalculator::~cPathCalculator()
{
}

//------------------------------------------------------------------------------
std::forward_list<cPosition> cPathCalculator::calcPath()
{
	std::forward_list<cPosition> path;

	// a field, that no node can be placed on, is never reached:
	// the search would visit the whole reachable area and find nothing
	if (const auto destination = destHandler->getExactDestination())
	{
		if (*destination != source && (!Map->isValidPosition (*destination) || !isPassable (*destination)))
		{
			return path;
		}
	}

	// generate open and closed list
	nodesHeap.resize (Map->getSize().x() * Map->getSize().y() + 1, nullptr);
	openList.resize (Map->getSize().x() * Map->getSize().y() + 1, nullptr);
	closedList.resize (Map->getSize().x() * Map->getSize().y() + 1, nullptr);
	blockedList.resize (Map->getSize().x() * Map->getSize().y() + 1, false);
	fieldCostList.resize (Map->getSize().x() * Map->getSize().y() + 1, -1);

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
				path.push_front (waypoint);

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

//------------------------------------------------------------------------------
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
			if (!Map->isValidPosition (currentPosition)) continue;

			// The map does not change during the search and isPassable() has no
			// side effects: an open or closed field has already passed the check
			// (the start field is closed first), and a field, that failed it,
			// fails again. So each field is checked only once per search.
			const int offset = Map->getOffset (currentPosition);
			if (closedList[offset] != nullptr) continue;

			if (openList[offset] == nullptr)
			{
				if (blockedList[offset]) continue;
				if (!isPassable (currentPosition))
				{
					blockedList[offset] = true;
					continue;
				}

				// generate new node
				sPathNode* NewNode = allocNode();
				NewNode->position = currentPosition;
				NewNode->costG = applyDiagonalCost (ParentNode->position, currentPosition, getFieldCost (currentPosition, offset)) + ParentNode->costG;
				NewNode->costH = destHandler->heuristicCost (currentPosition);
				NewNode->costF = NewNode->costG + NewNode->costH;
				NewNode->prev = ParentNode;
				openList[offset] = NewNode;
				insertToHeap (NewNode, false);
			}
			else
			{
				// modify existing node
				int costG, costH, costF;
				costG = applyDiagonalCost (ParentNode->position, currentPosition, getFieldCost (currentPosition, offset)) + ParentNode->costG;
				costH = destHandler->heuristicCost (currentPosition);
				costF = costG + costH;
				if (costF < openList[offset]->costF)
				{
					openList[offset]->costG = costG;
					openList[offset]->costH = costH;
					openList[offset]->costF = costF;
					openList[offset]->prev = ParentNode;
					insertToHeap (openList[offset], true);
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
bool cPathCalculator::isPassable (const cPosition& position) const
{
	if (Map->possiblePlace (*Vehicle, position)) return true;

	// when we have a group of units, the units will not block each other
	if (!group) return false;

	const auto& field = Map->getField (position);
	// get the blocking unit
	cVehicle* blockingUnit = (Vehicle->getStaticUnitData().factorAir > 0) ? field.getPlane() : field.getVehicle();
	// check whether the blocking unit is the group
	return ranges::contains (*group, blockingUnit);
}

//------------------------------------------------------------------------------
int cPathCalculator::getFieldCost (const cPosition& position, int offset)
{
	// the map does not change during the search: the costs of a field are the same from every side
	if (fieldCostList[offset] < 0)
	{
		fieldCostList[offset] = calcFieldCost (position, Vehicle, Map);
	}
	return fieldCostList[offset];
}

//------------------------------------------------------------------------------
void cPathCalculator::swapHeapNodes (int a, int b)
{
	std::swap (nodesHeap[a], nodesHeap[b]);
	nodesHeap[a]->heapIndex = a;
	nodesHeap[b]->heapIndex = b;
}

//------------------------------------------------------------------------------
sPathNode* cPathCalculator::allocNode()
{
	// allocate new memory block if necessary
	if (blocksize <= 0)
	{
		MemBlocks.emplace_back (std::vector<sPathNode> (10));
		blocksize = MEM_BLOCK_SIZE;
		blocknum++;
	}
	blocksize--;
	return &MemBlocks[blocknum - 1][blocksize];
}

//------------------------------------------------------------------------------
void cPathCalculator::insertToHeap (sPathNode* Node, bool exists)
{
	int i = 0;
	if (exists)
	{
		// the position of the existing node (no search through the heap)
		i = Node->heapIndex;
		assert (i >= 1 && i <= heapCount);
	}
	else
	{
		// add the new node in the end
		heapCount++;
		nodesHeap[heapCount] = Node;
		Node->heapIndex = heapCount;
		i = heapCount;
	}
	// resort the nodes
	while (i > 1)
	{
		assert (nodesHeap[i] == Node);
		if (Node->costF < nodesHeap[i / 2]->costF)
		{
			swapHeapNodes (i / 2, i);
			i = i / 2;
		}
		else
			break;
	}
}

//------------------------------------------------------------------------------
void cPathCalculator::deleteFirstFromHeap()
{
	// overwrite the first node by the last one
	nodesHeap[1] = nodesHeap[heapCount];
	nodesHeap[1]->heapIndex = 1;
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
			swapHeapNodes (u, v);
		}
		else
			break;
	}
}
