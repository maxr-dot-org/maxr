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
#include "hud.h"
#include "main.h"
#include "mouse.h"
#include "game.h"
#include "sound.h"
#include "dialog.h"
#include "keyinp.h"
#include "fonts.h"
#include "menu.h"


// Funktionen der Hud-Klasse /////////////////////////////////////////////////
cHud::cHud ( void )
{
	TNT=false;
	Radar=false;
	Nebel=false;
	Gitter=false;
	Scan=false;
	Reichweite=false;
	Munition=false;
	Treffer=false;
	Farben=false;
	Status=false;
	Studie=false;
	LastOverEnde=false;
	Lock=false;
	Zoom=64;
	LastZoom=64;
	OffX=0;
	OffY=0;
	Praeferenzen=false;
	PlayFLC=true;
	PausePressed=false;PlayPressed=false;
	HelpPressed=false;ChatPressed=false;
	EndePressed=false;ErledigenPressed=false;
	NextPressed=false;PrevPressed=false;
	CenterPressed=false;DateiPressed=false;
	LogPressed=false;
	Ende=false;
}

cHud::~cHud ( void )
{}

void cHud::SwitchTNT ( bool set )
{
	SDL_Rect scr={334,24,27,28},dest={136,413,27,28};
	if ( set )
		{
			scr.x=362;
			scr.y=24;
		}

	BlitButton(scr, dest, "", false);
	TNT=set;
	game->fDrawHud=true;
	game->fDrawMMap=true;
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::SwitchRadar ( bool set )
{
	SDL_Rect scr={334,53,27,26},dest={136,387,27,26};
	if ( set )
		{
			scr.x=362;
			scr.y=53;
		}

	BlitButton(scr, dest, "", false);
	Radar=set;
	game->fDrawHud=true;
	game->fDrawMMap=true;
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::SwitchNebel ( bool set )
{
	SDL_Rect scr={277,78,55,18},dest={112,330,55,18};
	if ( set )
		{
			scr.x=110;
			scr.y=78;
		}

	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~Fog"),  set, true);
	Nebel=set;
	game->fDrawHud=true;
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::SwitchGitter ( bool set )
{
	SDL_Rect scr={277,61,55,17},dest={112,313,55,17};
	if ( set )
		{
			scr.x=110;
			scr.y=61;
		}

	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~Grid"), set, true);
	Gitter=set;
	game->fDrawHud=true;
	game->fDrawMap=true;
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::SwitchScan ( bool set )
{
	SDL_Rect scr={277,44,55,18},dest={112,296,55,18};
	if ( set )
		{
			scr.x=110;
			scr.y=44;
		}

	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~Scan"), set, true);
	Scan=set;
	game->fDrawHud=true;
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::SwitchReichweite ( bool set )
{
	SDL_Rect scr={222,78,55,18},dest={57,330,55,18};
	if ( set )
		{
			scr.x=55;
			scr.y=78;
		}

	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~Range"), set, true);
	Reichweite=set;
	game->fDrawHud=true;
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::SwitchMunition ( bool set )
{
	SDL_Rect scr={222,61,55,17},dest={57,313,55,17};
	if ( set )
		{
			scr.x=55;
			scr.y=61;
		}
	
	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~Ammo"),  set, true);
	Munition=set;
	game->fDrawHud=true;
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::SwitchTreffer ( bool set )
{
	SDL_Rect scr={222,44,55,18},dest={57,296,55,18};
	if ( set )
		{
			scr.x=55;
			scr.y=44;
		}

	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~Hitpoints"), set, true);
	Treffer=set;
	game->fDrawHud=true;
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::SwitchFarben ( bool set )
{
	SDL_Rect scr={167,78,55,18},dest={2,330,55,18};
	if ( set )
		{
			scr.x=0;
			scr.y=78;
		}

	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~Color"),  set, true);
	Farben=set;
	game->fDrawHud=true;
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::SwitchStatus ( bool set )
{
	SDL_Rect scr={167,61,55,17},dest={2,313,55,17};
	if ( set )
		{
			scr.x=0;
			scr.y=61;
		}

	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~Status"),  set, true);
	Status=set;
	game->fDrawHud=true;
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::SwitchStudie ( bool set )
{
	SDL_Rect scr={167,44,55,18},dest={2,296,55,18};
	if ( set )
		{
			scr.x=0;
			scr.y=44;
		}
		
	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~Survey"), set, true);
	Studie=set;
	game->fDrawHud=true;
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::SwitchLock ( bool set )
{
	SDL_Rect scr={397,321,21,22},dest={32,227,21,22};
	if ( set )
		{
			scr.x=397;
			scr.y=298;
		}

	BlitButton(scr, dest, "", false);
	Lock=set;
	game->fDrawHud=true;
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::DoZoom ( int x,int y )
{
	if ( x<=12 ) x=0;else x-=12;
	if ( x>104 ) x=104;
//  x=64-(int)(x*(64-((448.0/game->map->size)<5?5:(448.0/game->map->size)))/104);
	x=64- ( int ) ( x* ( 64- ( ( ( SettingsData.iScreenW-192.0 ) /game->map->size ) <5?5: ( ( SettingsData.iScreenW-192.0 ) /game->map->size ) ) ) /104 );
	if ( x<1 ) x=1;
	SetZoom ( x,y );
}

void cHud::SetZoom ( int zoom,int DestY )
{
	static int lastz=64;
	SDL_Rect scr,dest;
//  if(zoom<448/game->map->size)zoom=448/game->map->size;
	if ( zoom< ( SettingsData.iScreenW-192 ) /game->map->size ) zoom= ( SettingsData.iScreenW-192 ) /game->map->size;
	if ( zoom<5 ) zoom=5;
	else if ( zoom>64 ) zoom=64;
	Zoom=zoom;
//  zoom-=((448.0/game->map->size)<5?5:(448.0/game->map->size));
	zoom-= ( int ) ( ( ( ( SettingsData.iScreenW-192.0 ) /game->map->size ) <5?5: ( ( SettingsData.iScreenW-192.0 ) /game->map->size ) ) );
	scr.x=0;
	scr.y=0;
	dest.x=19;
	dest.y=DestY;
	dest.w=scr.w=132;
	dest.h=scr.h=20;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,GraphicsData.gfx_hud,&dest );
//  zoom=106-(int)(zoom*106.0/(64-((448.0/game->map->size)<5?5:(448.0/game->map->size))));
	zoom=106- ( int ) ( zoom*106.0/ ( 64- ( ( ( SettingsData.iScreenW-192.0 ) /game->map->size ) <5?5: ( ( SettingsData.iScreenW-192.0 ) /game->map->size ) ) ) );
	scr.x=132;
	scr.y=1;
	dest.x=20+zoom;
	dest.y=DestY+1;
	dest.w=scr.w=25;
	dest.h=scr.h=14;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,GraphicsData.gfx_hud,&dest );
	game->fDrawHud=true;
	game->fDrawMap=true;
	game->fDrawMMap=true;

	if ( lastz!=Zoom )
	{
		int off;
//    off=(int)(((448.0/(float)lastz*64)-(448.0/(float)Zoom*64))/2);
		off= ( int ) ( ( ( ( SettingsData.iScreenW-192.0 ) / ( float ) lastz*64 )- ( ( SettingsData.iScreenW-192.0 ) / ( float ) Zoom*64 ) ) /2 );
		lastz=Zoom;

		OffX+=off;
		OffY+=off;
//    off=game->map->size*64-(int)((448.0/Zoom)*64);
		off=game->map->size*64- ( int ) ( ( ( SettingsData.iScreenW-192.0 ) /Zoom ) *64 );
		while ( OffX>off ) OffX--;
		while ( OffY>off ) OffY--;
		while ( OffX<0 ) OffX++;
		while ( OffY<0 ) OffY++;
	}
	ScaleSurfaces();
}

void cHud::CheckButtons ( void )
{
	int x,y;
	x=mouse->x;
	y=mouse->y;
	if ( x<170 )
	{
		if ( x>=136&&x<=136+27&&y>=413&&y<=413+28 ) {SwitchTNT ( !TNT );return;}
		if ( x>=136&&x<=136+27&&y>=387&&y<=387+28 ) {SwitchRadar ( !Radar );return;}
		if ( x>=112&&x<=112+55&&y>=332&&y<=332+17 ) {SwitchNebel ( !Nebel );return;}
		if ( x>=112&&x<=112+55&&y>=314&&y<=314+17 ) {SwitchGitter ( !Gitter );return;}
		if ( x>=112&&x<=112+55&&y>=296&&y<=296+17 ) {SwitchScan ( !Scan );return;}
		if ( x>=57&&x<=57+55&&y>=332&&y<=332+17 ) {SwitchReichweite ( !Reichweite );return;}
		if ( x>=57&&x<=57+55&&y>=314&&y<=314+17 ) {SwitchMunition ( !Munition );return;}
		if ( x>=57&&x<=57+55&&y>=296&&y<=296+17 ) {SwitchTreffer ( !Treffer );return;}
		if ( x>=0&&x<=55&&y>=332&&y<=332+17 ) {SwitchFarben ( !Farben );return;}
		if ( x>=0&&x<=55&&y>=314&&y<=314+17 ) {SwitchStatus ( !Status );return;}
		if ( x>=0&&x<=55&&y>=296&&y<=296+17 ) {SwitchStudie ( !Studie );return;}
		if ( x>=32&&x<=52&&y>=227&&y<=248 ) {SwitchLock ( !Lock );return;}
	}
}

void cHud::CheckOneClick ( void )
{
	static int lastX=-1;
	int x,y;
	x=mouse->x;
	y=mouse->y;
	if ( x<170&&y<296 )
	{
		if ( x!=lastX&&x>=20&&y>=272&&x<=150&&y<=292 ) {DoZoom ( x-20 );lastX=x;return;}
	}
	if ( x>=15&&x<15+112&&y>=356&&y<356+112 ) {DoMinimapClick ( x,y );return;}
	lastX=x;
}

void cHud::DoAllHud ( void )
{
	bool s;
	s=SettingsData.bSoundEnabled;
	SettingsData.bSoundEnabled=false;
	
	DateiButton(false);
	PraeferenzenButton(false);
	PrevButton(false);
	ErledigenButton(false);
	NextButton(false);
	LogButton(false);
	ChatButton(false);
	DateiButton(false);
	SwitchTNT ( TNT );
	SwitchRadar ( Radar );
	SwitchNebel ( Nebel );
	SwitchGitter ( Gitter );
	SwitchScan ( Scan );
	SwitchReichweite ( Reichweite );
	SwitchMunition ( Munition );
	SwitchTreffer ( Treffer );
	SwitchFarben ( Farben );
	SwitchStatus ( Status );
	SwitchStudie ( Studie );
	SwitchLock ( Lock );
	SetZoom ( Zoom );
	SettingsData.bSoundEnabled=s;
	ShowRunde();
}

void cHud::CheckScroll ( bool pure )
{
	int x,y;
	x=mouse->x;
	y=mouse->y;

	if ( x<=0&&y>30&&y<SettingsData.iScreenH-30-18 ) {mouse->SetCursor ( CPfeil4 );DoScroll ( 4 );return;}
	if ( x>=SettingsData.iScreenW-18&&y>30&&y<SettingsData.iScreenH-30-18 ) {mouse->SetCursor ( CPfeil6 );DoScroll ( 6 );return;}

	if ( ( x<=0&&y<=30 ) || ( y<=0&&x<=30 ) ) {mouse->SetCursor ( CPfeil7 );DoScroll ( 7 );return;}
	if ( ( x>=SettingsData.iScreenW-18&&y<=30 ) || ( y<=0&&x>=SettingsData.iScreenW-30-18 ) ) {mouse->SetCursor ( CPfeil9 );DoScroll ( 9 );return;}

	if ( y<=0&&x>30&&x<SettingsData.iScreenW-30-18 ) {mouse->SetCursor ( CPfeil8 );DoScroll ( 8 );return;}
	if ( y>=SettingsData.iScreenH-18&&x>30&&x<SettingsData.iScreenW-30-18 ) {mouse->SetCursor ( CPfeil2 );DoScroll ( 2 );return;}

	if ( ( x<=0&&y>=SettingsData.iScreenH-30-18 ) || ( y>=SettingsData.iScreenH-18&&x<=30 ) ) {mouse->SetCursor ( CPfeil1 );DoScroll ( 1 );return;}
	if ( ( x>=SettingsData.iScreenW-18&&y>=SettingsData.iScreenH-30-18 ) || ( y>=SettingsData.iScreenH-18&&x>=SettingsData.iScreenW-30-18 ) ) {mouse->SetCursor ( CPfeil3 );DoScroll ( 3 );return;}

	if ( pure )
	{
		if ( mouse->SetCursor ( CHand ) )
		{
			game->fDrawMap=true;
		}
		return;
	}

	if ( game->SelectedVehicle&&game->SelectedVehicle->PlaceBand&&game->SelectedVehicle->owner==game->ActivePlayer )
	{
		if ( x>=180 )
		{
			if ( mouse->SetCursor ( CBand ) )
			{
				game->fDrawMap=true;
			}
		}
		else
		{
			if ( mouse->SetCursor ( CNo ) )
			{
				game->fDrawMap=true;
			}
		}
	}
	else if ( ( game->SelectedVehicle&&game->SelectedVehicle->Transfer&&game->SelectedVehicle->owner==game->ActivePlayer ) || ( game->SelectedBuilding&&game->SelectedBuilding->Transfer&&game->SelectedBuilding->owner==game->ActivePlayer ) )
	{
		if ( game->SelectedVehicle )
		{
			if ( game->OverObject&&game->SelectedVehicle->CanTransferTo ( game->OverObject ) )
			{
				if ( mouse->SetCursor ( CTransf ) )
				{
					game->fDrawMap=true;
				}
			}
			else
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
		}
		else
		{
			if ( game->OverObject&&game->SelectedBuilding->CanTransferTo ( game->OverObject ) )
			{
				if ( mouse->SetCursor ( CTransf ) )
				{
					game->fDrawMap=true;
				}
			}
			else
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
		}
	}
	else if ( !game->HelpActive )
	{

		if ( x<180 )
		{
			if ( mouse->SetCursor ( CHand ) )
			{
				game->OverObject=NULL;
				game->fDrawMap=true;
			}
			return;
		}

		if ( y<2+21&&y>=2&&x>=390&&x<390+72 )
		{
			if ( mouse->SetCursor ( CHand ) )
			{
				game->fDrawMap=true;
			}
			game->OverObject=NULL;
			LastOverEnde=true;
			return;
		}
		else if ( LastOverEnde )
		{
			LastOverEnde=false;
			MouseMoveCallback ( true );
		}

		if ( ( game->SelectedVehicle&&game->SelectedVehicle->MenuActive&&game->SelectedVehicle->MouseOverMenu ( x,y ) ) ||
		        ( game->SelectedBuilding&&game->SelectedBuilding->MenuActive&&game->SelectedBuilding->MouseOverMenu ( x,y ) ) )
		{
			if ( mouse->SetCursor ( CHand ) )
			{
				game->fDrawMap=true;
			}
		}
		else if ( game->SelectedVehicle&&game->SelectedVehicle->AttackMode&&game->SelectedVehicle->owner==game->ActivePlayer&&x>=180&&y>=18&&x<SettingsData.iScreenW-12&&y<SettingsData.iScreenH-14 )
		{
			if ( game->SelectedVehicle->IsInRange ( mouse->GetKachelOff() ) )
			{
				if ( mouse->SetCursor ( CAttack ) )
				{
					game->fDrawMap=true;
				}
			}
			else
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
		}
		else if ( game->SelectedVehicle&&game->SelectedVehicle->StealActive&&game->SelectedVehicle->owner==game->ActivePlayer&&x>=180&&y>=18&&x<SettingsData.iScreenW-12&&y<SettingsData.iScreenH-14 )
		{
			if ( game->SelectedVehicle->IsInRangeCommando ( mouse->GetKachelOff(),true ) )
			{
				if ( mouse->SetCursor ( CSteal ) )
				{
					game->fDrawMap=true;
					MouseMoveCallback ( true );
				}
			}
			else
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
		}
		else if ( game->SelectedVehicle&&game->SelectedVehicle->DisableActive&&game->SelectedVehicle->owner==game->ActivePlayer&&x>=180&&y>=18&&x<SettingsData.iScreenW-12&&y<SettingsData.iScreenH-14 )
		{
			if ( game->SelectedVehicle->IsInRangeCommando ( mouse->GetKachelOff(),false ) )
			{
				if ( mouse->SetCursor ( CDisable ) )
				{
					game->fDrawMap=true;
					MouseMoveCallback ( true );
				}
			}
			else
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
		}
		else if ( game->SelectedBuilding&&game->SelectedBuilding->AttackMode&&game->SelectedBuilding->owner==game->ActivePlayer&&x>=180&&y>=18&&x<SettingsData.iScreenW-12&&y<SettingsData.iScreenH-14 )
		{
			if ( game->SelectedBuilding->IsInRange ( mouse->GetKachelOff() ) )
			{
				if ( mouse->SetCursor ( CAttack ) )
				{
					game->fDrawMap=true;
				}
			}
			else
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
		}
		else if ( game->SelectedVehicle&&game->SelectedVehicle->owner==game->ActivePlayer&&game->SelectedVehicle->CanAttackObject ( mouse->GetKachelOff() ) )
		{
			if ( mouse->SetCursor ( CAttack ) )
			{
				game->fDrawMap=true;
				MouseMoveCallback ( true );
			}
		}
		else if ( game->SelectedBuilding&&game->SelectedBuilding->owner==game->ActivePlayer&&game->SelectedBuilding->CanAttackObject ( mouse->GetKachelOff() ) )
		{
			if ( mouse->SetCursor ( CAttack ) )
			{
				game->fDrawMap=true;
				MouseMoveCallback ( true );
			}
		}
		else if ( game->SelectedVehicle&&game->SelectedVehicle->owner==game->ActivePlayer&&game->SelectedVehicle->MuniActive )
		{
			if ( game->SelectedVehicle->CanMuni ( mouse->GetKachelOff() ) )
			{
				if ( mouse->SetCursor ( CMuni ) )
				{
					game->fDrawMap=true;
				}
			}
			else
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
		}
		else if ( game->SelectedVehicle&&game->SelectedVehicle->owner==game->ActivePlayer&&game->SelectedVehicle->RepairActive )
		{
			if ( game->SelectedVehicle->CanRepair ( mouse->GetKachelOff() ) )
			{
				if ( mouse->SetCursor ( CRepair ) )
				{
					game->fDrawMap=true;
				}
			}
			else
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
		}
		else if ( game->OverObject&&! ( game->SelectedVehicle&&game->SelectedVehicle->owner==game->ActivePlayer&& ( ( game->SelectedVehicle->data.can_drive!=DRIVE_AIR&&!game->OverObject->vehicle&& ( !game->OverObject->top||game->OverObject->top->data.is_connector ) ) || ( game->SelectedVehicle->data.can_drive==DRIVE_AIR&&!game->OverObject->plane ) ) ) &&! ( game->SelectedBuilding&&game->SelectedBuilding->owner==game->ActivePlayer&&game->SelectedBuilding->BuildList&&game->SelectedBuilding->BuildList->Count&&!game->SelectedBuilding->IsWorking&&game->SelectedBuilding->BuildList->BuildListItems[0]->metall_remaining<=0 ) &&! ( game->SelectedBuilding&&game->SelectedBuilding->owner==game->ActivePlayer&&game->SelectedBuilding->LoadActive ) &&! ( game->SelectedBuilding&&game->SelectedBuilding->owner==game->ActivePlayer&&game->SelectedBuilding->ActivatingVehicle ) &&! ( game->SelectedVehicle&&game->SelectedVehicle->owner==game->ActivePlayer&&game->SelectedVehicle->LoadActive )
		          &&! ( game->SelectedVehicle&&game->SelectedVehicle->owner==game->ActivePlayer&&game->SelectedVehicle->ActivatingVehicle ) )
		{
			if ( mouse->SetCursor ( CSelect ) )
			{
				game->fDrawMap=true;
			}
		}
		else if ( game->SelectedVehicle&&game->SelectedVehicle->owner==game->ActivePlayer&& ( ( !game->SelectedVehicle->IsBuilding&&!game->SelectedVehicle->IsClearing&&!game->SelectedVehicle->LoadActive&&!game->SelectedVehicle->ActivatingVehicle ) || ( game->SelectedVehicle->IsBuilding&&game->SelectedVehicle->BuildRounds==0 ) || ( game->SelectedVehicle->IsClearing&&game->SelectedVehicle->ClearingRounds==0 ) ) &&x>=180&&y>=18&&x<180+ ( SettingsData.iScreenW-192 ) &&y<18+ ( SettingsData.iScreenH-32 ) )
		{
			if ( game->SelectedVehicle->MoveJobActive )
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
			else if ( game->SelectedVehicle->CanDrive ( mouse->GetKachelOff() ) )
			{
				if ( mouse->SetCursor ( CMove ) )
				{
					game->fDrawMap=true;
				}
			}
			else
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
		}
		else if ( game->SelectedBuilding&&game->SelectedBuilding->owner==game->ActivePlayer&&game->SelectedBuilding->BuildList&&game->SelectedBuilding->BuildList->Count&&!game->SelectedBuilding->IsWorking&&game->SelectedBuilding->BuildList->BuildListItems[0]->metall_remaining<=0 )
		{
			if ( game->SelectedBuilding->CanExitTo ( mouse->GetKachelOff(),game->SelectedBuilding->BuildList->BuildListItems[0]->typ ) )
			{
				if ( mouse->SetCursor ( CActivate ) )
				{
					game->fDrawMap=true;
				}
			}
			else
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
		}
		else if ( game->SelectedBuilding&&game->SelectedBuilding->owner==game->ActivePlayer&&game->SelectedBuilding->ActivatingVehicle )
		{
			if ( game->SelectedBuilding->CanExitTo ( mouse->GetKachelOff(),game->SelectedBuilding->StoredVehicles->VehicleItems[game->SelectedBuilding->VehicleToActivate]->typ ) )
			{
				if ( mouse->SetCursor ( CActivate ) )
				{
					game->fDrawMap=true;
				}
			}
			else
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
		}
		else if ( game->SelectedVehicle&&game->SelectedVehicle->owner==game->ActivePlayer&&game->SelectedVehicle->ActivatingVehicle )
		{
			if ( game->SelectedVehicle->CanExitTo ( mouse->GetKachelOff(),game->SelectedVehicle->StoredVehicles->VehicleItems[game->SelectedVehicle->VehicleToActivate]->typ ) )
			{
				if ( mouse->SetCursor ( CActivate ) )
				{
					game->fDrawMap=true;
				}
			}
			else
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
		}
		else if ( game->SelectedBuilding&&game->SelectedBuilding->owner==game->ActivePlayer&&game->SelectedBuilding->LoadActive )
		{
			if ( game->SelectedBuilding->CanLoad ( mouse->GetKachelOff() ) )
			{
				if ( mouse->SetCursor ( CLoad ) )
				{
					game->fDrawMap=true;
				}
			}
			else
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
		}
		else if ( game->SelectedVehicle&&game->SelectedVehicle->owner==game->ActivePlayer&&game->SelectedVehicle->LoadActive )
		{
			if ( game->SelectedVehicle->CanLoad ( mouse->GetKachelOff() ) )
			{
				if ( mouse->SetCursor ( CLoad ) )
				{
					game->fDrawMap=true;
				}
			}
			else
			{
				if ( mouse->SetCursor ( CNo ) )
				{
					game->fDrawMap=true;
				}
			}
		}
		else
		{
			if ( mouse->SetCursor ( CHand ) )
			{
				game->fDrawMap=true;
			}
		}
	}
	else
	{
		if ( mouse->SetCursor ( CHelp ) )
		{
			game->fDrawMap=true;
		}
	}
}

void cHud::DoScroll ( int dir )
{
	static int lx=0,ly=0;
	cHud *hud;
	int step;
	if ( game->SelectedBuilding )
	{
		game->SelectedBuilding->MenuActive=false;
	}
	hud=game->hud;
	step=64/Zoom;
	step*=SettingsData.iScrollSpeed;
	switch ( dir )
	{
		case 1:
			hud->OffX-=step;
			hud->OffY+=step;
			break;
		case 2:
			hud->OffY+=step;
			break;
		case 3:
			hud->OffX+=step;
			hud->OffY+=step;
			break;
		case 4:
			hud->OffX-=step;
			break;
		case 6:
			hud->OffX+=step;
			break;
		case 7:
			hud->OffX-=step;
			hud->OffY-=step;
			break;
		case 8:
			hud->OffY-=step;
			break;
		case 9:
			hud->OffX+=step;
			hud->OffY-=step;
			break;
	}
	if ( hud->OffX<0 ) hud->OffX=0;
	if ( hud->OffY<0 ) hud->OffY=0;
//  step=game->map->size*64-(int)((448.0/Zoom)*64);
	step=game->map->size*64- ( int ) ( ( ( SettingsData.iScreenW-192.0 ) /Zoom ) *64 );
	if ( hud->OffX>=step ) hud->OffX=step;
	step=game->map->size*64- ( int ) ( ( ( SettingsData.iScreenH-32.0 ) /Zoom ) *64 );
	if ( hud->OffY>=step ) hud->OffY=step;
	if ( lx==OffX&&ly==OffY ) return;
	game->fDrawMap=true;
	game->fDrawMMap=true;
	lx=OffX;ly=OffY;
}

void cHud::DoMinimapClick ( int x,int y )
{
	static int lx=0,ly=0;
	if ( lx==x&&ly==y ) return;
	lx=x;ly=y;
	OffX= ( int ) ( ( game->map->size/112.0 ) * ( x-15 ) *64- ( 224.0/Zoom ) *64 );
	OffY= ( int ) ( ( game->map->size/112.0 ) * ( y-356 ) *64- ( 224.0/Zoom ) *64 );
	game->fDrawMMap=true;
	game->fDrawMap=true;
	DoScroll ( 0 );
}

void cHud::ChechMouseOver ( void )
{
	static int lb=0;
	int x,y,b;
	x=mouse->x;
	y=mouse->y;
	b=mouse->GetMouseButton();
	if ( b==4 ) b=0;
	if ( y>274|| ( x>179&&y>22 ) ) return;
	// Der Pr�ferenzen-Button:
	if ( x>=86&&x<=86+67&&y>=4&&y<=4+18 )
	{
		if ( b ) PraeferenzenButton ( true );
		else if ( lb )
		{
			PlayFX ( SoundData.SNDHudButton );
			game->ChangeObjectName=false;
			game->ChatInput=false;
			showPreferences();
			PraeferenzenButton ( false );
		}
	}
	else if ( Praeferenzen )
	{
		PraeferenzenButton ( false );
	}
	// Der Pause-Button:
	if ( x>=146&&x<164&&y>=143&&y<161 )
	{
		if ( b ) PauseButton ( true );
		else if ( lb )
		{
			PlayFX ( SoundData.SNDHudButton );
			PlayFLC=false;
			PauseButton ( false );
		}
	}
	else if ( PausePressed )
	{
		PauseButton ( false );
	}
	// Der Play-Button:
	if ( x>=146&&x<164&&y>=123&&y<140 )
	{
		if ( b ) PlayButton ( true );
		else if ( lb )
		{
			PlayFX ( SoundData.SNDHudButton );
			PlayFLC=true;
			PlayButton ( false );
		}
	}
	else if ( PlayPressed )
	{
		PlayButton ( false );
	}
	// Der Hilfe-Button:
	if ( x>=20&&x<46&&y>=250&&y<274 )
	{
		if ( b ) HelpButton ( true );
		else if ( lb )
		{
			PlayFX ( SoundData.SNDHudButton );
			game->HelpActive=true;
			HelpButton ( false );
		}
	}
	else if ( HelpPressed )
	{
		HelpButton ( false );
	}
	// Der Chat-Button:
	if ( x>=51&&x<100&&y>=252&&y<272 )
	{
		if ( b ) ChatButton ( true );
		else if ( lb )
		{
			PlayFX ( SoundData.SNDHudButton );
			game->ChatInput=true;
			InputStr="";
			ChatButton ( false );
		}
	}
	else if ( ChatPressed )
	{
		ChatButton ( false );
	}
	// Der Log-Button:
	if ( x>=102&&x<150&&y>=252&&y<272 )
	{
		if ( b ) LogButton ( true );
		else if ( lb )
		{
			PlayFX ( SoundData.SNDHudButton );
			//TODO: Log-Men� aufrufen...
			game->AddMessage ( lngPack.i18n( "Text~Error_Messages~INFO_Not_Implemented") );

			LogButton ( false );
		}
	}
	else if ( LogPressed )
	{
		LogButton ( false );
	}
	// Der Ende-Button:
	if ( x>=392&&x<460&&y>=5&&y<20 )
	{
		if ( b ) EndeButton ( true );
		else if ( lb&&!Ende )
		{
			MakeMeMyEnd();
		}
	}
	else if ( EndePressed&&!Ende )
	{
		EndeButton ( false );
	}
	// Der Erledigen-Button:
	if ( x>=99&&x<124&&y>=227&&y<251 )
	{
		if ( b ) ErledigenButton ( true );
		else if ( lb )
		{
			PlayFX ( SoundData.SNDHudButton );
			if ( game->SelectedVehicle&&game->SelectedVehicle->mjob&&game->SelectedVehicle->mjob->Suspended&&game->SelectedVehicle->data.speed )
			{
				if ( game->engine->fstcpip && !game->engine->fstcpip->bServer )
				{
					// Client
					game->SelectedVehicle->mjob->finished=true;
					game->SelectedVehicle->mjob=NULL;
					game->SelectedVehicle->MoveJobActive=false;    

					string sMessage;
					sMessage = iToStr ( game->SelectedVehicle->PosX + game->SelectedVehicle->PosY * game->map->size ) + "#" ;
					if ( game->SelectedVehicle->data.can_drive == DRIVE_AIR ) sMessage += "1";
					else sMessage += "0";
					game->engine->fstcpip->FSTcpIpSend ( MSG_ERLEDIGEN , sMessage.c_str() );
				}
				else
				{
					// SP/Host
					game->SelectedVehicle->mjob->CalcNextDir();
					game->engine->AddActiveMoveJob ( game->SelectedVehicle->mjob );
				}
			}
			ErledigenButton ( false );
		}
	}
	else if ( ErledigenPressed )
	{
		ErledigenButton ( false );
	}
	// Der Next-Button:
	if ( x>=124&&x<163&&y>=227&&y<250 )
	{
		if ( b ) NextButton ( true );
		else if ( lb )
		{
			cVehicle *v;
			PlayFX ( SoundData.SNDHudButton );
			v=game->ActivePlayer->GetNextVehicle();
			if ( v )
			{
				if ( game->SelectedVehicle )
				{
					game->SelectedVehicle->Deselct();
					StopFXLoop ( game->ObjectStream );
				}
				v->Select();
				v->Center();
				game->ObjectStream=v->PlayStram();
				game->SelectedVehicle=v;
			}
			NextButton ( false );
		}
	}
	else if ( NextPressed )
	{
		NextButton ( false );
	}
	// Der Prev-Button:
	if ( x>=60&&x<98&&y>=227&&y<250 )
	{
		if ( b ) PrevButton ( true );
		else if ( lb )
		{
			cVehicle *v;
			PlayFX ( SoundData.SNDHudButton );
			v=game->ActivePlayer->GetPrevVehicle();
			if ( v )
			{
				if ( game->SelectedVehicle )
				{
					game->SelectedVehicle->Deselct();
					StopFXLoop ( game->ObjectStream );
				}
				v->Select();
				v->Center();
				game->ObjectStream=v->PlayStram();
				game->SelectedVehicle=v;
			}
			PrevButton ( false );
		}
	}
	else if ( PrevPressed )
	{
		PrevButton ( false );
	}
	// Der Center-Button:
	if ( x>=4&&x<26&&y>=227&&y<249 )
	{
		if ( b ) CenterButton ( true );
		else if ( lb )
		{
			PlayFX ( SoundData.SNDHudButton );
			if ( game->SelectedVehicle )
			{
				game->SelectedVehicle->Center();
			}
			else if ( game->SelectedBuilding )
			{
				game->SelectedBuilding->Center();
			}
			CenterButton ( false );
		}
	}
	else if ( CenterPressed )
	{
		CenterButton ( false );
	}
	// Der Datei-Button:
	if ( x>=17&&x<84&&y>=3&&y<23 )
	{
		if ( b ) DateiButton ( true );
		else if ( lb )
		{
			PlayFX ( SoundData.SNDHudButton );
			ShowDateiMenu( true );
			DateiButton ( false );
		}
	}
	else if ( DateiPressed )
	{
		DateiButton ( false );
	}
	lb=b;
}

void cHud::PraeferenzenButton ( bool set )
{
	SDL_Rect scr={0,169,67,20},dest={86,4,67,20};
	if ( set )
		{
			scr.x=195;
			scr.y=0;
			Praeferenzen=true;
		}
	else
	{
		Praeferenzen=false;
	}
	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~Settings"), set, true );
}

// Setzt den Monitor zur�ck:
void cHud::ResetVideoMonitor ( void )
{
	SDL_Rect scr,dest;
	scr.x=295;
	scr.y=98;
	dest.x=10;
	dest.y=29;
	scr.w=dest.w=128;
	scr.h=dest.h=128;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,GraphicsData.gfx_hud,&dest );
}

void cHud::PauseButton ( bool set )
{
	SDL_Rect scr={19,132,19,19},dest={146,143,19,19};
	if ( set )
		{
			scr.x=176;
			scr.y=0;
			PausePressed=true;
		}
	else
	{
		PausePressed=false;
	}
	BlitButton(scr, dest, "", set );
}

void cHud::PlayButton ( bool set )
{
	SDL_Rect scr={0,132,19,18},dest={146,123,19,18};
	if ( set )
		{
			scr.x=157;
			scr.y=0;
			PlayPressed=true;
		}
	else
	{
		PlayPressed=false;
	}
	BlitButton(scr, dest, "", set );
}

void cHud::HelpButton ( bool set )
{
	SDL_Rect scr={268,151,26,24},dest={20,250,26,24};
	if ( set )
		{
			scr.x=366;
			scr.y=0;
			HelpPressed=true;
		}
	else
	{
		HelpPressed=false;
	}
	BlitButton(scr, dest, "", set );
}

void cHud::ChatButton ( bool set )
{
	SDL_Rect scr={245,130,49,20},dest={51,252,49,20};
	if ( set )
		{
			scr.x=210;
			scr.y=21;
			ChatPressed=true;
		}
	else
	{
		ChatPressed=false;
	}
	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~Chat"), set, true ); 
}

void cHud::LogButton ( bool set )
{
	SDL_Rect scr={196,129,49,20},dest={101,252,49,20};
	if ( set )
		{
			scr.x=160;
			scr.y=21;
			LogPressed=true;
		}
	else
	{
		LogPressed=false;
	}
	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~Log"), set, true ); 
}

void cHud::EndeButton ( bool set )
{
	SDL_Rect scr={0,151,70,17},dest={391,4,70,17};
	if ( set )
		{
			scr.x=22;
			scr.y=21;
			EndePressed=true;
		}
	else
	{
		EndePressed=false;
	}
	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~End"), set );
}

void cHud::ShowRunde ( void )
{
	SDL_Rect scr,dest;
	scr.x=156;
	scr.y=215;
	dest.w=scr.w=55;
	dest.h=scr.h=15;
	dest.x=471;
	dest.y=5;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,GraphicsData.gfx_hud,&dest );
	font->showTextCentered(498,7, iToStr(game->Runde), LATIN_NORMAL, GraphicsData.gfx_hud);
	game->fDrawHud=true;
}

void cHud::ErledigenButton ( bool set )
{
	SDL_Rect scr={132,172,25,24},dest={99,227,25,24};
	if ( set )
		{
			scr.x=262;
			scr.y=0;
			ErledigenPressed=true;
		}
	else
	{
		ErledigenPressed=false;
	}
	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~Proceed"), set );
}

void cHud::NextButton ( bool set )
{
	SDL_Rect scr={158,172,39,23},dest={124,227,39,23};
	if ( set )
		{
			scr.x=288;
			scr.y=0;
			NextPressed=true;
		}
	else
	{
		NextPressed=false;
	}
	BlitButton(scr, dest, ">>  ", set );
}

void cHud::PrevButton ( bool set )
{
	SDL_Rect scr={198,172,38,23},dest={60,227,38,23};
	if ( set )
		{
			scr.x=327;
			scr.y=0;
			PrevPressed=true;
		}
	else
	{
		PrevPressed=false;
	}
	BlitButton(scr, dest, "  <<", set );
}

void cHud::CenterButton ( bool set )
{
	SDL_Rect scr={139,149,21,22},dest={4,227,21,22};
	if ( set )
		{
			scr.x=0;
			scr.y=21;
			CenterPressed=true;
		}
	else
	{
		CenterPressed=false;
	}
	BlitButton(scr, dest, "", set);
}

void cHud::DateiButton ( bool set )
{
	SDL_Rect scr={71,151,67,20},dest={17,3,67,20};
	
	if ( set )
		{
			scr.x=93; //change source position
			scr.y=21;
			DateiPressed=true;
		}
	else
	{
		DateiPressed=false;
	}
	BlitButton(scr, dest, lngPack.i18n( "Text~Hud~Files"), set, true);
}

int cHud::BlitButton(SDL_Rect scr, SDL_Rect dest, string sText, bool bPressed)
{
	return BlitButton(GraphicsData.gfx_hud_stuff,scr,GraphicsData.gfx_hud,dest, sText, bPressed, false);
}

int cHud::BlitButton(SDL_Rect scr, SDL_Rect dest, string sText, bool bPressed, bool bSmallFont)
{
	return BlitButton(GraphicsData.gfx_hud_stuff,scr,GraphicsData.gfx_hud,dest, sText, bPressed, bSmallFont);
}

int cHud::BlitButton(SDL_Surface *sfSrc, SDL_Rect scr, SDL_Surface *sfDest, SDL_Rect dest, string sText, bool bPressed, bool bSmallFont)
{
	int iPx = 3;  //for moving fonts 1 pixel down on click
	if(bPressed) iPx += 1;
	SDL_BlitSurface ( sfSrc,&scr,sfDest,&dest );
	if(!bSmallFont)
	{
		font->showTextCentered(dest.x+dest.w/2,dest.y+iPx, sText, LATIN_NORMAL, sfDest);
	}
	else
	{	
		if(bPressed)
		{
			font->showTextCentered(dest.x+dest.w/2,dest.y+iPx+2, sText, LATIN_SMALL_GREEN, sfDest);
			//iPx only +2 because small buttons aren't big enough for moving text on them
		}
		else 
		{
			font->showTextCentered(dest.x+dest.w/2,dest.y+iPx+3, sText, LATIN_SMALL_RED, sfDest);
		}
		font->showTextCentered(dest.x+dest.w/2-1,dest.y+iPx+2, sText, LATIN_SMALL_WHITE, sfDest);
	}
	return 0;
}

void cHud::ScaleSurfaces ( void )
{
	int i, k, sizex, sizey;
	TList *tlist;
	float fak;
	if ( Zoom==LastZoom ) return;

	// Terrain:
	tlist=game->map->TerrainInUse;
	for ( i=0;i<tlist->Count;i++ )
	{
		sTerrain *t;
		t= tlist->TerItems[i];
		if ( Zoom==64 )
		{
			t->sf->w=t->sf_org->w;t->sf->h=t->sf_org->h;
			if ( t->overlay )
			{
				SDL_SetColorKey ( t->sf_org,SDL_SRCCOLORKEY,-1 );
				SDL_SetColorKey ( t->shw_org,SDL_SRCCOLORKEY,-1 );
			}
			SDL_BlitSurface ( t->sf_org,NULL,t->sf,NULL );
			t->shw->w=t->shw_org->w;t->shw->h=t->shw_org->h;
			SDL_BlitSurface ( t->shw_org,NULL,t->shw,NULL );
			if ( t->overlay )
			{
				SDL_SetColorKey ( t->sf_org,SDL_SRCCOLORKEY,0xFF00FF );
				SDL_SetColorKey ( t->shw_org,SDL_SRCCOLORKEY,0xFFCD00CD );
				SDL_SetColorKey ( t->sf,SDL_SRCCOLORKEY,0xFF00FF );
				SDL_SetColorKey ( t->shw,SDL_SRCCOLORKEY,0xCD00CD );
			}
		}
		else
		{
			ScaleSurface2 ( t->sf_org,t->sf,Zoom );
			ScaleSurface2 ( t->shw_org,t->shw,Zoom );
			if ( t->overlay )
			{
				SDL_SetColorKey ( t->shw,SDL_SRCCOLORKEY,0xFFCD00CD );
			}
		}
	}
	// Vehicles:
	fak = ( float ) ( Zoom/64.0 );
	for ( i=0;i<UnitsData.vehicle_anz;i++ )
	{
		for ( k=0;k<8;k++ )
		{
			sizex= (int) ( UnitsData.vehicle[i].img_org[k]->w*fak );
			sizey= (int) ( UnitsData.vehicle[i].img_org[k]->h*fak );
			ScaleSurfaceAdv2 ( UnitsData.vehicle[i].img_org[k],UnitsData.vehicle[i].img[k],sizex,sizey );
			sizex= (int) ( UnitsData.vehicle[i].shw_org[k]->w*fak );
			sizey= (int) ( UnitsData.vehicle[i].shw_org[k]->h*fak );
			ScaleSurfaceAdv2 ( UnitsData.vehicle[i].shw_org[k],UnitsData.vehicle[i].shw[k],sizex,sizey );
		}
		if ( UnitsData.vehicle[i].build_org )
		{
			sizey= (int) ( UnitsData.vehicle[i].build_org->h*fak );
			sizex=sizey*4;
			ScaleSurfaceAdv2 ( UnitsData.vehicle[i].build_org,UnitsData.vehicle[i].build,sizex,sizey );
			sizex= (int) ( UnitsData.vehicle[i].build_shw_org->w*fak );
			sizey= (int) ( UnitsData.vehicle[i].build_shw_org->h*fak );
			ScaleSurfaceAdv2 ( UnitsData.vehicle[i].build_shw_org,UnitsData.vehicle[i].build_shw,sizex,sizey );
		}
		if ( UnitsData.vehicle[i].clear_small_org )
		{
			sizey= (int) ( UnitsData.vehicle[i].clear_small_org->h*fak );
			sizex=sizey*4;
			ScaleSurfaceAdv2 ( UnitsData.vehicle[i].clear_small_org,UnitsData.vehicle[i].clear_small,sizex,sizey );
			sizex= (int) ( UnitsData.vehicle[i].clear_small_shw_org->w*fak );
			sizey= (int) ( UnitsData.vehicle[i].clear_small_shw_org->h*fak );
			ScaleSurfaceAdv2 ( UnitsData.vehicle[i].clear_small_shw_org,UnitsData.vehicle[i].clear_small_shw,sizex,sizey );
		}
		if ( UnitsData.vehicle[i].overlay_org )
		{
			sizey= (int) ( UnitsData.vehicle[i].overlay_org->h*fak );
			sizex= (int) ( UnitsData.vehicle[i].overlay_org->w*fak );
			ScaleSurfaceAdv2 ( UnitsData.vehicle[i].overlay_org,UnitsData.vehicle[i].overlay,sizex,sizey );
		}
	}
	// Buildings:
	fak= ( float ) ( Zoom/64.0 );
	for ( i=0;i<UnitsData.building_anz;i++ )
	{
		ScaleSurfaceAdv2 ( UnitsData.building[i].img_org,UnitsData.building[i].img, (int) UnitsData.building[i].img_org->w*fak, (int) UnitsData.building[i].img_org->h*fak );
		ScaleSurfaceAdv2 ( UnitsData.building[i].shw_org,UnitsData.building[i].shw, (int) UnitsData.building[i].shw_org->w*fak, (int) UnitsData.building[i].shw_org->h*fak );
		if ( UnitsData.building[i].eff_org )
		{
			if ( Zoom==64 ) ScaleSurfaceAdv2 ( UnitsData.building[i].eff_org,UnitsData.building[i].eff, (int) UnitsData.building[i].eff_org->w*fak, (int) UnitsData.building[i].eff_org->h*fak );
			else ScaleSurfaceAdv2Spec ( UnitsData.building[i].eff_org,UnitsData.building[i].eff, (int) UnitsData.building[i].eff_org->w*fak, (int) UnitsData.building[i].eff_org->h*fak );
		}
	}
	ScaleSurfaceAdv2 ( UnitsData.dirt_small_org,UnitsData.dirt_small, (int) UnitsData.dirt_small_org->w*fak, (int) UnitsData.dirt_small_org->h*fak );
	ScaleSurfaceAdv2 ( UnitsData.dirt_small_shw_org,UnitsData.dirt_small_shw, (int) UnitsData.dirt_small_shw_org->w*fak, (int) UnitsData.dirt_small_shw_org->h*fak );
	ScaleSurfaceAdv2 ( UnitsData.dirt_big_org,UnitsData.dirt_big, (int) UnitsData.dirt_big_org->w*fak, (int) UnitsData.dirt_big_org->h*fak );
	ScaleSurfaceAdv2 ( UnitsData.dirt_big_shw_org,UnitsData.dirt_big_shw, (int) UnitsData.dirt_big_shw_org->w*fak, (int) UnitsData.dirt_big_shw_org->h*fak );

	// B�nder:
	ScaleSurface2 ( GraphicsData.gfx_band_small_org,GraphicsData.gfx_band_small,Zoom );
	ScaleSurface2 ( GraphicsData.gfx_band_big_org,GraphicsData.gfx_band_big,Zoom*2 );

	// Resources:
	ScaleSurface2 ( ResourceData.res_metal_org,ResourceData.res_metal,Zoom );
	ScaleSurface2 ( ResourceData.res_oil_org,ResourceData.res_oil,Zoom );
	ScaleSurface2 ( ResourceData.res_gold_org,ResourceData.res_gold,Zoom );

	// Big Beton:
	ScaleSurface2 ( GraphicsData.gfx_big_beton_org,GraphicsData.gfx_big_beton,Zoom*2 );

	// Andere:
	ScaleSurface2 ( GraphicsData.gfx_exitpoints_org,GraphicsData.gfx_exitpoints,Zoom );
	// ScaleSurface2(GraphicsData.gfx_build_finished_org,GraphicsData.gfx_build_finished,Zoom*2);

	// FX:
#define SCALE_FX(a,b) ScaleSurfaceAdv2(a[0],a[1],(int)((a[0]->w/a[0]->h*b)),b);
	SCALE_FX ( EffectsData.fx_explo_small0,Zoom );
	SCALE_FX ( EffectsData.fx_explo_small1,Zoom*2 );
	SCALE_FX ( EffectsData.fx_explo_small2,Zoom*2 );
	SCALE_FX ( EffectsData.fx_explo_big0,Zoom*2 );
	SCALE_FX ( EffectsData.fx_explo_big1,Zoom*2 );
	SCALE_FX ( EffectsData.fx_explo_big2,Zoom*2 );
	SCALE_FX ( EffectsData.fx_explo_big3,Zoom*2 );
	SCALE_FX ( EffectsData.fx_explo_big4,Zoom*2 );
	SCALE_FX ( EffectsData.fx_muzzle_big,Zoom );
	SCALE_FX ( EffectsData.fx_muzzle_small,Zoom );
	SCALE_FX ( EffectsData.fx_muzzle_med,Zoom );
	SCALE_FX ( EffectsData.fx_hit,Zoom );
	SCALE_FX ( EffectsData.fx_smoke, ( int ) ( Zoom/4 ) );
	SCALE_FX ( EffectsData.fx_rocket, ( int ) ( Zoom/2.28 ) );
	SCALE_FX ( EffectsData.fx_dark_smoke, ( int ) ( 0.375*Zoom ) );
	SCALE_FX ( EffectsData.fx_tracks,Zoom );
	SCALE_FX ( EffectsData.fx_corpse,Zoom );
	SCALE_FX ( EffectsData.fx_absorb,Zoom );

	LastZoom=Zoom;
}

// Dr�ck Ende:
void cHud::MakeMeMyEnd ( void )
{
	PlayFX ( SoundData.SNDHudButton );
	if ( game->engine->CheckVehiclesMoving ( false ) )
	{
		EndeButton ( false );
		game->AddMessage ( lngPack.i18n( "Text~Comp~Turn_Wait") );
	}
	else
	{
//    if(game->engine->DoEndActions()&&(!game->engine->com||game->engine->com->server)){
		if ( game->engine->DoEndActions() )
		{
			game->AddMessage ( lngPack.i18n( "Text~Comp~Turn_Automove") );
			if ( !game->engine->fstcpip || game->engine->fstcpip->bServer )
			{
				game->WantToEnd = true;
				if( !game->HotSeat )
				{
					Ende=true;
				}
			}
			else
			{
				Ende = false;
				EndeButton ( false );
			}
		}
		else
		{
			if ( game->HotSeat )
			{
				if ( game->MakeHotSeatEnde() )
				{
					game->engine->EndePressed ( game->ActivePlayer->Nr );
					Ende=true;
				}
			}
			else
			{
				game->engine->EndePressed ( game->ActivePlayer->Nr );
				Ende=true;
			}
		}
	}
}
