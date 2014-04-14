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

#include "buildings.h"

#include "attackJobs.h"
#include "client.h"
#include "clientevents.h"
#include "clist.h"
#include "fxeffects.h"
#include "hud.h"
#include "main.h"
#include "netmessage.h"
#include "pcx.h"
#include "player.h"
#include "server.h"
#include "serverevents.h"
#include "settings.h"
#include "upgradecalculator.h"
#include "vehicles.h"
#include "video.h"
#include "unifonts.h"

using namespace std;

//--------------------------------------------------------------------------
// cBuilding Implementation
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
cBuilding::cBuilding (const sUnitData* b, cPlayer* Owner, unsigned int ID) :
	cUnit ((Owner != 0 && b != 0) ? Owner->getUnitDataCurrentVersion (b->ID) : 0,
		   Owner,
		   ID),
	next (0),
	prev (0),
	BuildList (0),
	isWorking (false)
{
	setSentryActive(data.canAttack != TERRAIN_NONE);

	RubbleTyp = 0;
	RubbleValue = 0;
	EffectAlpha = 0;
	EffectInc = true;
	StartUp = 0;
	researchArea = cResearch::kAttackResearch;
	uiData = b ? UnitsData.getBuildingUI (b->ID) : 0;
	points = 0;
	lastShots = 0;

	if (Owner == NULL || b == NULL)
	{
		return;
	}

	BaseN = false;
	BaseBN = false;
	BaseE = false;
	BaseBE = false;
	BaseS = false;
	BaseBS = false;
	BaseW = false;
	BaseBW = false;
	RepeatBuild = false;

	MaxMetalProd = 0;
	MaxGoldProd = 0;
	MaxOilProd = 0;
	data.setHitpoints(data.hitpointsMax);
	data.setAmmo(data.ammoMax);
	SubBase = NULL;
	BuildSpeed = 0;

	if (data.isBig)
	{
		DamageFXPointX  = random (64) + 32;
		DamageFXPointY  = random (64) + 32;
		DamageFXPointX2 = random (64) + 32;
		DamageFXPointY2 = random (64) + 32;
	}
	else
	{
		DamageFXPointX = random (64 - 24);
		DamageFXPointY = random (64 - 24);
	}

	refreshData();
}

//--------------------------------------------------------------------------
cBuilding::~cBuilding()
{
}

//----------------------------------------------------
/** Returns a string with the current state */
//----------------------------------------------------
string cBuilding::getStatusStr (const cPlayer* player) const
{
	if (isDisabled())
	{
		string sText;
		sText = lngPack.i18n ("Text~Comp~Disabled") + " (";
		sText += iToStr (getDisabledTurns()) + ")";
		return sText;
	}
	if (isUnitWorking () || (factoryHasJustFinishedBuilding () && isDisabled () == false))
	{
		// Factory:
		if (!data.canBuild.empty () && !BuildList.empty () && owner == player)
		{
			const sBuildList& buildListItem = BuildList[0];
			const string& unitName = buildListItem.type.getUnitDataOriginalVersion()->name;
			string sText;

			if (buildListItem.metall_remaining > 0)
			{
				int iRound;

				iRound = (int) ceilf (buildListItem.metall_remaining / (float) MetalPerRound);
				sText = lngPack.i18n ("Text~Comp~Producing") + ": ";
				sText += unitName + " (";
				sText += iToStr (iRound) + ")";

				if (font->getTextWide (sText, FONT_LATIN_SMALL_WHITE) > 126)
				{
					sText = lngPack.i18n ("Text~Comp~Producing") + ":\n";
					sText += unitName + " (";
					sText += iToStr (iRound) + ")";
				}

				return sText;
			}
			else //new unit is rdy + which kind of unit
			{
				sText = lngPack.i18n ("Text~Comp~Producing_Fin");
				sText += ": ";
				sText += unitName;

				if (font->getTextWide (sText) > 126)
				{
					sText = lngPack.i18n ("Text~Comp~Producing_Fin");
					sText += ":\n";
					sText += unitName;
				}
				return sText;
			}
		}

		// Research Center
		if (data.canResearch && owner == player)
		{
			string sText = lngPack.i18n ("Text~Comp~Working") + "\n";
			for (int area = 0; area < cResearch::kNrResearchAreas; area++)
			{
				if (owner->researchCentersWorkingOnArea[area] > 0)
				{
					switch (area)
					{
						case cResearch::kAttackResearch: sText += lngPack.i18n ("Text~Others~Attack"); break;
						case cResearch::kShotsResearch: sText += lngPack.i18n ("Text~Others~Shots_7"); break;
						case cResearch::kRangeResearch: sText += lngPack.i18n ("Text~Others~Range"); break;
						case cResearch::kArmorResearch: sText += lngPack.i18n ("Text~Others~Armor_7"); break;
						case cResearch::kHitpointsResearch: sText += lngPack.i18n ("Text~Others~Hitpoints_7"); break;
						case cResearch::kSpeedResearch: sText += lngPack.i18n ("Text~Others~Speed"); break;
						case cResearch::kScanResearch: sText += lngPack.i18n ("Text~Others~Scan"); break;
						case cResearch::kCostResearch: sText += lngPack.i18n ("Text~Others~Costs"); break;
					}
					sText += ": " + iToStr (owner->researchLevel.getRemainingTurns (area, owner->researchCentersWorkingOnArea[area])) + "\n";
				}
			}
			return sText;
		}

		// Goldraffinerie:
		if (data.convertsGold && owner == player)
		{
			string sText;
			sText = lngPack.i18n ("Text~Comp~Working") + "\n";
			sText += lngPack.i18n ("Text~Title~Credits") + ": ";
			sText += iToStr (owner->Credits);
			return sText;
		}

		return lngPack.i18n ("Text~Comp~Working");
	}

	if (isAttacking())
		return lngPack.i18n ("Text~Comp~AttackingStatusStr");
	else if (isBeeingAttacked())
		return lngPack.i18n ("Text~Comp~IsBeeingAttacked");
	else if (isSentryActive())
		return lngPack.i18n ("Text~Comp~Sentry");
	else if (isManualFireActive())
		return lngPack.i18n ("Text~Comp~ReactionFireOff");

	return lngPack.i18n ("Text~Comp~Waits");
}

