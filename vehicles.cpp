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
#include "menus.h"
#include "settings.h"
#include "hud.h"


//-----------------------------------------------------------------------------
// cVehicle Class Implementation
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
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
	FlightHigh = 0;
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
	groupSelected = false;
	owner = Owner;
	data = owner->VehicleData[typ->nr];
	data.hitpointsCur = data.hitpointsMax;
	data.ammoCur = data.ammoMax;
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

//-----------------------------------------------------------------------------
cVehicle::~cVehicle ()
{
	if ( ClientMoveJob )
	{
		ClientMoveJob->release();
		ClientMoveJob->Vehicle = NULL;
		if ( ClientMoveJob->endMoveAction ) ClientMoveJob->endMoveAction->handleDelVehicle ( this );
	}
	if ( ServerMoveJob )
	{
		ServerMoveJob->release();
		ServerMoveJob->Vehicle = NULL;
	}

	if ( autoMJob )
		delete autoMJob;

	if ( bSentryStatus )
		owner->deleteSentryVehicle ( this );

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
	if ( Server )
	{
		for ( unsigned int i = 0; i < Server->AJobs.Size(); i++ )
		{
			if ( Server->AJobs[i]->vehicle == this ) Server->AJobs[i]->vehicle = NULL;
		}
	}
	if ( Client )
	{
		for ( unsigned int i = 0; i < Client->attackJobs.Size(); i++ )
		{
			if ( Client->attackJobs[i]->vehicle == this ) Client->attackJobs[i]->vehicle = NULL;
		}
	}

	while( passiveEndMoveActions.Size() )
	{
		cEndMoveAction *endMoveAction = passiveEndMoveActions[0];
		passiveEndMoveActions.Delete ( 0 );
		delete endMoveAction;
	}

	if ( Client && Client->gameGUI.getSelVehicle() == this )
	{
		Client->gameGUI.deselectUnit();
	}
}

