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
#include "hud.h"
#include "main.h"
#include "mouse.h"
#include "sound.h"
#include "dialog.h"
#include "unifonts.h"
#include "menu.h"
#include "client.h"
#include "serverevents.h"
#include "keys.h"
#include "input.h"
#include "pcx.h"
#include "player.h"

// Funktionen der Hud-Klasse /////////////////////////////////////////////////
cHud::cHud ( void )
{
	TNT=false;
	bShowPlayers=false;
	MinimapZoom=false;
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
	minimapOffsetX = 0;
	minimapOffsetY = 0;
	minimapZoomFactor = 0;
	Praeferenzen=false;
	PlayFLC=true;
	PausePressed=false;PlayPressed=false;
	HelpPressed=false;ChatPressed=false;
	EndePressed=false;ErledigenPressed=false;
	NextPressed=false;PrevPressed=false;
	CenterPressed=false;DateiPressed=false;
	LogPressed=false;
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
	Client->bFlagDrawHud=true;
	Client->bFlagDrawMMap=true;
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::SwitchPlayers ( bool set )
{
	SDL_Rect scr={317,478,27,28},dest={136,439,27,28};
	if ( set )
		{
			scr.x=344;
			scr.y=478;
		}

	BlitButton(scr, dest, "", false);
	bShowPlayers=set;

	if(Client)
	{
		for ( unsigned int i = 0; i < Client->PlayerList->Size(); i++ )
		{
			cPlayer* const Player = (*Client->PlayerList)[i];
			if (Player)
			{
				ExtraPlayers(Player->name, GetColorNr(Player->color), i, Player->bFinishedTurn);
			}
			else
			{ //should not happen
				ExtraPlayers("~disabled~", 0, i, false, false);
			}
		}
		Client->bFlagDrawHud=true;
		Client->bFlagDrawMMap=true;
	}
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::SwitchMinimapZoom ( bool set )
{
	SDL_Rect scr={334,53,27,26},dest={136,387,27,26};
	if ( set )
		{
			scr.x=362;
			scr.y=53;
		}

	BlitButton(scr, dest, "", false);
	MinimapZoom=set;
	Client->bFlagDrawHud=true;
	Client->bFlagDrawMMap=true;
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
	Client->bFlagDrawHud=true;
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
	Client->bFlagDrawHud=true;
	Client->bFlagDrawMap=true;
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
	Client->bFlagDrawHud=true;
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
	Client->bFlagDrawHud=true;
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
	Client->bFlagDrawHud=true;
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
	Client->bFlagDrawHud=true;
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
	Client->bFlagDrawHud=true;
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
	Client->bFlagDrawHud=true;
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
	Client->bFlagDrawHud=true;
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
	Client->bFlagDrawHud=true;
	PlayFX ( SoundData.SNDHudSwitch );
}

void cHud::DoZoom ( int x,int y )
{
	if ( x<=12 ) x=0;else x-=12;
	if ( x>104 ) x=104;
	x=64- ( int ) ( x* ( 64- ( ( ( SettingsData.iScreenW-192.0 ) /Client->Map->size ) <5?5: ( ( SettingsData.iScreenW-192.0 ) /Client->Map->size ) ) ) /104 );
	if ( x<1 ) x=1;
	SetZoom ( x,y );
}

void cHud::SetZoom ( int zoom,int DestY )
{
	//check for minimum and maximum zoom factor
	int iMinZoom = (int) ceil( (float) max(SettingsData.iScreenH - 32, SettingsData.iScreenW - 192) / Client->Map->size);

	if ( zoom < iMinZoom ) zoom = iMinZoom;
	if ( zoom < 5 ) zoom = 5;
	else if ( zoom > 64 ) zoom = 64;

	static int lastz=64;
	SDL_Rect scr,dest;
//  if(zoom<448/Client->Map->size)zoom=448/Client->Map->size;
	
	Zoom=zoom;
//  zoom-=((448.0/Client->Map->size)<5?5:(448.0/Client->Map->size));
	zoom-= ( int ) ( ( ( ( SettingsData.iScreenW-192.0 ) /Client->Map->size ) <5?5: ( ( SettingsData.iScreenW-192.0 ) /Client->Map->size ) ) );
	scr.x=0;
	scr.y=0;
	dest.x=19;
	dest.y=DestY;
	scr.w=132;
	scr.h=20;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,GraphicsData.gfx_hud,&dest );
//  zoom=106-(int)(zoom*106.0/(64-((448.0/Client->Map->size)<5?5:(448.0/Client->Map->size))));
	zoom=106- ( int ) ( zoom*106.0/ ( 64- ( ( ( SettingsData.iScreenW-192.0 ) /Client->Map->size ) <5?5: ( ( SettingsData.iScreenW-192.0 ) /Client->Map->size ) ) ) );
	scr.x=132;
	scr.y=1;
	dest.x=20+zoom;
	dest.y=DestY+1;
	scr.w=25;
	scr.h=14;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,GraphicsData.gfx_hud,&dest );
	Client->bFlagDrawHud=true;
	Client->bFlagDrawMap=true;
	Client->bFlagDrawMMap=true;

	if ( lastz!=Zoom )
	{
		int off;
//    off=(int)(((448.0/(float)lastz*64)-(448.0/(float)Zoom*64))/2);
		off= ( int ) ( ( ( ( SettingsData.iScreenW-192.0 ) / ( float ) lastz*64 )- ( ( SettingsData.iScreenW-192.0 ) / ( float ) Zoom*64 ) ) /2 );
		lastz=Zoom;

		OffX+=off;
		OffY+=off;
//    off=Client->Map->size*64-(int)((448.0/Zoom)*64);
		off=Client->Map->size*64- ( int ) ( ( ( SettingsData.iScreenW-192.0 ) /Zoom ) *64 );
		if ( OffX > off ) OffX = off;
		if ( OffY > off ) OffY = off;
		if ( OffX < 0   ) OffX = 0;
		if ( OffY < 0   ) OffY = 0;
	}
	if ( SettingsData.bPreScale ) ScaleSurfaces();
}

