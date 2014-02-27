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
#include <sstream>

#include "hud.h"

#include "attackJobs.h"
#include "autosurface.h"
#include "buildings.h"
#include "client.h"
#include "clientevents.h"
#include "clist.h"
#include "dialog.h"
#include "events.h"
#include "fxeffects.h"
#include "keys.h"
#include "log.h"
#include "main.h"
#include "netmessage.h"
#include "pcx.h"
#include "player.h"
#include "settings.h"
#include "sound.h"
#include "unifonts.h"
#include "vehicles.h"
#include "video.h"

#include "events/eventmanager.h"
#include "input/mouse/mouse.h"
#include "input/keyboard/keyboard.h"
#include "utility/scopedoperation.h"

using namespace std;

#if defined (_MSC_VER)
// The purpose of this warning is to avoid to use not fully constructed object.
// As we use 'this' only to save back reference
// and use it later once object is fully initialized
// just ignore the warning.
# pragma warning (disable:4355) // 'this': used in base member initializer list
// appear in cGameGUI constructor
#endif

cMouseBox::cMouseBox()
{
	invalidate();
}

bool cMouseBox::isTooSmall() const
{
	if (!isValid()) return true;

	auto diff = box.getMaxCorner() - box.getMinCorner();
	return std::abs(diff[0]) < 0.5 && std::abs(diff[1]) < 0.5;
}

bool cMouseBox::isValid() const
{
	return isValidStart() && isValidEnd();
}

bool cMouseBox::isValidStart() const
{
	return box.getMinCorner()[0] != -1 && box.getMinCorner()[1];
}

bool cMouseBox::isValidEnd() const
{
	return box.getMaxCorner()[0] != -1 && box.getMaxCorner()[1] != -1;
}

void cMouseBox::invalidate()
{
	box.getMinCorner()[0] = -1;
	box.getMinCorner()[1] = -1;
	box.getMaxCorner()[0] = -1;
	box.getMaxCorner()[1] = -1;
}

cBox<cFixedVector<double, 2>>& cMouseBox::getBox()
{
	return box;
}

const cBox<cFixedVector<double, 2>>& cMouseBox::getBox() const
{
	return box;
}

cBox<cPosition> cMouseBox::getCorrectedMapBox() const
{
	cPosition minPosition(static_cast<int>(std::min(box.getMinCorner()[0], box.getMaxCorner()[0])), static_cast<int>(std::min(box.getMinCorner()[1], box.getMaxCorner()[1])));
	cPosition maxPosition(static_cast<int>(std::max(box.getMinCorner()[0], box.getMaxCorner()[0])), static_cast<int>(std::max(box.getMinCorner()[1], box.getMaxCorner()[1])));

	return cBox<cPosition>(minPosition, maxPosition);
}

cDebugOutput::cDebugOutput() :
	server (0), client (0)
{
	debugAjobs = false;
	debugBaseServer = false;
	debugBaseClient = false;
	debugSentry = false;
	debugFX = false;
	debugTraceServer = false;
	debugTraceClient = false;
	debugPlayers = false;
	showFPS = true;
	debugCache = false;
	debugSync = true;
}

void cDebugOutput::setClient (cClient* client_)
{
	client = client_;
}
void cDebugOutput::setServer (cServer* server_)
{
	server = server_;
}

