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

#include "movejobs.h"
#include "player.h"
#include "serverevents.h"
#include "clientevents.h"
#include "server.h"
#include "client.h"
#include "vehicles.h"

cPathCalculator::cPathCalculator( int ScrX, int ScrY, int DestX, int DestY, cMap *Map, cVehicle *Vehicle )
{
	this->DestX = DestX;
	this->DestY = DestY;
	this->Map = Map;
	this->Vehicle = Vehicle;
	Waypoints = NULL;
	bPlane = Vehicle->data.can_drive == DRIVE_AIR;
	bShip = Vehicle->data.can_drive == DRIVE_SEA;

	calcPath ( ScrX, ScrY );
}

void cPathCalculator::calcPath ( int ScrX, int ScrY )
{
	bool bReturn;
	PathCalcMap = ( char* ) malloc ( Map->size*Map->size );
	memset ( PathCalcMap, 0, Map->size*Map->size );
	FoundEnd=NULL;
	PathCalcRoot = new sPathCalc;
	PathCalcRoot->prev = NULL;
	PathCalcRoot->X = ScrX;
	PathCalcRoot->Y = ScrY;
	PathCalcRoot->WayCosts = 0;
	PathCalcRoot->CostsGes = PathCalcRoot->WayCosts + calcDest ( ScrX,ScrY );
	PathCalcMap[ScrX+ScrY*Map->size] = 1;
	PathCalcEnds.Add( PathCalcRoot );
	PathCalcAll.Add( PathCalcRoot );

	if ( ( bReturn = createNextPath() ) != false )
	{
		sWaypoint *Waypoint, *PrevWaypoint;
		PrevWaypoint = NULL;
		while ( FoundEnd )
		{
			Waypoint = ( sWaypoint* ) malloc ( sizeof ( sWaypoint ) );
			Waypoint->next = PrevWaypoint;
			Waypoint->Costs = FoundEnd->WayCosts;
			Waypoint->X = FoundEnd->X;
			Waypoint->Y = FoundEnd->Y;
			PrevWaypoint = Waypoint;
			FoundEnd = FoundEnd->prev;
		}
		Waypoints = Waypoint;
	}
	while ( PathCalcAll.Size() )
	{
		delete PathCalcAll[0];
		PathCalcAll.Delete( 0 );
	}
	free ( PathCalcMap );
	if ( bReturn == false )
	{
		sWaypoint *NextWaypoint;
		while ( Waypoints )
		{
			NextWaypoint = Waypoints->next;
			free ( Waypoints );
			Waypoints = NextWaypoint;
		}
		Waypoints = NULL;
	}
}

int cPathCalculator::calcDest ( int x, int y )
{
	return ( abs ( x-DestX ) + abs ( y-DestY ) );
}

bool cPathCalculator::addPoint ( int x, int y, float m, sPathCalc *PathCalc )
{
	sPathCalc *newPathCalc;
	if ( checkPossiblePoint ( x, y ) )
	{
		newPathCalc = new sPathCalc;
		newPathCalc->prev=  PathCalc;
		newPathCalc->X = x;
		newPathCalc->Y = y;
		newPathCalc->WayCosts = getWayCost ( x, y, &newPathCalc->road )*m;
		newPathCalc->CostsGes = newPathCalc->WayCosts+calcDest ( newPathCalc->X, newPathCalc->Y );
		PathCalcEnds.Add ( newPathCalc );
		PathCalcAll.Add ( newPathCalc );
		PathCalcMap[x+y*Map->size] = 1;
		if ( x == DestX && y == DestY )
		{
			FoundEnd = newPathCalc;
			return true;
		}
	}
	return false;
}

bool cPathCalculator::createNextPath ( )
{
	sPathCalc *PathCalc, *PathCalcEnd;
	int iIndex;

	if ( PathCalcEnds.Size() == 0 || PathCalcAll.Size() > MAX_PATHFINDING ) return false;

	PathCalc = PathCalcEnds[0];
	iIndex = 0;
	if ( !bPlane )
	{
		for ( unsigned int i = 0; i < PathCalcEnds.Size(); i++ )
		{
			PathCalcEnd = PathCalcEnds[i];
			if ( PathCalcEnd->road )
			{
				PathCalc = PathCalcEnd;
				iIndex = i;
				break;
			}
			if ( PathCalcEnd->CostsGes < ( PathCalc->CostsGes ) >> 1 )
			{
				PathCalc = PathCalcEnd;
				iIndex = i;
			}
		}
	}
	else
	{
		for ( unsigned int i = 0; i < PathCalcEnds.Size(); i++ )
		{
			PathCalcEnd = PathCalcEnds[i];
			if ( PathCalcEnd->CostsGes < ( PathCalc->CostsGes ) >> 1 )
			{
				PathCalc = PathCalcEnd;
				iIndex = i;
			}
		}
	}
	PathCalcEnds.Delete ( iIndex );
	if ( addPoint ( PathCalc->X, PathCalc->Y-1, 1, PathCalc ) ) return true;
	if ( addPoint ( PathCalc->X+1, PathCalc->Y, 1, PathCalc ) ) return true;
	if ( addPoint ( PathCalc->X, PathCalc->Y+1, 1, PathCalc ) ) return true;
	if ( addPoint ( PathCalc->X-1, PathCalc->Y,1, PathCalc ) ) return true;
	if ( addPoint ( PathCalc->X-1, PathCalc->Y-1, 1.5, PathCalc ) ) return true;
	if ( addPoint ( PathCalc->X+1, PathCalc->Y-1, 1.5, PathCalc ) ) return true;
	if ( addPoint ( PathCalc->X+1, PathCalc->Y+1, 1.5, PathCalc ) ) return true;
	if ( addPoint ( PathCalc->X-1, PathCalc->Y+1, 1.5, PathCalc ) ) return true;

	return createNextPath();
}

