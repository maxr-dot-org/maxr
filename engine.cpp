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
#include "engine.h"
#include "game.h"
#include "sound.h"
#include "fonts.h"
#include "mouse.h"
#include "networkmessages.h"

// Funktionen der Engine Klasse //////////////////////////////////////////////
cEngine::cEngine ( cMap *Map,cTCP *network )
{
	map=Map;
	mjobs=NULL;
	cAutoMJob::init(this);
	ActiveMJobs=new cList<cMJobs*>;
	AJobs=new cList<cAJobs*>;
	this->network=network;
	mutex = SDL_CreateMutex();
	EndeCount=0;
	RundenendeActionsReport=0;
	if ( network&&network->bServer )
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
	while ( AJobs->iCount )
	{
		delete AJobs->Items[AJobs->iCount - 1];
	}
	delete AJobs;
	if ( PingList )
	{
		while ( PingList->iCount )
		{
			delete PingList->Items[0];
			PingList->Delete( 0 );
		}
		delete PingList;
	}
	StopLog();
	SDL_DestroyMutex(mutex);
}

// Lässt die Engine laufen:
void cEngine::Run ( void )
{
	int i;
	// Network
	if(network)
	{
		// Look for new messages
		if ( network->iStatus == STAT_CONNECTED && network->bReceiveThreadFinished )
		{
			SDL_WaitThread ( network->TCPReceiveThread, NULL ); // free the last memory allocated by the thread. If not done so, SDL_CreateThread will hang after about 1010 successfully created threads
			network->TCPReceiveThread = SDL_CreateThread ( Receive,NULL );
		}
		// Handle incomming messages
		HandleGameMessages();
	}

	// Diese Aktionen nur Zeitgebunden ausführen:
	if ( !timer0 ) return;
	SDL_LockMutex(mutex);

	//run auto move jobs
	cAutoMJob::handleAutoMoveJobs();

	// Alle Move-Jobs bearbeiten:
	for ( i=0;i<ActiveMJobs->iCount;i++ )
	{
		bool WasMoving,BuildAtTarget;
		cMJobs *job;
		cVehicle *v;
		job=ActiveMJobs->Items[i];
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
				if( network && network->bServer )
				{
					SendIntIntBool( v->PosX + v->PosY * map->size, job->DestX + job->DestY * map->size, job->plane, MSG_END_MOVE_FOR_NOW);
					v->MoveJobActive = false;
				}
				else if( !network )
				{
					v->MoveJobActive = false;
				}
				ActiveMJobs->Delete ( i );
			}
			else
			{
				if ( v&&v->mjob==job )
				{
					if( network && network->bServer )
					{
						SendIntBool( v->PosX + v->PosY * map->size, job->plane,  MSG_END_MOVE);
						v->MoveJobActive = false;
					}
					else if( !network )
					{
						v->MoveJobActive = false;
					}
					v->mjob=NULL;
				}
				ActiveMJobs->Delete ( i );
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
				v->BuildCosts=v->BuildCostsStart;
				if ( network )
				{
					SendStartBuild(v->PosX + v->PosY * map->size, v->BuildingTyp, v->BuildRounds, v->BuildCosts, v->BandX, v->BandY, MSG_START_BUILD);
				}
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
	for ( i=0;i<AJobs->iCount;i++ )
	{
		bool destroyed;
		cAJobs *aj;
		aj=AJobs->Items[i];
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
		AJobs->Delete ( i );
		i--;
	}
	SDL_UnlockMutex(mutex);
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
		v->name = name;
		if ( network && !override )
		{
			SendChangeUnitName( posx, posy, name, plane, MSG_CHANGE_VEH_NAME );
		}
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
		b->name = name;
		if ( network && !override )
		{
			SendChangeUnitName( posx, posy, name, base, MSG_CHANGE_BUI_NAME );
		}
	}
}

// Fügt einen Bewegungs-Job ein (und gibt eine Referenz zurück). Mit ClientMove
// wird ein Client-MoveJob erstellt:
cMJobs *cEngine::AddMoveJob ( int ScrOff,int DestOff,bool ClientMove,bool plane,bool suspended )
{
	cMJobs *job;
	string sMessage;
	if( network && !network->bServer && !ClientMove)
	{
		// Client:
		SendIntIntBool(ScrOff, DestOff, plane, MSG_ADD_MOVEJOB);
		return NULL;
	}
	else
	{
		// Server/SP:
		job=new cMJobs ( map,ScrOff,DestOff,plane );
		job->ClientMove=ClientMove;
		SDL_LockMutex(mutex);
		job->next=mjobs;
		mjobs=job;
		SDL_UnlockMutex(mutex);

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
			if( !network || network->bServer || job->vehicle->owner == game->ActivePlayer )
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
		if( network && network->bServer && job->waypoints && job->waypoints->next )
		{
			SendIntIntBool( job->waypoints->X+job->waypoints->Y*map->size, job->waypoints->next->X+job->waypoints->next->Y*map->size, job->plane, MSG_MOVE_TO );
		}

		if ( job&&ClientMove&&job->vehicle->BuildPath&&job->vehicle->data.can_build==BUILD_SMALL&& ( job->vehicle->BandX!=job->vehicle->PosX||job->vehicle->BandY!=job->vehicle->PosY ) &&!job->finished&&job->vehicle->data.cargo>=job->vehicle->BuildCosts*job->vehicle->BuildRoundsStart )
		{
			job->BuildAtTarget=true;
		}
		return job;
	}
}

