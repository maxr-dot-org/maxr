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
#include "ui/graphical/game/animations/animation.h"
#include "game/data/player/player.h"
#include "input/mouse/mouse.h"
#include "game/logic/client.h"
#include "game/logic/server2.h"
#include "video.h"
#include "utility/unifonts.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "utility/mathtools.h"
#include "utility/string/toString.h"
#include "utility/indexiterator.h"
#include "utility/language.h"
#include "utility/listhelpers.h"

//------------------------------------------------------------------------------
cDebugOutputWidget::cDebugOutputWidget (const cBox<cPosition>& area) :
	cWidget (area),
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
	debugSync (false),
	debugStealth (false)
{}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setClient (const cClient* client_)
{
	client = client_;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setServer (const cServer2* server_)
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
void cDebugOutputWidget::setDebugStealth(bool value)
{
	debugStealth = value;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	cWidget::draw (destination, clipRect);

	if (!client) return;

	auto font = cUnicodeFont::font.get();

	const cPlayer& player = client->getActivePlayer();

	setPrintPosition(cPosition(getEndPosition().x() - 200, getPosition().y()));

	if (debugPlayers)
	{
		print("Players: " + toString(client->model.getPlayerList().size()));

		SDL_Rect rDest = {drawPosition.x(), drawPosition.y(), 20, 10};
		SDL_Rect rSrc = {0, 0, 20, 10};
		SDL_Rect rDotDest  = {drawPosition.x() - 10, drawPosition.y(), 10, 10};
		SDL_Rect rBlackOut = {drawPosition.x() + 20, drawPosition.y(),  0, 10};
		const auto& playerList = client->model.getPlayerList();
		for (size_t i = 0; i != playerList.size(); ++i)
		{
			// HACK SHOWFINISHEDPLAYERS
			SDL_Rect rDot = {10, 0, 10, 10}; // for green dot

			if (playerList[i]->getHasFinishedTurn() /* && playerList[i] != &player*/)
			{
				SDL_BlitSurface (GraphicsData.gfx_player_ready.get(), &rDot, &destination, &rDotDest);
			}
#if 0
			else if (playerList[i] == &player && client->bWantToEnd)
			{
				SDL_BlitSurface (GraphicsData.gfx_player_ready.get(), &rDot, &destination, &rDotDest);
			}
#endif
			else
			{
				rDot.x = 0; // for red dot
				SDL_BlitSurface (GraphicsData.gfx_player_ready.get(), &rDot, &destination, &rDotDest);
			}

			SDL_BlitSurface (playerList[i]->getColor().getTexture(), &rSrc, &destination, &rDest);
			if (playerList[i].get() == &player)
			{
				std::string sTmpLine = " " + playerList[i]->getName() + ", nr: " + iToStr (playerList[i]->getId()) + " << you! ";
				// black out background for better recognizing
				rBlackOut.w = font->getTextWide (sTmpLine, FONT_LATIN_SMALL_WHITE);
				SDL_FillRect (&destination, &rBlackOut, 0xFF000000);
				font->showText (rBlackOut.x, drawPosition.y() + 1, sTmpLine, FONT_LATIN_SMALL_WHITE);
			}
			else
			{
				std::string sTmpLine = " " + playerList[i]->getName() + ", nr: " + iToStr (playerList[i]->getId()) + " ";
				// black out background for better recognizing
				rBlackOut.w = font->getTextWide (sTmpLine, FONT_LATIN_SMALL_WHITE);
				SDL_FillRect (&destination, &rBlackOut, 0xFF000000);
				font->showText (rBlackOut.x, drawPosition.y() + 1, sTmpLine, FONT_LATIN_SMALL_WHITE);
			}
			// use 10 for pixel high of dots instead of text high
			drawPosition.y() += 10;
			rDest.y = rDotDest.y = rBlackOut.y = drawPosition.y();
		}
	}

	if (debugAjobs)
	{
		print("ClientAttackJobs: " + toString(client->getModel().attackJobs.size()));
		if (server)
		{
			print("ServerAttackJobs: " + toString(server->getModel().attackJobs.size()));
		}
	}

	if (debugBaseClient)
	{
		print("subbases: " + toString(player.base.SubBases.size()));
	}

	if (debugBaseServer && server)
	{
		const auto serverPlayer = server->model.getPlayer (player.getId());
		print("subbases: " + toString(serverPlayer->base.SubBases.size()));
	}

	if (debugFX)
	{
		if (gameMap)
		{
			print("total-animations-count: " + iToStr (gameMap->animations.size()));

			const auto runningAnimations = std::count_if (gameMap->animations.cbegin(), gameMap->animations.cend(), [ ] (const std::unique_ptr<cAnimation>& animation) { return animation->isRunning(); });
			print("running-animations-count: " + iToStr (runningAnimations));
			const auto finishedAnimations = std::count_if (gameMap->animations.cbegin(), gameMap->animations.cend(), [] (const std::unique_ptr<cAnimation>& animation) { return animation->isFinished(); });
			print("finished-animations-count: " + iToStr (finishedAnimations));
			print("gui-fx-count: " + iToStr (gameMap->effects.size()));
		}
		print("client-fx-count: " + iToStr (client->getModel().effectsList.size()));
	}
	if (debugTraceServer || debugTraceClient)
	{
		trace();
	}
	if (debugCache && gameMap)
	{
		const auto& drawingCache = gameMap->getDrawingCache();

		print("Max cache size: " + iToStr (drawingCache.getMaxCacheSize()));
		print("cache size: " + iToStr (drawingCache.getCacheSize()));
		print("cache hits: " + iToStr (drawingCache.getCacheHits()));
		print("cache misses: " + iToStr (drawingCache.getCacheMisses()));
		print("not cached: " + iToStr (drawingCache.getNotCached()));
	}

	if (debugSync)
	{
		font->showText(drawPosition.x() - 10, drawPosition.y(), "-Client:", FONT_LATIN_SMALL_YELLOW);
		drawPosition.y() += font->getFontHeight(FONT_LATIN_SMALL_YELLOW);
		if (server)
		{
			print("-Server:");
			font->showText (drawPosition.x(), drawPosition.y(), "Server Time: ", FONT_LATIN_SMALL_WHITE);
			font->showText(drawPosition.x() + 110, drawPosition.y(), iToStr(server->model.getGameTime()), FONT_LATIN_SMALL_WHITE);
			drawPosition.y() += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

			font->showText (drawPosition.x(), drawPosition.y(), "Net MSG Queue: ", FONT_LATIN_SMALL_WHITE);
			font->showText (drawPosition.x() + 110, drawPosition.y(), iToStr (server->eventQueue.safe_size()), FONT_LATIN_SMALL_WHITE);
			drawPosition.y() += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

			font->showText (drawPosition.x(), drawPosition.y(), "EventCounter: ", FONT_LATIN_SMALL_WHITE);
			font->showText (drawPosition.x() + 110, drawPosition.y(), iToStr (server->gameTimer.eventCounter), FONT_LATIN_SMALL_WHITE);
			drawPosition.y() += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

			for (const auto& player : server->model.playerList)
			{
				font->showText(drawPosition.x(), drawPosition.y(), "Client " + iToStr(player->getId()) + lngPack.i18n("Text~Punctuation~Colon"), FONT_LATIN_SMALL_WHITE);
				drawPosition.y() += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

				font->showText(drawPosition.x() + 10, drawPosition.y(), "Client time: ", FONT_LATIN_SMALL_WHITE);
				font->showText(drawPosition.x() + 110, drawPosition.y(), iToStr(server->gameTimer.receivedTime.at(player->getId())), FONT_LATIN_SMALL_WHITE);
				drawPosition.y() += font->getFontHeight(FONT_LATIN_SMALL_WHITE);


				if (server->gameTimer.clientDebugData.at(player->getId()).crcOK)
					font->showText(drawPosition.x() + 10, drawPosition.y(), "Sync OK", FONT_LATIN_SMALL_GREEN);
				else
					font->showText(drawPosition.x() + 10, drawPosition.y(), "Out of Sync!", FONT_LATIN_SMALL_RED);
				drawPosition.y() += font->getFontHeight(FONT_LATIN_SMALL_WHITE);

				const auto& debugData= server->gameTimer.clientDebugData.at(player->getId());
				font->showText(drawPosition.x() + 10, drawPosition.y(), "Timebuffer: ", FONT_LATIN_SMALL_WHITE);
				font->showText(drawPosition.x() + 110, drawPosition.y(), iToStr((int)debugData.timeBuffer), FONT_LATIN_SMALL_WHITE);
				drawPosition.y() += font->getFontHeight(FONT_LATIN_SMALL_WHITE);

				font->showText(drawPosition.x() + 10, drawPosition.y(), "Ticks per Frame: ", FONT_LATIN_SMALL_WHITE);
				font->showText(drawPosition.x() + 110, drawPosition.y(), iToStr((int)debugData.ticksPerFrame), FONT_LATIN_SMALL_WHITE);
				drawPosition.y() += font->getFontHeight(FONT_LATIN_SMALL_WHITE);

				font->showText(drawPosition.x() + 10, drawPosition.y(), "Queue Size: ", FONT_LATIN_SMALL_WHITE);
				font->showText(drawPosition.x() + 110, drawPosition.y(), iToStr((int)debugData.queueSize), FONT_LATIN_SMALL_WHITE);
				drawPosition.y() += font->getFontHeight(FONT_LATIN_SMALL_WHITE);

				font->showText(drawPosition.x() + 10, drawPosition.y(), "Event counter: ", FONT_LATIN_SMALL_WHITE);
				font->showText(drawPosition.x() + 110, drawPosition.y(), iToStr((int)debugData.eventCounter), FONT_LATIN_SMALL_WHITE);
				drawPosition.y() += font->getFontHeight(FONT_LATIN_SMALL_WHITE);

				font->showText(drawPosition.x() + 10, drawPosition.y(), "Ping (ms): ", FONT_LATIN_SMALL_WHITE);
				font->showText(drawPosition.x() + 110, drawPosition.y(), iToStr((int)debugData.ping), FONT_LATIN_SMALL_WHITE);
				drawPosition.y() += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
			}
		}

		font->showText (drawPosition.x() - 10, drawPosition.y(), "-Client:", FONT_LATIN_SMALL_YELLOW);
		drawPosition.y() += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		eUnicodeFontType fontType = FONT_LATIN_SMALL_GREEN;
		if (client->gameTimer->debugRemoteChecksum != client->gameTimer->localChecksum)
			fontType = FONT_LATIN_SMALL_RED;
		font->showText (drawPosition.x(), drawPosition.y(), "Server Checksum: ", FONT_LATIN_SMALL_WHITE);
		font->showText (drawPosition.x() + 110, drawPosition.y(), "0x" + iToHex (client->gameTimer->debugRemoteChecksum), fontType);
		drawPosition.y() += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (drawPosition.x(), drawPosition.y(), "Client Checksum: ", FONT_LATIN_SMALL_WHITE);
		font->showText (drawPosition.x() + 110, drawPosition.y(), "0x" + iToHex (client->gameTimer->localChecksum), fontType);
		drawPosition.y() += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (drawPosition.x(), drawPosition.y(), "Client Time: ", FONT_LATIN_SMALL_WHITE);
		font->showText(drawPosition.x() + 110, drawPosition.y(), iToStr(client->model.getGameTime()), FONT_LATIN_SMALL_WHITE);
		drawPosition.y() += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (drawPosition.x(), drawPosition.y(), "Net MGS Queue: ", FONT_LATIN_SMALL_WHITE);
		font->showText (drawPosition.x() + 110, drawPosition.y(), iToStr (client->eventQueue2.safe_size()), FONT_LATIN_SMALL_WHITE);
		drawPosition.y() += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (drawPosition.x(), drawPosition.y(), "EventCounter: ", FONT_LATIN_SMALL_WHITE);
		font->showText (drawPosition.x() + 110, drawPosition.y(), iToStr (client->gameTimer->eventCounter), FONT_LATIN_SMALL_WHITE);
		drawPosition.y() += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (drawPosition.x(), drawPosition.y(), "Time Buffer: ", FONT_LATIN_SMALL_WHITE);
		font->showText(drawPosition.x() + 110, drawPosition.y(), iToStr(client->gameTimer->getReceivedTime() - client->model.getGameTime()), FONT_LATIN_SMALL_WHITE);
		drawPosition.y() += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (drawPosition.x(), drawPosition.y(), "Ticks per Frame ", FONT_LATIN_SMALL_WHITE);
		static unsigned int lastGameTime = 0;
		font->showText(drawPosition.x() + 110, drawPosition.y(), iToStr(client->model.getGameTime() - lastGameTime), FONT_LATIN_SMALL_WHITE);
		lastGameTime = client->model.getGameTime();
		drawPosition.y() += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText(drawPosition.x(), drawPosition.y(), "Ping: ", FONT_LATIN_SMALL_WHITE);
		font->showText(drawPosition.x() + 110, drawPosition.y(), iToStr(client->gameTimer->ping), FONT_LATIN_SMALL_WHITE);
		drawPosition.y() += font->getFontHeight(FONT_LATIN_SMALL_WHITE);

	}

	if (debugStealth)
	{
		drawDetectedByPlayerList();
		drawDetectionMaps();
	}
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::setPrintPosition(cPosition position)
{
	drawPosition = position;
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::print(const std::string& text, eUnicodeFontType font_ /*= FONT_LATIN_SMALL_WHITE*/)
{
	cUnicodeFont::font->showText(drawPosition.x(), drawPosition.y(), text, font_);
	drawPosition.y() += cUnicodeFont::font->getFontHeight(font_);
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::trace()
{
	if (!gameMap) return;

	auto mouse = gameMap->getActiveMouse();
	if (!mouse) return;

	if (!gameMap->getArea().withinOrTouches (mouse->getPosition())) return;

	const auto mapPosition = gameMap->getMapTilePosition (mouse->getPosition());

	const cMapField* field;

	if (debugTraceServer && server)
	{
		if (!server->getModel().getMap()->isValidPosition (mapPosition)) return;
		field = &server->getModel().getMap()->getField (mapPosition);
	}
	else
	{
		if (!client->getModel().getMap()->isValidPosition (mapPosition)) return;
		field = &client->getModel().getMap()->getField (mapPosition);
	}

	cPosition drawingPosition = getPosition() + cPosition (0, 0);

	if (field->getVehicle()) { traceVehicle (*field->getVehicle(), drawingPosition); drawingPosition.y() += 20; }
	if (field->getPlane()) { traceVehicle (*field->getPlane(), drawingPosition); drawingPosition.y() += 20; }
	const auto& buildings = field->getBuildings();
	for (auto it = buildings.begin(); it != buildings.end(); ++it)
	{
		traceBuilding (**it, drawingPosition); drawingPosition.y() += 20;
	}
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::traceVehicle (const cVehicle& vehicle, cPosition& drawPosition)
{
	std::string tmpString;

	auto font = cUnicodeFont::font.get();

	tmpString = "name: \"" + vehicle.getDisplayName() + "\" id: \"" + iToStr (vehicle.iID) + "\" owner: \"" + vehicle.getOwner()->getName() + "\" posX: +" + iToStr (vehicle.getPosition().x()) + " posY: " + iToStr (vehicle.getPosition().y()) + " offX: " + iToStr (vehicle.getMovementOffset().x()) + " offY: " + iToStr (vehicle.getMovementOffset().y());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = "dir: " + iToStr (vehicle.dir) + " moving: +" + iToStr (vehicle.isUnitMoving()) + " mjob: " + pToStr (vehicle.getMoveJob()) + " speed: " + iToStr (vehicle.data.getSpeed());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = " attacking: " + iToStr (vehicle.isAttacking()) + " on sentry: +" + iToStr (vehicle.isSentryActive()) + " ditherx: " + iToStr (vehicle.ditherX) + " dithery: " + iToStr (vehicle.ditherY);
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = "is_building: " + iToStr (vehicle.isUnitBuildingABuilding()) + " building_typ: " + vehicle.getBuildingType().getText() + " build_costs: +" + iToStr (vehicle.getBuildCosts()) + " build_rounds: " + iToStr (vehicle.getBuildTurns()) + " build_round_start: " + iToStr (vehicle.getBuildTurnsStart());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = " bandx: " + iToStr (vehicle.bandPosition.x()) + " bandy: +" + iToStr (vehicle.bandPosition.y()) + " build_big_saved_pos: " + iToStr (vehicle.buildBigSavedPosition.x()) + "x" + iToStr (vehicle.buildBigSavedPosition.y()) + " build_path: " + iToStr (vehicle.BuildPath);
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = " is_clearing: " + iToStr (vehicle.isUnitClearing()) + " clearing_rounds: +" + iToStr (vehicle.getClearingTurns()) + " clear_big: " + iToStr (vehicle.getIsBig()) + " loaded: " + iToStr (vehicle.isUnitLoaded());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = "commando_rank: " + fToStr (Round (vehicle.getCommandoRank(), 2)) + " disabled: " + iToStr (vehicle.getDisabledTurns());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = "clear_mines: +" + iToStr (vehicle.isUnitClearingMines()) + " lay_mines: " + iToStr (vehicle.isUnitLayingMines());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = " stored_vehicles_count: " + iToStr ((int)vehicle.storedUnits.size());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	for (size_t i = 0; i != vehicle.storedUnits.size(); ++i)
	{
		const cVehicle* storedVehicle = vehicle.storedUnits[i];
		font->showText (drawPosition, " store " + iToStr (i) + ": \"" + storedVehicle->getDisplayName() + "\"", FONT_LATIN_SMALL_WHITE);
		drawPosition.y() += 8;
	}

	if (debugTraceServer)
	{
		tmpString = "seen by players: owner";
		for (size_t i = 0; i != vehicle.seenByPlayerList.size(); ++i)
		{
			tmpString += ", \"" + vehicle.seenByPlayerList[i]->getName() + "\"";
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

	auto font = cUnicodeFont::font.get();

	tmpString = "name: \"" + building.getDisplayName() + "\" id: \"" + iToStr (building.iID) + "\" owner: \"" + (building.getOwner() ? building.getOwner()->getName() : "<null>") + "\" posX: +" + iToStr (building.getPosition().x()) + " posY: " + iToStr (building.getPosition().y());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = "dir: " + iToStr (building.dir) + " on sentry: +" + iToStr (building.isSentryActive()) + " sub_base: " + pToStr (building.subBase);
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = "attacking: " + iToStr (building.isAttacking()) + " UnitsData.dirt_typ: " + iToStr (building.rubbleTyp) + " UnitsData.dirt_value: +" + iToStr (building.rubbleValue) + " big_dirt: " + iToStr (building.getIsBig()) + " is_working: " + iToStr (building.isUnitWorking());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = " max_metal_p: " + iToStr (building.maxMetalProd) +
				" max_oil_p: " + iToStr (building.maxOilProd) +
				" max_gold_p: " + iToStr (building.maxGoldProd);
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = "disabled: " + iToStr (building.getDisabledTurns());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	tmpString = " stored_vehicles_count: " + iToStr ((int)building.storedUnits.size());
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	for (size_t i = 0; i != building.storedUnits.size(); ++i)
	{
		const cVehicle* storedVehicle = building.storedUnits[i];
		font->showText (drawPosition, " store " + iToStr (i) + ": \"" + storedVehicle->getDisplayName() + "\"", FONT_LATIN_SMALL_WHITE);
		drawPosition.y() += 8;
	}

	const size_t buildingBuildListSize = building.getBuildListSize();
	tmpString =
		"build_speed: "        + iToStr (building.getBuildSpeed()) +
		" repeat_build: "      + iToStr (building.getRepeatBuild()) +
		" build_list_count: +" + iToStr ((int)buildingBuildListSize);
	font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
	drawPosition.y() += 8;

	for (size_t i = 0; i != buildingBuildListSize; ++i)
	{
		const auto& item = building.getBuildListItem (i);
		font->showText (drawPosition, "  build " + iToStr (i) + lngPack.i18n ("Text~Punctuation~Colon") + item.getType().getText() + " \"" + client->getModel().getUnitsData()->getStaticUnitData(item.getType()).getName() + "\"", FONT_LATIN_SMALL_WHITE);
		drawPosition.y() += 8;
	}

	if (debugTraceServer)
	{
		tmpString = "seen by players: owner";
		for (size_t i = 0; i != building.seenByPlayerList.size(); ++i)
		{
			tmpString += ", \"" + building.seenByPlayerList[i]->getName() + "\"";
		}
		font->showText (drawPosition, tmpString, FONT_LATIN_SMALL_WHITE);
		drawPosition.y() += 8;
	}
}

void cDebugOutputWidget::drawDetectedByPlayerList()
{
	if (!gameMap || !client) return;
	const auto& map = *client->getModel().getMap();
	const auto zoomedTileSize = gameMap->getZoomedTileSize();
	const auto tileDrawingRange = gameMap->computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = gameMap->getZoomedStartTilePixelOffset();

	auto font = cUnicodeFont::font.get();

	for (auto i = makeIndexIterator(tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
	{
		auto& mapField = map.getField(*i);
		auto building = mapField.getBuilding();
		if (building == nullptr) continue;

		auto drawDestination = gameMap->computeTileDrawingArea (zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, building->getPosition());
		drawDestination.x += 4;
		drawDestination.y += 4;

		for (const auto& playerId : building->detectedByPlayerList)
		{
			if (Contains(building->detectedInThisTurnByPlayerList, playerId))
			{
				font->showText(drawDestination.x, drawDestination.y, iToStr(playerId) + "#", FONT_LATIN_SMALL_RED);
			}
			else
			{
				font->showText(drawDestination.x, drawDestination.y, iToStr(playerId), FONT_LATIN_SMALL_RED);
			}
			drawDestination.y += font->getFontHeight(FONT_LATIN_SMALL_RED);
		}
	}

	for (auto i = makeIndexIterator(tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
	{
		auto& mapField = map.getField(*i);
		auto vehicle = mapField.getVehicle();
		if (vehicle == nullptr) continue;

		auto drawDestination = gameMap->computeTileDrawingArea(zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, vehicle->getPosition());
		drawDestination.x += (int)(4 + vehicle->getMovementOffset().x() * gameMap->getZoomFactor());
		drawDestination.y += (int)(4 + vehicle->getMovementOffset().y() * gameMap->getZoomFactor());

		for (const auto& playerId : vehicle->detectedByPlayerList)
		{
			if (Contains(vehicle->detectedInThisTurnByPlayerList, playerId))
			{
				font->showText(drawDestination.x, drawDestination.y, iToStr(playerId) + "#", FONT_LATIN_SMALL_RED);
			}
			else
			{
				font->showText(drawDestination.x, drawDestination.y, iToStr(playerId), FONT_LATIN_SMALL_RED);
			}
			drawDestination.y += font->getFontHeight(FONT_LATIN_SMALL_RED);
		}
	}
}

//------------------------------------------------------------------------------
void cDebugOutputWidget::drawDetectionMaps()
{
	if (!gameMap || !client) return;

	const auto zoomedTileSize = gameMap->getZoomedTileSize();
	const auto tileDrawingRange = gameMap->computeTileDrawingRange();
	const auto zoomedStartTilePixelOffset = gameMap->getZoomedStartTilePixelOffset();

	auto font = cUnicodeFont::font.get();

	for (auto i = makeIndexIterator(tileDrawingRange.first, tileDrawingRange.second); i.hasMore(); i.next())
	{
		auto drawDestination = gameMap->computeTileDrawingArea(zoomedTileSize, zoomedStartTilePixelOffset, tileDrawingRange.first, *i);
		drawDestination.x += zoomedTileSize.x() - font->getTextWide("SLM");
		drawDestination.y += 4;

		for (const auto& player : client->getModel().getPlayerList())
		{
			std::string s;
			s += player->hasSeaDetection(*i)  ? "S" : " ";
			s += player->hasLandDetection(*i) ? "L" : " ";
			s += player->hasMineDetection(*i) ? "M" : " ";
			if (s != "   ")
			{
				font->showText(drawDestination.x, drawDestination.y, iToStr(player->getId()) + s, FONT_LATIN_SMALL_YELLOW);
			}
			drawDestination.y += font->getFontHeight(FONT_LATIN_SMALL_YELLOW);
		}
	}
}
