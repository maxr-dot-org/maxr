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
#include "player.h"
#include "game.h"
#include "menu.h"
#include "eventmessages.h"

// Funktionen der Player-Klasse //////////////////////////////////////////////
cPlayer::cPlayer ( string Name,SDL_Surface *Color,int nr )
{
	int i;
	name=Name;
	color=Color;
	Nr=nr;
	// Die Vehicle Eigenschaften kopieren:
	VehicleData= ( sUnitData* ) malloc ( sizeof ( sUnitData ) *UnitsData.vehicle_anz );
	for ( i=0;i<UnitsData.vehicle_anz;i++ )
	{
		VehicleData[i]=UnitsData.vehicle[i].data;
	}
	// Die Building Eigenschaften kopieren:
	BuildingData= ( sUnitData* ) malloc ( sizeof ( sUnitData ) *UnitsData.building_anz );
	for ( i=0;i<UnitsData.building_anz;i++ )
	{
		BuildingData[i]=UnitsData.building[i].data;
	}
	DetectLandMap=NULL;
	DetectSeaMap=NULL;
	ScanMap=NULL;
	WachMapAir=NULL;
	WachMapGround=NULL;
	VehicleList=NULL;
	BuildingList=NULL;
	ResourceMap=NULL;
	ShieldMap=NULL;
	base=new cBase ( this );
	WachpostenAir=new cList<sWachposten*>;
	WachpostenGround=new cList<sWachposten*>;
	ResearchCount=0;
	UnusedResearch=0;
	Credits=0;
	ReportVehicles=new cList<sReport*>;
	ReportBuildings=new cList<sReport*>;
	ReportForschungFinished=false;
	LockList=new cList<sLockElem*>;
	iSocketNum = -1;
}

cPlayer::~cPlayer ( void )
{

	while ( WachpostenAir->iCount )
	{
		delete WachpostenAir->Items[WachpostenAir->iCount - 1];
		WachpostenAir->Delete( WachpostenAir->iCount - 1 );
	}
	delete WachpostenAir;

	while ( WachpostenGround->iCount )
	{
		delete WachpostenGround->Items[WachpostenGround->iCount - 1];
		WachpostenGround->Delete( WachpostenGround->iCount - 1 );
	}
	delete WachpostenGround;

	// Erst alle geladenen Vehicles löschen:
	cVehicle *ptr=VehicleList;
	while ( ptr )
	{
		if ( ptr->StoredVehicles&&!ptr->Loaded )
		{
			ptr->DeleteStored();
		}
		ptr=ptr->next;
	}
	// Jetzt alle Vehicles löschen:
	while ( VehicleList )
	{
		cVehicle *ptr;
		ptr=VehicleList->next;
		VehicleList->Wachposten=false;
		delete VehicleList;
		VehicleList=ptr;
	}
	while ( BuildingList )
	{
		cBuilding *ptr;
		ptr=BuildingList->next;
		BuildingList->Wachposten=false;

		// Stored Vehicles are already deleted; just clear the list
		if ( BuildingList->StoredVehicles )
		{
			while( BuildingList->StoredVehicles->iCount > 0 )
			{
				BuildingList->StoredVehicles->Delete( BuildingList->StoredVehicles->iCount - 1 );
			}
		}

		delete BuildingList;
		BuildingList=ptr;
	}
	free ( VehicleData );
	free ( BuildingData );
	if ( ScanMap ) free ( ScanMap );
	if ( WachMapAir ) free ( WachMapAir );
	if ( WachMapGround ) free ( WachMapGround );
	if ( ResourceMap ) free ( ResourceMap );
	if ( ShieldMap ) free ( ShieldMap );
	delete base;

	if ( DetectLandMap ) free ( DetectLandMap );
	if ( DetectSeaMap ) free ( DetectSeaMap );

	while ( ReportVehicles->iCount )
	{
		delete ReportVehicles->Items[0];
		ReportVehicles->Delete ( 0 );
	}
	delete ReportVehicles;
	while ( ReportBuildings->iCount )
	{
		delete ReportBuildings->Items[0];
		ReportBuildings->Delete ( 0 );
	}
	delete ReportBuildings;
	while ( LockList->iCount )
	{
		delete LockList->Items[0];
		LockList->Delete ( 0 );
	}
	delete LockList;
	
}

