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

#include "vehicleuidata.h"

#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/player/player.h"
#include "game/data/units/unitdata.h"
#include "game/data/units/vehicle.h"
#include "output/video/video.h"
#include "resources/playercolor.h"
#include "resources/uidata.h"
#include "utility/mathtools.h"
#include "utility/narrow_cast.h"

#include <cassert>

namespace
{
	//--------------------------------------------------------------------------
	void blitWithPreScale (SDL_Surface& org_src, SDL_Surface& src, SDL_Rect* srcrect, SDL_Surface& dest, SDL_Rect* destrect, float factor, int frames = 1)
	{
		if (!cSettings::getInstance().shouldDoPrescale())
		{
			const int height = (int) (org_src.h * factor);
			const int width = (frames > 1) ? height * frames : narrow_cast<int> (org_src.w * factor);
			if (src.w != width || src.h != height)
			{
				scaleSurface (&org_src, &src, width, height);
			}
		}
		blittAlphaSurface (&src, srcrect, &dest, destrect);
	}

} // namespace

//------------------------------------------------------------------------------
void sVehicleUIData::render_shadow (const cVehicle& vehicle, const cMapView& map, SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor) const
{
	if (map.isWater (vehicle.getPosition()) && (vehicle.getStaticUnitData().isStealthOn & eTerrainFlag::Sea)) return;

	if (vehicle.alphaEffectValue && cSettings::getInstance().isAlphaEffects())
	{
		SDL_SetSurfaceAlphaMod (shw[vehicle.dir].get(), narrow_cast<Uint8> (vehicle.alphaEffectValue / 5));
	}
	else
	{
		SDL_SetSurfaceAlphaMod (shw[vehicle.dir].get(), 50);
	}
	SDL_Rect tmp = dest;

	// draw shadow
	if (vehicle.getFlightHeight() > 0)
	{
		int high = ((int) (Round (shw_org[vehicle.dir]->w * zoomFactor) * (vehicle.getFlightHeight() / 64.0f)));
		tmp.x += high;
		tmp.y += high;

		blitWithPreScale (*shw_org[vehicle.dir], *shw[vehicle.dir], nullptr, surface, &tmp, zoomFactor);
	}
	else if (vehicle.getStaticData().animationMovement)
	{
		const Uint16 size = narrow_cast<Uint16> (img_org[vehicle.dir]->h * zoomFactor);
		SDL_Rect r = {Sint16 (vehicle.WalkFrame * size), 0, size, size};
		blitWithPreScale (*shw_org[vehicle.dir], *shw[vehicle.dir], &r, surface, &tmp, zoomFactor);
	}
	else
		blitWithPreScale (*shw_org[vehicle.dir], *shw[vehicle.dir], nullptr, surface, &tmp, zoomFactor);
}

