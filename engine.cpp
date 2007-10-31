//////////////////////////////////////////////////////////////////////////////
// M.A.X. - engine.cpp
//////////////////////////////////////////////////////////////////////////////
#include "engine.h"
#include "game.h"
#include "sound.h"
#include "fonts.h"
#include "mouse.h"

// Funktionen der Engine Klasse //////////////////////////////////////////////
cEngine::cEngine ( cMap *Map,cFSTcpIp *fstcpip )
{
	map=Map;
	mjobs=NULL;
	ActiveMJobs=new TList;
	AJobs=new TList;
	this->fstcpip=fstcpip;
	EndeCount=0;
	RundenendeActionsReport=0;
	if ( fstcpip&&fstcpip->server )
	{
		SyncNo=0;
	}
	else
	{
		SyncNo=-1;
	}
	PingList=NULL;
	LogFile=NULL;
	LogHistory=NULL;
}

cEngine::~cEngine ( void )
{
	// Alle Listen und Objekte löschen:
	while ( mjobs )
	{
		cMJobs *next;
		next=mjobs->next;
		delete mjobs;
		mjobs=next;
	}
	delete ActiveMJobs;
	delete AJobs;
	if ( PingList )
	{
		while ( PingList->Count )
		{
			// delete (sPing*)(PingList->Items[0]);
			// PingList->Delete(0);
		}
		delete PingList;
	}
	StopLog();
}

// Lässt die Engine laufen:
void cEngine::Run ( void )
{
	int i;

	// Diese Aktionen nur Zeitgebunden ausführen:
	if ( !timer0 ) return;

	// Alle Move-Jobs bearbeiten:
	for ( i=0;i<ActiveMJobs->Count;i++ )
	{
		bool WasMoving,BuildAtTarget;
		cMJobs *job;
		cVehicle *v;
		job=ActiveMJobs->MJobsItems[i];
		v=job->vehicle;
		if ( v )
		{
			WasMoving=v->MoveJobActive;
		}
		else
		{
			WasMoving=false;
		}
		BuildAtTarget=job->BuildAtTarget;
		// Prüfen, ob der Job erledigt ist:
		if ( job->finished||job->EndForNow )
		{
			job->Suspended=true;
			// Den erledigten Job löschen:
			cMJobs *ptr,*last;
			if ( job->EndForNow&&v )
			{
				v->MoveJobActive=false;
				ActiveMJobs->DeleteMJobs ( i );
			}
			else
			{
				if ( v&&v->mjob==job )
				{
					v->MoveJobActive=false;
					v->mjob=NULL;
				}
				ActiveMJobs->DeleteMJobs ( i );
				ptr=mjobs;
				last=NULL;
				while ( ptr )
				{
					if ( ptr==job )
					{
						if ( !last )
						{
							mjobs=ptr->next;
						}
						else
						{
							last->next=ptr->next;
						}
						delete ptr;
						break;
					}
					last=ptr;
					ptr=ptr->next;
				}
			}
			i--;
			// Prüfen, ob ein Sound zu spielen ist:
			if ( v==game->SelectedVehicle&&WasMoving )
			{
				StopFXLoop ( game->ObjectStream );
				if ( map->IsWater ( v->PosX+v->PosY*map->size ) &&v->data.can_drive!=DRIVE_AIR )
				{
					PlayFX ( v->typ->StopWater );
				}
				else
				{
					PlayFX ( v->typ->Stop );
				}
				game->ObjectStream=v->PlayStram();
			}
			// Prüfen, ob gebaut werden soll:
			if ( BuildAtTarget )
			{
				v->IsBuilding=true;
				v->BuildRounds=v->BuildRoundsStart;
				if ( game->SelectedVehicle==v )
				{
					// Den Building Sound machen:
					StopFXLoop ( game->ObjectStream );
					game->ObjectStream=v->PlayStram();
				}
			}
			continue;
		}
		// Prüfen, ob das Vehicle gedreht werden muss:
		if ( job->next_dir!=job->vehicle->dir&&job->vehicle->data.speed )
		{
			job->vehicle->rotating=true;
			if ( timer1 )
			{
				job->vehicle->RotateTo ( job->next_dir );
			}
			continue;
		}
		else
		{
			job->vehicle->rotating=false;
		}
		// Prüfen, ob sich das Vehicle grade nicht bewegt:
		if ( !job->vehicle->moving )
		{
			job->StartMove();
			if ( job->finished ) continue;
			if ( !job->vehicle->moving ) continue;
		}
		// Das Vehicle bewegen:
		job->DoTheMove();
		game->fDrawMap=true;
	}
	// Alle Attack-Jobs bearbeiten:
	for ( i=0;i<AJobs->Count;i++ )
	{
		bool destroyed;
		cAJobs *aj;
		aj=AJobs->AJobsItems[i];
		// Prüfen, ob das Vehicle gedreht werden muss:
		if ( aj->vehicle )
		{
			if ( aj->FireDir!=aj->vehicle->dir )
			{
				aj->vehicle->rotating=true;
				aj->vehicle->RotateTo ( aj->FireDir );
				continue;
			}
			else
			{
				aj->vehicle->rotating=false;
			}
		}
		else
		{
			if ( aj->FireDir!=aj->building->dir&&! ( aj->building&&aj->building->data.is_expl_mine ) )
			{
				aj->building->RotateTo ( aj->FireDir );
				continue;
			}
		}
		// Die Mündungsfeuere Animation abspielen:
		if ( !aj->MuzzlePlayed )
		{
			aj->PlayMuzzle();
			continue;
		}
		destroyed=aj->MakeImpact();
		if ( !aj->MineDetonation )
		{
			aj->MakeClusters();
			if ( aj->vehicle ) aj->vehicle->Attacking=false;else aj->building->Attacking=false;
		}

		if ( aj->Wache&&!destroyed )
		{
			if ( aj->ScrBuilding )
			{
				cBuilding *b;
				b=map->GO[aj->scr].top;
				if ( b&&b->data.shots )
				{
					AddAttackJob ( aj->scr,aj->dest,false,aj->ScrAir,aj->DestAir,aj->ScrBuilding,true );
				}
				else
				{
					cVehicle *v;
					if ( aj->DestAir )
					{
						v=map->GO[aj->dest].plane;
					}
					else
					{
						v=map->GO[aj->dest].vehicle;
					}
					if ( v ) v->InWachRange();
				}
			}
			else if ( aj->ScrAir )
			{
				cVehicle *v;
				v=map->GO[aj->scr].plane;
				if ( v&&v->data.shots )
				{
					AddAttackJob ( aj->scr,aj->dest,false,aj->ScrAir,aj->DestAir,aj->ScrBuilding,true );
				}
				else
				{
					if ( aj->DestAir )
					{
						v=map->GO[aj->dest].plane;
					}
					else
					{
						v=map->GO[aj->dest].vehicle;
					}
					if ( v ) v->InWachRange();
				}
			}
			else
			{
				cVehicle *v;
				v=map->GO[aj->scr].vehicle;
				if ( v&&v->data.shots )
				{
					AddAttackJob ( aj->scr,aj->dest,false,aj->ScrAir,aj->DestAir,aj->ScrBuilding,true );
				}
				else
				{
					if ( aj->DestAir )
					{
						v=map->GO[aj->dest].plane;
					}
					else
					{
						v=map->GO[aj->dest].vehicle;
					}
					if ( v ) v->InWachRange();
				}
			}

		}

		delete aj;
		AJobs->DeleteAJobs ( i );
		i--;
	}
}

