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
#include "dialog.h"
#include "events.h"
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

using namespace std;

//--------------------------------------------------------------------------
// cBuilding Implementation
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
cBuilding::cBuilding (const sUnitData* b, cPlayer* Owner, unsigned int ID) :
	cUnit ( (Owner != 0 && b != 0) ? Owner->getUnitDataCurrentVersion (b->ID) : 0,
			Owner,
			ID),
	next (0),
	prev (0),
	BuildList (0)
{
	sentryActive = data.canAttack != TERRAIN_NONE;

	RubbleTyp = 0;
	RubbleValue = 0;
	EffectAlpha = 0;
	EffectInc = true;
	StartUp = 0;
	IsWorking = false;
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
	hasBeenAttacked = false;

	MaxMetalProd = 0;
	MaxGoldProd = 0;
	MaxOilProd = 0;
	data.hitpointsCur = data.hitpointsMax;
	data.ammoCur = data.ammoMax;
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
string cBuilding::getStatusStr (const cGameGUI& gameGUI) const
{
	if (isDisabled())
	{
		string sText;
		sText = lngPack.i18n ("Text~Comp~Disabled") + " (";
		sText += iToStr (turnsDisabled) + ")";
		return sText;
	}
	if (IsWorking || (factoryHasJustFinishedBuilding() && isDisabled() == false))
	{
		const cPlayer* activePlayer = gameGUI.getClient()->getActivePlayer();
		// Factory:
		if (!data.canBuild.empty() && !BuildList.empty() && owner == activePlayer)
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
		if (data.canResearch && owner == activePlayer)
		{
			string sText = lngPack.i18n ("Text~Comp~Working") + "\n";
			for (int area = 0; area < cResearch::kNrResearchAreas; area++)
			{
				if (owner->researchCentersWorkingOnArea[area] > 0)
				{
					switch (area)
					{
						case cResearch::kAttackResearch: sText += lngPack.i18n ("Text~Vehicles~Damage"); break;
						case cResearch::kShotsResearch: sText += lngPack.i18n ("Text~Hud~Shots"); break;
						case cResearch::kRangeResearch: sText += lngPack.i18n ("Text~Hud~Range"); break;
						case cResearch::kArmorResearch: sText += lngPack.i18n ("Text~Vehicles~Armor"); break;
						case cResearch::kHitpointsResearch: sText += lngPack.i18n ("Text~Hud~Hitpoints"); break;
						case cResearch::kSpeedResearch: sText += lngPack.i18n ("Text~Hud~Speed"); break;
						case cResearch::kScanResearch: sText += lngPack.i18n ("Text~Hud~Scan"); break;
						case cResearch::kCostResearch: sText += lngPack.i18n ("Text~Vehicles~Costs"); break;
					}
					sText += ": " + iToStr (owner->researchLevel.getRemainingTurns (area, owner->researchCentersWorkingOnArea[area])) + "\n";
				}
			}
			return sText;
		}

		// Goldraffinerie:
		if (data.convertsGold && owner == activePlayer)
		{
			string sText;
			sText = lngPack.i18n ("Text~Comp~Working") + "\n";
			sText += lngPack.i18n ("Text~Title~Credits") + ": ";
			sText += iToStr (owner->Credits);
			return sText;
		}

		return lngPack.i18n ("Text~Comp~Working");
	}

	if (sentryActive)
		return lngPack.i18n ("Text~Comp~Sentry");
	else if (manualFireActive)
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
		lastShots = std::min (this->data.shotsMax, this->data.ammoCur);
		return 1;
	}

	if (data.shotsCur < data.shotsMax)
	{
		data.shotsCur = std::min (this->data.shotsMax, this->data.ammoCur);
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

//--------------------------------------------------------------------------
void cBuilding::draw (SDL_Rect* screenPos, cGameGUI& gameGUI)
{
	SDL_Rect dest, tmp;
	float factor = (float) gameGUI.getTileSize() / 64.0f;
	cPlayer* activePlayer = gameGUI.getClient()->getActivePlayer();
	// draw the damage effects
	if (gameGUI.timer100ms && data.hasDamageEffect &&
		data.hitpointsCur < data.hitpointsMax &&
		cSettings::getInstance().isDamageEffects() &&
		(owner == activePlayer || activePlayer->canSeeAnyAreaUnder (*this)))
	{
		int intense = (int) (200 - 200 * ( (float) data.hitpointsCur / data.hitpointsMax));
		gameGUI.addFx (new cFxDarkSmoke (PosX * 64 + DamageFXPointX, PosY * 64 + DamageFXPointY, intense, gameGUI.getWindDir()));

		if (data.isBig && intense > 50)
		{
			intense -= 50;
			gameGUI.addFx (new cFxDarkSmoke (PosX * 64 + DamageFXPointX2, PosY * 64 + DamageFXPointY2, intense, gameGUI.getWindDir()));
		}
	}

	dest.x = dest.y = 0;
	bool bDraw = false;
	SDL_Surface* drawingSurface = gameGUI.getDCache()->getCachedImage (*this);
	if (drawingSurface == NULL)
	{
		// no cached image found. building needs to be redrawn.
		bDraw = true;
		drawingSurface = gameGUI.getDCache()->createNewEntry (*this);
	}

	if (drawingSurface == NULL)
	{
		// image will not be cached. So blitt directly to the screen buffer.
		dest = *screenPos;
		drawingSurface = buffer;
	}

	if (bDraw)
	{
		render (&gameGUI, drawingSurface, dest, (float) gameGUI.getTileSize() / 64.0f, cSettings::getInstance().isShadows(), true);
	}

	// now check, whether the image has to be blitted to screen buffer
	if (drawingSurface != buffer)
	{
		dest = *screenPos;
		SDL_BlitSurface (drawingSurface, NULL, buffer, &dest);

		// all following graphic operations are drawn directly to buffer
		dest = *screenPos;
	}

	if (!owner) return;

	if (StartUp)
	{
		if (gameGUI.timer100ms)
			StartUp += 25;

		if (StartUp >= 255)
			StartUp = 0;
	}

	// draw the effect if necessary
	if (data.powerOnGraphic && cSettings::getInstance().isAnimations() && (IsWorking || !data.canWork))
	{
		tmp = dest;
		SDL_SetAlpha (uiData->eff, SDL_SRCALPHA, EffectAlpha);

		CHECK_SCALING (uiData->eff, uiData->eff_org, factor);
		SDL_BlitSurface (uiData->eff, NULL, buffer, &tmp);

		if (gameGUI.timer100ms)
		{
			if (EffectInc)
			{
				EffectAlpha += 30;

				if (EffectAlpha > 220)
				{
					EffectAlpha = 255;
					EffectInc = false;
				}
			}
			else
			{
				EffectAlpha -= 30;

				if (EffectAlpha < 30)
				{
					EffectAlpha = 0;
					EffectInc = true;
				}
			}
		}
	}

	// draw the mark, when a build order is finished
	if ( ( (!BuildList.empty() && !IsWorking && BuildList[0].metall_remaining <= 0) || (data.canResearch && owner->researchFinished)) && owner == gameGUI.getClient()->getActivePlayer())
	{
		const Uint32 color = 0xFF00 - (0x1000 * (gameGUI.getAnimationSpeed() % 0x8));
		const Uint16 max = data.isBig ? 2 * gameGUI.getTileSize() - 3 : gameGUI.getTileSize() - 3;
		SDL_Rect d = {Sint16 (dest.x + 2), Sint16 (dest.y + 2), max, max};

		DrawRectangle (buffer, d, color, 3);
	}

#if 0
	// disabled color-frame for buildings
	//   => now it's original game behavior - see ticket #542 (GER) = FIXED
	// but maybe as setting interresting
	//   => ticket #784 (ENG) (so I just commented it) = TODO

	// draw a colored frame if necessary
	if (gameGUI.colorChecked())
	{
		const Uint32 color = *static_cast<Uint32*> (owner->getColorSurface()->pixels);
		const Uint16 max = data.isBig ? 2 * gameGUI.getTileSize() - 1 : gameGUI.getTileSize() - 1;
		SDL_Rect d = {Sint16 (dest.x + 1), Sint16 (dest.y + 1), max, max};

		DrawRectangle (buffer, d, color, 1);
	}
#endif
	// draw the seleted-unit-flash-frame for bulidings
	if (gameGUI.getSelectedUnit() == this)
	{
		Uint16 max = data.isBig ? gameGUI.getTileSize() * 2 : gameGUI.getTileSize();
		const int len = max / 4;
		max -= 3;
		SDL_Rect d = {Sint16 (dest.x + 2), Sint16 (dest.y + 2), max, max};
		DrawSelectionCorner(buffer, d, len, gameGUI.getBlinkColor());
	}

	// draw health bar
	if (gameGUI.hitsChecked())
		gameGUI.drawHealthBar (*this, *screenPos);

	// draw ammo bar
	if (gameGUI.ammoChecked() && data.canAttack && data.ammoMax > 0)
		gameGUI.drawMunBar (*this, *screenPos);

	// draw status
	if (gameGUI.statusChecked())
		gameGUI.drawStatus (*this, *screenPos);

	// attack job debug output
	if (gameGUI.getAJobDebugStatus())
	{
		cServer* server = gameGUI.getClient()->getServer();
		const cBuilding* serverBuilding = NULL;
		if (server) serverBuilding = server->Map->fields[server->Map->getOffset (PosX, PosY)].getBuilding();
		if (isBeeingAttacked) font->showText (dest.x + 1, dest.y + 1, "C: attacked", FONT_LATIN_SMALL_WHITE);
		if (serverBuilding && serverBuilding->isBeeingAttacked) font->showText (dest.x + 1, dest.y + 9, "S: attacked", FONT_LATIN_SMALL_YELLOW);
		if (attacking) font->showText (dest.x + 1, dest.y + 17, "C: attacking", FONT_LATIN_SMALL_WHITE);
		if (serverBuilding && serverBuilding->attacking) font->showText (dest.x + 1, dest.y + 25, "S: attacking", FONT_LATIN_SMALL_YELLOW);
	}
}

void cBuilding::render_rubble (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow)
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
			CHECK_SCALING (UnitsData.dirt_big_shw, UnitsData.dirt_big_shw_org, zoomFactor);
			SDL_BlitSurface (UnitsData.dirt_big_shw, &src, surface, &tmp);
		}
		else
		{
			CHECK_SCALING (UnitsData.dirt_small_shw, UnitsData.dirt_small_shw_org, zoomFactor);
			SDL_BlitSurface (UnitsData.dirt_small_shw, &src, surface, &tmp);
		}
	}

	// draw the building
	tmp = dest;

	if (data.isBig)
	{
		CHECK_SCALING (UnitsData.dirt_big, UnitsData.dirt_big_org, zoomFactor);
		SDL_BlitSurface (UnitsData.dirt_big, &src, surface, &tmp);
	}
	else
	{
		CHECK_SCALING (UnitsData.dirt_small, UnitsData.dirt_small_org, zoomFactor);
		SDL_BlitSurface (UnitsData.dirt_small, &src, surface, &tmp);
	}
}