//--------------------------------------------------------------------------
/** Refreshs all data to the maximum values */
//--------------------------------------------------------------------------
int cBuilding::refreshData()
{
	if (isDisabled())
	{
		lastShots = std::min (this->data.shotsMax, this->data.getAmmo());
		return 1;
	}

	if (data.getShots () < data.shotsMax)
	{
		data.setShots(std::min (this->data.shotsMax, this->data.getAmmo()));
		return 1;
	}
	return 0;
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

void cBuilding::render_rubble (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const
{
	assert (!owner);

	SDL_Rect src;

	if (data.isBig)
	{
		if (!UnitsData.dirt_big) return;
		src.w = src.h = (int) (UnitsData.dirt_big_org->h * zoomFactor);
	}
	else
	{
		if (!UnitsData.dirt_small) return;
		src.w = src.h = (int) (UnitsData.dirt_small_org->h * zoomFactor);
	}

	src.x = src.w * RubbleTyp;
	SDL_Rect tmp = dest;
	src.y = 0;

	// draw the shadows
	if (drawShadow)
	{
		if (data.isBig)
		{
			CHECK_SCALING (*UnitsData.dirt_big_shw, *UnitsData.dirt_big_shw_org, zoomFactor);
			SDL_BlitSurface (UnitsData.dirt_big_shw.get (), &src, surface, &tmp);
		}
		else
		{
			CHECK_SCALING (*UnitsData.dirt_small_shw, *UnitsData.dirt_small_shw_org, zoomFactor);
			SDL_BlitSurface (UnitsData.dirt_small_shw.get (), &src, surface, &tmp);
		}
	}

	// draw the building
	tmp = dest;

	if (data.isBig)
	{
		CHECK_SCALING (*UnitsData.dirt_big, *UnitsData.dirt_big_org, zoomFactor);
		SDL_BlitSurface (UnitsData.dirt_big.get (), &src, surface, &tmp);
	}
	else
	{
		CHECK_SCALING (*UnitsData.dirt_small, *UnitsData.dirt_small_org, zoomFactor);
		SDL_BlitSurface (UnitsData.dirt_small.get (), &src, surface, &tmp);
	}
}

void cBuilding::render_beton (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor) const
{
	SDL_Rect tmp = dest;
	if (data.isBig)
	{
		CHECK_SCALING (*GraphicsData.gfx_big_beton, *GraphicsData.gfx_big_beton_org, zoomFactor);

		if (StartUp && cSettings::getInstance().isAlphaEffects())
			SDL_SetSurfaceAlphaMod (GraphicsData.gfx_big_beton.get (), StartUp);
		else
			SDL_SetSurfaceAlphaMod (GraphicsData.gfx_big_beton.get (), 254);

		SDL_BlitSurface (GraphicsData.gfx_big_beton.get (), NULL, surface, &tmp);
	}
	else
	{
		CHECK_SCALING (*UnitsData.ptr_small_beton, *UnitsData.ptr_small_beton_org, zoomFactor);
		if (StartUp && cSettings::getInstance().isAlphaEffects())
			SDL_SetSurfaceAlphaMod (UnitsData.ptr_small_beton, StartUp);
		else
			SDL_SetSurfaceAlphaMod (UnitsData.ptr_small_beton, 254);

		SDL_BlitSurface (UnitsData.ptr_small_beton, NULL, surface, &tmp);
		SDL_SetSurfaceAlphaMod (UnitsData.ptr_small_beton, 254);
	}
}

void cBuilding::render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, int frameNr, int alpha) const
{
	// read the size:
	SDL_Rect src;
	src.x = 0;
	src.y = 0;
	if (data.hasFrames)
	{
		src.w = Round (64.0f * zoomFactor);
		src.h = Round (64.0f * zoomFactor);
	}
	else
	{
		src.w = (int) (uiData->img_org->w * zoomFactor);
		src.h = (int) (uiData->img_org->h * zoomFactor);
	}

	// blit the players color and building graphic
	if (data.hasPlayerColor) SDL_BlitSurface (owner->getColorSurface (), NULL, GraphicsData.gfx_tmp.get (), NULL);
	else SDL_FillRect (GraphicsData.gfx_tmp.get (), NULL, 0xFFFF00FF);

	if (data.hasFrames)
	{
		src.x = frameNr * Round (64.0f * zoomFactor);

		CHECK_SCALING (*uiData->img, *uiData->img_org, zoomFactor);
		SDL_BlitSurface (uiData->img, &src, GraphicsData.gfx_tmp.get (), NULL);

		src.x = 0;
	}
	else if (data.hasClanLogos)
	{
		CHECK_SCALING (*uiData->img, *uiData->img_org, zoomFactor);
		src.x = 0;
		src.y = 0;
		src.w = (int) (128 * zoomFactor);
		src.h = (int) (128 * zoomFactor);
		// select clan image
		if (owner->getClan() != -1)
			src.x = (int) ((owner->getClan() + 1) * 128 * zoomFactor);
		SDL_BlitSurface (uiData->img, &src, GraphicsData.gfx_tmp.get (), NULL);
	}
	else
	{
		CHECK_SCALING (*uiData->img, *uiData->img_org, zoomFactor);
		SDL_BlitSurface (uiData->img, NULL, GraphicsData.gfx_tmp.get (), NULL);
	}

	// draw the building
	SDL_Rect tmp = dest;

	src.x = 0;
	src.y = 0;

	SDL_SetSurfaceAlphaMod (GraphicsData.gfx_tmp.get (), alpha);
	SDL_BlitSurface (GraphicsData.gfx_tmp.get (), &src, surface, &tmp);
}


void cBuilding::render (unsigned long long animationTime, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow, bool drawConcrete) const
{
	// Note: when changing something in this function,
	// make sure to update the caching rules!

	// check, if it is dirt:
	if (!owner)
	{
		render_rubble (surface, dest, zoomFactor, drawShadow);
		return;
	}

	// draw the concrete
	if (data.hasBetonUnderground && drawConcrete)
	{
		render_beton (surface, dest, zoomFactor);
	}
	// draw the connector slots:
	if ((this->SubBase && !StartUp) || data.isConnectorGraphic)
	{
		drawConnectors (surface, dest, zoomFactor, drawShadow);
		if (data.isConnectorGraphic) return;
	}

	// draw the shadows
	if (drawShadow)
	{
		SDL_Rect tmp = dest;
		if (StartUp && cSettings::getInstance().isAlphaEffects())
			SDL_SetSurfaceAlphaMod (uiData->shw, StartUp / 5);
		else
			SDL_SetSurfaceAlphaMod (uiData->shw, 50);

		CHECK_SCALING (*uiData->shw, *uiData->shw_org, zoomFactor);
		blittAlphaSurface (uiData->shw, NULL, surface, &tmp);
	}

	int frameNr = dir;
	if (data.hasFrames && data.isAnimated && cSettings::getInstance().isAnimations() &&
		isDisabled() == false)
	{
		frameNr = (animationTime % data.hasFrames);
	}

	int alpha = 254;
	if (StartUp && cSettings::getInstance().isAlphaEffects()) alpha = StartUp;
	render_simple (surface, dest, zoomFactor, frameNr, alpha);
}

//--------------------------------------------------------------------------
void cBuilding::updateNeighbours (const cMap& map)
{
	int iPosOff = map.getOffset (PosX, PosY);
	if (!data.isBig)
	{
		owner->base.checkNeighbour (iPosOff - map.getSize(), *this);
		owner->base.checkNeighbour (iPosOff + 1, *this);
		owner->base.checkNeighbour (iPosOff + map.getSize(), *this);
		owner->base.checkNeighbour (iPosOff - 1, *this);
	}
	else
	{
		owner->base.checkNeighbour (iPosOff - map.getSize(), *this);
		owner->base.checkNeighbour (iPosOff - map.getSize() + 1, *this);
		owner->base.checkNeighbour (iPosOff + 2, *this);
		owner->base.checkNeighbour (iPosOff + 2 + map.getSize(), *this);
		owner->base.checkNeighbour (iPosOff + map.getSize() * 2, *this);
		owner->base.checkNeighbour (iPosOff + map.getSize() * 2 + 1, *this);
		owner->base.checkNeighbour (iPosOff - 1, *this);
		owner->base.checkNeighbour (iPosOff - 1 + map.getSize(), *this);
	}
	CheckNeighbours (map);
}

//--------------------------------------------------------------------------
/** Checks, if there are neighbours */
//--------------------------------------------------------------------------
void cBuilding::CheckNeighbours (const cMap& map)
{
#define CHECK_NEIGHBOUR(x, y, m) \
	if (map.isValidPos (x, y)) \
	{ \
		const cBuilding* b = map.fields[map.getOffset (x, y)].getTopBuilding(); \
		if (b && b->owner == owner && b->data.connectsToBase) \
		{m = true;}else{m = false;} \
	}

	if (!data.isBig)
	{
		CHECK_NEIGHBOUR (PosX    , PosY - 1, BaseN)
		CHECK_NEIGHBOUR (PosX + 1, PosY    , BaseE)
		CHECK_NEIGHBOUR (PosX    , PosY + 1, BaseS)
		CHECK_NEIGHBOUR (PosX - 1, PosY    , BaseW)
	}
	else
	{
		CHECK_NEIGHBOUR (PosX    , PosY - 1, BaseN)
		CHECK_NEIGHBOUR (PosX + 1, PosY - 1, BaseBN)
		CHECK_NEIGHBOUR (PosX + 2, PosY    , BaseE)
		CHECK_NEIGHBOUR (PosX + 2, PosY + 1, BaseBE)
		CHECK_NEIGHBOUR (PosX    , PosY + 2, BaseS)
		CHECK_NEIGHBOUR (PosX + 1, PosY + 2, BaseBS)
		CHECK_NEIGHBOUR (PosX - 1, PosY    , BaseW)
		CHECK_NEIGHBOUR (PosX - 1, PosY + 1, BaseBW)
	}
}

//--------------------------------------------------------------------------
/** Draws the connectors at the building: */
//--------------------------------------------------------------------------
void cBuilding::drawConnectors (SDL_Surface* surface, SDL_Rect dest, float zoomFactor, bool drawShadow) const
{
	SDL_Rect src, temp;

	CHECK_SCALING (*UnitsData.ptr_connector, *UnitsData.ptr_connector_org, zoomFactor);
	CHECK_SCALING (*UnitsData.ptr_connector_shw, *UnitsData.ptr_connector_shw_org, zoomFactor);

	if (StartUp) SDL_SetSurfaceAlphaMod (UnitsData.ptr_connector, StartUp);
	else SDL_SetSurfaceAlphaMod (UnitsData.ptr_connector, 254);

	src.y = 0;
	src.x = 0;
	src.h = src.w = UnitsData.ptr_connector->h;

	if (!data.isBig)
	{
		if (BaseN &&  BaseE &&  BaseS &&  BaseW) src.x = 15;
		else if (BaseN &&  BaseE &&  BaseS && !BaseW) src.x = 13;
		else if (BaseN &&  BaseE && !BaseS &&  BaseW) src.x = 12;
		else if (BaseN &&  BaseE && !BaseS && !BaseW) src.x =  8;
		else if (BaseN && !BaseE &&  BaseS &&  BaseW) src.x = 11;
		else if (BaseN && !BaseE &&  BaseS && !BaseW) src.x =  5;
		else if (BaseN && !BaseE && !BaseS &&  BaseW) src.x =  7;
		else if (BaseN && !BaseE && !BaseS && !BaseW) src.x =  1;
		else if (!BaseN &&  BaseE &&  BaseS &&  BaseW) src.x = 14;
		else if (!BaseN &&  BaseE &&  BaseS && !BaseW) src.x =  9;
		else if (!BaseN &&  BaseE && !BaseS &&  BaseW) src.x =  6;
		else if (!BaseN &&  BaseE && !BaseS && !BaseW) src.x =  2;
		else if (!BaseN && !BaseE &&  BaseS &&  BaseW) src.x = 10;
		else if (!BaseN && !BaseE &&  BaseS && !BaseW) src.x =  3;
		else if (!BaseN && !BaseE && !BaseS &&  BaseW) src.x =  4;
		else if (!BaseN && !BaseE && !BaseS && !BaseW) src.x =  0;
		src.x *= src.h;

		if (src.x != 0 || data.isConnectorGraphic)
		{
			// blit shadow
			temp = dest;
			if (drawShadow) blittAlphaSurface (UnitsData.ptr_connector_shw, &src, surface, &temp);
			// blit the image
			temp = dest;
			SDL_BlitSurface (UnitsData.ptr_connector, &src, surface, &temp);
		}
	}
	else
	{
		// make connector stubs of big buildings.
		// upper left field
		src.x = 0;
		if (BaseN &&  BaseW) src.x = 7;
		else if (BaseN && !BaseW) src.x = 1;
		else if (!BaseN &&  BaseW) src.x = 4;
		src.x *= src.h;

		if (src.x != 0)
		{
			temp = dest;
			if (drawShadow) blittAlphaSurface (UnitsData.ptr_connector_shw, &src, surface, &temp);
			temp = dest;
			SDL_BlitSurface (UnitsData.ptr_connector, &src, surface, &temp);
		}

		// upper right field
		src.x = 0;
		dest.x += Round (64.0f * zoomFactor);
		if (BaseBN &&  BaseE) src.x = 8;
		else if (BaseBN && !BaseE) src.x = 1;
		else if (!BaseBN &&  BaseE) src.x = 2;
		src.x *= src.h;

		if (src.x != 0)
		{
			temp = dest;
			if (drawShadow) blittAlphaSurface (UnitsData.ptr_connector_shw, &src, surface, &temp);
			temp = dest;
			SDL_BlitSurface (UnitsData.ptr_connector, &src, surface, &temp);
		}

		// lower right field
		src.x = 0;
		dest.y += Round (64.0f * zoomFactor);
		if (BaseBE && BaseBS) src.x = 9;
		else if (BaseBE && !BaseBS) src.x = 2;
		else if (!BaseBE && BaseBS) src.x = 3;
		src.x *= src.h;

		if (src.x != 0)
		{
			temp = dest;
			if (drawShadow) blittAlphaSurface (UnitsData.ptr_connector_shw, &src, surface, &temp);
			temp = dest;
			SDL_BlitSurface (UnitsData.ptr_connector, &src, surface, &temp);
		}

		// lower left field
		src.x = 0;
		dest.x -= Round (64.0f * zoomFactor);
		if (BaseS && BaseBW) src.x = 10;
		else if (BaseS && !BaseBW) src.x = 3;
		else if (!BaseS && BaseBW) src.x = 4;
		src.x *= src.h;

		if (src.x != 0)
		{
			temp = dest;
			if (drawShadow) blittAlphaSurface (UnitsData.ptr_connector_shw, &src, surface, &temp);
			temp = dest;
			SDL_BlitSurface (UnitsData.ptr_connector, &src, surface, &temp);
		}
	}
}

//--------------------------------------------------------------------------
/** starts the building for the server thread */
//--------------------------------------------------------------------------
void cBuilding::ServerStartWork (cServer& server)
{
	if (isUnitWorking ())
	{
		sendDoStartWork (server, *this);
		return;
	}

	//-- first check all requirements

	if (isDisabled())
	{
		sendChatMessageToClient (server, "Text~Comp~Building_Disabled", SERVER_ERROR_MESSAGE, owner->getNr());
		return;
	}

	// needs human workers:
	if (data.needsHumans)
		if (SubBase->HumanNeed + data.needsHumans > SubBase->HumanProd)
		{
			sendChatMessageToClient (server, "Text~Comp~Team_Insufficient", SERVER_ERROR_MESSAGE, owner->getNr());
			return;
		}

	// needs gold:
	if (data.convertsGold)
	{
		if (data.convertsGold + SubBase->GoldNeed > SubBase->getGoldProd() + SubBase->getGold())
		{
			sendChatMessageToClient (server, "Text~Comp~Gold_Insufficient", SERVER_ERROR_MESSAGE, owner->getNr());
			return;
		}
	}

	// needs raw material:
	if (data.needsMetal)
	{
		if (SubBase->MetalNeed + min (MetalPerRound, BuildList[0].metall_remaining) > SubBase->getMetalProd() + SubBase->getMetal())
		{
			sendChatMessageToClient (server, "Text~Comp~Metal_Insufficient", SERVER_ERROR_MESSAGE, owner->getNr());
			return;
		}
	}

	// needs oil:
	if (data.needsOil)
	{
		// check if there is enough Oil for the generators
		// (current production + reserves)
		if (data.needsOil + SubBase->OilNeed > SubBase->getOil () + SubBase->getMaxOilProd())
		{
			sendChatMessageToClient (server, "Text~Comp~Fuel_Insufficient", SERVER_ERROR_MESSAGE, owner->getNr());
			return;
		}
		else if (data.needsOil + SubBase->OilNeed > SubBase->getOil () + SubBase->getOilProd ())
		{
			// increase oil production
			int missingOil = data.needsOil + SubBase->OilNeed - (SubBase->getOil () + SubBase->getOilProd ());

			int metal = SubBase->getMetalProd();
			int gold = SubBase->getGoldProd();

			// temporay decrease metal and gold production
			SubBase->setMetalProd (0);
			SubBase->setGoldProd (0);

			SubBase->changeOilProd (missingOil);

			SubBase->setGoldProd (gold);
			SubBase->setMetalProd (metal);

			sendChatMessageToClient (server, "Text~Comp~Adjustments_Fuel_Increased", SERVER_INFO_MESSAGE, owner->getNr(), iToStr (missingOil));
			if (SubBase->getMetalProd() < metal)
				sendChatMessageToClient (server, "Text~Comp~Adjustments_Metal_Decreased", SERVER_INFO_MESSAGE, owner->getNr(), iToStr (metal - SubBase->getMetalProd()));
			if (SubBase->getGoldProd() < gold)
				sendChatMessageToClient (server, "Text~Comp~Adjustments_Gold_Decreased", SERVER_INFO_MESSAGE, owner->getNr(), iToStr (gold - SubBase->getGoldProd()));
		}
	}

	// IsWorking is set to true before checking the energy production.
	// So if an energy generator has to be started,
	// it can use the fuel production of this building
	// (when this building is a mine).
	setWorking(true);

	// set mine values. This has to be undone, if the energy is insufficient
	if (data.canMineMaxRes > 0)
	{
		int mineFree = data.canMineMaxRes;

		SubBase->changeMetalProd (MaxMetalProd);
		mineFree -= MaxMetalProd;

		SubBase->changeGoldProd (min (MaxGoldProd, mineFree));
		mineFree -= min (MaxGoldProd, mineFree);

		SubBase->changeOilProd (min (MaxOilProd, mineFree));
	}

	// Energy consumers:
	if (data.needsEnergy)
	{
		if (data.needsEnergy + SubBase->EnergyNeed > SubBase->EnergyProd)
		{
			// try to increase energy production
			if (!SubBase->increaseEnergyProd (server, data.needsEnergy + SubBase->EnergyNeed - SubBase->EnergyProd))
			{
				setWorking(false);

				// reset mine values
				if (data.canMineMaxRes > 0)
				{
					int metal = SubBase->getMetalProd();
					int oil =  SubBase->getOilProd();
					int gold = SubBase->getGoldProd();

					SubBase->setMetalProd (0);
					SubBase->setOilProd (0);
					SubBase->setGoldProd (0);

					SubBase->setMetalProd (min (metal, SubBase->getMaxAllowedMetalProd()));
					SubBase->setGoldProd (min (gold, SubBase->getMaxAllowedGoldProd()));
					SubBase->setOilProd (min (oil, SubBase->getMaxAllowedOilProd()));
				}

				sendChatMessageToClient (server, "Text~Comp~Energy_Insufficient", SERVER_ERROR_MESSAGE, owner->getNr());
				return;
			}
			sendChatMessageToClient (server, "Text~Comp~Energy_ToLow", SERVER_INFO_MESSAGE, owner->getNr());
		}
	}

	//-- everything is ready to start the building

	SubBase->EnergyProd += data.produceEnergy;
	SubBase->EnergyNeed += data.needsEnergy;

	SubBase->HumanNeed += data.needsHumans;
	SubBase->HumanProd += data.produceHumans;

	SubBase->OilNeed += data.needsOil;

	// raw material consumer:
	if (data.needsMetal)
		SubBase->MetalNeed += min (MetalPerRound, BuildList[0].metall_remaining);

	// gold consumer:
	SubBase->GoldNeed += data.convertsGold;

	// research building
	if (data.canResearch)
	{
		owner->workingResearchCenterCount++;
		owner->researchCentersWorkingOnArea[researchArea]++;
	}

	if (data.canScore)
	{
		sendNumEcos (server, *owner);
	}

	sendSubbaseValues (server, *SubBase, owner->getNr());
	sendDoStartWork (server, *this);
}

//------------------------------------------------------------
/** starts the building in the client thread */
//------------------------------------------------------------
void cBuilding::clientStartWork ()
{
	if (isUnitWorking ())
		return;
	setWorking(true);
	EffectAlpha = 0;
	if (data.canResearch)
		owner->startAResearch (researchArea);
}

//--------------------------------------------------------------------------
/** Stops the building's working in the server thread */
//--------------------------------------------------------------------------
void cBuilding::ServerStopWork (cServer& server, bool override)
{
	if (!isUnitWorking ())
	{
		sendDoStopWork (server, *this);
		return;
	}

	// energy generators
	if (data.produceEnergy)
	{
		if (SubBase->EnergyNeed > SubBase->EnergyProd - data.produceEnergy && !override)
		{
			sendChatMessageToClient (server, "Text~Comp~Energy_IsNeeded", SERVER_ERROR_MESSAGE, owner->getNr());
			return;
		}

		SubBase->EnergyProd -= data.produceEnergy;
		SubBase->OilNeed -= data.needsOil;
	}

	setWorking(false);

	// Energy consumers:
	if (data.needsEnergy)
		SubBase->EnergyNeed -= data.needsEnergy;

	// raw material consumer:
	if (data.needsMetal)
		SubBase->MetalNeed -= min (MetalPerRound, BuildList[0].metall_remaining);

	// gold consumer
	if (data.convertsGold)
		SubBase->GoldNeed -= data.convertsGold;

	// human consumer
	if (data.needsHumans)
		SubBase->HumanNeed -= data.needsHumans;

	// Minen:
	if (data.canMineMaxRes > 0)
	{
		int metal = SubBase->getMetalProd();
		int oil =  SubBase->getOilProd();
		int gold = SubBase->getGoldProd();

		SubBase->setMetalProd (0);
		SubBase->setOilProd (0);
		SubBase->setGoldProd (0);

		SubBase->setMetalProd (min (metal, SubBase->getMaxAllowedMetalProd()));
		SubBase->setGoldProd (min (gold, SubBase->getMaxAllowedGoldProd()));
		SubBase->setOilProd (min (oil, SubBase->getMaxAllowedOilProd()));
	}

	if (data.canResearch)
	{
		owner->workingResearchCenterCount--;
		owner->researchCentersWorkingOnArea[researchArea]--;
	}

	if (data.canScore)
	{
		sendNumEcos (server, *owner);
	}

	sendSubbaseValues (server, *SubBase, owner->getNr());
	sendDoStopWork (server, *this);
}

//------------------------------------------------------------
/** stops the building in the client thread */
//------------------------------------------------------------
void cBuilding::clientStopWork ()
{
	if (!isUnitWorking ())
		return;
	setWorking(false);
	if (data.canResearch)
		owner->stopAResearch (researchArea);
}

//------------------------------------------------------------
bool cBuilding::canTransferTo (const cPosition& position, const cMapField& overUnitField) const
{
	if (overUnitField.getVehicle ())
	{
		const cVehicle* v = overUnitField.getVehicle ();

		if (v->owner != this->owner)
			return false;

		if (v->data.storeResType != data.storeResType)
			return false;

		if (v->isUnitBuildingABuilding () || v->isUnitClearing ())
			return false;

		for (size_t i = 0; i != SubBase->buildings.size(); ++i)
		{
			const cBuilding* b = SubBase->buildings[i];

			if (b->isNextTo (position)) return true;
		}

		return false;
	}
	else if (overUnitField.getTopBuilding ())
	{
		const cBuilding* b = overUnitField.getTopBuilding ();

		if (b == this)
			return false;

		if (b->SubBase != SubBase)
			return false;

		if (b->owner != this->owner)
			return false;

		if (data.storeResType != b->data.storeResType)
			return false;

		return true;
	}
	return false;
}

//--------------------------------------------------------------------------
bool cBuilding::canExitTo (const cPosition& position, const cMap& map, const sUnitData& vehicleData) const
{
	if (!map.possiblePlaceVehicle (vehicleData, position.x (), position.y (), owner)) return false;
	if (!isNextTo (position)) return false;

	return true;
}

//--------------------------------------------------------------------------
bool cBuilding::canLoad (int x, int y, const cMap& map, bool checkPosition) const
{
	if (map.isValidPos (x, y) == false) return false;
	const int offset = map.getOffset (x, y);

	if (canLoad (map.fields[offset].getPlane(), checkPosition)) return true;
	else return canLoad (map.fields[offset].getVehicle(), checkPosition);
}

//--------------------------------------------------------------------------
/** returns, if the vehicle can be loaded from its position: */
//--------------------------------------------------------------------------
bool cBuilding::canLoad (const cVehicle* Vehicle, bool checkPosition) const
{
	if (!Vehicle) return false;

	if (Vehicle->isUnitLoaded ()) return false;

	if (data.storageUnitsCur == data.storageUnitsMax) return false;

	if (checkPosition && !isNextTo (Vehicle->PosX, Vehicle->PosY)) return false;

	if (!Contains (data.storeUnitsTypes, Vehicle->data.isStorageType)) return false;

	if (Vehicle->ClientMoveJob && (Vehicle->moving || Vehicle->isAttacking() || Vehicle->MoveJobActive)) return false;

	if (Vehicle->owner != owner || Vehicle->isUnitBuildingABuilding () || Vehicle->isUnitClearing ()) return false;

	if (Vehicle->isBeeingAttacked ()) return false;

	return true;
}

//--------------------------------------------------------------------------
/** loads a vehicle: */
//--------------------------------------------------------------------------
void cBuilding::storeVehicle (cVehicle& vehicle, cMap& map)
{
	map.deleteVehicle (vehicle);
	if (vehicle.isSentryActive())
	{
		vehicle.owner->deleteSentry (vehicle);
	}

	vehicle.setLoaded(true);

	storedUnits.push_back (&vehicle);
	data.storageUnitsCur++;

	owner->doScan();
}

//-----------------------------------------------------------------------
// Unloads a vehicle
void cBuilding::exitVehicleTo (cVehicle& vehicle, int offset, cMap& map)
{
	Remove (storedUnits, &vehicle);

	data.storageUnitsCur--;

	map.addVehicle (vehicle, offset);

	vehicle.PosX = offset % map.getSize();
	vehicle.PosY = offset / map.getSize ();

	vehicle.setLoaded (false);

	owner->doScan();
}

//-------------------------------------------------------------------------------
// Draws big symbols for the info menu:
//-------------------------------------------------------------------------------
void cBuilding::DrawSymbolBig (eSymbolsBig sym, int x, int y, int maxx, int value, int orgvalue, SDL_Surface* sf)
{
	SDL_Rect src = {0, 0, 0, 0};

	switch (sym)
	{
		case SBSpeed:
			src.x = 0;
			src.y = 109;
			src.w = 11;
			src.h = 12;
			break;

		case SBHits:
			src.x = 11;
			src.y = 109;
			src.w = 7;
			src.h = 11;
			break;

		case SBAmmo:
			src.x = 18;
			src.y = 109;
			src.w = 9;
			src.h = 14;
			break;

		case SBAttack:
			src.x = 27;
			src.y = 109;
			src.w = 10;
			src.h = 14;
			break;

		case SBShots:
			src.x = 37;
			src.y = 109;
			src.w = 15;
			src.h = 7;
			break;

		case SBRange:
			src.x = 52;
			src.y = 109;
			src.w = 13;
			src.h = 13;
			break;

		case SBArmor:
			src.x = 65;
			src.y = 109;
			src.w = 11;
			src.h = 14;
			break;

		case SBScan:
			src.x = 76;
			src.y = 109;
			src.w = 13;
			src.h = 13;
			break;

		case SBMetal:
			src.x = 89;
			src.y = 109;
			src.w = 12;
			src.h = 15;
			break;

		case SBOil:
			src.x = 101;
			src.y = 109;
			src.w = 11;
			src.h = 12;
			break;

		case SBGold:
			src.x = 112;
			src.y = 109;
			src.w = 13;
			src.h = 10;
			break;

		case SBEnergy:
			src.x = 125;
			src.y = 109;
			src.w = 13;
			src.h = 17;
			break;

		case SBHuman:
			src.x = 138;
			src.y = 109;
			src.w = 12;
			src.h = 16;
			break;
	}

	maxx -= src.w;

	if (orgvalue < value)
	{
		maxx -= src.w + 3;
	}

	int offx = src.w;

	while (offx * value >= maxx)
	{
		offx--;

		if (offx < 4)
		{
			value /= 2;
			orgvalue /= 2;
			offx = src.w;
		}
	}

	SDL_Rect dest;
	dest.x = x;
	dest.y = y;

	Uint32 color = SDL_MapRGB (sf->format, 0xFC, 0, 0);
	for (int i = 0; i < value; i++)
	{
		if (i == orgvalue)
		{
			SDL_Rect mark;
			dest.x += src.w + 3;
			mark.x = dest.x - src.w / 2;
			mark.y = dest.y;
			mark.w = 1;
			mark.h = src.h;
			SDL_FillRect (sf, &mark, color);
		}

		SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get (), &src, sf, &dest);

		dest.x += offx;
	}
}