// Fügt ein Vehicle in die Listes des Spielser ein:
cVehicle *cPlayer::AddVehicle ( int posx,int posy,sVehicle *v )
{
	cVehicle *n;

	n=new cVehicle ( v,this );
	n->PosX=posx;
	n->PosY=posy;
	n->prev=NULL;
	if ( VehicleList!=NULL )
	{
		VehicleList->prev=n;
	}
	n->next=VehicleList;
	VehicleList=n;
	n->GenerateName();
	SpecialCircle ( n->PosX,n->PosY,n->data.scan,ScanMap );
	return n;
}

// Erstellt die Maps:
void cPlayer::InitMaps ( int MapSizeX )
{
	MapSize=MapSizeX*MapSizeX;
	// Scanner-Map:
	ScanMap= ( char* ) malloc ( MapSize );
	memset ( ScanMap,0,MapSize );
	// Ressource-Map
	ResourceMap= ( char* ) malloc ( MapSize );
	memset ( ResourceMap,0,MapSize );

	base->map=game->map;
	// Wach-Map:
	WachMapAir= ( char* ) malloc ( MapSize );
	memset ( WachMapAir,0,MapSize );
	WachMapGround= ( char* ) malloc ( MapSize );
	memset ( WachMapGround,0,MapSize );

	// Detect-Maps:
	DetectLandMap= ( char* ) malloc ( MapSize );
	memset ( DetectLandMap,0,MapSize );
	DetectSeaMap= ( char* ) malloc ( MapSize );
	memset ( DetectSeaMap,0,MapSize );

	// Die Research-Map:
	memset ( ResearchTechs,0,sizeof ( sResearch ) *8 );
	int i;
	for ( i=0;i<8;i++ )
	{
		ResearchTechs[i].MaxRounds=ResearchInits[i];
		ResearchTechs[i].RoundsRemaining=ResearchTechs[i].MaxRounds;
	}

	// Die Shield-Map:
	if ( game->AlienTech )
	{
		ShieldMap= ( char* ) malloc ( MapSize );
		memset ( ShieldMap,0,MapSize );
	}

	// ShieldColor zuweisen:
	ShieldColor=OtherData.ShieldColors[GetColorNr ( color ) ];
}

// Fügt ein Building in die Listes des Spielser ein:
cBuilding *cPlayer::AddBuilding ( int posx,int posy,sBuilding *b )
{
	cBuilding *n;

	n=new cBuilding ( b,this,base );
	n->PosX=posx;
	n->PosY=posy;
	n->prev=NULL;
	if ( BuildingList!=NULL )
	{
		BuildingList->prev=n;
	}
	n->next=BuildingList;
	BuildingList=n;
	n->GenerateName();
	if ( n->data.is_mine ) n->CheckRessourceProd();
	if ( n->data.scan )
	{
		if ( n->data.is_big ) SpecialCircleBig ( n->PosX,n->PosY,n->data.scan,ScanMap );
		else SpecialCircle ( n->PosX,n->PosY,n->data.scan,ScanMap );
	}
	if ( n->data.can_attack ) AddWachpostenB ( n );
	return n;
}

// Fügt einen Wachposten ein:
void cPlayer::AddWachpostenV ( cVehicle *v )
{
	sWachposten *n;
	n=new sWachposten;
	n->b=NULL;
	n->v=v;
	if ( v->data.can_attack==ATTACK_AIR )
	{
		WachpostenAir->Add ( n );
		SpecialCircle ( v->PosX,v->PosY,v->data.range,WachMapAir );
	}
	else if ( v->data.can_attack==ATTACK_AIRnLAND )
	{
		WachpostenAir->Add ( n );
		SpecialCircle ( v->PosX,v->PosY,v->data.range,WachMapAir );
		WachpostenGround->Add ( n );
		SpecialCircle ( v->PosX,v->PosY,v->data.range,WachMapGround );
	}
	else
	{
		WachpostenGround->Add ( n );
		SpecialCircle ( v->PosX,v->PosY,v->data.range,WachMapGround );
	}
}

// Fügt einen Wachposten ein:
void cPlayer::AddWachpostenB ( cBuilding *b )
{
	sWachposten *n;
	n=new sWachposten;
	n->b=b;
	n->v=NULL;
	if ( b->data.can_attack==ATTACK_AIR )
	{
		WachpostenAir->Add ( n );
		SpecialCircle ( b->PosX,b->PosY,b->data.range,WachMapAir );
	}
	else if ( b->data.can_attack==ATTACK_AIRnLAND )
	{
		WachpostenAir->Add ( n );
		SpecialCircle ( b->PosX,b->PosY,b->data.range,WachMapAir );
		WachpostenGround->Add ( n );
		SpecialCircle ( b->PosX,b->PosY,b->data.range,WachMapGround );
	}
	else
	{
		WachpostenGround->Add ( n );
		SpecialCircle ( b->PosX,b->PosY,b->data.range,WachMapGround );
	}
}