//-----------------------------------------------------------------------------
/** Draws the vehicle */
//-----------------------------------------------------------------------------
void cVehicle::draw ( SDL_Rect screenPosition )
{
	SDL_Rect tmp;

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
		Client->iObjectStream = playStream();
	}

	//make damage effect
	if ( Client->timer100ms && data.hitpointsCur < data.hitpointsMax && SettingsData.bDamageEffects && ( owner == Client->ActivePlayer || Client->ActivePlayer->ScanMap[PosX+PosY*Client->Map->size] ) )
	{
		int intense = ( int ) ( 100 - 100 * ( ( float ) data.hitpointsCur / data.hitpointsMax ) );
		Client->addFX ( fxDarkSmoke, PosX*64 + DamageFXPointX + OffX, PosY*64 + DamageFXPointY + OffY, intense );
	}

	//make landing and take off of planes
	if ( data.factorAir > 0 && Client->timer50ms )
	{
		// check, if the plane should land
		cBuilding *b = Client->Map->fields[PosX+PosY*Client->Map->size].getTopBuilding();

		if ( b && b->owner == owner && b->data.canBeLandedOn && !ClientMoveJob && !moving && !Attacking )
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

	// make the dithering
	if ( Client->timer100ms )
	{
		if ( FlightHigh > 0 )
		{

			if ( moving || ANIMATION_SPEED % 10 == 0 )
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
		else
		{
			ditherX = 0;
			ditherY = 0;
		}
	}

	//rotate vehicles to the right direction for building/clearing
	if ( ( IsBuilding || IsClearing ) && Client->timer100ms )
	{
		if ( data.isBig )
			dir = 0;
		else
			RotateTo ( 0 );
	}

	//run start up effect
	if ( StartUp )
	{
		if ( Client->timer50ms )
			StartUp += 25;

		if ( StartUp >= 255 )
			StartUp = 0;

		//max StartUp value for undetected stealth units is 100, because they stay half visible
		if ( (data.isStealthOn&TERRAIN_SEA) && Client->Map->IsWater ( PosX + PosY*Client->Map->size, true ) && DetectedByPlayerList.Size() == 0 && owner == Client->ActivePlayer )
		{
			if ( StartUp > 100 ) StartUp = 0;
		}
	}

	if ( IsBuilding && dir == 0 && BigBetonAlpha < 255 )
	{
		if ( Client->timer50ms )
			BigBetonAlpha += 25;

		if ( BigBetonAlpha > 255 )
			BigBetonAlpha = 255;
	}

	//calculate screen position
	int ox = (int) (OffX * Client->gameGUI.getZoom());
	int oy = (int) (OffY * Client->gameGUI.getZoom());

	if ( !IsBuilding && !IsClearing )
	{
		screenPosition.x += ox;
		screenPosition.y += oy;
	}
	if ( FlightHigh > 0 )
	{
		screenPosition.x += ditherX;
		screenPosition.y += ditherY;
	}

	SDL_Rect dest;
	dest.x = dest.y = 0;
	bool bDraw = false;
	SDL_Surface* drawingSurface = Client->gameGUI.getDCache()->getCachedImage(this);
	if ( drawingSurface == NULL )
	{
		//no cached image found. building needs to be redrawn.
		bDraw = true;
		drawingSurface = Client->gameGUI.getDCache()->createNewEntry(this);
	}

	if ( drawingSurface == NULL )
	{
		//image will not be cached. So blitt directly to the screen buffer.
		dest = screenPosition;
		drawingSurface = buffer;
	}

	if ( bDraw )
	{
		render ( drawingSurface, dest );
	}

	//now check, whether the image has to be blitted to screen buffer
	if ( drawingSurface != buffer )
	{
		dest = screenPosition;
		SDL_BlitSurface( drawingSurface, NULL, buffer, &dest );
	}

	// draw overlay if necessary:
	if ( data.hasOverlay && SettingsData.bAnimations )
	{
		SDL_Rect src;

		tmp = screenPosition;
		src.h = src.w = (int)(typ->overlay_org->h*Client->gameGUI.getZoom());
		tmp.x += (int)(Client->gameGUI.getTileSize()) / 2 - src.h / 2;
		tmp.y += (int)(Client->gameGUI.getTileSize()) / 2 - src.h / 2;
		src.y = 0;
		src.x = Disabled ? 0 : ( int ) ( ( typ->overlay_org->h * ( ( ANIMATION_SPEED % ( (int)(typ->overlay_org->w*Client->gameGUI.getZoom()) / src.h ) ) ) ) * Client->gameGUI.getZoom() );

		if ( StartUp && SettingsData.bAlphaEffects )
			SDL_SetAlpha ( typ->overlay, SDL_SRCALPHA, StartUp );
		else
			SDL_SetAlpha ( typ->overlay, SDL_SRCALPHA, 255 );

		blitWithPreScale ( typ->overlay_org, typ->overlay, &src, buffer, &tmp, Client->gameGUI.getZoom() );
	}

	//remove the dithering for the following operations
	if ( FlightHigh > 0 )
	{
		screenPosition.x -= ditherX;
		screenPosition.y -= ditherY;
	}

	// draw indication, when building is complete
	if ( IsBuilding && BuildRounds == 0  && owner == Client->ActivePlayer && !BuildPath )
	{
		SDL_Rect d, t;
		int max, nr;
		nr = 0xFF00 - ( ( ANIMATION_SPEED % 0x8 ) * 0x1000 );

		if ( data.isBig )
			max = ( Client->gameGUI.getTileSize() * 2) - 3;
		else
			max = Client->gameGUI.getTileSize() - 3;

		d.x = screenPosition.x + 2;
		d.y = screenPosition.y + 2;
		d.w = max;
		d.h = 3;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.y += max - 3;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.y = screenPosition.y + 2 + oy;
		d.w = 3;
		d.h = max;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.x += max - 3;
		SDL_FillRect ( buffer, &d, nr );
	}

	// Draw the colored frame if necessary
	if ( Client->gameGUI.colorChecked() )
	{
		SDL_Rect d, t;
		int max, nr;
		nr = * ( unsigned int* ) owner->color->pixels;

		if ( data.isBig ) max = ( (int)(Client->gameGUI.getTileSize()) - 1 ) * 2;
		else max = (int)(Client->gameGUI.getTileSize()) - 1;

		d.x = screenPosition.x + 1;
		d.y = screenPosition.y + 1;
		d.w = max;
		d.h = 1;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.y += max - 1;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.y = screenPosition.y + 1;
		d.w = 1;
		d.h = max;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.x += max - 1;
		SDL_FillRect ( buffer, &d, nr );
	}

	// draw the group selected frame if necessary
	if ( groupSelected )
	{
		Uint32 color = 0xFFFF00;
		SDL_Rect d;

		d.w = (int)(Client->gameGUI.getTileSize())-2;
		d.h = 1;
		d.x = screenPosition.x + 1;
		d.y = screenPosition.y + 1;
		SDL_FillRect ( buffer, &d, color );

		d.w = (int)(Client->gameGUI.getTileSize())-2;
		d.h = 1;
		d.x = screenPosition.x + 1;
		d.y = screenPosition.y + (int)(Client->gameGUI.getTileSize())-1;
		SDL_FillRect ( buffer, &d, color );

		d.w = 1;
		d.h = (int)(Client->gameGUI.getTileSize())-2;
		d.x = screenPosition.x + 1;
		d.y = screenPosition.y + 1;
		SDL_FillRect ( buffer, &d, color );

		d.w = 1;
		d.h = (int)(Client->gameGUI.getTileSize())-2;
		d.x = screenPosition.x + (int)(Client->gameGUI.getTileSize())-1;
		d.y = screenPosition.y + 1;
		SDL_FillRect ( buffer, &d, color );
	}
	if ( Client->gameGUI.getSelVehicle() == this )
	{
		SDL_Rect d, t;
		int len, max;

		if ( ( IsBuilding || IsClearing ) && data.isBig ) max = (int)(Client->gameGUI.getTileSize()) * 2;
		else max = (int)(Client->gameGUI.getTileSize());

		len = max / 4;

		d.x = screenPosition.x + 1;
		d.y = screenPosition.y + 1;
		d.w = len;
		d.h = 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
		d = t;
		d.x += max - len - 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
		d = t;
		d.y += max - 2;
		t = d;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
		d = t;
		d.x = screenPosition.x + 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
		d = t;
		d.y = screenPosition.y + 1;
		d.w = 1;
		d.h = len;
		t = d;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
		d = t;
		d.x += max - 2;
		t = d;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
		d = t;
		d.y += max - len - 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
		d = t;
		d.x = screenPosition.x + 1;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
	}

	//draw health bar
	if ( Client->gameGUI.hitsChecked() )
		drawHealthBar();

	//draw ammo bar
	if ( Client->gameGUI.ammoChecked() && data.canAttack)
		DrawMunBar();

	//draw status info
	if ( Client->gameGUI.statusChecked() )
		drawStatus();

	//attack job debug output
	if ( Client->gameGUI.getAJobDebugStatus() )
	{
		cVehicle* serverVehicle = NULL;
		if ( Server ) serverVehicle = Server->Map->fields[PosX + PosY * Server->Map->size].getVehicles();
		if ( bIsBeeingAttacked ) font->showText(screenPosition.x + 1,screenPosition.y + 1, "C: attacked", FONT_LATIN_SMALL_WHITE );
		if ( serverVehicle && serverVehicle->bIsBeeingAttacked ) font->showText(screenPosition.x + 1,screenPosition.y + 9, "S: attacked", FONT_LATIN_SMALL_YELLOW );
		if ( Attacking ) font->showText(screenPosition.x + 1,screenPosition.y + 17, "C: attacking", FONT_LATIN_SMALL_WHITE );
		if ( serverVehicle && serverVehicle->Attacking ) font->showText(screenPosition.x + 1,screenPosition.y + 25, "S: attacking", FONT_LATIN_SMALL_YELLOW );
	}
}

void cVehicle::render( SDL_Surface* surface, const SDL_Rect& dest )
{
	//Note: when changing something in this function, make sure to update the caching rules!
	SDL_Rect src, tmp;

	float factor = (float)Client->gameGUI.getTileSize()/(float)64.0;

	//draw working engineers and bulldozers:
	if ( (IsBuilding || ( IsClearing && data.isBig )) && dir == 0 )
	{
		//draw beton if nessesary
		tmp = dest;
		if ( IsBuilding && data.isBig && ( !Client->Map->IsWater(PosX+PosY*Client->Map->size) || Client->Map->fields[PosX+PosY*Client->Map->size].getBaseBuilding()) )
		{
			SDL_SetAlpha ( GraphicsData.gfx_big_beton, SDL_SRCALPHA, BigBetonAlpha );
			CHECK_SCALING(GraphicsData.gfx_big_beton, GraphicsData.gfx_big_beton_org, factor );
			SDL_BlitSurface ( GraphicsData.gfx_big_beton, NULL, surface, &tmp );
		}

		// draw shadow
		tmp = dest;
		if ( SettingsData.bShadows ) blitWithPreScale ( typ->build_shw_org, typ->build_shw, NULL, surface, &tmp, factor );

		// draw player color
		src.y = 0;
		src.h = src.w = (int)(typ->build_org->h*factor);
		src.x = ( ANIMATION_SPEED % 4 ) * src.w;
		SDL_BlitSurface ( owner->color, NULL, GraphicsData.gfx_tmp, NULL );
		blitWithPreScale ( typ->build_org, typ->build, &src, GraphicsData.gfx_tmp, NULL, factor, 4 );

		// draw vehicle
		src.x = 0;
		src.y = 0;
		tmp = dest;
		SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, 255 );
		SDL_BlitSurface ( GraphicsData.gfx_tmp, &src, surface, &tmp );

		return;
	}

	if ( ( IsClearing && !data.isBig ) && dir == 0 )
	{
		// draw shadow
		tmp = dest;
		if ( SettingsData.bShadows )
			blitWithPreScale ( typ->clear_small_shw_org, typ->clear_small_shw, NULL, surface, &tmp, factor );

		// draw player color
		src.y = 0;
		src.h = src.w = (int)(typ->clear_small_org->h*factor);
		src.x = ( ANIMATION_SPEED % 4 ) * src.w;
		SDL_BlitSurface ( owner->color, NULL, GraphicsData.gfx_tmp, NULL );

		blitWithPreScale ( typ->clear_small_org, typ->clear_small, &src, GraphicsData.gfx_tmp, NULL, factor, 4 );

		// draw vehicle
		src.x = 0;
		src.y = 0;
		tmp = dest;
		SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, 255 );
		SDL_BlitSurface ( GraphicsData.gfx_tmp, &src, surface, &tmp );

		return;
	}

	//draw all other vehicles:

	// read the size:
	src.w = (int)(typ->img_org[dir]->w*Client->gameGUI.getZoom());
	src.h = (int)(typ->img_org[dir]->h*Client->gameGUI.getZoom());

	// draw shadow
	tmp = dest;
	if ( SettingsData.bShadows && ! ( (data.isStealthOn&TERRAIN_SEA) && Client->Map->IsWater ( PosX + PosY*Client->Map->size, true ) ) )
	{
		if ( StartUp && SettingsData.bAlphaEffects ) SDL_SetAlpha ( typ->shw[dir], SDL_SRCALPHA, StartUp / 5 );
		else SDL_SetAlpha ( typ->shw[dir], SDL_SRCALPHA, 50 );


		// draw shadow
		if ( FlightHigh > 0 )
		{
			int high = ( ( int ) ( (int)(Client->gameGUI.getTileSize()) * ( FlightHigh / 64.0 ) ) );
			tmp.x += high;
			tmp.y += high;

			blitWithPreScale ( typ->shw_org[dir], typ->shw[dir], NULL, surface, &tmp, Client->gameGUI.getZoom() );
		}
		else if ( data.animationMovement )
		{
			SDL_Rect r;
			r.h = r.w = (int) (typ->img_org[dir]->h*Client->gameGUI.getZoom());
			r.x = r.w * WalkFrame;
			r.y = 0;
			blitWithPreScale ( typ->shw_org[dir], typ->shw[dir], &r, surface, &tmp, Client->gameGUI.getZoom() );
		}
		else
			blitWithPreScale ( typ->shw_org[dir], typ->shw[dir], NULL, surface, &tmp, Client->gameGUI.getZoom() );
	}

	// draw player color
	SDL_BlitSurface ( owner->color, NULL, GraphicsData.gfx_tmp, NULL );

	if ( data.animationMovement )
	{
		src.w = src.h = tmp.h = tmp.w = (int) (typ->img_org[dir]->h*Client->gameGUI.getZoom());
		tmp.x = WalkFrame * tmp.w;
		tmp.y = 0;
		blitWithPreScale ( typ->img_org[dir], typ->img[dir], &tmp, GraphicsData.gfx_tmp, NULL, Client->gameGUI.getZoom() );
	}
	else
		blitWithPreScale ( typ->img_org[dir], typ->img[dir], NULL, GraphicsData.gfx_tmp, NULL, Client->gameGUI.getZoom() );


	// draw the vehicle
	src.x = 0;
	src.y = 0;
	tmp = dest;

	if ( StartUp && SettingsData.bAlphaEffects )
	{
		SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, StartUp );
	}
	else
	{
		bool water = Client->Map->IsWater(PosX + PosY*Client->Map->size, true);
		//if the vehicle can also drive on land, we have to check, whether there is a brige, platform, etc.
		//because the vehicle will drive on the bridge
		cBuilding* building = Client->Map->fields[PosX + PosY*Client->Map->size].getBaseBuilding();
		if ( building && data.factorGround > 0 && ( building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE ) ) water = false;

		if ( (data.isStealthOn&TERRAIN_SEA) && water && DetectedByPlayerList.Size() == 0 && owner == Client->ActivePlayer ) SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, 100 );
		else SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, 255 );
	}

	blittAlphaSurface ( GraphicsData.gfx_tmp, &src, surface, &tmp );

}

