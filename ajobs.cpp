//////////////////////////////////////////////////////////////////////////////
// M.A.X. - ajobs.cpp
//////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include "ajobs.h"
#include "game.h"

// Funktionen der AJobs Klasse ///////////////////////////////////////////////
cAJobs::cAJobs ( cMap *Map,int ScrOff,int DestOff,bool scr_air,bool dest_air,bool scr_building,bool wache )
{
	float dx,dy,r;
	map=Map;
	wait=0;
	ScrX=ScrOff%map->size;
	ScrY=ScrOff/map->size;
	DestX=DestOff%map->size;
	DestY=DestOff/map->size;
	dest=DestOff;
	scr=ScrOff;
	ScrAir=scr_air;
	DestAir=dest_air;
	Wache=wache;
	ScrBuilding=scr_building;
	if ( ScrOff==DestOff )
	{
		MineDetonation=true;
	}
	else
	{
		MineDetonation=false;
	}
	if ( scr_building )
	{
		vehicle=NULL;
		if ( MineDetonation )
		{
			building=map->GO[ScrOff].base;
		}
		else
		{
			building=map->GO[ScrOff].top;
		}
		building->Attacking=true;
	}
	else
	{
		building=NULL;
		if ( !ScrAir )
		{
			vehicle=map->GO[ScrOff].vehicle;
		}
		else
		{
			vehicle=map->GO[ScrOff].plane;
		}
		vehicle->Attacking=true;
	}
	MuzzlePlayed=false;
	// Die Feuerrichtung ausrechnen:
	dx=DestX-ScrX;
	dy=- ( DestY-ScrY );
	r=sqrt ( dx*dx+dy*dy );
	if ( r<=0.001 )
	{
		if ( vehicle ) FireDir=vehicle->dir;else FireDir=building->dir;
	}
	else
	{
		dx/=r;
		dy/=r;
		r=asin ( dx ) *57.29577951;
		if ( dy>=0 )
		{
			if ( r<0 ) r+=360;
		}
		else
		{
			r=180-r;
		}
		if ( r>=337.5||r<=22.5 ) FireDir=0;
		else if ( r>=22.5&&r<=67.5 ) FireDir=1;
		else if ( r>=67.5&&r<=112.5 ) FireDir=2;
		else if ( r>=112.5&&r<=157.5 ) FireDir=3;
		else if ( r>=157.5&&r<=202.5 ) FireDir=4;
		else if ( r>=202.5&&r<=247.5 ) FireDir=5;
		else if ( r>=247.5&&r<=292.5 ) FireDir=6;
		else if ( r>=292.5&&r<=337.5 ) FireDir=7;
		else FireDir= ( vehicle?vehicle->dir:building->dir );
	}
}

cAJobs::~cAJobs ( void )
{}

