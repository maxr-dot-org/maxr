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

#include "vehicles.h"

#include "attackJobs.h"
#include "automjobs.h"
#include "buildings.h"
#include "client.h"
#include "clientevents.h"
#include "clist.h"
#include "dialog.h"
#include "events.h"
#include "files.h"
#include "fxeffects.h"
#include "hud.h"
#include "map.h"
#include "menus.h"
#include "mouse.h"
#include "pcx.h"
#include "player.h"
#include "server.h"
#include "settings.h"
#include "sound.h"
#include "unifonts.h"
#include "video.h"

using namespace std;

//-----------------------------------------------------------------------------
// cVehicle Class Implementation
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
cVehicle::cVehicle (const sVehicle* v, cPlayer* Owner, unsigned int ID) :
	cUnit (cUnit::kUTVehicle, & (Owner->VehicleData[v->nr]), Owner, ID),
	next (0),
	prev (0)
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
	BigBetonAlpha = 0;
	lastShots = 0;
	lastSpeed = 0;

	DamageFXPointX = random (7) + 26 - 3;
	DamageFXPointY = random (7) + 26 - 3;
	refreshData();
}

//-----------------------------------------------------------------------------
cVehicle::~cVehicle()
{
	if (ClientMoveJob)
	{
		ClientMoveJob->release();
		ClientMoveJob->Vehicle = NULL;
	}
	if (ServerMoveJob)
	{
		ServerMoveJob->release();
		ServerMoveJob->Vehicle = NULL;
	}

	delete autoMJob;
}

//-----------------------------------------------------------------------------
/** Draws the vehicle */
//-----------------------------------------------------------------------------
void cVehicle::draw (SDL_Rect screenPosition, cGameGUI& gameGUI)
{
	//make damage effect
	cPlayer* activePlayer = gameGUI.getClient()->getActivePlayer();
	if (gameGUI.timer100ms && data.hitpointsCur < data.hitpointsMax &&
		cSettings::getInstance().isDamageEffects() &&
		(owner == activePlayer || activePlayer->canSeeAnyAreaUnder (*this)))
	{
		int intense = (int) (100 - 100 * ( (float) data.hitpointsCur / data.hitpointsMax));
		gameGUI.addFx (new cFxDarkSmoke (PosX * 64 + DamageFXPointX + OffX, PosY * 64 + DamageFXPointY + OffY, intense, gameGUI.getWindDir()));
	}

	//make landing and take off of planes
	if (data.factorAir > 0 && gameGUI.timer50ms)
	{
		if (canLand (*gameGUI.getClient()->getMap()))
		{
			FlightHigh -= 8;
			FlightHigh = std::max (FlightHigh, 0);
		}
		else
		{
			FlightHigh += 8;
			FlightHigh = std::min (64, FlightHigh);
		}
	}

	// make the dithering
	if (gameGUI.timer100ms)
	{
		if (FlightHigh > 0)
		{

			if (moving || gameGUI.getAnimationSpeed() % 10 == 0)
			{
				ditherX = 0;
				ditherY = 0;
			}
			else
			{
				ditherX = random (2) - 1;
				ditherY = random (2) - 1;
			}
		}
		else
		{
			ditherX = 0;
			ditherY = 0;
		}
	}

	//run start up effect
	if (StartUp)
	{
		if (gameGUI.timer50ms)
			StartUp += 25;

		if (StartUp >= 255)
			StartUp = 0;

		//max StartUp value for undetected stealth units is 100, because they stay half visible
		if ( (data.isStealthOn & TERRAIN_SEA) && gameGUI.getClient()->getMap()->isWater (PosX, PosY) && detectedByPlayerList.size() == 0 && owner == gameGUI.getClient()->getActivePlayer())
		{
			if (StartUp > 100) StartUp = 0;
		}
	}

	if (IsBuilding && !job && BigBetonAlpha < 255u)
	{
		if (gameGUI.timer50ms)
			BigBetonAlpha += 25;

		BigBetonAlpha = std::min (255u, BigBetonAlpha);
	}

	//calculate screen position
	int ox = (int) (OffX * gameGUI.getZoom());
	int oy = (int) (OffY * gameGUI.getZoom());

	screenPosition.x += ox;
	screenPosition.y += oy;

	if (FlightHigh > 0)
	{
		screenPosition.x += ditherX;
		screenPosition.y += ditherY;
	}

	SDL_Rect dest;
	dest.x = dest.y = 0;
	bool bDraw = false;
	SDL_Surface* drawingSurface = gameGUI.getDCache()->getCachedImage (*this);
	if (drawingSurface == NULL)
	{
		//no cached image found. building needs to be redrawn.
		bDraw = true;
		drawingSurface = gameGUI.getDCache()->createNewEntry (*this);
	}

	if (drawingSurface == NULL)
	{
		//image will not be cached. So blitt directly to the screen buffer.
		dest = screenPosition;
		drawingSurface = buffer;
	}

	if (bDraw)
	{
		render (gameGUI.getClient(), drawingSurface, dest, (float) gameGUI.getTileSize() / 64.0f, cSettings::getInstance().isShadows());
	}

	//now check, whether the image has to be blitted to screen buffer
	if (drawingSurface != buffer)
	{
		dest = screenPosition;
		SDL_BlitSurface (drawingSurface, NULL, buffer, &dest);
	}

	// draw overlay if necessary:
	drawOverlayAnimation (gameGUI.getClient(), buffer, screenPosition, gameGUI.getZoom());

	//remove the dithering for the following operations
	if (FlightHigh > 0)
	{
		screenPosition.x -= ditherX;
		screenPosition.y -= ditherY;
	}

	//remove movement offset for working units
	if (IsBuilding || IsClearing)
	{
		screenPosition.x -= ox;
		screenPosition.y -= oy;
	}

	// draw indication, when building is complete
	if (IsBuilding && BuildRounds == 0  && owner == gameGUI.getClient()->getActivePlayer() && !BuildPath)
	{
		SDL_Rect d, t;
		int max, nr;
		nr = 0xFF00 - ( (gameGUI.getAnimationSpeed() % 0x8) * 0x1000);

		if (data.isBig)
			max = (gameGUI.getTileSize() * 2) - 3;
		else
			max = gameGUI.getTileSize() - 3;

		d.x = screenPosition.x + 2;
		d.y = screenPosition.y + 2;
		d.w = max;
		d.h = 3;
		t = d;
		SDL_FillRect (buffer, &d, nr);
		d = t;
		d.y += max - 3;
		t = d;
		SDL_FillRect (buffer, &d, nr);
		d = t;
		d.y = screenPosition.y + 2;
		d.w = 3;
		d.h = max;
		t = d;
		SDL_FillRect (buffer, &d, nr);
		d = t;
		d.x += max - 3;
		SDL_FillRect (buffer, &d, nr);
	}

	// Draw the colored frame if necessary
	if (gameGUI.colorChecked())
	{
		SDL_Rect d, t;
		int max;
		int nr = *static_cast<Uint32*> (owner->getColorSurface()->pixels);

		if (data.isBig) max = (gameGUI.getTileSize() - 1) * 2;
		else max = gameGUI.getTileSize() - 1;

		d.x = screenPosition.x + 1;
		d.y = screenPosition.y + 1;
		d.w = max;
		d.h = 1;
		t = d;
		SDL_FillRect (buffer, &d, nr);
		d = t;
		d.y += max - 1;
		t = d;
		SDL_FillRect (buffer, &d, nr);
		d = t;
		d.y = screenPosition.y + 1;
		d.w = 1;
		d.h = max;
		t = d;
		SDL_FillRect (buffer, &d, nr);
		d = t;
		d.x += max - 1;
		SDL_FillRect (buffer, &d, nr);
	}

	// draw the group selected frame if necessary
	if (groupSelected)
	{
		Uint32 color = 0xFFFF00;
		SDL_Rect d;

		d.w = gameGUI.getTileSize() - 2;
		d.h = 1;
		d.x = screenPosition.x + 1;
		d.y = screenPosition.y + 1;
		SDL_FillRect (buffer, &d, color);

		d.w = gameGUI.getTileSize() - 2;
		d.h = 1;
		d.x = screenPosition.x + 1;
		d.y = screenPosition.y + gameGUI.getTileSize() - 1;
		SDL_FillRect (buffer, &d, color);

		d.w = 1;
		d.h = gameGUI.getTileSize() - 2;
		d.x = screenPosition.x + 1;
		d.y = screenPosition.y + 1;
		SDL_FillRect (buffer, &d, color);

		d.w = 1;
		d.h = gameGUI.getTileSize() - 2;
		d.x = screenPosition.x + gameGUI.getTileSize() - 1;
		d.y = screenPosition.y + 1;
		SDL_FillRect (buffer, &d, color);
	}
	if (gameGUI.getSelectedUnit() == this)
	{
		SDL_Rect d, t;
		int len, max;

		if (data.isBig) max = gameGUI.getTileSize() * 2;
		else max = gameGUI.getTileSize();

		len = max / 4;

		d.x = screenPosition.x + 1;
		d.y = screenPosition.y + 1;
		d.w = len;
		d.h = 1;
		t = d;
		SDL_FillRect (buffer, &d, gameGUI.getBlinkColor());
		d = t;
		d.x += max - len - 1;
		t = d;
		SDL_FillRect (buffer, &d, gameGUI.getBlinkColor());
		d = t;
		d.y += max - 2;
		t = d;
		SDL_FillRect (buffer, &d, gameGUI.getBlinkColor());
		d = t;
		d.x = screenPosition.x + 1;
		t = d;
		SDL_FillRect (buffer, &d, gameGUI.getBlinkColor());
		d = t;
		d.y = screenPosition.y + 1;
		d.w = 1;
		d.h = len;
		t = d;
		SDL_FillRect (buffer, &d, gameGUI.getBlinkColor());
		d = t;
		d.x += max - 2;
		t = d;
		SDL_FillRect (buffer, &d, gameGUI.getBlinkColor());
		d = t;
		d.y += max - len - 1;
		t = d;
		SDL_FillRect (buffer, &d, gameGUI.getBlinkColor());
		d = t;
		d.x = screenPosition.x + 1;
		SDL_FillRect (buffer, &d, gameGUI.getBlinkColor());
	}

	//draw health bar
	if (gameGUI.hitsChecked())
		drawHealthBar (gameGUI, screenPosition);

	//draw ammo bar
	if (gameGUI.ammoChecked() && data.canAttack)
		drawMunBar (gameGUI, screenPosition);

	//draw status info
	if (gameGUI.statusChecked())
		drawStatus (gameGUI, screenPosition);

	//attack job debug output
	if (gameGUI.getAJobDebugStatus())
	{
		cServer* server = gameGUI.getClient()->getServer();
		cVehicle* serverVehicle = NULL;
		if (server) serverVehicle = server->Map->fields[server->Map->getOffset (PosX, PosY)].getVehicle();
		if (isBeeingAttacked) font->showText (screenPosition.x + 1, screenPosition.y + 1, "C: attacked", FONT_LATIN_SMALL_WHITE);
		if (serverVehicle && serverVehicle->isBeeingAttacked) font->showText (screenPosition.x + 1, screenPosition.y + 9, "S: attacked", FONT_LATIN_SMALL_YELLOW);
		if (attacking) font->showText (screenPosition.x + 1, screenPosition.y + 17, "C: attacking", FONT_LATIN_SMALL_WHITE);
		if (serverVehicle && serverVehicle->attacking) font->showText (screenPosition.x + 1, screenPosition.y + 25, "S: attacking", FONT_LATIN_SMALL_YELLOW);
	}
}