// Ändert den Namen eines Vehicles:
void cEngine::ChangeVehicleName ( int posx,int posy,string name,bool override,bool plane )
{
	cVehicle *v;
	if ( plane )
	{
		v=map->GO[posx+posy*map->size].plane;
	}
	else
	{
		v=map->GO[posx+posy*map->size].vehicle;
	}
	if ( v )
	{
		v->name=name;
	}
}

// Ändert den Namen eines Buildings:
void cEngine::ChangeBuildingName ( int posx,int posy,string name,bool override,bool base )
{
	cBuilding *b;
	if ( base )
	{
		b=map->GO[posx+posy*map->size].base;
	}
	else
	{
		b=map->GO[posx+posy*map->size].top;
	}
	if ( b )
	{
		b->name=name;
	}
}

// Fügt einen Bewegungs-Job ein (und gibt eine Referenz zurück). Mit ClientMove
// wird ein Client-MoveJob erstellt:
cMJobs *cEngine::AddMoveJob ( int ScrOff,int DestOff,bool ClientMove,bool plane,bool suspended )
{
	cMJobs *job;
	// Server/SP:
	job=new cMJobs ( map,ScrOff,DestOff,plane );
	job->ClientMove=ClientMove;
	job->next=mjobs;
	mjobs=job;

	if ( !job->finished )
	{
		if ( suspended )
		{
			job->Suspended=true;
		}
		else
		{
			AddActiveMoveJob ( job );
		}
		if ( job->vehicle->InWachRange() )
		{
			job->finished=true;
			return job;
		}
	}
	else
	{
		return job;
	}

	// Ggf den Bauvorgang abschließen:
	if ( job->vehicle->IsBuilding&&!job->finished )
	{
		job->vehicle->IsBuilding=false;
		job->vehicle->BuildOverride=false;
		{
			if ( job->vehicle->data.can_build==BUILD_BIG )
			{
				if ( job->vehicle->PosX!=job->vehicle->BandX||job->vehicle->PosY!=job->vehicle->BandY ) map->GO[job->vehicle->BandX+job->vehicle->BandY*map->size].vehicle=NULL;
				if ( job->vehicle->PosX!=job->vehicle->BandX+1||job->vehicle->PosY!=job->vehicle->BandY ) map->GO[job->vehicle->BandX+1+job->vehicle->BandY*map->size].vehicle=NULL;
				if ( job->vehicle->PosX!=job->vehicle->BandX+1||job->vehicle->PosY!=job->vehicle->BandY+1 ) map->GO[job->vehicle->BandX+1+ ( job->vehicle->BandY+1 ) *map->size].vehicle=NULL;
				if ( job->vehicle->PosX!=job->vehicle->BandX||job->vehicle->PosY!=job->vehicle->BandY+1 ) map->GO[job->vehicle->BandX+ ( job->vehicle->BandY+1 ) *map->size].vehicle=NULL;
				// Das Gebäude erstellen:
				AddBuilding ( job->vehicle->BandX,job->vehicle->BandY,UnitsData.building+job->vehicle->BuildingTyp,job->vehicle->owner );
			}
			else
			{
				// Das Gebäude erstellen:
				AddBuilding ( job->vehicle->PosX,job->vehicle->PosY,UnitsData.building+job->vehicle->BuildingTyp,job->vehicle->owner );
			}
		}
	}
	else
		// Oder ggf den Clearvorgang abschließen:
		if ( job->vehicle->IsClearing&&!job->finished )
		{
			job->vehicle->IsClearing=false;
			job->vehicle->BuildOverride=false;
			if ( job->vehicle->ClearBig )
			{
				if ( job->vehicle->PosX!=job->vehicle->BandX||job->vehicle->PosY!=job->vehicle->BandY ) map->GO[job->vehicle->BandX+job->vehicle->BandY*map->size].vehicle=NULL;
				if ( job->vehicle->PosX!=job->vehicle->BandX+1||job->vehicle->PosY!=job->vehicle->BandY ) map->GO[job->vehicle->BandX+1+job->vehicle->BandY*map->size].vehicle=NULL;
				if ( job->vehicle->PosX!=job->vehicle->BandX+1||job->vehicle->PosY!=job->vehicle->BandY+1 ) map->GO[job->vehicle->BandX+1+ ( job->vehicle->BandY+1 ) *map->size].vehicle=NULL;
				if ( job->vehicle->PosX!=job->vehicle->BandX||job->vehicle->PosY!=job->vehicle->BandY+1 ) map->GO[job->vehicle->BandX+ ( job->vehicle->BandY+1 ) *map->size].vehicle=NULL;
			}
			// Den Dirt löschen:
			job->vehicle->data.cargo+=map->GO[job->vehicle->PosX+job->vehicle->PosY*map->size].base->DirtValue;
			if ( job->vehicle->data.cargo>job->vehicle->data.max_cargo ) job->vehicle->data.cargo=job->vehicle->data.max_cargo;
			if ( job->vehicle==game->SelectedVehicle ) job->vehicle->ShowDetails();
			game->DeleteDirt ( map->GO[job->vehicle->PosX+job->vehicle->PosY*map->size].base );
		}

	// Die Move-Message absetzen:

	if ( job&&ClientMove&&job->vehicle->BuildPath&&job->vehicle->data.can_build==BUILD_SMALL&& ( job->vehicle->BandX!=job->vehicle->PosX||job->vehicle->BandY!=job->vehicle->PosY ) &&!job->finished&&job->vehicle->data.cargo>=job->vehicle->BuildCosts*job->vehicle->BuildRoundsStart )
	{
		job->BuildAtTarget=true;
	}

	return job;
}

