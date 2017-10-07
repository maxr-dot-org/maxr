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

#ifndef game_logic_pathcalculatorH
#define game_logic_pathcalculatorH

#include "utility/position.h"
#include <forward_list>

class cVehicle;
class cUnit;
class cMapView;

/* node structure for pathfinding */
struct sPathNode
{
	/* x and y coords */
	cPosition position;
	/* the different cost types */
	int costF, costG, costH;
	/* previous node of this one in the hole path */
	sPathNode* prev;
};

enum ePathDestinationTypes
{
	PATH_DEST_TYPE_POS,
	PATH_DEST_TYPE_LOAD,
	PATH_DEST_TYPE_ATTACK
};

class cPathDestHandler
{
	ePathDestinationTypes type;

	const cVehicle* srcVehicle;

	const cUnit* destUnit;
	cPosition destination;
public:
	cPathDestHandler(ePathDestinationTypes type_, const cPosition& destination, const cVehicle* srcVehicle_, const cUnit* destUnit_);

	bool hasReachedDestination(const cPosition& position) const;
	int heuristicCost(const cPosition& source) const;
};


class cPathCalculator
{
	void init (const cPosition& source, const cMapView& Map, const cVehicle& Vehicle, const std::vector<cVehicle*>* group);

public:
	cPathCalculator (const cVehicle& Vehicle, const cMapView& Map, const cPosition& destPosition, const std::vector<cVehicle*>* group);
	cPathCalculator (const cVehicle& Vehicle, const cMapView& Map, const cUnit& destUnit,  bool load);
	cPathCalculator (const cVehicle& Vehicle, const cMapView& Map, const cPosition& destPosition, bool attack);
	~cPathCalculator();

	/**
	* calculates the best path in costs and length
	*@author alzi alias DoctorDeath
	*/
	std::forward_list<cPosition> calcPath();

	/**
	* calculates the costs for moving from the source- to the destinationfield
	*@author alzi alias DoctorDeath
	*/
	template<typename T>
	static int calcNextCost(const cPosition& source, const cPosition& destination, const cVehicle* vehicle, const T* map);

	/* the map on which the path will be calculated */
	const cMapView* Map;
	/* the moving vehicle */
	const cVehicle* Vehicle;
	/* if more then one vehicle is moving in a group this is the list of all moving vehicles */
	const std::vector<cVehicle*>* group;
	/* source and destination coords */
	cPosition source;
	bool bPlane, bShip;
	cPathDestHandler* destHandler;


private:
	/* memoryblocks for the nodes */
	sPathNode** MemBlocks;
	/* number of blocks */
	int blocknum;
	/* restsize of the last block */
	int blocksize;

	/* heaplist where all nodes are sorted by there costF value */
	std::vector<sPathNode*> nodesHeap;
	/* open nodes map */
	std::vector<sPathNode*> openList;
	/* closed nodes map */
	std::vector<sPathNode*> closedList;
	/* number of nodes saved on the heaplist; equal to number of nodes in the openlist */
	int heapCount;
	/**
	* expands the nodes around the overgiven one
	*@author alzi alias DoctorDeath
	*/
	void expandNodes (sPathNode* Node);
	/**
	* returns a pointer to allocated memory and allocets a new block in memory if necessary
	*@author alzi alias DoctorDeath
	*/
	sPathNode* allocNode();
	/**
	* inserts a node into the heaplist and sets it to the right position by its costF value.
	*@author alzi alias DoctorDeath
	*/
	void insertToHeap (sPathNode* Node, bool exists);
	/**
	* deletes the first node in the heaplist and resorts the rest.
	*@author alzi alias DoctorDeath
	*/
	void deleteFirstFromHeap();
};

template<typename T>
static int cPathCalculator::calcNextCost(const cPosition& source, const cPosition& destination, const cVehicle* vehicle, const T* map)
{
	static_assert(std::is_same<T, cMap>::value || std::is_same<T, cMapView>::value, "Type must be cMap or cMapView");

	int costs;
	// first we check whether the unit can fly
	if (vehicle->getStaticUnitData().factorAir > 0)
	{
		if (source.x() != destination.x() && source.y() != destination.y()) return (int)(4 * 1.5f * vehicle->getStaticUnitData().factorAir);
		else return (int)(4 * vehicle->getStaticUnitData().factorAir);
	}
	const cBuilding* building = map->getField(destination).getBaseBuilding();
	// moving on water will cost more
	if (map->isWater(destination) && (!building || (building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_BENEATH_SEA || building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE)) && vehicle->getStaticUnitData().factorSea > 0) costs = (int)(4 * vehicle->getStaticUnitData().factorSea);
	else if (map->isCoast(destination) && !building && vehicle->getStaticUnitData().factorCoast > 0) costs = (int)(4 * vehicle->getStaticUnitData().factorCoast);
	else if (vehicle->getStaticUnitData().factorGround > 0) costs = (int)(4 * vehicle->getStaticUnitData().factorGround);
	else
	{
		Log.write("Where can this unit move? " + iToStr(vehicle->iID), cLog::eLOG_TYPE_NET_WARNING);
		costs = 4;
	}
	// moving on a road is cheaper
	if (building && building->getStaticUnitData().modifiesSpeed != 0) costs = (int)(costs * building->getStaticUnitData().modifiesSpeed);

	// multiply with the factor 1.5 for diagonal movements
	if (source.x() != destination.x() && source.y() != destination.y()) costs = (int)(costs * 1.5f);
	return costs;
}

#endif //game_logic_pathcalculatorH