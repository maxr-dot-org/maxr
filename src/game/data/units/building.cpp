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

#include "game/data/units/building.h"

#include "game/logic/client.h"
#include "game/logic/clientevents.h"
#include "utility/listhelpers.h"
#include "game/logic/fxeffects.h"
#include "main.h"
#include "netmessage.h"
#include "pcx.h"
#include "game/data/player/player.h"
#include "game/logic/server.h"
#include "game/logic/serverevents.h"
#include "settings.h"
#include "game/logic/upgradecalculator.h"
#include "game/data/units/vehicle.h"
#include "video.h"
#include "unifonts.h"
#include "game/data/report/savedreportsimple.h"
#include "game/data/report/special/savedreportresourcechanged.h"
#include "utility/random.h"

#include "ui/sound/soundmanager.h"
#include "ui/sound/effects/soundeffectvoice.h"

using namespace std;


//--------------------------------------------------------------------------
cBuildListItem::cBuildListItem ()
{}

//--------------------------------------------------------------------------
cBuildListItem::cBuildListItem (sID type_, int remainingMetal_) :
	type (type_),
	remainingMetal (remainingMetal_)
{}

//--------------------------------------------------------------------------
cBuildListItem::cBuildListItem (const cBuildListItem& other) :
	type (other.type),
	remainingMetal (other.remainingMetal)
{}

//--------------------------------------------------------------------------
cBuildListItem::cBuildListItem (cBuildListItem&& other) :
	type (std::move (other.type)),
	remainingMetal (std::move (other.remainingMetal))
{}

//--------------------------------------------------------------------------
cBuildListItem& cBuildListItem::operator=(const cBuildListItem& other)
{
	type = other.type;
	remainingMetal = other.remainingMetal;
	return *this;
}

//--------------------------------------------------------------------------
cBuildListItem& cBuildListItem::operator=(cBuildListItem&& other)
{
	type = std::move (other.type);
	remainingMetal = std::move (other.remainingMetal);
	return *this;
}

//--------------------------------------------------------------------------
const sID& cBuildListItem::getType () const
{
	return type;
}

//--------------------------------------------------------------------------
void cBuildListItem::setType (const sID& type_)
{
	auto oldType = type;
	type = type_;
	if (type != oldType) typeChanged ();
}

//--------------------------------------------------------------------------
int cBuildListItem::getRemainingMetal () const
{
	return remainingMetal;
}

//--------------------------------------------------------------------------
void cBuildListItem::setRemainingMetal (int value)
{
	std::swap (remainingMetal, value);
	if (value != remainingMetal) remainingMetalChanged ();
}

