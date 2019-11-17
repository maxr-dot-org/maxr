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
#include "utility/listhelpers.h"
#include "game/logic/fxeffects.h"
#include "main.h"
//#include "pcx.h"
#include "netmessage2.h"
#include "game/data/player/player.h"
#include "settings.h"
#include "game/logic/upgradecalculator.h"
#include "game/data/units/vehicle.h"
#include "video.h"
#include "utility/unifonts.h"
#include "game/data/report/savedreportsimple.h"
#include "game/data/report/special/savedreportresourcechanged.h"
#include "utility/random.h"

#include "ui/sound/soundmanager.h"
#include "ui/sound/effects/soundeffectvoice.h"
#include "utility/crc.h"
#include "game/data/map/mapview.h"
#include "game/data/map/mapfieldview.h"

using namespace std;


//--------------------------------------------------------------------------
cBuildListItem::cBuildListItem()
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
cBuildListItem& cBuildListItem::operator= (const cBuildListItem& other)
{
	type = other.type;
	remainingMetal = other.remainingMetal;
	return *this;
}

//--------------------------------------------------------------------------
cBuildListItem& cBuildListItem::operator= (cBuildListItem && other)
{
	type = std::move (other.type);
	remainingMetal = std::move (other.remainingMetal);
	return *this;
}

//--------------------------------------------------------------------------
const sID& cBuildListItem::getType() const
{
	return type;
}

//--------------------------------------------------------------------------
void cBuildListItem::setType (const sID& type_)
{
	auto oldType = type;
	type = type_;
	if (type != oldType) typeChanged();
}

//--------------------------------------------------------------------------
int cBuildListItem::getRemainingMetal() const
{
	return remainingMetal;
}

//--------------------------------------------------------------------------
void cBuildListItem::setRemainingMetal (int value)
{
	std::swap (remainingMetal, value);
	if (value != remainingMetal) remainingMetalChanged();
}

//--------------------------------------------------------------------------
uint32_t cBuildListItem::getChecksum(uint32_t crc) const
{
	crc = calcCheckSum(type, crc);
	crc = calcCheckSum(remainingMetal, crc);

	return crc;
}

//--------------------------------------------------------------------------
// cBuilding Implementation
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
cBuilding::cBuilding (const cStaticUnitData* staticData, const cDynamicUnitData* data, cPlayer* owner, unsigned int ID) :
	cUnit(data, staticData, owner, ID),
	isWorking (false),
	wasWorking(false),
	metalPerRound(0),
	effectAlpha(0)
{
	setSentryActive (staticData && staticData->canAttack != TERRAIN_NONE);

	rubbleTyp = 0;
	rubbleValue = 0;
	researchArea = cResearch::kAttackResearch;
	uiData = staticData ? UnitsUiData.getBuildingUI(staticData->ID) : nullptr;
	points = 0;

	BaseN = false;
	BaseBN = false;
	BaseE = false;
	BaseBE = false;
	BaseS = false;
	BaseBS = false;
	BaseW = false;
	BaseBW = false;
	repeatBuild = false;

	maxMetalProd = 0;
	maxGoldProd = 0;
	maxOilProd = 0;
	metalProd = 0;
	goldProd = 0;
	oilProd = 0;

	subBase = nullptr;
	buildSpeed = 0;

	if (isBig)
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
		DamageFXPointX2 = 0;
		DamageFXPointY2 = 0;
	}

	refreshData();

	buildListChanged.connect ([&]() { statusChanged(); });
	buildListFirstItemDataChanged.connect ([&]() { statusChanged(); });
	researchAreaChanged.connect ([&]() { statusChanged(); });
	metalPerRoundChanged.connect([&]() { statusChanged(); });
	ownerChanged.connect ([&]() { registerOwnerEvents(); });

	registerOwnerEvents();
}

//--------------------------------------------------------------------------
cBuilding::~cBuilding()
{
}