// Fügt einen Movejob in die Liste der aktiven Jobs ein:
void cEngine::AddActiveMoveJob ( cMJobs *job )
{
	ActiveMJobs->AddMJobs ( job );
	job->Suspended=false;
}

// Reserviert ein Feld in der GO-Map:
void cEngine::Reservieren ( int x,int y,bool plane )
{
	if ( !plane )
	{
		map->GO[x+y*map->size].reserviert=true;
	}
	else
	{
		map->GO[x+y*map->size].air_reserviert=true;
	}
}

// Setzt ein Vehicle in der GO-Map um:
void cEngine::MoveVehicle ( int FromX,int FromY,int ToX,int ToY,bool override,bool plane )
{
	cVehicle *v;
	if ( !override&& ( ( !plane&& ( map->GO[ToX+ToY*map->size].vehicle||!map->GO[ToX+ToY*map->size].reserviert ) ) || ( plane&& ( map->GO[ToX+ToY*map->size].plane||!map->GO[ToX+ToY*map->size].air_reserviert ) ) ) ) return;
	if ( !plane )
	{
		v=map->GO[FromX+FromY*map->size].vehicle;
	}
	else
	{
		v=map->GO[FromX+FromY*map->size].plane;
	}
	if ( !v ) return;
	if ( !plane )
	{
		map->GO[FromX+FromY*map->size].vehicle=NULL;
		map->GO[ToX+ToY*map->size].vehicle=v;
		map->GO[ToX+ToY*map->size].reserviert=false;
	}
	else
	{
		map->GO[FromX+FromY*map->size].plane=NULL;
		map->GO[ToX+ToY*map->size].plane=v;
		map->GO[ToX+ToY*map->size].air_reserviert=false;
	}
	v->PosX=ToX;
	v->PosY=ToY;
	// Nur Client:
	v->OffX=0;
	v->OffY=0;

	// Ggf Minen zur Detonation bringen:
	if ( !v->data.can_detect_mines&&v->data.can_drive!=DRIVE_AIR&&map->GO[v->PosX+v->PosY*map->size].base&&map->GO[v->PosX+v->PosY*map->size].base->data.is_expl_mine&&map->GO[v->PosX+v->PosY*map->size].base->owner!=v->owner )
	{
		map->GO[v->PosX+v->PosY*map->size].base->Detonate();
		v->moving=false;
		v->WalkFrame=0;
		if ( v->mjob )
		{
			v->mjob->finished=true;
			v->mjob=NULL;
		}
		v->MoveJobActive=false;
	}

	// Nur Server (SP):
	{
		if ( v->mjob&&v->mjob->waypoints&&v->mjob->waypoints->next&&!v->mjob->Suspended )
		{
			// Das vehicle Bewegen und den nächsten Move machen:
			v->mjob->StartMove();
			if ( v->mjob )
			{
				unsigned char msg[48];
				msg[0]='#';
				msg[1]=36;
				msg[2]=MSG_MOVE_VEHICLE;
				( ( int* ) ( msg+3 ) ) [0]=FromX;
				( ( int* ) ( msg+3 ) ) [1]=FromY;
				( ( int* ) ( msg+3 ) ) [2]=ToX;
				( ( int* ) ( msg+3 ) ) [3]=ToY;
				msg[35]=v->mjob->plane;
				if ( !v->mjob->finished )
				{
					msg[36]='#';
					msg[37]=12;
					msg[38]=MSG_MOVE_TO;
					( ( int* ) ( msg+39 ) ) [0]=v->mjob->waypoints->X+v->mjob->waypoints->Y*map->size;
					( ( int* ) ( msg+39 ) ) [1]=v->mjob->waypoints->next->X+v->mjob->waypoints->next->Y*map->size;
					msg[47]=v->mjob->plane;
					// fstcpip->Send(msg,48);
				}
				else
				{
					// fstcpip->Send(msg,36);
				}
			}
		}
		else
		{
			// Nur das Vehicle bewegen:
			unsigned char msg[36];
			msg[0]='#';
			msg[1]=36;
			msg[2]=MSG_MOVE_VEHICLE;
			( ( int* ) ( msg+3 ) ) [0]=FromX;
			( ( int* ) ( msg+3 ) ) [1]=FromY;
			( ( int* ) ( msg+3 ) ) [2]=ToX;
			( ( int* ) ( msg+3 ) ) [3]=ToY;
			msg[35]=v->data.can_drive==DRIVE_AIR;
		}
	}
	// Ggf nach Rohstoffen suchen:
	if ( v->data.can_survey )
	{
		v->DoSurvey();
	}
	// Ggf beschießen lassen:
	v->InWachRange();
	// Ggf Minen suchen:
	if ( v->data.can_detect_mines&&v->owner==game->ActivePlayer )
	{
		v->DetectMines();
	}
	// Ggf Minen legen/räumen:
	if ( v->data.can_lay_mines&&v->owner==game->ActivePlayer )
	{
		if ( v->LayMines )
		{
			v->LayMine();
			if ( v->data.cargo<=0 )
			{
				v->LayMines=false;
			}
		}
		else if ( v->ClearMines )
		{
			v->ClearMine();
			if ( v->data.cargo>=v->data.max_cargo )
			{
				v->ClearMines=false;
			}
		}
	}
	// Ggf Meldung machen:
	if ( v->owner!=game->ActivePlayer&&game->ActivePlayer->ScanMap[ToX+ToY*map->size]&&!game->ActivePlayer->ScanMap[FromX+FromY*map->size]&&v->detected )
	{
		char str[50];
		sprintf ( str,"%s entdeckt",v->name.c_str() );
		game->AddCoords ( str,v->PosX,v->PosY );
		if ( random ( 2,0 ) ==0 ) PlayVoice ( VoiceData.VOIDetected1 );
		else PlayVoice ( VoiceData.VOIDetected2 );
	}
	return;
}

