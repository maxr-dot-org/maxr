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

#include <algorithm>

#include "ui/graphical/game/temp/unitdrawingengine.h"
#include "ui/graphical/game/animations/animationtimer.h"
#include "ui/graphical/game/unitselection.h"
#include "video.h"
#include "buildings.h"
#include "vehicles.h"
#include "game/data/player/player.h"
#include "map.h"
#include "utility/random.h"
#include "utility/drawing.h"
#include "utility/box.h"

//--------------------------------------------------------------------------
cUnitDrawingEngine::cUnitDrawingEngine (std::shared_ptr<cAnimationTimer> animationTimer_) :
	animationTimer (std::move (animationTimer_)),
	drawingCache (animationTimer),
	blinkColor (cColor::white()),
	shouldDrawHits (false),
	shouldDrawStatus (false),
	shouldDrawAmmo (false),
	shouldDrawColor (false)
{
	signalConnectionManager.connect (animationTimer->triggered100ms, std::bind (&cUnitDrawingEngine::rotateBlinkColor, this));
}

//--------------------------------------------------------------------------
void cUnitDrawingEngine::setDrawHits (bool drawHits)
{
	shouldDrawHits = drawHits;
}

//--------------------------------------------------------------------------
void cUnitDrawingEngine::setDrawStatus (bool drawStatus)
{
	shouldDrawStatus = drawStatus;
}

//--------------------------------------------------------------------------
void cUnitDrawingEngine::setDrawAmmo (bool drawAmmo)
{
	shouldDrawAmmo = drawAmmo;
}

//--------------------------------------------------------------------------
void cUnitDrawingEngine::setDrawColor (bool drawColor)
{
	shouldDrawColor = drawColor;
}

