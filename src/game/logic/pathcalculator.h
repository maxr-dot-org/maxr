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
class cMap;

// structures for the calculation of the path
struct sWaypoint
{
	sWaypoint() :
		costs(0)
	{}

	cPosition position;
	int costs;

	template <typename T>
	void serialize(T& archive)
	{
		archive & position;
		archive & costs;
	}
};

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
	void init (const cPosition& source, const cMap& Map, const cVehicle& Vehicle, const std::vector<cVehicle*>* group);

public:
	cPathCalculator (const cVehicle& Vehicle, const cMap& Map, const cPosition& destPosition, const std::vector<cVehicle*>* group);
	cPathCalculator (const cVehicle& Vehicle, const cMap& Map, const cUnit& destUnit,  bool load);
	cPathCalculator (const cVehicle& Vehicle, const cMap& Map, const cPosition& destPosition, bool attack);
	~cPathCalculator();

	/**
	* calculates the best path in costs and length
	*@author alzi alias DoctorDeath
	*/
	std::forward_list<sWaypoint> calcPath();

	/**
	* calculates the costs for moving from the source- to the destinationfield
	*@author alzi alias DoctorDeath
	*/
	int calcNextCost (const cPosition& source, const cPosition& destination) const;

	/* the map on which the path will be calculated */
	const cMap* Map;
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

#endif //game_logic_pathcalculatorH