// Fügt ein Vehicle ein:
void cEngine::AddVehicle ( int posx,int posy,sVehicle *v,cPlayer *p,bool init,bool engine_call )
{
	cVehicle *n;
	// Das Fahrzeug erzeugen:
	n=p->AddVehicle ( posx,posy,v );
	// Das Fahrzeug platzieren:
	if ( n->data.can_drive!=DRIVE_AIR )
	{
		int off=posx+map->size*posy;
		if ( map->GO[off].vehicle==NULL ) map->GO[off].vehicle=n;
	}
	else
	{
		int off=posx+map->size*posy;
		if ( map->GO[off].plane==NULL ) map->GO[off].plane=n;
	}
	// Startup:
	if ( !init ) n->StartUp=10;
	// Ggf mit dem Gutachter scannen:
	if ( n->data.can_survey )
	{
		n->DoSurvey();
	}
	if ( !init ) n->InWachRange();
}

// Fügt ein Building ein:
void cEngine::AddBuilding ( int posx,int posy,sBuilding *b,cPlayer *p,bool init )
{
	cBuilding *n;
	int off;
	// Das Building erzeugen:
	n = p->AddBuilding ( posx,posy,b );
	// Das Building platzieren:
	if ( n->data.is_base )
	{
		map->GO[posx+map->size*posy].base = n;
	}
	else
	{
		off=posx+map->size*posy;
#define DELETE_OBJ(a,b) if(a){if(a->prev){a->prev->next=a->next;if(a->next)a->next->prev=a->prev;}else{a->owner->b=a->next;if(a->next)a->next->prev=NULL;}if(a->base)a->base->DeleteBuilding(a);delete a;}
		if ( n->data.is_big )
		{
			DELETE_OBJ ( map->GO[off].top,BuildingList )
			map->GO[off].top=n;
			if ( map->GO[off].base&&map->GO[off].base->data.is_road )
			{
				DELETE_OBJ ( map->GO[off].base,BuildingList )
				map->GO[off].base = NULL;
			}
			off++;
			DELETE_OBJ ( map->GO[off].top,BuildingList )
			map->GO[off].top=n;
			if ( map->GO[off].base&&map->GO[off].base->data.is_road )
			{
				DELETE_OBJ ( map->GO[off].base,BuildingList )
				map->GO[off].base=NULL;
			}
			off+=map->size;
			DELETE_OBJ ( map->GO[off].top,BuildingList )
			map->GO[off].top=n;
			if ( map->GO[off].base&&map->GO[off].base->data.is_road )
			{
				DELETE_OBJ ( map->GO[off].base,BuildingList )
				map->GO[off].base=NULL;
			}
			off--;
			DELETE_OBJ ( map->GO[off].top,BuildingList )
			map->GO[off].top=n;
			if ( map->GO[off].base&&map->GO[off].base->data.is_road )
			{
				DELETE_OBJ ( map->GO[off].base,BuildingList )
				map->GO[off].base=NULL;
			}
		}
		else
		{
			DELETE_OBJ ( map->GO[off].top,BuildingList )
			map->GO[off].top=n;
			if ( !n->data.is_connector&&map->GO[off].base&&map->GO[off].base->data.is_road )
			{
				DELETE_OBJ ( map->GO[off].base,BuildingList )
				map->GO[off].base=NULL;
			}
		}
#undef DELETE_OBJ
	}
	if ( !init ) n->StartUp=10;
	// Das Gebäude in die Basis integrieren:
	p->base->AddBuilding ( n );

}

