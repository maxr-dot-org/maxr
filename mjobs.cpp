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
#include <math.h>
#include "mjobs.h"
#include "game.h"
#include "sound.h"
#include "client.h"
#include "fonts.h"
#include "server.h"

// Funktionen der MJobs Klasse ///////////////////////////////////////////////
cMJobs::cMJobs ( cMap *Map, int ScrOff, int DestOff, bool Plane, int iVehicleID, cList<cPlayer*> *PlayerList, bool bServerCall )
{
	map=Map;
	finished=false;
	EndForNow=false;
	ClientMove=false;
	Suspended=false;
	BuildAtTarget=false;
	waypoints=NULL;
	SavedSpeed=0;
	plane=Plane;
	bIsServerJob = bServerCall;

	vehicle = NULL;
	for ( int i = 0; i < PlayerList->iCount && vehicle == NULL; i++ )
	{
		vehicle = PlayerList->Items[i]->VehicleList;
		while ( vehicle )
		{
			if ( vehicle->iID == iVehicleID ) break;
			vehicle = vehicle->next;
		}
	}
	if ( vehicle->mjob )
	{
		SavedSpeed = vehicle->mjob->SavedSpeed;
		vehicle->mjob->release();
		vehicle->moving = false;
		vehicle->MoveJobActive = false;
	}
	vehicle->mjob=this;
	ship=vehicle->data.can_drive==DRIVE_SEA;
	ScrX=ScrOff%map->size;
	ScrY=ScrOff/map->size;
	DestX=DestOff%map->size;
	DestY=DestOff/map->size;

	/*if ( vehicle->Wachposten )
	{
		vehicle->owner->DeleteWachpostenV ( vehicle );
		vehicle->Wachposten=false;
	}*/
}

cMJobs::~cMJobs ( void )
{
	DeleteWaypoints();
}

// Veranlasst das Löschen des Move-Jobs:
void cMJobs::release ()
{
	EndForNow = false;
	finished = true;
	if ( bIsServerJob && Server ) Server->releaseMoveJob ( this );
	else if ( Client ) Client->releaseMoveJob ( this );
}

// Berechnet den kürzesten Weg zum Ziel:
bool cMJobs::CalcPath ( void )
{
	bool ret;
	PathCalcEnds=new cList<sPathCalc*>;
	PathCalcAll=new cList<sPathCalc*>;
	// Die PathCalcMap erzeugen:
	PathCalcMap= ( char* ) malloc ( map->size*map->size );
	memset ( PathCalcMap,0,map->size*map->size );
	// Den Root erzeugen:
	FoundEnd=NULL;
	PathCalcRoot= ( sPathCalc* ) malloc ( sizeof ( sPathCalc ) );
	PathCalcRoot->prev=NULL;
	PathCalcRoot->X=ScrX;
	PathCalcRoot->Y=ScrY;
	PathCalcRoot->WayCosts=0;
	PathCalcRoot->CostsGes=PathCalcRoot->WayCosts+CalcDest ( ScrX,ScrY );
	PathCalcMap[ScrX+ScrY*map->size]=1;
	PathCalcEnds->Add( PathCalcRoot );
	PathCalcAll->Add( PathCalcRoot );
	// Den Weg finden:
	if ( ( ret=CreateNextPath() ) !=false )
	{
		sWaypoint *wp,*prev;
		// Die Liste mit den Waypoints erzeugen:
		DeleteWaypoints();
		prev=NULL;
		while ( FoundEnd )
		{
			wp= ( sWaypoint* ) malloc ( sizeof ( sWaypoint ) );
			wp->next=prev;
			wp->Costs=FoundEnd->WayCosts;
			wp->X=FoundEnd->X;
			wp->Y=FoundEnd->Y;
			prev=wp;
			FoundEnd=FoundEnd->prev;
		}
		waypoints=wp;
	}
	// Alle Punkte löschen:
	while ( PathCalcAll->iCount )
	{
		free ( PathCalcAll->Items[0] );
		PathCalcAll->Delete( 0 );
	}
	free ( PathCalcMap );
	delete PathCalcEnds;
	delete PathCalcAll;
	return ret;
}