//-------------------------------------------------------------------------------
/** checks the resources that are available under the mining station */
//--------------------------------------------------------------------------

void cBuilding::CheckRessourceProd (const cServer& server)
{
	int pos = server.Map->getOffset (PosX, PosY);

	MaxMetalProd = 0;
	MaxGoldProd = 0;
	MaxOilProd = 0;
	const sResources* res = &server.Map->getResource (pos);

	switch (res->typ)
	{
		case RES_METAL: MaxMetalProd += res->value; break;
		case RES_GOLD:  MaxGoldProd  += res->value; break;
		case RES_OIL:   MaxOilProd   += res->value; break;
	}

	pos++;
	res = &server.Map->getResource (pos);
	switch (res->typ)
	{
		case RES_METAL: MaxMetalProd += res->value; break;
		case RES_GOLD:  MaxGoldProd  += res->value; break;
		case RES_OIL:   MaxOilProd   += res->value; break;
	}

	pos += server.Map->getSize();
	res = &server.Map->getResource (pos);
	switch (res->typ)
	{
		case RES_METAL: MaxMetalProd += res->value; break;
		case RES_GOLD:  MaxGoldProd  += res->value; break;
		case RES_OIL:   MaxOilProd   += res->value; break;
	}

	pos--;
	res = &server.Map->getResource (pos);
	switch (res->typ)
	{
		case RES_METAL: MaxMetalProd += res->value; break;
		case RES_GOLD:  MaxGoldProd  += res->value; break;
		case RES_OIL:   MaxOilProd   += res->value; break;
	}

	MaxMetalProd = min (MaxMetalProd, data.canMineMaxRes);
	MaxGoldProd  = min (MaxGoldProd,  data.canMineMaxRes);
	MaxOilProd   = min (MaxOilProd,   data.canMineMaxRes);
}