void cBuilding::render_beton (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor)
{
	SDL_Rect tmp = dest;
	if (data.isBig)
	{
		CHECK_SCALING (GraphicsData.gfx_big_beton, GraphicsData.gfx_big_beton_org, zoomFactor);

		if (StartUp && cSettings::getInstance().isAlphaEffects())
			SDL_SetAlpha (GraphicsData.gfx_big_beton, SDL_SRCALPHA, StartUp);
		else
			SDL_SetAlpha (GraphicsData.gfx_big_beton, SDL_SRCALPHA, 255);

		SDL_BlitSurface (GraphicsData.gfx_big_beton, NULL, surface, &tmp);
	}
	else
	{
		CHECK_SCALING (UnitsData.ptr_small_beton, UnitsData.ptr_small_beton_org, zoomFactor);
		if (StartUp && cSettings::getInstance().isAlphaEffects())
			SDL_SetAlpha (UnitsData.ptr_small_beton, SDL_SRCALPHA, StartUp);
		else
			SDL_SetAlpha (UnitsData.ptr_small_beton, SDL_SRCALPHA, 255);

		SDL_BlitSurface (UnitsData.ptr_small_beton, NULL, surface, &tmp);
		SDL_SetAlpha (UnitsData.ptr_small_beton, SDL_SRCALPHA, 255);
	}
}