//----------------------------------------------------
/** Returns a string with the current state */
//----------------------------------------------------
string cBuilding::getStatusStr (const cPlayer* whoWantsToKnow, const cUnitsData& unitsData) const
{
	auto font = cUnicodeFont::font.get();
	if (isDisabled())
	{
		string sText;
		sText = lngPack.i18n ("Text~Comp~Disabled") + " (";
		sText += iToStr (getDisabledTurns()) + ")";
		return sText;
	}
	if (isUnitWorking() || (factoryHasJustFinishedBuilding() && isDisabled() == false))
	{
		// Factory:
		if (!staticData->canBuild.empty() && !buildList.empty() && getOwner() == whoWantsToKnow)
		{
			const cBuildListItem& buildListItem = buildList[0];
			const string& unitName = unitsData.getStaticUnitData(buildListItem.getType()).getName();
			string sText;

			if (buildListItem.getRemainingMetal() > 0)
			{
				int iRound;

				iRound = (int) ceilf (buildListItem.getRemainingMetal() / (float)getMetalPerRound());
				sText = lngPack.i18n ("Text~Comp~Producing") + lngPack.i18n ("Text~Punctuation~Colon");
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
				sText += lngPack.i18n ("Text~Punctuation~Colon");
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
		if (staticData->canResearch && getOwner() == whoWantsToKnow)
		{
			string sText = lngPack.i18n ("Text~Comp~Working") + "\n";
			for (int area = 0; area < cResearch::kNrResearchAreas; area++)
			{
				if (getOwner()->getResearchCentersWorkingOnArea ((cResearch::ResearchArea)area) > 0)
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
					sText += lngPack.i18n ("Text~Punctuation~Colon") + iToStr (getOwner()->getResearchState().getRemainingTurns (area, getOwner()->getResearchCentersWorkingOnArea ((cResearch::ResearchArea)area))) + "\n";
				}
			}
			return sText;
		}

		// Goldraffinerie:
		if (staticData->convertsGold && getOwner() == whoWantsToKnow)
		{
			string sText;
			sText = lngPack.i18n ("Text~Comp~Working") + "\n";
			sText += lngPack.i18n ("Text~Title~Credits") + lngPack.i18n ("Text~Punctuation~Colon");
			sText += iToStr (getOwner()->getCredits());
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

	//GoldRaf idle + gold-amount
	if (staticData->convertsGold && getOwner() == whoWantsToKnow && !isUnitWorking())
	{
		string sText;
		sText = lngPack.i18n("Text~Comp~Waits") + "\n";
		sText += lngPack.i18n("Text~Title~Credits") + lngPack.i18n("Text~Punctuation~Colon");
		sText += iToStr(getOwner()->getCredits());
		return sText;
	}

	//Research centre idle + projects
	// Research Center
	if (staticData->canResearch && getOwner() == whoWantsToKnow && !isUnitWorking())
	{
		string sText = lngPack.i18n("Text~Comp~Waits") + "\n";
		for (int area = 0; area < cResearch::kNrResearchAreas; area++)
		{
			if (getOwner()->getResearchCentersWorkingOnArea((cResearch::ResearchArea)area) > 0)
			{
				switch (area)
				{
				case cResearch::kAttackResearch: sText += lngPack.i18n("Text~Others~Attack"); break;
				case cResearch::kShotsResearch: sText += lngPack.i18n("Text~Others~Shots_7"); break;
				case cResearch::kRangeResearch: sText += lngPack.i18n("Text~Others~Range"); break;
				case cResearch::kArmorResearch: sText += lngPack.i18n("Text~Others~Armor_7"); break;
				case cResearch::kHitpointsResearch: sText += lngPack.i18n("Text~Others~Hitpoints_7"); break;
				case cResearch::kSpeedResearch: sText += lngPack.i18n("Text~Others~Speed"); break;
				case cResearch::kScanResearch: sText += lngPack.i18n("Text~Others~Scan"); break;
				case cResearch::kCostResearch: sText += lngPack.i18n("Text~Others~Costs"); break;
				}
				sText += lngPack.i18n("Text~Punctuation~Colon") + iToStr(getOwner()->getResearchState().getRemainingTurns(area, getOwner()->getResearchCentersWorkingOnArea((cResearch::ResearchArea)area))) + "\n";
			}
		}
		return sText;
	}


	return lngPack.i18n ("Text~Comp~Waits");
}


//--------------------------------------------------------------------------
void cBuilding::makeReport (cSoundManager& soundManager) const
{
	if (staticData && staticData->canResearch && isUnitWorking() && getOwner() && getOwner()->isCurrentTurnResearchAreaFinished (getResearchArea()))
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

	if (data.getShots() < data.getShotsMax())
	{
		data.setShots (std::min (this->data.getShotsMax(), this->data.getAmmo()));
		return true;
	}
	return false;
}

void cBuilding::render_rubble (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const
{
	assert (isRubble());

	SDL_Rect src;

	if (isBig)
	{
		if (!UnitsUiData.rubbleBig->img) return;
		src.w = src.h = (int) (UnitsUiData.rubbleBig->img_org->h * zoomFactor);
	}
	else
	{
		if (!UnitsUiData.rubbleSmall->img) return;
		src.w = src.h = (int) (UnitsUiData.rubbleSmall->img_org->h * zoomFactor);
	}

	src.x = src.w * rubbleTyp;
	SDL_Rect tmp = dest;
	src.y = 0;

	// draw the shadows
	if (drawShadow)
	{
		if (isBig)
		{
			CHECK_SCALING (*UnitsUiData.rubbleBig->shw, *UnitsUiData.rubbleBig->shw_org, zoomFactor);
			SDL_BlitSurface (UnitsUiData.rubbleBig->shw.get(), &src, surface, &tmp);
		}
		else
		{
			CHECK_SCALING (*UnitsUiData.rubbleSmall->shw, *UnitsUiData.rubbleSmall->shw_org, zoomFactor);
			SDL_BlitSurface (UnitsUiData.rubbleSmall->shw.get(), &src, surface, &tmp);
		}
	}

	// draw the building
	tmp = dest;

	if (isBig)
	{
		CHECK_SCALING (*UnitsUiData.rubbleBig->img, *UnitsUiData.rubbleBig->img_org, zoomFactor);
		SDL_BlitSurface (UnitsUiData.rubbleBig->img.get(), &src, surface, &tmp);
	}
	else
	{
		CHECK_SCALING (*UnitsUiData.rubbleSmall->img, *UnitsUiData.rubbleSmall->img_org, zoomFactor);
		SDL_BlitSurface (UnitsUiData.rubbleSmall->img.get(), &src, surface, &tmp);
	}
}

//------------------------------------------------------------------------------
void cBuilding::render_beton (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor) const
{
	SDL_Rect tmp = dest;
	if (isBig)
	{
		CHECK_SCALING (*GraphicsData.gfx_big_beton, *GraphicsData.gfx_big_beton_org, zoomFactor);

		if (alphaEffectValue && cSettings::getInstance().isAlphaEffects())
			SDL_SetSurfaceAlphaMod (GraphicsData.gfx_big_beton.get(), alphaEffectValue);
		else
			SDL_SetSurfaceAlphaMod (GraphicsData.gfx_big_beton.get(), 254);

		SDL_BlitSurface (GraphicsData.gfx_big_beton.get(), nullptr, surface, &tmp);
	}
	else
	{
		CHECK_SCALING (*UnitsUiData.ptr_small_beton, *UnitsUiData.ptr_small_beton_org, zoomFactor);
		if (alphaEffectValue && cSettings::getInstance().isAlphaEffects())
			SDL_SetSurfaceAlphaMod(UnitsUiData.ptr_small_beton, alphaEffectValue);
		else
			SDL_SetSurfaceAlphaMod(UnitsUiData.ptr_small_beton, 254);

		SDL_BlitSurface(UnitsUiData.ptr_small_beton, nullptr, surface, &tmp);
		SDL_SetSurfaceAlphaMod(UnitsUiData.ptr_small_beton, 254);
	}
}

//------------------------------------------------------------------------------
void cBuilding::connectFirstBuildListItem()
{
	buildListFirstItemSignalConnectionManager.disconnectAll();
	if (!buildList.empty())
	{
		buildListFirstItemSignalConnectionManager.connect(buildList[0].remainingMetalChanged, [this]() { buildListFirstItemDataChanged(); });
		buildListFirstItemSignalConnectionManager.connect(buildList[0].typeChanged, [this]() { buildListFirstItemDataChanged(); });
	}
}

//------------------------------------------------------------------------------
void cBuilding::render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, unsigned long long animationTime, int alpha) const
{
	int frameNr = dir;
	if (uiData->hasFrames && uiData->isAnimated && cSettings::getInstance().isAnimations() &&
		isDisabled() == false)
	{
		frameNr = (animationTime % uiData->hasFrames);
	}

	render_simple (surface, dest, zoomFactor, *uiData, getOwner(), frameNr, alpha);
}

/*static*/ void cBuilding::render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, const sBuildingUIData& uiData, const cPlayer* owner, int frameNr, int alpha)
{
	// read the size:
	SDL_Rect src;
	src.x = 0;
	src.y = 0;
	if (uiData.hasFrames)
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
	if (uiData.hasPlayerColor && owner) SDL_BlitSurface(owner->getColor().getTexture(), nullptr, GraphicsData.gfx_tmp.get(), nullptr);
	else SDL_FillRect (GraphicsData.gfx_tmp.get(), nullptr, 0x00FF00FF);

	if (uiData.hasFrames)
	{
		src.x = frameNr * Round (64.0f * zoomFactor);

		CHECK_SCALING (*uiData.img, *uiData.img_org, zoomFactor);
		SDL_BlitSurface (uiData.img.get(), &src, GraphicsData.gfx_tmp.get(), nullptr);

		src.x = 0;
	}
	else if (uiData.hasClanLogos)
	{
		CHECK_SCALING (*uiData.img, *uiData.img_org, zoomFactor);
		src.x = 0;
		src.y = 0;
		src.w = (int) (128 * zoomFactor);
		src.h = (int) (128 * zoomFactor);
		// select clan image
		if (owner && owner->getClan() != -1)
			src.x = (int) ((owner->getClan() + 1) * 128 * zoomFactor);
		SDL_BlitSurface (uiData.img.get(), &src, GraphicsData.gfx_tmp.get(), nullptr);
	}
	else
	{
		CHECK_SCALING (*uiData.img, *uiData.img_org, zoomFactor);
		SDL_BlitSurface (uiData.img.get(), nullptr, GraphicsData.gfx_tmp.get(), nullptr);
	}

	// draw the building
	SDL_Rect tmp = dest;

	src.x = 0;
	src.y = 0;

	SDL_SetSurfaceAlphaMod (GraphicsData.gfx_tmp.get(), alpha);
	SDL_BlitSurface (GraphicsData.gfx_tmp.get(), &src, surface, &tmp);
}


void cBuilding::render (unsigned long long animationTime, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow, bool drawConcrete) const
{
	// Note: when changing something in this function,
	// make sure to update the caching rules!

	// check, if it is dirt:
	if (isRubble())
	{
		render_rubble (surface, dest, zoomFactor, drawShadow);
		return;
	}

	// draw the concrete
	if (uiData->hasBetonUnderground && drawConcrete)
	{
		render_beton (surface, dest, zoomFactor);
	}
	// draw the connector slots:
	if ((this->subBase && !alphaEffectValue) || uiData->isConnectorGraphic)
	{
		drawConnectors (surface, dest, zoomFactor, drawShadow);
		if (uiData->isConnectorGraphic) return;
	}

	// draw the shadows
	if (drawShadow)
	{
		SDL_Rect tmp = dest;
		if (alphaEffectValue && cSettings::getInstance().isAlphaEffects())
			SDL_SetSurfaceAlphaMod (uiData->shw.get(), alphaEffectValue / 5);
		else
			SDL_SetSurfaceAlphaMod (uiData->shw.get(), 50);

		CHECK_SCALING (*uiData->shw, *uiData->shw_org, zoomFactor);
		blittAlphaSurface (uiData->shw.get(), nullptr, surface, &tmp);
	}

	render_simple (surface, dest, zoomFactor, animationTime, alphaEffectValue && cSettings::getInstance().isAlphaEffects() ? alphaEffectValue : 254);
}

//--------------------------------------------------------------------------
void cBuilding::updateNeighbours (const cMap& map)
{
	if (!isBig)
	{
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 0, -1), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 1,  0), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 0,  1), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition (-1,  0), *this, map);
	}
	else
	{
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 0, -1), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 1, -1), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 2,  0), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 2,  1), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 0,  2), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 1,  2), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition (-1,  0), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition (-1,  1), *this, map);
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
		if (b && b->getOwner() == getOwner() && b->staticData->connectsToBase) \
		{m = true;}else{m = false;} \
	}

	if (!isBig)
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

	CHECK_SCALING(*UnitsUiData.ptr_connector, *UnitsUiData.ptr_connector_org, zoomFactor);
	CHECK_SCALING(*UnitsUiData.ptr_connector_shw, *UnitsUiData.ptr_connector_shw_org, zoomFactor);

	if (alphaEffectValue) SDL_SetSurfaceAlphaMod(UnitsUiData.ptr_connector, alphaEffectValue);
	else SDL_SetSurfaceAlphaMod(UnitsUiData.ptr_connector, 254);

	src.y = 0;
	src.x = 0;
	src.h = src.w = UnitsUiData.ptr_connector->h;

	if (!isBig)
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

		if (src.x != 0 || uiData->isConnectorGraphic)
		{
			// blit shadow
			temp = dest;
			if (drawShadow) blittAlphaSurface (UnitsUiData.ptr_connector_shw, &src, surface, &temp);
			// blit the image
			temp = dest;
			SDL_BlitSurface (UnitsUiData.ptr_connector, &src, surface, &temp);
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
			if (drawShadow) blittAlphaSurface (UnitsUiData.ptr_connector_shw, &src, surface, &temp);
			temp = dest;
			SDL_BlitSurface (UnitsUiData.ptr_connector, &src, surface, &temp);
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
			if (drawShadow) blittAlphaSurface (UnitsUiData.ptr_connector_shw, &src, surface, &temp);
			temp = dest;
			SDL_BlitSurface (UnitsUiData.ptr_connector, &src, surface, &temp);
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
			if (drawShadow) blittAlphaSurface (UnitsUiData.ptr_connector_shw, &src, surface, &temp);
			temp = dest;
			SDL_BlitSurface (UnitsUiData.ptr_connector, &src, surface, &temp);
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
			if (drawShadow) blittAlphaSurface (UnitsUiData.ptr_connector_shw, &src, surface, &temp);
			temp = dest;
			SDL_BlitSurface (UnitsUiData.ptr_connector, &src, surface, &temp);
		}
	}
}