// Fügt einen Movejob in die Liste der aktiven Jobs ein:
void cEngine::AddActiveMoveJob ( cMJobs *job )
{
	SDL_LockMutex(mutex);
	ActiveMJobs->Add ( job );
	job->Suspended=false;
	SDL_UnlockMutex(mutex);
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
	SDL_LockMutex(mutex);
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
	// Client only:
	if( network && !network->bServer && ( v->OffX || v->OffY ) )
	{
		if( v->mjob )
		{
			v->mjob->finished = true;
			v->data.speed += v->mjob->SavedSpeed;
			v->mjob->SavedSpeed = 0;
			if( v->mjob->waypoints && v->mjob->waypoints->next )
			{
				v->DecSpeed( v->mjob->waypoints->next->Costs );
			}
			v->mjob = NULL;      
		}
		v->moving = false;
		v->rotating = false;
		v->WalkFrame = 0;
		if( game->SelectedVehicle == v ) v->ShowDetails();
		v->owner->DoScan();
		MouseMoveCallback( true );
	}
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
	
	// Ggf wieder verstecken
	if ( v->detection_override )
	{
		v->detected = false;
		v->detection_override = false;
	}

	// Server only:
	if( network && network->bServer ){
		if ( v->mjob&&v->mjob->waypoints&&v->mjob->waypoints->next&&!v->mjob->Suspended )
		{
			// Move the vehicle and make the next move:
			v->mjob->StartMove();
			if ( v->mjob )
			{
				if ( !v->mjob->finished )
				{
					SendMoveVehicle( FromX, FromY, ToX, ToY, plane);
					SendIntIntBool( v->mjob->waypoints->X+v->mjob->waypoints->Y*map->size, v->mjob->waypoints->next->X+v->mjob->waypoints->next->Y*map->size, v->mjob->plane, MSG_MOVE_TO );
				}
				else
				{
					SendMoveVehicle( FromX, FromY, ToX, ToY, plane);
				}
			}
		}
		else
		{
			// Just move the vehicle:
			SendMoveVehicle( FromX, FromY, ToX, ToY, ( v->data.can_drive == DRIVE_AIR ) );
		}
	}
	SDL_UnlockMutex(mutex);
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
	if ( v->owner != game->ActivePlayer && game->ActivePlayer->ScanMap[ToX+ToY*map->size] && !game->ActivePlayer->ScanMap[FromX+FromY*map->size] && v->detected )
	{
		string sTmp = v->name + " " + lngPack.i18n ( "Text~Comp~Detected" );
		game->AddCoords ( sTmp, v->PosX, v->PosY );
	
		if ( random ( 2, 0 ) == 0 )
		{
			PlayVoice ( VoiceData.VOIDetected1 );
		}
		else
		{
			PlayVoice ( VoiceData.VOIDetected2 );
		}
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

	if( network && !engine_call && !init )
	{
		SendAddVehicle( p->Nr, v->nr, posx, posy );
	}
}

void DeleteObject( cBuilding *Building )
{
	if( Building )
	{
		if( Building->prev )
		{
			Building->prev->next = Building->next;
			if( Building->next )
			{
				Building->next->prev = Building->prev;
			}
		}
		else
		{
			Building->owner->BuildingList = Building->next;
			if( Building->next )
			{
				Building->next->prev = NULL;
			}
		}
		if( Building->base )
		{
			Building->base->DeleteBuilding( Building );
		}
		delete Building;
	}
}

// Fügt ein Building ein:
void cEngine::AddBuilding ( int posx,int posy,sBuilding *b,cPlayer *p,bool init )
{
	cBuilding *n;
	int off;
	// Das Building erzeugen:
	n = p->AddBuilding ( posx,posy,b );
	// Das Building platzieren:
	off=posx+map->size*posy;
	if ( n->data.is_base )
	{
		if(map->GO[off].base)
		{
			map->GO[off].subbase = map->GO[off].base;
			map->GO[off].base = n;
		}
		else
		{
			map->GO[off].base = n;
		}
	}
	else
	{
		if ( n->data.is_big )
		{
			game->map->GO[off].top;
			game->map->GO[off+1].top;
			game->map->GO[off+map->size].top;
			game->map->GO[off+map->size+1].top;
			DeleteObject ( map->GO[off].top );
			map->GO[off].top=n;
			if ( map->GO[off].base&&(map->GO[off].base->data.is_road || map->GO[off].base->data.is_expl_mine) )
			{
				DeleteObject ( map->GO[off].base );
				map->GO[off].base = NULL;
			}
			off++;
			DeleteObject ( map->GO[off].top );
			map->GO[off].top=n;
			if ( map->GO[off].base&&(map->GO[off].base->data.is_road || map->GO[off].base->data.is_expl_mine) )
			{
				DeleteObject ( map->GO[off].base );
				map->GO[off].base=NULL;
			}
			off+=map->size;
			DeleteObject ( map->GO[off].top );
			map->GO[off].top=n;
			if ( map->GO[off].base&&(map->GO[off].base->data.is_road || map->GO[off].base->data.is_expl_mine) )
			{
				DeleteObject ( map->GO[off].base );
				map->GO[off].base=NULL;
			}
			off--;
			DeleteObject ( map->GO[off].top );
			map->GO[off].top=n;
			if ( map->GO[off].base&&(map->GO[off].base->data.is_road || map->GO[off].base->data.is_expl_mine) )
			{
				DeleteObject ( map->GO[off].base );
				map->GO[off].base=NULL;
			}
		}
		else
		{
			DeleteObject ( map->GO[off].top );
			map->GO[off].top=n;
			if ( !n->data.is_connector&&map->GO[off].base&&(map->GO[off].base->data.is_road || map->GO[off].base->data.is_expl_mine) )
			{
				DeleteObject ( map->GO[off].base );
				map->GO[off].base=NULL;
			}
		}
	}
	if ( !init ) n->StartUp=10;
	// Das Gebäude in die Basis integrieren:
	p->base->AddBuilding ( n );

	if( network && p == game->ActivePlayer && !init && network->bServer )
	{
		SendAddBuilding( posx, posy, b->nr, p->Nr );
	}
}

// Wird aufgerufen, wenn ein Objekt zerstört werden soll:
void cEngine::DestroyObject ( int off,bool air )
{
	cVehicle *vehicle=NULL;
	cBuilding *building=NULL;
	// Host only
	if( network && network->bServer )
	{
		SendDestroyObject(off, air);
	}
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
			if ( air )
			{
				game->AddFX ( fxExploAir,vehicle->PosX*64+32,vehicle->PosY*64+32,0 );
			}
			else if ( map->IsWater ( vehicle->PosX + vehicle->PosY * map->size ) )
			{
				game->AddFX ( fxExploWater,vehicle->PosX*64+32,vehicle->PosY*64+32,0 );
			}
			else
			{
				game->AddFX ( fxExploSmall,vehicle->PosX*64+32,vehicle->PosY*64+32,0 );
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
			if ( map->GO[off].base&&map->GO[off].base->owner )
			{
				cBuilding *building=map->GO[off].base;
				if ( building==game->SelectedBuilding )
				{
					building->Deselct();
					game->SelectedBuilding=NULL;
				}
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
				if ( !map->GO[off].base ) game->AddDirt ( vehicle->PosX,vehicle->PosY,vehicle->data.iBuilt_Costs/2,false );
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
			game->AddFX ( fxExploSmall,building->PosX*64+32,building->PosY*64+32,0 );
		}
		else
		{
			game->AddFX ( fxExploBig,building->PosX*64+64,building->PosY*64+64,0 );
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
				if ( !map->GO[off].base ) game->AddDirt ( building->PosX,building->PosY,building->data.iBuilt_Costs/2,true );
			}
			else
			{
				map->GO[off].top=NULL;
				// Dirt erstellen:
				if ( !map->GO[off].base ) game->AddDirt ( building->PosX,building->PosY,building->data.iBuilt_Costs/2,false );
			}
		}
		else
		{
			if(map->GO[off].subbase)
			{
				map->GO[off].base = map->GO[off].subbase;
				map->GO[off].subbase = NULL;
			}
			else
			{
				map->GO[off].base=NULL;
			}
			// Dirt erstellen:
			if ( !map->GO[off].base ) game->AddDirt ( building->PosX,building->PosY,building->data.iBuilt_Costs/2,false );
		}
		building->owner->DoScan();
		delete building;
		MouseMoveCallback ( true );
	}
	game->fDrawMMap=true;
}

// Starts pinging
void cEngine::Ping( void )
{
	if( !network ) return;

	PingList = new cList<sPing*>;
	if( network->bServer )
	{
		for( int i = 0 ; i < game->PlayerList->iCount ; i++ )
		{
			sPing *Ping;
			cPlayer *Player;
			Player = game->PlayerList->Items[i];
			if( Player == game->ActivePlayer ) continue;
			Ping = new sPing;
			memset( Ping, 0, sizeof( sPing ) );
			Ping->PlayerID = Player->Nr;
			PingList->Add( Ping );
		}
	}
	else
	{
		sPing *Ping;
		Ping = new sPing;
		memset( Ping, 0, sizeof( sPing ) );
		PingList->Add( Ping );
	}

	PingStart = SDL_GetTicks();
	for( int i = 0 ; i < PING_COUNT ; i++ )
	{
		network->TCPSend( MSG_PING, iToStr( i ).c_str() );
		if( i % 10 == 0 )
		{
			SDL_Delay( 10 );
		}
	}
}

// Prüft, ob Jemand besiegt wurde:
void cEngine::CheckDefeat ( void )
{
	cPlayer *p;
	int i;
	string sTmpString;

	for ( i=0;i<game->PlayerList->iCount;i++ )
	{
		p=game->PlayerList->Items[i];
		if ( p->IsDefeated() )
		{
			sTmpString = lngPack.i18n( "Text~Multiplayer~Player") + " ";
			sTmpString += p->name + " ";
			sTmpString += lngPack.i18n( "Text~Comp~Defeated") + "!";
			game->AddMessage ( sTmpString );

			if ( game->HotSeat )
			{
				if ( p==game->ActivePlayer )
				{
					game->HotSeatPlayer++;
					if ( game->HotSeatPlayer>=game->PlayerList->iCount ) game->HotSeatPlayer=0;
					game->ActivePlayer->HotHud=* ( game->hud );
					game->ActivePlayer=game->PlayerList->Items[game->HotSeatPlayer];

					delete p;
					game->PlayerList->Delete ( i );
					i--;
				}
				else
				{
					delete p;
					game->PlayerList->Delete ( i );
					i--;
				}
			}
			else if( network )
			{
				if ( p == game->ActivePlayer )
				{
					network->TCPSend( MSG_HOST_DEFEAT, "" );

					cList<cPlayer *> *save = game->PlayerList;
					for( int k = 0; k < save->iCount; k++ )
					{
						cPlayer *pp;
						pp = save->Items[k];
						if ( pp == p ) continue;
						game->PlayerList->Add( pp );
					}
					// TOFIX: Gamesave 100 will be overwritten!
					game->Save( "HostDefeat.sav", 100 );
					delete game->PlayerList;
					game->PlayerList = save;
				}
				else
				{
					network->TCPSend( MSG_PLAYER_DEFEAT, iToStr( p->Nr ).c_str() );

					delete p;
					game->PlayerList->Delete( i );
					i--;
					network->iMin_clients--;
				}
			}
		}
	}
}

// Fügt einen Reporteintrag in die entsprechende Liste ein:
void cEngine::AddReport ( string name,bool vehicle )
{
	sReport *r;
	int i;
	if ( vehicle )
	{
		for ( i=0;i<game->ActivePlayer->ReportVehicles->iCount;i++ )
		{
			r=game->ActivePlayer->ReportVehicles->Items[i];
			if ( !r->name.compare ( name ) )
			{
				r->anz++;
				return;
			}
		}
		r=new sReport;
		r->name=name;
		r->anz=1;
		game->ActivePlayer->ReportVehicles->Add ( r );
	}
	else
	{
		for ( i=0;i<game->ActivePlayer->ReportBuildings->iCount;i++ )
		{
			r=game->ActivePlayer->ReportBuildings->Items[i];
			if ( !r->name.compare ( name ) )
			{
				r->anz++;
				return;
			}
		}
		r=new sReport;
		r->name=name;
		r->anz=1;
		game->ActivePlayer->ReportBuildings->Add ( r );
	}
}

