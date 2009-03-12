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
#include "buttons.h"
#include "math.h"
#include "vehicles.h"
#include "unifonts.h"
#include "mouse.h"
#include "sound.h"
#include "map.h"
#include "dialog.h"
#include "files.h"
#include "pcx.h"
#include "events.h"
#include "client.h"
#include "server.h"
#include "clientevents.h"
#include "attackJobs.h"

// Funktionen der Vehicle Klasse /////////////////////////////////////////////
cVehicle::cVehicle ( sVehicle *v, cPlayer *Owner )
{
	typ = v;
	PosX = 0;
	PosY = 0;
	BandX = 0;
	BandY = 0;
	OffX = 0;
	OffY = 0;
	dir = 0;
	ditherX = 0;
	ditherY = 0;
	StartUp = 0;
	FlightHigh = 64;
	WalkFrame = 0;
	CommandoRank = 0;
	Disabled = 0;
	BuildingTyp.iFirstPart = 0;
	BuildingTyp.iSecondPart = 0;
	BuildCosts = 0;
	BuildRounds = 0;
	BuildRoundsStart = 0;
	ClearingRounds = 0;
	VehicleToActivate = 0;
	BuildBigSavedPos = 0;
	selected = false;
	owner = Owner;
	data = owner->VehicleData[typ->nr];
	data.hit_points = data.max_hit_points;
	data.ammo = data.max_ammo;
	ClientMoveJob = NULL;
	ServerMoveJob = NULL;
	autoMJob = NULL;
	hasAutoMoveJob = false;
	moving = false;
	MoveJobActive = false;
	MenuActive = false;
	AttackMode = false;
	Attacking = false;
	IsBuilding = false;
	PlaceBand = false;
	IsClearing = false;
	bSentryStatus = false;
	Transfer = false;
	LoadActive = false;
	ActivatingVehicle = false;
	MuniActive = false;
	RepairActive = false;
	BuildPath = false;
	LayMines = false;
	ClearMines = false;
	Loaded = false;
	StealActive = false;
	DisableActive = false;
	IsLocked = false;
	bIsBeeingAttacked = false;
	BigBetonAlpha = 0;

	DamageFXPointX = random(7) + 26 - 3;
	DamageFXPointY = random(7) + 26 - 3;
	refreshData();
}

cVehicle::~cVehicle ( void )
{
	if ( ClientMoveJob )
	{
		ClientMoveJob->release();
		ClientMoveJob->Vehicle = NULL;
	}
	if ( ServerMoveJob )
	{
		ServerMoveJob->release();
		ServerMoveJob->Vehicle = NULL;
	}

	if ( autoMJob )
	{
		delete autoMJob;
	}

	if ( bSentryStatus )
	{
		owner->deleteSentryVehicle ( this );
	}

	DeleteStored();


	if ( IsLocked )
	{
		cPlayer *p;
		unsigned int i;

		for ( i = 0;i < Client->PlayerList->Size();i++ )
		{
			p = (*Client->PlayerList)[i];
			p->DeleteLock ( this );
		}
	}

	while ( SeenByPlayerList.Size() )
	{
		SeenByPlayerList.Delete ( SeenByPlayerList.Size()-1 );
	}
}

// Malt das Vehicle:
void cVehicle::Draw ( SDL_Rect *dest )
{
	SDL_Rect scr, tmp;
	int ox = 0, oy = 0;

	// Workarounds:

	if ( ( !ClientMoveJob || !MoveJobActive ) && ( OffX != 0 || OffY != 0 ) )
	{
		OffX = 0;
		OffY = 0;
	}

	if ( !ClientMoveJob && ( MoveJobActive || moving ) )
	{
		//TODO: remove
		MoveJobActive = false;
		moving = false;
		StopFXLoop ( Client->iObjectStream );
		Client->iObjectStream = PlayStram();
	}

	//make damage effect
	if ( Client->iTimer1 && data.hit_points < data.max_hit_points && SettingsData.bDamageEffects && ( owner == Client->ActivePlayer || Client->ActivePlayer->ScanMap[PosX+PosY*Client->Map->size] ) )
	{
		int intense = ( int ) ( 100 - 100 * ( ( float ) data.hit_points / data.max_hit_points ) );
		Client->addFX ( fxDarkSmoke, PosX*64 + DamageFXPointX + OffX, PosY*64 + DamageFXPointY + OffY, intense );
	}

	//make landing and take off of planes
	if ( data.can_drive == DRIVE_AIR && Client->iTimer0 )
	{
		// check, if the plane should land
		cBuilding *b = Client->Map->fields[PosX+PosY*Client->Map->size].getTopBuilding();

		if ( b && b->owner == owner && b->data.is_pad && !ClientMoveJob && !moving && !Attacking )
		{
			FlightHigh -= 8;
			if ( FlightHigh < 0 ) FlightHigh = 0;
		}
		else
		{
			FlightHigh += 8;
			if ( FlightHigh > 64 ) FlightHigh = 64;
		}
	}


	float newzoom = (float)( 64.0 / Client->Hud.Zoom );
	float factor = (float)(Client->Hud.Zoom/64.0);

	if ( OffX )
		ox = ( int ) ( OffX / newzoom );

	if ( OffY )
		oy = ( int ) ( OffY / newzoom );

	tmp = *dest;

	tmp.x += ox;

	tmp.y += oy;

	if ( (IsBuilding || IsClearing) && data.is_big )
		dir = 0;

	// Check, if the vehicle is building
	if ( ( !IsBuilding && !IsClearing ) || dir != 0 )
	{
		if ( ( IsBuilding || IsClearing ) && Client->iTimer0 )
		{
			// rotate vehicle
			RotateTo ( 0 );
		}

		// read the size:
		scr.w = (int)(typ->img_org[dir]->w*factor);

		scr.h = (int)(typ->img_org[dir]->h*factor);

		// draw shadow
		if ( SettingsData.bShadows && ! ( data.is_stealth_sea && Client->Map->IsWater ( PosX + PosY*Client->Map->size, true ) ) )
		{
			if ( StartUp && SettingsData.bAlphaEffects )
			{
				SDL_SetAlpha ( typ->shw[dir], SDL_SRCALPHA, StartUp / 5 );

				if ( data.can_drive != DRIVE_AIR )
				{
					if ( data.is_human )
					{
						SDL_Rect r;
						r.h = r.w = typ->img[dir]->h;
						r.x = r.w * WalkFrame;
						r.y = 0;
						blitWithPreScale ( typ->shw_org[dir], typ->shw[dir], &r, buffer, &tmp, factor );
					}
					else
					{
						blitWithPreScale ( typ->shw_org[dir], typ->shw[dir], NULL, buffer, &tmp, factor );
					}
				}
				else
				{
					int high;
					high = ( ( int ) ( Client->Hud.Zoom * ( FlightHigh / 64.0 ) ) );
					tmp.x += high + ditherX;
					tmp.y += high + ditherY;
					blitWithPreScale ( typ->shw_org[dir], typ->shw[dir], NULL, buffer, &tmp, factor );
				}

				SDL_SetAlpha ( typ->shw[dir], SDL_SRCALPHA, 50 );
			}
			else
			{
				if ( data.can_drive != DRIVE_AIR )
				{
					if ( data.is_human )
					{
						SDL_Rect r;
						r.h = r.w = typ->img[dir]->h;
						r.x = r.w * WalkFrame;
						r.y = 0;
						blitWithPreScale ( typ->shw_org[dir], typ->shw[dir], &r, buffer, &tmp, factor );
					}
					else
					{
						blitWithPreScale ( typ->shw_org[dir], typ->shw[dir], NULL, buffer, &tmp, factor );
					}
				}
				else
				{
					// draw shadow
					if ( FlightHigh > 0 )
					{
						int high = ( ( int ) ( Client->Hud.Zoom * ( FlightHigh / 64.0 ) ) );
						tmp.x += high + ditherX;
						tmp.y += high + ditherY;
					}

					blitWithPreScale ( typ->shw_org[dir], typ->shw[dir], NULL, buffer, &tmp, factor );
				}
			}
		}

		// draw player color
		SDL_BlitSurface ( owner->color, NULL, GraphicsData.gfx_tmp, NULL );

		if ( data.is_human )
		{
			scr.w = scr.h = tmp.h = tmp.w = typ->img[dir]->h;
			tmp.x = WalkFrame * tmp.w;
			tmp.y = 0;
			blitWithPreScale ( typ->img_org[dir], typ->img[dir], &tmp, GraphicsData.gfx_tmp, NULL, factor );
		}
		else
		{
			blitWithPreScale ( typ->img_org[dir], typ->img[dir], NULL, GraphicsData.gfx_tmp, NULL, factor );
		}

		// draw the vehicle
		scr.x = 0;

		scr.y = 0;

		tmp = *dest;

		if ( FlightHigh > 0 )
		{
			tmp.x += ox + ditherX;
			tmp.y += oy + ditherY;
		}
		else
		{
			tmp.x += ox;
			tmp.y += oy;
		}

		if ( StartUp && SettingsData.bAlphaEffects )
		{
			SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, StartUp );
			SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, buffer, &tmp );
			SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, 255 );

			if ( Client->iTimer0 )
				StartUp += 25;

			if ( StartUp >= 255 )
				StartUp = 0;

			if ( data.is_stealth_sea && Client->Map->IsWater ( PosX + PosY*Client->Map->size, true ) && 
				 DetectedByPlayerList.Size() == 0 && owner == Client->ActivePlayer )
				StartUp = 0;
		}
		else
		{
			bool water = Client->Map->IsWater(PosX + PosY*Client->Map->size, true);
			//if the vehicle can also drive on land, we have to check, whether there is a brige, platform, etc.
			//because the vehicle will drive on the bridge
			cBuilding* building = Client->Map->fields[PosX + PosY*Client->Map->size].getBaseBuilding();
			if ( building && data.can_drive != DRIVE_SEA && ( building->data.is_bridge || building->data.is_platform || building->data.is_road )) water = false;

			if ( data.is_stealth_sea && water &&
				 DetectedByPlayerList.Size() == 0 && owner == Client->ActivePlayer )
			{
				SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, 100 );
				SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, buffer, &tmp );
				SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, 255 );
			}
			else
			{
				SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, buffer, &tmp );
			}
		}

		// draw overlay if necessary:
		if ( data.has_overlay && SettingsData.bAnimations )
		{
			tmp = *dest;
			scr.h = scr.w = (int)(typ->overlay_org->h*factor);
			tmp.x += Client->Hud.Zoom / 2 - scr.h / 2 + ox + ditherX;
			tmp.y += Client->Hud.Zoom / 2 - scr.h / 2 + oy + ditherY;
			scr.y = 0;

			if ( Disabled )
			{
				scr.x = 0;
			}
			else
			{
				scr.x = ( int ) ( ( typ->overlay_org->h * ( ( Client->iFrame % ( (int)(typ->overlay_org->w*factor) / scr.h ) ) ) ) / newzoom );
			}

			if ( StartUp && SettingsData.bAlphaEffects )
			{
				SDL_SetAlpha ( typ->overlay, SDL_SRCALPHA, StartUp );
				blitWithPreScale ( typ->overlay_org, typ->overlay, &scr, buffer, &tmp, factor );
				SDL_SetAlpha ( typ->overlay, SDL_SRCALPHA, 255 );

				if ( Client->iTimer0 )
					StartUp += 25;

				if ( StartUp >= 255 )
					StartUp = 0;
			}
			else
			{
				blitWithPreScale ( typ->overlay_org, typ->overlay, &scr, buffer, &tmp, factor );
			}
		}
	}
	else
		if ( IsBuilding || ( IsClearing && data.is_big ) )
		{
			//draw beton if nessesary
			if ( IsBuilding && data.is_big && ( !Client->Map->IsWater(PosX+PosY*Client->Map->size) || Client->Map->fields[PosX+PosY*Client->Map->size].getBaseBuilding()) )
			{
				SDL_SetAlpha ( GraphicsData.gfx_big_beton, SDL_SRCALPHA, BigBetonAlpha );
				if ( !SettingsData.bPreScale && ( GraphicsData.gfx_big_beton->w != Client->Hud.Zoom*2 || GraphicsData.gfx_big_beton->h != Client->Hud.Zoom*2 ) ) scaleSurface ( GraphicsData.gfx_big_beton_org, GraphicsData.gfx_big_beton, Client->Hud.Zoom*2, Client->Hud.Zoom*2 );
				SDL_BlitSurface ( GraphicsData.gfx_big_beton, NULL, buffer, &tmp );
				tmp = *dest;

				if ( BigBetonAlpha < 255 )
				{
					if ( Client->iTimer0 )
						BigBetonAlpha += 25;

					if ( BigBetonAlpha > 255 )
						BigBetonAlpha = 255;
				}
			}

			// draw shadow
			if ( SettingsData.bShadows )
			{
				blitWithPreScale ( typ->build_shw_org, typ->build_shw, NULL, buffer, &tmp, factor );
			}

			// draw player color
			scr.y = 0;

			scr.h = scr.w = (int)(typ->build_org->h*factor);

			scr.x = ( Client->iFrame % 4 ) * scr.w;

			SDL_BlitSurface ( owner->color, NULL, GraphicsData.gfx_tmp, NULL );

			blitWithPreScale ( typ->build_org, typ->build, &scr, GraphicsData.gfx_tmp, NULL, factor, 4 );

			// draw vehicle
			scr.x = 0;

			scr.y = 0;

			tmp = *dest;

			tmp.x += ox;

			tmp.y += oy;

			SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, buffer, &tmp );

			// draw indication, when building is complete
			if ( ( ( IsBuilding && BuildRounds == 0 ) || ( IsClearing && ClearingRounds == 0 ) ) && owner == Client->ActivePlayer && !BuildPath )
			{
				SDL_Rect d, t;
				int max, nr;
				nr = 0xFF00 - ( ( Client->iFrame % 0x8 ) * 0x1000 );

				if ( data.can_build == BUILD_BIG || IsClearing )
				{
					max = ( Client->Hud.Zoom * 2) - 3;
				}
				else
				{
					max = Client->Hud.Zoom - 3;
				}

				d.x = dest->x + 2 + ox;

				d.y = dest->y + 2 + oy;
				d.w = max;
				d.h = 3;
				t = d;
				SDL_FillRect ( buffer, &d, nr );
				d = t;
				d.y += max - 3;
				t = d;
				SDL_FillRect ( buffer, &d, nr );
				d = t;
				d.y = dest->y + 2 + oy;
				d.w = 3;
				d.h = max;
				t = d;
				SDL_FillRect ( buffer, &d, nr );
				d = t;
				d.x += max - 3;
				SDL_FillRect ( buffer, &d, nr );
			}
		}
		else
		{
			// draw shadow
			if ( SettingsData.bShadows )
			{
				blitWithPreScale ( typ->clear_small_shw_org, typ->clear_small_shw, NULL, buffer, &tmp, factor );
			}

			// draw player color
			scr.y = 0;

			scr.h = scr.w = typ->clear_small->h;

			scr.x = ( Client->iFrame % 4 ) * scr.w;

			SDL_BlitSurface ( owner->color, NULL, GraphicsData.gfx_tmp, NULL );

			blitWithPreScale ( typ->clear_small_org, typ->clear_small, &scr, GraphicsData.gfx_tmp, NULL, factor, 4 );

			// draw vehicle
			scr.x = 0;

			scr.y = 0;

			tmp = *dest;

			tmp.x += ox;

			tmp.y += oy;

			SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, buffer, &tmp );

			// draw indication, when building is complete
			if ( ClearingRounds == 0 && owner == Client->ActivePlayer )
			{
				SDL_Rect d, t;
				int max, nr;
				nr = 0xFF00 - ( ( Client->iFrame % 0x8 ) * 0x1000 );
				max = Client->Hud.Zoom - 3;
				d.x = dest->x + 2 + ox;
				d.y = dest->y + 2 + oy;
				d.w = max;
				d.h = 1;
				t = d;
				SDL_FillRect ( buffer, &d, nr );
				d = t;
				d.y += max - 1;
				t = d;
				SDL_FillRect ( buffer, &d, nr );
				d = t;
				d.y = dest->y + 2 + oy;
				d.w = 1;
				d.h = max;
				t = d;
				SDL_FillRect ( buffer, &d, nr );
				d = t;
				d.x += max - 1;
				SDL_FillRect ( buffer, &d, nr );
			}
		}

	// Ggf den farbigen Rahmen malen:
	if ( Client->Hud.Farben )
	{
		SDL_Rect d, t;
		int max, nr;
		nr = * ( unsigned int* ) owner->color->pixels;

		if ( data.is_big )
		{
			max = ( Client->Hud.Zoom - 1 ) * 2;
		}
		else
		{
			max = Client->Hud.Zoom - 1;
		}

		d.x = dest->x + 1 + ox;

		d.y = dest->y + 1 + oy;
		d.w = max;
		d.h = 1;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.y += max - 1;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.y = dest->y + 1 + oy;
		d.w = 1;
		d.h = max;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.x += max - 1;
		SDL_FillRect ( buffer, &d, nr );
	}

	// Ggf den Rahmen malen:
	if ( selected )
	{
		SDL_Rect d, t;
		int len, max;

		if ( ( IsBuilding || IsClearing ) && data.is_big )
		{
			max = Client->Hud.Zoom * 2;
		}
		else
		{
			max = Client->Hud.Zoom;
		}

		len = max / 4;

		d.x = dest->x + 1 + ox;
		d.y = dest->y + 1 + oy;
		d.w = len;
		d.h = 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
		d = t;
		d.x += max - len - 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
		d = t;
		d.y += max - 2;
		t = d;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
		d = t;
		d.x = dest->x + 1 + ox;
		t = d;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
		d = t;
		d.y = dest->y + 1 + oy;
		d.w = 1;
		d.h = len;
		t = d;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
		d = t;
		d.x += max - 2;
		t = d;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
		d = t;
		d.y += max - len - 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
		d = t;
		d.x = dest->x + 1 + ox;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
	}

	// Das Dithering machen:

	if ( data.can_drive == DRIVE_AIR && Client->iTimer0 )
	{
		if ( moving || Client->iFrame % 10 == 0 )
		{
			ditherX = 0;
			ditherY = 0;
		}
		else
		{
			ditherX = random(2) - 1;
			ditherY = random(2) - 1;
		}
	}

	// Draw the bridge, if necessary
	if ( data.can_drive == DRIVE_SEA )
	{
#define TEST_BRIDGE(x,y) PosX+x>=0&&PosX+x<Client->Map->size&&PosY+y>=0&&PosY+y<Client->Map->size&&Client->Map->fields[PosX+(x)+(PosY+(y))*Client->Map->size].getBaseBuilding()&&Client->Map->fields[PosX+(x)+(PosY+(y))*Client->Map->size].getBaseBuilding()->data.is_bridge

		if ( TEST_BRIDGE ( 0, 0 ) )
		{
			Client->Map->fields[PosX+PosY*Client->Map->size].getBaseBuilding()->Draw ( dest );
		}

		if ( OffX > 0 && OffY == 0 && TEST_BRIDGE ( 1, 0 ) )
		{
			tmp = *dest;
			tmp.x += Client->Hud.Zoom;
			Client->Map->fields[PosX+1+PosY*Client->Map->size].getBaseBuilding()->Draw ( &tmp );
		}
		else if ( OffX < 0 && OffY == 0 && TEST_BRIDGE ( -1, 0 ) )
		{
			tmp = *dest;
			tmp.x -= Client->Hud.Zoom;
			Client->Map->fields[PosX-1+PosY*Client->Map->size].getBaseBuilding()->Draw ( &tmp );
		}
		else if ( OffX == 0 && OffY > 0 && TEST_BRIDGE ( 0, 1 ) )
		{
			tmp = *dest;
			tmp.y += Client->Hud.Zoom;
			Client->Map->fields[PosX+ ( PosY+1 ) *Client->Map->size].getBaseBuilding()->Draw ( &tmp );
		}
		else if ( OffX == 0 && OffY < 0 && TEST_BRIDGE ( 0, -1 ) )
		{
			tmp = *dest;
			tmp.y -= Client->Hud.Zoom;
			Client->Map->fields[PosX+ ( PosY-1 ) *Client->Map->size].getBaseBuilding()->Draw ( &tmp );
		}
		else if ( OffX > 0 && OffY > 0 && TEST_BRIDGE ( 1, 1 ) )
		{
			tmp = *dest;
			tmp.x += Client->Hud.Zoom;
			tmp.y += Client->Hud.Zoom;
			Client->Map->fields[PosX+1+ ( PosY+1 ) *Client->Map->size].getBaseBuilding()->Draw( &tmp );
		}
		else if ( OffX < 0 && OffY < 0 && TEST_BRIDGE ( -1, -1 ) )
		{
			tmp = *dest;
			tmp.x -= Client->Hud.Zoom;
			tmp.y -= Client->Hud.Zoom;
			Client->Map->fields[PosX-1+ ( PosY-1 ) *Client->Map->size].getBaseBuilding()->Draw ( &tmp );
		}
		else if ( OffX > 0 && OffY < 0 && TEST_BRIDGE ( 1, -1 ) )
		{
			tmp = *dest;
			tmp.x += Client->Hud.Zoom;
			tmp.y -= Client->Hud.Zoom;
			Client->Map->fields[PosX+1+ ( PosY-1 ) *Client->Map->size].getBaseBuilding()->Draw ( &tmp );
		}
		else if ( OffX < 0 && OffY > 0 && TEST_BRIDGE ( -1, 1 ) )
		{
			tmp = *dest;
			tmp.x -= Client->Hud.Zoom;
			tmp.y += Client->Hud.Zoom;
			Client->Map->fields[PosX-1+ ( PosY+1 ) *Client->Map->size].getBaseBuilding()->Draw ( &tmp );
		}
	}

	//draw health bar
	if ( Client->Hud.Treffer )
	{
		DrawHelthBar();
	}

	//draw ammo bar
	if ( Client->Hud.Munition && data.can_attack)
	{
		DrawMunBar();
	}

	//draw status info
	if ( Client->Hud.Status )
	{
		drawStatus();
	}

	//attack job debug output
	if ( Client->bDebugAjobs )
	{
		cVehicle* serverVehicle = NULL;
		if ( Server ) serverVehicle = Server->Map->fields[PosX + PosY * Server->Map->size].getVehicles();
		if ( bIsBeeingAttacked ) font->showText(dest->x + 1,dest->y + 1, "C: attacked", FONT_LATIN_SMALL_WHITE );
		if ( serverVehicle && serverVehicle->bIsBeeingAttacked ) font->showText(dest->x + 1,dest->y + 9, "S: attacked", FONT_LATIN_SMALL_YELLOW );
		if ( Attacking ) font->showText(dest->x + 1,dest->y + 17, "C: attacking", FONT_LATIN_SMALL_WHITE );
		if ( serverVehicle && serverVehicle->Attacking ) font->showText(dest->x + 1,dest->y + 25, "S: attacking", FONT_LATIN_SMALL_YELLOW );
	}

}