//--------------------------------------------------------------------------
void cUnitDrawingEngine::drawUnit (const cBuilding& building, SDL_Rect destination, float zoomFactor, const cUnitSelection* unitSelection, const cPlayer* player)
{
	SDL_Rect dest = {0, 0, 0, 0};
	bool bDraw = false;
	SDL_Surface* drawingSurface = drawingCache.getCachedImage (building, zoomFactor);
	if (drawingSurface == NULL)
	{
		// no cached image found. building needs to be redrawn.
		bDraw = true;
		drawingSurface = drawingCache.createNewEntry (building, zoomFactor);
	}

	if (drawingSurface == NULL)
	{
		// image will not be cached. So blitt directly to the screen buffer.
		dest = destination;
		drawingSurface = cVideo::buffer;
	}

	if (bDraw)
	{
		building.render (animationTimer->getAnimationTime(), drawingSurface, dest, zoomFactor, cSettings::getInstance ().isShadows (), true);
	}

	// now check, whether the image has to be blitted to screen buffer
	if (drawingSurface != cVideo::buffer)
	{
		dest = destination;
		SDL_BlitSurface (drawingSurface, NULL, cVideo::buffer, &dest);

		// all following graphic operations are drawn directly to buffer
		dest = destination;
	}

	if (!building.owner) return;

	// draw the effect if necessary
	if (building.data.powerOnGraphic && cSettings::getInstance ().isAnimations () && (building.isUnitWorking () || !building.data.canWork))
	{
		SDL_Rect tmp = dest;
		SDL_SetSurfaceAlphaMod (building.uiData->eff.get(), building.effectAlpha);

		CHECK_SCALING (*building.uiData->eff, *building.uiData->eff_org, zoomFactor);
        SDL_BlitSurface (building.uiData->eff.get (), NULL, cVideo::buffer, &tmp);
	}

	// draw the mark, when a build order is finished
	if (building.owner == player && ((!building.isBuildListEmpty() && !building.isUnitWorking () && building.getBuildListItem(0).getRemainingMetal () <= 0) ||
									 (building.data.canResearch && building.owner->researchFinished)))
	{
		const cColor finishedMarkColor = cColor::green();
		const cBox<cPosition> d (cPosition (dest.x + 2, dest.y + 2), cPosition (dest.x + 2 + (building.data.isBig ? 2 * destination.w - 3 : destination.w - 3), dest.y + 2 + (building.data.isBig ? 2 * destination.h - 3 : destination.h - 3)));

		drawRectangle (*cVideo::buffer, d, finishedMarkColor.exchangeGreen (255 - 16 * (animationTimer->getAnimationTime () % 0x8)), 3);
	}

#if 0
	// disabled color-frame for buildings
	//   => now it's original game behavior - see ticket #542 (GER) = FIXED
	// but maybe as setting interresting
	//   => ticket #784 (ENG) (so I just commented it) = TODO

	// draw a colored frame if necessary
	if (shouldDrawColor)
	{
		const Uint32 color = 0xFF000000 | *static_cast<Uint32*> (building.owner->getColorSurface ()->pixels);
		SDL_Rect d = {Sint16 (dest.x + 1), Sint16 (dest.y + 1), building.data.isBig ? 2 * destination.w - 1 : destination.w - 1, building.data.isBig ? 2 * destination.h - 1 : destination.h - 1};

		DrawRectangle (cVideo::buffer, d, color, 1);
	}
#endif
	// draw the seleted-unit-flash-frame for bulidings
	if (unitSelection && &building == unitSelection->getSelectedBuilding())
	{
		Uint16 maxX = building.data.isBig ? destination.w  * 2 : destination.w;
		Uint16 maxY = building.data.isBig ? destination.h  * 2 : destination.h;
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
	if (shouldDrawAmmo && (!player || building.owner == player) && building.data.canAttack && building.data.ammoMax > 0)
	{
		drawMunBar (building, destination);
	}

	// draw status
	if (shouldDrawStatus)
	{
		drawStatus (building, destination);
	}

	// attack job debug output
	//if (gameGUI.getAJobDebugStatus ())
	//{
	//	cServer* server = gameGUI.getClient ()->getServer ();
	//	const cBuilding* serverBuilding = NULL;
	//	if (server) serverBuilding = server->Map->fields[server->Map->getOffset (PosX, PosY)].getBuilding ();
	//	if (building.isBeeingAttacked) font->showText (dest.x + 1, dest.y + 1, "C: attacked", FONT_LATIN_SMALL_WHITE);
	//	if (serverBuilding && serverBuilding->isBeeingAttacked) font->showText (dest.x + 1, dest.y + 9, "S: attacked", FONT_LATIN_SMALL_YELLOW);
	//	if (building.attacking) font->showText (dest.x + 1, dest.y + 17, "C: attacking", FONT_LATIN_SMALL_WHITE);
	//	if (serverBuilding && serverBuilding->attacking) font->showText (dest.x + 1, dest.y + 25, "S: attacking", FONT_LATIN_SMALL_YELLOW);
	//}
}

//--------------------------------------------------------------------------
void cUnitDrawingEngine::drawUnit (const cVehicle& vehicle, SDL_Rect destination, float zoomFactor, const cMap& map, const cUnitSelection* unitSelection, const cPlayer* player)
{
	// calculate screen position
	int ox = (int)(vehicle.getMovementOffset().x() * zoomFactor);
	int oy = (int)(vehicle.getMovementOffset().y() * zoomFactor);

	destination.x += ox;
	destination.y += oy;

	if (vehicle.getFlightHeight () > 0)
	{
		destination.x += vehicle.ditherX;
		destination.y += vehicle.ditherY;
	}

	SDL_Rect dest;
	dest.x = dest.y = 0;
	bool bDraw = false;
	SDL_Surface* drawingSurface = drawingCache.getCachedImage (vehicle, zoomFactor, map);
	if (drawingSurface == NULL)
	{
		// no cached image found. building needs to be redrawn.
		bDraw = true;
		drawingSurface = drawingCache.createNewEntry (vehicle, zoomFactor, map);
	}

	if (drawingSurface == NULL)
	{
		// image will not be cached. So blitt directly to the screen buffer.
		dest = destination;
		drawingSurface = cVideo::buffer;
	}

	if (bDraw)
	{
		vehicle.render (&map, animationTimer->getAnimationTime(), player, drawingSurface, dest, zoomFactor, cSettings::getInstance ().isShadows ());
	}

	// now check, whether the image has to be blitted to screen buffer
	if (drawingSurface != cVideo::buffer)
	{
		dest = destination;
		SDL_BlitSurface (drawingSurface, NULL, cVideo::buffer, &dest);
	}

	// draw overlay if necessary:
	vehicle.drawOverlayAnimation (animationTimer->getAnimationTime(), cVideo::buffer, destination, zoomFactor);

	// remove the dithering for the following operations
	if (vehicle.getFlightHeight () > 0)
	{
		destination.x -= vehicle.ditherX;
		destination.y -= vehicle.ditherY;
	}

	// remove movement offset for working units
	if (vehicle.isUnitBuildingABuilding () || vehicle.isUnitClearing ())
	{
		destination.x -= ox;
		destination.y -= oy;
	}

	// draw indication, when building is complete
	if (vehicle.isUnitBuildingABuilding () && vehicle.getBuildTurns () == 0 && vehicle.owner == player && !vehicle.BuildPath)
	{
		const cColor finishedMarkColor = cColor::green ();
		const cBox<cPosition> d (cPosition (destination.x + 2, destination.y + 2), cPosition (destination.x + 2 + (vehicle.data.isBig ? 2 * destination.w - 3 : destination.w - 3), destination.y + 2 + (vehicle.data.isBig ? 2 * destination.h - 3 : destination.h - 3)));

		drawRectangle (*cVideo::buffer, d, finishedMarkColor.exchangeGreen (255 - 16 * (animationTimer->getAnimationTime () % 0x8)), 3);
	}

	// Draw the colored frame if necessary
	if (shouldDrawColor)
	{
		const cBox<cPosition> d (cPosition (destination.x + 1, destination.y + 1), cPosition (destination.x + 1 + (vehicle.data.isBig ? 2 * destination.w - 1 : destination.w - 1), destination.y + 1 + (vehicle.data.isBig ? 2 * destination.h - 1 : destination.h - 1)));

		drawRectangle (*cVideo::buffer, d, vehicle.owner->getColor ().getColor ());
	}

	// draw the group selected frame if necessary
	if (unitSelection && unitSelection->getSelectedUnitsCount() > 1 && unitSelection->isSelected (vehicle))
	{
		const cColor groupSelectionColor = cColor::yellow ();
		const cBox<cPosition> d (cPosition (destination.x + 2, destination.y + 2), cPosition (destination.x + 2 + (vehicle.data.isBig ? 2 * destination.w - 3 : destination.w - 3), destination.y + 2 + (vehicle.data.isBig ? 2 * destination.h - 3 : destination.h - 3)));

		drawRectangle (*cVideo::buffer, d, groupSelectionColor, 1);
	}
	// draw the seleted-unit-flash-frame for vehicles
	if (unitSelection && &vehicle == unitSelection->getSelectedVehicle())
	{
		Uint16 maxX = vehicle.data.isBig ? destination.w * 2 : destination.w;
		Uint16 maxY = vehicle.data.isBig ? destination.h * 2 : destination.h;
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
	if (shouldDrawAmmo && (!player || vehicle.owner == player) && vehicle.data.canAttack)
	{
		drawMunBar (vehicle, destination);
	}

	// draw status info
	if (shouldDrawStatus)
	{
		drawStatus (vehicle, destination);
	}

	// attack job debug output
	//if (gameGUI.getAJobDebugStatus ())
	//{
	//	cServer* server = gameGUI.getClient ()->getServer ();
	//	cVehicle* serverVehicle = NULL;
	//	if (server) serverVehicle = server->Map->fields[server->Map->getOffset (PosX, PosY)].getVehicle ();
	//	if (isBeeingAttacked) font->showText (destination.x + 1, destination.y + 1, "C: attacked", FONT_LATIN_SMALL_WHITE);
	//	if (serverVehicle && serverVehicle->isBeeingAttacked) font->showText (destination.x + 1, destination.y + 9, "S: attacked", FONT_LATIN_SMALL_YELLOW);
	//	if (attacking) font->showText (destination.x + 1, destination.y + 17, "C: attacking", FONT_LATIN_SMALL_WHITE);
	//	if (serverVehicle && serverVehicle->attacking) font->showText (destination.x + 1, destination.y + 25, "S: attacking", FONT_LATIN_SMALL_YELLOW);
	//}
}

//--------------------------------------------------------------------------
void cUnitDrawingEngine::drawHealthBar (const cUnit& unit, SDL_Rect destination)
{
	SDL_Rect r1;
	r1.x = destination.x + destination.w / 10 + 1;
	r1.y = destination.y + destination.h / 10;
	r1.w = destination.w * 8 / 10;
	r1.h = destination.h / 8;

	if (unit.data.isBig)
	{
		r1.w += destination.w;
		r1.h *= 2;
	}

	if (r1.h <= 2)
		r1.h = 3;

	SDL_Rect r2;
	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.w = (int)(((float)(r1.w - 2) / unit.data.hitpointsMax) * unit.data.getHitpoints ());
	r2.h = r1.h - 2;

	SDL_FillRect (cVideo::buffer, &r1, 0xFF000000);

	Uint32 color;
	if (unit.data.getHitpoints () > unit.data.hitpointsMax / 2)
		color = 0xFF04AE04; // green
	else if (unit.data.getHitpoints () > unit.data.hitpointsMax / 4)
		color = 0xFFDBDE00; // orange
	else
		color = 0xFFE60000; // red
	SDL_FillRect (cVideo::buffer, &r2, color);
}

//--------------------------------------------------------------------------
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
	r2.w = (int)(((float)(r1.w - 2) / unit.data.ammoMax) * unit.data.getAmmo ());
	r2.h = r1.h - 2;

	SDL_FillRect (cVideo::buffer, &r1, 0xFF000000);

	if (unit.data.getAmmo () > unit.data.ammoMax / 2)
		SDL_FillRect (cVideo::buffer, &r2, 0xFF04AE04);
	else if (unit.data.getAmmo () > unit.data.ammoMax / 4)
		SDL_FillRect (cVideo::buffer, &r2, 0xFFDBDE00);
	else
		SDL_FillRect (cVideo::buffer, &r2, 0xFFE60000);
}

//--------------------------------------------------------------------------
void cUnitDrawingEngine::drawStatus (const cUnit& unit, SDL_Rect destination)
{
	SDL_Rect speedSymbol = {244, 97, 8, 10};
	SDL_Rect shotsSymbol = {254, 97, 5, 10};
	SDL_Rect disabledSymbol = {150, 109, 25, 25};
	SDL_Rect dest;

	if (unit.isDisabled ())
	{
		if (destination.w < 25)
			return;
		dest.x = destination.x + destination.w / 2 - 12;
		dest.y = destination.y + destination.h / 2 - 12;
		SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get (), &disabledSymbol, cVideo::buffer, &dest);
	}
	else
	{
		dest.y = destination.y + destination.h - 11;
		dest.x = destination.x + destination.w / 2 - 4;
		if (unit.data.isBig)
		{
			dest.y += (destination.h / 2);
			dest.x += (destination.w / 2);
		}
		if (unit.data.speedCur >= 4)
		{
			if (unit.data.getShots ())
				dest.x -= destination.w / 4;

			SDL_Rect destCopy = dest;
			SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get (), &speedSymbol, cVideo::buffer, &destCopy);
		}

		dest.x = destination.x + destination.w / 2 - 4;
		if (unit.data.getShots ())
		{
			if (unit.data.speedCur)
				dest.x += destination.w / 4;
			SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get (), &shotsSymbol, cVideo::buffer, &dest);
		}
	}
}

//--------------------------------------------------------------------------
void cUnitDrawingEngine::rotateBlinkColor ()
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
