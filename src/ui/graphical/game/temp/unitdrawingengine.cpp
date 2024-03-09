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

#include "ui/graphical/game/temp/unitdrawingengine.h"

#include "SDLutility/drawing.h"
#include "game/data/gui/gameguistate.h"
#include "game/data/gui/unitselection.h"
#include "game/data/map/map.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "output/video/video.h"
#include "resources/buildinguidata.h"
#include "resources/uidata.h"
#include "resources/vehicleuidata.h"
#include "ui/graphical/game/animations/animationtimer.h"
#include "utility/box.h"
#include "utility/listhelpers.h"
#include "utility/narrow_cast.h"
#include "utility/random.h"

//------------------------------------------------------------------------------
cUnitDrawingEngine::cUnitDrawingEngine (std::shared_ptr<cAnimationTimer> animationTimer_, std::shared_ptr<const cFrameCounter> frameCounter) :
	animationTimer (std::move (animationTimer_)),
	drawingCache (frameCounter),
	blinkColor (cRgbColor::white())
{
	signalConnectionManager.connect (animationTimer->triggered100ms, [this]() { rotateBlinkColor(); });
}

//------------------------------------------------------------------------------
void cUnitDrawingEngine::setDrawHits (bool drawHits)
{
	shouldDrawHits = drawHits;
}

//------------------------------------------------------------------------------
void cUnitDrawingEngine::setDrawStatus (bool drawStatus)
{
	shouldDrawStatus = drawStatus;
}

//------------------------------------------------------------------------------
void cUnitDrawingEngine::setDrawAmmo (bool drawAmmo)
{
	shouldDrawAmmo = drawAmmo;
}

//------------------------------------------------------------------------------
void cUnitDrawingEngine::setDrawColor (bool drawColor)
{
	shouldDrawColor = drawColor;
}