void cVehicle::drawOverlayAnimation (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, int frameNr, int alpha)
{
	if (data.hasOverlay == false || cSettings::getInstance().isAnimations() == false) return;

	const Uint16 size = (Uint16) (typ->overlay_org->h * zoomFactor);
	SDL_Rect src = {Sint16 (frameNr * size), 0, size, size};

	SDL_Rect tmp = dest;
	const int offset = Round (64.0f * zoomFactor) / 2 - src.h / 2;
	tmp.x += offset;
	tmp.y += offset;

	SDL_SetAlpha (typ->overlay, SDL_SRCALPHA, alpha);
	blitWithPreScale (typ->overlay_org, typ->overlay, &src, surface, &tmp, zoomFactor);
}

void cVehicle::drawOverlayAnimation (const cClient* client, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor)
{
	if (data.hasOverlay == false || cSettings::getInstance().isAnimations() == false) return;

	int frameNr = 0;
	if (client && turnsDisabled == 0)
	{
		frameNr = client->getGameGUI().getAnimationSpeed() % (typ->overlay_org->w / typ->overlay_org->h);
	}

	int alpha = 255;
	if (StartUp && cSettings::getInstance().isAlphaEffects())
		alpha = StartUp;
	drawOverlayAnimation (surface, dest, zoomFactor, frameNr, alpha);
}

void cVehicle::render_BuildingOrBigClearing (const cClient& client, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow)
{
	assert ( (IsBuilding || (IsClearing && data.isBig)) && job == NULL);
	// draw beton if nessesary
	SDL_Rect tmp = dest;
	const cMap& map = *client.getMap();
	if (IsBuilding && data.isBig && (!map.isWaterOrCoast (PosX, PosY) || map.fields[map.getOffset (PosX, PosY)].getBaseBuilding()))
	{
		SDL_SetAlpha (GraphicsData.gfx_big_beton, SDL_SRCALPHA, BigBetonAlpha);
		CHECK_SCALING (GraphicsData.gfx_big_beton, GraphicsData.gfx_big_beton_org, zoomFactor);
		SDL_BlitSurface (GraphicsData.gfx_big_beton, NULL, surface, &tmp);
	}

	// draw shadow
	tmp = dest;
	if (drawShadow) blitWithPreScale (typ->build_shw_org, typ->build_shw, NULL, surface, &tmp, zoomFactor);

	// draw player color
	SDL_Rect src;
	src.y = 0;
	src.h = src.w = (int) (typ->build_org->h * zoomFactor);
	src.x = (client.getGameGUI().getAnimationSpeed() % 4) * src.w;
	SDL_BlitSurface (owner->getColorSurface(), NULL, GraphicsData.gfx_tmp, NULL);
	blitWithPreScale (typ->build_org, typ->build, &src, GraphicsData.gfx_tmp, NULL, zoomFactor, 4);

	// draw vehicle
	src.x = 0;
	src.y = 0;
	tmp = dest;
	SDL_SetAlpha (GraphicsData.gfx_tmp, SDL_SRCALPHA, 255);
	SDL_BlitSurface (GraphicsData.gfx_tmp, &src, surface, &tmp);
}

void cVehicle::render_smallClearing (const cClient& client, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow)
{
	assert (IsClearing && !data.isBig && job == NULL);

	// draw shadow
	SDL_Rect tmp = dest;
	if (drawShadow)
		blitWithPreScale (typ->clear_small_shw_org, typ->clear_small_shw, NULL, surface, &tmp, zoomFactor);

	// draw player color
	SDL_Rect src;
	src.y = 0;
	src.h = src.w = (int) (typ->clear_small_org->h * zoomFactor);
	src.x = (client.getGameGUI().getAnimationSpeed() % 4) * src.w;
	SDL_BlitSurface (owner->getColorSurface(), NULL, GraphicsData.gfx_tmp, NULL);
	blitWithPreScale (typ->clear_small_org, typ->clear_small, &src, GraphicsData.gfx_tmp, NULL, zoomFactor, 4);

	// draw vehicle
	src.x = 0;
	src.y = 0;
	tmp = dest;
	SDL_SetAlpha (GraphicsData.gfx_tmp, SDL_SRCALPHA, 255);
	SDL_BlitSurface (GraphicsData.gfx_tmp, &src, surface, &tmp);
}

void cVehicle::render_shadow (const cClient& client, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor)
{
	if (client.getMap()->isWater (PosX, PosY) && (data.isStealthOn & TERRAIN_SEA)) return;

	if (StartUp && cSettings::getInstance().isAlphaEffects()) SDL_SetAlpha (typ->shw[dir], SDL_SRCALPHA, StartUp / 5);
	else SDL_SetAlpha (typ->shw[dir], SDL_SRCALPHA, 50);
	SDL_Rect tmp = dest;

	// draw shadow
	if (FlightHigh > 0)
	{
		int high = ( (int) ( (int) (client.getGameGUI().getTileSize()) * (FlightHigh / 64.0f)));
		tmp.x += high;
		tmp.y += high;

		blitWithPreScale (typ->shw_org[dir], typ->shw[dir], NULL, surface, &tmp, zoomFactor);
	}
	else if (data.animationMovement)
	{
		const Uint16 size = (int) (typ->img_org[dir]->h * zoomFactor);
		SDL_Rect r = {Sint16 (WalkFrame * size), 0, size, size};
		blitWithPreScale (typ->shw_org[dir], typ->shw[dir], &r, surface, &tmp, zoomFactor);
	}
	else
		blitWithPreScale (typ->shw_org[dir], typ->shw[dir], NULL, surface, &tmp, zoomFactor);
}

void cVehicle::render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, int alpha)
{
	// draw player color
	SDL_BlitSurface (owner->getColorSurface(), NULL, GraphicsData.gfx_tmp, NULL);

	// read the size:
	SDL_Rect src;
	src.w = (int) (typ->img_org[dir]->w * zoomFactor);
	src.h = (int) (typ->img_org[dir]->h * zoomFactor);

	if (data.animationMovement)
	{
		SDL_Rect tmp;
		src.w = src.h = tmp.h = tmp.w = (int) (typ->img_org[dir]->h * zoomFactor);
		tmp.x = WalkFrame * tmp.w;
		tmp.y = 0;
		blitWithPreScale (typ->img_org[dir], typ->img[dir], &tmp, GraphicsData.gfx_tmp, NULL, zoomFactor);
	}
	else
		blitWithPreScale (typ->img_org[dir], typ->img[dir], NULL, GraphicsData.gfx_tmp, NULL, zoomFactor);

	// draw the vehicle
	src.x = 0;
	src.y = 0;
	SDL_Rect tmp = dest;

	SDL_SetAlpha (GraphicsData.gfx_tmp, SDL_SRCALPHA, alpha);
	blittAlphaSurface (GraphicsData.gfx_tmp, &src, surface, &tmp);
}

