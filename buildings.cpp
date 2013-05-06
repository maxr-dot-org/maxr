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
#include "buildings.h"
#include "main.h"
#include "unifonts.h"
#include "mouse.h"
#include "files.h"
#include "pcx.h"
#include "events.h"
#include "serverevents.h"
#include "clientevents.h"
#include "client.h"
#include "server.h"
#include "netmessage.h"
#include "upgradecalculator.h"
#include "menus.h"
#include "dialog.h"
#include "settings.h"
#include "hud.h"
#include "video.h"
#include "vehicles.h"
#include "player.h"
#include "attackJobs.h"
#include "fxeffects.h"

using namespace std;

//--------------------------------------------------------------------------
// cBuilding Implementation
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
cBuilding::cBuilding (const sBuilding* b, cPlayer* Owner, unsigned int ID)
	: cUnit (cUnit::kUTBuilding,
			 ( (Owner != 0 && b != 0) ? & (Owner->BuildingData[b->nr]) : 0),
			 Owner,
			 ID)
{
	RubbleTyp = 0;
	RubbleValue = 0;
	EffectAlpha = 0;
	EffectInc = true;
	StartUp = 0;
	IsWorking = false;
	researchArea = cResearch::kAttackResearch;
	IsLocked = false;
	typ = b;
	points = 0;
	lastShots = 0;

	if (Owner == NULL || b == NULL)
	{
		BuildList = NULL;
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
	BuildList = NULL;

	if (!data.canBuild.empty())
		BuildList = new cList<sBuildList*>;

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
	if (BuildList)
	{
		for (size_t i = 0; i != BuildList->Size(); ++i)
		{
			delete (*BuildList) [i];
		}
		delete BuildList;
	}

	if (IsLocked)
	{
		cList<cPlayer*>& playerList = *Client->getPlayerList();
		for (unsigned int i = 0; i < playerList.Size(); i++)
		{
			cPlayer* p = playerList[i];
			p->DeleteLock (this);
		}
	}
}

//----------------------------------------------------
/** Returns a string with the current state */
//----------------------------------------------------
string cBuilding::getStatusStr(const cGameGUI& gameGUI) const
{
	if (turnsDisabled > 0)
	{
		string sText;
		sText = lngPack.i18n ("Text~Comp~Disabled") + " (";
		sText += iToStr (turnsDisabled) + ")";
		return sText;
	}
	if (IsWorking)
	{
		const cPlayer* activePlayer = gameGUI.getClient()->getActivePlayer();
		// Factory:
		if (!data.canBuild.empty() && BuildList && BuildList->Size() && owner == activePlayer)
		{
			sBuildList* buildListItem = (*BuildList) [0];

			if (buildListItem->metall_remaining > 0)
			{
				string sText;
				int iRound;

				iRound = (int) ceil (buildListItem->metall_remaining / (double) MetalPerRound);
				sText = lngPack.i18n ("Text~Comp~Producing") + ": ";
				sText += buildListItem->type.getVehicle()->data.name + " (";
				sText += iToStr (iRound) + ")";

				if (font->getTextWide (sText, FONT_LATIN_SMALL_WHITE) > 126)
				{
					sText = lngPack.i18n ("Text~Comp~Producing") + ":\n";
					sText += buildListItem->type.getVehicle()->data.name + " (";
					sText += iToStr (iRound) + ")";
				}

				return sText;
			}
			else
			{
				return lngPack.i18n ("Text~Comp~Producing_Fin");
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
	if (turnsDisabled > 0)
	{
		if (data.ammoCur >= data.shotsMax)
			lastShots = data.shotsMax;
		else
			lastShots = data.ammoCur;
		return 1;
	}

	if (data.shotsCur < data.shotsMax)
	{
		if (data.ammoCur >= data.shotsMax)
			data.shotsCur = data.shotsMax;
		else
			data.shotsCur = data.ammoCur;
		return 1;
	}
	return 0;
}

//--------------------------------------------------------------------------
void cBuilding::draw (SDL_Rect* screenPos, cGameGUI& gameGUI)
{
	SDL_Rect dest, tmp;
	float factor = (float) gameGUI.getTileSize() / (float) 64.0;

	// draw the damage effects
	if (gameGUI.timer100ms && data.hasDamageEffect && data.hitpointsCur < data.hitpointsMax && cSettings::getInstance().isDamageEffects() && (owner == gameGUI.getClient()->getActivePlayer() || gameGUI.getClient()->getActivePlayer()->ScanMap[PosX + PosY * gameGUI.getClient()->getMap()->size]))
	{
		int intense = (int) (200 - 200 * ( (float) data.hitpointsCur / data.hitpointsMax));
		gameGUI.addFx (new cFxDarkSmoke (PosX * 64 + DamageFXPointX, PosY * 64 + DamageFXPointY, intense, gameGUI.getWindDir ()));

		if (data.isBig && intense > 50)
		{
			intense -= 50;
			gameGUI.addFx (new cFxDarkSmoke (PosX * 64 + DamageFXPointX2, PosY * 64 + DamageFXPointY2, intense, gameGUI.getWindDir ()));
		}
	}

	dest.x = dest.y = 0;
	bool bDraw = false;
	SDL_Surface* drawingSurface = gameGUI.getDCache()->getCachedImage (this);
	if (drawingSurface == NULL)
	{
		//no cached image found. building needs to be redrawn.
		bDraw = true;
		drawingSurface = gameGUI.getDCache()->createNewEntry (this);
	}

	if (drawingSurface == NULL)
	{
		//image will not be cached. So blitt directly to the screen buffer.
		dest = *screenPos;
		drawingSurface = buffer;
	}

	if (bDraw)
	{
		render (drawingSurface, dest, (float) gameGUI.getTileSize() / (float) 64.0, cSettings::getInstance().isShadows(), true);
	}

	//now check, whether the image has to be blitted to screen buffer
	if (drawingSurface != buffer)
	{
		dest = *screenPos;
		SDL_BlitSurface (drawingSurface, NULL, buffer, &dest);

		//all following graphic operations are drawn directly to buffer
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
		SDL_SetAlpha (typ->eff, SDL_SRCALPHA, EffectAlpha);

		CHECK_SCALING (typ->eff, typ->eff_org, factor);
		SDL_BlitSurface (typ->eff, NULL, buffer, &tmp);

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
	if ( ( (BuildList && BuildList->Size() && !IsWorking && (*BuildList) [0]->metall_remaining <= 0) || (data.canResearch && owner->researchFinished)) && owner == gameGUI.getClient()->getActivePlayer())
	{
		SDL_Rect d, t;
		int max, nr;
		nr = 0xFF00 - ( (ANIMATION_SPEED % 0x8) * 0x1000);
		max = (int) (gameGUI.getTileSize() - 2) * 2;
		d.x = dest.x + 2;
		d.y = dest.y + 2;
		d.w = max;
		d.h = 1;
		t = d;
		SDL_FillRect (buffer, &d, nr);
		d = t;
		d.y += max - 1;
		t = d;
		SDL_FillRect (buffer, &d, nr);
		d = t;
		d.y = dest.y + 2;
		d.w = 1;
		d.h = max;
		t = d;
		SDL_FillRect (buffer, &d, nr);
		d = t;
		d.x += max - 1;
		SDL_FillRect (buffer, &d, nr);
	}

	// draw a colored frame if necessary
	if (gameGUI.colorChecked())
	{
		SDL_Rect d, t;
		int nr = * ( (unsigned int*) (owner->color->pixels));
		int max = data.isBig ? ( (int) (gameGUI.getTileSize()) - 1) * 2 : (int) (gameGUI.getTileSize()) - 1;

		d.x = dest.x + 1;
		d.y = dest.y + 1;
		d.w = max;
		d.h = 1;
		t = d;
		SDL_FillRect (buffer, &d, nr);
		d = t;
		d.y += max - 1;
		t = d;
		SDL_FillRect (buffer, &d, nr);
		d = t;
		d.y = dest.y + 1;
		d.w = 1;
		d.h = max;
		t = d;
		SDL_FillRect (buffer, &d, nr);
		d = t;
		d.x += max - 1;
		SDL_FillRect (buffer, &d, nr);
	}

	// draw a colored frame if necessary
	if (gameGUI.getSelBuilding() == this)
	{
		SDL_Rect d, t;
		int max = data.isBig ? (int) (gameGUI.getTileSize()) * 2 : (int) (gameGUI.getTileSize());
		int len = max / 4;

		d.x = dest.x + 1;
		d.y = dest.y + 1;
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
		d.x = dest.x + 1;
		t = d;
		SDL_FillRect (buffer, &d, gameGUI.getBlinkColor());
		d = t;
		d.y = dest.y + 1;
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
		d.x = dest.x + 1;
		SDL_FillRect (buffer, &d, gameGUI.getBlinkColor());
	}

	//draw health bar
	if (gameGUI.hitsChecked())
		drawHealthBar (gameGUI, *screenPos);

	//draw ammo bar
	if (gameGUI.ammoChecked() && data.canAttack && data.ammoMax > 0)
		drawMunBar (gameGUI, *screenPos);

	//draw status
	if (gameGUI.statusChecked())
		drawStatus (gameGUI, *screenPos);

	//attack job debug output
	if (gameGUI.getAJobDebugStatus())
	{
		cBuilding* serverBuilding = NULL;
		if (Server) serverBuilding = Server->Map->fields[PosX + PosY * Server->Map->size].getBuildings();
		if (isBeeingAttacked) font->showText (dest.x + 1, dest.y + 1, "C: attacked", FONT_LATIN_SMALL_WHITE);
		if (serverBuilding && serverBuilding->isBeeingAttacked) font->showText (dest.x + 1, dest.y + 9, "S: attacked", FONT_LATIN_SMALL_YELLOW);
		if (attacking) font->showText (dest.x + 1, dest.y + 17, "C: attacking", FONT_LATIN_SMALL_WHITE);
		if (serverBuilding && serverBuilding->attacking) font->showText (dest.x + 1, dest.y + 25, "S: attacking", FONT_LATIN_SMALL_YELLOW);
	}
}

void cBuilding::render (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow, bool drawConcrete)
{
	//Note: when changing something in this function, make sure to update the caching rules!
	SDL_Rect src, tmp;
	src.x = 0;
	src.y = 0;

	// check, if it is dirt:
	if (!owner)
	{
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
		tmp = dest;
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

		return;
	}

	// read the size:
	if (data.hasFrames)
	{
		src.w = Round (64.0 * zoomFactor);
		src.h = Round (64.0 * zoomFactor);
	}
	else
	{
		src.w = (int) (typ->img_org->w * zoomFactor);
		src.h = (int) (typ->img_org->h * zoomFactor);
	}

	// draw the concrete
	tmp = dest;
	if (data.hasBetonUnderground && drawConcrete)
	{
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

	tmp = dest;

	// draw the connector slots:
	if ( (this->SubBase && !StartUp) || data.isConnectorGraphic)
	{
		drawConnectors (surface, dest, zoomFactor, drawShadow);
		if (data.isConnectorGraphic) return;
	}

	// draw the shadows
	if (drawShadow)
	{
		if (StartUp && cSettings::getInstance().isAlphaEffects())
			SDL_SetAlpha (typ->shw, SDL_SRCALPHA, StartUp / 5);
		else
			SDL_SetAlpha (typ->shw, SDL_SRCALPHA, 50);

		CHECK_SCALING (typ->shw, typ->shw_org, zoomFactor);
		blittAlphaSurface (typ->shw, NULL, surface, &tmp);
	}

	// blit the players color and building graphic
	if (data.hasPlayerColor) SDL_BlitSurface (owner->color, NULL, GraphicsData.gfx_tmp, NULL);
	else SDL_FillRect (GraphicsData.gfx_tmp, NULL, 0xFF00FF);

	if (data.hasFrames)
	{
		if (data.isAnimated && cSettings::getInstance().isAnimations() && turnsDisabled == 0 && Client)
		{
			src.x = (ANIMATION_SPEED % data.hasFrames) * Round (64.0 * zoomFactor);
		}
		else
		{
			src.x = dir * Round (64.0 * zoomFactor);
		}

		CHECK_SCALING (typ->img, typ->img_org, zoomFactor);
		SDL_BlitSurface (typ->img, &src, GraphicsData.gfx_tmp, NULL);

		src.x = 0;
	}
	else if (data.hasClanLogos)
	{
		CHECK_SCALING (typ->img, typ->img_org, zoomFactor);
		src.x = 0;
		src.y = 0;
		src.w = (int) (128 * zoomFactor);
		src.h = (int) (128 * zoomFactor);
		//select clan image
		if (owner->getClan() != -1)
			src.x = (int) ( (owner->getClan() + 1) * 128 * zoomFactor);
		SDL_BlitSurface (typ->img, &src, GraphicsData.gfx_tmp, NULL);

	}
	else
	{
		CHECK_SCALING (typ->img, typ->img_org, zoomFactor);
		SDL_BlitSurface (typ->img, NULL, GraphicsData.gfx_tmp, NULL);
	}

	// draw the building
	tmp = dest;

	src.x = 0;
	src.y = 0;

	if (StartUp && cSettings::getInstance().isAlphaEffects()) SDL_SetAlpha (GraphicsData.gfx_tmp, SDL_SRCALPHA, StartUp);
	else SDL_SetAlpha (GraphicsData.gfx_tmp, SDL_SRCALPHA, 255);

	SDL_BlitSurface (GraphicsData.gfx_tmp, &src, surface, &tmp);
}

//--------------------------------------------------------------------------
void cBuilding::updateNeighbours (const cMap* Map)
{
	int iPosOff = PosX + PosY * Map->size;
	if (!data.isBig)
	{
		owner->base.checkNeighbour (iPosOff - Map->size, this);
		owner->base.checkNeighbour (iPosOff + 1, this);
		owner->base.checkNeighbour (iPosOff + Map->size, this);
		owner->base.checkNeighbour (iPosOff - 1, this);
	}
	else
	{
		owner->base.checkNeighbour (iPosOff - Map->size, this);
		owner->base.checkNeighbour (iPosOff - Map->size + 1, this);
		owner->base.checkNeighbour (iPosOff + 2, this);
		owner->base.checkNeighbour (iPosOff + 2 + Map->size, this);
		owner->base.checkNeighbour (iPosOff + Map->size * 2, this);
		owner->base.checkNeighbour (iPosOff + Map->size * 2 + 1, this);
		owner->base.checkNeighbour (iPosOff - 1, this);
		owner->base.checkNeighbour (iPosOff - 1 + Map->size, this);
	}
	CheckNeighbours (Map);
}

//--------------------------------------------------------------------------
/** Checks, if there are neighbours */
//--------------------------------------------------------------------------
void cBuilding::CheckNeighbours (const cMap* Map)
{
#define CHECK_NEIGHBOUR(x,y,m)								\
	if(x >= 0 && x < Map->size && y >= 0 && y < Map->size ) \
	{														\
		const cBuilding* b = Map->fields[(x) + (y) * Map->size].getTopBuilding();		\
		if ( b && b->owner == owner && b->data.connectsToBase )			\
		{m=true;}else{m=false;}							\
	}														\

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
			//blit shadow
			temp = dest;
			if (drawShadow) blittAlphaSurface (UnitsData.ptr_connector_shw, &src, surface, &temp);
			//blit the image
			temp = dest;
			SDL_BlitSurface (UnitsData.ptr_connector, &src, surface, &temp);
		}

	}
	else
	{
		//make connector stubs of big buildings.
		//upper left field
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

		//upper right field
		src.x = 0;
		dest.x += Round (64.0 * zoomFactor);
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

		//lower right field
		src.x = 0;
		dest.y += Round (64.0 * zoomFactor);
		if (BaseBE &&  BaseBS) src.x = 9;
		else if (BaseBE && !BaseBS) src.x = 2;
		else if (!BaseBE &&  BaseBS) src.x = 3;
		src.x *= src.h;

		if (src.x != 0)
		{
			temp = dest;
			if (drawShadow) blittAlphaSurface (UnitsData.ptr_connector_shw, &src, surface, &temp);
			temp = dest;
			SDL_BlitSurface (UnitsData.ptr_connector, &src, surface, &temp);
		}

		//lower left field
		src.x = 0;
		dest.x -= Round (64.0 * zoomFactor);
		if (BaseS &&  BaseBW) src.x = 10;
		else if (BaseS && !BaseBW) src.x =  3;
		else if (!BaseS &&  BaseBW) src.x =  4;
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
void cBuilding::ServerStartWork()
{
	if (IsWorking)
	{
		sendDoStartWork (this);
		return;
	}

	//-- first check all requirements

	if (turnsDisabled > 0)
	{
		sendChatMessageToClient ("Text~Comp~Building_Disabled", SERVER_ERROR_MESSAGE, owner->Nr);
		return;
	}

	// needs human workers:
	if (data.needsHumans)
		if (SubBase->HumanNeed + data.needsHumans > SubBase->HumanProd)
		{
			sendChatMessageToClient ("Text~Comp~Team_Insufficient", SERVER_ERROR_MESSAGE, owner->Nr);
			return;
		}

	// needs gold:
	if (data.convertsGold)
	{
		if (data.convertsGold + SubBase->GoldNeed > SubBase->getGoldProd() + SubBase->Gold)
		{
			sendChatMessageToClient ("Text~Comp~Gold_Insufficient", SERVER_ERROR_MESSAGE, owner->Nr);
			return;
		}
	}

	// needs raw material:
	if (data.needsMetal)
	{
		if (SubBase->MetalNeed + min (MetalPerRound, (*BuildList) [0]->metall_remaining) > SubBase->getMetalProd() + SubBase->Metal)
		{
			sendChatMessageToClient ("Text~Comp~Metal_Insufficient", SERVER_ERROR_MESSAGE, owner->Nr);
			return;
		}
	}

	// needs oil:
	if (data.needsOil)
	{
		// check if there is enough Oil for the generators (current prodiction + reserves)
		if (data.needsOil + SubBase->OilNeed > SubBase->Oil + SubBase->getMaxOilProd())
		{
			sendChatMessageToClient ("Text~Comp~Fuel_Insufficient", SERVER_ERROR_MESSAGE, owner->Nr);
			return;
		}
		else if (data.needsOil + SubBase->OilNeed > SubBase->Oil + SubBase->getOilProd())
		{
			//increase oil production
			int missingOil = data.needsOil + SubBase->OilNeed - (SubBase->Oil + SubBase->getOilProd());

			int metal = SubBase->getMetalProd();
			int gold = SubBase->getGoldProd();

			SubBase->setMetalProd (0);	//temporay decrease metal and gold production
			SubBase->setGoldProd (0);

			SubBase->changeOilProd (missingOil);

			SubBase->setGoldProd (gold);
			SubBase->setMetalProd (metal);

			sendChatMessageToClient ("Text~Comp~Adjustments_Fuel_Increased", SERVER_INFO_MESSAGE, owner->Nr, iToStr (missingOil));
			if (SubBase->getMetalProd() < metal)
				sendChatMessageToClient ("Text~Comp~Adjustments_Metal_Decreased", SERVER_INFO_MESSAGE, owner->Nr, iToStr (metal - SubBase->getMetalProd()));
			if (SubBase->getGoldProd() < gold)
				sendChatMessageToClient ("Text~Comp~Adjustments_Gold_Decreased", SERVER_INFO_MESSAGE, owner->Nr, iToStr (gold - SubBase->getGoldProd()));
		}
	}

	// IsWorking is set to true before checking the energy production. So if an energy generator has to be started,
	// it can use the fuel production of this building (when this building is a mine).
	IsWorking = true;

	//set mine values. This has to be undone, if the energy is insufficient
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
			//try to increase energy production
			if (!SubBase->increaseEnergyProd (data.needsEnergy + SubBase->EnergyNeed - SubBase->EnergyProd))
			{
				IsWorking = false;

				//reset mine values
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

				sendChatMessageToClient ("Text~Comp~Energy_Insufficient", SERVER_ERROR_MESSAGE, owner->Nr);
				return;
			}
			sendChatMessageToClient ("Text~Comp~Energy_ToLow", SERVER_INFO_MESSAGE, owner->Nr);
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
		SubBase->MetalNeed += min (MetalPerRound, (*BuildList) [0]->metall_remaining);

	// gold consumer:
	SubBase->GoldNeed += data.convertsGold;

	// research building
	if (data.canResearch)
	{
		owner->ResearchCount++;
		owner->researchCentersWorkingOnArea[researchArea]++;
	}

	if (data.canScore)
	{
		sendNumEcos (owner);
	}

	sendSubbaseValues (SubBase, owner->Nr);
	sendDoStartWork (this);
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
	if (gameGUI.getSelBuilding() == this)
	{
		StopFXLoop (gameGUI.iObjectStream);
		PlayFX (typ->Start);
		gameGUI.iObjectStream = playStream();
	}
	if (data.canResearch)
		owner->startAResearch (researchArea);
}

//--------------------------------------------------------------------------
/** Stops the building's working in the server thread */
//--------------------------------------------------------------------------
void cBuilding::ServerStopWork (bool override)
{
	if (!IsWorking)
	{
		sendDoStopWork (this);
		return;
	}

	// energy generators
	if (data.produceEnergy)
	{
		if (SubBase->EnergyNeed > SubBase->EnergyProd - data.produceEnergy && !override)
		{
			sendChatMessageToClient ("Text~Comp~Energy_IsNeeded", SERVER_ERROR_MESSAGE, owner->Nr);
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
		SubBase->MetalNeed -= min (MetalPerRound, (*BuildList) [0]->metall_remaining);

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
		owner->ResearchCount--;
		owner->researchCentersWorkingOnArea[researchArea]--;
	}

	if (data.canScore)
	{
		sendNumEcos (owner);
	}

	sendSubbaseValues (SubBase, owner->Nr);
	sendDoStopWork (this);
}

//------------------------------------------------------------
/** stops the building in the client thread */
//------------------------------------------------------------
void cBuilding::ClientStopWork (cGameGUI& gameGUI)
{
	if (!IsWorking)
		return;
	IsWorking = false;
	if (gameGUI.getSelBuilding() == this)
	{
		StopFXLoop (gameGUI.iObjectStream);
		PlayFX (typ->Stop);
		gameGUI.iObjectStream = playStream();
	}
	if (data.canResearch)
		owner->stopAResearch (researchArea);
}

//------------------------------------------------------------
bool cBuilding::CanTransferTo (cMapField* OverUnitField)
{
	int x = mouse->getKachelX();
	int y = mouse->getKachelY();

	if (OverUnitField->getVehicles())
	{
		const cVehicle* v = OverUnitField->getVehicles();

		if (v->owner != this->owner)
			return false;

		if (v->data.storeResType != data.storeResType)
			return false;

		if (v->IsBuilding || v->IsClearing)
			return false;

		for (unsigned int i = 0; i < SubBase->buildings.Size(); i++)
		{
			const cBuilding* b = SubBase->buildings[i];

			if (b->data.isBig)
			{
				if (x < b->PosX - 1 || x > b->PosX + 2 || y < b->PosY - 1 || y > b->PosY + 2)
					continue;
			}
			else
			{
				if (x < b->PosX - 1 || x > b->PosX + 1 || y < b->PosY - 1 || y > b->PosY + 1)
					continue;
			}

			return true;
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
void cBuilding::DrawExitPoints (const sVehicle* typ, cGameGUI& gameGUI)
{
	int const spx = getScreenPosX();
	int const spy = getScreenPosY();
	cMap* map = gameGUI.getClient()->getMap();
	const int tilesize = gameGUI.getTileSize();
	T_2<int> offsets[12] = {T_2<int> (-1, -1), T_2<int> (0, -1), T_2<int> (1, -1), T_2<int> (2, -1),
							T_2<int> (-1,  0),                                   T_2<int> (2, 0),
							T_2<int> (-1,  1),                                   T_2<int> (2, 1),
							T_2<int> (-1,  2), T_2<int> (0,  2), T_2<int> (1,  2), T_2<int> (2, 2)
						   };

	for (int i = 0; i != 12; ++i)
	{
		if (canExitTo (PosX + offsets[i].x, PosY + offsets[i].y, map, typ))
			gameGUI.drawExitPoint (spx + offsets[i].x * tilesize, spy + offsets[i].y * tilesize);
	}
}

//--------------------------------------------------------------------------
bool cBuilding::canExitTo (const int x, const int y, const cMap* map, const sVehicle* typ) const
{
	if (!map->possiblePlaceVehicle (typ->data, x, y, owner)) return false;
	if (!isNextTo (x, y)) return false;

	return true;
}

//--------------------------------------------------------------------------
bool cBuilding::canLoad (int x, int y, const cMap* Map, bool checkPosition)
{
	if (x < 0 || x >= Map->size || y < 0 || y >= Map->size) return false;
	int offset = x + y * Map->size;

	if (canLoad (Map->fields[offset].getPlanes(), checkPosition)) return true;
	else return canLoad (Map->fields[offset].getVehicles(), checkPosition);

	return false;
}

//--------------------------------------------------------------------------
/** returns, if the vehicle can be loaded from its position: */
//--------------------------------------------------------------------------
bool cBuilding::canLoad (cVehicle* Vehicle, bool checkPosition)
{
	if (!Vehicle) return false;

	if (Vehicle->Loaded) return false;

	if (data.storageUnitsCur == data.storageUnitsMax) return false;

	if (checkPosition && !isNextTo (Vehicle->PosX, Vehicle->PosY)) return false;

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

//--------------------------------------------------------------------------
/** loads a vehicle: */
//--------------------------------------------------------------------------
void cBuilding::storeVehicle (cVehicle* Vehicle, cMap* Map)
{
	Map->deleteVehicle (Vehicle);
	if (Vehicle->sentryActive)
	{
		Vehicle->owner->deleteSentry (Vehicle);
	}

	Vehicle->Loaded = true;

	storedUnits.Add (Vehicle);
	data.storageUnitsCur++;

	owner->DoScan();
}

//-----------------------------------------------------------------------
// Unloads a vehicle
void cBuilding::exitVehicleTo (cVehicle* Vehicle, int offset, cMap* Map)
{
	storedUnits.Remove (Vehicle);

	data.storageUnitsCur--;

	Map->addVehicle (Vehicle, offset);

	Vehicle->PosX = offset % Map->size;
	Vehicle->PosY = offset / Map->size;
	Vehicle->Loaded = false;

	owner->DoScan();
}

//-------------------------------------------------------------------------------
// Draws big symbols for the info menu:
//-------------------------------------------------------------------------------
void cBuilding::DrawSymbolBig (eSymbolsBig sym, int x, int y, int maxx, int value, int orgvalue, SDL_Surface* sf)
{
	SDL_Rect src, dest;
	int i, offx;

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

	offx = src.w;

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

	dest.x = x;

	dest.y = y;

	for (i = 0; i < value; i++)
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
void cBuilding::CheckRessourceProd(const cServer& server)
{
	int pos = PosX + PosY * server.Map->size;

	MaxMetalProd = 0;
	MaxGoldProd = 0;
	MaxOilProd = 0;

	if (server.Map->Resources[pos].typ == RES_METAL)
	{
		MaxMetalProd += server.Map->Resources[pos].value;
	}
	else if (server.Map->Resources[pos].typ == RES_OIL)
	{
		MaxOilProd += server.Map->Resources[pos].value;
	}
	else if (server.Map->Resources[pos].typ == RES_GOLD)
	{
		MaxGoldProd += server.Map->Resources[pos].value;
	}

	pos++;

	if (server.Map->Resources[pos].typ == RES_METAL)
	{
		MaxMetalProd += server.Map->Resources[pos].value;
	}
	else if (server.Map->Resources[pos].typ == RES_OIL)
	{
		MaxOilProd += server.Map->Resources[pos].value;
	}
	else if (server.Map->Resources[pos].typ == RES_GOLD)
	{
		MaxGoldProd += server.Map->Resources[pos].value;
	}

	pos += server.Map->size;

	if (server.Map->Resources[pos].typ == RES_METAL)
	{
		MaxMetalProd += server.Map->Resources[pos].value;
	}
	else if (server.Map->Resources[pos].typ == RES_OIL)
	{
		MaxOilProd += server.Map->Resources[pos].value;
	}
	else if (server.Map->Resources[pos].typ == RES_GOLD)
	{
		MaxGoldProd += server.Map->Resources[pos].value;
	}

	pos--;

	if (server.Map->Resources[pos].typ == RES_METAL)
	{
		MaxMetalProd += server.Map->Resources[pos].value;
	}
	else if (server.Map->Resources[pos].typ == RES_OIL)
	{
		MaxOilProd += server.Map->Resources[pos].value;
	}
	else if (server.Map->Resources[pos].typ == RES_GOLD)
	{
		MaxGoldProd += server.Map->Resources[pos].value;
	}

	MaxMetalProd = min (MaxMetalProd, data.canMineMaxRes);
	MaxGoldProd  = min (MaxGoldProd,  data.canMineMaxRes);
	MaxOilProd   = min (MaxOilProd,   data.canMineMaxRes);
}

//--------------------------------------------------------------------------
/** Draw the attack cursor */
//--------------------------------------------------------------------------
void cBuilding::DrawAttackCursor (const cGameGUI& gameGUI, int x, int y)
{
	SDL_Rect r;
	int wp = 0, wc = 0, t = 0;
	cVehicle* v;
	cBuilding* b;

	selectTarget (v, b, x, y, data.canAttack, Client->getMap());

	if (! (v || b) || (v && v == gameGUI.getSelVehicle()) || (b && b == gameGUI.getSelBuilding()))
	{
		r.x = 1;
		r.y = 29;
		r.h = 3;
		r.w = 35;
		SDL_FillRect (GraphicsData.gfx_Cattack, &r, 0);
		return;
	}

	if (v)
		t = v->data.hitpointsCur;
	else if (b)
		t = b->data.hitpointsCur;

	if (t)
	{
		if (v)
			wc = (int) ( (float) t / v->data.hitpointsMax * 35);
		else if (b)
			wc = (int) ( (float) t / b->data.hitpointsMax * 35);
	}
	else
	{
		wc = 0;
	}

	if (v)
		t = v->calcHealth (data.damage);
	else if (b)
		t = b->calcHealth (data.damage);

	if (t)
	{
		if (v)
			wp = (int) ( (float) t / v->data.hitpointsMax * 35);
		else if (b)
			wp = (int) ( (float) t / b->data.hitpointsMax * 35);
	}
	else
	{
		wp = 0;
	}

	r.x = 1;

	r.y = 29;
	r.h = 3;
	r.w = wp;

	if (r.w)
		SDL_FillRect (GraphicsData.gfx_Cattack, &r, 0x00FF00);

	r.x += r.w;

	r.w = wc - wp;

	if (r.w)
		SDL_FillRect (GraphicsData.gfx_Cattack, &r, 0xFF0000);

	r.x += r.w;

	r.w = 35 - wc;

	if (r.w)
		SDL_FillRect (GraphicsData.gfx_Cattack, &r, 0);
}

//--------------------------------------------------------------------------
/** calculates the costs and the duration of the 3 buildspeeds for the vehicle with the given base costs
	iRemainingMetal is only needed for recalculating costs of vehicles in the Buildqueue and is set per default to -1 */
//--------------------------------------------------------------------------
void cBuilding::CalcTurboBuild (int* iTurboBuildRounds, int* iTurboBuildCosts, int iVehicleCosts, int iRemainingMetal)
{
	//first calc costs for a new Vehical

	//1x
	iTurboBuildCosts[0] = iVehicleCosts;

	//2x
	int a = iTurboBuildCosts[0];
	iTurboBuildCosts[1] = iTurboBuildCosts[0];

	while (a >= 2 * data.needsMetal)
	{
		iTurboBuildCosts[1] += 2 * data.needsMetal;
		a -= 2 * data.needsMetal;
	}

	//4x
	iTurboBuildCosts[2] = iTurboBuildCosts[1];
	a = iTurboBuildCosts[1];

	while (a >= 15)
	{
		iTurboBuildCosts[2] += (12 * data.needsMetal - min (a, 8 * data.needsMetal));
		a -= 8 * data.needsMetal;
	}

	//now this is a litle bit tricky ...
	//trying to calculate a plausible value, if we are changing the speed of an already started build-job
	if (iRemainingMetal >= 0)
	{
		float WorkedRounds;

		switch (BuildSpeed)    //BuildSpeed here is the previous build speed
		{

			case 0:
				WorkedRounds = (iTurboBuildCosts[0] - iRemainingMetal) / (float) (1 * data.needsMetal);
				iTurboBuildCosts[0] -= (int) (1    *  1 * data.needsMetal * WorkedRounds);
				iTurboBuildCosts[1] -= (int) (0.5  *  4 * data.needsMetal * WorkedRounds);
				iTurboBuildCosts[2] -= (int) (0.25 * 12 * data.needsMetal * WorkedRounds);
				break;

			case 1:
				WorkedRounds = (iTurboBuildCosts[1] - iRemainingMetal) / (float) (4 * data.needsMetal);
				iTurboBuildCosts[0] -= (int) (2   *  1 * data.needsMetal * WorkedRounds);
				iTurboBuildCosts[1] -= (int) (1   *  4 * data.needsMetal * WorkedRounds);
				iTurboBuildCosts[2] -= (int) (0.5 * 12 * data.needsMetal * WorkedRounds);
				break;

			case 2:
				WorkedRounds = (iTurboBuildCosts[2] - iRemainingMetal) / (float) (12 * data.needsMetal);
				iTurboBuildCosts[0] -= (int) (4 *  1 * data.needsMetal * WorkedRounds);
				iTurboBuildCosts[1] -= (int) (2 *  4 * data.needsMetal * WorkedRounds);
				iTurboBuildCosts[2] -= (int) (1 * 12 * data.needsMetal * WorkedRounds);
				break;
		}
	}


	//calc needed Rounds
	iTurboBuildRounds[0] = (int) ceil (iTurboBuildCosts[0] / (double) (1 * data.needsMetal));

	if (data.maxBuildFactor > 1)
	{
		iTurboBuildRounds[1] = (int) ceil (iTurboBuildCosts[1] / (double) (4 * data.needsMetal));
		iTurboBuildRounds[2] = (int) ceil (iTurboBuildCosts[2] / (double) (12 * data.needsMetal));
	}
	else
	{
		iTurboBuildRounds[1] = 0;
		iTurboBuildRounds[2] = 0;
	}
}

//------------------------------------------------------------------------
void cBuilding::sendUpgradeBuilding (const cClient& client, const cBuilding* building, bool upgradeAll)
{
	if (building == 0 || building->owner == 0)
		return;

	const sUnitData& currentVersion = building->data;
	const sUnitData& upgradedVersion = building->owner->BuildingData[building->typ->nr];
	if (currentVersion.version >= upgradedVersion.version)
		return; // already uptodate

	cNetMessage* msg = new cNetMessage (GAME_EV_WANT_BUILDING_UPGRADE);
	msg->pushBool (upgradeAll);
	msg->pushInt32 (building->iID);

	client.sendNetMessage (msg);
}

//--------------------------------------------------------------------------
void cBuilding::Select(cGameGUI& gameGUI)
{
	if (!owner) return;

	//load video
	if (gameGUI.getFLC() != NULL)
	{
		FLI_Close (gameGUI.getFLC());
		gameGUI.setFLC (NULL);
	}
	gameGUI.setVideoSurface (typ->video);

	// play sound:
	if (owner->researchFinished && data.canResearch)
		PlayVoice (VoiceData.VOIResearchComplete);
	else if (factoryHasJustFinishedBuilding())
	{
		int i = random (4);
		if (i == 0)
			PlayVoice (VoiceData.VOIBuildDone1);
		else if (i == 1)
			PlayVoice (VoiceData.VOIBuildDone2);
		else if (i == 2)
			PlayVoice (VoiceData.VOIBuildDone3);
		else
			PlayVoice (VoiceData.VOIBuildDone4);
	}
	else if (!IsWorking)
		PlayFX (SoundData.SNDHudButton);

	// display the details:
	gameGUI.setUnitDetailsData (NULL, this);
}

//--------------------------------------------------------------------------
void cBuilding::Deselct(cGameGUI& gameGUI)
{
	// Den Hintergrund wiederherstellen:
	StopFXLoop (gameGUI.iObjectStream);
	gameGUI.iObjectStream = -1;
	gameGUI.setVideoSurface (NULL);
	gameGUI.setUnitDetailsData (NULL, NULL);
}

//----------------------------------------------------------------
/** Playback of the soundstream that belongs to this building */
//----------------------------------------------------------------
int cBuilding::playStream()
{
	if (IsWorking)
		return PlayFXLoop (typ->Running);
	else
		return PlayFXLoop (typ->Wait);
	return 0;
}

//--------------------------------------------------------------------------
bool cBuilding::isDetectedByPlayer (const cPlayer* player) const
{
	for (unsigned int i = 0; i < detectedByPlayerList.Size(); i++)
	{
		if (detectedByPlayerList[i] == player) return true;
	}
	return false;
}

//--------------------------------------------------------------------------
void cBuilding::setDetectedByPlayer (cPlayer* player, bool addToDetectedInThisTurnList)
{
	if (!isDetectedByPlayer (player))
		detectedByPlayerList.Add (player);
}

//--------------------------------------------------------------------------
void cBuilding::resetDetectedByPlayer (const cPlayer* player)
{
	detectedByPlayerList.Remove (player);
}

//--------------------------------------------------------------------------
void cBuilding::makeDetection()
{
	//check whether the building has been detected by others
	if (data.isStealthOn == TERRAIN_NONE) return;

	if (data.isStealthOn & AREA_EXP_MINE)
	{
		int offset = PosX + PosY * Server->Map->size;
		for (unsigned int i = 0; i < Server->PlayerList->Size(); i++)
		{
			cPlayer* player = (*Server->PlayerList) [i];
			if (player == owner) continue;
			if (player->DetectMinesMap[offset])
			{
				setDetectedByPlayer (player);
			}
		}
	}
}

//--------------------------------------------------------------------------
void sBuilding::scaleSurfaces (float factor)
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
sUnitData* cBuilding::getUpgradedUnitData() const
{
	return & (owner->BuildingData[typ->nr]);
}

//-----------------------------------------------------------------------------
bool cBuilding::factoryHasJustFinishedBuilding() const
{
	return (BuildList && BuildList->Size() > 0 && isUnitWorking() == false && (*BuildList) [0]->metall_remaining <= 0);
}

//-----------------------------------------------------------------------------
void cBuilding::executeBuildCommand (cGameGUI& gameGUI)
{
	cVehiclesBuildMenu buildMenu (gameGUI, owner, this);
	buildMenu.show();
}

//-----------------------------------------------------------------------------
void cBuilding::executeMineManagerCommand (const cClient& client)
{
	cMineManagerMenu mineManager (client, this);
	mineManager.show();
}

//-----------------------------------------------------------------------------
void cBuilding::executeStopCommand (const cClient& client)
{
	sendWantStopWork (client, this);
}

//-----------------------------------------------------------------------------
void cBuilding::executeActivateStoredVehiclesCommand (cClient& client)
{
	cStorageMenu storageMenu (client, storedUnits, 0, this);
	storageMenu.show();
}

//-----------------------------------------------------------------------------
void cBuilding::executeUpdateBuildingCommmand (const cClient& client, bool updateAllOfSameType)
{
	sendUpgradeBuilding (client, this, updateAllOfSameType);
}

//-----------------------------------------------------------------------------
void cBuilding::executeSelfDestroyCommand (const cClient& client)
{
	cDestructMenu destructMenu;
	if (destructMenu.show() == 0)
		sendWantSelfDestroy (client, this);
}

//-----------------------------------------------------------------------------
bool cBuilding::buildingCanBeStarted() const
{
	return (data.canWork && isUnitWorking() == false
			&& ( (BuildList && BuildList->Size() > 0) || data.canBuild.empty()));
}

//-----------------------------------------------------------------------------
bool cBuilding::buildingCanBeUpgraded() const
{
	return (data.version != owner->BuildingData[typ->nr].version && SubBase && SubBase->Metal >= 2);
}