// Löscht einen Wachposten:
void cPlayer::DeleteWachpostenV ( cVehicle *v )
{
	sWachposten *ptr;
	int i;
	if ( v->data.can_attack==ATTACK_AIR )
	{
		for ( i=0;i<WachpostenAir->iCount;i++ )
		{
			ptr=WachpostenAir->Items[i];
			if ( ptr->v==v )
			{
				WachpostenAir->Delete ( i );
				delete ptr;
				break;
			}
		}
		RefreshWacheAir();
	}
	else if ( v->data.can_attack==ATTACK_AIRnLAND )
	{
		for ( i=0;i<WachpostenAir->iCount;i++ )
		{
			ptr=WachpostenAir->Items[i];
			if ( ptr->v==v )
			{
				WachpostenAir->Delete ( i );
				delete ptr;
				break;
			}
		}
		for ( i=0;i<WachpostenGround->iCount;i++ )
		{
			ptr=WachpostenGround->Items[i];
			if ( ptr->v==v )
			{
				WachpostenGround->Delete ( i );
				delete ptr;
				break;
			}
		}
		RefreshWacheAir();
		RefreshWacheGround();
	}
	else
	{
		for ( i=0;i<WachpostenGround->iCount;i++ )
		{
			ptr=WachpostenGround->Items[i];
			if ( ptr->v==v )
			{
				WachpostenGround->Delete ( i );
				delete ptr;
				break;
			}
		}
		RefreshWacheGround();
	}
}

// Löscht einen Wachposten:
void cPlayer::DeleteWachpostenB ( cBuilding *b )
{
	sWachposten *ptr;
	int i;
	if ( b->data.can_attack==ATTACK_AIR )
	{
		for ( i=0;i<WachpostenAir->iCount;i++ )
		{
			ptr=WachpostenAir->Items[i];
			if ( ptr->b==b )
			{
				WachpostenAir->Delete ( i );
				delete ptr;
				break;
			}
		}
		RefreshWacheAir();
	}
	else if ( b->data.can_attack==ATTACK_AIRnLAND )
	{
		for ( i=0;i<WachpostenAir->iCount;i++ )
		{
			ptr=WachpostenAir->Items[i];
			if ( ptr->b==b )
			{
				WachpostenAir->Delete ( i );
				delete ptr;
				break;
			}
		}
		for ( i=0;i<WachpostenGround->iCount;i++ )
		{
			ptr=WachpostenGround->Items[i];
			if ( ptr->b==b )
			{
				WachpostenGround->Delete ( i );
				delete ptr;
				break;
			}
		}
		RefreshWacheAir();
		RefreshWacheGround();
	}
	else
	{
		if(WachpostenGround) //FIXME: workaround since sometimes WachpostenGround is already deleted -> seg fault -- beko
		{
			for ( i=0;i<WachpostenGround->iCount;i++ )
			{
				ptr=WachpostenGround->Items[i];
				if ( ptr->b==b )
				{
					WachpostenGround->Delete ( i );
					delete ptr;
					break;
				}
			}
		}
		RefreshWacheGround();

	}
}

// Aktualisiert die Luftwache:
void cPlayer::RefreshWacheAir ( void )
{
	sWachposten *ptr;
	int i;
	memset ( WachMapAir,0,MapSize );
	for ( i=0;i<WachpostenAir->iCount;i++ )
	{
		ptr=WachpostenAir->Items[i];
		if ( ptr->v )
		{
			SpecialCircle ( ptr->v->PosX,ptr->v->PosY,ptr->v->data.range,WachMapAir );
		}
		else
		{
			SpecialCircle ( ptr->b->PosX,ptr->b->PosY,ptr->b->data.range,WachMapAir );
		}
	}
}

// Aktualisiert die Bodenwache:
void cPlayer::RefreshWacheGround ( void )
{
	sWachposten *ptr;
	int i;
	memset ( WachMapGround,0,MapSize );
	for ( i=0;i<WachpostenGround->iCount;i++ )
	{
		ptr=WachpostenGround->Items[i];
		if ( ptr->v )
		{
			SpecialCircle ( ptr->v->PosX,ptr->v->PosY,ptr->v->data.range,WachMapGround );
		}
		else
		{
			SpecialCircle ( ptr->b->PosX,ptr->b->PosY,ptr->b->data.range,WachMapGround );
		}
	}
}