void cVehicle::render (const cClient* client, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow)
{
	// Note: when changing something in this function,
	// make sure to update the caching rules!

	// draw working engineers and bulldozers:
	if (client && job == NULL)
	{
		if (IsBuilding || (IsClearing && data.isBig))
		{
			render_BuildingOrBigClearing (*client, surface, dest, zoomFactor, drawShadow);
			return;
		}
		if (IsClearing && !data.isBig)
		{
			render_smallClearing (*client, surface, dest, zoomFactor, drawShadow);
			return;
		}
	}

	// draw all other vehicles:

	// draw shadow
	if (drawShadow && client)
	{
		render_shadow (*client, surface, dest, zoomFactor);
	}

	int alpha = 255;
	if (client)
	{
		if (StartUp && cSettings::getInstance().isAlphaEffects())
		{
			alpha = StartUp;
		}
		else
		{
			const cMap& map = *client->getMap();
			bool water = map.isWater (PosX, PosY);
			// if the vehicle can also drive on land, we have to check,
			// whether there is a brige, platform, etc.
			// because the vehicle will drive on the bridge
			cBuilding* building = map.fields[map.getOffset (PosX, PosY)].getBaseBuilding();
			if (building && data.factorGround > 0 && (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)) water = false;

			if (water && (data.isStealthOn & TERRAIN_SEA) && detectedByPlayerList.empty() && owner == client->getActivePlayer()) alpha = 100;
		}
	}
	render_simple (surface, dest, zoomFactor, alpha);
}

//-----------------------------------------------------------------------------
/** Selects the vehicle */
//-----------------------------------------------------------------------------
void cVehicle::Select (cGameGUI& gameGUI)
{
	// load the video
	if (gameGUI.getFLC() != NULL) FLI_Close (gameGUI.getFLC());
	if (FileExists (typ->FLCFile))
	{
		gameGUI.setFLC (FLI_Open (SDL_RWFromFile (typ->FLCFile, "rb"), NULL));
	}
	else
	{
		//in case the flc video doesn't exist we use the storage image instead
		gameGUI.setFLC (NULL);
		gameGUI.setVideoSurface (typ->storage);
	}

	MakeReport (gameGUI);
	gameGUI.setUnitDetailsData (this);
}

bool cVehicle::refreshData_Build (cServer& server)
{
	if (IsBuilding == false || BuildRounds == 0) return false;

	data.storageResCur -= (BuildCosts / BuildRounds);
	BuildCosts -= (BuildCosts / BuildRounds);

	data.storageResCur = std::max (this->data.storageResCur, 0);

	BuildRounds--;
	if (BuildRounds != 0) return true;

	const cMap& map = *server.Map;
	server.addReport (BuildingTyp, false, owner->getNr());

	//handle pathbuilding
	//here the new building is added (if possible) and the move job to the next field is generated
	//the new build event is generated in cServer::handleMoveJobs()
	if (BuildPath)
	{
		// Find a next position that either a) is something we can't move to (in which case we cancel
		// the path building, or b) doesn't have a building type that we're trying to build.
		int  nextX       = PosX;
		int  nextY       = PosY;
		bool found_next  = false;

		while (!found_next && (nextX != BandX || nextY != BandY))
		{
			// Calculate the next position in the path.
			if (PosX > BandX) nextX--;
			if (PosX < BandX) nextX++;
			if (PosY > BandY) nextY--;
			if (PosY < BandY) nextY++;
			// Can we move to this position? If not, we need to kill the path building now.
			if (!map.possiblePlace (*this, nextX, nextY))
			{
				// Try sidestepping stealth units before giving up.
				server.sideStepStealthUnit (nextX, nextY, this);
				if (!map.possiblePlace (*this, nextX, nextY))
				{
					// We can't build along this path any more.
					break;
				}
			}
			// Can we build at this next position?
			if (map.possiblePlaceBuilding (BuildingTyp.getBuilding()->data, nextX, nextY))
			{
				// We can build here.
				found_next = true;
				break;
			}
		}

		// If we've found somewhere to move to, move there now.
		if (found_next && server.addMoveJob (PosX, PosY, nextX, nextY, this))
		{
			IsBuilding = false;
			server.addUnit (PosX, PosY, BuildingTyp.getBuilding(), owner);
			// Begin the movment immediately, so no other unit can block the destination field.
			this->ServerMoveJob->checkMove();
		}

		else
		{
			if (BuildingTyp.getUnitDataOriginalVersion()->surfacePosition != sUnitData::SURFACE_POS_GROUND)
			{
				server.addUnit (PosX, PosY, BuildingTyp.getBuilding(), owner);
				IsBuilding = false;
			}
			BuildPath = false;
			sendBuildAnswer (server, false, *this);
		}
	}
	else
	{
		//add building immediatly if it doesn't require the engineer to drive away
		if (BuildingTyp.getUnitDataOriginalVersion()->surfacePosition != data.surfacePosition)
		{
			IsBuilding = false;
			server.addUnit (PosX, PosY, BuildingTyp.getBuilding(), owner);
		}
	}
	return true;
}

bool cVehicle::refreshData_Clear (cServer& server)
{
	if (IsClearing == false || ClearingRounds == 0) return false;

	ClearingRounds--;

	cMap& map = *server.Map;

	if (ClearingRounds != 0) return true;

	IsClearing = false;
	cBuilding* Rubble = map.fields[map.getOffset (PosX, PosY)].getRubble();
	if (data.isBig)
	{
		int size = map.getSize();
		map.moveVehicle (*this, BuildBigSavedPos % size, BuildBigSavedPos / size);
		sendStopClear (server, *this, BuildBigSavedPos, owner->getNr());
		for (unsigned int i = 0; i < seenByPlayerList.size(); i++)
		{
			sendStopClear (server, *this, BuildBigSavedPos, seenByPlayerList[i]->getNr());
		}
	}
	else
	{
		sendStopClear (server, *this, -1, owner->getNr());
		for (unsigned int i = 0; i < seenByPlayerList.size(); i++)
		{
			sendStopClear (server, *this, -1, seenByPlayerList[i]->getNr());
		}
	}
	data.storageResCur += Rubble->RubbleValue;
	data.storageResCur = std::min (data.storageResMax, data.storageResCur);
	server.deleteRubble (Rubble);

	return true;
}

//-----------------------------------------------------------------------------
/** Initializes all unit data to its maxiumum values */
//-----------------------------------------------------------------------------

