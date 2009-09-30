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
#include "math.h"
#include "settings.h"

cPathDestHandler::cPathDestHandler ( ePathDestinationTypes type_, int destX_, int destY_, cVehicle *srcVehicle_, cBuilding *destBuilding_, cVehicle *destVehicle_ ) :
	type ( type_ ),
	destX ( destX_ ),
	destY ( destY_ ),
	srcVehicle ( srcVehicle_ ),
	destBuilding ( destBuilding_ ),
	destVehicle ( destVehicle_ )
{ }

bool cPathDestHandler::hasReachedDestination( int x, int y )
{
	switch ( type )
	{
	case PATH_DEST_TYPE_POS:
		return ( x == destX && y == destY );
	case PATH_DEST_TYPE_LOAD:
		return ( ( destBuilding && destBuilding->isNextTo ( x, y ) ) ||
				( destVehicle && destVehicle->isNextTo ( x, y ) ) );
	case PATH_DEST_TYPE_ATTACK:
		x -= destX;
		y -= destY;
		return ( sqrt ( ( double ) ( x*x + y*y ) ) <= srcVehicle->data.range );
	default:
		return true;
	}
	return false;
}

int cPathDestHandler::heuristicCost ( int srcX, int srcY )
{
	switch ( type )
	{
	case PATH_DEST_TYPE_POS:
	case PATH_DEST_TYPE_LOAD:
		return 0;
	case PATH_DEST_TYPE_ATTACK:
	default:
		{
			int deltaX = destX - srcX;
			int deltaY = destY - srcY;

			return Round ( sqrt ( (double)deltaX*deltaX + deltaY*deltaY ) );
		}
	}
}

cPathCalculator::cPathCalculator( int ScrX, int ScrY, int DestX, int DestY, cMap *Map, cVehicle *Vehicle, cList<cVehicle*> *group )
{
	destHandler = new cPathDestHandler ( PATH_DEST_TYPE_POS, DestX, DestY, NULL, NULL, NULL );
	init ( ScrX, ScrY, Map, Vehicle, group );
}


cPathCalculator::cPathCalculator( int ScrX, int ScrY, cVehicle *destVehicle, cBuilding *destBuilding, cMap *Map, cVehicle *Vehicle, bool load )
{
	destHandler = new cPathDestHandler ( load ? PATH_DEST_TYPE_LOAD : PATH_DEST_TYPE_ATTACK, 0, 0, Vehicle, destBuilding, destVehicle );
	init ( ScrX, ScrY, Map, Vehicle, NULL );
}

cPathCalculator::cPathCalculator( int ScrX, int ScrY, cMap *Map, cVehicle *Vehicle, int attackX, int attackY )
{
	destHandler = new cPathDestHandler ( PATH_DEST_TYPE_ATTACK, attackX, attackY, Vehicle, NULL, NULL );
	init ( ScrX, ScrY, Map, Vehicle, NULL );
}

void cPathCalculator::init( int ScrX, int ScrY, cMap *Map, cVehicle *Vehicle, cList<cVehicle*> *group )
{
	this->ScrX = ScrX;
	this->ScrY = ScrY;
	this->Map = Map;
	this->Vehicle = Vehicle;
	this->group = group;
	bPlane = Vehicle->data.factorAir > 0;
	bShip = Vehicle->data.factorSea > 0 && Vehicle->data.factorGround == 0;

	Waypoints = NULL;
	MemBlocks = NULL;
	nodesHeap = NULL;
	openList = NULL;
	closedList = NULL;

	blocknum = 0;
	blocksize = 0;
	heapCount = 0;
}

cPathCalculator::~cPathCalculator()
{
	delete destHandler;
	if ( MemBlocks != NULL )
	{
		for ( int i = 0; i < blocknum; i++ )
		{
			delete MemBlocks[i];
		}
		free ( MemBlocks );
	}
	if ( nodesHeap != NULL ) delete [] nodesHeap;
	if ( openList != NULL ) delete [] openList;
	if ( closedList != NULL ) delete [] closedList;
}

sWaypoint* cPathCalculator::calcPath ()
{
	// generate open and closed list
	nodesHeap = new sPathNode*[Map->size*Map->size+1];
	openList = new sPathNode*[Map->size*Map->size+1];
	closedList = new sPathNode*[Map->size*Map->size+1];
	fill <sPathNode**, sPathNode*> ( nodesHeap, &nodesHeap[Map->size*Map->size+1], NULL );
	fill <sPathNode**, sPathNode*> ( openList, &openList[Map->size*Map->size+1], NULL );
	fill <sPathNode**, sPathNode*> ( closedList, &closedList[Map->size*Map->size+1], NULL );

	// generate startnode
	sPathNode *StartNode = allocNode ();
	StartNode->x = ScrX;
	StartNode->y = ScrY;
	StartNode->costG = 0;
	StartNode->costH = destHandler->heuristicCost ( ScrX, ScrY );
	StartNode->costF = StartNode->costG+StartNode->costH;

	StartNode->prev = NULL;
	openList[ScrX+ScrY*Map->size] = StartNode;
	insertToHeap ( StartNode, false );

	while ( heapCount > 0 )
	{
		// get the node with the lowest F value
		sPathNode *CurrentNode = nodesHeap[1];

		// move it from the open to the closed list
		openList[CurrentNode->x+CurrentNode->y*Map->size] = NULL;
		closedList[CurrentNode->x+CurrentNode->y*Map->size] = CurrentNode;
		deleteFirstFromHeap();

		// generate waypoints when destination has been reached
		if ( destHandler->hasReachedDestination ( CurrentNode->x, CurrentNode->y ) )
		{
			sWaypoint *NextWaypoint;
			Waypoints = new sWaypoint;

			sPathNode *NextNode = CurrentNode;
			NextNode->next = NULL;
			do
			{
				NextNode->prev->next = NextNode;
				NextNode = NextNode->prev;
			}
			while ( NextNode->prev != NULL );


			NextWaypoint = Waypoints;
			NextWaypoint->X = NextNode->x;
			NextWaypoint->Y = NextNode->y;
			NextWaypoint->Costs = 0;
			do
			{
				NextNode = NextNode->next;

				NextWaypoint->next = new sWaypoint;
				NextWaypoint->next->X = NextNode->x;
				NextWaypoint->next->Y = NextNode->y;
				NextWaypoint->next->Costs = calcNextCost ( NextNode->prev->x, NextNode->prev->y, NextWaypoint->next->X, NextWaypoint->next->Y );
				NextWaypoint = NextWaypoint->next;
			}
			while ( NextNode->next != NULL );

			NextWaypoint->next = NULL;

			return Waypoints;
		}

		// expand node
		expandNodes ( CurrentNode );
	}

	// there is no path to the destination field
	Waypoints = NULL;


	return Waypoints;
}