// Malt das Mündungsfeuer:
void cAJobs::PlayMuzzle ( void )
{
	int offx=0,offy=0;
	int typ= ( vehicle?vehicle->data.muzzle_typ:building->data.muzzle_typ );
	if ( building&&building->data.is_expl_mine )
	{
		MuzzlePlayed=true;
		PlayFX ( building->typ->Attack );
		return;
	}
	switch ( typ )
	{
		case MUZZLE_BIG:
			if ( wait++!=0 )
			{
				if ( wait>2 ) MuzzlePlayed=true;
				return;
			}
			switch ( FireDir )
			{
				case 0:
					offy-=40;
					break;
				case 1:
					offx+=32;
					offy-=32;
					break;
				case 2:
					offx+=40;
					break;
				case 3:
					offx+=32;
					offy+=32;
					break;
				case 4:
					offy+=40;
					break;
				case 5:
					offx-=32;
					offy+=32;
					break;
				case 6:
					offx-=40;
					break;
				case 7:
					offx-=32;
					offy-=32;
					break;
			}
			if ( vehicle )
			{
				game->AddFX ( fxMuzzleBig,vehicle->PosX*64+offx,vehicle->PosY*64+offy,FireDir );
			}
			else
			{
				game->AddFX ( fxMuzzleBig,building->PosX*64+offx,building->PosY*64+offy,FireDir );
			}
			break;
		case MUZZLE_SMALL:
			if ( wait++!=0 )
			{
				if ( wait>2 ) MuzzlePlayed=true;
				return;
			}
			if ( vehicle )
			{
				game->AddFX ( fxMuzzleSmall,vehicle->PosX*64,vehicle->PosY*64,FireDir );
			}
			else
			{
				game->AddFX ( fxMuzzleSmall,building->PosX*64,building->PosY*64,FireDir );
			}
			break;
		case MUZZLE_ROCKET:
		case MUZZLE_ROCKET_CLUSTER:
		{
			sFXRocketInfos *ri;
			if ( wait++!=0 ) return;
			ri=new sFXRocketInfos;
			ri->DestX=DestX*64;
			ri->DestY=DestY*64;
			ri->aj=this;
			ri->dir=FireDir;
			if ( vehicle )
			{
				ri->ScrX=vehicle->PosX*64;
				ri->ScrY=vehicle->PosY*64;
				game->AddFX ( fxRocket,vehicle->PosX*64,vehicle->PosY*64, ( int ) ri );
			}
			else
			{
				ri->ScrX=building->PosX*64;
				ri->ScrY=building->PosY*64;
				game->AddFX ( fxRocket,building->PosX*64,building->PosY*64, ( int ) ri );
			}
			break;
		}
		case MUZZLE_MED:
		case MUZZLE_MED_LONG:
			if ( wait++!=0 )
			{
				if ( wait>2 ) MuzzlePlayed=true;
				return;
			}
			switch ( FireDir )
			{
				case 0:
					offy-=20;
					break;
				case 1:
					offx+=12;
					offy-=12;
					break;
				case 2:
					offx+=20;
					break;
				case 3:
					offx+=12;
					offy+=12;
					break;
				case 4:
					offy+=20;
					break;
				case 5:
					offx-=12;
					offy+=12;
					break;
				case 6:
					offx-=20;
					break;
				case 7:
					offx-=12;
					offy-=12;
					break;
			}
			if ( typ==MUZZLE_MED )
			{
				if ( vehicle )
				{
					game->AddFX ( fxMuzzleMed,vehicle->PosX*64+offx,vehicle->PosY*64+offy,FireDir );
				}
				else
				{
					game->AddFX ( fxMuzzleMed,building->PosX*64+offx,building->PosY*64+offy,FireDir );
				}
			}
			else
			{
				if ( vehicle )
				{
					game->AddFX ( fxMuzzleMedLong,vehicle->PosX*64+offx,vehicle->PosY*64+offy,FireDir );
				}
				else
				{
					game->AddFX ( fxMuzzleMedLong,building->PosX*64+offx,building->PosY*64+offy,FireDir );
				}
			}
			break;
		case MUZZLE_TORPEDO:
		{
			sFXRocketInfos *ri;
			if ( wait++!=0 ) return;
			ri=new sFXRocketInfos;
			ri->DestX=DestX*64;
			ri->DestY=DestY*64;
			ri->aj=this;
			ri->dir=FireDir;
			if ( vehicle )
			{
				ri->ScrX=vehicle->PosX*64;
				ri->ScrY=vehicle->PosY*64;
				game->AddFX ( fxTorpedo,vehicle->PosX*64,vehicle->PosY*64, ( int ) ri );
			}
			else
			{
				ri->ScrX=building->PosX*64;
				ri->ScrY=building->PosY*64;
				game->AddFX ( fxTorpedo,building->PosX*64,building->PosY*64, ( int ) ri );
			}
			break;
		}
		case MUZZLE_SNIPER:
			MuzzlePlayed=true;
			break;
	}
	if ( vehicle )
	{
		PlayFX ( vehicle->typ->Attack );
	}
	else
	{
		PlayFX ( building->typ->Attack );
	}

	// Die Stats ändern:
	if ( vehicle )
	{
		vehicle->data.shots--;
		vehicle->data.ammo--;
		if ( !vehicle->data.shots ) vehicle->AttackMode=false;
		if ( !vehicle->data.can_drive_and_fire ) vehicle->data.speed-= (int)(( ( float ) vehicle->data.max_speed ) /vehicle->data.max_shots);
		if ( game->SelectedVehicle&&game->SelectedVehicle==vehicle )
		{
			vehicle->ShowDetails();
		}
	}
	else
	{
		building->data.shots--;
		building->data.ammo--;
		if ( !building->data.shots ) building->AttackMode=false;
		if ( game->SelectedBuilding&&game->SelectedBuilding==building )
		{
			building->ShowDetails();
		}
	}
}