void cBuilding::render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, int frameNr, int alpha)
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
	if (data.hasPlayerColor) SDL_BlitSurface (owner->getColorSurface(), NULL, GraphicsData.gfx_tmp, NULL);
	else SDL_FillRect (GraphicsData.gfx_tmp, NULL, 0x00FF00FF);

	if (data.hasFrames)
	{
		src.x = frameNr * Round (64.0f * zoomFactor);

		CHECK_SCALING (uiData->img, uiData->img_org, zoomFactor);
		SDL_BlitSurface (uiData->img, &src, GraphicsData.gfx_tmp, NULL);

		src.x = 0;
	}
	else if (data.hasClanLogos)
	{
		CHECK_SCALING (uiData->img, uiData->img_org, zoomFactor);
		src.x = 0;
		src.y = 0;
		src.w = (int) (128 * zoomFactor);
		src.h = (int) (128 * zoomFactor);
		// select clan image
		if (owner->getClan() != -1)
			src.x = (int) ( (owner->getClan() + 1) * 128 * zoomFactor);
		SDL_BlitSurface (uiData->img, &src, GraphicsData.gfx_tmp, NULL);
	}
	else
	{
		CHECK_SCALING (uiData->img, uiData->img_org, zoomFactor);
		SDL_BlitSurface (uiData->img, NULL, GraphicsData.gfx_tmp, NULL);
	}

	// draw the building
	SDL_Rect tmp = dest;

	src.x = 0;
	src.y = 0;

	SDL_SetAlpha (GraphicsData.gfx_tmp, SDL_SRCALPHA, alpha);
	SDL_BlitSurface (GraphicsData.gfx_tmp, &src, surface, &tmp);
}


