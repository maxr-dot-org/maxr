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

#include "mjobs.h"	// for MJOB_TYPES, sPathCalc and sWaypoint

class cPathCalculator
{
public:
	cPathCalculator( int ScrX, int ScrY, int DestX, int DestY, cMap *Map, cVehicle *Vehicle );

	cMap *Map;
	cVehicle *Vehicle;
	int DestX, DestY;
	bool bPlane, bShip;

	sWaypoint *Waypoints;

	char *PathCalcMap;
	sPathCalc *PathCalcRoot;
	sPathCalc *FoundEnd;
	cList<sPathCalc*> PathCalcEnds;
	cList<sPathCalc*> PathCalcAll;

	void calcPath( int ScrX, int ScrY );
	int calcDest( int x, int y );
	bool addPoint( int x, int y, float m, sPathCalc *PathCalc );
	bool createNextPath();
	bool checkPossiblePoint( int x, int y );
	bool checkPointNotBlocked ( int x, int y );
	int getWayCost( int x, int y ,bool *road );
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
	bool bPlane, bShip;

	sWaypoint *Waypoints;

	bool generateFromMessage( cNetMessage *message );
	bool calcPath();
	void release();
	bool checkPointNotBlocked( int x, int y );
	bool checkMove();
	void moveVehicle();
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
	bool bPlane, bShip;

	sWaypoint *Waypoints;

	bool generateFromMessage( cNetMessage *message );
	bool calcPath();
	void release();
	void handleNextMove( int iNextDestX, int iNextDestY, int iType );
	void moveVehicle();
	void doEndMoveVehicle ();
	void calcNextDir ();
	void drawArrow ( SDL_Rect Dest, SDL_Rect *LastDest, bool bSpezial );
};

#endif // movejobsH