//--------------------------------------------------------------------------
// cBuilding Implementation
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
cBuilding::cBuilding (const sUnitData* b, cPlayer* Owner, unsigned int ID) :
	cUnit ((Owner != 0 && b != 0) ? Owner->getUnitDataCurrentVersion (b->ID) : 0,
		   Owner,
		   ID),
	isWorking (false)
{
	setSentryActive(data.canAttack != TERRAIN_NONE);

	RubbleTyp = 0;
	RubbleValue = 0;
	researchArea = cResearch::kAttackResearch;
	uiData = b ? UnitsData.getBuildingUI (b->ID) : 0;
	points = 0;

	if (Owner == nullptr || b == nullptr)
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
	data.setHitpoints(data.getHitpointsMax());
	data.setAmmo(data.getAmmoMax());
	SubBase = nullptr;
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

	buildListChanged.connect ([&](){ statusChanged (); });
	buildListFirstItemDataChanged.connect ([&](){ statusChanged (); });
	researchAreaChanged.connect ([&](){ statusChanged (); });
	ownerChanged.connect ([&](){ registerOwnerEvents (); });

	registerOwnerEvents ();
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
		if (!data.canBuild.empty () && !buildList.empty () && getOwner () == player)
		{
			const cBuildListItem& buildListItem = buildList[0];
			const string& unitName = buildListItem.getType ().getUnitDataOriginalVersion ()->name;
			string sText;

			if (buildListItem.getRemainingMetal () > 0)
			{
				int iRound;

				iRound = (int) ceilf (buildListItem.getRemainingMetal () / (float) MetalPerRound);
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
		if (data.canResearch && getOwner () == player)
		{
			string sText = lngPack.i18n ("Text~Comp~Working") + "\n";
			for (int area = 0; area < cResearch::kNrResearchAreas; area++)
			{
				if (getOwner ()->getResearchCentersWorkingOnArea ((cResearch::ResearchArea)area) > 0)
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
					sText += ": " + iToStr (getOwner ()->getResearchState ().getRemainingTurns (area, getOwner ()->getResearchCentersWorkingOnArea ((cResearch::ResearchArea)area))) + "\n";
				}
			}
			return sText;
		}

		// Goldraffinerie:
		if (data.convertsGold && getOwner () == player)
		{
			string sText;
			sText = lngPack.i18n ("Text~Comp~Working") + "\n";
			sText += lngPack.i18n ("Text~Title~Credits") + ": ";
			sText += iToStr (getOwner ()->getCredits ());
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
void cBuilding::makeReport (cSoundManager& soundManager) const
{
	if (data.canResearch && isUnitWorking () && getOwner () && getOwner ()->isCurrentTurnResearchAreaFinished (getResearchArea ()))
	{
		soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, VoiceData.VOIResearchComplete));
	}
}

//--------------------------------------------------------------------------
/** Refreshs all data to the maximum values */
//--------------------------------------------------------------------------
bool cBuilding::refreshData()
{
	// NOTE: according to MAX 1.04 units get their shots/movepoints back even if they are disabled

	if (data.getShots () < data.getShotsMax())
	{
		data.setShots(std::min (this->data.getShotsMax(), this->data.getAmmo()));
		return true;
	}
	return false;
}

void cBuilding::render_rubble (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const
{
	assert (!getOwner ());

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

		if (alphaEffectValue && cSettings::getInstance ().isAlphaEffects ())
			SDL_SetSurfaceAlphaMod (GraphicsData.gfx_big_beton.get (), alphaEffectValue);
		else
			SDL_SetSurfaceAlphaMod (GraphicsData.gfx_big_beton.get (), 254);

		SDL_BlitSurface (GraphicsData.gfx_big_beton.get (), nullptr, surface, &tmp);
	}
	else
	{
		CHECK_SCALING (*UnitsData.ptr_small_beton, *UnitsData.ptr_small_beton_org, zoomFactor);
		if (alphaEffectValue && cSettings::getInstance ().isAlphaEffects ())
			SDL_SetSurfaceAlphaMod (UnitsData.ptr_small_beton, alphaEffectValue);
		else
			SDL_SetSurfaceAlphaMod (UnitsData.ptr_small_beton, 254);

		SDL_BlitSurface (UnitsData.ptr_small_beton, nullptr, surface, &tmp);
		SDL_SetSurfaceAlphaMod (UnitsData.ptr_small_beton, 254);
	}
}

void cBuilding::render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, int animationTime, int alpha) const
{
	int frameNr = dir;
	if (data.hasFrames && data.isAnimated && cSettings::getInstance ().isAnimations () &&
		isDisabled () == false)
	{
		frameNr = (animationTime % data.hasFrames);
	}

	render_simple (surface, dest, zoomFactor, data, *uiData, getOwner (), frameNr, alpha);
}

/*static*/ void cBuilding::render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, const sUnitData& data, const sBuildingUIData& uiData, const cPlayer* owner, int frameNr, int alpha)
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
		src.w = (int) (uiData.img_org->w * zoomFactor);
		src.h = (int) (uiData.img_org->h * zoomFactor);
	}

	// blit the players color and building graphic
	if (data.hasPlayerColor && owner) SDL_BlitSurface (owner->getColor ().getTexture (), nullptr, GraphicsData.gfx_tmp.get (), nullptr);
	else SDL_FillRect (GraphicsData.gfx_tmp.get (), nullptr, 0x00FF00FF);

	if (data.hasFrames)
	{
		src.x = frameNr * Round (64.0f * zoomFactor);

		CHECK_SCALING (*uiData.img, *uiData.img_org, zoomFactor);
		SDL_BlitSurface (uiData.img.get (), &src, GraphicsData.gfx_tmp.get (), nullptr);

		src.x = 0;
	}
	else if (data.hasClanLogos)
	{
		CHECK_SCALING (*uiData.img, *uiData.img_org, zoomFactor);
		src.x = 0;
		src.y = 0;
		src.w = (int) (128 * zoomFactor);
		src.h = (int) (128 * zoomFactor);
		// select clan image
		if (owner && owner->getClan () != -1)
			src.x = (int)((owner->getClan () + 1) * 128 * zoomFactor);
		SDL_BlitSurface (uiData.img.get (), &src, GraphicsData.gfx_tmp.get (), nullptr);
	}
	else
	{
		CHECK_SCALING (*uiData.img, *uiData.img_org, zoomFactor);
		SDL_BlitSurface (uiData.img.get (), nullptr, GraphicsData.gfx_tmp.get (), nullptr);
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
	if (!getOwner ())
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
	if ((this->SubBase && !alphaEffectValue) || data.isConnectorGraphic)
	{
		drawConnectors (surface, dest, zoomFactor, drawShadow);
		if (data.isConnectorGraphic) return;
	}

	// draw the shadows
	if (drawShadow)
	{
		SDL_Rect tmp = dest;
		if (alphaEffectValue && cSettings::getInstance ().isAlphaEffects ())
			SDL_SetSurfaceAlphaMod (uiData->shw.get (), alphaEffectValue / 5);
		else
			SDL_SetSurfaceAlphaMod (uiData->shw.get (), 50);

		CHECK_SCALING (*uiData->shw, *uiData->shw_org, zoomFactor);
		blittAlphaSurface (uiData->shw.get (), nullptr, surface, &tmp);
	}

	render_simple (surface, dest, zoomFactor, animationTime, alphaEffectValue && cSettings::getInstance ().isAlphaEffects () ? alphaEffectValue : 254);
}