//------------------------------------------------------------------------------
void cUnitDrawingEngine::drawUnit (const cBuilding& building, SDL_Rect destination, float zoomFactor, const cUnitSelection* unitSelection, const cPlayer* player, const std::vector<cResearch::eResearchArea>& currentTurnResearchAreasFinished)
{
	// call getAnimationTime only once in this method and save the result,
	// to avoid a changing time within this method
	const unsigned long long animationTime = animationTimer->getAnimationTime();

	SDL_Rect dest = {0, 0, 0, 0};
	SDL_Surface* drawingSurface = drawingCache.getCachedImage (building, zoomFactor);
	if (drawingSurface == nullptr)
	{
		// no cached image found. building needs to be redrawn.
		drawingSurface = drawingCache.createNewEntry (building, zoomFactor);

		if (drawingSurface == nullptr)
		{
			// image will not be cached. So blitt directly to the screen buffer.
			dest = destination;
			drawingSurface = cVideo::buffer;
		}

		render (building, animationTime, *drawingSurface, dest, zoomFactor, cSettings::getInstance().isShadows(), true);
	}

	// now check, whether the image has to be blitted to screen buffer
	if (drawingSurface != cVideo::buffer)
	{
		dest = destination;
		SDL_BlitSurface (drawingSurface, nullptr, cVideo::buffer, &dest);

		// all following graphic operations are drawn directly to buffer
		dest = destination;
	}

	if (building.isRubble()) return;

	auto& uiData = UnitsUiData.getBuildingUI (building);
	// draw the effect if necessary
	if (uiData.staticData.powerOnGraphic && cSettings::getInstance().isAnimations() && (building.isUnitWorking() || !building.getStaticData().canWork))
	{
		SDL_Rect tmp = dest;
		SDL_SetSurfaceAlphaMod (uiData.eff.get(), narrow_cast<Uint8> (building.effectAlpha));

		CHECK_SCALING (*uiData.eff, *uiData.eff_org, zoomFactor);
		SDL_BlitSurface (uiData.eff.get(), nullptr, cVideo::buffer, &tmp);
	}

	// draw the mark, when a build order is finished
	if (building.getOwner() == player && ((!building.isBuildListEmpty() && !building.isUnitWorking() && building.getBuildListItem (0).getRemainingMetal() <= 0) || (building.getStaticData().canResearch && ranges::contains (currentTurnResearchAreasFinished, building.getResearchArea()))))
	{
		const cRgbColor finishedMarkColor = cRgbColor::green();
		const cBox<cPosition> d (cPosition (dest.x + 2, dest.y + 2), cPosition (dest.x + 2 + (building.getIsBig() ? 2 * destination.w - 3 : destination.w - 3), dest.y + 2 + (building.getIsBig() ? 2 * destination.h - 3 : destination.h - 3)));

		drawRectangle (*cVideo::buffer, d, finishedMarkColor.exchangeGreen (255 - 16 * (animationTime % 0x8)), 3);
	}

#if 0
	// disabled color-frame for buildings
	//   => now it's original game behavior - see ticket #542 (GER) = FIXED
	// but maybe as setting interesting
	//   => ticket #784 (ENG) (so I just commented it) = TODO

	// draw a colored frame if necessary
	if (shouldDrawColor)
	{
		const Uint32 color = 0xFF000000 | *static_cast<Uint32*> (building.owner->getColorSurface()->pixels);
		SDL_Rect d = {Sint16 (dest.x + 1), Sint16 (dest.y + 1), building.data.isBig ? 2 * destination.w - 1 : destination.w - 1, building.data.isBig ? 2 * destination.h - 1 : destination.h - 1};

		DrawRectangle (cVideo::buffer, d, color, 1);
	}
#endif
	// draw the selected-unit-flash-frame for buildings
	if (unitSelection && &building == unitSelection->getSelectedBuilding())
	{
		auto maxX = building.getIsBig() ? destination.w * 2 : destination.w;
		auto maxY = building.getIsBig() ? destination.h * 2 : destination.h;
		const int len = maxX / 4;
		maxX -= 3;
		maxY -= 3;
		const cBox<cPosition> d (cPosition (dest.x + 2, dest.y + 2), cPosition (dest.x + 2 + maxX, dest.y + 2 + maxY));

		drawSelectionCorner (*cVideo::buffer, d, blinkColor, len);
	}

	// draw health bar
	if (shouldDrawHits)
	{
		drawHealthBar (building, destination);
	}

	// draw ammo bar
	if (shouldDrawAmmo && (!player || building.getOwner() == player) && building.getStaticUnitData().canAttack && building.data.getAmmoMax() > 0)
	{
		drawMunBar (building, destination);
	}

	// draw status
	if (shouldDrawStatus)
	{
		drawStatus (building, destination);
	}
}