//--------------------------------------------------------------------------
/** starts the building for the server thread */
//--------------------------------------------------------------------------
void cBuilding::startWork ()
{
	if (isUnitWorking())
	{
		return;
	}

	if (isDisabled())
	{
		//TODO: is report needed?
		//sendSavedReport (server, cSavedReportSimple (eSavedReportType::BuildingDisabled), getOwner());
		return;
	}

	if (!subBase->startBuilding(this))
		return;

	// research building
	if (staticData->canResearch)
	{
		getOwner()->startAResearch (researchArea);
	}

	if (staticData->canScore)
	{
		//sendNumEcos (server, *getOwner());
	}
}

//--------------------------------------------------------------------------
/** Stops the building's working */
//--------------------------------------------------------------------------
void cBuilding::stopWork (bool forced)
{
	if (!isUnitWorking()) return;

	if (!subBase->stopBuilding(this, forced))
		return;
	
	if (staticData->canResearch)
	{
		getOwner()->stopAResearch (researchArea);
	}

	if (staticData->canScore)
	{
		//sendNumEcos (server, *getOwner());
	}
}

bool cBuilding::canTransferTo(const cPosition& position, const cMapView& map) const
{
	const auto& field = map.getField(position);

	const cUnit* unit = field.getVehicle();
	if (unit)
	{
		return canTransferTo(*unit);
	}

	unit = field.getTopBuilding();
	if (unit)
	{
		return canTransferTo(*unit);
	}

	return false;
}