// Berechnet die Entvernung zwischen den beiden Punkten:
int cMJobs::CalcDest ( int x,int y )
{
//  return sqrt((x-DestX)*(x-DestX)+(y-DestY)*(y-DestY));
//  return sqrt((x-DestX)*(x-DestX)+(y-DestY)*(y-DestY))*5;
	return ( abs ( x-DestX ) +abs ( y-DestY ) );
}

bool cMJobs::AddPoint ( int x,int y,float m, sPathCalc *p )
{
	sPathCalc *n;
	if ( CheckPossiblePoint ( x,y ) )
	{
		n= ( sPathCalc* ) malloc ( sizeof ( sPathCalc ) );
		n->prev=p;
		n->X=x;
		n->Y=y;
		n->WayCosts=GetWayCost ( n->X,n->Y,& ( n->road ) ) *m;
		n->CostsGes=n->WayCosts+CalcDest ( n->X,n->Y );
		PathCalcEnds->Add ( n );
		PathCalcAll->Add ( n );
		PathCalcMap[n->X+n->Y*map->size]=1;
		if ( x==DestX&&y==DestY )
		{
			FoundEnd=n;
			return true;
		}
	}
	return false;
}

// Erzeugt um den Punkt mit den geringsten Kosten die Nächsten:
bool cMJobs::CreateNextPath ( void )
{
	sPathCalc *p,*pp;
	int i,index;

	if ( PathCalcEnds->iCount==0||PathCalcAll->iCount>MAX_PATHFINDING ) return false;
	// Den Endpunkt mit den geringsten Kosten suchen:
	p=PathCalcEnds->Items[0];
	index=0;
	if ( !plane )
	{
		for ( i=0;i<PathCalcEnds->iCount;i++ )
		{
			pp=PathCalcEnds->Items[i];
			if ( pp->road )
			{
				p=pp;
				index=i;
				break;
			}
			if ( pp->CostsGes< ( p->CostsGes ) >>1 )
			{
				p=pp;
				index=i;
			}
		}
	}
	else
	{
		for ( i=0;i<PathCalcEnds->iCount;i++ )
		{
			pp=PathCalcEnds->Items[i];
			if ( pp->CostsGes< ( p->CostsGes ) >>1 )
			{
				p=pp;
				index=i;
			}
		}
	}
	// Den gefundenen Punkt aus der Endpunkteliste löschen:
	PathCalcEnds->Delete ( index );
	// Neue Knotenpunkte erzeugen:
//#define AddPoint(x,y,m) if(CheckPossiblePoint(x,y)){n=(sPathCalc*)malloc(sizeof(sPathCalc));n->prev=p;n->X=x;n->Y=y;n->WayCosts=GetWayCost(n->X,n->Y,&(n->road))*m;n->CostsGes=n->WayCosts+CalcDest(n->X,n->Y);PathCalcEnds->Add(n);PathCalcAll->AddPathCalc(n);PathCalcMap[n->X+n->Y*map->size]=1;if(x==DestX&&y==DestY){FoundEnd=n;return true;}}
//#define AddPoint(x,y,m) if(CheckPossiblePoint(x,y)){n=(sPathCalc*)malloc(sizeof(sPathCalc));n->prev=p;n->X=x;n->Y=y;n->WayCosts=GetWayCost(n->X,n->Y,&(n->road))*m;n->CostsGes=n->WayCosts+CalcDest(n->X,n->Y);PathCalcEnds->Add(n);PathCalcAll->Add(n);PathCalcMap[n->X+n->Y*map->size]=1;game->map->Kacheln[(x)+(y)*game->map->size]=game->map->DefaultWater;if(x==DestX&&y==DestY){FoundEnd=n;return true;}}
	if ( AddPoint ( p->X,p->Y-1,1,p ) ) return true;
	if ( AddPoint ( p->X+1,p->Y,1,p ) ) return true;
	if ( AddPoint ( p->X,p->Y+1,1,p ) ) return true;
	if ( AddPoint ( p->X-1,p->Y,1,p ) ) return true;
	if ( AddPoint ( p->X-1,p->Y-1,1.5,p ) ) return true;
	if ( AddPoint ( p->X+1,p->Y-1,1.5,p ) ) return true;
	if ( AddPoint ( p->X+1,p->Y+1,1.5,p ) ) return true;
	if ( AddPoint ( p->X-1,p->Y+1,1.5,p ) ) return true;

	return CreateNextPath();
}