//------------------------------------------------------------------------------
void sVehicleUIData::render_smallClearing (const cVehicle& vehicle, unsigned long long animationTime, SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const
{
	assert (vehicle.isUnitClearing() && !vehicle.getIsBig() && !vehicle.jobActive);
	// draw shadow
	SDL_Rect tmp = dest;
	if (drawShadow)
		blitWithPreScale (*clear_small_shw_org, *clear_small_shw, nullptr, surface, &tmp, zoomFactor);

	// draw player color
	SDL_Rect src;
	src.y = 0;
	src.h = src.w = (int) (clear_small_org->h * zoomFactor);
	src.x = (animationTime % 4) * src.w;
	if (vehicle.getOwner())
	{
		SDL_BlitSurface (cPlayerColor::getTexture (vehicle.getOwner()->getColor()), nullptr, GraphicsData.gfx_tmp.get(), nullptr);
		blitWithPreScale (*clear_small_org, *clear_small, &src, *GraphicsData.gfx_tmp, nullptr, zoomFactor, 4);
	}
	// draw vehicle
	src.x = 0;
	src.y = 0;
	tmp = dest;
	SDL_SetSurfaceAlphaMod (GraphicsData.gfx_tmp.get(), 254);
	SDL_BlitSurface (GraphicsData.gfx_tmp.get(), &src, &surface, &tmp);
}

//------------------------------------------------------------------------------
void sVehicleUIData::render_BuildingOrBigClearing (const cVehicle& vehicle, const cMapView& map, unsigned long long animationTime, SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const
{
	assert ((vehicle.isUnitBuildingABuilding() || (vehicle.isUnitClearing() && vehicle.getIsBig())) && !vehicle.jobActive);

	// draw beton if necessary
	SDL_Rect tmp = dest;
	if (vehicle.isUnitBuildingABuilding() && vehicle.getIsBig() && (!map.isWaterOrCoast (vehicle.getPosition()) || map.getField (vehicle.getPosition()).getBaseBuilding()))
	{
		SDL_SetSurfaceAlphaMod (GraphicsData.gfx_big_beton.get(), narrow_cast<Uint8> (vehicle.bigBetonAlpha));
		CHECK_SCALING (*GraphicsData.gfx_big_beton, *GraphicsData.gfx_big_beton_org, zoomFactor);
		SDL_BlitSurface (GraphicsData.gfx_big_beton.get(), nullptr, &surface, &tmp);
	}

	// draw shadow
	tmp = dest;
	if (drawShadow) blitWithPreScale (*build_shw_org, *build_shw, nullptr, surface, &tmp, zoomFactor);

	// draw player color
	SDL_Rect src;
	src.y = 0;
	src.h = src.w = (int) (build_org->h * zoomFactor);
	src.x = (animationTime % 4) * src.w;
	SDL_BlitSurface (cPlayerColor::getTexture (vehicle.getOwner()->getColor()), nullptr, GraphicsData.gfx_tmp.get(), nullptr);
	blitWithPreScale (*build_org, *build, &src, *GraphicsData.gfx_tmp, nullptr, zoomFactor, 4);

	// draw vehicle
	src.x = 0;
	src.y = 0;
	tmp = dest;
	SDL_SetSurfaceAlphaMod (GraphicsData.gfx_tmp.get(), 254);
	SDL_BlitSurface (GraphicsData.gfx_tmp.get(), &src, &surface, &tmp);
}

//------------------------------------------------------------------------------
void sVehicleUIData::render_simple (SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, const sStaticVehicleData& vehicleData, const cPlayer* owner, int dir, int walkFrame, int alpha) const
{
	render_simple(surface, dest, zoomFactor, vehicleData, owner ? std::make_optional(owner->getColor()) : std::nullopt, dir, walkFrame, alpha);
}

//------------------------------------------------------------------------------
void sVehicleUIData::render_simple (SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, const sStaticVehicleData& vehicleData, std::optional<cRgbColor> playerColor, int dir, int walkFrame, int alpha) const
{
	// draw player color
	if (playerColor)
	{
		SDL_Surface* src = cPlayerColor::getTexture (*playerColor);
		SDL_Surface* dst = GraphicsData.gfx_tmp.get();
		SDL_BlitSurface (src, nullptr, dst, nullptr);
	}

	// read the size:
	SDL_Rect src;
	src.w = (int) (img_org[dir]->w * zoomFactor);
	src.h = (int) (img_org[dir]->h * zoomFactor);

	if (vehicleData.animationMovement)
	{
		SDL_Rect tmp;
		src.w = src.h = tmp.h = tmp.w = (int) (img_org[dir]->h * zoomFactor);
		tmp.x = walkFrame * tmp.w;
		tmp.y = 0;
		blitWithPreScale (*img_org[dir], *img[dir], &tmp, *GraphicsData.gfx_tmp, nullptr, zoomFactor);
	}
	else
		blitWithPreScale (*img_org[dir], *img[dir], nullptr, *GraphicsData.gfx_tmp, nullptr, zoomFactor);

	// draw the vehicle
	src.x = 0;
	src.y = 0;
	SDL_Rect tmp = dest;

	SDL_SetSurfaceAlphaMod (GraphicsData.gfx_tmp.get(), narrow_cast<Uint8> (alpha));
	blittAlphaSurface (GraphicsData.gfx_tmp.get(), &src, &surface, &tmp);
}

//------------------------------------------------------------------------------
void render (const cVehicle& vehicle, const cMapView* map, unsigned long long animationTime, const cPlayer* activePlayer, SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow)
{
	// Note: when changing something in this function,
	// make sure to update the caching rules!

	const auto& uiData = *UnitsUiData.getVehicleUI (vehicle.getStaticUnitData().ID);

	// draw working engineers and bulldozers:
	if (map && !vehicle.jobActive)
	{
		if (vehicle.isUnitBuildingABuilding() || (vehicle.isUnitClearing() && vehicle.getIsBig()))
		{
			uiData.render_BuildingOrBigClearing (vehicle, *map, animationTime, surface, dest, zoomFactor, drawShadow);
			return;
		}
		if (vehicle.isUnitClearing() && !vehicle.getIsBig())
		{
			assert (vehicle.isUnitClearing() && !vehicle.getIsBig() && !vehicle.jobActive);
			uiData.render_smallClearing (vehicle, animationTime, surface, dest, zoomFactor, drawShadow);
			return;
		}
	}

	// draw all other vehicles:

	// draw shadow
	if (drawShadow && map)
	{
		uiData.render_shadow (vehicle, *map, surface, dest, zoomFactor);
	}

	int alpha = 254;
	if (map)
	{
		if (vehicle.alphaEffectValue && cSettings::getInstance().isAlphaEffects())
		{
			alpha = vehicle.alphaEffectValue;
		}

		bool water = map->isWater (vehicle.getPosition());
		// if the vehicle can also drive on land, we have to check,
		// whether there is a bridge, platform, etc.
		// because the vehicle will drive on the bridge
		const cBuilding* building = map->getField (vehicle.getPosition()).getBaseBuilding();
		if (building && vehicle.getStaticUnitData().factorGround > 0 && (building->getStaticUnitData().surfacePosition == eSurfacePosition::Base || building->getStaticUnitData().surfacePosition == eSurfacePosition::AboveSea || building->getStaticUnitData().surfacePosition == eSurfacePosition::AboveBase)) water = false;

		if (water && (vehicle.getStaticUnitData().isStealthOn & eTerrainFlag::Sea) && !vehicle.isDetectedByAnyPlayer() && vehicle.getOwner() == activePlayer) alpha = std::min (alpha, 100);
	}
	render_simple (vehicle, surface, dest, zoomFactor, alpha);
}

//------------------------------------------------------------------------------
void sVehicleUIData::drawOverlayAnimation (SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, int frameNr, int alpha) const
{
	if (staticData.hasOverlay == false || cSettings::getInstance().isAnimations() == false) return;

	const Uint16 size = narrow_cast<Uint16> (overlay_org->h * zoomFactor);
	const Uint16 srcX = narrow_cast<Uint16> (Round ((overlay_org->h * frameNr) * zoomFactor));
	SDL_Rect src = {srcX, 0, size, size};

	SDL_Rect tmp = dest;
	const int offset = Round (64.0f * zoomFactor) / 2 - src.h / 2;
	tmp.x += offset;
	tmp.y += offset;

	SDL_SetSurfaceAlphaMod (overlay.get(), narrow_cast<Uint8> (alpha));
	blitWithPreScale (*overlay_org, *overlay, &src, surface, &tmp, zoomFactor);
}

//------------------------------------------------------------------------------
void render_simple (const cVehicle& vehicle, SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, int alpha)
{
	auto* uiData = UnitsUiData.getVehicleUI (vehicle.getStaticUnitData().ID);
	uiData->render_simple (surface, dest, zoomFactor, vehicle.getStaticData(), vehicle.getOwner(), vehicle.dir, vehicle.WalkFrame, alpha);
}

//------------------------------------------------------------------------------
void drawOverlayAnimation (const cVehicle& vehicle, SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor, int frameNr, int alpha)
{
	auto* uiData = UnitsUiData.getVehicleUI (vehicle.getStaticUnitData().ID);
	uiData->drawOverlayAnimation (surface, dest, zoomFactor, frameNr, alpha);
}

//------------------------------------------------------------------------------
void drawOverlayAnimation (const cVehicle& vehicle, unsigned long long animationTime, SDL_Surface& surface, const SDL_Rect& dest, float zoomFactor)
{
	const auto& uiData = *UnitsUiData.getVehicleUI (vehicle.getStaticUnitData().ID);

	if (uiData.staticData.hasOverlay == false || cSettings::getInstance().isAnimations() == false) return;
	int frameNr = 0;
	if (vehicle.isDisabled() == false)
	{
		frameNr = animationTime % (uiData.overlay_org->w / uiData.overlay_org->h);
	}

	uiData.drawOverlayAnimation (surface, dest, zoomFactor, frameNr, vehicle.alphaEffectValue && cSettings::getInstance().isAlphaEffects() ? vehicle.alphaEffectValue : 254);
}