//------------------------------------------------------------
bool cBuilding::canTransferTo (const cUnit& unit) const
{
	if (unit.getOwner() != getOwner())
		return false;

	if (&unit == this)
		return false;


	if (unit.isAVehicle())
	{
		const cVehicle* v = static_cast<const cVehicle*>(&unit);


		if (v->getStaticUnitData().storeResType != staticData->storeResType)
			return false;

		if (v->isUnitBuildingABuilding() || v->isUnitClearing())
			return false;

		for (const auto b : subBase->getBuildings())
		{
			if (b->isNextTo (v->getPosition())) return true;
		}

		return false;
	}
	else if (unit.isABuilding())
	{
		const cBuilding* b = static_cast<const cBuilding*>(&unit);
		if (b->subBase != subBase)
			return false;

		if (staticData->storeResType != b->getStaticUnitData().storeResType)
			return false;

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
bool cBuilding::canExitTo(const cPosition& position, const cMap& map, const cStaticUnitData& vehicleData) const
{
	if (!map.possiblePlaceVehicle(vehicleData, position, getOwner())) return false;
	if (!isNextTo(position)) return false;

	return true;
}

//--------------------------------------------------------------------------
bool cBuilding::canExitTo(const cPosition& position, const cMapView& map, const cStaticUnitData& vehicleData) const
{
	if (!map.possiblePlaceVehicle(vehicleData, position)) return false;
	if (!isNextTo(position)) return false;

	return true;
}

//--------------------------------------------------------------------------
bool cBuilding::canLoad (const cPosition& position, const cMapView& map, bool checkPosition) const
{
	if (map.isValidPosition (position) == false) return false;

	if (canLoad (map.getField (position).getPlane(), checkPosition)) return true;
	else return canLoad (map.getField (position).getVehicle(), checkPosition);
}

//--------------------------------------------------------------------------
/** returns, if the vehicle can be loaded from its position: */
//--------------------------------------------------------------------------
bool cBuilding::canLoad (const cVehicle* vehicle, bool checkPosition) const
{
	if (!vehicle) return false;

	if (vehicle->isUnitLoaded()) return false;

	if (storedUnits.size() == staticData->storageUnitsMax) return false;

	if (checkPosition && !isNextTo (vehicle->getPosition())) return false;

	if (!Contains(staticData->storeUnitsTypes, vehicle->getStaticUnitData().isStorageType)) return false;

	if (vehicle->isUnitMoving() || vehicle->isAttacking()) return false;

	if (vehicle->getOwner() != getOwner() || vehicle->isUnitBuildingABuilding() || vehicle->isUnitClearing()) return false;

	if (vehicle->isBeeingAttacked()) return false;

	return true;
}

//------------------------------------------------------------------------------
bool cBuilding::canSupply(const cUnit* unit, eSupplyType supplyType) const
{
	if (unit == nullptr || unit->isABuilding())
		return false;

	if (subBase->getMetalStored() <= 0)
		return false;

	if (!Contains(storedUnits, static_cast<const cVehicle*>(unit)))
		return false;

	switch (supplyType)
	{
	case eSupplyType::REARM:
		if (unit->getStaticUnitData().canAttack == false || unit->data.getAmmo() >= unit->data.getAmmoMax())
			return false;
		if (!staticData->canRearm)
			return false;
		break;
	case eSupplyType::REPAIR:
		if (unit->data.getHitpoints() >= unit->data.getHitpointsMax())
			return false;
		if (!staticData->canRepair)
			return false;
		break;
	default:
		return false;
	}

	return true;
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

		SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get(), &src, sf, &dest);

		dest.x += offx;
	}
}