bool cPathCalculator::checkPossiblePoint ( int x, int y )
{
	if ( x < 0 || y < 0 || x >= Map->size || y >= Map->size ) return false;
	if ( PathCalcMap[x+y*Map->size] ) return false;
	if ( !bPlane )
	{
		if ( Map->terrain[Map->Kacheln[x+y*Map->size]].blocked ) return false;
		if ( Vehicle->data.can_drive == DRIVE_LAND && Map->IsWater ( x+y*Map->size ) && !( Map->GO[x+y*Map->size].base && ( Map->GO[x+y*Map->size].base->data.is_bridge || Map->GO[x+y*Map->size].base->data.is_platform || Map->GO[x+y*Map->size].base->data.is_road ) ) ) return false;
		if ( Vehicle->data.can_drive == DRIVE_SEA && (!Map->IsWater ( x+y*Map->size, true, true ) || ( Map->GO[x+y*Map->size].base && ( Map->GO[x+y*Map->size].base->data.is_platform || Map->GO[x+y*Map->size].base->data.is_road ) ) ) ) return false;
	}
	return checkPointNotBlocked ( x,y );
}

bool cPathCalculator::checkPointNotBlocked ( int x, int y )
{
	if ( !Vehicle->owner->ScanMap[x+y*Map->size] ) return true;
	if ( !bPlane && ( Map->GO[x+y*Map->size].vehicle || Map->GO[x+y*Map->size].reserviert ) ) return false;
	else if ( bPlane && ( Map->GO[x+y*Map->size].plane || Map->GO[x+y*Map->size].air_reserviert ) ) return false;
	if ( !bPlane && Map->GO[x+y*Map->size].top && !Map->GO[x+y*Map->size].top->data.is_connector ) return false;
	return true;
}

int cPathCalculator::getWayCost ( int x, int y, bool *bRoad )
{
	int iCosts, iPos;
	*bRoad = false;
	if ( bPlane || bShip ) return 4;
	iCosts = 4;
	iPos = x+y*Map->size;
	if ( Map->IsWater ( iPos ) && !( Map->GO[iPos].base && !Map->GO[iPos].base->data.is_expl_mine ) ) iCosts = 12;
	else if ( ( Map->GO[iPos].base&& ( Map->GO[iPos].base->data.is_road || Map->GO[iPos].base->data.is_bridge ) ) || ( Map->GO[iPos].subbase && Map->GO[iPos].subbase->data.is_road ) )
	{
		iCosts = 2;
		*bRoad = true;
	}
	if ( Vehicle->data.can_survey && iCosts > 4 ) iCosts = 4;
	return iCosts;
}

cServerMoveJob::cServerMoveJob ( int iSrcOff, int iDestOff, bool bPlane, cVehicle *Vehicle )
{
	if ( !Server ) return;

	Map = Server->Map;
	this->Vehicle = Vehicle;
	ScrX = iSrcOff%Map->size;
	ScrY = iSrcOff/Map->size;
	DestX = iDestOff%Map->size;
	DestY = iDestOff/Map->size;
	this->bPlane = bPlane;
	bShip = Vehicle->data.can_drive == DRIVE_SEA;
	bFinished = false;
	bEndForNow = false;
	iSavedSpeed = 0;

	// unset sentry status when moving vehicle
	if ( Vehicle->bSentryStatus )
	{
		Vehicle->owner->deleteSentryVehicle ( Vehicle );
		Vehicle->bSentryStatus = false;
	}
	sendUnitData ( Vehicle, Vehicle->owner->Nr );
	for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
	{
		sendUnitData ( Vehicle, *Vehicle->SeenByPlayerList[i] );
	}

	if ( Vehicle->ServerMoveJob )
	{
		iSavedSpeed = Vehicle->ServerMoveJob->iSavedSpeed;
		Vehicle->ServerMoveJob->release();
		Vehicle->moving = false;
		Vehicle->MoveJobActive = false;
	}
	Vehicle->ServerMoveJob = this;
}

cServerMoveJob::~cServerMoveJob()
{
	sWaypoint *NextWaypoint;
	while ( Waypoints )
	{
		NextWaypoint = Waypoints->next;
		free ( Waypoints );
		Waypoints = NextWaypoint;
	}
	Waypoints = NULL;
}