//--------------------------------------------------------------------------
void cBuilding::updateNeighbours (const cMap& map)
{
	if (!data.isBig)
	{
		getOwner ()->base.checkNeighbour (getPosition () + cPosition (0, -1), *this);
		getOwner ()->base.checkNeighbour (getPosition () + cPosition (1, 0), *this);
		getOwner ()->base.checkNeighbour (getPosition () + cPosition (0, 1), *this);
		getOwner ()->base.checkNeighbour (getPosition () + cPosition (-1, 0), *this);
	}
	else
	{
		getOwner ()->base.checkNeighbour (getPosition () + cPosition (0, -1), *this);
		getOwner ()->base.checkNeighbour (getPosition () + cPosition (1, -1), *this);
		getOwner ()->base.checkNeighbour (getPosition () + cPosition (2, 0), *this);
		getOwner ()->base.checkNeighbour (getPosition () + cPosition (2, 1), *this);
		getOwner ()->base.checkNeighbour (getPosition () + cPosition (0, 2), *this);
		getOwner ()->base.checkNeighbour (getPosition () + cPosition (1, 2), *this);
		getOwner ()->base.checkNeighbour (getPosition () + cPosition (-1, 0), *this);
		getOwner ()->base.checkNeighbour (getPosition () + cPosition (-1, 1), *this);
	}
	CheckNeighbours (map);
}