// Läßt alle Objekte des Spielers Scannen:
void cPlayer::DoScan ( void )
{
	cVehicle *vp;
	cBuilding *bp;

	memset ( ScanMap,0,MapSize );
	memset ( DetectLandMap,0,MapSize );
	memset ( DetectSeaMap,0,MapSize );

	// Die Vehicle-List durchgehen:
	vp=VehicleList;
	while ( vp )
	{
		if ( vp->Loaded )
		{
			vp=vp->next;
			continue;
		}

		if ( vp->Disabled )
		{
			ScanMap[vp->PosX+vp->PosY*game->map->size]=1;
		}
		else
		{
			SpecialCircle ( vp->PosX,vp->PosY,vp->data.scan,ScanMap );

			// Detection:
			if ( vp->data.can_detect_land )
			{
				SpecialCircle ( vp->PosX,vp->PosY,vp->data.scan,DetectLandMap );
			}
			else if ( vp->data.can_detect_sea )
			{
				SpecialCircle ( vp->PosX,vp->PosY,vp->data.scan,DetectSeaMap );
			}
		}

		vp=vp->next;
	}
	// Die Building-List durchgehen:
	bp=BuildingList;
	while ( bp )
	{

		if ( bp->Disabled )
		{
			ScanMap[bp->PosX+bp->PosY*game->map->size]=1;
		}
		else
		{
			if ( bp->data.scan )
			{
				if ( bp->data.is_big ) SpecialCircleBig ( bp->PosX,bp->PosY,bp->data.scan,ScanMap );
				else SpecialCircle ( bp->PosX,bp->PosY,bp->data.scan,ScanMap );
			}
		}
		bp=bp->next;
	}
}


// Gibt das nächste Vehicle zurück, dass noch schießen/sich bewegen kann:
cVehicle *cPlayer::GetNextVehicle ( void )
{
	cVehicle *v,*start;
	bool next=false;
	if ( game->SelectedVehicle&&game->SelectedVehicle->owner==this )
	{
		start=game->SelectedVehicle;
		next=true;
	}
	else
	{
		start=VehicleList;
	}
	if ( !start ) return NULL;
	v=start;
	do
	{
		if ( !next&& ( v->data.speed||v->data.shots ) ) return v;
		next=false;
		if ( v->next )
		{
			v=v->next;
		}
		else
		{
			v=VehicleList;
		}
	}
	while ( v!=start );
	return NULL;
}

// Gibt das vorherige Vehicle zurück, dass noch schießen/sich bewegen kann:
cVehicle *cPlayer::GetPrevVehicle ( void )
{
	cVehicle *v,*start;
	bool next=false;
	if ( game->SelectedVehicle&&game->SelectedVehicle->owner==this )
	{
		start=game->SelectedVehicle;
		next=true;
	}
	else
	{
		start=VehicleList;
	}
	if ( !start ) return NULL;
	v=start;
	do
	{
		if ( !next&& ( v->data.speed||v->data.shots ) ) return v;
		next=false;
		if ( v->prev )
		{
			v=v->prev;
		}
		else
		{
			while ( v->next )
			{
				v=v->next;
			}
		}
	}
	while ( v!=start );
	return NULL;
}

// Startet eine Forschungsstation:
void cPlayer::StartAResearch ( void )
{
	ResearchCount++;
	UnusedResearch++;
}

// Stoppt eine Forschungsstation:
void cPlayer::StopAReserach ( void )
{
	int i;
	ResearchCount--;
	if ( UnusedResearch )
	{
		UnusedResearch--;
		return;
	}
	for ( i=0;i<8;i++ )
	{
		if ( ResearchTechs[i].working_on )
		{
			ResearchTechs[i].working_on--;
			break;
		}
	}
}