bool cVehicle::refreshData()
{
	if (turnsDisabled > 0)
	{
		lastSpeed = data.speedMax;

		if (data.ammoCur >= data.shotsMax)
			lastShots = data.shotsMax;
		else
			lastShots = data.ammoCur;

		return true;
	}
	if (data.speedCur < data.speedMax || data.shotsCur < data.shotsMax)
	{
		data.speedCur = data.speedMax;

		if (data.ammoCur >= data.shotsMax)
			data.shotsCur = data.shotsMax;
		else
			data.shotsCur = data.ammoCur;

		/*// Regenerieren:
		if (data.is_alien && data.hitpointsCur < data.hitpointsMax)
		{
			data.hitpointsCur++;
		}*/
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
/** Draws the path of the vehicle */
//-----------------------------------------------------------------------------
void cVehicle::DrawPath (cGameGUI& gameGUI)
{
	int mx = 0, my = 0, sp, save;
	SDL_Rect dest, ndest;
	sWaypoint* wp;

	if (!ClientMoveJob || !ClientMoveJob->Waypoints || owner != gameGUI.getClient()->getActivePlayer())
	{
		if (!BuildPath || (BandX == PosX && BandY == PosY) || gameGUI.mouseInputMode == placeBand) return;

		mx = PosX;
		my = PosY;

		if (mx < BandX)
			sp = 4;
		else if (mx > BandX)
			sp = 3;
		else if (my < BandY)
			sp = 1;
		else
			sp = 6;

		while (mx != BandX || my != BandY)
		{
			dest.x = 180 - (int) (gameGUI.getOffsetX() * gameGUI.getZoom()) + gameGUI.getTileSize() * mx;
			dest.y = 18 - (int) (gameGUI.getOffsetY() * gameGUI.getZoom()) + gameGUI.getTileSize() * my;

			SDL_BlitSurface (OtherData.WayPointPfeileSpecial[sp][64 - gameGUI.getTileSize()], NULL, buffer, &dest);

			if (mx < BandX)
				mx++;
			else if (mx > BandX)
				mx--;

			if (my < BandY)
				my++;
			else if (my > BandY)
				my--;
		}

		dest.x = 180 - (int) (gameGUI.getOffsetX() * gameGUI.getZoom()) + gameGUI.getTileSize() * mx;
		dest.y = 18 - (int) (gameGUI.getOffsetY() * gameGUI.getZoom()) + gameGUI.getTileSize() * my;

		SDL_BlitSurface (OtherData.WayPointPfeileSpecial[sp][64 - gameGUI.getTileSize()], NULL, buffer, &dest);
		return;
	}

	sp = data.speedCur;

	if (sp)
	{
		save = 0;
		sp += ClientMoveJob->iSavedSpeed;
	}
	else save = ClientMoveJob->iSavedSpeed;

	dest.x = 180 - (int) (gameGUI.getOffsetX() * gameGUI.getZoom()) + gameGUI.getTileSize() * PosX;
	dest.y = 18 - (int) (gameGUI.getOffsetY() * gameGUI.getZoom()) + gameGUI.getTileSize() * PosY;
	wp = ClientMoveJob->Waypoints;
	ndest = dest;

	while (wp)
	{
		if (wp->next)
		{
			ndest.x += mx = wp->next->X * gameGUI.getTileSize() - wp->X * gameGUI.getTileSize();
			ndest.y += my = wp->next->Y * gameGUI.getTileSize() - wp->Y * gameGUI.getTileSize();
		}
		else
		{
			ndest.x += mx;
			ndest.y += my;
		}

		if (sp == 0)
		{
			ClientMoveJob->drawArrow (dest, &ndest, true);
			sp += data.speedMax + save;
			save = 0;
		}
		else
		{
			ClientMoveJob->drawArrow (dest, &ndest, false);
		}

		dest = ndest;
		wp = wp->next;

		if (wp)
		{
			sp -= wp->Costs;

			if (wp->next && sp < wp->next->Costs)
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
string cVehicle::getStatusStr (const cGameGUI& gameGUI) const
{
	if (turnsDisabled > 0)
	{
		string sText;
		sText = lngPack.i18n ("Text~Comp~Disabled") + " (";
		sText += iToStr (turnsDisabled) + ")";
		return sText;
	}
	else if ( !(data.canCapture || data.canDisable) && owner == gameGUI.getClient()->getActivePlayer())
	{
		if (ClientMoveJob)
			return lngPack.i18n ("Text~Comp~Moving");
		else if (manualFireActive)
			return lngPack.i18n ("Text~Comp~ReactionFireOff");
		else if (sentryActive)
			return lngPack.i18n ("Text~Comp~Sentry");
	}
	else if (autoMJob)
		return lngPack.i18n ("Text~Comp~Surveying");
	else if (IsBuilding)
	{
		if (owner != gameGUI.getClient()->getActivePlayer())
			return lngPack.i18n ("Text~Comp~Producing");
		else
		{
			if (BuildRounds)
			{
				string sText;
				sText = lngPack.i18n ("Text~Comp~Producing");
				sText += ": ";
				sText += (string) owner->getUnitDataCurrentVersion (BuildingTyp)->name + " (";
				sText += iToStr (BuildRounds);
				sText += ")";

				if (font->getTextWide (sText) > 126)
				{
					sText = lngPack.i18n ("Text~Comp~Producing");
					sText += ":\n";
					sText += (string) owner->getUnitDataCurrentVersion (BuildingTyp)->name + " (";
					sText += iToStr (BuildRounds);
					sText += ")";
				}
				return sText;
			}
			else
				return lngPack.i18n ("Text~Comp~Producing_Fin");
		}
	}
	else if (ClearMines)
		return lngPack.i18n ("Text~Comp~Clearing_Mine");
	else if (LayMines)
		return lngPack.i18n ("Text~Comp~Laying");
	else if (IsClearing)
	{
		if (ClearingRounds)
		{
			string sText;
			sText = lngPack.i18n ("Text~Comp~Clearing") + " (";
			sText += iToStr (ClearingRounds) + ")";
			return sText;
		}
		else
			return lngPack.i18n ("Text~Comp~Clearing_Fin");
	}
	
	// for infiltrators
	else if ( (data.canCapture || data.canDisable) && owner == gameGUI.getClient()->getActivePlayer())
	{
		string sTmp;
		if (ClientMoveJob)
			sTmp = lngPack.i18n ("Text~Comp~Moving") + "\n";
		else if (manualFireActive)
			sTmp = lngPack.i18n ("Text~Comp~ReactionFireOff") + "\n";
		else if (sentryActive)
			sTmp = lngPack.i18n ("Text~Comp~Sentry") + "\n";
		else sTmp = lngPack.i18n ("Text~Comp~Waits") + "\n";

		if (CommandoRank < 1) sTmp += lngPack.i18n ("Text~Comp~CommandoRank_Greenhorn");
		else if (CommandoRank < 3) sTmp += lngPack.i18n ("Text~Comp~CommandoRank_Average");
		else if (CommandoRank < 6) sTmp += lngPack.i18n ("Text~Comp~CommandoRank_Veteran");
		else if (CommandoRank < 11) sTmp += lngPack.i18n ("Text~Comp~CommandoRank_Expert");
		else if (CommandoRank < 19) sTmp += lngPack.i18n ("Text~Comp~CommandoRank_Elite");
		else sTmp += lngPack.i18n ("Text~Comp~CommandoRank_GrandMaster");
		if (CommandoRank > 0)
			sTmp += " +" + iToStr ( (int) CommandoRank);
		return sTmp;
	}

	return lngPack.i18n ("Text~Comp~Waits");
}

//-----------------------------------------------------------------------------
/** Plays the soundstream, that belongs to this vehicle */
//-----------------------------------------------------------------------------
int cVehicle::playStream (const cGameGUI& gameGUI)
{
	const cClient& client = *gameGUI.getClient();
	const cMap& map = *client.getMap();
	const cBuilding* building = map[map.getOffset (PosX, PosY)].getBaseBuilding();
	bool water = map.isWater (PosX, PosY);
	if (data.factorGround > 0 && building && (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)) water = false;

	if (IsBuilding && (BuildRounds || client.getActivePlayer() != owner))
		return PlayFXLoop (SoundData.SNDBuilding);
	else if (IsClearing)
		return PlayFXLoop (SoundData.SNDClearing);
	else if (water && data.factorSea > 0)
		return PlayFXLoop (typ->WaitWater);
	else
		return PlayFXLoop (typ->Wait);
}

//-----------------------------------------------------------------------------
/** Starts the MoveSound */
//-----------------------------------------------------------------------------
void cVehicle::StartMoveSound (cGameGUI& gameGUI)
{
	const cMap& map = *gameGUI.getClient()->getMap();
	const cBuilding* building = map.fields[map.getOffset (PosX, PosY)].getBaseBuilding();
	bool water = map.isWater (PosX, PosY);
	if (data.factorGround > 0 && building && (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)) water = false;
	StopFXLoop (gameGUI.iObjectStream);

	if (!MoveJobActive)
	{
		if (water && data.factorSea != 0)
			PlayFX (typ->StartWater);
		else
			PlayFX (typ->Start);
	}

	if (water && data.factorSea != 0)
		gameGUI.iObjectStream = PlayFXLoop (typ->DriveWater);
	else
		gameGUI.iObjectStream = PlayFXLoop (typ->Drive);
}

//-----------------------------------------------------------------------------
/** Reduces the remaining speedCur and shotsCur during movement */
//-----------------------------------------------------------------------------
void cVehicle::DecSpeed (int value)
{
	data.speedCur -= value;

	if (data.canAttack)
	{
		float f;
		int s;
		f = ( (float) data.shotsMax / data.speedMax);
		s = (int) (data.speedCur * f);

		if (!data.canDriveAndFire && s < data.shotsCur && s >= 0)
			data.shotsCur = s;
	}
}

//-----------------------------------------------------------------------------
void cVehicle::calcTurboBuild (int* const iTurboBuildRounds, int* const iTurboBuildCosts, int iBuild_Costs)
{
	// calculate building time and costs
	int a, rounds, costs;

	iTurboBuildRounds[0] = 0;
	iTurboBuildRounds[1] = 0;
	iTurboBuildRounds[2] = 0;

	//prevent division by zero
	if (data.needsMetal == 0) data.needsMetal = 1;

	//step 1x
	if (data.storageResCur >= iBuild_Costs)
	{
		iTurboBuildCosts[0] = iBuild_Costs;
		iTurboBuildRounds[0] = (int) ceilf (iTurboBuildCosts[0] / (float) (data.needsMetal));
	}

	//step 2x
	a = iTurboBuildCosts[0];
	rounds = iTurboBuildRounds[0];
	costs = iTurboBuildCosts[0];

	while (a >= 4 && data.storageResCur >= costs + 4)
	{
		rounds--;
		costs += 4;
		a -= 4;
	}

	if (rounds < iTurboBuildRounds[0] && rounds > 0 && iTurboBuildRounds[0])
	{
		iTurboBuildCosts[1] = costs;
		iTurboBuildRounds[1] = rounds;
	}

	//step 4x
	a = iTurboBuildCosts[1];
	rounds = iTurboBuildRounds[1];
	costs = iTurboBuildCosts[1];

	while (a >= 10 && costs < data.storageResMax - 2)
	{
		int inc = 24 - min (16, a);
		if (costs + inc > data.storageResCur) break;

		rounds--;
		costs += inc;
		a -= 16;
	}

	if (rounds < iTurboBuildRounds[1] && rounds > 0 && iTurboBuildRounds[1])
	{
		iTurboBuildCosts[2] = costs;
		iTurboBuildRounds[2] = rounds;
	}
}

//-----------------------------------------------------------------------------
/** Finds the next fitting position for the band */
//-----------------------------------------------------------------------------
void cVehicle::FindNextband (cGameGUI& gameGUI)
{
	bool pos[4] = {false, false, false, false};
	int x = mouse->getKachelX (gameGUI);
	int y = mouse->getKachelY (gameGUI);
	const cMap& map = *gameGUI.getClient()->getMap();

	//check, which positions are available
	sUnitData BuildingType = *BuildingTyp.getUnitDataOriginalVersion();
	if (map.possiblePlaceBuilding (BuildingType, PosX - 1, PosY - 1)
		&& map.possiblePlaceBuilding (BuildingType, PosX    , PosY - 1)
		&& map.possiblePlaceBuilding (BuildingType, PosX - 1, PosY))
	{
		pos[0] = true;
	}

	if (map.possiblePlaceBuilding (BuildingType, PosX    , PosY - 1)
		&& map.possiblePlaceBuilding (BuildingType, PosX + 1, PosY - 1)
		&& map.possiblePlaceBuilding (BuildingType, PosX + 1, PosY))
	{
		pos[1] = true;
	}

	if (map.possiblePlaceBuilding (BuildingType, PosX + 1, PosY)
		&& map.possiblePlaceBuilding (BuildingType, PosX + 1, PosY + 1)
		&& map.possiblePlaceBuilding (BuildingType, PosX    , PosY + 1))
	{
		pos[2] = true;
	}

	if (map.possiblePlaceBuilding (BuildingType, PosX - 1, PosY)
		&& map.possiblePlaceBuilding (BuildingType, PosX - 1, PosY + 1)
		&& map.possiblePlaceBuilding (BuildingType, PosX    , PosY + 1))
	{
		pos[3] = true;
	}

	//chose the position, which matches the cursor position, if available
	if (x <= PosX && y <= PosY && pos[0])
	{
		BandX = PosX - 1;
		BandY = PosY - 1;
		return;
	}

	if (x >= PosX && y <= PosY && pos[1])
	{
		BandX = PosX;
		BandY = PosY - 1;
		return;
	}

	if (x >= PosX && y >= PosY && pos[2])
	{
		BandX = PosX;
		BandY = PosY;
		return;
	}

	if (x <= PosX && y >= PosY && pos[3])
	{
		BandX = PosX - 1;
		BandY = PosY;
		return;
	}

	//if the best position is not available, chose the next free one
	if (pos[0])
	{
		BandX = PosX - 1;
		BandY = PosY - 1;
		return;
	}

	if (pos[1])
	{
		BandX = PosX;
		BandY = PosY - 1;
		return;
	}

	if (pos[2])
	{
		BandX = PosX;
		BandY = PosY;
		return;
	}

	if (pos[3])
	{
		BandX = PosX - 1;
		BandY = PosY;
		return;
	}

	if (BuildingTyp.getUnitDataOriginalVersion()->isBig)
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
void cVehicle::doSurvey (const cServer& server)
{
	const cMap& map = *server.Map;
	const int minx = std::max (PosX - 1, 0);
	const int maxx = std::min (PosX + 1, map.getSize() - 1);
	const int miny = std::max (PosY - 1, 0);
	const int maxy = std::min (PosY + 1, map.getSize() - 1);

	for (int y = miny; y <= maxy; ++y)
		for (int x = minx; x <= maxx; ++x)
			owner->exploreResource (map.getOffset (x, y));
}

//-----------------------------------------------------------------------------
/** Makes the report */
//-----------------------------------------------------------------------------
void cVehicle::MakeReport (cGameGUI& gameGUI)
{
	if (owner != gameGUI.getClient()->getActivePlayer())
		return;

	if (turnsDisabled > 0)
	{
		// Disabled:
		PlayRandomVoice (VoiceData.VOIUnitDisabledByEnemy);
	}
	else if (data.hitpointsCur > data.hitpointsMax / 2)
	{
		// Status green
		if (ClientMoveJob && ClientMoveJob->endMoveAction && ClientMoveJob->endMoveAction->type_ == EMAT_ATTACK)
		{
			PlayRandomVoice (VoiceData.VOIAttacking);
		}
		else if (autoMJob)
		{
			PlayRandomVoice (VoiceData.VOISurveying);
		}
		else if (data.speedCur == 0)
		{
			// no more movement
			PlayVoice (VoiceData.VOINoSpeed);
		}
		else if (IsBuilding)
		{
			// Beim bau:
			if (!BuildRounds)
			{
				// Bau beendet:
				PlayRandomVoice (VoiceData.VOIBuildDone);
			}
		}
		else if (IsClearing)
		{
			// removing dirt
			PlayVoice (VoiceData.VOIClearing);
		}
		else if (data.canAttack && data.ammoCur <= data.ammoMax / 4 && data.ammoCur != 0)
		{
			// red ammo-status but still ammo left
			PlayRandomVoice (VoiceData.VOIAmmoLow);
		}
		else if (data.canAttack && data.ammoCur == 0)
		{
			// no ammo left
			PlayRandomVoice (VoiceData.VOIAmmoEmpty);
		}
		else if (sentryActive)
		{
			// on sentry:
			PlayVoice (VoiceData.VOISentry);
		}
		else if (ClearMines)
		{
			PlayRandomVoice (VoiceData.VOIClearingMines);
		}
		else if (LayMines)
		{
			PlayVoice (VoiceData.VOILayingMines);
		}
		else
		{
			PlayRandomVoice (VoiceData.VOIOK);
		}
	}
	else if (data.hitpointsCur > data.hitpointsMax / 4)
	{
		// Status yellow:
		PlayRandomVoice (VoiceData.VOIStatusYellow);
	}
	else
	{
		// Status red:
		PlayRandomVoice (VoiceData.VOIStatusRed);
	}
}

//-----------------------------------------------------------------------------
/** checks, if resources can be transfered to the unit */
//-----------------------------------------------------------------------------
bool cVehicle::CanTransferTo (const cGameGUI& gameGUI, cMapField* OverUnitField) const
{
	const int x = mouse->getKachelX (gameGUI);
	const int y = mouse->getKachelY (gameGUI);

	if (x < PosX - 1 || x > PosX + 1 || y < PosY - 1 || y > PosY + 1)
		return false;

	if (OverUnitField->getVehicle())
	{
		const cVehicle* v = OverUnitField->getVehicle();

		if (v == this)
			return false;

		if (v->owner != this->owner)
			return false;

		if (v->data.storeResType != data.storeResType)
			return false;

		if (v->IsBuilding || v->IsClearing)
			return false;

		return true;
	}
	else if (OverUnitField->getTopBuilding())
	{
		const cBuilding* b = OverUnitField->getTopBuilding();

		if (b->owner != this->owner)
			return false;

		if (!b->SubBase)
			return false;

		if (data.storeResType == sUnitData::STORE_RES_METAL && b->SubBase->MaxMetal == 0)
			return false;

		if (data.storeResType == sUnitData::STORE_RES_OIL && b->SubBase->MaxOil == 0)
			return false;

		if (data.storeResType == sUnitData::STORE_RES_GOLD && b->SubBase->MaxGold == 0)
			return false;

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::makeAttackOnThis (cServer& server, cUnit* opponentUnit, const string& reasonForLog) const
{
	cUnit* target = selectTarget (PosX, PosY, opponentUnit->data.canAttack, server.Map);
	if (target == this)
	{
		int iOff = server.Map->getOffset (PosX, PosY);
		Log.write (" Server: " + reasonForLog + ": attacking offset " + iToStr (iOff) + " Agressor ID: " + iToStr (opponentUnit->iID), cLog::eLOG_TYPE_NET_DEBUG);
		server.AJobs.push_back (new cServerAttackJob (server, opponentUnit, iOff, true));
		if (ServerMoveJob != 0)
			ServerMoveJob->bFinished = true;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::makeSentryAttack (cServer& server, cUnit* sentryUnit) const
{
	if (sentryUnit != 0 && sentryUnit->sentryActive && sentryUnit->canAttackObjectAt (PosX, PosY, server.Map, true))
	{
		if (makeAttackOnThis (server, sentryUnit, "sentry reaction"))
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::InSentryRange (cServer& server)
{
	int iOff = server.Map->getOffset (PosX, PosY);
	std::vector<cPlayer*>& playerList = *server.PlayerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		cPlayer* Player = playerList[i];

		if (Player == owner) continue;

		// Don't attack undiscovered stealth units
		if (data.isStealthOn != TERRAIN_NONE && !isDetectedByPlayer (Player)) continue;
		// Don't attack units out of scan range
		if (!Player->canSeeAnyAreaUnder (*this)) continue;
		// Check sentry type
		if (data.factorAir > 0 && Player->hasSentriesAir (iOff) == 0) continue;
		// Check sentry type
		if (data.factorAir == 0 && Player->hasSentriesGround (iOff) == 0) continue;

		for (cVehicle* vehicle = Player->VehicleList; vehicle; vehicle = vehicle->next)
		{
			if (makeSentryAttack (server, vehicle))
				return true;
		}
		for (cBuilding* building = Player->BuildingList; building; building = building->next)
		{
			if (makeSentryAttack (server, building))
				return true;
		}
	}

	return provokeReactionFire (server);
}

//-----------------------------------------------------------------------------
bool cVehicle::isOtherUnitOffendedByThis (cServer& server, const cUnit& otherUnit) const
{
	// don't treat the cheap buildings (connectors, roads, beton blocks) as offendable
	bool otherUnitIsCheapBuilding = (otherUnit.isBuilding() && otherUnit.data.ID.getUnitDataOriginalVersion()->buildCosts > 2); //FIXME: isn't the check inverted?

	if (otherUnitIsCheapBuilding == false
		&& isInRange (otherUnit.PosX, otherUnit.PosY)
		&& canAttackObjectAt (otherUnit.PosX, otherUnit.PosY, server.Map, true, false))
	{
		// test, if this vehicle can really attack the opponentVehicle
		cUnit* target = selectTarget (otherUnit.PosX, otherUnit.PosY, data.canAttack, server.Map);
		if (target == &otherUnit)
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::doesPlayerWantToFireOnThisVehicleAsReactionFire (cServer& server, const cPlayer* player) const
{
	if (server.isTurnBasedGame())
	{
		// In the turn based game style, the opponent always fires on the unit if he can, regardless if the unit is offending or not.
		return true;
	}
	else
	{
		// check if there is a vehicle or building of player, that is offended

		for (const cVehicle* opponentVehicle = player->VehicleList;
			 opponentVehicle != 0;
			 opponentVehicle = opponentVehicle->next)
		{
			if (isOtherUnitOffendedByThis (server, *opponentVehicle))
				return true;
		}
		for (const cBuilding* opponentBuilding = player->BuildingList;
			 opponentBuilding != 0;
			 opponentBuilding = opponentBuilding->next)
		{
			if (isOtherUnitOffendedByThis (server, *opponentBuilding))
				return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::doReactionFireForUnit (cServer& server, cUnit* opponentUnit) const
{
	if (opponentUnit->sentryActive == false && opponentUnit->manualFireActive == false
		&& opponentUnit->canAttackObjectAt (PosX, PosY, server.Map, true)
		// Possible TODO: better handling of stealth units. e.g. do reaction fire, if already detected?
		&& (opponentUnit->isVehicle() == false || opponentUnit->data.isStealthOn == TERRAIN_NONE))
	{
		if (makeAttackOnThis (server, opponentUnit, "reaction fire"))
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::doReactionFire (cServer& server, cPlayer* player) const
{
	// search a unit of the opponent, that could fire on this vehicle
	// first look for a building
	for (cBuilding* opponentBuilding = player->BuildingList;
		 opponentBuilding != 0;
		 opponentBuilding = opponentBuilding->next)
	{
		if (doReactionFireForUnit (server, opponentBuilding))
			return true;
	}
	for (cVehicle* opponentVehicle = player->VehicleList;
		 opponentVehicle != 0;
		 opponentVehicle = opponentVehicle->next)
	{
		if (doReactionFireForUnit (server, opponentVehicle))
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::provokeReactionFire (cServer& server)
{
	// unit can't fire, so it can't provoke a reaction fire
	if (data.canAttack == false || data.shotsCur <= 0 || data.ammoCur <= 0)
		return false;

	std::vector<cPlayer*>& playerList = *server.PlayerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		cPlayer* player = playerList[i];
		if (player == owner)
			continue;

		if (data.isStealthOn != TERRAIN_NONE && !isDetectedByPlayer (player))
			continue;
		// The vehicle can't be seen by the opposing player.
		// No possibility for reaction fire.
		if (player->canSeeAnyAreaUnder (*this) == false)
			continue;

		if (doesPlayerWantToFireOnThisVehicleAsReactionFire (server, player) == false)
			continue;

		if (doReactionFire (server, player))
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
/** Draws exitpoints for a vehicle, that should be exited */
//-----------------------------------------------------------------------------
void cVehicle::DrawExitPoints (const sVehicle* const typ, cGameGUI& gameGUI) const
{
	int const spx = getScreenPosX (gameGUI);
	int const spy = getScreenPosY (gameGUI);
	const cMap* map = gameGUI.getClient()->getMap();
	const int tilesize = gameGUI.getTileSize();
	T_2<int> offsets[8] = {T_2<int> (-1, -1), T_2<int> (0, -1), T_2<int> (1, -1),
						   T_2<int> (-1, 0),                  T_2<int> (1, 0),
						   T_2<int> (-1, 1), T_2<int> (0,  1), T_2<int> (1, 1)
						  };

	for (int i = 0; i != 8; ++i)
	{
		if (canExitTo (PosX + offsets[i].x, PosY + offsets[i].y, map, typ))
			gameGUI.drawExitPoint (spx + offsets[i].x * tilesize, spy + offsets[i].y * tilesize);
	}
}

//-----------------------------------------------------------------------------
bool cVehicle::canExitTo (const int x, const int y, const cMap* map, const sVehicle* typ) const
{
	if (!map->possiblePlaceVehicle (typ->data, x, y, owner)) return false;
	if (data.factorAir > 0 && (x != PosX || y != PosY)) return false;
	if (!isNextTo (x, y)) return false;

	return true;
}

//-----------------------------------------------------------------------------
bool cVehicle::canLoad (int x, int y, const cMap* Map, bool checkPosition) const
{
	if (Map->isValidPos (x, y) == false) return false;

	return canLoad (Map->fields[Map->getOffset (x, y)].getVehicle(), checkPosition);
}

//-----------------------------------------------------------------------------
bool cVehicle::canLoad (const cVehicle* Vehicle, bool checkPosition) const
{
	if (!Vehicle) return false;

	if (Vehicle->Loaded) return false;

	if (data.storageUnitsCur >= data.storageUnitsMax)	return false;

	if (checkPosition && !isNextTo (Vehicle->PosX, Vehicle->PosY)) return false;

	if (checkPosition && data.factorAir > 0 && (Vehicle->PosX != PosX || Vehicle->PosY != PosY)) return false;

	size_t i;
	for (i = 0; i < data.storeUnitsTypes.size(); i++)
	{
		if (data.storeUnitsTypes[i].compare (Vehicle->data.isStorageType) == 0) break;
	}
	if (i == data.storeUnitsTypes.size()) return false;

	if (Vehicle->ClientMoveJob && (Vehicle->moving || Vehicle->attacking || Vehicle->MoveJobActive)) return false;

	if (Vehicle->owner != owner || Vehicle->IsBuilding || Vehicle->IsClearing) return false;

	if (Vehicle->isBeeingAttacked) return false;

	return true;
}

//-----------------------------------------------------------------------------
void cVehicle::storeVehicle (cVehicle* Vehicle, cMap* Map)
{
	Map->deleteVehicle (*Vehicle);
	if (Vehicle->sentryActive)
	{
		Vehicle->owner->deleteSentry (Vehicle);
	}

	Vehicle->manualFireActive = false;

	Vehicle->Loaded = true;

	storedUnits.push_back (Vehicle);
	data.storageUnitsCur++;

	owner->doScan();
}

//-----------------------------------------------------------------------------
/** Exits a vehicle */
//-----------------------------------------------------------------------------
void cVehicle::exitVehicleTo (cVehicle* Vehicle, int offset, cMap* Map)
{
	Remove (storedUnits, Vehicle);

	data.storageUnitsCur--;

	Map->addVehicle (*Vehicle, offset);

	Vehicle->PosX = offset % Map->getSize();
	Vehicle->PosY = offset / Map->getSize();
	Vehicle->Loaded = false;
	//Vehicle->data.shotsCur = 0;

	owner->doScan();
}

//-----------------------------------------------------------------------------
/** Checks, if an object can get ammunition. */
//-----------------------------------------------------------------------------
bool cVehicle::canSupply (const cClient& client, int x, int y, int supplyType) const
{
	const cMap& map = *client.getMap();

	if (map.isValidPos (x, y) == false) return false;

	cMapField& field = map.fields[map.getOffset (x, y)];
	if (field.getVehicle()) return canSupply (field.getVehicle(), supplyType);
	else if (field.getPlane()) return canSupply (field.getPlane(), supplyType);
	else if (field.getTopBuilding()) return canSupply (field.getTopBuilding(), supplyType);

	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::canSupply (const cUnit* unit, int supplyType) const
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

	if (unit->isVehicle() && unit->data.factorAir > 0 && static_cast<const cVehicle*> (unit)->FlightHigh > 0)
		return false;

	switch (supplyType)
	{
		case SUPPLY_TYPE_REARM:
			if (unit == this || unit->data.canAttack == false || unit->data.ammoCur >= unit->data.ammoMax
				|| (unit->isVehicle() && static_cast<const cVehicle*> (unit)->isUnitMoving())
				|| unit->attacking)
				return false;
			break;
		case SUPPLY_TYPE_REPAIR:
			if (unit == this || unit->data.hitpointsCur >= unit->data.hitpointsMax
				|| (unit->isVehicle() && static_cast<const cVehicle*> (unit)->isUnitMoving())
				|| unit->attacking)
				return false;
			break;
		default:
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool cVehicle::layMine (cServer& server)
{
	if (data.storageResCur <= 0) return false;

	const cMap& map = *server.Map;
	if ( (data.factorSea > 0 && data.factorGround == 0))
	{
		if (!map.possiblePlaceBuilding (*UnitsData.specialIDSeaMine.getUnitDataOriginalVersion(), PosX, PosY, this)) return false;
		server.addUnit (PosX, PosY, UnitsData.specialIDSeaMine.getBuilding(), owner, false);
	}
	else
	{
		if (!map.possiblePlaceBuilding (*UnitsData.specialIDLandMine.getUnitDataOriginalVersion(), PosX, PosY, this)) return false;
		server.addUnit (PosX, PosY, UnitsData.specialIDLandMine.getBuilding(), owner, false);
	}
	data.storageResCur--;

	if (data.storageResCur <= 0) LayMines = false;

	return true;
}

//-----------------------------------------------------------------------------
bool cVehicle::clearMine (cServer& server)
{
	const cMap& map = *server.Map;
	cBuilding* Mine = map.fields[map.getOffset (PosX, PosY)].getMine();

	if (!Mine || Mine->owner != owner || data.storageResCur >= data.storageResMax) return false;
	if (Mine->data.factorGround > 0 && data.factorGround == 0) return false;
	if (Mine->data.factorSea > 0 && data.factorSea == 0) return false;

	server.deleteUnit (Mine);
	data.storageResCur++;

	if (data.storageResCur >= data.storageResMax) ClearMines = false;

	return true;
}

//-----------------------------------------------------------------------------
/** Checks if the target is on a neighbour field and if it can be stolen or disabled */
//-----------------------------------------------------------------------------
bool cVehicle::canDoCommandoAction (int x, int y, const cMap* map, bool steal) const
{
	if ( (steal && data.canCapture == false) || (steal == false && data.canDisable == false))
		return false;

	if (isNextTo (x, y) == false || data.shotsCur == 0)
		return false;

	int off = map->getOffset (x, y);
	const cUnit* vehicle  = map->fields[off].getVehicle();
	const cUnit* building = map->fields[off].getBuilding();
	const cUnit* unit = vehicle ? vehicle : building;

	if (unit == NULL) return false;

	if (unit->isBuilding() && unit->owner == 0) return false;   // rubble
	if (steal && unit->data.canBeCaptured == false) return false;
	if (steal == false && unit->data.canBeDisabled == false) return false;
	if (steal && unit->storedUnits.size() > 0) return false;
	if (unit->owner == owner) return false;
	if (unit->isVehicle() && unit->data.factorAir > 0 && static_cast<const cVehicle*> (unit)->FlightHigh > 0) return false;

	return true;
}

//-----------------------------------------------------------------------------
/** draws the commando-cursors: */
//-----------------------------------------------------------------------------
void cVehicle::drawCommandoCursor (cGameGUI& gameGUI, int x, int y, bool steal) const
{
	cMap& map = *gameGUI.getClient()->getMap();
	cMapField& field = map.fields[map.getOffset (x, y)];
	SDL_Surface* sf;
	const cUnit* unit = 0;

	if (steal)
	{
		unit = field.getVehicle();
		sf = GraphicsData.gfx_Csteal;
	}
	else
	{
		unit = field.getVehicle();
		if (unit == 0)
			unit = field.getTopBuilding();
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
int cVehicle::calcCommandoChance (const cUnit* destUnit, bool steal) const
{
	if (destUnit == 0)
		return 0;

	// TODO: include cost research and clan modifications? Or should always the basic version without clanmods be used?
	// TODO: Bug for buildings? /3? or correctly /2, because constructing buildings takes two resources per turn?
	int destTurn = destUnit->data.buildCosts / 3;

	int factor = steal ? 1 : 4;
	int srcLevel = (int) CommandoRank + 7;

	// The chance to disable or steal a unit depends on
	// the infiltrator ranking and the buildcosts
	// (or 'turns' in the original game) of the target unit.
	// The chance rises linearly with a higher ranking of the infiltrator.
	// The chance of a unexperienced infiltrator will be calculated like
	// he has the ranking 7.
	// Disabling has a 4 times higher chance than stealing.
	int chance = Round ( (8.f * srcLevel) / (35 * destTurn) * factor * 100);
	chance = std::min (90, chance);

	return chance;
}

//-----------------------------------------------------------------------------
int cVehicle::calcCommandoTurns (const cUnit* destUnit) const
{
	if (destUnit == 0)
		return 1;

	int vehiclesTable[13] = { 0, 0, 0, 5, 8, 3, 3, 0, 0, 0, 1, 0, -4 };
	int destTurn, srcLevel;

	if (destUnit->isVehicle())
	{
		destTurn = destUnit->data.buildCosts / 3;
		srcLevel = (int) CommandoRank;
		if (destTurn > 0 && destTurn < 13)
			srcLevel += vehiclesTable[destTurn];
	}
	else
	{
		destTurn = destUnit->data.buildCosts / 2;
		srcLevel = (int) CommandoRank + 8;
	}

	int turns = (int) (1.0f / destTurn * srcLevel);
	turns = std::max (turns, 1);
	return turns;
}

//-----------------------------------------------------------------------------
bool cVehicle::isDetectedByPlayer (const cPlayer* player) const
{
	return Contains (detectedByPlayerList, player);
}

//-----------------------------------------------------------------------------
void cVehicle::setDetectedByPlayer (cServer& server, cPlayer* player, bool addToDetectedInThisTurnList)
{
	bool wasDetected = (detectedByPlayerList.size() > 0);

	if (!isDetectedByPlayer (player))
		detectedByPlayerList.push_back (player);

	if (!wasDetected) sendDetectionState (server, *this);

	if (addToDetectedInThisTurnList && Contains (detectedInThisTurnByPlayerList, player) == false)
		detectedInThisTurnByPlayerList.push_back (player);
}

//-----------------------------------------------------------------------------
void cVehicle::resetDetectedByPlayer (cServer& server, cPlayer* player)
{
	bool wasDetected = (detectedByPlayerList.size() > 0);

	Remove (detectedByPlayerList, player);
	Remove (detectedInThisTurnByPlayerList, player);

	if (wasDetected && detectedByPlayerList.size() == 0) sendDetectionState (server, *this);
}

//-----------------------------------------------------------------------------
bool cVehicle::wasDetectedInThisTurnByPlayer (const cPlayer* player) const
{
	return Contains (detectedInThisTurnByPlayerList, player);
}

//-----------------------------------------------------------------------------
void cVehicle::clearDetectedInThisTurnPlayerList()
{
	detectedInThisTurnByPlayerList.clear();
}

//-----------------------------------------------------------------------------
void cVehicle::tryResetOfDetectionStateAfterMove (cServer& server)
{
	std::vector<cPlayer*> playersThatDetectThisVehicle = calcDetectedByPlayer (server);

	bool foundPlayerToReset = true;
	while (foundPlayerToReset)
	{
		foundPlayerToReset = false;
		for (unsigned int i = 0; i < detectedByPlayerList.size(); i++)
		{
			if (Contains (playersThatDetectThisVehicle, detectedByPlayerList[i]) == false
				&& Contains (detectedInThisTurnByPlayerList, detectedByPlayerList[i]) == false)
			{
				resetDetectedByPlayer (server, detectedByPlayerList[i]);
				foundPlayerToReset = true;
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
std::vector<cPlayer*> cVehicle::calcDetectedByPlayer (cServer& server) const
{
	std::vector<cPlayer*> playersThatDetectThisVehicle;
	//check whether the vehicle has been detected by others
	if (data.isStealthOn != TERRAIN_NONE)   // the vehicle is a stealth vehicle
	{
		cMap& map = *server.Map;
		int offset = map.getOffset (PosX, PosY);
		std::vector<cPlayer*>& playerList = *server.PlayerList;
		for (unsigned int i = 0; i < playerList.size(); i++)
		{
			cPlayer* player = playerList[i];
			if (player == owner)
				continue;
			bool isOnWater = map.isWater (offset);
			bool isOnCoast = map.isCoast (offset) && (isOnWater == false);

			//if the vehicle can also drive on land, we have to check, whether there is a brige, platform, etc.
			//because the vehicle will drive on the bridge
			const cBuilding* building = map.fields[offset].getBaseBuilding();
			if (data.factorGround > 0 && building
				&& (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE
					|| building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA
					|| building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE))
			{
				isOnWater = false;
				isOnCoast = false;
			}

			if ( (data.isStealthOn & TERRAIN_GROUND)
				 && (player->hasLandDetection (offset) || (! (data.isStealthOn & TERRAIN_COAST) && isOnCoast)
					 || isOnWater))
			{
				playersThatDetectThisVehicle.push_back (player);
			}

			if ( (data.isStealthOn & TERRAIN_SEA)
				 && (player->hasSeaDetection (offset) || isOnWater == false))
			{
				playersThatDetectThisVehicle.push_back (player);
			}
		}
	}
	return playersThatDetectThisVehicle;
}

//-----------------------------------------------------------------------------
void cVehicle::makeDetection (cServer& server)
{
	//check whether the vehicle has been detected by others
	std::vector<cPlayer*> playersThatDetectThisVehicle = calcDetectedByPlayer (server);
	for (unsigned int i = 0; i < playersThatDetectThisVehicle.size(); i++)
		setDetectedByPlayer (server, playersThatDetectThisVehicle[i]);

	//detect other units
	if (data.canDetectStealthOn == false) return;

	cMap& map = *server.Map;
	const int minx = std::max (PosX - data.scan, 0);
	const int maxx = std::min (PosX + data.scan, map.getSize() - 1);
	const int miny = std::max (PosY - data.scan, 0);
	const int maxy = std::min (PosY + data.scan, map.getSize() - 1);

	for (int x = minx; x <= maxx; ++x)
	{
		for (int y = miny; y <= maxy; ++y)
		{
			const int offset = map.getOffset (x, y);
			cVehicle* vehicle = map.fields[offset].getVehicle();
			cBuilding* building = map.fields[offset].getMine();

			if (vehicle && vehicle->owner != owner)
			{
				if ( (data.canDetectStealthOn & TERRAIN_GROUND) && owner->hasLandDetection (offset) && (vehicle->data.isStealthOn & TERRAIN_GROUND))
				{
					vehicle->setDetectedByPlayer (server, owner);
				}
				if ( (data.canDetectStealthOn & TERRAIN_SEA) && owner->hasSeaDetection (offset) && (vehicle->data.isStealthOn & TERRAIN_SEA))
				{
					vehicle->setDetectedByPlayer (server, owner);
				}
			}
			if (building && building->owner != owner)
			{
				if ( (data.canDetectStealthOn & AREA_EXP_MINE) && owner->hasMineDetection (offset) && (building->data.isStealthOn & AREA_EXP_MINE))
				{
					building->setDetectedByPlayer (server, owner);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
sVehicle::sVehicle() :
	build (NULL), build_org (NULL),
	build_shw (NULL), build_shw_org (NULL),
	clear_small (NULL), clear_small_org (NULL),
	clear_small_shw (NULL), clear_small_shw_org (NULL),
	overlay (NULL), overlay_org (NULL),
	storage (NULL),
	FLCFile (NULL),
	data(),
	nr (-1),
	info (NULL),
	Wait (NULL), WaitWater (NULL),
	Start (NULL), StartWater (NULL),
	Stop (NULL), StopWater (NULL),
	Drive (NULL), DriveWater (NULL),
	Attack (NULL)
{
	for (int i = 0; i != 8; ++i) img[i] = NULL;
	for (int i = 0; i != 8; ++i) img_org[i] = NULL;
	for (int i = 0; i != 8; ++i) shw[i] = NULL;
	for (int i = 0; i != 8; ++i) shw_org[i] = NULL;
}

//-----------------------------------------------------------------------------
void sVehicle::scaleSurfaces (float factor)
{
	int width, height;
	for (int i = 0; i < 8; i++)
	{
		width = (int) (img_org[i]->w * factor);
		height = (int) (img_org[i]->h * factor);
		scaleSurface (img_org[i], img[i], width, height);
		width = (int) (shw_org[i]->w * factor);
		height = (int) (shw_org[i]->h * factor);
		scaleSurface (shw_org[i], shw[i], width, height);
	}
	if (build_org)
	{
		height = (int) (build_org->h * factor);
		width = height * 4;
		scaleSurface (build_org, build, width, height);
		width = (int) (build_shw_org->w * factor);
		height = (int) (build_shw_org->h * factor);
		scaleSurface (build_shw_org, build_shw, width, height);
	}
	if (clear_small_org)
	{
		height = (int) (clear_small_org->h * factor);
		width = height * 4;
		scaleSurface (clear_small_org, clear_small, width, height);
		width = (int) (clear_small_shw_org->w * factor);
		height = (int) (clear_small_shw_org->h * factor);
		scaleSurface (clear_small_shw_org, clear_small_shw, width, height);
	}
	if (overlay_org)
	{
		height = (int) (overlay_org->h * factor);
		width = (int) (overlay_org->w * factor);
		scaleSurface (overlay_org, overlay, width, height);
	}
}

//-----------------------------------------------------------------------------
void cVehicle::blitWithPreScale (SDL_Surface* org_src, SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dest, SDL_Rect* destrect, float factor, int frames)
{
	if (!cSettings::getInstance().shouldDoPrescale())
	{
		int width, height;
		height = (int) (org_src->h * factor);
		if (frames > 1) width = height * frames;
		else width = (int) (org_src->w * factor);
		if (src->w != width || src->h != height)
		{
			scaleSurface (org_src, src, width, height);
		}
	}
	blittAlphaSurface (src, srcrect, dest, destrect);
}

cBuilding* cVehicle::getContainerBuilding()
{
	if (!Loaded) return NULL;

	for (cBuilding* building = owner->BuildingList; building; building = building->next)
		if (Contains (building->storedUnits, this)) return building;

	return NULL;
}

cVehicle* cVehicle::getContainerVehicle()
{
	if (!Loaded) return NULL;

	for (cVehicle* vehicle = owner->VehicleList; vehicle; vehicle = vehicle->next)
		if (Contains (vehicle->storedUnits, this)) return vehicle;

	return NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- methods, that already have been extracted as part of cUnit refactoring
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
bool cVehicle::canBeStoppedViaUnitMenu() const
{
	return (ClientMoveJob != 0 || (isUnitBuildingABuilding() && BuildRounds > 0) || (isUnitClearing() && ClearingRounds > 0));
}

//-----------------------------------------------------------------------------
void cVehicle::executeBuildCommand (cGameGUI& gameGUI)
{
	if (ClientMoveJob)
		sendWantStopMove (*gameGUI.getClient(), iID);
	cBuildingsBuildMenu buildMenu (*gameGUI.getClient(), owner, this);
	buildMenu.show (gameGUI.getClient());
}

//-----------------------------------------------------------------------------
void cVehicle::executeStopCommand (const cClient& client)
{
	if (ClientMoveJob != 0)
		sendWantStopMove (client, iID);
	else if (isUnitBuildingABuilding())
		sendWantStopBuilding (client, iID);
	else if (isUnitClearing())
		sendWantStopClear (client, *this);
}

//-----------------------------------------------------------------------------
void cVehicle::executeAutoMoveJobCommand (cClient& client)
{
	if (data.canSurvey == false)
		return;
	if (autoMJob == 0)
	{
		autoMJob = new cAutoMJob (client, this);
	}
	else
	{
		delete autoMJob;
		autoMJob = 0;
	}
}

//-----------------------------------------------------------------------------
void cVehicle::executeActivateStoredVehiclesCommand (cClient& client)
{
	cStorageMenu storageMenu (client, storedUnits, *this);
	storageMenu.show (&client);
}

//-----------------------------------------------------------------------------
void cVehicle::executeLayMinesCommand (const cClient& client)
{
	LayMines = !LayMines;
	ClearMines = false;
	sendMineLayerStatus (client, *this);
}

//-----------------------------------------------------------------------------
void cVehicle::executeClearMinesCommand (const cClient& client)
{
	ClearMines = !ClearMines;
	LayMines = false;
	sendMineLayerStatus (client, *this);
}

//-----------------------------------------------------------------------------
sUnitData* cVehicle::getUpgradedUnitData() const
{
	return & (owner->VehicleData[typ->nr]);
}

bool cVehicle::canLand (const cMap& map) const
{
	if (data.factorAir == 0)  return true;    //true, because normal vehicles are always "landed"

	if (moving || ClientMoveJob || (ServerMoveJob && ServerMoveJob->Waypoints && ServerMoveJob->Waypoints->next) || attacking) return false;     //vehicle busy?

	//landing pad there?
	const std::vector<cBuilding*>& buildings = map[map.getOffset (PosX, PosY)].getBuildings();
	std::vector<cBuilding*>::const_iterator b_it = buildings.begin();
	for (; b_it != buildings.end(); ++b_it)
	{
		if ( (*b_it)->data.canBeLandedOn)
			break;
	}
	if (b_it == buildings.end()) return false;

	//is the landing pad already occupied?
	const std::vector<cVehicle*>& v = map[map.getOffset (PosX, PosY)].getPlanes();
	for (std::vector<cVehicle*>::const_iterator it = v.begin(); it != v.end(); ++it)
	{
		const cVehicle& vehicle = **it;
		if (vehicle.FlightHigh < 64 && vehicle.iID != iID)
			return false;
	}

	//returning true before checking owner, because a stolen vehicle
	//can stay on an enemy landing pad until it is moved
	if (FlightHigh == 0) return true;

	if ( (*b_it)->owner != owner) return false;

	return true;
}