void cBuilding::render (const cGameGUI* gameGUI, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow, bool drawConcrete)
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
	if ( (this->SubBase && !StartUp) || data.isConnectorGraphic)
	{
		drawConnectors (surface, dest, zoomFactor, drawShadow);
		if (data.isConnectorGraphic) return;
	}

	// draw the shadows
	if (drawShadow)
	{
		SDL_Rect tmp = dest;
		if (StartUp && cSettings::getInstance().isAlphaEffects())
			SDL_SetAlpha (uiData->shw, SDL_SRCALPHA, StartUp / 5);
		else
			SDL_SetAlpha (uiData->shw, SDL_SRCALPHA, 50);

		CHECK_SCALING (uiData->shw, uiData->shw_org, zoomFactor);
		blittAlphaSurface (uiData->shw, NULL, surface, &tmp);
	}

	int frameNr = dir;
	if (data.hasFrames && data.isAnimated && cSettings::getInstance().isAnimations() &&
		isDisabled() == false && gameGUI)
	{
		frameNr = (gameGUI->getAnimationSpeed() % data.hasFrames);
	}

	int alpha = 255;
	if (StartUp && cSettings::getInstance().isAlphaEffects()) alpha = StartUp;
	render_simple (surface, dest, zoomFactor, frameNr, alpha);
}

//--------------------------------------------------------------------------
void cBuilding::updateNeighbours (const cMap* Map)
{
	int iPosOff = Map->getOffset (PosX, PosY);
	if (!data.isBig)
	{
		owner->base.checkNeighbour (iPosOff - Map->getSize(), *this);
		owner->base.checkNeighbour (iPosOff + 1, *this);
		owner->base.checkNeighbour (iPosOff + Map->getSize(), *this);
		owner->base.checkNeighbour (iPosOff - 1, *this);
	}
	else
	{
		owner->base.checkNeighbour (iPosOff - Map->getSize(), *this);
		owner->base.checkNeighbour (iPosOff - Map->getSize() + 1, *this);
		owner->base.checkNeighbour (iPosOff + 2, *this);
		owner->base.checkNeighbour (iPosOff + 2 + Map->getSize(), *this);
		owner->base.checkNeighbour (iPosOff + Map->getSize() * 2, *this);
		owner->base.checkNeighbour (iPosOff + Map->getSize() * 2 + 1, *this);
		owner->base.checkNeighbour (iPosOff - 1, *this);
		owner->base.checkNeighbour (iPosOff - 1 + Map->getSize(), *this);
	}
	CheckNeighbours (Map);
}