//--------------------------------------------------------------------------
/** calculates the costs and the duration of the 3 buildspeeds
 * for the vehicle with the given base costs
 * iRemainingMetal is only needed for recalculating costs of vehicles
 * in the Buildqueue and is set per default to -1 */
//--------------------------------------------------------------------------
void cBuilding::calcTurboBuild (std::array<int, 3>& turboBuildRounds, std::array<int, 3>& turboBuildCosts, int vehicleCosts, int remainingMetal) const
{
	// first calc costs for a new Vehical

	// 1x
	turboBuildCosts[0] = vehicleCosts;

	// 2x
	int a = turboBuildCosts[0];
	turboBuildCosts[1] = turboBuildCosts[0];

	while (a >= 2 * data.needsMetal)
	{
		turboBuildCosts[1] += 2 * data.needsMetal;
		a -= 2 * data.needsMetal;
	}

	// 4x
	turboBuildCosts[2] = turboBuildCosts[1];
	a = turboBuildCosts[1];

	while (a >= 15)
	{
		turboBuildCosts[2] += (12 * data.needsMetal - min (a, 8 * data.needsMetal));
		a -= 8 * data.needsMetal;
	}

	// now this is a litle bit tricky ...
	// trying to calculate a plausible value,
	// if we are changing the speed of an already started build-job
	if (remainingMetal >= 0)
	{
		float WorkedRounds;

		switch (BuildSpeed)  // BuildSpeed here is the previous build speed
		{
			case 0:
				WorkedRounds = (turboBuildCosts[0] - remainingMetal) / (1.f * data.needsMetal);
				turboBuildCosts[0] -= (int) (1     *  1 * data.needsMetal * WorkedRounds);
				turboBuildCosts[1] -= (int) (0.5f  *  4 * data.needsMetal * WorkedRounds);
				turboBuildCosts[2] -= (int) (0.25f * 12 * data.needsMetal * WorkedRounds);
				break;

			case 1:
				WorkedRounds = (turboBuildCosts[1] - remainingMetal) / (float)(4 * data.needsMetal);
				turboBuildCosts[0] -= (int) (2    *  1 * data.needsMetal * WorkedRounds);
				turboBuildCosts[1] -= (int) (1    *  4 * data.needsMetal * WorkedRounds);
				turboBuildCosts[2] -= (int) (0.5f * 12 * data.needsMetal * WorkedRounds);
				break;

			case 2:
				WorkedRounds = (turboBuildCosts[2] - remainingMetal) / (float)(12 * data.needsMetal);
				turboBuildCosts[0] -= (int) (4 *  1 * data.needsMetal * WorkedRounds);
				turboBuildCosts[1] -= (int) (2 *  4 * data.needsMetal * WorkedRounds);
				turboBuildCosts[2] -= (int) (1 * 12 * data.needsMetal * WorkedRounds);
				break;
		}
	}

	// calc needed Rounds
	turboBuildRounds[0] = (int) ceilf (turboBuildCosts[0] / (1.f * data.needsMetal));

	if (data.maxBuildFactor > 1)
	{
		turboBuildRounds[1] = (int) ceilf (turboBuildCosts[1] / (4.f * data.needsMetal));
		turboBuildRounds[2] = (int) ceilf (turboBuildCosts[2] / (12.f * data.needsMetal));
	}
	else
	{
		turboBuildRounds[1] = 0;
		turboBuildRounds[2] = 0;
	}
}