bool cServerMoveJob::generateFromMessage ( cNetMessage *message )
{
	if ( message->iType != GAME_EV_MOVE_JOB_CLIENT ) return false;
	int iCount = 0;
	int iWaypointOff;
	int iReceivedCount = message->popInt16();

	cLog::write(" Server: Received MoveJob: VehicleID: " + iToStr( Vehicle->iID ) + ", SrcX: " + iToStr( ScrX ) + ", SrcY: " + iToStr( ScrY ) + ", DestX: " + iToStr( DestX ) + ", DestY: " + iToStr( DestY ) + ", WaypointCount: " + iToStr( iReceivedCount ), cLog::eLOG_TYPE_NET_DEBUG);
	//if the vehicle is under attack, cancel the movejob
	if ( Vehicle->bIsBeeingAttacked )
	{
		cLog::write(" Server: cannot move a vehicle currently under attack", cLog::eLOG_TYPE_NET_DEBUG );
		sendNextMove( Vehicle->iID, ScrX+ScrY*Map->size, MJOB_FINISHED, Vehicle->owner->Nr );
		return false;
	}

	// Add the waypoints
	sWaypoint *Waypoint = ( sWaypoint* ) malloc ( sizeof ( sWaypoint ) );
	Waypoints = Waypoint;
	while ( iCount < iReceivedCount )
	{
		iWaypointOff = message->popInt32();
		Waypoint->X = iWaypointOff%Map->size;
		Waypoint->Y = iWaypointOff/Map->size;
		Waypoint->Costs = message->popInt16();
		Waypoint->next = NULL;

		iCount++;

		if ( iCount < iReceivedCount )
		{
			Waypoint->next = ( sWaypoint* ) malloc ( sizeof ( sWaypoint ) );
			Waypoint = Waypoint->next;
		}
	}
	calcNextDir ();

	//check whether the vehicle has to be hided
	//we check here the next waypoint of the vehicle, so other payers will not see in which direction the vehicle was driving
	if ( Waypoints && Waypoints->next && Vehicle->data.speed )
	{
		int offset = Waypoints->next->X + Waypoints->next->Y * Server->Map->size;
		for ( int i = 0; i < Vehicle->DetectedByPlayerList.Size(); i++ )
		{
			cPlayer* player = Vehicle->DetectedByPlayerList[i];
			if ( Vehicle->data.is_stealth_land && ( !player->DetectLandMap[offset] && !Server->Map->IsWater(offset) ))
			{
				Vehicle->resetDetectedByPlayer( player );
				i--;
			}
			else if ( Vehicle->data.is_stealth_sea && ( !player->DetectSeaMap[offset] && Server->Map->IsWater(offset) ))
			{
				Vehicle->resetDetectedByPlayer( player );
				i--;
			}
		}

		Server->checkPlayerUnits();
	}

	return true;
}

bool cServerMoveJob::calcPath()
{
	if ( ScrX == DestX && ScrY == DestY ) return false;

	cPathCalculator PathCalculator( ScrX, ScrY, DestX, DestY, Map, Vehicle );
	Waypoints = PathCalculator.Waypoints;
	if ( Waypoints )
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
	cLog::write ( " Server: Released old movejob", cLog::eLOG_TYPE_NET_DEBUG );
	for ( unsigned int i = 0; i < Server->ActiveMJobs.Size(); i++ )
	{
		if ( this == Server->ActiveMJobs[i] ) return;
	}
	Server->addActiveMoveJob ( this );
	cLog::write ( " Server: Added released movejob to avtive ones", cLog::eLOG_TYPE_NET_DEBUG );
}

bool cServerMoveJob::checkPointNotBlocked( int x, int y )
{
	if ( !Vehicle->owner->ScanMap[x+y*Map->size] ) return true;
	if ( !bPlane && ( Map->GO[x+y*Map->size].vehicle || Map->GO[x+y*Map->size].reserviert ) ) return false;
	else if ( bPlane && ( Map->GO[x+y*Map->size].plane || Map->GO[x+y*Map->size].air_reserviert ) ) return false;
	if ( !bPlane && Map->GO[x+y*Map->size].top && !Map->GO[x+y*Map->size].top->data.is_connector ) return false;
	return true;
}

