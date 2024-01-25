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

#include "buildinguidata.h"

#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "output/video/video.h"
#include "resources/playercolor.h"
#include "resources/uidata.h"
#include "utility/mathtools.h"

#include <cassert>

namespace
{
	//--------------------------------------------------------------------------
	void render_beton (SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, const cBuilding& building)
	{
		SDL_Rect tmp = dest;
		if (building.getIsBig())
		{
			CHECK_SCALING (*GraphicsData.gfx_big_beton, *GraphicsData.gfx_big_beton_org, zoomFactor);

			if (building.alphaEffectValue && cSettings::getInstance().isAlphaEffects())
				SDL_SetSurfaceAlphaMod (GraphicsData.gfx_big_beton.get(), building.alphaEffectValue);
			else
				SDL_SetSurfaceAlphaMod (GraphicsData.gfx_big_beton.get(), 254);

			SDL_BlitSurface (GraphicsData.gfx_big_beton.get(), nullptr, &surface, &tmp);
		}
		else
		{
			CHECK_SCALING (*UnitsUiData.ptr_small_beton, *UnitsUiData.ptr_small_beton_org, zoomFactor);
			if (building.alphaEffectValue && cSettings::getInstance().isAlphaEffects())
				SDL_SetSurfaceAlphaMod (UnitsUiData.ptr_small_beton, building.alphaEffectValue);
			else
				SDL_SetSurfaceAlphaMod (UnitsUiData.ptr_small_beton, 254);

			SDL_BlitSurface (UnitsUiData.ptr_small_beton, nullptr, &surface, &tmp);
			SDL_SetSurfaceAlphaMod (UnitsUiData.ptr_small_beton, 254);
		}
	}

	//--------------------------------------------------------------------------
	void render_rubble (SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow, const cBuilding& building)
	{
		assert (building.isRubble());

		SDL_Rect src;

		if (building.getIsBig())
		{
			if (!UnitsUiData.rubbleBig->img) return;
			src.w = src.h = (int) (UnitsUiData.rubbleBig->img_org->h * zoomFactor);
		}
		else
		{
			if (!UnitsUiData.rubbleSmall->img) return;
			src.w = src.h = (int) (UnitsUiData.rubbleSmall->img_org->h * zoomFactor);
		}

		src.x = src.w * building.rubbleTyp;
		SDL_Rect tmp = dest;
		src.y = 0;

		// draw the shadows
		if (drawShadow)
		{
			if (building.getIsBig())
			{
				CHECK_SCALING (*UnitsUiData.rubbleBig->shw, *UnitsUiData.rubbleBig->shw_org, zoomFactor);
				SDL_BlitSurface (UnitsUiData.rubbleBig->shw.get(), &src, &surface, &tmp);
			}
			else
			{
				CHECK_SCALING (*UnitsUiData.rubbleSmall->shw, *UnitsUiData.rubbleSmall->shw_org, zoomFactor);
				SDL_BlitSurface (UnitsUiData.rubbleSmall->shw.get(), &src, &surface, &tmp);
			}
		}

		// draw the building
		tmp = dest;

		if (building.getIsBig())
		{
			CHECK_SCALING (*UnitsUiData.rubbleBig->img, *UnitsUiData.rubbleBig->img_org, zoomFactor);
			SDL_BlitSurface (UnitsUiData.rubbleBig->img.get(), &src, &surface, &tmp);
		}
		else
		{
			CHECK_SCALING (*UnitsUiData.rubbleSmall->img, *UnitsUiData.rubbleSmall->img_org, zoomFactor);
			SDL_BlitSurface (UnitsUiData.rubbleSmall->img.get(), &src, &surface, &tmp);
		}
	}

} // namespace

//------------------------------------------------------------------------------
void sBuildingUIData::render_simple (SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, const cPlayer* owner, int frameNr, int alpha) const
{
	render_simple(surface, dest, zoomFactor, owner ? owner->getClan() : -1, owner ? std::make_optional(owner->getColor()) : std::nullopt, frameNr, alpha);
}