//-------------------------------------------------------------------------------
/** checks the resources that are available under the mining station */
//--------------------------------------------------------------------------

void cBuilding::initMineRessourceProd (const cMap& map)
{
	if (!staticData->canMineMaxRes) return;

	auto position = getPosition();

	maxMetalProd = 0;
	maxGoldProd = 0;
	maxOilProd = 0;
	const sResources* res = &map.getResource (position);

	switch (res->typ)
	{
		case eResourceType::Metal: maxMetalProd += res->value; break;
		case eResourceType::Gold:  maxGoldProd  += res->value; break;
		case eResourceType::Oil:   maxOilProd   += res->value; break;
	}

	if (isBig)
	{
		position.x()++;
		res = &map.getResource(position);
		switch (res->typ)
		{
		case eResourceType::Metal: maxMetalProd += res->value; break;
		case eResourceType::Gold:  maxGoldProd += res->value; break;
		case eResourceType::Oil:   maxOilProd += res->value; break;
		}

		position.y()++;
		res = &map.getResource(position);
		switch (res->typ)
		{
		case eResourceType::Metal: maxMetalProd += res->value; break;
		case eResourceType::Gold:  maxGoldProd += res->value; break;
		case eResourceType::Oil:   maxOilProd += res->value; break;
		}

		position.x()--;
		res = &map.getResource(position);
		switch (res->typ)
		{
		case eResourceType::Metal: maxMetalProd += res->value; break;
		case eResourceType::Gold:  maxGoldProd += res->value; break;
		case eResourceType::Oil:   maxOilProd += res->value; break;
		}
	}

	maxMetalProd = min (maxMetalProd, staticData->canMineMaxRes);
	maxGoldProd  = min (maxGoldProd, staticData->canMineMaxRes);
	maxOilProd   = min (maxOilProd, staticData->canMineMaxRes);

	// set default mine allocation
	int freeProductionCapacity = staticData->canMineMaxRes;
	metalProd = maxMetalProd;
	freeProductionCapacity -= metalProd;
	goldProd = min(maxGoldProd, freeProductionCapacity);
	freeProductionCapacity -= goldProd;
	oilProd = min(maxOilProd, freeProductionCapacity);

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

	while (a >= 2 * staticData->needsMetal)
	{
		turboBuildCosts[1] += 2 * staticData->needsMetal;
		a -= 2 * staticData->needsMetal;
	}

	// 4x
	turboBuildCosts[2] = turboBuildCosts[1];
	a = turboBuildCosts[1];

	while (a >= 15)
	{
		turboBuildCosts[2] += (12 * staticData->needsMetal - min(a, 8 * staticData->needsMetal));
		a -= 8 * staticData->needsMetal;
	}

	// now this is a litle bit tricky ...
	// trying to calculate a plausible value,
	// if we are changing the speed of an already started build-job
	if (remainingMetal >= 0)
	{
		float WorkedRounds;

		switch (buildSpeed)  // BuildSpeed here is the previous build speed
		{
			case 0:
				WorkedRounds = (turboBuildCosts[0] - remainingMetal) / (1.f * staticData->needsMetal);
				turboBuildCosts[0] -= (int) (1     *  1 * staticData->needsMetal * WorkedRounds);
				turboBuildCosts[1] -= (int) (0.5f  *  4 * staticData->needsMetal * WorkedRounds);
				turboBuildCosts[2] -= (int) (0.25f * 12 * staticData->needsMetal * WorkedRounds);
				break;

			case 1:
				WorkedRounds = (turboBuildCosts[1] - remainingMetal) / (float)(4 * staticData->needsMetal);
				turboBuildCosts[0] -= (int) (2    *  1 * staticData->needsMetal * WorkedRounds);
				turboBuildCosts[1] -= (int) (1    *  4 * staticData->needsMetal * WorkedRounds);
				turboBuildCosts[2] -= (int) (0.5f * 12 * staticData->needsMetal * WorkedRounds);
				break;

			case 2:
				WorkedRounds = (turboBuildCosts[2] - remainingMetal) / (float)(12 * staticData->needsMetal);
				turboBuildCosts[0] -= (int) (4 *  1 * staticData->needsMetal * WorkedRounds);
				turboBuildCosts[1] -= (int) (2 *  4 * staticData->needsMetal * WorkedRounds);
				turboBuildCosts[2] -= (int) (1 * 12 * staticData->needsMetal * WorkedRounds);
				break;
		}
	}

	// calc needed turns
	turboBuildRounds[0] = (int) ceilf (turboBuildCosts[0] / (1.f * staticData->needsMetal));

	if (staticData->maxBuildFactor > 1)
	{
		turboBuildRounds[1] = (int) ceilf (turboBuildCosts[1] / (4.f * staticData->needsMetal));
		turboBuildRounds[2] = (int) ceilf (turboBuildCosts[2] / (12.f * staticData->needsMetal));
	}
	else
	{
		turboBuildRounds[1] = 0;
		turboBuildRounds[2] = 0;
	}
}