// Zeigt einen sReport zum Rundenbeginn an:
void cEngine::MakeRundenstartReport ( void )
{
	sReport *r;
	string sReportMsg = "";
	string stmp = "";
	string sTmp = lngPack.i18n( "Text~Comp~Turn_Start") + " " + iToStr(game->Runde);
	game->AddMessage(sTmp);
	int anz = 0;
	
	while ( game->ActivePlayer->ReportBuildings->iCount )
	{
		r=game->ActivePlayer->ReportBuildings->Items[0];
		if ( anz ) sReportMsg+=", ";
		anz+=r->anz;
		stmp = iToStr(r->anz) + " " + r->name;
		sReportMsg += r->anz>1?stmp:r->name;
		delete r;
		game->ActivePlayer->ReportBuildings->Delete ( 0 );
	}
	while ( game->ActivePlayer->ReportVehicles->iCount )
	{
		r=game->ActivePlayer->ReportVehicles->Items[0];
		if ( anz ) sReportMsg+=", ";
		anz+=r->anz;
		stmp = iToStr(r->anz) + " " + r->name;
		sReportMsg+=r->anz>1?stmp:r->name;
		delete r;
		game->ActivePlayer->ReportVehicles->Delete ( 0 );
	}

	if ( anz==0 )
	{
		if ( !game->ActivePlayer->ReportForschungFinished ) PlayVoice ( VoiceData.VOIStartNone );
		game->ActivePlayer->ReportForschungFinished=false;
		return;
	}
	if ( anz==1 )
	{
		sReportMsg+=" "+lngPack.i18n( "Text~Comp~Finished") +".";
		if ( !game->ActivePlayer->ReportForschungFinished ) PlayVoice ( VoiceData.VOIStartOne );
	}
	else
	{
		sReportMsg+=" "+lngPack.i18n( "Text~Comp~Finished2") +".";
		if ( !game->ActivePlayer->ReportForschungFinished ) PlayVoice ( VoiceData.VOIStartMore );
	}
	game->ActivePlayer->ReportForschungFinished=false;
	game->AddMessage ( sReportMsg );
}

// Bereitet das Logging vor:
void cEngine::StartLog ( void )
{
	if ( LogFile ) SDL_RWclose ( LogFile );
	LogFile=SDL_RWFromFile ( "engine.log","w" );
	if ( LogHistory )
	{
		delete LogHistory;
	}
	LogHistory=new cList<string>;
}