// Forschen am Rundenende:
void cPlayer::DoResearch ( void )
{
	bool complete=false;
	int i;
	for ( i=0;i<8;i++ )
	{
		if ( ResearchTechs[i].working_on )
		{
			ResearchTechs[i].RoundsRemaining-=ResearchTechs[i].working_on;
			if ( ResearchTechs[i].RoundsRemaining<=0 )
			{
				double a,b,c,d;
				int x;
				switch ( i )
				{
					case 0:
						sendChatMessage ( lngPack.i18n ( "Text~Comp~Research_Attack" ));
						a=0.5;
						break;
					case 1:
						sendChatMessage ( lngPack.i18n ( "Text~Comp~Research_Shoots" ) );
						a=0.5;
						break;
					case 2:
						sendChatMessage ( lngPack.i18n ( "Text~Comp~Research_Range" ) );
						a=1.0;
						break;
					case 3:
						sendChatMessage ( lngPack.i18n ( "Text~Comp~Research_Armor" ) );
						a=0.25;
						break;
					case 4:
						sendChatMessage ( lngPack.i18n ( "Text~Comp~Research_Hitpoints" ) );
						a=0.25;
						break;
					case 5:
						sendChatMessage ( lngPack.i18n ( "Text~Comp~Research_Speed" ) );
						a=0.5;
						break;
					case 6:
						sendChatMessage ( lngPack.i18n ( "Text~Comp~Research_Scan" ) );
						a=1.0;
						break;
					case 7:
						sendChatMessage ( lngPack.i18n ( "Text~Comp~Research_Costs" ) );
						a=1.0;
						break;
				}
				b=0.0000083418154;
				c=-9.4455792;
				d=6.4756817;
				ResearchTechs[i].level+=0.1;
				x=(int)(ResearchTechs[i].level*10)+1;
				ResearchTechs[i].MaxRounds= Round((a*b*pow((x-c),d)));
				ResearchTechs[i].RoundsRemaining=ResearchTechs[i].MaxRounds;
				complete=true;

				// Nun alle Sachen verbessern:
				DoTheResearch ( i );
			}
		}
	}
	if ( complete )
	{
		PlayVoice ( VoiceData.VOIResearchComplete );
		ReportForschungFinished=true;
	}
}