//--------------------------------------------------------------------------
/** Checks, if there are neighbours */
//--------------------------------------------------------------------------
void cBuilding::CheckNeighbours (const cMap* Map)
{
#define CHECK_NEIGHBOUR(x, y, m) \
	if (Map->isValidPos (x, y)) \
	{ \
		const cBuilding* b = Map->fields[Map->getOffset (x, y)].getTopBuilding(); \
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
void cBuilding::drawConnectors (SDL_Surface* surface, SDL_Rect dest, float zoomFactor, bool drawShadow)
{
	SDL_Rect src, temp;

	CHECK_SCALING (UnitsData.ptr_connector, UnitsData.ptr_connector_org, zoomFactor);
	CHECK_SCALING (UnitsData.ptr_connector_shw, UnitsData.ptr_connector_shw_org, zoomFactor);

	if (StartUp) SDL_SetAlpha (UnitsData.ptr_connector, SDL_SRCALPHA, StartUp);
	else SDL_SetAlpha (UnitsData.ptr_connector, SDL_SRCALPHA, 255);

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
	if (IsWorking)
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
		if (data.convertsGold + SubBase->GoldNeed > SubBase->getGoldProd() + SubBase->Gold)
		{
			sendChatMessageToClient (server, "Text~Comp~Gold_Insufficient", SERVER_ERROR_MESSAGE, owner->getNr());
			return;
		}
	}

	// needs raw material:
	if (data.needsMetal)
	{
		if (SubBase->MetalNeed + min (MetalPerRound, BuildList[0].metall_remaining) > SubBase->getMetalProd() + SubBase->Metal)
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
		if (data.needsOil + SubBase->OilNeed > SubBase->Oil + SubBase->getMaxOilProd())
		{
			sendChatMessageToClient (server, "Text~Comp~Fuel_Insufficient", SERVER_ERROR_MESSAGE, owner->getNr());
			return;
		}
		else if (data.needsOil + SubBase->OilNeed > SubBase->Oil + SubBase->getOilProd())
		{
			// increase oil production
			int missingOil = data.needsOil + SubBase->OilNeed - (SubBase->Oil + SubBase->getOilProd());

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
	IsWorking = true;

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
				IsWorking = false;

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
void cBuilding::ClientStartWork (cGameGUI& gameGUI)
{
	if (IsWorking)
		return;
	IsWorking = true;
	EffectAlpha = 0;
	if (gameGUI.getSelectedUnit() == this)
	{
		gameGUI.stopFXLoop();
		PlayFX (uiData->Start);
		gameGUI.playStream (*this);
	}
	if (data.canResearch)
		owner->startAResearch (researchArea);
}

//--------------------------------------------------------------------------
/** Stops the building's working in the server thread */
//--------------------------------------------------------------------------
void cBuilding::ServerStopWork (cServer& server, bool override)
{
	if (!IsWorking)
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

	IsWorking = false;

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
void cBuilding::ClientStopWork (cGameGUI& gameGUI)
{
	if (!IsWorking)
		return;
	IsWorking = false;
	if (gameGUI.getSelectedUnit() == this)
	{
		gameGUI.stopFXLoop();
		PlayFX (uiData->Stop);
		gameGUI.playStream (*this);
	}
	if (data.canResearch)
		owner->stopAResearch (researchArea);
}

//------------------------------------------------------------
bool cBuilding::CanTransferTo (int x, int y, cMapField* OverUnitField) const
{
	if (OverUnitField->getVehicle())
	{
		const cVehicle* v = OverUnitField->getVehicle();

		if (v->owner != this->owner)
			return false;

		if (v->data.storeResType != data.storeResType)
			return false;

		if (v->IsBuilding || v->IsClearing)
			return false;

		for (size_t i = 0; i != SubBase->buildings.size(); ++i)
		{
			const cBuilding* b = SubBase->buildings[i];

			if (b->isNextTo (x, y)) return true;
		}

		return false;
	}
	else if (OverUnitField->getTopBuilding())
	{
		const cBuilding* b = OverUnitField->getTopBuilding();

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
/** draws the exit points for a vehicle of the given type: */
//--------------------------------------------------------------------------
void cBuilding::DrawExitPoints (const sUnitData& vehicleData, cGameGUI& gameGUI)
{
	int const spx = gameGUI.getScreenPosX (*this);
	int const spy = gameGUI.getScreenPosY (*this);
	cMap* map = gameGUI.getClient()->getMap();
	const int tilesize = gameGUI.getTileSize();
	T_2<int> offsets[12] = {T_2<int> (-1, -1), T_2<int> (0, -1), T_2<int> (1, -1), T_2<int> (2, -1),
							T_2<int> (-1,  0),                                   T_2<int> (2, 0),
							T_2<int> (-1,  1),                                   T_2<int> (2, 1),
							T_2<int> (-1,  2), T_2<int> (0,  2), T_2<int> (1,  2), T_2<int> (2, 2)
						   };

	for (int i = 0; i != 12; ++i)
	{
		if (canExitTo (PosX + offsets[i].x, PosY + offsets[i].y, *map, vehicleData))
			gameGUI.drawExitPoint (spx + offsets[i].x * tilesize, spy + offsets[i].y * tilesize);
	}
}

//--------------------------------------------------------------------------
bool cBuilding::canExitTo (const int x, const int y, const cMap& map, const sUnitData& vehicleData) const
{
	if (!map.possiblePlaceVehicle (vehicleData, x, y, owner)) return false;
	if (!isNextTo (x, y)) return false;

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

	if (Vehicle->Loaded) return false;

	if (data.storageUnitsCur == data.storageUnitsMax) return false;

	if (checkPosition && !isNextTo (Vehicle->PosX, Vehicle->PosY)) return false;

	if (!Contains (data.storeUnitsTypes, Vehicle->data.isStorageType)) return false;

	if (Vehicle->ClientMoveJob && (Vehicle->moving || Vehicle->attacking || Vehicle->MoveJobActive)) return false;

	if (Vehicle->owner != owner || Vehicle->IsBuilding || Vehicle->IsClearing) return false;

	if (Vehicle->isBeeingAttacked) return false;

	return true;
}

//--------------------------------------------------------------------------
/** loads a vehicle: */
//--------------------------------------------------------------------------
void cBuilding::storeVehicle (cVehicle* Vehicle, cMap* Map)
{
	Map->deleteVehicle (*Vehicle);
	if (Vehicle->sentryActive)
	{
		Vehicle->owner->deleteSentry (*Vehicle);
	}

	Vehicle->Loaded = true;

	storedUnits.push_back (Vehicle);
	data.storageUnitsCur++;

	owner->doScan();
}

//-----------------------------------------------------------------------
// Unloads a vehicle
void cBuilding::exitVehicleTo (cVehicle* Vehicle, int offset, cMap* Map)
{
	Remove (storedUnits, Vehicle);

	data.storageUnitsCur--;

	Map->addVehicle (*Vehicle, offset);

	Vehicle->PosX = offset % Map->getSize();
	Vehicle->PosY = offset / Map->getSize();
	Vehicle->Loaded = false;

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
			SDL_FillRect (sf, &mark, 0xFC0000);
		}

		SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &src, sf, &dest);

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
void cBuilding::CalcTurboBuild (int* iTurboBuildRounds, int* iTurboBuildCosts, int iVehicleCosts, int iRemainingMetal)
{
	// first calc costs for a new Vehical

	// 1x
	iTurboBuildCosts[0] = iVehicleCosts;

	// 2x
	int a = iTurboBuildCosts[0];
	iTurboBuildCosts[1] = iTurboBuildCosts[0];

	while (a >= 2 * data.needsMetal)
	{
		iTurboBuildCosts[1] += 2 * data.needsMetal;
		a -= 2 * data.needsMetal;
	}

	// 4x
	iTurboBuildCosts[2] = iTurboBuildCosts[1];
	a = iTurboBuildCosts[1];

	while (a >= 15)
	{
		iTurboBuildCosts[2] += (12 * data.needsMetal - min (a, 8 * data.needsMetal));
		a -= 8 * data.needsMetal;
	}

	// now this is a litle bit tricky ...
	// trying to calculate a plausible value,
	// if we are changing the speed of an already started build-job
	if (iRemainingMetal >= 0)
	{
		float WorkedRounds;

		switch (BuildSpeed)  // BuildSpeed here is the previous build speed
		{
			case 0:
				WorkedRounds = (iTurboBuildCosts[0] - iRemainingMetal) / (1.f * data.needsMetal);
				iTurboBuildCosts[0] -= (int) (1     *  1 * data.needsMetal * WorkedRounds);
				iTurboBuildCosts[1] -= (int) (0.5f  *  4 * data.needsMetal * WorkedRounds);
				iTurboBuildCosts[2] -= (int) (0.25f * 12 * data.needsMetal * WorkedRounds);
				break;

			case 1:
				WorkedRounds = (iTurboBuildCosts[1] - iRemainingMetal) / (float) (4 * data.needsMetal);
				iTurboBuildCosts[0] -= (int) (2    *  1 * data.needsMetal * WorkedRounds);
				iTurboBuildCosts[1] -= (int) (1    *  4 * data.needsMetal * WorkedRounds);
				iTurboBuildCosts[2] -= (int) (0.5f * 12 * data.needsMetal * WorkedRounds);
				break;

			case 2:
				WorkedRounds = (iTurboBuildCosts[2] - iRemainingMetal) / (float) (12 * data.needsMetal);
				iTurboBuildCosts[0] -= (int) (4 *  1 * data.needsMetal * WorkedRounds);
				iTurboBuildCosts[1] -= (int) (2 *  4 * data.needsMetal * WorkedRounds);
				iTurboBuildCosts[2] -= (int) (1 * 12 * data.needsMetal * WorkedRounds);
				break;
		}
	}

	// calc needed Rounds
	iTurboBuildRounds[0] = (int) ceilf (iTurboBuildCosts[0] / (1.f * data.needsMetal));

	if (data.maxBuildFactor > 1)
	{
		iTurboBuildRounds[1] = (int) ceilf (iTurboBuildCosts[1] / (4.f * data.needsMetal));
		iTurboBuildRounds[2] = (int) ceilf (iTurboBuildCosts[2] / (12.f * data.needsMetal));
	}
	else
	{
		iTurboBuildRounds[1] = 0;
		iTurboBuildRounds[2] = 0;
	}
}

//--------------------------------------------------------------------------
void cBuilding::Select (cGameGUI& gameGUI)
{
	if (!owner) return;

	// load video
	if (gameGUI.getFLC() != NULL)
	{
		FLI_Close (gameGUI.getFLC());
		gameGUI.setFLC (NULL);
	}
	gameGUI.setVideoSurface (uiData->video);

	// play sound:
	if (owner->researchFinished && data.canResearch && isDisabled() == false)
		PlayVoice (VoiceData.VOIResearchComplete);
	else if (factoryHasJustFinishedBuilding() && isDisabled() == false)
	{
		PlayRandomVoice (VoiceData.VOIBuildDone);
	}
	else if (!IsWorking)
		PlayFX (SoundData.SNDHudButton);

	// display the details:
	gameGUI.setUnitDetailsData (this);

	// some sounds for special moments
	// (disabled as long as you are not the owner)
	if (gameGUI.getClient()->getActivePlayer() == owner)
	{
		// running out of ammo
		if (data.canAttack)
		{
			if (data.ammoCur <= data.ammoMax / 4 && data.ammoCur != 0)
			{
				// red ammo-status but still ammo left
				PlayRandomVoice (VoiceData.VOIAmmoLow);
			}
			else if (data.ammoCur == 0)
			{
				// no ammo left
				PlayRandomVoice (VoiceData.VOIAmmoEmpty);
			}
		}
		// damaged
		if (data.hitpointsCur <= data.hitpointsMax / 2 && data.hitpointsCur > data.hitpointsMax / 4)
		{
			// Status yellow:
			PlayRandomVoice (VoiceData.VOIStatusYellow);
		}
		else if (data.hitpointsCur <= data.hitpointsMax / 4)
		{
			// Status red:
			PlayRandomVoice (VoiceData.VOIStatusRed);
		}
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
void cBuilding::executeBuildCommand (cGameGUI& gameGUI)
{
	cVehiclesBuildMenu buildMenu (gameGUI, owner, this);
	buildMenu.show (gameGUI.getClient());
}

//-----------------------------------------------------------------------------
void cBuilding::executeMineManagerCommand (cClient& client)
{
	cMineManagerMenu mineManager (client, this);
	mineManager.show (&client);
}

//-----------------------------------------------------------------------------
void cBuilding::executeStopCommand (const cClient& client)
{
	sendWantStopWork (client, *this);
}

//-----------------------------------------------------------------------------
void cBuilding::executeActivateStoredVehiclesCommand (cClient& client)
{
	cStorageMenu storageMenu (client, storedUnits, *this);
	storageMenu.show (&client);
}

//-----------------------------------------------------------------------------
void cBuilding::executeUpdateBuildingCommmand (const cClient& client, bool updateAllOfSameType)
{
	sendUpgradeBuilding (client, *this, updateAllOfSameType);
}

//-----------------------------------------------------------------------------
void cBuilding::executeSelfDestroyCommand (cClient& client)
{
	cDestructMenu destructMenu;
	if (destructMenu.show (&client) == 0)
		sendWantSelfDestroy (client, *this);
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
	return (data.version != upgraded.version && SubBase && SubBase->Metal >= 2);
}
