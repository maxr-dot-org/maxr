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
#include <cmath>

#include "vehicles.h"

#include "attackJobs.h"
#include "automjobs.h"
#include "buildings.h"
#include "client.h"
#include "clientevents.h"
#include "clist.h"
#include "files.h"
#include "fxeffects.h"
#include "hud.h"
#include "log.h"
#include "map.h"
#include "menus.h"
#include "pcx.h"
#include "player.h"
#include "server.h"
#include "settings.h"
#include "video.h"
#include "sound.h"
#include "unifonts.h"
#include "input/mouse/mouse.h"
#include "gui/application.h"
#include "gui/menu/windows/windowbuildbuildings/windowbuildbuildings.h"

using namespace std;

//-----------------------------------------------------------------------------
// cVehicle Class Implementation
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
cVehicle::cVehicle (const sUnitData& v, cPlayer* Owner, unsigned int ID) :
	cUnit (Owner ? Owner->getUnitDataCurrentVersion (v.ID) : &v, Owner, ID),
	next (0),
	prev (0),
	loaded (false),
	isBuilding (false),
	buildingTyp (),
	buildCosts (0),
	buildTurns (0),
	buildTurnsStart (0),
	buildCostsStart (0),
	isClearing (false),
	clearingTurns (0),
	layMines (false),
	clearMines (false),
	commandoRank (0)
{
	uiData = UnitsData.getVehicleUI (v.ID);
	bandPosition = 0;
	tileMovementOffset = 0;
	ditherX = 0;
	ditherY = 0;
	StartUp = 0;
	FlightHigh = 0;
	WalkFrame = 0;
	buildBigSavedPosition = 0;
	data.setHitpoints(data.hitpointsMax);
	data.setAmmo(data.ammoMax);
	ClientMoveJob = NULL;
	ServerMoveJob = NULL;
	autoMJob = NULL;
	hasAutoMoveJob = false;
	moving = false;
	MoveJobActive = false;
	BuildPath = false;
	BigBetonAlpha = 0;
	lastShots = 0;
	lastSpeed = 0;

	DamageFXPointX = random (7) + 26 - 3;
	DamageFXPointY = random (7) + 26 - 3;
	refreshData();

	clearingTurnsChanged.connect ([&](){ statusChanged (); });
	buildingTurnsChanged.connect ([&](){ statusChanged (); });
	buildingTypeChanged.connect ([&](){ statusChanged (); });
	commandoRankChanged.connect ([&](){ statusChanged (); });
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

namespace
{

// TODO: Factorize with code in menuitems.cpp
void DrawRectangle (SDL_Surface* surface, const SDL_Rect& rectangle, Uint32 color, Uint16 borderSize)
{
	SDL_Rect line_h = {rectangle.x, rectangle.y, rectangle.w, borderSize};
	SDL_FillRect (surface, &line_h, color);
	line_h.y += rectangle.h - borderSize;
	SDL_FillRect (surface, &line_h, color);
	SDL_Rect line_v = {rectangle.x, rectangle.y, borderSize, rectangle.h};
	SDL_FillRect (surface, &line_v, color);
	line_v.x += rectangle.w - borderSize;
	SDL_FillRect (surface, &line_v, color);
}

// TODO: Factorize with code in menuitems.cpp
void DrawSelectionCorner (SDL_Surface* surface, const SDL_Rect& rectangle, Uint16 cornerSize, Uint32 color)
{
	SDL_Rect line_h = { rectangle.x, rectangle.y, cornerSize, 1 };
	SDL_FillRect (surface, &line_h, color);
	line_h.x += rectangle.w - 1 - cornerSize;
	SDL_FillRect (surface, &line_h, color);
	line_h.x = rectangle.x;
	line_h.y += rectangle.h - 1;
	SDL_FillRect (surface, &line_h, color);
	line_h.x += rectangle.w - 1 - cornerSize;
	SDL_FillRect (surface, &line_h, color);

	SDL_Rect line_v = { rectangle.x, rectangle.y, 1, cornerSize };
	SDL_FillRect (surface, &line_v, color);
	line_v.y += rectangle.h - 1 - cornerSize;
	SDL_FillRect (surface, &line_v, color);
	line_v.x += rectangle.w - 1;
	line_v.y = rectangle.y;
	SDL_FillRect (surface, &line_v, color);
	line_v.y += rectangle.h - 1 - cornerSize;
	SDL_FillRect (surface, &line_v, color);
}

}

void cVehicle::drawOverlayAnimation (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, int frameNr, int alpha) const
{
	if (data.hasOverlay == false || cSettings::getInstance().isAnimations() == false) return;

	const Uint16 size = (Uint16) (uiData->overlay_org->h * zoomFactor);
	SDL_Rect src = {Sint16 (frameNr * size), 0, size, size};

	SDL_Rect tmp = dest;
	const int offset = Round (64.0f * zoomFactor) / 2 - src.h / 2;
	tmp.x += offset;
	tmp.y += offset;

	SDL_SetSurfaceAlphaMod (uiData->overlay, alpha);
	blitWithPreScale (uiData->overlay_org, uiData->overlay, &src, surface, &tmp, zoomFactor);
}

void cVehicle::drawOverlayAnimation (unsigned long long animationTime, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor) const
{
	if (data.hasOverlay == false || cSettings::getInstance().isAnimations() == false) return;

	int frameNr = 0;
	if (isDisabled() == false)
	{
		frameNr = animationTime % (uiData->overlay_org->w / uiData->overlay_org->h);
	}

	int alpha = 254;
	if (StartUp && cSettings::getInstance().isAlphaEffects())
		alpha = StartUp;
	drawOverlayAnimation (surface, dest, zoomFactor, frameNr, alpha);
}

void cVehicle::render_BuildingOrBigClearing (const cMap& map, unsigned long long animationTime, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const
{
	assert ((isUnitBuildingABuilding () || (isUnitClearing () && data.isBig)) && job == NULL);
	// draw beton if necessary
	SDL_Rect tmp = dest;
	if (isUnitBuildingABuilding () && data.isBig && (!map.isWaterOrCoast (getPosition()) || map.getField(getPosition()).getBaseBuilding ()))
	{
		SDL_SetSurfaceAlphaMod (GraphicsData.gfx_big_beton.get (), BigBetonAlpha);
		CHECK_SCALING (*GraphicsData.gfx_big_beton, *GraphicsData.gfx_big_beton_org, zoomFactor);
		SDL_BlitSurface (GraphicsData.gfx_big_beton.get (), NULL, surface, &tmp);
	}

	// draw shadow
	tmp = dest;
	if (drawShadow) blitWithPreScale (uiData->build_shw_org, uiData->build_shw, NULL, surface, &tmp, zoomFactor);

	// draw player color
	SDL_Rect src;
	src.y = 0;
	src.h = src.w = (int) (uiData->build_org->h * zoomFactor);
	src.x = (animationTime % 4) * src.w;
	SDL_BlitSurface (owner->getColorSurface (), NULL, GraphicsData.gfx_tmp.get (), NULL);
	blitWithPreScale (uiData->build_org, uiData->build, &src, GraphicsData.gfx_tmp.get (), NULL, zoomFactor, 4);

	// draw vehicle
	src.x = 0;
	src.y = 0;
	tmp = dest;
	SDL_SetSurfaceAlphaMod (GraphicsData.gfx_tmp.get (), 254);
	SDL_BlitSurface (GraphicsData.gfx_tmp.get (), &src, surface, &tmp);
}

void cVehicle::render_smallClearing (unsigned long long animationTime, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const
{
	assert (isUnitClearing () && !data.isBig && job == NULL);

	// draw shadow
	SDL_Rect tmp = dest;
	if (drawShadow)
		blitWithPreScale (uiData->clear_small_shw_org, uiData->clear_small_shw, NULL, surface, &tmp, zoomFactor);

	// draw player color
	SDL_Rect src;
	src.y = 0;
	src.h = src.w = (int) (uiData->clear_small_org->h * zoomFactor);
	src.x = (animationTime % 4) * src.w;
	SDL_BlitSurface (owner->getColorSurface (), NULL, GraphicsData.gfx_tmp.get (), NULL);
	blitWithPreScale (uiData->clear_small_org, uiData->clear_small, &src, GraphicsData.gfx_tmp.get (), NULL, zoomFactor, 4);

	// draw vehicle
	src.x = 0;
	src.y = 0;
	tmp = dest;
	SDL_SetSurfaceAlphaMod (GraphicsData.gfx_tmp.get (), 254);
	SDL_BlitSurface (GraphicsData.gfx_tmp.get (), &src, surface, &tmp);
}

void cVehicle::render_shadow (const cStaticMap& map, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor) const
{
	if (map.isWater (getPosition()) && (data.isStealthOn & TERRAIN_SEA)) return;

	if (StartUp && cSettings::getInstance().isAlphaEffects()) SDL_SetSurfaceAlphaMod (uiData->shw[dir], StartUp / 5);
	else SDL_SetSurfaceAlphaMod (uiData->shw[dir], 50);
	SDL_Rect tmp = dest;

	// draw shadow
	if (FlightHigh > 0)
	{
		// TODO: implement
		//int high = ((int) ((int) (client.getGameGUI().getTileSize()) * (FlightHigh / 64.0f)));
		//tmp.x += high;
		//tmp.y += high;

		//blitWithPreScale (uiData->shw_org[dir], uiData->shw[dir], NULL, surface, &tmp, zoomFactor);
	}
	else if (data.animationMovement)
	{
		const Uint16 size = (int) (uiData->img_org[dir]->h * zoomFactor);
		SDL_Rect r = {Sint16 (WalkFrame * size), 0, size, size};
		blitWithPreScale (uiData->shw_org[dir], uiData->shw[dir], &r, surface, &tmp, zoomFactor);
	}
	else
		blitWithPreScale (uiData->shw_org[dir], uiData->shw[dir], NULL, surface, &tmp, zoomFactor);
}

void cVehicle::render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, int alpha) const
{
	// draw player color
	SDL_BlitSurface (owner->getColorSurface (), NULL, GraphicsData.gfx_tmp.get (), NULL);

	// read the size:
	SDL_Rect src;
	src.w = (int) (uiData->img_org[dir]->w * zoomFactor);
	src.h = (int) (uiData->img_org[dir]->h * zoomFactor);

	if (data.animationMovement)
	{
		SDL_Rect tmp;
		src.w = src.h = tmp.h = tmp.w = (int) (uiData->img_org[dir]->h * zoomFactor);
		tmp.x = WalkFrame * tmp.w;
		tmp.y = 0;
		blitWithPreScale (uiData->img_org[dir], uiData->img[dir], &tmp, GraphicsData.gfx_tmp.get (), NULL, zoomFactor);
	}
	else
		blitWithPreScale (uiData->img_org[dir], uiData->img[dir], NULL, GraphicsData.gfx_tmp.get (), NULL, zoomFactor);

	// draw the vehicle
	src.x = 0;
	src.y = 0;
	SDL_Rect tmp = dest;

	SDL_SetSurfaceAlphaMod (GraphicsData.gfx_tmp.get (), alpha);
	blittAlphaSurface (GraphicsData.gfx_tmp.get (), &src, surface, &tmp);
}

void cVehicle::render (const cMap* map, unsigned long long animationTime, const cPlayer* activePlayer, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const
{
	// Note: when changing something in this function,
	// make sure to update the caching rules!

	// draw working engineers and bulldozers:
	if (map && job == NULL)
	{
		if (isUnitBuildingABuilding () || (isUnitClearing () && data.isBig))
		{
			render_BuildingOrBigClearing (*map, animationTime, surface, dest, zoomFactor, drawShadow);
			return;
		}
		if (isUnitClearing () && !data.isBig)
		{
			render_smallClearing (animationTime, surface, dest, zoomFactor, drawShadow);
			return;
		}
	}

	// draw all other vehicles:

	// draw shadow
	if (drawShadow && map)
	{
		render_shadow (*map->staticMap, surface, dest, zoomFactor);
	}

	int alpha = 254;
	if (map)
	{
		if (StartUp && cSettings::getInstance().isAlphaEffects())
		{
			alpha = StartUp;
		}
		else
		{
			bool water = map->isWater (getPosition());
			// if the vehicle can also drive on land, we have to check,
			// whether there is a brige, platform, etc.
			// because the vehicle will drive on the bridge
			cBuilding* building = map->getField(getPosition()).getBaseBuilding ();
			if (building && data.factorGround > 0 && (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)) water = false;

			if (water && (data.isStealthOn & TERRAIN_SEA) && detectedByPlayerList.empty() && owner == activePlayer) alpha = 100;
		}
	}
	render_simple (surface, dest, zoomFactor, alpha);
}

bool cVehicle::refreshData_Build (cServer& server)
{
	if (isUnitBuildingABuilding () == false || getBuildTurns () == 0) return false;

	data.storageResCur -= (getBuildCosts () / getBuildTurns ());
	setBuildCosts(getBuildCosts() - (getBuildCosts () / getBuildTurns ()));

	data.storageResCur = std::max (this->data.storageResCur, 0);

	setBuildTurns(getBuildTurns()-1);
	if (getBuildTurns () != 0) return true;

	const cMap& map = *server.Map;
	server.addReport (getBuildingType (), owner->getNr ());

	// handle pathbuilding
	// here the new building is added (if possible) and
	// the move job to the next field is generated
	// the new build event is generated in cServer::handleMoveJobs()
	if (BuildPath)
	{
		// Find a next position that either
		// a) is something we can't move to
		//  (in which case we cancel the path building)
		// or b) doesn't have a building type that we're trying to build.
		cPosition nextPosition(getPosition());
		bool found_next  = false;

		while (!found_next && (nextPosition != bandPosition))
		{
			// Calculate the next position in the path.
			if (getPosition().x() > bandPosition.x()) nextPosition.x()--;
			if (getPosition().x() < bandPosition.x()) nextPosition.x()++;
			if (getPosition().y() > bandPosition.y()) nextPosition.y()--;
			if (getPosition().y() < bandPosition.y()) nextPosition.y()++;
			// Can we move to this position?
			// If not, we need to kill the path building now.
			if (!map.possiblePlace (*this, nextPosition))
			{
				// Try sidestepping stealth units before giving up.
				server.sideStepStealthUnit (nextPosition, *this);
				if (!map.possiblePlace (*this, nextPosition))
				{
					// We can't build along this path any more.
					break;
				}
			}
			// Can we build at this next position?
			if (map.possiblePlaceBuilding (*getBuildingType ().getUnitDataOriginalVersion (), nextPosition))
			{
				// We can build here.
				found_next = true;
				break;
			}
		}

		// If we've found somewhere to move to, move there now.
		if (found_next && server.addMoveJob (getPosition(), nextPosition, this))
		{
			setBuildingABuilding (false);
			server.addBuilding (getPosition(), getBuildingType (), owner);
			// Begin the movement immediately,
			// so no other unit can block the destination field.
			this->ServerMoveJob->checkMove();
		}

		else
		{
			if (getBuildingType ().getUnitDataOriginalVersion ()->surfacePosition != sUnitData::SURFACE_POS_GROUND)
			{
				server.addBuilding (getPosition(), getBuildingType (), owner);
				setBuildingABuilding (false);
			}
			BuildPath = false;
			sendBuildAnswer (server, false, *this);
		}
	}
	else
	{
		// add building immediately
		// if it doesn't require the engineer to drive away
		if (getBuildingType ().getUnitDataOriginalVersion ()->surfacePosition != data.surfacePosition)
		{
			setBuildingABuilding(false);
			server.addBuilding (getPosition(), getBuildingType (), owner);
		}
	}
	return true;
}

bool cVehicle::refreshData_Clear (cServer& server)
{
	if (isUnitClearing () == false || getClearingTurns () == 0) return false;

	setClearingTurns(getClearingTurns() - 1);

	cMap& map = *server.Map;

	if (getClearingTurns () != 0) return true;

	setClearing(false);
	cBuilding* Rubble = map.getField(getPosition()).getRubble();
	if (data.isBig)
	{
		map.moveVehicle (*this, buildBigSavedPosition);
		sendStopClear (server, *this, buildBigSavedPosition, *owner);
		for (size_t i = 0; i != seenByPlayerList.size(); ++i)
		{
			sendStopClear (server, *this, buildBigSavedPosition, *seenByPlayerList[i]);
		}
	}
	else
	{
		sendStopClear (server, *this, cPosition(-1, -1), *owner);
		for (size_t i = 0; i != seenByPlayerList.size(); ++i)
		{
			sendStopClear (server, *this, cPosition (-1, -1), *seenByPlayerList[i]);
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
	if (isDisabled())
	{
		lastSpeed = data.speedMax;
		lastShots = std::min (data.getAmmo (), data.shotsMax);
		return true;
	}
	if (data.speedCur < data.speedMax || data.getShots () < data.shotsMax)
	{
		data.speedCur = data.speedMax;
		data.setShots (std::min (data.getAmmo (), data.shotsMax));

#if 0
		// Regeneration:
		if (data.is_alien && data.hitpointsCur < data.hitpointsMax)
		{
			data.hitpointsCur++;
		}
#endif
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
/** Returns a string with the current state */
//-----------------------------------------------------------------------------
string cVehicle::getStatusStr (const cPlayer* player) const
{
	if (isDisabled())
	{
		string sText;
		sText = lngPack.i18n ("Text~Comp~Disabled") + " (";
		sText += iToStr (getDisabledTurns()) + ")";
		return sText;
	}
	else if (autoMJob)
		return lngPack.i18n ("Text~Comp~Surveying");
	else if (isUnitBuildingABuilding ())
	{
		if (owner != player)
			return lngPack.i18n ("Text~Comp~Producing");
		else
		{
			string sText;
			if (getBuildTurns ())
			{
				sText = lngPack.i18n ("Text~Comp~Producing");
				sText += ": ";
				sText += (string)owner->getUnitDataCurrentVersion (getBuildingType ())->name + " (";
				sText += iToStr (getBuildTurns ());
				sText += ")";

				if (font->getTextWide (sText) > 126)
				{
					sText = lngPack.i18n ("Text~Comp~Producing");
					sText += ":\n";
					sText += (string)owner->getUnitDataCurrentVersion (getBuildingType ())->name + " (";
					sText += iToStr (getBuildTurns ());
					sText += ")";
				}
				return sText;
			}
			else //small building is rdy + activate after engineere moves away
			{
				sText = lngPack.i18n ("Text~Comp~Producing_Fin");
				sText += ": ";
				sText += (string)owner->getUnitDataCurrentVersion (getBuildingType ())->name;

				if (font->getTextWide (sText) > 126)
				{
					sText = lngPack.i18n ("Text~Comp~Producing_Fin");
					sText += ":\n";
					sText += (string)owner->getUnitDataCurrentVersion (getBuildingType ())->name;
				}
				return sText;
			}
		}
	}
	else if (isUnitClearingMines ())
		return lngPack.i18n ("Text~Comp~Clearing_Mine");
	else if (isUnitLayingMines ())
		return lngPack.i18n ("Text~Comp~Laying");
	else if (isUnitClearing ())
	{
		if (getClearingTurns ())
		{
			string sText;
			sText = lngPack.i18n ("Text~Comp~Clearing") + " (";
			sText += iToStr (getClearingTurns ()) + ")";
			return sText;
		}
		else
			return lngPack.i18n ("Text~Comp~Clearing_Fin");
	}

	// generate other infos for normall non-unit-related-events and infiltartors
	string sTmp;
	{
		if (ClientMoveJob && ClientMoveJob->endMoveAction && ClientMoveJob->endMoveAction->type_ == EMAT_ATTACK)
			sTmp = lngPack.i18n ("Text~Comp~MovingToAttack");
		else if (ClientMoveJob)
			sTmp = lngPack.i18n ("Text~Comp~Moving");
		else if (isAttacking())
			sTmp = lngPack.i18n ("Text~Comp~AttackingStatusStr");
		else if (isBeeingAttacked ())
			sTmp = lngPack.i18n ("Text~Comp~IsBeeingAttacked");
		else if (isManualFireActive())
			sTmp = lngPack.i18n ("Text~Comp~ReactionFireOff");
		else if (isSentryActive())
			sTmp = lngPack.i18n ("Text~Comp~Sentry");
		else sTmp = lngPack.i18n ("Text~Comp~Waits");

		// extra info only for infiltrators
			// TODO should it be original behavior (as it is now) or
			// don't display CommandRank for enemy (could also be a bug in original...?)
		if ((data.canCapture || data.canDisable) /* && owner == gameGUI.getClient()->getActivePlayer()*/ )
		{
			sTmp += "\n";
			if (getCommandoRank() < 1.f) sTmp += lngPack.i18n ("Text~Comp~CommandoRank_Greenhorn");
			else if (getCommandoRank () < 3.f) sTmp += lngPack.i18n ("Text~Comp~CommandoRank_Average");
			else if (getCommandoRank () < 6.f) sTmp += lngPack.i18n ("Text~Comp~CommandoRank_Veteran");
			else if (getCommandoRank () < 11.f) sTmp += lngPack.i18n ("Text~Comp~CommandoRank_Expert");
			else if (getCommandoRank () < 19.f) sTmp += lngPack.i18n ("Text~Comp~CommandoRank_Elite");
			else sTmp += lngPack.i18n ("Text~Comp~CommandoRank_GrandMaster");
			if (getCommandoRank () > 0.f)
				sTmp += " +" + iToStr ((int)getCommandoRank ());

		}

		return sTmp;
	}

	return lngPack.i18n ("Text~Comp~Waits");
}

//-----------------------------------------------------------------------------
/** Reduces the remaining speedCur and shotsCur during movement */
//-----------------------------------------------------------------------------
void cVehicle::DecSpeed (int value)
{
	data.speedCur -= value;

	if (data.canAttack == false || data.canDriveAndFire) return;

	const int s = data.speedCur * data.shotsMax / data.speedMax;
	data.setShots(std::min (data.getShots (), s));
}

//-----------------------------------------------------------------------------
void cVehicle::calcTurboBuild (std::array<int, 3>& turboBuildTurns, std::array<int, 3>& turboBuildCosts, int buildCosts) const
{
	turboBuildTurns[0] = 0;
	turboBuildTurns[1] = 0;
	turboBuildTurns[2] = 0;

	// step 1x
	if (data.storageResCur >= buildCosts)
	{
		turboBuildCosts[0] = buildCosts;
		// prevent division by zero
		const auto needsMetal = data.needsMetal == 0 ? 1 : data.needsMetal;
		turboBuildTurns[0] = (int)ceilf (turboBuildCosts[0] / (float)(needsMetal));
	}

	// step 2x
	// calculate building time and costs
	int a = turboBuildCosts[0];
	int rounds = turboBuildTurns[0];
	int costs = turboBuildCosts[0];

	while (a >= 4 && data.storageResCur >= costs + 4)
	{
		rounds--;
		costs += 4;
		a -= 4;
	}

	if (rounds < turboBuildTurns[0] && rounds > 0 && turboBuildTurns[0])
	{
		turboBuildCosts[1] = costs;
		turboBuildTurns[1] = rounds;
	}

	// step 4x
	a = turboBuildCosts[1];
	rounds = turboBuildTurns[1];
	costs = turboBuildCosts[1];

	while (a >= 10 && costs < data.storageResMax - 2)
	{
		int inc = 24 - min (16, a);
		if (costs + inc > data.storageResCur) break;

		rounds--;
		costs += inc;
		a -= 16;
	}

	if (rounds < turboBuildTurns[1] && rounds > 0 && turboBuildTurns[1])
	{
		turboBuildCosts[2] = costs;
		turboBuildTurns[2] = rounds;
	}
}

//-----------------------------------------------------------------------------
/** Scans for resources */
//-----------------------------------------------------------------------------
void cVehicle::doSurvey (const cServer& server)
{
	const cMap& map = *server.Map;
	const int minx = std::max (getPosition().x() - 1, 0);
	const int maxx = std::min (getPosition().x() + 1, map.getSize().x() - 1);
	const int miny = std::max (getPosition().y() - 1, 0);
	const int maxy = std::min (getPosition().y() + 1, map.getSize().y() - 1);

	for (int y = miny; y <= maxy; ++y)
	{
		for (int x = minx; x <= maxx; ++x)
		{
			const cPosition position (x, y);
			owner->exploreResource (position);
		}
	}
}

//-----------------------------------------------------------------------------
/** Makes the report */
//-----------------------------------------------------------------------------
void cVehicle::makeReport ()
{
	if (isDisabled())
	{
		// Disabled:
		PlayRandomVoice (VoiceData.VOIUnitDisabledByEnemy);
	}
	else if (data.getHitpoints () > data.hitpointsMax / 2)
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
			PlayVoice (VoiceData.VOINoSpeed.get ());
		}
		else if (isUnitBuildingABuilding ())
		{
			// Beim bau:
			if (!getBuildTurns ())
			{
				// Bau beendet:
				PlayRandomVoice (VoiceData.VOIBuildDone);
			}
		}
		else if (isUnitClearing ())
		{
			// removing dirt
			PlayVoice (VoiceData.VOIClearing.get ());
		}
		else if (data.canAttack && data.getAmmo () <= data.ammoMax / 4 && data.getAmmo () != 0)
		{
			// red ammo-status but still ammo left
			PlayRandomVoice (VoiceData.VOIAmmoLow);
		}
		else if (data.canAttack && data.getAmmo () == 0)
		{
			// no ammo left
			PlayRandomVoice (VoiceData.VOIAmmoEmpty);
		}
		else if (isSentryActive())
		{
			// on sentry:
			PlayVoice (VoiceData.VOISentry.get ());
		}
		else if (isUnitClearingMines ())
		{
			PlayRandomVoice (VoiceData.VOIClearingMines);
		}
		else if (isUnitLayingMines ())
		{
			PlayVoice (VoiceData.VOILayingMines.get ());
		}
		else
		{
			PlayRandomVoice (VoiceData.VOIOK);
		}
	}
	else if (data.getHitpoints () > data.hitpointsMax / 4)
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
/** checks, if resources can be transferred to the unit */
//-----------------------------------------------------------------------------
bool cVehicle::canTransferTo (const cPosition& position, const cMapField& overUnitField) const
{
	if (isNextTo (position) == false)
		return false;

	if (overUnitField.getVehicle ())
	{
		const cVehicle* v = overUnitField.getVehicle ();

		if (v == this)
			return false;

		if (v->owner != this->owner)
			return false;

		if (v->data.storeResType != data.storeResType)
			return false;

		if (v->isUnitBuildingABuilding () || v->isUnitClearing ())
			return false;

		return true;
	}
	else if (overUnitField.getTopBuilding ())
	{
		const cBuilding* b = overUnitField.getTopBuilding ();

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
	const cUnit* target = selectTarget (getPosition(), opponentUnit->data.canAttack, *server.Map);
	if (target != this) return false;

	Log.write (" Server: " + reasonForLog + ": attacking position " + iToStr (getPosition().x()) + "x" + iToStr (getPosition().y()) + " Aggressor ID: " + iToStr (opponentUnit->iID), cLog::eLOG_TYPE_NET_DEBUG);
	server.AJobs.push_back (new cServerAttackJob (server, opponentUnit, getPosition(), true));
	if (ServerMoveJob != 0)
		ServerMoveJob->bFinished = true;
	return true;
}

//-----------------------------------------------------------------------------
bool cVehicle::makeSentryAttack (cServer& server, cUnit* sentryUnit) const
{
	if (sentryUnit != 0 && sentryUnit->isSentryActive() && sentryUnit->canAttackObjectAt (getPosition(), *server.Map, true))
	{
		if (makeAttackOnThis (server, sentryUnit, "sentry reaction"))
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::InSentryRange (cServer& server)
{
	const auto& playerList = server.playerList;
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		cPlayer& player = *playerList[i];

		if (&player == owner) continue;

		// Don't attack undiscovered stealth units
		if (data.isStealthOn != TERRAIN_NONE && !isDetectedByPlayer (&player)) continue;
		// Don't attack units out of scan range
		if (!player.canSeeAnyAreaUnder (*this)) continue;
		// Check sentry type
		if (data.factorAir > 0 && player.hasSentriesAir (getPosition ()) == 0) continue;
		// Check sentry type
		if (data.factorAir == 0 && player.hasSentriesGround (getPosition ()) == 0) continue;

		for (cVehicle* vehicle = player.VehicleList; vehicle; vehicle = vehicle->next)
		{
			if (makeSentryAttack (server, vehicle))
				return true;
		}
		for (cBuilding* building = player.BuildingList; building; building = building->next)
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
	// don't treat the cheap buildings
	// (connectors, roads, beton blocks) as offendable
	bool otherUnitIsCheapBuilding = (otherUnit.isABuilding() && otherUnit.data.ID.getUnitDataOriginalVersion()->buildCosts > 2); //FIXME: isn't the check inverted?

	if (otherUnitIsCheapBuilding == false
		&& isInRange (otherUnit.getPosition())
		&& canAttackObjectAt (otherUnit.getPosition(), *server.Map, true, false))
	{
		// test, if this vehicle can really attack the opponentVehicle
		cUnit* target = selectTarget (otherUnit.getPosition(), data.canAttack, *server.Map);
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
		// In the turn based game style,
		// the opponent always fires on the unit if he can,
		// regardless if the unit is offending or not.
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
	if (opponentUnit->isSentryActive() == false && opponentUnit->isManualFireActive() == false
		&& opponentUnit->canAttackObjectAt (getPosition(), *server.Map, true)
		// Possible TODO: better handling of stealth units.
		// e.g. do reaction fire, if already detected ?
		&& (opponentUnit->isAVehicle() == false || opponentUnit->data.isStealthOn == TERRAIN_NONE))
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
	if (data.canAttack == false || data.getShots () <= 0 || data.getAmmo () <= 0)
		return false;

	const auto& playerList = server.playerList;
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		cPlayer& player = *playerList[i];
		if (&player == owner)
			continue;

		if (data.isStealthOn != TERRAIN_NONE && !isDetectedByPlayer (&player))
			continue;
		// The vehicle can't be seen by the opposing player.
		// No possibility for reaction fire.
		if (player.canSeeAnyAreaUnder (*this) == false)
			continue;

		if (doesPlayerWantToFireOnThisVehicleAsReactionFire (server, &player) == false)
			continue;

		if (doReactionFire (server, &player))
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool cVehicle::canExitTo (const cPosition& position, const cMap& map, const sUnitData& unitData) const
{
	if (!map.possiblePlaceVehicle (unitData, position, owner)) return false;
	if (data.factorAir > 0 && (position != getPosition())) return false;
	if (!isNextTo (position)) return false;

	return true;
}

//-----------------------------------------------------------------------------
bool cVehicle::canLoad (const cPosition& position, const cMap& map, bool checkPosition) const
{
	if (map.isValidPosition (position) == false) return false;

	return canLoad (map.getField (position).getVehicle (), checkPosition);
}

//-----------------------------------------------------------------------------
bool cVehicle::canLoad (const cVehicle* Vehicle, bool checkPosition) const
{
	if (!Vehicle) return false;

	if (Vehicle->isUnitLoaded ()) return false;

	if (data.storageUnitsCur >= data.storageUnitsMax) return false;

	if (checkPosition && !isNextTo (Vehicle->getPosition())) return false;

	if (checkPosition && data.factorAir > 0 && (Vehicle->getPosition() != getPosition())) return false;

	if (!Contains (data.storeUnitsTypes, Vehicle->data.isStorageType)) return false;

	if (Vehicle->ClientMoveJob && (Vehicle->moving || Vehicle->isAttacking() || Vehicle->MoveJobActive)) return false;

	if (Vehicle->owner != owner || Vehicle->isUnitBuildingABuilding () || Vehicle->isUnitClearing ()) return false;

	if (Vehicle->isBeeingAttacked ()) return false;

	return true;
}

//-----------------------------------------------------------------------------
void cVehicle::storeVehicle (cVehicle& vehicle, cMap& map)
{
	map.deleteVehicle (vehicle);
	if (vehicle.isSentryActive())
	{
		vehicle.owner->deleteSentry (vehicle);
	}

	vehicle.setManualFireActive(false);

	vehicle.setLoaded (true);

	storedUnits.push_back (&vehicle);
	data.storageUnitsCur++;

	owner->doScan();
}

//-----------------------------------------------------------------------------
/** Exits a vehicle */
//-----------------------------------------------------------------------------
void cVehicle::exitVehicleTo (cVehicle& vehicle, const cPosition& position, cMap& map)
{
	Remove (storedUnits, &vehicle);

	data.storageUnitsCur--;

	map.addVehicle (vehicle, position);

	vehicle.setPosition(position);

	vehicle.setLoaded (false);
	//vehicle.data.shotsCur = 0;

	owner->doScan();
}

//-----------------------------------------------------------------------------
/** Checks, if an object can get ammunition. */
//-----------------------------------------------------------------------------
bool cVehicle::canSupply (const cMap& map, const cPosition& position, int supplyType) const
{
	if (map.isValidPosition (position) == false) return false;

	const cMapField& field = map.getField (position);
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

	if (unit->isNextTo (getPosition()) == false)
		return false;

	if (unit->isAVehicle() && unit->data.factorAir > 0 && static_cast<const cVehicle*> (unit)->FlightHigh > 0)
		return false;

	switch (supplyType)
	{
		case SUPPLY_TYPE_REARM:
			if (unit == this || unit->data.canAttack == false || unit->data.getAmmo () >= unit->data.ammoMax
				|| (unit->isAVehicle() && static_cast<const cVehicle*> (unit)->isUnitMoving())
				|| unit->isAttacking())
				return false;
			break;
		case SUPPLY_TYPE_REPAIR:
			if (unit == this || unit->data.getHitpoints () >= unit->data.hitpointsMax
				|| (unit->isAVehicle() && static_cast<const cVehicle*> (unit)->isUnitMoving())
				|| unit->isAttacking())
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
	if (data.factorSea > 0 && data.factorGround == 0)
	{
		if (!map.possiblePlaceBuilding (*UnitsData.specialIDSeaMine.getUnitDataOriginalVersion(), getPosition(), this)) return false;
		server.addBuilding (getPosition(), UnitsData.specialIDSeaMine, owner, false);
	}
	else
	{
		if (!map.possiblePlaceBuilding (*UnitsData.specialIDLandMine.getUnitDataOriginalVersion(), getPosition(), this)) return false;
		server.addBuilding (getPosition(), UnitsData.specialIDLandMine, owner, false);
	}
	data.storageResCur--;

	if (data.storageResCur <= 0) setLayMines(false);

	return true;
}

//-----------------------------------------------------------------------------
bool cVehicle::clearMine (cServer& server)
{
	const cMap& map = *server.Map;
	cBuilding* Mine = map.getField(getPosition()).getMine();

	if (!Mine || Mine->owner != owner || data.storageResCur >= data.storageResMax) return false;
	if (Mine->data.factorGround > 0 && data.factorGround == 0) return false;
	if (Mine->data.factorSea > 0 && data.factorSea == 0) return false;

	server.deleteUnit (Mine);
	data.storageResCur++;

	if (data.storageResCur >= data.storageResMax) setClearMines(false);

	return true;
}

//-----------------------------------------------------------------------------
/** Checks if the target is on a neighbor field and if it can be stolen or disabled */
//-----------------------------------------------------------------------------
bool cVehicle::canDoCommandoAction (const cPosition& position, const cMap& map, bool steal) const
{
	const auto& field = map.getField (position);

	const cUnit* unit = field.getPlane ();
	if (canDoCommandoAction (unit, steal)) return true;

	unit = field.getVehicle ();
	if (canDoCommandoAction (unit, steal)) return true;

	unit = field.getBuilding ();
	if (canDoCommandoAction (unit, steal)) return true;

	return false;
}

bool cVehicle::canDoCommandoAction (const cUnit* unit, bool steal) const
{
	if (unit == NULL) return false;

	if ((steal && data.canCapture == false) || (steal == false && data.canDisable == false))
		return false;
	if (data.getShots() == 0) return false;

	if (unit->isNextTo (getPosition()) == false)
		return false;

	if (steal == false && unit->isDisabled ()) return false;
	if (unit->isABuilding () && unit->owner == 0) return false;   // rubble
	if (steal && unit->data.canBeCaptured == false) return false;
	if (steal == false && unit->data.canBeDisabled == false) return false;
	if (steal && unit->storedUnits.empty () == false) return false;
	if (unit->owner == owner) return false;
	if (unit->isAVehicle () && unit->data.factorAir > 0 && static_cast<const cVehicle*> (unit)->FlightHigh > 0) return false;

	return true;
}

//-----------------------------------------------------------------------------
int cVehicle::calcCommandoChance (const cUnit* destUnit, bool steal) const
{
	if (destUnit == 0)
		return 0;

	// TODO: include cost research and clan modifications ?
	// Or should always the basic version without clanmods be used ?
	// TODO: Bug for buildings ? /3? or correctly /2,
	// because constructing buildings takes two resources per turn ?
	int destTurn = destUnit->data.buildCosts / 3;

	int factor = steal ? 1 : 4;
	int srcLevel = (int) getCommandoRank() + 7;

	// The chance to disable or steal a unit depends on
	// the infiltrator ranking and the buildcosts
	// (or 'turns' in the original game) of the target unit.
	// The chance rises linearly with a higher ranking of the infiltrator.
	// The chance of a unexperienced infiltrator will be calculated like
	// he has the ranking 7.
	// Disabling has a 4 times higher chance than stealing.
	int chance = Round ((8.f * srcLevel) / (35 * destTurn) * factor * 100);
	chance = std::min (90, chance);

	return chance;
}

//-----------------------------------------------------------------------------
int cVehicle::calcCommandoTurns (const cUnit* destUnit) const
{
	if (destUnit == 0)
		return 1;

	const int vehiclesTable[13] = { 0, 0, 0, 5, 8, 3, 3, 0, 0, 0, 1, 0, -4 };
	int destTurn, srcLevel;

	if (destUnit->isAVehicle())
	{
		destTurn = destUnit->data.buildCosts / 3;
		srcLevel = (int) getCommandoRank();
		if (destTurn > 0 && destTurn < 13)
			srcLevel += vehiclesTable[destTurn];
	}
	else
	{
		destTurn = destUnit->data.buildCosts / 2;
		srcLevel = (int) getCommandoRank() + 8;
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
	bool wasDetected = (detectedByPlayerList.empty() == false);

	if (!isDetectedByPlayer (player))
		detectedByPlayerList.push_back (player);

	if (!wasDetected) sendDetectionState (server, *this);

	if (addToDetectedInThisTurnList && Contains (detectedInThisTurnByPlayerList, player) == false)
		detectedInThisTurnByPlayerList.push_back (player);
}

//-----------------------------------------------------------------------------
void cVehicle::resetDetectedByPlayer (cServer& server, cPlayer* player)
{
	bool wasDetected = (detectedByPlayerList.empty() == false);

	Remove (detectedByPlayerList, player);
	Remove (detectedInThisTurnByPlayerList, player);

	if (wasDetected && detectedByPlayerList.empty()) sendDetectionState (server, *this);
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
	// check whether the vehicle has been detected by others
	if (data.isStealthOn != TERRAIN_NONE)  // the vehicle is a stealth vehicle
	{
		cMap& map = *server.Map;
		const auto& playerList = server.playerList;
		for (unsigned int i = 0; i < playerList.size(); i++)
		{
			cPlayer& player = *playerList[i];
			if (&player == owner)
				continue;
			bool isOnWater = map.isWater (getPosition());
			bool isOnCoast = map.isCoast (getPosition()) && (isOnWater == false);

			// if the vehicle can also drive on land, we have to check,
			// whether there is a brige, platform, etc.
			// because the vehicle will drive on the bridge
			const cBuilding* building = map.getField(getPosition()).getBaseBuilding();
			if (data.factorGround > 0 && building
				&& (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE
					|| building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA
					|| building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE))
			{
				isOnWater = false;
				isOnCoast = false;
			}

			if ((data.isStealthOn & TERRAIN_GROUND)
				&& (player.hasLandDetection (getPosition()) || (! (data.isStealthOn & TERRAIN_COAST) && isOnCoast)
					|| isOnWater))
			{
				playersThatDetectThisVehicle.push_back (&player);
			}

			if ((data.isStealthOn & TERRAIN_SEA)
				&& (player.hasSeaDetection (getPosition()) || isOnWater == false))
			{
				playersThatDetectThisVehicle.push_back (&player);
			}
		}
	}
	return playersThatDetectThisVehicle;
}

//-----------------------------------------------------------------------------
void cVehicle::makeDetection (cServer& server)
{
	// check whether the vehicle has been detected by others
	std::vector<cPlayer*> playersThatDetectThisVehicle = calcDetectedByPlayer (server);
	for (unsigned int i = 0; i < playersThatDetectThisVehicle.size(); i++)
		setDetectedByPlayer (server, playersThatDetectThisVehicle[i]);

	// detect other units
	if (data.canDetectStealthOn == false) return;

	cMap& map = *server.Map;
	const int minx = std::max (getPosition().x() - data.scan, 0);
	const int maxx = std::min (getPosition().x() + data.scan, map.getSize().x() - 1);
	const int miny = std::max (getPosition().y() - data.scan, 0);
	const int maxy = std::min (getPosition().y() + data.scan, map.getSize().x() - 1);

	for (int x = minx; x <= maxx; ++x)
	{
		for (int y = miny; y <= maxy; ++y)
		{
			const cPosition position (x, y);

			cVehicle* vehicle = map.getField (position).getVehicle ();
			cBuilding* building = map.getField (position).getMine ();

			if (vehicle && vehicle->owner != owner)
			{
				if ((data.canDetectStealthOn & TERRAIN_GROUND) && owner->hasLandDetection (position) && (vehicle->data.isStealthOn & TERRAIN_GROUND))
				{
					vehicle->setDetectedByPlayer (server, owner);
				}
				if ((data.canDetectStealthOn & TERRAIN_SEA) && owner->hasSeaDetection (position) && (vehicle->data.isStealthOn & TERRAIN_SEA))
				{
					vehicle->setDetectedByPlayer (server, owner);
				}
			}
			if (building && building->owner != owner)
			{
				if ((data.canDetectStealthOn & AREA_EXP_MINE) && owner->hasMineDetection (position) && (building->data.isStealthOn & AREA_EXP_MINE))
				{
					building->setDetectedByPlayer (server, owner);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
sVehicleUIData::sVehicleUIData() :
	build (NULL), build_org (NULL),
	build_shw (NULL), build_shw_org (NULL),
	clear_small (NULL), clear_small_org (NULL),
	clear_small_shw (NULL), clear_small_shw_org (NULL),
	overlay (NULL), overlay_org (NULL),
	storage (NULL),
	FLCFile (NULL),
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
void sVehicleUIData::scaleSurfaces (float factor)
{
	int width, height;
	for (int i = 0; i < 8; ++i)
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
void cVehicle::blitWithPreScale (SDL_Surface* org_src, SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dest, SDL_Rect* destrect, float factor, int frames) const
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
	if (!isUnitLoaded ()) return NULL;

	for (cBuilding* building = owner->BuildingList; building; building = building->next)
		if (Contains (building->storedUnits, this)) return building;

	return NULL;
}

cVehicle* cVehicle::getContainerVehicle()
{
	if (!isUnitLoaded()) return NULL;

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
	return (ClientMoveJob != 0 || (isUnitBuildingABuilding () && getBuildTurns () > 0) || (isUnitClearing () && getClearingTurns () > 0));
}

//-----------------------------------------------------------------------------
void cVehicle::executeStopCommand (const cClient& client) const
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
void cVehicle::executeLayMinesCommand (const cClient& client)
{
	setLayMines (!isUnitLayingMines ());
	setClearMines(false);
	sendMineLayerStatus (client, *this);
}

//-----------------------------------------------------------------------------
void cVehicle::executeClearMinesCommand (const cClient& client)
{
	setClearing (!isUnitClearingMines ());
	setLayMines(false);
	sendMineLayerStatus (client, *this);
}

//-----------------------------------------------------------------------------
bool cVehicle::canLand (const cMap& map) const
{
	// normal vehicles are always "landed"
	if (data.factorAir == 0) return true;

	if (moving || ClientMoveJob || (ServerMoveJob && ServerMoveJob->Waypoints && ServerMoveJob->Waypoints->next) || isAttacking()) return false;     //vehicle busy?

	// landing pad there?
	const std::vector<cBuilding*>& buildings = map.getField(getPosition()).getBuildings();
	std::vector<cBuilding*>::const_iterator b_it = buildings.begin();
	for (; b_it != buildings.end(); ++b_it)
	{
		if ((*b_it)->data.canBeLandedOn)
			break;
	}
	if (b_it == buildings.end()) return false;

	// is the landing pad already occupied?
	const std::vector<cVehicle*>& v = map.getField(getPosition()).getPlanes();
	for (std::vector<cVehicle*>::const_iterator it = v.begin(); it != v.end(); ++it)
	{
		const cVehicle& vehicle = **it;
		if (vehicle.FlightHigh < 64 && vehicle.iID != iID)
			return false;
	}

	// returning true before checking owner, because a stolen vehicle
	// can stay on an enemy landing pad until it is moved
	if (FlightHigh == 0) return true;

	if ((*b_it)->owner != owner) return false;

	return true;
}

//-----------------------------------------------------------------------------
void cVehicle::setLoaded (bool value)
{
	std::swap (loaded, value);
	if (value != loaded)
	{
		if (loaded) stored ();
		else activated ();
	}
}

//-----------------------------------------------------------------------------
void cVehicle::setClearing (bool value)
{
	std::swap (isClearing, value);
	if (value != isClearing) clearingChanged ();
}

//-----------------------------------------------------------------------------
void cVehicle::setBuildingABuilding (bool value)
{
	std::swap (isBuilding, value);
	if (value != isBuilding) buildingChanged ();
}

//-----------------------------------------------------------------------------
void cVehicle::setLayMines (bool value)
{
	std::swap (layMines, value);
	if (value != layMines) layingMinesChanged ();
}

//-----------------------------------------------------------------------------
void cVehicle::setClearMines (bool value)
{
	std::swap (clearMines, value);
	if (value != clearMines) clearingMinesChanged ();
}

//-----------------------------------------------------------------------------
int cVehicle::getClearingTurns () const
{
	return clearingTurns;
}

//-----------------------------------------------------------------------------
void cVehicle::setClearingTurns (int value)
{
	std::swap (clearingTurns, value);
	if (value != clearingTurns) clearingTurnsChanged ();
}

//-----------------------------------------------------------------------------
float cVehicle::getCommandoRank () const
{
	return commandoRank;
}

//-----------------------------------------------------------------------------
void cVehicle::setCommandoRank (float value)
{
	std::swap (commandoRank, value);
	if (value != commandoRank) commandoRankChanged ();
}

//-----------------------------------------------------------------------------
const sID& cVehicle::getBuildingType () const
{
	return buildingTyp;
}

//-----------------------------------------------------------------------------
void cVehicle::setBuildingType (const sID& id)
{
	auto oldId = id;
	buildingTyp = id;
	if (buildingTyp != oldId) buildingTypeChanged ();
}

//-----------------------------------------------------------------------------
int cVehicle::getBuildCosts () const
{
	return buildCosts;
}

//-----------------------------------------------------------------------------
void cVehicle::setBuildCosts (int value)
{
	std::swap (buildCosts, value);
	if (value != buildCosts) buildingCostsChanged ();
}

//-----------------------------------------------------------------------------
int cVehicle::getBuildTurns () const
{
	return buildTurns;
}

//-----------------------------------------------------------------------------
void cVehicle::setBuildTurns (int value)
{
	std::swap (buildTurns, value);
	if (value != buildTurns) buildingTurnsChanged ();
}

//-----------------------------------------------------------------------------
int cVehicle::getBuildCostsStart () const
{
	return buildCostsStart;
}

//-----------------------------------------------------------------------------
void cVehicle::setBuildCostsStart (int value)
{
	std::swap (buildCostsStart, value);
	//if (value != buildCostsStart) event ();
}

//-----------------------------------------------------------------------------
int cVehicle::getBuildTurnsStart () const
{
	return buildTurnsStart;
}

//-----------------------------------------------------------------------------
void cVehicle::setBuildTurnsStart (int value)
{
	std::swap (buildTurnsStart, value);
	//if (value != buildTurnsStart) event ();
}