//------------------------------------------------------------------------------
void sBuildingUIData::render_simple (SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, int clan, std::optional<cRgbColor> playerColor, int frameNr, int alpha) const
{
	// read the size:
	SDL_Rect src;
	src.x = 0;
	src.y = 0;
	if (hasFrames)
	{
		src.w = Round (64.0f * zoomFactor);
		src.h = Round (64.0f * zoomFactor);
	}
	else
	{
		src.w = (int) (img_org->w * zoomFactor);
		src.h = (int) (img_org->h * zoomFactor);
	}

	// blit the players color and building graphic
	if (staticData.hasPlayerColor && playerColor)
		SDL_BlitSurface (cPlayerColor::getTexture (*playerColor), nullptr, GraphicsData.gfx_tmp.get(), nullptr);
	else
		SDL_FillRect (GraphicsData.gfx_tmp.get(), nullptr, 0x00FF00FF);

	if (hasFrames)
	{
		src.x = frameNr * Round (64.0f * zoomFactor);

		CHECK_SCALING (*img, *img_org, zoomFactor);
		SDL_BlitSurface (img.get(), &src, GraphicsData.gfx_tmp.get(), nullptr);

		src.x = 0;
	}
	else if (staticData.hasClanLogos)
	{
		CHECK_SCALING (*img, *img_org, zoomFactor);
		src.x = 0;
		src.y = 0;
		src.w = (int) (128 * zoomFactor);
		src.h = (int) (128 * zoomFactor);
		// select clan image
		if (clan != -1)
			src.x = (int) ((clan + 1) * 128 * zoomFactor);
		SDL_BlitSurface (img.get(), &src, GraphicsData.gfx_tmp.get(), nullptr);
	}
	else
	{
		CHECK_SCALING (*img, *img_org, zoomFactor);
		SDL_BlitSurface (img.get(), nullptr, GraphicsData.gfx_tmp.get(), nullptr);
	}

	// draw the building
	SDL_Rect tmp = dest;

	src.x = 0;
	src.y = 0;

	SDL_SetSurfaceAlphaMod (GraphicsData.gfx_tmp.get(), alpha);
	SDL_BlitSurface (GraphicsData.gfx_tmp.get(), &src, &surface, &tmp);
}

//------------------------------------------------------------------------------
void sBuildingUIData::render_simple (SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, const cBuilding& building, unsigned long long animationTime, int alpha) const
{
	int frameNr = building.dir;
	if (hasFrames && staticData.isAnimated && cSettings::getInstance().isAnimations() && building.isDisabled() == false)
	{
		frameNr = (animationTime % hasFrames);
	}
	render_simple (surface, dest, zoomFactor, building.getOwner(), frameNr, alpha);
}