//--------------------------------------------------------------------------
sBuildingUIData::sBuildingUIData():
	hasClanLogos(false),
	hasDamageEffect(false),
	hasBetonUnderground(false),
	hasPlayerColor(false),
	hasOverlay(false),
	buildUpGraphic(false),
	powerOnGraphic(false),
	isAnimated(false),
	isConnectorGraphic(false),
	hasFrames(0)
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
	Wait (std::move (other.Wait)),
	hasClanLogos(other.hasClanLogos),
	hasDamageEffect(other.hasDamageEffect),
	hasBetonUnderground(other.hasBetonUnderground),
	hasPlayerColor(other.hasPlayerColor),
	hasOverlay(other.hasOverlay),
	buildUpGraphic(other.buildUpGraphic),
	powerOnGraphic(other.powerOnGraphic),
	isAnimated(isAnimated),
	isConnectorGraphic(isConnectorGraphic),
	hasFrames(other.hasFrames)
{}

//--------------------------------------------------------------------------
sBuildingUIData& sBuildingUIData::operator= (sBuildingUIData && other)
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

	hasClanLogos = other.hasClanLogos;
	hasDamageEffect = other.hasDamageEffect;
	hasBetonUnderground = other.hasBetonUnderground;
	hasPlayerColor = other.hasPlayerColor;
	hasOverlay = other.hasOverlay;
	buildUpGraphic = other.buildUpGraphic;
	powerOnGraphic = other.powerOnGraphic;
	isAnimated = other.isAnimated;
	isConnectorGraphic = other.isConnectorGraphic;
	hasFrames = other.hasFrames;

	return *this;
}