bool cServerMoveJob::checkMove()
{
	bool bInSentryRange;
	if ( !Vehicle || !Waypoints || !Waypoints->next )
	{
		bFinished = true;
		return false;
	}

	bInSentryRange = Vehicle->InSentryRange();
	if ( !checkPointNotBlocked ( Waypoints->next->X, Waypoints->next->Y ) || bInSentryRange )
	{
		cLog::write( " Server: Next point is blocked: ID: " + iToStr ( Vehicle->iID ) + ", X: " + iToStr ( Waypoints->next->X ) + ", Y: " + iToStr ( Waypoints->next->Y ), LOG_TYPE_NET_DEBUG );
		// if the next point would be the last, finish the job here
		if ( Waypoints->next->X == DestX && Waypoints->next->Y == DestY )
		{
			bFinished = true;
		}
		// else delete the movejob and inform the client that he has to find a new path
		else
		{
			for ( int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
			{
				sendNextMove( Vehicle->iID, Vehicle->PosX+Vehicle->PosY*Map->size, MJOB_BLOCKED, *Vehicle->SeenByPlayerList[i]);
			}
			sendNextMove ( Vehicle->iID, Vehicle->PosX+Vehicle->PosY*Map->size, MJOB_BLOCKED, Vehicle->owner->Nr );
		}
		return false;
	}

	// not enough waypoints for this move
	if ( Vehicle->data.speed < Waypoints->next->Costs )
	{
		cLog::write( " Server: Vehicle has not enough waypoints for the next move -> EndForNow: ID: " + iToStr ( Vehicle->iID ) + ", X: " + iToStr ( Waypoints->next->X ) + ", Y: " + iToStr ( Waypoints->next->Y ), LOG_TYPE_NET_DEBUG );
		iSavedSpeed += Vehicle->data.speed;
		Vehicle->data.speed = 0;
		bEndForNow = true;
		return true;
	}

	Vehicle->MoveJobActive = true;
	Vehicle->moving = true;

	// reserv the next field
	if ( !bPlane ) Map->GO[Waypoints->next->X+Waypoints->next->Y*Map->size].reserviert = true;
	else Map->GO[Waypoints->next->X+Waypoints->next->Y*Map->size].air_reserviert = true;

	// send move command to all players who can see the unit
	for ( int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
	{
		sendNextMove( Vehicle->iID, Vehicle->PosX+Vehicle->PosY*Map->size, MJOB_OK, *Vehicle->SeenByPlayerList[i]);
	}
	sendNextMove ( Vehicle->iID, Vehicle->PosX+Vehicle->PosY*Map->size, MJOB_OK, Vehicle->owner->Nr );
	return true;
}

void cServerMoveJob::moveVehicle()
{
	int iSpeed;
	if ( !Vehicle ) return;
	if ( Vehicle->data.is_human )
	{
		Vehicle->WalkFrame++;
		if ( Vehicle->WalkFrame >= 13 ) Vehicle->WalkFrame = 0;
		iSpeed = MOVE_SPEED/2;
	}
	else if ( !(Vehicle->data.can_drive == DRIVE_AIR) && !(Vehicle->data.can_drive == DRIVE_SEA) )
	{
		iSpeed = MOVE_SPEED;
		if ( Waypoints && Waypoints->next && Map->GO[Waypoints->next->X+Waypoints->next->Y*Map->size].base&& ( Map->GO[Waypoints->next->X+Waypoints->next->Y*Map->size].base->data.is_road || Map->GO[Waypoints->next->X+Waypoints->next->Y*Map->size].base->data.is_bridge ) ) iSpeed *= 2;
	}
	else if ( Vehicle->data.can_drive == DRIVE_AIR ) iSpeed = MOVE_SPEED*2;
	else iSpeed = MOVE_SPEED;

	switch ( iNextDir )
	{
		case 0:
			Vehicle->OffY -= iSpeed;
			break;
		case 1:
			Vehicle->OffY -= iSpeed;
			Vehicle->OffX += iSpeed;
			break;
		case 2:
			Vehicle->OffX += iSpeed;
			break;
		case 3:
			Vehicle->OffX += iSpeed;
			Vehicle->OffY += iSpeed;
			break;
		case 4:
			Vehicle->OffY += iSpeed;
			break;
		case 5:
			Vehicle->OffX -= iSpeed;
			Vehicle->OffY += iSpeed;
			break;
		case 6:
			Vehicle->OffX -= iSpeed;
			break;
		case 7:
			Vehicle->OffX -= iSpeed;
			Vehicle->OffY -= iSpeed;
			break;
	}


	// check whether the point has been reached:
	if ( Vehicle->OffX >= 64 || Vehicle->OffY >= 64 || Vehicle->OffX <= -64 || Vehicle->OffY <= -64 )
	{
		cLog::write(" Server: Vehicle reached the next field: ID: " + iToStr ( Vehicle->iID )+ ", X: " + iToStr ( Waypoints->next->X ) + ", Y: " + iToStr ( Waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);
		sWaypoint *Waypoint;
		Waypoint = Waypoints->next;
		free ( Waypoints );
		Waypoints = Waypoint;

		if ( Vehicle->data.can_drive == DRIVE_AIR )
		{
			Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].plane = NULL;
			Map->GO[Waypoints->X+Waypoints->Y*Map->size].plane = Vehicle;
			Map->GO[Waypoints->X+Waypoints->Y*Map->size].air_reserviert = false;
		}
		else
		{
			Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].vehicle = NULL;
			Map->GO[Waypoints->X+Waypoints->Y*Map->size].vehicle = Vehicle;
			Map->GO[Waypoints->X+Waypoints->Y*Map->size].reserviert = false;
		}
		Vehicle->OffX = 0;
		Vehicle->OffY = 0;
		Vehicle->PosX = Waypoints->X;
		Vehicle->PosY = Waypoints->Y;

		if ( Waypoints->next == NULL )
		{
			bFinished = true;
		}

		Vehicle->data.speed += iSavedSpeed;
		iSavedSpeed = 0;
		Vehicle->DecSpeed ( Waypoints->Costs );

		// check for results of the move

		// make mines explode if necessary
		cBuilding* building = Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].base;
		if ( Vehicle->data.can_drive != DRIVE_AIR && building && building->data.is_expl_mine && building->owner != Vehicle->owner )
		{
			Server->AJobs.Add( new cServerAttackJob( building, Vehicle->PosX+Vehicle->PosY*Map->size ));
			bEndForNow = true;
		}

		// search for resources if necessary
		if ( Vehicle->data.can_survey )
		{
			sendResources( Vehicle, Map );
			Vehicle->doSurvey();
		}

		// let other units fire on this one
		Vehicle->InSentryRange();

		// search for mines if necessary
		if ( Vehicle->data.can_detect_mines )
		{
			Vehicle->detectMines();
		}

		// lay/clear mines if necessary
		if ( Vehicle->data.can_lay_mines )
		{
			bool bResult = false;
			if ( Vehicle->LayMines ) bResult = Vehicle->layMine();
			else if ( Vehicle->ClearMines ) bResult = Vehicle->clearMine();
			if ( bResult )
			{
				// send new unit values
				sendUnitData( Vehicle, Vehicle->owner->Nr );
				for ( int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
				{
					sendUnitData(Vehicle, *Vehicle->SeenByPlayerList[i]);
				}
			}
		}

		Vehicle->owner->DoScan();

		//hide vehicle
		while ( Vehicle->DetectedByPlayerList.Size() ) Vehicle->DetectedByPlayerList.Delete(0);

		//handle detection
		Vehicle->makeDetection();

		Vehicle->moving = false;
		calcNextDir();
	}
}

void cServerMoveJob::calcNextDir()
{
	if ( !Waypoints || !Waypoints->next ) return;
#define TESTXY_CND(a,b) if( Waypoints->X a Waypoints->next->X && Waypoints->Y b Waypoints->next->Y )
	TESTXY_CND ( ==,> ) iNextDir = 0;
	else TESTXY_CND ( <,> ) iNextDir = 1;
	else TESTXY_CND ( <,== ) iNextDir = 2;
	else TESTXY_CND ( <,< ) iNextDir = 3;
	else TESTXY_CND ( ==,< ) iNextDir = 4;
	else TESTXY_CND ( >,< ) iNextDir = 5;
	else TESTXY_CND ( >,== ) iNextDir = 6;
	else TESTXY_CND ( >,> ) iNextDir = 7;
}

cClientMoveJob::cClientMoveJob ( int iSrcOff, int iDestOff, bool bPlane, cVehicle *Vehicle )
{
	if ( !Client ) return;

	Map = Client->Map;
	this->Vehicle = Vehicle;
	ScrX = iSrcOff%Map->size;
	ScrY = iSrcOff/Map->size;
	DestX = iDestOff%Map->size;
	DestY = iDestOff/Map->size;
	this->bPlane = bPlane;
	bShip = Vehicle->data.can_drive == DRIVE_SEA;
	bFinished = false;
	bEndForNow = false;
	iSavedSpeed = 0;

	if ( Vehicle->ClientMoveJob )
	{
		Vehicle->ClientMoveJob->release();
		Vehicle->moving = false;
		Vehicle->MoveJobActive = false;
	}
	Vehicle->ClientMoveJob = this;
}

cClientMoveJob::~cClientMoveJob()
{
	sWaypoint *NextWaypoint;
	while ( Waypoints )
	{
		NextWaypoint = Waypoints->next;
		free ( Waypoints );
		Waypoints = NextWaypoint;
	}
	Waypoints = NULL;
}

bool cClientMoveJob::generateFromMessage( cNetMessage *message )
{
	if ( message->iType != GAME_EV_MOVE_JOB_SERVER ) return false;
	int iCount = 0;
	int iWaypointOff;
	int iReceivedCount = message->popInt16();

	cLog::write(" Client: Received MoveJob: VehicleID: " + iToStr( Vehicle->iID ) + ", SrcX: " + iToStr( ScrX ) + ", SrcY: " + iToStr( ScrY ) + ", DestX: " + iToStr( DestX ) + ", DestY: " + iToStr( DestY ) + ", WaypointCount: " + iToStr( iReceivedCount ), cLog::eLOG_TYPE_NET_DEBUG);

	// Add the waypoints
	sWaypoint *Waypoint = ( sWaypoint* ) malloc ( sizeof ( sWaypoint ) );
	Waypoints = Waypoint;
	while ( iCount < iReceivedCount )
	{
		iWaypointOff = message->popInt32();
		Waypoint->X = iWaypointOff%Map->size;
		Waypoint->Y = iWaypointOff/Map->size;
		Waypoint->Costs = message->popInt16();
		Waypoint->next = NULL;

		iCount++;

		if ( iCount < iReceivedCount )
		{
			Waypoint->next = ( sWaypoint* ) malloc ( sizeof ( sWaypoint ) );
			Waypoint = Waypoint->next;
		}
	}
	calcNextDir ();
	return true;
}

bool cClientMoveJob::calcPath()
{
	if ( ScrX == DestX && ScrY == DestY ) return false;

	cPathCalculator PathCalculator( ScrX, ScrY, DestX, DestY, Map, Vehicle );
	Waypoints = PathCalculator.Waypoints;
	if ( Waypoints )
	{
		calcNextDir();
		return true;
	}
	return false;
}

void cClientMoveJob::release()
{
	bEndForNow = false;
	bFinished = true;
	cLog::write ( " Client: Released old movejob", cLog::eLOG_TYPE_NET_DEBUG );
	for (int i = 0; i < Client->ActiveMJobs.Size(); i++)
	{
		if ( this == Client->ActiveMJobs[i] ) return;
	}
	Client->addActiveMoveJob ( this );
	cLog::write ( " Client: Added released movejob to avtive ones", cLog::eLOG_TYPE_NET_DEBUG );
}

void cClientMoveJob::handleNextMove( int iNextDestX, int iNextDestY, int iType )
{
	// set vehicle to server position
	if ( Vehicle->PosX != iNextDestX || Vehicle->PosY != iNextDestY )
	{
		// the client is faster then the server and has already
		// reached the last field or the next will be the last,
		// then stop the vehicle
		if ( Waypoints == NULL || Waypoints->next == NULL )
		{
			cLog::write ( " Client: Client has already reached the last field", cLog::eLOG_TYPE_NET_DEBUG );
			bFinished = true;
			Vehicle->OffX = Vehicle->OffY = 0;
			return;
		}
		else
		{
			// server is one field faster then client
			if ( iNextDestX == Waypoints->next->X && iNextDestY == Waypoints->next->Y )
			{
				cLog::write ( " Client: Server is one field faster then client", cLog::eLOG_TYPE_NET_DEBUG );
				doEndMoveVehicle();
			}
			else
			{
				// check whether the destination field is one of the next in the waypointlist
				// if not it must have been one that has been deleted already
				bool bServerIsFaster = false;
				sWaypoint *Waypoint = Waypoints->next->next;
				while ( Waypoint )
				{
					if ( Waypoint->X == iNextDestX && Waypoint->Y == iNextDestY )
					{
						bServerIsFaster = true;
						break;
					}
					Waypoint = Waypoint->next;
				}
				// the server is more then one field faster
				if ( bServerIsFaster )
				{
					cLog::write ( " Client: Server is more then one field faster", cLog::eLOG_TYPE_NET_DEBUG );
					if ( Vehicle->data.can_drive == DRIVE_AIR )
					{
						Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].plane = NULL;
						Map->GO[iNextDestX+iNextDestY*Map->size].plane = Vehicle;
					}
					else
					{
						Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].vehicle = NULL;
						Map->GO[iNextDestX+iNextDestY*Map->size].vehicle = Vehicle;
					}
					Vehicle->PosX = iNextDestX;
					Vehicle->PosY = iNextDestY;
					Vehicle->OffX = Vehicle->OffY = 0;

					Waypoint = Waypoints;
					while ( Waypoint )
					{
						if ( Waypoint->next->X != iNextDestX && Waypoint->next->Y != iNextDestY )
						{
							Waypoints = Waypoint->next;
							free ( Waypoint );
							Waypoint = Waypoints;
						}
						else
						{
							Waypoints = Waypoint;
							break;
						}
					}
				}
				// the client is faster
				else
				{
					cLog::write ( " Client: Client is faster (one or more fields) deactivating movejob; Vehicle-ID: " + iToStr ( Vehicle->iID ), cLog::eLOG_TYPE_NET_DEBUG );
					// just stop the vehicle and wait for the next commando of the server
					for ( unsigned int i = 0; i < Client->ActiveMJobs.Size(); i++ )
					{
						if ( Client->ActiveMJobs[i] == this ) Client->ActiveMJobs.Delete ( i );
					}
					Vehicle->MoveJobActive = false;
					bEndForNow = true;
					return;
				}
			}
		}
	}
	switch ( iType )
	{
	case MJOB_OK:
		{
			Vehicle->moving = true;
			if ( !Vehicle->MoveJobActive ) Vehicle->StartMoveSound();
			if ( bEndForNow && !Vehicle->MoveJobActive )
			{
				bEndForNow = false;
				Client->addActiveMoveJob ( Vehicle->ClientMoveJob );
				cLog::write ( " Client: reactivated movejob; Vehicle-ID: " + iToStr ( Vehicle->iID ), cLog::eLOG_TYPE_NET_DEBUG );
			}
			Vehicle->MoveJobActive = true;
			cLog::write(" Client: The movejob is ok: DestX: " + iToStr ( Waypoints->next->X ) + ", DestY: " + iToStr ( Waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);
		}
		break;
	case MJOB_STOP:
		{
			cLog::write(" Client: The movejob will end for now", cLog::eLOG_TYPE_NET_DEBUG);
			bSuspended = true;
			bEndForNow = true;
		}
		break;
	case MJOB_FINISHED:
		{
			cLog::write(" Client: The movejob is finished", cLog::eLOG_TYPE_NET_DEBUG);
			release ();
		}
		break;
	case MJOB_BLOCKED:
		{
			cLog::write(" Client: Movejob is finished becouse the next field is blocked: DestX: " + iToStr ( Waypoints->next->X ) + ", DestY: " + iToStr ( Waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);
			bFinished = true;
			// TODO: Calc a new path and start the new job
		}
		break;
	}
}

void cClientMoveJob::moveVehicle()
{
	if ( Vehicle == NULL || Vehicle->ClientMoveJob != this ) return;

	// do not move the vehicle, if the movejob hasn't got any more waypoints
	if ( Waypoints == NULL || Waypoints->next == NULL ) return;

	int iSpeed;
	if ( Vehicle->data.is_human )
	{
		Vehicle->WalkFrame++;
		if ( Vehicle->WalkFrame >= 13 ) Vehicle->WalkFrame = 0;
		iSpeed = MOVE_SPEED/2;
	}
	else if ( !(Vehicle->data.can_drive == DRIVE_AIR) && !(Vehicle->data.can_drive == DRIVE_SEA) )
	{
		iSpeed = MOVE_SPEED;
		if ( Waypoints && Waypoints->next && Map->GO[Waypoints->next->X+Waypoints->next->Y*Map->size].base&& ( Map->GO[Waypoints->next->X+Waypoints->next->Y*Map->size].base->data.is_road || Map->GO[Waypoints->next->X+Waypoints->next->Y*Map->size].base->data.is_bridge ) ) iSpeed*=2;
	}
	else if ( Vehicle->data.can_drive == DRIVE_AIR ) iSpeed = MOVE_SPEED*2;
	else iSpeed = MOVE_SPEED;

	// Ggf Tracks malen:
	if ( SettingsData.bMakeTracks && Vehicle->data.make_tracks && !Map->IsWater ( Vehicle->PosX+Vehicle->PosY*Map->size,false ) &&!
	        ( Waypoints && Waypoints->next && Map->terrain[Map->Kacheln[Waypoints->next->X+Waypoints->next->Y*Map->size]].water ) &&
	        ( Vehicle->owner == Client->ActivePlayer || Client->ActivePlayer->ScanMap[Vehicle->PosX+Vehicle->PosY*Map->size] ) )
	{
		if ( !Vehicle->OffX && !Vehicle->OffY )
		{
			switch ( Vehicle->dir )
			{
				case 0:
					Client->addFX ( fxTracks,Vehicle->PosX*64,Vehicle->PosY*64-10,0 );
					break;
				case 4:
					Client->addFX ( fxTracks,Vehicle->PosX*64,Vehicle->PosY*64+10,0 );
					break;
				case 2:
					Client->addFX ( fxTracks,Vehicle->PosX*64+10,Vehicle->PosY*64,2 );
					break;
				case 6:
					Client->addFX ( fxTracks,Vehicle->PosX*64-10,Vehicle->PosY*64,2 );
					break;
				case 1:
					Client->addFX ( fxTracks,Vehicle->PosX*64,Vehicle->PosY*64,1 );
					break;
				case 5:
					Client->addFX ( fxTracks,Vehicle->PosX*64,Vehicle->PosY*64,1 );
					break;
				case 3:
					Client->addFX ( fxTracks,Vehicle->PosX*64,Vehicle->PosY*64,3 );
					break;
				case 7:
					Client->addFX ( fxTracks,Vehicle->PosX*64,Vehicle->PosY*64,3 );
					break;
			}
		}
		else if ( abs ( Vehicle->OffX ) == iSpeed*2 || abs ( Vehicle->OffY ) == iSpeed*2 )
		{
			switch ( Vehicle->dir )
			{
				case 1:
					Client->addFX ( fxTracks,Vehicle->PosX*64+26,Vehicle->PosY*64-26,1 );
					break;
				case 5:
					Client->addFX ( fxTracks,Vehicle->PosX*64-26,Vehicle->PosY*64+26,1 );
					break;
				case 3:
					Client->addFX ( fxTracks,Vehicle->PosX*64+26,Vehicle->PosY*64+26,3 );
					break;
				case 7:
					Client->addFX ( fxTracks,Vehicle->PosX*64-26,Vehicle->PosY*64-26,3 );
					break;
			}
		}
	}

	switch ( iNextDir )
	{
		case 0:
			Vehicle->OffY -= iSpeed;
			break;
		case 1:
			Vehicle->OffY -= iSpeed;
			Vehicle->OffX += iSpeed;
			break;
		case 2:
			Vehicle->OffX += iSpeed;
			break;
		case 3:
			Vehicle->OffX += iSpeed;
			Vehicle->OffY += iSpeed;
			break;
		case 4:
			Vehicle->OffY += iSpeed;
			break;
		case 5:
			Vehicle->OffX -= iSpeed;
			Vehicle->OffY += iSpeed;
			break;
		case 6:
			Vehicle->OffX -= iSpeed;
			break;
		case 7:
			Vehicle->OffX -= iSpeed;
			Vehicle->OffY -= iSpeed;
			break;
	}

	// check whether the point has been reached:
	if ( Vehicle->OffX >= 64 || Vehicle->OffY >= 64 || Vehicle->OffX <= -64 || Vehicle->OffY <= -64 )
	{
		cLog::write(" Client: Vehicle reached the next field: ID: " + iToStr ( Vehicle->iID )+ ", X: " + iToStr ( Waypoints->next->X ) + ", Y: " + iToStr ( Waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);
		doEndMoveVehicle ();
	}
}

void cClientMoveJob::doEndMoveVehicle ()
{
	if ( Vehicle == NULL || Vehicle->ClientMoveJob != this ) return;

	if ( Waypoints->next == NULL )
	{
		// this is just to avoid errors, this should normaly never happen.
		bFinished = true;
		return;
	}

	Vehicle->data.speed += iSavedSpeed;
	iSavedSpeed = 0;
	Vehicle->DecSpeed ( Waypoints->next->Costs );
	Vehicle->moving = false;
	Vehicle->WalkFrame = 0;

	sWaypoint *Waypoint;
	Waypoint = Waypoints->next;
	free ( Waypoints );
	Waypoints = Waypoint;

	Vehicle->moving = false;

	if ( !bPlane )
	{
		if ( Map->GO[Waypoints->X+Waypoints->Y*Map->size].vehicle != NULL || Map->GO[Waypoints->X+Waypoints->Y*Map->size].top != NULL && !Map->GO[Waypoints->X+Waypoints->Y*Map->size].top->data.is_connector )
		{
			cLog::write ( " Client: Next waypoint for vehicle with ID \"" + iToStr( Vehicle->iID )  + "\" is blocked by an other unit", cLog::eLOG_TYPE_NET_ERROR );
			bFinished = true;
		}
		else
		{
			Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].vehicle = NULL;
			Map->GO[Waypoints->X+Waypoints->Y*Map->size].vehicle = Vehicle;
			Vehicle->PosX = Waypoints->X;
			Vehicle->PosY = Waypoints->Y;
		}
	}
	else
	{
		Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].plane = NULL;
		if ( Map->GO[Waypoints->X+Waypoints->Y*Map->size].plane != NULL )
		{
			cLog::write ( " Client: Next waypoint for plane with ID \"" + iToStr( Vehicle->iID )  + "\" is blocked by an other plane", cLog::eLOG_TYPE_NET_ERROR );
			bFinished = true;
		}
		else
		{
			Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].plane = NULL;
			Map->GO[Waypoints->X+Waypoints->Y*Map->size].plane = Vehicle;
			Vehicle->PosX = Waypoints->X;
			Vehicle->PosY = Waypoints->Y;
		}
	}
	Vehicle->OffX = 0;
	Vehicle->OffY = 0;

	Vehicle->owner->DoScan();
	
	Client->bFlagDrawMMap = true; 
	Client->mouseMoveCallback( true ); 

	calcNextDir();
}