void cDebugOutput::draw()
{
	const int DEBUGOUT_X_POS (Video.getResolutionX() - 200);
	const cGameGUI& gui = client->getGameGUI();
	const cPlayer& player = client->getActivePlayer();

	int debugOff = 30;

	if (debugPlayers)
	{
		font->showText (DEBUGOUT_X_POS, debugOff, "Players: " + iToStr ((int) client->getPlayerList().size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		SDL_Rect rDest = { Sint16 (DEBUGOUT_X_POS), Sint16 (debugOff), 20, 10 };
		SDL_Rect rSrc = { 0, 0, 20, 10 };
		SDL_Rect rDotDest = { Sint16 (DEBUGOUT_X_POS - 10), Sint16 (debugOff), 10, 10 };
		SDL_Rect rBlackOut = { Sint16 (DEBUGOUT_X_POS + 20), Sint16 (debugOff), 0, 10 };
		const std::vector<cPlayer*>& playerList = client->getPlayerList();
		for (size_t i = 0; i != playerList.size(); ++i)
		{
			// HACK SHOWFINISHEDPLAYERS
			SDL_Rect rDot = { 10, 0, 10, 10 }; // for green dot

			if (playerList[i]->bFinishedTurn /* && playerList[i] != &player*/)
			{
				SDL_BlitSurface (GraphicsData.gfx_player_ready, &rDot, cVideo::buffer, &rDotDest);
			}
#if 0
			else if (playerList[i] == &player && client->bWantToEnd)
			{
				SDL_BlitSurface (GraphicsData.gfx_player_ready, &rDot, cVideo::buffer, &rDotDest);
			}
#endif
			else
			{
				rDot.x = 0; // for red dot
				SDL_BlitSurface (GraphicsData.gfx_player_ready, &rDot, cVideo::buffer, &rDotDest);
			}

			SDL_BlitSurface (playerList[i]->getColorSurface(), &rSrc, cVideo::buffer, &rDest);
			if (playerList[i] == &player)
			{
				string sTmpLine = " " + playerList[i]->getName() + ", nr: " + iToStr (playerList[i]->getNr()) + " << you! ";
				// black out background for better recognizing
				rBlackOut.w = font->getTextWide (sTmpLine, FONT_LATIN_SMALL_WHITE);
				SDL_FillRect (cVideo::buffer, &rBlackOut, 0xFF000000);
				font->showText (rBlackOut.x, debugOff + 1, sTmpLine, FONT_LATIN_SMALL_WHITE);
			}
			else
			{
				string sTmpLine = " " + playerList[i]->getName() + ", nr: " + iToStr (playerList[i]->getNr()) + " ";
				// black out background for better recognizing
				rBlackOut.w = font->getTextWide (sTmpLine, FONT_LATIN_SMALL_WHITE);
				SDL_FillRect (cVideo::buffer, &rBlackOut, 0xFF000000);
				font->showText (rBlackOut.x, debugOff + 1, sTmpLine, FONT_LATIN_SMALL_WHITE);
			}
			// use 10 for pixel high of dots instead of text high
			debugOff += 10;
			rDest.y = rDotDest.y = rBlackOut.y = debugOff;
		}
	}

	if (debugAjobs)
	{
		font->showText (DEBUGOUT_X_POS, debugOff, "ClientAttackJobs: " + iToStr ((int) client->attackJobs.size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		if (server)
		{
			font->showText (DEBUGOUT_X_POS, debugOff, "ServerAttackJobs: " + iToStr ((int) server->AJobs.size()), FONT_LATIN_SMALL_WHITE);
			debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		}
	}

	if (debugBaseClient)
	{
		font->showText (DEBUGOUT_X_POS, debugOff, "subbases: " + iToStr ((int) player.base.SubBases.size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
	}

	if (debugBaseServer)
	{
		cPlayer* serverPlayer = server->getPlayerFromNumber (player.getNr());
		font->showText (DEBUGOUT_X_POS, debugOff, "subbases: " + iToStr ((int) serverPlayer->base.SubBases.size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
	}

	if (debugFX)
	{
#if 0
		font->showText (DEBUGOUT_X_POS, debugOff, "fx-count: " + iToStr ((int) FXList.size() + (int) FXListBottom.size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS, debugOff, "wind-dir: " + iToStr ((int) (fWindDir * 57.29577f)), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
#endif
	}
	if (debugTraceServer || debugTraceClient)
	{
		trace();
	}
	if (debugCache)
	{
		const cDrawingCache& dCache = gui.dCache;
		font->showText (DEBUGOUT_X_POS, debugOff, "Max cache size: " + iToStr (dCache.getMaxCacheSize()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS, debugOff, "cache size: " + iToStr (dCache.getCacheSize()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS, debugOff, "cache hits: " + iToStr (dCache.getCacheHits()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS, debugOff, "cache misses: " + iToStr (dCache.getCacheMisses()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS, debugOff, "not cached: " + iToStr (dCache.getNotCached()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
	}

	if (showFPS)
	{
		font->showText (DEBUGOUT_X_POS, debugOff, "Frame: " + iToStr (gui.frame), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS, debugOff, "FPS: " + iToStr (Round (gui.framesPerSecond)), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS, debugOff, "Cycles/s: " + iToStr (Round (gui.cyclesPerSecond)), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS, debugOff, "Load: " + iToStr (gui.loadValue / 10) + "." + iToStr (gui.loadValue % 10) + "%", FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
	}
	if (debugSync)
	{
		font->showText (DEBUGOUT_X_POS - 20, debugOff, "Sync debug:", FONT_LATIN_SMALL_YELLOW);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		if (server)
		{
			font->showText (DEBUGOUT_X_POS - 10, debugOff, "-Server:", FONT_LATIN_SMALL_YELLOW);
			debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
			font->showText (DEBUGOUT_X_POS, debugOff, "Server Time: ", FONT_LATIN_SMALL_WHITE);
			font->showText (DEBUGOUT_X_POS + 110, debugOff, iToStr (server->gameTimer.gameTime), FONT_LATIN_SMALL_WHITE);
			debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

			font->showText (DEBUGOUT_X_POS, debugOff, "Net MSG Queue: ", FONT_LATIN_SMALL_WHITE);
			font->showText (DEBUGOUT_X_POS + 110, debugOff, iToStr (server->eventQueue.size()), FONT_LATIN_SMALL_WHITE);
			debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

			font->showText (DEBUGOUT_X_POS, debugOff, "EventCounter: ", FONT_LATIN_SMALL_WHITE);
			font->showText (DEBUGOUT_X_POS + 110, debugOff, iToStr (server->gameTimer.eventCounter), FONT_LATIN_SMALL_WHITE);
			debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

			font->showText (DEBUGOUT_X_POS, debugOff, "-Client Lag: ", FONT_LATIN_SMALL_WHITE);
			debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

			for (size_t i = 0; i != server->PlayerList.size(); ++i)
			{
				eUnicodeFontType fontType = FONT_LATIN_SMALL_WHITE;
				if (server->gameTimer.getReceivedTime (i) + PAUSE_GAME_TIMEOUT < server->gameTimer.gameTime)
					fontType = FONT_LATIN_SMALL_RED;
				font->showText (DEBUGOUT_X_POS + 10, debugOff, "Client " + iToStr (i) + ": ", fontType);
				font->showText (DEBUGOUT_X_POS + 110, debugOff, iToStr (server->gameTimer.gameTime - server->gameTimer.getReceivedTime (i)), fontType);
				debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
			}
		}

		font->showText (DEBUGOUT_X_POS - 10, debugOff, "-Client:", FONT_LATIN_SMALL_YELLOW);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		eUnicodeFontType fontType = FONT_LATIN_SMALL_GREEN;
		if (client->gameTimer.debugRemoteChecksum != client->gameTimer.localChecksum)
			fontType = FONT_LATIN_SMALL_RED;
		font->showText (DEBUGOUT_X_POS, debugOff, "Server Checksum: ", FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS + 110, debugOff, "0x" + iToHex (client->gameTimer.debugRemoteChecksum), fontType);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (DEBUGOUT_X_POS, debugOff, "Client Checksum: ", FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS + 110, debugOff, "0x" + iToHex (client->gameTimer.localChecksum), fontType);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (DEBUGOUT_X_POS, debugOff, "Client Time: ", FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS + 110, debugOff, iToStr (client->gameTimer.gameTime), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (DEBUGOUT_X_POS, debugOff, "Net MGS Queue: ", FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS + 110, debugOff, iToStr (client->getEventHandling().eventQueue.size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (DEBUGOUT_X_POS, debugOff, "EventCounter: ", FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS + 110, debugOff, iToStr (client->gameTimer.eventCounter), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (DEBUGOUT_X_POS, debugOff, "Time Buffer: ", FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS + 110, debugOff, iToStr (client->gameTimer.getReceivedTime() - client->gameTimer.gameTime), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (DEBUGOUT_X_POS, debugOff, "Ticks per Frame ", FONT_LATIN_SMALL_WHITE);
		static unsigned int lastGameTime = 0;
		font->showText (DEBUGOUT_X_POS + 110, debugOff, iToStr (client->gameTimer.gameTime - lastGameTime), FONT_LATIN_SMALL_WHITE);
		lastGameTime = client->gameTimer.gameTime;
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (DEBUGOUT_X_POS, debugOff, "Time Adjustment: ", FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS + 110, debugOff, iToStr (client->gameTimer.gameTimeAdjustment), FONT_LATIN_SMALL_WHITE);
		static int totalAdjust = 0;
		totalAdjust += client->gameTimer.gameTimeAdjustment;
		client->gameTimer.gameTimeAdjustment = 0;
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText (DEBUGOUT_X_POS, debugOff, "TotalAdj.: ", FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS + 110, debugOff, iToStr (totalAdjust), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
	}
}

void cDebugOutput::trace()
{
	const auto mouseTilePosition = client->getGameGUI().getTilePosition(cMouse::getInstance().getPosition());
	int x = mouseTilePosition.x();
	int y = mouseTilePosition.y();
	if (x < 0 || y < 0) return;

	cMapField* field;

	if (debugTraceServer) field = &server->Map->fields[server->Map->getOffset (x, y)];
	else field = &client->Map->fields[client->Map->getOffset (x, y)];

	y = 18 + 5 + 8;
	x = 180 + 5;

	if (field->getVehicle()) { traceVehicle (*field->getVehicle(), &y, x); y += 20; }
	if (field->getPlane()) { traceVehicle (*field->getPlane(), &y, x); y += 20; }
	const std::vector<cBuilding*>& buildings = field->getBuildings();
	for (std::vector<cBuilding*>::const_iterator it = buildings.begin(); it != buildings.end(); ++it)
	{
		traceBuilding (**it, &y, x); y += 20;
	}
}

void cDebugOutput::traceVehicle (const cVehicle& vehicle, int* y, int x)
{
	string tmpString;

	tmpString = "name: \"" + vehicle.getDisplayName() + "\" id: \"" + iToStr (vehicle.iID) + "\" owner: \"" + vehicle.owner->getName() + "\" posX: +" + iToStr (vehicle.PosX) + " posY: " + iToStr (vehicle.PosY) + " offX: " + iToStr (vehicle.OffX) + " offY: " + iToStr (vehicle.OffY);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "dir: " + iToStr (vehicle.dir) + " moving: +" + iToStr (vehicle.moving) + " mjob: " + pToStr (vehicle.ClientMoveJob) + " speed: " + iToStr (vehicle.data.speedCur) + " mj_active: " + iToStr (vehicle.MoveJobActive);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = " attacking: " + iToStr (vehicle.attacking) + " on sentry: +" + iToStr (vehicle.sentryActive) + " ditherx: " + iToStr (vehicle.ditherX) + " dithery: " + iToStr (vehicle.ditherY);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "is_building: " + iToStr (vehicle.IsBuilding) + " building_typ: " + vehicle.BuildingTyp.getText() + " build_costs: +" + iToStr (vehicle.BuildCosts) + " build_rounds: " + iToStr (vehicle.BuildRounds) + " build_round_start: " + iToStr (vehicle.BuildRoundsStart);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = " bandx: " + iToStr (vehicle.BandX) + " bandy: +" + iToStr (vehicle.BandY) + " build_big_saved_pos: " + iToStr (vehicle.BuildBigSavedPos) + " build_path: " + iToStr (vehicle.BuildPath);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = " is_clearing: " + iToStr (vehicle.IsClearing) + " clearing_rounds: +" + iToStr (vehicle.ClearingRounds) + " clear_big: " + iToStr (vehicle.data.isBig) + " loaded: " + iToStr (vehicle.Loaded);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "commando_rank: " + fToStr (Round (vehicle.CommandoRank, 2)) + " disabled: " + iToStr (vehicle.turnsDisabled);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "is_locked: " + iToStr (vehicle.isLocked()) + " clear_mines: +" + iToStr (vehicle.ClearMines) + " lay_mines: " + iToStr (vehicle.LayMines);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = " stored_vehicles_count: " + iToStr ((int) vehicle.storedUnits.size());
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	for (size_t i = 0; i != vehicle.storedUnits.size(); ++i)
	{
		const cVehicle* storedVehicle = vehicle.storedUnits[i];
		font->showText (x, *y, " store " + iToStr (i) + ": \"" + storedVehicle->getDisplayName() + "\"", FONT_LATIN_SMALL_WHITE);
		*y += 8;
	}

	if (debugTraceServer)
	{
		tmpString = "seen by players: owner";
		for (size_t i = 0; i != vehicle.seenByPlayerList.size(); ++i)
		{
			tmpString += ", \"" + vehicle.seenByPlayerList[i]->getName() + "\"";
		}
		font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
		*y += 8;
	}

	tmpString = "flight height: " + iToStr (vehicle.FlightHigh);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;
}

void cDebugOutput::traceBuilding (const cBuilding& building, int* y, int x)
{
	string tmpString;

	tmpString = "name: \"" + building.getDisplayName() + "\" id: \"" + iToStr (building.iID) + "\" owner: \"" + (building.owner ? building.owner->getName() : "<null>") + "\" posX: +" + iToStr (building.PosX) + " posY: " + iToStr (building.PosY);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "dir: " + iToStr (building.dir) + " on sentry: +" + iToStr (building.sentryActive) + " sub_base: " + pToStr (building.SubBase);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "attacking: " + iToStr (building.attacking) + " UnitsData.dirt_typ: " + iToStr (building.RubbleTyp) + " UnitsData.dirt_value: +" + iToStr (building.RubbleValue) + " big_dirt: " + iToStr (building.data.isBig) + " is_working: " + iToStr (building.IsWorking);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = " max_metal_p: " + iToStr (building.MaxMetalProd) +
				" max_oil_p: " + iToStr (building.MaxOilProd) +
				" max_gold_p: " + iToStr (building.MaxGoldProd);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "is_locked: " + iToStr (building.isLocked()) +
				" disabled: " + iToStr (building.turnsDisabled);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = " stored_vehicles_count: " + iToStr ((int) building.storedUnits.size());
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	for (size_t i = 0; i != building.storedUnits.size(); ++i)
	{
		const cVehicle* storedVehicle = building.storedUnits[i];
		font->showText (x, *y, " store " + iToStr (i) + ": \"" + storedVehicle->getDisplayName() + "\"", FONT_LATIN_SMALL_WHITE);
		*y += 8;
	}

	const size_t buildingBuildListSize = building.BuildList.size();
	tmpString =
		"build_speed: "        + iToStr (building.BuildSpeed) +
		" repeat_build: "      + iToStr (building.RepeatBuild) +
		" build_list_count: +" + iToStr ((int) buildingBuildListSize);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	for (size_t i = 0; i != buildingBuildListSize; ++i)
	{
		const sBuildList& BuildingList = building.BuildList[i];
		font->showText (x, *y, "  build " + iToStr (i) + ": " + BuildingList.type.getText() + " \"" + BuildingList.type.getUnitDataOriginalVersion()->name + "\"", FONT_LATIN_SMALL_WHITE);
		*y += 8;
	}

	if (debugTraceServer)
	{
		tmpString = "seen by players: owner";
		for (size_t i = 0; i != building.seenByPlayerList.size(); ++i)
		{
			tmpString += ", \"" + building.seenByPlayerList[i]->getName() + "\"";
		}
		font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
		*y += 8;
	}
}

Uint32 TimerCallback (Uint32 interval, void* arg)
{
	reinterpret_cast<cGameGUI*> (arg)->Timer();
	return interval;
}

cGameGUI::cGameGUI() :
	cMenu (generateSurface()),
	client (NULL),
	iObjectStream (-1),
	msgCoordsX (-1),
	msgCoordsY (-1),
	miniMapOffX (0),
	miniMapOffY (0),
	needMiniMapDraw (true),
	overUnitField (NULL),
	FxList (new cFxContainer),
	zoomSlider (20, 274, 1.0f, 1.0f, this, 130, cMenuSlider::SLIDER_TYPE_HUD_ZOOM, cMenuSlider::SLIDER_DIR_RIGHTMIN),
	endButton (391, 4, lngPack.i18n ("Text~Others~End"), cMenuButton::BUTTON_TYPE_HUD_END, FONT_LATIN_NORMAL),
	preferencesButton (86, 4, lngPack.i18n ("Text~Others~Settings"), cMenuButton::BUTTON_TYPE_HUD_PREFERENCES, FONT_LATIN_SMALL_WHITE),
	filesButton (17, 3, lngPack.i18n ("Text~Others~Files"), cMenuButton::BUTTON_TYPE_HUD_FILES, FONT_LATIN_SMALL_WHITE),
	playButton (146, 123, "", cMenuButton::BUTTON_TYPE_HUD_PLAY),
	stopButton (146, 143, "", cMenuButton::BUTTON_TYPE_HUD_STOP),
	FLCImage (10, 29, NULL),
	unitDetails (8, 171, false),
	surveyButton (2, 296, lngPack.i18n ("Text~Others~Survey"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_00, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	hitsButton (57, 296, lngPack.i18n ("Text~Others~Hitpoints_7"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_01, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	scanButton (112, 296, lngPack.i18n ("Text~Others~Scan"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_02, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	statusButton (2, 296 + 18, lngPack.i18n ("Text~Others~Status"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_10, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	ammoButton (57, 296 + 18, lngPack.i18n ("Text~Others~Ammo"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_11, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	gridButton (112, 296 + 18, lngPack.i18n ("Text~Others~Grid"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_12, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	colorButton (2, 296 + 18 + 16, lngPack.i18n ("Text~Others~Color"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_20, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	rangeButton (57, 296 + 18 + 16, lngPack.i18n ("Text~Others~Range"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_21, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	fogButton (112, 296 + 18 + 16, lngPack.i18n ("Text~Others~Fog"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_22, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	lockButton (32, 227, "", false, false, cMenuCheckButton::CHECKBOX_HUD_LOCK, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	TNTButton (136, 413, "", false, false, cMenuCheckButton::CHECKBOX_HUD_TNT, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	twoXButton (136, 387, "", false, false, cMenuCheckButton::CHECKBOX_HUD_2X, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	playersButton (136, 439, "", false, false, cMenuCheckButton::CHECKBOX_HUD_PLAYERS, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	helpButton (20, 250, "", cMenuButton::BUTTON_TYPE_HUD_HELP),
	centerButton (4, 227, "", cMenuButton::BUTTON_TYPE_HUD_CENTER),
	reportsButton (101, 252, lngPack.i18n ("Text~Others~Log"), cMenuButton::BUTTON_TYPE_HUD_REPORT, FONT_LATIN_SMALL_WHITE),
	chatButton (51, 252, lngPack.i18n ("Text~Others~Chat"), cMenuButton::BUTTON_TYPE_HUD_CHAT, FONT_LATIN_SMALL_WHITE),
	nextButton (124, 227, ">>", cMenuButton::BUTTON_TYPE_HUD_NEXT, FONT_LATIN_SMALL_WHITE),
	prevButton (60, 227, "<<", cMenuButton::BUTTON_TYPE_HUD_PREV, FONT_LATIN_SMALL_WHITE),
	doneButton (99, 227, lngPack.i18n ("Text~Others~Proceed"), cMenuButton::BUTTON_TYPE_HUD_DONE, FONT_LATIN_SMALL_WHITE),
	miniMapImage (MINIMAP_POS_X, MINIMAP_POS_Y),
	coordsLabel (265 + 32, (Video.getResolutionY() - 21) + 3),
	unitNameLabel (343 + 106, (Video.getResolutionY() - 21) + 3),
	turnLabel (498, 7),
	timeLabel (564, 7),
	chatBox (HUD_LEFT_WIDTH + 5, Video.getResolutionY() - 48, this),
	infoTextLabel (HUD_LEFT_WIDTH + (Video.getResolutionX() - HUD_TOTAL_WIDTH) / 2, 235, "", FONT_LATIN_BIG),
	infoTextAdditionalLabel (HUD_LEFT_WIDTH + (Video.getResolutionX() - HUD_TOTAL_WIDTH) / 2, 235 + font->getFontHeight (FONT_LATIN_BIG), ""),
	selUnitStatusStr (12, 40, "", FONT_LATIN_SMALL_WHITE),
	selUnitNamePrefixStr (12, 30, "", FONT_LATIN_SMALL_GREEN),
	selUnitNameEdit (12, 30, 123, 10, this, FONT_LATIN_SMALL_GREEN, cMenuLineEdit::LE_TYPE_JUST_TEXT),
	playersInfo (Video.getResolutionY() >= 768 ? 3 : 161, Video.getResolutionY() >= 768 ? 482 : 480 - 82, Video.getResolutionY() >= 768),
	iTimerTime (0),
	dCache (nullptr)
{
	//dCache.setGameGUI (*this);
	unitMenuActive = false;
	selectedMenuButtonIndex = -1;
	vehicleToActivate = 0;
	frame = 0;
	zoom = 1.0f;
	offX = offY = 0;
	framesPerSecond = cyclesPerSecond = 0;
	loadValue = 0;
	activeItem = NULL;
	helpActive = false;
	showPlayers = false;
	blinkColor = 0x00FFFFFF;
	FLC = NULL;
	playFLC = true;
	mouseInputMode = normalInput;
	TimerID = SDL_AddTimer (50, TimerCallback, this);
	selectedUnit = NULL;
	setWind (random (360));

	zoomSlider.setMoveCallback (&zoomSliderMoved);
	menuItems.push_back (&zoomSlider);
	menuItems.push_back (zoomSlider.scroller.get());

	endButton.setReleasedFunction (&endReleased);
	menuItems.push_back (&endButton);

	preferencesButton.setReleasedFunction (&preferencesReleased);
	menuItems.push_back (&preferencesButton);
	filesButton.setReleasedFunction (&filesReleased);
	menuItems.push_back (&filesButton);

	playButton.setReleasedFunction (&playReleased);
	menuItems.push_back (&playButton);
	stopButton.setReleasedFunction (&stopReleased);
	menuItems.push_back (&stopButton);

	FLCImage.setReleasedFunction (&swithAnimationReleased);
	menuItems.push_back (&FLCImage);

	menuItems.push_back (&unitDetails);

	// generate checkbuttons
	menuItems.push_back (&surveyButton);
	menuItems.push_back (&hitsButton);
	menuItems.push_back (&scanButton);
	menuItems.push_back (&statusButton);
	menuItems.push_back (&ammoButton);
	menuItems.push_back (&gridButton);
	menuItems.push_back (&colorButton);
	menuItems.push_back (&rangeButton);
	menuItems.push_back (&fogButton);
	menuItems.push_back (&lockButton);

	TNTButton.setClickedFunction (&changedMiniMap);
	menuItems.push_back (&TNTButton);
	twoXButton.setClickedFunction (&twoXReleased);
	menuItems.push_back (&twoXButton);
	playersButton.setClickedFunction (&playersReleased);
	menuItems.push_back (&playersButton);

	helpButton.setReleasedFunction (&helpReleased);
	menuItems.push_back (&helpButton);
	centerButton.setReleasedFunction (&centerReleased);
	menuItems.push_back (&centerButton);

	reportsButton.setReleasedFunction (&reportsReleased);
	menuItems.push_back (&reportsButton);
	chatButton.setReleasedFunction (&chatReleased);
	menuItems.push_back (&chatButton);

	nextButton.setReleasedFunction (&nextReleased);
	menuItems.push_back (&nextButton);
	prevButton.setReleasedFunction (&prevReleased);
	menuItems.push_back (&prevButton);
	doneButton.setReleasedFunction (&doneReleased);
	menuItems.push_back (&doneButton);

	miniMapImage.setClickedFunction (&miniMapClicked);
	miniMapImage.setRightClickedFunction (&miniMapRightClicked);
	menuItems.push_back (&miniMapImage);

	coordsLabel.setCentered (true);
	menuItems.push_back (&coordsLabel);

	unitNameLabel.setCentered (true);
	menuItems.push_back (&unitNameLabel);

	turnLabel.setCentered (true);
	menuItems.push_back (&turnLabel);

	timeLabel.setCentered (true);
	menuItems.push_back (&timeLabel);

	chatBox.setDisabled (true);
	chatBox.setReturnPressedFunc (&chatBoxReturnPressed);
	menuItems.push_back (&chatBox);

	infoTextLabel.setCentered (true);
	infoTextLabel.setDisabled (true);
	menuItems.push_back (&infoTextLabel);

	infoTextAdditionalLabel.setCentered (true);
	infoTextAdditionalLabel.setDisabled (true);
	menuItems.push_back (&infoTextAdditionalLabel);

	menuItems.push_back (&selUnitStatusStr);
	menuItems.push_back (&selUnitNamePrefixStr);

	selUnitNameEdit.setReturnPressedFunc (unitNameReturnPressed);
	menuItems.push_back (&selUnitNameEdit);

	playersInfo.setDisabled (true);
	menuItems.push_back (&playersInfo);

	updateTurn(1);

	using namespace std::placeholders;

	signalConnectionManagerPermanent.connect(Video.screenShotTaken, std::bind(&cGameGUI::screenShotTaken, this, _1));
}

void cGameGUI::activate()
{
	cMenu::activate();

	using namespace std::placeholders;

	signalConnectionManagerActive.connect(cMouse::getInstance().moved, std::bind(&cGameGUI::mouseMoved, this, _1));
	signalConnectionManagerActive.connect(cMouse::getInstance().pressed, std::bind(&cGameGUI::mouseButtonPressed, this, _1, _2));
	signalConnectionManagerActive.connect(cMouse::getInstance().released, std::bind(&cGameGUI::mouseButtonReleased, this, _1, _2));
	signalConnectionManagerActive.connect(cMouse::getInstance().wheelMoved, std::bind(&cGameGUI::mouseWheelMoved, this, _1, _2));

	signalConnectionManagerActive.connect(cKeyboard::getInstance().keyPressed, std::bind(&cGameGUI::keyPressed, this, _1, _2));
	signalConnectionManagerActive.connect(cKeyboard::getInstance().keyReleased, std::bind(&cGameGUI::keyReleased, this, _1, _2));
}

void cGameGUI::deactivate()
{
	cMenu::deactivate();

	signalConnectionManagerActive.disconnectAll();
}

void cGameGUI::mouseMoved(cMouse& mouse)
{
	if(checkScroll(mouse)) return;

	updateMouseCursor();

	// update mouseboxes
	if(mouseBox.isValidStart() && mouse.isButtonPressed(eMouseButtonType::Left) && !mouse.isButtonPressed(eMouseButtonType::Right) &&
	   mouse.getPosition().x() > HUD_LEFT_WIDTH)
	{
		mouseBox.getBox().getMaxCorner()[0] = (mouse.getPosition().x() - HUD_LEFT_WIDTH + (offX * getZoom())) / getTileSize();
		mouseBox.getBox().getMaxCorner()[1] = (mouse.getPosition().y() - HUD_TOP_HIGHT + (offY * getZoom())) / getTileSize();
	}
	if(rightMouseBox.isValidStart() && mouse.isButtonPressed(eMouseButtonType::Right) && !mouse.isButtonPressed(eMouseButtonType::Left) &&
	   mouse.getPosition().x() > HUD_LEFT_WIDTH)
	{
		rightMouseBox.getBox().getMaxCorner()[0] = (mouse.getPosition().x() - HUD_LEFT_WIDTH + (offX * getZoom())) / getTileSize();
		rightMouseBox.getBox().getMaxCorner()[1] = (mouse.getPosition().y() - HUD_TOP_HIGHT + (offY * getZoom())) / getTileSize();
	}

	static cPosition lastPos(0, 0);
	// the rightMouseBox is only used to trigger the right mouse button scroll
	if(!rightMouseBox.isTooSmall() &&  mouse.getPosition().x() > HUD_LEFT_WIDTH)
	{
		if(lastPos.x() != -1 && lastPos.y() != -1)
		{
			const int speed = 5;
			cPosition offset = (mouse.getPosition() - lastPos) * speed;

			setOffsetPosition(offX + offset.x(), offY + offset.y());
		}
		lastPos = mouse.getPosition();
	}
	else
	{
		lastPos = -1;
	}

	// check minimap
	if(miniMapImage.getIsClicked() || miniMapImage.getWasClicked())
	{
		miniMapClicked(this);
	}
}

void cGameGUI::mouseButtonPressed(cMouse& mouse, eMouseButtonType button)
{
	for(unsigned int i = 0; i < menuItems.size(); i++)
	{
		if(!menuItems[i]->isDisabled() && menuItems[i]->overItem(mouse.getPosition().x(), mouse.getPosition().y())) return;
	}

	if(selectedUnit && unitMenuActive && areCoordsOverMenu(*selectedUnit, mouse.getPosition()))
	{
		if(button == eMouseButtonType::Left) setMenuSelection(*selectedUnit);
		return;
	}

	if(!rightMouseBox.isValidStart() && button == eMouseButtonType::Right && !mouse.isButtonPressed(eMouseButtonType::Left) &&
	   mouse.getPosition().x() > HUD_LEFT_WIDTH && mouse.getPosition().y() > 20)
	{
		rightMouseBox.getBox().getMinCorner()[0] = (mouse.getPosition().x() - HUD_LEFT_WIDTH + (offX * getZoom())) / getTileSize();
		rightMouseBox.getBox().getMinCorner()[1] = (mouse.getPosition().y() - HUD_TOP_HIGHT + (offY * getZoom())) / getTileSize();
	}

	if(!mouseBox.isValidStart() && button == eMouseButtonType::Left && !mouse.isButtonPressed(eMouseButtonType::Right) &&
	   mouse.getPosition().x() > HUD_LEFT_WIDTH && mouse.getPosition().y() > 20)
	{
		mouseBox.getBox().getMinCorner()[0] = (mouse.getPosition().x() - HUD_LEFT_WIDTH + (offX * getZoom())) / getTileSize();
		mouseBox.getBox().getMinCorner()[1] = (mouse.getPosition().y() - HUD_TOP_HIGHT + (offY * getZoom())) / getTileSize();
	}
}

void cGameGUI::mouseButtonReleased(cMouse& mouse, eMouseButtonType button)
{
	for(unsigned int i = 0; i < menuItems.size(); i++)
	{
		if(!menuItems[i]->isDisabled() && menuItems[i]->overItem(mouse.getPosition().x(), mouse.getPosition().y())) return;
	}

	if(selectedUnit && unitMenuActive && areCoordsOverMenu(*selectedUnit, mouse.getPosition()))
	{
		if(button == eMouseButtonType::Left) menuReleased(*selectedUnit);
		return;
	}

	const bool changeAllowed = !client->isFreezed();

	cVehicle* overVehicle = NULL;
	cVehicle* overPlane = NULL;
	cBuilding* overBuilding = NULL;
	cBuilding* overBaseBuilding = NULL;
	if(overUnitField)
	{
		overVehicle = overUnitField->getVehicle();
		overPlane = overUnitField->getPlane();
		overBuilding = overUnitField->getTopBuilding();
		overBaseBuilding = overUnitField->getBaseBuilding();
	}

	const auto mouseTilePosition = getTilePosition(cMouse::getInstance().getPosition());

	if(MouseStyle == OldSchool && button == eMouseButtonType::Right && !mouse.isButtonPressed(eMouseButtonType::Left) && overUnitField)
	{
		if(selectedUnit && (overVehicle == selectedUnit ||
							overPlane == selectedUnit ||
							overBuilding == selectedUnit ||
							overBaseBuilding == selectedUnit))
		{
			cUnitHelpMenu helpMenu(&selectedUnit->data, selectedUnit->owner);
			switchTo(helpMenu, client);
		}
		else if(overUnitField) selectUnit(overUnitField, true);
	}
	else
	{
		bool mouseOverSelectedUnit = selectedUnit && selectedUnit->isAbove(mouseTilePosition);

		if(button == eMouseButtonType::Right && ((!mouse.isButtonPressed(eMouseButtonType::Left) && rightMouseBox.isTooSmall()) ||
			(MouseStyle == OldSchool && mouse.isButtonPressed(eMouseButtonType::Left))))
		{
			if(helpActive)
			{
				helpActive = false;
			}
			else
			{
				deselectGroup();
				if(overUnitField && mouseOverSelectedUnit)
				{
					std::vector<cVehicle*>& planes = overUnitField->getPlanes();
					cUnit* next = NULL;

					const cVehicle* selectedVehicle = getSelectedVehicle();
					const cBuilding* selectedBuilding = getSelectedBuilding();
					if(selectedVehicle)
					{
						std::vector<cVehicle*>::iterator it = std::find(planes.begin(), planes.end(), selectedVehicle);

						if(it == planes.end())
						{
							if(overBuilding) next = overBuilding;
							else if(overBaseBuilding) next = overBaseBuilding;
							else if(overPlane) next = overPlane;
						}
						else
						{
							++it;

							if(it != planes.end()) next = *it;
							else if(overVehicle) next = overVehicle;
							else if(overBuilding) next = overBuilding;
							else if(overBaseBuilding) next = overBaseBuilding;
							else if(planes.size() > 1)
							{
								next = planes[0];
							}
						}
					}
					else if(selectedBuilding)
					{
						if(overBuilding == selectedBuilding)
						{
							if(overBaseBuilding) next = overBaseBuilding;
							else if(overPlane) next = overPlane;
							else if(overVehicle) next = overVehicle;
						}
						else
						{
							if(overPlane) next = overPlane;
							else if(overVehicle) next = overVehicle;
							else if(overBuilding) next = overBuilding;
						}
					}

					deselectUnit();

					if(next) selectUnit(*next);
				}
				else
				{
					deselectUnit();
				}
			}
		}
	}

	cVehicle* selectedVehicle = getSelectedVehicle();
	cBuilding* selectedBuilding = getSelectedBuilding();

	if(button == eMouseButtonType::Right && !mouse.isButtonPressed(eMouseButtonType::Left))
	{
		rightMouseBox.invalidate();
	}

	if(button == eMouseButtonType::Left && !mouse.isButtonPressed(eMouseButtonType::Right))
	{
		// Store the currently selected unit to determine
		// if the lock state of the clicked unit maybe has to be changed.
		// If the selected unit changes during the click handling,
		// then the newly selected unit has to be added / removed
		// from the "locked units" list.
		cUnit* oldSelectedUnitForLock = selectedUnit;

		if(!mouseBox.isTooSmall())
		{
			selectBoxVehicles(mouseBox.getCorrectedMapBox());
		}
		else if(changeAllowed && selectedUnit && mouse.getCursorType() == eMouseCursorType::Transfer)
		{
			if(overVehicle)
			{
				cDialogTransfer transferDialog(*this, *selectedUnit, *overVehicle);
				switchTo(transferDialog, client);
			}
			else if(overBuilding)
			{
				cDialogTransfer transferDialog(*this, *selectedUnit, *overBuilding);
				switchTo(transferDialog, client);
			}
		}
		else if(changeAllowed && selectedVehicle && mouseInputMode == placeBand && mouse.getCursorType() == eMouseCursorType::Band)
		{
			const cMap& map = *client->getMap();
			mouseInputMode = normalInput;

			if(selectedVehicle->BuildingTyp.getUnitDataOriginalVersion()->isBig)
			{
				sendWantBuild(*client, selectedVehicle->iID, selectedVehicle->BuildingTyp, selectedVehicle->BuildRounds, map.getOffset(selectedVehicle->BandX, selectedVehicle->BandY), false, 0);
			}
			else
			{
				sendWantBuild(*client, selectedVehicle->iID, selectedVehicle->BuildingTyp, selectedVehicle->BuildRounds, map.getOffset(selectedVehicle->PosX, selectedVehicle->PosY), true, map.getOffset(selectedVehicle->BandX, selectedVehicle->BandY));
			}
		}
		else if(changeAllowed && mouse.getCursorType() == eMouseCursorType::Activate && selectedUnit && mouseInputMode == activateVehicle)
		{
			sendWantActivate(*client, selectedUnit->iID, selectedUnit->isAVehicle(), selectedUnit->storedUnits[vehicleToActivate]->iID, mouseTilePosition.x(), mouseTilePosition.y());
			updateMouseCursor();
		}
		else if(changeAllowed && mouse.getCursorType() == eMouseCursorType::Activate && selectedBuilding && selectedBuilding->BuildList.size())
		{
			sendWantExitFinishedVehicle(*client, *selectedBuilding, mouseTilePosition.x(), mouseTilePosition.y());
		}
		else if(changeAllowed && mouse.getCursorType() == eMouseCursorType::Load && selectedBuilding && mouseInputMode == loadMode)
		{
			if(overVehicle && selectedBuilding->canLoad(overVehicle, false))
			{
				if(selectedBuilding->isNextTo(overVehicle->PosX, overVehicle->PosY)) sendWantLoad(*client, selectedBuilding->iID, false, overVehicle->iID);
				else
				{
					cPathCalculator pc(overVehicle->PosX, overVehicle->PosY, *selectedBuilding, *client->getMap(), *overVehicle, true);
					sWaypoint* path = pc.calcPath();
					if(path)
					{
						sendMoveJob(*client, path, overVehicle->iID);
						sendEndMoveAction(*client, overVehicle->iID, selectedBuilding->iID, EMAT_GET_IN);
					}
					else
					{
						PlayRandomVoice(VoiceData.VOINoPath);
					}
				}
			}
			else if(overPlane && selectedBuilding->canLoad(overPlane, false))
			{
				if(selectedBuilding->isNextTo(overPlane->PosX, overPlane->PosY)) sendWantLoad(*client, selectedBuilding->iID, false, overPlane->iID);
				else
				{
					cPathCalculator pc(overPlane->PosX, overPlane->PosY, *selectedBuilding, *client->getMap(), *overPlane, true);
					sWaypoint* path = pc.calcPath();
					if(path)
					{
						sendMoveJob(*client, path, overPlane->iID);
						sendEndMoveAction(*client, overPlane->iID, selectedBuilding->iID, EMAT_GET_IN);
					}
					else
					{
						PlayRandomVoice(VoiceData.VOINoPath);
					}
				}
			}
		}
		else if(changeAllowed && mouse.getCursorType() == eMouseCursorType::Load && selectedVehicle && mouseInputMode == loadMode)
		{
			if(selectedVehicle->data.factorAir > 0 && overVehicle)
			{
				if(overVehicle->PosX == selectedVehicle->PosX && overVehicle->PosY == selectedVehicle->PosY) sendWantLoad(*client, selectedVehicle->iID, true, overVehicle->iID);
				else
				{
					cPathCalculator pc(selectedVehicle->PosX, selectedVehicle->PosY, overVehicle->PosX, overVehicle->PosY, *client->getMap(), *selectedVehicle);
					sWaypoint* path = pc.calcPath();
					if(path)
					{
						sendMoveJob(*client, path, selectedVehicle->iID);
						sendEndMoveAction(*client, selectedVehicle->iID, overVehicle->iID, EMAT_LOAD);
					}
					else
					{
						PlayRandomVoice(VoiceData.VOINoPath);
					}
				}
			}
			else if(overVehicle)
			{
				if(selectedVehicle->isNextTo(overVehicle->PosX, overVehicle->PosY)) sendWantLoad(*client, selectedVehicle->iID, true, overVehicle->iID);
				else
				{
					cPathCalculator pc(overVehicle->PosX, overVehicle->PosY, *selectedVehicle, *client->getMap(), *overVehicle, true);
					sWaypoint* path = pc.calcPath();
					if(path)
					{
						sendMoveJob(*client, path, overVehicle->iID);
						sendEndMoveAction(*client, overVehicle->iID, selectedVehicle->iID, EMAT_GET_IN);
					}
					else
					{
						PlayRandomVoice(VoiceData.VOINoPath);
					}
				}
			}
		}
		else if(changeAllowed && mouse.getCursorType() == eMouseCursorType::Muni && selectedVehicle && mouseInputMode == muniActive)
		{
			if(overVehicle) sendWantSupply(*client, overVehicle->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REARM);
			else if(overPlane && overPlane->FlightHigh == 0) sendWantSupply(*client, overPlane->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REARM);
			else if(overBuilding) sendWantSupply(*client, overBuilding->iID, false, selectedVehicle->iID, true, SUPPLY_TYPE_REARM);
		}
		else if(changeAllowed && mouse.getCursorType() == eMouseCursorType::Repair && selectedVehicle && mouseInputMode == repairActive)
		{
			if(overVehicle) sendWantSupply(*client, overVehicle->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REPAIR);
			else if(overPlane && overPlane->FlightHigh == 0) sendWantSupply(*client, overPlane->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REPAIR);
			else if(overBuilding) sendWantSupply(*client, overBuilding->iID, false, selectedVehicle->iID, true, SUPPLY_TYPE_REPAIR);
		}
		else if(!helpActive)
		{
			//Hud.CheckButtons();
			// check whether the mouse is over a unit menu:
			if((selectedUnit && unitMenuActive && areCoordsOverMenu(*selectedUnit, mouse.getPosition())))
			{
			}
			else
				// check, if the player wants to attack:
			if(changeAllowed && mouse.getCursorType() == eMouseCursorType::Attack && selectedVehicle && !selectedVehicle->attacking && !selectedVehicle->MoveJobActive)
			{
				cUnit* target = selectTarget(mouseTilePosition.x(), mouseTilePosition.y(), selectedVehicle->data.canAttack, *client->getMap());

				if(selectedVehicle->isInRange(mouseTilePosition.x(), mouseTilePosition.y()))
				{
					// find target ID
					int targetId = 0;
					if(target && target->isAVehicle()) targetId = target->iID;

					Log.write(" Client: want to attack " + iToStr(mouseTilePosition.x()) + ":" + iToStr(mouseTilePosition.y()) + ", Vehicle ID: " + iToStr(targetId), cLog::eLOG_TYPE_NET_DEBUG);
					sendWantAttack(*client, targetId, client->getMap()->getOffset(mouseTilePosition.x(), mouseTilePosition.y()), selectedVehicle->iID, true);
				}
				else if(target)
				{
					cPathCalculator pc(selectedVehicle->PosX, selectedVehicle->PosY, *client->getMap(), *selectedVehicle, mouseTilePosition.x(), mouseTilePosition.y());
					sWaypoint* path = pc.calcPath();
					if(path)
					{
						sendMoveJob(*client, path, selectedVehicle->iID);
						sendEndMoveAction(*client, selectedVehicle->iID, target->iID, EMAT_ATTACK);
					}
					else
					{
						PlayRandomVoice(VoiceData.VOINoPath);
					}
				}
			}
			else if(changeAllowed && mouse.getCursorType() == eMouseCursorType::Attack && selectedBuilding && !selectedBuilding->attacking)
			{
				// find target ID
				int targetId = 0;
				cUnit* target = selectTarget(mouseTilePosition.x(), mouseTilePosition.y(), selectedBuilding->data.canAttack, *client->getMap());
				if(target && target->isAVehicle()) targetId = target->iID;
				const cMap& map = *client->getMap();

				const int offset = map.getOffset(selectedBuilding->PosX, selectedBuilding->PosY);
				sendWantAttack(*client, targetId, map.getOffset(mouseTilePosition.x(), mouseTilePosition.y()), offset, false);
			}
			else if(changeAllowed && mouse.getCursorType() == eMouseCursorType::Steal && selectedVehicle)
			{
				if(overVehicle) sendWantComAction(*client, selectedVehicle->iID, overVehicle->iID, true, true);
				else if(overPlane && overPlane->FlightHigh == 0) sendWantComAction(*client, selectedVehicle->iID, overVehicle->iID, true, true);
			}
			else if(changeAllowed && mouse.getCursorType() == eMouseCursorType::Disable && selectedVehicle)
			{
				if(overVehicle) sendWantComAction(*client, selectedVehicle->iID, overVehicle->iID, true, false);
				else if(overPlane && overPlane->FlightHigh == 0) sendWantComAction(*client, selectedVehicle->iID, overPlane->iID, true, false);
				else if(overBuilding) sendWantComAction(*client, selectedVehicle->iID, overBuilding->iID, false, false);
			}
			else if(MouseStyle == OldSchool && overUnitField && selectUnit(overUnitField, false))
			{
			}
			else if(changeAllowed && mouse.getCursorType() == eMouseCursorType::Move && selectedVehicle && !selectedVehicle->moving && !selectedVehicle->attacking)
			{
				if(selectedVehicle->IsBuilding)
				{
					sendWantEndBuilding(*client, *selectedVehicle, mouseTilePosition.x(), mouseTilePosition.y());
				}
				else
				{
					if(selectedVehiclesGroup.size() > 1) client->startGroupMove(selectedVehiclesGroup, mouseTilePosition.x(), mouseTilePosition.y());
					else client->addMoveJob(*selectedVehicle, mouseTilePosition.x(), mouseTilePosition.y());
				}
			}
			else if(overUnitField)
			{
				// open unit menu
				if(changeAllowed && selectedVehicle && (Contains(overUnitField->getPlanes(), selectedVehicle) || overVehicle == selectedVehicle))
				{
					if(!selectedVehicle->moving)
					{
						unitMenuActive = !unitMenuActive;
						if(unitMenuActive) selectedMenuButtonIndex = -1;
						PlayFX(SoundData.SNDHudButton);
					}
				}
				else if(changeAllowed && selectedBuilding && (overBaseBuilding == selectedBuilding || overBuilding == selectedBuilding))
				{
					unitMenuActive = !unitMenuActive;
					if(unitMenuActive) selectedMenuButtonIndex = -1;
					PlayFX(SoundData.SNDHudButton);
				}
				// select unit when using modern style
				else if(MouseStyle == Modern) selectUnit(overUnitField, true);
			}
		}
		else if(overUnitField)
		{
			if(overPlane)
			{
				cUnitHelpMenu helpMenu(&overPlane->data, overPlane->owner);
				switchTo(helpMenu, client);
			}
			else if(overVehicle)
			{
				cUnitHelpMenu helpMenu(&overVehicle->data, overVehicle->owner);
				switchTo(helpMenu, client);
			}
			else if(overBuilding)
			{
				cUnitHelpMenu helpMenu(&overBuilding->data, overBuilding->owner);
				switchTo(helpMenu, client);
			}
			else if(overBaseBuilding)
			{
				cUnitHelpMenu helpMenu(&overBaseBuilding->data, overBaseBuilding->owner);
				switchTo(helpMenu, client);
			}
			helpActive = false;
		}

		// toggle the lock state of an enemy unit,
		// if it is newly selected / deselected
		if(overUnitField && lockChecked())
		{
			cPlayer& player = client->getActivePlayer();
			if(selectedUnit && selectedUnit != oldSelectedUnitForLock && selectedUnit->owner != &player)
			{
				player.toggleLock(*overUnitField);
			}
		}

		mouseBox.invalidate();
	}
}

void cGameGUI::mouseWheelMoved(cMouse& mouse, const cPosition& amount)
{
	const bool wheelUp = amount.y() > 0;
	const bool wheelDown = amount.y() < 0;

	if(wheelUp)
	{
		setZoom(getZoom() + 0.05f, true, true);
	}
	else if(wheelDown)
	{
		setZoom(getZoom() - 0.05f, true, true);
	}
}

void cGameGUI::keyPressed(cKeyboard& keyboard, SDL_Keycode key)
{
	// first check whether the end key was pressed
	if((activeItem != &chatBox || chatBox.isDisabled()) && activeItem != &selUnitNameEdit && key == KeysList.KeyEndTurn && !client->isFreezed())
	{
		if(!endButton.getIsClicked()) endButton.clicked(this);
		return;
	}

	// check whether the player wants to abort waiting
	if(client->getFreezeMode(FREEZE_WAIT_FOR_RECONNECT) && key == SDLK_F2)
	{
		sendAbortWaiting(*client);
	}


	cVehicle* selectedVehicle = getSelectedVehicle();
	cBuilding* selectedBuilding = getSelectedBuilding();
	cPlayer& player = client->getActivePlayer();
	const cMap& map = *client->getMap();

	if(key == KeysList.KeyExit)
	{
		cDialogYesNo yesNoDialog(lngPack.i18n("Text~Comp~End_Game"));
		if(switchTo(yesNoDialog, client) == 0) end = true;
	}
	else if(activeItem && !activeItem->isDisabled() && false/*&& activeItem->handleKeyInput(keyboard, key, this)*/)
	{
	}
	else if(key == KeysList.KeyJumpToAction)
	{
		if(msgCoordsX != -1)
		{
			const int offsetX = msgCoordsX * 64 - ((int)(((float)(Video.getResolutionX() - HUD_TOTAL_WIDTH) / (2 * getTileSize())) * 64.f)) + 32;
			const int offsetY = msgCoordsY * 64 - ((int)(((float)(Video.getResolutionY() - HUD_TOTAL_HIGHT) / (2 * getTileSize())) * 64.f)) + 32;
			setOffsetPosition(offsetX, offsetY);
			msgCoordsX = -1;
		}
	}
	else if(key == KeysList.KeyChat)
	{
		if(!keyboard.isAnyModifierActive(toEnumFlag(eKeyModifierType::AltLeft) | toEnumFlag(eKeyModifierType::AltRight)))
		{
			if(chatBox.isDisabled()) chatBox.setDisabled(false);
			activeItem = &chatBox;
			chatBox.setActivity(true);
		}
	}
	// scroll and zoom hotkeys
	else if(key == KeysList.KeyScroll1) doScroll(1);
	else if(key == KeysList.KeyScroll3) doScroll(3);
	else if(key == KeysList.KeyScroll7) doScroll(7);
	else if(key == KeysList.KeyScroll9) doScroll(9);
	else if(key == KeysList.KeyScroll2a || key == KeysList.KeyScroll2b) doScroll(2);
	else if(key == KeysList.KeyScroll4a || key == KeysList.KeyScroll4b) doScroll(4);
	else if(key == KeysList.KeyScroll6a || key == KeysList.KeyScroll6b) doScroll(6);
	else if(key == KeysList.KeyScroll8a || key == KeysList.KeyScroll8b) doScroll(8);
	else if(key == KeysList.KeyZoomIna || key == KeysList.KeyZoomInb) setZoom(getZoom() + 0.05f, true, false);
	else if(key == KeysList.KeyZoomOuta || key == KeysList.KeyZoomOutb) setZoom(getZoom() - 0.05f, true, false);
	// position handling hotkeys
	else if(key == KeysList.KeyCenterUnit && selectedUnit) center(*selectedUnit);
	else if(key == SDLK_F5 && keyboard.isAnyModifierActive(toEnumFlag(eKeyModifierType::AltLeft) | toEnumFlag(eKeyModifierType::AltRight))) savePosition(0);
	else if(key == SDLK_F6 && keyboard.isAnyModifierActive(toEnumFlag(eKeyModifierType::AltLeft) | toEnumFlag(eKeyModifierType::AltRight))) savePosition(1);
	else if(key == SDLK_F7 && keyboard.isAnyModifierActive(toEnumFlag(eKeyModifierType::AltLeft) | toEnumFlag(eKeyModifierType::AltRight))) savePosition(2);
	else if(key == SDLK_F8 && keyboard.isAnyModifierActive(toEnumFlag(eKeyModifierType::AltLeft) | toEnumFlag(eKeyModifierType::AltRight))) savePosition(3);
	else if(key == SDLK_F5 && savedPositions[0].offsetX >= 0 && savedPositions[0].offsetY >= 0) jumpToSavedPos(0);
	else if(key == SDLK_F6 && savedPositions[1].offsetX >= 0 && savedPositions[1].offsetY >= 0) jumpToSavedPos(1);
	else if(key == SDLK_F7 && savedPositions[2].offsetX >= 0 && savedPositions[2].offsetY >= 0) jumpToSavedPos(2);
	else if(key == SDLK_F8 && savedPositions[3].offsetX >= 0 && savedPositions[3].offsetY >= 0) jumpToSavedPos(3);

	else if(key == KeysList.KeyUnitDone) { doneReleased(this); }
	else if(key == KeysList.KeyUnitDoneAndNext) { doneReleased(this); nextReleased(this); }
	else if(key == KeysList.KeyUnitNext) { nextReleased(this); }
	else if(key == KeysList.KeyUnitPrev) { prevReleased(this); }

	// Hotkeys for the unit menus
	// allowed KeyUnitMenuInfo for disabled units
	else if(key == KeysList.KeyUnitMenuInfo && selectedUnit)
	{
		cUnitHelpMenu helpMenu(&selectedUnit->data, selectedUnit->owner);
		switchTo(helpMenu, client);
	}
	// disable most hotkeys while selected unit is disabled
	else if(selectedUnit && selectedUnit->isDisabled() == false && selectedUnit->owner == &player && !client->isFreezed())
	{
		if(key == KeysList.KeyUnitMenuAttack && selectedUnit->data.canAttack && selectedUnit->data.shotsCur)
		{
			mouseInputMode = mouseInputAttackMode;
			updateMouseCursor();
		}
		else if(key == KeysList.KeyUnitMenuBuild && selectedVehicle && !selectedVehicle->data.canBuild.empty() && !selectedVehicle->IsBuilding)
		{
			sendWantStopMove(*client, selectedVehicle->iID);
			cBuildingsBuildMenu buildMenu(*client, &player, selectedVehicle);
			switchTo(buildMenu, client);
		}
		else if(key == KeysList.KeyUnitMenuBuild && selectedBuilding && !selectedBuilding->data.canBuild.empty())
		{
			cVehiclesBuildMenu buildMenu(*this, &player, selectedBuilding);
			switchTo(buildMenu, client);
		}
		else if(key == KeysList.KeyUnitMenuTransfer && selectedVehicle && selectedVehicle->data.storeResType != sUnitData::STORE_RES_NONE && !selectedVehicle->IsBuilding && !selectedVehicle->IsClearing)
		{
			mouseInputMode = transferMode;
		}
		else if(key == KeysList.KeyUnitMenuTransfer && selectedBuilding && selectedBuilding->data.storeResType != sUnitData::STORE_RES_NONE)
		{
			mouseInputMode = transferMode;
		}
		else if(key == KeysList.KeyUnitMenuAutomove && selectedVehicle && selectedVehicle->data.canSurvey)
		{
			for(size_t i = 1; i < selectedVehiclesGroup.size(); ++i)
			{
				selectedVehiclesGroup[i]->executeAutoMoveJobCommand(*client);
			}
			selectedVehicle->executeAutoMoveJobCommand(*client);
		}
		else if(key == KeysList.KeyUnitMenuStart && selectedBuilding && selectedBuilding->data.canWork && !selectedBuilding->IsWorking && (selectedBuilding->BuildList.size() || selectedBuilding->data.canBuild.empty()))
		{
			sendWantStartWork(*client, *selectedBuilding);
		}
		else if(key == KeysList.KeyUnitMenuStop && selectedVehicle && (selectedVehicle->ClientMoveJob || (selectedVehicle->IsBuilding && selectedVehicle->BuildRounds) || (selectedVehicle->IsClearing && selectedVehicle->ClearingRounds)))
		{
			if(selectedVehicle->ClientMoveJob)
			{
				for(size_t i = 1; i < selectedVehiclesGroup.size(); ++i)
				{
					if(selectedVehiclesGroup[i]->ClientMoveJob) sendWantStopMove(*client, selectedVehiclesGroup[i]->iID);
				}
				sendWantStopMove(*client, selectedVehicle->iID);
			}
			else if(selectedVehicle->IsBuilding)
			{
				for(size_t i = 1; i < selectedVehiclesGroup.size(); ++i)
				{
					if(selectedVehiclesGroup[i]->IsBuilding && selectedVehiclesGroup[i]->BuildRounds) sendWantStopBuilding(*client, selectedVehiclesGroup[i]->iID);
				}
				sendWantStopBuilding(*client, selectedVehicle->iID);
			}
			else if(selectedVehicle->IsClearing)
			{
				for(size_t i = 1; i < selectedVehiclesGroup.size(); ++i)
				{
					if(selectedVehiclesGroup[i]->IsClearing && selectedVehiclesGroup[i]->ClearingRounds) sendWantStopClear(*client, *selectedVehiclesGroup[i]);
				}
				sendWantStopClear(*client, *selectedVehicle);
			}
		}
		else if(key == KeysList.KeyUnitMenuStop && selectedBuilding && selectedBuilding->IsWorking && !client->isFreezed())
		{
			sendWantStopWork(*client, *selectedBuilding);
		}
		else if(key == KeysList.KeyUnitMenuClear && selectedVehicle && selectedVehicle->data.canClearArea && map.fields[map.getOffset(selectedVehicle->PosX, selectedVehicle->PosY)].getRubble() && !selectedVehicle->IsClearing)
		{
			for(size_t i = 1; i < selectedVehiclesGroup.size(); ++i)
			{
				if(selectedVehiclesGroup[i]->data.canClearArea && map.fields[map.getOffset(selectedVehiclesGroup[i]->PosX, selectedVehiclesGroup[i]->PosY)].getRubble() && !selectedVehiclesGroup[i]->IsClearing) sendWantStartClear(*client, *selectedVehiclesGroup[i]);
			}
			sendWantStartClear(*client, *selectedVehicle);
		}
		else if(key == KeysList.KeyUnitMenuSentry && selectedVehicle)
		{
			for(size_t i = 1; i < selectedVehiclesGroup.size(); ++i)
			{
				if(selectedVehicle->sentryActive == selectedVehiclesGroup[i]->sentryActive)
				{
					sendChangeSentry(*client, selectedVehiclesGroup[i]->iID, true);
				}
			}
			sendChangeSentry(*client, selectedVehicle->iID, true);
		}
		else if(key == KeysList.KeyUnitMenuSentry && selectedBuilding && (selectedBuilding->sentryActive || selectedBuilding->data.canAttack))
		{
			sendChangeSentry(*client, selectedBuilding->iID, false);
		}
		else if(key == KeysList.KeyUnitMenuManualFire && selectedVehicle && (selectedVehicle->manualFireActive || selectedVehicle->data.canAttack))
		{
			for(size_t i = 1; i < selectedVehiclesGroup.size(); ++i)
			{
				if((selectedVehiclesGroup[i]->manualFireActive || selectedVehiclesGroup[i]->data.canAttack)
				   && selectedVehicle->manualFireActive == selectedVehiclesGroup[i]->manualFireActive)
				{
					sendChangeManualFireStatus(*client, selectedVehiclesGroup[i]->iID, true);
				}
			}
			sendChangeManualFireStatus(*client, selectedVehicle->iID, true);
		}
		else if(key == KeysList.KeyUnitMenuManualFire && selectedBuilding && (selectedBuilding->manualFireActive || selectedBuilding->data.canAttack))
		{
			sendChangeManualFireStatus(*client, selectedBuilding->iID, false);
		}
		else if(key == KeysList.KeyUnitMenuActivate && selectedUnit && selectedUnit->data.storageUnitsMax > 0)
		{
			cStorageMenu storageMenu(*client, selectedUnit->storedUnits, *selectedUnit);
			switchTo(storageMenu, client);
		}
		else if(key == KeysList.KeyUnitMenuLoad && selectedUnit && selectedUnit->data.storageUnitsMax > 0)
		{
			toggleMouseInputMode(loadMode);
		}
		else if(key == KeysList.KeyUnitMenuReload && selectedVehicle && selectedVehicle->data.canRearm && selectedVehicle->data.storageResCur >= 2)
		{
			mouseInputMode = muniActive;
		}
		else if(key == KeysList.KeyUnitMenuRepair && selectedVehicle && selectedVehicle->data.canRepair && selectedVehicle->data.storageResCur >= 2)
		{
			mouseInputMode = repairActive;
		}
		else if(key == KeysList.KeyUnitMenuLayMine && selectedVehicle && selectedVehicle->data.canPlaceMines && selectedVehicle->data.storageResCur > 0)
		{
			for(size_t i = 1; i < selectedVehiclesGroup.size(); ++i)
			{
				if(selectedVehiclesGroup[i]->data.canPlaceMines || selectedVehiclesGroup[i]->data.storageResCur > 0) selectedVehiclesGroup[i]->executeLayMinesCommand(*client);
			}
			selectedVehicle->executeLayMinesCommand(*client);
		}
		else if(key == KeysList.KeyUnitMenuClearMine && selectedVehicle && selectedVehicle->data.canPlaceMines && selectedVehicle->data.storageResCur < selectedVehicle->data.storageResMax)
		{
			for(size_t i = 1; i < selectedVehiclesGroup.size(); ++i)
			{
				if(selectedVehiclesGroup[i]->data.canPlaceMines || selectedVehiclesGroup[i]->data.storageResCur < selectedVehiclesGroup[i]->data.storageResMax) selectedVehiclesGroup[i]->executeClearMinesCommand(*client);
			}
			selectedVehicle->executeClearMinesCommand(*client);
		}
		else if(key == KeysList.KeyUnitMenuDisable && selectedVehicle && selectedVehicle->data.canDisable && selectedVehicle->data.shotsCur)
		{
			mouseInputMode = disableMode;
		}
		else if(key == KeysList.KeyUnitMenuSteal && selectedVehicle && selectedVehicle->data.canCapture && selectedVehicle->data.shotsCur)
		{
			mouseInputMode = stealMode;
		}
		else if(key == KeysList.KeyUnitMenuDistribute && selectedBuilding && selectedBuilding->data.canMineMaxRes > 0 && selectedBuilding->IsWorking)
		{
			cMineManagerMenu mineManager(*client, selectedBuilding);
			switchTo(mineManager, client);
		}
		else if(key == KeysList.KeyUnitMenuResearch && selectedBuilding && selectedBuilding->data.canResearch && selectedBuilding->IsWorking)
		{
			cDialogResearch researchDialog(*client);
			switchTo(researchDialog, client);
		}
		else if(key == KeysList.KeyUnitMenuUpgrade && selectedBuilding && selectedBuilding->data.convertsGold)
		{
			cUpgradeMenu upgradeMenu(*client);
			switchTo(upgradeMenu, client);
		}
		else if(key == KeysList.KeyUnitMenuDestroy && selectedBuilding && selectedBuilding->data.canSelfDestroy)
		{
			cDestructMenu destructMenu;
			if(switchTo(destructMenu, client) == 0) sendWantSelfDestroy(*client, *selectedBuilding);
		}
	}
	// Hotkeys for the hud
	else if(key == KeysList.KeyFog) setFog(!fogChecked());
	else if(key == KeysList.KeyGrid) setGrid(!gridChecked());
	else if(key == KeysList.KeyScan) setScan(!scanChecked());
	else if(key == KeysList.KeyRange) setRange(!rangeChecked());
	else if(key == KeysList.KeyAmmo) setAmmo(!ammoChecked());
	else if(key == KeysList.KeyHitpoints) setHits(!hitsChecked());
	else if(key == KeysList.KeyColors) setColor(!colorChecked());
	else if(key == KeysList.KeyStatus) setStatus(!statusChecked());
	else if(key == KeysList.KeySurvey) setSurvey(!surveyChecked());
}

void cGameGUI::keyReleased(cKeyboard& keyboard, SDL_Keycode key)
{
	// check whether the end key was released
	if((activeItem != &chatBox || chatBox.isDisabled()) && activeItem != &selUnitNameEdit && key == KeysList.KeyEndTurn && !client->isFreezed())
	{
		if(endButton.getIsClicked() && !client->bWantToEnd) endButton.released(this);
		return;
	}
}

void cGameGUI::screenShotTaken(const std::string& fileName)
{
	addMessage(lngPack.i18n("Text~Comp~Screenshot_Done", fileName));
}

void cGameGUI::setClient (cClient* client)
{
	assert (client);
	assert (client->getMap());

	this->client = client;
	debugOutput.setClient (client);
	debugOutput.setServer (client->getServer());

	zoomSlider.setBorders (calcMinZoom(), 1.f);
	{
		AutoSurface mini (generateMiniMapSurface());
		miniMapImage.setImage (mini);
		needMiniMapDraw = false;
	}
	playersInfo.setClient (*client);
}

float cGameGUI::calcMinZoom() const
{
	const int mapSize = client->getMap()->getSize();
	float minZoom = (float) ((max (Video.getResolutionY() - HUD_TOTAL_HIGHT, Video.getResolutionX() - HUD_TOTAL_WIDTH) / (float) mapSize) / 64.0f);
	minZoom = max (minZoom, ((int) (64.0f * minZoom) + (minZoom >= 1.0f ? 0 : 1)) / 64.0f);

	return minZoom;
}

void cGameGUI::recalcPosition (bool resetItemPositions)
{
	background = generateSurface();
	cMenu::recalcPosition (resetItemPositions);

	// reset minimal zoom
	const float minZoom = calcMinZoom();
	zoomSlider.setBorders (minZoom, 1.0f);
	setZoom (zoom, true, false);

	// move some items around
	coordsLabel.move (coordsLabel.getPosition().x, (Video.getResolutionY() - 21) + 3);
	unitNameLabel.move (unitNameLabel.getPosition().x, (Video.getResolutionY() - 21) + 3);
	chatBox.move (chatBox.getPosition().x, Video.getResolutionY() - 48);
	infoTextLabel.move (HUD_LEFT_WIDTH + (Video.getResolutionX() - HUD_TOTAL_WIDTH) / 2, infoTextLabel.getPosition().y);
	infoTextAdditionalLabel.move (HUD_LEFT_WIDTH + (Video.getResolutionX() - HUD_TOTAL_WIDTH) / 2, 235 + font->getFontHeight (FONT_LATIN_BIG));
}

cGameGUI::~cGameGUI()
{
	stopFXLoop();

	SDL_RemoveTimer (TimerID);

	if (FLC) FLI_Close (FLC);

	for (size_t i = 0; i != messages.size(); ++i)
	{
		delete messages[i];
	}
}

void cGameGUI::playFXLoop (sSOUND* sound)
{
	iObjectStream = PlayFXLoop (sound);
}

void cGameGUI::stopFXLoop()
{
	StopFXLoop (iObjectStream);
	iObjectStream = -1;
}

//----------------------------------------------------------------
/** Playback of the soundstream that belongs to this building */
//----------------------------------------------------------------
void cGameGUI::playStream (const cBuilding& building)
{
	if (building.IsWorking)
		playFXLoop (building.uiData->Running);
	else
		playFXLoop (building.uiData->Wait);
}

//----------------------------------------------------------------
/** Playback of the soundstream that belongs to this vehicle */
//----------------------------------------------------------------
void cGameGUI::playStream (const cVehicle& vehicle)
{
	const cMap& map = *client->getMap();
	const cBuilding* building = map[map.getOffset (vehicle.PosX, vehicle.PosY)].getBaseBuilding();
	bool water = map.isWater (vehicle.PosX, vehicle.PosY);
	if (vehicle.data.factorGround > 0 && building && (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)) water = false;

	if (vehicle.IsBuilding && (vehicle.BuildRounds || &client->getActivePlayer() != vehicle.owner))
		playFXLoop (SoundData.SNDBuilding);
	else if (vehicle.IsClearing)
		playFXLoop (SoundData.SNDClearing);
	else if (water && vehicle.data.factorSea > 0)
		playFXLoop (vehicle.uiData->WaitWater);
	else
		playFXLoop (vehicle.uiData->Wait);
}

//-----------------------------------------------------------------------------
/** Starts the MoveSound */
//-----------------------------------------------------------------------------
void cGameGUI::startMoveSound (const cVehicle& vehicle)
{
	const cMap& map = *client->getMap();
	const cBuilding* building = map.fields[map.getOffset (vehicle.PosX, vehicle.PosY)].getBaseBuilding();
	bool water = map.isWater (vehicle.PosX, vehicle.PosY);
	if (vehicle.data.factorGround > 0 && building && (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)) water = false;
	stopFXLoop();

	if (!vehicle.MoveJobActive)
	{
		if (water && vehicle.data.factorSea != 0)
			PlayFX (vehicle.uiData->StartWater);
		else
			PlayFX (vehicle.uiData->Start);
	}

	if (water && vehicle.data.factorSea != 0)
		playFXLoop (vehicle.uiData->DriveWater);
	else
		playFXLoop (vehicle.uiData->Drive);
}

void cGameGUI::Timer()
{
	iTimerTime++;
}

void cGameGUI::handleTimer()
{
	static unsigned int iLast = 0;
	timer50ms = false;
	timer100ms = false;
	timer400ms = false;
	if (iTimerTime == iLast) return;

	iLast = iTimerTime;
	timer50ms = true;
	if (iTimerTime & 0x1) timer100ms = true;
	if ((iTimerTime & 0x3) == 3) timer400ms = true;
}

void cGameGUI::handleMessages()
{
	if (messages.empty()) return;
	int iHeight = 0;
	//delete old messages
	for (int i = (int) messages.size() - 1; i >= 0; i--)
	{
		sMessage* message = messages[i];
		if (message->age + MSG_TICKS < SDL_GetTicks() || iHeight > 200)
		{
			delete message;
			messages.erase (messages.begin() + i);
			continue;
		}
		iHeight += 17 + font->getFontHeight() * (message->len / (Video.getResolutionX() - 300));
	}
}

void cGameGUI::addMessage (const string& sMsg)
{
	sMessage* const Message = new sMessage (sMsg, SDL_GetTicks());
	messages.push_back (Message);
	if (cSettings::getInstance().isDebug()) Log.write (Message->msg, cLog::eLOG_TYPE_DEBUG);
}

void cGameGUI::addCoords (const sSavedReportMessage& msg)
{
	//e.g. [85,22] missel MK I is under attack (F1)
	const string str = msg.getFullMessage() + " (" + GetKeyString (KeysList.KeyJumpToAction) + ")";
	addMessage (str);
	msgCoordsX = msg.xPos;
	msgCoordsY = msg.yPos;
}

int cGameGUI::show (cClient* client)
{
	activate();
	auto deactivator = makeScopedOperation(std::bind(&cGameGUI::deactivate, this));

	// do startup actions
	openPanel();
	startup = true;
	if (client->isFreezed()) setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_Until", client->getPlayerFromNumber (0)->getName()), "");

	int lastMouseX = 0;
	int lastMouseY = 0;

	while (!exiting())
	{
		cEventManager::getInstance().run();
		client->gameTimer.run (this);

		if (!cSettings::getInstance().shouldUseFastMode()) SDL_Delay (1);

		if (startup)
		{
			const cPlayer& player = client->getActivePlayer();

			if (player.BuildingList) center (*player.BuildingList);
			else if (player.VehicleList) center (*player.VehicleList);
			startup = false;
		}

		handleMessages();

		handleTimer();
		if (timer50ms)
		{
			// run effects, which are not synchronous to game time
			runFx();
			client->handleTurnTime(); // TODO: remove
		}

		checkScroll(cMouse::getInstance());
		changeWindDir();
		updateStatusText();

		if (needMiniMapDraw)
		{
			AutoSurface mini (generateMiniMapSurface());
			miniMapImage.setImage (mini);
			needMiniMapDraw = false;
		}
		if (timer100ms)
		{
			if (FLC != NULL && playFLC)
			{
				FLI_NextFrame (FLC);
				FLCImage.setImage (FLC->surface);
			}
			rotateBlinkColor();
		}

		draw();
		frame++;

		handleFramesPerSecond();
	}

	// code to work with DEDICATED_SERVER:
	// network client sends the server, that it disconnects itself
	if (client->getServer() == 0)
	{
		cNetMessage* message = new cNetMessage (GAME_EV_WANT_DISCONNECT);
		client->sendNetMessage (message);
	}
	// end

	closePanel();

	// flush event queue before exiting menu
	cEventManager::getInstance().run();

	if (end) return 0;
	assert (terminate);
	return 1;
}

void cGameGUI::updateInfoTexts()
{
	const int playerNumber = client->getFreezeInfoPlayerNumber();
	const cPlayer* player = client->getPlayerFromNumber (playerNumber);

	if (client->getFreezeMode (FREEZE_WAIT_FOR_OTHERS))
	{
		setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_Until", player->getName()), "");
	}
	else if (client->getFreezeMode (FREEZE_PAUSE))
	{
		setInfoTexts (lngPack.i18n ("Text~Multiplayer~Pause"), "");
	}
	else if (client->getFreezeMode (FREEZE_WAIT_FOR_SERVER))
	{
		setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_For_Server"), "");
	}
	else if (client->getFreezeMode (FREEZE_WAIT_FOR_RECONNECT))
	{
		std::string s = client->getServer() ? lngPack.i18n ("Text~Multiplayer~Abort_Waiting") : "";
		setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_Reconnect"), s);
	}
	else if (client->getFreezeMode (FREEZE_WAIT_FOR_PLAYER))
	{
		setInfoTexts (lngPack.i18n ("Text~Multiplayer~No_Response", player->getName()), "");
	}
	else if (client->getFreezeMode (FREEZE_WAIT_FOR_TURNEND))
	{
		setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_TurnEnd"), "");
	}
	else
	{
		setInfoTexts ("", "");
	}
}

void cGameGUI::onRemoveUnit (cUnit& unit)
{
	if (getSelectedUnit() == &unit)
	{
		deselectUnit();
	}
	if (unit.isAVehicle())
	{
		Remove (selectedVehiclesGroup, static_cast<cVehicle*> (&unit));
	}
	callMiniMapDraw();
}

void cGameGUI::onLostConnection()
{
	const string msgString = lngPack.i18n ("Text~Multiplayer~Lost_Connection", "server");
	addMessage (msgString);
	client->getActivePlayer().addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
	//TODO: ask user for reconnect
}

void cGameGUI::onAddedBuilding (const cBuilding& building)
{
	if (building.owner != &client->getActivePlayer()) return;
	if (building.data.ID == UnitsData.specialIDLandMine) PlayFX (SoundData.SNDLandMinePlace);
	else if (building.data.ID == UnitsData.specialIDSeaMine) PlayFX (SoundData.SNDSeaMinePlace);
}

void cGameGUI::onChat_errorMessage (const std::string& msg)
{
	PlayFX (SoundData.SNDQuitsch);
	addMessage (msg);
	client->getActivePlayer().addSavedReport (msg, sSavedReportMessage::REPORT_TYPE_COMP);
}

void cGameGUI::onChat_infoMessage (const std::string& msg)
{
	addMessage (msg);
	client->getActivePlayer().addSavedReport (msg, sSavedReportMessage::REPORT_TYPE_COMP);
}

void cGameGUI::onChat_userMessage (const std::string& msg)
{
	PlayFX (SoundData.SNDChat);
	addMessage (msg);
	client->getActivePlayer().addSavedReport (msg, sSavedReportMessage::REPORT_TYPE_CHAT);
}

void cGameGUI::onVehicleStored (const cUnit& storingUnit, const cVehicle& storedVehicle)
{
	auto mouseTilePosition = getTilePosition(cMouse::getInstance().getPosition());
	if(storedVehicle.PosX == mouseTilePosition.x() && storedVehicle.PosY == mouseTilePosition.y()) updateMouseCursor();

	checkMouseInputMode();

	if (&storedVehicle == getSelectedUnit()) deselectUnit();

	PlayFX (SoundData.SNDLoad);
}

SDL_Surface* cGameGUI::generateSurface()
{
	SDL_Surface* surface = SDL_CreateRGBSurface (0, Video.getResolutionX(), Video.getResolutionY(), Video.getColDepth(), 0, 0, 0, 0);

	SDL_FillRect (surface, NULL, 0x00FF00FF);
	SDL_SetColorKey (surface, SDL_TRUE, 0x00FF00FF);

	const std::string gfxPath = cSettings::getInstance().getGfxPath() + PATH_DELIMITER;
	{
		AutoSurface tmpSurface (LoadPCX (gfxPath + "hud_left.pcx"));
		if (tmpSurface)
		{
			SDL_BlitSurface (tmpSurface, NULL, surface, NULL);
		}
	}

	SDL_Rect dest;
	{
		AutoSurface tmpSurface (LoadPCX (gfxPath + "hud_top.pcx"));
		if (tmpSurface)
		{
			SDL_Rect src = {0, 0, Uint16 (tmpSurface->w), Uint16 (tmpSurface->h) };
			dest.x = HUD_LEFT_WIDTH;
			dest.y = 0;
			SDL_BlitSurface (tmpSurface, &src, surface, &dest);
			src.x = 1275;
			src.w = 18;
			src.h = HUD_TOP_HIGHT;
			dest.x = surface->w - HUD_TOP_HIGHT;
			SDL_BlitSurface (tmpSurface, &src, surface, &dest);
		}
	}

	{
		AutoSurface tmpSurface (LoadPCX (gfxPath + "hud_right.pcx"));
		if (tmpSurface)
		{
			SDL_Rect src = {0, 0, Uint16 (tmpSurface->w), Uint16 (tmpSurface->h) };
			dest.x = surface->w - HUD_RIGHT_WIDTH;
			dest.y = HUD_TOP_HIGHT;
			SDL_BlitSurface (tmpSurface, &src, surface, &dest);
		}
	}

	{
		AutoSurface tmpSurface (LoadPCX (gfxPath + "hud_bottom.pcx"));
		if (tmpSurface)
		{
			SDL_Rect src = {0, 0, Uint16 (tmpSurface->w), Uint16 (tmpSurface->h) };
			dest.x = HUD_LEFT_WIDTH;
			dest.y = surface->h - 24;
			SDL_BlitSurface (tmpSurface, &src, surface, &dest);
			src.x = 1275;
			src.w = 23;
			src.h = 24;
			dest.x = surface->w - 23;
			SDL_BlitSurface (tmpSurface, &src, surface, &dest);
			src.x = 1299;
			src.w = 16;
			src.h = 22;
			dest.x = HUD_LEFT_WIDTH - 16;
			dest.y = surface->h - 22;
			SDL_BlitSurface (tmpSurface, &src, surface, &dest);
		}
	}

	if (Video.getResolutionY() > 480)
	{
		AutoSurface tmpSurface (LoadPCX (gfxPath + "logo.pcx"));
		if (tmpSurface)
		{
			dest.x = 9;
			dest.y = Video.getResolutionY() - HUD_TOTAL_HIGHT - 15;
			SDL_BlitSurface (tmpSurface, NULL, surface, &dest);
		}
	}
	return surface;
}

void cGameGUI::generateMiniMapSurface_landscape (SDL_Surface* minimapSurface, int zoomFactor)
{
	Uint32* minimap = static_cast<Uint32*> (minimapSurface->pixels);
	const cStaticMap& staticMap = *client->getMap()->staticMap;
	const int mapSize = client->getMap()->getSize();
	for (int miniMapX = 0; miniMapX < MINIMAP_SIZE; ++miniMapX)
	{
		// calculate the field on the map
		int terrainx = (miniMapX * mapSize) / (MINIMAP_SIZE * zoomFactor) + miniMapOffX;
		terrainx = std::min (terrainx, mapSize - 1);

		// calculate the position within the terrain graphic
		// (for better rendering of maps < 112)
		const int offsetx = ((miniMapX * mapSize) % (MINIMAP_SIZE * zoomFactor)) * 64 / (MINIMAP_SIZE * zoomFactor);

		for (int miniMapY = 0; miniMapY < MINIMAP_SIZE; ++miniMapY)
		{
			int terrainy = (miniMapY * mapSize) / (MINIMAP_SIZE * zoomFactor) + miniMapOffY;
			terrainy = std::min (terrainy, mapSize - 1);
			const int offsety = ((miniMapY * mapSize) % (MINIMAP_SIZE * zoomFactor)) * 64 / (MINIMAP_SIZE * zoomFactor);

			const sTerrain& terrain = staticMap.getTerrain (terrainx, terrainy);
			const Uint8* terrainPixels = reinterpret_cast<const Uint8*> (terrain.sf_org->pixels);
			const Uint8 index = terrainPixels[offsetx + offsety * 64];
			const SDL_Color sdlcolor = terrain.sf_org->format->palette->colors[index];
			const Uint32 color = (sdlcolor.r << 16) + (sdlcolor.g << 8) + sdlcolor.b;

			minimap[miniMapX + miniMapY * MINIMAP_SIZE] = color;
		}
	}
}

void cGameGUI::generateMiniMapSurface_fog (SDL_Surface* minimapSurface, int zoomFactor)
{
	Uint32* minimap = static_cast<Uint32*> (minimapSurface->pixels);
	const cPlayer& player = client->getActivePlayer();
	const cStaticMap& staticMap = *client->getMap()->staticMap;
	const int mapSize = client->getMap()->getSize();

	for (int miniMapX = 0; miniMapX < MINIMAP_SIZE; ++miniMapX)
	{
		int terrainx = (miniMapX * mapSize) / (MINIMAP_SIZE * zoomFactor) + miniMapOffX;
		for (int miniMapY = 0; miniMapY < MINIMAP_SIZE; ++miniMapY)
		{
			int terrainy = (miniMapY * mapSize) / (MINIMAP_SIZE * zoomFactor) + miniMapOffY;

			if (player.ScanMap[staticMap.getOffset (terrainx, terrainy)]) continue;

			Uint8* color = reinterpret_cast<Uint8*> (&minimap[miniMapX + miniMapY * MINIMAP_SIZE]);
			color[0] = static_cast<Uint8> (color[0] * 0.6f);
			color[1] = static_cast<Uint8> (color[1] * 0.6f);
			color[2] = static_cast<Uint8> (color[2] * 0.6f);
		}
	}
}

void cGameGUI::generateMiniMapSurface_units (SDL_Surface* minimapSurface, int zoomFactor)
{
	// here we go through each map field instead of
	// through each minimap pixel, to make sure,
	// that every unit is displayed and has the same size on the minimap.

	cMap& map = *client->getMap();
	const int mapSize = client->getMap()->getSize();
	// the size of the rect, that is drawn for each unit
	const int size = std::max (2, MINIMAP_SIZE * zoomFactor / mapSize);
	SDL_Rect rect;
	rect.w = size;
	rect.h = size;

	const cPlayer& player = client->getActivePlayer();
	for (int mapx = 0; mapx < mapSize; ++mapx)
	{
		rect.x = ((mapx - miniMapOffX) * MINIMAP_SIZE * zoomFactor) / mapSize;
		if (rect.x < 0 || rect.x >= MINIMAP_SIZE) continue;
		for (int mapy = 0; mapy < mapSize; ++mapy)
		{
			rect.y = ((mapy - miniMapOffY) * MINIMAP_SIZE * zoomFactor) / mapSize;
			if (rect.y < 0 || rect.y >= MINIMAP_SIZE) continue;

			const int offset = map.getOffset (mapx, mapy);
			if (!player.ScanMap[offset]) continue;

			cMapField& field = map[offset];

			// draw building
			const cBuilding* building = field.getBuilding();
			if (building && building->owner)
			{
				if (!tntChecked() || building->data.canAttack)
				{
					unsigned int color = *static_cast<Uint32*> (building->owner->getColorSurface()->pixels);
					SDL_FillRect (minimapSurface, &rect, color);
				}
			}

			// draw vehicle
			const cVehicle* vehicle = field.getVehicle();
			if (vehicle)
			{
				if (!tntChecked() || vehicle->data.canAttack)
				{
					unsigned int color = *static_cast<Uint32*> (vehicle->owner->getColorSurface()->pixels);
					SDL_FillRect (minimapSurface, &rect, color);
				}
			}

			// draw plane
			vehicle = field.getPlane();
			if (vehicle)
			{
				if (!tntChecked() || vehicle->data.canAttack)
				{
					unsigned int color = *static_cast<Uint32*> (vehicle->owner->getColorSurface()->pixels);
					SDL_FillRect (minimapSurface, &rect, color);
				}
			}
		}
	}
}

void cGameGUI::generateMiniMapSurface_borders (SDL_Surface* minimapSurface, int zoomFactor)
{
	const int mapSize = client->getMap()->getSize();
	int startx = (int) (((offX / 64.0f - miniMapOffX) * MINIMAP_SIZE * zoomFactor) / mapSize);
	int starty = (int) (((offY / 64.0f - miniMapOffY) * MINIMAP_SIZE * zoomFactor) / mapSize);
	int endx = (int) (startx + ((Video.getResolutionX() - HUD_TOTAL_WIDTH) * MINIMAP_SIZE * zoomFactor) / (mapSize * (getZoom() * 64.0f)));
	int endy = (int) (starty + ((Video.getResolutionY() - HUD_TOTAL_HIGHT) * MINIMAP_SIZE * zoomFactor) / (mapSize * (getZoom() * 64.0f)));

	// workaround
	if (endx == MINIMAP_SIZE) endx = MINIMAP_SIZE - 1;
	if (endy == MINIMAP_SIZE) endy = MINIMAP_SIZE - 1;

	Uint32* minimap = static_cast<Uint32*> (minimapSurface->pixels);
	for (int y = starty; y <= endy; ++y)
	{
		if (y < 0 || y >= MINIMAP_SIZE) continue;
		if (startx >= 0 && startx < MINIMAP_SIZE)
		{
			minimap[y * MINIMAP_SIZE + startx] = MINIMAP_COLOR;
		}
		if (endx >= 0 && endx < MINIMAP_SIZE)
		{
			minimap[y * MINIMAP_SIZE + endx] = MINIMAP_COLOR;
		}
	}
	for (int x = startx; x <= endx; ++x)
	{
		if (x < 0 || x >= MINIMAP_SIZE) continue;
		if (starty >= 0 && starty < MINIMAP_SIZE)
		{
			minimap[starty * MINIMAP_SIZE + x] = MINIMAP_COLOR;
		}
		if (endy >= 0 && endy < MINIMAP_SIZE)
		{
			minimap[endy * MINIMAP_SIZE + x] = MINIMAP_COLOR;
		}
	}
}

SDL_Surface* cGameGUI::generateMiniMapSurface()
{
	SDL_Surface* minimapSurface = SDL_CreateRGBSurface (0, MINIMAP_SIZE, MINIMAP_SIZE, 32, 0, 0, 0, 0);

	// set zoom factor
	const int displayedMapWidth = (int) ((Video.getResolutionX() - HUD_TOTAL_WIDTH) / getZoom());
	const int displayedMapHight = (int) ((Video.getResolutionY() - HUD_TOTAL_HIGHT) / getZoom());
	const int zoomFactor = twoXChecked() ? MINIMAP_ZOOM_FACTOR : 1;

	if (zoomFactor != 1)
	{
		if (offX < miniMapOffX * 64) miniMapOffX -= cSettings::getInstance().getScrollSpeed() / 10;
		else if (offX + displayedMapWidth > miniMapOffX * 64 + (MINIMAP_SIZE * 64) / MINIMAP_ZOOM_FACTOR) miniMapOffX += cSettings::getInstance().getScrollSpeed() / 10;

		if (offY < miniMapOffY * 64) miniMapOffY -= cSettings::getInstance().getScrollSpeed() / 10;
		else if (offY + displayedMapHight > miniMapOffY * 64 + (MINIMAP_SIZE * 64) / MINIMAP_ZOOM_FACTOR) miniMapOffY += cSettings::getInstance().getScrollSpeed() / 10;

		const int mapSize = client->getMap()->getSize();
		miniMapOffX = std::max (miniMapOffX, 0);
		miniMapOffY = std::max (miniMapOffY, 0);
		miniMapOffX = std::min (mapSize - (mapSize / zoomFactor), miniMapOffX);
		miniMapOffY = std::min (mapSize - (mapSize / zoomFactor), miniMapOffY);
	}

	// draw the landscape
	generateMiniMapSurface_landscape (minimapSurface, zoomFactor);

	// draw the fog
	generateMiniMapSurface_fog (minimapSurface, zoomFactor);

	// draw the units
	generateMiniMapSurface_units (minimapSurface, zoomFactor);

	// draw the screen borders
	generateMiniMapSurface_borders (minimapSurface, zoomFactor);

	return minimapSurface;
}

void cGameGUI::setOffsetPosition (int x, int y)
{
	offX = x;
	offY = y;

	checkOffsetPosition();
}

void cGameGUI::checkOffsetPosition()
{
	offX = max (offX, 0);
	offY = max (offY, 0);
	const int mapSize = client->getMap()->getSize();

	const int maxX = mapSize * 64 - (int) ((Video.getResolutionX() - HUD_TOTAL_WIDTH) / getZoom());
	const int maxY = mapSize * 64 - (int) ((Video.getResolutionY() - HUD_TOTAL_HIGHT) / getZoom());
	offX = min (offX, maxX);
	offY = min (offY, maxY);

	callMiniMapDraw();
	updateMouseCursor();
}

void cGameGUI::setZoom (float newZoom, bool setScroller, bool centerToMouse)
{
	zoom = newZoom;
	zoom = std::max (zoom, this->zoomSlider.getMinValue());
	zoom = std::min (zoom, 1.f);
	if (setScroller) this->zoomSlider.setValue (zoom);

	static float lastZoom = 1.f;
	if (lastZoom != getZoom())
	{
		const auto& mousePosition = cMouse::getInstance().getPosition();
		// change x screen offset
		float lastScreenPixel = (Video.getResolutionX() - HUD_TOTAL_WIDTH) / lastZoom;
		float newScreenPixel  = (Video.getResolutionX() - HUD_TOTAL_WIDTH) / getZoom();
		int off;
		if (centerToMouse)
		{
			off = (int)((lastScreenPixel - newScreenPixel) * (mousePosition.x() - HUD_LEFT_WIDTH) / (Video.getResolutionX() - HUD_TOTAL_WIDTH));
		}
		else
		{
			off = (int) (lastScreenPixel - newScreenPixel) / 2;
		}

		offX += off;

		// change y screen offset
		lastScreenPixel = (Video.getResolutionY() - HUD_TOTAL_HIGHT) / lastZoom;
		newScreenPixel  = (Video.getResolutionY() - HUD_TOTAL_HIGHT) / getZoom();
		if (centerToMouse)
		{
			off = (int)((lastScreenPixel - newScreenPixel) * (mousePosition.y() - HUD_TOP_HIGHT) / (Video.getResolutionY() - HUD_TOTAL_HIGHT));
		}
		else
		{
			off = (int) (lastScreenPixel - newScreenPixel) / 2;
		}

		offY += off;

		lastZoom = getZoom();
	}
	if (cSettings::getInstance().shouldDoPrescale()) scaleSurfaces();
	scaleColors();

	checkOffsetPosition();
}

float cGameGUI::getZoom() const
{
	return getTileSize() / 64.f;
}

int cGameGUI::getTileSize() const
{
	return Round (64.f * zoom);
}

void cGameGUI::setVideoSurface (SDL_Surface* videoSurface)
{
	FLCImage.setImage (videoSurface);
}

void cGameGUI::setFLC (FLI_Animation* FLC_)
{
	FLC = FLC_;
	if (FLC)
	{
		FLI_Rewind (FLC);
		FLI_NextFrame (FLC);
		FLCImage.setImage (FLC->surface);
	}
	else FLCImage.setImage (NULL);
}

void cGameGUI::setUnitDetailsData (cUnit* unit)
{
	unitDetails.setSelection (*client, unit);

	if (unit)
	{
		selUnitNamePrefixStr.setText (unit->getNamePrefix());
		selUnitNameEdit.setText (unit->isNameOriginal() ? unit->data.name : unit->getName());
	}
	else
	{
		selUnitNamePrefixStr.setText ("");
		selUnitNameEdit.setText ("");
		selUnitNameEdit.setLocked (true);
		return;
	}

	selUnitNameEdit.setLocked (false);
	const int xPosition = 12 + font->getTextWide (selUnitNamePrefixStr.getText() + " ", FONT_LATIN_SMALL_GREEN);
	selUnitNameEdit.move (xPosition, 30);
	selUnitNameEdit.setSize (135 - xPosition, 10);
}

void cGameGUI::updateTurn (int turn)
{
	turnLabel.setText (iToStr (turn));
}

void cGameGUI::updateTurnTime (int time)
{
	timeLabel.setText (time < 0 ? "" : iToStr (time));
}

void cGameGUI::callMiniMapDraw()
{
	needMiniMapDraw = true;
}

void cGameGUI::setEndButtonLock (bool locked)
{
	endButton.setLocked (locked);
}

void cGameGUI::handleFramesPerSecond()
{
	static Uint32 lastTicks = 0;
	static Uint32 lastFrame = 0;
	static Uint32 cycles = 0;
	static Uint32 inverseLoad = 0; // this is 10*(100% - load)
	static Uint32 lastTickLoad = 0;

	cycles++;
	const Uint32 ticks = SDL_GetTicks();

	if (ticks != lastTickLoad) inverseLoad++;
	lastTickLoad = ticks;

	if (ticks > lastTicks + 1000)
	{
		const float a = 0.5f; // low pass filter coefficient

		framesPerSecond = (1.f - a) * (frame - lastFrame) * 1000.f / (ticks - lastTicks) + a * framesPerSecond;
		lastFrame = frame;

		cyclesPerSecond = (1.f - a) * cycles * 1000.f / (ticks - lastTicks) + a * cyclesPerSecond;
		cycles = 0;

		loadValue = Round ((1.f - a) * (1000.f - inverseLoad) + a * loadValue);
		inverseLoad = 0;

		lastTicks = ticks;
	}
}

void cGameGUI::rotateBlinkColor()
{
	static bool dec = true;
	if (dec)
	{
		blinkColor -= 0x000A0A0A;
		if (blinkColor <= 0x00A0A0A0) dec = false;
	}
	else
	{
		blinkColor += 0x000A0A0A;
		if (blinkColor >= 0x00FFFFFF) dec = true;
	}
}

bool cGameGUI::checkScroll(cMouse& mouse)
{
	if (mouse.getPosition().x() <= 0 && mouse.getPosition().y() > 30 && mouse.getPosition().y() < Video.getResolutionY() - 30 - 18)
	{
		mouse.setCursorType(eMouseCursorType::ArrowLeft);
		doScroll (4);
		return true;
	}
	else if(mouse.getPosition().x() >= Video.getResolutionX() - 18 && mouse.getPosition().y() > 30 && mouse.getPosition().y() < Video.getResolutionY() - 30 - 18)
	{
		mouse.setCursorType(eMouseCursorType::ArrowRight);
		doScroll (6);
		return true;
	}
	else if((mouse.getPosition().x() <= 0 && mouse.getPosition().y() <= 30) || (mouse.getPosition().y() <= 0 && mouse.getPosition().x() <= 30))
	{
		mouse.setCursorType(eMouseCursorType::ArrowLeftUp);
		doScroll (7);
		return true;
	}
	else if((mouse.getPosition().x() >= Video.getResolutionX() - 18 && mouse.getPosition().y() <= 30) || (mouse.getPosition().y() <= 0 && mouse.getPosition().x() >= Video.getResolutionX() - 30 - 18))
	{
		mouse.setCursorType(eMouseCursorType::ArrowRightUp);
		doScroll (9);
		return true;
	}
	else if (mouse.getPosition().y() <= 0 && mouse.getPosition().x() > 30 && mouse.getPosition().x() < Video.getResolutionX() - 30 - 18)
	{
		mouse.setCursorType(eMouseCursorType::ArrowUp);
		doScroll (8);
		return true;
	}
	else if (mouse.getPosition().y() >= Video.getResolutionY() - 18 && mouse.getPosition().x() > 30 && mouse.getPosition().x() < Video.getResolutionX() - 30 - 18)
	{
		mouse.setCursorType(eMouseCursorType::ArrowDown);
		doScroll (2);
		return true;
	}
	else if ((mouse.getPosition().x() <= 0 && mouse.getPosition().y() >= Video.getResolutionY() - 30 - 18) || (mouse.getPosition().y() >= Video.getResolutionY() - 18 && mouse.getPosition().x() <= 30))
	{
		mouse.setCursorType(eMouseCursorType::ArrowLeftDown);
		doScroll (1);
		return true;
	}
	else if ((mouse.getPosition().x() >= Video.getResolutionX() - 18 && mouse.getPosition().y() >= Video.getResolutionY() - 30 - 18) || (mouse.getPosition().y() >= Video.getResolutionY() - 18 && mouse.getPosition().x() >= Video.getResolutionX() - 30 - 18))
	{
		mouse.setCursorType(eMouseCursorType::ArrowRightDown);
		doScroll (3);
		return true;
	}
	return false;
}

void cGameGUI::updateUnderMouseObject(const cMouse& mouse)
{
	const auto mouseTilePosition = getTilePosition(mouse.getPosition());
	const int x = mouseTilePosition.x();
	const int y = mouseTilePosition.y();

	const cMap& map = *client->getMap();

	if (map.isValidPos (x, y) == false) return;

	// draw the coordinates:

	// array to get map coords in scheme XXX-YYY\0 = 8 characters
	// a case where I accept an array since I don't know a better
	// method to format x and y easily with leading 0 -- beko
	char str[8];
	TIXML_SNPRINTF (str, sizeof (str), "%.3d-%.3d", x, y);
	coordsLabel.setText (str);
	const cPlayer& player = client->getActivePlayer();

	if (!player.ScanMap[map.getOffset (x, y)])
	{
		overUnitField = NULL;
		if(mouse.getCursorType() == eMouseCursorType::Attack)
		{
			SDL_Rect r = {1, 29, 35, 3};
			SDL_FillRect (GraphicsData.gfx_Cattack, &r, 0);
		}
		return;
	}
	// check whether there is a unit under the mouse:
	overUnitField = map.fields + (map.getOffset (x, y));
	cVehicle* selectedVehicle = getSelectedVehicle();
	if(mouse.getCursorType() == eMouseCursorType::Steal && selectedVehicle)
	{
		selectedVehicle->drawCommandoCursor (*this, x, y, true);
	}
	else if(mouse.getCursorType() == eMouseCursorType::Disable && selectedVehicle)
	{
		selectedVehicle->drawCommandoCursor (*this, x, y, false);
	}
	if (overUnitField->getVehicle() != NULL)
	{
		const cVehicle& vehicle = *overUnitField->getVehicle();
		// FIXME: displaying ownername to unit name
		// may cause an overdraw on the infobox.
		// This needs either to separate infobox or
		// a length check in the future.
		// that goes for unitnames itself too. -- beko
		unitNameLabel.setText (vehicle.getDisplayName() + " (" + vehicle.owner->getName() + ")");
		if(mouse.getCursorType() == eMouseCursorType::Attack)
		{
			drawAttackCursor (x, y);
		}
	}
	else if (overUnitField->getPlane() != NULL)
	{
		const cVehicle& plane = *overUnitField->getPlane();
		unitNameLabel.setText (plane.getDisplayName() + " (" + plane.owner->getName() + ")");
		if(mouse.getCursorType() == eMouseCursorType::Attack)
		{
			drawAttackCursor (x, y);
		}
	}
	else if (overUnitField->getTopBuilding() != NULL)
	{
		const cBuilding& topBuilding = *overUnitField->getTopBuilding();
		unitNameLabel.setText (topBuilding.getDisplayName() + " (" + topBuilding.owner->getName() + ")");
		if(mouse.getCursorType() == eMouseCursorType::Attack)
		{
			drawAttackCursor (x, y);
		}
	}
	else if (overUnitField->getBaseBuilding() && overUnitField->getBaseBuilding()->owner)
	{
		const cBuilding& baseBuilding = *overUnitField->getBaseBuilding();
		unitNameLabel.setText (baseBuilding.getDisplayName() + " (" + baseBuilding.owner->getName() + ")");
		if(mouse.getCursorType() == eMouseCursorType::Attack)
		{
			drawAttackCursor (x, y);
		}
	}
	else
	{
		unitNameLabel.setText ("");
		if(mouse.getCursorType() == eMouseCursorType::Attack)
		{
			SDL_Rect r = {1, 29, 35u, 3u};
			SDL_FillRect (GraphicsData.gfx_Cattack, &r, 0);
		}
		overUnitField = NULL;
	}
	// place band:
	if (selectedVehicle && mouseInputMode == placeBand)
	{
		selectedVehicle->FindNextband (*this);
	}
}

void cGameGUI::incFrame()
{
	frame++;
}

void cGameGUI::setStartup (bool startup_)
{
	startup = startup_;
}

cVehicle* cGameGUI::getSelectedVehicle()
{
	return static_cast<cVehicle*> (selectedUnit && selectedUnit->isAVehicle() ? selectedUnit : NULL);
}

cBuilding* cGameGUI::getSelectedBuilding()
{
	return static_cast<cBuilding*> (selectedUnit && selectedUnit->isABuilding() ? selectedUnit : NULL);
}

void cGameGUI::selectUnit (cUnit& unit)
{
	cVehicle* vehicle = static_cast<cVehicle*> (unit.isAVehicle() ? &unit : NULL);

	if (vehicle && vehicle->Loaded) return;

	cBuilding* building = static_cast<cBuilding*> (unit.isABuilding() ? &unit : NULL);

	deselectUnit();
	getClient()->getActivePlayer().savedHud->selUnitID = unit.iID;
	selectedUnit = &unit;
	unitMenuActive = false;
	mouseInputMode = normalInput;
	if (vehicle)
	{
		vehicle->Select (*this);
		playStream (*vehicle);
	}
	else
	{
		building->Select (*this);
		playStream (*building);
	}
	updateMouseCursor();
}

void cGameGUI::deselectUnit()
{
	if (selectedUnit)
	{
		cVehicle* selectedVehicle = getSelectedVehicle();
		if (selectedVehicle)
		{
			selectedVehicle->groupSelected = false;
			if (mouseInputMode == placeBand) selectedVehicle->BuildPath = false;
		}
		stopFXLoop();
		setVideoSurface (NULL);
		setUnitDetailsData (NULL);

		stopFXLoop();
	}
	getClient()->getActivePlayer().savedHud->selUnitID = 0;
	selectedUnit = NULL;

	mouseInputMode = normalInput;

	updateMouseCursor();
}

void cGameGUI::setInfoTexts (const string& infoText, const string& additionalInfoText)
{
	infoTextLabel.setText (infoText);
	infoTextLabel.setDisabled (infoText.empty());
	infoTextAdditionalLabel.setText (additionalInfoText);
	infoTextAdditionalLabel.setDisabled (additionalInfoText.empty());
}

void cGameGUI::updateMouseCursor()
{
	updateUnderMouseObject(cMouse::getInstance());

	auto& mouse = cMouse::getInstance();

	const auto mousePosition = mouse.getPosition();
	const int x = mousePosition.x();
	const int y = mousePosition.y();
	const auto mouseTilePosition = getTilePosition(cMouse::getInstance().getPosition());
	const int mouseMapX = mouseTilePosition.x();
	const int mouseMapY = mouseTilePosition.y();

	for (unsigned int i = 0; i < menuItems.size(); i++)
	{
		if (menuItems[i]->overItem (x, y) && !menuItems[i]->isDisabled())
		{
			mouse.setCursorType(eMouseCursorType::Hand);
			return;
		}
	}

	cVehicle* selectedVehicle = getSelectedVehicle();
	cBuilding* selectedBuilding = getSelectedBuilding();
	if (selectedVehicle && mouseInputMode == placeBand && selectedVehicle->owner == &client->getActivePlayer())
	{
		if (x >= HUD_LEFT_WIDTH)
		{
			mouse.setCursorType(eMouseCursorType::Band);
		}
		else
		{
			mouse.setCursorType(eMouseCursorType::No);
		}
	}
	else if ((selectedUnit && mouseInputMode == transferMode && selectedUnit->owner == &client->getActivePlayer()))
	{
		if (overUnitField && selectedUnit->CanTransferTo (mouseMapX, mouseMapY, overUnitField))
		{
			mouse.setCursorType(eMouseCursorType::Transfer);
		}
		else
		{
			mouse.setCursorType(eMouseCursorType::No);
		}
	}
	else if (!helpActive)
	{
		if (x < HUD_LEFT_WIDTH)
		{
			if(mouse.setCursorType(eMouseCursorType::Hand))
			{
				overUnitField = NULL;
			}
			return;
		}

		if(selectedUnit && unitMenuActive && areCoordsOverMenu(*selectedUnit, mousePosition))
		{
			mouse.setCursorType(eMouseCursorType::Hand);
		}
		else if (selectedVehicle && mouseInputMode == mouseInputAttackMode && selectedVehicle->owner == &client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT)
		{
			if (! (selectedVehicle->data.muzzleType == sUnitData::MUZZLE_TYPE_TORPEDO && !client->getMap()->isWaterOrCoast (mouseMapX, mouseMapY)))
			{
				if(mouse.setCursorType(eMouseCursorType::Attack))
				{
					drawAttackCursor (mouseMapX, mouseMapY);
				}
			}
			else
			{
				mouse.setCursorType(eMouseCursorType::No);
			}
		}
		// Infiltrators: manual action from unit-menu
		// disable vs. vehicle/building
		else if (selectedVehicle && mouseInputMode == disableMode && selectedVehicle->owner == &client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT)
		{
			if (selectedVehicle->canDoCommandoAction (mouseMapX, mouseMapY, *client->getMap(), false)
				&& (!overUnitField->getVehicle() || overUnitField->getVehicle()->isDisabled() == false)
				&& (!overUnitField->getBuilding() || overUnitField->getBuilding()->isDisabled() == false)
			   )
			{
				if(mouse.setCursorType(eMouseCursorType::Disable))
				{
					selectedVehicle->drawCommandoCursor (*this, mouseMapX, mouseMapY, false);
				}
			}
			else
			{
				mouse.setCursorType(eMouseCursorType::No);
			}
		}
		// steal vs. vehicle
		else if (selectedVehicle && mouseInputMode == stealMode && selectedVehicle->owner == &client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT)
		{
			if (selectedVehicle->canDoCommandoAction (mouseMapX, mouseMapY, *client->getMap(), true))
			{
				if(mouse.setCursorType(eMouseCursorType::Steal))
				{
					selectedVehicle->drawCommandoCursor (*this, mouseMapX, mouseMapY, true);
				}
			}
			else
			{
				mouse.setCursorType(eMouseCursorType::No);
			}
		}
		// Infiltrators: auto-action
		// no disable vs. disabled building
		else if (selectedVehicle && selectedVehicle->owner == &client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT && selectedVehicle->canDoCommandoAction (mouseMapX, mouseMapY, *client->getMap(), false) && overUnitField->getBuilding() && overUnitField->getBuilding()->isDisabled())
		{
			mouse.setCursorType(eMouseCursorType::No);
		}
		// vehicle can be disabled, and if it is ...
		else if (selectedVehicle && selectedVehicle->owner == &client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT && selectedVehicle->canDoCommandoAction (mouseMapX, mouseMapY, *client->getMap(), false) && (!overUnitField->getVehicle() || overUnitField->getVehicle()->isDisabled() == false))
		{
			if(mouse.setCursorType(eMouseCursorType::Disable))
			{
				selectedVehicle->drawCommandoCursor (*this, mouseMapX, mouseMapY, false);
			}
		}
		// ... disabled (the) vehicle can be stolen
		// (without selecting the 'steal' from menu)
		else if (selectedVehicle && selectedVehicle->owner == &client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT && selectedVehicle->canDoCommandoAction (mouseMapX, mouseMapY, *client->getMap(), true))
		{
			if(mouse.setCursorType(eMouseCursorType::Steal))
			{
				selectedVehicle->drawCommandoCursor (*this, mouseMapX, mouseMapY, true);
			}
		}
		else if (selectedBuilding && mouseInputMode == mouseInputAttackMode && selectedBuilding->owner == &client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT)
		{
			if (selectedBuilding->isInRange (mouseMapX, mouseMapY))
			{
				if(mouse.setCursorType(eMouseCursorType::Attack))
				{
					drawAttackCursor (mouseMapX, mouseMapY);
				}
			}
			else
			{
				mouse.setCursorType(eMouseCursorType::No);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == &client->getActivePlayer() && selectedVehicle->canAttackObjectAt (mouseMapX, mouseMapY, *client->getMap(), false, false))
		{
			if(mouse.setCursorType(eMouseCursorType::Attack))
			{
				drawAttackCursor (mouseMapX, mouseMapY);
			}
		}
		else if (selectedBuilding && selectedBuilding->owner == &client->getActivePlayer() && selectedBuilding->canAttackObjectAt (mouseMapX, mouseMapY, *client->getMap()))
		{
			if(mouse.setCursorType(eMouseCursorType::Attack))
			{
				drawAttackCursor (mouseMapX, mouseMapY);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == &client->getActivePlayer() && mouseInputMode == muniActive)
		{
			if (selectedVehicle->canSupply (*client, mouseMapX, mouseMapY, SUPPLY_TYPE_REARM))
			{
				mouse.setCursorType(eMouseCursorType::Muni);
			}
			else
			{
				mouse.setCursorType(eMouseCursorType::No);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == &client->getActivePlayer() && mouseInputMode == repairActive)
		{
			if (selectedVehicle->canSupply (*client, mouseMapX, mouseMapY, SUPPLY_TYPE_REPAIR))
			{
				mouse.setCursorType(eMouseCursorType::Repair);
			}
			else
			{
				mouse.setCursorType(eMouseCursorType::No);
			}
		}
		else if (overUnitField &&
				 (
					 overUnitField->getVehicle() ||
					 overUnitField->getPlane() ||
					 (
						 overUnitField->getBuilding() &&
						 overUnitField->getBuilding()->owner
					 )
				 ) &&
				 (
					 !selectedVehicle ||
					 selectedVehicle->owner != &client->getActivePlayer() ||
					 (
						 (
							 selectedVehicle->data.factorAir > 0 ||
							 overUnitField->getVehicle() ||
							 (
								 overUnitField->getTopBuilding() &&
								 overUnitField->getTopBuilding()->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE
							 ) ||
							 (
								 MouseStyle == OldSchool &&
								 overUnitField->getPlane()
							 )
						 ) &&
						 (
							 selectedVehicle->data.factorAir == 0 ||
							 overUnitField->getPlane() ||
							 (
								 MouseStyle == OldSchool &&
								 (
									 overUnitField->getVehicle() ||
									 (
										 overUnitField->getTopBuilding() &&
										 overUnitField->getTopBuilding()->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE &&
										 !overUnitField->getTopBuilding()->data.canBeLandedOn
									 )
								 )
							 )
						 ) &&
						 mouseInputMode != loadMode &&
						 mouseInputMode != activateVehicle
					 )
				 ) &&
				 (
					 !selectedBuilding ||
					 selectedBuilding->owner != &client->getActivePlayer() ||
					 (
						 (
							 selectedBuilding->BuildList.empty() ||
							 selectedBuilding->IsWorking ||
							 selectedBuilding->BuildList[0].metall_remaining > 0
						 ) &&
						 mouseInputMode != loadMode &&
						 mouseInputMode != activateVehicle
					 )
				 )
				)
		{
			mouse.setCursorType(eMouseCursorType::Select);
		}
		else if (selectedVehicle && selectedVehicle->owner == &client->getActivePlayer() && mouseInputMode == loadMode)
		{
			if (selectedVehicle->canLoad (mouseMapX, mouseMapY, *client->getMap(), false))
			{
				mouse.setCursorType(eMouseCursorType::Load);
			}
			else
			{
				mouse.setCursorType(eMouseCursorType::No);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == &client->getActivePlayer() && mouseInputMode == activateVehicle)
		{
			if (selectedVehicle->canExitTo (mouseMapX, mouseMapY, *client->getMap(), selectedVehicle->storedUnits[vehicleToActivate]->data) && selectedUnit->isDisabled() == false)
			{
				mouse.setCursorType(eMouseCursorType::Activate);
			}
			else
			{
				mouse.setCursorType(eMouseCursorType::No);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == &client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < HUD_LEFT_WIDTH + (Video.getResolutionX() - HUD_TOTAL_WIDTH) && y < HUD_TOP_HIGHT + (Video.getResolutionY() - HUD_TOTAL_HIGHT))
		{
			if (!selectedVehicle->IsBuilding && !selectedVehicle->IsClearing && mouseInputMode != loadMode && mouseInputMode != activateVehicle)
			{
				if (selectedVehicle->MoveJobActive)
				{
					mouse.setCursorType(eMouseCursorType::No);
				}
				else if (client->getMap()->possiblePlace (*selectedVehicle, mouseMapX, mouseMapY, true))
				{
					mouse.setCursorType(eMouseCursorType::Move);
				}
				else
				{
					mouse.setCursorType(eMouseCursorType::No);
				}
			}
			else if (selectedVehicle->IsBuilding || selectedVehicle->IsClearing)
			{
				if (((selectedVehicle->IsBuilding && selectedVehicle->BuildRounds == 0) ||
					 (selectedVehicle->IsClearing && selectedVehicle->ClearingRounds == 0)) &&
					client->getMap()->possiblePlace (*selectedVehicle, mouseMapX, mouseMapY) && selectedVehicle->isNextTo (mouseMapX, mouseMapY))
				{
					mouse.setCursorType(eMouseCursorType::Move);
				}
				else
				{
					mouse.setCursorType(eMouseCursorType::No);
				}
			}
		}
		else if (
			selectedBuilding &&
			selectedBuilding->owner == &client->getActivePlayer() &&
			!selectedBuilding->BuildList.empty() &&
			!selectedBuilding->IsWorking &&
			selectedBuilding->BuildList[0].metall_remaining <= 0)
		{
			if (selectedBuilding->canExitTo (mouseMapX, mouseMapY, *client->getMap(), *selectedBuilding->BuildList[0].type.getUnitDataOriginalVersion()) && selectedUnit->isDisabled() == false)
			{
				mouse.setCursorType(eMouseCursorType::Activate);
			}
			else
			{
				mouse.setCursorType(eMouseCursorType::No);
			}
		}
		else if (selectedBuilding && selectedBuilding->owner == &client->getActivePlayer() && mouseInputMode == activateVehicle && selectedUnit->isDisabled() == false)
		{
			if (selectedBuilding->canExitTo (mouseMapX, mouseMapY, *client->getMap(), selectedBuilding->storedUnits[vehicleToActivate]->data))
			{
				mouse.setCursorType(eMouseCursorType::Activate);
			}
			else
			{
				mouse.setCursorType(eMouseCursorType::No);
			}
		}
		else if (selectedBuilding && selectedBuilding->owner == &client->getActivePlayer() && mouseInputMode == loadMode)
		{
			if (selectedBuilding->canLoad (mouseMapX, mouseMapY, *client->getMap(), false))
			{
				mouse.setCursorType(eMouseCursorType::Load);
			}
			else
			{
				mouse.setCursorType(eMouseCursorType::No);
			}
		}
		else
		{
			mouse.setCursorType(eMouseCursorType::Hand);
		}
	}
	else
	{
		mouse.setCursorType(eMouseCursorType::Help);
	}
}

void cGameGUI::doScroll (int dir)
{
	const int step = cSettings::getInstance().getScrollSpeed();
	//                  SW, S, SE, E, X, W, NW, N, NE
	int offsetsX[9] = { -1, 0, 1, -1, 0, 1, -1,  0,  1};
	int offsetsY[9] = {  1, 1, 1,  0, 0, 0, -1, -1, -1};
	--dir;
	if (dir < 0 || dir >= 9) return;
	setOffsetPosition (offX + offsetsX[dir] * step, offY + offsetsY[dir] * step);
}

void cGameGUI::doCommand (const string& cmd)
{
	cServer* server = client->getServer();
	cPlayer& player = client->getActivePlayer();

	if (cmd.compare ("/fps on") == 0) { debugOutput.showFPS = true;}
	else if (cmd.compare ("/fps off") == 0) { debugOutput.showFPS = false;}
	else if (cmd.compare ("/base client") == 0) { debugOutput.debugBaseClient = true; debugOutput.debugBaseServer = false;}
	else if (cmd.compare ("/base server") == 0) { if (server) debugOutput.debugBaseServer = true; debugOutput.debugBaseClient = false;}
	else if (cmd.compare ("/base off") == 0) { debugOutput.debugBaseServer = false; debugOutput.debugBaseClient = false;}
	else if (cmd.compare ("/sentry server") == 0) { if (server) debugOutput.debugSentry = true;}
	else if (cmd.compare ("/sentry off") == 0) { debugOutput.debugSentry = false;}
	else if (cmd.compare ("/fx on") == 0) { debugOutput.debugFX = true;}
	else if (cmd.compare ("/fx off") == 0) { debugOutput.debugFX = false;}
	else if (cmd.compare ("/trace server") == 0) { if (server) debugOutput.debugTraceServer = true; debugOutput.debugTraceClient = false;}
	else if (cmd.compare ("/trace client") == 0) { debugOutput.debugTraceClient = true; debugOutput.debugTraceServer = false;}
	else if (cmd.compare ("/trace off") == 0) { debugOutput.debugTraceServer = false; debugOutput.debugTraceClient = false;}
	else if (cmd.compare ("/ajobs on") == 0) { debugOutput.debugAjobs = true;}
	else if (cmd.compare ("/ajobs off") == 0) { debugOutput.debugAjobs = false;}
	else if (cmd.compare ("/players on") == 0) { debugOutput.debugPlayers = true;}
	else if (cmd.compare ("/players off") == 0) { debugOutput.debugPlayers = false;}
	else if (cmd.compare ("/singlestep") == 0) { cGameTimer::syncDebugSingleStep = !cGameTimer::syncDebugSingleStep;}
	else if (cmd.compare (0, 12, "/cache size ") == 0)
	{
		int size = atoi (cmd.substr (12, cmd.length()).c_str());
		// since atoi is too stupid to report an error,
		// do an extra check, when the number is 0
		if (size == 0 && cmd[12] != '0')
		{
			addMessage ("Wrong parameter");
			return;
		}
		getDCache()->setMaxCacheSize (size);
	}
	else if (cmd.compare ("/cache flush") == 0)
	{
		getDCache()->flush();
	}
	else if (cmd.compare ("/cache debug on") == 0)
	{
		debugOutput.debugCache = true;
	}
	else if (cmd.compare ("/cache debug off") == 0)
	{
		debugOutput.debugCache = false;
	}
	else if (cmd.compare (0, 6, "/kick ") == 0)
	{
		if (!server)
		{
			addMessage ("Command can only be used by Host");
			return;
		}
		if (cmd.length() <= 6)
		{
			addMessage ("Wrong parameter");
			return;
		}
		cPlayer* Player = server->getPlayerFromString (cmd.substr (6, cmd.length()));

		// server can not be kicked
		if (!Player || Player->getNr() == 0)
		{
			addMessage ("Wrong parameter");
			return;
		}

		server->kickPlayer (Player);
	}
	else if (cmd.compare (0, 9, "/credits ") == 0)
	{
		if (!server)
		{
			addMessage ("Command can only be used by Host");
			return;
		}
		if (cmd.length() <= 9)
		{
			addMessage ("Wrong parameter");
			return;
		}
		const string playerStr = cmd.substr (9, cmd.find_first_of (" ", 9) - 9);
		const string creditsStr = cmd.substr (cmd.find_first_of (" ", 9) + 1, cmd.length());

		cPlayer* Player = server->getPlayerFromString (playerStr);

		if (!Player)
		{
			addMessage ("Wrong parameter");
			return;
		}
		const int credits = atoi (creditsStr.c_str());

		Player->Credits = credits;

		sendCredits (*server, credits, Player->getNr());
	}
	else if (cmd.compare (0, 12, "/disconnect ") == 0)
	{
		if (!server)
		{
			addMessage ("Command can only be used by Host");
			return;
		}
		if (cmd.length() <= 12)
		{
			addMessage ("Wrong parameter");
			return;
		}
		cPlayer* Player = server->getPlayerFromString (cmd.substr (12, cmd.length()));

		// server cannot be disconnected
		// can not disconnect local players
		if (!Player || Player->getNr() == 0 || Player->isLocal())
		{
			addMessage ("Wrong parameter");
			return;
		}

		cNetMessage* message = new cNetMessage (TCP_CLOSE);
		message->pushInt16 (Player->getSocketNum());
		server->pushEvent (message);
	}
	else if (cmd.compare (0, 9, "/deadline") == 0)
	{
		if (!server)
		{
			addMessage ("Command can only be used by Host");
			return;
		}
		if (cmd.length() <= 9)
		{
			addMessage ("Wrong parameter");
			return;
		}

		const int i = atoi (cmd.substr (9, cmd.length()).c_str());
		if (i == 0 && cmd[10] != '0')
		{
			addMessage ("Wrong parameter");
			return;
		}

		server->setDeadline (i);
		Log.write ("Deadline changed to " + iToStr (i), cLog::eLOG_TYPE_INFO);
	}
	else if (cmd.compare (0, 7, "/resync") == 0)
	{
		if (cmd.length() > 7)
		{
			if (!server)
			{
				addMessage ("Command can only be used by Host");
				return;
			}
			cPlayer* player = client->getPlayerFromString (cmd.substr (7, 8));
			if (!player)
			{
				addMessage ("Wrong parameter");
				return;
			}
			sendRequestResync (*client, player->getNr());
		}
		else
		{
			if (server)
			{
				const std::vector<cPlayer*>& playerList = server->PlayerList;
				for (unsigned int i = 0; i < playerList.size(); i++)
				{
					sendRequestResync (*client, playerList[i]->getNr());
				}
			}
			else
			{
				sendRequestResync (*client, client->getActivePlayer().getNr());
			}
		}
	}
	else if (cmd.compare (0, 5, "/mark") == 0)
	{
		std::string cmdArg (cmd);
		cmdArg.erase (0, 5);
		cNetMessage* message = new cNetMessage (GAME_EV_WANT_MARK_LOG);
		message->pushString (cmdArg);
		client->sendNetMessage (message);
	}
	else if (cmd.compare (0, 7, "/color ") == 0)
	{
		int cl = 0;
		sscanf (cmd.c_str(), "color %d", &cl);
		cl %= 8;
		player.setColor (cl);
	}
	else if (cmd.compare ("/fog off") == 0)
	{
		if (!server)
		{
			addMessage ("Command can only be used by Host");
			return;
		}
		server->getPlayerFromNumber (player.getNr())->revealMap();
		player.revealMap();
	}
	else if (cmd.compare ("/survey") == 0)
	{
		if (!server)
		{
			addMessage ("Command can only be used by Host");
			return;
		}
		client->getMap()->assignRessources (*server->Map);
		player.revealResource();
	}
	else if (cmd.compare (0, 6, "/pause") == 0)
	{
		if (!server)
		{
			addMessage ("Command can only be used by Host");
			return;
		}
		server->enableFreezeMode (FREEZE_PAUSE);
	}
	else if (cmd.compare (0, 7, "/resume") == 0)
	{
		if (!server)
		{
			addMessage ("Command can only be used by Host");
			return;
		}
		server->disableFreezeMode (FREEZE_PAUSE);
	}
	else
	{
		addMessage ("Unknown command");
	}
}

void cGameGUI::setWind (int dir)
{
	// 360 / (2 * PI) = 57.29577f;
	windDir = dir / 57.29577f;
}

void cGameGUI::selectUnit_vehicle (cVehicle& vehicle)
{
	// TO FIX: add that the unit renaming will be aborted here when active
	if (selectedUnit == &vehicle)
	{
		if (selectedUnit->owner == &client->getActivePlayer())
		{
			unitMenuActive = !unitMenuActive;
			PlayFX (SoundData.SNDHudButton);
		}
	}
	else
	{
		selectUnit (vehicle);
	}
}

void cGameGUI::selectUnit_building (cBuilding& building)
{
	// TO FIX: add that the unit renaming will be aborted here when active
	if (selectedUnit == &building)
	{
		if (selectedUnit->owner == &client->getActivePlayer())
		{
			unitMenuActive = !unitMenuActive;
			PlayFX (SoundData.SNDHudButton);
		}
	}
	else
	{
		selectUnit (building);
	}
}

bool cGameGUI::selectUnit (cMapField* OverUnitField, bool base)
{
	deselectGroup();
	cVehicle* plane = OverUnitField->getPlane();
	if (plane && !plane->moving)
	{
		selectUnit_vehicle (*plane);
		return true;
	}
	cVehicle* vehicle = OverUnitField->getVehicle();
	if (vehicle && !vehicle->moving && ! (plane && (unitMenuActive || vehicle->owner != &client->getActivePlayer())))
	{
		selectUnit_vehicle (*vehicle);
		return true;
	}
	cBuilding* topBuilding = OverUnitField->getTopBuilding();
	const cVehicle* selectedVehicle = getSelectedVehicle();
	if (topBuilding && (base || ((topBuilding->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE || !selectedVehicle) && (!OverUnitField->getTopBuilding()->data.canBeLandedOn || (!selectedVehicle || selectedVehicle->data.factorAir == 0)))))
	{
		selectUnit_building (*topBuilding);
		return true;
	}
	cBuilding* baseBuilding = OverUnitField->getBaseBuilding();
	if ((base || !selectedVehicle) && baseBuilding && baseBuilding->owner != NULL)
	{
		selectUnit_building (*baseBuilding);
		return true;
	}
	return false;
}

void cGameGUI::selectBoxVehicles(const cBox<cPosition>& box)
{
	deselectGroup();
	const cPlayer& player = client->getActivePlayer();
	cMap& map = *client->getMap();
	bool newSelected = true;
	for(int x = box.getMinCorner().x(); x <= box.getMaxCorner().x(); ++x)
	{
		for(int y = box.getMinCorner().y(); y <= box.getMaxCorner().y(); ++y)
		{
			const int offset = map.getOffset (x, y);

			cVehicle* vehicle = map[offset].getVehicle();
			if (!vehicle || vehicle->owner != &player) vehicle = map[offset].getPlane();

			if (vehicle && vehicle->owner == &player && !vehicle->IsBuilding && !vehicle->IsClearing && !vehicle->moving)
			{
				if (vehicle == selectedUnit)
				{
					newSelected = false;
					selectedVehiclesGroup.insert (selectedVehiclesGroup.begin(), vehicle);
				}
				else selectedVehiclesGroup.push_back (vehicle);
				vehicle->groupSelected = true;
			}
		}
	}
	if (newSelected && selectedVehiclesGroup.empty() == false)
	{
		selectUnit (*selectedVehiclesGroup[0]);
	}
	if (selectedVehiclesGroup.size() == 1)
	{
		selectedVehiclesGroup[0]->groupSelected = false;
		selectedVehiclesGroup.erase (selectedVehiclesGroup.begin());
	}
}

void cGameGUI::updateStatusText()
{
	if (selectedUnit) selUnitStatusStr.setText (selectedUnit->getStatusStr (*this));
	else selUnitStatusStr.setText ("");
}

void cGameGUI::deselectGroup()
{
	for (size_t i = 0; i != selectedVehiclesGroup.size(); ++i)
	{
		selectedVehiclesGroup[i]->groupSelected = false;
	}
	selectedVehiclesGroup.clear();
}

void cGameGUI::changeWindDir()
{
	if (timer400ms && cSettings::getInstance().isDamageEffects())
	{
		static int nextChange = 25;
		static int nextDirChange = 25;
		static int dir = 90;
		static int change = 3;
		if (nextChange == 0)
		{
			nextChange = 10 + random (20);
			dir += change;
			setWind (dir);
			if (dir >= 360) dir -= 360;
			else if (dir < 0) dir += 360;

			if (nextDirChange == 0)
			{
				nextDirChange = random (25) + 10;
				change        = random (11) - 5;
			}
			else nextDirChange--;
		}
		else nextChange--;
	}
}

bool cGameGUI::loadPanelGraphics()
{
	if (!panelTopGraphic) panelTopGraphic = LoadPCX (cSettings::getInstance().getGfxPath() + PATH_DELIMITER + "panel_top.pcx");
	if (!panelBottomGraphic) panelBottomGraphic = LoadPCX (cSettings::getInstance().getGfxPath() + PATH_DELIMITER + "panel_top.pcx");

	if (!panelTopGraphic || !panelBottomGraphic) return false;
	return true;
}

void cGameGUI::helpReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	gui->helpActive = !gui->helpActive;
	gui->updateMouseCursor();
}

void cGameGUI::centerReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	if (gui->selectedUnit) gui->center (*gui->selectedUnit);
}

void cGameGUI::reportsReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	cReportsMenu reportMenu (*gui->getClient());
	gui->switchTo(reportMenu, gui->getClient());
}

void cGameGUI::chatReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	gui->chatBox.setDisabled (!gui->chatBox.isDisabled());
	if (gui->activeItem) gui->activeItem->setActivity (false);
	gui->activeItem = &gui->chatBox;
	gui->chatBox.setActivity (true);
}

void cGameGUI::nextReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	cUnit* unit = gui->client->getActivePlayer().getNextUnit (gui->getSelectedUnit());
	if (unit)
	{
		gui->selectUnit (*unit);
		gui->center (*unit);
	}
}

void cGameGUI::prevReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	cUnit* unit = gui->client->getActivePlayer().getPrevUnit (gui->getSelectedUnit());
	if (unit)
	{
		gui->selectUnit (*unit);
		gui->center (*unit);
	}
}

void cGameGUI::doneReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);

	if(cKeyboard::getInstance().isAnyModifierActive(toEnumFlag(eKeyModifierType::ShiftLeft) | toEnumFlag(eKeyModifierType::ShiftRight)))
	{
		sendMoveJobResume (*gui->client, 0);
		return;
	}

	cUnit* unit = gui->selectedUnit;

	if (unit && unit->owner == &gui->client->getActivePlayer())
	{
		gui->center (*unit);
		unit->isMarkedAsDone = true;
		sendMoveJobResume (*gui->client, unit->iID);
	}
}

void cGameGUI::twoXReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	gui->resetMiniMapOffset();
	gui->callMiniMapDraw();
}

void cGameGUI::playersReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	gui->showPlayers = gui->playersButton.isChecked();

	gui->playersInfo.setDisabled (!gui->showPlayers);
}

void cGameGUI::changedMiniMap (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	gui->callMiniMapDraw();
}

void cGameGUI::resetMiniMapOffset()
{
	const int zoomFactor = twoXChecked() ? MINIMAP_ZOOM_FACTOR : 1;

	if (zoomFactor == 1)
	{
		miniMapOffX = 0;
		miniMapOffY = 0;
	}
	else
	{
		int centerPosX = (int) (offX / 64.0f + (Video.getResolutionX() - 192.0f) / (getTileSize() * 2));
		int centerPosY = (int) (offY / 64.0f + (Video.getResolutionY() -  32.0f) / (getTileSize() * 2));
		const int mapSize = client->getMap()->getSize();
		miniMapOffX = centerPosX - (mapSize / (zoomFactor * 2));
		miniMapOffY = centerPosY - (mapSize / (zoomFactor * 2));

		miniMapOffX = std::max (miniMapOffX, 0);
		miniMapOffY = std::max (miniMapOffY, 0);
		miniMapOffX = std::min (mapSize - (mapSize / zoomFactor), miniMapOffX);
		miniMapOffY = std::min (mapSize - (mapSize / zoomFactor), miniMapOffY);
	}
}

void cGameGUI::miniMapClicked (void* parent)
{
	const auto& mousePosition = cMouse::getInstance().getPosition();

	static int lastX = 0;
	static int lastY = 0;
	const int x = mousePosition.x();
	const int y = mousePosition.y();
	if (lastX == x && lastY == y) return;

	cGameGUI* gui = static_cast<cGameGUI*> (parent);

	const int displayedMapWidth = (int) ((Video.getResolutionX() - HUD_TOTAL_WIDTH) / gui->getZoom());
	const int displayedMapHight = (int) ((Video.getResolutionY() - HUD_TOTAL_HIGHT) / gui->getZoom());
	const int zoomFactor = gui->twoXChecked() ? MINIMAP_ZOOM_FACTOR : 1;
	const int mapSize = gui->client->getMap()->getSize();

	gui->offX = gui->miniMapOffX * 64 + ((x - MINIMAP_POS_X) * mapSize * 64) / (MINIMAP_SIZE * zoomFactor);
	gui->offY = gui->miniMapOffY * 64 + ((y - MINIMAP_POS_Y) * mapSize * 64) / (MINIMAP_SIZE * zoomFactor);
	gui->offX -= displayedMapWidth / 2;
	gui->offY -= displayedMapHight / 2;

	lastX = x;
	lastY = y;

	gui->checkOffsetPosition();
}

void cGameGUI::miniMapRightClicked (void* parent)
{
	const auto& mousePosition = cMouse::getInstance().getPosition();

	cGameGUI* gui = static_cast<cGameGUI*> (parent);

	cVehicle* selectedVehicle = gui->getSelectedVehicle();
	if (!selectedVehicle || selectedVehicle->owner != &gui->client->getActivePlayer()) return;

	const int x = mousePosition.x();
	const int y = mousePosition.y();
	const int zoomFactor = gui->twoXChecked() ? MINIMAP_ZOOM_FACTOR : 1;
	const int mapSize = gui->client->getMap()->getSize();
	const int destX = gui->miniMapOffX + ((x - MINIMAP_POS_X) * mapSize) / (MINIMAP_SIZE * zoomFactor);
	const int destY = gui->miniMapOffY + ((y - MINIMAP_POS_Y) * mapSize) / (MINIMAP_SIZE * zoomFactor);

	gui->client->addMoveJob (*selectedVehicle, destX, destY, &gui->selectedVehiclesGroup);
}

void cGameGUI::miniMapMovedOver (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	if (gui->miniMapImage.getIsClicked())
	{
		gui->miniMapClicked (parent);
	}
}

void cGameGUI::zoomSliderMoved (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	gui->setZoom (gui->zoomSlider.getValue(), false, false);
}

void cGameGUI::endReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	gui->endButton.setLocked (true);
	gui->client->handleEnd();
}

void cGameGUI::preferencesReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	cDialogPreferences preferencesDialog(&gui->getClient()->getActivePlayer());
	gui->switchTo(preferencesDialog, gui->getClient());
}

void cGameGUI::filesReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	cLoadSaveMenu loadSaveMenu (*gui->getClient(), gui->getClient()->getServer());
	if(gui->switchTo(loadSaveMenu, gui->getClient()) != 1) gui->end = true;
}

void cGameGUI::playReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	gui->playFLC = true;
}

void cGameGUI::stopReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	gui->playFLC = false;
}

void cGameGUI::swithAnimationReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	gui->playFLC = !gui->playFLC;
}

void cGameGUI::chatBoxReturnPressed (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	string chatString = gui->chatBox.getText();
	if (!chatString.empty())
	{
		if (chatString[0] == '/') gui->doCommand (chatString);
		else sendChatMessageToServer (*gui->client, gui->client->getActivePlayer().getName() + ": " + chatString);
		gui->chatBox.setText ("");
	}
	gui->chatBox.setActivity (false);
	gui->activeItem = NULL;
	gui->chatBox.setDisabled (true);
}

void cGameGUI::unitNameReturnPressed (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	string nameString = gui->selUnitNameEdit.getText();

	if (gui->selectedUnit) sendWantChangeUnitName (*gui->client, nameString, gui->selectedUnit->iID);

	gui->selUnitNameEdit.setActivity (false);
	gui->activeItem = NULL;
}

void cGameGUI::preDrawFunction()
{
	// draw the map screen with everything on it
	const int zoomOffX = (int) (offX * getZoom());
	const int zoomOffY = (int) (offY * getZoom());
	const int startX = std::max (0, (offX - 1) / 64 - 1);
	const int startY = std::max (0, (offY - 1) / 64 - 1);
	cStaticMap& staticMap = *client->getMap()->staticMap;
	const int mapSize = client->getMap()->getSize();
	int endX = Round (offX / 64.0f + (float) (Video.getResolutionX() - HUD_TOTAL_WIDTH) / getTileSize());
	endX = std::min (endX, mapSize - 1);
	int endY = Round (offY / 64.0f + (float) (Video.getResolutionY() - HUD_TOTAL_HIGHT) / getTileSize());
	endY = std::min (endY, mapSize - 1);

	if (timer400ms) staticMap.generateNextAnimationFrame();

	SDL_Rect clipRect = { HUD_LEFT_WIDTH, HUD_TOP_HIGHT, Uint16 (Video.getResolutionX() - HUD_TOTAL_WIDTH), Uint16 (Video.getResolutionY() - HUD_TOTAL_HIGHT) };
	SDL_SetClipRect (cVideo::buffer, &clipRect);

	drawTerrain (zoomOffX, zoomOffY);
	if (gridChecked()) drawGrid (zoomOffX, zoomOffY);

	drawFx (true);

	dCache.resetStatistics();

	drawBaseUnits (startX, startY, endX, endY, zoomOffX, zoomOffY);
	drawTopBuildings (startX, startY, endX, endY, zoomOffX, zoomOffY);
	drawShips (startX, startY, endX, endY, zoomOffX, zoomOffY);
	drawAboveSeaBaseUnits (startX, startY, endX, endY, zoomOffX, zoomOffY);
	drawVehicles (startX, startY, endX, endY, zoomOffX, zoomOffY);
	drawConnectors (startX, startY, endX, endY, zoomOffX, zoomOffY);
	drawPlanes (startX, startY, endX, endY, zoomOffX, zoomOffY);

	cVehicle* selectedVehicle = getSelectedVehicle();
	if (surveyChecked() || (selectedVehicle && selectedVehicle->owner == &client->getActivePlayer() && selectedVehicle->data.canSurvey))
	{
		drawResources (startX, startY, endX, endY, zoomOffX, zoomOffY);
	}

	if (selectedVehicle && ((selectedVehicle->ClientMoveJob && selectedVehicle->ClientMoveJob->bSuspended) || selectedVehicle->BuildPath))
	{
		selectedVehicle->DrawPath (*this);
	}

	debugOutput.draw();

	drawSelectionBox (zoomOffX, zoomOffY);

	SDL_SetClipRect (cVideo::buffer, NULL);

	drawUnitCircles();

	if (selectedUnit && unitMenuActive) drawMenu (*selectedUnit);

	drawFx (false);

	displayMessages();
}

void cGameGUI::drawTerrain (int zoomOffX, int zoomOffY)
{
	const cPlayer& player = client->getActivePlayer();
	const int tileSize = getTileSize();
	SDL_Rect dest;
	dest.y = HUD_TOP_HIGHT - zoomOffY;
	// draw the terrain
	const cStaticMap& staticMap = *client->getMap()->staticMap;
	const int mapSize = client->getMap()->getSize();

	for (int y = 0; y < mapSize; ++y)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX;
		if (dest.y >= HUD_TOP_HIGHT - tileSize)
		{
			int pos = y * mapSize;
			for (int x = 0; x < mapSize; ++x)
			{
				if (dest.x >= HUD_LEFT_WIDTH - tileSize)
				{
					SDL_Rect tmp = dest;
					const sTerrain& terr = staticMap.getTerrain (pos);

					// draw the fog:
					if (fogChecked() && !player.ScanMap[pos])
					{
						if (!cSettings::getInstance().shouldDoPrescale() && (terr.shw->w != tileSize || terr.shw->h != tileSize)) scaleSurface (terr.shw_org, terr.shw, tileSize, tileSize);
						SDL_BlitSurface (terr.shw, NULL, cVideo::buffer, &tmp);
					}
					else
					{
						if (!cSettings::getInstance().shouldDoPrescale() && (terr.sf->w != tileSize || terr.sf->h != tileSize)) scaleSurface (terr.sf_org, terr.sf, tileSize, tileSize);
						SDL_BlitSurface (terr.sf, NULL, cVideo::buffer, &tmp);
					}
				}
				pos++;
				dest.x += tileSize;
				if (dest.x > Video.getResolutionX() - 13) break;
			}
		}
		dest.y += tileSize;
		if (dest.y > Video.getResolutionY() - 15) break;
	}
}

void cGameGUI::drawGrid (int zoomOffX, int zoomOffY)
{
	const int tileSize = getTileSize();
	SDL_Rect dest;
	dest.x = HUD_LEFT_WIDTH;
	dest.y = HUD_TOP_HIGHT + tileSize - (zoomOffY % tileSize);
	dest.w = Video.getResolutionX() - HUD_TOTAL_WIDTH;
	dest.h = 1;
	for (int y = 0; y < (Video.getResolutionY() - HUD_TOTAL_HIGHT) / tileSize + 1; y++)
	{
		SDL_FillRect (cVideo::buffer, &dest, GRID_COLOR);
		dest.y += tileSize;
	}
	dest.x = HUD_LEFT_WIDTH + tileSize - (zoomOffX % tileSize);
	dest.y = HUD_TOP_HIGHT;
	dest.w = 1;
	dest.h = Video.getResolutionY() - HUD_TOTAL_HIGHT;
	for (int x = 0; x < (Video.getResolutionX() - HUD_TOTAL_WIDTH) / tileSize + 1; x++)
	{
		SDL_FillRect (cVideo::buffer, &dest, GRID_COLOR);
		dest.x += tileSize;
	}
}

void cGameGUI::addFx (cFx* fx)
{
	FxList->push_front (fx);
	fx->playSound (*this);
}

void cGameGUI::drawFx (bool bottom) const
{
	SDL_Rect clipRect = { HUD_LEFT_WIDTH, HUD_TOP_HIGHT, Uint16 (Video.getResolutionX() - HUD_TOTAL_WIDTH), Uint16 (Video.getResolutionY() - HUD_TOTAL_HIGHT) };
	SDL_SetClipRect (cVideo::buffer, &clipRect);
	SDL_SetClipRect (cVideo::buffer, &clipRect);

	client->FxList->draw (*this, bottom);
	FxList->draw (*this, bottom);

	SDL_SetClipRect (cVideo::buffer, NULL);
}

void cGameGUI::runFx()
{
	FxList->run();
}

SDL_Rect cGameGUI::calcScreenPos (int x, int y) const
{
	SDL_Rect pos;
	pos.x = HUD_LEFT_WIDTH - ((int) ((offX - x) * getZoom()));
	pos.y = HUD_TOP_HIGHT  - ((int) ((offY - y) * getZoom()));

	return pos;
}

void cGameGUI::drawAttackCursor (int x, int y) const
{
	assert(cMouse::getInstance().getCursorType() == eMouseCursorType::Attack);

	if (selectedUnit == NULL) return;

	const sUnitData& data = selectedUnit->data;
	cUnit* target = selectTarget (x, y, data.canAttack, *client->getMap());

	if (!target || (target == selectedUnit))
	{
		SDL_Rect r = {1, 29, 35, 3};
		SDL_FillRect (GraphicsData.gfx_Cattack, &r, 0);
		return;
	}

	int t = target->data.hitpointsCur;
	int wc = (int) ((float) t / target->data.hitpointsMax * 35);

	t = target->calcHealth (data.damage);

	int wp = 0;
	if (t)
	{
		wp = (int) ((float) t / target->data.hitpointsMax * 35);
	}
	SDL_Rect r = {1, 29, Uint16 (wp), 3};

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

void cGameGUI::drawBaseUnits (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	const int tileSize = getTileSize();
	SDL_Rect dest;
	// draw rubble and all base buildings (without bridges)
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	const cPlayer& player = client->getActivePlayer();
	const cMap& map = *client->getMap();

	for (int y = startY; y <= endY; y++)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map.getOffset (startX, y);
		for (int x = startX; x <= endX; x++)
		{
			std::vector<cBuilding*>& buildings = map.fields[pos].getBuildings();
			for (std::vector<cBuilding*>::reverse_iterator it = buildings.rbegin(); it != buildings.rend(); ++it)
			{
				if ((*it)->data.surfacePosition != sUnitData::SURFACE_POS_BENEATH_SEA &&
					(*it)->data.surfacePosition != sUnitData::SURFACE_POS_BASE &&
					(*it)->owner) break;

				if (player.canSeeAnyAreaUnder (**it))
				{
					// Draw big unit only once
					// TODO: bug when (x,y) is outside of the drawing screen.
					if ((*it)->PosX == x && (*it)->PosY == y)
					{
						(*it)->draw (&dest, *this);
					}
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawTopBuildings_DebugBaseClient (const cBuilding& building, const SDL_Rect& dest)
{
	assert (debugOutput.debugBaseClient && building.SubBase);

	SDL_Rect tmp = { dest.x, dest.y, Uint16 (getTileSize()), 8u };
	if (building.data.isBig) tmp.w *= 2;
	const sSubBase* sb = building.SubBase;
	// the VS compiler gives a warning on casting a pointer to long.
	// therefore we will first cast to long long
	// and then cut this to Unit32 again.
	SDL_FillRect (cVideo::buffer, &tmp, (Uint32) (long long) (sb));
	font->showText (dest.x + 1, dest.y + 1, iToStr (sb->getID()), FONT_LATIN_SMALL_WHITE);
	string sTmp = "m " + iToStr (sb->Metal) + "/" + iToStr (sb->MaxMetal) + " +" + iToStr (sb->getMetalProd() - sb->MetalNeed);
	font->showText (dest.x + 1, dest.y + 1 + 8, sTmp, FONT_LATIN_SMALL_WHITE);

	sTmp = "o " + iToStr (sb->Oil) + "/" + iToStr (sb->MaxOil) + " +" + iToStr (sb->getOilProd() - sb->OilNeed);
	font->showText (dest.x + 1, dest.y + 1 + 16, sTmp, FONT_LATIN_SMALL_WHITE);

	sTmp = "g " + iToStr (sb->Gold) + "/" + iToStr (sb->MaxGold) + " +" + iToStr (sb->getGoldProd() - sb->GoldNeed);
	font->showText (dest.x + 1, dest.y + 1 + 24, sTmp, FONT_LATIN_SMALL_WHITE);
}

void cGameGUI::drawTopBuildings_DebugBaseServer (const cBuilding& building, const SDL_Rect& dest, unsigned int offset)
{
	assert (debugOutput.debugBaseServer && building.SubBase);

	const sSubBase* sb = client->getServer()->Map->fields[offset].getBuilding()->SubBase;
	if (sb == NULL) return;

	SDL_Rect tmp = { dest.x, dest.y, Uint16 (getTileSize()), 8u };
	if (building.data.isBig) tmp.w *= 2;

	// the VS compiler gives a warning on casting a pointer to long.
	// therefore we will first cast to long long
	// and then cut this to Unit32 again.
	SDL_FillRect (cVideo::buffer, &tmp, (Uint32) (long long) (sb));
	font->showText (dest.x + 1, dest.y + 1, iToStr (sb->getID()), FONT_LATIN_SMALL_WHITE);
	string sTmp = "m " + iToStr (sb->Metal) + "/" + iToStr (sb->MaxMetal) + " +" + iToStr (sb->getMetalProd() - sb->MetalNeed);
	font->showText (dest.x + 1, dest.y + 1 + 8, sTmp, FONT_LATIN_SMALL_WHITE);

	sTmp = "o " + iToStr (sb->Oil) + "/" + iToStr (sb->MaxOil) + " +" + iToStr (sb->getOilProd() - sb->OilNeed);
	font->showText (dest.x + 1, dest.y + 1 + 16, sTmp, FONT_LATIN_SMALL_WHITE);

	sTmp = "g " + iToStr (sb->Gold) + "/" + iToStr (sb->MaxGold) + " +" + iToStr (sb->getGoldProd() - sb->GoldNeed);
	font->showText (dest.x + 1, dest.y + 1 + 24, sTmp, FONT_LATIN_SMALL_WHITE);
}

void cGameGUI::drawTopBuildings (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	SDL_Rect dest;
	const int tileSize = getTileSize();
	const cPlayer& player = client->getActivePlayer();
	// draw top buildings (except connectors)
	const cMap& map = *client->getMap();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; ++y, dest.y += tileSize)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map.getOffset (startX, y);
		for (int x = startX; x <= endX; ++x, ++pos, dest.x += tileSize)
		{
			cBuilding* building = map.fields[pos].getBuilding();
			if (building == NULL) continue;
			if (building->data.surfacePosition != sUnitData::SURFACE_POS_GROUND) continue;
			if (!player.canSeeAnyAreaUnder (*building)) continue;
			// make sure a big building is drawn only once
			// TODO: BUG: when PosX,PosY is outside of drawing screen
			if (building->PosX != x || building->PosY != y) continue;

			building->draw (&dest, *this);
			if (debugOutput.debugBaseClient && building->SubBase)
				drawTopBuildings_DebugBaseClient (*building, dest);
			if (debugOutput.debugBaseServer && building->SubBase)
				drawTopBuildings_DebugBaseServer (*building, dest, pos);
		}
	}
}

void cGameGUI::drawShips (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	SDL_Rect dest;
	const int tileSize = getTileSize();
	const cMap& map = *client->getMap();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; ++y, dest.y += tileSize)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map.getOffset (startX, y);
		for (int x = startX; x <= endX; ++x, ++pos, dest.x += tileSize)
		{
			cVehicle* vehicle = map.fields[pos].getVehicle();
			if (vehicle == NULL) continue;
			if (vehicle->data.factorSea > 0 && vehicle->data.factorGround == 0)
			{
				vehicle->draw (dest, *this);
			}
		}
	}
}

void cGameGUI::drawAboveSeaBaseUnits (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	SDL_Rect dest;
	const int tileSize = getTileSize();
	const cPlayer& player = client->getActivePlayer();
	const cMap& map = *client->getMap();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; ++y, dest.y += tileSize)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map.getOffset (startX, y);
		for (int x = startX; x <= endX; ++x, ++pos, dest.x += tileSize)
		{
			std::vector<cBuilding*>& buildings = map.fields[pos].getBuildings();

			for (std::vector<cBuilding*>::iterator it = buildings.begin(); it != buildings.end(); ++it)
			{
				if ((*it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)
				{
					(*it)->draw (&dest, *this);
				}
			}
			for (std::vector<cBuilding*>::iterator it = buildings.begin(); it != buildings.end(); ++it)
			{
				if ((*it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)
				{
					(*it)->draw (&dest, *this);
				}
			}

			cVehicle* vehicle = map.fields[pos].getVehicle();
			if (vehicle && (vehicle->IsClearing || vehicle->IsBuilding) &&
				player.canSeeAnyAreaUnder (*vehicle))
			{
				// make sure a big vehicle is drawn only once
				// TODO: BUG: when PosX,PosY is outside of drawing screen
				if (vehicle->PosX == x && vehicle->PosY == y)
				{
					vehicle->draw (dest, *this);
				}
			}
		}
	}
}

void cGameGUI::drawVehicles (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	SDL_Rect dest;
	const int tileSize = getTileSize();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	const cMap& map = *client->getMap();
	for (int y = startY; y <= endY; ++y, dest.y += tileSize)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map.getOffset (startX, y);
		for (int x = startX; x <= endX; ++x, ++pos, dest.x += tileSize)
		{
			cVehicle* vehicle = map.fields[pos].getVehicle();
			if (vehicle == NULL) continue;
			if (vehicle->data.factorGround != 0 && !vehicle->IsBuilding && !vehicle->IsClearing)
			{
				vehicle->draw (dest, *this);
			}
		}
	}
}

void cGameGUI::drawConnectors (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	SDL_Rect dest;
	const int tileSize = getTileSize();
	const cMap& map = *client->getMap();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; ++y, dest.y += tileSize)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map.getOffset (startX, y);
		for (int x = startX; x <= endX; ++x, ++pos, dest.x += tileSize)
		{
			cBuilding* building = map.fields[pos].getTopBuilding();
			if (building == NULL) continue;
			if (building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE)
			{
				building->draw (&dest, *this);
			}
		}
	}
}

void cGameGUI::drawPlanes (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	SDL_Rect dest;
	const int tileSize = getTileSize();
	const cMap& map = *client->getMap();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; ++y, dest.y += tileSize)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map.getOffset (startX, y);
		for (int x = startX; x <= endX; ++x, ++pos, dest.x += tileSize)
		{
			std::vector<cVehicle*>& planes = map.fields[pos].getPlanes();
			for (std::vector<cVehicle*>::reverse_iterator it = planes.rbegin();
				 it != planes.rend();
				 ++it)
			{
				cVehicle& plane = **it;
				plane.draw (dest, *this);
			}
		}
	}
}

void cGameGUI::drawResources (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	const int tileSize = getTileSize();
	const cPlayer& player = client->getActivePlayer();
	const cMap& map = *client->getMap();
	SDL_Rect dest, tmp, src = { 0, 0, Uint16 (tileSize), Uint16 (tileSize) };
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; ++y, dest.y += tileSize)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map.getOffset (startX, y);
		for (int x = startX; x <= endX; ++x, ++pos, dest.x += tileSize)
		{
			if (!player.hasResourceExplored (pos)) continue;
			if (map.isBlocked (pos)) continue;

			const sResources& resource = map.getResource (pos);
			if (resource.typ == RES_NONE)
			{
				src.x = 0;
				tmp = dest;
				if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_metal->w != ResourceData.res_metal_org->w / 64 * tileSize || ResourceData.res_metal->h != tileSize)) scaleSurface (ResourceData.res_metal_org, ResourceData.res_metal, ResourceData.res_metal_org->w / 64 * tileSize, tileSize);
				SDL_BlitSurface (ResourceData.res_metal, &src, cVideo::buffer, &tmp);
			}
			else
			{
				src.x = resource.value * tileSize;
				tmp = dest;
				if (resource.typ == RES_METAL)
				{
					if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_metal->w != ResourceData.res_metal_org->w / 64 * tileSize || ResourceData.res_metal->h != tileSize)) scaleSurface (ResourceData.res_metal_org, ResourceData.res_metal, ResourceData.res_metal_org->w / 64 * tileSize, tileSize);
					SDL_BlitSurface (ResourceData.res_metal, &src, cVideo::buffer, &tmp);
				}
				else if (resource.typ == RES_OIL)
				{
					if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_oil->w != ResourceData.res_oil_org->w / 64 * tileSize || ResourceData.res_oil->h != tileSize)) scaleSurface (ResourceData.res_oil_org, ResourceData.res_oil, ResourceData.res_oil_org->w / 64 * tileSize, tileSize);
					SDL_BlitSurface (ResourceData.res_oil, &src, cVideo::buffer, &tmp);
				}
				else // Gold
				{
					if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_gold->w != ResourceData.res_gold_org->w / 64 * tileSize || ResourceData.res_gold->h != tileSize)) scaleSurface (ResourceData.res_gold_org, ResourceData.res_gold, ResourceData.res_gold_org->w / 64 * tileSize, tileSize);
					SDL_BlitSurface (ResourceData.res_gold, &src, cVideo::buffer, &tmp);
				}
			}
		}
	}
}

void cGameGUI::drawSelectionBox (int zoomOffX, int zoomOffY)
{
	if (!mouseBox.isValid()) return;

	const int mouseTopX = static_cast<int> (std::min(mouseBox.getBox().getMinCorner()[0], mouseBox.getBox().getMaxCorner()[0]) * getTileSize());
	const int mouseTopY = static_cast<int> (std::min(mouseBox.getBox().getMinCorner()[1], mouseBox.getBox().getMaxCorner()[1]) * getTileSize());
	const int mouseBottomX = static_cast<int> (std::max(mouseBox.getBox().getMinCorner()[0], mouseBox.getBox().getMaxCorner()[0]) * getTileSize());
	const int mouseBottomY = static_cast<int> (std::max(mouseBox.getBox().getMinCorner()[1], mouseBox.getBox().getMaxCorner()[1]) * getTileSize());
	const Uint32 color = 0xFFFFFF00;
	SDL_Rect d;

	d.x = mouseTopX - zoomOffX + HUD_LEFT_WIDTH;
	d.y = mouseBottomY - zoomOffY + 20;
	d.w = mouseBottomX - mouseTopX;
	d.h = 1;
	SDL_FillRect (cVideo::buffer, &d, color);

	d.x = mouseTopX - zoomOffX + HUD_LEFT_WIDTH;
	d.y = mouseTopY - zoomOffY + 20;
	d.w = mouseBottomX - mouseTopX;
	d.h = 1;
	SDL_FillRect (cVideo::buffer, &d, color);

	d.x = mouseTopX - zoomOffX + HUD_LEFT_WIDTH;
	d.y = mouseTopY - zoomOffY + 20;
	d.w = 1;
	d.h = mouseBottomY - mouseTopY;
	SDL_FillRect (cVideo::buffer, &d, color);

	d.x = mouseBottomX - zoomOffX + HUD_LEFT_WIDTH;
	d.y = mouseTopY - zoomOffY + 20;
	d.w = 1;
	d.h = mouseBottomY - mouseTopY;
	SDL_FillRect (cVideo::buffer, &d, color);
}

void cGameGUI::displayMessages()
{
	if (messages.empty()) return;

	int height = 0;
	for (int i = (int) messages.size() - 1; i >= 0; i--)
	{
		const sMessage* message = messages[i];
		height += 17 + font->getFontHeight() * (message->len / (Video.getResolutionX() - 300));
	}

	if (cSettings::getInstance().isAlphaEffects())
	{
		SDL_Rect rect = { 180, 30, Uint16 (Video.getResolutionX() - 200), Uint16 (height + 6) };
		Video.applyShadow (&rect);
	}
	SDL_Rect dest = { 180 + 2, 34, Video.getResolutionX() - 204, height};

	for (unsigned int i = 0; i < messages.size(); i++)
	{
		const sMessage* message = messages[i];
		string msgString = message->msg;
		// HACK TO SHOW PLAYERCOLOR IN CHAT
		SDL_Surface* color = NULL;
		for (unsigned int i = 0; i < msgString.length(); i++)
		{
			if (msgString[i] == ':')   //scan for chatmessages from _players_
			{
				string tmpString = msgString.substr (0, i);
				cPlayer* const Player = client->getPlayerFromString (tmpString);
				if (Player)
				{
					color = Player->getColorSurface();
				}
				break;
			}
		}
		if (color != NULL)
		{
			const int CELLSPACE = 3;
			SDL_Rect rColorSrc = { 0, 0, 10, Uint16 (font->getFontHeight()) };
			SDL_Rect rDest = dest;
			rDest.w = rColorSrc.w;
			rDest.h = rColorSrc.h;
			SDL_BlitSurface (color, &rColorSrc, cVideo::buffer, &rDest);  // blit color
			dest.x += rColorSrc.w + CELLSPACE; // add border for color
			dest.w -= rColorSrc.w + CELLSPACE;
			dest.y = font->showTextAsBlock (dest, msgString);
			dest.x -= rColorSrc.w + CELLSPACE; // reset border from color
			dest.w += rColorSrc.w + CELLSPACE;
		}
		else
		{
			dest.y = font->showTextAsBlock (dest, msgString);
		}
		dest.y += 5;
	}
}

void cGameGUI::scaleColors()
{
	for (int i = 0; i < PLAYERCOLORS; i++)
	{
		scaleSurface (OtherData.colors_org[i], OtherData.colors[i], (int) (OtherData.colors_org[i]->w * getZoom()), (int) (OtherData.colors_org[i]->h * getZoom()));
	}
}

void cGameGUI::scaleSurfaces()
{
	// Terrain:
	client->getMap()->staticMap->scaleSurfaces (getTileSize());

	UnitsData.scaleSurfaces (getZoom());
	// Bänder:
	if (GraphicsData.gfx_band_small_org && GraphicsData.gfx_band_small) scaleSurface (GraphicsData.gfx_band_small_org, GraphicsData.gfx_band_small, getTileSize(), getTileSize());
	if (GraphicsData.gfx_band_big_org && GraphicsData.gfx_band_big) scaleSurface (GraphicsData.gfx_band_big_org, GraphicsData.gfx_band_big, getTileSize() * 2, getTileSize() * 2);

	// Resources:
	if (ResourceData.res_metal_org && ResourceData.res_metal) scaleSurface (ResourceData.res_metal_org, ResourceData.res_metal, ResourceData.res_metal_org->w / 64 * getTileSize(), getTileSize());
	if (ResourceData.res_oil_org && ResourceData.res_oil) scaleSurface (ResourceData.res_oil_org, ResourceData.res_oil, ResourceData.res_oil_org->w / 64 * getTileSize(), getTileSize());
	if (ResourceData.res_gold_org && ResourceData.res_gold) scaleSurface (ResourceData.res_gold_org, ResourceData.res_gold, ResourceData.res_gold_org->w / 64 * getTileSize(), getTileSize());

	// Big Beton:
	if (GraphicsData.gfx_big_beton_org && GraphicsData.gfx_big_beton) scaleSurface (GraphicsData.gfx_big_beton_org, GraphicsData.gfx_big_beton, getTileSize() * 2, getTileSize() * 2);

	// Andere:
	if (GraphicsData.gfx_exitpoints_org && GraphicsData.gfx_exitpoints) scaleSurface (GraphicsData.gfx_exitpoints_org, GraphicsData.gfx_exitpoints, GraphicsData.gfx_exitpoints_org->w / 64 * getTileSize(), getTileSize());

	// FX:
#define SCALE_FX(a) if (a) scaleSurface(a[0], a[1], (a[0]->w * getTileSize()) / 64, (a[0]->h * getTileSize()) / 64);
	SCALE_FX (EffectsData.fx_explo_small);
	SCALE_FX (EffectsData.fx_explo_big);
	SCALE_FX (EffectsData.fx_explo_water);
	SCALE_FX (EffectsData.fx_explo_air);
	SCALE_FX (EffectsData.fx_muzzle_big);
	SCALE_FX (EffectsData.fx_muzzle_small);
	SCALE_FX (EffectsData.fx_muzzle_med);
	SCALE_FX (EffectsData.fx_hit);
	SCALE_FX (EffectsData.fx_smoke);
	SCALE_FX (EffectsData.fx_rocket);
	SCALE_FX (EffectsData.fx_dark_smoke);
	SCALE_FX (EffectsData.fx_tracks);
	SCALE_FX (EffectsData.fx_corpse);
	SCALE_FX (EffectsData.fx_absorb);
}

void cGameGUI::openPanel()
{
	PlayFX (SoundData.SNDPanelOpen);
	SDL_Rect top = { 0, Sint16 ((Video.getResolutionY() / 2) - 479), 171, 479 };
	SDL_Rect bottom = { 0, Sint16 (Video.getResolutionY() / 2), 171, 481 };
	SDL_BlitSurface (GraphicsData.gfx_panel_top, NULL, cVideo::buffer, NULL);
	SDL_Rect tmp = bottom;
	SDL_BlitSurface (GraphicsData.gfx_panel_bottom, NULL, cVideo::buffer, &tmp);
	while (top.y > -479)
	{
		Video.draw();
		SDL_Delay (10);
		top.y -= 10;
		bottom.y += 10;
		draw (false, false);
		tmp = top;
		SDL_BlitSurface (GraphicsData.gfx_panel_top, NULL, cVideo::buffer, &tmp);
		SDL_BlitSurface (GraphicsData.gfx_panel_bottom, NULL, cVideo::buffer, &bottom);
	}
}

void cGameGUI::closePanel()
{
	PlayFX (SoundData.SNDPanelClose);
	SDL_Rect top = { 0, -480, 171, 479 };
	SDL_Rect bottom = { 0, Sint16 (Video.getResolutionY()), 171, 481 };
	while (bottom.y > Video.getResolutionY() / 2)
	{
		Video.draw();
		SDL_Delay (10);
		top.y += 10;
		if (top.y > (Video.getResolutionY() / 2) - 479 - 9) top.y = (Video.getResolutionY() / 2) - 479;
		bottom.y -= 10;
		if (bottom.y < Video.getResolutionY() / 2 + 9) bottom.y = Video.getResolutionY() / 2;
		draw (false, false);
		SDL_Rect tmp = top;
		SDL_BlitSurface (GraphicsData.gfx_panel_top, NULL, cVideo::buffer, &tmp);
		tmp = bottom;
		SDL_BlitSurface (GraphicsData.gfx_panel_bottom, NULL, cVideo::buffer, &tmp);
	}
	Video.draw();
	SDL_Delay (100);
}

void cGameGUI::drawUnitCircles()
{
	SDL_Rect clipRect = { HUD_LEFT_WIDTH, HUD_TOP_HIGHT, Uint16 (Video.getResolutionX() - HUD_TOTAL_WIDTH), Uint16 (Video.getResolutionY() - HUD_TOTAL_HIGHT) };
	SDL_SetClipRect (cVideo::buffer, &clipRect);

	cVehicle* selectedVehicle = getSelectedVehicle();
	cBuilding* selectedBuilding = getSelectedBuilding();
	const cPlayer& player = client->getActivePlayer();

	if (selectedVehicle && selectedUnit->isDisabled() == false)
	{
		cVehicle& v = *selectedVehicle;
		const bool movementOffset = !v.IsBuilding && !v.IsClearing;
		const int spx = getScreenPosX (v, movementOffset);
		const int spy = getScreenPosY (v, movementOffset);
		if (scanChecked())
		{
			if (v.data.isBig)
			{
				drawCircle (spx + getTileSize(), spy + getTileSize(), v.data.scan * getTileSize(), SCAN_COLOR, cVideo::buffer);
			}
			else
			{
				drawCircle (spx + getTileSize() / 2, spy + getTileSize() / 2, v.data.scan * getTileSize(), SCAN_COLOR, cVideo::buffer);
			}
		}
		if (rangeChecked())
		{
			if (v.data.canAttack & TERRAIN_AIR) drawCircle (spx + getTileSize() / 2, spy + getTileSize() / 2, v.data.range * getTileSize() + 2, RANGE_AIR_COLOR, cVideo::buffer);
			else drawCircle (spx + getTileSize() / 2, spy + getTileSize() / 2, v.data.range * getTileSize() + 1, RANGE_GROUND_COLOR, cVideo::buffer);
		}
		if (v.owner == &player &&
			(
				(v.IsBuilding && v.BuildRounds == 0) ||
				(v.IsClearing && v.ClearingRounds == 0)
			) && !v.BuildPath)
		{
			const cMap& map = *client->getMap();

			if (v.data.isBig)
			{
				if (map.possiblePlace (v, v.PosX - 1, v.PosY - 1)) drawExitPoint (spx - getTileSize(),     spy - getTileSize());
				if (map.possiblePlace (v, v.PosX    , v.PosY - 1)) drawExitPoint (spx,                spy - getTileSize());
				if (map.possiblePlace (v, v.PosX + 1, v.PosY - 1)) drawExitPoint (spx + getTileSize(),     spy - getTileSize());
				if (map.possiblePlace (v, v.PosX + 2, v.PosY - 1)) drawExitPoint (spx + getTileSize() * 2, spy - getTileSize());
				if (map.possiblePlace (v, v.PosX - 1, v.PosY)) drawExitPoint (spx - getTileSize(),     spy);
				if (map.possiblePlace (v, v.PosX + 2, v.PosY)) drawExitPoint (spx + getTileSize() * 2, spy);
				if (map.possiblePlace (v, v.PosX - 1, v.PosY + 1)) drawExitPoint (spx - getTileSize(),     spy + getTileSize());
				if (map.possiblePlace (v, v.PosX + 2, v.PosY + 1)) drawExitPoint (spx + getTileSize() * 2, spy + getTileSize());
				if (map.possiblePlace (v, v.PosX - 1, v.PosY + 2)) drawExitPoint (spx - getTileSize(),     spy + getTileSize() * 2);
				if (map.possiblePlace (v, v.PosX    , v.PosY + 2)) drawExitPoint (spx,                spy + getTileSize() * 2);
				if (map.possiblePlace (v, v.PosX + 1, v.PosY + 2)) drawExitPoint (spx + getTileSize(),     spy + getTileSize() * 2);
				if (map.possiblePlace (v, v.PosX + 2, v.PosY + 2)) drawExitPoint (spx + getTileSize() * 2, spy + getTileSize() * 2);
			}
			else
			{
				if (map.possiblePlace (v, v.PosX - 1, v.PosY - 1)) drawExitPoint (spx - getTileSize(), spy - getTileSize());
				if (map.possiblePlace (v, v.PosX    , v.PosY - 1)) drawExitPoint (spx,            spy - getTileSize());
				if (map.possiblePlace (v, v.PosX + 1, v.PosY - 1)) drawExitPoint (spx + getTileSize(), spy - getTileSize());
				if (map.possiblePlace (v, v.PosX - 1, v.PosY)) drawExitPoint (spx - getTileSize(), spy);
				if (map.possiblePlace (v, v.PosX + 1, v.PosY)) drawExitPoint (spx + getTileSize(), spy);
				if (map.possiblePlace (v, v.PosX - 1, v.PosY + 1)) drawExitPoint (spx - getTileSize(), spy + getTileSize());
				if (map.possiblePlace (v, v.PosX    , v.PosY + 1)) drawExitPoint (spx,            spy + getTileSize());
				if (map.possiblePlace (v, v.PosX + 1, v.PosY + 1)) drawExitPoint (spx + getTileSize(), spy + getTileSize());
			}
		}
		if (mouseInputMode == placeBand)
		{
			if (v.BuildingTyp.getUnitDataOriginalVersion()->isBig)
			{
				SDL_Rect dest;
				dest.x = HUD_LEFT_WIDTH - (int) (offX * getZoom()) + getTileSize() * v.BandX;
				dest.y = HUD_TOP_HIGHT - (int) (offY * getZoom()) + getTileSize() * v.BandY;
				CHECK_SCALING (GraphicsData.gfx_band_big, GraphicsData.gfx_band_big_org, getTileSize() / 64.0f);
				SDL_BlitSurface (GraphicsData.gfx_band_big, NULL, cVideo::buffer, &dest);
			}
			else
			{
				const auto mouseTilePosition = getTilePosition(cMouse::getInstance().getPosition());
				const int x = mouseTilePosition.x();
				const int y = mouseTilePosition.y();
				if (x == v.PosX || y == v.PosY)
				{
					SDL_Rect dest;
					dest.x = HUD_LEFT_WIDTH - (int) (offX * getZoom()) + getTileSize() * x;
					dest.y = HUD_TOP_HIGHT - (int) (offY * getZoom()) + getTileSize() * y;
					CHECK_SCALING (GraphicsData.gfx_band_small, GraphicsData.gfx_band_small_org, getTileSize() / 64.0f);
					SDL_BlitSurface (GraphicsData.gfx_band_small, NULL, cVideo::buffer, &dest);
					v.BandX = x;
					v.BandY = y;
				}
				else
				{
					v.BandX = v.PosX;
					v.BandY = v.PosY;
				}
			}
		}
		if (mouseInputMode == activateVehicle && v.owner == &player)
		{
			v.DrawExitPoints (v.storedUnits[vehicleToActivate]->data, *this);
		}
	}
	else if (selectedBuilding && selectedUnit->isDisabled() == false)
	{
		const int spx = getScreenPosX (*selectedBuilding);
		const int spy = getScreenPosY (*selectedBuilding);
		if (scanChecked())
		{
			if (selectedBuilding->data.isBig)
			{
				drawCircle (spx + getTileSize(),
							spy + getTileSize(),
							selectedBuilding->data.scan * getTileSize(), SCAN_COLOR, cVideo::buffer);
			}
			else
			{
				drawCircle (spx + getTileSize() / 2,
							spy + getTileSize() / 2,
							selectedBuilding->data.scan * getTileSize(), SCAN_COLOR, cVideo::buffer);
			}
		}
		if (rangeChecked() && (selectedBuilding->data.canAttack & TERRAIN_GROUND) && !selectedBuilding->data.explodesOnContact)
		{
			drawCircle (spx + getTileSize() / 2,
						spy + getTileSize() / 2,
						selectedBuilding->data.range * getTileSize() + 2, RANGE_GROUND_COLOR, cVideo::buffer);
		}
		if (rangeChecked() && (selectedBuilding->data.canAttack & TERRAIN_AIR))
		{
			drawCircle (spx + getTileSize() / 2,
						spy + getTileSize() / 2,
						selectedBuilding->data.range * getTileSize() + 2, RANGE_AIR_COLOR, cVideo::buffer);
		}

		if (selectedBuilding->BuildList.empty() == false &&
			!selectedBuilding->IsWorking &&
			selectedBuilding->BuildList[0].metall_remaining <= 0 &&
			selectedBuilding->owner == &player)
		{
			selectedBuilding->DrawExitPoints (*selectedBuilding->BuildList[0].type.getUnitDataOriginalVersion(), *this);
		}
		if (mouseInputMode == activateVehicle && selectedBuilding->owner == &player)
		{
			selectedBuilding->DrawExitPoints (selectedBuilding->storedUnits[vehicleToActivate]->data, *this);
		}
	}
	drawLockList (client->getActivePlayer());

	SDL_SetClipRect (cVideo::buffer, NULL);
}

//--------------------------------------------------------------------------
/** Draws all entries, that are in the lock list. */
//--------------------------------------------------------------------------
void cGameGUI::drawLockList (cPlayer& player)
{
	if (!lockChecked()) return;
	const int tileSize = getTileSize();
	for (size_t i = 0; i < player.LockList.size(); i++)
	{
		cUnit* unit = player.LockList[i];

		if (!player.canSeeAnyAreaUnder (*unit))
		{
			unit->lockerPlayer = NULL;
			player.LockList.erase (player.LockList.begin() + i);
			i--;
			continue;
		}
		const SDL_Rect screenPos = {Sint16 (getScreenPosX (*unit)), Sint16 (getScreenPosY (*unit)), 0, 0};

		if (scanChecked())
		{
			if (unit->data.isBig)
				drawCircle (screenPos.x + tileSize, screenPos.y + tileSize, unit->data.scan * tileSize, SCAN_COLOR, cVideo::buffer);
			else
				drawCircle (screenPos.x + tileSize / 2, screenPos.y + tileSize / 2, unit->data.scan * tileSize, SCAN_COLOR, cVideo::buffer);
		}
		if (rangeChecked() && (unit->data.canAttack & TERRAIN_GROUND))
			drawCircle (screenPos.x + tileSize / 2, screenPos.y + tileSize / 2,
						unit->data.range * tileSize + 1, RANGE_GROUND_COLOR, cVideo::buffer);
		if (rangeChecked() && (unit->data.canAttack & TERRAIN_AIR))
			drawCircle (screenPos.x + tileSize / 2, screenPos.y + tileSize / 2,
						unit->data.range * tileSize + 2, RANGE_AIR_COLOR, cVideo::buffer);
		if (ammoChecked() && unit->data.canAttack)
			drawMunBar (*unit, screenPos);
		if (hitsChecked())
			drawHealthBar (*unit, screenPos);
	}
}

void cGameGUI::drawExitPoint (int x, int y)
{
	const int nr = getAnimationSpeed() % 5;
	SDL_Rect src;
	src.x = getTileSize() * nr;
	src.y = 0;
	src.w = getTileSize();
	src.h = getTileSize();
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	const float factor = getTileSize() / 64.0f;

	CHECK_SCALING (GraphicsData.gfx_exitpoints, GraphicsData.gfx_exitpoints_org, factor);
	SDL_BlitSurface (GraphicsData.gfx_exitpoints, &src, cVideo::buffer, &dest);
}

void cGameGUI::toggleMouseInputMode (eMouseInputMode mode)
{
	if (mouseInputMode == mode)
		mouseInputMode = normalInput;
	else
		mouseInputMode = mode;

	updateMouseCursor();
}

void cGameGUI::checkMouseInputMode()
{
	switch (mouseInputMode)
	{
		case mouseInputAttackMode:
		case disableMode:
		case stealMode:
			if (selectedUnit && !selectedUnit->data.shotsCur)
				mouseInputMode = normalInput;
			break;
		case loadMode:
			if (selectedUnit && selectedUnit->data.storageUnitsCur == selectedUnit->data.storageUnitsMax)
				mouseInputMode = normalInput;
			break;
		default:
			break;
	}
}

void cGameGUI::savePosition (int slotNumber)
{
	if (slotNumber < 0 || slotNumber >= MAX_SAVE_POSITIONS) return;

	savedPositions[slotNumber].offsetX = offX + (int) ((Video.getResolutionX() - HUD_TOTAL_WIDTH) / getZoom() / 2);
	savedPositions[slotNumber].offsetY = offY + (int) ((Video.getResolutionY() - HUD_TOTAL_HIGHT) / getZoom() / 2);
}

void cGameGUI::jumpToSavedPos (int slotNumber)
{
	if (slotNumber < 0 || slotNumber >= MAX_SAVE_POSITIONS) return;

	const int offsetX = savedPositions[slotNumber].offsetX - (int) ((Video.getResolutionX() - HUD_TOTAL_WIDTH) / getZoom() / 2);
	const int offsetY = savedPositions[slotNumber].offsetY - (int) ((Video.getResolutionY() - HUD_TOTAL_HIGHT) / getZoom() / 2);

	setOffsetPosition (offsetX, offsetY);
}

//------------------------------------------------------------------------------
/** Returns the size of the menu and the position */
//------------------------------------------------------------------------------
SDL_Rect cGameGUI::getMenuSize (const cUnit& unit) const
{
	SDL_Rect dest;
	dest.x = getScreenPosX (unit);
	dest.y = getScreenPosY (unit);
	dest.h = getNumberOfMenuEntries (unit) * 22;
	dest.w = 42;
	const int i = dest.h;
	int size = getTileSize();

	if (unit.data.isBig)
		size *= 2;

	if (dest.x + size + 42 >= Video.getResolutionX() - 12)
		dest.x -= 42;
	else
		dest.x += size;

	if (dest.y - (i - size) / 2 <= 24)
	{
		dest.y -= (i - size) / 2;
		dest.y += - (dest.y - 24);
	}
	else if (dest.y - (i - size) / 2 + i >= Video.getResolutionY() - 24)
	{
		dest.y -= (i - size) / 2;
		dest.y -= (dest.y + i) - (Video.getResolutionY() - 24);
	}
	else
		dest.y -= (i - size) / 2;

	return dest;
}

//------------------------------------------------------------------------------
/** Returns true, if the coordinates are in the menu's space */
//------------------------------------------------------------------------------
bool cGameGUI::areCoordsOverMenu(const cUnit& unit, const cPosition& position) const
{
	const SDL_Rect r = getMenuSize (unit);

	if(position.x() < r.x || position.x() > r.x + r.w)
		return false;
	if(position.y() < r.y || position.y() > r.y + r.h)
		return false;
	return true;
}

//------------------------------------------------------------------------------
void cGameGUI::setMenuSelection (const cUnit& unit)
{
	SDL_Rect dest = getMenuSize (unit);
	selectedMenuButtonIndex = (cMouse::getInstance().getPosition().y() - dest.y) / 22;
}

//------------------------------------------------------------------------------
int cGameGUI::getNumberOfMenuEntries (const cUnit& unit) const
{
	int result = 2; // Info/Help + Done

	if (unit.owner != &client->getActivePlayer())
		return result;
	if (unit.isDisabled()) return result;

	// Attack
	if (unit.data.canAttack && unit.data.shotsCur)
		++result;

	// Build
	if (unit.data.canBuild.empty() == false && unit.isUnitBuildingABuilding() == false)
		++result;

	// Distribute
	if (unit.data.canMineMaxRes > 0 && unit.isUnitWorking())
		++result;

	// Transfer
	if (unit.data.storeResType != sUnitData::STORE_RES_NONE && unit.isUnitBuildingABuilding() == false && unit.isUnitClearing() == false)
		++result;

	// Start
	if (unit.data.canWork && unit.buildingCanBeStarted())
		++result;

	// Auto survey
	if (unit.data.canSurvey)
		++result;

	// Stop
	if (unit.canBeStoppedViaUnitMenu())
		++result;

	// Remove
	if (unit.data.canClearArea && client->getMap()->fields[client->getMap()->getOffset (unit.PosX, unit.PosY)].getRubble() && unit.isUnitClearing() == false)
		++result;

	// Manual Fire
	if (unit.manualFireActive || unit.data.canAttack)
		++result;

	// Sentry
	if (unit.sentryActive || unit.data.canAttack || (!unit.isABuilding() && !unit.canBeStoppedViaUnitMenu()))
		++result;

	// Activate / Load
	if (unit.data.storageUnitsMax > 0)
		result += 2;

	// Research
	if (unit.data.canResearch && unit.isUnitWorking())
		++result;

	// Gold upgrades screen
	if (unit.data.convertsGold)
		++result;

	// Update building(s)
	if (unit.buildingCanBeUpgraded())
		result += 2;

	// Self destruct
	if (unit.data.canSelfDestroy)
		++result;

	// Ammo
	if (unit.data.canRearm && unit.data.storageResCur >= 1)
		++result;

	// Repair
	if (unit.data.canRepair && unit.data.storageResCur >= 1)
		++result;

	// Lay Mines
	if (unit.data.canPlaceMines && unit.data.storageResCur > 0)
		++result;

	// Clear Mines
	if (unit.data.canPlaceMines && unit.data.storageResCur < unit.data.storageResMax)
		++result;

	// Sabotage/disable
	if (unit.data.canCapture && unit.data.shotsCur)
		++result;

	// Steal
	if (unit.data.canDisable && unit.data.shotsCur)
		++result;

	return result;
}

//------------------------------------------------------------------------------
void cGameGUI::drawMenu (const cUnit& unit)
{
	if (unit.isBeeingAttacked)
		return;
	if (unit.isUnitMoving())
		return;

	if (mouseInputMode == activateVehicle)
	{
		unitMenuActive = false;
		return;
	}

	if (unit.factoryHasJustFinishedBuilding())
		return;

	SDL_Rect dest = getMenuSize (unit);
	bool markerPossible = (areCoordsOverMenu(unit, cMouse::getInstance().getPosition()) && (selectedMenuButtonIndex == (cMouse::getInstance().getPosition().y() - dest.y) / 22));
	int nr = 0;

	if (unit.isDisabled() == false && unit.owner == &client->getActivePlayer())
	{
		// Attack:
		if (unit.data.canAttack && unit.data.shotsCur)
		{
			bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || mouseInputMode == mouseInputAttackMode;
			drawContextItem (lngPack.i18n ("Text~Others~Attack_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Build:
		if (unit.data.canBuild.empty() == false && unit.isUnitBuildingABuilding() == false)
		{
			bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Others~Build_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Distribute:
		if (unit.data.canMineMaxRes > 0 && unit.isUnitWorking())
		{
			bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Others~Distribution_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Transfer:
		if (unit.data.storeResType != sUnitData::STORE_RES_NONE && unit.isUnitBuildingABuilding() == false && unit.isUnitClearing() == false)
		{
			bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || mouseInputMode == transferMode;
			drawContextItem (lngPack.i18n ("Text~Others~Transfer_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Start:
		if (unit.data.canWork && unit.buildingCanBeStarted())
		{
			bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Others~Start_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Auto survey movejob of surveyor
		if (unit.data.canSurvey)
		{
			bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || unit.isAutoMoveJobActive();
			drawContextItem (lngPack.i18n ("Text~Others~Auto_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Stop:
		if (unit.canBeStoppedViaUnitMenu())
		{
			bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Others~Stop_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Remove:
		if (unit.data.canClearArea && client->getMap()->fields[client->getMap()->getOffset (unit.PosX, unit.PosY)].getRubble() && unit.isUnitClearing() == false)
		{
			bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Others~Clear_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Manual fire
		if ((unit.manualFireActive || unit.data.canAttack))
		{
			bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || unit.manualFireActive;
			drawContextItem (lngPack.i18n ("Text~Others~ManualFireMode_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Sentry status:
		if ((unit.sentryActive || unit.data.canAttack || (!unit.isABuilding() && !unit.canBeStoppedViaUnitMenu())))
		{
			bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || unit.sentryActive;
			drawContextItem (lngPack.i18n ("Text~Others~Sentry"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Activate / Load:
		if (unit.data.storageUnitsMax > 0)
		{
			// Activate:
			bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Others~Active_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;

			// Load:
			isMarked = (markerPossible && selectedMenuButtonIndex == nr) || mouseInputMode == loadMode;
			drawContextItem (lngPack.i18n ("Text~Others~Load_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// research
		if (unit.data.canResearch && unit.isUnitWorking())
		{
			bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Others~Research"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// gold upgrades screen
		if (unit.data.convertsGold)
		{
			bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Others~Upgrademenu_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Updates:
		if (unit.buildingCanBeUpgraded())
		{
			// update this building
			bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Others~Upgradethis_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
			
			// Update all buildings of this type in this subbase
			isMarked = markerPossible && selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Others~UpgradeAll_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Self destruct
		if (unit.data.canSelfDestroy)
		{
			bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Others~Destroy_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Ammo:
		if (unit.data.canRearm && unit.data.storageResCur >= 1)
		{
			bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || mouseInputMode == muniActive;
			drawContextItem (lngPack.i18n ("Text~Others~Reload_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Repair:
		if (unit.data.canRepair && unit.data.storageResCur >= 1)
		{
			bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || mouseInputMode == repairActive;
			drawContextItem (lngPack.i18n ("Text~Others~Repair_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Lay mines:
		if (unit.data.canPlaceMines && unit.data.storageResCur > 0)
		{
			bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || unit.isUnitLayingMines();
			drawContextItem (lngPack.i18n ("Text~Others~Seed"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Collect/clear mines:
		if (unit.data.canPlaceMines && unit.data.storageResCur < unit.data.storageResMax)
		{
			bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || unit.isUnitClearingMines();
			drawContextItem (lngPack.i18n ("Text~Others~Clear_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Sabotage/disable:
		if (unit.data.canDisable && unit.data.shotsCur)
		{
			bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || mouseInputMode == disableMode;
			drawContextItem (lngPack.i18n ("Text~Others~Disable_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

		// Steal:
		if (unit.data.canCapture && unit.data.shotsCur)
		{
			bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || mouseInputMode == stealMode;
			drawContextItem (lngPack.i18n ("Text~Others~Steal_7"), isMarked, dest.x, dest.y, cVideo::buffer);
			dest.y += 22;
			++nr;
		}

	}
	// Info:
	bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
	drawContextItem (lngPack.i18n ("Text~Others~Info_7"), isMarked, dest.x, dest.y, cVideo::buffer);
	dest.y += 22;
	++nr;

	// Done:
	isMarked = markerPossible && selectedMenuButtonIndex == nr;
	drawContextItem (lngPack.i18n ("Text~Others~Done_7"), isMarked, dest.x, dest.y, cVideo::buffer);
}

//------------------------------------------------------------------------------
void cGameGUI::menuReleased (cUnit& unit)
{
	SDL_Rect dest = getMenuSize (unit);
	int exeNr = -1000;
	if(areCoordsOverMenu(unit, cMouse::getInstance().getPosition()))
		exeNr = (cMouse::getInstance().getPosition().y() - dest.y) / 22;

	if (exeNr != selectedMenuButtonIndex)
	{
		selectedMenuButtonIndex = -1;
		return;
	}
	if (unit.isUnitMoving() || unit.isBeeingAttacked)
		return;

	if (unit.factoryHasJustFinishedBuilding())
		return;

	int nr = 0;

	// no menu if something disabled - original behavior -- nonsinn
	if (unit.isDisabled() == false && unit.owner == &client->getActivePlayer())
	{
		// attack:
		if (unit.data.canAttack && unit.data.shotsCur)
		{
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				toggleMouseInputMode (mouseInputAttackMode);
				return;
			}
			++nr;
		}

		// Build:
		if (unit.data.canBuild.empty() == false && unit.isUnitBuildingABuilding() == false)
		{
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				unit.executeBuildCommand (*this);
				return;
			}
			++nr;
		}

		// distribute:
		if (unit.data.canMineMaxRes > 0 && unit.isUnitWorking())
		{
			if (exeNr == nr)
			{
				cBuilding* building = static_cast<cBuilding*> (&unit);
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				building->executeMineManagerCommand (*this);
				return;
			}
			++nr;
		}

		// transfer:
		if (unit.data.storeResType != sUnitData::STORE_RES_NONE && unit.isUnitBuildingABuilding() == false && unit.isUnitClearing() == false)
		{
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				toggleMouseInputMode (transferMode);
				return;
			}
			++nr;
		}

		// Start:
		if (unit.data.canWork && unit.buildingCanBeStarted())
		{
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				sendWantStartWork (*client, unit);
				return;
			}
			++nr;
		}

		// auto
		if (unit.data.canSurvey)
		{
			if (exeNr == nr)
			{
				cVehicle* vehicle = static_cast<cVehicle*> (&unit);
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				vehicle->executeAutoMoveJobCommand (*client);
				return;
			}
			++nr;
		}

		// stop:
		if (unit.canBeStoppedViaUnitMenu())
		{
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				unit.executeStopCommand (*client);
				return;
			}
			++nr;
		}

		// remove:
		if (unit.data.canClearArea && client->getMap()->fields[client->getMap()->getOffset (unit.PosX, unit.PosY)].getRubble() != 0 && unit.isUnitClearing() == false)
		{
			if (exeNr == nr)
			{
				assert (unit.isAVehicle());
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				sendWantStartClear (*client, static_cast<cVehicle&> (unit));
				return;
			}
			++nr;
		}

		// manual Fire:
		if (unit.manualFireActive || unit.data.canAttack)
		{
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				sendChangeManualFireStatus (*client, unit.iID, unit.isAVehicle());
				return;
			}
			++nr;
		}

		// sentry:
		if (unit.sentryActive || unit.data.canAttack || (!unit.isABuilding() && !unit.canBeStoppedViaUnitMenu()))
		{
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				sendChangeSentry (*client, unit.iID, unit.isAVehicle());
				return;
			}
			++nr;
		}

		// activate/load:
		if (unit.data.storageUnitsMax > 0)
		{
			// activate:
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				unit.executeActivateStoredVehiclesCommand(*this);
				return;
			}
			++nr;

			// load:
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				toggleMouseInputMode (loadMode);
				return;
			}
			++nr;
		}

		// research
		if (unit.data.canResearch && unit.isUnitWorking())
		{
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				cDialogResearch researchDialog (*client);
				switchTo(researchDialog, client);
				return;
			}
			++nr;
		}

		// gold upgrades screen
		if (unit.data.convertsGold)
		{
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				cUpgradeMenu upgradeMenu(*client);
				switchTo(upgradeMenu, client);
				return;
			}
			++nr;
		}

		// Updates:
		if (unit.buildingCanBeUpgraded())
		{
			// update this building
			if (exeNr == nr)
			{
				cBuilding* building = static_cast<cBuilding*> (&unit);

				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				building->executeUpdateBuildingCommmand (*client, false);
				return;
			}
			++nr;
			
			// Update all buildings of this type in this subbase
			if (exeNr == nr)
			{
				cBuilding* building = static_cast<cBuilding*> (&unit);

				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				building->executeUpdateBuildingCommmand (*client, true);
				return;
			}
			++nr;
		}

		// Self destruct
		if (unit.data.canSelfDestroy)
		{
			if (exeNr == nr)
			{
				cBuilding* building = static_cast<cBuilding*> (&unit);

				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				building->executeSelfDestroyCommand(*this);
				return;
			}
			++nr;
		}

		// rearm:
		if (unit.data.canRearm && unit.data.storageResCur >= 1)
		{
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				toggleMouseInputMode (muniActive);
				return;
			}
			++nr;
		}

		// repair:
		if (unit.data.canRepair && unit.data.storageResCur >= 1)
		{
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				toggleMouseInputMode (repairActive);
				return;
			}
			++nr;
		}

		// lay mines:
		if (unit.data.canPlaceMines && unit.data.storageResCur > 0)
		{
			if (exeNr == nr)
			{
				cVehicle* vehicle = static_cast<cVehicle*> (&unit);
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				vehicle->executeLayMinesCommand (*client);
				return;
			}
			++nr;
		}

		// clear mines:
		if (unit.data.canPlaceMines && unit.data.storageResCur < unit.data.storageResMax)
		{
			if (exeNr == nr)
			{
				cVehicle* vehicle = static_cast<cVehicle*> (&unit);
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				vehicle->executeClearMinesCommand (*client);
				return;
			}
			++nr;
		}

		// disable:
		if (unit.data.canDisable && unit.data.shotsCur > 0)
		{
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				toggleMouseInputMode (disableMode);
				return;
			}
			++nr;
		}

		// steal:
		if (unit.data.canCapture && unit.data.shotsCur > 0)
		{
			if (exeNr == nr)
			{
				unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				toggleMouseInputMode (stealMode);
				return;
			}
			++nr;
		}
	}
	// help/info:
	if (exeNr == nr)
	{
		unitMenuActive = false;
		PlayFX (SoundData.SNDObjectMenu);
		cUnitHelpMenu helpMenu(&unit.data, unit.owner);
		switchTo(helpMenu, client);
		return;
	}
	++nr;

	// done:
	if (exeNr == nr)
	{
		unitMenuActive = false;
		PlayFX (SoundData.SNDObjectMenu);
		if (unit.owner == &client->getActivePlayer())
		{
			unit.isMarkedAsDone = true;
			sendMoveJobResume (*client, unit.iID);
		}
		return;
	}
}

//------------------------------------------------------------------------------
/** Returns the screen x position of the unit */
//------------------------------------------------------------------------------
int cGameGUI::getScreenPosX (const cUnit& unit, bool movementOffset) const
{
	const int offset = movementOffset ? unit.getMovementOffsetX() : 0;
	return 180 - ((int) ((getOffsetX() - offset) * getZoom())) + getTileSize() * unit.PosX;
}

//------------------------------------------------------------------------------
/** Returns the screen y position of the unit */
//------------------------------------------------------------------------------
int cGameGUI::getScreenPosY (const cUnit& unit, bool movementOffset) const
{
	const int offset = movementOffset ? unit.getMovementOffsetY() : 0;
	return 18 - ((int) ((getOffsetY() - offset) * getZoom())) + getTileSize() * unit.PosY;
}

//------------------------------------------------------------------------------
/** Centers on this unit */
//------------------------------------------------------------------------------
void cGameGUI::center (const cUnit& unit)
{
	const int offX = unit.PosX * 64 - ((int) (((float) (Video.getResolutionX() - 192) / (2 * getTileSize())) * 64)) + 32;
	const int offY = unit.PosY * 64 - ((int) (((float) (Video.getResolutionY() - 32)  / (2 * getTileSize())) * 64)) + 32;
	setOffsetPosition (offX, offY);
}

//------------------------------------------------------------------------------
/** Draws the ammunition bar over the unit */
//------------------------------------------------------------------------------
void cGameGUI::drawMunBar (const cUnit& unit, const SDL_Rect& screenPos) const
{
	if (unit.owner != &getClient()->getActivePlayer())
		return;

	SDL_Rect r1;
	r1.x = screenPos.x + getTileSize() / 10 + 1;
	r1.y = screenPos.y + getTileSize() / 10 + getTileSize() / 8;
	r1.w = getTileSize() * 8 / 10;
	r1.h = getTileSize() / 8;

	if (r1.h <= 2)
	{
		r1.y += 1;
		r1.h = 3;
	}

	SDL_Rect r2;
	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.w = (int) (((float) (r1.w - 2) / unit.data.ammoMax) * unit.data.ammoCur);
	r2.h = r1.h - 2;

	SDL_FillRect (cVideo::buffer, &r1, 0xFF000000);

	if (unit.data.ammoCur > unit.data.ammoMax / 2)
		SDL_FillRect (cVideo::buffer, &r2, 0xFF04AE04);
	else if (unit.data.ammoCur > unit.data.ammoMax / 4)
		SDL_FillRect (cVideo::buffer, &r2, 0xFFDBDE00);
	else
		SDL_FillRect (cVideo::buffer, &r2, 0xFFE60000);
}

//------------------------------------------------------------------------------
/** draws the health bar over the unit */
//------------------------------------------------------------------------------
void cGameGUI::drawHealthBar (const cUnit& unit, const SDL_Rect& screenPos) const
{
	SDL_Rect r1;
	r1.x = screenPos.x + getTileSize() / 10 + 1;
	r1.y = screenPos.y + getTileSize() / 10;
	r1.w = getTileSize() * 8 / 10;
	r1.h = getTileSize() / 8;

	if (unit.data.isBig)
	{
		r1.w += getTileSize();
		r1.h *= 2;
	}

	if (r1.h <= 2)
		r1.h = 3;

	SDL_Rect r2;
	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.w = (int) (((float) (r1.w - 2) / unit.data.hitpointsMax) * unit.data.hitpointsCur);
	r2.h = r1.h - 2;

	SDL_FillRect (cVideo::buffer, &r1, 0xFF000000);

	Uint32 color;
	if (unit.data.hitpointsCur > unit.data.hitpointsMax / 2)
		color = 0xFF04AE04; // green
	else if (unit.data.hitpointsCur > unit.data.hitpointsMax / 4)
		color = 0xFFDBDE00; // orange
	else
		color = 0xFFE60000; // red
	SDL_FillRect (cVideo::buffer, &r2, color);
}

//------------------------------------------------------------------------------
void cGameGUI::drawStatus (const cUnit& unit, const SDL_Rect& screenPos) const
{
	SDL_Rect speedSymbol = {244, 97, 8, 10};
	SDL_Rect shotsSymbol = {254, 97, 5, 10};
	SDL_Rect disabledSymbol = {150, 109, 25, 25};
	SDL_Rect dest;

	if (unit.isDisabled())
	{
		if (getTileSize() < 25)
			return;
		dest.x = screenPos.x + getTileSize() / 2 - 12;
		dest.y = screenPos.y + getTileSize() / 2 - 12;
		SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &disabledSymbol, cVideo::buffer, &dest);
	}
	else
	{
		dest.y = screenPos.y + getTileSize() - 11;
		dest.x = screenPos.x + getTileSize() / 2 - 4;
		if (unit.data.isBig)
		{
			dest.y += (getTileSize() / 2);
			dest.x += (getTileSize() / 2);
		}
		if (unit.data.speedCur >= 4)
		{
			if (unit.data.shotsCur)
				dest.x -= getTileSize() / 4;

			SDL_Rect destCopy = dest;
			SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &speedSymbol, cVideo::buffer, &destCopy);
		}

		dest.x = screenPos.x + getTileSize() / 2 - 4;
		if (unit.data.shotsCur)
		{
			if (unit.data.speedCur)
				dest.x += getTileSize() / 4;
			SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &shotsSymbol, cVideo::buffer, &dest);
		}
	}
}

cPosition cGameGUI::getTilePosition(const cPosition& screenPosition) const
{
	const cMap& map = *getClient()->getMap();

	cPosition tilePosition;

	if(screenPosition.x() < 180 || screenPosition.x() > 180 + (Video.getResolutionX() - 192))
	{
		tilePosition.x() = -1;
	}
	else
	{
		int X = (int)((screenPosition.x() - 180 + getOffsetX() * getZoom()) / getTileSize());
		tilePosition.x() = std::min(X, map.getSize() - 1);
	}

	if(screenPosition.y() < 18 || screenPosition.y() > 18 + (Video.getResolutionY() - 32))
	{
		tilePosition.y() = -1;
	}
	else
	{
		int Y = (int)((screenPosition.y() - 18 + getOffsetY() * getZoom()) / getTileSize());
		tilePosition.y() = std::min(Y, map.getSize() - 1);
	}

	return tilePosition;
}