//-----------------------------------------------------------------------------
/** Selects the vehicle */
//-----------------------------------------------------------------------------
void cVehicle::Select ()
{
	// load the video
	if ( Client->gameGUI.getFLC() != NULL ) FLI_Close ( Client->gameGUI.getFLC() );
	if( FileExists(typ->FLCFile) )
	{
		Client->gameGUI.setFLC ( FLI_Open ( SDL_RWFromFile ( typ->FLCFile, "rb" ), NULL ) );
	}
	else
	{	//in case the flc video doesn't exist we use the storage image instead
		Client->gameGUI.setFLC ( NULL );
		Client->gameGUI.setVideoSurface ( typ->storage );
	}

	MakeReport();
	Client->gameGUI.setUnitDetailsData ( this, NULL );
}

//-----------------------------------------------------------------------------
/** Deselects the vehicle */
//-----------------------------------------------------------------------------
void cVehicle::Deselct ()
{
	SDL_Rect src, dest;
	groupSelected = false;
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
	// redraw the background
	src.x = 0;
	src.y = 215;
	src.w = 155;
	src.h = 48;
	dest.x = 8;
	dest.y = 171;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &src, GraphicsData.gfx_hud, &dest );
	StopFXLoop ( Client->iObjectStream );
	Client->iObjectStream = -1;
	Client->gameGUI.setFLC ( NULL );
	Client->gameGUI.setUnitDetailsData ( NULL, NULL );
}

//-----------------------------------------------------------------------------
/** Generates the name of the vehicle based on the version */
//-----------------------------------------------------------------------------
void cVehicle::GenerateName ()
{
	string rome, tmp_name;
	int nr, tmp;
	string::size_type tmp_name_idx;
	rome = "";
	nr = data.version + 1;	// +1, because the numbers in the name start at 1, not at 0

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

	// concatenate the name
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

//-----------------------------------------------------------------------------
/** Initializes all unit data to its maxiumum values */
//-----------------------------------------------------------------------------
int cVehicle::refreshData ()
{
	int iReturn = 0;
	if ( data.speedCur < data.speedMax || data.shotsCur < data.shotsMax )
	{
		data.speedCur = data.speedMax;

		if ( data.ammoCur >= data.shotsMax )
			data.shotsCur = data.shotsMax;
		else
			data.shotsCur = data.ammoCur;

		/*// Regenerieren:
		if ( data.is_alien && data.hitpointsCur < data.hitpointsMax )
		{
			data.hitpointsCur++;
		}*/
		iReturn = 1;
	}

	// Build
	if ( IsBuilding && BuildRounds )
	{

		data.storageResCur -= ( BuildCosts / BuildRounds );
		BuildCosts -= ( BuildCosts / BuildRounds );

		if ( data.storageResCur < 0 ) data.storageResCur = 0;

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

				//sidestep stealth unit if nessesary
				if ( (PosX != BandX || PosY != BandY) && !Server->Map->possiblePlace(this, nextX, nextY ) )
				{
					Server->sideStepStealthUnit(nextX, nextY, this );
				}

				if ( (PosX != BandX || PosY != BandY) && Server->addMoveJob( PosX + PosY*Server->Map->size, nextX + nextY*Server->Map->size, this ) )
				{
					IsBuilding = false;
					Server->addUnit( PosX, PosY, BuildingTyp.getBuilding(), owner );
					this->ServerMoveJob->checkMove();	//begin the movment immediately, so no other unit can block the destination field
				}
				else
				{
					if ( BuildingTyp.getUnitDataOriginalVersion()->surfacePosition != sUnitData::SURFACE_POS_GROUND )
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
				if ( BuildingTyp.getUnitDataOriginalVersion()->surfacePosition != data.surfacePosition )
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
			if ( data.isBig )
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
			data.storageResCur += Rubble->RubbleValue;
			if ( data.storageResCur > data.storageResMax) data.storageResCur = data.storageResMax;
			Server->deleteRubble ( Rubble );
		}

		iReturn = 1;
	}
	return iReturn;
}

//-----------------------------------------------------------------------------
/** Returns the x-position of the vehicle on the screen */
//-----------------------------------------------------------------------------
int cVehicle::GetScreenPosX() const
{
	return 180 - ( ( int ) ( ( Client->gameGUI.getOffsetX() - OffX ) * Client->gameGUI.getZoom() ) ) + (int)(Client->gameGUI.getTileSize())*PosX;
}

//-----------------------------------------------------------------------------
/** Returns the y-position of the vehicle on the screen */
//-----------------------------------------------------------------------------
int cVehicle::GetScreenPosY() const
{
	return 18 - ( ( int ) ( ( Client->gameGUI.getOffsetY() - OffY ) * Client->gameGUI.getZoom() ) ) + (int)(Client->gameGUI.getTileSize())*PosY;
}

//-----------------------------------------------------------------------------
/** Draws the path of the vehicle */
//-----------------------------------------------------------------------------
void cVehicle::DrawPath ()
{
	int mx = 0, my = 0, sp, save;
	SDL_Rect dest, ndest;
	sWaypoint *wp;

	if ( !ClientMoveJob || !ClientMoveJob->Waypoints || owner != Client->ActivePlayer )
	{
		if ( !BuildPath || ( BandX == PosX && BandY == PosY ) || PlaceBand ) return;

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
			dest.x = 180 - ( int ) ( Client->gameGUI.getOffsetX() * Client->gameGUI.getZoom() ) + Client->gameGUI.getTileSize() * mx;
			dest.y = 18 - ( int ) ( Client->gameGUI.getOffsetY() * Client->gameGUI.getZoom() ) + Client->gameGUI.getTileSize() * my;

			SDL_BlitSurface ( OtherData.WayPointPfeileSpecial[sp][64-Client->gameGUI.getTileSize()], NULL, buffer, &dest );

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

		dest.x = 180 - ( int ) ( Client->gameGUI.getOffsetX() * Client->gameGUI.getZoom() ) + Client->gameGUI.getTileSize() * mx;
		dest.y = 18 - ( int ) ( Client->gameGUI.getOffsetY() * Client->gameGUI.getZoom() ) + Client->gameGUI.getTileSize() * my;

		SDL_BlitSurface ( OtherData.WayPointPfeileSpecial[sp][64-Client->gameGUI.getTileSize()], NULL, buffer, &dest );
		return;
	}

	sp = data.speedCur;

	if ( sp )
	{
		save = 0;
		sp += ClientMoveJob->iSavedSpeed;
	}
	else save = ClientMoveJob->iSavedSpeed;

	dest.x = 180 - ( int ) ( Client->gameGUI.getOffsetX() * Client->gameGUI.getZoom() ) + Client->gameGUI.getTileSize() * PosX;
	dest.y = 18 - ( int ) ( Client->gameGUI.getOffsetY() * Client->gameGUI.getZoom() ) + Client->gameGUI.getTileSize() * PosY;
	wp = ClientMoveJob->Waypoints;
	ndest = dest;

	while ( wp )
	{
		if ( wp->next )
		{
			ndest.x += mx = wp->next->X * Client->gameGUI.getTileSize() - wp->X * Client->gameGUI.getTileSize();
			ndest.y += my = wp->next->Y * Client->gameGUI.getTileSize() - wp->Y * Client->gameGUI.getTileSize();
		}
		else
		{
			ndest.x += mx;
			ndest.y += my;
		}

		if ( sp == 0 )
		{
			ClientMoveJob->drawArrow ( dest, &ndest, true );
			sp += data.speedMax + save;
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

//-----------------------------------------------------------------------------
/** rotates the vehicle to the given direction */
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
/** Returns a string with the current state */
//-----------------------------------------------------------------------------
string cVehicle::getStatusStr ()
{
	if ( autoMJob )
		return lngPack.i18n ( "Text~Comp~Surveying" );
	else if ( ClientMoveJob )
		return lngPack.i18n ( "Text~Comp~Moving" );
	else if ( bSentryStatus )
		return lngPack.i18n ( "Text~Comp~Sentry" );
	else if ( IsBuilding )
	{
		if ( owner != Client->ActivePlayer )
			return lngPack.i18n ( "Text~Comp~Producing" );
		else
		{
			if ( BuildRounds )
			{
				string sText;
				sText = lngPack.i18n ( "Text~Comp~Producing" );
				sText += ": ";
				sText += ( string ) BuildingTyp.getUnitDataCurrentVersion ( owner )->name + " (";
				sText += iToStr ( BuildRounds );
				sText += ")";

				if ( font->getTextWide ( sText ) > 126 )
				{
					sText = lngPack.i18n ( "Text~Comp~Producing" );
					sText += ":\n";
					sText += ( string ) BuildingTyp.getUnitDataCurrentVersion ( owner )->name + " (";
					sText += iToStr ( BuildRounds );
					sText += ")";
				}
				return sText;
			}
			else
				return lngPack.i18n ( "Text~Comp~Producing_Fin" );
		}
	}
	else if ( ClearMines )
		return lngPack.i18n ( "Text~Comp~Clearing_Mine" );
	else if ( LayMines )
		return lngPack.i18n ( "Text~Comp~Laying" );
	else if ( IsClearing )
	{
		if ( ClearingRounds )
		{
			string sText;
			sText = lngPack.i18n ( "Text~Comp~Clearing" ) + " (";
			sText += iToStr ( ClearingRounds ) + ")";
			return sText;
		}
		else
			return lngPack.i18n ( "Text~Comp~Clearing_Fin" );
	}
	else if ( ( data.canCapture || data.canDisable ) && owner == Client->ActivePlayer )
	{
		string sTmp = lngPack.i18n ( "Text~Comp~Waits" ) + "\n";

		if ( CommandoRank < 1 ) sTmp += lngPack.i18n ( "Text~Comp~Greenhorn" );
		else if ( CommandoRank < 3 ) sTmp += lngPack.i18n ( "Text~Comp~Average" );
		else if ( CommandoRank < 6 ) sTmp += lngPack.i18n ( "Text~Comp~Veteran" );
		else if ( CommandoRank < 11 ) sTmp += lngPack.i18n ( "Text~Comp~Expert" );
		else sTmp += lngPack.i18n ( "Text~Comp~Elite" );
		if ( CommandoRank > 0 )
			sTmp += " +" + iToStr ( (int)CommandoRank );
		return sTmp;
	}
	else if ( Disabled )
	{
		string sText;
		sText = lngPack.i18n ( "Text~Comp~Disabled" ) + " (";
		sText += iToStr ( Disabled ) + ")";
		return sText;
	}

	return lngPack.i18n ( "Text~Comp~Waits" );
}

//-----------------------------------------------------------------------------
/** Plays the soundstream, that belongs to this vehicle */
//-----------------------------------------------------------------------------
int cVehicle::playStream ()
{
	cBuilding *building = (*Client->Map)[PosX + PosY*Client->Map->size].getBaseBuilding();
	bool water = Client->Map->IsWater ( PosX + PosY * Client->Map->size, true );
	if ( data.factorGround > 0 && building && ( building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA ) ) water = false;

	if ( IsBuilding && ( BuildRounds || Client->ActivePlayer != owner ))
		return PlayFXLoop ( SoundData.SNDBuilding );
	else if ( IsClearing )
		return PlayFXLoop ( SoundData.SNDClearing );
	else if ( water && data.factorSea > 0 )
		return PlayFXLoop ( typ->WaitWater );
	else
		return PlayFXLoop ( typ->Wait );
}

//-----------------------------------------------------------------------------
/** Starts the MoveSound */
//-----------------------------------------------------------------------------
void cVehicle::StartMoveSound ()
{
	bool water;
	// That's the moment, too, to hide the menu
	MenuActive = false;

	cBuilding* building = Client->Map->fields[PosX + PosY * Client->Map->size].getBaseBuilding();
	water = Client->Map->IsWater ( PosX + PosY * Client->Map->size, true );
	if ( data.factorGround > 0 && building && ( building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA ) ) water = false;
	StopFXLoop ( Client->iObjectStream );

	if ( !MoveJobActive )
	{
		if ( water && data.factorSea != 0 )
			PlayFX ( typ->StartWater );
		else
			PlayFX ( typ->Start );
	}

	if ( water && data.factorSea != 0 )
		Client->iObjectStream = PlayFXLoop ( typ->DriveWater );
	else
		Client->iObjectStream = PlayFXLoop ( typ->Drive );
}

void cVehicle::menuReleased ()
{
	int nr = 0, exeNr;
	SDL_Rect dest = GetMenuSize();
	if ( MouseOverMenu ( mouse->x, mouse->y ) ) exeNr = ( mouse->y - dest.y ) / 22;
	if ( exeNr != selMenuNr ) return;

	if ( moving || bIsBeeingAttacked ) return;

	// attack:
	if ( data.canAttack && data.shotsCur )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			AttackMode = !AttackMode;
			Client->gameGUI.updateMouseCursor ();
			return;
		}
		nr++;
	}

	// build:
	if ( !data.canBuild.empty() && !IsBuilding )
	{
		if ( exeNr == nr )
		{
			if ( ClientMoveJob ) sendWantStopMove ( iID );

			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			cBuildingsBuildMenu buildMenu ( owner, this );
			buildMenu.show();
			return;
		}
		nr++;
	}

	// transfer:
	if ( data.storeResType != sUnitData::STORE_RES_NONE && !IsBuilding && !IsClearing )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			Transfer = !Transfer;
			return;
		}
		nr++;
	}

	// auto
	if ( data.canSurvey )
	{
		if ( exeNr == nr )
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
		nr++;
	}

	// stop:
	if ( ClientMoveJob || ( IsBuilding && BuildRounds ) || ( IsClearing && ClearingRounds ) )
	{
		if ( exeNr == nr )
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
		nr++;
	}

	// clear:
	if ( data.canClearArea && Client->Map->fields[PosX+PosY*Client->Map->size].getRubble() && !IsClearing )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );

			sendWantStartClear ( this );
			return;
		}
		nr++;
	}

	// sentry:
	if ( bSentryStatus || data.canAttack )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			sendChangeSentry ( iID, true );
			return;
		}
		nr++;
	}

	// activate/load:
	if ( data.storageUnitsMax > 0 )
	{
		// activatew:
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );

			cStorageMenu storageMenu ( StoredVehicles, this, NULL );
			storageMenu.show();
			return;
		}
		nr++;

		// load:
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			LoadActive = !LoadActive;
			return;
		}
		nr++;
	}

	// rearm:
	if ( data.canRearm && data.storageResCur >= 2 )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			MuniActive = !MuniActive;
			return;
		}
		nr++;
	}

	// repair:
	if ( data.canRepair && data.storageResCur >= 2 )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			RepairActive = !RepairActive;
			return;
		}
		nr++;
	}

	// lay mines:
	if ( data.canPlaceMines && data.storageResCur > 0 )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			LayMines = !LayMines;
			ClearMines = false;
			sendMineLayerStatus( this );
			return;
		}
		nr++;
	}

	// clear mines:
	if ( data.canPlaceMines && data.storageResCur < data.storageResMax )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			ClearMines = !ClearMines;
			LayMines = false;
			sendMineLayerStatus ( this );
			return;
		}
		nr++;
	}

	// disable:
	if ( data.canDisable && data.shotsCur )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			DisableActive = !DisableActive;
			StealActive = false;
			return;
		}
		nr++;
	}

	// steal:
	if ( data.canCapture && data.shotsCur )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			StealActive = !StealActive;
			DisableActive = false;
			return;
		}
		nr++;
	}

	// help:
	if ( exeNr == nr )
	{
		MenuActive = false;
		PlayFX ( SoundData.SNDObjectMenu );
		cUnitHelpMenu helpMenu ( &data, owner );
		helpMenu.show();
		return;
	}
	nr++;

	// done:
	if ( exeNr == nr )
	{
		MenuActive = false;
		PlayFX ( SoundData.SNDObjectMenu );
		return;
	}
}