// Kümmert sich um den Einschlag, und gibt true zurück, wenn das Ziel zerstört
// wurde:
bool cAJobs::MakeImpact ( void )
{
	cVehicle *target=NULL;
	cBuilding *btarget=NULL;
	bool destroyed=false;
	bool PlayImpact=false;
	bool WasMine=false;
	string name;

	if ( ScrX==DestX&&ScrY==DestY&&!MineDetonation ) return false;

	// Ziel auswählen:
	if ( vehicle&&vehicle->data.can_attack==ATTACK_AIRnLAND )
	{
		target=map->GO[dest].vehicle;
		btarget=map->GO[dest].top;
		if ( !btarget&&map->GO[dest].base&&map->GO[dest].base->owner ) btarget=map->GO[dest].base;
		if ( !target&&!btarget )
		{
			target=map->GO[dest].plane;
			DestAir=true;
		}
		else
		{
			DestAir=false;
		}
	}
	else
	{
		if ( !DestAir )
		{
			target=map->GO[dest].vehicle;
			btarget=map->GO[dest].top;
			if ( !btarget&&map->GO[dest].base&&map->GO[dest].base->owner ) btarget=map->GO[dest].base;
		}
		else
		{
			target=map->GO[dest].plane;
		}
	}
	if ( target&&target->data.is_stealth_sea )
	{
		if ( building )
		{
			if ( building->data.can_attack!=ATTACK_SUB_LAND ) target=NULL;
		}
		else if ( vehicle )
		{
			if ( vehicle->data.can_attack!=ATTACK_SUB_LAND ) target=NULL;
		}
		else target=NULL;
	}

	// Ggf Schild:
	if ( game->AlienTech )
	{
		cPlayer *s=NULL;
		if ( target&&target->owner->ShieldMap[dest] ) s=target->owner;
		else if ( btarget&&btarget->owner->ShieldMap[dest] ) s=btarget->owner;
		if ( s )
		{
			game->AddFX ( fxAbsorb,DestX*64,DestY*64,0 );
			if ( s->ShieldImpact ( dest, ( /*vehicle?vehicle->data.damage:*/building->data.damage ) ) ) return false;
		}
	}

	// Impact:
	if ( !target&&!btarget ) PlayImpact=true;
	else
	{
		if ( target )
		{
			if ( target->owner==game->ActivePlayer ) WasMine=true;
			name=target->name;
			target->data.hit_points=target->CalcHelth ( ( vehicle?vehicle->data.damage:building->data.damage ) );
			if ( target->data.hit_points>0 )
			{
				PlayImpact=true;
				if ( game->SelectedVehicle&&game->SelectedVehicle==target ) target->ShowDetails();
				if ( vehicle&&vehicle->owner==game->ActivePlayer ) MouseMoveCallback ( true );
				else if ( building&&building->owner==game->ActivePlayer ) MouseMoveCallback ( true );
				if ( MineDetonation ) DetonateMine();
			}
			else
			{
				// nur SP/Server:
				game->engine->DestroyObject ( dest,DestAir );
				destroyed=true;
			}
		}
		else if ( btarget )
		{
			if ( btarget->owner==game->ActivePlayer ) WasMine=true;
			name=btarget->name;
			btarget->data.hit_points=btarget->CalcHelth ( ( vehicle?vehicle->data.damage:building->data.damage ) );
			if ( btarget->data.hit_points>0 )
			{
				PlayImpact=true;
				if ( game->SelectedBuilding&&game->SelectedBuilding==btarget ) btarget->ShowDetails();
				if ( vehicle&&vehicle->owner==game->ActivePlayer ) MouseMoveCallback ( true );
				else if ( building&&building->owner==game->ActivePlayer ) MouseMoveCallback ( true );
			}
			else
			{
				// nur SP/Server:
				game->engine->DestroyObject ( btarget->PosX+btarget->PosY*map->size,false );
				destroyed=true;
			}
		}
	}
	if ( PlayImpact&&SettingsData.bAlphaEffects )
	{
		game->AddFX ( fxHit,DestX*64,DestY*64,0 );
	}

	if ( destroyed )
	{
		if ( WasMine )
		{
			char str[50];
			sprintf ( str,"%s zerstört",name.c_str() );
			game->AddCoords ( str,DestX,DestY );
			PlayVoice ( VoiceData.VOIDestroyedUs );
		}
	}
	else
	{
		if ( target&&target->owner==game->ActivePlayer )
		{
			char str[50];
			if ( destroyed||target->data.hit_points<=0 )
			{
				sprintf ( str,"%s zerstört",name.c_str() );
				PlayVoice ( VoiceData.VOIDestroyedUs );
			}
			else
			{
				sprintf ( str,"%s wird angegriffen",name.c_str() );
				PlayVoice ( VoiceData.VOIAttackingUs );
			}
			game->AddCoords ( str,DestX,DestY );
		}
		else if ( btarget&&btarget->owner==game->ActivePlayer )
		{
			char str[50];
			if ( destroyed||btarget->data.hit_points<=0 )
			{
				sprintf ( str,"%s zerstört",name.c_str() );
				PlayVoice ( VoiceData.VOIDestroyedUs );
			}
			else
			{
				sprintf ( str,"%s wird angegriffen",name.c_str() );
				PlayVoice ( VoiceData.VOIAttackingUs );
			}
			game->AddCoords ( str,DestX,DestY );
		}
	}

	return destroyed;
}

