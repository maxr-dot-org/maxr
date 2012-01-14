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
#include "buildings.h"
#include "player.h"
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
#include "video.h"

using namespace std;

//-----------------------------------------------------------------------------
// cVehicle Class Implementation
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
cVehicle::cVehicle ( sVehicle *v, cPlayer *Owner )
: cUnit (cUnit::kUTVehicle, &(Owner->VehicleData[v->nr]), Owner)
{
	typ = v;
	BandX = 0;
	BandY = 0;
	OffX = 0;
	OffY = 0;
	ditherX = 0;
	ditherY = 0;
	StartUp = 0;
	FlightHigh = 0;
	WalkFrame = 0;
	CommandoRank = 0;
	BuildingTyp.iFirstPart = 0;
	BuildingTyp.iSecondPart = 0;
	BuildCosts = 0;
	BuildRounds = 0;
	BuildRoundsStart = 0;
	ClearingRounds = 0;
	VehicleToActivate = 0;
	BuildBigSavedPos = 0;
	groupSelected = false;
	data.hitpointsCur = data.hitpointsMax;
	data.ammoCur = data.ammoMax;
	ClientMoveJob = NULL;
	ServerMoveJob = NULL;
	autoMJob = NULL;
	hasAutoMoveJob = false;
	moving = false;
	MoveJobActive = false;
	IsBuilding = false;
	IsClearing = false;
	BuildPath = false;
	LayMines = false;
	ClearMines = false;
	Loaded = false;
	IsLocked = false;
	BigBetonAlpha = 0;
	lastShots = 0;
	lastSpeed = 0;

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
		//if ( ClientMoveJob->endMoveAction ) ClientMoveJob->endMoveAction->handleDelVehicle ( this );
	}
	if ( ServerMoveJob )
	{
		ServerMoveJob->release();
		ServerMoveJob->Vehicle = NULL;
	}

	if ( autoMJob )
		delete autoMJob;

	if ( sentryActive )
		owner->deleteSentryVehicle ( this );

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
		for (unsigned int i = 0; i < Server->AJobs.Size (); i++)
		{
			if (Server->AJobs[i]->unit == this) 
				Server->AJobs[i]->unit = 0;
		}
	}
	if ( Client )
	{
		for ( unsigned int i = 0; i < Client->attackJobs.Size(); i++ )
		{
			if ( Client->attackJobs[i]->vehicle == this ) Client->attackJobs[i]->vehicle = NULL;
		}
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
	if ( Client->timer100ms && data.hitpointsCur < data.hitpointsMax && cSettings::getInstance().isDamageEffects() && ( owner == Client->ActivePlayer || Client->ActivePlayer->ScanMap[PosX+PosY*Client->Map->size] ) )
	{
		int intense = ( int ) ( 100 - 100 * ( ( float ) data.hitpointsCur / data.hitpointsMax ) );
		Client->addFX ( fxDarkSmoke, PosX*64 + DamageFXPointX + OffX, PosY*64 + DamageFXPointY + OffY, intense );
	}

	//make landing and take off of planes
	if ( data.factorAir > 0 && Client->timer50ms )
	{
		// check, if the plane should land
		cBuilding *b = Client->Map->fields[PosX+PosY*Client->Map->size].getTopBuilding();

		if ( b && b->owner == owner && b->data.canBeLandedOn && !ClientMoveJob && !moving && !attacking )
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
			rotateTo ( 0 );
	}

	//run start up effect
	if ( StartUp )
	{
		if ( Client->timer50ms )
			StartUp += 25;

		if ( StartUp >= 255 )
			StartUp = 0;

		//max StartUp value for undetected stealth units is 100, because they stay half visible
		if ( (data.isStealthOn&TERRAIN_SEA) && Client->Map->isWater ( PosX, PosY, true ) && detectedByPlayerList.Size() == 0 && owner == Client->ActivePlayer )
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
	if ( data.hasOverlay && cSettings::getInstance().isAnimations() )
	{
		SDL_Rect src;

		tmp = screenPosition;
		src.h = src.w = (int)(typ->overlay_org->h*Client->gameGUI.getZoom());
		tmp.x += (int)(Client->gameGUI.getTileSize()) / 2 - src.h / 2;
		tmp.y += (int)(Client->gameGUI.getTileSize()) / 2 - src.h / 2;
		src.y = 0;
		src.x = turnsDisabled > 0 ? 0 : ( int ) ( ( typ->overlay_org->h * ( ( ANIMATION_SPEED % ( (int)(typ->overlay_org->w*Client->gameGUI.getZoom()) / src.h ) ) ) ) * Client->gameGUI.getZoom() );

		if ( StartUp && cSettings::getInstance().isAlphaEffects() )
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
		drawMunBar();

	//draw status info
	if ( Client->gameGUI.statusChecked() )
		drawStatus();

	//attack job debug output
	if ( Client->gameGUI.getAJobDebugStatus() )
	{
		cVehicle* serverVehicle = NULL;
		if ( Server ) serverVehicle = Server->Map->fields[PosX + PosY * Server->Map->size].getVehicles();
		if ( isBeeingAttacked ) font->showText(screenPosition.x + 1,screenPosition.y + 1, "C: attacked", FONT_LATIN_SMALL_WHITE );
		if ( serverVehicle && serverVehicle->isBeeingAttacked ) font->showText(screenPosition.x + 1,screenPosition.y + 9, "S: attacked", FONT_LATIN_SMALL_YELLOW );
		if ( attacking ) font->showText(screenPosition.x + 1,screenPosition.y + 17, "C: attacking", FONT_LATIN_SMALL_WHITE );
		if ( serverVehicle && serverVehicle->attacking ) font->showText(screenPosition.x + 1,screenPosition.y + 25, "S: attacking", FONT_LATIN_SMALL_YELLOW );
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
		if ( IsBuilding && data.isBig && ( !Client->Map->isWater(PosX, PosY) || Client->Map->fields[PosX+PosY*Client->Map->size].getBaseBuilding()) )
		{
			SDL_SetAlpha ( GraphicsData.gfx_big_beton, SDL_SRCALPHA, BigBetonAlpha );
			CHECK_SCALING(GraphicsData.gfx_big_beton, GraphicsData.gfx_big_beton_org, factor );
			SDL_BlitSurface ( GraphicsData.gfx_big_beton, NULL, surface, &tmp );
		}

		// draw shadow
		tmp = dest;
		if ( cSettings::getInstance().isShadows() ) blitWithPreScale ( typ->build_shw_org, typ->build_shw, NULL, surface, &tmp, factor );

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
		if ( cSettings::getInstance().isShadows() )
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
	if ( cSettings::getInstance().isShadows() && ! ( (data.isStealthOn&TERRAIN_SEA) && Client->Map->isWater ( PosX, PosY, true ) ) )
	{
		if ( StartUp && cSettings::getInstance().isAlphaEffects() ) SDL_SetAlpha ( typ->shw[dir], SDL_SRCALPHA, StartUp / 5 );
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

	if ( StartUp && cSettings::getInstance().isAlphaEffects() )
	{
		SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, StartUp );
	}
	else
	{
		bool water = Client->Map->isWater(PosX, PosY, true);
		//if the vehicle can also drive on land, we have to check, whether there is a brige, platform, etc.
		//because the vehicle will drive on the bridge
		cBuilding* building = Client->Map->fields[PosX + PosY*Client->Map->size].getBaseBuilding();
		if ( building && data.factorGround > 0 && ( building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE ) ) water = false;

		if ( (data.isStealthOn&TERRAIN_SEA) && water && detectedByPlayerList.Size() == 0 && owner == Client->ActivePlayer ) SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, 100 );
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
	groupSelected = false;
	if ( Client->gameGUI.mouseInputMode == placeBand ) BuildPath = false;
	// redraw the background
	StopFXLoop ( Client->iObjectStream );
	Client->iObjectStream = -1;
	Client->gameGUI.setFLC ( NULL );
	Client->gameGUI.setUnitDetailsData ( NULL, NULL );
}

//-----------------------------------------------------------------------------
/** Initializes all unit data to its maxiumum values */
//-----------------------------------------------------------------------------
int cVehicle::refreshData ()
{
	int iReturn = 0;

	if ( turnsDisabled > 0 )
	{
		lastSpeed = data.speedMax;

		if ( data.ammoCur >= data.shotsMax )
			lastShots = data.shotsMax;
		else
			lastShots = data.ammoCur;

		return 1;
	}
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
				// Find a next position that either a) is something we can't move to (in which case we cancel
				// the path building, or b) doesn't have a building type that we're trying to build.
				int     nextX       = PosX;
				int     nextY       = PosY;
				bool    found_next  = false;

				while ( !found_next && ( ( nextX != BandX ) || ( nextY != BandY ) ) )
				{
				// Calculate the next position in the path.
					if ( PosX > BandX ) nextX--;
					if ( PosX < BandX ) nextX++;
					if ( PosY > BandY ) nextY--;
					if ( PosY < BandY ) nextY++;
					// Can we move to this position? If not, we need to kill the path building now.
					if ( !Server->Map->possiblePlace( this, nextX, nextY ) )
					{
						// Try sidestepping stealth units before giving up.
						Server->sideStepStealthUnit( nextX, nextY, this );
						if ( !Server->Map->possiblePlace( this, nextX, nextY ) )
						{
							// We can't build along this path any more.
							break;
						}
					}
					// Can we build at this next position?
					if ( Server->Map->possiblePlaceBuilding( BuildingTyp.getBuilding()->data, nextX, nextY ) )
					{
						// We can build here.
						found_next = true;
						break;
					}
				}

				// If we've found somewhere to move to, move there now.
				if ( found_next && Server->addMoveJob( PosX, PosY, nextX, nextY, this ) )
				{
					IsBuilding = false;
					Server->addUnit( PosX, PosY, BuildingTyp.getBuilding(), owner );
					// Begin the movment immediately, so no other unit can block the destination field.
					this->ServerMoveJob->checkMove();
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
				for ( unsigned int i = 0; i < seenByPlayerList.Size(); i++ )
				{
					sendStopClear ( this, BuildBigSavedPos, seenByPlayerList[i]->Nr );
				}
			}
			else
			{
				sendStopClear ( this, -1, owner->Nr );
				for ( unsigned int i = 0; i < seenByPlayerList.Size(); i++ )
				{
					sendStopClear ( this, -1, seenByPlayerList[i]->Nr );
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
/** Draws the path of the vehicle */
//-----------------------------------------------------------------------------
void cVehicle::DrawPath ()
{
	int mx = 0, my = 0, sp, save;
	SDL_Rect dest, ndest;
	sWaypoint *wp;

	if ( !ClientMoveJob || !ClientMoveJob->Waypoints || owner != Client->ActivePlayer )
	{
		if ( !BuildPath || ( BandX == PosX && BandY == PosY ) || Client->gameGUI.mouseInputMode == placeBand ) return;

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
/** Returns a string with the current state */
//-----------------------------------------------------------------------------
string cVehicle::getStatusStr ()
{
	if ( autoMJob )
		return lngPack.i18n ( "Text~Comp~Surveying" );
	else if ( ClientMoveJob )
		return lngPack.i18n ( "Text~Comp~Moving" );
	else if ( manualFireActive )
		return lngPack.i18n ( "Text~Comp~ReactionFireOff" );
	else if ( sentryActive )
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
	else if ( turnsDisabled > 0 )
	{
		string sText;
		sText = lngPack.i18n ( "Text~Comp~Disabled" ) + " (";
		sText += iToStr ( turnsDisabled ) + ")";
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
	bool water = Client->Map->isWater ( PosX, PosY, true );
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

	cBuilding* building = Client->Map->fields[PosX + PosY * Client->Map->size].getBaseBuilding();
	water = Client->Map->isWater ( PosX, PosY, true );
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
/** Draws the attack cursor */
//-----------------------------------------------------------------------------
void cVehicle::DrawAttackCursor ( int x, int y )
{
	SDL_Rect r;
	int wp, wc, t;
	cVehicle *v;
	cBuilding *b;

	selectTarget(v, b, x, y, data.canAttack, Client->Map );

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
		t = v->calcHealth ( data.damage );
	else
		t = b->calcHealth ( data.damage );

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
void cVehicle::calcTurboBuild(int* const iTurboBuildRounds, int* const iTurboBuildCosts, int iBuild_Costs )
{
	// calculate building time and costs
	int a, rounds, costs;

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
	a = iTurboBuildCosts[0];
	rounds = iTurboBuildRounds[0];
	costs = iTurboBuildCosts[0];

	while ( a >= 4 && data.storageResCur >= costs + 4 )
	{
		rounds--;
		costs += 4;
		a -= 4;
	}

	if ( rounds < iTurboBuildRounds[0] && rounds > 0 && iTurboBuildRounds[0])
	{
		iTurboBuildCosts[1] = costs;
		iTurboBuildRounds[1] = rounds;
	}

	//step 4x
	a = iTurboBuildCosts[1];
	rounds = iTurboBuildRounds[1];
	costs = iTurboBuildCosts[1];

	while ( a >= 10 && costs < data.storageResMax - 2)
	{
		int inc = 24 - min(16,a);
		if ( costs + inc > data.storageResCur ) break;

		rounds--;
		costs += inc;
		a -= 16;
	}

	if ( rounds < iTurboBuildRounds[1] && rounds > 0 && iTurboBuildRounds[1] )
	{
		iTurboBuildCosts[2] = costs;
		iTurboBuildRounds[2] = rounds;
	}
}

//-----------------------------------------------------------------------------
/** Finds the next fitting position for the band */
//-----------------------------------------------------------------------------
void cVehicle::FindNextband ()
{
	bool pos[4] = {false, false, false, false};
	int x = mouse->getKachelX();
	int y = mouse->getKachelY();

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

	if ( turnsDisabled > 0 )
	{
		// Disabled:
		PlayVoice ( VoiceData.VOIDisabled );
	}
	else if ( data.hitpointsCur > data.hitpointsMax / 2 )
	{
		// Status green
		if ( ClientMoveJob && ClientMoveJob->endMoveAction && ClientMoveJob->endMoveAction->type_ == EMAT_ATTACK )
		{
			if ( random(2) )
				PlayVoice( VoiceData.VOIAttacking1 );
			else
				PlayVoice( VoiceData.VOIAttacking2 );
		}
		else if ( autoMJob )
		{
			if ( random(2) )
				PlayVoice( VoiceData.VOISurveying );
			else
				PlayVoice( VoiceData.VOISurveying2 );
		}
		else if ( data.speedCur == 0 )
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
				int i = random(4);
				if (i == 0)
					PlayVoice ( VoiceData.VOIBuildDone1 );
				else if (i == 1)
					PlayVoice ( VoiceData.VOIBuildDone2 );
				else if (i == 2)
					PlayVoice ( VoiceData.VOIBuildDone3 );
				else
					PlayVoice ( VoiceData.VOIBuildDone4 );
			}
		}
		else if ( IsClearing )
		{
			// removing dirt
			PlayVoice ( VoiceData.VOIClearing );
		}
		else if ( data.canAttack && !data.ammoCur )
		{
			// no ammo
			if (random(2))
				PlayVoice ( VoiceData.VOILowAmmo1 );
			else
				PlayVoice ( VoiceData.VOILowAmmo2 );
		}
		else if ( sentryActive )
		{
			// on sentry:
			PlayVoice ( VoiceData.VOISentry );
		}
		else if ( ClearMines )
		{
			if (random(2))
				PlayVoice ( VoiceData.VOIClearingMines );
			else
				PlayVoice ( VoiceData.VOIClearingMines2 );
		}
		else if ( LayMines )
		{
			PlayVoice ( VoiceData.VOILayingMines );
		}
		else
		{
			int nr;
			// Alles OK:
			nr = random(4);

			if ( nr == 0 )
				PlayVoice ( VoiceData.VOIOK1 );
			else if ( nr == 1 )
				PlayVoice ( VoiceData.VOIOK2 );
			else if ( nr == 2 )
				PlayVoice ( VoiceData.VOIOK3 );
			else
				PlayVoice ( VoiceData.VOIOK4 );
		}
	}
	else if ( data.hitpointsCur > data.hitpointsMax / 4 )
		// Status yellow:
		if (random(2))
			PlayVoice ( VoiceData.VOIStatusYellow );
		else
			PlayVoice ( VoiceData.VOIStatusYellow2 );
	else
		// Status red:
		if (random(2))
			PlayVoice ( VoiceData.VOIStatusRed );
		else
			PlayVoice ( VoiceData.VOIStatusRed2 );
}

//-----------------------------------------------------------------------------
/** checks, if resources can be transfered to the unit */
//-----------------------------------------------------------------------------
bool cVehicle::CanTransferTo ( cMapField *OverUnitField )
{
	cBuilding *b;
	cVehicle *v;
	int x = mouse->getKachelX();
	int y = mouse->getKachelY();

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
bool cVehicle::makeAttackOnThis (cUnit* opponentUnit, string reasonForLog) const
{
	cVehicle* targetVehicle = 0;
	cBuilding* targetBuilding = 0;
	selectTarget (targetVehicle, targetBuilding, PosX, PosY, opponentUnit->data.canAttack, Server->Map);
	if (targetVehicle == this)
	{
		int iOff = PosX + PosY * Server->Map->size;
		Log.write (" Server: " + reasonForLog + ": attacking offset " + iToStr (iOff) + " Agressor ID: " + iToStr (opponentUnit->iID), cLog::eLOG_TYPE_NET_DEBUG);
		Server->AJobs.Add (new cServerAttackJob (opponentUnit, iOff, true));
		if (ServerMoveJob != 0)
			ServerMoveJob->bFinished = true;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::makeSentryAttack (sSentry* sentry) const
{
	cUnit* sentryUnit = (sentry->b != 0 ? (cUnit*)sentry->b : (cUnit*)sentry->v);
	if (sentryUnit != 0 && sentryUnit->canAttackObjectAt (PosX, PosY, Server->Map, true))
	{
		if (makeAttackOnThis (sentryUnit, "sentry reaction"))
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::InSentryRange ()
{
	sSentry* Sentry = 0;
	cPlayer* Player = 0;

	int iOff = PosX + PosY * Server->Map->size;

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
				if (makeSentryAttack (Sentry))
					return true;
			}
		}
		else
		{
			if ( ! ( Player->SentriesMapGround[iOff] && Player->ScanMap[iOff] ) ) continue; //check next player

			for ( unsigned int k = 0;k < Player->SentriesGround.Size();k++ )
			{
				Sentry = Player->SentriesGround[k];
				if (makeSentryAttack (Sentry))
					return true;
			}
		}
	}

	return provokeReactionFire ();
}

//-----------------------------------------------------------------------------
bool cVehicle::isOtherUnitOffendedByThis (cUnit* otherUnit) const
{
	// don't treat the cheap buildings (connectors, roads, beton blocks) as offendable
	bool otherUnitIsCheapBuilding = (otherUnit->isBuilding () && otherUnit->data.ID.getUnitDataOriginalVersion ()->buildCosts > 2);
	
	if (otherUnitIsCheapBuilding == false 
		&& isInRange (otherUnit->PosX, otherUnit->PosY) 
		&& canAttackObjectAt (otherUnit->PosX, otherUnit->PosY, Server->Map, true, false))
	{
		// test, if this vehicle can really attack the opponentVehicle
		cVehicle* selectedTargetVehicle = 0;
		cBuilding* selectedTargetBuilding = 0;
		selectTarget (selectedTargetVehicle, selectedTargetBuilding, otherUnit->PosX, otherUnit->PosY, data.canAttack, Server->Map);
		if (selectedTargetVehicle == otherUnit || selectedTargetBuilding == otherUnit)
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::doesPlayerWantToFireOnThisVehicleAsReactionFire (cPlayer* player) const
{
	if (Server->isTurnBasedGame ())
	{
		// In the turn based game style, the opponent always fires on the unit if he can, regardless if the unit is offending or not.
		return true;
	}
	else
	{
		// check if there is a vehicle or building of player, that is offended
		
		cUnit* opponentVehicle = player->VehicleList;
		while (opponentVehicle != 0)
		{
			if (isOtherUnitOffendedByThis (opponentVehicle))
				return true;
			opponentVehicle = opponentVehicle->next;
		}
		cUnit* opponentBuilding = player->BuildingList;
		while (opponentBuilding != 0)
		{
			if (isOtherUnitOffendedByThis (opponentBuilding))
				return true;
			opponentBuilding = (cBuilding*)opponentBuilding->next;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::doReactionFireForUnit (cUnit* opponentUnit) const
{
	if (opponentUnit->sentryActive == false && opponentUnit->manualFireActive == false
		&& opponentUnit->canAttackObjectAt (PosX, PosY, Server->Map, true)
		// Possible TODO: better handling of stealth units. e.g. do reaction fire, if already detected?
		&& (opponentUnit->isVehicle () == false || opponentUnit->data.isStealthOn == TERRAIN_NONE)) 
	{
		if (makeAttackOnThis (opponentUnit, "reaction fire"))
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::doReactionFire (cPlayer* player) const
{
	// search a unit of the opponent, that could fire on this vehicle
	// first look for a building
	cUnit* opponentBuilding = player->BuildingList;
	while (opponentBuilding != 0)
	{
		if (doReactionFireForUnit (opponentBuilding))
			return true;		
		opponentBuilding = opponentBuilding->next;
	}
	cUnit* opponentVehicle = player->VehicleList;
	while (opponentVehicle != 0)
	{
		if (doReactionFireForUnit (opponentVehicle))
			return true;
		opponentVehicle = opponentVehicle->next;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::provokeReactionFire ()
{
	// unit can't fire, so it can't provoke a reaction fire
	if (data.canAttack == false || data.shotsCur <= 0 || data.ammoCur <= 0)
		return false;
		
	int iOff = PosX + PosY * Server->Map->size;	

	for (unsigned int i = 0; i < Server->PlayerList->Size (); i++)
	{
		cPlayer* player = (*Server->PlayerList)[i];
		if (player == owner) 
			continue;
		
		if (data.isStealthOn != TERRAIN_NONE && !isDetectedByPlayer (player))
			continue;
		
		if (player->ScanMap[iOff] == false) // The vehicle can't be seen by the opposing player. No possibility for reaction fire.
			continue;
		
		if (doesPlayerWantToFireOnThisVehicleAsReactionFire (player) == false)
			continue;
		
		if (doReactionFire (player))
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
/** Draws exitpoints for a vehicle, that should be exited */
//-----------------------------------------------------------------------------
void cVehicle::DrawExitPoints(sVehicle* const typ) const
{
	int const spx = getScreenPosX();
	int const spy = getScreenPosY();

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
	if ( !map->possiblePlaceVehicle( typ->data, x, y, owner ) ) return false;
	if ( data.factorAir > 0 && ( x != PosX || y != PosY ) ) return false;
	if ( !isNextTo(x, y) ) return false;

	return true;
}

//-----------------------------------------------------------------------------
bool cVehicle::canLoad ( int x, int y, cMap *Map, bool checkPosition )
{
	if ( x < 0 || x >= Map->size || y < 0 || y >= Map->size ) return false;

	return canLoad ( Map->fields[x + y * Map->size].getVehicles(), checkPosition );
}

//-----------------------------------------------------------------------------
bool cVehicle::canLoad ( cVehicle *Vehicle, bool checkPosition )
{
	if ( !Vehicle ) return false;

	if ( Vehicle->Loaded ) return false;

	if ( data.storageUnitsCur >= data.storageUnitsMax )	return false;

	if ( checkPosition && !isNextTo ( Vehicle->PosX, Vehicle->PosY ) ) return false;

	if ( checkPosition && data.factorAir > 0 && ( Vehicle->PosX != PosX || Vehicle->PosY != PosY ) ) return false;

	size_t i;
	for ( i = 0; i < data.storeUnitsTypes.size(); i++ )
	{
		if ( data.storeUnitsTypes[i].compare ( Vehicle->data.isStorageType ) == 0 ) break;
	}
	if ( i == data.storeUnitsTypes.size() ) return false;

	if ( Vehicle->ClientMoveJob && ( Vehicle->moving || Vehicle->attacking || Vehicle->MoveJobActive ) ) return false;

	if ( Vehicle->owner != owner || Vehicle->IsBuilding || Vehicle->IsClearing ) return false;

	if ( Vehicle->isBeeingAttacked ) return false;

	return true;
}

//-----------------------------------------------------------------------------
void cVehicle::storeVehicle( cVehicle *Vehicle, cMap *Map )
{
	Map->deleteVehicle ( Vehicle );
	if ( Vehicle->sentryActive )
	{
		Vehicle->owner->deleteSentryVehicle( Vehicle);
		Vehicle->sentryActive = false;
	}
	if ( Vehicle->manualFireActive )
		Vehicle->manualFireActive = false;

	Vehicle->Loaded = true;

	storedUnits.Add ( Vehicle );
	data.storageUnitsCur++;

	owner->DoScan();
}

//-----------------------------------------------------------------------------
/** Exits a vehicle */
//-----------------------------------------------------------------------------
void cVehicle::exitVehicleTo( cVehicle *Vehicle, int offset, cMap *Map )
{
	for ( unsigned int i = 0; i < storedUnits.Size(); i++ )
	{
		if ( storedUnits[i] == Vehicle )
		{
			storedUnits.Delete ( i );
			break;
		}
		if ( i == storedUnits.Size()-1 ) return;
	}

	data.storageUnitsCur--;

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
bool cVehicle::canSupply (int x, int y, int supplyType) const
{
	if (x < 0 || x >= Client->Map->size || y < 0 || y >= Client->Map->size) 
		return false;

	cMapField& field = Client->Map->fields[x + y * Client->Map->size];
	if (field.getVehicles ()) return canSupply (field.getVehicles (), supplyType);
	else if (field.getPlanes ()) return canSupply (field.getPlanes (), supplyType);
	else if (field.getTopBuilding ()) return canSupply (field.getTopBuilding (), supplyType);

	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::canSupply (cUnit* unit, int supplyType) const
{
	if (unit == 0) 
		return false;

	if (data.storageResCur <= 0) 
		return false;

	if (unit->data.isBig == false)
	{
		if (unit->PosX > PosX + 1 || unit->PosX < PosX - 1 || unit->PosY > PosY + 1 || unit->PosY < PosY - 1) 
			return false;
	}
	else
	{
		if (unit->PosX > PosX + 1 || unit->PosX < PosX - 2 || unit->PosY > PosY + 1 || unit->PosY < PosY - 2) 
			return false;
	}
	
	if (unit->isVehicle () && unit->data.factorAir > 0 && ((cVehicle*)unit)->FlightHigh > 0) 
		return false;

	switch (supplyType)
	{
	case SUPPLY_TYPE_REARM:
		if (unit == this || unit->data.canAttack == false || unit->data.ammoCur >= unit->data.ammoMax 
			|| (unit->isVehicle () && ((cVehicle*)unit)->isUnitMoving ()) 
			|| unit->attacking) 
			return false;
		break;
	case SUPPLY_TYPE_REPAIR:
		if (unit == this || unit->data.hitpointsCur >= unit->data.hitpointsMax 
			|| (unit->isVehicle () && ((cVehicle*)unit)->isUnitMoving ()) 
			|| unit->attacking) 
			return false;
		break;
	default:
		return false;
	}

	return true;
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
bool cVehicle::canDoCommandoAction (int x, int y, cMap* map, bool steal) const
{
	if ((steal && data.canCapture == false) || (steal == false && data.canDisable == false))
		return false;

	if (isNextTo (x, y) == false || data.shotsCur == 0) 
		return false;

	int off = x + y * map->size;
	cUnit* vehicle  = map->fields[off].getVehicles ();
	cUnit* building = map->fields[off].getBuildings ();
	cUnit* unit = vehicle ? vehicle : building;

	if (unit != 0)
	{
		bool result = true;
		if (unit->isBuilding () && unit->owner == 0) return false; // rubble
		if (steal && unit->data.canBeCaptured == false) return false;
		if (steal == false && unit->data.canBeDisabled == false) return false;
		if (steal && unit->storedUnits.Size () > 0) return false;
		if (unit->owner == owner) return false;
		if (unit->isVehicle () && unit->data.factorAir > 0 && ((cVehicle*)unit)->FlightHigh > 0) return false;

		if (result) 
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
/** draws the commando-cursors: */
//-----------------------------------------------------------------------------
void cVehicle::drawCommandoCursor (int x, int y, bool steal) const
{
	cMapField& field = Client->Map->fields[x + y * Client->Map->size];
	SDL_Surface* sf;

	cUnit* unit = 0;
	if (steal)
	{
		unit = field.getVehicles ();
		sf = GraphicsData.gfx_Csteal;
	}
	else
	{
		unit = field.getVehicles ();
		if (unit == 0) 
			unit = field.getTopBuilding ();
		sf = GraphicsData.gfx_Cdisable;
	}

	SDL_Rect r;
	r.x = 1;
	r.y = 28;
	r.h = 3;
	r.w = 35;

	if (unit == 0)
	{
		SDL_FillRect (sf, &r, 0);
		return;
	}

	SDL_FillRect (sf, &r, 0xFF0000);
	r.w = 35 * calcCommandoChance (unit, steal) / 100;
	SDL_FillRect (sf, &r, 0x00FF00);
}

//-----------------------------------------------------------------------------
int cVehicle::calcCommandoChance (cUnit* destUnit, bool steal) const
{
	if (destUnit == 0)
		return 0;

	// TODO: include cost research and clan modifications? Or should always the basic version without clanmods be used?
	// TODO: Bug for buildings? /3? or correctly /2, because constructing buildings takes two resources per turn?
	int destTurn = destUnit->data.buildCosts / 3;
	
	int factor = steal ? 1 : 4;
	int srcLevel = (int)CommandoRank+7;

	/* The chance to disable or steal a unit depends on the infiltratorranking and the
	buildcosts (or 'turns' in the original game) of the target unit. The chance rises
	linearly with a higher ranking of the infiltrator. The chance of a unexperienced
	infiltrator will be calculated like he has the ranking 7. Disabling has a 4 times
	higher chance then stealing.
	*/
	int chance = Round ((float)(8 * srcLevel) / (35 * destTurn) * factor * 100);
	if (chance > 90) 
		chance = 90;

	return chance;
}

//-----------------------------------------------------------------------------
int cVehicle::calcCommandoTurns (cUnit* destUnit) const
{
	if (destUnit == 0)
		return 1;

	int vehiclesTable[13] = { 0, 0, 0, 5, 8, 3, 3, 0, 0, 0, 1, 0, -4 };
	int destTurn, srcLevel;

	if (destUnit->isVehicle ())
	{
		destTurn = destUnit->data.buildCosts / 3;
		srcLevel = (int)CommandoRank;
		if (destTurn > 0 && destTurn < 13)
			srcLevel += vehiclesTable[destTurn];
	}
	else
	{
		destTurn = destUnit->data.buildCosts / 2;
		srcLevel = (int)CommandoRank + 8;
	}

	int turns = (int)(1.0 / destTurn * srcLevel);
	if (turns < 1)
		turns = 1;
	return turns;
}

//-----------------------------------------------------------------------------
bool cVehicle::isDetectedByPlayer( const cPlayer* player )
{
	for ( unsigned int i = 0; i < detectedByPlayerList.Size(); i++ )
	{
		if (detectedByPlayerList[i] == player) return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void cVehicle::setDetectedByPlayer( cPlayer* player )
{
	bool wasDetected = ( detectedByPlayerList.Size() > 0 );

	if (!isDetectedByPlayer( player ))
		detectedByPlayerList.Add( player );

	if ( !wasDetected ) sendDetectionState( this );
}

//-----------------------------------------------------------------------------
void cVehicle::resetDetectedByPlayer( cPlayer* player )
{
	bool wasDetected = ( detectedByPlayerList.Size() > 0 );

	for ( unsigned int i = 0; i < detectedByPlayerList.Size(); i++ )
	{
		if ( detectedByPlayerList[i] == player ) detectedByPlayerList.Delete(i);
	}

	if ( wasDetected && detectedByPlayerList.Size() == 0 ) sendDetectionState( this );
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
			bool water = Server->Map->isWater(PosX, PosY, true);
			bool coast = Server->Map->isWater(PosX, PosY) && !water;

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
	if ( !cSettings::getInstance().shouldDoPrescale() )
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



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- methods, that already have been extracted as part of cUnit refactoring
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
bool cVehicle::canBeStoppedViaUnitMenu () const
{ 
	return (ClientMoveJob != 0 || (isUnitBuildingABuilding () && BuildRounds > 0) || (isUnitClearing () && ClearingRounds > 0));
}

//-----------------------------------------------------------------------------
void cVehicle::executeBuildCommand ()
{
	if (ClientMoveJob)
		sendWantStopMove (iID);
	cBuildingsBuildMenu buildMenu (owner, this);
	buildMenu.show ();	
}

//-----------------------------------------------------------------------------
void cVehicle::executeStopCommand ()
{
	if (ClientMoveJob != 0)
		sendWantStopMove (iID);
	else if (isUnitBuildingABuilding ())
		sendWantStopBuilding (iID);
	else if (isUnitClearing ())
		sendWantStopClear (this);
}

//-----------------------------------------------------------------------------
void cVehicle::executeAutoMoveJobCommand ()
{
	if (data.canSurvey == false)
		return;
	if (autoMJob == 0)
	{
		autoMJob = new cAutoMJob (this);
	}
	else
	{
		delete autoMJob;
		autoMJob = 0;
	}
}

//-----------------------------------------------------------------------------
void cVehicle::executeActivateStoredVehiclesCommand ()
{
	cStorageMenu storageMenu (storedUnits, this, 0);
	storageMenu.show ();
}

//-----------------------------------------------------------------------------
void cVehicle::executeLayMinesCommand ()
{
	LayMines = !LayMines;
	ClearMines = false;
	sendMineLayerStatus (this);	
}

//-----------------------------------------------------------------------------
void cVehicle::executeClearMinesCommand ()
{
	ClearMines = !ClearMines;
	LayMines = false;
	sendMineLayerStatus (this);
}

//-----------------------------------------------------------------------------
sUnitData* cVehicle::getUpgradedUnitData () const
{
	return &(owner->VehicleData[typ->nr]);
}

//-----------------------------------------------------------------------------
bool cVehicle::treatAsBigForMenuDisplay () const
{
	return (IsBuilding && BuildingTyp.getUnitDataOriginalVersion ()->isBig);
}