//-----------------------------------------------------------------------------
/** Draws the vehicle menu: */
//-----------------------------------------------------------------------------
void cVehicle::DrawMenu ( sMouseState *mouseState )
{
	int nr = 0;
	static int LastselMenuNr = -1;
	bool bSelection = false;
	SDL_Rect dest = GetMenuSize();

	if ( moving || bIsBeeingAttacked ) return;

	if ( mouseState && mouseState->leftButtonPressed && MouseOverMenu ( mouse->x, mouse->y ) && ( ( selMenuNr == -1 && LastselMenuNr == -1 ) || LastselMenuNr == ( mouse->y - dest.y ) / 22 ) )
	{
		selMenuNr = ( mouse->y - dest.y ) / 22;
	}
	else if ( mouseState && mouseState->leftButtonPressed && MouseOverMenu ( mouse->x, mouse->y ) && selMenuNr != -1 && selMenuNr != ( mouse->y - dest.y ) / 22 )
	{
		LastselMenuNr = selMenuNr;
		selMenuNr = -1;
	}

	// Angriff:
	if ( data.canAttack && data.shotsCur )
	{
		bSelection = selMenuNr == nr || AttackMode;

		drawContextItem( lngPack.i18n ( "Text~Context~Attack" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Bauen:
	if ( !data.canBuild.empty() && !IsBuilding )
	{
		bSelection = selMenuNr == nr;

		drawContextItem( lngPack.i18n ( "Text~Context~Build" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Transfer:
	if ( data.storeResType != sUnitData::STORE_RES_NONE && !IsBuilding && !IsClearing )
	{
		bSelection = selMenuNr == nr || Transfer;

		drawContextItem( lngPack.i18n ( "Text~Context~Transfer" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Auto
	if ( data.canSurvey )
	{
		if ( ( autoMJob == NULL && selMenuNr == nr ) || ( autoMJob != NULL && selMenuNr != nr ) )
		{
			bSelection = true;
		}
		else
		{
			bSelection = false;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Auto" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Stop:
	if ( ClientMoveJob || ( IsBuilding && BuildRounds ) || ( IsClearing && ClearingRounds ) )
	{
		bSelection = selMenuNr == nr;

		drawContextItem( lngPack.i18n ( "Text~Context~Stop" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Entfernen:
	if ( data.canClearArea && Client->Map->fields[PosX+PosY*Client->Map->size].getRubble() && !IsClearing )
	{
		bSelection = selMenuNr == nr;


		drawContextItem( lngPack.i18n ( "Text~Context~Clear" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Sentry:
	if ( bSentryStatus || data.canAttack )
	{
		bSelection = selMenuNr == nr || bSentryStatus;

		drawContextItem( lngPack.i18n ( "Text~Context~Sentry" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Aktivieren/Laden:
	if ( data.storageUnitsMax > 0 )
	{
		// Aktivieren:
		bSelection = selMenuNr == nr;

		drawContextItem( lngPack.i18n ( "Text~Context~Active" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
		// Laden:
		bSelection = selMenuNr == nr || LoadActive;

		drawContextItem( lngPack.i18n ( "Text~Context~Load" ), bSelection, dest.x, dest.y, buffer );
		dest.y += 22;
		nr++;
	}

	// Aufaden:
	if ( data.canRearm && data.storageResCur >= 2 )
	{
		bSelection = selMenuNr == nr || MuniActive;

		drawContextItem( lngPack.i18n ( "Text~Context~Reload" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Reparatur:
	if ( data.canRepair && data.storageResCur >= 2 )
	{
		bSelection = selMenuNr == nr || RepairActive;

		drawContextItem( lngPack.i18n ( "Text~Context~Repair" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Minen legen:
	if ( data.canPlaceMines && data.storageResCur > 0 )
	{
		bSelection = selMenuNr == nr || LayMines;

		drawContextItem( lngPack.i18n ( "Text~Context~Seed" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Minen sammeln:
	if ( data.canPlaceMines && data.storageResCur < data.storageResMax )
	{
		bSelection = selMenuNr == nr || ClearMines;

		drawContextItem( lngPack.i18n ( "Text~Context~Clear" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Sabotage:
	if ( data.canDisable && data.shotsCur )
	{
		bSelection = selMenuNr == nr || DisableActive;

		drawContextItem( lngPack.i18n ( "Text~Context~Disable" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Stehlen:
	if ( data.canCapture && data.shotsCur )
	{
		bSelection = selMenuNr == nr || StealActive;

		drawContextItem( lngPack.i18n ( "Text~Context~Steal" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Info:
	bSelection = selMenuNr == nr;
	drawContextItem( lngPack.i18n ( "Text~Context~Info" ), bSelection, dest.x, dest.y, buffer );
	dest.y += 22;
	nr++;

	// Fertig:
	bSelection = selMenuNr == nr;
	drawContextItem( lngPack.i18n ( "Text~Context~Done" ), bSelection, dest.x, dest.y, buffer );
}

//-----------------------------------------------------------------------------
/** Returns the number of points in the menu: */
//-----------------------------------------------------------------------------
int cVehicle::GetMenuPointAnz ()
{
	int nr = 2;

	if ( !data.canBuild.empty() && !IsBuilding )
		nr++;

	if ( data.canSurvey )
		nr++;

	if ( data.storeResType != sUnitData::STORE_RES_NONE && !IsBuilding && !IsClearing )
		nr++;

	if ( data.canAttack && data.shotsCur )
		nr++;

	if ( ClientMoveJob || ( IsBuilding && BuildRounds ) || ( IsClearing && ClearingRounds ) )
		nr++;

	if ( data.canClearArea && Client->Map->fields[PosX+PosY*Client->Map->size].getRubble() && !IsClearing )
		nr++;

	if ( bSentryStatus || data.canAttack )
		nr++;

	if ( data.storageUnitsMax > 0 )
		nr += 2;

	if ( data.canRearm && data.storageResCur >= 2 )
		nr++;

	if ( data.canRepair && data.storageResCur >= 2 )
		nr++;

	if ( data.canPlaceMines && data.storageResCur > 0 )
		nr++;

	if ( data.canPlaceMines && data.storageResCur < data.storageResMax )
		nr++;

	if ( data.canCapture && data.shotsCur )
		nr++;

	if ( data.canDisable && data.shotsCur )
		nr++;

	return nr;
}

//-----------------------------------------------------------------------------
/** Returns the size of the menu and the position */
//-----------------------------------------------------------------------------
SDL_Rect cVehicle::GetMenuSize ()
{
	SDL_Rect dest;
	int i, size;
	dest.x = GetScreenPosX();
	dest.y = GetScreenPosY();
	dest.h = i = GetMenuPointAnz() * 22;
	dest.w = 42;
	size = Client->gameGUI.getTileSize();

	if ( IsBuilding && BuildingTyp.getUnitDataOriginalVersion()->isBig )
		size *= 2;

	if ( dest.x + size + 42 >= SettingsData.iScreenW - 12 )
		dest.x -= 42;
	else
		dest.x += size;

	if ( dest.y - ( i - size ) / 2 <= 24 )
	{
		dest.y -= ( i - size ) / 2;
		dest.y += - ( dest.y - 24 );
	}
	else if ( dest.y - ( i - size ) / 2 + i >= SettingsData.iScreenH - 24 )
	{
		dest.y -= ( i - size ) / 2;
		dest.y -= ( dest.y + i ) - ( SettingsData.iScreenH - 24 );
	}
	else
		dest.y -= ( i - size ) / 2;

	return dest;
}

//-----------------------------------------------------------------------------
/** Returns true, if the mouse coordinates are in the menu's space */
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
/** Reduces the remaining speedCur and shotsCur during movement */
//-----------------------------------------------------------------------------
void cVehicle::DecSpeed ( int value )
{
	data.speedCur -= value;

	if ( data.canAttack )
	{
		float f;
		int s;
		f = ( ( float ) data.shotsMax / data.speedMax );
		s = ( int ) ( data.speedCur * f );

		if ( !data.canDriveAndFire && s < data.shotsCur && s >= 0 )
			data.shotsCur = s;
	}
}

//-----------------------------------------------------------------------------
/** Draws the ammunition bar over the vehicle */
//-----------------------------------------------------------------------------
void cVehicle::DrawMunBar() const
{
	SDL_Rect r1, r2;
	r1.x = GetScreenPosX() + Client->gameGUI.getTileSize()/10 + 1;
	r1.w = Client->gameGUI.getTileSize() * 8 / 10 ;
	r1.h = Client->gameGUI.getTileSize() / 8;
	r1.y = GetScreenPosY() + Client->gameGUI.getTileSize()/10 + Client->gameGUI.getTileSize() / 8;

	if ( r1.h <= 2  )
	{
		r1.y += 1;
		r1.h = 3;
	}

	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.h = r1.h - 2;
	r2.w = ( int ) ( ( ( float ) ( r1.w - 2 ) / data.ammoMax ) * data.ammoCur );

	SDL_FillRect ( buffer, &r1, 0 );

	if ( data.ammoCur > data.ammoMax / 2 )
	{
		SDL_FillRect ( buffer, &r2, 0x04AE04 );
	}
	else if ( data.ammoCur > data.ammoMax / 4 )
	{
		SDL_FillRect ( buffer, &r2, 0xDBDE00 );
	}
	else
	{
		SDL_FillRect ( buffer, &r2, 0xE60000 );
	}
}

//-----------------------------------------------------------------------------
/** Draws the hitpoints bar over the vehicle */
//-----------------------------------------------------------------------------
void cVehicle::drawHealthBar() const
{
	SDL_Rect r1, r2;
	r1.x = GetScreenPosX() + Client->gameGUI.getTileSize()/10 + 1;
	r1.w = Client->gameGUI.getTileSize()* 8 / 10 ;
	r1.h = Client->gameGUI.getTileSize() / 8;
	r1.y = GetScreenPosY() + Client->gameGUI.getTileSize()/10;

	if ( r1.h <= 2  )
		r1.h = 3;

	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.h = r1.h - 2;
	r2.w = ( int ) ( ( ( float ) ( r1.w - 2 ) / data.hitpointsMax ) * data.hitpointsCur );

	SDL_FillRect ( buffer, &r1, 0 );

	if ( data.hitpointsCur > data.hitpointsMax / 2 )
	{
		SDL_FillRect ( buffer, &r2, 0x04AE04 );
	}
	else if ( data.hitpointsCur > data.hitpointsMax / 4 )
	{
		SDL_FillRect ( buffer, &r2, 0xDBDE00 );
	}
	else
	{
		SDL_FillRect ( buffer, &r2, 0xE60000 );
	}
}

//-----------------------------------------------------------------------------
void cVehicle::drawStatus() const
{
	SDL_Rect dest;
	SDL_Rect speedSymbol = {244, 97, 8, 10 };
	SDL_Rect shotsSymbol = {254, 97, 5, 10 };
	SDL_Rect disabledSymbol = {150, 109, 25, 25};

	if ( Disabled )
	{
		if ( Client->gameGUI.getTileSize() < 25 ) return;
		dest.x = GetScreenPosX() + Client->gameGUI.getTileSize()/2 - 12;
		dest.y = GetScreenPosY() + Client->gameGUI.getTileSize()/2 - 12;
		SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &disabledSymbol, buffer, &dest );
	}
	else
		{
		dest.y = GetScreenPosY() + Client->gameGUI.getTileSize() - 11;
		dest.x = GetScreenPosX() + Client->gameGUI.getTileSize()/2 - 4;
		if ( data.isBig )
		{
			dest.y += (Client->gameGUI.getTileSize()/2);
			dest.x += (Client->gameGUI.getTileSize()/2);
		}
		if ( data.speedCur >= 4 )
		{
			if ( data.shotsCur ) dest.x -= Client->gameGUI.getTileSize()/4;
			SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &speedSymbol, buffer, &dest );
		}

		dest.x = GetScreenPosX() + Client->gameGUI.getTileSize()/2 - 4;
		if ( data.shotsCur )
		{
			if ( data.speedCur ) dest.x += Client->gameGUI.getTileSize()/4;
			SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &shotsSymbol, buffer, &dest );
		}
	}
}

//-----------------------------------------------------------------------------
/** Centers on this vehicle */
//-----------------------------------------------------------------------------
void cVehicle::Center ()
{
	int offX = PosX * 64 - ( ( int ) ( ( ( float ) (SettingsData.iScreenW - 192) / (2 * Client->gameGUI.getTileSize() ) ) * 64 ) ) + 32;
	int offY = PosY * 64 - ( ( int ) ( ( ( float ) (SettingsData.iScreenH - 32 ) / (2 * Client->gameGUI.getTileSize() ) ) * 64 ) ) + 32;
	Client->gameGUI.setOffsetPosition ( offX, offY );
}

//-----------------------------------------------------------------------------
/** Checks, if the vehicle can attack the object */
//-----------------------------------------------------------------------------

bool cVehicle::CanAttackObject ( int off, cMap *Map, bool override, bool checkRange )
{
	if ( Loaded )
		return false;

	if ( !data.canAttack )
		return false;

	if ( !data.shotsCur )
		return false;

	if ( !data.ammoCur )
		return false;

	if ( Attacking )
		return false;

	if ( bIsBeeingAttacked )
		return false;

	if ( off < 0 )
		return false;

	if ( checkRange && !IsInRange ( off, Map ) )
		return false;

	if ( data.muzzleType == sUnitData::MUZZLE_TYPE_TORPEDO && !Map->IsWater( off ) )
		return false;

	if ( !owner->ScanMap[off] )
		return override?true:false;

	if ( override )
		return true;

	cVehicle *targetVehicle = NULL;
	cBuilding *targetBuilding = NULL;

	selectTarget( targetVehicle, targetBuilding, off, data.canAttack, Map );

	if ( targetVehicle )
	{
		if ( Client && ( targetVehicle == Client->gameGUI.getSelVehicle() || targetVehicle->owner == Client->ActivePlayer ) )
			return false;
	}
	else if ( targetBuilding )
	{
		if ( Client && ( targetBuilding == Client->gameGUI.getSelBuilding() || targetBuilding->owner == Client->ActivePlayer ) )
			return false;
	}
	else
		return false;

	return true;
}

//-----------------------------------------------------------------------------
/** Checks, if the target lies in range */
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
/** Draws the attack cursor */
//-----------------------------------------------------------------------------
void cVehicle::DrawAttackCursor ( int offset )
{
	SDL_Rect r;
	int wp, wc, t;
	cVehicle *v;
	cBuilding *b;

	selectTarget(v, b, offset, data.canAttack, Client->Map );

	if ( !(v || b) || ( v && v == Client->gameGUI.getSelVehicle() ) || ( b && b == Client->gameGUI.getSelBuilding() ) )
	{
		r.x = 1;
		r.y = 29;
		r.h = 3;
		r.w = 35;
		SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0 );
		return;
	}

	if ( v )
		t = v->data.hitpointsCur;
	else
		t = b->data.hitpointsCur;

	if ( t )
	{
		if ( v )
			wc = ( int ) ( ( float ) t / v->data.hitpointsMax * 35 );
		else
			wc = ( int ) ( ( float ) t / b->data.hitpointsMax * 35 );
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
			wp = ( int ) ( ( float ) t / v->data.hitpointsMax * 35 );
		else
			wp = ( int ) ( ( float ) t / b->data.hitpointsMax * 35 );
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

//-----------------------------------------------------------------------------
/** returns the remaining hitpoints after an attack */
//-----------------------------------------------------------------------------
int cVehicle::CalcHelth ( int damage )
{
	damage -= data.armor;

	if ( damage <= 0 )
	{
		//minimum damage is 1
		damage = 1;
	}

	int hp;
	hp = data.hitpointsCur - damage;

	if ( hp < 0 )
	{
		return 0;
	}

	return hp;
}


//-----------------------------------------------------------------------------
void cVehicle::calcTurboBuild(int* const iTurboBuildRounds, int* const iTurboBuildCosts, int iBuild_Costs )
{
	// calculate building time and costs

	iTurboBuildRounds[0] = 0;
	iTurboBuildRounds[1] = 0;
	iTurboBuildRounds[2] = 0;

	//prevent division by zero
	if ( data.needsMetal == 0 ) data.needsMetal = 1;

	//step 1x
	if ( data.storageResCur >= iBuild_Costs )
	{
		iTurboBuildCosts[0] = iBuild_Costs;
		iTurboBuildRounds[0] = ( int ) ceil ( iTurboBuildCosts[0] / ( double ) ( data.needsMetal ) );
	}

	//step 2x
	if ( ( iTurboBuildRounds[0] > 1 ) && ( data.storageResCur >= iTurboBuildCosts[0] + 4 ) )
	{
		iTurboBuildRounds[1] = iTurboBuildRounds[0];
		iTurboBuildCosts[1] = iTurboBuildCosts[0];

		while ( ( data.storageResCur >= iTurboBuildCosts[1] + 4 ) && ( iTurboBuildRounds[1] > 1 ) )
		{
			iTurboBuildRounds[1]--;
			iTurboBuildCosts[1] += 4;

			if ( iTurboBuildCosts[1] + 4 > 2*iTurboBuildCosts[0] )
				break;
		}
	}

	//step 4x
	if (
		( iTurboBuildRounds[1] > 1 )
		&& ( iTurboBuildCosts[1] <= 48 )
		&& ( data.storageResCur >= iTurboBuildCosts[1] + 8 )
		&& ( data.storageResCur >= 24 )
		)
	{
		iTurboBuildRounds[2] = iTurboBuildRounds[1];
		iTurboBuildCosts[2] = iTurboBuildCosts[1];

		while (
			( data.storageResCur >= iTurboBuildCosts[2] + 8 )
			&& ( iTurboBuildRounds[2] > 1 )
			&& ( iTurboBuildCosts[2] <= 48 )
			&& ( (iTurboBuildRounds[2]-1)*2 >= iTurboBuildRounds[1] )
			&& ( data.storageResCur >= (iTurboBuildRounds[1] - iTurboBuildRounds[2] + 1) * 24 )
			)
		{
			iTurboBuildRounds[2]--;
			iTurboBuildCosts[2] += 8;

		}
		if ( (iTurboBuildRounds[1] - iTurboBuildRounds[2]) * 24 > iTurboBuildCosts[2] )
			iTurboBuildCosts[2] = (iTurboBuildRounds[1] - iTurboBuildRounds[2]) * 24;
	}
}

//-----------------------------------------------------------------------------
/** Finds the next fitting position for the band */
//-----------------------------------------------------------------------------
void cVehicle::FindNextband ()
{
	bool pos[4] = {false, false, false, false};
	int x, y;
	mouse->GetKachel ( &x, &y );

	//check, which positions are available
	sUnitData BuildingType = *BuildingTyp.getUnitDataOriginalVersion();
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

	if ( BuildingTyp.getUnitDataOriginalVersion()->isBig )
	{
		BandX = PosX;
		BandY = PosY;
		return;
	}

	//PlaceBand = false;
}

//-----------------------------------------------------------------------------
/** Scans for resources ( This function is only called by the server) */
//-----------------------------------------------------------------------------
void cVehicle::doSurvey ()
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

//-----------------------------------------------------------------------------
/** Makes the report */
//-----------------------------------------------------------------------------
void cVehicle::MakeReport ()
{
	if ( owner != Client->ActivePlayer )
		return;

	if ( Disabled )
	{
		// Disabled:
		PlayVoice ( VoiceData.VOIDisabled );
	}
	else if ( data.hitpointsCur > data.hitpointsMax / 2 )
	{
		// Status green
		if ( data.speedCur == 0 )
		{
			// no more movement
			PlayVoice ( VoiceData.VOINoSpeed );
		}
		else if ( IsBuilding )
		{
			// Beim bau:
			if ( !BuildRounds )
			{
				// Bau beendet:
				if (random(2))
					PlayVoice ( VoiceData.VOIBuildDone1 );
				else
					PlayVoice ( VoiceData.VOIBuildDone2 );
			}
		}
		else if ( IsClearing )
		{
			// removing dirt
			if ( ClearingRounds )
				PlayVoice ( VoiceData.VOIClearing );
			else
				PlayVoice ( VoiceData.VOIOK2 );
		}
		else if ( data.canAttack && !data.ammoCur )
		{
			// no ammo
			if (random(2))
				PlayVoice ( VoiceData.VOILowAmmo1 );
			else
				PlayVoice ( VoiceData.VOILowAmmo2 );
		}
		else if ( bSentryStatus )
		{
			// on sentry:
			PlayVoice ( VoiceData.VOIWachposten );
		}
		else if ( ClearMines )
			PlayVoice ( VoiceData.VOIClearingMines );
		else if ( LayMines )
			PlayVoice ( VoiceData.VOILayingMines );
		else
		{
			int nr;
			// Alles OK:
			nr = random(3);

			if ( nr == 0 )
				PlayVoice ( VoiceData.VOIOK1 );
			else if ( nr == 1 )
				PlayVoice ( VoiceData.VOIOK2 );
			else
				PlayVoice ( VoiceData.VOIOK3 );
		}
	}
	else if ( data.hitpointsCur > data.hitpointsMax / 4 )
		// Status yellow:
		PlayVoice ( VoiceData.VOIStatusYellow );
	else
		// Status red:
		PlayVoice ( VoiceData.VOIStatusRed );
}

//-----------------------------------------------------------------------------
/** checks, if resources can be transfered to the unit */
//-----------------------------------------------------------------------------
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

		if ( v->data.storeResType != data.storeResType )
			return false;

		if ( v->IsBuilding || v->IsClearing )
			return false;

		return true;
	}
	else if ( OverUnitField->getTopBuilding() )
	{
		b = OverUnitField->getTopBuilding();

		if ( b->owner != Client->ActivePlayer )
			return false;

		if ( !b->SubBase )
			return false;

		if ( data.storeResType == sUnitData::STORE_RES_METAL && b->SubBase->MaxMetal == 0 )
			return false;

		if ( data.storeResType == sUnitData::STORE_RES_OIL && b->SubBase->MaxOil == 0 )
			return false;

		if ( data.storeResType == sUnitData::STORE_RES_GOLD && b->SubBase->MaxGold == 0 )
			return false;

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
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

		if ( data.isStealthOn != TERRAIN_NONE && !isDetectedByPlayer( Player ) ) continue;	//check next player

		if ( data.factorAir > 0 )
		{
			if ( ! ( Player->SentriesMapAir[iOff] && Player->ScanMap[iOff] ) ) continue; //check next player

			for ( unsigned int k = 0; k < Player->SentriesAir.Size(); k++ )
			{
				Sentry = Player->SentriesAir[k];

				if ( Sentry->b && Sentry->b->CanAttackObject ( iOff, Server->Map, true ) )
				{
					cVehicle* targetVehicle;
					cBuilding* targetBuilding;
					selectTarget( targetVehicle, targetBuilding, iOff, Sentry->b->data.canAttack, Server->Map );
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
					selectTarget( targetVehicle, targetBuilding, iOff, Sentry->v->data.canAttack, Server->Map );
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
					selectTarget( targetVehicle, targetBuilding, iOff, Sentry->b->data.canAttack, Server->Map );
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
					selectTarget( targetVehicle, targetBuilding, iOff, Sentry->v->data.canAttack, Server->Map );
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

//-----------------------------------------------------------------------------
/** Draws exitpoints for a vehicle, that should be exited */
//-----------------------------------------------------------------------------
void cVehicle::DrawExitPoints(sVehicle* const typ) const
{
	int spx, spy, size;
	spx = GetScreenPosX();
	spy = GetScreenPosY();
	size = Client->Map->size;

	if ( canExitTo ( PosX - 1, PosY - 1, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx - Client->gameGUI.getTileSize(), spy - Client->gameGUI.getTileSize() );
	if ( canExitTo ( PosX    , PosY - 1, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx, spy - Client->gameGUI.getTileSize() );
	if ( canExitTo ( PosX + 1, PosY - 1, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx + Client->gameGUI.getTileSize(), spy - Client->gameGUI.getTileSize() );
	if ( canExitTo ( PosX - 1, PosY    , Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx - Client->gameGUI.getTileSize(), spy );
	if ( canExitTo ( PosX + 1, PosY    , Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx + Client->gameGUI.getTileSize(), spy );
	if ( canExitTo ( PosX - 1, PosY + 1, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx - Client->gameGUI.getTileSize(), spy + Client->gameGUI.getTileSize() );
	if ( canExitTo ( PosX    , PosY + 1, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx, spy + Client->gameGUI.getTileSize() );
	if ( canExitTo ( PosX + 1, PosY + 1, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx + Client->gameGUI.getTileSize(), spy + Client->gameGUI.getTileSize() );
}

//-----------------------------------------------------------------------------
bool cVehicle::canExitTo ( const int x, const int y, const cMap* map, const sVehicle *typ ) const
{
	if ( !map->possiblePlaceVehicle( typ->data, x, y ) ) return false;
	if ( data.factorAir > 0 && ( x != PosX || y != PosY ) ) return false;
	if ( !isNextTo(x, y) ) return false;

	return true;
}

//-----------------------------------------------------------------------------
bool cVehicle::canLoad ( int off, cMap *Map, bool checkPosition )
{
	if ( off < 0 || off > Map->size*Map->size ) return false;

	return canLoad ( Map->fields[off].getVehicles(), checkPosition );
}

//-----------------------------------------------------------------------------
bool cVehicle::canLoad ( cVehicle *Vehicle, bool checkPosition )
{
	if ( !Vehicle ) return false;

	if ( Vehicle->Loaded ) return false;

	if ( data.storageUnitsCur >= data.storageUnitsMax )	return false;

	if ( checkPosition && !isNextTo ( Vehicle->PosX, Vehicle->PosY ) ) return false;

	if ( checkPosition && data.factorAir > 0 && ( Vehicle->PosX != PosX || Vehicle->PosY != PosY ) ) return false;

	int i;
	for ( i = 0; i < (int)data.storeUnitsTypes.size(); i++ )
	{
		if ( data.storeUnitsTypes[i].compare ( Vehicle->data.isStorageType ) == 0 ) break;
	}
	if ( i == data.storeUnitsTypes.size() ) return false;

	if ( Vehicle->ClientMoveJob && ( Vehicle->moving || Vehicle->Attacking || Vehicle->MoveJobActive ) ) return false;

	if ( Vehicle->owner == owner && !Vehicle->IsBuilding && !Vehicle->IsClearing ) return true;

	if ( Vehicle->bIsBeeingAttacked ) return false;

	return false;
}

//-----------------------------------------------------------------------------
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
	data.storageUnitsCur++;

	if ( data.storageUnitsCur == data.storageUnitsMax ) LoadActive = false;

	owner->DoScan();
}

//-----------------------------------------------------------------------------
/** Exits a vehicle */
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

	data.storageUnitsCur--;

	ActivatingVehicle = false;

	Map->addVehicle ( Vehicle, offset );

	Vehicle->PosX = offset % Map->size;
	Vehicle->PosY = offset / Map->size;
	Vehicle->Loaded = false;
	//Vehicle->data.shotsCur = 0;

	owner->DoScan();
}

//-----------------------------------------------------------------------------
/** Checks, if an object can get ammunition. */
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

	if ( data.storageResCur <= 0 ) return false;

	if ( Vehicle->PosX > PosX+1 || Vehicle->PosX < PosX-1 || Vehicle->PosY > PosY+1 || Vehicle->PosY < PosY-1 ) return false;

	if ( Vehicle->data.factorAir > 0 && Vehicle->FlightHigh > 0 ) return false;

	switch ( iType )
	{
	case SUPPLY_TYPE_REARM:
		if ( Vehicle == this || !Vehicle->data.canAttack || Vehicle->data.ammoCur >= Vehicle->data.ammoMax || Vehicle->moving || Vehicle->Attacking ) return false;
		break;
	case SUPPLY_TYPE_REPAIR:
		if ( Vehicle == this || Vehicle->data.hitpointsCur >= Vehicle->data.hitpointsMax || Vehicle->moving || Vehicle->Attacking ) return false;
		break;
	default:
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool cVehicle::canSupply( cBuilding *Building, int iType )
{
	if ( data.storageResCur <= 0 ) return false;

	if ( !Building->data.isBig )
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
		if ( !Building->data.canAttack || Building->data.ammoCur >= Building->data.ammoMax ) return false;
		break;
	case SUPPLY_TYPE_REPAIR:
		if ( Building->data.hitpointsCur >= Building->data.hitpointsMax ) return false;
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
	data.version = upgradeVersion.version;

	if (data.hitpointsCur == data.hitpointsMax)
		data.hitpointsCur = upgradeVersion.hitpointsMax; // TODO: check behaviour in original
	data.hitpointsMax = upgradeVersion.hitpointsMax;

	data.ammoMax = upgradeVersion.ammoMax; // don't change the current ammo-amount!

	data.speedMax = upgradeVersion.speedMax;

	data.armor = upgradeVersion.armor;
	data.scan = upgradeVersion.scan;
	data.range = upgradeVersion.range;
	data.shotsMax = upgradeVersion.shotsMax; // TODO: check behaviour in original
	data.damage = upgradeVersion.damage;
	data.buildCosts = upgradeVersion.buildCosts;

	GenerateName();
}

//-----------------------------------------------------------------------------
bool cVehicle::layMine ()
{
	if ( data.storageResCur <= 0 ) return false;

	if ( ( data.factorSea > 0 && data.factorGround == 0 ) )
	{
		if ( !Server->Map->possiblePlaceBuilding( *specialIDSeaMine.getUnitDataOriginalVersion(), PosX, PosY, this)) return false;
		Server->addUnit(PosX, PosY, specialIDSeaMine.getBuilding(), owner, false);
	}
	else
	{
		if ( !Server->Map->possiblePlaceBuilding( *specialIDLandMine.getUnitDataOriginalVersion(), PosX, PosY, this)) return false;
		Server->addUnit(PosX, PosY, specialIDLandMine.getBuilding(), owner, false);
	}
	data.storageResCur--;

	if ( data.storageResCur <= 0 ) LayMines = false;

	return true;
}

//-----------------------------------------------------------------------------
bool cVehicle::clearMine ()
{
	cBuilding* Mine = Server->Map->fields[PosX+PosY*Server->Map->size].getMine();

	if ( !Mine || Mine->owner != owner || data.storageResCur >= data.storageResMax ) return false;
	if ( Mine->data.factorGround > 0 && data.factorGround == 0 ) return false;
	if ( Mine->data.factorSea > 0 && data.factorSea == 0 ) return false;

	Server->deleteUnit ( Mine );
	data.storageResCur++;

	if ( data.storageResCur >= data.storageResMax ) ClearMines = false;

	return true;
}

//-----------------------------------------------------------------------------
/** Checks if the target is on a neighbour field and if it can be stolen or disabled */
//-----------------------------------------------------------------------------
bool cVehicle::canDoCommandoAction ( int x, int y, cMap *map, bool steal )
{
	if ( steal && !data.canCapture ) return false;
	if ( !steal && !data.canDisable ) return false;

	int off, boff;
	off = x + y * map->size;
	boff = PosX + PosY * map->size;

	if ( !isNextTo ( x, y ) ) return false;
	if ( !data.shotsCur ) return false;

	cVehicle*  vehicle  = map->fields[off].getVehicles();
	cBuilding* building = map->fields[off].getBuildings();

	bool result = true;
	if ( vehicle )
	{
		if ( steal && !vehicle->data.canBeCaptured ) result = false;
		if ( !steal && !vehicle->data.canBeDisabled ) result = false;

		if ( vehicle->data.factorAir > 0 && vehicle->FlightHigh > 0 ) result = false;
		if ( steal && vehicle->StoredVehicles.Size() ) result = false;

		if ( vehicle->owner == owner ) result = false;

		if (result) return true;
	}

	if ( building )
	{
		if ( !building->owner ) result = false;
		if ( steal && !building->data.canBeCaptured ) result = false;
		if ( !steal && !building->data.canBeDisabled ) result = false;

		if ( steal && building->StoredVehicles.Size() ) result = false;
		if ( building->owner == owner ) result = false;

		if (result) return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
/** draws the commando-cursors: */
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
int cVehicle::calcCommandoChance( cVehicle *destVehicle, cBuilding *destBuilding, bool steal )
{
	int destTurn, factor, srcLevel, chance;

	if ( !destVehicle && !destBuilding ) return 0;

	if ( destVehicle ) destTurn = destVehicle->data.buildCosts/3; // TODO: include cost research and clan modifications? Or should always the basic version without clanmods be used?
	else if ( destBuilding ) destTurn = destBuilding->data.buildCosts/3; // TODO: Bug? /3? or correctly /2, because constructing buildings takes two resources per turn?

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

//-----------------------------------------------------------------------------
int cVehicle::calcCommandoTurns( cVehicle *destVehicle, cBuilding *destBuilding )
{
	int vehiclesTable[13] = { 0, 0, 0, 5, 8, 3, 3, 0, 0, 0, 1, 0, -4 };
	int destTurn, srcLevel;

	if ( destVehicle )
	{
		destTurn = destVehicle->data.buildCosts/3;
		srcLevel = (int)CommandoRank;
		if ( destTurn > 0 && destTurn < 13 ) srcLevel += vehiclesTable[destTurn];
	}
	else if ( destBuilding )
	{
		destTurn = destBuilding->data.buildCosts/2;
		srcLevel = (int)CommandoRank+8;
	}
	else return 1;

	int turns = (int)(1.0/destTurn*srcLevel);
	if ( turns < 1 ) turns = 1;
	return turns;
}

//-----------------------------------------------------------------------------
void cVehicle::DeleteStored ()
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

//-----------------------------------------------------------------------------
bool cVehicle::isDetectedByPlayer( const cPlayer* player )
{
	for ( unsigned int i = 0; i < DetectedByPlayerList.Size(); i++ )
	{
		if (DetectedByPlayerList[i] == player) return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void cVehicle::setDetectedByPlayer( cPlayer* player )
{
	bool wasDetected = ( DetectedByPlayerList.Size() > 0 );

	if (!isDetectedByPlayer( player ))
		DetectedByPlayerList.Add( player );

	if ( !wasDetected ) sendDetectionState( this );
}

//-----------------------------------------------------------------------------
void cVehicle::resetDetectedByPlayer( cPlayer* player )
{
	bool wasDetected = ( DetectedByPlayerList.Size() > 0 );

	for ( unsigned int i = 0; i < DetectedByPlayerList.Size(); i++ )
	{
		if ( DetectedByPlayerList[i] == player ) DetectedByPlayerList.Delete(i);
	}

	if ( wasDetected && DetectedByPlayerList.Size() == 0 ) sendDetectionState( this );
}

//-----------------------------------------------------------------------------
void cVehicle::makeDetection()
{
	//check whether the vehicle has been detected by others
	if ( data.isStealthOn != TERRAIN_NONE )
	{
		int offset = PosX + PosY * Server->Map->size;
		for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
		{
			cPlayer* player = (*Server->PlayerList)[i];
			if ( player == owner ) continue;
			bool water = Server->Map->IsWater(offset, true);
			bool coast = Server->Map->IsWater(offset) && !water;

			//if the vehicle can also drive on land, we have to check, whether there is a brige, platform, etc.
			//because the vehicle will drive on the bridge
			cBuilding* building = Server->Map->fields[offset].getBaseBuilding();
			if ( data.factorGround > 0 && building && ( building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE ) ) water = coast = false;

			if ( (data.isStealthOn&TERRAIN_GROUND) && ( player->DetectLandMap[offset] || ( !(data.isStealthOn&TERRAIN_COAST) && coast ) || water ))
			{
				setDetectedByPlayer( player );
			}

			if ( (data.isStealthOn&TERRAIN_SEA) && ( player->DetectSeaMap[offset] || !water ))
			{
				setDetectedByPlayer( player );
			}
		}
	}

	//detect other units
	if ( data.canDetectStealthOn )
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
					if ( (data.canDetectStealthOn&TERRAIN_GROUND) && owner->DetectLandMap[offset] && (vehicle->data.isStealthOn&TERRAIN_GROUND) )
					{
						vehicle->setDetectedByPlayer( owner );
					}
					if ( (data.canDetectStealthOn&TERRAIN_SEA) && owner->DetectSeaMap[offset] && (vehicle->data.isStealthOn&TERRAIN_SEA) )
					{
						vehicle->setDetectedByPlayer( owner );
					}
				}
				if ( building && building->owner != owner )
				{
					if ( (data.canDetectStealthOn&AREA_EXP_MINE) && owner->DetectMinesMap[offset] && (building->data.isStealthOn&AREA_EXP_MINE) )
					{
						building->setDetectedByPlayer( owner );
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool cVehicle::isNextTo( int x, int y) const
{
	if ( x + 1 < PosX || y + 1 < PosY ) return false;

	if ( data.isBig )
	{
		if ( x - 2 > PosX || y - 2 > PosY ) return false;
	}
	else
	{
		if ( x - 1 > PosX || y - 1 > PosY ) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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
	blittAlphaSurface ( src, srcrect, dest, destrect );
}