// Beendet das Logging vor:
void cEngine::StopLog ( void )
{
	if ( !LogFile ) return;
	SDL_RWclose ( LogFile );
	LogFile=NULL;
	if ( LogHistory )
	{
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
		if ( LogHistory->iCount>=54 )
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

// Wird aufgerufen, wenn ein Spieler Ende drückt:
void cEngine::EndePressed ( int PlayerNr )
{
	EndeCount++;
	if ( network )
	{
		network->TCPSend( MSG_ENDE_PRESSED, iToStr ( PlayerNr ).c_str() );

		if ( network->bServer && game->PlayRounds )
		{
			int i,next;
			cBuilding *b;
			cVehicle *v;
			cPlayer *p;
			for ( i = 0; i < game->PlayerList->iCount; i++ )
			{
				p = game->PlayerList->Items[i];
				if ( p->Nr == game->ActiveRoundPlayerNr )
				{
					if ( i < game->PlayerList->iCount - 1 )
					{
						p = game->PlayerList->Items[i+1];
					}
					else
					{
						p = game->PlayerList->Items[0];
					}
					next=p->Nr;
					if(p!=game->ActivePlayer)
					{
						game->AddMessage( p->name + lngPack.i18n ( "Text~Multiplayer~Player_Turn" ) );
					}
					break;
				}
			}

			v = p->VehicleList;
			while ( v )
			{
				v->RefreshData();
				v = v->next;
			}
			b = p->BuildingList;
			while ( b )
			{
				if( b->data.can_attack ) b->RefreshData();
				b = b->next;
			}
			network->TCPSend( MSG_PLAY_ROUNDS_NEXT,iToStr ( next ).c_str() );
			game->ActiveRoundPlayerNr = next;
		}
	}
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
	if ( !game->PlayRounds || ( network && network->bServer ) )
	{
		game->hud->Ende = false;
		game->hud->EndeButton ( false );
	}
	game->Runde++;
	game->hud->ShowRunde();

	// Alle Buildings wieder aufladen:
	for ( i=0;i<game->PlayerList->iCount;i++ )
	{
		bool ShieldChaned;
		cBuilding *b;
		cPlayer *p;
		p=game->PlayerList->Items[i];

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
	for ( i=0;i<game->PlayerList->iCount;i++ )
	{
		cVehicle *v;
		cPlayer *p;
		p=game->PlayerList->Items[i];

		v=p->VehicleList;
		while ( v )
		{
			if ( v->detection_override && v->owner == game->ActivePlayer )
			{
				v->detected = false;
				v->detection_override = false;
			}
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
	if( !network || network->bServer )
	{
		for ( i=0;i<game->PlayerList->iCount;i++ )
		{
			cVehicle *v;
			cPlayer *p;
			p=game->PlayerList->Items[i];

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

	// Sync if necessary:
	if( network && network->bServer )
	{
		int iLastMouseX = -1, iLastMouseY = -1;
		// Wait until all clients have finished their round
		while( RundenendeActionsReport < game->PlayerList->iCount -1 )
		{
			SDL_PumpEvents();
			mouse->GetPos();

			game->HandleTimer();
			Run();

			if( mouse->x != iLastMouseX || mouse->y != iLastMouseY )
			{
				mouse->draw( true, screen );
			}
			iLastMouseX = mouse->x;
			iLastMouseY = mouse->y;
			SDL_Delay( 1 );
		}
		RundenendeActionsReport = 0;

		// Request resync
		network->TCPSend ( MSG_START_SYNC, iToStr ( SyncNo ).c_str() );
		font->showTextCentered( 320, 235, lngPack.i18n( "Text~Multiplayer~Network_Syncing"), LATIN_BIG, buffer );
		SHOW_SCREEN
		mouse->MoveCallback = false;
		mouse->SetCursor( CHand );
		mouse->draw( false, screen );
		SyncWaiting = network->iMin_clients;

		int iC = 0;
		while( SyncWaiting > 0 && network->GetConnectionCount() == network->iMin_clients )
		{
			SDL_PumpEvents();
			mouse->GetPos();

			game->HandleTimer();
			// Send sync again from time to time
			if( timer2 )
			{
				iC++;
				if( iC >= 20 )
				{
					network->TCPSend ( MSG_START_SYNC, iToStr ( SyncNo ).c_str() );
					iC = 0;
				}
			}
			if( mouse->x != iLastMouseX || mouse->y != iLastMouseY )
			{
				mouse->draw( true, screen );
			}
			iLastMouseX = mouse->x;
			iLastMouseY = mouse->y;
			HandleGameMessages();
			SDL_Delay(1);
		}

		mouse->MoveCallback = true;
		SyncNo++;
	}

	if ( SettingsData.bAutoSave )
	{
		game->MakeAutosave();
	}

	CheckDefeat();

	if( network && !network->bServer )
	{
		network->TCPSend( MSG_REPORT_R_E_A, "" );
	}

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
		if ( network && !network->bServer )
		{
			// Client:
			if( v->mjob && v->data.speed && !v->MoveJobActive && !v->moving && !v->mjob->finished )
			{
				if( v->mjob->Suspended && v->data.speed > 0 ) v->mjob->Suspended = false;
				if( !v->mjob->Suspended && v->mjob->waypoints && v->mjob->waypoints->next )
				{
					v->mjob->finished=true;
					v->mjob=NULL;
					v->MoveJobActive=false;

					SendIntBool( v->PosX + v->PosY * map->size, ( v->data.can_drive == DRIVE_AIR ), MSG_ERLEDIGEN);

					todo=true;          
				}
			}
		}
		else
		{
			// SP/Host:
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
	return ActiveMJobs->iCount>0;
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
		for ( i=0;i<ActiveMJobs->iCount;i++ )
		{
			if ( ActiveMJobs->Items[i] == mjobs )
			{
				ActiveMJobs->Delete ( i );
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
	// Host / SP
	if( !network || network->bServer || override )
	{
		cAJobs *aj;
		aj=new cAJobs ( map,ScrOff,DestOff,ScrAir,DestAir,ScrBuilding,Wache );
		AJobs->Add ( aj );
		if( network && network->bServer )
		{
			SendAddAtackJob(ScrOff, DestOff, ScrAir, DestAir, ScrBuilding);
		}
	}
	//Client
	else
	{
		SendAddAtackJob(ScrOff, DestOff, ScrAir, DestAir, ScrBuilding);
	}
}

// Empfängt eine Nachricht aus dem Netzwerk:
void cEngine::HandleGameMessages()
{
	cNetMessage *msg;
	string sMsgString;
	for ( int iNum = 0; iNum < network->NetMessageList->iCount; iNum++ )
	{
		msg = network->NetMessageList->Items[iNum];
		sMsgString = ( char * ) msg->msg;
		switch( msg->typ )
		{
			// Chatmessages:
			case MSG_CHAT:
			{
				game->AddMessage( sMsgString );
				PlayFX( SoundData.SNDChat );
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// Add movejob:
			case MSG_ADD_MOVEJOB:
			{
				cMJobs *job;
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				job = AddMoveJob( atoi( Strings->Items[0].c_str() ),atoi( Strings->Items[1].c_str() ),false,atoi( Strings->Items[2].c_str() ) );
				delete Strings;
				// Check if path is barred:
				if(job->finished)
				{
					network->TCPSend(MSG_NO_PATH,"");
				}
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// Move vehicle:
			case MSG_MOVE_VEHICLE:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				MoveVehicle ( atoi( Strings->Items[0].c_str() ),atoi( Strings->Items[1].c_str() ),atoi( Strings->Items[2].c_str() ),atoi( Strings->Items[3].c_str() ),true,atoi( Strings->Items[4].c_str() ) );
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// Move vehicle for a field:
			case MSG_MOVE_TO:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				AddMoveJob( atoi( Strings->Items[0].c_str() ),atoi( Strings->Items[1].c_str() ),true,atoi( Strings->Items[2].c_str() ));
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// Path is barred:
			case MSG_NO_PATH:
			{
				if( random( 1,0 ) )
				{
					PlayVoice(VoiceData.VOINoPath1);
				}
				else
				{
					PlayVoice(VoiceData.VOINoPath2);
				}
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// End of a movejobs:
			case MSG_END_MOVE:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				cVehicle *v;
				if( atoi ( Strings->Items[1].c_str() ) == 0 )
				{
					v=map->GO[atoi ( Strings->Items[0].c_str() )].vehicle;
				}
				else
				{
					v=map->GO[atoi ( Strings->Items[0].c_str() )].plane;        
				}
				delete Strings;
				if( v )
				{
					v->MoveJobActive=false;
					if( v == game->SelectedVehicle )
					{
						StopFXLoop( game->ObjectStream );
						if( map->IsWater( v->PosX + v->PosY * map->size) && v->data.can_drive != DRIVE_AIR )
						{
							PlayFX( v->typ->StopWater );
						}
						else
						{
							PlayFX( v->typ->Stop );
						}
						game->ObjectStream = v->PlayStram();
					}
				}
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// changes vehicle name
			case MSG_CHANGE_VEH_NAME:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				ChangeVehicleName( atoi( Strings->Items[0].c_str() ), atoi( Strings->Items[1].c_str() ), Strings->Items[2], true, atoi( Strings->Items[3].c_str() ));
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// end of movejob for current turn
			case MSG_END_MOVE_FOR_NOW:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				cVehicle *v;
				if( atoi( Strings->Items[2].c_str() ) == 0 )
				{
					v = map->GO[atoi( Strings->Items[0].c_str() ) ].vehicle;
				}
				else
				{
					v = map->GO[atoi( Strings->Items[0].c_str() ) ].plane;
				}
				
				if( !v )
				{
					delete Strings;
					delete network->NetMessageList->Items[iNum];
					network->NetMessageList->Delete ( iNum );
					break;
				}
				if( v->owner == game->ActivePlayer )
				{
					AddMoveJob(v->PosX + v->PosY * map->size,atoi( Strings->Items[1].c_str() ),true,atoi( Strings->Items[2].c_str() ) );
				}
				delete Strings;

				v->MoveJobActive = false;
				if( v == game->SelectedVehicle )
				{
					StopFXLoop( game->ObjectStream );
					if( map->IsWater( v->PosX + v->PosY * map->size ) && v->data.can_drive != DRIVE_AIR )
					{
						PlayFX( v->typ->StopWater );
					}
					else
					{
						PlayFX( v->typ->Stop );
					}
					game->ObjectStream = v->PlayStram();
				}
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// changes name of player
			case MSG_CHANGE_PLAYER_NAME:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				for( int i = 0; i < game->PlayerList->iCount; i++ )
				{
					cPlayer *Player;
					Player = game->PlayerList->Items[i];
					if( Player->Nr == atoi( Strings->Items[0].c_str() ) )
					{
						Player->name = Strings->Items[1];
						break;
					}
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// notification of pressed end-turn button:
			case MSG_ENDE_PRESSED:
			{
				if( atoi ( sMsgString.c_str() ) == game->ActivePlayer->Nr )
				{
					delete network->NetMessageList->Items[iNum];
					network->NetMessageList->Delete ( iNum );
					break;
				}
				if( network->bServer )
				{
					EndePressed ( atoi ( sMsgString.c_str() ) );
				}
				else
				{
					EndeCount++;
				}
				for ( int k = 0 ; k < game->PlayerList->iCount; k++)
				{
					cPlayer *p;
					p = game->PlayerList->Items[k];
					if ( p->Nr == atoi ( sMsgString.c_str() ) )
					{
						game->AddMessage( lngPack.i18n ( "Text~Multiplayer~Player" ) + " " + p->name + lngPack.i18n ( "Text~Multiplayer~Player_Turn_End" ));
						break;
					}
				}
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// notification of canceling a movejob:
			case MSG_MJOB_STOP:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				cVehicle *v;
				if( atoi( Strings->Items[1].c_str() ) == 0)
				{
					v = map->GO[atoi( Strings->Items[0].c_str() )].vehicle;
				}else{
					v = map->GO[atoi( Strings->Items[0].c_str() )].plane;        
				}
				delete Strings;
				if( v && v->mjob )
				{
					v->mjob->finished = true;
					v->mjob = NULL;
					v->MoveJobActive = false;
				}
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// new attackjob:
			case MSG_ADD_ATTACKJOB:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				if( !network->bServer )
				{
					AddAttackJob( atoi( Strings->Items[0].c_str() ), atoi( Strings->Items[1].c_str() ), true, atoi( Strings->Items[2].c_str() ),atoi( Strings->Items[3].c_str() ),atoi( Strings->Items[4].c_str() ) );
				}
				else
				{
					AddAttackJob( atoi( Strings->Items[0].c_str() ), atoi( Strings->Items[1].c_str() ), false, atoi( Strings->Items[2].c_str() ),atoi( Strings->Items[3].c_str() ),atoi( Strings->Items[4].c_str() ) );
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// destroy object:
			case MSG_DESTROY_OBJECT:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				DestroyObject( atoi( Strings->Items[0].c_str() ), atoi( Strings->Items[1].c_str() ) );
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// execute a movejob:
			case MSG_ERLEDIGEN:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				cVehicle *v;
				if( atoi( Strings->Items[1].c_str() ) == 0)
				{
					v=map->GO[atoi( Strings->Items[0].c_str() )].vehicle;
				}else
				{
					v=map->GO[atoi( Strings->Items[0].c_str() )].plane;        
				}
				delete Strings;

				if( v && v->mjob )
				{
					v->mjob->CalcNextDir();
					AddActiveMoveJob(v->mjob);
					if( network->bServer && v->mjob->waypoints && v->mjob->waypoints->next )
					{
						SendIntIntBool( v->mjob->waypoints->X+v->mjob->waypoints->Y*map->size, v->mjob->waypoints->next->X+v->mjob->waypoints->next->Y*map->size, v->mjob->plane, MSG_MOVE_TO );
					}
				}
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// notifiation of saved speed:
			case MSG_SAVED_SPEED:
			{
				cVehicle *v;
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				if( Strings->Items[2].compare("0") != 0 )
				{
					v = map->GO[atoi( Strings->Items[0].c_str() )].vehicle;
				}
				else
				{
					v = map->GO[atoi( Strings->Items[0].c_str() )].plane;
				}
				if( v && v->owner == game->ActivePlayer )
				{
					v->data.speed += atoi( Strings->Items[1].c_str() );
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// notification of new buildingname:
			case MSG_CHANGE_BUI_NAME:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				ChangeBuildingName( atoi( Strings->Items[0].c_str() ), atoi( Strings->Items[1].c_str() ), Strings->Items[2], true, atoi( Strings->Items[3].c_str() ));
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// starts buildprocess of building:
			case MSG_START_BUILD:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				cVehicle *v;
				if ( v = map->GO[atoi( Strings->Items[0].c_str() )].vehicle )
				{
					v->IsBuilding = true;
					v->BuildingTyp = atoi( Strings->Items[1].c_str() );
					v->BuildRounds = atoi( Strings->Items[2].c_str() );
					v->BuildCosts = atoi( Strings->Items[3].c_str() );
					v->BandX = atoi( Strings->Items[4].c_str() );
					v->BandY = atoi( Strings->Items[5].c_str() );     
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// cancels buildprocess of building:
			case MSG_STOP_BUILD:
			{
				cVehicle *v;
				if ( v = map->GO[atoi( sMsgString.c_str() )].vehicle )
				{
					v->IsBuilding = false;
					v->IsClearing = false;
					v->BuildPath = false;
					if(v->data.can_build == BUILD_BIG || v->ClearBig)
					{
						map->GO[v->BandX + v->BandY * map->size].vehicle = NULL;
						map->GO[v->BandX + 1 + v->BandY * map->size].vehicle  = NULL;
						map->GO[v->BandX + 1 + ( v->BandY + 1 ) * map->size].vehicle = NULL;
						map->GO[v->BandX + ( v->BandY + 1 ) * map->size].vehicle = NULL;
						map->GO[v->BuildBigSavedPos].vehicle = v;
						v->PosX = v->BuildBigSavedPos % map->size;
						v->PosY = v->BuildBigSavedPos / map->size;
					}
				}
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// adds a new building
			case MSG_ADD_BUILDING:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				cPlayer *pl;
				sBuilding *b;
				for( int i = 0 ;i < game->PlayerList->iCount; i++ )
				{
					pl = game->PlayerList->Items[i];
					if( pl->Nr == atoi( Strings->Items[3].c_str() ) ) break;
				}
				b = UnitsData.building + atoi( Strings->Items[2].c_str() );
				UpdateBuilding( b->data, pl->BuildingData[b->nr] )
				AddBuilding( atoi( Strings->Items[0].c_str() ), atoi( Strings->Items[1].c_str() ), b, pl );
				if( b->data.is_base || b->data.is_connector )
				{
					cVehicle *v;
					v = map->GO[atoi( Strings->Items[0].c_str() ) + atoi( Strings->Items[1].c_str() ) * map->size].vehicle;
					if( v && v->data.can_build == BUILD_SMALL )
					{
						v->IsBuilding = false;
					}
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// starts construction of a big building:
			case MSG_START_BUILD_BIG:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				if( map->GO[atoi( Strings->Items[0].c_str() )].vehicle )
				{
					cVehicle *v;
					v = map->GO[atoi( Strings->Items[0].c_str() )].vehicle;
					v->BuildBigSavedPos = v->PosX + v->PosY * map->size;
					v->IsBuilding = true;
					v->BuildingTyp = atoi( Strings->Items[1].c_str() );
					v->BuildRounds = atoi( Strings->Items[2].c_str() );
					v->BuildCosts = atoi( Strings->Items[3].c_str() );
					v->BandX = atoi( Strings->Items[4].c_str() );
					v->BandY = atoi( Strings->Items[5].c_str() );
					map->GO[v->BandX + v->BandY * map->size].vehicle = v;
					map->GO[v->BandX + 1 + v->BandY * map->size].vehicle = v;
					map->GO[v->BandX + (v->BandY+1) * map->size].vehicle = v;
					map->GO[v->BandX + 1 + (v->BandY+1) * map->size].vehicle = v;
					v->PosX = v->BandX;
					v->PosY = v->BandY;
					if( !map->IsWater( v->PosX + v->PosY * map->size ) )
					{
						v->ShowBigBeton = true;
						v->BigBetonAlpha = 10;
					}else
					{
						v->ShowBigBeton = false;
						v->BigBetonAlpha = 255;
					}
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// center construction unit within construction side:
			case MSG_RESET_CONSTRUCTOR:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				cVehicle *v;
				if( v = map->GO[atoi( Strings->Items[0].c_str() )].vehicle )
				{
					v->PosX += atoi( Strings->Items[1].c_str() );
					v->PosY += atoi( Strings->Items[2].c_str() );
					v->BuildOverride = true;
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// starts cleaning of area (e.g. bulldozer)
			case MSG_START_CLEAR:
			{
				cVehicle *Vehicle;
				Vehicle = map->GO[atoi ( sMsgString.c_str() )].vehicle;
				if( Vehicle )
				{
					Vehicle->IsClearing = true;
					Vehicle->ClearingRounds = map->GO[Vehicle->PosX + Vehicle->PosY * map->size].base->DirtValue / 4 + 1;
					Vehicle->ClearBig = map->GO[Vehicle->PosX + Vehicle->PosY * map->size].base->data.is_big;
					if( game->SelectedVehicle == Vehicle )
					{
						StopFXLoop( game->ObjectStream );
						game->ObjectStream = Vehicle->PlayStram();
					}
					if( Vehicle->ClearBig )
					{
						Vehicle->PosX = map->GO[Vehicle->PosX + Vehicle->PosY * map->size].base->PosX;
						Vehicle->PosY = map->GO[Vehicle->PosX + Vehicle->PosY * map->size].base->PosY;
						Vehicle->BandX = Vehicle->PosX;
						Vehicle->BandY = Vehicle->PosY;
						map->GO[Vehicle->PosX + Vehicle->PosY * map->size].vehicle = Vehicle;
						map->GO[Vehicle->PosX + 1 + Vehicle->PosY * map->size].vehicle = Vehicle;
						map->GO[Vehicle->PosX + 1 + ( Vehicle->PosY + 1 ) * map->size].vehicle = Vehicle;
						map->GO[Vehicle->PosX + ( Vehicle->PosY + 1 ) * map->size].vehicle = Vehicle;
					}
				}
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// board/load a vehicle into
			case MSG_STORE_VEHICLE:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				cVehicle *Vehicle;
				cBuilding *Building;
				if( atoi( Strings->Items[0].c_str() ) )
				{
					Vehicle = map->GO[atoi( Strings->Items[2].c_str() )].plane;
				}
				else
				{
					Vehicle = map->GO[atoi( Strings->Items[2].c_str() )].vehicle;
				}
				if( atoi( Strings->Items[1].c_str() ) )
				{
					Building = map->GO[atoi( Strings->Items[3].c_str() )].top;
					if( Building && Vehicle )
					{
						Building->StoreVehicle(atoi( Strings->Items[2].c_str() ));
					}
				}
				else
				{
					cVehicle *VehicleSource;
					if( atoi( Strings->Items[4].c_str() ) )
					{
						VehicleSource = map->GO[atoi( Strings->Items[3].c_str() )].plane;
					}
					else
					{
						VehicleSource = map->GO[atoi( Strings->Items[3].c_str() )].vehicle;
					}
					if( VehicleSource && Vehicle )
					{
						VehicleSource->StoreVehicle(atoi( Strings->Items[2].c_str() ));
					}
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// unboard/unload a vehicle from
			case MSG_ACTIVATE_VEHICLE:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				if( atoi( Strings->Items[0].c_str() ) )
				{
					cBuilding *Building;
					Building = map->GO[atoi( Strings->Items[4].c_str() )].top;
					if( Building )
					{
						cVehicle *Vehicle;
						if( Building->StoredVehicles && Building->StoredVehicles->iCount > atoi( Strings->Items[2].c_str() ) )
						{
							Vehicle = Building->StoredVehicles->Items[atoi( Strings->Items[2].c_str() )];
							Vehicle->data.hit_points = atoi( Strings->Items[5].c_str() );
							Vehicle->data.ammo = atoi( Strings->Items[6].c_str() );
						}
						Building->ExitVehicleTo(atoi( Strings->Items[2].c_str() ),atoi( Strings->Items[3].c_str() ),true);
					}
				}
				else
				{
					cVehicle *Vehicle;
					if( atoi( Strings->Items[1].c_str() ) )
					{
						Vehicle = map->GO[atoi( Strings->Items[4].c_str() )].plane;
					}
					else
					{
						Vehicle = map->GO[atoi( Strings->Items[4].c_str() )].vehicle;
					}
					if( Vehicle )
					{
						Vehicle->ExitVehicleTo(atoi( Strings->Items[2].c_str() ),atoi( Strings->Items[3].c_str() ),true);
					}
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// activates a building (start work)
			case MSG_START_WORK:
			{
				map->GO[atoi( sMsgString.c_str() )].top->StartWork( true );
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// deactivated a building (stop work)
			case MSG_STOP_WORK:
			{
				map->GO[atoi( sMsgString.c_str() )].top->StopWork( false, true );
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// add a new vehicle to game
			case MSG_ADD_VEHICLE:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				cPlayer *Player = NULL;
				for( int i = 0 ; i < game->PlayerList->iCount ; i++ )
				{
					if( game->PlayerList->Items[i]->Nr == atoi ( Strings->Items[0].c_str() ) )
					{
						Player = game->PlayerList->Items[i];
						break;
					}
				}
				if( Player )
				{
					AddVehicle(atoi ( Strings->Items[2].c_str() ),atoi ( Strings->Items[3].c_str() ),UnitsData.vehicle + atoi ( Strings->Items[1].c_str() ),Player,false,true);
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// repair a unit
			case MSG_REPAIR:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				if( atoi ( Strings->Items[0].c_str() ) )
				{
					cBuilding *Building;
					Building = map->GO[atoi ( Strings->Items[2].c_str() )].top;
					Building->data.hit_points = atoi ( Strings->Items[3].c_str() );
					if( game->SelectedBuilding == Building ) Building->ShowDetails();
				}
				else
				{
					cVehicle *Vehicle;
					if( atoi ( Strings->Items[1].c_str() ) )
					{
						Vehicle = map->GO[atoi ( Strings->Items[2].c_str() )].plane;
					}
					else
					{
						Vehicle = map->GO[atoi ( Strings->Items[2].c_str() )].vehicle;
					}
					Vehicle->data.hit_points = atoi ( Strings->Items[3].c_str() );
					if( game->SelectedVehicle == Vehicle ) Vehicle->ShowDetails();
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// reload a unit
			case MSG_RELOAD:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				if( atoi ( Strings->Items[0].c_str() ) )
				{
					cBuilding *Building;
					Building = map->GO[atoi ( Strings->Items[2].c_str() )].top;
					Building->data.ammo = atoi ( Strings->Items[3].c_str() );
					if( game->SelectedBuilding == Building ) Building->ShowDetails();
				}
				else
				{
					cVehicle *Vehicle;
					if( atoi ( Strings->Items[1].c_str() ) )
					{
						Vehicle = map->GO[atoi ( Strings->Items[2].c_str() )].plane;
					}
					else
					{
						Vehicle = map->GO[atoi ( Strings->Items[2].c_str() )].vehicle;
					}
					Vehicle->data.ammo = atoi ( Strings->Items[3].c_str() );
					if( game->SelectedVehicle == Vehicle ) Vehicle->ShowDetails();
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// change sentrymode of unit:
			case MSG_WACHE:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				cVehicle *Vehicle;
				if( atoi ( Strings->Items[0].c_str() ) )
				{
					Vehicle = map->GO[atoi ( Strings->Items[1].c_str() )].plane;
				}
				else
				{
					Vehicle = map->GO[atoi ( Strings->Items[1].c_str() )].vehicle;
				}
				if( Vehicle )
				{
					Vehicle->Wachposten = atoi ( Strings->Items[2].c_str() );
					Vehicle->Wachwechsel();
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// clears field of claymore:
			case MSG_CLEAR_MINE:
			{
				cBuilding *Building;
				Building = map->GO[atoi( sMsgString.c_str() )].base;
				if( Building )
				{
					if( Building == game->SelectedBuilding ) Building->Deselct();
					if ( map->GO[atoi( sMsgString.c_str() )].subbase )
					{
						map->GO[atoi( sMsgString.c_str() )].base = map->GO[atoi( sMsgString.c_str() )].subbase;
						map->GO[atoi( sMsgString.c_str() )].subbase = NULL;
					}
					else
					{
						map->GO[atoi( sMsgString.c_str() )].base = NULL;
					}
					if( Building->prev )
					{
						cBuilding *PrevBuilding;
						PrevBuilding = Building->prev;
						PrevBuilding->next = Building->next;
						if( PrevBuilding->next ) PrevBuilding->next->prev = PrevBuilding;
					}
					else
					{
						Building->owner->BuildingList = Building->next;
						if( Building->next ) Building->next->prev = NULL;
					}
					delete Building;
				}
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// upgrades a unit:
			case MSG_UPGRADE:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				cPlayer *Player;
				for( int i = 0 ; i < game->PlayerList->iCount ; i++ )
				{
					Player = game->PlayerList->Items[i];
					if( Player->Nr == atoi( Strings->Items[0].c_str() ) ) break;
				}
				if( Player == game->ActivePlayer )
				{
					delete Strings;
					delete network->NetMessageList->Items[iNum];
					network->NetMessageList->Delete ( iNum );
					break;
				}

				if( atoi( Strings->Items[1].c_str() ) == 1 )
				{
					sUnitData *VehicleData;
					VehicleData = Player->VehicleData + atoi( Strings->Items[2].c_str() );

					VehicleData->damage = atoi( Strings->Items[3].c_str() );
					VehicleData->range = atoi( Strings->Items[4].c_str() );
					VehicleData->max_shots = atoi( Strings->Items[5].c_str() );
					VehicleData->max_ammo = atoi( Strings->Items[6].c_str() );
					VehicleData->max_hit_points = atoi( Strings->Items[7].c_str() );
					VehicleData->armor = atoi( Strings->Items[8].c_str() );
					VehicleData->scan = atoi( Strings->Items[9].c_str() );
					VehicleData->costs = atoi( Strings->Items[10].c_str() );
					VehicleData->max_speed = atoi( Strings->Items[11].c_str() );

					VehicleData->version++;
				}
				else
				{
					sUnitData *BuildingData;
					BuildingData = Player->BuildingData + atoi( Strings->Items[2].c_str() );

					BuildingData->damage = atoi( Strings->Items[3].c_str() );
					BuildingData->range = atoi( Strings->Items[4].c_str() );
					BuildingData->max_shots = atoi( Strings->Items[5].c_str() );
					BuildingData->max_ammo = atoi( Strings->Items[6].c_str() );
					BuildingData->max_hit_points = atoi( Strings->Items[7].c_str() );
					BuildingData->armor = atoi( Strings->Items[8].c_str() );
					BuildingData->scan = atoi( Strings->Items[9].c_str() );
					BuildingData->costs = atoi( Strings->Items[10].c_str() );

					BuildingData->version++;
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// finished research
			case MSG_RESEARCH:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				cPlayer *Player;
				for( int i = 0 ; i < game->PlayerList->iCount ; i++ )
				{
					Player = game->PlayerList->Items[i];
					if( Player->Nr == atoi( Strings->Items[0].c_str() ) ) break;
				}
				if( Player != game->ActivePlayer )
				{
					Player->DoTheResearch( atoi( Strings->Items[1].c_str() ) );
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// improve/update building:
			case MSG_UPDATE_BUILDING:
			{
				cBuilding *Building;
				Building = map->GO[atoi( sMsgString.c_str() )].top;
				if( Building && Building->owner != game->ActivePlayer)
				{
					UpdateBuilding(Building->data,Building->owner->BuildingData[Building->typ->nr]);
					Building->GenerateName();
					if( Building == game->SelectedBuilding ) Building->ShowDetails();
				}
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// commando failed:
			case MSG_COMMANDO_MISTAKE:
			{
				cVehicle *Vehicle;
				Vehicle = map->GO[atoi( sMsgString.c_str() )].vehicle;
				if( Vehicle )
				{
					Vehicle->detected = true;
					Vehicle->detection_override = true;
					if( game->ActivePlayer->ScanMap[Vehicle->PosX + Vehicle->PosY * map->size] )
					{
						game->AddCoords( ((string)Vehicle->data.name + " " + lngPack.i18n( "Text~Comp~Detected")), Vehicle->PosX, Vehicle->PosY );
						if( random( 2, 0 ) == 0 ) PlayVoice( VoiceData.VOIDetected1 );
						else PlayVoice( VoiceData.VOIDetected2 );
					}
				}
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// commando success:
			case MSG_COMMANDO_SUCCESS:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );

				cVehicle *Vehicle,*TargetVehicle;
				cBuilding *TargetBuilding;
				bool bSteal;

				bSteal = atoi ( Strings->Items[0].c_str() );
				Vehicle = map->GO[atoi ( Strings->Items[1].c_str() )].vehicle;
				TargetVehicle = map->GO[atoi ( Strings->Items[2].c_str() )].vehicle;
				TargetBuilding = map->GO[atoi ( Strings->Items[2].c_str() )].top;
				if( Vehicle )
				{
					if( TargetVehicle )
					{
						if( bSteal )
						{
							if( TargetVehicle->owner == game->ActivePlayer )
							{
								game->AddCoords( ((string)TargetVehicle->data.name + " " + lngPack.i18n( "Text~Comp~Stolen")), TargetVehicle->PosX, TargetVehicle->PosY );
							}
							TargetVehicle->owner = Vehicle->owner;
						}
						else
						{
							if( TargetVehicle->owner == game->ActivePlayer )
							{
								game->AddCoords( ((string)TargetVehicle->data.name + " " + lngPack.i18n( "Text~Comp~Disabled")), TargetVehicle->PosX, TargetVehicle->PosY );
							}
							TargetVehicle->Disabled = 2 + Vehicle->CommandoRank / 2;
							TargetVehicle->data.speed = 0;
							TargetVehicle->data.shots = 0;
							if( TargetVehicle->mjob )
							{
								TargetVehicle->mjob->finished = true;
								TargetVehicle->mjob = NULL;
								TargetVehicle->moving = false;
								TargetVehicle->MoveJobActive = false;
							}
						}
					}
					else if( TargetBuilding )
					{
						if( TargetBuilding->owner == game->ActivePlayer )
						{
							game->AddCoords( ((string)TargetBuilding->data.name + " " + lngPack.i18n( "Text~Comp~Disabled")), TargetBuilding->PosX, TargetBuilding->PosY );
						}
						TargetBuilding->Disabled = 2 + Vehicle->CommandoRank / 2;
						TargetBuilding->data.shots = 0;
						TargetBuilding->StopWork( true );
						TargetBuilding->owner->DoScan();
					}
					if( Vehicle->CommandoRank < 5) Vehicle->CommandoRank++;
				}
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// request resync:
			case MSG_START_SYNC:
			{
				int iSync;
				iSync = atoi( sMsgString.c_str() );

				cPlayer *Player = game->ActivePlayer;
				string sMsg;
				sSyncPlayer *SyncPlayer;

				SyncPlayer = new sSyncPlayer();

				SyncPlayer->TNT = game->hud->TNT;
				SyncPlayer->Radar = game->hud->Radar;
				SyncPlayer->Nebel = game->hud->Nebel;
				SyncPlayer->Gitter = game->hud->Gitter;
				SyncPlayer->Scan = game->hud->Scan;
				SyncPlayer->Reichweite = game->hud->Reichweite;
				SyncPlayer->Munition = game->hud->Munition;
				SyncPlayer->Treffer = game->hud->Treffer;
				SyncPlayer->Farben = game->hud->Farben;
				SyncPlayer->Status = game->hud->Status;
				SyncPlayer->Studie = game->hud->Studie;
				SyncPlayer->PlayFLC = game->hud->PlayFLC;
				SyncPlayer->Zoom = game->hud->Zoom;
				SyncPlayer->OffX = game->hud->OffX;
				SyncPlayer->OffY = game->hud->OffY;
				SyncPlayer->Lock = game->hud->Lock;

				SyncPlayer->PlayerID = Player->Nr;
				SyncPlayer->Credits = Player->Credits;
				memcpy( SyncPlayer->ResearchTechs, Player->ResearchTechs, sizeof(sResearch) * 8 );
				SyncPlayer->ResearchCount = Player->ResearchCount;
				SyncPlayer->UnusedResearch = Player->UnusedResearch;
				if( Player->VehicleList || Player->BuildingList )
				{
					cBuilding *Building = Player->BuildingList;
					cVehicle *Vehicle = Player->VehicleList;
					SyncPlayer->EndOfSync = false;

					sSyncVehicle *SyncVehicle = new sSyncVehicle();
					while( Vehicle )
					{
						if( Vehicle->Loaded )
						{
							if( SyncVehicle )
							{
								if( Vehicle->next == NULL && Building == NULL )
								{
									SyncVehicle->EndOfSync = true;
								}
								else
								{
									SyncVehicle->EndOfSync = false;
								}
							}
							else if( !Player->BuildingList && Vehicle->next == NULL )
							{
								SyncPlayer->EndOfSync = true;
							}
							Vehicle = Vehicle->next;
							continue;
						}

						SyncVehicle->PlayerID = Player->Nr;
						SyncVehicle->isPlane = Vehicle->data.can_drive == DRIVE_AIR;
						SyncVehicle->off = Vehicle->PosX + Vehicle->PosY * map->size;
						SyncVehicle->IsBuilding = Vehicle->IsBuilding;
						SyncVehicle->BuildingTyp = Vehicle->BuildingTyp;
						SyncVehicle->BuildCosts = Vehicle->BuildCosts;
						SyncVehicle->BuildRounds = Vehicle->BuildRounds;
						SyncVehicle->BuildRoundsStart = Vehicle->BuildRoundsStart;
						SyncVehicle->BandX = Vehicle->BandX;
						SyncVehicle->BandY = Vehicle->BandY;
						SyncVehicle->IsClearing = Vehicle->IsClearing;
						SyncVehicle->ClearingRounds = Vehicle->ClearingRounds;
						SyncVehicle->ClearBig = Vehicle->ClearBig;
						SyncVehicle->ShowBigBeton = Vehicle->ShowBigBeton;
						SyncVehicle->FlightHigh = Vehicle->FlightHigh;
						SyncVehicle->LayMines = Vehicle->LayMines;
						SyncVehicle->ClearMines = Vehicle->ClearMines;
						SyncVehicle->Loaded = Vehicle->Loaded;
						SyncVehicle->CommandoRank = Vehicle->CommandoRank;
						SyncVehicle->Disabled = Vehicle->Disabled;
						SyncVehicle->Ammo = Vehicle->data.ammo;
						SyncVehicle->Cargo = Vehicle->data.cargo;

						if( Vehicle->next == NULL && Building == NULL )
						{
							SyncVehicle->EndOfSync = true;
						}
						else
						{
							SyncVehicle->EndOfSync = false;
						}
						SendVehicleSync( SyncVehicle );

						Vehicle = Vehicle->next;
					}

					while( Building )
					{
						sSyncBuilding *SyncBuilding = new sSyncBuilding();;

						SyncBuilding->PlayerID=  Player->Nr;
						SyncBuilding->isBase = Building->data.is_base;
						SyncBuilding->off = Building->PosX+Building->PosY*map->size;
						if ( map->GO[SyncBuilding->off].top && map->GO[SyncBuilding->off].top == Building )
						{
							SyncBuilding->iTyp = 0;
						}
						else if( map->GO[SyncBuilding->off].base && map->GO[SyncBuilding->off].base == Building )
						{
							SyncBuilding->iTyp = 1;
						}
						else
						{
							SyncBuilding->iTyp = 2;
						}
						SyncBuilding->IsWorking = Building->IsWorking;
						SyncBuilding->MetalProd = Building->MetalProd;
						SyncBuilding->OilProd = Building->OilProd;
						SyncBuilding->GoldProd = Building->GoldProd;
						SyncBuilding->MaxMetalProd = Building->MaxMetalProd;
						SyncBuilding->MaxOilProd = Building->MaxOilProd;
						SyncBuilding->MaxGoldProd = Building->MaxGoldProd;
						SyncBuilding->BuildSpeed = Building->BuildSpeed;
						SyncBuilding->RepeatBuild = Building->RepeatBuild;
						SyncBuilding->Disabled = Building->Disabled;
						SyncBuilding->Ammo = Building->data.ammo;
						SyncBuilding->Load = Building->data.cargo;

						if( Building->BuildList && Building->BuildList->iCount > 0 )
						{
							int *ptr = NULL;
							SyncBuilding->BuildList = Building->BuildList->iCount;

							// TODO: Add sending buildlist here
						}
						else
						{
							SyncBuilding->BuildList = 0;
						}

						if( Building->next == NULL )
						{
							SyncBuilding->EndOfSync = true;
						}
						else
						{
							SyncBuilding->EndOfSync = false;
						}

						SendBuildingSync( SyncBuilding );
						Building = Building->next;
					}
				}
				else
				{
					SyncPlayer->EndOfSync = true;
				}
				SendPlayerSync( SyncPlayer );

				SyncNo = iSync;
				delete SyncPlayer;

				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// sync players:
			case MSG_SYNC_PLAYER:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );

				cPlayer *Player;
				for(int i = 0 ; i < game->PlayerList->iCount ; i++ )
				{
					Player = game->PlayerList->Items[i];
					if( Player->Nr == atoi ( Strings->Items[0].c_str() ) ) break;
				}

				Player->Credits = atoi ( Strings->Items[2].c_str() );
				Player->ResearchCount = atoi ( Strings->Items[3].c_str() );
				Player->UnusedResearch = atoi ( Strings->Items[4].c_str() );

				Player->HotHud.TNT = atoi ( Strings->Items[5].c_str() );
				Player->HotHud.Radar = atoi ( Strings->Items[6].c_str() );
				Player->HotHud.Nebel = atoi ( Strings->Items[7].c_str() );
				Player->HotHud.Gitter = atoi ( Strings->Items[8].c_str() );
				Player->HotHud.Scan = atoi ( Strings->Items[9].c_str() );
				Player->HotHud.Reichweite = atoi ( Strings->Items[10].c_str() );
				Player->HotHud.Munition = atoi ( Strings->Items[11].c_str() );
				Player->HotHud.Treffer = atoi ( Strings->Items[12].c_str() );
				Player->HotHud.Farben = atoi ( Strings->Items[13].c_str() );
				Player->HotHud.Status = atoi ( Strings->Items[14].c_str() );
				Player->HotHud.Studie = atoi ( Strings->Items[15].c_str() );
				Player->HotHud.PlayFLC = atoi ( Strings->Items[16].c_str() );
				Player->HotHud.Zoom = atoi ( Strings->Items[17].c_str() );
				Player->HotHud.OffX = atoi ( Strings->Items[18].c_str() );
				Player->HotHud.OffY = atoi ( Strings->Items[19].c_str() );
				Player->HotHud.Lock = atoi ( Strings->Items[20].c_str() );

				for( int i = 0; i < 8; i++ )
				{
					Player->ResearchTechs[i].working_on = atoi ( Strings->Items[21 + i*4].c_str() );
					Player->ResearchTechs[i].RoundsRemaining = atoi ( Strings->Items[22 + i*4].c_str() );
					Player->ResearchTechs[i].MaxRounds = atoi ( Strings->Items[23 + i*4].c_str() );
					Player->ResearchTechs[i].level = atoi ( Strings->Items[24 + i*4].c_str() );
				}
				if( atoi ( Strings->Items[1].c_str() ) ) SyncWaiting--;

				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// sync vehicles:
			case MSG_SYNC_VEHICLE:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );

				cVehicle *Vehicle;
				cPlayer *Player;
				for(int i = 0 ; i < game->PlayerList->iCount ; i++ )
				{
					Player = game->PlayerList->Items[i];
					if( Player->Nr == atoi ( Strings->Items[0].c_str() ) ) break;
				}

				if( Strings->Items[2].c_str() )
				{
					Vehicle = map->GO[atoi ( Strings->Items[3].c_str())].plane;
				}
				else
				{
					Vehicle = map->GO[atoi ( Strings->Items[3].c_str())].vehicle;
				}
				if( !Vehicle || Vehicle->owner != Player )
				{
					delete Strings;
					delete network->NetMessageList->Items[iNum];
					network->NetMessageList->Delete ( iNum );
					break;
				}

				Vehicle->PosX = atoi ( Strings->Items[3].c_str() ) % map->size;
				Vehicle->PosY = atoi ( Strings->Items[3].c_str() ) / map->size;
				Vehicle->IsBuilding = atoi ( Strings->Items[4].c_str() );
				Vehicle->BuildingTyp = atoi ( Strings->Items[5].c_str() );
				Vehicle->BuildCosts = atoi ( Strings->Items[6].c_str() );
				Vehicle->BuildRounds = atoi ( Strings->Items[7].c_str() );
				Vehicle->BuildRoundsStart = atoi ( Strings->Items[8].c_str() );
				Vehicle->BandX = atoi ( Strings->Items[9].c_str() );
				Vehicle->BandY = atoi ( Strings->Items[10].c_str() );
				Vehicle->IsClearing = atoi ( Strings->Items[11].c_str() );
				Vehicle->ClearingRounds = atoi ( Strings->Items[12].c_str() );
				Vehicle->ClearBig = atoi ( Strings->Items[13].c_str() );
				Vehicle->ShowBigBeton = atoi ( Strings->Items[14].c_str() );
				Vehicle->FlightHigh = atoi ( Strings->Items[15].c_str() );
				Vehicle->LayMines = atoi ( Strings->Items[16].c_str() );
				Vehicle->ClearMines = atoi ( Strings->Items[17].c_str() );
				Vehicle->Loaded = atoi ( Strings->Items[18].c_str() );
				Vehicle->CommandoRank = atoi ( Strings->Items[19].c_str() );
				Vehicle->Disabled = atoi ( Strings->Items[20].c_str() );
				Vehicle->data.ammo = atoi ( Strings->Items[21].c_str() );
				Vehicle->data.cargo = atoi ( Strings->Items[22].c_str() );

				if(atoi ( Strings->Items[1].c_str() ) ) SyncWaiting--;

				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// sync buildings:
			case MSG_SYNC_BUILDING:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );

				cBuilding *Building;
				cPlayer *Player;
				for( int i = 0 ; i < game->PlayerList->iCount ; i++ )
				{
					Player = game->PlayerList->Items[i];
					if( Player->Nr == atoi ( Strings->Items[0].c_str() ) ) break;
				}

				if ( atoi ( Strings->Items[2].c_str() ) == 0 ) // top
				{
					Building = map->GO[atoi ( Strings->Items[3].c_str() )].top;
				}
				else if ( atoi ( Strings->Items[2].c_str() ) == 1 ) // base
				{
					Building = map->GO[atoi ( Strings->Items[3].c_str() )].base;
				}
				else // subbase
				{
					Building = map->GO[atoi ( Strings->Items[3].c_str() )].subbase;
				}
				if( !Building || Building->owner != Player )
				{
					delete Strings;
					delete network->NetMessageList->Items[iNum];
					network->NetMessageList->Delete ( iNum );
					break;
				}

				Building->PosX = atoi ( Strings->Items[3].c_str() ) % map->size;
				Building->PosY = atoi ( Strings->Items[3].c_str() ) / map->size;
				Building->data.is_base = atoi ( Strings->Items[4].c_str() );
				Building->IsWorking = atoi ( Strings->Items[5].c_str() );
				Building->MetalProd = atoi ( Strings->Items[6].c_str() );
				Building->OilProd = atoi ( Strings->Items[7].c_str() );
				Building->GoldProd = atoi ( Strings->Items[8].c_str() );
				Building->MaxMetalProd = atoi ( Strings->Items[9].c_str() );
				Building->MaxOilProd = atoi ( Strings->Items[10].c_str() );
				Building->MaxGoldProd = atoi ( Strings->Items[11].c_str() );
				Building->BuildSpeed = atoi ( Strings->Items[12].c_str() );
				Building->RepeatBuild=  atoi ( Strings->Items[13].c_str() );
				Building->Disabled = atoi ( Strings->Items[14].c_str() );
				Building->data.ammo = atoi ( Strings->Items[15].c_str() );
				Building->data.cargo = atoi ( Strings->Items[16].c_str() );

				if(atoi ( Strings->Items[1].c_str() ) ) SyncWaiting--;

				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// updates a unit in storage/hangar/dock:
			case MSG_UPDATE_STORED:
			{
				cList<string> *Strings;
				Strings = SplitMessage ( sMsgString );
				int iPlayerNr, iIndex, iOff;
				cBuilding *Building;

				iPlayerNr = atoi ( Strings->Items[0].c_str() );
				iIndex = atoi ( Strings->Items[1].c_str() );
				iOff = atoi ( Strings->Items[2].c_str() );
				Building = map->GO[iOff].top;
				if( !Building || !Building->StoredVehicles || Building->StoredVehicles->iCount <= iIndex || Building->owner->Nr != iPlayerNr )
				{
					delete Strings;
					delete network->NetMessageList->Items[iNum];
					network->NetMessageList->Delete ( iNum );
					break;
				}
				// TOFIX: Add copying data here!
				//Building->StoredVehicles->Items[iIndex]->data = *((sVehicleData*)(msg+12));
				delete Strings;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// turn finished:
			case MSG_REPORT_R_E_A:
			{
				RundenendeActionsReport++;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// Ping:
			case MSG_PING:
			{
				network->TCPSend( MSG_PONG, iToStr( game->ActivePlayer->Nr ).c_str() );
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// Pong:
			case MSG_PONG:
			{
				int iIndex = 0;
				sPing *Ping;
				if( !PingList ) break;
				if( network->bServer )
				{
					cPlayer *Player;
					for( int i = 0 ; i < game->PlayerList->iCount ; i++ )
					{
						Player = game->PlayerList->Items[i];
						for( int k = 0 ; k < PingList->iCount ; k++ )
						{
							Ping = PingList->Items[k];
							if( Player->Nr == Ping->PlayerID )
							{
								iIndex = k;
								break;
							}
						}
					}
				}
				Ping = PingList->Items[iIndex];
				if( Ping->rx_count >= PING_COUNT )
				{
					delete network->NetMessageList->Items[iNum];
					network->NetMessageList->Delete ( iNum );
					break;
				}
				Ping->rx[Ping->rx_count++] = atoi( sMsgString.c_str() );

				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// host defeated:
			case MSG_HOST_DEFEAT:
			{
				game->Defeated = true;
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// player defeated:
			case MSG_PLAYER_DEFEAT:
			{
				cPlayer *Player;
				for( int i = 0 ; i < game->PlayerList->iCount ; i++ )
				{
					Player = game->PlayerList->Items[i] ;
					if( Player->Nr != atoi ( sMsgString.c_str() ) ) continue;

					if( Player != game->ActivePlayer )
					{
						game->AddMessage( lngPack.i18n( "Text~Multiplayer~Player") + " " + Player->name  + " " + lngPack.i18n( "Text~Comp~Defeated") + "!" );
						delete Player;
						game->PlayerList->Delete( i );
					}
					else
					{
						game->Defeated = true;
					}
					break;
				}
				delete network->NetMessageList->Items[iNum];		
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// next player in round-playing-mode:
			case MSG_PLAY_ROUNDS_NEXT:
			{
				cBuilding *b;
				cVehicle *v;
				cPlayer *p;
				int next;
				next = atoi ( sMsgString.c_str() );
				game->ActiveRoundPlayerNr = next;
				if ( next == game->ActivePlayer->Nr )
				{
					game->hud->Ende = false;
					game->hud->EndeButton ( false );
					p = game->ActivePlayer;
				}
				else
				{
					for(int k = 0 ; k < game->PlayerList->iCount ; k++)
					{
						p = game->PlayerList->Items[k];
						if( p->Nr == next )
						{
							game->AddMessage( p->name + lngPack.i18n ( "Text~Multiplayer~Player_Turn" ) );
							break;
						}
					}
				}

				v = p->VehicleList;
				while ( v )
				{
					v->RefreshData();
					v = v->next;
				}
				b = p->BuildingList;
				while ( b )
				{
					if ( b->data.can_attack ) b->RefreshData();
					b = b->next;
				}
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
			// If the messages isn't known just delete it
			default:
			{
				delete network->NetMessageList->Items[iNum];
				network->NetMessageList->Delete ( iNum );
				break;
			}
		}
	}
}

// Sends a chat-message:
void cEngine::SendChatMessage(const char *str)
{
	if(network){
		string sChatMessage = str;
		if(sChatMessage.length() > 255)
		{
			sChatMessage.erase( 255 );
		}
		network->TCPSend(MSG_CHAT, sChatMessage.c_str());
	}
	game->AddMessage(str);
	PlayFX(SoundData.SNDChat);
}

cList<string>* cEngine::SplitMessage ( string sMsg )
{
	cList<string> *Strings;
	Strings = new cList<string>;
	int npos=0;
	for ( int i=0; npos != string::npos; i++ )
	{
		Strings->Add( sMsg.substr ( npos, ( sMsg.find ( NET_MSG_SEPERATOR,npos )-npos ) ) );
		npos = ( int ) sMsg.find ( NET_MSG_SEPERATOR,npos );
		if ( npos != string::npos )
			npos++;
	}
	return Strings;
}