// Führt eine konkrete Forschung durch:
void cPlayer::DoTheResearch ( int i )
{
	int k;

	for ( k=0;k<UnitsData.vehicle_anz;k++ )
	{
		int before;
		switch ( i )
		{
			case 0:
				before=VehicleData[k].damage;
				VehicleData[k].damage-= ( int ) ( UnitsData.vehicle[k].data.damage* ( ResearchTechs[i].level-0.1 ) );
				VehicleData[k].damage+= ( int ) ( UnitsData.vehicle[k].data.damage*ResearchTechs[i].level );
				if ( VehicleData[k].damage!=before ) VehicleData[k].version++;
				break;
			case 1:
				before=VehicleData[k].max_shots;
				VehicleData[k].max_shots-= ( int ) ( UnitsData.vehicle[k].data.max_shots* ( ResearchTechs[i].level-0.1 ) );
				VehicleData[k].max_shots+= ( int ) ( UnitsData.vehicle[k].data.max_shots*ResearchTechs[i].level );
				if ( VehicleData[k].max_shots!=before ) VehicleData[k].version++;
				break;
			case 2:
				before=VehicleData[k].range;
				VehicleData[k].range-= ( int ) ( UnitsData.vehicle[k].data.range* ( ResearchTechs[i].level-0.1 ) );
				VehicleData[k].range+= ( int ) ( UnitsData.vehicle[k].data.range*ResearchTechs[i].level );
				if ( VehicleData[k].range!=before ) VehicleData[k].version++;
				break;
			case 3:
				before=VehicleData[k].armor;
				VehicleData[k].armor-= ( int ) ( UnitsData.vehicle[k].data.armor* ( ResearchTechs[i].level-0.1 ) );
				VehicleData[k].armor+= ( int ) ( UnitsData.vehicle[k].data.armor*ResearchTechs[i].level );
				if ( VehicleData[k].armor!=before ) VehicleData[k].version++;
				break;
			case 4:
				before=VehicleData[k].max_hit_points;
				VehicleData[k].max_hit_points-= ( int ) ( UnitsData.vehicle[k].data.max_hit_points* ( ResearchTechs[i].level-0.1 ) );
				VehicleData[k].max_hit_points+= ( int ) ( UnitsData.vehicle[k].data.max_hit_points*ResearchTechs[i].level );
				if ( VehicleData[k].max_hit_points!=before ) VehicleData[k].version++;
				break;
			case 5:
				before=VehicleData[k].max_speed;
				VehicleData[k].max_speed-= ( int ) ( UnitsData.vehicle[k].data.max_speed* ( ResearchTechs[i].level-0.1 ) );
				VehicleData[k].max_speed+= ( int ) ( UnitsData.vehicle[k].data.max_speed*ResearchTechs[i].level );
				if ( VehicleData[k].max_speed!=before ) VehicleData[k].version++;
				break;
			case 6:
				before=VehicleData[k].scan;
				VehicleData[k].scan-= ( int ) ( UnitsData.vehicle[k].data.scan* ( ResearchTechs[i].level-0.1 ) );
				VehicleData[k].scan+= ( int ) ( UnitsData.vehicle[k].data.scan*ResearchTechs[i].level );
				if ( VehicleData[k].scan!=before ) VehicleData[k].version++;
				break;
			case 7:
				VehicleData[k].iBuilt_Costs = Round ( UnitsData.vehicle[k].data.iBuilt_Costs * pow( 0.92, ResearchTechs[i].level * 10 ) );
				if ( VehicleData[k].iBuilt_Costs == 0 )
				{
					VehicleData[k].iBuilt_Costs = 1;
				}
				cVehicle *veh = VehicleList;
				while ( veh != NULL )
				{
					if ( veh->typ->nr == k )
					{
						veh->data.iBuilt_Costs = VehicleData[k].iBuilt_Costs;
					}
					veh = veh->next;
				}
				break;
		}
	}
	for ( k=0;k<UnitsData.building_anz;k++ )
	{
		int before;
		switch ( i )
		{
			case 0:
				before=BuildingData[k].damage;
				BuildingData[k].damage-= ( int ) ( UnitsData.building[k].data.damage* ( ResearchTechs[i].level-0.1 ) );
				BuildingData[k].damage+= ( int ) ( UnitsData.building[k].data.damage*ResearchTechs[i].level );
				if ( BuildingData[k].damage!=before ) BuildingData[k].version++;
				break;
			case 1:
				before=BuildingData[k].max_shots;
				BuildingData[k].max_shots-= ( int ) ( UnitsData.building[k].data.max_shots* ( ResearchTechs[i].level-0.1 ) );
				BuildingData[k].max_shots+= ( int ) ( UnitsData.building[k].data.max_shots*ResearchTechs[i].level );
				if ( BuildingData[k].max_shots!=before ) BuildingData[k].version++;
				break;
			case 2:
				before=BuildingData[k].range;
				BuildingData[k].range-= ( int ) ( UnitsData.building[k].data.range* ( ResearchTechs[i].level-0.1 ) );
				BuildingData[k].range+= ( int ) ( UnitsData.building[k].data.range*ResearchTechs[i].level );
				if ( BuildingData[k].range!=before ) BuildingData[k].version++;
				break;
			case 3:
				before=BuildingData[k].armor;
				BuildingData[k].armor-= ( int ) ( UnitsData.building[k].data.armor* ( ResearchTechs[i].level-0.1 ) );
				BuildingData[k].armor+= ( int ) ( UnitsData.building[k].data.armor*ResearchTechs[i].level );
				if ( BuildingData[k].armor!=before ) BuildingData[k].version++;
				break;
			case 4:
				before=BuildingData[k].max_hit_points;
				BuildingData[k].max_hit_points-= ( int ) ( UnitsData.building[k].data.max_hit_points* ( ResearchTechs[i].level-0.1 ) );
				BuildingData[k].max_hit_points+= ( int ) ( UnitsData.building[k].data.max_hit_points*ResearchTechs[i].level );
				if ( BuildingData[k].max_hit_points!=before ) BuildingData[k].version++;
				break;
			case 5:
				break;
			case 6:
				before=BuildingData[k].scan;
				BuildingData[k].scan-= ( int ) ( UnitsData.building[k].data.scan* ( ResearchTechs[i].level-0.1 ) );
				BuildingData[k].scan+= ( int ) ( UnitsData.building[k].data.scan*ResearchTechs[i].level );
				if ( BuildingData[k].scan!=before ) BuildingData[k].version++;
				break;
			case 7:
				BuildingData[k].iBuilt_Costs = Round ( UnitsData.building[k].data.iBuilt_Costs * pow( 0.92, ResearchTechs[i].level * 10 ) );
				if ( BuildingData[k].iBuilt_Costs == 0 )
				{
					BuildingData[k].iBuilt_Costs = 1;
				}
				cBuilding *bui = BuildingList;
				while ( bui != NULL )
				{
					if ( bui->typ->nr == k )
					{
						bui->data.iBuilt_Costs = BuildingData[k].iBuilt_Costs;
					}
					bui = bui->next;
				}
				break;
		}
	}
}