//--------------------------------------------------------------------------
void sBuildingUIData::scaleSurfaces (float factor)
{
	scaleSurface (img_org.get(), img.get(), (int) (img_org->w * factor), (int) (img_org->h * factor));
	scaleSurface (shw_org.get(), shw.get(), (int) (shw_org->w * factor), (int) (shw_org->h * factor));
	if (eff_org) scaleSurface (eff_org.get(), eff.get(), (int) (eff_org->w * factor), (int) (eff_org->h * factor));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- methods, that already have been extracted as part of cUnit refactoring
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
bool cBuilding::factoryHasJustFinishedBuilding() const
{
	return (!buildList.empty() && isUnitWorking() == false && buildList[0].getRemainingMetal() <= 0);
}

//-----------------------------------------------------------------------------
bool cBuilding::buildingCanBeStarted() const
{
	return (staticData->canWork && isUnitWorking() == false
		&& (!buildList.empty() || staticData->canBuild.empty()));
}

//-----------------------------------------------------------------------------
bool cBuilding::buildingCanBeUpgraded() const
{
	const cDynamicUnitData& upgraded = *getOwner()->getUnitDataCurrentVersion (data.getId());
	return (data.getVersion() != upgraded.getVersion() && subBase && subBase->getMetalStored() >= 2);
}

//-----------------------------------------------------------------------------
bool cBuilding::isBuildListEmpty() const
{
	return buildList.empty();
}

//-----------------------------------------------------------------------------
size_t cBuilding::getBuildListSize() const
{
	return buildList.size();
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

	connectFirstBuildListItem();

	buildListChanged();
}

//-----------------------------------------------------------------------------
void cBuilding::addBuildListItem (cBuildListItem item)
{
	buildList.push_back (std::move (item));

	connectFirstBuildListItem();

	buildListChanged();
}

//-----------------------------------------------------------------------------
void cBuilding::removeBuildListItem (size_t index)
{
	buildList.erase (buildList.begin() + index);

	connectFirstBuildListItem();

	buildListChanged();
}

//-----------------------------------------------------------------------------
int cBuilding::getBuildSpeed() const
{
	return buildSpeed;
}

//-----------------------------------------------------------------------------
int cBuilding::getMetalPerRound() const
{
	if (buildList.size() > 0)
		return min(metalPerRound, buildList[0].getRemainingMetal());
	else
		return 0;
}

//-----------------------------------------------------------------------------
bool cBuilding::getRepeatBuild() const
{
	return repeatBuild;
}

//-----------------------------------------------------------------------------
void cBuilding::setWorking (bool value)
{
	std::swap (isWorking, value);
	if (value != isWorking) workingChanged();
}

//-----------------------------------------------------------------------------
void cBuilding::setBuildSpeed(int value)
{
	std::swap(buildSpeed, value);
	if(value != buildSpeed) buildSpeedChanged();
}

//-----------------------------------------------------------------------------
void cBuilding::setMetalPerRound(int value)
{
	std::swap(metalPerRound, value);
	if(value != metalPerRound) metalPerRoundChanged();
}

//-----------------------------------------------------------------------------
void cBuilding::setRepeatBuild(bool value)
{
	std::swap(repeatBuild, value);
	if(value != repeatBuild) repeatBuildChanged();
}

int cBuilding::getMaxProd(eResourceType type) const
{
	switch (type)
	{
	case eResourceType::Metal:
		return maxMetalProd;
	case eResourceType::Gold:
		return maxGoldProd;
	case eResourceType::Oil:
		return maxOilProd;
	default:
		return 0;
	}
}

//-----------------------------------------------------------------------------
void cBuilding::setResearchArea (cResearch::ResearchArea area)
{
	std::swap (researchArea, area);
	if (researchArea != area) researchAreaChanged();
}

//-----------------------------------------------------------------------------
cResearch::ResearchArea cBuilding::getResearchArea() const
{
	return researchArea;
}

//------------------------------------------------------------------------------
void cBuilding::setRubbleValue(int value, cCrossPlattformRandom& randomGenerator)
{
	rubbleValue = value;

	if (isBig)
	{
		rubbleTyp = randomGenerator.get(2);
		uiData = UnitsUiData.rubbleBig;
	}
	else
	{
		rubbleTyp = randomGenerator.get(5);
		uiData = UnitsUiData.rubbleSmall;
	}
}

//------------------------------------------------------------------------------
int cBuilding::getRubbleValue() const
{
	return rubbleValue;
}

//------------------------------------------------------------------------------
uint32_t cBuilding::getChecksum(uint32_t crc) const
{
	crc = cUnit::getChecksum(crc);
	crc = calcCheckSum(rubbleTyp, crc);
	crc = calcCheckSum(rubbleValue, crc);
	crc = calcCheckSum(BaseN, crc);
	crc = calcCheckSum(BaseE, crc);
	crc = calcCheckSum(BaseS, crc);
	crc = calcCheckSum(BaseW, crc);
	crc = calcCheckSum(BaseBN, crc);
	crc = calcCheckSum(BaseBE, crc);
	crc = calcCheckSum(BaseBS, crc);
	crc = calcCheckSum(BaseBW, crc);
	crc = calcCheckSum(metalProd, crc);
	crc = calcCheckSum(oilProd, crc);
	crc = calcCheckSum(goldProd, crc);
	crc = calcCheckSum(wasWorking, crc);
	crc = calcCheckSum(points, crc);
	crc = calcCheckSum(isWorking, crc);
	crc = calcCheckSum(buildSpeed, crc);
	crc = calcCheckSum(metalPerRound, crc);
	crc = calcCheckSum(repeatBuild, crc);
	crc = calcCheckSum(maxMetalProd, crc);
	crc = calcCheckSum(maxOilProd, crc);
	crc = calcCheckSum(maxGoldProd, crc);
	crc = calcCheckSum(researchArea, crc);
	crc = calcCheckSum(buildList, crc);

	return crc;
}

//-----------------------------------------------------------------------------
void cBuilding::registerOwnerEvents()
{
	ownerSignalConnectionManager.disconnectAll();

	if (getOwner() == nullptr || staticData == nullptr) return;

	if (staticData->convertsGold)
	{
		ownerSignalConnectionManager.connect (getOwner()->creditsChanged, [&]() { statusChanged(); });
	}

	if (staticData->canResearch)
	{
		ownerSignalConnectionManager.connect (getOwner()->researchCentersWorkingOnAreaChanged, [&] (cResearch::ResearchArea) { statusChanged(); });
		ownerSignalConnectionManager.connect (getOwner()->getResearchState().neededResearchPointsChanged, [&] (cResearch::ResearchArea) { statusChanged(); });
		ownerSignalConnectionManager.connect (getOwner()->getResearchState().currentResearchPointsChanged, [&] (cResearch::ResearchArea) { statusChanged(); });
	}
}