// Wird aufgerufen, wenn ein Objekt zerstört werden soll:
void cEngine::DestroyObject ( int off,bool air )
{
	cVehicle *vehicle=NULL;
	cBuilding *building=NULL;
	// Das Objekt zerstören:
	if ( air ) vehicle=map->GO[off].plane;
	else
	{
		vehicle=map->GO[off].vehicle;
		if ( map->GO[off].top ) building=map->GO[off].top;
		else building=map->GO[off].base;
	}
	if ( vehicle )
	{
		if ( game->SelectedVehicle&&game->SelectedVehicle==vehicle )
		{
			vehicle->Deselct();
			game->SelectedVehicle=NULL;
		}
		if ( vehicle->mjob )
		{
			vehicle->mjob->finished=true;
			vehicle->mjob->vehicle=NULL;
		}
		if ( vehicle->prev )
		{
			cVehicle *v;
			v=vehicle->prev;
			v->next=vehicle->next;
			if ( v->next ) v->next->prev=v;
		}
		else
		{
			vehicle->owner->VehicleList=vehicle->next;
			if ( vehicle->next ) vehicle->next->prev=NULL;
		}
		// Annimation abspielen:
		{
			int nr;
			nr=random ( 3,0 );
			if ( nr==0 )
			{
				game->AddFX ( fxExploSmall0,vehicle->PosX*64+32,vehicle->PosY*64+32,0 );
			}
			else if ( nr==1 )
			{
				game->AddFX ( fxExploSmall1,vehicle->PosX*64+32,vehicle->PosY*64+32,0 );
			}
			else
			{
				game->AddFX ( fxExploSmall2,vehicle->PosX*64+32,vehicle->PosY*64+32,0 );
			}
		}
		// fahrzeug löschen:
		if ( air ) map->GO[off].plane=NULL;
		else
		{
			if ( vehicle->IsBuilding&&vehicle->data.can_build==BUILD_BIG )
			{
				map->GO[vehicle->BandX+vehicle->BandY*map->size].vehicle=NULL;
				map->GO[vehicle->BandX+1+vehicle->BandY*map->size].vehicle=NULL;
				map->GO[vehicle->BandX+1+ ( vehicle->BandY+1 ) *map->size].vehicle=NULL;
				map->GO[vehicle->BandX+ ( vehicle->BandY+1 ) *map->size].vehicle=NULL;
			}
			else
			{
				map->GO[off].vehicle=NULL;
			}

			// Ggf Mine löschen:
			if ( map->GO[off].base&&map->GO[off].base->owner&&map->GO[off].base->data.is_expl_mine )
			{
				cBuilding *building=map->GO[off].base;
				if ( building==game->SelectedBuilding ) building->Deselct();
				if ( building->prev )
				{
					cBuilding *pb;
					pb=building->prev;
					pb->next=building->next;
					if ( pb->next ) pb->next->prev=pb;
				}
				else
				{
					building->owner->BuildingList=building->next;
					if ( building->next ) building->next->prev=NULL;
				}
				map->GO[off].base=NULL;
				delete building;
			}

			// Überreste erstellen:
			if ( vehicle->data.is_human )
			{
				// Leiche erstellen:
				game->AddFX ( fxCorpse,vehicle->PosX*64,vehicle->PosY*64,0 );
			}
			else
			{
				// Dirt erstellen:
				if ( !map->GO[off].base ) game->AddDirt ( vehicle->PosX,vehicle->PosY,vehicle->data.costs/2,false );
			}
		}
		vehicle->owner->DoScan();
		delete vehicle;
		MouseMoveCallback ( true );
	}
	else if ( building&&building->owner )
	{
		if ( game->SelectedBuilding&&game->SelectedBuilding==building )
		{
			building->Deselct();
			game->SelectedBuilding=NULL;
		}
		if ( building->prev )
		{
			cBuilding *b;
			b=building->prev;
			b->next=building->next;
			if ( b->next ) b->next->prev=b;
		}
		else
		{
			building->owner->BuildingList=building->next;
			if ( building->next ) building->next->prev=NULL;
		}
		// Die Basis neu berechnen:
		if ( building->owner->base )
		{
			building->owner->base->DeleteBuilding ( building );
		}
		// Annimation abspielen:
		if ( !building->data.is_big )
		{
			int nr;
			nr=random ( 3,0 );
			if ( nr==0 )
			{
				game->AddFX ( fxExploSmall0,building->PosX*64+32,building->PosY*64+32,0 );
			}
			else if ( nr==1 )
			{
				game->AddFX ( fxExploSmall1,building->PosX*64+32,building->PosY*64+32,0 );
			}
			else
			{
				game->AddFX ( fxExploSmall2,building->PosX*64+32,building->PosY*64+32,0 );
			}
		}
		else
		{
			int nr;
			nr=random ( 4,0 );
			if ( nr==0 )
			{
				game->AddFX ( fxExploBig0,building->PosX*64+64,building->PosY*64+64,0 );
			}
			else if ( nr==1 )
			{
				game->AddFX ( fxExploBig1,building->PosX*64+64,building->PosY*64+64,0 );
			}
			else if ( nr==2 )
			{
				game->AddFX ( fxExploBig2,building->PosX*64+64,building->PosY*64+64,0 );
			}
			else
			{
				game->AddFX ( fxExploBig2,building->PosX*64+64,building->PosY*64+64,0 );
			}
		}
		// Building löschen:
		if ( map->GO[off].top )
		{
			if ( building->data.is_big )
			{
				map->GO[off].top=NULL;
				map->GO[off+1].top=NULL;
				map->GO[off+1+map->size].top=NULL;
				map->GO[off+map->size].top=NULL;
				// Dirt erstellen:
				if ( !map->GO[off].base ) game->AddDirt ( building->PosX,building->PosY,building->data.costs/2,true );
			}
			else
			{
				map->GO[off].top=NULL;
				// Dirt erstellen:
				if ( !map->GO[off].base ) game->AddDirt ( building->PosX,building->PosY,building->data.costs/2,false );
			}
		}
		else
		{
			map->GO[off].base=NULL;
			// Dirt erstellen:
			if ( !map->GO[off].base ) game->AddDirt ( building->PosX,building->PosY,building->data.costs/2,false );
		}
		building->owner->DoScan();
		delete building;
		MouseMoveCallback ( true );
	}
	game->fDrawMMap=true;
}