//--------------------------------------------------------------------------
bool cBuilding::isDetectedByPlayer (const cPlayer* player) const
{
	return Contains (detectedByPlayerList, player);
}

//--------------------------------------------------------------------------
void cBuilding::setDetectedByPlayer (cServer& server, cPlayer* player, bool addToDetectedInThisTurnList)
{
	if (!isDetectedByPlayer (player))
		detectedByPlayerList.push_back (player);
}

//--------------------------------------------------------------------------
void cBuilding::resetDetectedByPlayer (const cPlayer* player)
{
	Remove (detectedByPlayerList, player);
}

//--------------------------------------------------------------------------
void cBuilding::makeDetection (cServer& server)
{
	// check whether the building has been detected by others
	if (data.isStealthOn == TERRAIN_NONE) return;

	if (data.isStealthOn & AREA_EXP_MINE)
	{
		int offset = server.Map->getOffset (PosX, PosY);
		std::vector<cPlayer*>& playerList = server.PlayerList;
		for (unsigned int i = 0; i < playerList.size(); i++)
		{
			cPlayer* player = playerList[i];
			if (player == owner) continue;
			if (player->hasMineDetection (offset))
			{
				setDetectedByPlayer (server, player);
			}
		}
	}
}

//--------------------------------------------------------------------------
sBuildingUIData::sBuildingUIData() :
	img (NULL), img_org (NULL),
	shw (NULL), shw_org (NULL),
	eff (NULL), eff_org (NULL),
	video (NULL),
	info (NULL),
	Start (NULL),
	Running (NULL),
	Stop (NULL),
	Attack (NULL),
	Wait (NULL)
{}