//--------------------------------------------------------------------------
/** Checks, if there are neighbours */
//--------------------------------------------------------------------------
void cBuilding::CheckNeighbours (const cMap& map)
{
#define CHECK_NEIGHBOUR(x, y, m) \
	if (map.isValidPosition (cPosition(x, y))) \
	{ \
		const cBuilding* b = map.getField(cPosition(x, y)).getTopBuilding(); \
		if (b && b->getOwner() == getOwner() && b->data.connectsToBase) \
		{m = true;}else{m = false;} \
	}

	if (!data.isBig)
	{
		CHECK_NEIGHBOUR (getPosition().x()    , getPosition().y() - 1, BaseN)
		CHECK_NEIGHBOUR (getPosition().x() + 1, getPosition().y()    , BaseE)
		CHECK_NEIGHBOUR (getPosition().x()    , getPosition().y() + 1, BaseS)
		CHECK_NEIGHBOUR (getPosition().x() - 1, getPosition().y()    , BaseW)
	}
	else
	{
		CHECK_NEIGHBOUR (getPosition().x()    , getPosition().y() - 1, BaseN)
		CHECK_NEIGHBOUR (getPosition().x() + 1, getPosition().y() - 1, BaseBN)
		CHECK_NEIGHBOUR (getPosition().x() + 2, getPosition().y()    , BaseE)
		CHECK_NEIGHBOUR (getPosition().x() + 2, getPosition().y() + 1, BaseBE)
		CHECK_NEIGHBOUR (getPosition().x()    , getPosition().y() + 2, BaseS)
		CHECK_NEIGHBOUR (getPosition().x() + 1, getPosition().y() + 2, BaseBS)
		CHECK_NEIGHBOUR (getPosition().x() - 1, getPosition().y()    , BaseW)
		CHECK_NEIGHBOUR (getPosition().x() - 1, getPosition().y() + 1, BaseBW)
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

	if (alphaEffectValue) SDL_SetSurfaceAlphaMod (UnitsData.ptr_connector, alphaEffectValue);
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
		sendSavedReport (server, cSavedReportSimple (eSavedReportType::BuildingDisabled), getOwner ());
		return;
	}

	// needs human workers:
	if (data.needsHumans)
	{
		if (SubBase->HumanNeed + data.needsHumans > SubBase->HumanProd)
		{
			sendSavedReport (server, cSavedReportSimple (eSavedReportType::TeamInsufficient), getOwner ());
			return;
		}
	}

	// needs gold:
	if (data.convertsGold)
	{
		if (data.convertsGold + SubBase->GoldNeed > SubBase->getGoldProd() + SubBase->getGold())
		{
			sendSavedReport (server, cSavedReportSimple (eSavedReportType::GoldInsufficient), getOwner ());
			return;
		}
	}

	// needs raw material:
	if (data.needsMetal)
	{
		if (SubBase->MetalNeed + std::min (MetalPerRound, buildList[0].getRemainingMetal ()) > SubBase->getMetalProd () + SubBase->getMetal ())
		{
			sendSavedReport (server, cSavedReportSimple (eSavedReportType::MetalInsufficient), getOwner ());
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
			sendSavedReport (server, cSavedReportSimple (eSavedReportType::FuelInsufficient), getOwner ());
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

			sendSavedReport (server, cSavedReportResourceChanged (RES_OIL, missingOil, true), getOwner ());
			if (SubBase->getMetalProd () < metal)
				sendSavedReport (server, cSavedReportResourceChanged (RES_METAL, metal - SubBase->getMetalProd (), false), getOwner ());
			if (SubBase->getGoldProd () < gold)
				sendSavedReport (server, cSavedReportResourceChanged (RES_GOLD, gold - SubBase->getGoldProd (), false), getOwner ());
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

				sendSavedReport (server, cSavedReportSimple (eSavedReportType::EnergyInsufficient), getOwner ());
				return;
			}
			sendSavedReport (server, cSavedReportSimple (eSavedReportType::EnergyToLow), getOwner ());
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
		SubBase->MetalNeed += std::min (MetalPerRound, buildList[0].getRemainingMetal ());

	// gold consumer:
	SubBase->GoldNeed += data.convertsGold;

	// research building
	if (data.canResearch)
	{
		getOwner ()->startAResearch (researchArea);
	}

	if (data.canScore)
	{
		sendNumEcos (server, *getOwner ());
	}

	sendSubbaseValues (server, *SubBase, *getOwner ());
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
	if (data.canResearch)
		getOwner ()->startAResearch (researchArea);
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
			sendSavedReport (server, cSavedReportSimple (eSavedReportType::EnergyIsNeeded), getOwner ());
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
		SubBase->MetalNeed -= std::min (MetalPerRound, buildList[0].getRemainingMetal ());

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
		getOwner ()->stopAResearch (researchArea);
	}

	if (data.canScore)
	{
		sendNumEcos (server, *getOwner ());
	}

	sendSubbaseValues (server, *SubBase, *getOwner ());
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
		getOwner ()->stopAResearch (researchArea);
}

//------------------------------------------------------------
bool cBuilding::canTransferTo (const cPosition& position, const cMapField& overUnitField) const
{
	if (overUnitField.getVehicle ())
	{
		const cVehicle* v = overUnitField.getVehicle ();

		if (v->getOwner () != this->getOwner ())
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

		if (b->getOwner () != this->getOwner ())
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
	if (!map.possiblePlaceVehicle (vehicleData, position, getOwner ())) return false;
	if (!isNextTo (position)) return false;

	return true;
}

//--------------------------------------------------------------------------
bool cBuilding::canLoad (const cPosition& position, const cMap& map, bool checkPosition) const
{
	if (map.isValidPosition (position) == false) return false;

	if (canLoad (map.getField(position).getPlane(), checkPosition)) return true;
	else return canLoad (map.getField(position).getVehicle(), checkPosition);
}

//--------------------------------------------------------------------------
/** returns, if the vehicle can be loaded from its position: */
//--------------------------------------------------------------------------
bool cBuilding::canLoad (const cVehicle* Vehicle, bool checkPosition) const
{
	if (!Vehicle) return false;

	if (Vehicle->isUnitLoaded ()) return false;

	if (data.getStoredUnits() == data.storageUnitsMax) return false;

	if (checkPosition && !isNextTo (Vehicle->getPosition())) return false;

	if (!Contains (data.storeUnitsTypes, Vehicle->data.isStorageType)) return false;

	if (Vehicle->getClientMoveJob () && (Vehicle->isUnitMoving () || Vehicle->isAttacking () || Vehicle->MoveJobActive)) return false;

	if (Vehicle->getOwner () != getOwner () || Vehicle->isUnitBuildingABuilding () || Vehicle->isUnitClearing ()) return false;

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
		vehicle.getOwner ()->deleteSentry (vehicle);
	}

	vehicle.setLoaded(true);
	vehicle.setIsBeeinAttacked(false);

	storedUnits.push_back (&vehicle);
	data.setStoredUnits(data.getStoredUnits ()+1);

	getOwner ()->doScan ();
}