// Prüft, ob Jemand besiegt wurde:
void cEngine::CheckDefeat ( void )
{
	cPlayer *p;
	int i;
	string sTmpString;

	for ( i=0;i<game->PlayerList->Count;i++ )
	{
		p=game->PlayerList->PlayerItems[i];
		if ( p->IsDefeated() )
		{
			sTmpString = "Spieler " + p->name + " besiegt!";
			game->AddMessage ( (char *)sTmpString.c_str() );

			if ( game->HotSeat )
			{
				if ( p==game->ActivePlayer )
				{
					game->HotSeatPlayer++;
					if ( game->HotSeatPlayer>=game->PlayerList->Count ) game->HotSeatPlayer=0;
					game->ActivePlayer->HotHud=* ( game->hud );
					game->ActivePlayer=game->PlayerList->PlayerItems[game->HotSeatPlayer];

					delete p;
					game->PlayerList->DeletePlayer ( i );
					i--;
				}
				else
				{
					delete p;
					game->PlayerList->DeletePlayer ( i );
					i--;
				}
			}
		}
	}
}

// Fügt einen Reporteintrag in die entsprechende Liste ein:
void cEngine::AddReport ( string name,bool vehicle )
{
	struct sReport *r;
	int i;
	if ( vehicle )
	{
		for ( i=0;i<game->ActivePlayer->ReportVehicles->Count;i++ )
		{
			r=game->ActivePlayer->ReportVehicles->ReportItems[i];
			if ( !r->name.compare ( name ) )
			{
				r->anz++;
				return;
			}
		}
		r=new sReport;
		r->name=name;
		r->anz=1;
		game->ActivePlayer->ReportVehicles->AddReport ( r );
	}
	else
	{
		for ( i=0;i<game->ActivePlayer->ReportBuildings->Count;i++ )
		{
			r=game->ActivePlayer->ReportBuildings->ReportItems[i];
			if ( !r->name.compare ( name ) )
			{
				r->anz++;
				return;
			}
		}
		r=new sReport;
		r->name=name;
		r->anz=1;
		game->ActivePlayer->ReportBuildings->AddReport ( r );
	}
}

