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
#include "math.h"
#include "vehicles.h"
#include "fonts.h"
#include "game.h"
#include "mouse.h"
#include "mjobs.h"
#include "sound.h"
#include "map.h"
#include "dialog.h"
#include "files.h"
#include "pcx.h"
#include "networkmessages.h"

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
	BuildingTyp = 0;
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
	mjob = NULL;
	autoMJob = NULL;
	moving = false;
	rotating = false;
	MoveJobActive = false;
	MenuActive = false;
	AttackMode = false;
	Attacking = false;
	IsBuilding = false;
	PlaceBand = false;
	BuildOverride = false;
	IsClearing = false;
	ClearBig = false;
	ShowBigBeton = false;
	Wachposten = false;
	Transfer = false;
	LoadActive = false;
	ActivatingVehicle = false;
	MuniActive = false;
	RepairActive = false;
	BuildPath = false;
	LayMines = false;
	ClearMines = false;
	detected = true;
	Loaded = false;
	StealActive = false;
	DisableActive = false;
	detection_override = false;
	IsLocked = false;
	StoredVehicles = NULL;

	if ( data.can_transport == TRANS_VEHICLES || data.can_transport == TRANS_MEN )
	{
		StoredVehicles = new cList<cVehicle*>;
	}

	DamageFXPointX = random ( 7, 0 ) + 26 - 3;

	DamageFXPointY = random ( 7, 0 ) + 26 - 3;
	RefreshData();

}

cVehicle::~cVehicle ( void )
{
	if ( mjob )
	{
		mjob->vehicle = NULL;
		mjob->Release();
	}

	if ( autoMJob )
	{
		delete autoMJob;
	}

	if ( Wachposten )
	{
		owner->DeleteWachpostenV ( this );
	}

	if ( StoredVehicles )
		DeleteStored();

	if ( Attacking )
	{
		int i;

		for ( i = 0;i < game->engine->AJobs->iCount;i++ )
		{
			cAJobs *a;
			a = game->engine->AJobs->Items[i];

			if ( a->vehicle == this )
			{
				game->engine->AJobs->Delete ( i );
				break;
			}
		}
	}

	if ( IsLocked )
	{
		cPlayer *p;
		int i;

		for ( i = 0;i < game->PlayerList->iCount;i++ )
		{
			p = game->PlayerList->Items[i];
			p->DeleteLock ( this );
		}
	}
}