void cClientMoveJob::calcNextDir ()
{
	if ( !Waypoints || !Waypoints->next ) return;
#define TESTXY_CND(a,b) if( Waypoints->X a Waypoints->next->X && Waypoints->Y b Waypoints->next->Y )
	TESTXY_CND ( ==,> ) iNextDir = 0;
	else TESTXY_CND ( <,> ) iNextDir = 1;
	else TESTXY_CND ( <,== ) iNextDir = 2;
	else TESTXY_CND ( <,< ) iNextDir = 3;
	else TESTXY_CND ( ==,< ) iNextDir = 4;
	else TESTXY_CND ( >,< ) iNextDir = 5;
	else TESTXY_CND ( >,== ) iNextDir = 6;
	else TESTXY_CND ( >,> ) iNextDir = 7;
}

void cClientMoveJob::drawArrow ( SDL_Rect Dest, SDL_Rect *LastDest, bool bSpezial )
{
	int iIndex = 0;
#define TESTXY_DP(a,b) if( Dest.x a LastDest->x && Dest.y b LastDest->y )
	TESTXY_DP ( >,< ) iIndex = 0;
	else TESTXY_DP ( ==,< ) iIndex = 1;
	else TESTXY_DP ( <,< ) iIndex = 2;
	else TESTXY_DP ( >,== ) iIndex = 3;
	else TESTXY_DP ( <,== ) iIndex = 4;
	else TESTXY_DP ( >,> ) iIndex = 5;
	else TESTXY_DP ( ==,> ) iIndex = 6;
	else TESTXY_DP ( <,> ) iIndex = 7;
	if ( bSpezial )
	{
		SDL_BlitSurface ( OtherData.WayPointPfeileSpecial[iIndex][64-Client->Hud.Zoom], NULL, buffer, &Dest );
	}
	else
	{
		SDL_BlitSurface ( OtherData.WayPointPfeile[iIndex][64-Client->Hud.Zoom], NULL, buffer, &Dest );
	}
}