// Zeigt einen Report zum Rundenbeginn an:
void cEngine::MakeRundenstartReport ( void )
{
	struct sReport *r;
	string Report;
	string stmp;
	char sztmp[32];
	char str[100];
	int anz;
	sprintf ( str,"Beginnen Sie Runde %d",game->Runde );
	game->AddMessage ( str );

	anz=0;
	Report="";
	while ( game->ActivePlayer->ReportBuildings->Count )
	{
		r=game->ActivePlayer->ReportBuildings->ReportItems[0];
		if ( anz ) Report+=", ";
		anz+=r->anz;
		sprintf ( sztmp,"%d",r->anz ); stmp = sztmp; stmp += " "; stmp += r->name;
		Report += r->anz>1?stmp:r->name;
		delete r;
		game->ActivePlayer->ReportBuildings->DeleteReport ( 0 );
	}
	while ( game->ActivePlayer->ReportVehicles->Count )
	{
		r=game->ActivePlayer->ReportVehicles->ReportItems[0];
		if ( anz ) Report+=", ";
		anz+=r->anz;
		sprintf ( sztmp,"%d",r->anz ); stmp = sztmp; stmp += " "; stmp += r->name;
		Report+=r->anz>1?stmp:r->name;
		delete r;
		game->ActivePlayer->ReportVehicles->DeleteReport ( 0 );
	}

	if ( anz==0 )
	{
		if ( !game->ActivePlayer->ReportForschungFinished ) PlayVoice ( VoiceData.VOIStartNone );
		game->ActivePlayer->ReportForschungFinished=false;
		return;
	}
	if ( anz==1 )
	{
		Report+=" wurde fertiggestellt.";
		if ( !game->ActivePlayer->ReportForschungFinished ) PlayVoice ( VoiceData.VOIStartOne );
	}
	else
	{
		Report+=" wurden fertiggestellt.";
		if ( !game->ActivePlayer->ReportForschungFinished ) PlayVoice ( VoiceData.VOIStartMore );
	}
	game->ActivePlayer->ReportForschungFinished=false;
	game->AddMessage ( ( char * ) Report.c_str() );
}

// Bereitet das Logging vor:
void cEngine::StartLog ( void )
{
	if ( LogFile ) SDL_RWclose ( LogFile );
	LogFile=SDL_RWFromFile ( "engine.log","w" );
	if ( LogHistory )
	{
		while ( LogHistory->Count )
		{
//      delete LogHistory->Items[0];
			LogHistory->Delete ( 0 );
		}
		delete LogHistory;
	}
	LogHistory=new TList;
}

// Beendet das Logging vor:
void cEngine::StopLog ( void )
{
	if ( !LogFile ) return;
	SDL_RWclose ( LogFile );
	LogFile=NULL;
	if ( LogHistory )
	{
		while ( LogHistory->Count )
		{
//      delete (AnsiString*)(LogHistory->Items[0]);
			LogHistory->Delete ( 0 );
		}
		delete LogHistory;
	}
	LogHistory=NULL;
}

// Loggt eine Nachricht:
void cEngine::LogMessage ( string msg )
{
	if ( !LogFile ) return;

	if ( game->ShowLog )
	{
		string str;
		if ( LogHistory->Count>=54 )
		{
//      delete LogHistory->Items[0];
			LogHistory->Delete ( 0 );
		}
		LogHistory->Add ( str );
		str=msg;
	}

	SDL_RWwrite ( LogFile,msg.c_str(),1, ( int ) msg.length() );
	SDL_RWwrite ( LogFile,"\n",1,1 );

	SDL_FreeRW ( LogFile );
}

// Ändert den Namen eines Spielers:
void cEngine::ChangePlayerName ( string name )
{
	/*unsigned char msg[200];
	int len;
	if(!fstcpip)return;
	len=name.Length()+8;
	msg[0]='#';
	msg[1]=len;
	msg[2]=MSG_CHANGE_PLAYER_NAME;
	((int*)(msg+3))[0]=game->ActivePlayer->Nr;
	strcpy(msg+7,name.c_str());
	fstcpip->Send(msg,len);*/
}

// Wird aufgerufen, wenn ein Spieler Ende drückt:
void cEngine::EndePressed ( int PlayerNr )
{
	EndeCount++;
}

// Prüft das Rundenende:
void cEngine::CheckEnde ( void )
{
	// SP:
	if ( EndeCount ) Rundenende();
}