// Prüft, ob der übergebene Punkt ein möglicher Endpunkt sein kann:
bool cMJobs::CheckPossiblePoint ( int x,int y )
{
	if ( x<0||y<0||x>=map->size||y>=map->size ) return false;
	if ( PathCalcMap[x+y*map->size] ) return false;
	if ( !plane )
	{
		if ( map->terrain[map->Kacheln[x+y*map->size]].blocked ) return false;
		if ( vehicle->data.can_drive==DRIVE_LAND && map->IsWater ( x+y*map->size ) && !( map->GO[x+y*map->size].base && ( map->GO[x+y*map->size].base->data.is_bridge || map->GO[x+y*map->size].base->data.is_platform || map->GO[x+y*map->size].base->data.is_road ) ) ) return false;
		if ( vehicle->data.can_drive==DRIVE_SEA && (!map->IsWater ( x+y*map->size,true,true ) || ( map->GO[x+y*map->size].base && ( map->GO[x+y*map->size].base->data.is_platform || map->GO[x+y*map->size].base->data.is_road ) ) ) ) return false;
	}
	return CheckPointNotBlocked ( x,y );
}

// Prüft, ob der Punkt nicht blockiert ist:
bool cMJobs::CheckPointNotBlocked ( int x,int y )
{
	if ( !vehicle->owner->ScanMap[x+y*map->size] ) return true;
	if ( !plane&& ( map->GO[x+y*map->size].vehicle||map->GO[x+y*map->size].reserviert ) ) return false;
	else if ( plane&& ( map->GO[x+y*map->size].plane||map->GO[x+y*map->size].air_reserviert ) ) return false;
	if ( !plane&&map->GO[x+y*map->size].top&&!map->GO[x+y*map->size].top->data.is_connector ) return false;
	return true;
}

// Liefert die Wegkosten der Kachel an den angegebenen Koordinaten:
int cMJobs::GetWayCost ( int x,int y,bool *road )
{
	int costs,pos;
	*road=false;
	if ( plane||ship ) return 4;
	costs=4;
	pos=x+y*map->size;
	if ( map->IsWater ( pos ) && !( map->GO[pos].base && !map->GO[pos].base->data.is_expl_mine ) ) costs=12;
else if ( ( map->GO[pos].base&& ( map->GO[pos].base->data.is_road||map->GO[pos].base->data.is_bridge ) ) || ( map->GO[pos].subbase && map->GO[pos].subbase->data.is_road ) ) {costs=2;*road=true;}
	if ( vehicle->data.can_survey&&costs>4 ) costs=4;
	return costs;
}

// Löscht die Liste mit den Waypoints:
void cMJobs::DeleteWaypoints ( void )
{
	sWaypoint *next;
	if ( !waypoints ) return;
	while ( waypoints )
	{
		next=waypoints->next;
		free ( waypoints );
		waypoints=next;
	}
	waypoints=NULL;
}