//-----------------------------------------------------------------------
// Unloads a vehicle
void cBuilding::exitVehicleTo(cVehicle& vehicle, const cPosition& position, cMap& map)
{
	Remove (storedUnits, &vehicle);

	data.setStoredUnits (data.getStoredUnits ()-1);

	map.addVehicle(vehicle, position);

	vehicle.setPosition(position);

	vehicle.setLoaded (false);

	getOwner ()->doScan ();
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
	auto position = getPosition ();

	MaxMetalProd = 0;
	MaxGoldProd = 0;
	MaxOilProd = 0;
	const sResources* res = &server.Map->getResource (position);

	switch (res->typ)
	{
		case RES_METAL: MaxMetalProd += res->value; break;
		case RES_GOLD:  MaxGoldProd  += res->value; break;
		case RES_OIL:   MaxOilProd   += res->value; break;
	}

	position.x()++;
	res = &server.Map->getResource (position);
	switch (res->typ)
	{
		case RES_METAL: MaxMetalProd += res->value; break;
		case RES_GOLD:  MaxGoldProd  += res->value; break;
		case RES_OIL:   MaxOilProd   += res->value; break;
	}

	position.y()++;
	res = &server.Map->getResource (position);
	switch (res->typ)
	{
		case RES_METAL: MaxMetalProd += res->value; break;
		case RES_GOLD:  MaxGoldProd  += res->value; break;
		case RES_OIL:   MaxOilProd   += res->value; break;
	}

	position.x()--;
	res = &server.Map->getResource (position);
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
		auto& playerList = server.playerList;
		for (unsigned int i = 0; i < playerList.size(); i++)
		{
			auto& player = *playerList[i];
			if (&player == getOwner ()) continue;
			if (player.hasMineDetection (getPosition ()))
			{
				setDetectedByPlayer (server, &player);
			}
		}
	}
}

//--------------------------------------------------------------------------
sBuildingUIData::sBuildingUIData()
{}

//--------------------------------------------------------------------------
sBuildingUIData::sBuildingUIData (sBuildingUIData&& other) :
	img (std::move (other.img)), img_org (std::move (other.img_org)),
	shw (std::move (other.shw)), shw_org (std::move (other.shw_org)),
	eff (std::move (other.eff)), eff_org (std::move (other.eff_org)),
	video (std::move (other.video)),
	info (std::move (other.info)),
	Start (std::move (other.Start)),
	Running (std::move (other.Running)),
	Stop (std::move (other.Stop)),
	Attack (std::move (other.Attack)),
	Wait (std::move (other.Wait))
{}

//--------------------------------------------------------------------------
sBuildingUIData& sBuildingUIData::operator=(sBuildingUIData&& other)
{
	img = std::move (other.img);
	img_org = std::move (other.img_org);
	shw = std::move (other.shw);
	shw_org = std::move (other.shw_org);
	eff = std::move (other.eff);
	eff_org = std::move (other.eff_org);
	video = std::move (other.video);
	info = std::move (other.info);

	Start = std::move (other.Start);
	Running = std::move (other.Running);
	Stop = std::move (other.Stop);
	Attack = std::move (other.Attack);
	Wait = std::move (other.Wait);
	return *this;
}

//--------------------------------------------------------------------------
void sBuildingUIData::scaleSurfaces (float factor)
{
	scaleSurface (img_org.get (), img.get (), (int)(img_org->w * factor), (int)(img_org->h * factor));
	scaleSurface (shw_org.get (), shw.get (), (int)(shw_org->w * factor), (int)(shw_org->h * factor));
	if (eff_org) scaleSurface (eff_org.get (), eff.get (), (int)(eff_org->w * factor), (int)(eff_org->h * factor));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- methods, that already have been extracted as part of cUnit refactoring
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
bool cBuilding::factoryHasJustFinishedBuilding() const
{
	return (!buildList.empty () && isUnitWorking () == false && buildList[0].getRemainingMetal () <= 0);
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
			&& (!buildList.empty () || data.canBuild.empty ()));
}