//------------------------------------------------------------------------------
void cUnitDrawingEngine::drawUnit (const cVehicle& vehicle, SDL_Rect destination, float zoomFactor, const cMapView& map, const cUnitSelection* unitSelection, const cPlayer* player)
{
	// call getAnimationTime only once in this method and save the result,
	// to avoid a changing time within this method
	const unsigned long long animationTime = animationTimer->getAnimationTime();

	// calculate screen position
	int ox = (int) (vehicle.getMovementOffset().x() * zoomFactor);
	int oy = (int) (vehicle.getMovementOffset().y() * zoomFactor);

	destination.x += ox;
	destination.y += oy;

	if (vehicle.getFlightHeight() > 0)
	{
		destination.x += vehicle.dither.x();
		destination.y += vehicle.dither.y();
	}

	SDL_Rect dest{};
	SDL_Surface* drawingSurface = drawingCache.getCachedImage (vehicle, zoomFactor, map, animationTime);
	if (drawingSurface == nullptr)
	{
		// no cached image found. building needs to be redrawn.
		drawingSurface = drawingCache.createNewEntry (vehicle, zoomFactor, map, animationTime);

		if (drawingSurface == nullptr)
		{
			// image will not be cached. So blitt directly to the screen buffer.
			dest = destination;
			drawingSurface = cVideo::buffer;
		}

		render (vehicle, &map, animationTime, player, *drawingSurface, dest, zoomFactor, cSettings::getInstance().isShadows());
	}

	// now check, whether the image has to be blitted to screen buffer
	if (drawingSurface != cVideo::buffer)
	{
		dest = destination;
		SDL_BlitSurface (drawingSurface, nullptr, cVideo::buffer, &dest);
	}

	// draw overlay if necessary:
	drawOverlayAnimation (vehicle, animationTime, *cVideo::buffer, destination, zoomFactor);

	// remove the dithering for the following operations
	if (vehicle.getFlightHeight() > 0)
	{
		destination.x -= vehicle.dither.x();
		destination.y -= vehicle.dither.y();
	}

	// remove movement offset for working units
	if (vehicle.isUnitBuildingABuilding() || vehicle.isUnitClearing())
	{
		destination.x -= ox;
		destination.y -= oy;
	}

	// draw indication, when building is complete
	if (vehicle.isUnitBuildingABuilding() && vehicle.getBuildTurns() == 0 && vehicle.getOwner() == player && !vehicle.bandPosition)
	{
		const cRgbColor finishedMarkColor = cRgbColor::green();
		const cBox<cPosition> d (cPosition (destination.x + 2, destination.y + 2), cPosition (destination.x + 2 + (vehicle.getIsBig() ? 2 * destination.w - 3 : destination.w - 3), destination.y + 2 + (vehicle.getIsBig() ? 2 * destination.h - 3 : destination.h - 3)));

		drawRectangle (*cVideo::buffer, d, finishedMarkColor.exchangeGreen (255 - 16 * (animationTime % 0x8)), 3);
	}

	// Draw the colored frame if necessary
	if (shouldDrawColor && vehicle.getOwner())
	{
		const cBox<cPosition> d (cPosition (destination.x + 1, destination.y + 1), cPosition (destination.x + 1 + (vehicle.getIsBig() ? 2 * destination.w - 1 : destination.w - 1), destination.y + 1 + (vehicle.getIsBig() ? 2 * destination.h - 1 : destination.h - 1)));

		drawRectangle (*cVideo::buffer, d, vehicle.getOwner()->getColor());
	}

	// draw the group selected frame if necessary
	if (unitSelection && unitSelection->getSelectedUnitsCount() > 1 && unitSelection->isSelected (vehicle))
	{
		const cRgbColor groupSelectionColor = cRgbColor::yellow();
		const cBox<cPosition> d (cPosition (destination.x + 2, destination.y + 2), cPosition (destination.x + 2 + (vehicle.getIsBig() ? 2 * destination.w - 3 : destination.w - 3), destination.y + 2 + (vehicle.getIsBig() ? 2 * destination.h - 3 : destination.h - 3)));

		drawRectangle (*cVideo::buffer, d, groupSelectionColor, 1);
	}
	// draw the seleted-unit-flash-frame for vehicles
	if (unitSelection && &vehicle == unitSelection->getSelectedVehicle())
	{
		auto maxX = vehicle.getIsBig() ? destination.w * 2 : destination.w;
		auto maxY = vehicle.getIsBig() ? destination.h * 2 : destination.h;
		const int len = maxX / 4;
		maxX -= 3;
		maxY -= 3;
		const cBox<cPosition> d (cPosition (destination.x + 2, destination.y + 2), cPosition (destination.x + 2 + maxX, destination.y + 2 + maxY));

		drawSelectionCorner (*cVideo::buffer, d, blinkColor, len);
	}

	// draw health bar
	if (shouldDrawHits)
	{
		drawHealthBar (vehicle, destination);
	}

	// draw ammo bar
	if (shouldDrawAmmo && (!player || vehicle.getOwner() == player) && vehicle.getStaticUnitData().canAttack)
	{
		drawMunBar (vehicle, destination);
	}

	// draw status info
	if (shouldDrawStatus)
	{
		drawStatus (vehicle, destination);
	}
}