//------------------------------------------------------------------------------
void sBuildingUIData::render (unsigned long long animationTime, SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, const cBuilding& building, bool drawShadow, bool drawConcrete) const
{
	// Note: when changing something in this function,
	// make sure to update the caching rules!

	// check, if it is dirt:
	if (building.isRubble())
	{
		render_rubble (surface, dest, zoomFactor, drawShadow, building);
		return;
	}

	// draw the concrete
	if (staticData.hasBetonUnderground && drawConcrete)
	{
		render_beton (surface, dest, zoomFactor, building);
	}
	// draw the connector slots:
	if ((building.subBase && !building.alphaEffectValue) || isConnectorGraphic)
	{
		drawConnectors (surface, dest, zoomFactor, building, drawShadow);
		if (isConnectorGraphic) return;
	}

	// draw the shadows
	if (drawShadow)
	{
		SDL_Rect tmp = dest;
		if (building.alphaEffectValue && cSettings::getInstance().isAlphaEffects())
			SDL_SetSurfaceAlphaMod (shw.get(), building.alphaEffectValue / 5);
		else
			SDL_SetSurfaceAlphaMod (shw.get(), 50);

		CHECK_SCALING (*shw, *shw_org, zoomFactor);
		blittAlphaSurface (shw.get(), nullptr, &surface, &tmp);
	}

	render_simple (surface, dest, zoomFactor, building, animationTime, building.alphaEffectValue && cSettings::getInstance().isAlphaEffects() ? building.alphaEffectValue : 254);
}
//------------------------------------------------------------------------------
/** Draws the connectors at the building: */
//------------------------------------------------------------------------------
void sBuildingUIData::drawConnectors (SDL_Surface& surface, SDL_Rect dest, float zoomFactor, const cBuilding& building, bool drawShadow) const
{
	SDL_Rect src, temp;

	CHECK_SCALING (*UnitsUiData.ptr_connector, *UnitsUiData.ptr_connector_org, zoomFactor);
	CHECK_SCALING (*UnitsUiData.ptr_connector_shw, *UnitsUiData.ptr_connector_shw_org, zoomFactor);

	if (building.alphaEffectValue)
		SDL_SetSurfaceAlphaMod (UnitsUiData.ptr_connector, building.alphaEffectValue);
	else
		SDL_SetSurfaceAlphaMod (UnitsUiData.ptr_connector, 254);

	src.y = 0;
	src.x = 0;
	src.h = src.w = UnitsUiData.ptr_connector->h;

	const auto BaseN = building.BaseN;
	const auto BaseW = building.BaseW;
	const auto BaseE = building.BaseE;
	const auto BaseS = building.BaseS;
	const auto BaseBN = building.BaseBN;
	const auto BaseBW = building.BaseBW;
	const auto BaseBE = building.BaseBE;
	const auto BaseBS = building.BaseBS;

	if (building.getIsBig())
	{
		// make connector stubs of big buildings.
		// upper left field
		src.x = 0;
		if (BaseN && BaseW)
			src.x = 7;
		else if (BaseN && !BaseW)
			src.x = 1;
		else if (!BaseN && BaseW)
			src.x = 4;
		src.x *= src.h;

		if (src.x != 0)
		{
			temp = dest;
			if (drawShadow) blittAlphaSurface (UnitsUiData.ptr_connector_shw, &src, &surface, &temp);
			temp = dest;
			SDL_BlitSurface (UnitsUiData.ptr_connector, &src, &surface, &temp);
		}

		// upper right field
		src.x = 0;
		dest.x += Round (64.0f * zoomFactor);
		if (BaseBN && BaseE)
			src.x = 8;
		else if (BaseBN && !BaseE)
			src.x = 1;
		else if (!BaseBN && BaseE)
			src.x = 2;
		src.x *= src.h;

		if (src.x != 0)
		{
			temp = dest;
			if (drawShadow) blittAlphaSurface (UnitsUiData.ptr_connector_shw, &src, &surface, &temp);
			temp = dest;
			SDL_BlitSurface (UnitsUiData.ptr_connector, &src, &surface, &temp);
		}

		// lower right field
		src.x = 0;
		dest.y += Round (64.0f * zoomFactor);
		if (BaseBE && BaseBS)
			src.x = 9;
		else if (BaseBE && !BaseBS)
			src.x = 2;
		else if (!BaseBE && BaseBS)
			src.x = 3;
		src.x *= src.h;

		if (src.x != 0)
		{
			temp = dest;
			if (drawShadow) blittAlphaSurface (UnitsUiData.ptr_connector_shw, &src, &surface, &temp);
			temp = dest;
			SDL_BlitSurface (UnitsUiData.ptr_connector, &src, &surface, &temp);
		}

		// lower left field
		src.x = 0;
		dest.x -= Round (64.0f * zoomFactor);
		if (BaseS && BaseBW)
			src.x = 10;
		else if (BaseS && !BaseBW)
			src.x = 3;
		else if (!BaseS && BaseBW)
			src.x = 4;
		src.x *= src.h;

		if (src.x != 0)
		{
			temp = dest;
			if (drawShadow) blittAlphaSurface (UnitsUiData.ptr_connector_shw, &src, &surface, &temp);
			temp = dest;
			SDL_BlitSurface (UnitsUiData.ptr_connector, &src, &surface, &temp);
		}
	}
	else
	{
		if (BaseN && BaseE && BaseS && BaseW)
			src.x = 15;
		else if (BaseN && BaseE && BaseS && !BaseW)
			src.x = 13;
		else if (BaseN && BaseE && !BaseS && BaseW)
			src.x = 12;
		else if (BaseN && BaseE && !BaseS && !BaseW)
			src.x = 8;
		else if (BaseN && !BaseE && BaseS && BaseW)
			src.x = 11;
		else if (BaseN && !BaseE && BaseS && !BaseW)
			src.x = 5;
		else if (BaseN && !BaseE && !BaseS && BaseW)
			src.x = 7;
		else if (BaseN && !BaseE && !BaseS && !BaseW)
			src.x = 1;
		else if (!BaseN && BaseE && BaseS && BaseW)
			src.x = 14;
		else if (!BaseN && BaseE && BaseS && !BaseW)
			src.x = 9;
		else if (!BaseN && BaseE && !BaseS && BaseW)
			src.x = 6;
		else if (!BaseN && BaseE && !BaseS && !BaseW)
			src.x = 2;
		else if (!BaseN && !BaseE && BaseS && BaseW)
			src.x = 10;
		else if (!BaseN && !BaseE && BaseS && !BaseW)
			src.x = 3;
		else if (!BaseN && !BaseE && !BaseS && BaseW)
			src.x = 4;
		else if (!BaseN && !BaseE && !BaseS && !BaseW)
			src.x = 0;
		src.x *= src.h;

		if (src.x != 0 || isConnectorGraphic)
		{
			// blit shadow
			temp = dest;
			if (drawShadow) blittAlphaSurface (UnitsUiData.ptr_connector_shw, &src, &surface, &temp);
			// blit the image
			temp = dest;
			SDL_BlitSurface (UnitsUiData.ptr_connector, &src, &surface, &temp);
		}
	}
}

//------------------------------------------------------------------------------
void render (const cBuilding& building, unsigned long long animationTime, SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow, bool drawConcrete)
{
	auto& uiData = UnitsUiData.getBuildingUI (building);
	uiData.render (animationTime, surface, dest, zoomFactor, building, drawShadow, drawConcrete);
}

//------------------------------------------------------------------------------
void render_simple (const cBuilding& building, SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, unsigned long long animationTime, int alpha)
{
	auto& uiData = UnitsUiData.getBuildingUI (building);
	uiData.render_simple (surface, dest, zoomFactor, building, animationTime, alpha);
}