// Malt das Vehicle:
void cVehicle::Draw ( SDL_Rect *dest )
{
	SDL_Rect scr, tmp;
	int ox = 0, oy = 0;

	// Workarounds:

	if ( ( !mjob || !MoveJobActive ) && ( OffX != 0 || OffY != 0 ) )
	{
		OffX = 0;
		OffY = 0;
	}

	if ( !mjob && ( MoveJobActive || moving || rotating ) )
	{
		MoveJobActive = false;
		moving = false;
		rotating = false;
		StopFXLoop ( game->ObjectStream );
		game->ObjectStream = PlayStram();
	}

	if ( mjob && moving && !MoveJobActive )
	{
		try
		{
			mjob->finished = true;
		}
		catch ( ... )
			{}

		mjob = NULL;

		moving = false;
	}

	// Den Schadenseffekt machen:
	if ( timer1 && data.hit_points < data.max_hit_points && SettingsData.bDamageEffects && !moving && ( owner == game->ActivePlayer || game->ActivePlayer->ScanMap[PosX+PosY*game->map->size] ) )
	{
		int intense = ( int ) ( 100 - 100 * ( ( float ) data.hit_points / data.max_hit_points ) );
		game->AddFX ( fxDarkSmoke, PosX*64 + DamageFXPointX, PosY*64 + DamageFXPointY, intense );
	}

	// Prüfen, ob das Vehicle gemalt werden soll:
	if ( ( data.is_stealth_sea || data.is_stealth_land ) && game->ActivePlayer != owner )
	{
		if ( data.is_stealth_land )
		{
			if ( game->ActivePlayer->DetectLandMap[PosX+PosY*game->map->size] || detection_override )
			{
				detected = true;
			}
			else
			{
				detected = false;
			}
		}
		else
		{
			if ( game->ActivePlayer->DetectSeaMap[PosX+PosY*game->map->size] || !game->map->IsWater ( PosX + PosY*game->map->size, true ) )
			{
				detected = true;
			}
			else
			{
				detected = false;
			}
		}

		if ( !detected )
			return;
	}

	float newzoom = ( 64.0 / game->hud->Zoom );

	if ( OffX )
		ox = ( int ) ( OffX / newzoom );

	if ( OffY )
		oy = ( int ) ( OffY / newzoom );

	tmp = *dest;

	tmp.x += ox;

	tmp.y += oy;

	if ( IsBuilding && data.can_build == BUILD_BIG )
		dir = 0;

	if ( IsClearing && ClearBig )
		dir = 0;

	// Prüfen, ob gebaut wird:
	if ( ( !IsBuilding && !IsClearing ) || dir != 0 || BuildOverride )
	{
		if ( ( IsBuilding || IsClearing ) && timer0 )
		{
			// Vehicle drehen:
			RotateTo ( 0 );
		}

		// Größe auslesen:
		scr.w = dest->w = typ->img[dir]->w;

		scr.h = dest->h = typ->img[dir]->h;

		// Den Schatten malen:
		if ( SettingsData.bShadows && ! ( data.is_stealth_sea && game->map->IsWater ( PosX + PosY*game->map->size, true ) ) )
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
						SDL_BlitSurface ( typ->shw[dir], &r, buffer, &tmp );
					}
					else
					{
						SDL_BlitSurface ( typ->shw[dir], NULL, buffer, &tmp );
					}
				}
				else
				{
					int high;
					high = ( ( int ) ( game->hud->Zoom * ( FlightHigh / 64.0 ) ) );
					tmp.x += high + ditherX;
					tmp.y += high + ditherY;
					SDL_BlitSurface ( typ->shw[dir], NULL, buffer, &tmp );
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
						SDL_BlitSurface ( typ->shw[dir], &r, buffer, &tmp );
					}
					else
					{
						SDL_BlitSurface ( typ->shw[dir], NULL, buffer, &tmp );
					}
				}
				else
				{
					cBuilding *b;
					int high;
					// Prüfen, ob das Flugzeug landen soll:
					b = game->map->GO[PosX+PosY*game->map->size].top;

					if ( timer0 )
					{
						if ( b && b->owner == owner && b->data.is_pad && !mjob && !rotating && !Attacking )
						{
							if ( FlightHigh > 0 )
								FlightHigh -= 8;
						}
						else
							if ( FlightHigh < 64 )
							{
								FlightHigh += 8;
							}
					}

					// Schatten malen:
					if ( FlightHigh > 0 )
					{
						high = ( ( int ) ( game->hud->Zoom * ( FlightHigh / 64.0 ) ) );
						tmp.x += high + ditherX;
						tmp.y += high + ditherY;
					}

					SDL_BlitSurface ( typ->shw[dir], NULL, buffer, &tmp );
				}
			}
		}

		// Die Spielerfarbe blitten:
		SDL_BlitSurface ( owner->color, NULL, GraphicsData.gfx_tmp, NULL );

		if ( data.is_human )
		{
			scr.w = scr.h = tmp.h = tmp.w = typ->img[dir]->h;
			tmp.x = WalkFrame * tmp.w;
			tmp.y = 0;
			SDL_BlitSurface ( typ->img[dir], &tmp, GraphicsData.gfx_tmp, NULL );
		}
		else
		{
			SDL_BlitSurface ( typ->img[dir], NULL, GraphicsData.gfx_tmp, NULL );
		}

		// Das Vehicle malen:
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

			if ( timer0 )
				StartUp += 25;

			if ( StartUp >= 255 )
				StartUp = 0;
		}
		else
		{
			if ( data.is_stealth_sea && game->map->IsWater ( PosX + PosY*game->map->size, true ) )
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

		// Ggf das Overlay malen:
		if ( data.has_overlay && SettingsData.bAnimations )
		{
			tmp = *dest;
			scr.h = scr.w = typ->overlay->h;
			tmp.x += game->hud->Zoom / 2 - scr.h / 2 + ox + ditherX;
			tmp.y += game->hud->Zoom / 2 - scr.h / 2 + oy + ditherY;
			scr.y = 0;
//      scr.x=scr.h*((game->Frame%(typ->overlay->w/scr.h)));

			if ( Disabled )
			{
				scr.x = 0;
			}
			else
			{
				scr.x = ( int ) ( ( typ->overlay_org->h * ( ( game->Frame % ( typ->overlay->w / scr.h ) ) ) ) / newzoom );
			}

			if ( StartUp && SettingsData.bAlphaEffects )
			{
				SDL_SetAlpha ( typ->overlay, SDL_SRCALPHA, StartUp );
				SDL_BlitSurface ( typ->overlay, &scr, buffer, &tmp );
				SDL_SetAlpha ( typ->overlay, SDL_SRCALPHA, 255 );

				if ( timer0 )
					StartUp += 25;

				if ( StartUp >= 255 )
					StartUp = 0;
			}
			else
			{
				SDL_BlitSurface ( typ->overlay, &scr, buffer, &tmp );
			}
		}
	}
	else
		if ( IsBuilding || ( IsClearing && ClearBig ) )
		{
			// Ggf den Beton malen:
			if ( ShowBigBeton )
			{
				SDL_SetAlpha ( GraphicsData.gfx_big_beton, SDL_SRCALPHA, BigBetonAlpha );
				SDL_BlitSurface ( GraphicsData.gfx_big_beton, NULL, buffer, &tmp );
				tmp = *dest;

				if ( BigBetonAlpha < 255 )
				{
					if ( timer0 )
						BigBetonAlpha += 25;

					if ( BigBetonAlpha > 255 )
						BigBetonAlpha = 255;
				}
			}

			// Den Schatten malen:
			if ( SettingsData.bShadows )
			{
				SDL_BlitSurface ( typ->build_shw, NULL, buffer, &tmp );
			}

			// Die Spielerfarbe blitten:
			scr.y = 0;

			scr.h = scr.w = typ->build->h;

			scr.x = ( game->Frame % 4 ) * scr.w;

			SDL_BlitSurface ( owner->color, NULL, GraphicsData.gfx_tmp, NULL );

			SDL_BlitSurface ( typ->build, &scr, GraphicsData.gfx_tmp, NULL );

			// Das Vehicle malen:
			scr.x = 0;

			scr.y = 0;

			tmp = *dest;

			tmp.x += ox;

			tmp.y += oy;

			SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, buffer, &tmp );

			// Ggf Markierung blitten, wenn der Bauvorgang abgeschlossen ist:
			if ( ( ( IsBuilding && BuildRounds == 0 ) || ( IsClearing && ClearingRounds == 0 ) ) && owner == game->ActivePlayer )
			{
				/*SDL_Rect d;
				   int nr1;//,nr2;
				   nr1=0xFF00-((game->Frame%0x8)*0x1000);
				SDL_SetColorKey(GraphicsData.gfx_build_finished_org,SDL_SRCCOLORKEY,0xFFFFFF);
				d.x=d.y=0;
				d.h=d.w=GraphicsData.gfx_build_finished->w;
				// GraphicsData.gfx_build_finished=SDL_CreateRGBSurface(SDL_HWSURFACE,GraphicsData.gfx_build_finished_org->w,GraphicsData.gfx_build_finished_org->h,32,0,0,0,0);
				SDL_FillRect(GraphicsData.gfx_build_finished, NULL, nr1);
				SDL_BlitSurface(GraphicsData.gfx_build_finished_org,NULL,GraphicsData.gfx_build_finished,NULL);
				SDL_SetColorKey(GraphicsData.gfx_build_finished,SDL_SRCCOLORKEY,0xFF00FF);
				   SDL_BlitSurface(GraphicsData.gfx_build_finished,&d,buffer,&tmp);*_/
				SDL_Rect sr, de;
				int nr, zoom;
				zoom = game->hud->Zoom;
				nr = game->Frame % 5;
				sr.x = nr * (zoom * 2); sr.y = 0;
				de.h = de.w = sr.h = sr.w = zoom * 2;
				de.x = tmp.x; de.y = tmp.y;
				SDL_BlitSurface(GraphicsData.gfx_build_finished,&sr,buffer,&de);*/

				SDL_Rect d, t;
				int max, nr;
				nr = 0xFF00 - ( ( game->Frame % 0x8 ) * 0x1000 );

				if ( data.can_build == BUILD_BIG || IsClearing )
				{
					max = ( game->hud->Zoom - 3 ) * 2;
				}
				else
				{
					max = game->hud->Zoom - 3;
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
			// Den Schatten malen:
			if ( SettingsData.bShadows )
			{
				SDL_BlitSurface ( typ->clear_small_shw, NULL, buffer, &tmp );
			}

			// Die Spielerfarbe blitten:
			scr.y = 0;

			scr.h = scr.w = typ->clear_small->h;

			scr.x = ( game->Frame % 4 ) * scr.w;

			SDL_BlitSurface ( owner->color, NULL, GraphicsData.gfx_tmp, NULL );

			SDL_BlitSurface ( typ->clear_small, &scr, GraphicsData.gfx_tmp, NULL );

			// Das Vehicle malen:
			scr.x = 0;

			scr.y = 0;

			tmp = *dest;

			tmp.x += ox;

			tmp.y += oy;

			SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, buffer, &tmp );

			// Ggf Markierung malen, wenn der Bauvorgang abgeschlossen ist:
			if ( ClearingRounds == 0 && owner == game->ActivePlayer )
			{
				SDL_Rect d, t;
				int max, nr;
				nr = 0xFF00 - ( ( game->Frame % 0x8 ) * 0x1000 );
				max = game->hud->Zoom - 3;
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
	if ( game->hud->Farben )
	{
		SDL_Rect d, t;
		int max, nr;
		nr = * ( unsigned int* ) owner->color->pixels;

		if ( ( IsBuilding && data.can_build == BUILD_BIG ) || ( IsClearing && ClearBig ) )
		{
			max = ( game->hud->Zoom - 1 ) * 2;
		}
		else
		{
			max = game->hud->Zoom - 1;
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

		if ( ( IsBuilding && data.can_build == BUILD_BIG ) || ( IsClearing && ClearBig ) )
		{
			max = game->hud->Zoom * 2;
		}
		else
		{
			max = game->hud->Zoom;
		}

		len = max / 4;

		d.x = dest->x + 1 + ox;
		d.y = dest->y + 1 + oy;
		d.w = len;
		d.h = 1;
		t = d;
		SDL_FillRect ( buffer, &d, game->BlinkColor );
		d = t;
		d.x += max - len - 1;
		t = d;
		SDL_FillRect ( buffer, &d, game->BlinkColor );
		d = t;
		d.y += max - 2;
		t = d;
		SDL_FillRect ( buffer, &d, game->BlinkColor );
		d = t;
		d.x = dest->x + 1 + ox;
		t = d;
		SDL_FillRect ( buffer, &d, game->BlinkColor );
		d = t;
		d.y = dest->y + 1 + oy;
		d.w = 1;
		d.h = len;
		t = d;
		SDL_FillRect ( buffer, &d, game->BlinkColor );
		d = t;
		d.x += max - 2;
		t = d;
		SDL_FillRect ( buffer, &d, game->BlinkColor );
		d = t;
		d.y += max - len - 1;
		t = d;
		SDL_FillRect ( buffer, &d, game->BlinkColor );
		d = t;
		d.x = dest->x + 1 + ox;
		SDL_FillRect ( buffer, &d, game->BlinkColor );
	}

	// Das Dithering machen:

	if ( data.can_drive == DRIVE_AIR && timer0 )
	{
		if ( moving || game->Frame % 10 == 0 )
		{
			ditherX = 0;
			ditherY = 0;
		}
		else
		{
			ditherX = random ( 2, 0 ) - 1;
			ditherY = random ( 2, 0 ) - 1;
		}
	}

	// Ggf die Brücke drüber malen:
	if ( data.can_drive == DRIVE_SEA )
	{
#define TEST_BRIDGE(x,y) PosX+x>=0&&PosX+x<game->map->size&&PosY+y>=0&&PosY+y<game->map->size&&game->map->GO[PosX+(x)+(PosY+(y))*game->map->size].base&&game->map->GO[PosX+(x)+(PosY+(y))*game->map->size].base->data.is_bridge

		if ( TEST_BRIDGE ( 0, 0 ) )
		{
			game->map->GO[PosX+PosY*game->map->size].base->Draw ( dest );
		}

		if ( OffX > 0 && OffY == 0 && TEST_BRIDGE ( 1, 0 ) )
		{
			tmp = *dest;
			tmp.x += game->hud->Zoom;
			game->map->GO[PosX+1+PosY*game->map->size].base->Draw ( &tmp );
		}
		else
			if ( OffX < 0 && OffY == 0 && TEST_BRIDGE ( -1, 0 ) )
			{
				tmp = *dest;
				tmp.x -= game->hud->Zoom;
				game->map->GO[PosX-1+PosY*game->map->size].base->Draw ( &tmp );
			}
			else
				if ( OffX == 0 && OffY > 0 && TEST_BRIDGE ( 0, 1 ) )
				{
					tmp = *dest;
					tmp.y += game->hud->Zoom;
					game->map->GO[PosX+ ( PosY+1 ) *game->map->size].base->Draw ( &tmp );
				}
				else
					if ( OffX == 0 && OffY < 0 && TEST_BRIDGE ( 0, -1 ) )
					{
						tmp = *dest;
						tmp.y -= game->hud->Zoom;
						game->map->GO[PosX+ ( PosY-1 ) *game->map->size].base->Draw ( &tmp );
					}
					else
						if ( OffX > 0 && OffY > 0 && TEST_BRIDGE ( 1, 1 ) )
						{
							tmp = *dest;
							tmp.x += game->hud->Zoom;
							tmp.y += game->hud->Zoom;
							game->map->GO[PosX+1+ ( PosY+1 ) *game->map->size].base->Draw ( &tmp );
						}
						else
							if ( OffX < 0 && OffY < 0 && TEST_BRIDGE ( -1, -1 ) )
							{
								tmp = *dest;
								tmp.x -= game->hud->Zoom;
								tmp.y -= game->hud->Zoom;
								game->map->GO[PosX-1+ ( PosY-1 ) *game->map->size].base->Draw ( &tmp );
							}
							else
								if ( OffX > 0 && OffY < 0 && TEST_BRIDGE ( 1, -1 ) )
								{
									tmp = *dest;
									tmp.x += game->hud->Zoom;
									tmp.y -= game->hud->Zoom;
									game->map->GO[PosX+1+ ( PosY-1 ) *game->map->size].base->Draw ( &tmp );
								}
								else
									if ( OffX < 0 && OffY > 0 && TEST_BRIDGE ( -1, 1 ) )
									{
										tmp = *dest;
										tmp.x -= game->hud->Zoom;
										tmp.y += game->hud->Zoom;
										game->map->GO[PosX-1+ ( PosY+1 ) *game->map->size].base->Draw ( &tmp );
									}
	}
}

// Wählt dieses Vehicle aus:
void cVehicle::Select ( void )
{
	int error;
	selected = true;
	// Das Video laden:

	if ( strcmp ( game->FLCname.c_str(), typ->FLCFile ) )
	{
		if ( game->FLC != NULL )
		{
			FLI_Close ( game->FLC );
		}

		if ( game->video )
		{
			game->video = NULL;
		}

		game->FLC = FLI_Open ( SDL_RWFromFile ( typ->FLCFile, "rb" ), &error );

		game->FLCname = typ->FLCFile;

		if ( error != 0 )
		{
			game->hud->ResetVideoMonitor();
			game->FLC = NULL;
			return;
		}

		FLI_Rewind ( game->FLC );

		FLI_NextFrame ( game->FLC );
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
	dest.w = scr.w = 155;
	dest.h = scr.h = 48;
	dest.x = 8;
	dest.y = 171;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, GraphicsData.gfx_hud, &dest );
	StopFXLoop ( game->ObjectStream );
	game->ObjectStream = -1;
}

// Erzeugt den Namen für das Vehicle aus der Versionsnummer:
void cVehicle::GenerateName ( void )
{
	string rome;
	int nr, tmp;
	rome = "";
	nr = data.version;

	// Römische Versionsnummer erzeugen (ist bis 899 richtig):

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
	name = ( string ) data.name;

	name += " MK ";

	name += rome;
}

bool cVehicle::CheckPathBuild ( int iOff, int iBuildingTyp )
{
	if ( UnitsData.building[iBuildingTyp].data.is_base )
	{
		if ( ( ( game->map->GO[iOff].base && ! ( game->map->GO[iOff].base->data.is_road || game->map->GO[iOff].base->data.is_platform ) ) || ( game->map->GO[iOff].top && !game->map->GO[iOff].top->data.is_connector ) ) )
		{
			return false;
		}
	}
	else
	{
		if ( game->map->GO[iOff].top && !game->map->GO[iOff].top->data.is_connector )
		{
			return false;
		}
	}

	if ( UnitsData.building[iBuildingTyp].data.build_on_water )
	{
		if ( !game->map->IsWater ( iOff ) )
		{
			return false;
		}
	}
	else
	{
		if ( game->map->IsWater ( iOff ) && ! ( game->map->GO[iOff].base && game->map->GO[iOff].base->data.is_platform ) && !UnitsData.building[iBuildingTyp].data.is_connector )
		{
			return false;
		}
	}

	return true;
}

// Aktalisiert alle Daten auf ihre Max-Werte:
void cVehicle::RefreshData ( void )
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

	// Regenerieren:
	if ( data.is_alien && data.hit_points < data.max_hit_points )
	{
		data.hit_points++;
	}

	// Bauen:
	if ( IsBuilding && BuildRounds )
	{

		data.cargo -= ( BuildCosts / BuildRounds );
		BuildCosts -= ( BuildCosts / BuildRounds );

		if ( data.cargo < 0 )
			data.cargo = 0;

		BuildRounds--;

		if ( BuildRounds == 0 && game->SelectedVehicle == this )
		{
			StopFXLoop ( game->ObjectStream );
			game->ObjectStream = PlayStram();
		}

		if ( BuildRounds == 0 && owner == game->ActivePlayer )
			game->engine->AddReport ( ( UnitsData.building + BuildingTyp )->data.name, false );

		if ( BuildRounds == 0 && ( UnitsData.building[BuildingTyp].data.is_base || UnitsData.building[BuildingTyp].data.is_connector ) && ( ! ( BandX != PosX || BandY != PosY ) || data.cargo == 0 ) )
		{
			IsBuilding = false;
			BuildPath = false;
			game->engine->AddBuilding ( PosX, PosY, UnitsData.building + BuildingTyp, owner );
		}

		if ( BuildPath && ( owner == game->ActivePlayer || game->HotSeat ) && BuildRounds == 0 && data.can_build == BUILD_SMALL && ( BandX != PosX || BandY != PosY ) )
		{
			cMJobs *mj = NULL;

			if ( data.cargo >= BuildCostsStart )
			{
				if ( BandX < PosX && CheckPathBuild ( PosX - 1 + PosY*game->map->size, BuildingTyp ) )
					mj = game->engine->AddMoveJob ( PosX + PosY * game->map->size, PosX - 1 + PosY * game->map->size, false, false );
				else
					if ( BandX > PosX && CheckPathBuild ( PosX + 1 + PosY*game->map->size, BuildingTyp ) )
						mj = game->engine->AddMoveJob ( PosX + PosY * game->map->size, PosX + 1 + PosY * game->map->size, false, false );
					else
						if ( BandY < PosY && CheckPathBuild ( PosX + ( PosY - 1 ) *game->map->size, BuildingTyp ) )
							mj = game->engine->AddMoveJob ( PosX + PosY * game->map->size, PosX + ( PosY - 1 ) * game->map->size, false, false );
						else
							if ( BandY > PosY && CheckPathBuild ( PosX + ( PosY + 1 ) *game->map->size, BuildingTyp ) )
								mj = game->engine->AddMoveJob ( PosX + PosY * game->map->size, PosX + ( PosY + 1 ) * game->map->size, false, false );
							else
							{
								BuildPath = false;
							}

				{
					if ( mj && !mj->finished && data.cargo >= BuildCostsStart )
						mj->BuildAtTarget = true;
					else
					{
						if ( mj )
							mj->finished = true;

						mjob = NULL;

						moving = false;

						IsBuilding = true;

						BuildPath = false;
					}
				}
			}
			else
			{
				BuildPath = false;
			}
		}
		else
		{
			if ( !BuildRounds )
				BuildPath = false;
		}
	}
	else
	{
		BuildPath = false;
	}

	// Räumen:
	if ( IsClearing && ClearingRounds )
	{
		ClearingRounds--;

		if ( ClearingRounds == 0 && game->SelectedVehicle == this )
		{
			StopFXLoop ( game->ObjectStream );
			game->ObjectStream = PlayStram();
		}
	}
}

// Zeigt die Eigenschaften des Vehicles an:
void cVehicle::ShowDetails ( void )
{
	SDL_Rect scr, dest;
	// Den Hintergrund wiederherstellen:
	scr.x = 0;
	scr.y = 215;
	dest.w = scr.w = 155;
	dest.h = scr.h = 48;
	dest.x = 8;
	dest.y = 171;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, GraphicsData.gfx_hud, &dest );
	// Die Hitpoints anzeigen:
	DrawNumber ( 31, 177, data.hit_points, data.max_hit_points, GraphicsData.gfx_hud );

	font->showText ( 55, 177, lngPack.i18n ( "Text~Hud~Hitpoints" ), LATIN_SMALL_WHITE, GraphicsData.gfx_hud );
	DrawSymbol ( SHits, 88, 174, 70, data.hit_points, data.max_hit_points, GraphicsData.gfx_hud );
	// Den Speed anzeigen:
	DrawNumber ( 31, 201, data.speed / 2, data.max_speed / 2, GraphicsData.gfx_hud );

	font->showText ( 55, 201, lngPack.i18n ( "Text~Hud~Speed" ), LATIN_SMALL_WHITE, GraphicsData.gfx_hud );
	DrawSymbol ( SSpeed, 88, 199, 70, data.speed / 2, data.max_speed / 2, GraphicsData.gfx_hud );
	// Zusätzliche Werte:

	if ( data.can_transport && owner == game->ActivePlayer )
	{
		// Transport:
		DrawNumber ( 31, 189, data.cargo, data.max_cargo, GraphicsData.gfx_hud );

		font->showText ( 55, 189, lngPack.i18n ( "Text~Hud~Cargo" ), LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

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
			if ( owner == game->ActivePlayer )
			{
				// Munition:
				DrawNumber ( 31, 189, data.ammo, data.max_ammo, GraphicsData.gfx_hud );

				font->showText ( 55, 189, lngPack.i18n ( "Text~Hud~AmmoShort" ), LATIN_SMALL_WHITE, GraphicsData.gfx_hud );
				DrawSymbol ( SAmmo, 88, 187, 70, data.ammo, data.max_ammo, GraphicsData.gfx_hud );
			}

			// Schüsse:
			DrawNumber ( 31, 212, data.shots, data.max_shots, GraphicsData.gfx_hud );

			font->showText ( 55, 212, lngPack.i18n ( "Text~Hud~Shots" ), LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

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
	dest.w = full.w;
	dest.h = full.h;

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

		font->showTextCentered ( x, y, sTmp , LATIN_SMALL_GREEN, sf );
	}
	else
		if ( value > maxvalue / 4 )
		{

			font->showTextCentered ( x, y, sTmp , LATIN_SMALL_YELLOW, sf );
		}
		else
		{

			font->showTextCentered ( x, y, sTmp , LATIN_SMALL_RED, sf );
		}
}

// Zeigt den Hilfebildschirm an:
void cVehicle::ShowHelp ( void )
{
#define DIALOG_W 640
#define DIALOG_H 480
#define BUTTON_W 150
#define BUTTON_H 29

	int LastMouseX = 0, LastMouseY = 0, LastB = 0, x, y, b;
	bool FertigPressed = false;
	bool ret = false;
	SDL_Rect rDialog = { SettingsData.iScreenW / 2 - DIALOG_W / 2, SettingsData.iScreenH / 2 - DIALOG_H / 2, DIALOG_W, DIALOG_H };
	SDL_Rect rDialogSrc = {0, 0, DIALOG_W, DIALOG_H};
	SDL_Rect rInfoTxt = {rDialog.x + 11, rDialog.y + 13, typ->info->w, typ->info->h};
	SDL_Rect rTxt = {rDialog.x + 349, rDialog.y + 66, 277, 181};
	SDL_Rect rTitle = {rDialog.x + 327, rDialog.y + 11, 160, 15};
	SDL_Rect rButton = { rDialog.x + 474, rDialog.y + 452, BUTTON_W, BUTTON_H };
	SDL_Surface *SfDialog;

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

	//draw button
	drawButton ( lngPack.i18n ( "Text~Button~Done" ), false, rButton.x, rButton.y, buffer );

	SHOW_SCREEN 	// Den Buffer anzeigen
	mouse->GetBack ( buffer );

	while ( 1 )
	{
		// Die Engine laufen lassen:
		game->engine->Run();
		game->HandleTimer();

		// Events holen:
		SDL_PumpEvents();

		// Die Maus machen:
		mouse->GetPos();
		b = mouse->GetMouseButton();
		x = mouse->x;
		y = mouse->y;

		if ( x != LastMouseX || y != LastMouseY )
		{
			mouse->draw ( true, screen );
		}

		// Fertig-Button:
		if ( x >= rButton.x && x < rButton.x + rButton.w  && y >= rButton.y && y < rButton.y + rButton.w )
		{
			if ( b && !FertigPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				drawButton ( lngPack.i18n ( "Text~Button~Done" ), true, rButton.x, rButton.y, buffer );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				FertigPressed = true;
			}
			else
				if ( !b && LastB )
				{
					return;
				}
		}
		else
			if ( FertigPressed )
			{
				drawButton ( lngPack.i18n ( "Text~Button~Done" ), false, rButton.x, rButton.y, buffer );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				FertigPressed = false;
			}

		LastMouseX = x;

		LastMouseY = y;
		LastB = b;
	}

	SDL_FreeSurface ( SfDialog );
}

// Malt große Symbole für das Info-Fenster:
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
	dest.w = scr.w;
	dest.h = scr.h;

	for ( i = 0;i < value;i++ )
	{
		if ( i == orgvalue )
		{
			SDL_Rect mark;
			dest.x += scr.w + 3;
			mark.x = dest.x - scr.w / 2;
			mark.y = dest.y;
			mark.w = 1;
			mark.h = dest.h;
			SDL_FillRect ( sf, &mark, 0xFC0000 );
		}

		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, sf, &dest );

		dest.x += offx;
	}
}

// Prüft, ob das Vehicle auf der Kachel an dem Offset fahren kann:
bool cVehicle::CanDrive ( int MapOff )
{
	int nr;

	if ( MapOff < 0 || MapOff >= game->map->size*game->map->size )
		return false;

	if ( ( IsBuilding && BuildRounds == 0 ) || ( IsClearing && ClearingRounds == 0 ) )
	{
		int off;
		// Kann nur 1 Feld im Umkreis befahren:
		off = PosX + PosY * game->map->size;

		if ( data.can_build == BUILD_BIG || ClearBig )
		{
			if ( ( MapOff < 0 ) || ( MapOff > game->map->size*MapOff > game->map->size ) ||
			        ( MapOff >= off - 1 - game->map->size && MapOff <= off + 2 - game->map->size ) ||
			        ( MapOff >= off - 1 && MapOff <= off + 2 ) ||
			        ( MapOff >= off - 1 + game->map->size && MapOff <= off + 2 + game->map->size ) ||
			        ( MapOff >= off - 1 + game->map->size*2 && MapOff <= off + 2 + game->map->size*2 ) )
				{}
			else
				return false;
		}
		else
		{
			if ( ( MapOff < 0 ) || ( MapOff > game->map->size*MapOff > game->map->size ) ||
			        ( MapOff >= off - 1 - game->map->size && MapOff <= off + 1 - game->map->size ) ||
			        ( MapOff >= off - 1 && MapOff <= off + 1 ) ||
			        ( MapOff >= off - 1 + game->map->size && MapOff <= off + 1 + game->map->size ) )
				{}
			else
				return false;
		}

		if ( game->map->GO[MapOff].reserviert || game->map->GO[MapOff].vehicle || ( game->map->GO[MapOff].top && !game->map->GO[MapOff].top->data.is_connector ) )
			return false;
	}

	nr = game->map->Kacheln[MapOff];

	switch ( data.can_drive )
	{

		case DRIVE_AIR:
			return true;

		case DRIVE_SEA:

			if ( !game->map->IsWater ( MapOff, true ) || ( game->map->GO[MapOff].base && ( game->map->GO[MapOff].base->data.is_platform || game->map->GO[MapOff].base->data.is_road ) ) )
				return false;

			return true;

		case DRIVE_LANDnSEA:
			if ( TerrainData.terrain[nr].blocked )
				return false;

			return true;

		case DRIVE_LAND:
			if ( TerrainData.terrain[nr].blocked || ( game->map->IsWater ( MapOff ) && ! ( game->map->GO[MapOff].base && ( game->map->GO[MapOff].base->data.is_bridge || game->map->GO[MapOff].base->data.is_platform || game->map->GO[MapOff].base->data.is_road ) ) ) )
				return false;

			return true;
	}

	return false;
}