void cPathCalculator::expandNodes ( sPathNode *ParentNode )
{
	// add all nearby nodes
	for ( int y = ParentNode->y-1; y <= ParentNode->y+1; y++ )
	{
		if ( y < 0 || y >= Map->size ) continue;

		for ( int x = ParentNode->x-1; x <= ParentNode->x+1; x++ )
		{
			if ( x < 0 || x >= Map->size ) continue;
			if ( x == ParentNode->x && y == ParentNode->y ) continue;

			if ( !Map->possiblePlaceVehicle( Vehicle->data, x, y, Vehicle->owner) )
			{
				// when we have a group of units, the units will not block each other
				if ( group )
				{
					bool isInGroup = false;
					// get the blocking unit
					cVehicle *blockingUnit;
					if ( Vehicle->data.factorAir > 0 ) blockingUnit = (*Map)[x+y*Map->size].getPlanes();
					else blockingUnit = (*Map)[x+y*Map->size].getVehicles();
					// check whether the blocking unit is the group
					for ( unsigned int i = 0; i < group->Size(); i++ )
					{
						if ( (*group)[i] == blockingUnit )
						{
							isInGroup = true;
							break;
						}
					}
					if ( !isInGroup ) continue;
				}
				else continue;
			}
			if ( closedList[x+y*Map->size] != NULL ) continue;

			if ( openList[x+y*Map->size] == NULL )
			{
				// generate new node
				sPathNode *NewNode = allocNode ();
				NewNode->x = x;
				NewNode->y = y;
				NewNode->costG = calcNextCost( ParentNode->x, ParentNode->y, x, y ) + ParentNode->costG;
				NewNode->costH = destHandler->heuristicCost ( x, y );
				NewNode->costF = NewNode->costG+NewNode->costH;
				NewNode->prev = ParentNode;
				openList[x+y*Map->size] = NewNode;
				insertToHeap ( NewNode, false );
			}
			else
			{
				// modify existing node
				int costG, costH, costF;
				costG = calcNextCost( ParentNode->x, ParentNode->y, x,y ) + ParentNode->costG;
				costH = destHandler->heuristicCost ( x, y );
				costF = costG+costH;
				if ( costF < openList[x+y*Map->size]->costF )
				{
					openList[x+y*Map->size]->costG = costG;
					openList[x+y*Map->size]->costH = costH;
					openList[x+y*Map->size]->costF = costF;
					openList[x+y*Map->size]->prev = ParentNode;
					insertToHeap ( openList[x+y*Map->size], true );
				}
			}
		}
	}
}

sPathNode *cPathCalculator::allocNode()
{
	// alloced new memory block if necessary
	if ( blocksize <= 0 )
	{
		MemBlocks = (sPathNode**) realloc ( MemBlocks, (blocknum+1)*sizeof(sPathNode*) );
		MemBlocks[blocknum] = new sPathNode[10];
		blocksize = MEM_BLOCK_SIZE;
		blocknum++;
	}
	blocksize--;
	return &MemBlocks[blocknum-1][blocksize];
}