//--------------------------------------------------------------------------
void sBuildingUIData::scaleSurfaces (float factor)
{
	scaleSurface (img_org, img, (int) (img_org->w * factor), (int) (img_org->h * factor));
	scaleSurface (shw_org, shw, (int) (shw_org->w * factor), (int) (shw_org->h * factor));
	if (eff_org) scaleSurface (eff_org, eff, (int) (eff_org->w * factor), (int) (eff_org->h * factor));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- methods, that already have been extracted as part of cUnit refactoring
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
bool cBuilding::factoryHasJustFinishedBuilding() const
{
	return (!BuildList.empty() && isUnitWorking() == false && BuildList[0].metall_remaining <= 0);
}

//-----------------------------------------------------------------------------
void cBuilding::executeStopCommand (const cClient& client) const
{
	sendWantStopWork (client, *this);
}

//-----------------------------------------------------------------------------
void cBuilding::executeUpdateBuildingCommmand (const cClient& client, bool updateAllOfSameType) const
{
	sendUpgradeBuilding (client, *this, updateAllOfSameType);
}

//-----------------------------------------------------------------------------
bool cBuilding::buildingCanBeStarted() const
{
	return (data.canWork && isUnitWorking() == false
			&& (!BuildList.empty() || data.canBuild.empty()));
}

//-----------------------------------------------------------------------------
bool cBuilding::buildingCanBeUpgraded() const
{
	const sUnitData& upgraded = *owner->getUnitDataCurrentVersion (data.ID);
	return (data.getVersion () != upgraded.getVersion () && SubBase && SubBase->getMetal () >= 2);
}

//-----------------------------------------------------------------------------
void cBuilding::setWorking (bool value)
{
	std::swap (isWorking, value);
	if (value != isWorking) workingChanged ();
}