// Liefert die X-Position des Vehicles auf dem Screen zurück:
int cVehicle::GetScreenPosX ( void )
{
	return 180 - ( ( int ) ( ( game->hud->OffX - OffX ) / ( 64.0 / game->hud->Zoom ) ) ) + game->hud->Zoom*PosX;
}

// Liefert die Y-Position des Vehicles auf dem Screen zurück:
int cVehicle::GetScreenPosY ( void )
{
	return 18 - ( ( int ) ( ( game->hud->OffY - OffY ) / ( 64.0 / game->hud->Zoom ) ) ) + game->hud->Zoom*PosY;
}

// Malt den Path des Vehicles:
void cVehicle::DrawPath ( void )
{
	int zoom, mx, my, sp, save;
	SDL_Rect dest, ndest;
	sWaypoint *wp;

	if ( !mjob || !mjob->waypoints || owner != game->ActivePlayer )
	{
		if ( !BuildPath || ( BandX == PosX && BandY == PosY ) || PlaceBand )
			return;

		zoom = game->hud->Zoom;

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
			dest.x = 180 - ( int ) ( game->hud->OffX / ( 64.0 / zoom ) ) + zoom * mx;
			dest.y = 18 - ( int ) ( game->hud->OffY / ( 64.0 / zoom ) ) + zoom * my;
			dest.w = zoom;
			dest.h = zoom;

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

		dest.x = 180 - ( int ) ( game->hud->OffX / ( 64.0 / zoom ) ) + zoom * mx;

		dest.y = 18 - ( int ) ( game->hud->OffY / ( 64.0 / zoom ) ) + zoom * my;
		dest.w = zoom;
		dest.h = zoom;
		SDL_BlitSurface ( OtherData.WayPointPfeileSpecial[sp][64-zoom], NULL, buffer, &dest );
		return;
	}

	sp = data.speed;

	if ( sp )
	{
		save = 0;
		sp += mjob->SavedSpeed;
	}
	else
		save = mjob->SavedSpeed;

	zoom = game->hud->Zoom;

	dest.x = 180 - ( int ) ( game->hud->OffX / ( 64.0 / zoom ) ) + zoom * PosX;

	dest.y = 18 - ( int ) ( game->hud->OffY / ( 64.0 / zoom ) ) + zoom * PosY;

	dest.w = zoom;

	dest.h = zoom;

	wp = mjob->waypoints;

	dest.x += mx = wp->X * zoom - mjob->waypoints->X * zoom;

	dest.y += my = wp->Y * zoom - mjob->waypoints->Y * zoom;

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
			mjob->DrawPfeil ( dest, &ndest, true );
			sp += data.max_speed + save;
			save = 0;
		}
		else
		{
			mjob->DrawPfeil ( dest, &ndest, false );
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

// Liefert einen String mit dem aktuellen Status zurück:
string cVehicle::GetStatusStr ( void )
{
	if ( autoMJob )
	{
		return lngPack.i18n ( "Text~Comp~Surveying" );
	}
	else
		if ( mjob )
		{
			return lngPack.i18n ( "Text~Comp~Moving" );
		}
		else
			if ( Wachposten )
			{
				return lngPack.i18n ( "Text~Comp~Sentry" );
			}
			else
				if ( IsBuilding )
				{
					if ( owner != game->ActivePlayer )
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
							sText += ( string ) owner->BuildingData[BuildingTyp].name + " (";
							sText += iToStr ( BuildRounds );
							sText += ")";

							if ( font->getTextWide ( sText ) > 126 )
							{
								sText = lngPack.i18n ( "Text~Comp~Producing" );
								sText += ":\n";
								sText += ( string ) owner->BuildingData[BuildingTyp].name + " (";
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
								if ( data.is_commando && owner == game->ActivePlayer )
								{
									string sTmp = lngPack.i18n ( "Text~Comp~Waits" ) + "\n";

									switch ( CommandoRank )
									{

										case 0:

										case 1:

										case 2:
											sTmp += lngPack.i18n ( "Text~Comp~Greenhorn" );
											break;

										case 3:

										case 4:

										case 5:
											sTmp += lngPack.i18n ( "Text~Comp~Veteran" );
											break;

										default: "invalid rank"; //dev messed up
									}

									switch ( CommandoRank )
									{

										case 0:
											sTmp += "";
											break;

										case 1:
											sTmp += " +1";
											break;

										case 2:
											sTmp += " +2";
											break;

										case 3:
											sTmp += " +3";
											break;

										case 4:
											sTmp += " +4";
											break;

										case 5:
											sTmp += " +5";
											break;

										default: ""; //dev messed up
									}

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

// Spielt den Soundstream am, der zu diesem Vehicle gehört:
int cVehicle::PlayStram ( void )
{
	if ( IsBuilding && BuildRounds )
	{
		return PlayFXLoop ( SoundData.SNDBuilding );
	}
	else
		if ( IsClearing && ClearingRounds )
		{
			return PlayFXLoop ( SoundData.SNDClearing );
		}
		else
			if ( game->map->IsWater ( PosX + PosY*game->map->size ) && data.can_drive != DRIVE_AIR )
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
	// Hier ist gleichzeitig der Moment um das Menü auszublenden:
	MenuActive = false;
	// Prüfen, ob ein Sound zu spielen ist:

	if ( this != game->SelectedVehicle )
		return;

	if ( data.can_drive == DRIVE_LAND || data.can_drive == DRIVE_LANDnSEA )
	{
		water = game->map->IsWater ( PosX + PosY * game->map->size ) && ! ( game->map->GO[PosX+PosY*game->map->size].base && ( game->map->GO[PosX+PosY*game->map->size].base->data.is_platform || game->map->GO[PosX+PosY*game->map->size].base->data.is_bridge || game->map->GO[PosX+PosY*game->map->size].base->data.is_road ) );
	}
	else
	{
		water = game->map->IsWater ( PosX + PosY * game->map->size && ! ( game->map->GO[PosX+PosY*game->map->size].base && ( game->map->GO[PosX+PosY*game->map->size].base->data.is_platform || game->map->GO[PosX+PosY*game->map->size].base->data.is_road ) ) );
	}

	StopFXLoop ( game->ObjectStream );

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
		game->ObjectStream = PlayFXLoop ( typ->DriveWater );
	}
	else
	{
		game->ObjectStream = PlayFXLoop ( typ->Drive );
	}
}

// Malt das Vehiclemenü:
void cVehicle::DrawMenu ( void )
{
	int nr = 0, SelMenu = -1, ExeNr = -1;
	static int LastNr = -1;
	SDL_Rect scr, dest;
	dest = GetMenuSize();
	dest.w = scr.w = 42;
	dest.h = scr.h = 21;
	Transfer = false;

	if ( moving || rotating )
		return;

	if ( mouse->GetMouseButton() && MouseOverMenu ( mouse->x, mouse->y ) )
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
		if ( SelMenu == nr )
			scr.y = 21;
		else
			scr.y = 0;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			AttackMode = true;
			game->hud->CheckScroll();
			MouseMoveCallback ( true );
			return;
		}

		scr.x = 588;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
	}

	// Bauen:
	if ( data.can_build && !IsBuilding )
	{
		if ( SelMenu == nr )
			scr.y = 21;
		else
			scr.y = 0;

		if ( ExeNr == nr )
		{
			if ( mjob )
			{
				mjob->finished = true;
				mjob = NULL;
				MoveJobActive = false;
				MenuActive = false;
				PlayFX ( SoundData.SNDObjectMenu );
				// Client only

				if ( game->engine->network && !game->engine->network->bServer )
				{
					SendIntBool(PosX + PosY * game->map->size, ( data.can_drive == DRIVE_AIR ), MSG_MJOB_STOP);
				}
			}

			MenuActive = false;

			PlayFX ( SoundData.SNDObjectMenu );
			ShowBuildMenu();
			return;
		}

		scr.x = 0;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
	}

	// Transfer:
	if ( ( data.can_transport == TRANS_METAL || data.can_transport == TRANS_OIL || data.can_transport == TRANS_GOLD ) && !IsBuilding && !IsClearing )
	{
		if ( SelMenu == nr )
			scr.y = 21;
		else
			scr.y = 0;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			Transfer = true;
			return;
		}

		scr.x = 42;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
	}

	// Auto
	if ( data.can_survey )
	{
		if ( ( autoMJob == NULL && SelMenu == nr ) || ( autoMJob != NULL && SelMenu != nr ) )
		{
			scr.y = 21;
		}
		else
		{
			scr.y = 0;
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

		scr.x = 168;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
	}

	// Stop:
	if ( mjob || ( IsBuilding && BuildRounds ) || ( IsClearing && ClearingRounds ) )
	{
		if ( SelMenu == nr )
			scr.y = 21;
		else
			scr.y = 0;

		if ( ExeNr == nr )
		{
			if ( mjob )
			{
				mjob->finished = true;
				mjob = NULL;
				MoveJobActive = false;
				MenuActive = false;
				PlayFX ( SoundData.SNDObjectMenu );
				// Client only

				if ( game->engine->network && !game->engine->network->bServer )
				{
					SendIntBool(PosX + PosY * game->map->size, ( data.can_drive == DRIVE_AIR ), MSG_MJOB_STOP);
				}
			}
			else
				if ( IsBuilding )
				{
					MenuActive = false;
					PlayFX ( SoundData.SNDObjectMenu );
					IsBuilding = false;
					BuildPath = false;

					if ( data.can_build == BUILD_BIG )
					{
						game->map->GO[BandX+BandY*game->map->size].vehicle = NULL;
						game->map->GO[BandX+1+BandY*game->map->size].vehicle = NULL;
						game->map->GO[BandX+1+ ( BandY+1 ) *game->map->size].vehicle = NULL;
						game->map->GO[BandX+ ( BandY+1 ) *game->map->size].vehicle = NULL;
						game->map->GO[BuildBigSavedPos].vehicle = this;
						PosX = BuildBigSavedPos % game->map->size;
						PosY = BuildBigSavedPos / game->map->size;
					}

					if ( game->SelectedVehicle && game->SelectedVehicle == this )
					{
						StopFXLoop ( game->ObjectStream );
						game->ObjectStream = PlayStram();
					}
				}
				else
				{
					MenuActive = false;
					PlayFX ( SoundData.SNDObjectMenu );
					IsClearing = false;

					if ( ClearBig )
					{
						game->map->GO[BandX+1+BandY*game->map->size].vehicle = NULL;
						game->map->GO[BandX+1+ ( BandY+1 ) *game->map->size].vehicle = NULL;
						game->map->GO[BandX+ ( BandY+1 ) *game->map->size].vehicle = NULL;
					}

					if ( game->SelectedVehicle && game->SelectedVehicle == this )
					{
						StopFXLoop ( game->ObjectStream );
						game->ObjectStream = PlayStram();
					}
				}

			return;
		}

		scr.x = 210;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
	}

	// Entfernen:
	if ( data.can_clear && game->map->GO[PosX+PosY*game->map->size].base && !game->map->GO[PosX+PosY*game->map->size].base->owner && !IsClearing )
	{
		if ( SelMenu == nr )
			scr.y = 21;
		else
			scr.y = 0;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			IsClearing = true;
			ClearingRounds = game->map->GO[PosX+PosY*game->map->size].base->DirtValue / 4 + 1;
			ClearBig = game->map->GO[PosX+PosY*game->map->size].base->data.is_big;
			// Den Clearing Sound machen:
			StopFXLoop ( game->ObjectStream );
			game->ObjectStream = PlayStram();
			// Umsetzen, wenn es großer Dreck ist:

			if ( ClearBig )
			{
				PosX = game->map->GO[PosX+PosY*game->map->size].base->PosX;
				PosY = game->map->GO[PosX+PosY*game->map->size].base->PosY;
				BandX = PosX;
				BandY = PosY;
				game->map->GO[PosX+PosY*game->map->size].vehicle = this;
				game->map->GO[PosX+1+PosY*game->map->size].vehicle = this;
				game->map->GO[PosX+1+ ( PosY+1 ) *game->map->size].vehicle = this;
				game->map->GO[PosX+ ( PosY+1 ) *game->map->size].vehicle = this;
			}

			return;
		}

		scr.x = 252;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
	}

	// Wachposten:
	if ( Wachposten || data.can_attack )
	{
		if ( SelMenu == nr )
			scr.y = 21;
		else
			scr.y = 0;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			Wachposten = !Wachposten;
			Wachwechsel();
			return;
		}

		scr.x = 84;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
	}

	// Aktivieren/Laden:
	if ( data.can_transport == TRANS_VEHICLES || data.can_transport == TRANS_MEN )
	{
		// Aktivieren:
		if ( SelMenu == nr )
			scr.y = 21;
		else
			scr.y = 0;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			ShowStorage();
			return;
		}

		scr.x = 462;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
		// Laden:

		if ( SelMenu == nr )
			scr.y = 21;
		else
			scr.y = 0;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			LoadActive = true;
			return;
		}

		scr.x = 420;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
	}

	// Aufaden:
	if ( data.can_reload && data.cargo >= 2 )
	{
		if ( SelMenu == nr )
			scr.y = 21;
		else
			scr.y = 0;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			MuniActive = true;
			return;
		}

		scr.x = 546;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
	}

	// Reparatur:
	if ( data.can_repair && data.cargo >= 2 )
	{
		if ( SelMenu == nr )
			scr.y = 21;
		else
			scr.y = 0;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			RepairActive = true;
			return;
		}

		scr.x = 294;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
	}

	// Minen legen:
	if ( data.can_lay_mines && data.cargo > 0 )
	{
		if ( SelMenu == nr )
			scr.y = 21;
		else
			scr.y = 0;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			LayMines = !LayMines;
			ClearMines = false;
			LayMine();
			return;
		}

		scr.x = 630;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
	}

	// Minen sammeln:
	if ( data.can_lay_mines && data.cargo < data.max_cargo )
	{
		if ( SelMenu == nr )
			scr.y = 21;
		else
			scr.y = 0;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			ClearMines = !ClearMines;
			LayMines = false;
			ClearMine();
			return;
		}

		scr.x = 252;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
	}

	// Commando-Funktionen:
	if ( data.is_commando && data.shots )
	{
		// Sabotage:
		if ( SelMenu == nr )
			scr.y = 21;
		else
			scr.y = 0;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			DisableActive = true;
			return;
		}

		scr.x = 336;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
		// Stehlen:

		if ( SelMenu == nr )
			scr.y = 21;
		else
			scr.y = 0;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			StealActive = true;
			return;
		}

		scr.x = 378;

		SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
		dest.y += 22;
		nr++;
	}

	// Info:
	if ( SelMenu == nr )
		scr.y = 21;
	else
		scr.y = 0;

	if ( ExeNr == nr )
	{
		MenuActive = false;
		PlayFX ( SoundData.SNDObjectMenu );
		ShowHelp();
		return;
	}

	scr.x = 840;

	SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
	dest.y += 22;
	nr++;
	// Fertig:

	if ( SelMenu == nr )
		scr.y = 21;
	else
		scr.y = 0;

	if ( ExeNr == nr )
	{
		MenuActive = false;
		PlayFX ( SoundData.SNDObjectMenu );
		return;
	}

	scr.x = 126;

	SDL_BlitSurface ( GraphicsData.gfx_object_menu, &scr, buffer, &dest );
}

// Liefert die Anzahl der Menüpunkte:
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

	if ( mjob || ( IsBuilding && BuildRounds ) || ( IsClearing && ClearingRounds ) )
		nr++;

	if ( data.can_clear && game->map->GO[PosX+PosY*game->map->size].base && !game->map->GO[PosX+PosY*game->map->size].base->owner && !IsClearing )
		nr++;

	if ( Wachposten || data.can_attack )
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