// Führt alle Rundenendenaktionen durch:
void cEngine::Rundenende ( void )
{
	int i;
	game->WantToEnd=false;
	EndeCount=0;
	if ( !game->PlayRounds )
	{
		game->hud->Ende=false;
		game->hud->EndeButton ( false );
	}
	game->Runde++;
	game->hud->ShowRunde();

	// Alle Buildings wieder aufladen:
	for ( i=0;i<game->PlayerList->Count;i++ )
	{
		bool ShieldChaned;
		cBuilding *b;
		cPlayer *p;
		p=game->PlayerList->PlayerItems[i];

		ShieldChaned=false;
		b=p->BuildingList;
		while ( b )
		{
			if ( b->Disabled )
			{
				b->Disabled--;
				if ( b->Disabled )
				{
					b=b->next;
					continue;
				}
			}
			if ( b->data.can_attack&&!game->HotSeat&&!game->PlayRounds ) b->RefreshData();
			if ( b->IsWorking&&b->data.max_shield&&b->data.shield<b->data.max_shield )
			{
				b->data.shield+=10;
				if ( b->data.shield>b->data.max_shield ) b->data.shield=b->data.max_shield;
				ShieldChaned=true;
			}
			b=b->next;
		}
		if ( ShieldChaned )
		{
			p->CalcShields();
		}
	}

	// Alle Vehicles wieder aufladen:
	for ( i=0;i<game->PlayerList->Count;i++ )
	{
		cVehicle *v;
		cPlayer *p;
		p=game->PlayerList->PlayerItems[i];

		v=p->VehicleList;
		while ( v )
		{
			if ( v->detection_override ) v->detection_override=false;
			if ( v->Disabled )
			{
				v->Disabled--;
				if ( v->Disabled )
				{
					v=v->next;
					continue;
				}
			}

			if ( !game->HotSeat&&!game->PlayRounds ) v->RefreshData();
			if ( v->mjob ) v->mjob->EndForNow=false;

			v=v->next;
		}
	}
	// Gun'em down:
	{
		for ( i=0;i<game->PlayerList->Count;i++ )
		{
			cVehicle *v;
			cPlayer *p;
			p=game->PlayerList->PlayerItems[i];

			v=p->VehicleList;
			while ( v )
			{
				v->InWachRange();
				v=v->next;
			}
		}
	}

	// Rohstoffe produzieren:
	game->ActivePlayer->base->Rundenende();

	// Forschen:
	game->ActivePlayer->DoResearch();

	if ( game->SelectedVehicle )
	{
		game->SelectedVehicle->ShowDetails();
	}
	else if ( game->SelectedBuilding )
	{
		game->SelectedBuilding->ShowDetails();
	}
	// Müll einsammeln:
	CollectTrash();

	// Ggf Autosave machen:
	if ( SettingsData.bAutoSave )
	{
		game->MakeAutosave();
	}

	CheckDefeat();

	// Meldung zum Rundenstart:
	MakeRundenstartReport();
}

// Führt alle Aktionen am Rundenende aus und gibt true zurück, wenn es was
// zu tun gab:
bool cEngine::DoEndActions ( void )
{
	cVehicle *v;
	bool todo=false;
	v=game->ActivePlayer->VehicleList;
	while ( v )
	{
		{
			// SP/Server:
			if ( v->mjob&&v->data.speed )
			{
				v->mjob->CalcNextDir();
				AddActiveMoveJob ( v->mjob );
				todo=true;
			}
		}
		v=v->next;
	}
	return todo;
}

// Prüft ob sich noch Fahrzeuge bewegen:
bool cEngine::CheckVehiclesMoving ( bool WantToEnd )
{
	return ActiveMJobs->Count>0;
}

// Sammelt den gesammten Müll ein:
void cEngine::CollectTrash ( void )
{
	cMJobs *j;
	int i;
	// Alle Movejobs einsammeln:
	while ( mjobs&&mjobs->finished )
	{
		j=mjobs->next;
		for ( i=0;i<ActiveMJobs->Count;i++ )
		{
			if ( ActiveMJobs->MJobsItems[i] == mjobs )
			{
				ActiveMJobs->DeleteMJobs ( i );
				break;
			}
		}
		if ( mjobs->vehicle )
		{
			try
			{
				mjobs->vehicle->mjob=NULL;
			}
			catch ( ... )
			{}}
		delete mjobs;
		mjobs=j;
	}
	j=mjobs;
	while ( j&&j->next )
	{
		if ( j->next->finished )
		{
			cMJobs *n;
			n=j->next->next;
			if ( j->next->vehicle&&j->next->vehicle->mjob==j->next ) j->next->vehicle->mjob=NULL;
			delete j->next;
			j->next=n;
		}
		else
		{
			j=j->next;
		}
	}
}

// Führt einen Angriff aus:
void cEngine::AddAttackJob ( int ScrOff,int DestOff,bool override,bool ScrAir,bool DestAir,bool ScrBuilding,bool Wache )
{
	if ( ScrOff==DestOff )
	{
		if ( !ScrBuilding||map->GO[ScrOff].base==NULL||map->GO[ScrOff].base->owner==NULL||!map->GO[ScrOff].base->data.is_expl_mine ) return;
	}
	cAJobs *aj;
	aj=new cAJobs ( map,ScrOff,DestOff,ScrAir,DestAir,ScrBuilding,Wache );
	AJobs->AddAJobs ( aj );
}
