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

#include "ui/graphical/game/widgets/debugoutputwidget.h"
#include "ui/graphical/game/widgets/gamemapwidget.h"
#include "game/data/player/player.h"
#include "input/mouse/mouse.h"
#include "client.h"
#include "server.h"
#include "video.h"
#include "unifonts.h"
#include "buildings.h"
#include "vehicles.h"

//------------------------------------------------------------------------------
cDebugOutputWidget::cDebugOutputWidget (const cPosition& position, int width) :
	cWidget (position),
	server (nullptr),
	client (nullptr),
	gameMap (nullptr),
	debugAjobs (false),
	debugBaseServer (false),
	debugBaseClient (false),
	debugSentry (false),
	debugFX (false),
	debugTraceServer (false),
	debugTraceClient (false),
	debugPlayers (false),
	debugCache (false),
	debugSync (false)
{
	resize (cPosition (width, 0));
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setClient (const cClient* client_)
{
	client = client_;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setServer (const cServer* server_)
{
	server = server_;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setGameMap (const cGameMapWidget* gameMap_)
{
	gameMap = gameMap_;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setDebugAjobs (bool value)
{
	debugAjobs = value;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setDebugBaseServer (bool value)
{
	debugBaseServer = value;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setDebugBaseClient (bool value)
{
	debugBaseClient = value;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setDebugSentry (bool value)
{
	debugSentry = value;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setDebugFX (bool value)
{
	debugFX = value;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setDebugTraceServer (bool value)
{
	debugTraceServer = value;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setDebugTraceClient (bool value)
{
	debugTraceClient = value;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setDebugPlayers (bool value)
{
	debugPlayers = value;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setDebugCache (bool value)
{
	debugCache = value;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setDebugSync (bool value)
{
	debugSync = value;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::draw ()
{
	if (!client) return;

	const cPlayer& player = client->getActivePlayer ();

	auto drawPositionX = getEndPosition().x() - 200;
	auto drawPositionY = getPosition ().y ();

	if (debugPlayers)
	{
		font->showText (drawPositionX, drawPositionY, "Players: " + iToStr ((int)client->getPlayerList ().size ()), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		SDL_Rect rDest = {Sint16 (drawPositionX), Sint16 (drawPositionY), 20, 10};
		SDL_Rect rSrc = {0, 0, 20, 10};
		SDL_Rect rDotDest = {Sint16 (drawPositionX - 10), Sint16 (drawPositionY), 10, 10};
		SDL_Rect rBlackOut = {Sint16 (drawPositionX + 20), Sint16 (drawPositionY), 0, 10};
		const auto& playerList = client->getPlayerList ();
		for (size_t i = 0; i != playerList.size (); ++i)
		{
			// HACK SHOWFINISHEDPLAYERS
			SDL_Rect rDot = {10, 0, 10, 10}; // for green dot

			if (playerList[i]->getHasFinishedTurn() /* && playerList[i] != &player*/)
			{
				SDL_BlitSurface (GraphicsData.gfx_player_ready.get (), &rDot, cVideo::buffer, &rDotDest);
			}
#if 0
			else if (playerList[i] == &player && client->bWantToEnd)
			{
				SDL_BlitSurface (GraphicsData.gfx_player_ready.get (), &rDot, cVideo::buffer, &rDotDest);
			}
#endif
			else
			{
				rDot.x = 0; // for red dot
				SDL_BlitSurface (GraphicsData.gfx_player_ready.get (), &rDot, cVideo::buffer, &rDotDest);
			}

			SDL_BlitSurface (playerList[i]->getColor().getTexture (), &rSrc, cVideo::buffer, &rDest);
			if (playerList[i].get() == &player)
			{
				std::string sTmpLine = " " + playerList[i]->getName () + ", nr: " + iToStr (playerList[i]->getNr ()) + " << you! ";
				// black out background for better recognizing
				rBlackOut.w = font->getTextWide (sTmpLine, FONT_LATIN_SMALL_WHITE);
				SDL_FillRect (cVideo::buffer, &rBlackOut, 0xFF000000);
				font->showText (rBlackOut.x, drawPositionY + 1, sTmpLine, FONT_LATIN_SMALL_WHITE);
			}
			else
			{
				std::string sTmpLine = " " + playerList[i]->getName () + ", nr: " + iToStr (playerList[i]->getNr ()) + " ";
				// black out background for better recognizing
				rBlackOut.w = font->getTextWide (sTmpLine, FONT_LATIN_SMALL_WHITE);
				SDL_FillRect (cVideo::buffer, &rBlackOut, 0xFF000000);
				font->showText (rBlackOut.x, drawPositionY + 1, sTmpLine, FONT_LATIN_SMALL_WHITE);
			}
			// use 10 for pixel high of dots instead of text high
			drawPositionY += 10;
			rDest.y = rDotDest.y = rBlackOut.y = drawPositionY;
		}
	}

	if (debugAjobs)
	{
		font->showText (drawPositionX, drawPositionY, "ClientAttackJobs: " + iToStr ((int)client->attackJobs.size ()), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		if (server)
		{
			font->showText (drawPositionX, drawPositionY, "ServerAttackJobs: " + iToStr ((int)server->AJobs.size ()), FONT_LATIN_SMALL_WHITE);
			drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		}
	}

	if (debugBaseClient)
	{
		font->showText (drawPositionX, drawPositionY, "subbases: " + iToStr ((int)player.base.SubBases.size ()), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
	}

	if (debugBaseServer && server)
	{
		const auto& serverPlayer = server->getPlayerFromNumber (player.getNr ());
		font->showText (drawPositionX, drawPositionY, "subbases: " + iToStr ((int)serverPlayer.base.SubBases.size ()), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
	}

	if (debugFX)
	{
#if 0
		font->showText (drawPositionX, drawPositionY, "fx-count: " + iToStr ((int)FXList.size () + (int)FXListBottom.size ()), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		font->showText (drawPositionX, drawPositionY, "wind-dir: " + iToStr ((int)(fWindDir * 57.29577f)), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
#endif
	}
	if (debugTraceServer || debugTraceClient)
	{
		trace ();
	}
	if (debugCache && gameMap)
	{
		const auto& drawingCache = gameMap->getDrawingCache();

		font->showText (getPosition ().x (), drawPositionY, "Max cache size: " + iToStr (drawingCache.getMaxCacheSize ()), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		font->showText (getPosition ().x (), drawPositionY, "cache size: " + iToStr (drawingCache.getCacheSize ()), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		font->showText (getPosition ().x (), drawPositionY, "cache hits: " + iToStr (drawingCache.getCacheHits ()), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		font->showText (getPosition ().x (), drawPositionY, "cache misses: " + iToStr (drawingCache.getCacheMisses ()), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		font->showText (getPosition ().x (), drawPositionY, "not cached: " + iToStr (drawingCache.getNotCached ()), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
	}

	if (debugSync)
	{
		font->showText (drawPositionX - 20, drawPositionY, "Sync debug:", FONT_LATIN_SMALL_YELLOW);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		if (server)
		{
			font->showText (drawPositionX - 10, drawPositionY, "-Server:", FONT_LATIN_SMALL_YELLOW);
			drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
			font->showText (drawPositionX, drawPositionY, "Server Time: ", FONT_LATIN_SMALL_WHITE);
			font->showText (drawPositionX + 110, drawPositionY, iToStr (server->gameTimer->gameTime), FONT_LATIN_SMALL_WHITE);
			drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

			font->showText (drawPositionX, drawPositionY, "Net MSG Queue: ", FONT_LATIN_SMALL_WHITE);
			font->showText (drawPositionX + 110, drawPositionY, iToStr (server->eventQueue.safe_size ()), FONT_LATIN_SMALL_WHITE);
			drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

			font->showText (drawPositionX, drawPositionY, "EventCounter: ", FONT_LATIN_SMALL_WHITE);
			font->showText (drawPositionX + 110, drawPositionY, iToStr (server->gameTimer->eventCounter), FONT_LATIN_SMALL_WHITE);
			drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

			font->showText (drawPositionX, drawPositionY, "-Client Lag: ", FONT_LATIN_SMALL_WHITE);
			drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

			for (size_t i = 0; i != server->playerList.size (); ++i)
			{
				eUnicodeFontType fontType = FONT_LATIN_SMALL_WHITE;
				if (server->gameTimer->getReceivedTime (i) + PAUSE_GAME_TIMEOUT < server->gameTimer->gameTime)
					fontType = FONT_LATIN_SMALL_RED;
				font->showText (drawPositionX + 10, drawPositionY, "Client " + iToStr (i) + ": ", fontType);
				font->showText (drawPositionX + 110, drawPositionY, iToStr (server->gameTimer->gameTime - server->gameTimer->getReceivedTime (i)), fontType);
				drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
			}
		}

		font->showText (drawPositionX - 10, drawPositionY, "-Client:", FONT_LATIN_SMALL_YELLOW);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		eUnicodeFontType fontType = FONT_LATIN_SMALL_GREEN;
		if (client->gameTimer->debugRemoteChecksum != client->gameTimer->localChecksum)
			fontType = FONT_LATIN_SMALL_RED;
		font->showText (drawPositionX, drawPositionY, "Server Checksum: ", FONT_LATIN_SMALL_WHITE);
		font->showText (drawPositionX + 110, drawPositionY, "0x" + iToHex (client->gameTimer->debugRemoteChecksum), fontType);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (drawPositionX, drawPositionY, "Client Checksum: ", FONT_LATIN_SMALL_WHITE);
		font->showText (drawPositionX + 110, drawPositionY, "0x" + iToHex (client->gameTimer->localChecksum), fontType);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (drawPositionX, drawPositionY, "Client Time: ", FONT_LATIN_SMALL_WHITE);
		font->showText (drawPositionX + 110, drawPositionY, iToStr (client->gameTimer->gameTime), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (drawPositionX, drawPositionY, "Net MGS Queue: ", FONT_LATIN_SMALL_WHITE);
		font->showText (drawPositionX + 110, drawPositionY, iToStr (client->eventQueue.safe_size ()), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (drawPositionX, drawPositionY, "EventCounter: ", FONT_LATIN_SMALL_WHITE);
		font->showText (drawPositionX + 110, drawPositionY, iToStr (client->gameTimer->eventCounter), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (drawPositionX, drawPositionY, "Time Buffer: ", FONT_LATIN_SMALL_WHITE);
		font->showText (drawPositionX + 110, drawPositionY, iToStr (client->gameTimer->getReceivedTime () - client->gameTimer->gameTime), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (drawPositionX, drawPositionY, "Ticks per Frame ", FONT_LATIN_SMALL_WHITE);
		static unsigned int lastGameTime = 0;
		font->showText (drawPositionX + 110, drawPositionY, iToStr (client->gameTimer->gameTime - lastGameTime), FONT_LATIN_SMALL_WHITE);
		lastGameTime = client->gameTimer->gameTime;
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (drawPositionX, drawPositionY, "Time Adjustment: ", FONT_LATIN_SMALL_WHITE);
		font->showText (drawPositionX + 110, drawPositionY, iToStr (client->gameTimer->gameTimeAdjustment), FONT_LATIN_SMALL_WHITE);
		static int totalAdjust = 0;
		totalAdjust += client->gameTimer->gameTimeAdjustment;
		client->gameTimer->gameTimeAdjustment = 0;
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (drawPositionX, drawPositionY, "TotalAdj.: ", FONT_LATIN_SMALL_WHITE);
		font->showText (drawPositionX + 110, drawPositionY, iToStr (totalAdjust), FONT_LATIN_SMALL_WHITE);
		drawPositionY += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
	}
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::trace ()
{
	if (!gameMap) return;

	auto mouse = gameMap->getActiveMouse ();
	if (!mouse) return;

	if (!gameMap->getArea ().withinOrTouches (mouse->getPosition ())) return;

	const auto mapPosition = gameMap->getMapTilePosition (mouse->getPosition ());

	cMapField* field;

	if (debugTraceServer && server)
	{
		if (!server->Map->isValidPosition (mapPosition)) return;
		field = &server->Map->getField (mapPosition);
	}
	else
	{
		if (!client->getMap ()->isValidPosition (mapPosition)) return;
		field = &client->Map->getField (mapPosition);
	}

	cPosition drawingPosition = getPosition () + cPosition(0,0);

	if (field->getVehicle ()) { traceVehicle (*field->getVehicle (), drawingPosition); drawingPosition.y() += 20; }
	if (field->getPlane ()) { traceVehicle (*field->getPlane (), drawingPosition); drawingPosition.y () += 20; }
	const auto& buildings = field->getBuildings ();
	for (auto it = buildings.begin (); it != buildings.end (); ++it)
	{
		traceBuilding (**it, drawingPosition); drawingPosition.y () += 20;
	}
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::traceVehicle (const cVehicle& vehicle, cPosition& drawPosition)
{
	std::string tmpString;

	tmpString = "name: \"" + vehicle.getDisplayName () + "\" id: \"" + iToStr (vehicle.iID) + "\" owner: \"" + vehicle.owner->getName () + "\" posX: +" + iToStr (vehicle.getPosition().x()) + " posY: " + iToStr (vehicle.getPosition().y()) + " offX: " + iToStr (vehicle.getMovementOffset().x()) + " offY: " + iToStr (vehicle.getMovementOffset().y());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = "dir: " + iToStr (vehicle.dir) + " moving: +" + iToStr (vehicle.isUnitMoving()) + " mjob: " + pToStr (vehicle.getClientMoveJob()) + " speed: " + iToStr (vehicle.data.speedCur) + " mj_active: " + iToStr (vehicle.MoveJobActive);
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = " attacking: " + iToStr (vehicle.isAttacking()) + " on sentry: +" + iToStr (vehicle.isSentryActive()) + " ditherx: " + iToStr (vehicle.ditherX) + " dithery: " + iToStr (vehicle.ditherY);
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = "is_building: " + iToStr (vehicle.isUnitBuildingABuilding()) + " building_typ: " + vehicle.getBuildingType().getText () + " build_costs: +" + iToStr (vehicle.getBuildCosts()) + " build_rounds: " + iToStr (vehicle.getBuildTurns()) + " build_round_start: " + iToStr (vehicle.getBuildTurnsStart());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = " bandx: " + iToStr (vehicle.bandPosition.x()) + " bandy: +" + iToStr (vehicle.bandPosition.y ()) + " build_big_saved_pos: " + iToStr (vehicle.buildBigSavedPosition.x ()) + "x" + iToStr (vehicle.buildBigSavedPosition.y ()) + " build_path: " + iToStr (vehicle.BuildPath);
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = " is_clearing: " + iToStr (vehicle.isUnitClearing()) + " clearing_rounds: +" + iToStr (vehicle.getClearingTurns()) + " clear_big: " + iToStr (vehicle.data.isBig) + " loaded: " + iToStr (vehicle.isUnitLoaded());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = "commando_rank: " + fToStr (Round (vehicle.getCommandoRank(), 2)) + " disabled: " + iToStr (vehicle.getDisabledTurns());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = "clear_mines: +" + iToStr (vehicle.isUnitClearingMines()) + " lay_mines: " + iToStr (vehicle.isUnitLayingMines());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = " stored_vehicles_count: " + iToStr ((int)vehicle.storedUnits.size ());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	for (size_t i = 0; i != vehicle.storedUnits.size (); ++i)
	{
		const cVehicle* storedVehicle = vehicle.storedUnits[i];
		font->showText (drawPosition, " store " + iToStr (i) + ": \"" + storedVehicle->getDisplayName () + "\"", FONT_LATIN_SMALL_WHITE);
		drawPosition.y() += 8;
	}

	if (debugTraceServer)
	{
		tmpString = "seen by players: owner";
		for (size_t i = 0; i != vehicle.seenByPlayerList.size (); ++i)
		{
			tmpString += ", \"" + vehicle.seenByPlayerList[i]->getName () + "\"";
		}
		font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
		drawPosition.y() += 8;
	}

	tmpString = "flight height: " + iToStr (vehicle.getFlightHeight());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::traceBuilding (const cBuilding& building, cPosition& drawPosition)
{
	std::string tmpString;

	tmpString = "name: \"" + building.getDisplayName () + "\" id: \"" + iToStr (building.iID) + "\" owner: \"" + (building.owner ? building.owner->getName () : "<null>") + "\" posX: +" + iToStr (building.getPosition().x()) + " posY: " + iToStr (building.getPosition().y());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = "dir: " + iToStr (building.dir) + " on sentry: +" + iToStr (building.isSentryActive()) + " sub_base: " + pToStr (building.SubBase);
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = "attacking: " + iToStr (building.isAttacking()) + " UnitsData.dirt_typ: " + iToStr (building.RubbleTyp) + " UnitsData.dirt_value: +" + iToStr (building.RubbleValue) + " big_dirt: " + iToStr (building.data.isBig) + " is_working: " + iToStr (building.isUnitWorking());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = " max_metal_p: " + iToStr (building.MaxMetalProd) +
		" max_oil_p: " + iToStr (building.MaxOilProd) +
		" max_gold_p: " + iToStr (building.MaxGoldProd);
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = "disabled: " + iToStr (building.getDisabledTurns());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = " stored_vehicles_count: " + iToStr ((int)building.storedUnits.size ());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	for (size_t i = 0; i != building.storedUnits.size (); ++i)
	{
		const cVehicle* storedVehicle = building.storedUnits[i];
		font->showText (drawPosition, " store " + iToStr (i) + ": \"" + storedVehicle->getDisplayName () + "\"", FONT_LATIN_SMALL_WHITE);
		drawPosition.y() += 8;
	}

	const size_t buildingBuildListSize = building.getBuildListSize ();
	tmpString =
		"build_speed: "        + iToStr (building.BuildSpeed) +
		" repeat_build: "      + iToStr (building.RepeatBuild) +
		" build_list_count: +" + iToStr ((int)buildingBuildListSize);
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	for (size_t i = 0; i != buildingBuildListSize; ++i)
	{
		const auto& item = building.getBuildListItem(i);
		font->showText (drawPosition, "  build " + iToStr (i) + ": " + item.getType ().getText () + " \"" + item.getType().getUnitDataOriginalVersion ()->name + "\"", FONT_LATIN_SMALL_WHITE);
		drawPosition.y() += 8;
	}

	if (debugTraceServer)
	{
		tmpString = "seen by players: owner";
		for (size_t i = 0; i != building.seenByPlayerList.size (); ++i)
		{
			tmpString += ", \"" + building.seenByPlayerList[i]->getName () + "\"";
		}
		font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
		drawPosition.y() += 8;
	}
}