// Läßt die Mine verschwinden:
void cAJobs::DetonateMine ( void )
{
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
	map->GO[scr].base=NULL;
	delete building;
}

// Kümmert sich um den Einschlag einer Clusterrakete:
void cAJobs::MakeClusters ( void )
{
	int dx,dy,d,damage,size;
	cPlayer *p;
	if ( ( vehicle?vehicle->data.muzzle_typ:building->data.muzzle_typ ) !=MUZZLE_ROCKET_CLUSTER ) return;

	size=map->size;
	d=dest;
	dx=DestX;
	dy=DestY;
	p= ( vehicle?vehicle->owner:building->owner );
	damage= ( vehicle?vehicle->data.damage:building->data.damage );
	( vehicle?vehicle->data.damage/=2:building->data.damage/=2 );

#define CLUSTERUS if(DestX>=0&&DestX<size&&DestY>=0&&DestY<size&&((map->GO[dest].vehicle&&map->GO[dest].vehicle->owner!=p&&map->GO[dest].vehicle->detected)||(map->GO[dest].base&&map->GO[dest].base->owner&&map->GO[dest].base->owner!=p&&map->GO[dest].base->detected)||(map->GO[dest].top&&map->GO[dest].top->owner!=p))){MakeImpact();}dest=d;DestX=dx;DestY=dy;

	dest--;DestX--;
	CLUSTERUS
	dest++;DestX++;
	CLUSTERUS
	dest-=size;DestY--;
	CLUSTERUS
	dest+=size;DestY++;
	CLUSTERUS

	dest-=size;DestY--;
	dest--;DestX--;
	CLUSTERUS
	dest-=size;DestY--;
	dest++;DestX++;
	CLUSTERUS
	dest+=size;DestY++;
	dest--;DestX--;
	CLUSTERUS
	dest+=size;DestY++;
	dest++;DestX++;
	CLUSTERUS

	( vehicle?vehicle->data.damage/=2:building->data.damage/=2 );
	dest-=2;DestX-=2;
	CLUSTERUS
	dest+=2;DestX+=2;
	CLUSTERUS
	dest-=size*2;DestY-=2;
	CLUSTERUS
	dest+=size*2;DestY+=2;
	CLUSTERUS

	( vehicle?vehicle->data.damage=damage:building->data.damage=damage );
}