// Berechnet alle Schilde neu:
void cPlayer::CalcShields ( void )
{
	cBuilding *b;
	if ( !ShieldMap ) return;
	memset ( ShieldMap,0,MapSize );
	b=BuildingList;
	while ( b )
	{
		if ( b->data.max_shield&&b->data.shield )
		{
			SpecialCircleBig ( b->PosX,b->PosY,b->data.range,ShieldMap );
		}
		b=b->next;
	}
}

// Macht einen Einschlag auf ein Schild und gibt true zurück,
// wenn er aufgehalten wurde:
bool cPlayer::ShieldImpact ( int dest,int damage )
{
	cBuilding *b;
	b=BuildingList;

	// Alle Gebäude durchgehen:
	while ( b )
	{
		if ( b->data.shield&&b->IsInRange ( dest ) )
		{
			int t;
			t=damage;
			t-=b->data.shield;
			if ( t<=0 )
			{
				b->data.shield-=damage;
				damage=0;
				if ( b==game->SelectedBuilding ) b->ShowDetails();
			}
			else
			{
				damage-=b->data.shield;
				b->data.shield=0;
				if ( b==game->SelectedBuilding ) b->ShowDetails();
			}
			if ( damage==0 ) break;
		}
		b=b->next;
	}
	return damage==0;
}

// Prüft, ob der Spieler besiegt ist:
bool cPlayer::IsDefeated ( void )
{
	cBuilding *b;
	// int i;
	// if(VehicleList)return false;
	b=BuildingList;
	while ( b )
	{
		if ( !b->data.is_bridge&&
		        !b->data.is_connector&&
		        !b->data.is_expl_mine&&
		        !b->data.is_pad&&
		        !b->data.is_platform&&
		        !b->data.is_road ) return false;
		b=b->next;
	}
	return true;
}

// Fügt ein Building in die Lock-Liste ein:
void cPlayer::AddLock ( cBuilding *b )
{
	sLockElem *elem;
	elem=new sLockElem;
	elem->b=b;
	b->IsLocked=true;
	LockList->Add ( elem );
}

// Fügt ein Vehicle in die Lock-Liste ein:
void cPlayer::AddLock ( cVehicle *v )
{
	sLockElem *elem;
	elem=new sLockElem;
	elem->v=v;
	elem->b=NULL;
	v->IsLocked=true;
	LockList->Add ( elem );
}

// FLöscht ein Vehicle aus der Lock-Liste:
void cPlayer::DeleteLock ( cVehicle *v )
{
	sLockElem *elem;
	int i;
	for ( i=0;i<LockList->iCount;i++ )
	{
		elem=LockList->Items[i];
		if ( elem->v==v )
		{
			v->IsLocked=false;
			delete elem;
			LockList->Delete ( i );
			return;
		}
	}
}

// FLöscht ein Building aus der Lock-Liste:
void cPlayer::DeleteLock ( cBuilding *b )
{
	sLockElem *elem;
	int i;
	for ( i=0;i<LockList->iCount;i++ )
	{
		elem=LockList->Items[i];
		if ( elem->b==b )
		{
			b->IsLocked=false;
			delete elem;
			LockList->Delete ( i );
			return;
		}
	}
}

// Prüft, ob das Building in der Lock-Liste ist:
bool cPlayer::InLockList ( cBuilding *b )
{
	sLockElem *elem;
	int i;
	for ( i=0;i<LockList->iCount;i++ )
	{
		elem=LockList->Items[i];
		if ( elem->b==b ) return true;
	}
	return false;
}

// Prüft, ob das Vehicle in der Lock-Liste ist:
bool cPlayer::InLockList ( cVehicle *v )
{
	sLockElem *elem;
	int i;
	for ( i=0;i<LockList->iCount;i++ )
	{
		elem=LockList->Items[i];
		if ( elem->v==v ) return true;
	}
	return false;
}

// Schaltet die Lock-Objekte unter der Maus um:
void cPlayer::ToggelLock ( sGameObjects *OverObject )
{
	if ( OverObject->base&&OverObject->base->owner!=this )
	{
		if ( InLockList ( OverObject->base ) ) DeleteLock ( OverObject->base );else AddLock ( OverObject->base );
	}
	if ( OverObject->top&&OverObject->top->owner!=this )
	{
		if ( InLockList ( OverObject->top ) ) DeleteLock ( OverObject->top );else AddLock ( OverObject->top );
	}
	if ( OverObject->base&&OverObject->base->owner!=this )
	{
		if ( InLockList ( OverObject->base ) ) DeleteLock ( OverObject->base );else AddLock ( OverObject->base );
	}
	if ( OverObject->top&&OverObject->top->owner!=this )
	{
		if ( InLockList ( OverObject->top ) ) DeleteLock ( OverObject->top );else AddLock ( OverObject->top );
	}
}