//------------------------------------------------------------------------------
void cUnitDrawingEngine::drawHealthBar (const cUnit& unit, SDL_Rect destination)
{
	SDL_Rect r1;
	r1.x = destination.x + destination.w / 10 + 1;
	r1.y = destination.y + destination.h / 10;
	r1.w = destination.w * 8 / 10;
	r1.h = destination.h / 8;

	if (unit.getIsBig())
	{
		r1.w += destination.w;
		r1.h *= 2;
	}

	if (r1.h <= 2)
		r1.h = 3;

	SDL_Rect r2;
	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.w = (int) (((float) (r1.w - 2) / unit.data.getHitpointsMax()) * unit.data.getHitpoints());
	r2.h = r1.h - 2;

	SDL_FillRect (cVideo::buffer, &r1, 0xFF000000);

	Uint32 color;
	if (unit.data.getHitpoints() > unit.data.getHitpointsMax() / 2)
		color = 0xFF04AE04; // green
	else if (unit.data.getHitpoints() > unit.data.getHitpointsMax() / 4)
		color = 0xFFDBDE00; // orange
	else
		color = 0xFFE60000; // red
	SDL_FillRect (cVideo::buffer, &r2, color);
}

//------------------------------------------------------------------------------
void cUnitDrawingEngine::drawMunBar (const cUnit& unit, SDL_Rect destination)
{
	SDL_Rect r1;
	r1.x = destination.x + destination.w / 10 + 1;
	r1.y = destination.y + destination.h / 10 + destination.h / 8;
	r1.w = destination.w * 8 / 10;
	r1.h = destination.h / 8;

	if (r1.h <= 2)
	{
		r1.y += 1;
		r1.h = 3;
	}

	SDL_Rect r2;
	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.w = (int) (((float) (r1.w - 2) / unit.data.getAmmoMax()) * unit.data.getAmmo());
	r2.h = r1.h - 2;

	SDL_FillRect (cVideo::buffer, &r1, 0xFF000000);

	if (unit.data.getAmmo() > unit.data.getAmmoMax() / 2)
		SDL_FillRect (cVideo::buffer, &r2, 0xFF04AE04);
	else if (unit.data.getAmmo() > unit.data.getAmmoMax() / 4)
		SDL_FillRect (cVideo::buffer, &r2, 0xFFDBDE00);
	else
		SDL_FillRect (cVideo::buffer, &r2, 0xFFE60000);
}

//------------------------------------------------------------------------------
void cUnitDrawingEngine::drawStatus (const cUnit& unit, SDL_Rect destination)
{
	SDL_Rect dest;

	if (unit.isDisabled())
	{
		if (destination.w < 25)
			return;
		dest.x = destination.x + destination.w / 2 - 12;
		dest.y = destination.y + destination.h / 2 - 12;
		if (unit.getIsBig())
		{
			dest.y += (destination.h / 2);
			dest.x += (destination.w / 2);
		}
		SDL_Rect disabledSymbol = cGraphicsData::getRect_Symbol_Disabled();
		SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get(), &disabledSymbol, cVideo::buffer, &dest);
	}
	else
	{
		dest.y = destination.y + destination.h - 11;
		dest.x = destination.x + destination.w / 2 - 4;
		if (unit.getIsBig())
		{
			dest.y += (destination.h / 2);
			dest.x += (destination.w / 2);
		}
		if (unit.data.getSpeed() >= 4)
		{
			if (unit.data.getShots())
				dest.x -= destination.w / 4;

			SDL_Rect speedSymbol = cGraphicsData::getRect_Symbol_Speed();
			SDL_Rect destCopy = dest;
			SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get(), &speedSymbol, cVideo::buffer, &destCopy);
		}

		dest.x = destination.x + destination.w / 2 - 4;
		if (unit.data.getShots())
		{
			if (unit.data.getSpeed())
				dest.x += destination.w / 4;
			SDL_Rect shotsSymbol = cGraphicsData::getRect_Symbol_Shots();
			SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get(), &shotsSymbol, cVideo::buffer, &dest);
		}
	}
}

//------------------------------------------------------------------------------
void cUnitDrawingEngine::rotateBlinkColor()
{
	static bool dec = true;
	if (dec)
	{
		blinkColor.r -= 0x0A;
		blinkColor.g -= 0x0A;
		blinkColor.b -= 0x0A;
		if (blinkColor.r <= 0xA0) dec = false;
	}
	else
	{
		blinkColor.r += 0x0A;
		blinkColor.g += 0x0A;
		blinkColor.b += 0x0A;
		if (blinkColor.r >= (0xFF - 0x0A)) dec = true;
	}
}