// Liefert die Größe des Menüs und Position zurück:
SDL_Rect cVehicle::GetMenuSize ( void )
{
	SDL_Rect dest;
	int i, size;
	dest.x = GetScreenPosX();
	dest.y = GetScreenPosY();
	dest.h = i = GetMenuPointAnz() * 22;
	dest.w = 42;
	size = game->hud->Zoom;

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

// Liefert true zurück, wenn die Koordinaten in dem Menübereich liegen:
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

	if ( game->SelectedVehicle == this )
		ShowDetails();
}

// Malt die Munitionsanzeige über das Fahrzeug:
void cVehicle::DrawMunBar ( void )
{
	SDL_Rect r1, r2;
	r2.x = ( r1.x = GetScreenPosX() ) + 1;
	r1.w = game->hud->Zoom;
	r2.h = ( r1.h = game->hud->Zoom / 6 ) - 2;

	if ( r1.h < 2 )
		r2.h = 1;

	r2.y = ( int ) ( r1.y = game->hud->Zoom - r1.h + GetScreenPosY() ) + 1;

	r2.w = ( int ) ( ( ( float ) ( r1.w - 2 ) / data.max_ammo ) * data.ammo );

	if ( data.ammo > data.max_ammo / 2 )
	{
		SDL_FillRect ( buffer, &r1, 0 );
		SDL_FillRect ( buffer, &r2, 0x04AE04 );
	}
	else
		if ( data.ammo > data.max_ammo / 4 )
		{
			SDL_FillRect ( buffer, &r1, 0 );
			SDL_FillRect ( buffer, &r2, 0xDBDE00 );
		}
		else
		{
			SDL_FillRect ( buffer, &r1, 0 );
			SDL_FillRect ( buffer, &r2, 0xE60000 );
		}
}

// Malt die Trefferanzeige über das Fahrzeug:
void cVehicle::DrawHelthBar ( void )
{
	SDL_Rect r1, r2;
	r2.x = ( r1.x = GetScreenPosX() ) + 1;
	r1.w = game->hud->Zoom;
	r2.h = ( r1.h = game->hud->Zoom / 6 ) - 2;

	if ( r1.h < 2 )
		r2.h = 1;

	r2.y = ( int ) ( r1.y = GetScreenPosY() ) + 1;

	r2.w = ( int ) ( ( ( float ) ( r1.w - 2 ) / data.max_hit_points ) * data.hit_points );

	if ( data.hit_points > data.max_hit_points / 2 )
	{
		SDL_FillRect ( buffer, &r1, 0 );
		SDL_FillRect ( buffer, &r2, 0x04AE04 );
	}
	else
		if ( data.hit_points > data.max_hit_points / 4 )
		{
			SDL_FillRect ( buffer, &r1, 0 );
			SDL_FillRect ( buffer, &r2, 0xDBDE00 );
		}
		else
		{
			SDL_FillRect ( buffer, &r1, 0 );
			SDL_FillRect ( buffer, &r2, 0xE60000 );
		}
}

// Zentriert auf dieses Vehicle:
void cVehicle::Center ( void )
{
	game->hud->OffX = PosX * 64 - ( ( int ) ( ( ( float ) 224 / game->hud->Zoom ) * 64 ) ) + 32;
	game->hud->OffY = PosY * 64 - ( ( int ) ( ( ( float ) 224 / game->hud->Zoom ) * 64 ) ) + 32;
	game->fDrawMap = true;
	game->hud->DoScroll ( 0 );
}

// Prüft, ob das Vehicle das Objekt angreifen kann:
bool cVehicle::CanAttackObject ( int off, bool override )
{
	cVehicle *v = NULL;
	cBuilding *b = NULL;

	if ( !data.can_attack )
		return false;

	if ( !data.shots )
		return false;

	if ( Attacking )
		return false;

	if ( off < 0 )
		return false;

	if ( !owner->ScanMap[off] )
		return false;

	if ( !IsInRange ( off ) )
		return false;

	if ( data.can_attack != ATTACK_AIR )
	{
		v = game->map->GO[off].vehicle;

		if ( game->map->GO[off].top )
		{
			b = game->map->GO[off].top;
		}
		else
			if ( game->map->GO[off].base && game->map->GO[off].base->owner && game->map->GO[off].base->detected )
			{
				b = game->map->GO[off].base;
			}
	}
	else
	{
		v = game->map->GO[off].plane;
	}

	if ( v && v->data.is_stealth_sea && data.can_attack != ATTACK_SUB_LAND )
		return false;

	if ( override )
		return true;

	if ( v && v->detected )
	{
		if ( v->owner == owner )
			return false;
	}
	else
		if ( b )
		{
			if ( b->owner == owner )
				return false;
		}
		else
		{
			return false;
		}

	return true;
}

// Prüft, ob das Ziel innerhalb der Reichweite liegt:
bool cVehicle::IsInRange ( int off )
{
	int x, y;
	x = off % game->map->size;
	y = off / game->map->size;
	x -= PosX;
	y -= PosY;

	if ( sqrt ( ( double ) ( x*x + y*y ) ) <= data.range )
	{
		return true;
	}

	return false;
}

// Malt den Attack-Cursor:
void cVehicle::DrawAttackCursor ( struct sGameObjects *go, int can_attack )
{
	SDL_Rect r;
	int wp, wc, t;
	cVehicle *v = NULL;
	cBuilding *b = NULL;

	if ( can_attack == ATTACK_AIRnLAND )
	{
		if ( go->top )
		{
			b = go->top;
		}
		else
			if ( go->vehicle )
			{
				v = go->vehicle;
			}
			else
				if ( go->base && go->base->owner )
				{
					b = go->base;
				}
				else
					if ( go->plane )
					{
						v = go->plane;
					}

		if ( !v && !b )
		{
			r.x = 1;
			r.y = 29;
			r.h = 3;
			r.w = 35;
			SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0 );
			return;
		}
	}
	else
	{
		if ( can_attack == ATTACK_LAND || can_attack == ATTACK_SUB_LAND )
		{
			if ( !go->vehicle && !go->base && !go->top )
			{
				r.x = 1;
				r.y = 29;
				r.h = 3;
				r.w = 35;
				SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0 );
				return;
			}

			if ( go->top )
			{
				b = go->top;
			}
			else
				if ( go->vehicle )
				{
					v = go->vehicle;
				}
				else
					if ( go->base && go->base->owner )
					{
						b = go->base;
					}

			if ( v && v->data.is_stealth_sea && can_attack != ATTACK_SUB_LAND )
				v = NULL;
		}

		if ( can_attack == ATTACK_AIR )
		{
			if ( !go->plane )
			{
				r.x = 1;
				r.y = 29;
				r.h = 3;
				r.w = 35;
				SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0 );
				return;
			}

			v = go->plane;
		}
	}

	if ( ( v && v == game->SelectedVehicle ) || ( b && b == game->SelectedBuilding ) || ( !v && !b ) )
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

// Berechnet die Hitpoints nach einem Treffer:
int cVehicle::CalcHelth ( int damage )
{
	int hp;
	hp = data.hit_points + data.armor - damage;

	if ( hp > data.hit_points )
		hp = data.hit_points;

	if ( hp < 0 )
		return 0;

	return hp;
}

// Struktur für die Build-List:

struct sBuildStruct
{
	SDL_Surface *sf;
	int id;
};