void cHud::CheckButtons ( void )
{
	int x,y;
	x=mouse->x;
	y=mouse->y;
	if ( x<170 )
	{
		if ( x>=136&&x<=136+27&&y>=413&&y<=413+28 ) {SwitchTNT ( !TNT );return;}
		if ( x>=136&&x<=136+27&&y>=439&&y<=439+28 ) 
		{
			SwitchPlayers( !bShowPlayers );
			reset();
			return;
		}
		if ( x>=136&&x<=136+27&&y>=387&&y<=387+28 ) {SwitchMinimapZoom ( !MinimapZoom );return;}
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
	if ( x>=15&&x<15+112&&y>=356&&y<356+112 ){DoMinimapClick ( x,y );return;}
	lastX=x;
}

void cHud::DoAllHud ( void )
{
	bool s;
	s=SettingsData.bSoundEnabled;
	SettingsData.bSoundEnabled=false;
	
	EndeButton(EndePressed);
	DateiButton(false);
	PraeferenzenButton(false);
	PrevButton(false);
	ErledigenButton(false);
	NextButton(false);
	LogButton(false);
	ChatButton(false);
	DateiButton(false);
	SwitchTNT ( TNT );
	SwitchPlayers( bShowPlayers );
	SwitchMinimapZoom ( MinimapZoom );
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
		mouse->SetCursor ( CHand );
		return;
	}

	cVehicle* selectedVehicle = Client->SelectedVehicle;
	cBuilding* selectedBuilding = Client->SelectedBuilding;

	if ( selectedVehicle&&selectedVehicle->PlaceBand&&selectedVehicle->owner==Client->ActivePlayer )
	{
		if ( x>=180 )
		{
			mouse->SetCursor ( CBand );
		}
		else
		{
			mouse->SetCursor ( CNo );
		}
	}
	else if ( ( selectedVehicle&&selectedVehicle->Transfer&&selectedVehicle->owner==Client->ActivePlayer ) || ( selectedBuilding&&selectedBuilding->Transfer&&selectedBuilding->owner==Client->ActivePlayer ) )
	{
		if ( selectedVehicle )
		{
			if ( Client->OverUnitField && selectedVehicle->CanTransferTo ( Client->OverUnitField ) )
			{
				mouse->SetCursor ( CTransf );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else
		{
			if ( Client->OverUnitField && selectedBuilding->CanTransferTo ( Client->OverUnitField ) )
			{
				mouse->SetCursor ( CTransf );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
	}
	else if ( !Client->bHelpActive )
	{

		if ( x<180 )
		{
			if ( mouse->SetCursor ( CHand ) )
			{
				Client->OverUnitField = NULL;
			}
			return;
		}

		if ( y<2+21&&y>=2&&x>=390&&x<390+72 )
		{
			mouse->SetCursor ( CHand );
			
			Client->OverUnitField = NULL;
			LastOverEnde=true;
			return;
		}
		else if ( LastOverEnde )
		{
			LastOverEnde=false;
			Client->mouseMoveCallback ( true );
		}

		if ( ( selectedVehicle&&selectedVehicle->MenuActive&&selectedVehicle->MouseOverMenu ( x,y ) ) ||
		        ( selectedBuilding&&selectedBuilding->MenuActive&&selectedBuilding->MouseOverMenu ( x,y ) ) )
		{
			mouse->SetCursor ( CHand );
		}
		else if ( selectedVehicle&&selectedVehicle->AttackMode&&selectedVehicle->owner==Client->ActivePlayer&&x>=180&&y>=18&&x<SettingsData.iScreenW-12&&y<SettingsData.iScreenH-14 )
		{
			if ( selectedVehicle->IsInRange ( mouse->GetKachelOff(), Client->Map ) && !( selectedVehicle->data.muzzle_typ == MUZZLE_TORPEDO && !Client->Map->IsWater( mouse->GetKachelOff() ) ))
			{
				if ( mouse->SetCursor ( CAttack ))
				{
					selectedVehicle->DrawAttackCursor( mouse->GetKachelOff());
				}
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->StealActive&&selectedVehicle->owner==Client->ActivePlayer&&x>=180&&y>=18&&x<SettingsData.iScreenW-12&&y<SettingsData.iScreenH-14 )
		{
			if ( selectedVehicle->canDoCommandoAction ( mouse->GetKachelOff()%Client->Map->size, mouse->GetKachelOff()/Client->Map->size, Client->Map, true ) )
			{
				if ( mouse->SetCursor ( CSteal ) )
				{
					selectedVehicle->drawCommandoCursor( mouse->GetKachelOff(), true );
				}
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->DisableActive&&selectedVehicle->owner==Client->ActivePlayer&&x>=180&&y>=18&&x<SettingsData.iScreenW-12&&y<SettingsData.iScreenH-14 )
		{
			if ( selectedVehicle->canDoCommandoAction ( mouse->GetKachelOff()%Client->Map->size, mouse->GetKachelOff()/Client->Map->size, Client->Map, false ) )
			{
				if ( mouse->SetCursor ( CDisable ) )
				{
					selectedVehicle->drawCommandoCursor( mouse->GetKachelOff(), false );
				}
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedBuilding&&selectedBuilding->AttackMode&&selectedBuilding->owner==Client->ActivePlayer&&x>=180&&y>=18&&x<SettingsData.iScreenW-12&&y<SettingsData.iScreenH-14 )
		{
			if ( selectedBuilding->IsInRange ( mouse->GetKachelOff(), Client->Map ) )
			{
				if ( mouse->SetCursor ( CAttack ))
				{
					selectedBuilding->DrawAttackCursor( mouse->GetKachelOff());
				}
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->owner==Client->ActivePlayer&&selectedVehicle->CanAttackObject ( mouse->GetKachelOff(), Client->Map ) )
		{
			if ( mouse->SetCursor ( CAttack ))
			{
				selectedVehicle->DrawAttackCursor( mouse->GetKachelOff() );
			}
		}
		else if ( selectedBuilding&&selectedBuilding->owner==Client->ActivePlayer&&selectedBuilding->CanAttackObject ( mouse->GetKachelOff(), Client->Map ) )
		{
			if ( mouse->SetCursor ( CAttack ))
			{
				selectedBuilding->DrawAttackCursor( mouse->GetKachelOff() );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->owner==Client->ActivePlayer&&selectedVehicle->MuniActive )
		{
			if ( selectedVehicle->canSupply ( mouse->GetKachelOff(), SUPPLY_TYPE_REARM ) )
			{
				mouse->SetCursor ( CMuni );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->owner==Client->ActivePlayer&&selectedVehicle->RepairActive )
		{
			if ( selectedVehicle->canSupply ( mouse->GetKachelOff(), SUPPLY_TYPE_REPAIR ) )
			{
				mouse->SetCursor ( CRepair );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if (Client->OverUnitField && 
				(
					!selectedVehicle                               ||
					selectedVehicle->owner != Client->ActivePlayer ||
					(
						(
							selectedVehicle->data.can_drive == DRIVE_AIR ||
							Client->OverUnitField->getVehicles() ||
							(
								Client->OverUnitField->getTopBuilding() &&
								!Client->OverUnitField->getTopBuilding()->data.is_connector
							) ||
							(
								MouseStyle == OldSchool &&
								Client->OverUnitField->getPlanes()
							)
						) &&
						(
							selectedVehicle->data.can_drive != DRIVE_AIR ||
							Client->OverUnitField->getPlanes() ||
							(
								MouseStyle == OldSchool &&
								(
									Client->OverUnitField->getVehicles() ||
									(
										Client->OverUnitField->getTopBuilding() &&
										!Client->OverUnitField->getTopBuilding()->data.is_connector &&
										!Client->OverUnitField->getTopBuilding()->data.is_pad
									)
								)
							)
						) &&
						!selectedVehicle->LoadActive &&
						!selectedVehicle->ActivatingVehicle
					)
				) &&
				(
					!selectedBuilding                               ||
					selectedBuilding->owner != Client->ActivePlayer ||
					(
						(
							!selectedBuilding->BuildList                    ||
							!selectedBuilding->BuildList->Size()            ||
							selectedBuilding->IsWorking                     ||
							(*selectedBuilding->BuildList)[0]->metall_remaining > 0
						) &&
						!selectedBuilding->LoadActive &&
						!selectedBuilding->ActivatingVehicle
					)
				))
		{
			mouse->SetCursor ( CSelect );
		}
		else if ( selectedVehicle&&selectedVehicle->owner==Client->ActivePlayer&&selectedVehicle->LoadActive )
		{
			if ( selectedVehicle->canLoad ( mouse->GetKachelOff(), Client->Map ) )
			{
				mouse->SetCursor ( CLoad );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->owner==Client->ActivePlayer&&selectedVehicle->ActivatingVehicle )
		{
			int x, y;
			mouse->GetKachel( &x, &y );
			if (selectedVehicle->canExitTo(x, y, Client->Map,selectedVehicle->StoredVehicles[selectedVehicle->VehicleToActivate]->typ) )
			{
				mouse->SetCursor ( CActivate );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->owner==Client->ActivePlayer && x>=180&&y>=18&&x<180+ ( SettingsData.iScreenW-192 ) && y<18+ ( SettingsData.iScreenH-32 ) )
		{
			if ( !selectedVehicle->IsBuilding&&!selectedVehicle->IsClearing&&!selectedVehicle->LoadActive&&!selectedVehicle->ActivatingVehicle )
			{
				if ( selectedVehicle->MoveJobActive )
				{
					mouse->SetCursor ( CNo );
				}
				else if ( Client->Map->possiblePlace( selectedVehicle, mouse->GetKachelOff() ))
				{
					mouse->SetCursor ( CMove );
				}
				else
				{
					mouse->SetCursor ( CNo );
				}
			}
			else if ( selectedVehicle->IsBuilding || selectedVehicle->IsClearing )
			{
				int x, y;
				mouse->GetKachel( &x, &y );
				if ( ( ( selectedVehicle->IsBuilding&&selectedVehicle->BuildRounds == 0 ) || 
					 ( selectedVehicle->IsClearing&&selectedVehicle->ClearingRounds == 0 ) ) &&
					 Client->Map->possiblePlace( selectedVehicle, mouse->GetKachelOff()) && selectedVehicle->isNextTo(x, y))
				{
					mouse->SetCursor( CMove );
				}
				else
				{
					mouse->SetCursor( CNo );
				}
			}
		}
		else if (
				selectedBuilding                                &&
				selectedBuilding->owner == Client->ActivePlayer &&
				selectedBuilding->BuildList                     &&
				selectedBuilding->BuildList->Size()             &&
				!selectedBuilding->IsWorking                    &&
				(*selectedBuilding->BuildList)[0]->metall_remaining <= 0)
		{
			int x, y;
			mouse->GetKachel( &x, &y);
			if ( selectedBuilding->canExitTo(x, y, Client->Map, (*selectedBuilding->BuildList)[0]->typ))
			{
				mouse->SetCursor ( CActivate );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedBuilding&&selectedBuilding->owner==Client->ActivePlayer&&selectedBuilding->ActivatingVehicle )
		{
			int x, y;
			mouse->GetKachel( &x, &y);
			if ( selectedBuilding->canExitTo(x, y, Client->Map, selectedBuilding->StoredVehicles[selectedBuilding->VehicleToActivate]->typ))
			{
				mouse->SetCursor ( CActivate );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedBuilding&&selectedBuilding->owner==Client->ActivePlayer&&selectedBuilding->LoadActive )
		{
			if ( selectedBuilding->canLoad ( mouse->GetKachelOff(), Client->Map ) )
			{
				mouse->SetCursor ( CLoad );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else
		{
			mouse->SetCursor ( CHand );
		}
	}
	else
	{
		mouse->SetCursor ( CHelp );
	}
}

void cHud::DoScroll ( int dir )
{
	static int lx=0,ly=0;
	int step;
	if ( Client->SelectedBuilding )
	{
		Client->SelectedBuilding->MenuActive=false;
	}
	cHud& hud = Client->Hud;
	step=64/Zoom;
	step*=SettingsData.iScrollSpeed;
	switch ( dir )
	{
		case 1:
			hud.OffX-=step;
			hud.OffY+=step;
			break;
		case 2:
			hud.OffY+=step;
			break;
		case 3:
			hud.OffX+=step;
			hud.OffY+=step;
			break;
		case 4:
			hud.OffX-=step;
			break;
		case 6:
			hud.OffX+=step;
			break;
		case 7:
			hud.OffX-=step;
			hud.OffY-=step;
			break;
		case 8:
			hud.OffY-=step;
			break;
		case 9:
			hud.OffX+=step;
			hud.OffY-=step;
			break;
	}
	if ( hud.OffX<0 ) hud.OffX=0;
	if ( hud.OffY<0 ) hud.OffY=0;
	step=Client->Map->size*64- ( int ) ( ( ( SettingsData.iScreenW-192.0 ) /Zoom ) *64 );
	if ( hud.OffX>=step ) hud.OffX=step;
	step=Client->Map->size*64- ( int ) ( ( ( SettingsData.iScreenH-32.0 ) /Zoom ) *64 );
	if ( hud.OffY>=step ) hud.OffY=step;
	if ( lx==OffX&&ly==OffY ) return;
	Client->bFlagDrawMap=true;
	Client->bFlagDrawMMap=true;
	lx=OffX;ly=OffY;
}

void cHud::DoMinimapClick ( int x,int y )
{
	static int lx=0,ly=0;
	if ( lx==x&&ly==y ) return;

	OffX = minimapOffsetX * 64 + ((x - MINIMAP_POS_X) * Client->Map->size * 64) / (MINIMAP_SIZE * minimapZoomFactor);
	OffY = minimapOffsetY * 64 + ((y - MINIMAP_POS_Y) * Client->Map->size * 64) / (MINIMAP_SIZE * minimapZoomFactor);
	OffX -= (SettingsData.iScreenW - 192) * 64 / (Zoom * 2);
	OffY -= (SettingsData.iScreenH -  32) * 64 / (Zoom * 2);

	//workaround for click and hold on the minimap while it is zoomed:
	//we warp the mouse so that it stays over the position of the screen
	//does not work as intended in some cases --Eiko
	int lastMinimapOffsetX = minimapOffsetX;
	int lastMinimapOffsetY = minimapOffsetY;
	Client->drawMiniMap();
	if ( lastMinimapOffsetX != minimapOffsetX )
	{
		x = MINIMAP_POS_X + MINIMAP_SIZE/2;
	}
	if ( lastMinimapOffsetY != minimapOffsetY )
	{
		y = MINIMAP_POS_Y - 1 + MINIMAP_SIZE/2;
	}
	SDL_WarpMouse( x, y );
	lx = x;
	ly = y;

	Client->bFlagDrawHud=true;
	Client->bFlagDrawMap=true;
	DoScroll ( 0 );
}

void cHud::CheckMouseOver ( sMouseState &MouseState )
{
	static int lb=0;
	int x,y,b;
	x=mouse->x;
	y=mouse->y;
	b = MouseState.leftButtonPressed;
	if ( y>274|| ( x>179&&y>22 ) ) return;
	// Der Präferenzen-Button:
	if ( x>=86&&x<=86+67&&y>=4&&y<=4+18 )
	{
		if ( b ) PraeferenzenButton ( true );
		else if ( lb )
		{
			PlayFX ( SoundData.SNDHudButton );
			Client->bChangeObjectName=false;
			Client->bChatInput=false;
			Client->isInMenu = true;
			showPreferences();
			Client->isInMenu = false;
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
			Client->bHelpActive=true;
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
			Client->bChatInput=true;
			InputHandler->setInputStr ( "" );
			InputHandler->setInputState ( true );
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
			//TODO: Log-Menü aufrufen...
			Client->addMessage ( lngPack.i18n( "Text~Error_Messages~INFO_Not_Implemented") );

			LogButton ( false );
		}
	}
	else if ( LogPressed )
	{
		LogButton ( false );
	}
	// the end-button:
	if ( x>=392&&x<460&&y>=5&&y<20 )
	{
		if ( b ) EndeButton ( true );
		else if ( lb&&!Client->bWantToEnd&&!Client->bWaitForOthers )
		{
			PlayFX ( SoundData.SNDHudButton );
			Client->handleEnd ();
		}
	}
	else if ( EndePressed&&!Client->bWantToEnd&&!Client->bWaitForOthers )
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
			if ( Client->SelectedVehicle&&Client->SelectedVehicle->ClientMoveJob&&Client->SelectedVehicle->ClientMoveJob->bSuspended&&Client->SelectedVehicle->data.speed )
			{
				Client->SelectedVehicle->ClientMoveJob->calcNextDir();
				//TODO: no engine!
				//Client->engine->AddActiveMoveJob ( Client->SelectedVehicle->mjob );
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
			v=Client->ActivePlayer->GetNextVehicle();
			if ( v )
			{
				if ( Client->SelectedVehicle )
				{
					Client->SelectedVehicle->Deselct();
					StopFXLoop ( Client->iObjectStream );
				}
				v->Select();
				v->Center();
				Client->iObjectStream=v->playStream();
				Client->SelectedVehicle=v;
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
			v=Client->ActivePlayer->GetPrevVehicle();
			if ( v )
			{
				if ( Client->SelectedVehicle )
				{
					Client->SelectedVehicle->Deselct();
					StopFXLoop ( Client->iObjectStream );
				}
				v->Select();
				v->Center();
				Client->iObjectStream=v->playStream();
				Client->SelectedVehicle=v;
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
			if ( Client->SelectedVehicle )
			{
				Client->SelectedVehicle->Center();
			}
			else if ( Client->SelectedBuilding )
			{
				Client->SelectedBuilding->Center();
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
			Client->isInMenu = true;
			ShowDateiMenu( true );
			Client->isInMenu = false;
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

// Setzt den Monitor zurück:
void cHud::ResetVideoMonitor ( void )
{
	SDL_Rect scr,dest;
	scr.x=295;
	scr.y=98;
	dest.x=10;
	dest.y=29;
	scr.w=128;
	scr.h=128;
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
	scr.w=55;
	scr.h=15;
	dest.x=471;
	dest.y=5;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,GraphicsData.gfx_hud,&dest );
	font->showTextCentered(498,7, iToStr(Client->iTurn), FONT_LATIN_NORMAL, GraphicsData.gfx_hud);
	Client->bFlagDrawHud=true;
}

void cHud::showTurnTime ( int iTime )
{
	SDL_Rect scr,dest;
	scr.x=156;
	scr.y=215;
	scr.w=55;
	scr.h=15;
	dest.x=537;
	dest.y=5;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,GraphicsData.gfx_hud,&dest );
	if ( iTime != -1 ) font->showTextCentered(564,7, iToStr( iTime ), FONT_LATIN_NORMAL, GraphicsData.gfx_hud);
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
		font->showTextCentered(dest.x+dest.w/2,dest.y+iPx, sText, FONT_LATIN_NORMAL, sfDest);
	}
	else
	{
		if(bPressed)
		{
			font->showTextCentered(dest.x+dest.w/2,dest.y+iPx+2, sText, FONT_LATIN_SMALL_GREEN, sfDest);
			//iPx only +2 because small buttons aren't big enough for moving text on them
		}
		else
		{
			font->showTextCentered(dest.x+dest.w/2,dest.y+iPx+3, sText, FONT_LATIN_SMALL_RED, sfDest);
		}
		font->showTextCentered(dest.x+dest.w/2-1,dest.y+iPx+2, sText, FONT_LATIN_SMALL_WHITE, sfDest);
	}
	return 0;
}

void cHud::ScaleSurfaces ( void )
{
	float factor;
	if ( Zoom==LastZoom ) return;

	// Terrain:
	sTerrain*& tlist = Client->Map->terrain;
	int numberOfTerrains = Client->Map->iNumberOfTerrains;
	for (int i = 0; i < numberOfTerrains; ++i)
	{
		sTerrain& t = tlist[i];
		scaleSurface ( t.sf_org, t.sf, Zoom, Zoom );
		scaleSurface ( t.shw_org, t.shw, Zoom, Zoom );
	}
	// Vehicles:
	factor = ( float ) ( Zoom/64.0 );
	for (size_t i = 0; i < UnitsData.vehicle.Size(); ++i)
	{
		UnitsData.vehicle[i].scaleSurfaces( factor );
	}
	// Buildings:
	for (size_t i = 0; i < UnitsData.building.Size(); ++i)
	{
		UnitsData.building[i].scaleSurfaces ( factor );
	}

	if ( UnitsData.dirt_small_org && UnitsData.dirt_small ) scaleSurface ( UnitsData.dirt_small_org,UnitsData.dirt_small, (int) ( UnitsData.dirt_small_org->w * factor ), (int) ( UnitsData.dirt_small_org->h * factor ) );
	if ( UnitsData.dirt_small_shw_org && UnitsData.dirt_small_shw ) scaleSurface ( UnitsData.dirt_small_shw_org,UnitsData.dirt_small_shw, (int) ( UnitsData.dirt_small_shw_org->w * factor ), (int) ( UnitsData.dirt_small_shw_org->h * factor ) );
	if ( UnitsData.dirt_big_org && UnitsData.dirt_big ) scaleSurface ( UnitsData.dirt_big_org,UnitsData.dirt_big, (int) ( UnitsData.dirt_big_org->w * factor ), (int) ( UnitsData.dirt_big_org->h * factor ) );
	if ( UnitsData.dirt_big_shw_org && UnitsData.dirt_big_shw ) scaleSurface ( UnitsData.dirt_big_shw_org,UnitsData.dirt_big_shw, (int) ( UnitsData.dirt_big_shw_org->w * factor ), (int) ( UnitsData.dirt_big_shw_org->h * factor ) );

	// Bänder:
	if ( GraphicsData.gfx_band_small_org && GraphicsData.gfx_band_small ) scaleSurface ( GraphicsData.gfx_band_small_org,GraphicsData.gfx_band_small,Zoom,Zoom );
	if ( GraphicsData.gfx_band_big_org && GraphicsData.gfx_band_big ) scaleSurface ( GraphicsData.gfx_band_big_org,GraphicsData.gfx_band_big,Zoom*2,Zoom*2 );

	// Resources:
	if ( ResourceData.res_metal_org && ResourceData.res_metal ) scaleSurface ( ResourceData.res_metal_org,ResourceData.res_metal,ResourceData.res_metal_org->w/64*Zoom,Zoom );
	if ( ResourceData.res_oil_org && ResourceData.res_oil ) scaleSurface ( ResourceData.res_oil_org,ResourceData.res_oil,ResourceData.res_oil_org->w/64*Zoom,Zoom );
	if ( ResourceData.res_gold_org && ResourceData.res_gold ) scaleSurface ( ResourceData.res_gold_org,ResourceData.res_gold,ResourceData.res_gold_org->w/64*Zoom,Zoom );

	// Big Beton:
	if ( GraphicsData.gfx_big_beton_org && GraphicsData.gfx_big_beton ) scaleSurface ( GraphicsData.gfx_big_beton_org,GraphicsData.gfx_big_beton,Zoom*2,Zoom*2 );

	// Andere:
	if ( GraphicsData.gfx_exitpoints_org && GraphicsData.gfx_exitpoints ) scaleSurface ( GraphicsData.gfx_exitpoints_org,GraphicsData.gfx_exitpoints,GraphicsData.gfx_exitpoints_org->w/64*Zoom,Zoom );

	// FX:
#define SCALE_FX(a) if (a) scaleSurface(a[0],a[1], (a[0]->w * Zoom)/64 , (a[0]->h * Zoom)/64);
	SCALE_FX ( EffectsData.fx_explo_small );
	SCALE_FX ( EffectsData.fx_explo_big );
	SCALE_FX ( EffectsData.fx_explo_water );
	SCALE_FX ( EffectsData.fx_explo_air );
	SCALE_FX ( EffectsData.fx_muzzle_big );
	SCALE_FX ( EffectsData.fx_muzzle_small );
	SCALE_FX ( EffectsData.fx_muzzle_med );
	SCALE_FX ( EffectsData.fx_hit );
	SCALE_FX ( EffectsData.fx_smoke );
	SCALE_FX ( EffectsData.fx_rocket );
	SCALE_FX ( EffectsData.fx_dark_smoke );
	SCALE_FX ( EffectsData.fx_tracks );
	SCALE_FX ( EffectsData.fx_corpse );
	SCALE_FX ( EffectsData.fx_absorb );

	LastZoom=Zoom;
}

void cHud::ExtraPlayers ( string sPlayer, int iColor, int iPos, bool bFinished, bool bActive)
{
	if(bShowPlayers)
	{
		//BEGIN PREP WORK
		//draw players beside minimap
		SDL_Rect rDest;
		SDL_Rect rSrc = { 0, 0, GraphicsData.gfx_hud_extra_players->w, GraphicsData.gfx_hud_extra_players->h};
		
		if(SettingsData.iScreenH >= 768) //draw players under minimap if screenres is big enough
		{
			rSrc.x = 18; //skip eyecandy spit before playerbar
	
			rDest.x = 3;
			rDest.y = 482 + GraphicsData.gfx_hud_extra_players->h * iPos; //draw players downwards
			rDest.w = GraphicsData.gfx_hud_extra_players->w-rSrc.x;
			rDest.h = GraphicsData.gfx_hud_extra_players->h;
		}
		else //draw players beside minimap if screenres is to small
		{
			rDest.x = 161;
			rDest.y = 480 - 82 - GraphicsData.gfx_hud_extra_players->h * iPos; //draw players upwards
			rDest.w = GraphicsData.gfx_hud_extra_players->w;
			rDest.h = GraphicsData.gfx_hud_extra_players->h;
		}
	
	
		SDL_Rect rDot = { 10 , 0, 10, 10 }; //for green dot
		SDL_Rect rDotDest = { rDest.x + 23 - rSrc.x, rDest.y + 6, rDot.w, rDot.h };
	
		SDL_Rect rColorSrc = { 0, 0, 10, 12 };
		SDL_Rect rColorDest = { rDest.x + 40 - rSrc.x, rDest.y + 6, rColorSrc.w, rColorSrc.h };
		//END PREP WORK
		//BEGIN DRAW PLAYERS
		SDL_BlitSurface( GraphicsData.gfx_hud_extra_players, &rSrc, GraphicsData.gfx_hud, &rDest ); //blit box
		if(!bFinished)
		{
			rDot.x = 0; //red dot
		}
		if(bActive) 
		{
			SDL_BlitSurface( GraphicsData.gfx_player_ready, &rDot, GraphicsData.gfx_hud, &rDotDest ); //blit dot
			SDL_BlitSurface(OtherData.colors[iColor], &rColorSrc, GraphicsData.gfx_hud, &rColorDest ); //blit color
		}
		else
		{
			font->showText(rColorDest.x+3, rColorDest.y+2, "X" , FONT_LATIN_SMALL_RED, GraphicsData.gfx_hud); //blit X for defeated/dropped players
		}
		font->showText(rDest.x+= (59 - rSrc.x), rDest.y+6, sPlayer , FONT_LATIN_NORMAL, GraphicsData.gfx_hud); //blit name
		//END DRAW PLAYERS
	}


}

void cHud::reset()
{
	SDL_FillRect ( GraphicsData.gfx_hud, NULL, 0xFF00FF );
	SDL_BlitSurface ( GraphicsData.gfx_hud_backup, NULL, GraphicsData.gfx_hud, NULL);
	DoAllHud();
	if ( Client && Client->SelectedVehicle ) Client->SelectedVehicle->ShowDetails();
	if ( Client && Client->SelectedBuilding ) Client->SelectedBuilding->ShowDetails();
}
