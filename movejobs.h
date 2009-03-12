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
#ifndef movejobsH
#define movejobsH

#include "map.h"

/* Size of a memory block while pathfinding */
#define MEM_BLOCK_SIZE 10

enum MJOB_TYPES
{
	MJOB_OK,
	MJOB_STOP,
	MJOB_FINISHED,
	MJOB_BLOCKED
};

// structures for the calculation of the path
struct sPathCalc
{
	sPathCalc *prev;
	int X,Y;
	int WayCosts;
	int CostsGes;
	bool road;
};

struct sWaypoint
{
	sWaypoint *next;
	int X,Y;
	int Costs;
};

/* node structure for pathfinding */
struct sPathNode
{
	/* x and y coords */
	int x, y;
	/* the difrent cost types */
	int costF, costG, costH;
	/* previous and next node of this one in the hole path */
	sPathNode *prev, *next;

	/* the costs to enter on this field */
	int fieldCosts;
};

class cPathCalculator
{
public:
	cPathCalculator( int ScrX, int ScrY, int DestX, int DestY, cMap *Map, cVehicle *Vehicle );
	~cPathCalculator();

	/**
	* calculates the best path in costs and length
	*@author alzi alias DoctorDeath
	*/
	sWaypoint* calcPath();
	/**
	* calculates the costs for moving from the source- to the destinationfield
	*@author alzi alias DoctorDeath
	*/
	int calcNextCost( int srcX, int srcY, int destX, int destY );

	/* the map on which the path will be calculated */
	cMap *Map;
	/* the moving vehicle */
	cVehicle *Vehicle;
	/* source and destination coords */
	int ScrX, ScrY, DestX, DestY;
	bool bPlane, bShip;


private:
	/* the waypoints of the found path*/
	sWaypoint *Waypoints;
	/* memoryblocks for the nodes */
	sPathNode **MemBlocks;
	/* number of blocks */
	int blocknum;
	/* restsize of the last block */
	int blocksize;

	/* heaplist where all nodes are sortet by there costF value */
	sPathNode **nodesHeap;
	/* open nodes map */
	sPathNode **openList;
	/* closed nodes map */
	sPathNode **closedList;
	/* number of nodes saved on the heaplist; equal to number of nodes in the openlist */
	int heapCount;
	/**
	* calculates the heuristic costs from the sourcefield to the total path destination
	*@author alzi alias DoctorDeath
	*/
	int heuristicCost ( int srcX, int srcY );
	/**
	* expands the nodes around the overgiven one
	*@author alzi alias DoctorDeath
	*/
	void expandNodes ( sPathNode *Node );
	/**
	* returns a pointer to allocated memory and allocets a new block in memory if necessary
	*@author alzi alias DoctorDeath
	*/
	sPathNode *allocNode();
	/**
	* inserts a node into the heaplist and sets it to the right position by its costF value.
	*@author alzi alias DoctorDeath
	*/
	void insertToHeap( sPathNode *Node, bool exists );
	/**
	* deletes the first node in the heaplist and resorts the rest.
	*@author alzi alias DoctorDeath
	*/
	void deleteFirstFromHeap();
};

class cServerMoveJob
{
public:
	cServerMoveJob ( int iSrcOff, int iDestOff, bool bPlane, cVehicle *Vehicle );
	~cServerMoveJob ();

	cMap *Map;
	cVehicle *Vehicle;

	int ScrX, ScrY;
	int DestX, DestY;
	bool bFinished;
	bool bEndForNow;
	int iNextDir;
	int iSavedSpeed;
	bool bPlane;

	sWaypoint *Waypoints;

	bool generateFromMessage( cNetMessage *message );
	bool calcPath();
	void release();
	bool checkMove();
	void sideStepStealthUnit( int PosX, int PosY );
	void moveVehicle();
	void doEndMoveVehicle();
	void calcNextDir();
};

class cClientMoveJob
{
public:
	cClientMoveJob ( int iSrcOff, int iDestOff, bool bPlane, cVehicle *Vehicle );
	~cClientMoveJob ();
	cMap *Map;
	cVehicle *Vehicle;

	int ScrX, ScrY;
	int DestX, DestY;
	bool bFinished;
	bool bEndForNow;
	bool bSuspended;
	int iNextDir;
	int iSavedSpeed;
	bool bPlane;
	bool bSoundRunning;

	sWaypoint *Waypoints;
	sWaypoint *lastWaypoints;

	void setVehicleToCoords(int x, int y);
	bool generateFromMessage( cNetMessage *message );
	bool calcPath();
	void release();
	void handleNextMove( int iServerPositionX, int iServerPositionY, int iType, int iSavedSpeed );
	void moveVehicle();
	void doEndMoveVehicle ();
	void calcNextDir ();
	void drawArrow ( SDL_Rect Dest, SDL_Rect *LastDest, bool bSpezial );
	void startMoveSound();
	void stopMoveSound();
};

#endif // movejobsH