// Zeigt das Build-Menü an:
void cVehicle::ShowBuildMenu ( void )
{
	int LastMouseX = 0, LastMouseY = 0, LastB = 0, x, y, b, i;
	SDL_Rect scr, dest;
	bool AbbruchPressed = false;
	bool FertigPressed = false;
	bool PfadPressed = false;
	bool Beschreibung = SettingsData.bShowDescription;
	bool DownPressed = false;
	bool UpPressed = false;
	cList<sBuildStruct*> *images;
	int selected = 0, offset = 0, BuildSpeed = 1;
	int iTurboBuildCosts[3];	//costs for the 3 turbo build steps
	int iTurboBuildRounds[3];	// needed rounds for the 3 turbo build steps
								// 0 rounds, means not available
#define BUTTON__W 77
#define BUTTON__H 23

	SDL_Rect rTxtDescription = {141, 266, 150, 13};
	SDL_Rect rTitle = {330, 11, 154, 13};
	SDL_Rect rBtnDone = {387, 452, BUTTON__W, BUTTON__H};
	SDL_Rect rBtnCancel = {300, 452, BUTTON__W, BUTTON__H};
	SDL_Rect rBtnPath = {339, 428, BUTTON__W, BUTTON__H};

	//IMPORTANT: just for reference. If you change these coordinates you'll have to change DrawBuildButtons, too! -- beko
	SDL_Rect rBtnSpeed1 = {292, 345, BUTTON__W, BUTTON__H}; //buildspeed * 1
	SDL_Rect rBtnSpeed2 = {292, 370, BUTTON__W, BUTTON__H}; //buildspeed * 2
	SDL_Rect rBtnSpeed3 = {292, 395, BUTTON__W, BUTTON__H}; //buildspeed * 4

	BandX = PosX;
	BandY = PosY;
	mouse->SetCursor ( CHand );
	mouse->draw ( false, buffer );
	SDL_BlitSurface ( GraphicsData.gfx_build_screen, NULL, buffer, NULL );

	if ( data.can_build != BUILD_BIG )
	{
		drawButton ( lngPack.i18n ( "Text~Button~Path" ), false, rBtnPath.x, rBtnPath.y, buffer );
	}

	drawButton ( lngPack.i18n ( "Text~Button~Done" ), false, rBtnDone.x, rBtnDone.y, buffer );

	drawButton ( lngPack.i18n ( "Text~Button~Cancel" ), false, rBtnCancel.x, rBtnCancel.y, buffer );


	font->showTextCentered ( rTxtDescription.x + rTxtDescription.w / 2, rTxtDescription.y, lngPack.i18n ( "Text~Comp~Description" ) );

	font->showTextCentered ( rTitle.x + rTitle.w / 2, rTitle.y, lngPack.i18n ( "Text~Title~Build" ) );


	// Den Haken:

	if ( Beschreibung )
	{
		dest.x = scr.x = 291;
		dest.y = scr.y = 264;
		dest.w = scr.w = 17;
		dest.h = scr.h = 17;
		SDL_BlitSurface ( GraphicsData.gfx_build_screen, &scr, buffer, &dest );
	}
	else
	{
		scr.x = 393;
		scr.y = 46;
		dest.x = 291;
		dest.y = 264;
		dest.w = scr.w = 18;
		dest.h = scr.h = 17;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
	}

	// Die Images erstellen:
	images = new cList<sBuildStruct*>;

	for ( i = 0;i < UnitsData.building_anz;i++ )
	{
		sBuildStruct *n;
		SDL_Surface *sf, *sf2;

		if ( UnitsData.building[i].data.is_expl_mine )
			continue;

		if ( data.can_build == BUILD_BIG && !UnitsData.building[i].data.is_big )
			continue;

		if ( data.can_build == BUILD_SMALL && UnitsData.building[i].data.is_big )
			continue;

		if ( !game->AlienTech && UnitsData.building[i].data.is_alien )
			continue;

		ScaleSurface ( UnitsData.building[i].img_org, &sf2, 32 );

		SDL_SetColorKey ( sf2, SDL_SRCCOLORKEY, 0xFFFFFF );

		sf = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, 32, 32, 32, 0, 0, 0, 0 );

		SDL_SetColorKey ( sf, SDL_SRCCOLORKEY, 0xFF00FF );

		SDL_BlitSurface ( game->ActivePlayer->color, NULL, sf, NULL );

		SDL_BlitSurface ( sf2, NULL, sf, NULL );

		SDL_FreeSurface ( sf2 );

		n = new sBuildStruct;

		n->sf = sf;

		n->id = i;

		images->Add ( n );
	}

	ShowBuildList ( images, selected, offset, Beschreibung, &BuildSpeed, iTurboBuildCosts, iTurboBuildRounds );

	DrawBuildButtons ( BuildSpeed );

	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );

	mouse->MoveCallback = false;

	while ( 1 )
	{
		if ( game->SelectedVehicle == NULL )
			break;

		// Die Engine laufen lassen:
		game->engine->Run();

		game->HandleTimer();

		// Events holen:
		SDL_PumpEvents();

		// Die Maus machen:
		mouse->GetPos();

		b = mouse->GetMouseButton();

		x = mouse->x;

		y = mouse->y;

		if ( x != LastMouseX || y != LastMouseY )
		{
			mouse->draw ( true, screen );
		}

		// Down-Button:
		if ( x >= 491 && x < 491 + 18 && y >= 440 && y < 440 + 17 && b && !DownPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x = 249;
			scr.y = 151;
			dest.w = scr.w = 18;
			dest.h = scr.h = 17;
			dest.x = 491;
			dest.y = 440;

			offset += 9;

			if ( offset > images->iCount - 9 )
			{
				offset = images->iCount - 9;
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
				dest.w = scr.w = 18;
				dest.h = scr.h = 17;
				dest.x = 491;
				dest.y = 440;
				SDL_BlitSurface ( GraphicsData.gfx_build_screen, &scr, buffer, &dest );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				DownPressed = false;
			}

		// Up-Button:
		if ( x >= 471 && x < 471 + 18 && y >= 440 && y < 440 + 17 && b && !UpPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x = 230;
			scr.y = 151;
			dest.w = scr.w = 18;
			dest.h = scr.h = 17;
			dest.x = 471;
			dest.y = 440;

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
				dest.w = scr.w = 18;
				dest.h = scr.h = 17;
				dest.x = 471;
				dest.y = 440;
				SDL_BlitSurface ( GraphicsData.gfx_build_screen, &scr, buffer, &dest );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				UpPressed = false;
			}

		// Abbruch-Button:
		if ( x >= rBtnCancel.x && x < rBtnCancel.x + rBtnCancel.w && y >= rBtnCancel.y && y < rBtnCancel.y + rBtnCancel.h )
		{
			if ( b && !AbbruchPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				drawButton ( lngPack.i18n ( "Text~Button~Cancel" ), true, rBtnCancel.x, rBtnCancel.y, buffer );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				AbbruchPressed = true;
			}
			else
				if ( !b && LastB )
				{
					break;
				}
		}
		else
			if ( AbbruchPressed )
			{
				drawButton ( lngPack.i18n ( "Text~Button~Cancel" ), false, rBtnCancel.x, rBtnCancel.y, buffer );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				AbbruchPressed = false;
			}

		// Fertig-Button:
		if ( x >= rBtnDone.x && x < rBtnDone.x + rBtnDone.w && y >= rBtnDone.y && y < rBtnDone.y + rBtnDone.h )
		{
			if ( b && !FertigPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				drawButton ( lngPack.i18n ( "Text~Button~Done" ), false, rBtnDone.x, rBtnDone.y, buffer );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				FertigPressed = true;
			}
			else
				if ( !b && LastB )
				{
					if ( BuildSpeed < 0 )
						break;

					if ( game->map->GO[PosX+PosY*game->map->size].base && !game->map->GO[PosX+PosY*game->map->size].base->owner )
						break;

					BuildingTyp = images->Items[selected]->id;

					if ( game->map->GO[PosX+PosY*game->map->size].base && ( game->map->GO[PosX+PosY*game->map->size].base->data.is_platform || game->map->GO[PosX+PosY*game->map->size].base->data.is_bridge ) && ( UnitsData.building[BuildingTyp].data.is_base && !UnitsData.building[BuildingTyp].data.is_road ) )
						break;

					if ( ( !game->map->GO[PosX+PosY*game->map->size].base || !game->map->GO[PosX+PosY*game->map->size].base->data.is_platform ) && !UnitsData.building[BuildingTyp].data.is_connector )
					{
						if ( game->map->IsWater ( PosX + PosY*game->map->size ) )
						{
							if ( !UnitsData.building[BuildingTyp].data.build_on_water )
								break;
						}
						else
							if ( UnitsData.building[BuildingTyp].data.build_on_water && ! ( UnitsData.building[BuildingTyp].data.is_bridge || UnitsData.building[BuildingTyp].data.is_platform ) )
								break;

						if ( TerrainData.terrain[game->map->Kacheln[PosX+PosY*game->map->size]].coast )
						{
							if ( !UnitsData.building[BuildingTyp].data.is_bridge && !UnitsData.building[BuildingTyp].data.is_platform )
								break;
						}
						else
							if ( !game->map->IsWater ( PosX + PosY*game->map->size ) && ( UnitsData.building[BuildingTyp].data.is_bridge || UnitsData.building[BuildingTyp].data.is_platform ) )
								break;
					}

					BuildRounds = iTurboBuildRounds[BuildSpeed];

					BuildCosts  = iTurboBuildCosts[BuildSpeed];

					if ( data.can_build != BUILD_BIG )
					{
						IsBuilding = true;
						// Den Building Sound machen:
						StopFXLoop ( game->ObjectStream );
						game->ObjectStream = PlayStram();
					}
					else
					{
						PlaceBand = true;
						BuildBigSavedPos = PosX + PosY * game->map->size;

						if ( !game->map->IsWater ( PosX + PosY*game->map->size ) )
						{
							ShowBigBeton = true;
							BigBetonAlpha = 10;
						}
						else
						{
							ShowBigBeton = false;
							BigBetonAlpha = 255;
						}

						FindNextband();
					}

					BuildRoundsStart = BuildRounds;

					break;
				}
		}
		else
			if ( FertigPressed )
			{
				drawButton ( lngPack.i18n ( "Text~Button~Done" ), false, rBtnDone.x, rBtnDone.y, buffer );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				FertigPressed = false;
			}

		// Pfad-Button:
		if ( data.can_build != BUILD_BIG && x >= rBtnPath.x && x < rBtnPath.x + rBtnPath.w && y >= rBtnPath.y && y < rBtnPath.y + rBtnPath.h )
		{
			if ( b && !PfadPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				drawButton ( lngPack.i18n ( "Text~Button~Path" ), true, rBtnPath.x, rBtnPath.y, buffer );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				PfadPressed = true;
			}
			else
				if ( !b && LastB )
				{
					if ( BuildSpeed < 0 )
						break;

					if ( game->map->GO[PosX+PosY*game->map->size].base && !game->map->GO[PosX+PosY*game->map->size].base->owner )
						break;

					BuildingTyp = images->Items[selected]->id;

					if ( game->map->GO[PosX+PosY*game->map->size].base && ( game->map->GO[PosX+PosY*game->map->size].base->data.is_platform || game->map->GO[PosX+PosY*game->map->size].base->data.is_bridge ) && ( UnitsData.building[BuildingTyp].data.is_base && !UnitsData.building[BuildingTyp].data.is_road ) )
						break;

					if ( ( !game->map->GO[PosX+PosY*game->map->size].base || !game->map->GO[PosX+PosY*game->map->size].base->data.is_platform ) && !UnitsData.building[BuildingTyp].data.is_connector )
					{
						if ( game->map->IsWater ( PosX + PosY*game->map->size ) )
						{
							if ( !UnitsData.building[BuildingTyp].data.build_on_water )
								break;
						}
						else
							if ( UnitsData.building[BuildingTyp].data.build_on_water && ! ( UnitsData.building[BuildingTyp].data.is_bridge || UnitsData.building[BuildingTyp].data.is_platform ) )
								break;

						if ( TerrainData.terrain[game->map->Kacheln[PosX+PosY*game->map->size]].coast )
						{
							if ( !UnitsData.building[BuildingTyp].data.is_bridge && !UnitsData.building[BuildingTyp].data.is_platform )
								break;
						}
						else
							if ( !game->map->IsWater ( PosX + PosY*game->map->size ) && ( UnitsData.building[BuildingTyp].data.is_bridge || UnitsData.building[BuildingTyp].data.is_platform ) )
								break;
					}

					BuildRounds = iTurboBuildRounds[BuildSpeed];

					BuildCosts  = iTurboBuildCosts[BuildSpeed];

					BuildRoundsStart = BuildRounds;
					BuildCostsStart = BuildCosts;
					PlaceBand = true;
					break;
				}
		}
		else
			if ( PfadPressed )
			{
				drawButton ( lngPack.i18n ( "Text~Button~Path" ), false, rBtnPath.x, rBtnPath.y, buffer );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				PfadPressed = false;
			}

		// Beschreibung Haken:
		if ( x >= 292 && x < 292 + 16 && y >= 265 && y < 265 + 15 && b && !LastB )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			Beschreibung = !Beschreibung;
			SettingsData.bShowDescription = Beschreibung;

			if ( Beschreibung )
			{
				dest.x = scr.x = 291;
				dest.y = scr.y = 264;
				dest.w = scr.w = 17;
				dest.h = scr.h = 17;
				SDL_BlitSurface ( GraphicsData.gfx_build_screen, &scr, buffer, &dest );
			}
			else
			{
				scr.x = 393;
				scr.y = 46;
				dest.x = 291;
				dest.y = 264;
				dest.w = scr.w = 18;
				dest.h = scr.h = 17;
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
		if ( x >= 490 && x < 490 + 70 && y >= 60 && y < 60 + 368 && b && !LastB )
		{
			int nr;
			nr = ( y - 60 ) / ( 32 + 10 );

			if ( images->iCount < 9 )
			{
				if ( nr >= images->iCount )
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

	// Alles Images löschen:
	for ( int i = 0;i < images->iCount; i++ )
	{
		sBuildStruct *ptr;
		ptr = images->Items[i];
		SDL_FreeSurface ( ptr->sf );
		delete ptr;
		images->Delete ( 0 );
	}

	delete images;

	mouse->MoveCallback = true;
}

// Zeigt die Liste mit den Images an:
void cVehicle::ShowBuildList ( cList<sBuildStruct*> *list, int selected, int offset, bool beschreibung, int *buildspeed, int *iTurboBuildCosts, int *iTurboBuildRounds )
{
	sBuildStruct *ptr;
	SDL_Rect dest, scr, text = { 530, 70, 80, 16};
	int i;
	scr.x = 479;
	scr.y = 52;
	scr.w = 150;
	scr.h = 378;
	SDL_BlitSurface ( GraphicsData.gfx_build_screen, &scr, buffer, &scr );
	scr.x = 373;
	scr.y = 344;
	scr.w = 77;
	scr.h = 72;
	SDL_BlitSurface ( GraphicsData.gfx_build_screen, &scr, buffer, &scr );
	scr.x = 0;
	scr.y = 0;
	scr.w = 32;
	scr.h = 32;
	dest.x = 490;
	dest.y = 58;
	dest.w = 32;
	dest.h = 32;

	for ( i = offset;i < list->iCount;i++ )
	{
		if ( i >= offset + 9 )
			break;

		// Das Bild malen:
		ptr = list->Items[i];

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
			tmp.x = 11;
			tmp.y = 13;
			tmp.w = UnitsData.building[ptr->id].info->w;
			tmp.h = UnitsData.building[ptr->id].info->h;
			SDL_BlitSurface ( UnitsData.building[ptr->id].info, NULL, buffer, &tmp );
			// Ggf die Beschreibung ausgeben:

			if ( beschreibung )
			{
				tmp.x += 10;
				tmp.y += 10;
				tmp.w -= 20;
				tmp.h -= 20;

				font->showTextAsBlock ( tmp, UnitsData.building[ptr->id].text );
			}

			// Die Details anzeigen:
			{
				cBuilding *tb;
				tmp.x = 11;
				tmp.y = 290;
				tmp.w = 260;
				tmp.h = 176;
				SDL_BlitSurface ( GraphicsData.gfx_build_screen, &tmp, buffer, &tmp );
				tb = new cBuilding ( UnitsData.building + ptr->id, game->ActivePlayer, NULL );
				tb->ShowBigDetails();
				delete tb;
			}

			// calculate building time and costs

			//TODO: check, if the algorithm works also correct, if the building costs are reduced by research
			iTurboBuildRounds[0] = 0;

			iTurboBuildRounds[1] = 0;

			iTurboBuildRounds[2] = 0;

			//prevent division by zero
			if ( data.iNeeds_Metal == 0 )
				data.iNeeds_Metal = 1;

			//step 1x
			if ( data.cargo >= owner->BuildingData[ptr->id].iBuilt_Costs )
			{
				if ( *buildspeed == -1 )
				{
					*buildspeed = 0;
				}

				iTurboBuildCosts[0] = owner->BuildingData[ptr->id].iBuilt_Costs;

				iTurboBuildRounds[0] = iTurboBuildCosts[0] / data.iNeeds_Metal;
			}

			//step 2x
			if ( ( iTurboBuildRounds[0] > 1 ) && ( iTurboBuildCosts[0] + 4 <= owner->BuildingData[ptr->id].iBuilt_Costs_Max ) && ( data.cargo >= iTurboBuildCosts[0] + 4 ) )
			{
				iTurboBuildRounds[1] = iTurboBuildRounds[0];
				iTurboBuildCosts[1] = iTurboBuildCosts[0];

				while ( ( data.cargo >= iTurboBuildCosts[1] + 4 ) && ( iTurboBuildRounds[1] > 1 ) )
				{
					iTurboBuildRounds[1]--;
					iTurboBuildCosts[1] += 4;

					if ( iTurboBuildCosts[1] + 4 > 2*iTurboBuildCosts[0] )
						break;

					if ( iTurboBuildCosts[1] + 4 > owner->BuildingData[ptr->id].iBuilt_Costs_Max )
						break;
				}
			}

			//step 4x
			if ( ( iTurboBuildRounds[1] > 1 ) && ( iTurboBuildCosts[1] + 8 <= owner->BuildingData[ptr->id].iBuilt_Costs_Max ) && ( data.cargo >= iTurboBuildCosts[1] + 8 ) )
			{
				iTurboBuildRounds[2] = iTurboBuildRounds[1];
				iTurboBuildCosts[2] = iTurboBuildCosts[1];

				while ( ( data.cargo >= iTurboBuildCosts[2] + 8 ) && ( iTurboBuildRounds[2] > 1 ) )
				{
					iTurboBuildRounds[2]--;
					iTurboBuildCosts[2] += 8;

					if ( iTurboBuildCosts[2] + 8 > owner->BuildingData[ptr->id].iBuilt_Costs_Max )
						break;
				}
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



			font->showTextCentered ( 389, 350, iToStr ( owner->BuildingData[ptr->id].iBuilt_Costs / data.iNeeds_Metal ) );



			font->showTextCentered ( 429, 350, iToStr ( owner->BuildingData[ptr->id].iBuilt_Costs ) );

			if ( iTurboBuildRounds[1] > 0 )
			{





				font->showTextCentered ( 389, 375, iToStr ( iTurboBuildRounds[1] ) );
				font->showTextCentered ( 429, 375, iToStr ( iTurboBuildCosts[1] ) );
			}

			if ( iTurboBuildRounds[2] > 0 )
			{




				font->showTextCentered ( 389, 400, iToStr ( iTurboBuildRounds[2] ) );
				font->showTextCentered ( 429, 400, iToStr ( iTurboBuildCosts[2] ) );
			}
		}

		// Text ausgeben:
		
		string sTmp = UnitsData.building[ptr->id].data.name;
		
		if ( font->getTextWide ( sTmp, LATIN_SMALL_WHITE ) > text.w )
		{
			text.y -= font->getFontHeight(LATIN_SMALL_WHITE) / 2;
			font->showTextAsBlock ( text, sTmp, LATIN_SMALL_WHITE);
			text.y += font->getFontHeight(LATIN_SMALL_WHITE) / 2;
		}
		else
		{
			font->showText ( text, sTmp, LATIN_SMALL_WHITE);
		}
		
		
		font->showTextCentered ( 616, text.y, iToStr ( owner->BuildingData[ptr->id].iBuilt_Costs ), LATIN_SMALL_WHITE );

		text.y += 32 + 10;
		dest.y += 32 + 10;
	}
}

void cVehicle::DrawBuildButtons ( int speed )
{
	SDL_Rect rBtnSpeed1 = {292, 345, BUTTON__W, BUTTON__H};
	SDL_Rect rBtnSpeed2 = {292, 370, BUTTON__W, BUTTON__H};
	SDL_Rect rBtnSpeed3 = {292, 395, BUTTON__W, BUTTON__H};

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

// Findet die nächste passende Position für das Band:
void cVehicle::FindNextband ( void )
{
	bool pos[4];
	int x, y, gms;
	gms = game->map->size;
	mouse->GetKachel ( &x, &y );

//#define CHECK_BAND(a,b) (PosX a>=0&&PosX a<gms&&PosY b>=0&&PosY b<gms&&!game->map->GO[PosX a+(PosY b)*gms].vehicle&&!game->map->GO[PosX a+(PosY b)*gms].reserviert&&!(game->map->GO[PosX a+(PosY b)*gms].top&&!game->map->GO[PosX a+(PosY b)*gms].top->data.is_connector)&&!(game->map->GO[PosX a+(PosY b)*gms].base&&game->map->GO[PosX a+(PosY b)*gms].base->owner==NULL)&&(!TerrainData.terrain[game->map->Kacheln[PosX a+(PosY b)*gms]].coast||(game->map->GO[PosX a+(PosY b)*gms].base&&game->map->GO[PosX a+(PosY b)*gms].base->data.is_platform)||UnitsData.building[BuildingTyp].data.is_connector)&&(!TerrainData.terrain[game->map->Kacheln[PosX a+(PosY b)*gms]].water||(game->map->GO[PosX a+(PosY b)*gms].base&&game->map->GO[PosX a+(PosY b)*gms].base->data.is_platform)||UnitsData.building[BuildingTyp].data.build_on_water||UnitsData.building[BuildingTyp].data.is_connector)&&!TerrainData.terrain[game->map->Kacheln[PosX a+(PosY b)*gms]].blocked)
#define CHECK_BAND(a,b) (PosX a>=0&&PosX a<gms&&PosY b>=0&&PosY b<gms&&!game->map->GO[PosX a+(PosY b)*gms].vehicle&&!game->map->GO[PosX a+(PosY b)*gms].reserviert&&!(game->map->GO[PosX a+(PosY b)*gms].top&&!game->map->GO[PosX a+(PosY b)*gms].top->data.is_connector)&&!(game->map->GO[PosX a+(PosY b)*gms].base&&game->map->GO[PosX a+(PosY b)*gms].base->owner==NULL)&&(!TerrainData.terrain[game->map->Kacheln[PosX a+(PosY b)*gms]].coast||(game->map->GO[PosX a+(PosY b)*gms].base&&game->map->GO[PosX a+(PosY b)*gms].base->data.is_platform)||UnitsData.building[BuildingTyp].data.is_connector)&&(!game->map->IsWater(PosX a+(PosY b)*gms)||(game->map->GO[PosX a+(PosY b)*gms].base&&game->map->GO[PosX a+(PosY b)*gms].base->data.is_platform)||UnitsData.building[BuildingTyp].data.build_on_water||UnitsData.building[BuildingTyp].data.is_connector)&&!TerrainData.terrain[game->map->Kacheln[PosX a+(PosY b)*gms]].blocked&&(UnitsData.building[BuildingTyp].data.build_on_water?game->map->IsWater(PosX a+(PosY b)*gms):1))

	if ( CHECK_BAND ( -1, -1 ) && CHECK_BAND ( + 0, -1 ) && CHECK_BAND ( -1, + 0 ) )
		pos[0] = true;
	else
		pos[0] = false;

	if ( CHECK_BAND ( + 0, -1 ) && CHECK_BAND ( + 1, -1 ) && CHECK_BAND ( + 1, + 0 ) )
		pos[1] = true;
	else
		pos[1] = false;

	if ( CHECK_BAND ( + 1, + 0 ) && CHECK_BAND ( + 1, + 1 ) && CHECK_BAND ( + 0, + 1 ) )
		pos[2] = true;
	else
		pos[2] = false;

	if ( CHECK_BAND ( -1, + 0 ) && CHECK_BAND ( -1, + 1 ) && CHECK_BAND ( + 0, + 1 ) )
		pos[3] = true;
	else
		pos[3] = false;

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

// Scannt nach Rohstoffen:
void cVehicle::DoSurvey ( void )
{
	char *ptr;
	ptr = owner->ResourceMap;

	if ( !ptr )
		return;

	ptr[PosX+PosY*game->map->size] = 1;

	if ( PosX > 0 )
		ptr[PosX-1+PosY*game->map->size] = 1;

	if ( PosX < game->map->size - 1 )
		ptr[PosX+1+PosY*game->map->size] = 1;

	if ( PosY > 0 )
		ptr[PosX+ ( PosY-1 ) *game->map->size] = 1;

	if ( PosX > 0 && PosY > 0 )
		ptr[PosX-1+ ( PosY-1 ) *game->map->size] = 1;

	if ( PosX < game->map->size - 1 && PosY > 0 )
		ptr[PosX+1+ ( PosY-1 ) *game->map->size] = 1;

	if ( PosY < game->map->size - 1 )
		ptr[PosX+ ( PosY+1 ) *game->map->size] = 1;

	if ( PosX > 0 && PosY < game->map->size - 1 )
		ptr[PosX-1+ ( PosY+1 ) *game->map->size] = 1;

	if ( PosX < game->map->size - 1 && PosY < game->map->size - 1 )
		ptr[PosX+1+ ( PosY+1 ) *game->map->size] = 1;
}

// Macht Meldung:
void cVehicle::MakeReport ( void )
{
	if ( owner != game->ActivePlayer )
		return;

	if ( Disabled )
	{
		// Disabled:
		PlayVoice ( VoiceData.VOIDisabled );
	}
	else
		if ( data.hit_points > data.max_hit_points / 2 )
		{
			// Status Grün:
			if ( data.speed == 0 )
			{
				// Bewegung erschöpft:
				PlayVoice ( VoiceData.VOINoSpeed );
			}
			else
				if ( IsBuilding )
				{
					// Beim bau:
					if ( !BuildRounds )
					{
						// Bau beendet:
						if ( random ( 2, 0 ) )
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
						// Räumen:
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
							if ( random ( 2, 0 ) )
							{
								PlayVoice ( VoiceData.VOILowAmmo1 );
							}
							else
							{
								PlayVoice ( VoiceData.VOILowAmmo2 );
							}
						}
						else
							if ( Wachposten )
							{
								// Wachposten:
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
										nr = random ( 3, 0 );

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

// Prüft, ob Rohstoffe zu dem GO transferiert werden können:
bool cVehicle::CanTransferTo ( sGameObjects *go )
{
	cBuilding *b;
	cVehicle *v;
	int x, y;
	mouse->GetKachel ( &x, &y );

	if ( x < PosX - 1 || x > PosX + 1 || y < PosY - 1 || y > PosY + 1 )
		return false;

	if ( go->vehicle )
	{
		v = go->vehicle;

		if ( v == this )
			return false;

		if ( v->owner != game->ActivePlayer )
			return false;

		if ( v->data.can_transport != data.can_transport )
			return false;

		return true;
	}
	else
		if ( go->top )
		{
			b = go->top;

			if ( b->owner != game->ActivePlayer )
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

// Zeigt das Transfer-Menü an:
void cVehicle::ShowTransfer ( sGameObjects *target )
{
	int LastMouseX = 0, LastMouseY = 0, LastB = 0, x, y, b;
	SDL_Rect scr, dest;
	bool AbbruchPressed = false;
	bool FertigPressed = false;
	bool IncPressed = false;
	bool DecPressed = false;
	bool MouseHot = false;
	int MaxTarget, Target;
	int Transf = 0;
	SDL_Surface *img;
	cVehicle *pv = NULL;
	cBuilding *pb = NULL;

#define POS_CANCEL_BTN 74+166, 125+159
#define POS_DONE___BTN 165+166, 125+159

	mouse->SetCursor ( CHand );
	mouse->draw ( false, buffer );
	game->DrawMap();
	SDL_BlitSurface ( GraphicsData.gfx_hud, NULL, buffer, NULL );

	if ( SettingsData.bAlphaEffects )
		SDL_BlitSurface ( GraphicsData.gfx_shadow, NULL, buffer, NULL );

	dest.x = 166;

	dest.y = 159;

	dest.w = GraphicsData.gfx_transfer->w;

	dest.h = GraphicsData.gfx_transfer->h;

	SDL_BlitSurface ( GraphicsData.gfx_transfer, NULL, buffer, &dest );

	// Die Images erstellen:
	ScaleSurfaceAdv2 ( typ->img_org[0], typ->img[0], typ->img_org[0]->w / 2, typ->img_org[0]->h / 2 );

	img = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, typ->img[0]->w, typ->img[0]->h, 32, 0, 0, 0, 0 );

	SDL_SetColorKey ( img, SDL_SRCCOLORKEY, 0xFF00FF );

	SDL_BlitSurface ( game->ActivePlayer->color, NULL, img, NULL );

	SDL_BlitSurface ( typ->img[0], NULL, img, NULL );

	dest.x = 88 + 166;

	dest.y = 20 + 159;

	dest.h = img->h;

	dest.w = img->w;

	SDL_BlitSurface ( img, NULL, buffer, &dest );

	SDL_FreeSurface ( img );

	if ( target->vehicle )
		pv = target->vehicle;
	else
		pb = target->top;

	if ( pv )
	{
		ScaleSurfaceAdv2 ( pv->typ->img_org[0], pv->typ->img[0], pv->typ->img_org[0]->w / 2, pv->typ->img_org[0]->h / 2 );
		img = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, pv->typ->img[0]->w, pv->typ->img[0]->h, 32, 0, 0, 0, 0 );
		SDL_SetColorKey ( img, SDL_SRCCOLORKEY, 0xFF00FF );
		SDL_BlitSurface ( game->ActivePlayer->color, NULL, img, NULL );
		SDL_BlitSurface ( pv->typ->img[0], NULL, img, NULL );
	}
	else
	{
		if ( pb->data.is_big )
		{
			ScaleSurfaceAdv2 ( pb->typ->img_org, pb->typ->img, pb->typ->img_org->w / 4, pb->typ->img_org->h / 4 );
			img = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, pb->typ->img->w, pb->typ->img->h, 32, 0, 0, 0, 0 );
			SDL_SetColorKey ( img, SDL_SRCCOLORKEY, 0xFF00FF );
			SDL_BlitSurface ( game->ActivePlayer->color, NULL, img, NULL );
			SDL_BlitSurface ( pb->typ->img, NULL, img, NULL );
		}
		else
		{
			ScaleSurfaceAdv2 ( pb->typ->img_org, pb->typ->img, pb->typ->img_org->w / 2, pb->typ->img_org->h / 2 );

			if ( pb->data.has_frames || pb->data.is_connector )
			{
				pb->typ->img->h = pb->typ->img->w = 32;
			}

			img = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, pb->typ->img->w, pb->typ->img->h, 32, 0, 0, 0, 0 );

			SDL_SetColorKey ( img, SDL_SRCCOLORKEY, 0xFF00FF );

			if ( !pb->data.is_connector )
			{
				SDL_BlitSurface ( game->ActivePlayer->color, NULL, img, NULL );
			}

			SDL_BlitSurface ( pb->typ->img, NULL, img, NULL );
		}
	}

	dest.x = 192 + 166;

	dest.y = 20 + 159;
	dest.h = dest.w = 32;
	SDL_BlitSurface ( img, NULL, buffer, &dest );
	SDL_FreeSurface ( img );

	// Text ausgeben:

	font->showTextCentered ( 102 + 166, 64 + 159, typ->data.name );

	if ( pv )
	{

		font->showTextCentered ( 208 + 166, 64 + 159, pv->typ->data.name );
		MaxTarget = pv->data.max_cargo;
		Target = pv->data.cargo;
	}
	else
	{

		font->showTextCentered ( 208 + 166, 64 + 159, pb->typ->data.name );

		if ( data.can_transport == TRANS_METAL )
		{
			MaxTarget = pb->SubBase->MaxMetal;
			Target = pb->SubBase->Metal;
		}
		else
			if ( data.can_transport == TRANS_OIL )
			{
				MaxTarget = pb->SubBase->MaxOil;
				Target = pb->SubBase->Oil;
			}
			else
			{
				MaxTarget = pb->SubBase->MaxGold;
				Target = pb->SubBase->Gold;
			}
	}

	Transf = MaxTarget;

	// Den Bar malen:
	MakeTransBar ( &Transf, MaxTarget, Target );

	drawButton ( lngPack.i18n ( "Text~Button~Cancel" ), false, POS_CANCEL_BTN, buffer );
	drawButton ( lngPack.i18n ( "Text~Button~Done" ), false, POS_DONE___BTN, buffer );

	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );

	mouse->MoveCallback = false;

	while ( 1 )
	{
		if ( game->SelectedVehicle == NULL )
			break;

		// Die Engine laufen lassen:
		game->engine->Run();

		game->HandleTimer();

		// Events holen:
		SDL_PumpEvents();

		// Die Maus machen:
		mouse->GetPos();

		b = mouse->GetMouseButton();

		if ( !b )
			MouseHot = true;

		if ( !MouseHot )
			b = 0;

		x = mouse->x;

		y = mouse->y;

		if ( x != LastMouseX || y != LastMouseY )
		{
			mouse->draw ( true, screen );
		}

		// Abbruch-Button:
		if ( x >= 76 + 166 && x < 153 + 166 && y >= 125 + 159 && y < 145 + 159 )
		{
			if ( b && !AbbruchPressed )
			{
				drawButton ( lngPack.i18n ( "Text~Button~Cancel" ), true, POS_CANCEL_BTN, buffer );
				PlayFX ( SoundData.SNDMenuButton );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				AbbruchPressed = true;
			}
			else
				if ( !b && LastB )
				{
					break;
				}
		}
		else
			if ( AbbruchPressed )
			{
				drawButton ( lngPack.i18n ( "Text~Button~Cancel" ), false, POS_CANCEL_BTN, buffer );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				AbbruchPressed = false;
			}

		// Fertig-Button:
		if ( x >= 165 + 166 && x < 165 + 77 + 166 && y >= 125 + 159 && y < 125 + 23 + 159 )
		{
			if ( b && !FertigPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				drawButton ( lngPack.i18n ( "Text~Button~Done" ), true, POS_DONE___BTN, buffer );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				FertigPressed = true;
			}
			else
				if ( !b && LastB )
				{
					if ( !Transf )
						break;

					data.cargo -= Transf;

					if ( pv )
					{
						pv->data.cargo += Transf;
					}
					else
					{
						if ( data.can_transport == TRANS_METAL )
						{
							pb->owner->base->AddMetal ( pb->SubBase, Transf );
						}
						else
							if ( data.can_transport == TRANS_OIL )
							{
								pb->owner->base->AddOil ( pb->SubBase, Transf );
							}
							else
							{
								pb->owner->base->AddGold ( pb->SubBase, Transf );
							}
					}

					ShowDetails();

					PlayVoice ( VoiceData.VOITransferDone );
					break;
				}
		}
		else
			if ( FertigPressed )
			{
				drawButton ( lngPack.i18n ( "Text~Button~Done" ), false, POS_DONE___BTN, buffer );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				FertigPressed = false;
			}

		// Inc-Button:
		if ( x >= 277 + 166 && x < 277 + 19 + 166 && y >= 88 + 159 && y < 88 + 18 + 159 && b && !IncPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x = 257;
			scr.y = 177;
			dest.w = scr.w = 19;
			dest.h = scr.h = 18;
			dest.x = 277 + 166;
			dest.y = 88 + 159;
			Transf++;
			MakeTransBar ( &Transf, MaxTarget, Target );
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
			SHOW_SCREEN
			mouse->draw ( false, screen );
			IncPressed = true;
		}
		else
			if ( !b && IncPressed )
			{
				scr.x = 277;
				scr.y = 88;
				dest.w = scr.w = 19;
				dest.h = scr.h = 18;
				dest.x = 277 + 166;
				dest.y = 88 + 159;
				SDL_BlitSurface ( GraphicsData.gfx_transfer, &scr, buffer, &dest );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				IncPressed = false;
			}

		// Dec-Button:
		if ( x >= 16 + 166 && x < 16 + 19 + 166 && y >= 88 + 159 && y < 88 + 18 + 159 && b && !DecPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x = 237;
			scr.y = 177;
			dest.w = scr.w = 19;
			dest.h = scr.h = 18;
			dest.x = 16 + 166;
			dest.y = 88 + 159;
			Transf--;
			MakeTransBar ( &Transf, MaxTarget, Target );
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
			SHOW_SCREEN
			mouse->draw ( false, screen );
			DecPressed = true;
		}
		else
			if ( !b && DecPressed )
			{
				scr.x = 16;
				scr.y = 88;
				dest.w = scr.w = 19;
				dest.h = scr.h = 18;
				dest.x = 16 + 166;
				dest.y = 88 + 159;
				SDL_BlitSurface ( GraphicsData.gfx_transfer, &scr, buffer, &dest );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				DecPressed = false;
			}

		// Klick auf den Bar:
		if ( x >= 44 + 166 && x < 44 + 223 + 166 && y >= 86 + 159 && y < 86 + 20 + 159 && b && !LastB )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			Transf = Round ( ( x - ( 44 + 166 ) ) * ( MaxTarget / 223.0 ) - Target );
			MakeTransBar ( &Transf, MaxTarget, Target );
			SHOW_SCREEN
			mouse->draw ( false, screen );
		}

		LastMouseX = x;

		LastMouseY = y;
		LastB = b;
	}

	// Die Images wiederherstellen:
	float newzoom = ( game->hud->Zoom / 64.0 );

	ScaleSurfaceAdv2 ( typ->img_org[0], typ->img[0], ( int ) ( typ->img_org[0]->w* newzoom ), ( int ) ( typ->img_org[0]->h* newzoom ) );

	if ( pv )
	{
		ScaleSurfaceAdv2 ( pv->typ->img_org[0], pv->typ->img[0], ( int ) ( pv->typ->img_org[0]->w* newzoom ), ( int ) ( pv->typ->img_org[0]->h* newzoom ) );
	}
	else
	{
		ScaleSurfaceAdv2 ( pb->typ->img_org, pb->typ->img, ( int ) ( pb->typ->img_org->w* newzoom ), ( int ) ( pb->typ->img_org->h* newzoom ) );
	}

	Transfer = false;

	mouse->MoveCallback = true;
}

// malt den Transfer-Bar (len von 0-223):
void cVehicle::DrawTransBar ( int len )
{
	SDL_Rect scr, dest;

	if ( len < 0 )
		len = 0;

	if ( len > 223 )
		len = 223;

	scr.x = 44;

	scr.y = 90;

	dest.w = scr.w = 223;

	dest.h = scr.h = 16;

	dest.x = 44 + 166;

	dest.y = 90 + 159;

	SDL_BlitSurface ( GraphicsData.gfx_transfer, &scr, buffer, &dest );

	scr.x = 156 + ( 223 - len );

	dest.w = scr.w = 223 - ( 223 - len );

	if ( data.can_transport == TRANS_METAL )
	{
		scr.y = 256;
	}
	else
		if ( data.can_transport == TRANS_OIL )
		{
			scr.y = 273;
		}
		else
		{
			scr.y = 290;
		}

	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
}

// Erzeugt den Transfer Balken:
void cVehicle::MakeTransBar ( int *trans, int MaxTarget, int Target )
{
	SDL_Rect scr, dest;
	// Den trans-Wert berichtigen:

	if ( data.cargo - *trans < 0 )
	{
		*trans += data.cargo - *trans;
	}

	if ( Target + *trans < 0 )
	{
		*trans -= Target + *trans;
	}

	if ( Target + *trans > MaxTarget )
	{
		*trans -= ( Target + *trans ) - MaxTarget;
	}

	if ( data.cargo - *trans > data.max_cargo )
	{
		*trans += ( data.cargo - *trans ) - data.max_cargo;
	}

	// Die Nummern machen:
	scr.x = 4;

	scr.y = 30;

	dest.x = 4 + 166;

	dest.y = 30 + 159;

	dest.w = scr.w = 78;

	dest.h = scr.h = 14;

	SDL_BlitSurface ( GraphicsData.gfx_transfer, &scr, buffer, &dest );



	font->showTextCentered ( 4 + 39 + 166, 30 + 159, iToStr ( data.cargo - *trans ) );

	scr.x = 229;

	dest.x = 229 + 166;

	SDL_BlitSurface ( GraphicsData.gfx_transfer, &scr, buffer, &dest );



	font->showTextCentered ( 229 + 39 + 166, 30 + 159, iToStr ( Target + *trans ) );

	scr.x = 141;

	scr.y = 15;

	dest.x = 141 + 166;

	dest.y = 15 + 159;

	dest.w = scr.w = 29;

	dest.h = scr.h = 21;

	SDL_BlitSurface ( GraphicsData.gfx_transfer, &scr, buffer, &dest );



	font->showTextCentered ( 155 + 166, 21 + 159, iToStr ( abs ( *trans ) ) );

	// Den Pfeil malen:
	if ( *trans < 0 )
	{
		scr.x = 122;
		scr.y = 263;
		dest.x = 143 + 166;
		dest.y = 44 + 159;
		dest.w = scr.w = 30;
		dest.h = scr.h = 16;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
	}
	else
	{
		scr.x = 143;
		scr.y = 44;
		dest.x = 143 + 166;
		dest.y = 44 + 159;
		dest.w = scr.w = 30;
		dest.h = scr.h = 16;
		SDL_BlitSurface ( GraphicsData.gfx_transfer, &scr, buffer, &dest );
	}

	DrawTransBar ( ( int ) ( 223 * ( float ) ( Target + *trans ) / MaxTarget ) );
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
		font->showTextCentered ( COLUMN_1, y, iToStr ( data.ammo ) );
		font->showText ( COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Ammo" ) );
		DrawSymbolBig ( SBAmmo, COLUMN_3, y - 2, 160, data.ammo, typ->data.max_ammo, buffer );
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


	font->showTextCentered ( COLUMN_1, y, iToStr ( data.max_speed / 2 ) ); //FIXME: might crash if e.g. max_speed = 3

	font->showText ( COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Speed" ) );

	DrawSymbolBig ( SBSpeed, COLUMN_3 , y - 2, 160, data.max_speed / 2, typ->data.max_speed / 2, buffer );

	DOLINEBREAK

	// Costs:


	font->showTextCentered ( COLUMN_1, y, iToStr ( data.iBuilt_Costs ) );

	font->showText ( COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Costs" ) );

	DrawSymbolBig ( SBMetal, COLUMN_3 , y - 2, 160, data.iBuilt_Costs, typ->data.iBuilt_Costs, buffer );
}

// Führt alle Maßnahmen durch, die mit einem Wachwechsel eintreten:
void cVehicle::Wachwechsel ( void )
{
	if ( Wachposten )
	{
		owner->AddWachpostenV ( this );
	}
	else
	{
		owner->DeleteWachpostenV ( this );
	}
}

// Prüft, ob sich das Vehicle in der Reichweite von einer Wache befindet:
bool cVehicle::InWachRange ( void )
{
	sWachposten *w;
	int i, off, k;
	cPlayer *p;

	off = PosX + PosY * game->map->size;

	for ( i = 0;i < game->PlayerList->iCount;i++ )
	{
		p = game->PlayerList->Items[i];

		if ( p == owner )
			continue;

		if ( data.is_stealth_land && !p->DetectLandMap[off] )
			return false;

		if ( data.is_stealth_sea && !p->DetectSeaMap[off] )
			return false;

		if ( data.can_drive == DRIVE_AIR )
		{
			if ( ! ( p->WachMapAir[off] && p->ScanMap[off] ) )
				return false;

			// Den finden, der das Vehicle angreifen kann:
			for ( k = 0;k < p->WachpostenAir->iCount;k++ )
			{
				w = p->WachpostenAir->Items[k];

				if ( w->b && w->b->CanAttackObject ( off, true ) )
				{
					game->engine->AddAttackJob ( w->b->PosX + w->b->PosY*game->map->size, off, false, false, true, true, true );

					if ( mjob )
					{
						mjob->finished = true;
						mjob = NULL;
					}

					return true;
				}

				if ( w->v && w->v->CanAttackObject ( off, true ) )
				{
					game->engine->AddAttackJob ( w->v->PosX + w->v->PosY*game->map->size, off, false, w->v->data.can_drive == DRIVE_AIR, true, false, true );

					if ( mjob )
					{
						mjob->finished = true;
						mjob = NULL;
					}

					return true;
				}
			}
		}
		else
		{
			if ( ! ( p->WachMapGround[off] && p->ScanMap[off] ) )
				return false;

			// Den finden, der das Vehicle angreifen kann:
			for ( k = 0;k < p->WachpostenGround->iCount;k++ )
			{
				w = p->WachpostenGround->Items[k];

				if ( w->b && w->b->CanAttackObject ( off, true ) )
				{
					game->engine->AddAttackJob ( w->b->PosX + w->b->PosY*game->map->size, off, false, false, false, true, true );

					if ( mjob )
					{
						mjob->finished = true;
						mjob = NULL;
					}

					return true;
				}

				if ( w->v && w->v->CanAttackObject ( off, true ) )
				{
					game->engine->AddAttackJob ( w->v->PosX + w->v->PosY*game->map->size, off, false, w->v->data.can_drive == DRIVE_AIR, false, false, true );

					if ( mjob )
					{
						mjob->finished = true;
						mjob = NULL;
					}

					return true;
				}
			}
		}
	}

	return false;
}

// Malt Ausgangspunkte für auszuladende Fahrzeuge:
void cVehicle::DrawExitPoints ( sVehicle *typ )
{
	int spx, spy, size;
	spx = GetScreenPosX();
	spy = GetScreenPosY();
	size = game->map->size;

	if ( PosX - 1 >= 0 && PosY - 1 >= 0 && CanExitTo ( PosX - 1 + ( PosY - 1 ) *size, typ ) )
		game->DrawExitPoint ( spx - game->hud->Zoom, spy - game->hud->Zoom );

	if ( PosY - 1 >= 0 && CanExitTo ( PosX + ( PosY - 1 ) *size, typ ) )
		game->DrawExitPoint ( spx, spy - game->hud->Zoom );

	if ( PosY - 1 >= 0 && PosX + 1 < size && CanExitTo ( PosX + 1 + ( PosY - 1 ) *size, typ ) )
		game->DrawExitPoint ( spx + game->hud->Zoom, spy - game->hud->Zoom );

	if ( PosX - 1 >= 0 && CanExitTo ( PosX - 1 + ( PosY ) *size, typ ) )
		game->DrawExitPoint ( spx - game->hud->Zoom, spy );

	if ( PosX + 1 < size && PosX + 1 < size && CanExitTo ( PosX + 1 + ( PosY ) *size, typ ) )
		game->DrawExitPoint ( spx + game->hud->Zoom, spy );

	if ( PosX - 1 >= 0 && PosY + 1 < size && CanExitTo ( PosX - 1 + ( PosY + 1 ) *size, typ ) )
		game->DrawExitPoint ( spx - game->hud->Zoom, spy + game->hud->Zoom );

	if ( PosY + 1 < size && CanExitTo ( PosX + ( PosY + 1 ) *size, typ ) )
		game->DrawExitPoint ( spx, spy + game->hud->Zoom );

	if ( PosY + 1 < size && PosX + 1 < size && CanExitTo ( PosX + 1 + ( PosY + 1 ) *size, typ ) )
		game->DrawExitPoint ( spx + game->hud->Zoom, spy + game->hud->Zoom );
}

bool cVehicle::CanExitTo ( int off, sVehicle *typ )
{
	int boff;

	if ( off < 0 || off >= game->map->size*game->map->size )
		return false;

	if ( abs ( ( off % game->map->size ) - PosX ) > 8 || abs ( ( off / game->map->size ) - PosY ) > 8 )
		return false;

	boff = PosX + PosY * game->map->size;

	if ( ( off > game->map->size*off > game->map->size ) ||
	        ( off >= boff - 1 - game->map->size && off <= boff + 2 - game->map->size ) ||
	        ( off >= boff - 1 && off <= boff + 2 ) ||
	        ( off >= boff - 1 + game->map->size && off <= boff + 2 + game->map->size ) )
		{}
	else
		return false;

	if ( ( typ->data.can_drive != DRIVE_AIR && ( ( game->map->GO[off].top && !game->map->GO[off].top->data.is_connector ) || game->map->GO[off].vehicle || TerrainData.terrain[game->map->Kacheln[off]].blocked ) ) ||
	        ( typ->data.can_drive == DRIVE_AIR && game->map->GO[off].plane ) ||
	        ( typ->data.can_drive == DRIVE_SEA && !game->map->IsWater ( off, true ) ) ||
	        ( typ->data.can_drive == DRIVE_LAND && game->map->IsWater ( off ) ) )
	{
		return false;
	}

	return true;
}

bool cVehicle::CanLoad ( int off )
{
	int boff;

	if ( data.cargo == data.max_cargo )
		return false;

	boff = PosX + PosY * game->map->size;

	if ( ( off > game->map->size*off > game->map->size ) ||
	        ( off >= boff - 1 - game->map->size && off <= boff + 2 - game->map->size ) ||
	        ( off >= boff - 1 && off <= boff + 2 ) ||
	        ( off >= boff - 1 + game->map->size && off <= boff + 2 + game->map->size ) )
		{}
	else
		return false;

	if ( !game->map->GO[off].vehicle )
		return false;

	if ( data.can_drive != DRIVE_AIR && game->map->GO[off].vehicle->data.can_drive == DRIVE_SEA )
		return false;

	if ( data.can_transport == TRANS_MEN && !game->map->GO[off].vehicle->data.is_human )
		return false;

	if ( game->map->GO[off].vehicle && game->map->GO[off].vehicle->owner == owner && !game->map->GO[off].vehicle->IsBuilding && !game->map->GO[off].vehicle->IsClearing )
		return true;

	return false;
}

void cVehicle::StoreVehicle ( int off )
{
	cVehicle *v = NULL;

	if ( data.cargo == data.max_cargo )
		return;

	if ( data.can_transport == TRANS_VEHICLES )
	{
		v = game->map->GO[off].vehicle;

		if ( v->data.can_drive == DRIVE_SEA && data.can_drive == DRIVE_SEA )
		{
			return;
		}

		game->map->GO[off].vehicle = NULL;
	}
	else
		if ( data.can_transport == TRANS_MEN )
		{
			v = game->map->GO[off].vehicle;
			game->map->GO[off].vehicle = NULL;
		}

	if ( !v )
		return;

	if ( v->mjob )
	{
		if ( v->moving || v->rotating || v->Attacking || v->MoveJobActive )
			return;

		if ( v->mjob )
		{
			v->mjob->finished = true;
			v->mjob = NULL;
			v->MoveJobActive = false;
		}
	}

	v->Loaded = true;

	StoredVehicles->Add ( v );
	data.cargo++;

	if ( game->SelectedVehicle && game->SelectedVehicle == this )
	{
		ShowDetails();
	}

	if ( v == game->SelectedVehicle )
	{
		v->Deselct();
		game->SelectedVehicle = NULL;
	}

	owner->DoScan();
}

void cVehicle::ShowStorage ( void )
{
	int LastMouseX = 0, LastMouseY = 0, LastB = 0, x, y, b, i, to;
	SDL_Surface *sf;
	SDL_Rect scr, dest;
	bool FertigPressed = false;
	bool DownPressed = false, DownEnabled = false;
	bool UpPressed = false, UpEnabled = false;
	bool AlleAktivierenEnabled = false;
	int offset = 0;

	LoadActive = false;
	mouse->SetCursor ( CHand );
	mouse->draw ( false, buffer );
	scr.x = 480;
	scr.y = 0;
	scr.w = 640 - 480;
	scr.h = 480;
	SDL_BlitSurface ( GraphicsData.gfx_storage, &scr, buffer, &scr );
	SDL_BlitSurface ( GraphicsData.gfx_storage_ground, NULL, buffer, NULL );
	to = 6;

	// Alle Buttons machen:
	// Fertig-Button:
	scr.w = dest.w = 94;
	scr.h = dest.h = 23;
	scr.x = 0;
	scr.y = 468;
	dest.x = 510;
	dest.y = 371;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
	// Down:

	if ( StoredVehicles->iCount > to )
	{
		DownEnabled = true;
		scr.x = 103;
		scr.y = 452;
		dest.h = scr.h = dest.w = scr.w = 25;
		dest.x = 530;
		dest.y = 426;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
	}

	// Alle Aktivieren:
	scr.x = 0;

	scr.y = 376;

	dest.w = scr.w = 94;

	dest.h = scr.h = 23;

	dest.x = 511;

	dest.y = 251;

	if ( StoredVehicles->iCount )
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
		AlleAktivierenEnabled = true;
	}

	// Vehicles anzeigen:
	DrawStored ( offset );

	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );

	mouse->MoveCallback = false;

	while ( 1 )
	{
		if ( game->SelectedVehicle == NULL )
			break;

		// Die Engine laufen lassen:
		game->engine->Run();

		game->HandleTimer();

		// Events holen:
		SDL_PumpEvents();

		// Die Maus machen:
		mouse->GetPos();

		b = mouse->GetMouseButton();

		x = mouse->x;

		y = mouse->y;

		if ( x != LastMouseX || y != LastMouseY )
		{
			mouse->draw ( true, screen );
		}

		// Down-Button:
		if ( DownEnabled )
		{
			if ( x >= 530 && x < 530 + 25 && y >= 426 && y < 426 + 25 && b && !DownPressed )
			{
				PlayFX ( SoundData.SNDObjectMenu );
				scr.x = 530;
				scr.y = 426;
				dest.w = scr.w = 25;
				dest.h = scr.h = 25;
				dest.x = 530;
				dest.y = 426;

				offset += to;

				if ( StoredVehicles->iCount <= offset + to )
					DownEnabled = false;

				DrawStored ( offset );

				SDL_BlitSurface ( GraphicsData.gfx_storage, &scr, buffer, &dest );

				scr.x = 130;

				scr.y = 452;

				dest.w = scr.w = 25;

				dest.h = scr.h = 25;

				dest.x = 504;

				dest.y = 426;

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
					dest.w = scr.w = 25;
					dest.h = scr.h = 25;
					dest.x = 530;
					dest.y = 426;
					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
					SHOW_SCREEN
					mouse->draw ( false, screen );
					DownPressed = false;
				}
		}

		// Up-Button:
		if ( UpEnabled )
		{
			if ( x >= 504 && x < 504 + 25 && y >= 426 && y < 426 + 25 && b && !UpPressed )
			{
				PlayFX ( SoundData.SNDObjectMenu );
				scr.x = 504;
				scr.y = 426;
				dest.w = scr.w = 25;
				dest.h = scr.h = 25;
				dest.x = 504;
				dest.y = 426;

				offset -= to;

				if ( offset == 0 )
					UpEnabled = false;

				DrawStored ( offset );

				SDL_BlitSurface ( GraphicsData.gfx_storage, &scr, buffer, &dest );

				mouse->draw ( false, screen );

				UpPressed = true;

				if ( StoredVehicles->iCount > to )
				{
					DownEnabled = true;
					scr.x = 103;
					scr.y = 452;
					dest.h = scr.h = dest.w = scr.w = 25;
					dest.x = 530;
					dest.y = 426;
					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
				}

				SHOW_SCREEN
			}
			else
				if ( !b && UpPressed && UpEnabled )
				{
					scr.x = 130;
					scr.y = 452;
					dest.w = scr.w = 25;
					dest.h = scr.h = 25;
					dest.x = 504;
					dest.y = 426;
					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
					SHOW_SCREEN
					mouse->draw ( false, screen );
					UpPressed = false;
				}
		}

		// Fertig-Button:
		if ( x >= 510 && x < 510 + 94 && y >= 371 && y < 371 + 23 )
		{
			if ( b && !FertigPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.w = dest.w = 94;
				scr.h = dest.h = 23;
				scr.x = 510;
				scr.y = 371;
				dest.x = 510;
				dest.y = 371;
				SDL_BlitSurface ( GraphicsData.gfx_storage, &scr, buffer, &dest );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				FertigPressed = true;
			}
			else
				if ( !b && LastB )
				{
					break;
				}
		}
		else
			if ( FertigPressed )
			{
				scr.w = dest.w = 94;
				scr.h = dest.h = 23;
				scr.x = 0;
				scr.y = 468;
				dest.x = 510;
				dest.y = 371;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				FertigPressed = false;
			}

		// Alle Aktivieren:
		if ( x >= 511 && x < 511 + 94 && y >= 251 && y < 251 + 23 && b && !LastB && AlleAktivierenEnabled )
		{
			sVehicle *typ;
			int size;
			PlayFX ( SoundData.SNDMenuButton );
			dest.w = 94;
			dest.h = 23;
			dest.x = 511;
			dest.y = 251;
			SDL_BlitSurface ( GraphicsData.gfx_storage, &dest, buffer, &dest );
			SHOW_SCREEN
			mouse->draw ( false, screen );

			while ( b )
			{
				SDL_PumpEvents();
				b = mouse->GetMouseButton();
			}

			game->OverObject = NULL;

			mouse->MoveCallback = true;

			PlayFX ( SoundData.SNDActivate );
			size = game->map->size;

			for ( i = 0;i < StoredVehicles->iCount; )
			{
				typ = StoredVehicles->Items[i]->typ;

				if ( PosX - 1 >= 0 && PosY - 1 >= 0 && CanExitTo ( PosX - 1 + ( PosY - 1 ) *size, typ ) )
				{
					ExitVehicleTo ( i, PosX - 1 + ( PosY - 1 ) *size, false );
					continue;
				}

				if ( PosY - 1 >= 0 && CanExitTo ( PosX + ( PosY - 1 ) *size, typ ) )
				{
					ExitVehicleTo ( i, PosX + ( PosY - 1 ) *size, false );
					continue;
				}

				if ( PosY - 1 >= 0 && CanExitTo ( PosX + 1 + ( PosY - 1 ) *size, typ ) )
				{
					ExitVehicleTo ( i, PosX + 1 + ( PosY - 1 ) *size, false );
					continue;
				}

				if ( PosX - 1 >= 0 && CanExitTo ( PosX - 1 + ( PosY ) *size, typ ) )
				{
					ExitVehicleTo ( i, PosX - 1 + ( PosY ) *size, false );
					continue;
				}

				if ( CanExitTo ( PosX + ( PosY ) *size, typ ) )
				{
					ExitVehicleTo ( i, PosX + ( PosY ) *size, false );
					continue;
				}

				if ( PosX + 1 < size && CanExitTo ( PosX + 1 + ( PosY ) *size, typ ) )
				{
					ExitVehicleTo ( i, PosX + 1 + ( PosY ) *size, false );
					continue;
				}

				if ( PosX - 1 >= 0 && PosY + 1 < size && CanExitTo ( PosX - 1 + ( PosY + 1 ) *size, typ ) )
				{
					ExitVehicleTo ( i, PosX - 1 + ( PosY + 1 ) *size, false );
					continue;
				}

				if ( PosY + 1 < size && CanExitTo ( PosX + ( PosY + 1 ) *size, typ ) )
				{
					ExitVehicleTo ( i, PosX + ( PosY + 1 ) *size, false );
					continue;
				}

				if ( PosY + 1 < size && CanExitTo ( PosX + 1 + ( PosY + 1 ) *size, typ ) )
				{
					ExitVehicleTo ( i, PosX + 1 + ( PosY + 1 ) *size, false );
					continue;
				}

				i++;
			}

			return;
		}

		// Buttons unter den Vehicles:
		dest.w = 73;

		dest.h = 23;

		sf = GraphicsData.gfx_storage_ground;

		for ( i = 0;i < to;i++ )
		{
			if ( StoredVehicles->iCount <= i + offset )
				break;

			if ( i == 0 )
			{
				dest.x = 8;
				dest.y = 191;
			}
			else
				if ( i == 1 )
				{
					dest.x = 163;
					dest.y = 191;
				}
				else
					if ( i == 2 )
					{
						dest.x = 318;
						dest.y = 191;
					}
					else
						if ( i == 3 )
						{
							dest.x = 8;
							dest.y = 426;
						}
						else
							if ( i == 4 )
							{
								dest.x = 163;
								dest.y = 426;
							}
							else
								if ( i == 5 )
								{
									dest.x = 318;
									dest.y = 426;
								}

			// Aktivieren:
			if ( x >= dest.x && x < dest.x + 73 && y >= dest.y && y < dest.y + 23 && b && !LastB )
			{
				PlayFX ( SoundData.SNDMenuButton );
				ActivatingVehicle = true;
				VehicleToActivate = i + offset;
				SDL_BlitSurface ( sf, &dest, buffer, &dest );
				SHOW_SCREEN
				mouse->draw ( false, screen );

				while ( b )
				{
					SDL_PumpEvents();
					b = mouse->GetMouseButton();
				}

				game->OverObject = NULL;

				mouse->MoveCallback = true;
				return;
			}
		}

		LastMouseX = x;

		LastMouseY = y;
		LastB = b;
	}

	mouse->MoveCallback = true;
}

// Malt alle Bilder der geladenen Vehicles:
void cVehicle::DrawStored ( int off )
{
	SDL_Rect scr, dest;
	SDL_Surface *sf;
	cVehicle *v;
	int i, to;

	to = 6;
	sf = GraphicsData.gfx_storage_ground;

	for ( i = 0;i < to;i++ )
	{
		if ( i + off >= StoredVehicles->iCount )
		{
			v = NULL;
		}
		else
		{
			v = StoredVehicles->Items[i+off];
		}

		// Das Bild malen:
		if ( i == 0 )
		{
			dest.x = 17;
			dest.y = 9;
		}
		else
			if ( i == 1 )
			{
				dest.x = 172;
				dest.y = 9;
			}
			else
				if ( i == 2 )
				{
					dest.x = 327;
					dest.y = 9;
				}
				else
					if ( i == 3 )
					{
						dest.x = 17;
						dest.y = 244;
					}
					else
						if ( i == 4 )
						{
							dest.x = 172;
							dest.y = 244;
						}
						else
							if ( i == 5 )
							{
								dest.x = 327;
								dest.y = 244;
							}

		dest.w = 128;

		dest.h = 128;

		SDL_BlitSurface ( sf, &dest, buffer, &dest );

		if ( v )
		{
			SDL_BlitSurface ( v->typ->storage, NULL, buffer, &dest );
			// Den Namen ausgeben:
			font->showText ( dest.x + 5, dest.y + 5, v->name, LATIN_SMALL_WHITE );
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

		scr.w = dest.w = 73;
		scr.h = dest.h = 23;

		if ( v )
		{
			scr.x = 156;
			scr.y = 431;
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &scr, buffer, &dest );
		}
		else
		{
			SDL_BlitSurface ( sf, &dest, buffer, &dest );
		}

		// Die zusätzlichen Infos anzeigen:
		dest.x += 9;

		dest.y -= 44 - 6;

		dest.w = 128;

		dest.h = 30;

		SDL_BlitSurface ( sf, &dest, buffer, &dest );

		dest.x += 6;

		if ( v )
		{
			// Die Hitpoints anzeigen:
			DrawNumber ( dest.x + 13, dest.y, v->data.hit_points, v->data.max_hit_points, buffer );

			font->showText ( dest.x + 27, dest.y, lngPack.i18n ( "Text~Hud~Hitpoints" ), LATIN_SMALL_WHITE );
			DrawSymbol ( SHits, dest.x + 60, dest.y, 58, v->data.hit_points, v->data.max_hit_points, buffer );
			// Die Munition anzeigen:

			if ( v->data.can_attack )
			{
				dest.y += 15;
				DrawNumber ( dest.x + 13, dest.y, v->data.ammo, v->data.max_ammo, buffer );

				font->showText ( dest.x + 27, dest.y, lngPack.i18n ( "Text~Hud~AmmoShort" ), LATIN_SMALL_WHITE );
				DrawSymbol ( SAmmo, dest.x + 60, dest.y, 58, v->data.ammo, v->data.max_ammo, buffer );
			}
		}
	}
}

// Läd ein Vehicle aus:
void cVehicle::ExitVehicleTo ( int nr, int off, bool engine_call )
{
	cVehicle *ptr;

	if ( !StoredVehicles || StoredVehicles->iCount <= nr )
		return;

	ptr = StoredVehicles->Items[nr];

	StoredVehicles->Delete ( nr );

	data.cargo--;

	ActivatingVehicle = false;

	if ( this == game->SelectedVehicle )
	{
		ShowDetails();
	}

	if ( ptr->data.can_drive == DRIVE_AIR )
	{
		game->map->GO[off].plane = ptr;
	}
	else
	{
		game->map->GO[off].vehicle = ptr;
	}

	ptr->PosX = off % game->map->size;

	ptr->PosY = off / game->map->size;
	ptr->Loaded = false;
	ptr->data.shots = 0;
	ptr->InWachRange();

	owner->DoScan();
}

// Prüft, ob das Objekt aufmunitioniert werden kann:
bool cVehicle::CanMuni ( int off )
{
	cBuilding *b;
	cVehicle *v;
	int boff;

	if ( data.cargo < 2 )
		return false;

	boff = PosX + PosY * game->map->size;

	if ( ( off > game->map->size*off > game->map->size ) ||
	        ( off >= boff - 1 - game->map->size && off <= boff + 2 - game->map->size ) ||
	        ( off >= boff - 1 && off <= boff + 2 ) ||
	        ( off >= boff - 1 + game->map->size && off <= boff + 2 + game->map->size ) )
		{}
	else
		return false;

	v = game->map->GO[off].vehicle;

	if ( !v && game->map->GO[off].plane && game->map->GO[off].plane->FlightHigh == 0 )
		v = game->map->GO[off].plane;

	if ( v )
	{
		if ( v == this || v->owner != owner || !v->data.can_attack || v->data.ammo == v->data.max_ammo || v->moving || v->rotating || v->Attacking )
			return false;

		return true;
	}

	b = game->map->GO[off].top;

	if ( b )
	{
		if ( b->owner != owner || !b->data.can_attack || b->data.ammo == b->data.max_ammo )
			return false;

		return true;
	}

	return false;
}

bool cVehicle::CanRepair ( int off )
{
	cBuilding *b;
	cVehicle *v;
	int boff;

	if ( data.cargo < 2 )
		return false;

	boff = PosX + PosY * game->map->size;

	if ( ( off > game->map->size*off > game->map->size ) ||
	        ( off >= boff - 1 - game->map->size && off <= boff + 2 - game->map->size ) ||
	        ( off >= boff - 1 && off <= boff + 2 ) ||
	        ( off >= boff - 1 + game->map->size && off <= boff + 2 + game->map->size ) )
		{}
	else
		return false;

	v = game->map->GO[off].vehicle;

	if ( !v && game->map->GO[off].plane && game->map->GO[off].plane->FlightHigh == 0 )
		v = game->map->GO[off].plane;

	if ( v )
	{
		if ( v == this || v->owner != owner || v->data.hit_points == v->data.max_hit_points || v->moving || v->rotating || v->Attacking )
			return false;

		return true;
	}

	b = game->map->GO[off].top;

	if ( b )
	{
		if ( b->owner != owner || b->data.hit_points == b->data.max_hit_points )
			return false;

		return true;
	}

	return false;
}

// Legt eine Mine:
void cVehicle::LayMine ( void )
{
	if ( !data.cargo )
		return;

	if ( game->map->GO[PosX+PosY*game->map->size].base && game->map->GO[PosX+PosY*game->map->size].base->data.is_expl_mine )
		return;

	if ( data.can_drive == DRIVE_SEA )
	{
		game->engine->AddBuilding ( PosX, PosY, UnitsData.building + BNrSeaMine, owner, false );
		PlayFX ( SoundData.SNDSeaMinePlace );
	}
	else
	{
		game->engine->AddBuilding ( PosX, PosY, UnitsData.building + BNrLandMine, owner, false );

		PlayFX ( SoundData.SNDLandMinePlace );
	}

	data.cargo--;

	if ( game->SelectedVehicle == this )
	{
		ShowDetails();
	}
}

// Räumt eine Mine:
void cVehicle::ClearMine ( void )
{
	cBuilding *b;
	int off;

	if ( data.cargo == data.max_cargo )
		return;

	off = PosX + PosY * game->map->size;

	b = game->map->GO[off].base;

	if ( !b || !b->data.is_expl_mine || b->owner != owner )
		return;

	// Mine räumen:
	game->map->GO[off].base = NULL;

	if ( b->prev )
	{
		cBuilding *pb;
		pb = b->prev;
		pb->next = b->next;

		if ( pb->next )
			pb->next->prev = pb;
	}
	else
	{
		b->owner->BuildingList = b->next;

		if ( b->next )
			b->next->prev = NULL;
	}

	delete b;

	if ( data.can_drive == DRIVE_SEA )
	{
		PlayFX ( SoundData.SNDSeaMineClear );
	}
	else
	{
		PlayFX ( SoundData.SNDLandMineClear );
	}

	data.cargo++;

	if ( game->SelectedVehicle == this )
	{
		ShowDetails();
	}
}

// Sucht nach Minen:
void cVehicle::DetectMines ( void )
{
	int off, size;
	size = game->map->size;
	off = PosX + PosY * size;

#define DETECT_MINE(a) if((a)>=0&&(a)<size*size&&game->map->GO[(a)].base&&game->map->GO[(a)].base->data.is_expl_mine)game->map->GO[(a)].base->detected=true;
	DETECT_MINE ( off - size - 1 )
	DETECT_MINE ( off - size )
	DETECT_MINE ( off - size + 1 )
	DETECT_MINE ( off - 1 )
	DETECT_MINE ( off )
	DETECT_MINE ( off + 1 )
	DETECT_MINE ( off + size - 1 )
	DETECT_MINE ( off + size )
	DETECT_MINE ( off + size + 1 )
}

// Prüft, ob das Ziel direkt neben einem steht, und ob es gestohlen werden kann:
bool cVehicle::IsInRangeCommando ( int off, bool steal )
{
	sGameObjects *go;
	int boff;
	boff = PosX + PosY * game->map->size;

	if ( off == boff )
		return false;

	if ( ( off > game->map->size*off > game->map->size ) ||
	        ( off >= boff - 1 - game->map->size && off <= boff + 2 - game->map->size ) ||
	        ( off >= boff - 1 && off <= boff + 2 ) ||
	        ( off >= boff - 1 + game->map->size && off <= boff + 2 + game->map->size ) )
		{}
	else
		return false;

	go = game->map->GO + off;

	if ( steal && go->vehicle && go->vehicle->owner != owner )
		return true;

	if ( !steal && ( ( go->vehicle && go->vehicle->owner != owner ) || ( go->top && go->top->owner != owner ) ) )
		return true;

	return false;
}

// Malt die Commando-Cursor:
void cVehicle::DrawCommandoCursor ( struct sGameObjects *go, bool steal )
{
	cBuilding *b = NULL;
	cVehicle *v = NULL;
	SDL_Surface *sf;
	SDL_Rect r;

	if ( steal )
	{
		v = go->vehicle;
		sf = GraphicsData.gfx_Csteal;
	}
	else
	{
		v = go->vehicle;

		if ( !v )
			b = go->top;

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

	r.w = 35 * ( int ) ( CalcCommandoChance ( steal ) / 100.0 );
	SDL_FillRect ( sf, &r, 0x00FF00 );
}

// Berechnet die Chance, dass die Commadooperation gelingt (0-100);
int cVehicle::CalcCommandoChance ( bool steal )
{
	if ( steal )
	{
		switch ( CommandoRank )
		{

			case 0:
				return 20;

			case 1:
				return 30;

			case 2:
				return 50;

			case 3:
				return 75;

			case 4:
				return 80;

			case 5:
				return 90;
		}
	}
	else
	{
		switch ( CommandoRank )
		{

			case 0:
				return 80;

			case 1:
				return 90;

			case 2:
				return 95;

			case 3:
				return 98;

			case 4:
				return 100;

			case 5:
				return 100;
		}
	}

	return 0;
}

// Führt eine Kommandooperation durch:
void cVehicle::CommandoOperation ( int off, bool steal )
{
	int chance;
	bool success;
	data.shots--;
	StealActive = false;
	DisableActive = false;

	chance = CalcCommandoChance ( steal );

	if ( random ( 100, 0 ) < chance )
		success = true;
	else
		success = false;

	if ( success )
	{
		if ( steal )
		{
			cVehicle *v;
			v = game->map->GO[off].vehicle;

			if ( v )
				v->owner = owner;

			PlayVoice ( VoiceData.VOIUnitStolen );
		}
		else
		{
			cVehicle *v;
			cBuilding *b;
			v = game->map->GO[off].vehicle;
			PlayVoice ( VoiceData.VOIUnitDisabled );

			if ( v )
			{
				v->Disabled = 2 + CommandoRank / 2;
				v->data.speed = 0;
				v->data.shots = 0;

				if ( v->mjob )
				{
					v->mjob->finished = true;
					v->mjob = NULL;
					v->moving = false;
					v->MoveJobActive = false;
				}
			}
			else
			{
				b = game->map->GO[off].top;

				if ( b )
				{
					b->Disabled = 2 + CommandoRank / 2;
					b->data.shots = 0;
					b->StopWork ( true );
				}
			}
		}

		if ( CommandoRank < 5 )
			CommandoRank++;
	}
	else
	{
		PlayVoice ( VoiceData.VOICommandoDetected );
	}

	if ( game->SelectedVehicle == this )
		ShowDetails();
}

void cVehicle::DeleteStored ( void )
{
	if ( !StoredVehicles )
		return;

	while ( StoredVehicles->iCount )
	{
		cVehicle *v;
		v = StoredVehicles->Items[0];

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
		StoredVehicles->Delete ( 0 );
	}

	delete StoredVehicles;

	StoredVehicles = NULL;
}