// Malt alle Einträge der Lock-Liste:
void cPlayer::DrawLockList ( cHud *hud )
{
	sLockElem *elem;
	int i,spx,spy,off;

	for ( i=0;i<LockList->iCount;i++ )
	{
		elem=LockList->Items[i];
		if ( elem->v )
		{
			off=elem->v->PosX+elem->v->PosY*game->map->size;
			if ( !ScanMap[off] )
			{
				DeleteLock ( elem->v );
				i--;
				continue;
			}
			spx=elem->v->GetScreenPosX();
			spy=elem->v->GetScreenPosY();

			if ( hud->Scan )
			{
				DrawCircle ( spx+hud->Zoom/2,
				             spy+hud->Zoom/2,
				             elem->v->data.scan*hud->Zoom,SCAN_COLOR,buffer );
			}
			if ( hud->Reichweite&& ( elem->v->data.can_attack==ATTACK_LAND||elem->v->data.can_attack==ATTACK_SUB_LAND ) )
			{
				DrawCircle ( spx+hud->Zoom/2,
				             spy+hud->Zoom/2,
				             elem->v->data.range*hud->Zoom+1,RANGE_GROUND_COLOR,buffer );
			}
			if ( hud->Reichweite&&elem->v->data.can_attack==ATTACK_AIR )
			{
				DrawCircle ( spx+hud->Zoom/2,
				             spy+hud->Zoom/2,
				             elem->v->data.range*hud->Zoom+2,RANGE_AIR_COLOR,buffer );
			}
			if ( hud->Munition&&elem->v->data.can_attack )
			{
				elem->v->DrawMunBar();
			}
			if ( hud->Treffer )
			{
				elem->v->DrawHelthBar();
			}
		}
		else if ( elem->b )
		{
			off=elem->b->PosX+elem->b->PosY*game->map->size;
			if ( !ScanMap[off] )
			{
				DeleteLock ( elem->b );
				i--;
				continue;
			}
			spx=elem->b->GetScreenPosX();
			spy=elem->b->GetScreenPosY();

			if ( hud->Scan )
			{
				if ( elem->b->data.is_big )
				{
					DrawCircle ( spx+hud->Zoom,
					             spy+hud->Zoom,
					             elem->b->data.scan*hud->Zoom,SCAN_COLOR,buffer );
				}
				else
				{
					DrawCircle ( spx+hud->Zoom/2,
					             spy+hud->Zoom/2,
					             elem->b->data.scan*hud->Zoom,SCAN_COLOR,buffer );
				}
			}
			if ( hud->Reichweite&& ( elem->b->data.can_attack==ATTACK_LAND||elem->b->data.can_attack==ATTACK_SUB_LAND ) &&!elem->b->data.is_expl_mine )
			{
				DrawCircle ( spx+hud->Zoom/2,
				             spy+hud->Zoom/2,
				             elem->b->data.range*hud->Zoom+2,RANGE_GROUND_COLOR,buffer );
			}
			if ( hud->Reichweite&&elem->b->data.can_attack==ATTACK_AIR )
			{
				DrawCircle ( spx+hud->Zoom/2,
				             spy+hud->Zoom/2,
				             elem->b->data.range*hud->Zoom+2,RANGE_AIR_COLOR,buffer );
			}
			if ( hud->Reichweite&&elem->b->data.max_shield )
			{
				if ( elem->b->data.is_big )
				{
					DrawCircle ( spx+hud->Zoom,
					             spy+hud->Zoom,
					             elem->b->data.range*hud->Zoom+3,RANGE_SHIELD_COLOR,buffer );
				}
				else
				{
					DrawCircle ( spx+hud->Zoom/2,
					             spy+hud->Zoom/2,
					             elem->b->data.range*hud->Zoom+3,RANGE_SHIELD_COLOR,buffer );
				}
			}
			if ( hud->Munition&&elem->b->data.can_attack&&!elem->b->data.is_expl_mine )
			{
				elem->b->DrawMunBar();
			}
			if ( hud->Treffer )
			{
				elem->b->DrawHelthBar();
			}
		}
	}
}