// Malt einen Pfeil für die Wegpunkte:
void cMJobs::DrawPfeil ( SDL_Rect dest,SDL_Rect *ldest,bool spezial )
{
	int index=0;
//#define TESTXY(a,b) if(ldest->x a dest.x&&ldest->y b dest.y)
#define TESTXY_DP(a,b) if(dest.x a ldest->x&&dest.y b ldest->y)
	TESTXY_DP ( >,< ) index=0;
	else TESTXY_DP ( ==,< ) index=1;
	else TESTXY_DP ( <,< ) index=2;
	else TESTXY_DP ( >,== ) index=3;
	else TESTXY_DP ( <,== ) index=4;
	else TESTXY_DP ( >,> ) index=5;
	else TESTXY_DP ( ==,> ) index=6;
	else TESTXY_DP ( <,> ) index=7;
	if ( spezial )
	{
		SDL_BlitSurface ( OtherData.WayPointPfeileSpecial[index][64-Client->Hud->Zoom],NULL,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( OtherData.WayPointPfeile[index][64-Client->Hud->Zoom],NULL,buffer,&dest );
	}
}

// Bestimmt die nächste Richtung:
void cMJobs::CalcNextDir ( void )
{
	if ( !waypoints||!waypoints->next ) return;
#define TESTXY_CND(a,b) if(waypoints->X a waypoints->next->X&&waypoints->Y b waypoints->next->Y)
	TESTXY_CND ( ==,> ) next_dir=0;
	else TESTXY_CND ( <,> ) next_dir=1;
	else TESTXY_CND ( <,== ) next_dir=2;
	else TESTXY_CND ( <,< ) next_dir=3;
	else TESTXY_CND ( ==,< ) next_dir=4;
	else TESTXY_CND ( >,< ) next_dir=5;
	else TESTXY_CND ( >,== ) next_dir=6;
	else TESTXY_CND ( >,> ) next_dir=7;
}

// Startet eine Bewegung:
void cMJobs::StartMove ( void )
{
	bool WachRange;
	if ( !vehicle||!waypoints||!waypoints->next ) return;
	WachRange=vehicle->InWachRange();
	if ( !CheckPointNotBlocked ( waypoints->next->X,waypoints->next->Y ) ||WachRange )
	{
		sWaypoint *wp;
		if ( waypoints->next->X==DestX&&waypoints->next->Y==DestY )
		{
			finished=true;
			return;
		}
		while ( waypoints )
		{
			wp=waypoints->next;
			free ( waypoints );
			waypoints=wp;
		}
		ScrX=vehicle->PosX;
		ScrY=vehicle->PosY;
		vehicle->moving=false;
		vehicle->MoveJobActive=false;
		vehicle->WalkFrame=0;

		if ( WachRange )
		{
			finished=true;
			return;
		}

		vehicle->StartMoveSound();

		vehicle->MoveJobActive=true;
		if ( !CalcPath() )
		{
			vehicle->MoveJobActive=false;
			vehicle->moving=false;
			finished=true;
			return;
		}
		CalcNextDir();
		return;
	}
	if ( vehicle->data.speed<waypoints->next->Costs )
	{
		SavedSpeed=vehicle->data.speed;
		vehicle->data.speed=0;
		EndForNow=true;
		if ( vehicle==game->SelectedVehicle ) vehicle->ShowDetails();
		return;
	}
	vehicle->moving=true;
	if ( !vehicle->MoveJobActive )
	{
		vehicle->StartMoveSound();
	}
	vehicle->MoveJobActive=true;
	game->engine->Reservieren ( waypoints->next->X,waypoints->next->Y,plane );
}

// moves the vehicle to the next field:
void cMJobs::DoTheMove ( void )
{
	int speed;
	if ( !vehicle ) return;
	if ( vehicle->data.is_human )
	{
		vehicle->WalkFrame++;
		if ( vehicle->WalkFrame >= 13 ) vehicle->WalkFrame = 0;
		speed = MOVE_SPEED/2;
	}
	else if ( !plane && !ship )
	{
		speed = MOVE_SPEED;
		if ( waypoints&&waypoints->next&&map->GO[waypoints->next->X+waypoints->next->Y*map->size].base&& ( map->GO[waypoints->next->X+waypoints->next->Y*map->size].base->data.is_road||map->GO[waypoints->next->X+waypoints->next->Y*map->size].base->data.is_bridge ) ) speed*=2;
	}
	else if ( plane ) speed = MOVE_SPEED*2;
	else speed = MOVE_SPEED;

	// Ggf Tracks malen:
	if ( SettingsData.bMakeTracks && vehicle->data.make_tracks && !map->IsWater ( vehicle->PosX+vehicle->PosY*map->size,false ) &&!
	        ( waypoints && waypoints->next && map->terrain[map->Kacheln[waypoints->next->X+waypoints->next->Y*map->size]].water ) &&
	        ( vehicle->owner == Client->ActivePlayer || Client->ActivePlayer->ScanMap[vehicle->PosX+vehicle->PosY*map->size] ) )
	{
		if ( !vehicle->OffX&&!vehicle->OffY )
		{
			switch ( vehicle->dir )
			{
				case 0:
					Client->addFX ( fxTracks,vehicle->PosX*64,vehicle->PosY*64-10,0 );
					break;
				case 4:
					Client->addFX ( fxTracks,vehicle->PosX*64,vehicle->PosY*64+10,0 );
					break;
				case 2:
					Client->addFX ( fxTracks,vehicle->PosX*64+10,vehicle->PosY*64,2 );
					break;
				case 6:
					Client->addFX ( fxTracks,vehicle->PosX*64-10,vehicle->PosY*64,2 );
					break;
				case 1:
					Client->addFX ( fxTracks,vehicle->PosX*64,vehicle->PosY*64,1 );
					break;
				case 5:
					Client->addFX ( fxTracks,vehicle->PosX*64,vehicle->PosY*64,1 );
					break;
				case 3:
					Client->addFX ( fxTracks,vehicle->PosX*64,vehicle->PosY*64,3 );
					break;
				case 7:
					Client->addFX ( fxTracks,vehicle->PosX*64,vehicle->PosY*64,3 );
					break;
			}
		}
		else if ( abs ( vehicle->OffX ) ==speed*2||abs ( vehicle->OffY ) ==speed*2 )
		{
			switch ( vehicle->dir )
			{
				case 1:
					Client->addFX ( fxTracks,vehicle->PosX*64+26,vehicle->PosY*64-26,1 );
					break;
				case 5:
					Client->addFX ( fxTracks,vehicle->PosX*64-26,vehicle->PosY*64+26,1 );
					break;
				case 3:
					Client->addFX ( fxTracks,vehicle->PosX*64+26,vehicle->PosY*64+26,3 );
					break;
				case 7:
					Client->addFX ( fxTracks,vehicle->PosX*64-26,vehicle->PosY*64-26,3 );
					break;
			}
		}
	}

	switch ( next_dir )
	{
		case 0:
			vehicle->OffY-=speed;
			break;
		case 1:
			vehicle->OffY-=speed;
			vehicle->OffX+=speed;
			break;
		case 2:
			vehicle->OffX+=speed;
			break;
		case 3:
			vehicle->OffX+=speed;
			vehicle->OffY+=speed;
			break;
		case 4:
			vehicle->OffY+=speed;
			break;
		case 5:
			vehicle->OffX-=speed;
			vehicle->OffY+=speed;
			break;
		case 6:
			vehicle->OffX-=speed;
			break;
		case 7:
			vehicle->OffX-=speed;
			vehicle->OffY-=speed;
			break;
	}

	// check whether the point has been reached:
	if ( vehicle->OffX >= 64 || vehicle->OffY >= 64 || vehicle->OffX <= -64 || vehicle->OffY <= -64 )
	{
		sWaypoint *wp;
		// Die Kosten abziehen:
		vehicle->data.speed += SavedSpeed;
		SavedSpeed = 0;
		vehicle->DecSpeed ( waypoints->next->Costs );
		vehicle->moving = false;
		vehicle->WalkFrame = 0;
		// go to the next point:
		wp = waypoints->next;
		free ( waypoints );
		waypoints = wp;
		vehicle->moving = false;

		if ( !plane )
		{
			map->GO[vehicle->PosX+vehicle->PosY*map->size].vehicle = NULL;
			map->GO[waypoints->X+waypoints->Y*map->size].vehicle = vehicle;
			map->GO[waypoints->X+waypoints->Y*map->size].reserviert = false;
		}
		else
		{
			map->GO[vehicle->PosX+vehicle->PosY*map->size].plane = NULL;
			map->GO[waypoints->X+waypoints->Y*map->size].plane = vehicle;
			map->GO[waypoints->X+waypoints->Y*map->size].air_reserviert = false;
		}
		vehicle->PosX = waypoints->X;
		vehicle->PosY = waypoints->Y;
		vehicle->OffX = 0;
		vehicle->OffY = 0;

		if ( waypoints->next == NULL )
		{
			finished = true;
		}

		vehicle->owner->DoScan();
		Client->bFlagDrawMMap = true;

		Client->mouseMoveCallback ( true );
		CalcNextDir();
	}
}