void cPathCalculator::insertToHeap( sPathNode *Node, bool exists )
{
	int i;
	if ( exists )
	{
		// get the number of the existing node
		for ( int j = 1; j <= heapCount; j++ )
		{
			if ( nodesHeap[j] == Node )
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
	while ( i > 1 )
	{
		if ( Node->costF < nodesHeap[i/2]->costF )
		{
			sPathNode *TempNode = nodesHeap[i/2];
			nodesHeap[i/2] = Node;
			nodesHeap[i] = TempNode;
			i = i/2;
		}
		else break;
	}
}

void cPathCalculator::deleteFirstFromHeap()
{
	// overwrite the first node by the last one
	nodesHeap[1] = nodesHeap[heapCount];
	nodesHeap[heapCount] = NULL;
	heapCount--;
	int v = 1, u;
	while ( true )
	{
		u = v;
		if ( 2*u+1 <= heapCount )	// both children in the heap exists
		{
			if ( nodesHeap[u]->costF >= nodesHeap[u*2]->costF ) v = 2*u;
			if ( nodesHeap[v]->costF >= nodesHeap[u*2+1]->costF ) v = 2*u+1;
		}
		else if ( 2*u <= heapCount )	// only one children exists
		{
			if ( nodesHeap[u]->costF >= nodesHeap[u*2]->costF ) v = 2*u;
		}
		// do the resort
		if ( u != v )
		{
			sPathNode *TempNode = nodesHeap[u];
			nodesHeap[u] = nodesHeap[v];
			nodesHeap[v] = TempNode;
		}
		else break;
	}
}

int cPathCalculator::calcNextCost( int srcX, int srcY, int destX, int destY )
{
	int costs, offset;
	// first we check whether the unit can fly
	if ( Vehicle->data.factorAir > 0 )
	{
		if ( srcX != destX && srcY != destY ) return (int)(4*Vehicle->data.factorAir*1.5);
		else return (int)(4*Vehicle->data.factorAir);
	}
	offset = destX+destY*Map->size;
	cBuilding* building = Map->fields[offset].getBaseBuilding();
	// moving on water will cost more
	if ( Map->terrain[Map->Kacheln[offset]].water && ( !building || ( building->data.surfacePosition == sUnitData::SURFACE_POS_BENEATH_SEA || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE ) )&& Vehicle->data.factorSea > 0 ) costs = (int)(4*Vehicle->data.factorSea);
	else if ( Map->terrain[Map->Kacheln[offset]].coast && !building && Vehicle->data.factorCoast > 0 ) costs = (int)(4*Vehicle->data.factorCoast);
	else if ( Vehicle->data.factorGround > 0 ) costs = (int)(4*Vehicle->data.factorGround);
	else
	{
		Log.write ( "Where can this unit move? " + iToStr ( Vehicle->iID ), cLog::eLOG_TYPE_NET_WARNING );
		costs = 4;
	}
	// moving on a road is cheaper
	if ( building && building->data.modifiesSpeed != 0 ) costs = (int)(costs*building->data.modifiesSpeed);

	// mutliplicate with the factor 1.5 for diagonal movements
	if ( srcX != destX && srcY != destY ) costs = (int)(costs*1.5);
	return costs;
}

void setOffset( cVehicle* Vehicle, int nextDir, int offset )
{
	switch ( nextDir )
	{
		case 0:
			Vehicle->OffY -= offset;
			break;
		case 1:
			Vehicle->OffY -= offset;
			Vehicle->OffX += offset;
			break;
		case 2:
			Vehicle->OffX += offset;
			break;
		case 3:
			Vehicle->OffX += offset;
			Vehicle->OffY += offset;
			break;
		case 4:
			Vehicle->OffY += offset;
			break;
		case 5:
			Vehicle->OffX -= offset;
			Vehicle->OffY += offset;
			break;
		case 6:
			Vehicle->OffX -= offset;
			break;
		case 7:
			Vehicle->OffX -= offset;
			Vehicle->OffY -= offset;
			break;
	}

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
	bFinished = false;
	bEndForNow = false;
	iSavedSpeed = 0;
	Waypoints = NULL;

	// unset sentry status when moving vehicle
	if ( Vehicle->bSentryStatus )
	{
		Vehicle->owner->deleteSentryVehicle ( Vehicle );
		Vehicle->bSentryStatus = false;
	}
	sendUnitData ( Vehicle, Vehicle->owner->Nr );
	for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
	{
		sendUnitData ( Vehicle, Vehicle->SeenByPlayerList[i]->Nr );
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
		delete Waypoints;
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

	Log.write(" Server: Received MoveJob: VehicleID: " + iToStr( Vehicle->iID ) + ", SrcX: " + iToStr( ScrX ) + ", SrcY: " + iToStr( ScrY ) + ", DestX: " + iToStr( DestX ) + ", DestY: " + iToStr( DestY ) + ", WaypointCount: " + iToStr( iReceivedCount ), cLog::eLOG_TYPE_NET_DEBUG);
	
	// Add the waypoints
	sWaypoint *Waypoint = new sWaypoint;
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
			Waypoint->next = new sWaypoint;
			Waypoint = Waypoint->next;
		}
	}
	calcNextDir ();

	return true;
}

bool cServerMoveJob::calcPath()
{
	if ( ScrX == DestX && ScrY == DestY ) return false;

	cPathCalculator PathCalculator( ScrX, ScrY, DestX, DestY, Map, Vehicle );
	Waypoints = PathCalculator.calcPath();
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
	Log.write ( " Server: Released old movejob", cLog::eLOG_TYPE_NET_DEBUG );
	for ( unsigned int i = 0; i < Server->ActiveMJobs.Size(); i++ )
	{
		if ( this == Server->ActiveMJobs[i] ) return;
	}
	Server->addActiveMoveJob ( this );
	Log.write ( " Server: Added released movejob to avtive ones", cLog::eLOG_TYPE_NET_DEBUG );
}

bool cServerMoveJob::checkMove()
{
	bool bInSentryRange;
	if ( !Vehicle || !Waypoints || !Waypoints->next )
	{
		bFinished = true;
		return false;
	}

	// not enough waypoints for this move?
	if ( Vehicle->data.speedCur < Waypoints->next->Costs )
	{
		Log.write( " Server: Vehicle has not enough waypoints for the next move -> EndForNow: ID: " + iToStr ( Vehicle->iID ) + ", X: " + iToStr ( Waypoints->next->X ) + ", Y: " + iToStr ( Waypoints->next->Y ), LOG_TYPE_NET_DEBUG );
		iSavedSpeed += Vehicle->data.speedCur;
		Vehicle->data.speedCur = 0;
		bEndForNow = true;
		return true;
	}

	bInSentryRange = Vehicle->InSentryRange();

	if ( !Server->Map->possiblePlace( Vehicle, Waypoints->next->X, Waypoints->next->Y) && !bInSentryRange )
	{
		Server->sideStepStealthUnit( Waypoints->next->X, Waypoints->next->Y, Vehicle );
	}
	
	//when the next field is still blocked, inform the client
	if ( !Server->Map->possiblePlace( Vehicle, Waypoints->next->X, Waypoints->next->Y) || bInSentryRange ) //TODO: bInSentryRange?? Why?
	{
		Log.write( " Server: Next point is blocked: ID: " + iToStr ( Vehicle->iID ) + ", X: " + iToStr ( Waypoints->next->X ) + ", Y: " + iToStr ( Waypoints->next->Y ), LOG_TYPE_NET_DEBUG );
		// if the next point would be the last, finish the job here
		if ( Waypoints->next->X == DestX && Waypoints->next->Y == DestY )
		{
			bFinished = true;
		}
		// else delete the movejob and inform the client that he has to find a new path
		else
		{
			sendNextMove ( Vehicle, MJOB_BLOCKED );
		}
		return false;
	}

	//next step can be executed. start the move and set the vehicle to the next field
	calcNextDir();
	Vehicle->MoveJobActive = true;
	Vehicle->moving = true;

	// send move command to all players who can see the unit
	sendNextMove ( Vehicle, MJOB_OK );

	Map->moveVehicle(Vehicle, Waypoints->next->X, Waypoints->next->Y );
	Vehicle->owner->DoScan();
	Vehicle->OffX = 0;
	Vehicle->OffY = 0;
	setOffset( Vehicle, iNextDir, -64 );

	return true;
}

void cServerMoveJob::moveVehicle()
{
	int iSpeed;
	if ( !Vehicle ) return;
	if ( Vehicle->data.animationMovement )
	{
		iSpeed = MOVE_SPEED/2;
	}
	else if ( !(Vehicle->data.factorAir > 0) && !(Vehicle->data.factorSea > 0 && Vehicle->data.factorGround == 0) )
	{
		iSpeed = MOVE_SPEED;
		cBuilding* building = Map->fields[Waypoints->next->X+Waypoints->next->Y*Map->size].getBaseBuilding();
		if ( Waypoints && Waypoints->next && building && building->data.modifiesSpeed ) iSpeed = (int)(iSpeed/building->data.modifiesSpeed);
	}
	else if ( Vehicle->data.factorAir > 0 ) iSpeed = MOVE_SPEED*2;
	else iSpeed = MOVE_SPEED;

	setOffset(Vehicle, iNextDir, iSpeed );

	// check whether the point has been reached:
	if ( abs( Vehicle->OffX ) < iSpeed && abs( Vehicle->OffY ) < iSpeed )
	{
		doEndMoveVehicle();
	}
}

void cServerMoveJob::doEndMoveVehicle()
{
	Log.write(" Server: Vehicle reached the next field: ID: " + iToStr ( Vehicle->iID )+ ", X: " + iToStr ( Waypoints->next->X ) + ", Y: " + iToStr ( Waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);
	sWaypoint *Waypoint;
	Waypoint = Waypoints->next;
	delete Waypoints;
	Waypoints = Waypoint;

	Vehicle->OffX = 0;
	Vehicle->OffY = 0;
	
	if ( Waypoints->next == NULL )
	{
		bFinished = true;
	}

	Vehicle->data.speedCur += iSavedSpeed;
	iSavedSpeed = 0;
	Vehicle->DecSpeed ( Waypoints->Costs );

	// check for results of the move

	// make mines explode if necessary
	cBuilding* mine = Map->fields[Vehicle->PosX+Vehicle->PosY*Map->size].getMine();
	if ( Vehicle->data.factorAir == 0 && mine && mine->owner != Vehicle->owner )
	{
		Server->AJobs.Add( new cServerAttackJob( mine, Vehicle->PosX+Vehicle->PosY*Map->size ));
		bEndForNow = true;
	}

	// search for resources if necessary
	if ( Vehicle->data.canSurvey )
	{
		sendVehicleResources( Vehicle, Map );
		Vehicle->doSurvey();
	}

	//handle detection
	Vehicle->makeDetection();

	// let other units fire on this one
	Vehicle->InSentryRange();

	// lay/clear mines if necessary
	if ( Vehicle->data.canPlaceMines )
	{
		bool bResult = false;
		if ( Vehicle->LayMines ) bResult = Vehicle->layMine();
		else if ( Vehicle->ClearMines ) bResult = Vehicle->clearMine();
		if ( bResult )
		{
			// send new unit values
			sendUnitData( Vehicle, Vehicle->owner->Nr );
			for ( unsigned int i = 0; i < Vehicle->SeenByPlayerList.Size(); i++ )
			{
				sendUnitData(Vehicle, Vehicle->SeenByPlayerList[i]->Nr );
			}
		}
	}

	Vehicle->moving = false;
	calcNextDir();

	//workaround for repairing/reloading
	cBuilding* b = (*Server->Map)[Vehicle->PosX+Vehicle->PosY*Server->Map->size].getBuildings();
	if ( Vehicle->data.factorAir > 0 && b && b->owner == Vehicle->owner && b->data.canBeLandedOn )
	{
		Vehicle->FlightHigh = 0;
	}
	else
	{
		Vehicle->FlightHigh = 64;
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

cEndMoveAction::cEndMoveAction( eEndMoveActionType endMoveActionType_, cBuilding *srcBuilding_, cVehicle *srcVehicle_, cBuilding *destBuilding_, cVehicle *destVehicle_, int destX_, int destY_ ) :
	endMoveActionType ( endMoveActionType_ ),
	srcBuilding ( srcBuilding_ ),
	srcVehicle ( srcVehicle_ ),
	destVehicle ( destVehicle_ ),
	destBuilding ( destBuilding_ ),
	destX ( destX_ ),
	destY ( destY_ ),
	moveJob ( NULL ),
	success ( false )
{
	if ( !srcVehicle && !srcBuilding ) return;
	if ( !destVehicle && !destBuilding ) return;

	switch ( endMoveActionType )
	{
		case EMAT_LOAD:
			generateLoadAction();
			break;
		case EMAT_GET_IN:
			generateGetInAction();
			break;
		case EMAT_ATTACK:
			generateAttackAction();
			break;
	}
}

cEndMoveAction::~cEndMoveAction()
{
	if ( moveJob ) moveJob->endMoveAction = NULL;
	for ( unsigned int i = 0; srcVehicle && i < srcVehicle->passiveEndMoveActions.Size(); i++ )
	{
		if ( srcVehicle->passiveEndMoveActions[i] == this ) srcVehicle->passiveEndMoveActions.Delete ( i );
	}
	for ( unsigned int i = 0; srcBuilding && i < srcBuilding->passiveEndMoveActions.Size(); i++ )
	{
		if ( srcBuilding->passiveEndMoveActions[i] == this ) srcBuilding->passiveEndMoveActions.Delete ( i );
	}
	for ( unsigned int i = 0; destVehicle && i < destVehicle->passiveEndMoveActions.Size(); i++ )
	{
		if ( destVehicle->passiveEndMoveActions[i] == this ) destVehicle->passiveEndMoveActions.Delete ( i );
	}
	for ( unsigned int i = 0; destBuilding && i < destBuilding->passiveEndMoveActions.Size(); i++ )
	{
		if ( destBuilding->passiveEndMoveActions[i] == this ) destBuilding->passiveEndMoveActions.Delete ( i );
	}
}

void cEndMoveAction::generateLoadAction()
{
	// only vehicles can load vehicles
	if ( !srcVehicle || !destVehicle ) return;
	// we support only planes
	if ( srcVehicle->data.factorAir == 0 ) return;

	// check whether the field above the destVehicle is free
	if ( !Client->Map->possiblePlace ( srcVehicle, destVehicle->PosX, destVehicle->PosY ) ) return;
	// check whether the destVehicle can be loaded
	if ( !srcVehicle->canLoad ( destVehicle, false ) ) return;

	// generate the Movejob
	int srcOffset = srcVehicle->PosX+srcVehicle->PosY*Client->Map->size; 
	int destOffset = destVehicle->PosX+destVehicle->PosY*Client->Map->size;
	moveJob = new cClientMoveJob ( srcOffset, destOffset, srcVehicle->data.factorAir > 0, srcVehicle );
	if ( moveJob->calcPath() )
	{
		// add the endMoveAction and send the movejob
		success = true;
		moveJob->endMoveAction = this;
		destVehicle->passiveEndMoveActions.Add ( this );
		sendMoveJob ( moveJob );
		Log.write(" Client: Added new movejob with endMoveAction: VehicleID: " + iToStr ( srcVehicle->iID ) + ", SrcX: " + iToStr ( srcVehicle->PosX ) + ", SrcY: " + iToStr ( srcVehicle->PosY ) + ", DestX: " + iToStr ( moveJob->DestX ) + ", DestY: " + iToStr ( moveJob->DestY ), cLog::eLOG_TYPE_NET_DEBUG);
	}
	else
	{
		// no path found so delete the movejob
		if ( moveJob->Vehicle ) moveJob->Vehicle->ClientMoveJob = NULL;
		delete moveJob;
		moveJob = NULL;
	}

}

void cEndMoveAction::generateGetInAction()
{
	// only vehicles can be loaded
	if ( !destVehicle ) return;
	// we need a srcUnit
	if ( !srcVehicle && !srcBuilding ) return;

	// check whether the srcUnit can load the destVehicle
	if ( ( srcVehicle && !srcVehicle->canLoad ( destVehicle, false ) ) ||
		 ( srcBuilding && !srcBuilding->canLoad ( destVehicle, false ) ) ) return;

	cPathCalculator PathCalculator( destVehicle->PosX, destVehicle->PosY, srcVehicle, srcBuilding, Client->Map, destVehicle, true );
	sWaypoint *waypoints = PathCalculator.calcPath();

	// generate the Movejob
	int srcOffset = destVehicle->PosX+destVehicle->PosY*Client->Map->size;
	if ( waypoints )
	{
		moveJob = new cClientMoveJob ( srcOffset, waypoints, destVehicle->data.factorAir > 0, destVehicle );
		// add the endMoveAction and send the movejob
		success = true;
		moveJob->endMoveAction = this;
		if ( srcVehicle ) srcVehicle->passiveEndMoveActions.Add ( this );
		if ( srcBuilding ) srcBuilding->passiveEndMoveActions.Add ( this );
		sendMoveJob ( moveJob );
		Log.write(" Client: Added new movejob with endMoveAction: VehicleID: " + iToStr ( destVehicle->iID ) + ", SrcX: " + iToStr ( destVehicle->PosX ) + ", SrcY: " + iToStr ( destVehicle->PosY ) + ", DestX: " + iToStr ( moveJob->DestX ) + ", DestY: " + iToStr ( moveJob->DestY ), cLog::eLOG_TYPE_NET_DEBUG);
	}
}

void cEndMoveAction::generateAttackAction()
{
	// only vehicles can move to fire
	if ( !srcVehicle ) return;
	// we need a target unit
	if ( !destVehicle && !destBuilding) return;
	// and we need a target field
	if ( destX == -1 || destY == -1 ) return;

	// check whether the srcVehicle can attack the dest field
	if ( !srcVehicle->CanAttackObject ( destX+destY*Client->Map->size, Client->Map, true, false ) ) return;

	// calculate path unit the destination fiel is in range
	cPathCalculator PathCalculator( srcVehicle->PosX, srcVehicle->PosY, Client->Map, srcVehicle, destX, destY );
	sWaypoint *waypoints = PathCalculator.calcPath();

	// generate the Movejob
	int srcOffset = srcVehicle->PosX+srcVehicle->PosY*Client->Map->size;
	if ( waypoints )
	{
		// create the movejob with the calculated path
		moveJob = new cClientMoveJob ( srcOffset, waypoints, srcVehicle->data.factorAir > 0, srcVehicle );
		// add the endMoveAction and send the movejob
		success = true;
		moveJob->endMoveAction = this;
		if ( destVehicle ) destVehicle->passiveEndMoveActions.Add ( this );
		if ( destBuilding ) destBuilding->passiveEndMoveActions.Add ( this );
		sendMoveJob ( moveJob );
		Log.write(" Client: Added new movejob with endMoveAction: VehicleID: " + iToStr ( srcVehicle->iID ) + ", SrcX: " + iToStr ( srcVehicle->PosX ) + ", SrcY: " + iToStr ( srcVehicle->PosY ) + ", DestX: " + iToStr ( moveJob->DestX ) + ", DestY: " + iToStr ( moveJob->DestY ), cLog::eLOG_TYPE_NET_DEBUG);
	}
}

void cEndMoveAction::execute()
{
	switch ( endMoveActionType )
	{
		case EMAT_LOAD:
			executeLoadAction();
			break;
		case EMAT_GET_IN:
			executeGetInAction();
			break;
		case EMAT_ATTACK:
			executeAttackAction();
			break;
	}
}

void cEndMoveAction::executeLoadAction()
{
	// only vehicles can load vehicles. We check this twice becouse we will not have the risk to crash the game
	if ( !srcVehicle || !destVehicle ) return;

	// execute the loading if possible
	if ( srcVehicle->canLoad ( destVehicle ) ) sendWantLoad ( srcVehicle->iID, true, destVehicle->iID );
}

void cEndMoveAction::executeGetInAction()
{
	// only vehicles can be loaded
	if ( !destVehicle ) return;
	// we need a srcUnit
	if ( !srcVehicle && !srcBuilding ) return;

	// execute the loading if possible
	if ( srcVehicle && srcVehicle->canLoad ( destVehicle ) ) sendWantLoad ( srcVehicle->iID, true, destVehicle->iID );
	else if ( srcBuilding && srcBuilding->canLoad ( destVehicle ) ) sendWantLoad ( srcBuilding->iID, false, destVehicle->iID ); 
}

void cEndMoveAction::executeAttackAction()
{
	// we need a source unit
	if ( !srcVehicle ) return;
	// we need a target
	if ( !destVehicle && !destBuilding ) return;

	int targetId = 0;
	int offset = destX+destY*Client->Map->size;

	// check whether the target unit is still at the target position
	cVehicle *targetVehicle;
	cBuilding *targetBuilding;
	selectTarget ( targetVehicle, targetBuilding, offset, srcVehicle->data.canAttack, Client->Map );
	if ( destVehicle )
	{
		if ( targetVehicle != destVehicle ) return;
		targetId = destVehicle->iID;
	}
	else if ( destBuilding && destBuilding != targetBuilding ) return;

	// check whether we can realy attack this offset now
	if ( !srcVehicle->CanAttackObject ( offset, Client->Map, true ) ) return;

	Log.write(" Client: want to attack on endMoveAction offset " + iToStr( offset ) + ", Vehicle ID: " + iToStr( targetId ), cLog::eLOG_TYPE_NET_DEBUG );
	sendWantAttack( targetId, offset, srcVehicle->iID, true );
}

bool cEndMoveAction::getSuccess()
{
	return success;
}

void cEndMoveAction::handleDelVehicle( cVehicle *delVehicle )
{
	if ( delVehicle == srcVehicle ) srcVehicle = NULL;
	if ( delVehicle == destVehicle ) destVehicle = NULL;
}

cClientMoveJob::cClientMoveJob ( int iSrcOff, int iDestOff, bool bPlane, cVehicle *Vehicle )
{	
	DestX = iDestOff%Client->Map->size;
	DestY = iDestOff/Client->Map->size;
	init ( iSrcOff, bPlane, Vehicle );
}

cClientMoveJob::cClientMoveJob ( int iSrcOff, sWaypoint *Waypoints, bool bPlane, cVehicle *Vehicle )
{
	this->Waypoints = Waypoints;
	sWaypoint *nextWaypoint = Waypoints;
	while ( nextWaypoint )
	{
		if ( !nextWaypoint->next )
		{
			DestX = nextWaypoint->X;
			DestY = nextWaypoint->Y;
		}
		nextWaypoint = nextWaypoint->next;
	}

	if ( Waypoints ) calcNextDir();
	
	init ( iSrcOff, bPlane, Vehicle );
}

void cClientMoveJob::init( int iSrcOff, bool bPlane, cVehicle *Vehicle )
{
	if ( !Client ) return;

	Map = Client->Map;
	this->Vehicle = Vehicle;
	ScrX = iSrcOff%Map->size;
	ScrY = iSrcOff/Map->size;
	this->bPlane = bPlane;
	bFinished = false;
	bEndForNow = false;
	bSoundRunning = false;
	iSavedSpeed = 0;
	lastWaypoints = NULL;
	bSuspended = false;

	/*if ( Vehicle->PosX != ScrX || Vehicle->PosY != ScrY )
	{
		Log.write(" Client: Vehicle with id " + iToStr ( Vehicle->iID ) + " is at wrong position (" + iToStr (Vehicle->PosX) + "x" + iToStr(Vehicle->PosY) + ") for movejob from " +  iToStr (ScrX) + "x" + iToStr (ScrY) + " to " + iToStr (DestX) + "x" + iToStr (DestY) + "resetting to right position", cLog::eLOG_TYPE_NET_WARNING);
		// set vehicle to correct position
		if  ( !bPlane && Vehicle == Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].vehicle ) Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].vehicle = NULL;
		else if ( bPlane && Vehicle == Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].plane ) Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].plane = NULL;
		Vehicle->PosX = ScrX;
		Vehicle->PosY = ScrY;
		if ( !bPlane ) Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].vehicle = Vehicle;
		else Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].plane = Vehicle;
	}*/

	if ( Vehicle->ClientMoveJob )
	{
		Vehicle->ClientMoveJob->release();
		Vehicle->moving = false;
		Vehicle->MoveJobActive = false;
	}
	Vehicle->ClientMoveJob = this;
	endMoveAction = NULL;
}

cClientMoveJob::~cClientMoveJob()
{
	sWaypoint *NextWaypoint;
	while ( Waypoints )
	{
		NextWaypoint = Waypoints->next;
		delete Waypoints;
		Waypoints = NextWaypoint;
	}
	while ( lastWaypoints )
	{
		NextWaypoint = lastWaypoints->next;
		delete lastWaypoints;
		lastWaypoints = NextWaypoint;
	}

	for ( unsigned int i = 0; i < Client->ActiveMJobs.Size(); i++ )
	{
		if ( Client->ActiveMJobs[i] == this ) 
		{
			Client->ActiveMJobs.Delete(i);
			i--;
		}
	}
	if ( endMoveAction ) delete endMoveAction;
}

void cClientMoveJob::setVehicleToCoords(int x, int y)
{
	if ( x == Waypoints->X && y == Waypoints->Y ) return;

	Log.write(" Client: mjob: setting vehicle " + iToStr(Vehicle->iID) + " to position " + iToStr(x) + " : " + iToStr(y), cLog::eLOG_TYPE_NET_DEBUG );
	//determine direction
	bool bForward = false;
	sWaypoint *Waypoint = Waypoints;
	while ( Waypoint )
	{
		if ( Waypoint->X == x && Waypoint->Y == y )
		{
			bForward = true;
			break;
		}
		Waypoint = Waypoint->next;
	}


	Map->moveVehicle( Vehicle, x, y );

	if ( bForward )
	{
		Waypoint = Waypoints;
		while ( Waypoint )
		{
			if ( Waypoint->X != x || Waypoint->Y != y )
			{
				Vehicle->DecSpeed( Waypoint->next->Costs );
				Waypoints = Waypoints->next;
				Waypoint->next = lastWaypoints;
				lastWaypoints = Waypoint;

				Waypoint = Waypoints;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		
		Waypoint = lastWaypoints;
		while ( Waypoint )
		{	
			Vehicle->DecSpeed( -Waypoints->Costs );
			lastWaypoints = lastWaypoints->next;
			Waypoint->next = Waypoints;
			Waypoints = Waypoint;
			if ( Waypoint->X == x && Waypoint->Y == y )
			{
				break;
			}
			Waypoint = lastWaypoints;
		}
	}
	
	calcNextDir();
	Vehicle->owner->DoScan();
	Client->gameGUI.updateMouseCursor();
	Client->gameGUI.callMiniMapDraw(); 
	Vehicle->moving = false;
	Vehicle->OffX = Vehicle->OffY = 0;

}

bool cClientMoveJob::generateFromMessage( cNetMessage *message )
{
	if ( message->iType != GAME_EV_MOVE_JOB_SERVER ) return false;
	int iCount = 0;
	int iWaypointOff;
	int iReceivedCount = message->popInt16();

	Log.write(" Client: Received MoveJob: VehicleID: " + iToStr( Vehicle->iID ) + ", SrcX: " + iToStr( ScrX ) + ", SrcY: " + iToStr( ScrY ) + ", DestX: " + iToStr( DestX ) + ", DestY: " + iToStr( DestY ) + ", WaypointCount: " + iToStr( iReceivedCount ), cLog::eLOG_TYPE_NET_DEBUG);

	// Add the waypoints
	sWaypoint *Waypoint = new sWaypoint;
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
			Waypoint->next = new sWaypoint;
			Waypoint = Waypoint->next;
		}
	}
	calcNextDir ();
	return true;
}

bool cClientMoveJob::calcPath( cList<cVehicle*> *group  )
{
	if ( ScrX == DestX && ScrY == DestY ) return false;

	cPathCalculator PathCalculator( ScrX, ScrY, DestX, DestY, Map, Vehicle, group );
	Waypoints = PathCalculator.calcPath();
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
	Log.write ( " Client: Released old movejob", cLog::eLOG_TYPE_NET_DEBUG );
	Client->addActiveMoveJob ( this );
	Log.write ( " Client: Added released movejob to avtive ones", cLog::eLOG_TYPE_NET_DEBUG );
}

void cClientMoveJob::handleNextMove( int iServerPositionX, int iServerPositionY, int iType, int iSavedSpeed )
{
	// the client is faster than the server and has already
	// reached the last field or the next will be the last,
	// then stop the vehicle
	if ( Waypoints == NULL || Waypoints->next == NULL )
	{
		Log.write ( " Client: Client has already reached the last field", cLog::eLOG_TYPE_NET_DEBUG );
		bEndForNow = true;
		Vehicle->OffX = Vehicle->OffY = 0;
	}
	else
	{
		// check whether the destination field is one of the next in the waypointlist
		// if not it must have been one that has been deleted already
		bool bServerIsFaster = false;
		sWaypoint *Waypoint = Waypoints->next->next;
		while ( Waypoint )
		{
			if ( Waypoint->X == iServerPositionX && Waypoint->Y == iServerPositionY )
			{
				bServerIsFaster = true;
				break;
			}
			Waypoint = Waypoint->next;
		}

		if ( iServerPositionX == Vehicle->PosX && iServerPositionY == Vehicle->PosY )
		{
			//the server has allready finished the current movement step
			Log.write ( " Client: Server is one field faster than client", cLog::eLOG_TYPE_NET_DEBUG );
			if ( Vehicle->moving ) doEndMoveVehicle();
		}
		else if ( iServerPositionX == Waypoints->X && iServerPositionY == Waypoints->Y )
		{
			//the server is driving towards the same field as the client. So do nothing.
		}
		else if ( bServerIsFaster )
		{
			//the server is faster than the client. So set so server position.
			Log.write ( " Client: Server is more than one field faster", cLog::eLOG_TYPE_NET_DEBUG );
			if ( Vehicle->moving ) doEndMoveVehicle();
			setVehicleToCoords( iServerPositionX, iServerPositionY );
		}
		else
		{
			//the client is more than one field faster, than the server. 
			//So wait, until the server reaches the current position.
			Log.write ( " Client: Client is faster (one or more fields) deactivating movejob; Vehicle-ID: " + iToStr ( Vehicle->iID ), cLog::eLOG_TYPE_NET_DEBUG );
			// just stop the vehicle and wait for the next commando of the server
			for ( unsigned int i = 0; i < Client->ActiveMJobs.Size(); i++ )
			{
				if ( Client->ActiveMJobs[i] == this ) Client->ActiveMJobs.Delete ( i );
			}
			if ( Vehicle->moving ) doEndMoveVehicle();
			bEndForNow = true;
			if ( iType == MJOB_OK ) return;
		}
	}

	switch ( iType )
	{
	case MJOB_OK:
		{
			if ( !Vehicle->MoveJobActive )
			{
				startMoveSound();
				Client->addActiveMoveJob( this );
			}
			if ( bEndForNow )
			{
				bEndForNow = false;
				Client->addActiveMoveJob ( Vehicle->ClientMoveJob );
				Log.write ( " Client: reactivated movejob; Vehicle-ID: " + iToStr ( Vehicle->iID ), cLog::eLOG_TYPE_NET_DEBUG );
			}
			Vehicle->MoveJobActive = true;
		}
		break;
	case MJOB_STOP:
		{
			Log.write(" Client: The movejob will end for now", cLog::eLOG_TYPE_NET_DEBUG);
			if ( Vehicle->moving ) doEndMoveVehicle();
			setVehicleToCoords( iServerPositionX, iServerPositionY );
			if ( bEndForNow ) Client->addActiveMoveJob(this);
			this->iSavedSpeed = iSavedSpeed;
			Vehicle->data.speedCur = 0;
			bSuspended = true;
			bEndForNow = true;
		}
		break;
	case MJOB_FINISHED:
		{
			Log.write(" Client: The movejob is finished", cLog::eLOG_TYPE_NET_DEBUG);
			if ( Vehicle->moving ) doEndMoveVehicle();
			setVehicleToCoords( iServerPositionX, iServerPositionY );
			release ();
		}
		break;
	case MJOB_BLOCKED:
		{
			if ( Vehicle->moving ) doEndMoveVehicle();
			setVehicleToCoords( iServerPositionX, iServerPositionY );
			Log.write(" Client: next field is blocked: DestX: " + iToStr ( Waypoints->next->X ) + ", DestY: " + iToStr ( Waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);

			if ( Vehicle->owner != Client->ActivePlayer ) 
			{
				bFinished = true;
				break;
			}
			ScrX = Vehicle->PosX;
			ScrY = Vehicle->PosY;
			if (calcPath())
			{
				sendMoveJob ( this );
			}
			else 
			{
				bFinished = true;

				if ( Vehicle == Client->gameGUI.getSelVehicle() ) 
				{				
					if ( random(2) )
						PlayVoice ( VoiceData.VOINoPath1 );
					else
						PlayVoice ( VoiceData.VOINoPath2 );
				}
			}
		}
		break;
	}
}

void cClientMoveJob::moveVehicle()
{
	if ( Vehicle == NULL || Vehicle->ClientMoveJob != this ) return;

	// do not move the vehicle, if the movejob hasn't got any more waypoints
	if ( Waypoints == NULL || Waypoints->next == NULL ) 
	{
		stopMoveSound();
		return;
	}

	if (!Vehicle->moving)
	{
		//check remaining speed
		if ( Vehicle->data.speedCur < Waypoints->next->Costs )
		{
			bSuspended = true;
			bEndForNow = true;
			stopMoveSound();
			return;
		}

		Map->moveVehicle(Vehicle, Waypoints->next->X, Waypoints->next->Y );
		Vehicle->owner->DoScan();
		Vehicle->OffX = 0;
		Vehicle->OffY = 0;
		setOffset(Vehicle, iNextDir, -64 );
		Vehicle->moving = true;

	}

	int iSpeed;
	if ( Vehicle->data.animationMovement )
	{
		Vehicle->WalkFrame++;
		if ( Vehicle->WalkFrame >= 13 ) Vehicle->WalkFrame = 1;
		iSpeed = MOVE_SPEED/2;
	}
	else if ( !(Vehicle->data.factorAir > 0) && !(Vehicle->data.factorSea > 0 && Vehicle->data.factorGround == 0) )
	{
		iSpeed = MOVE_SPEED;
		cBuilding* building = Map->fields[Waypoints->next->X+Waypoints->next->Y*Map->size].getBaseBuilding();
		if ( Waypoints && Waypoints->next && building && building->data.modifiesSpeed ) iSpeed = (int)(iSpeed/building->data.modifiesSpeed);
	}
	else if ( Vehicle->data.factorAir > 0 ) iSpeed = MOVE_SPEED*2;
	else iSpeed = MOVE_SPEED;

	// Ggf Tracks malen:
	if ( SettingsData.bMakeTracks && Vehicle->data.makeTracks && !Map->IsWater ( Vehicle->PosX+Vehicle->PosY*Map->size,false ) &&!
	        ( Waypoints && Waypoints->next && Map->terrain[Map->Kacheln[Waypoints->next->X+Waypoints->next->Y*Map->size]].water ) &&
	        ( Vehicle->owner == Client->ActivePlayer || Client->ActivePlayer->ScanMap[Vehicle->PosX+Vehicle->PosY*Map->size] ) )
	{
		if ( abs(Vehicle->OffX) == 64 || abs(Vehicle->OffY) == 64 )
		{
			switch ( Vehicle->dir )
			{
				case 0:
					Client->addFX ( fxTracks,Vehicle->PosX*64+Vehicle->OffX,Vehicle->PosY*64-10+Vehicle->OffY,0 );
					break;
				case 4:
					Client->addFX ( fxTracks,Vehicle->PosX*64+Vehicle->OffX,Vehicle->PosY*64+10+Vehicle->OffY,0 );
					break;
				case 2:
					Client->addFX ( fxTracks,Vehicle->PosX*64+10+Vehicle->OffX,Vehicle->PosY*64+Vehicle->OffY,2 );
					break;
				case 6:
					Client->addFX ( fxTracks,Vehicle->PosX*64-10+Vehicle->OffX,Vehicle->PosY*64+Vehicle->OffY,2 );
					break;
				case 1:
				case 5:
					Client->addFX ( fxTracks,Vehicle->PosX*64+Vehicle->OffX,Vehicle->PosY*64+Vehicle->OffY,1 );
					break;
				case 3:
				case 7:
					Client->addFX ( fxTracks,Vehicle->PosX*64+Vehicle->OffX,Vehicle->PosY*64+Vehicle->OffY,3 );
					break;
			}
		}
		else if ( abs ( Vehicle->OffX ) == 64-(iSpeed*2) || abs ( Vehicle->OffY ) == 64-(iSpeed*2) )
		{
			switch ( Vehicle->dir )
			{
				case 1:
					Client->addFX ( fxTracks,Vehicle->PosX*64+26+Vehicle->OffX,Vehicle->PosY*64-26+Vehicle->OffY,1 );
					break;
				case 5:
					Client->addFX ( fxTracks,Vehicle->PosX*64-26+Vehicle->OffX,Vehicle->PosY*64+26+Vehicle->OffY,1 );
					break;
				case 3:
					Client->addFX ( fxTracks,Vehicle->PosX*64+26+Vehicle->OffX,Vehicle->PosY*64+26+Vehicle->OffY,3 );
					break;
				case 7:
					Client->addFX ( fxTracks,Vehicle->PosX*64-26+Vehicle->OffX,Vehicle->PosY*64-26+Vehicle->OffY,3 );
					break;
			}
		}
	}

	setOffset(Vehicle, iNextDir, iSpeed);

	// check whether the point has been reached:
	if ( abs( Vehicle->OffX ) < iSpeed && abs( Vehicle->OffY ) < iSpeed )
	{
		Log.write(" Client: Vehicle reached the next field: ID: " + iToStr ( Vehicle->iID )+ ", X: " + iToStr ( Waypoints->next->X ) + ", Y: " + iToStr ( Waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);
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

	Vehicle->data.speedCur += iSavedSpeed;
	iSavedSpeed = 0;
	Vehicle->DecSpeed ( Waypoints->next->Costs );
	Vehicle->WalkFrame = 0;

	sWaypoint *Waypoint = Waypoints;
	Waypoints = Waypoints->next;
	Waypoint->next = lastWaypoints;
	lastWaypoints = Waypoint;


	Vehicle->moving = false;

	Vehicle->OffX = 0;
	Vehicle->OffY = 0;
	
	Client->gameGUI.callMiniMapDraw(); 
	Client->gameGUI.updateMouseCursor(); 

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
	int iIndex = -1;
#define TESTXY_DP(a,b) if( Dest.x a LastDest->x && Dest.y b LastDest->y )
	TESTXY_DP ( >,< ) iIndex = 0;
	else TESTXY_DP ( ==,< ) iIndex = 1;
	else TESTXY_DP ( <,< ) iIndex = 2;
	else TESTXY_DP ( >,== ) iIndex = 3;
	else TESTXY_DP ( <,== ) iIndex = 4;
	else TESTXY_DP ( >,> ) iIndex = 5;
	else TESTXY_DP ( ==,> ) iIndex = 6;
	else TESTXY_DP ( <,> ) iIndex = 7;

	if ( iIndex == -1 ) return;

	if ( bSpezial )
	{
		SDL_BlitSurface ( OtherData.WayPointPfeileSpecial[iIndex][64-Client->gameGUI.getTileSize()], NULL, buffer, &Dest );
	}
	else
	{
		SDL_BlitSurface ( OtherData.WayPointPfeile[iIndex][64-Client->gameGUI.getTileSize()], NULL, buffer, &Dest );
	}
}

void cClientMoveJob::startMoveSound()
{
	if ( Vehicle == Client->gameGUI.getSelVehicle()) Vehicle->StartMoveSound();
	bSoundRunning = true;
}

void cClientMoveJob::stopMoveSound()
{
	if ( !bSoundRunning ) return;

	bSoundRunning = false;

	if ( Vehicle == Client->gameGUI.getSelVehicle() )
	{
		cBuilding* building = Client->Map->fields[Vehicle->PosX+Vehicle->PosY*Client->Map->size].getBaseBuilding();
		bool water = Client->Map->IsWater ( Vehicle->PosX+Vehicle->PosY*Client->Map->size );
		if ( Vehicle->data.factorGround > 0 && building && ( building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA ) ) water = false;

		StopFXLoop ( Client->iObjectStream );
		if ( water && Vehicle->data.factorSea > 0 ) PlayFX ( Vehicle->typ->StopWater );
		else PlayFX ( Vehicle->typ->Stop );

		Client->iObjectStream = Vehicle->playStream();
	}
}
