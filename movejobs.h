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

#include <SDL.h>
#include <vector>

#include "utility/signal/signal.h"
#include "utility/position.h"

class cClient;
class cMap;
class cNetMessage;
class cServer;
class cUnit;
class cVehicle;

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
struct sWaypoint
{
	sWaypoint* next;
	cPosition position;
	int Costs;
};

/* node structure for pathfinding */
struct sPathNode
{
	/* x and y coords */
	cPosition position;
	/* the difrent cost types */
	int costF, costG, costH;
	/* previous and next node of this one in the hole path */
	sPathNode* prev, *next;

	/* the costs to enter on this field */
	int fieldCosts;
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
	cPathDestHandler (ePathDestinationTypes type_, const cPosition& destination, const cVehicle* srcVehicle_, const cUnit* destUnit_);

	bool hasReachedDestination(const cPosition& position) const;
	int heuristicCost(const cPosition& source) const;
};

class cPathCalculator
{
	void init(const cPosition& source, const cMap& Map, const cVehicle& Vehicle, const std::vector<cVehicle*>* group);

public:
	cPathCalculator(const cPosition& source, const cPosition& destination, const cMap& Map, const cVehicle& Vehicle, const std::vector<cVehicle*>* group = NULL);
	cPathCalculator(const cPosition& source, const cUnit& destUnit, const cMap& Map, const cVehicle& Vehicle, bool load);
	cPathCalculator(const cPosition& source, const cMap& Map, const cVehicle& Vehicle, const cPosition& attack);
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
	int calcNextCost(const cPosition& source, const cPosition& destination) const;

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
	/* the waypoints of the found path*/
	sWaypoint* Waypoints;
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


enum eEndMoveActionType
{
	EMAT_LOAD,
	EMAT_GET_IN,
	EMAT_ATTACK
};

class cEndMoveAction
{
public:
	cVehicle* vehicle_;
	eEndMoveActionType type_;
	int destID_;		//we store the ID and not a pointer to vehicle/building,
	//so we don't have to invalidate the pointer, when the dest unit gets destroyed

private:
	void executeLoadAction (cServer& server);
	void executeGetInAction (cServer& server);
	void executeAttackAction (cServer& server);

public:
	cEndMoveAction (cVehicle* vehicle, int destID, eEndMoveActionType type);

	void execute (cServer& server);
};

class cServerMoveJob
{
	cServer* server;
public:
	cServerMoveJob(cServer& server, const cPosition& source, const cPosition& destination, cVehicle* Vehicle);
	~cServerMoveJob();

	cMap* Map;
	cVehicle* Vehicle;

	cPosition source;
	cPosition destination;
	bool bFinished;
	bool bEndForNow;
	int iNextDir;
	int iSavedSpeed;
	bool bPlane;
	cEndMoveAction* endAction;

	sWaypoint* Waypoints;

	static cServerMoveJob* generateFromMessage (cServer& server, cNetMessage& message);

	bool calcPath();
	void release();
	bool checkMove();
	void moveVehicle();
	void doEndMoveVehicle();
	void calcNextDir();
	void stop();
	void resume();
	void addEndAction (int destID, eEndMoveActionType type);
};

class cClientMoveJob
{
	cClient* client;

	void init (const cPosition& source, cVehicle* Vehicle);
public:
	static sWaypoint* calcPath(const cMap& map, const cPosition& source, const cPosition& destination, const cVehicle& vehicle, const std::vector<cVehicle*>* group = NULL);

	cClientMoveJob (cClient& client_, const cPosition& source, const cPosition& destination, cVehicle* Vehicle);
	~cClientMoveJob();
	cMap* Map;
	cVehicle* Vehicle;
	cEndMoveAction* endMoveAction;

	cPosition source;
	cPosition destination;
	bool bFinished;
	bool bEndForNow;
	bool bSuspended;
	int iNextDir;
	int iSavedSpeed;
	bool bPlane;
	bool bSoundRunning;

	sWaypoint* Waypoints;

	bool generateFromMessage (cNetMessage& message);

	void release();
	void handleNextMove (int iType, int iSavedSpeed);
	void moveVehicle();
	void doEndMoveVehicle();
	void calcNextDir();
	void drawArrow (SDL_Rect Dest, SDL_Rect* LastDest, bool bSpezial);

	// TODO: check when this signal get triggered
	mutable cSignal<void (const cVehicle&)> activated;
	mutable cSignal<void (const cVehicle&)> stopped;
	mutable cSignal<void (const cVehicle&)> moved;
	mutable cSignal<void (const cVehicle&)> blocked;
};

#endif // movejobsH
