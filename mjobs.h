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
#ifndef mjobsH
#define mjobsH
#include "map.h"
#include "vehicles.h"

enum MJOB_TYPES
{
	MJOB_OK,
	MJOB_STOP,
	MJOB_BLOCKED
};

// structures for the calculation of the path /////////////////////////////////
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

// The MJob-class /////////////////////////////////////////////////////////////
class cMJobs
{
public:
	cMJobs(cMap *Map, int ScrOff, int DestOff, bool Plane, int iVehicleID, cList<cPlayer*> *PlayerList, bool bServerCall );
	~cMJobs(void);

	cMap *map;
	int ScrX,ScrY,DestX,DestY;
	cMJobs *next;
	bool finished;
	bool EndForNow;
	bool ClientMove;
	cVehicle *vehicle;
	int next_dir;
	int SavedSpeed;
	bool Suspended;
	bool plane,ship;
	bool BuildAtTarget;
	bool bIsServerJob;

	char *PathCalcMap;
	sPathCalc *PathCalcRoot;
	cList<sPathCalc*> *PathCalcEnds;
	cList<sPathCalc*> *PathCalcAll;
	sPathCalc *FoundEnd;

	sWaypoint *waypoints;

	void release();
	bool CalcPath(void);
	int CalcDest(int x,int y);
	bool AddPoint(int x, int y, float m, sPathCalc *p);
	bool CreateNextPath(void);
	bool CheckPossiblePoint(int x,int y);
	bool CheckPointNotBlocked(int x,int y);
	int GetWayCost(int x,int y,bool *road);
	void DeleteWaypoints(void);
	void DrawPfeil(SDL_Rect dest,SDL_Rect *ldest,bool spezial);
	void CalcNextDir(void);
	void StartMove(void);
	void DoTheMove(void);
};

#endif