// Selects the vehicle
void cVehicle::Select ( void )
{
	int error;
	selected = true;
	// Das Video laden:

	if ( Client->FLC != NULL )
	{
		FLI_Close ( Client->FLC );
		
	}

	Client->FLC = NULL;
	Client->sFLCname = "";
	Client->video = NULL;
	

	if(FileExists(typ->FLCFile))
	{
		Client->FLC = FLI_Open ( SDL_RWFromFile ( typ->FLCFile, "rb" ), &error );
		Client->sFLCname = typ->FLCFile;
	}
	else
	{	//in case the flc video doesn't exist we use the storage image instead
		Client->video = typ->storage;
	}

	if ( error != 0 )
	{
		Client->Hud.ResetVideoMonitor();
	}
	else
	{
		FLI_Rewind ( Client->FLC );

		FLI_NextFrame ( Client->FLC );
	}


	// Meldung machen:
	MakeReport();

	// Die Eigenschaften anzeigen:
	ShowDetails();
}

// Deselectiert das Vehicle:
void cVehicle::Deselct ( void )
{
	SDL_Rect scr, dest;
	selected = false;
	MenuActive = false;
	AttackMode = false;
	if ( PlaceBand ) BuildPath = false;
	PlaceBand = false;
	Transfer = false;
	LoadActive = false;
	ActivatingVehicle = false;
	MuniActive = false;
	RepairActive = false;
	StealActive = false;
	DisableActive = false;
	// Den Hintergrund wiederherstellen:
	scr.x = 0;
	scr.y = 215;
	scr.w = 155;
	scr.h = 48;
	dest.x = 8;
	dest.y = 171;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, GraphicsData.gfx_hud, &dest );
	StopFXLoop ( Client->iObjectStream );
	Client->iObjectStream = -1;
}

// Generates the name of the vehicle based on the version
void cVehicle::GenerateName ( void )
{
	string rome, tmp_name;
	int nr, tmp;
	string::size_type tmp_name_idx;
	rome = "";
	nr = data.version;

	// genrate roman version number (correct until 899)

	if ( nr > 100 )
	{
		tmp = nr / 100;
		nr %= 100;

		while ( tmp-- )
		{
			rome += "C";
		}
	}

	if ( nr >= 90 )
	{
		rome += "XC";
		nr -= 90;
	}

	if ( nr >= 50 )
	{
		nr -= 50;
		rome += "L";
	}

	if ( nr >= 40 )
	{
		nr -= 40;
		rome += "XL";
	}

	if ( nr >= 10 )
	{
		tmp = nr / 10;
		nr %= 10;

		while ( tmp-- )
		{
			rome += "X";
		}
	}

	if ( nr == 9 )
	{
		nr -= 9;
		rome += "IX";
	}

	if ( nr >= 5 )
	{
		nr -= 5;
		rome += "V";
	}

	if ( nr == 4 )
	{
		nr -= 4;
		rome += "IV";
	}

	while ( nr-- )
	{
		rome += "I";
	}

	// Den Namen zusammenbauen:
	if ( name.length() == 0 )
	{
		// prefix
		name = "MK ";
		name += rome;
		name += " ";
		// object name
		name += ( string ) data.name;
	}
	else
	{
		// check for MK prefix
		tmp_name = name.substr(0,2);
		if ( 0 == (int)tmp_name.compare("MK") )
		{
			// current name, without prefix
			tmp_name_idx = name.find_first_of(" ", 4 );
			if( tmp_name_idx != string::npos )
			{
				tmp_name = ( string )name.substr(tmp_name_idx);
				// prefix
				name = "MK ";
				name += rome;
				// name
				name += tmp_name;
			}
			else
			{
				tmp_name = name;
				// prefix
				name = "MK ";
				name += rome;
				name += " ";
				// name
				name += tmp_name;
			}
		}
		else
		{
			tmp_name = name;
			name = "MK ";
			name += rome;
			name += " ";
			name += tmp_name;
		}
	}
}

// Aktalisiert alle Daten auf ihre Max-Werte:
int cVehicle::refreshData ()
{
	int iReturn = 0;
	if ( data.speed < data.max_speed || data.shots < data.max_shots )
	{
		data.speed = data.max_speed;

		if ( data.ammo >= data.max_shots )
		{
			data.shots = data.max_shots;
		}
		else
		{
			data.shots = data.ammo;
		}

		/*// Regenerieren:
		if ( data.is_alien && data.hit_points < data.max_hit_points )
		{
			data.hit_points++;
		}*/
		iReturn = 1;
	}


	// Bauen:
	if ( IsBuilding && BuildRounds )
	{

		data.cargo -= ( BuildCosts / BuildRounds );
		BuildCosts -= ( BuildCosts / BuildRounds );

		if ( data.cargo < 0 ) data.cargo = 0;

		BuildRounds--;

		if ( BuildRounds == 0 )
		{
			Server->addReport ( BuildingTyp, false, owner->Nr );
			
			//handle pathbuilding
			//here the new building is added (if possible) and the move job to the next field is generated
			//the new build event is generated in cServer::handleMoveJobs()
			if ( BuildPath )
			{
				int nextX = PosX;
				if ( PosX > BandX ) nextX--;
				if ( PosX < BandX ) nextX++;
				int nextY = PosY;
				if ( PosY > BandY ) nextY--;
				if ( PosY < BandY ) nextY++;

				if ( (PosX != BandX || PosY != BandY) && Server->addMoveJob( PosX + PosY*Server->Map->size, nextX + nextY*Server->Map->size, this ) )
				{
					IsBuilding = false;
					Server->addUnit( PosX, PosY, BuildingTyp.getBuilding(), owner );
					this->ServerMoveJob->checkMove();	//begin the movment immediately, so no other unit can block the destination field
				}
				else
				{
					if ( BuildingTyp.getUnitData()->is_base || BuildingTyp.getUnitData()->is_connector)
					{
						Server->addUnit( PosX, PosY, BuildingTyp.getBuilding(), owner );
						IsBuilding = false;
					}
					BuildPath = false;
					sendBuildAnswer(false, this );
				}
			}
			else
			{
				//add building immediatly if it doesn't require the engineer to drive away
				if ( BuildingTyp.getUnitData()->is_base || BuildingTyp.getUnitData()->is_connector)
				{
					IsBuilding = false;
					Server->addUnit( PosX, PosY, BuildingTyp.getBuilding(), owner );
				}
			}
		}

		iReturn = 1;
	}

	// removing dirt
	if ( IsClearing && ClearingRounds )
	{
		ClearingRounds--;

		if ( ClearingRounds == 0 )
		{
			IsClearing = false;
			cBuilding *Rubble = Server->Map->fields[PosX+PosY*Server->Map->size].getRubble();
			if ( data.is_big )
			{
				Server->Map->moveVehicle ( this, BuildBigSavedPos );
				sendStopClear ( this, BuildBigSavedPos, owner->Nr );
				for ( unsigned int i = 0; i < SeenByPlayerList.Size(); i++ )
				{
					sendStopClear ( this, BuildBigSavedPos, SeenByPlayerList[i]->Nr );
				}
			}
			else
			{
				sendStopClear ( this, -1, owner->Nr );
				for ( unsigned int i = 0; i < SeenByPlayerList.Size(); i++ )
				{
					sendStopClear ( this, -1, SeenByPlayerList[i]->Nr );
				}
			}
			data.cargo += Rubble->RubbleValue;
			if ( data.cargo > data.max_cargo ) data.cargo = data.max_cargo;
			Server->deleteRubble ( Rubble );
		}

		iReturn = 1;
	}
	return iReturn;
}