//-----------------------------------------------------------------------------
bool cBuilding::buildingCanBeUpgraded() const
{
	const sUnitData& upgraded = *getOwner ()->getUnitDataCurrentVersion (data.ID);
	return (data.getVersion () != upgraded.getVersion () && SubBase && SubBase->getMetal () >= 2);
}

//-----------------------------------------------------------------------------
bool cBuilding::isBuildListEmpty () const
{
	return buildList.empty ();
}

//-----------------------------------------------------------------------------
size_t cBuilding::getBuildListSize () const
{
	return buildList.size ();
}

//-----------------------------------------------------------------------------
const cBuildListItem& cBuilding::getBuildListItem (size_t index) const
{
	return buildList[index];
}

//-----------------------------------------------------------------------------
cBuildListItem& cBuilding::getBuildListItem (size_t index)
{
	return buildList[index];
}

//-----------------------------------------------------------------------------
void cBuilding::setBuildList (std::vector<cBuildListItem> buildList_)
{
	buildList = std::move (buildList_);

	buildListFirstItemSignalConnectionManager.disconnectAll ();
	if (!buildList.empty ())
	{
		buildListFirstItemSignalConnectionManager.connect (buildList[0].remainingMetalChanged, [this]() { buildListFirstItemDataChanged (); });
		buildListFirstItemSignalConnectionManager.connect (buildList[0].typeChanged, [this]() { buildListFirstItemDataChanged (); });
	}

	buildListChanged ();
}

//-----------------------------------------------------------------------------
void cBuilding::addBuildListItem (cBuildListItem item)
{
	buildList.push_back (std::move (item));

	buildListFirstItemSignalConnectionManager.disconnectAll ();
	if (!buildList.empty ())
	{
		buildListFirstItemSignalConnectionManager.connect (buildList[0].remainingMetalChanged, [this]() { buildListFirstItemDataChanged (); });
		buildListFirstItemSignalConnectionManager.connect (buildList[0].typeChanged, [this]() { buildListFirstItemDataChanged (); });
	}

	buildListChanged ();
}

//-----------------------------------------------------------------------------
void cBuilding::removeBuildListItem (size_t index)
{
	buildList.erase (buildList.begin () + index);

	buildListFirstItemSignalConnectionManager.disconnectAll ();
	if (!buildList.empty ())
	{
		buildListFirstItemSignalConnectionManager.connect (buildList[0].remainingMetalChanged, [this]() { buildListFirstItemDataChanged (); });
		buildListFirstItemSignalConnectionManager.connect (buildList[0].typeChanged, [this]() { buildListFirstItemDataChanged (); });
	}

	buildListChanged ();
}

//-----------------------------------------------------------------------------
void cBuilding::setWorking (bool value)
{
	std::swap (isWorking, value);
	if (value != isWorking) workingChanged ();
}

//-----------------------------------------------------------------------------
void cBuilding::setResearchArea (cResearch::ResearchArea area)
{
	std::swap (researchArea, area);
	if (researchArea != area) researchAreaChanged ();
}

//-----------------------------------------------------------------------------
cResearch::ResearchArea cBuilding::getResearchArea () const
{
	return researchArea;
}

//-----------------------------------------------------------------------------
void cBuilding::registerOwnerEvents ()
{
	ownerSignalConnectionManager.disconnectAll ();

	if (getOwner () == nullptr) return;

	if (data.convertsGold)
	{
		ownerSignalConnectionManager.connect (getOwner ()->creditsChanged, [&]() { statusChanged (); });
	}

	if (data.canResearch)
	{
		ownerSignalConnectionManager.connect (getOwner ()->researchCentersWorkingOnAreaChanged, [&](cResearch::ResearchArea) { statusChanged (); });
		ownerSignalConnectionManager.connect (getOwner ()->getResearchState ().neededResearchPointsChanged, [&](cResearch::ResearchArea) { statusChanged (); });
		ownerSignalConnectionManager.connect (getOwner ()->getResearchState ().currentResearchPointsChanged, [&](cResearch::ResearchArea) { statusChanged (); });
	}
}
