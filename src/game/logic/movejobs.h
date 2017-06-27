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

#ifndef game_logic_movejobsH
#define game_logic_movejobsH

#include <SDL.h>
#include <vector>

#include "utility/signal/signal.h"
#include "utility/position.h"
#include "pathcalculator.h"

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
	cServerMoveJob (cServer& server, const cPosition& source, const cPosition& destination, cVehicle* Vehicle);
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
	static sWaypoint* calcPath (const cMap& map, const cPosition& source, const cPosition& destination, const cVehicle& vehicle, const std::vector<cVehicle*>* group = nullptr);

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

	sWaypoint* Waypoints;

	bool generateFromMessage (cNetMessage& message);

	void release();
	void handleNextMove (int iType, int iSavedSpeed);
	void moveVehicle();
	void doEndMoveVehicle();
	void calcNextDir();
	void drawArrow (SDL_Rect Dest, SDL_Rect* LastDest, bool bSpezial) const;

	// TODO: check when this signal get triggered
	mutable cSignal<void (const cVehicle&)> activated;
	mutable cSignal<void (const cVehicle&)> stopped;
	mutable cSignal<void (const cVehicle&)> moved;
	mutable cSignal<void (const cVehicle&)> blocked;
};

#endif // game_logic_movejobsH