// Zeigt die Eigenschaften des Vehicles an:
void cVehicle::ShowDetails ( void )
{
	SDL_Rect scr, dest;
	// Den Hintergrund wiederherstellen:
	scr.x = 0;
	scr.y = 215;
	scr.w = 155;
	scr.h = 48;
	dest.x = 8;
	dest.y = 171;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, GraphicsData.gfx_hud, &dest );
	// Die Hitpoints anzeigen:
	DrawNumber ( 31, 177, data.hit_points, data.max_hit_points, GraphicsData.gfx_hud );

	font->showText ( 55, 177, lngPack.i18n ( "Text~Hud~Hitpoints" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );
	DrawSymbol ( SHits, 88, 174, 70, data.hit_points, data.max_hit_points, GraphicsData.gfx_hud );
	// Den Speed anzeigen:
	DrawNumber ( 31, 201, data.speed / 4, data.max_speed / 4, GraphicsData.gfx_hud );

	font->showText ( 55, 201, lngPack.i18n ( "Text~Hud~Speed" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );
	DrawSymbol ( SSpeed, 88, 199, 70, data.speed / 4, data.max_speed / 4, GraphicsData.gfx_hud );
	// additional values

	if ( data.can_transport && owner == Client->ActivePlayer )
	{
		// Transport:
		DrawNumber ( 31, 189, data.cargo, data.max_cargo, GraphicsData.gfx_hud );

		font->showText ( 55, 189, lngPack.i18n ( "Text~Hud~Cargo" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

		switch ( data.can_transport )
		{

			case TRANS_METAL:
				DrawSymbol ( SMetal, 88, 186, 70, data.cargo, data.max_cargo, GraphicsData.gfx_hud );
				break;

			case TRANS_OIL:
				DrawSymbol ( SOil, 88, 186, 70, data.cargo, data.max_cargo, GraphicsData.gfx_hud );
				break;

			case TRANS_GOLD:
				DrawSymbol ( SGold, 88, 187, 70, data.cargo, data.max_cargo, GraphicsData.gfx_hud );
				break;

			case TRANS_VEHICLES:
				DrawSymbol ( STrans, 88, 186, 70, data.cargo, data.max_cargo, GraphicsData.gfx_hud );
				break;

			case TRANS_MEN:
				DrawSymbol ( SHuman, 88, 186, 70, data.cargo, data.max_cargo, GraphicsData.gfx_hud );
				break;
		}
	}
	else
		if ( data.can_attack )
		{
			if ( owner == Client->ActivePlayer )
			{
				// Munition:
				DrawNumber ( 31, 189, data.ammo, data.max_ammo, GraphicsData.gfx_hud );

				font->showText ( 55, 189, lngPack.i18n ( "Text~Hud~AmmoShort" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );
				DrawSymbol ( SAmmo, 88, 187, 70, data.ammo, data.max_ammo, GraphicsData.gfx_hud );
			}

			// shots
			DrawNumber ( 31, 212, data.shots, data.max_shots, GraphicsData.gfx_hud );

			font->showText ( 55, 212, lngPack.i18n ( "Text~Hud~Shots" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

			DrawSymbol ( SShots, 88, 212, 70, data.shots, data.max_shots, GraphicsData.gfx_hud );
		}
}

// Malt eine Reihe von Symbolen:
void cVehicle::DrawSymbol ( eSymbols sym, int x, int y, int maxx, int value, int maxvalue, SDL_Surface *sf )
{
	SDL_Rect full, empty, dest;
	int i, to, step, offx;

	switch ( sym )
	{

		case SSpeed:
			full.x = 0;
			empty.y = full.y = 98;
			empty.w = full.w = 7;
			empty.h = full.h = 7;
			empty.x = 7;
			break;

		case SHits:
			empty.y = full.y = 98;
			empty.w = full.w = 6;
			empty.h = full.h = 9;

			if ( value > maxvalue / 2 )
			{
				full.x = 14;
				empty.x = 20;
			}
			else
				if ( value > maxvalue / 4 )
				{
					full.x = 26;
					empty.x = 32;
				}
				else
				{
					full.x = 38;
					empty.x = 44;
				}

			break;

		case SAmmo:
			full.x = 50;
			empty.y = full.y = 98;
			empty.w = full.w = 5;
			empty.h = full.h = 7;
			empty.x = 55;
			break;

		case SMetal:
			full.x = 60;
			empty.y = full.y = 98;
			empty.w = full.w = 7;
			empty.h = full.h = 10;
			empty.x = 67;
			break;

		case SEnergy:
			full.x = 74;
			empty.y = full.y = 98;
			empty.w = full.w = 7;
			empty.h = full.h = 7;
			empty.x = 81;
			break;

		case SShots:
			full.x = 88;
			empty.y = full.y = 98;
			empty.w = full.w = 8;
			empty.h = full.h = 4;
			empty.x = 96;
			break;

		case SOil:
			full.x = 104;
			empty.y = full.y = 98;
			empty.w = full.w = 8;
			empty.h = full.h = 9;
			empty.x = 112;
			break;

		case SGold:
			full.x = 120;
			empty.y = full.y = 98;
			empty.w = full.w = 9;
			empty.h = full.h = 8;
			empty.x = 129;
			break;

		case STrans:
			full.x = 138;
			empty.y = full.y = 98;
			empty.w = full.w = 16;
			empty.h = full.h = 8;
			empty.x = 154;
			break;

		case SHuman:
			full.x = 170;
			empty.y = full.y = 98;
			empty.w = full.w = 8;
			empty.h = full.h = 9;
			empty.x = 178;
			break;
	}

	to = maxvalue;

	step = 1;
	offx = full.w;

	while ( offx*to > maxx )
	{
		offx--;

		if ( offx < 4 )
		{
			to /= 2;
			step *= 2;
			offx = full.w;
		}
	}

	dest.x = x;

	dest.y = y;

	for ( i = 0;i < to;i++ )
	{
		if ( value > 0 )
		{
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &full, sf, &dest );
		}
		else
		{
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &empty, sf, &dest );
		}

		dest.x += offx;

		value -= step;
	}
}

// Malt eine Nummer/Nummer auf das Surface:
void cVehicle::DrawNumber ( int x, int y, int value, int maxvalue, SDL_Surface *sf )
{
	string sTmp = iToStr ( value ) + "/" + iToStr ( maxvalue );

	if ( value > maxvalue / 2 )
	{

		font->showTextCentered ( x, y, sTmp , FONT_LATIN_SMALL_GREEN, sf );
	}
	else
		if ( value > maxvalue / 4 )
		{

			font->showTextCentered ( x, y, sTmp , FONT_LATIN_SMALL_YELLOW, sf );
		}
		else
		{

			font->showTextCentered ( x, y, sTmp , FONT_LATIN_SMALL_RED, sf );
		}
}

// Zeigt den Hilfebildschirm an:
void cVehicle::ShowHelp ( void )
{
#define BUTTON_W 150
#define BUTTON_H 29

	int LastMouseX = 0, LastMouseY = 0, LastB = 0, x, y, b;
	SDL_Rect rDialog = { MENU_OFFSET_X, MENU_OFFSET_Y, DIALOG_W, DIALOG_H };
	SDL_Rect rDialogSrc = {0, 0, DIALOG_W, DIALOG_H};
	SDL_Rect rInfoTxt = {MENU_OFFSET_X + 11, MENU_OFFSET_Y + 13, typ->info->w, typ->info->h};
	SDL_Rect rTxt = {MENU_OFFSET_X + 349, MENU_OFFSET_Y + 66, 277, 181};
	SDL_Rect rTitle = {MENU_OFFSET_X + 327, MENU_OFFSET_Y + 11, 160, 15};
	SDL_Surface *SfDialog;
	Client->isInMenu = true;

	PlayFX ( SoundData.SNDHudButton );
	mouse->SetCursor ( CHand );
	mouse->draw ( false, buffer );

	if ( SettingsData.bAlphaEffects )
	{
		SDL_BlitSurface ( GraphicsData.gfx_shadow, NULL, buffer, NULL );
	}


	SfDialog = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, DIALOG_W, DIALOG_H, SettingsData.iColourDepth, 0, 0, 0, 0 );

	if ( FileExists ( GFXOD_HELP ) )
	{
		LoadPCXtoSF ( GFXOD_HELP, SfDialog );
	}


	// Den Hilfebildschirm blitten:
	SDL_BlitSurface ( SfDialog, &rDialogSrc, buffer, &rDialog );

	// Das Infobild blitten:
	SDL_BlitSurface ( typ->info, NULL, buffer, &rInfoTxt );

	//show menu title
	font->showTextCentered ( rTitle.x + rTitle.w / 2, rTitle.y, lngPack.i18n ( "Text~Title~Unitinfo" ) );


	// show text
	font->showTextAsBlock ( rTxt, typ->text );

	// get unit details
	ShowBigDetails();

	NormalButton btn_done(MENU_OFFSET_X + 474, MENU_OFFSET_Y + 452, "Text~Button~Done");
	btn_done.Draw();

	SHOW_SCREEN 	// Den Buffer anzeigen
	mouse->GetBack ( buffer );

	while ( 1 )
	{
		if ( Client->SelectedVehicle != this && !Client->bHelpActive ) break;
		if ( !Client->isInMenu ) break;

		Client->handleTimer();
		Client->doGameActions();

		// Events holen:
		EventHandler->HandleEvents();

		// Die Maus machen:
		mouse->GetPos();
		b = (int)Client->getMouseState().leftButtonPressed;
		x = mouse->x;
		y = mouse->y;

		if ( x != LastMouseX || y != LastMouseY )
		{
			mouse->draw ( true, screen );
		}

		if (btn_done.CheckClick(x, y, b > LastB, b < LastB))
		{
			break;
		}

		LastMouseX = x;

		LastMouseY = y;
		LastB = b;
	}

	SDL_FreeSurface ( SfDialog );
	Client->isInMenu = false;
}

// draws big symbols for the info window
void cVehicle::DrawSymbolBig ( eSymbolsBig sym, int x, int y, int maxx, int value, int orgvalue, SDL_Surface *sf )
{
	SDL_Rect scr, dest;
	int i, offx;

	switch ( sym )
	{

		case SBSpeed:
			scr.x = 0;
			scr.y = 109;
			scr.w = 11;
			scr.h = 12;
			break;

		case SBHits:
			scr.x = 11;
			scr.y = 109;
			scr.w = 7;
			scr.h = 11;
			break;

		case SBAmmo:
			scr.x = 18;
			scr.y = 109;
			scr.w = 9;
			scr.h = 14;
			break;

		case SBAttack:
			scr.x = 27;
			scr.y = 109;
			scr.w = 10;
			scr.h = 14;
			break;

		case SBShots:
			scr.x = 37;
			scr.y = 109;
			scr.w = 15;
			scr.h = 7;
			break;

		case SBRange:
			scr.x = 52;
			scr.y = 109;
			scr.w = 13;
			scr.h = 13;
			break;

		case SBArmor:
			scr.x = 65;
			scr.y = 109;
			scr.w = 11;
			scr.h = 14;
			break;

		case SBScan:
			scr.x = 76;
			scr.y = 109;
			scr.w = 13;
			scr.h = 13;
			break;

		case SBMetal:
			scr.x = 89;
			scr.y = 109;
			scr.w = 12;
			scr.h = 15;
			break;

		case SBOil:
			scr.x = 101;
			scr.y = 109;
			scr.w = 11;
			scr.h = 12;
			break;

		case SBGold:
			scr.x = 112;
			scr.y = 109;
			scr.w = 13;
			scr.h = 10;
			break;
	}

	maxx -= scr.w;

	if ( orgvalue < value )
	{
		maxx -= scr.w + 3;
	}

	offx = scr.w;

	while ( offx*value > maxx )
	{
		offx--;

		if ( offx < 4 )
		{
			value /= 2;
			orgvalue /= 2;
			offx = scr.w;
		}
	}

	dest.x = x;

	dest.y = y;

	for ( i = 0;i < value;i++ )
	{
		if ( i == orgvalue )
		{
			SDL_Rect mark;
			dest.x += scr.w + 3;
			mark.x = dest.x - scr.w / 2;
			mark.y = dest.y;
			mark.w = 1;
			mark.h = scr.h;
			SDL_FillRect ( sf, &mark, 0xFC0000 );
		}

		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, sf, &dest );

		dest.x += offx;
	}
}

// Returns the x-position of the vehicle on the screen
int cVehicle::GetScreenPosX(void) const
{
	return 180 - ( ( int ) ( ( Client->Hud.OffX - OffX ) / ( 64.0 / Client->Hud.Zoom ) ) ) + Client->Hud.Zoom*PosX;
}

// Returns the y-position of the vehicle on the screen
int cVehicle::GetScreenPosY(void) const
{
	return 18 - ( ( int ) ( ( Client->Hud.OffY - OffY ) / ( 64.0 / Client->Hud.Zoom ) ) ) + Client->Hud.Zoom*PosY;
}

// Malt den Path des Vehicles:
void cVehicle::DrawPath ( void )
{
	int zoom, mx, my, sp, save;
	SDL_Rect dest, ndest;
	sWaypoint *wp;

	if ( !ClientMoveJob || !ClientMoveJob->Waypoints || owner != Client->ActivePlayer )
	{
		if ( !BuildPath || ( BandX == PosX && BandY == PosY ) || PlaceBand )
			return;

		zoom = Client->Hud.Zoom;

		mx = PosX;

		my = PosY;

		if ( mx < BandX )
			sp = 4;
		else
			if ( mx > BandX )
				sp = 3;
			else
				if ( my < BandY )
					sp = 1;
				else
					sp = 6;

		while ( mx != BandX || my != BandY )
		{
			dest.x = 180 - ( int ) ( Client->Hud.OffX / ( 64.0 / zoom ) ) + zoom * mx;
			dest.y = 18 - ( int ) ( Client->Hud.OffY / ( 64.0 / zoom ) ) + zoom * my;

			SDL_BlitSurface ( OtherData.WayPointPfeileSpecial[sp][64-zoom], NULL, buffer, &dest );

			if ( mx < BandX )
				mx++;
			else
				if ( mx > BandX )
					mx--;

			if ( my < BandY )
				my++;
			else
				if ( my > BandY )
					my--;
		}

		dest.x = 180 - ( int ) ( Client->Hud.OffX / ( 64.0 / zoom ) ) + zoom * mx;

		dest.y = 18 - ( int ) ( Client->Hud.OffY / ( 64.0 / zoom ) ) + zoom * my;
		SDL_BlitSurface ( OtherData.WayPointPfeileSpecial[sp][64-zoom], NULL, buffer, &dest );
		return;
	}

	sp = data.speed;

	if ( sp )
	{
		save = 0;
		sp += ClientMoveJob->iSavedSpeed;
	}
	else
		save = ClientMoveJob->iSavedSpeed;

	zoom = Client->Hud.Zoom;

	dest.x = 180 - ( int ) ( Client->Hud.OffX / ( 64.0 / zoom ) ) + zoom * PosX;

	dest.y = 18 - ( int ) ( Client->Hud.OffY / ( 64.0 / zoom ) ) + zoom * PosY;

	wp = ClientMoveJob->Waypoints;

	dest.x += mx = wp->X * zoom - ClientMoveJob->Waypoints->X * zoom;

	dest.y += my = wp->Y * zoom - ClientMoveJob->Waypoints->Y * zoom;

	ndest = dest;

	while ( wp )
	{
		if ( wp->next )
		{
			ndest.x += mx = wp->next->X * zoom - wp->X * zoom;
			ndest.y += my = wp->next->Y * zoom - wp->Y * zoom;
		}
		else
		{
			ndest.x += mx;
			ndest.y += my;
		}

		if ( sp == 0 )
		{
			ClientMoveJob->drawArrow ( dest, &ndest, true );
			sp += data.max_speed + save;
			save = 0;
		}
		else
		{
			ClientMoveJob->drawArrow ( dest, &ndest, false );
		}

		dest = ndest;

		wp = wp->next;

		if ( wp )
		{
			sp -= wp->Costs;

			if ( wp->next && sp < wp->next->Costs )
			{
				save = sp;
				sp = 0;
			}
		}
	}
}

// Dreht das Vehicle in die angegebene Richtung:
void cVehicle::RotateTo ( int Dir )
{
	if ( Dir < 0 || Dir >= 8 ) return;
	int i, t, dest;

	if ( dir == Dir )
		return;

	t = dir;

	for ( i = 0;i < 8;i++ )
	{
		if ( t == Dir )
		{
			dest = i;
			break;
		}

		t++;

		if ( t > 7 )
			t = 0;
	}

	if ( dest < 4 )
		dir++;
	else
		dir--;

	if ( dir < 0 )
		dir += 8;
	else
		if ( dir > 7 )
			dir -= 8;
}

// Returns a string with the current state
string cVehicle::GetStatusStr ()
{
	if ( autoMJob )
	{
		return lngPack.i18n ( "Text~Comp~Surveying" );
	}
	else
		if ( ClientMoveJob )
		{
			return lngPack.i18n ( "Text~Comp~Moving" );
		}
		else
			if ( bSentryStatus )
			{
				return lngPack.i18n ( "Text~Comp~Sentry" );
			}
			else
				if ( IsBuilding )
				{
					if ( owner != Client->ActivePlayer )
					{
						return lngPack.i18n ( "Text~Comp~Producing" );
					}
					else
					{
						if ( BuildRounds )
						{
							string sText;
							sText = lngPack.i18n ( "Text~Comp~Producing" );
							sText += ": ";
							sText += ( string ) BuildingTyp.getUnitData ( owner )->name + " (";
							sText += iToStr ( BuildRounds );
							sText += ")";

							if ( font->getTextWide ( sText ) > 126 )
							{
								sText = lngPack.i18n ( "Text~Comp~Producing" );
								sText += ":\n";
								sText += ( string ) BuildingTyp.getUnitData ( owner )->name + " (";
								sText += iToStr ( BuildRounds );
								sText += ")";
							}

							return sText;
						}
						else
						{
							return lngPack.i18n ( "Text~Comp~Producing_Fin" );
						}
					}
				}
				else
					if ( ClearMines )
					{
						return lngPack.i18n ( "Text~Comp~Clearing_Mine" );
					}
					else
						if ( LayMines )
						{
							return lngPack.i18n ( "Text~Comp~Laying" );
						}
						else
							if ( IsClearing )
							{
								if ( ClearingRounds )
								{
									string sText;
									sText = lngPack.i18n ( "Text~Comp~Clearing" ) + " (";
									sText += iToStr ( ClearingRounds ) + ")";
									return sText;
								}
								else
								{
									return lngPack.i18n ( "Text~Comp~Clearing_Fin" );
								}
							}
							else
								if ( data.is_commando && owner == Client->ActivePlayer )
								{
									string sTmp = lngPack.i18n ( "Text~Comp~Waits" ) + "\n";

									if ( CommandoRank < 1 ) sTmp += lngPack.i18n ( "Text~Comp~Greenhorn" );
									else if ( CommandoRank < 3 ) sTmp += lngPack.i18n ( "Text~Comp~Average" );
									else if ( CommandoRank < 6 ) sTmp += lngPack.i18n ( "Text~Comp~Veteran" );
									else if ( CommandoRank < 11 ) sTmp += lngPack.i18n ( "Text~Comp~Expert" );
									else sTmp += lngPack.i18n ( "Text~Comp~Elite" );

									if ( CommandoRank > 0 ) sTmp += " +" + iToStr ( (int)CommandoRank );

									return sTmp;
								}
								else
									if ( Disabled )
									{
										string sText;
										sText = lngPack.i18n ( "Text~Comp~Disabled" ) + " (";
										sText += iToStr ( Disabled ) + ")";
										return sText;
									}

	return lngPack.i18n ( "Text~Comp~Waits" );
}

// Plays the soundstream, that belongs to this vehicle
int cVehicle::PlayStram ( void )
{
	if ( IsBuilding && ( BuildRounds || Client->ActivePlayer != owner ))
	{
		return PlayFXLoop ( SoundData.SNDBuilding );
	}
	else
		if ( IsClearing )
		{
			return PlayFXLoop ( SoundData.SNDClearing );
		}
		else
			if ( Client->Map->IsWater ( PosX + PosY*Client->Map->size ) && data.can_drive != DRIVE_AIR )
			{
				return PlayFXLoop ( typ->WaitWater );
			}
			else
			{
				return PlayFXLoop ( typ->Wait );
			}
}

// Startet den MoveSound:
void cVehicle::StartMoveSound ( void )
{
	bool water;
	// That's the moment, too, to hide the menu
	MenuActive = false;
	// check, if a sound has to be played

	if ( this != Client->SelectedVehicle )
		return;

	cBuilding* building = Client->Map->fields[PosX + PosY * Client->Map->size].getBaseBuilding();
	water = Client->Map->IsWater ( PosX + PosY * Client->Map->size ) && ! ( building && ( building->data.is_platform || building->data.is_bridge || building->data.is_road ) );

	StopFXLoop ( Client->iObjectStream );

	if ( !MoveJobActive )
	{
		if ( water && data.can_drive != DRIVE_AIR )
		{
			PlayFX ( typ->StartWater );
		}
		else
		{
			PlayFX ( typ->Start );
		}
	}

	if ( water && data.can_drive != DRIVE_AIR )
	{
		Client->iObjectStream = PlayFXLoop ( typ->DriveWater );
	}
	else
	{
		Client->iObjectStream = PlayFXLoop ( typ->Drive );
	}
}

// Draws the vehicle menu:
void cVehicle::DrawMenu ( sMouseState *mouseState )
{
	int nr = 0, SelMenu = -1, ExeNr = -1;
	static int LastNr = -1;
	bool bSelection = false;
	SDL_Rect dest;
	dest = GetMenuSize();
	Transfer = false;

	if ( moving || bIsBeeingAttacked )
		return;

	if ( mouseState && mouseState->leftButtonPressed && MouseOverMenu ( mouse->x, mouse->y ) )
	{
		SelMenu = ( mouse->y - dest.y ) / 22;
		LastNr = SelMenu;
	}
	else
		if ( MouseOverMenu ( mouse->x, mouse->y ) )
		{
			ExeNr = LastNr;
			LastNr = -1;
		}
		else
		{
			SelMenu = -1;
			LastNr = -1;
		}

	// Angriff:
	if ( data.can_attack && data.shots )
	{
		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			AttackMode = true;
			Client->Hud.CheckScroll();
			Client->mouseMoveCallback ( true );
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Attack" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Bauen:
	if ( data.can_build && !IsBuilding )
	{
		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }

		if ( ExeNr == nr )
		{
			if ( ClientMoveJob ) sendWantStopMove ( iID );

			MenuActive = false;

			PlayFX ( SoundData.SNDObjectMenu );
			ShowBuildMenu();
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Build" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Transfer:
	if ( ( data.can_transport == TRANS_METAL || data.can_transport == TRANS_OIL || data.can_transport == TRANS_GOLD ) && !IsBuilding && !IsClearing )
	{
		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			Transfer = true;
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Transfer" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Auto
	if ( data.can_survey )
	{
		if ( ( autoMJob == NULL && SelMenu == nr ) || ( autoMJob != NULL && SelMenu != nr ) )
		{
			bSelection = true;
		}
		else
		{
			bSelection = false;
		}

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );

			if ( autoMJob == NULL )
			{
				autoMJob = new cAutoMJob ( this );
			}
			else
			{
				delete autoMJob;
				autoMJob = NULL;
			}

			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Auto" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Stop:
	if ( ClientMoveJob || ( IsBuilding && BuildRounds ) || ( IsClearing && ClearingRounds ) )
	{
		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			if ( ClientMoveJob )
			{
				sendWantStopMove ( iID );
			}
			else if ( IsBuilding )
			{
				sendWantStopBuilding ( iID );
			}
			else if ( IsClearing )
			{
				sendWantStopClear ( this );
			}

			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Stop" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Entfernen:
	if ( data.can_clear && Client->Map->fields[PosX+PosY*Client->Map->size].getRubble() && !IsClearing )
	{
		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );

			sendWantStartClear ( this );
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Clear" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Sentry:
	if ( bSentryStatus || data.can_attack )
	{
		bSelection = SelMenu == nr || bSentryStatus;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			sendChangeSentry ( iID, true );
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Sentry" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Aktivieren/Laden:
	if ( data.can_transport == TRANS_VEHICLES || data.can_transport == TRANS_MEN )
	{
		// Aktivieren:
		if ( SelMenu == nr ) bSelection = true;
		else bSelection = false;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			showStorage();
			return;
		}


		drawContextItem( lngPack.i18n ( "Text~Context~Active" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
		// Laden:

		if ( SelMenu == nr || LoadActive ) bSelection = true;
		else bSelection = false;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			LoadActive = !LoadActive;
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Load" ), bSelection, dest.x, dest.y, buffer );
		dest.y += 22;
		nr++;
	}

	// Aufaden:
	if ( data.can_reload && data.cargo >= 2 )
	{
		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			MuniActive = true;
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Reload" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Reparatur:
	if ( data.can_repair && data.cargo >= 2 )
	{
		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			RepairActive = true;
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Repair" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Minen legen:
	if ( data.can_lay_mines && data.cargo > 0 )
	{
		if ( SelMenu == nr || LayMines )
			bSelection = true;
		else
			bSelection = false;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			LayMines = !LayMines;
			ClearMines = false;
			sendMineLayerStatus( this );
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Seed" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Minen sammeln:
	if ( data.can_lay_mines && data.cargo < data.max_cargo )
	{
		if ( SelMenu == nr || ClearMines )
			bSelection = true;
		else
			bSelection = false;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			ClearMines = !ClearMines;
			LayMines = false;
			sendMineLayerStatus ( this );
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Clear" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Commando-Funktionen:
	if ( data.is_commando && data.shots )
	{
		// Sabotage:
		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			DisableActive = true;
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Disable" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
		// Stehlen:

		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			StealActive = true;
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Steal" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Info:
	if ( SelMenu == nr ) { bSelection = true; }
	else { bSelection = false; }

	if ( ExeNr == nr )
	{
		MenuActive = false;
		PlayFX ( SoundData.SNDObjectMenu );
		ShowHelp();
		return;
	}

	drawContextItem( lngPack.i18n ( "Text~Context~Info" ), bSelection, dest.x, dest.y, buffer );

	dest.y += 22;
	nr++;
	// Fertig:

	if ( SelMenu == nr ) { bSelection = true; }
	else { bSelection = false; }

	if ( ExeNr == nr )
	{
		MenuActive = false;
		PlayFX ( SoundData.SNDObjectMenu );
		return;
	}

	drawContextItem( lngPack.i18n ( "Text~Context~Done" ), bSelection, dest.x, dest.y, buffer );
}

// Returns the number of points in the menu:
int cVehicle::GetMenuPointAnz ( void )
{
	int nr = 2;

	if ( data.can_build && !IsBuilding )
		nr++;

	if ( data.can_survey )
		nr++;

	if ( ( data.can_transport == TRANS_METAL || data.can_transport == TRANS_OIL || data.can_transport == TRANS_GOLD ) && !IsBuilding && !IsClearing )
		nr++;

	if ( data.can_attack && data.shots )
		nr++;

	if ( ClientMoveJob || ( IsBuilding && BuildRounds ) || ( IsClearing && ClearingRounds ) )
		nr++;

	if ( data.can_clear && Client->Map->fields[PosX+PosY*Client->Map->size].getRubble() && !IsClearing )
		nr++;

	if ( bSentryStatus || data.can_attack )
		nr++;

	if ( data.can_transport == TRANS_VEHICLES || data.can_transport == TRANS_MEN )
		nr += 2;

	if ( data.can_reload && data.cargo >= 2 )
		nr++;

	if ( data.can_repair && data.cargo >= 2 )
		nr++;

	if ( data.can_lay_mines && data.cargo > 0 )
		nr++;

	if ( data.can_lay_mines && data.cargo < data.max_cargo )
		nr++;

	if ( data.is_commando && data.shots )
		nr += 2;

	return nr;
}

// Returns the size of the menu and the position
SDL_Rect cVehicle::GetMenuSize ( void )
{
	SDL_Rect dest;
	int i, size;
	dest.x = GetScreenPosX();
	dest.y = GetScreenPosY();
	dest.h = i = GetMenuPointAnz() * 22;
	dest.w = 42;
	size = Client->Hud.Zoom;

	if ( IsBuilding && data.can_build == BUILD_BIG )
		size *= 2;

	if ( dest.x + size + 42 >= SettingsData.iScreenW - 12 )
	{
		dest.x -= 42;
	}
	else
	{
		dest.x += size;
	}

	if ( dest.y - ( i - size ) / 2 <= 24 )
	{
		dest.y -= ( i - size ) / 2;
		dest.y += - ( dest.y - 24 );
	}
	else
		if ( dest.y - ( i - size ) / 2 + i >= SettingsData.iScreenH - 24 )
		{
			dest.y -= ( i - size ) / 2;
			dest.y -= ( dest.y + i ) - ( SettingsData.iScreenH - 24 );
		}
		else
		{
			dest.y -= ( i - size ) / 2;
		}

	return dest;
}

// Returns true, if the mouse coordinates are in the menu's space
bool cVehicle::MouseOverMenu ( int mx, int my )
{
	SDL_Rect r;
	r = GetMenuSize();

	if ( mx < r.x || mx > r.x + r.w )
		return false;

	if ( my < r.y || my > r.y + r.h )
		return false;

	return true;
}

// Vermindert die Geschwindigkeit bei der Bewegung:
void cVehicle::DecSpeed ( int value )
{
	data.speed -= value;

	if ( data.can_attack )
	{
		float f;
		int s;
		f = ( ( float ) data.max_shots / data.max_speed );
		s = ( int ) ( data.speed * f );

		if ( !data.can_drive_and_fire && s < data.shots && s >= 0 )
			data.shots = s;
	}

	if ( Client->SelectedVehicle == this )
		ShowDetails();
}

// Draws the ammunition bar over the vehicle
void cVehicle::DrawMunBar(void) const
{
	SDL_Rect r1, r2;
	r1.x = GetScreenPosX() + Client->Hud.Zoom/10 + 1;
	r1.w = Client->Hud.Zoom * 8 / 10 ;
	r1.h = Client->Hud.Zoom / 8;
	r1.y = GetScreenPosY() + Client->Hud.Zoom/10 + Client->Hud.Zoom / 8;

	if ( r1.h <= 2  )
	{
		r1.y += 1;
		r1.h = 3;
	}

	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.h = r1.h - 2;
	r2.w = ( int ) ( ( ( float ) ( r1.w - 2 ) / data.max_ammo ) * data.ammo );

	SDL_FillRect ( buffer, &r1, 0 );

	if ( data.ammo > data.max_ammo / 2 )
	{
		SDL_FillRect ( buffer, &r2, 0x04AE04 );
	}
	else if ( data.ammo > data.max_ammo / 4 )
	{
		SDL_FillRect ( buffer, &r2, 0xDBDE00 );
	}
	else
	{
		SDL_FillRect ( buffer, &r2, 0xE60000 );
	}
}
// Draws the hitpoints bar over the vehicle
void cVehicle::DrawHelthBar(void) const
{
	SDL_Rect r1, r2;
	r1.x = GetScreenPosX() + Client->Hud.Zoom/10 + 1;
	r1.w = Client->Hud.Zoom * 8 / 10 ;
	r1.h = Client->Hud.Zoom / 8;
	r1.y = GetScreenPosY() + Client->Hud.Zoom/10;

	if ( r1.h <= 2  )
		r1.h = 3;

	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.h = r1.h - 2;
	r2.w = ( int ) ( ( ( float ) ( r1.w - 2 ) / data.max_hit_points ) * data.hit_points );

	SDL_FillRect ( buffer, &r1, 0 );

	if ( data.hit_points > data.max_hit_points / 2 )
	{
		SDL_FillRect ( buffer, &r2, 0x04AE04 );
	}
	else if ( data.hit_points > data.max_hit_points / 4 )
	{
		SDL_FillRect ( buffer, &r2, 0xDBDE00 );
	}
	else
	{
		SDL_FillRect ( buffer, &r2, 0xE60000 );
	}
}

void cVehicle::drawStatus() const
{
	SDL_Rect dest;
	SDL_Rect speedSymbol = {244, 97, 8, 10 };
	SDL_Rect shotsSymbol = {254, 97, 5, 10 };
	SDL_Rect disabledSymbol = {150, 109, 25, 25};

	if ( Disabled )
	{
		if ( Client->Hud.Zoom < 25 ) return;
		dest.x = GetScreenPosX() + Client->Hud.Zoom/2 - 12;
		dest.y = GetScreenPosY() + Client->Hud.Zoom/2 - 12;
		SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &disabledSymbol, buffer, &dest );
	}
	else
		{
		dest.y = GetScreenPosY() + Client->Hud.Zoom - 11;
		dest.x = GetScreenPosX() + Client->Hud.Zoom/2 - 4;
		if ( data.is_big )
		{
			dest.y += 32;
			dest.x += 32;
		}
		if ( data.speed >= 4 )
		{
			if ( data.shots ) dest.x -= Client->Hud.Zoom/4;
			SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &speedSymbol, buffer, &dest );
		}

		dest.x = GetScreenPosX() + Client->Hud.Zoom/2 - 4;
		if ( data.shots )
		{
			if ( data.speed ) dest.x += Client->Hud.Zoom/4;
			SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &shotsSymbol, buffer, &dest );
		}
	}
}

// Zentriert auf dieses Vehicle:
void cVehicle::Center ( void )
{
	Client->Hud.OffX = PosX * 64 - ( ( int ) ( ( ( float ) (SettingsData.iScreenW - 192) / (2 * Client->Hud.Zoom) ) * 64 ) ) + 32;
	Client->Hud.OffY = PosY * 64 - ( ( int ) ( ( ( float ) (SettingsData.iScreenH - 32 ) / (2 * Client->Hud.Zoom) ) * 64 ) ) + 32;
	Client->bFlagDrawMap = true;
	Client->Hud.DoScroll ( 0 );
}

// Checks, if the vehicle can attack the object
bool cVehicle::CanAttackObject ( int off, cMap *Map, bool override )
{
	cVehicle *v = NULL;
	cBuilding *b = NULL;

	if ( Loaded )
		return false;

	if ( !data.can_attack )
		return false;

	if ( !data.shots )
		return false;

	if ( !data.ammo )
		return false;

	if ( Attacking )
		return false;

	if ( bIsBeeingAttacked )
		return false;

	if ( off < 0 )
		return false;

	if ( !IsInRange ( off, Map ) )
		return false;

	if ( data.muzzle_typ == MUZZLE_TORPEDO && !Map->IsWater( off ) )
		return false;

	if ( !owner->ScanMap[off] )
		return override?true:false;

	if ( override )
		return true;

	selectTarget(v, b, off, data.can_attack, Map );
	
	if ( v )
	{
		if ( Client && ( v == Client->SelectedVehicle || v->owner == Client->ActivePlayer ) )
			return false;
	}
	else if ( b )
	{
		if ( Client && ( b == Client->SelectedBuilding || b->owner == Client->ActivePlayer ) )
			return false;
	}
	else
	{
		return false;
	}

	return true;
}

// Checks, if the target lies in range
bool cVehicle::IsInRange ( int off, cMap *Map )
{
	int x, y;
	x = off % Map->size;
	y = off / Map->size;
	x -= PosX;
	y -= PosY;

	if ( sqrt ( ( double ) ( x*x + y*y ) ) <= data.range )
	{
		return true;
	}

	return false;
}

// Malt den Attack-Cursor:
void cVehicle::DrawAttackCursor ( int offset )
{
	SDL_Rect r;
	int wp, wc, t;
	cVehicle *v;
	cBuilding *b;

	selectTarget(v, b, offset, data.can_attack, Client->Map );

	if ( !(v || b) || ( v && v == Client->SelectedVehicle ) || ( b && b == Client->SelectedBuilding ) )
	{
		r.x = 1;
		r.y = 29;
		r.h = 3;
		r.w = 35;
		SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0 );
		return;
	}

	if ( v )
		t = v->data.hit_points;
	else
		t = b->data.hit_points;

	if ( t )
	{
		if ( v )
			wc = ( int ) ( ( float ) t / v->data.max_hit_points * 35 );
		else
			wc = ( int ) ( ( float ) t / b->data.max_hit_points * 35 );
	}
	else
	{
		wc = 0;
	}

	if ( v )
		t = v->CalcHelth ( data.damage );
	else
		t = b->CalcHelth ( data.damage );

	if ( t )
	{
		if ( v )
			wp = ( int ) ( ( float ) t / v->data.max_hit_points * 35 );
		else
			wp = ( int ) ( ( float ) t / b->data.max_hit_points * 35 );
	}
	else
	{
		wp = 0;
	}

	r.x = 1;

	r.y = 29;
	r.h = 3;
	r.w = wp;

	if ( r.w )
		SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0x00FF00 );

	r.x += r.w;

	r.w = wc - wp;

	if ( r.w )
		SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0xFF0000 );

	r.x += r.w;

	r.w = 35 - wc;

	if ( r.w )
		SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0 );
}

// returns the remaining hitpoints after an attack
int cVehicle::CalcHelth ( int damage )
{
	damage -= data.armor;

	if ( damage <= 0 )
	{
		//minimum damage is 1
		damage = 1;
	}

	int hp;
	hp = data.hit_points - damage;

	if ( hp < 0 )
	{
		return 0;
	}

	return hp;
}

// Displays the build menu
void cVehicle::ShowBuildMenu ( void )
{
	int LastMouseX = 0, LastMouseY = 0, LastB = 0, x, y, b;
	SDL_Rect scr, dest;
	bool Beschreibung = SettingsData.bShowDescription;
	bool DownPressed = false;
	bool UpPressed = false;
	int selected = 0, offset = 0, BuildSpeed = 1;
	int iTurboBuildCosts[3];	//costs for the 3 turbo build steps
	int iTurboBuildRounds[3];	// needed rounds for the 3 turbo build steps
								// 0 rounds, means not available
	Client->isInMenu = true;

#define BUTTON__W 77
#define BUTTON__H 23

	SDL_Rect rDialog = { MENU_OFFSET_X, MENU_OFFSET_Y, DIALOG_W, DIALOG_H };
	SDL_Rect rTxtDescription = {MENU_OFFSET_X + 141, MENU_OFFSET_Y + 266, 150, 13};
	SDL_Rect rTitle = {MENU_OFFSET_X + 330, MENU_OFFSET_Y + 11, 154, 13};

	//IMPORTANT: just for reference. If you change these coordinates you'll have to change DrawBuildButtons, too! -- beko
	SDL_Rect rBtnSpeed1 = {MENU_OFFSET_X + 292, MENU_OFFSET_Y + 345, BUTTON__W, BUTTON__H}; //buildspeed * 1
	SDL_Rect rBtnSpeed2 = {MENU_OFFSET_X + 292, MENU_OFFSET_Y + 370, BUTTON__W, BUTTON__H}; //buildspeed * 2
	SDL_Rect rBtnSpeed3 = {MENU_OFFSET_X + 292, MENU_OFFSET_Y + 395, BUTTON__W, BUTTON__H}; //buildspeed * 4

	BandX = PosX;
	BandY = PosY;
	mouse->SetCursor ( CHand );
	mouse->draw ( false, buffer );
	SDL_BlitSurface ( GraphicsData.gfx_build_screen, NULL, buffer, &rDialog );

	NormalButton btn_path(  MENU_OFFSET_X + 339, MENU_OFFSET_Y + 428, "Text~Button~Path");
	NormalButton btn_done(  MENU_OFFSET_X + 387, MENU_OFFSET_Y + 452, "Text~Button~Done");
	NormalButton btn_cancel(MENU_OFFSET_X + 300, MENU_OFFSET_Y + 452, "Text~Button~Cancel");
	if (data.can_build != BUILD_BIG) btn_path.Draw();
	btn_done.Draw();
	btn_cancel.Draw();

	font->showTextCentered ( rTxtDescription.x + rTxtDescription.w / 2, rTxtDescription.y, lngPack.i18n ( "Text~Comp~Description" ) );

	font->showTextCentered ( rTitle.x + rTitle.w / 2, rTitle.y, lngPack.i18n ( "Text~Title~Build" ) );


	// Den Haken:

	if ( Beschreibung )
	{
		scr.x = 291;
		scr.y = 264;
		dest.x = MENU_OFFSET_X + 291;
		dest.y = MENU_OFFSET_Y + 264;
		scr.w = 17;
		scr.h = 17;
		SDL_BlitSurface ( GraphicsData.gfx_build_screen, &scr, buffer, &dest );
	}
	else
	{
		scr.x = 393;
		scr.y = 46;
		dest.x = MENU_OFFSET_X + 291;
		dest.y = MENU_OFFSET_Y + 264;
		scr.w = 18;
		scr.h = 17;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
	}

	// Die Images erstellen:
	cList<sBuildStruct*> images;

	for (size_t i = 0; i < UnitsData.building.Size(); ++i)
	{
		SDL_Surface *sf, *sf2;

		if ( UnitsData.building[i].data.is_expl_mine )
			continue;

		if ( data.can_build == BUILD_BIG && !UnitsData.building[i].data.is_big )
			continue;

		if ( data.can_build == BUILD_SMALL && UnitsData.building[i].data.is_big )
			continue;

		if ( !Client->bAlienTech && UnitsData.building[i].data.is_alien )
			continue;

		int width = 32;
		if ( UnitsData.building[i].img_org->w > UnitsData.building[i].img_org->h ) width = UnitsData.building[i].img_org->w/64*32;
		sf2 = scaleSurface ( UnitsData.building[i].img_org, NULL, width, 32 );

		SDL_SetColorKey ( sf2, SDL_SRCCOLORKEY, 0xFFFFFF );

		sf = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, 32, 32, 32, 0, 0, 0, 0 );

		SDL_SetColorKey ( sf, SDL_SRCCOLORKEY, 0xFF00FF );

		SDL_BlitSurface ( Client->ActivePlayer->color, NULL, sf, NULL );

		SDL_BlitSurface ( sf2, NULL, sf, NULL );

		SDL_FreeSurface ( sf2 );

		sBuildStruct* const n = new sBuildStruct(sf, UnitsData.building[i].data.ID);
		images.Add ( n );
	}

	ShowBuildList ( images, selected, offset, Beschreibung, &BuildSpeed, iTurboBuildCosts, iTurboBuildRounds );

	DrawBuildButtons ( BuildSpeed );

	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );

	mouse->MoveCallback = false;

	while ( 1 )
	{
		Client->handleTimer();
		Client->doGameActions();

		// Events holen:
		EventHandler->HandleEvents();

		if ( Client->SelectedVehicle != this ) break;
		if ( !Client->isInMenu ) break;

		// Die Maus machen:
		mouse->GetPos();

		b = (int)Client->getMouseState().leftButtonPressed;

		x = mouse->x;

		y = mouse->y;

		if ( x != LastMouseX || y != LastMouseY )
		{
			mouse->draw ( true, screen );
		}

		// Down-Button:
		if ( x >= MENU_OFFSET_X + 491 && x < MENU_OFFSET_X + 491 + 18 && y >= MENU_OFFSET_Y + 440 && y < MENU_OFFSET_Y + 440 + 17 && b && !DownPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x = 249;
			scr.y = 151;
			scr.w = 18;
			scr.h = 17;
			dest.x = MENU_OFFSET_X + 491;
			dest.y = MENU_OFFSET_Y + 440;

			offset += 9;

			if ( offset > (int)images.Size() - 9 )
			{
				offset = (int)images.Size() - 9;
			}

			if ( selected < offset )
				selected = offset;

			ShowBuildList ( images, selected, offset, Beschreibung, &BuildSpeed, iTurboBuildCosts, iTurboBuildRounds );

			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );

			SHOW_SCREEN
			mouse->draw ( false, screen );

			DownPressed = true;
		}
		else
			if ( !b && DownPressed )
			{
				scr.x = 491;
				scr.y = 440;
				scr.w = 18;
				scr.h = 17;
				dest.x = MENU_OFFSET_X + 491;
				dest.y = MENU_OFFSET_Y + 440;
				SDL_BlitSurface ( GraphicsData.gfx_build_screen, &scr, buffer, &dest );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				DownPressed = false;
			}

		// Up-Button:
		if ( x >= MENU_OFFSET_X + 471 && x < MENU_OFFSET_X + 471 + 18 && y >= MENU_OFFSET_Y + 440 && y < MENU_OFFSET_Y + 440 + 17 && b && !UpPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x = 230;
			scr.y = 151;
			scr.w = 18;
			scr.h = 17;
			dest.x = MENU_OFFSET_X + 471;
			dest.y = MENU_OFFSET_Y + 440;

			offset -= 9;

			if ( offset < 0 )
			{
				offset = 0;
			}

			if ( selected > offset + 8 )
				selected = offset + 8;

			ShowBuildList ( images, selected, offset, Beschreibung, &BuildSpeed, iTurboBuildCosts, iTurboBuildRounds );

			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );

			SHOW_SCREEN
			mouse->draw ( false, screen );

			UpPressed = true;
		}
		else
			if ( !b && UpPressed )
			{
				scr.x = 471;
				scr.y = 440;
				scr.w = 18;
				scr.h = 17;
				dest.x = MENU_OFFSET_X + 471;
				dest.y = MENU_OFFSET_Y + 440;
				SDL_BlitSurface ( GraphicsData.gfx_build_screen, &scr, buffer, &dest );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				UpPressed = false;
			}

		bool const down = b > LastB;
		bool const up   = b < LastB;

		if (btn_cancel.CheckClick(x, y, down, up))
		{
			break;
		}

		if (btn_done.CheckClick(x, y, down, up))
		{
			if ( BuildSpeed < 0 ) break;

			if ( data.can_build != BUILD_BIG )
			{
				sendWantBuild(iID, images[selected]->ID, BuildSpeed, PosX + PosY * Client->Map->size, false, 0);
			}
			else
			{
				PlaceBand = true;
				BuildBigSavedPos = PosX + PosY * Client->Map->size;

				// save building information temporary to have them when placing band is finished
				BuildingTyp = images[selected]->ID;
				BuildRounds = BuildSpeed;

				FindNextband();
			}

			break;
		}

		// Pfad-Button:
		if (data.can_build != BUILD_BIG && btn_path.CheckClick(x, y, down, up))
		{
			if ( BuildSpeed < 0 ) break;

			BuildingTyp = images[selected]->ID;
			BuildRounds = BuildSpeed;

			PlaceBand = true;
			break;
		}

		// Beschreibung Haken:
		if ( x >= MENU_OFFSET_X + 292 && x < MENU_OFFSET_X + 292 + 16 && y >= MENU_OFFSET_Y + 265 && y < MENU_OFFSET_Y + 265 + 15 && b && !LastB )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			Beschreibung = !Beschreibung;
			SettingsData.bShowDescription = Beschreibung;

			if ( Beschreibung )
			{
				scr.x = 291;
				scr.y = 264;
				dest.x = MENU_OFFSET_X + 291;
				dest.y = MENU_OFFSET_Y + 264;
				scr.w = 17;
				scr.h = 17;
				SDL_BlitSurface ( GraphicsData.gfx_build_screen, &scr, buffer, &dest );
			}
			else
			{
				scr.x = 393;
				scr.y = 46;
				dest.x = MENU_OFFSET_X + 291;
				dest.y = MENU_OFFSET_Y + 264;
				scr.w = 18;
				scr.h = 17;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
			}

			ShowBuildList ( images, selected, offset, Beschreibung, &BuildSpeed, iTurboBuildCosts, iTurboBuildRounds );

			SHOW_SCREEN
			mouse->draw ( false, screen );
		}

		// 1x Button:
		if ( ( x >= rBtnSpeed1.x && x < rBtnSpeed1.x + rBtnSpeed1.w && y >= rBtnSpeed1.y && y < rBtnSpeed1.y + rBtnSpeed1.h && b && !LastB ) && ( iTurboBuildRounds[0] > 0 ) )
		{
			PlayFX ( SoundData.SNDMenuButton );
			BuildSpeed = 0;
			DrawBuildButtons ( BuildSpeed );
			SHOW_SCREEN
			mouse->draw ( false, screen );
		}

		// 2x Button:
		if ( ( x >= rBtnSpeed2.x && x < rBtnSpeed2.x + rBtnSpeed2.w && y >= rBtnSpeed2.y && y < rBtnSpeed2.y + rBtnSpeed2.h && b && !LastB ) && ( iTurboBuildRounds[1] > 0 ) )
		{
			PlayFX ( SoundData.SNDMenuButton );
			BuildSpeed = 1;
			DrawBuildButtons ( BuildSpeed );
			SHOW_SCREEN
			mouse->draw ( false, screen );
		}

		// 4x Button:
		if ( ( x >= rBtnSpeed3.x && x < rBtnSpeed3.x + rBtnSpeed3.w && y >= rBtnSpeed3.y && y < rBtnSpeed3.y + rBtnSpeed3.h && b && !LastB ) && ( iTurboBuildRounds[2] > 0 ) )
		{
			PlayFX ( SoundData.SNDMenuButton );
			BuildSpeed = 2;
			DrawBuildButtons ( BuildSpeed );
			SHOW_SCREEN
			mouse->draw ( false, screen );
		}

		// Klick in die Liste:
		if ( x >= MENU_OFFSET_X + 490 && x < MENU_OFFSET_X + 490 + 70 && y >= MENU_OFFSET_Y + 60 && y < MENU_OFFSET_Y + 60 + 368 && b && !LastB )
		{
			int nr;
			nr = ( y - MENU_OFFSET_Y - 60 ) / ( 32 + 10 );

			if ( images.Size() < 9 )
			{
				if ( nr >= (int)images.Size() )
					nr = -1;
			}
			else
			{
				if ( nr >= 10 )
					nr = -1;

				nr += offset;
			}

			if ( nr != -1 )
			{
				PlayFX ( SoundData.SNDObjectMenu );
				selected = nr;
				ShowBuildList ( images, selected, offset, Beschreibung, &BuildSpeed, iTurboBuildCosts, iTurboBuildRounds );
				SHOW_SCREEN
				mouse->draw ( false, screen );
			}
		}

		LastMouseX = x;

		LastMouseY = y;
		LastB = b;
	}

	// cleanup all images
	while ( images.Size() )
	{
		sBuildStruct *ptr;
		ptr = images[images.Size() - 1];
		SDL_FreeSurface ( ptr->sf );
		delete ptr;
		images.Delete ( images.Size() - 1 );
	}

	mouse->MoveCallback = true;
	Client->isInMenu = false;
}

// Zeigt die Liste mit den Images an:
void cVehicle::ShowBuildList(cList<sBuildStruct*>& list, int const selected, int const offset, bool const beschreibung, int* const buildspeed, int* const iTurboBuildCosts, int* const iTurboBuildRounds)
{
	sBuildStruct *ptr;

	SDL_Rect dest, scr, text = { MENU_OFFSET_X + 530, MENU_OFFSET_Y + 70, 80, 16};
	scr.x = 479;
	scr.y = 52;
	dest.x = MENU_OFFSET_X + 479;
	dest.y = MENU_OFFSET_Y + 52;
	scr.w = 150;
	scr.h = 378;
	SDL_BlitSurface ( GraphicsData.gfx_build_screen, &scr, buffer, &dest );
	scr.x = 373;
	scr.y = 344;
	dest.x = MENU_OFFSET_X + 373;
	dest.y = MENU_OFFSET_Y + 344;
	scr.w = 77;
	scr.h = 72;
	SDL_BlitSurface ( GraphicsData.gfx_build_screen, &scr, buffer, &dest );
	scr.x = 0;
	scr.y = 0;
	scr.w = 32;
	scr.h = 32;
	dest.x = MENU_OFFSET_X + 490;
	dest.y = MENU_OFFSET_Y + 58;

	for ( unsigned int i = offset;i < list.Size();i++ )
	{
		if ( (int)i >= offset + 9 )
			break;

		// Das Bild malen:
		ptr = list[i];

		SDL_BlitSurface ( ptr->sf, &scr, buffer, &dest );

		// Ggf noch Rahmen drum:
		if ( selected == i )
		{
			SDL_Rect tmp;
			tmp = dest;
			tmp.x -= 4;
			tmp.y -= 4;
			tmp.h = 1;
			tmp.w = 8;
			SDL_FillRect ( buffer, &tmp, 0xE0E0E0 );
			tmp.x += 30;
			SDL_FillRect ( buffer, &tmp, 0xE0E0E0 );
			tmp.y += 38;
			SDL_FillRect ( buffer, &tmp, 0xE0E0E0 );
			tmp.x -= 30;
			SDL_FillRect ( buffer, &tmp, 0xE0E0E0 );
			tmp.y = dest.y - 4;
			tmp.w = 1;
			tmp.h = 8;
			SDL_FillRect ( buffer, &tmp, 0xE0E0E0 );
			tmp.x += 38;
			SDL_FillRect ( buffer, &tmp, 0xE0E0E0 );
			tmp.y += 31;
			SDL_FillRect ( buffer, &tmp, 0xE0E0E0 );
			tmp.x -= 38;
			SDL_FillRect ( buffer, &tmp, 0xE0E0E0 );
			// Das Bild neu malen:
			tmp.x = MENU_OFFSET_X + 11;
			tmp.y = MENU_OFFSET_Y + 13;
			tmp.w = ptr->ID.getBuilding()->info->w;
			tmp.h = ptr->ID.getBuilding()->info->h;
			SDL_BlitSurface ( ptr->ID.getBuilding()->info, NULL, buffer, &tmp );
			// Ggf die Beschreibung ausgeben:

			if ( beschreibung )
			{
				tmp.x += 10;
				tmp.y += 10;
				tmp.w -= 20;
				tmp.h -= 20;

				font->showTextAsBlock ( tmp, ptr->ID.getBuilding()->text );
			}

			// Die Details anzeigen:
			{
				SDL_Rect tmp2;
				tmp.x = 11;
				tmp.y = 290;
				tmp2.x = MENU_OFFSET_X + 11;
				tmp2.y = MENU_OFFSET_Y + 290;
				tmp.w = tmp2.w = 260;
				tmp.h = tmp2.h = 176;
				SDL_BlitSurface ( GraphicsData.gfx_build_screen, &tmp, buffer, &tmp2 );
				cBuilding tb(ptr->ID.getBuilding(), Client->ActivePlayer, NULL);
				tb.ShowBigDetails();
			}

			calcTurboBuild( iTurboBuildRounds, iTurboBuildCosts, ptr->ID.getUnitData( owner )->iBuilt_Costs, ptr->ID.getUnitData( owner )->iBuilt_Costs_Max );

			if ( *buildspeed == -1 && iTurboBuildRounds[0] != 0 )
			{
				*buildspeed = 0;
			}

			//reduce buildspeed, if necesary
			if ( ( *buildspeed == 2 ) && ( iTurboBuildRounds[2] == 0 ) )
				*buildspeed = 1;

			if ( ( *buildspeed == 1 ) && ( iTurboBuildRounds[1] == 0 ) )
				*buildspeed = 0;

			if ( ( *buildspeed == 0 ) && ( iTurboBuildRounds[0] == 0 ) )
				*buildspeed = -1;


			// Die Bauzeiten eintragen:

			DrawBuildButtons ( *buildspeed );


			font->showTextCentered ( MENU_OFFSET_X + 389, MENU_OFFSET_Y + 350, iToStr ( ptr->ID.getUnitData( owner )->iBuilt_Costs / data.iNeeds_Metal ) );
			font->showTextCentered ( MENU_OFFSET_X + 429, MENU_OFFSET_Y + 350, iToStr ( ptr->ID.getUnitData( owner )->iBuilt_Costs ) );

			if ( iTurboBuildRounds[1] > 0 )
			{
				font->showTextCentered ( MENU_OFFSET_X + 389, MENU_OFFSET_Y + 375, iToStr ( iTurboBuildRounds[1] ) );
				font->showTextCentered ( MENU_OFFSET_X + 429, MENU_OFFSET_Y + 375, iToStr ( iTurboBuildCosts[1] ) );
			}

			if ( iTurboBuildRounds[2] > 0 )
			{
				font->showTextCentered ( MENU_OFFSET_X + 389, MENU_OFFSET_Y + 400, iToStr ( iTurboBuildRounds[2] ) );
				font->showTextCentered ( MENU_OFFSET_X + 429, MENU_OFFSET_Y + 400, iToStr ( iTurboBuildCosts[2] ) );
			}
		}

		// Text ausgeben:

		string sTmp = ptr->ID.getUnitData()->name;

		if ( font->getTextWide ( sTmp, FONT_LATIN_SMALL_WHITE ) > text.w )
		{
			text.y -= font->getFontHeight(FONT_LATIN_SMALL_WHITE) / 2;
			font->showTextAsBlock ( text, sTmp, FONT_LATIN_SMALL_WHITE);
			text.y += font->getFontHeight(FONT_LATIN_SMALL_WHITE) / 2;
		}
		else
		{
			font->showText ( text, sTmp, FONT_LATIN_SMALL_WHITE);
		}


		font->showTextCentered ( MENU_OFFSET_X + 616, text.y, iToStr ( ptr->ID.getUnitData( owner )->iBuilt_Costs ), FONT_LATIN_SMALL_WHITE );

		text.y += 32 + 10;
		dest.y += 32 + 10;
	}
}

void cVehicle::calcTurboBuild(int* const iTurboBuildRounds, int* const iTurboBuildCosts, int iBuild_Costs, int iBuild_Costs_Max )
{
	// calculate building time and costs

	iTurboBuildRounds[0] = 0;
	iTurboBuildRounds[1] = 0;
	iTurboBuildRounds[2] = 0;

	//prevent division by zero
	if ( data.iNeeds_Metal == 0 ) data.iNeeds_Metal = 1;

	//step 1x
	if ( data.cargo >= iBuild_Costs )
	{
		iTurboBuildCosts[0] = iBuild_Costs;

		iTurboBuildRounds[0] = iTurboBuildCosts[0] / data.iNeeds_Metal;
	}

	//step 2x
	if ( ( iTurboBuildRounds[0] > 1 ) && ( iTurboBuildCosts[0] + 4 <= iBuild_Costs_Max ) && ( data.cargo >= iTurboBuildCosts[0] + 4 ) )
	{
		iTurboBuildRounds[1] = iTurboBuildRounds[0];
		iTurboBuildCosts[1] = iTurboBuildCosts[0];

		while ( ( data.cargo >= iTurboBuildCosts[1] + 4 ) && ( iTurboBuildRounds[1] > 1 ) )
		{
			iTurboBuildRounds[1]--;
			iTurboBuildCosts[1] += 4;

			if ( iTurboBuildCosts[1] + 4 > 2*iTurboBuildCosts[0] )
				break;

			if ( iTurboBuildCosts[1] + 4 > iBuild_Costs_Max )
				break;
		}
	}

	//step 4x
	if ( ( iTurboBuildRounds[1] > 1 ) && ( iTurboBuildCosts[1] + 8 <= iBuild_Costs_Max ) && ( data.cargo >= iTurboBuildCosts[1] + 8 ) )
	{
		iTurboBuildRounds[2] = iTurboBuildRounds[1];
		iTurboBuildCosts[2] = iTurboBuildCosts[1];

		while ( ( data.cargo >= iTurboBuildCosts[2] + 8 ) && ( iTurboBuildRounds[2] > 1 ) )
		{
			iTurboBuildRounds[2]--;
			iTurboBuildCosts[2] += 8;

			if ( iTurboBuildCosts[2] + 8 > iBuild_Costs_Max )
				break;
		}
	}
}

void cVehicle::DrawBuildButtons ( int speed )
{
	SDL_Rect rBtnSpeed1 = {MENU_OFFSET_X + 292, MENU_OFFSET_Y + 345, BUTTON__W, BUTTON__H};
	SDL_Rect rBtnSpeed2 = {MENU_OFFSET_X + 292, MENU_OFFSET_Y + 370, BUTTON__W, BUTTON__H};
	SDL_Rect rBtnSpeed3 = {MENU_OFFSET_X + 292, MENU_OFFSET_Y + 395, BUTTON__W, BUTTON__H};

	string sTmp = lngPack.i18n ( "Text~Button~Build" );

	if ( speed == 0 )
	{
		drawButton ( sTmp + " x1", true, rBtnSpeed1.x, rBtnSpeed1.y, buffer );
	}
	else
	{
		drawButton ( sTmp + " x1", false, rBtnSpeed1.x, rBtnSpeed1.y, buffer );
	}

	if ( speed == 1 )
	{
		drawButton ( sTmp + " x2", true, rBtnSpeed2.x, rBtnSpeed2.y, buffer );
	}
	else
	{
		drawButton ( sTmp + " x2", false, rBtnSpeed2.x, rBtnSpeed2.y, buffer );
	}

	if ( speed == 2 )
	{
		drawButton ( sTmp + " x4", true, rBtnSpeed3.x, rBtnSpeed3.y, buffer );
	}
	else
	{
		drawButton ( sTmp + " x4", false, rBtnSpeed3.x, rBtnSpeed3.y, buffer );
	}
}

// Finds the next fitting position for the band
void cVehicle::FindNextband ( void )
{
	bool pos[4] = {false, false, false, false};
	int x, y;
	mouse->GetKachel ( &x, &y );

	//check, which positions are available
	sUnitData BuildingType = *BuildingTyp.getUnitData();
	if ( Client->Map->possiblePlaceBuilding(BuildingType, PosX - 1, PosY - 1)
	  && Client->Map->possiblePlaceBuilding(BuildingType, PosX    , PosY - 1)
	  && Client->Map->possiblePlaceBuilding(BuildingType, PosX - 1, PosY    ) )
	{
		pos[0] = true;
	}
	
	if ( Client->Map->possiblePlaceBuilding(BuildingType, PosX    , PosY - 1)
	  && Client->Map->possiblePlaceBuilding(BuildingType, PosX + 1, PosY - 1)
	  && Client->Map->possiblePlaceBuilding(BuildingType, PosX + 1, PosY    ) )
	{
		pos[1] = true;
	}

	if ( Client->Map->possiblePlaceBuilding(BuildingType, PosX + 1, PosY    )
	  && Client->Map->possiblePlaceBuilding(BuildingType, PosX + 1, PosY + 1)
	  && Client->Map->possiblePlaceBuilding(BuildingType, PosX    , PosY + 1) )
	{
		pos[2] = true;
	}

	if ( Client->Map->possiblePlaceBuilding(BuildingType, PosX - 1, PosY    )
	  && Client->Map->possiblePlaceBuilding(BuildingType, PosX - 1, PosY + 1)
	  && Client->Map->possiblePlaceBuilding(BuildingType, PosX    , PosY + 1) )
	{
		pos[3] = true;
	}

	//chose the position, which matches the cursor position, if available
	if ( x <= PosX && y <= PosY && pos[0] )
	{
		BandX = PosX - 1;
		BandY = PosY - 1;
		return;
	}

	if ( x >= PosX && y <= PosY && pos[1] )
	{
		BandX = PosX;
		BandY = PosY - 1;
		return;
	}

	if ( x >= PosX && y >= PosY && pos[2] )
	{
		BandX = PosX;
		BandY = PosY;
		return;
	}

	if ( x <= PosX && y >= PosY && pos[3] )
	{
		BandX = PosX - 1;
		BandY = PosY;
		return;
	}

	//if the best position is not available, chose the next free one
	if ( pos[0] )
	{
		BandX = PosX - 1;
		BandY = PosY - 1;
		return;
	}

	if ( pos[1] )
	{
		BandX = PosX;
		BandY = PosY - 1;
		return;
	}

	if ( pos[2] )
	{
		BandX = PosX;
		BandY = PosY;
		return;
	}

	if ( pos[3] )
	{
		BandX = PosX - 1;
		BandY = PosY;
		return;
	}

	if ( data.can_build != BUILD_BIG )
	{
		BandX = PosX;
		BandY = PosY;
		return;
	}

	PlaceBand = false;
}

// Scanes for resources ( This function is only called by the server)
void cVehicle::doSurvey ( void )
{
	char *ptr;
	ptr = owner->ResourceMap;

	if ( !ptr ) return;

	ptr[PosX+PosY*Server->Map->size] = 1;
	if ( PosX > 0 ) ptr[PosX-1+PosY*Server->Map->size] = 1;
	if ( PosX < Server->Map->size - 1 ) ptr[PosX+1+PosY*Server->Map->size] = 1;
	if ( PosY > 0 ) ptr[PosX+ ( PosY-1 ) *Server->Map->size] = 1;
	if ( PosX > 0 && PosY > 0 ) ptr[PosX-1+ ( PosY-1 ) *Server->Map->size] = 1;
	if ( PosX < Server->Map->size - 1 && PosY > 0 ) ptr[PosX+1+ ( PosY-1 ) *Server->Map->size] = 1;
	if ( PosY < Server->Map->size - 1 ) ptr[PosX+ ( PosY+1 ) *Server->Map->size] = 1;
	if ( PosX > 0 && PosY < Server->Map->size - 1 ) ptr[PosX-1+ ( PosY+1 ) *Server->Map->size] = 1;
	if ( PosX < Server->Map->size - 1 && PosY < Server->Map->size - 1 ) ptr[PosX+1+ ( PosY+1 ) *Server->Map->size] = 1;
}

// Macht Meldung:
void cVehicle::MakeReport ( void )
{
	if ( owner != Client->ActivePlayer )
		return;

	if ( Disabled )
	{
		// Disabled:
		PlayVoice ( VoiceData.VOIDisabled );
	}
	else
		if ( data.hit_points > data.max_hit_points / 2 )
		{
			// Status green
			if ( data.speed == 0 )
			{
				// no more movement
				PlayVoice ( VoiceData.VOINoSpeed );
			}
			else
				if ( IsBuilding )
				{
					// Beim bau:
					if ( !BuildRounds )
					{
						// Bau beendet:
						if (random(2))
						{
							PlayVoice ( VoiceData.VOIBuildDone1 );
						}
						else
						{
							PlayVoice ( VoiceData.VOIBuildDone2 );
						}
					}
				}
				else
					if ( IsClearing )
					{
						// removing dirt
						if ( ClearingRounds )
						{
							PlayVoice ( VoiceData.VOIClearing );
						}
						else
						{
							PlayVoice ( VoiceData.VOIOK2 );
						}
					}
					else
						if ( data.can_attack && !data.ammo )
						{
							// Keine Munition:
							if (random(2))
							{
								PlayVoice ( VoiceData.VOILowAmmo1 );
							}
							else
							{
								PlayVoice ( VoiceData.VOILowAmmo2 );
							}
						}
						else
							if ( bSentryStatus )
							{
								// on sentry:
								PlayVoice ( VoiceData.VOIWachposten );
							}
							else
								if ( ClearMines )
								{
									PlayVoice ( VoiceData.VOIClearingMines );
								}
								else
									if ( LayMines )
									{
										PlayVoice ( VoiceData.VOILayingMines );
									}
									else
									{
										int nr;
										// Alles OK:
										nr = random(3);

										if ( nr == 0 )
										{
											PlayVoice ( VoiceData.VOIOK1 );
										}
										else
											if ( nr == 1 )
											{
												PlayVoice ( VoiceData.VOIOK2 );
											}
											else
											{
												PlayVoice ( VoiceData.VOIOK3 );
											}
									}
		}
		else
			if ( data.hit_points > data.max_hit_points / 4 )
			{
				// Status Gelb:
				PlayVoice ( VoiceData.VOIStatusYellow );
			}
			else
			{
				// Status Rot:
				PlayVoice ( VoiceData.VOIStatusRed );
			}
}

// checks, if resources can be transfered to the unit
bool cVehicle::CanTransferTo ( cMapField *OverUnitField )
{
	cBuilding *b;
	cVehicle *v;
	int x, y;
	mouse->GetKachel ( &x, &y );

	if ( x < PosX - 1 || x > PosX + 1 || y < PosY - 1 || y > PosY + 1 )
		return false;

	if ( OverUnitField->getVehicles() )
	{
		v = OverUnitField->getVehicles();

		if ( v == this )
			return false;

		if ( v->owner != Client->ActivePlayer )
			return false;

		if ( v->data.can_transport != data.can_transport )
			return false;

		if ( v->IsBuilding || v->IsClearing )
			return false;

		return true;
	}
	else
		if ( OverUnitField->getTopBuilding() )
		{
			b = OverUnitField->getTopBuilding();

			if ( b->owner != Client->ActivePlayer )
				return false;

			if ( data.can_transport == TRANS_METAL && b->SubBase->MaxMetal == 0 )
				return false;

			if ( data.can_transport == TRANS_OIL && b->SubBase->MaxOil == 0 )
				return false;

			if ( data.can_transport == TRANS_GOLD && b->SubBase->MaxGold == 0 )
				return false;

			return true;
		}

	return false;
}

void cVehicle::ShowBigDetails ( void )
{
	SDL_Rect dest = { SettingsData.iScreenW / 2 - DIALOG_W / 2 + 16, SettingsData.iScreenH / 2 - DIALOG_H / 2, 242, 1 };

#define COLUMN_1 dest.x+27
#define COLUMN_2 dest.x+42
#define COLUMN_3 dest.x+95
#define DOLINEBREAK dest.y = y + 14; SDL_FillRect ( buffer, &dest, 0xFC0000 ); y += 19;
	int y;
	y = dest.y + 297;

	if ( data.can_attack )
	{
		// Damage:
		font->showTextCentered ( COLUMN_1, y, iToStr ( data.damage ) );
		font->showText ( COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Damage" ) );
		DrawSymbolBig ( SBAttack, COLUMN_3 , y - 3, 160, data.damage, typ->data.damage, buffer );
		DOLINEBREAK

		// Shots:
		font->showTextCentered ( COLUMN_1, y, iToStr ( data.max_shots ) );
		font->showText ( COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Shoots" ) );
		DrawSymbolBig ( SBShots, COLUMN_3, y + 2, 160, data.max_shots, typ->data.max_shots, buffer );
		DOLINEBREAK

		// Range:
		font->showTextCentered ( COLUMN_1, y, iToStr ( data.range ) );
		font->showText ( COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Range" ) );
		DrawSymbolBig ( SBRange, COLUMN_3, y - 2, 160, data.range, typ->data.range, buffer );
		DOLINEBREAK

		// Ammo:
		font->showTextCentered ( COLUMN_1, y, iToStr ( data.max_ammo ) );
		font->showText ( COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Ammo" ) );
		DrawSymbolBig ( SBAmmo, COLUMN_3, y - 2, 160, data.max_ammo, typ->data.max_ammo, buffer );
		DOLINEBREAK
	}

	if ( data.can_transport == TRANS_METAL || data.can_transport == TRANS_OIL || data.can_transport == TRANS_GOLD )
	{
		// Metall:


		font->showTextCentered ( COLUMN_1, y, iToStr ( data.max_cargo ) );
		font->showText ( COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Cargo" ) );

		switch ( data.can_transport )
		{

			case TRANS_METAL:
				DrawSymbolBig ( SBMetal, COLUMN_3 , y - 2, 160, data.max_cargo, typ->data.max_cargo, buffer );
				break;

			case TRANS_OIL:
				DrawSymbolBig ( SBOil, COLUMN_3 , y - 2, 160, data.max_cargo, typ->data.max_cargo, buffer );
				break;

			case TRANS_GOLD:
				DrawSymbolBig ( SBGold, COLUMN_3 , y - 2, 160, data.max_cargo, typ->data.max_cargo, buffer );
				break;
		}

		DOLINEBREAK
	}

	// Armor:


	font->showTextCentered ( COLUMN_1, y, iToStr ( data.armor ) );

	font->showText ( COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Armor" ) );

	DrawSymbolBig ( SBArmor, COLUMN_3, y - 2, 160, data.armor, typ->data.armor, buffer );

	DrawSymbolBig ( SBArmor, COLUMN_3 , y - 2, 160, data.armor, typ->data.armor, buffer );

	DOLINEBREAK

	// Hitpoints:


	font->showTextCentered ( COLUMN_1, y, iToStr ( data.max_hit_points ) );

	font->showText ( COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Hitpoints" ) );

	DrawSymbolBig ( SBHits, COLUMN_3 , y - 1, 160, data.max_hit_points, typ->data.max_hit_points, buffer );

	DOLINEBREAK

	// Scan:


	font->showTextCentered ( COLUMN_1, y, iToStr ( data.scan ) );

	font->showText ( COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Scan" ) );

	DrawSymbolBig ( SBScan, COLUMN_3 , y - 2, 160, data.scan, typ->data.scan, buffer );

	DOLINEBREAK

	// Speed:


	font->showTextCentered ( COLUMN_1, y, iToStr ( data.max_speed / 4 ) ); //FIXME: might crash if e.g. max_speed = 3

	font->showText ( COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Speed" ) );

	DrawSymbolBig ( SBSpeed, COLUMN_3 , y - 2, 160, data.max_speed / 4, typ->data.max_speed / 4, buffer );

	DOLINEBREAK

	// Costs:


	font->showTextCentered ( COLUMN_1, y, iToStr ( data.iBuilt_Costs ) );

	font->showText ( COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Costs" ) );

	DrawSymbolBig ( SBMetal, COLUMN_3 , y - 2, 160, data.iBuilt_Costs, typ->data.iBuilt_Costs, buffer );
}

bool cVehicle::InSentryRange ()
{
	sSentry *Sentry;
	int iOff; 
	cPlayer *Player;

	iOff = PosX + PosY * Server->Map->size;

	for ( unsigned int i = 0;i < Server->PlayerList->Size();i++ )
	{
		Player = (*Server->PlayerList)[i];

		if ( Player == owner ) continue;

		if ( data.is_stealth_land && !isDetectedByPlayer( Player ) ) continue;	//check next player
		if ( data.is_stealth_sea  && !isDetectedByPlayer( Player ) ) continue;	//check next player

		if ( data.can_drive == DRIVE_AIR )
		{
			if ( ! ( Player->SentriesMapAir[iOff] && Player->ScanMap[iOff] ) ) continue; //check next player

			for ( unsigned int k = 0; k < Player->SentriesAir.Size(); k++ )
			{
				Sentry = Player->SentriesAir[k];

				if ( Sentry->b && Sentry->b->CanAttackObject ( iOff, Server->Map, true ) )
				{
					cVehicle* targetVehicle;
					cBuilding* targetBuilding;
					selectTarget( targetVehicle, targetBuilding, iOff, Sentry->b->data.can_attack, Server->Map );
					if ( targetBuilding || targetVehicle )
					{
						Server->AJobs.Add( new cServerAttackJob( Sentry->b, iOff ) );

						if ( ServerMoveJob )
						{
							ServerMoveJob->bFinished = true;
						}

						return true;
					}
				}

				if ( Sentry->v && Sentry->v->CanAttackObject ( iOff, Server->Map, true ) )
				{
					cVehicle* targetVehicle;
					cBuilding* targetBuilding;
					selectTarget( targetVehicle, targetBuilding, iOff, Sentry->v->data.can_attack, Server->Map );
					if ( targetBuilding || targetVehicle )
					{
						Server->AJobs.Add( new cServerAttackJob( Sentry->v, iOff ) );

						if ( ServerMoveJob )
						{
							ServerMoveJob->bFinished = true;
						}

						return true;
					}
				}
			}
		}
		else
		{
			if ( ! ( Player->SentriesMapGround[iOff] && Player->ScanMap[iOff] ) ) continue; //check next player

			for ( unsigned int k = 0;k < Player->SentriesGround.Size();k++ )
			{
				Sentry = Player->SentriesGround[k];

				if ( Sentry->b && Sentry->b->CanAttackObject ( iOff, Server->Map, true ) )
				{
					cVehicle* targetVehicle;
					cBuilding* targetBuilding;
					selectTarget( targetVehicle, targetBuilding, iOff, Sentry->b->data.can_attack, Server->Map );
					if ( targetBuilding || targetVehicle )
					{
						Server->AJobs.Add( new cServerAttackJob( Sentry->b, iOff ) );

						if ( ServerMoveJob )
						{
							ServerMoveJob->bFinished = true;
						}

						return true;
					}
				}

				if ( Sentry->v && Sentry->v->CanAttackObject ( iOff, Server->Map, true ) )
				{
					cVehicle* targetVehicle;
					cBuilding* targetBuilding;
					selectTarget( targetVehicle, targetBuilding, iOff, Sentry->v->data.can_attack, Server->Map );
					if ( targetBuilding || targetVehicle )
					{
						Server->AJobs.Add( new cServerAttackJob( Sentry->v, iOff ) );

						if ( ServerMoveJob )
						{
							ServerMoveJob->bFinished = true;
						}

						return true;
					}
				}
			}
		}
	}

	return false;
}

// Draws exitpoints for a vehicle, that should be exited
void cVehicle::DrawExitPoints(sVehicle* const typ) const
{
	int spx, spy, size;
	spx = GetScreenPosX();
	spy = GetScreenPosY();
	size = Client->Map->size;

	if ( canExitTo ( PosX - 1, PosY - 1, Client->Map, typ ) ) Client->drawExitPoint ( spx - Client->Hud.Zoom, spy - Client->Hud.Zoom );
	if ( canExitTo ( PosX    , PosY - 1, Client->Map, typ ) ) Client->drawExitPoint ( spx, spy - Client->Hud.Zoom );
	if ( canExitTo ( PosX + 1, PosY - 1, Client->Map, typ ) ) Client->drawExitPoint ( spx + Client->Hud.Zoom, spy - Client->Hud.Zoom );
	if ( canExitTo ( PosX - 1, PosY    , Client->Map, typ ) ) Client->drawExitPoint ( spx - Client->Hud.Zoom, spy );
	if ( canExitTo ( PosX + 1, PosY    , Client->Map, typ ) ) Client->drawExitPoint ( spx + Client->Hud.Zoom, spy );
	if ( canExitTo ( PosX - 1, PosY + 1, Client->Map, typ ) ) Client->drawExitPoint ( spx - Client->Hud.Zoom, spy + Client->Hud.Zoom );
	if ( canExitTo ( PosX    , PosY + 1, Client->Map, typ ) ) Client->drawExitPoint ( spx, spy + Client->Hud.Zoom );
	if ( canExitTo ( PosX + 1, PosY + 1, Client->Map, typ ) ) Client->drawExitPoint ( spx + Client->Hud.Zoom, spy + Client->Hud.Zoom );
}

bool cVehicle::canExitTo ( const int x, const int y, const cMap* map, const sVehicle *typ ) const
{
	if ( !map->possiblePlaceVehicle( typ->data, x, y ) ) return false;
	if ( data.can_drive == DRIVE_AIR && ( x != PosX || y != PosY ) ) return false;
	if ( !isNextTo(x, y) ) return false;

	return true;
}

bool cVehicle::canLoad ( int off, cMap *Map )
{
	if ( off < 0 || off > Map->size*Map->size ) return false;

	return canLoad ( Map->fields[off].getVehicles() );
}

bool cVehicle::canLoad ( cVehicle *Vehicle )
{
	if ( !Vehicle ) return false;

	if ( data.cargo >= data.max_cargo )	return false;

	if ( !isNextTo ( Vehicle->PosX, Vehicle->PosY ) ) return false;

	if ( data.can_drive == DRIVE_AIR && ( Vehicle->PosX != PosX || Vehicle->PosY != PosY ) ) return false;

	if ( data.can_drive != DRIVE_AIR && Vehicle->data.can_drive == DRIVE_SEA ) return false;

	if ( data.can_transport == TRANS_MEN && !Vehicle->data.is_human ) return false;

	if ( Vehicle->ClientMoveJob && ( Vehicle->moving || Vehicle->Attacking || Vehicle->MoveJobActive ) ) return false;

	if ( Vehicle->owner == owner && !Vehicle->IsBuilding && !Vehicle->IsClearing ) return true;

	return false;
}

void cVehicle::storeVehicle( cVehicle *Vehicle, cMap *Map )
{
	Map->deleteVehicle ( Vehicle );
	if ( Vehicle->bSentryStatus )
	{
		Vehicle->owner->deleteSentryVehicle( Vehicle);
		Vehicle->bSentryStatus = false;
	}

	Vehicle->Loaded = true;

	StoredVehicles.Add ( Vehicle );
	data.cargo++;

	if ( data.can_drive == DRIVE_AIR || data.cargo == data.max_cargo ) LoadActive = false;

	owner->DoScan();
}

void cVehicle::showStorage ()
{
	int LastMouseX = 0, LastMouseY = 0, LastB = 0, x, y, b, to;
	SDL_Surface *sf;
	SDL_Rect scr, dest;
	bool DownPressed = false, DownEnabled = false;
	bool UpPressed = false, UpEnabled = false;
	//bool AlleAktivierenEnabled = false;
	int offset = 0;
	Client->isInMenu = true;

	#define BUTTON__W 77
	#define BUTTON__H 23

	SDL_Rect rDialog = { SettingsData.iScreenW / 2 - DIALOG_W / 2, SettingsData.iScreenH / 2 - DIALOG_H / 2, DIALOG_W, DIALOG_H };

	SDL_Rect rBtnAllActive = {rDialog.x + 518, rDialog.y + 246, BUTTON__W, BUTTON__H};

	LoadActive = false;
	mouse->SetCursor ( CHand );
	mouse->draw ( false, buffer );
	scr.x = 480;
	scr.y = 0;
	scr.w = 640 - 480;
	scr.h = 480;
	dest.x = rDialog.x + scr.x;
	dest.y = rDialog.y + scr.y;
	SDL_BlitSurface ( GraphicsData.gfx_storage, &scr, buffer, &dest );
	dest.x = rDialog.x;
	dest.y = rDialog.y;
	SDL_BlitSurface ( GraphicsData.gfx_storage_ground, NULL, buffer, &dest );
	to = 6;

	// Alle Buttons machen:
	NormalButton btn_done(rDialog.x + 518, rDialog.y + 371, "Text~Button~Done");
	btn_done.Draw();
	// Down:

	if ( (int)StoredVehicles.Size() > to )
	{
		DownEnabled = true;
		scr.x = 103;
		scr.y = 452;
		scr.h = scr.w = 25;
		dest.x = 530;
		dest.y = 426;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
	}

	// Alle Aktivieren:
	if ( StoredVehicles.Size() ) drawButton ( lngPack.i18n ( "Text~Button~Active" ), false, rBtnAllActive.x, rBtnAllActive.y, buffer );
	else drawButton ( lngPack.i18n ( "Text~Button~Active" ), true, rBtnAllActive.x, rBtnAllActive.y, buffer );

	// Vehicles anzeigen:
	DrawStored ( offset );

	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );

	mouse->MoveCallback = false;

	while ( 1 )
	{
		if ( Client->SelectedVehicle != this ) break;
		if ( !Client->isInMenu ) break;

		Client->handleTimer();
		Client->doGameActions();

		// Events holen:
		EventHandler->HandleEvents();

		// Die Maus machen:
		mouse->GetPos();

		b = (int)Client->getMouseState().leftButtonPressed;

		x = mouse->x;

		y = mouse->y;

		if ( x != LastMouseX || y != LastMouseY )
		{
			mouse->draw ( true, screen );
		}

		// Down-Button:
		if ( DownEnabled )
		{
			if ( x >= rDialog.x + 530 && x < rDialog.x +  530 + 25 && y >= rDialog.y + 426 && y < rDialog.y + 426 + 25 && b && !DownPressed )
			{
				PlayFX ( SoundData.SNDObjectMenu );
				scr.x = 530;
				scr.y = 426;
				scr.w = 25;
				scr.h = 25;
				dest.x = rDialog.x + 530;
				dest.y = rDialog.y + 426;

				offset += to;

				if ( (int)StoredVehicles.Size() <= offset + to )
					DownEnabled = false;

				DrawStored ( offset );

				SDL_BlitSurface ( GraphicsData.gfx_storage, &scr, buffer, &dest );

				scr.x = 130;

				scr.y = 452;

				scr.w = 25;

				scr.h = 25;

				dest.x = rDialog.x + 504;

				dest.y = rDialog.y + 426;

				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );

				UpEnabled = true;

				SHOW_SCREEN
				mouse->draw ( false, screen );

				DownPressed = true;
			}
			else
				if ( !b && DownPressed && DownEnabled )
				{
					scr.x = 103;
					scr.y = 452;
					scr.w = 25;
					scr.h = 25;
					dest.x = rDialog.x + 530;
					dest.y = rDialog.y + 426;
					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
					SHOW_SCREEN
					mouse->draw ( false, screen );
					DownPressed = false;
				}
		}

		// Up-Button:
		if ( UpEnabled )
		{
			if ( x >= rDialog.x + 504 && x < rDialog.x + 504 + 25 && y >= rDialog.y + 426 && y < rDialog.y + 426 + 25 && b && !UpPressed )
			{
				PlayFX ( SoundData.SNDObjectMenu );
				scr.x = 504;
				scr.y = 426;
				scr.w = 25;
				scr.h = 25;
				dest.x = rDialog.x + 504;
				dest.y = rDialog.y + 426;

				offset -= to;

				if ( offset == 0 )
					UpEnabled = false;

				DrawStored ( offset );

				SDL_BlitSurface ( GraphicsData.gfx_storage, &scr, buffer, &dest );

				mouse->draw ( false, screen );

				UpPressed = true;

				if ( (int)StoredVehicles.Size() > to )
				{
					DownEnabled = true;
					scr.x = 103;
					scr.y = 452;
					scr.h = scr.w = 25;
					dest.x = rDialog.x + 530;
					dest.y = rDialog.y + 426;
					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
				}

				SHOW_SCREEN
			}
			else
				if ( !b && UpPressed && UpEnabled )
				{
					scr.x = 130;
					scr.y = 452;
					scr.w = 25;
					scr.h = 25;
					dest.x = rDialog.x + 504;
					dest.y = rDialog.y + 426;
					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
					SHOW_SCREEN
					mouse->draw ( false, screen );
					UpPressed = false;
				}
		}

		if (btn_done.CheckClick(x, y, b > LastB, b < LastB))
		{
			break;
		}

		if ( x >= rBtnAllActive.x && x < rBtnAllActive.x + rBtnAllActive.w && y >= rBtnAllActive.y && y < rBtnAllActive.y + rBtnAllActive.h && b && !LastB )
		{
			PlayFX ( SoundData.SNDMenuButton );
			drawButton ( lngPack.i18n ( "Text~Button~Active" ), false, rBtnAllActive.x, rBtnAllActive.y, buffer );
			SHOW_SCREEN
			mouse->draw ( false, screen );

			while ( b )
			{
				EventHandler->HandleEvents();
				Client->doGameActions();
				b = (int)Client->getMouseState().leftButtonPressed;
			}

			Client->OverUnitField = NULL;

			mouse->MoveCallback = true;

			PlayFX ( SoundData.SNDActivate );

			bool hasCheckedPlace[9];
			for ( int i = 0; i < 9; i++ )
			{
				hasCheckedPlace[i] = false;
			}

			for ( unsigned int i = 0; i < StoredVehicles.Size(); i++ )
			{
				cVehicle *vehicle = StoredVehicles[i];
				bool activated = false;
				for ( int ypos = PosY-1, poscount = 0; ypos <= PosY+1; ypos++ )
				{
					if ( ypos < 0 || ypos >= Client->Map->size ) continue;
					for ( int xpos = PosX-1; xpos <= PosX+1; xpos++, poscount++ )
					{
						if ( xpos < 0 || xpos >= Client->Map->size ) continue;
						if ( canExitTo ( xpos, ypos, Client->Map, vehicle->typ ) && !hasCheckedPlace[poscount] )
						{
							sendWantActivate ( iID, true, vehicle->iID, xpos, ypos );
							hasCheckedPlace[poscount] = true;
							activated = true;
							break;
						}
					}
					if ( activated ) break;
				}
			}
			break;
		}

		// Buttons unter den Vehicles:
		sf = GraphicsData.gfx_storage_ground;

		for ( int i = 0; i < to;i++ )
		{
			if ( (int)StoredVehicles.Size() <= i + offset )
				break;

			switch ( i )
			{

				case 0:
					dest.x = rDialog.x + 8;
					dest.y = rDialog.y + 191;
					break;

				case 1:
					dest.x = rDialog.x + 163;
					dest.y = rDialog.y + 191;
					break;

				case 2:
					dest.x = rDialog.x + 318;
					dest.y = rDialog.y + 191;
					break;

				case 3:
					dest.x = rDialog.x + 8;
					dest.y = rDialog.y + 426;
					break;

				case  4:
					dest.x = rDialog.x + 163;
					dest.y = rDialog.y + 426;
					break;

				case 5:
					dest.x = rDialog.x + 318;
					dest.y = rDialog.y + 426;
					break;
			}


			// Aktivieren:
			if ( x >= dest.x && x < dest.x + 73 && y >= dest.y && y < dest.y + 23 && b && !LastB )
			{
				PlayFX ( SoundData.SNDMenuButton );
				VehicleToActivate = i + offset;
				drawButton ( lngPack.i18n ( "Text~Button~Active" ), true, dest.x, dest.y, buffer );
				SHOW_SCREEN
				mouse->draw ( false, screen );

				while ( b )
				{
					EventHandler->HandleEvents();
					Client->doGameActions();
					b = (int)Client->getMouseState().leftButtonPressed;
				}

				if ( data.can_drive == DRIVE_AIR ) sendWantActivate ( iID, true, StoredVehicles[VehicleToActivate]->iID, PosX, PosY );
				else ActivatingVehicle = true;

				Client->OverUnitField = NULL;
				mouse->MoveCallback = true;
				Client->isInMenu = false;
				return;
			}

		}

		LastMouseX = x;

		LastMouseY = y;
		LastB = b;
	}

	mouse->MoveCallback = true;
	Client->isInMenu = false;
}

// Malt alle Bilder der geladenen Vehicles:
void cVehicle::DrawStored ( int off )
{
	SDL_Rect scr, dest;
	SDL_Rect rDialog = { SettingsData.iScreenW / 2 - DIALOG_W / 2, SettingsData.iScreenH / 2 - DIALOG_H / 2, DIALOG_W, DIALOG_H };
	SDL_Surface *sf;
	cVehicle *v;
	int i, to;

	to = 6;

	sf = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, DIALOG_W, DIALOG_H, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_BlitSurface ( GraphicsData.gfx_storage_ground, NULL, sf, NULL );

	for ( i = 0;i < to;i++ )
	{
		if ( i + off >= (int)StoredVehicles.Size() )
		{
			v = NULL;
		}
		else
		{
			v = StoredVehicles[i + off];
		}

		// Das Bild malen:
		switch ( i )
		{

			case 0 :
				dest.x = rDialog.x + ( scr.x = 17 );
				dest.y = rDialog.y + ( scr.y = 9 );
				break;

			case 1 :
				dest.x = rDialog.x + ( scr.x = 172 );
				dest.y = rDialog.y + ( scr.y = 9 );
				break;

			case 2 :
				dest.x = rDialog.x + ( scr.x = 327 );
				dest.y = rDialog.y + ( scr.y = 9 );
				break;

			case 3 :
				dest.x = rDialog.x + ( scr.x = 17 );
				dest.y = rDialog.y + ( scr.y = 244 );
				break;

			case 4 :
				dest.x = rDialog.x + ( scr.x = 172 );
				dest.y = rDialog.y + ( scr.y = 244 );
				break;

			case 5 :
				dest.x = rDialog.x + ( scr.x = 327 );
				dest.y = rDialog.y + ( scr.y = 244 );
				break;
		}

		scr.w = 128; //hangarwidth
		scr.h = 128; //hangarsize
		SDL_BlitSurface ( sf, &scr, buffer, &dest );

		if ( v )
		{
			SDL_BlitSurface ( v->typ->storage, NULL, buffer, &dest );
			// Den Namen ausgeben:
			font->showText ( dest.x + 5, dest.y + 5, v->name, FONT_LATIN_SMALL_WHITE );
			/*
			if (
			{
				char str[100];
				strcpy ( str,v->name.c_str() );
				str[strlen ( str )-1]=0;
				while (
				{
					str[strlen ( str )-1]=0;
				}

			}
			else
			{

			}*/
		}
		else
		{
			SDL_BlitSurface ( GraphicsData.gfx_edepot, NULL, buffer, &dest );
		}

		// Die Buttons malen:
		// Aktivieren:
		if ( to == 4 )
		{
			dest.x += 27;
		}
		else
		{
			dest.x -= 9;
		}

		dest.y += 182;
		if ( v )
		{
			drawButton ( lngPack.i18n ( "Text~Button~Active" ), false, dest.x, dest.y, buffer );
		}
		else
		{
			drawButton ( lngPack.i18n ( "Text~Button~Active" ), true, dest.x, dest.y, buffer );
		}


		// Display the additional info
		dest.x += 9;

		dest.y -= 44 - 6;

		scr.w = 128;

		scr.h = 30;

		scr.x = dest.x - rDialog.x;

		scr.y = dest.y - rDialog.y;

		SDL_BlitSurface ( sf, &scr, buffer, &dest );

		dest.x += 6;

		if ( v )
		{
			// Die Hitpoints anzeigen:
			DrawNumber ( dest.x + 13, dest.y, v->data.hit_points, v->data.max_hit_points, buffer );

			font->showText ( dest.x + 27, dest.y, lngPack.i18n ( "Text~Hud~Hitpoints" ), FONT_LATIN_SMALL_WHITE );
			DrawSymbol ( SHits, dest.x + 60, dest.y, 58, v->data.hit_points, v->data.max_hit_points, buffer );
			// Die Munition anzeigen:

			if ( v->data.can_attack )
			{
				dest.y += 15;
				DrawNumber ( dest.x + 13, dest.y, v->data.ammo, v->data.max_ammo, buffer );

				font->showText ( dest.x + 27, dest.y, lngPack.i18n ( "Text~Hud~AmmoShort" ), FONT_LATIN_SMALL_WHITE );
				DrawSymbol ( SAmmo, dest.x + 60, dest.y, 58, v->data.ammo, v->data.max_ammo, buffer );
			}
		}
	}
	SDL_FreeSurface(sf);
}

//-----------------------------------------------------------------------------
// Exits a vehicle
//-----------------------------------------------------------------------------
void cVehicle::exitVehicleTo( cVehicle *Vehicle, int offset, cMap *Map )
{
	for ( unsigned int i = 0; i < StoredVehicles.Size(); i++ )
	{
		if ( StoredVehicles[i] == Vehicle )
		{
			StoredVehicles.Delete ( i );
			break;
		}
		if ( i == StoredVehicles.Size()-1 ) return;
	}

	data.cargo--;

	ActivatingVehicle = false;

	Map->addVehicle ( Vehicle, offset );

	Vehicle->PosX = offset % Map->size;
	Vehicle->PosY = offset / Map->size;
	Vehicle->Loaded = false;
	//Vehicle->data.shots = 0;

	owner->DoScan();
}

//-----------------------------------------------------------------------------
// Checks, if an object can get ammunition.
//-----------------------------------------------------------------------------
bool cVehicle::canSupply ( int iOff, int iType )
{
	if ( iOff < 0 || iOff > Client->Map->size*Client->Map->size ) return false;

	cMapField& field = Client->Map->fields[iOff];
	if ( field.getVehicles() ) return canSupply ( field.getVehicles(), iType );
	else if ( field.getPlanes() ) return canSupply ( field.getPlanes(), iType );
	else if ( field.getTopBuilding() ) return canSupply ( field.getTopBuilding(), iType );

	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::canSupply( cVehicle *Vehicle, int iType )
{
	if ( !Vehicle ) return false;

	if ( data.cargo <= 0 ) return false;

	if ( Vehicle->PosX > PosX+1 || Vehicle->PosX < PosX-1 || Vehicle->PosY > PosY+1 || Vehicle->PosY < PosY-1 ) return false;

	if ( Vehicle->data.can_drive == DRIVE_AIR && Vehicle->FlightHigh > 0 ) return false;

	switch ( iType )
	{
	case SUPPLY_TYPE_REARM:
		if ( Vehicle == this || !Vehicle->data.can_attack || Vehicle->data.ammo >= Vehicle->data.max_ammo || Vehicle->moving || Vehicle->Attacking ) return false;
		break;
	case SUPPLY_TYPE_REPAIR:
		if ( Vehicle == this || Vehicle->data.hit_points >= Vehicle->data.max_hit_points || Vehicle->moving || Vehicle->Attacking ) return false;
		break;
	default:
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool cVehicle::canSupply( cBuilding *Building, int iType )
{
	if ( data.cargo <= 0 ) return false;

	if ( !Building->data.is_big )
	{
		if ( Building->PosX > PosX+1 || Building->PosX < PosX-1 || Building->PosY > PosY+1 || Building->PosY < PosY-1 ) return false;
	}
	else
	{
		if ( Building->PosX > PosX+1 || Building->PosX < PosX-2 || Building->PosY > PosY+1 || Building->PosY < PosY-2 ) return false;
	}

	switch ( iType )
	{
	case SUPPLY_TYPE_REARM:
		if ( !Building->data.can_attack || Building->data.ammo >= Building->data.max_ammo ) return false;
		break;
	case SUPPLY_TYPE_REPAIR:
		if ( Building->data.hit_points >= Building->data.max_hit_points ) return false;
		break;
	default:
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
void cVehicle::upgradeToCurrentVersion ()
{
	sUnitData& upgradeVersion = owner->VehicleData[typ->nr];
	data.version = upgradeVersion.version; // TODO: iVersion?
	
	if (data.hit_points == data.max_hit_points)
		data.hit_points = upgradeVersion.max_hit_points; // TODO: check behaviour in original
	data.max_hit_points = upgradeVersion.max_hit_points;
	
	data.max_ammo = upgradeVersion.max_ammo; // don't change the current ammo-amount!
	
	data.max_speed = upgradeVersion.max_speed;
	
	data.armor = upgradeVersion.armor;
	data.scan = upgradeVersion.scan;
	data.range = upgradeVersion.range;
	data.max_shots = upgradeVersion.max_shots; // TODO: check behaviour in original
	data.damage = upgradeVersion.damage;
	data.iBuilt_Costs = upgradeVersion.iBuilt_Costs;
	
	GenerateName();
}

//-----------------------------------------------------------------------------
bool cVehicle::layMine ()
{
	if ( data.cargo <= 0 ) return false;
	
	if (data.can_drive == DRIVE_SEA) 
	{
		if ( !Server->Map->possiblePlaceBuilding( UnitsData.building[BNrSeaMine].data, PosX, PosY, this)) return false;
		Server->addUnit(PosX, PosY, &UnitsData.building[BNrSeaMine], owner, false);
	}
	else 
	{
		if ( !Server->Map->possiblePlaceBuilding( UnitsData.building[BNrLandMine].data, PosX, PosY, this)) return false;
		Server->addUnit(PosX, PosY, &UnitsData.building[BNrLandMine], owner, false);
	}
	data.cargo--;

	if ( data.cargo <= 0 ) LayMines = false;

	return true;
}

bool cVehicle::clearMine ()
{
	cBuilding* Mine = Server->Map->fields[PosX+PosY*Server->Map->size].getMine();

	if ( !Mine || Mine->owner != owner || data.cargo >= data.max_cargo ) return false;

	Server->deleteUnit ( Mine );
	data.cargo++;

	if ( data.cargo >= data.max_cargo ) ClearMines = false;

	return true;
}

// Checks if the target is on a neighbour field and if it can be stolen or disabled
bool cVehicle::canDoCommandoAction ( int x, int y, cMap *map, bool steal )
{
	int off, boff;
	off = x + y * map->size; 
	boff = PosX + PosY * map->size;

	if ( !isNextTo ( x, y ) ) return false;
	if ( !data.shots ) return false;

	cVehicle*  vehicle  = map->fields[off].getVehicles();
	cBuilding* building = map->fields[off].getBuildings();

	if ( steal && vehicle && vehicle->StoredVehicles.Size() ) return false;

	if ( steal && vehicle && vehicle->owner != owner ) return true;

	if ( !steal && ( ( vehicle && vehicle->owner != owner ) || ( building && building->owner != owner ) ) ) return true;

	return false;
}

// draws the commando-cursors:
void cVehicle::drawCommandoCursor ( int off, bool steal )
{
	cMapField* field = Client->Map->fields + off;
	cBuilding *b = NULL;
	cVehicle *v = NULL;
	SDL_Surface *sf;
	SDL_Rect r;

	if ( steal )
	{
		v = field->getVehicles();
		sf = GraphicsData.gfx_Csteal;
	}
	else
	{
		v = field->getVehicles();
		if ( !v ) b = field->getTopBuilding();
		sf = GraphicsData.gfx_Cdisable;
	}

	r.x = 1;
	r.y = 28;
	r.h = 3;
	r.w = 35;

	if ( !v && !b )
	{
		SDL_FillRect ( sf, &r, 0 );
		return;
	}

	SDL_FillRect ( sf, &r, 0xFF0000 );
	r.w = 35*calcCommandoChance( v, b, steal )/100;
	SDL_FillRect ( sf, &r, 0x00FF00 );
}

int cVehicle::calcCommandoChance( cVehicle *destVehicle, cBuilding *destBuilding, bool steal )
{
	int destTurn, factor, srcLevel, chance;

	if ( !destVehicle && !destBuilding ) return 0;

	if ( destVehicle ) destTurn = destVehicle->data.iBuilt_Costs/3;
	else if ( destBuilding ) destTurn = destBuilding->data.iBuilt_Costs/3;

	factor = steal ? 1 : 4;
	srcLevel = (int)CommandoRank+7;

	/* The chance to disable or steal a unit depends on the infiltratorranking and the
	buildcosts (or 'turns' in the original game) of the target unit. The chance rises
	linearly with a higher ranking of the infiltrator. The chance of a unexperienced
	infiltrator will be calculated like he has the ranking 7. Disabling has a 4 times
	higher chance then stealing.
	*/
	chance = Round( (float)(8*srcLevel)/(35*destTurn)*factor*100 );

	if ( chance > 90 ) chance = 90;
	return chance;
}

void cVehicle::DeleteStored ( void )
{
	while ( StoredVehicles.Size() )
	{
		cVehicle *v;
		v = StoredVehicles[0];

		if ( v->prev )
		{
			cVehicle *vp;
			vp = v->prev;
			vp->next = v->next;

			if ( v->next )
				v->next->prev = vp;
		}
		else
		{
			v->owner->VehicleList = v->next;

			if ( v->next )
				v->next->prev = NULL;
		}

		v->DeleteStored();

		delete v;
		StoredVehicles.Delete ( 0 );
	}
}

bool cVehicle::isDetectedByPlayer( cPlayer* player )
{
	for ( unsigned int i = 0; i < DetectedByPlayerList.Size(); i++ )
	{
		if (DetectedByPlayerList[i] == player) return true;
	}
	return false;
}

void cVehicle::setDetectedByPlayer( cPlayer* player )
{
	bool wasDetected = ( DetectedByPlayerList.Size() > 0 );

	if (!isDetectedByPlayer( player ))
		DetectedByPlayerList.Add( player );

	if ( !wasDetected ) sendDetectionState( this );
}

void cVehicle::resetDetectedByPlayer( cPlayer* player )
{
	bool wasDetected = ( DetectedByPlayerList.Size() > 0 );

	for ( unsigned int i = 0; i < DetectedByPlayerList.Size(); i++ )
	{
		if ( DetectedByPlayerList[i] == player ) DetectedByPlayerList.Delete(i);
	}

	if ( wasDetected && DetectedByPlayerList.Size() == 0 ) sendDetectionState( this );
}

void cVehicle::makeDetection()
{
	//check whether the vehicle has been detected by others
	if ( data.is_stealth_land || data.is_stealth_sea )
	{
		int offset = PosX + PosY * Server->Map->size;
		for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
		{
			cPlayer* player = (*Server->PlayerList)[i];
			if ( player == owner ) continue;

			if ( data.is_stealth_land && ( player->DetectLandMap[offset] || Server->Map->IsWater(offset) ))
			{
				setDetectedByPlayer( player );
			}
			
			bool water = Server->Map->IsWater(offset, true);
			//if the vehicle can also drive on land, we have to check, whether there is a brige, platform, etc.
			//because the vehicle will drive on the bridge
			cBuilding* building = Server->Map->fields[offset].getBaseBuilding();
			if ( data.can_drive != DRIVE_SEA && building && ( building->data.is_bridge || building->data.is_platform || building->data.is_road )) water = false;

			if ( data.is_stealth_sea && ( player->DetectSeaMap[offset] || !water ))
			{
				setDetectedByPlayer( player );
			}
		}
	}
		
	//detect other units
	if ( data.can_detect_land || data.can_detect_sea || data.can_detect_mines )
	{
		for ( int x = PosX - data.scan; x < PosX + data.scan; x++)
		{
			if ( x < 0 || x >= Server->Map->size ) continue;
			for ( int y = PosY - data.scan; y < PosY + data.scan; y++)
			{
				if ( y < 0 || y >= Server->Map->size ) continue;
				
				int offset = x + y * Server->Map->size;
				cVehicle* vehicle = Server->Map->fields[offset].getVehicles();
				cBuilding* building = Server->Map->fields[offset].getMine();

				if ( vehicle && vehicle->owner != owner )
				{
					if ( data.can_detect_land && owner->DetectLandMap[offset] && vehicle->data.is_stealth_land )
					{
						vehicle->setDetectedByPlayer( owner );
					}
					if ( data.can_detect_sea && owner->DetectSeaMap[offset] && vehicle->data.is_stealth_sea )
					{
						vehicle->setDetectedByPlayer( owner );
					}
				}
				if ( building && building->owner != owner )
				{
					if ( data.can_detect_mines && owner->DetectMinesMap[offset] && building->data.is_expl_mine )
					{
						building->setDetectedByPlayer( owner );
					}
				}
			}
		}
	}
}

bool cVehicle::isNextTo( int x, int y) const
{
	if ( x + 1 < PosX || y + 1 < PosY ) return false;

	if ( data.is_big )
	{
		if ( x - 2 > PosX || y - 2 > PosY ) return false;
	}
	else
	{
		if ( x - 1 > PosX || y - 1 > PosY ) return false;
	}

	return true;
}

void sVehicle::scaleSurfaces( float factor )
{
	int width, height;
	for ( int i = 0; i < 8; i++ )
	{
		width= (int) ( img_org[i]->w*factor );
		height= (int) ( img_org[i]->h*factor );
		scaleSurface ( img_org[i], img[i], width, height );
		width= (int) ( shw_org[i]->w*factor );
		height= (int) ( shw_org[i]->h*factor );
		scaleSurface ( shw_org[i], shw[i], width, height );
	}
	if ( build_org )
	{
		height= (int) ( build_org->h*factor );
		width=height*4;
		scaleSurface ( build_org, build, width, height );
		width= (int) ( build_shw_org->w*factor );
		height= (int) ( build_shw_org->h*factor );
		scaleSurface ( build_shw_org, build_shw, width, height );
	}
	if ( clear_small_org )
	{
		height= (int) ( clear_small_org->h*factor );
		width=height*4;
		scaleSurface ( clear_small_org, clear_small, width, height );
		width= (int) ( clear_small_shw_org->w*factor );
		height= (int) ( clear_small_shw_org->h*factor );
		scaleSurface ( clear_small_shw_org, clear_small_shw, width, height );
	}
	if ( overlay_org )
	{
		height= (int) ( overlay_org->h*factor );
		width= (int) ( overlay_org->w*factor );
		scaleSurface ( overlay_org, overlay, width, height );
	}
}

void cVehicle::blitWithPreScale ( SDL_Surface *org_src, SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dest, SDL_Rect *destrect, float factor, int frames )
{
	if ( !SettingsData.bPreScale )
	{
		int width, height;
		height = (int) ( org_src->h*factor );
		if ( frames > 1 ) width = height*frames;
		else width = (int) ( org_src->w*factor );
		if ( src->w != width || src->h != height )
		{
			scaleSurface ( org_src, src, width, height );
		}
	}
	SDL_BlitSurface ( src, srcrect, dest, destrect );
}
