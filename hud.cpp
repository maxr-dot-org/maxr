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
#include <math.h>
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
#include "input.h"
#include "keys.h"
#include "main.h"
#include "mouse.h"
#include "netmessage.h"
#include "pcx.h"
#include "player.h"
#include "settings.h"
#include "sound.h"
#include "unifonts.h"
#include "vehicles.h"
#include "video.h"

using namespace std;

#if defined (_MSC_VER)
// The purpose of this warning is to avoid to use not fully constructed object.
// As we use 'this' only to save back reference
// and use it later once object is fully initialized
// just ignore the warning.
# pragma warning (disable:4355) // 'this': used in base member initializer list
// appear in cGameGUI constructor
#endif

sMouseBox::sMouseBox() :
	startX (-1),
	startY (-1),
	endX (-1),
	endY (-1)
{}

bool sMouseBox::isTooSmall() const
{
	if (!isValid()) return true;
	return fabsf (endX - startX) <= 0.5f || fabsf (endY - startY) <= 0.5f;
}

bool sMouseBox::isValid() const
{
	return startX != -1.f && startY != -1.f && endX != -1.f && endY != -1.f;
}

void sMouseBox::invalidate()
{
	startX = -1.f;
	startY = -1.f;
	endX = -1.f;
	endY = -1.f;
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
#define DEBUGOUT_X_POS (Video.getResolutionX() - 200)

	int debugOff = 30;

	const cGameGUI& gui = client->gameGUI;
	const cPlayer* player = client->gameGUI.player;

	if (debugPlayers)
	{
		font->showText (DEBUGOUT_X_POS, debugOff, "Players: " + iToStr ( (int) client->PlayerList->size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		SDL_Rect rDest = { Sint16 (DEBUGOUT_X_POS), Sint16 (debugOff), 20, 10 };
		SDL_Rect rSrc = { 0, 0, 20, 10 };
		SDL_Rect rDotDest = { Sint16 (DEBUGOUT_X_POS - 10), Sint16 (debugOff), 10, 10 };
		SDL_Rect rBlackOut = { Sint16 (DEBUGOUT_X_POS + 20), Sint16 (debugOff), 0, 10 };
		const std::vector<cPlayer*>& playerList = *client->PlayerList;
		for (unsigned int i = 0; i < playerList.size(); i++)
		{
			//HACK SHOWFINISHEDPLAYERS
			SDL_Rect rDot = { 10, 0, 10, 10 }; //for green dot

			if (playerList[i]->bFinishedTurn /* && playerList[i] != player*/)
			{
				SDL_BlitSurface (GraphicsData.gfx_player_ready, &rDot, buffer, &rDotDest);
			}
#if 0
			else if (playerList[i] == player && client->bWantToEnd)
			{
				SDL_BlitSurface (GraphicsData.gfx_player_ready, &rDot, buffer, &rDotDest);
			}
#endif
			else
			{
				rDot.x = 0; //for red dot
				SDL_BlitSurface (GraphicsData.gfx_player_ready, &rDot, buffer, &rDotDest);
			}

			SDL_BlitSurface (playerList[i]->getColorSurface(), &rSrc, buffer, &rDest);
			if (playerList[i] == player)
			{
				string sTmpLine = " " + playerList[i]->getName() + ", nr: " + iToStr (playerList[i]->getNr()) + " << you! ";
				rBlackOut.w = font->getTextWide (sTmpLine, FONT_LATIN_SMALL_WHITE);  //black out background for better recognizing
				SDL_FillRect (buffer, &rBlackOut, 0x000000);
				font->showText (rBlackOut.x, debugOff + 1, sTmpLine, FONT_LATIN_SMALL_WHITE);
			}
			else
			{
				string sTmpLine = " " + playerList[i]->getName() + ", nr: " + iToStr (playerList[i]->getNr()) + " ";
				rBlackOut.w = font->getTextWide (sTmpLine, FONT_LATIN_SMALL_WHITE);  //black out background for better recognizing
				SDL_FillRect (buffer, &rBlackOut, 0x000000);
				font->showText (rBlackOut.x, debugOff + 1, sTmpLine, FONT_LATIN_SMALL_WHITE);
			}
			debugOff += 10; //use 10 for pixel high of dots instead of text high
			rDest.y = rDotDest.y = rBlackOut.y = debugOff;
		}
	}

	if (debugAjobs)
	{
		font->showText (DEBUGOUT_X_POS, debugOff, "ClientAttackJobs: " + iToStr ( (int) client->attackJobs.size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		if (server)
		{
			font->showText (DEBUGOUT_X_POS, debugOff, "ServerAttackJobs: " + iToStr ( (int) server->AJobs.size()), FONT_LATIN_SMALL_WHITE);
			debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		}
	}

	if (debugBaseClient)
	{
		font->showText (DEBUGOUT_X_POS, debugOff, "subbases: " + iToStr ( (int) player->base.SubBases.size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
	}

	if (debugBaseServer)
	{
		cPlayer* serverPlayer = server->getPlayerFromNumber (player->getNr());
		font->showText (DEBUGOUT_X_POS, debugOff, "subbases: " + iToStr ( (int) serverPlayer->base.SubBases.size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
	}

	if (debugFX)
	{
#if 0
		font->showText (DEBUGOUT_X_POS, debugOff, "fx-count: " + iToStr ( (int) FXList.size() + (int) FXListBottom.size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		font->showText (DEBUGOUT_X_POS, debugOff, "wind-dir: " + iToStr ( (int) (fWindDir * 57.29577)), FONT_LATIN_SMALL_WHITE);
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

			for (unsigned int i = 0; i < server->PlayerList->size(); i++)
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
	cMapField* field;

	int x = mouse->getKachelX (client->gameGUI);
	int y = mouse->getKachelY (client->gameGUI);
	if (x < 0 || y < 0) return;

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

	tmpString = "dir: " + iToStr (vehicle.dir) + " moving: +" + iToStr (vehicle.moving) + " mjob: "  + pToStr (vehicle.ClientMoveJob) + " speed: " + iToStr (vehicle.data.speedCur) + " mj_active: " + iToStr (vehicle.MoveJobActive);
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

	tmpString = "commando_rank: " + dToStr (Round (vehicle.CommandoRank, 2)) + " disabled: " + iToStr (vehicle.turnsDisabled);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "is_locked: " + iToStr (vehicle.isLocked()) + " clear_mines: +" + iToStr (vehicle.ClearMines) + " lay_mines: " + iToStr (vehicle.LayMines);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString =
		" vehicle_to_activate: +"  + iToStr (vehicle.VehicleToActivate) +
		" stored_vehicles_count: " + iToStr ( (int) vehicle.storedUnits.size());
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	if (vehicle.storedUnits.size())
	{
		for (unsigned int i = 0; i < vehicle.storedUnits.size(); i++)
		{
			const cVehicle* storedVehicle = vehicle.storedUnits[i];
			font->showText (x, *y, " store " + iToStr (i) + ": \"" + storedVehicle->getDisplayName() + "\"", FONT_LATIN_SMALL_WHITE);
			*y += 8;
		}
	}

	if (debugTraceServer)
	{
		tmpString = "seen by players: owner";
		for (unsigned int i = 0; i < vehicle.seenByPlayerList.size(); i++)
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

	tmpString = " max_metal_p: " + iToStr (building.MaxMetalProd) + " max_oil_p: " + iToStr (building.MaxOilProd) + " max_gold_p: " + iToStr (building.MaxGoldProd);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "is_locked: " + iToStr (building.isLocked()) + " disabled: " + iToStr (building.turnsDisabled) + " vehicle_to_activate: " + iToStr (building.VehicleToActivate);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = " stored_vehicles_count: " + iToStr ( (int) building.storedUnits.size());
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	if (building.storedUnits.size())
	{
		for (unsigned int i = 0; i < building.storedUnits.size(); i++)
		{
			const cVehicle* storedVehicle = building.storedUnits[i];
			font->showText (x, *y, " store " + iToStr (i) + ": \"" + storedVehicle->getDisplayName() + "\"", FONT_LATIN_SMALL_WHITE);
			*y += 8;
		}
	}

	tmpString =
		"build_speed: "        + iToStr (building.BuildSpeed)  +
		" repeat_build: "      + iToStr (building.RepeatBuild) +
		" build_list_count: +" + iToStr (building.BuildList ? (int) building.BuildList->size() : 0);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	if (building.BuildList && building.BuildList->size())
	{
		const sBuildList* BuildingList;
		for (unsigned int i = 0; i < building.BuildList->size(); i++)
		{
			BuildingList = (*building.BuildList) [i];
			font->showText (x, *y, "  build " + iToStr (i) + ": " + BuildingList->type.getText() + " \"" + BuildingList->type.getVehicle()->data.name + "\"", FONT_LATIN_SMALL_WHITE);
			*y += 8;
		}
	}

	if (debugTraceServer)
	{
		tmpString = "seen by players: owner";
		for (unsigned int i = 0; i < building.seenByPlayerList.size(); i++)
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


cGameGUI::cGameGUI (cMap* map_) :
	cMenu (generateSurface()),
	client (NULL),
	iObjectStream (-1),
	msgCoordsX (-1),
	msgCoordsY (-1),
	player (NULL),
	map (map_),
	miniMapOffX (0),
	miniMapOffY (0),
	shiftPressed (false),
	overUnitField (NULL),
	zoomSlider (20, 274, calcMinZoom(), 1.0f, this, 130, cMenuSlider::SLIDER_TYPE_HUD_ZOOM, cMenuSlider::SLIDER_DIR_RIGHTMIN),
	endButton (391, 4, lngPack.i18n ("Text~Hud~End"), cMenuButton::BUTTON_TYPE_HUD_END, FONT_LATIN_NORMAL),
	preferencesButton (86, 4, lngPack.i18n ("Text~Hud~Settings"), cMenuButton::BUTTON_TYPE_HUD_PREFERENCES, FONT_LATIN_SMALL_WHITE),
	filesButton (17, 3, lngPack.i18n ("Text~Hud~Files"), cMenuButton::BUTTON_TYPE_HUD_FILES, FONT_LATIN_SMALL_WHITE),
	playButton (146, 123, "", cMenuButton::BUTTON_TYPE_HUD_PLAY),
	stopButton (146, 143, "", cMenuButton::BUTTON_TYPE_HUD_STOP),
	FLCImage (10, 29, NULL),
	unitDetails (8, 171, false, player),
	surveyButton (2, 296, lngPack.i18n ("Text~Hud~Survey"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_00, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	hitsButton (57, 296, lngPack.i18n ("Text~Hud~Hitpoints"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_01, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	scanButton (112, 296, lngPack.i18n ("Text~Hud~Scan"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_02, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	statusButton (2, 296 + 18, lngPack.i18n ("Text~Hud~Status"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_10, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	ammoButton (57, 296 + 18, lngPack.i18n ("Text~Hud~Ammo"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_11, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	gridButton (112, 296 + 18, lngPack.i18n ("Text~Hud~Grid"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_12, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	colorButton (2, 296 + 18 + 16, lngPack.i18n ("Text~Hud~Color"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_20, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	rangeButton (57, 296 + 18 + 16, lngPack.i18n ("Text~Hud~Range"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_21, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	fogButton (112, 296 + 18 + 16, lngPack.i18n ("Text~Hud~Fog"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_22, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	lockButton (32, 227, "", false, false, cMenuCheckButton::CHECKBOX_HUD_LOCK, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	TNTButton (136, 413, "", false, false, cMenuCheckButton::CHECKBOX_HUD_TNT, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	twoXButton (136, 387, "", false, false, cMenuCheckButton::CHECKBOX_HUD_2X, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	playersButton (136, 439, "", false, false, cMenuCheckButton::CHECKBOX_HUD_PLAYERS, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch),
	helpButton (20, 250, "", cMenuButton::BUTTON_TYPE_HUD_HELP),
	centerButton (4, 227, "", cMenuButton::BUTTON_TYPE_HUD_CENTER),
	reportsButton (101, 252, lngPack.i18n ("Text~Hud~Log"), cMenuButton::BUTTON_TYPE_HUD_REPORT, FONT_LATIN_SMALL_WHITE),
	chatButton (51, 252, lngPack.i18n ("Text~Hud~Chat"), cMenuButton::BUTTON_TYPE_HUD_CHAT, FONT_LATIN_SMALL_WHITE),
	nextButton (124, 227, ">>", cMenuButton::BUTTON_TYPE_HUD_NEXT, FONT_LATIN_SMALL_WHITE),
	prevButton (60, 227, "<<", cMenuButton::BUTTON_TYPE_HUD_PREV, FONT_LATIN_SMALL_WHITE),
	doneButton (99, 227, lngPack.i18n ("Text~Hud~Proceed"), cMenuButton::BUTTON_TYPE_HUD_DONE, FONT_LATIN_SMALL_WHITE),
	miniMapImage (MINIMAP_POS_X, MINIMAP_POS_Y, generateMiniMapSurface()),
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
	iTimerTime (0)
{
	dCache.setGameGUI (*this);
	unitMenuActive = false;
	frame = 0;
	zoom = 1.0f;
	offX = offY = 0;
	framesPerSecond = cyclesPerSecond = 0;
	loadValue = 0;
	panelTopGraphic = NULL, panelBottomGraphic = NULL;
	activeItem = NULL;
	helpActive = false;
	showPlayers = false;
	blinkColor = 0xFFFFFF;
	FLC = NULL;
	playFLC = true;
	mouseInputMode = normalInput;
	TimerID = SDL_AddTimer (50, TimerCallback, this);

	selectedUnit = NULL;

	calcMinZoom();

	setWind (random (360));

	zoomSlider.setMoveCallback (&zoomSliderMoved);
	menuItems.push_back (&zoomSlider);
	menuItems.push_back (zoomSlider.scroller);

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

	updateTurn (1);
}

void cGameGUI::setClient (cClient* client)
{
	this->client = client;
	debugOutput.setClient (client);
	debugOutput.setServer (client->getServer());

	for (size_t i = 0; i < client->getPlayerList().size(); i++)
	{
		cPlayer& p = *client->getPlayerList() [i];
		const int xPos = Video.getResolutionY() >= 768 ? 3 : 161;
		const int yPos = Video.getResolutionY() >= 768 ? (482 + GraphicsData.gfx_hud_extra_players->h * i) : (480 - 82 - GraphicsData.gfx_hud_extra_players->h * i);

		cMenuPlayerInfo* playerInfo = new cMenuPlayerInfo (xPos, yPos, p);
		playerInfo->setDisabled (true);
		playersInfo.push_back (playerInfo);

		menuItems.push_back (playerInfo);
	}
}

float cGameGUI::calcMinZoom()
{
	minZoom = (float) ( (max (Video.getResolutionY() - HUD_TOTAL_HIGHT, Video.getResolutionX() - HUD_TOTAL_WIDTH) / (float) map->getSize()) / 64.0);
	minZoom = max (minZoom, ( (int) (64.0 * minZoom) + (minZoom >= 1.0 ? 0 : 1)) / 64.0f);

	return minZoom;
}

void cGameGUI::recalcPosition (bool resetItemPositions)
{
	background = generateSurface();
	cMenu::recalcPosition (resetItemPositions);

	// reset minimal zoom
	calcMinZoom();
	setZoom (zoom, true, false);
	zoomSlider.setBorders (minZoom, 1.0f);

	// move some items around
	coordsLabel.move (coordsLabel.getPosition().x, (Video.getResolutionY() - 21) + 3);
	unitNameLabel.move (unitNameLabel.getPosition().x, (Video.getResolutionY() - 21) + 3);
	chatBox.move (chatBox.getPosition().x, Video.getResolutionY() - 48);
	infoTextLabel.move (HUD_LEFT_WIDTH + (Video.getResolutionX() - HUD_TOTAL_WIDTH) / 2, infoTextLabel.getPosition().y);
	infoTextAdditionalLabel.move (HUD_LEFT_WIDTH + (Video.getResolutionX() - HUD_TOTAL_WIDTH) / 2, 235 + font->getFontHeight (FONT_LATIN_BIG));
}

cGameGUI::~cGameGUI()
{
	SDL_RemoveTimer (TimerID);

	if (FLC) FLI_Close (FLC);
	for (size_t i = 0; i != playersInfo.size(); ++i)
	{
		delete playersInfo[i];
	}
	for (size_t i = 0; i != messages.size(); ++i)
	{
		delete messages[i];
	}
	for (size_t i = 0; i != FxList.size(); ++i)
	{
		delete FxList[i];
	}
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
	if (iTimerTime != iLast)
	{
		iLast = iTimerTime;
		timer50ms = true;
		if (iTimerTime & 0x1) timer100ms = true;
		if ( (iTimerTime & 0x3) == 3) timer400ms = true;
	}
}

void cGameGUI::handleMessages()
{
	if (messages.size() == 0) return;
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
		iHeight += 17 + font->getFontHeight() * (message->len  / (Video.getResolutionX() - 300));
	}
}

void cGameGUI::addMessage (const string& sMsg)
{
	sMessage* const Message = new sMessage (sMsg, SDL_GetTicks());
	messages.push_back (Message);
	if (cSettings::getInstance().isDebug()) Log.write (Message->msg, cLog::eLOG_TYPE_DEBUG);
}

string cGameGUI::addCoords (const string& msg, int x, int y)
{
	stringstream strStream;
	//e.g. [85,22] missel MK I is under attack (F1)
	strStream << "[" << x << "," << y << "] " << msg << " (" << GetKeyString (KeysList.KeyJumpToAction) << ")";
	addMessage (strStream.str());
	msgCoordsX = x;
	msgCoordsY = y;
	return strStream.str();
}

int cGameGUI::show (cClient* client)
{
	drawnEveryFrame = true;

	// do startup actions
	makePanel (true);
	startup = true;
	if (client->isFreezed ()) setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_Until", client->getPlayerFromNumber (0)->getName()), "");

	int lastMouseX = 0, lastMouseY = 0;

	while (!end && !terminate)
	{
		cEventHandling::handleInputEvents (*this, client);
		client->gameTimer.run (this);

		mouse->GetPos();
		if (mouse->moved())
		{
			handleMouseMove();

			for (unsigned int i = 0; i < menuItems.size(); i++)
			{
				cMenuItem* menuItem = menuItems[i];
				if (menuItem->overItem (lastMouseX, lastMouseY) && !menuItem->overItem (mouse->x, mouse->y)) menuItem->hoveredAway (this);
				else if (!menuItem->overItem (lastMouseX, lastMouseY) && menuItem->overItem (mouse->x, mouse->y)) menuItem->hoveredOn (this);
				else if (menuItem->overItem (lastMouseX, lastMouseY) && menuItem->overItem (mouse->x, mouse->y)) menuItem->movedMouseOver (lastMouseX, lastMouseY, this);
				else if (menuItem == activeItem) menuItem->somewhereMoved();
			}
		}

		lastMouseX = mouse->x;
		lastMouseY = mouse->y;

		if (!cSettings::getInstance().shouldUseFastMode()) SDL_Delay (1);

		if (startup)
		{
			if (player->BuildingList) player->BuildingList->center (*this);
			else if (player->VehicleList) player->VehicleList->center (*this);
			startup = false;
		}

		handleMessages();

		handleTimer();
		if (timer10ms)
		{
			//run effects, which are not synchronous to game time
			runFx ();
			client->handleTurnTime(); //TODO: remove
		}

		checkScroll();
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

	// code to work with DEDICATED_SERVER - network client sends the server, that it disconnects itself
	if (client->getServer() == 0)
	{
		cNetMessage* message = new cNetMessage (GAME_EV_WANT_DISCONNECT);
		client->sendNetMessage (message);
	}
	// end

	makePanel (false);

	cEventHandling::handleInputEvents (*this, client); //flush event queue before exiting menu

	if (end) return 0;
	assert (terminate);
	return 1;
}

void cGameGUI::updateInfoTexts ()
{
	int playerNumber = client->getFreezeInfoPlayerNumber ();
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

SDL_Surface* cGameGUI::generateSurface()
{
	SDL_Surface* surface = SDL_CreateRGBSurface (SDL_HWSURFACE, Video.getResolutionX(), Video.getResolutionY(), Video.getColDepth(), 0, 0, 0, 0);

	SDL_FillRect (surface, NULL, 0xFF00FF);
	SDL_SetColorKey (surface, SDL_SRCCOLORKEY, 0xFF00FF);

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
		AutoSurface tmpSurface (LoadPCX (cSettings::getInstance().getGfxPath() + PATH_DELIMITER + "logo.pcx"));
		if (tmpSurface)
		{
			dest.x = 9;
			dest.y = Video.getResolutionY() - HUD_TOTAL_HIGHT - 15;
			SDL_BlitSurface (tmpSurface, NULL, surface, &dest);
		}
	}
	return surface;
}

SDL_Surface* cGameGUI::generateMiniMapSurface()
{
	SDL_Surface* minimapSurface = SDL_CreateRGBSurface (SDL_SWSURFACE, MINIMAP_SIZE, MINIMAP_SIZE, 32, 0, 0, 0, 0);
	Uint32* minimap = static_cast<Uint32*> (minimapSurface->pixels);

	//set zoom factor
	const int displayedMapWidth = (int) ( (Video.getResolutionX() - HUD_TOTAL_WIDTH) / getZoom());
	const int displayedMapHight = (int) ( (Video.getResolutionY() - HUD_TOTAL_HIGHT) / getZoom());
	const int zoomFactor = twoXChecked() ? MINIMAP_ZOOM_FACTOR : 1;

	if (zoomFactor != 1)
	{
		if (offX < miniMapOffX * 64) miniMapOffX -= cSettings::getInstance().getScrollSpeed() / 10;
		else if (offX + displayedMapWidth > miniMapOffX * 64 + (MINIMAP_SIZE * 64) / MINIMAP_ZOOM_FACTOR) miniMapOffX += cSettings::getInstance().getScrollSpeed() / 10;

		if (offY < miniMapOffY * 64) miniMapOffY -= cSettings::getInstance().getScrollSpeed() / 10;
		else if (offY + displayedMapHight > miniMapOffY * 64 + (MINIMAP_SIZE * 64) / MINIMAP_ZOOM_FACTOR) miniMapOffY += cSettings::getInstance().getScrollSpeed() / 10;

		miniMapOffX = std::max (miniMapOffX, 0);
		miniMapOffY = std::max (miniMapOffY, 0);
		miniMapOffX = std::min (map->getSize() - (map->getSize() / zoomFactor), miniMapOffX);
		miniMapOffY = std::min (map->getSize() - (map->getSize() / zoomFactor), miniMapOffY);
	}

	//draw the landscape
	for (int miniMapX = 0; miniMapX < MINIMAP_SIZE; miniMapX++)
	{
		//calculate the field on the map
		int terrainx = (miniMapX * map->getSize()) / (MINIMAP_SIZE * zoomFactor) + miniMapOffX;
		terrainx = std::min (terrainx, map->getSize() - 1);

		//calculate the position within the terrain graphic (for better rendering of maps < 112)
		const int offsetx  = ( (miniMapX * map->getSize()) % (MINIMAP_SIZE * zoomFactor)) * 64 / (MINIMAP_SIZE * zoomFactor);

		for (int miniMapY = 0; miniMapY < MINIMAP_SIZE; miniMapY++)
		{
			int terrainy = (miniMapY * map->getSize()) / (MINIMAP_SIZE * zoomFactor) + miniMapOffY;
			terrainy = std::min (terrainy, map->getSize() - 1);
			const int offsety  = ( (miniMapY * map->getSize()) % (MINIMAP_SIZE * zoomFactor)) * 64 / (MINIMAP_SIZE * zoomFactor);

			SDL_Color sdlcolor;
			const sTerrain& terrain = map->staticMap->getTerrain (terrainx, terrainy);
			const Uint8* terrainPixels = reinterpret_cast<const Uint8*> (terrain.sf_org->pixels);
			Uint8 index = terrainPixels[offsetx + offsety * 64];
			sdlcolor = terrain.sf_org->format->palette->colors[index];
			Uint32 color = (sdlcolor.r << 16) + (sdlcolor.g << 8) + sdlcolor.b;

			minimap[miniMapX + miniMapY * MINIMAP_SIZE] = color;
		}
	}

	if (player)
	{
		//draw the fog
		for (int miniMapX = 0; miniMapX < MINIMAP_SIZE; miniMapX++)
		{
			int terrainx = (miniMapX * map->getSize()) / (MINIMAP_SIZE * zoomFactor) + miniMapOffX;
			for (int miniMapY = 0; miniMapY < MINIMAP_SIZE; miniMapY++)
			{
				int terrainy = (miniMapY * map->getSize()) / (MINIMAP_SIZE * zoomFactor) + miniMapOffY;

				if (!player->ScanMap[map->getOffset (terrainx, terrainy)])
				{
					Uint8* color = reinterpret_cast<Uint8*> (&minimap[miniMapX + miniMapY * MINIMAP_SIZE]);
					color[0] = (Uint8) (color[0] * 0.6f);
					color[1] = (Uint8) (color[1] * 0.6f);
					color[2] = (Uint8) (color[2] * 0.6f);
				}
			}
		}

		//draw the units
		//here we go through each map field instead of through each minimap pixel,
		//to make sure, that every unit is diplayed and has the same size on the minimap.

		//the size of the rect, that is drawn for each unit
		int size = (int) ceilf ( (float) MINIMAP_SIZE * zoomFactor / map->getSize());
		size = std::max (size, 2);
		SDL_Rect rect;
		rect.h = size;
		rect.w = size;

		for (int mapx = 0; mapx < map->getSize(); mapx++)
		{
			rect.x = ( (mapx - miniMapOffX) * MINIMAP_SIZE * zoomFactor) / map->getSize();
			if (rect.x < 0 || rect.x >= MINIMAP_SIZE) continue;
			for (int mapy = 0; mapy < map->getSize(); mapy++)
			{
				rect.y = ( (mapy - miniMapOffY) * MINIMAP_SIZE * zoomFactor) / map->getSize();
				if (rect.y < 0 || rect.y >= MINIMAP_SIZE) continue;

				if (!player->ScanMap[map->getOffset (mapx, mapy)]) continue;

				cMapField& field = (*map) [map->getOffset (mapx, mapy)];

				//draw building
				const cBuilding* building = field.getBuilding();
				if (building && building->owner)
				{
					if (!tntChecked() || building->data.canAttack)
					{
						unsigned int color = *static_cast<Uint32*> (building->owner->getColorSurface()->pixels);
						SDL_FillRect (minimapSurface, &rect, color);
					}
				}

				//draw vehicle
				const cVehicle* vehicle = field.getVehicle();
				if (vehicle)
				{
					if (!tntChecked() || vehicle->data.canAttack)
					{
						unsigned int color = *static_cast<Uint32*> (vehicle->owner->getColorSurface()->pixels);
						SDL_FillRect (minimapSurface, &rect, color);
					}
				}

				//draw plane
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

	//draw the screen borders
	int startx = (int) ( ( ( (offX / 64.0) - miniMapOffX) * MINIMAP_SIZE * zoomFactor) / map->getSize());
	int starty = (int) ( ( ( (offY / 64.0) - miniMapOffY) * MINIMAP_SIZE * zoomFactor) / map->getSize());
	int endx = (int) (startx + ( (Video.getResolutionX() - HUD_TOTAL_WIDTH) * MINIMAP_SIZE * zoomFactor) / (map->getSize() * (getZoom() * 64.0)));
	int endy = (int) (starty + ( (Video.getResolutionY() - HUD_TOTAL_HIGHT) * MINIMAP_SIZE * zoomFactor) / (map->getSize() * (getZoom() * 64.0)));

	if (endx == MINIMAP_SIZE) endx = MINIMAP_SIZE - 1;   //workaround
	if (endy == MINIMAP_SIZE) endy = MINIMAP_SIZE - 1;

	for (int y = starty; y <= endy; y++)
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
	for (int x = startx; x <= endx; x++)
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

	const int maxX = map->getSize() * 64 - (int) ( (Video.getResolutionX() - HUD_TOTAL_WIDTH) / getZoom());
	const int maxY = map->getSize() * 64 - (int) ( (Video.getResolutionY() - HUD_TOTAL_HIGHT) / getZoom());
	offX = min (offX, maxX);
	offY = min (offY, maxY);

	callMiniMapDraw();
	updateMouseCursor();
}

void cGameGUI::setZoom (float newZoom, bool setScroller, bool centerToMouse)
{
	zoom = newZoom;
	zoom = std::max (zoom, minZoom);
	zoom = std::min (zoom, 1.f);

	if (setScroller) this->zoomSlider.setValue (zoom);

	static float lastZoom = 1.f;
	if (lastZoom != getZoom())
	{
		//change x screen offset
		int off;

		float lastScreenPixel = (Video.getResolutionX() - HUD_TOTAL_WIDTH) / lastZoom;
		float newScreenPixel  = (Video.getResolutionX() - HUD_TOTAL_WIDTH) / getZoom();
		if (centerToMouse)
		{
			off = (int) ( (lastScreenPixel - newScreenPixel) * (mouse->x - HUD_LEFT_WIDTH) / (Video.getResolutionX() - HUD_TOTAL_WIDTH));
		}
		else
		{
			off = (int) (lastScreenPixel - newScreenPixel) / 2;
		}

		offX += off;

		//change y screen offset
		lastScreenPixel = (Video.getResolutionY() - HUD_TOTAL_HIGHT) / lastZoom;
		newScreenPixel  = (Video.getResolutionY() - HUD_TOTAL_HIGHT) / getZoom();
		if (centerToMouse)
		{
			off = (int) ( (lastScreenPixel - newScreenPixel) * (mouse->y - HUD_TOP_HIGHT) / (Video.getResolutionY() - HUD_TOTAL_HIGHT));
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

void cGameGUI::setPlayer (cPlayer* player_)
{
	player = player_;
	unitDetails.setOwner (player);
}

void cGameGUI::setUnitDetailsData (cVehicle* vehicle, cBuilding* building)
{
	unitDetails.setSelection (*client, vehicle, building);

	if (vehicle)
	{
		selUnitNamePrefixStr.setText (vehicle->getNamePrefix());
		selUnitNameEdit.setText (vehicle->isNameOriginal() ? vehicle->data.name : vehicle->getName());
	}
	else if (building)
	{
		selUnitNamePrefixStr.setText (building->getNamePrefix());
		selUnitNameEdit.setText (building->isNameOriginal() ? building->data.name : building->getName());
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
	static Uint32 inverseLoad = 0; //this is 10*(100% - load)
	static Uint32 lastTickLoad = 0;

	cycles++;
	Uint32 ticks = SDL_GetTicks();

	if (ticks != lastTickLoad) inverseLoad++;
	lastTickLoad = ticks;

	if (ticks > lastTicks + 1000)
	{
		float a = 0.5f; //low pass filter coefficient

		framesPerSecond = (1 - a) * (frame - lastFrame) * 1000 / (float) (ticks - lastTicks) + a * framesPerSecond;
		lastFrame = frame;

		cyclesPerSecond = (1 - a) * cycles * 1000 / (float) (ticks - lastTicks) + a * cyclesPerSecond;
		cycles = 0;

		loadValue = Round ( (1 - a) * (1000 - inverseLoad) + a * loadValue);
		inverseLoad = 0;

		lastTicks = ticks;
	}
}

void cGameGUI::rotateBlinkColor()
{
	static bool dec = true;
	if (dec)
	{
		blinkColor -= 0x0A0A0A;
		if (blinkColor <= 0xA0A0A0) dec = false;
	}
	else
	{
		blinkColor += 0x0A0A0A;
		if (blinkColor >= 0xFFFFFF) dec = true;
	}
}

bool cGameGUI::checkScroll()
{
	if (mouse->x <= 0 && mouse->y > 30 && mouse->y < Video.getResolutionY() - 30 - 18)
	{
		mouse->SetCursor (CPfeil4);
		doScroll (4);
		return true;
	}
	else if (mouse->x >= Video.getResolutionX() - 18 && mouse->y > 30 && mouse->y < Video.getResolutionY() - 30 - 18)
	{
		mouse->SetCursor (CPfeil6);
		doScroll (6);
		return true;
	}
	else if ( (mouse->x <= 0 && mouse->y <= 30) || (mouse->y <= 0 && mouse->x <= 30))
	{
		mouse->SetCursor (CPfeil7);
		doScroll (7);
		return true;
	}
	else if ( (mouse->x >= Video.getResolutionX() - 18 && mouse->y <= 30) || (mouse->y <= 0 && mouse->x >= Video.getResolutionX() - 30 - 18))
	{
		mouse->SetCursor (CPfeil9);
		doScroll (9);
		return true;
	}
	else if (mouse->y <= 0 && mouse->x > 30 && mouse->x < Video.getResolutionX() - 30 - 18)
	{
		mouse->SetCursor (CPfeil8);
		doScroll (8);
		return true;
	}
	else if (mouse->y >= Video.getResolutionY() - 18 && mouse->x > 30 && mouse->x < Video.getResolutionX() - 30 - 18)
	{
		mouse->SetCursor (CPfeil2);
		doScroll (2);
		return true;
	}
	else if ( (mouse->x <= 0 && mouse->y >= Video.getResolutionY() - 30 - 18) || (mouse->y >= Video.getResolutionY() - 18 && mouse->x <= 30))
	{
		mouse->SetCursor (CPfeil1);
		doScroll (1);
		return true;
	}
	else if ( (mouse->x >= Video.getResolutionX() - 18 && mouse->y >= Video.getResolutionY() - 30 - 18) || (mouse->y >= Video.getResolutionY() - 18 && mouse->x >= Video.getResolutionX() - 30 - 18))
	{
		mouse->SetCursor (CPfeil3);
		doScroll (3);
		return true;
	}
	return false;
}

void cGameGUI::updateUnderMouseObject()
{
	int x = mouse->getKachelX (*this);
	int y = mouse->getKachelY (*this);

	if (map->isValidPos (x, y) == false) return;

	// draw the coordinates:
	/* array to get map coords in scheme XXX-YYY\0 = 8 characters
	a case where I accept an array since I don't know a better
	method to format x and y easily with leading 0 -- beko */
	char str[8];
	TIXML_SNPRINTF (str, sizeof (str), "%.3d-%.3d", x, y);
	coordsLabel.setText (str);

	if (!player->ScanMap[map->getOffset (x, y)])
	{
		overUnitField = NULL;
		if (mouse->cur == GraphicsData.gfx_Cattack)
		{
			SDL_Rect r = {1, 29, 35, 3};
			SDL_FillRect (GraphicsData.gfx_Cattack, &r, 0);
		}
		return;
	}
	// check wether there is a unit under the mouse:
	overUnitField = map->fields + (map->getOffset (x, y));
	cVehicle* selectedVehicle = getSelectedVehicle();
	if (mouse->cur == GraphicsData.gfx_Csteal && selectedVehicle)
	{
		selectedVehicle->drawCommandoCursor (*this, x, y, true);
	}
	else if (mouse->cur == GraphicsData.gfx_Cdisable && selectedVehicle)
	{
		selectedVehicle->drawCommandoCursor (*this, x, y, false);
	}
	if (overUnitField->getVehicle() != NULL)
	{
		const cVehicle& vehicle = *overUnitField->getVehicle();
		// FIXME: displaying ownername to unit name may cause an overdraw on the infobox.
		// This needs either a seperate infobox or a length check in the future.
		// that goes for unitnames itself too. -- beko
		unitNameLabel.setText (vehicle.getDisplayName() + " (" + vehicle.owner->getName() + ")");
		if (mouse->cur == GraphicsData.gfx_Cattack)
		{
			drawAttackCursor (x, y);
		}
	}
	else if (overUnitField->getPlane() != NULL)
	{
		const cVehicle& plane = *overUnitField->getPlane();
		unitNameLabel.setText (plane.getDisplayName() + " (" + plane.owner->getName() + ")");
		if (mouse->cur == GraphicsData.gfx_Cattack)
		{
			drawAttackCursor (x, y);
		}
	}
	else if (overUnitField->getTopBuilding() != NULL)
	{
		const cBuilding& topBuilding = *overUnitField->getTopBuilding();
		unitNameLabel.setText (topBuilding.getDisplayName() + " (" + topBuilding.owner->getName() + ")");
		if (mouse->cur == GraphicsData.gfx_Cattack)
		{
			drawAttackCursor (x, y);
		}
	}
	else if (overUnitField->getBaseBuilding() && overUnitField->getBaseBuilding()->owner)
	{
		const cBuilding& baseBuilding = *overUnitField->getBaseBuilding();
		unitNameLabel.setText (baseBuilding.getDisplayName() + " (" + baseBuilding.owner->getName() + ")");
		if (mouse->cur == GraphicsData.gfx_Cattack)
		{
			drawAttackCursor (x, y);
		}
	}
	else
	{
		unitNameLabel.setText ("");
		if (mouse->cur == GraphicsData.gfx_Cattack)
		{
			SDL_Rect r;
			r.x = 1; r.y = 29;
			r.h = 3; r.w = 35;
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
	return static_cast<cVehicle*> (selectedUnit && selectedUnit->isVehicle() ? selectedUnit : NULL);
}

cBuilding* cGameGUI::getSelectedBuilding()
{
	return static_cast<cBuilding*> (selectedUnit && selectedUnit->isBuilding() ? selectedUnit : NULL);
}

void cGameGUI::selectUnit (cUnit& unit)
{
	cVehicle* vehicle = static_cast<cVehicle*> (unit.isVehicle() ? &unit : NULL);

	if (vehicle && vehicle->Loaded) return;

	cBuilding* building = static_cast<cBuilding*> (unit.isBuilding() ? &unit : NULL);

	deselectUnit();
	selectedUnit = &unit;
	unitMenuActive = false;
	mouseInputMode = normalInput;
	if (vehicle)
	{
		vehicle->Select (*this);
		iObjectStream = vehicle->playStream (*this);
	}
	else
	{
		building->Select (*this);
		iObjectStream = building->playStream();
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
		StopFXLoop (iObjectStream);
		iObjectStream = -1;
		setVideoSurface (NULL);
		setUnitDetailsData (NULL, NULL);

		StopFXLoop (iObjectStream);
	}
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
	updateUnderMouseObject();

	const int x = mouse->x;
	const int y = mouse->y;
	const int mouseMapX = mouse->getKachelX (*this);
	const int mouseMapY = mouse->getKachelY (*this);

	for (unsigned int i = 0; i < menuItems.size(); i++)
	{
		if (menuItems[i]->overItem (x, y) && !menuItems[i]->isDisabled())
		{
			mouse->SetCursor (CHand);
			return;
		}
	}

	cVehicle* selectedVehicle = getSelectedVehicle();
	cBuilding* selectedBuilding = getSelectedBuilding();
	if (selectedVehicle && mouseInputMode == placeBand && selectedVehicle->owner == client->getActivePlayer())
	{
		if (x >= HUD_LEFT_WIDTH)
		{
			mouse->SetCursor (CBand);
		}
		else
		{
			mouse->SetCursor (CNo);
		}
	}
	else if ( (selectedUnit && mouseInputMode == transferMode && selectedUnit->owner == client->getActivePlayer()))
	{
		if (overUnitField && selectedUnit->CanTransferTo (*this, overUnitField))
		{
			mouse->SetCursor (CTransf);
		}
		else
		{
			mouse->SetCursor (CNo);
		}
	}
	else if (!helpActive)
	{
		if (x < HUD_LEFT_WIDTH)
		{
			if (mouse->SetCursor (CHand))
			{
				overUnitField = NULL;
			}
			return;
		}

		if (selectedUnit && unitMenuActive && selectedUnit->areCoordsOverMenu (*this, x, y))
		{
			mouse->SetCursor (CHand);
		}
		else if (selectedVehicle && mouseInputMode == mouseInputAttackMode && selectedVehicle->owner == client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT)
		{
			if (! (selectedVehicle->data.muzzleType == sUnitData::MUZZLE_TYPE_TORPEDO && !client->getMap()->isWaterOrCoast (mouseMapX, mouseMapY)))
			{
				if (mouse->SetCursor (CAttack))
				{
					drawAttackCursor (mouseMapX, mouseMapY);
				}
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else if (selectedVehicle && mouseInputMode == disableMode && selectedVehicle->owner == client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT)
		{
			if (selectedVehicle->canDoCommandoAction (mouseMapX, mouseMapY, client->getMap(), false))
			{
				if (mouse->SetCursor (CDisable))
				{
					selectedVehicle->drawCommandoCursor (*this, mouseMapX, mouseMapY, false);
				}
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else if (selectedVehicle && mouseInputMode == stealMode && selectedVehicle->owner == client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT)
		{
			if (selectedVehicle->canDoCommandoAction (mouseMapX, mouseMapY, client->getMap(), true))
			{
				if (mouse->SetCursor (CSteal))
				{
					selectedVehicle->drawCommandoCursor (*this, mouseMapX, mouseMapY, true);
				}
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT && selectedVehicle->canDoCommandoAction (mouseMapX, mouseMapY, client->getMap(), false) && (!overUnitField->getVehicle() || !overUnitField->getVehicle()->turnsDisabled))
		{
			if (mouse->SetCursor (CDisable))
			{
				selectedVehicle->drawCommandoCursor (*this, mouseMapX, mouseMapY, false);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT && selectedVehicle->canDoCommandoAction (mouseMapX, mouseMapY, client->getMap(), true))
		{
			if (mouse->SetCursor (CSteal))
			{
				selectedVehicle->drawCommandoCursor (*this, mouseMapX, mouseMapY, true);
			}
		}
		else if (selectedBuilding && mouseInputMode == mouseInputAttackMode && selectedBuilding->owner == client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT)
		{
			if (selectedBuilding->isInRange (mouseMapX, mouseMapY))
			{
				if (mouse->SetCursor (CAttack))
				{
					drawAttackCursor (mouseMapX, mouseMapY);
				}
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == client->getActivePlayer() && selectedVehicle->canAttackObjectAt (mouseMapX, mouseMapY, client->getMap(), false, false))
		{
			if (mouse->SetCursor (CAttack))
			{
				drawAttackCursor (mouseMapX, mouseMapY);
			}
		}
		else if (selectedBuilding && selectedBuilding->owner == client->getActivePlayer() && selectedBuilding->canAttackObjectAt (mouseMapX, mouseMapY, client->getMap()))
		{
			if (mouse->SetCursor (CAttack))
			{
				drawAttackCursor (mouseMapX, mouseMapY);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == client->getActivePlayer() && mouseInputMode == muniActive)
		{
			if (selectedVehicle->canSupply (*client, mouseMapX, mouseMapY, SUPPLY_TYPE_REARM))
			{
				mouse->SetCursor (CMuni);
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == client->getActivePlayer() && mouseInputMode == repairActive)
		{
			if (selectedVehicle->canSupply (*client, mouseMapX, mouseMapY, SUPPLY_TYPE_REPAIR))
			{
				mouse->SetCursor (CRepair);
			}
			else
			{
				mouse->SetCursor (CNo);
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
					 !selectedVehicle                               ||
					 selectedVehicle->owner != client->getActivePlayer() ||
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
					 !selectedBuilding                               ||
					 selectedBuilding->owner != client->getActivePlayer() ||
					 (
						 (
							 !selectedBuilding->BuildList                    ||
							 !selectedBuilding->BuildList->size()            ||
							 selectedBuilding->IsWorking                     ||
							 (*selectedBuilding->BuildList) [0]->metall_remaining > 0
						 ) &&
						 mouseInputMode != loadMode &&
						 mouseInputMode != activateVehicle
					 )
				 )
				)
		{
			mouse->SetCursor (CSelect);
		}
		else if (selectedVehicle && selectedVehicle->owner == client->getActivePlayer() && mouseInputMode == loadMode)
		{
			if (selectedVehicle->canLoad (mouseMapX, mouseMapY, client->getMap(), false))
			{
				mouse->SetCursor (CLoad);
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == client->getActivePlayer() && mouseInputMode == activateVehicle)
		{
			if (selectedVehicle->canExitTo (mouseMapX, mouseMapY, client->getMap(), selectedVehicle->storedUnits[selectedVehicle->VehicleToActivate]->typ))
			{
				mouse->SetCursor (CActivate);
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < HUD_LEFT_WIDTH + (Video.getResolutionX() - HUD_TOTAL_WIDTH) && y < HUD_TOP_HIGHT + (Video.getResolutionY() - HUD_TOTAL_HIGHT))
		{
			if (!selectedVehicle->IsBuilding && !selectedVehicle->IsClearing && mouseInputMode != loadMode && mouseInputMode != activateVehicle)
			{
				if (selectedVehicle->MoveJobActive)
				{
					mouse->SetCursor (CNo);
				}
				else if (client->getMap()->possiblePlace (*selectedVehicle, mouseMapX, mouseMapY, true))
				{
					mouse->SetCursor (CMove);
				}
				else
				{
					mouse->SetCursor (CNo);
				}
			}
			else if (selectedVehicle->IsBuilding || selectedVehicle->IsClearing)
			{
				if ( ( (selectedVehicle->IsBuilding && selectedVehicle->BuildRounds == 0) ||
					   (selectedVehicle->IsClearing && selectedVehicle->ClearingRounds == 0)) &&
					 client->getMap()->possiblePlace (*selectedVehicle, mouseMapX, mouseMapY) && selectedVehicle->isNextTo (mouseMapX, mouseMapY))
				{
					mouse->SetCursor (CMove);
				}
				else
				{
					mouse->SetCursor (CNo);
				}
			}
		}
		else if (
			selectedBuilding                                &&
			selectedBuilding->owner == client->getActivePlayer() &&
			selectedBuilding->BuildList                     &&
			selectedBuilding->BuildList->size()             &&
			!selectedBuilding->IsWorking                    &&
			(*selectedBuilding->BuildList) [0]->metall_remaining <= 0)
		{
			if (selectedBuilding->canExitTo (mouseMapX, mouseMapY, client->getMap(), (*selectedBuilding->BuildList) [0]->type.getVehicle()))
			{
				mouse->SetCursor (CActivate);
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else if (selectedBuilding && selectedBuilding->owner == client->getActivePlayer() && mouseInputMode == activateVehicle)
		{
			if (selectedBuilding->canExitTo (mouseMapX, mouseMapY, client->getMap(), selectedBuilding->storedUnits[selectedBuilding->VehicleToActivate]->typ))
			{
				mouse->SetCursor (CActivate);
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else if (selectedBuilding && selectedBuilding->owner == client->getActivePlayer() && mouseInputMode == loadMode)
		{
			if (selectedBuilding->canLoad (mouseMapX, mouseMapY, client->getMap(), false))
			{
				mouse->SetCursor (CLoad);
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else
		{
			mouse->SetCursor (CHand);
		}
	}
	else
	{
		mouse->SetCursor (CHelp);
	}
}

void cGameGUI::handleMouseMove()
{
	if (checkScroll()) return;

	updateMouseCursor();

	// update mouseboxes
	if (savedMouseState.leftButtonPressed && !savedMouseState.rightButtonPressed && mouseBox.startX != -1 && mouse->x > HUD_LEFT_WIDTH)
	{
		mouseBox.endX = (mouse->x - HUD_LEFT_WIDTH + (offX * getZoom())) / getTileSize();
		mouseBox.endY = (mouse->y - HUD_TOP_HIGHT + (offY * getZoom())) / getTileSize();
	}
	if (savedMouseState.rightButtonPressed && !savedMouseState.leftButtonPressed && rightMouseBox.startX != -1 && mouse->x > HUD_LEFT_WIDTH)
	{
		rightMouseBox.endX = (mouse->x - HUD_LEFT_WIDTH + (offX * getZoom())) / getTileSize();
		rightMouseBox.endY = (mouse->y - HUD_TOP_HIGHT + (offY * getZoom())) / getTileSize();
	}

	static int lastX, lastY;
	if (!rightMouseBox.isTooSmall() && mouse->x > HUD_LEFT_WIDTH)	//the rightMouseBox is only used to trigger the right mouse button scroll
	{
		if (lastX != -1 && lastY != -1)
		{
			int speed = 5;
			int newPosX = offX + (mouse->x - lastX) * speed;
			int newPosY = offY + (mouse->y - lastY) * speed;

			setOffsetPosition (newPosX, newPosY);
		}
		lastX = mouse->x;
		lastY = mouse->y;
	}
	else
	{
		lastX = lastY = -1;
	}

	// check minimap
	if (miniMapImage.getIsClicked() || miniMapImage.getWasClicked())
	{
		miniMapClicked (this);
	}
}

void cGameGUI::handleMouseInputExtended (sMouseState mouseState)
{
	for (unsigned int i = 0; i < menuItems.size(); i++)
	{
		if (!menuItems[i]->isDisabled() && menuItems[i]->overItem (mouseState.x, mouseState.y)) return;
	}

	if (&selUnitNameEdit == activeItem)
	{
		selUnitNameEdit.setActivity (false);
		activeItem = NULL;
	}

	bool changeAllowed = !client->isFreezed ();

	savedMouseState = mouseState;

	cVehicle* overVehicle = NULL;
	cVehicle* overPlane = NULL;
	cBuilding* overBuilding = NULL;
	cBuilding* overBaseBuilding = NULL;
	if (overUnitField)
	{
		overVehicle = overUnitField->getVehicle();
		overPlane = overUnitField->getPlane();
		overBuilding = overUnitField->getTopBuilding();
		overBaseBuilding = overUnitField->getBaseBuilding();
	}

	if (selectedUnit && unitMenuActive && selectedUnit->areCoordsOverMenu (*this, mouseState.x, mouseState.y))
	{
		if (mouseState.leftButtonPressed) selectedUnit->setMenuSelection (*this);
		else if (mouseState.leftButtonReleased && !mouseState.rightButtonPressed) selectedUnit->menuReleased (*this);
		return;
	}

	const int mouseMapX = mouse->getKachelX (*this);
	const int mouseMapY = mouse->getKachelY (*this);
	// handle input on the map
	if (MouseStyle == OldSchool && mouseState.rightButtonReleased && !mouseState.leftButtonPressed && overUnitField)
	{
		if (selectedUnit && (overVehicle == selectedUnit ||
							 overPlane == selectedUnit ||
							 overBuilding == selectedUnit ||
							 overBaseBuilding == selectedUnit))
		{
			cUnitHelpMenu helpMenu (*client, &selectedUnit->data, selectedUnit->owner);
			helpMenu.show (client);
		}
		else if (overUnitField) selectUnit (overUnitField, true);
	}
	else
	{
		bool mouseOverSelectedUnit = false;
		if (selectedUnit && mouseMapX == selectedUnit->PosX && mouseMapY == selectedUnit->PosY) mouseOverSelectedUnit = true;
		if (selectedUnit && selectedUnit->data.isBig && mouseMapX >= selectedUnit->PosX && mouseMapX <= selectedUnit->PosX + 1
			&& mouseMapY >= selectedUnit->PosY && mouseMapY <= selectedUnit->PosY + 1) mouseOverSelectedUnit = true;

		if ( (mouseState.rightButtonReleased && !mouseState.leftButtonPressed && rightMouseBox.isTooSmall()) || (MouseStyle == OldSchool && mouseState.leftButtonPressed && mouseState.rightButtonReleased))
		{
			if (helpActive)
			{
				helpActive = false;
			}
			else
			{
				deselectGroup();
				if (overUnitField && mouseOverSelectedUnit)
				{
					std::vector<cVehicle*>& planes = overUnitField->getPlanes();
					cUnit* next = NULL;

					const cVehicle* selectedVehicle = getSelectedVehicle();
					const cBuilding* selectedBuilding = getSelectedBuilding();
					if (selectedVehicle)
					{
						std::vector<cVehicle*>::iterator it = std::find (planes.begin(), planes.end(), selectedVehicle);

						if (it == planes.end())
						{
							if (overBuilding) next = overBuilding;
							else if (overBaseBuilding) next = overBaseBuilding;
							else if (overPlane) next = overPlane;
						}
						else
						{
							++it;

							if (it != planes.end()) next = *it;
							else if (overVehicle) next = overVehicle;
							else if (overBuilding) next = overBuilding;
							else if (overBaseBuilding) next = overBaseBuilding;
							else if (planes.size() > 1)
							{
								next = planes[0];
							}
						}
					}
					else if (selectedBuilding)
					{
						if (overBuilding == selectedBuilding)
						{
							if (overBaseBuilding) next = overBaseBuilding;
							else if (overPlane) next = overPlane;
							else if (overVehicle) next = overVehicle;
						}
						else
						{
							if (overPlane) next = overPlane;
							else if (overVehicle) next = overVehicle;
							else if (overBuilding) next = overBuilding;
						}
					}

					deselectUnit();

					if (next) selectUnit (*next);
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

	if (mouseState.rightButtonReleased && !mouseState.leftButtonPressed)
	{
		rightMouseBox.invalidate();
	}
	else if (mouseState.rightButtonPressed && !mouseState.leftButtonPressed && rightMouseBox.startX == -1 && mouseState.x > HUD_LEFT_WIDTH && mouseState.y > 20)
	{
		rightMouseBox.startX = (mouseState.x - HUD_LEFT_WIDTH + (offX * getZoom())) / getTileSize();
		rightMouseBox.startY = (mouseState.y - HUD_TOP_HIGHT + (offY * getZoom())) / getTileSize();
	}
	if (mouseState.leftButtonReleased && !mouseState.rightButtonPressed)
	{
		// Store the currently selected unit to determine
		// if the lock state of the clicked unit maybe has to be changed.
		// If the selected unit changes during the click handling,
		// then the newly selected unit has to be added / removed
		// from the "locked units" list.
		cUnit* oldSelectedUnitForLock = selectedUnit;

		if (!mouseBox.isTooSmall())
		{
			selectBoxVehicles (mouseBox);
		}
		else if (changeAllowed && selectedUnit && mouse->cur == GraphicsData.gfx_Ctransf)
		{
			if (overVehicle)
			{
				cDialogTransfer transferDialog (*client, *selectedUnit, NULL, overVehicle);
				transferDialog.show (client);
			}
			else if (overBuilding)
			{
				cDialogTransfer transferDialog (*client, *selectedUnit, overBuilding, NULL);
				transferDialog.show (client);
			}
		}
		else if (changeAllowed && selectedVehicle && mouseInputMode == placeBand && mouse->cur == GraphicsData.gfx_Cband)
		{
			mouseInputMode = normalInput;

			if (selectedVehicle->BuildingTyp.getUnitDataOriginalVersion()->isBig)
			{
				sendWantBuild (*client, selectedVehicle->iID, selectedVehicle->BuildingTyp, selectedVehicle->BuildRounds, map->getOffset (selectedVehicle->BandX, selectedVehicle->BandY), false, 0);
			}
			else
			{
				sendWantBuild (*client, selectedVehicle->iID, selectedVehicle->BuildingTyp, selectedVehicle->BuildRounds, map->getOffset (selectedVehicle->PosX, selectedVehicle->PosY), true, map->getOffset (selectedVehicle->BandX, selectedVehicle->BandY));
			}
		}
		else if (changeAllowed && mouse->cur == GraphicsData.gfx_Cactivate && selectedUnit && mouseInputMode == activateVehicle)
		{
			sendWantActivate (*client, selectedUnit->iID, selectedUnit->isVehicle(), selectedUnit->storedUnits[selectedUnit->VehicleToActivate]->iID, mouseMapX, mouseMapY);
			updateMouseCursor();
		}
		else if (changeAllowed && mouse->cur == GraphicsData.gfx_Cactivate && selectedBuilding && selectedBuilding->BuildList && selectedBuilding->BuildList->size())
		{
			sendWantExitFinishedVehicle (*client, *selectedBuilding, mouseMapX, mouseMapY);
		}
		else if (changeAllowed && mouse->cur == GraphicsData.gfx_Cload && selectedBuilding && mouseInputMode == loadMode)
		{
			if (overVehicle && selectedBuilding->canLoad (overVehicle, false))
			{
				if (selectedBuilding->isNextTo (overVehicle->PosX, overVehicle->PosY)) sendWantLoad (*client, selectedBuilding->iID, false, overVehicle->iID);
				else
				{
					cPathCalculator pc (overVehicle->PosX, overVehicle->PosY, NULL, selectedBuilding, client->getMap(), overVehicle, true);
					sWaypoint* path = pc.calcPath();
					if (path)
					{
						sendMoveJob (*client, path, overVehicle->iID);
						sendEndMoveAction (*client, overVehicle->iID, selectedBuilding->iID, EMAT_GET_IN);
					}
					else
					{
						if (random (2)) PlayVoice (VoiceData.VOINoPath1);
						else PlayVoice (VoiceData.VOINoPath2);
					}
				}
			}
			else if (overPlane && selectedBuilding->canLoad (overPlane, false))
			{
				if (selectedBuilding->isNextTo (overPlane->PosX, overPlane->PosY)) sendWantLoad (*client, selectedBuilding->iID, false, overPlane->iID);
				else
				{
					cPathCalculator pc (overPlane->PosX, overPlane->PosY, NULL, selectedBuilding, client->getMap(), overPlane, true);
					sWaypoint* path = pc.calcPath();
					if (path)
					{
						sendMoveJob (*client, path, overPlane->iID);
						sendEndMoveAction (*client, overPlane->iID, selectedBuilding->iID, EMAT_GET_IN);
					}
					else
					{
						if (random (2)) PlayVoice (VoiceData.VOINoPath1);
						else PlayVoice (VoiceData.VOINoPath2);
					}
				}
			}
		}
		else if (changeAllowed && mouse->cur == GraphicsData.gfx_Cload && selectedVehicle && mouseInputMode == loadMode)
		{
			if (selectedVehicle->data.factorAir > 0 && overVehicle)
			{
				if (overVehicle->PosX == selectedVehicle->PosX && overVehicle->PosY == selectedVehicle->PosY) sendWantLoad (*client, selectedVehicle->iID, true, overVehicle->iID);
				else
				{
					cPathCalculator pc (selectedVehicle->PosX, selectedVehicle->PosY, overVehicle->PosX, overVehicle->PosY, client->getMap(), selectedVehicle);
					sWaypoint* path = pc.calcPath();
					if (path)
					{
						sendMoveJob (*client, path, selectedVehicle->iID);
						sendEndMoveAction (*client, selectedVehicle->iID, overVehicle->iID, EMAT_LOAD);
					}
					else
					{
						if (random (2)) PlayVoice (VoiceData.VOINoPath1);
						else PlayVoice (VoiceData.VOINoPath2);
					}
				}
			}
			else if (overVehicle)
			{
				if (selectedVehicle->isNextTo (overVehicle->PosX, overVehicle->PosY)) sendWantLoad (*client, selectedVehicle->iID, true, overVehicle->iID);
				else
				{
					cPathCalculator pc (overVehicle->PosX, overVehicle->PosY, selectedVehicle, NULL, client->getMap(), overVehicle, true);
					sWaypoint* path = pc.calcPath();
					if (path)
					{
						sendMoveJob (*client, path, overVehicle->iID);
						sendEndMoveAction (*client, overVehicle->iID, selectedVehicle->iID, EMAT_GET_IN);
					}
					else
					{
						if (random (2)) PlayVoice (VoiceData.VOINoPath1);
						else PlayVoice (VoiceData.VOINoPath2);
					}
				}
			}
		}
		else if (changeAllowed && mouse->cur == GraphicsData.gfx_Cmuni && selectedVehicle && mouseInputMode == muniActive)
		{
			if (overVehicle) sendWantSupply (*client, overVehicle->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REARM);
			else if (overPlane && overPlane->FlightHigh == 0) sendWantSupply (*client, overPlane->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REARM);
			else if (overBuilding) sendWantSupply (*client, overBuilding->iID, false, selectedVehicle->iID, true, SUPPLY_TYPE_REARM);
		}
		else if (changeAllowed && mouse->cur == GraphicsData.gfx_Crepair && selectedVehicle && mouseInputMode == repairActive)
		{
			if (overVehicle) sendWantSupply (*client, overVehicle->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REPAIR);
			else if (overPlane && overPlane->FlightHigh == 0) sendWantSupply (*client, overPlane->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REPAIR);
			else if (overBuilding) sendWantSupply (*client, overBuilding->iID, false, selectedVehicle->iID, true, SUPPLY_TYPE_REPAIR);
		}
		else if (!helpActive)
		{
			//Hud.CheckButtons();
			// check whether the mouse is over a unit menu:
			if ( (selectedUnit && unitMenuActive && selectedUnit->areCoordsOverMenu (*this, mouseState.x, mouseState.y)))
			{
			}
			else
				// check, if the player wants to attack:
				if (changeAllowed && mouse->cur == GraphicsData.gfx_Cattack && selectedVehicle && !selectedVehicle->attacking && !selectedVehicle->MoveJobActive)
				{
					cUnit* target = selectTarget (mouseMapX, mouseMapY, selectedVehicle->data.canAttack, client->getMap());

					if (selectedVehicle->isInRange (mouseMapX, mouseMapY))
					{
						//find target ID
						int targetId = 0;
						if (target && target->isVehicle()) targetId = target->iID;

						Log.write (" Client: want to attack " + iToStr (mouseMapX) + ":" + iToStr (mouseMapY) + ", Vehicle ID: " + iToStr (targetId), cLog::eLOG_TYPE_NET_DEBUG);
						sendWantAttack (*client, targetId, client->getMap()->getOffset (mouseMapX, mouseMapY), selectedVehicle->iID, true);
					}
					else if (target)
					{
						cPathCalculator pc (selectedVehicle->PosX, selectedVehicle->PosY, client->getMap(), selectedVehicle, mouseMapX, mouseMapY);
						sWaypoint* path = pc.calcPath();
						if (path)
						{
							sendMoveJob (*client, path, selectedVehicle->iID);
							sendEndMoveAction (*client, selectedVehicle->iID, target->iID, EMAT_ATTACK);
						}
						else
						{
							if (random (2)) PlayVoice (VoiceData.VOINoPath1);
							else PlayVoice (VoiceData.VOINoPath2);
						}
					}
				}
				else if (changeAllowed && mouse->cur == GraphicsData.gfx_Cattack && selectedBuilding && !selectedBuilding->attacking)
				{
					//find target ID
					int targetId = 0;
					cUnit* target = selectTarget (mouseMapX, mouseMapY, selectedBuilding->data.canAttack, client->getMap());
					if (target && target->isVehicle()) targetId = target->iID;

					int offset = map->getOffset (selectedBuilding->PosX, selectedBuilding->PosY);
					sendWantAttack (*client, targetId, client->getMap()->getOffset (mouseMapX, mouseMapY), offset, false);
				}
				else if (changeAllowed && mouse->cur == GraphicsData.gfx_Csteal && selectedVehicle)
				{
					if (overVehicle) sendWantComAction (*client, selectedVehicle->iID, overVehicle->iID, true, true);
					else if (overPlane && overPlane->FlightHigh == 0) sendWantComAction (*client, selectedVehicle->iID, overVehicle->iID, true, true);
				}
				else if (changeAllowed && mouse->cur == GraphicsData.gfx_Cdisable && selectedVehicle)
				{
					if (overVehicle) sendWantComAction (*client, selectedVehicle->iID, overVehicle->iID, true, false);
					else if (overPlane && overPlane->FlightHigh == 0) sendWantComAction (*client, selectedVehicle->iID, overPlane->iID, true, false);
					else if (overBuilding) sendWantComAction (*client, selectedVehicle->iID, overBuilding->iID, false, false);
				}
				else if (MouseStyle == OldSchool && overUnitField && selectUnit (overUnitField, false))
				{}
				else if (changeAllowed && mouse->cur == GraphicsData.gfx_Cmove && selectedVehicle && !selectedVehicle->moving && !selectedVehicle->attacking)
				{
					if (selectedVehicle->IsBuilding)
					{
						sendWantEndBuilding (*client, *selectedVehicle, mouseMapX, mouseMapY);
					}
					else
					{
						if (selectedVehiclesGroup.size() > 1) client->startGroupMove();
						else client->addMoveJob (selectedVehicle, mouseMapX, mouseMapY);
					}
				}
				else if (overUnitField)
				{
					// open unit menu
					if (changeAllowed && selectedVehicle && (Contains (overUnitField->getPlanes(), selectedVehicle) || overVehicle == selectedVehicle))
					{
						if (!selectedVehicle->moving)
						{
							unitMenuActive = !unitMenuActive;
							if (unitMenuActive) selectedVehicle->selectedMenuButtonIndex = -1;
							PlayFX (SoundData.SNDHudButton);
						}
					}
					else if (changeAllowed && selectedBuilding && (overBaseBuilding == selectedBuilding || overBuilding == selectedBuilding))
					{
						unitMenuActive = !unitMenuActive;
						if (unitMenuActive) selectedBuilding->selectedMenuButtonIndex = -1;
						PlayFX (SoundData.SNDHudButton);
					}
					// select unit when using modern style
					else if (MouseStyle == Modern) selectUnit (overUnitField, true);
				}
		}
		else if (overUnitField)
		{
			if (overPlane)
			{
				cUnitHelpMenu helpMenu (*client, &overPlane->data, overPlane->owner);
				helpMenu.show (client);
			}
			else if (overVehicle)
			{
				cUnitHelpMenu helpMenu (*client, &overVehicle->data, overVehicle->owner);
				helpMenu.show (client);
			}
			else if (overBuilding)
			{
				cUnitHelpMenu helpMenu (*client, &overBuilding->data, overBuilding->owner);
				helpMenu.show (client);
			}
			else if (overBaseBuilding)
			{
				cUnitHelpMenu helpMenu (*client, &overBaseBuilding->data, overBaseBuilding->owner);
				helpMenu.show (client);
			}
			helpActive = false;
		}

		// toggle the lock state of an enemy unit, if it is newly selected / deselected
		if (overUnitField && lockChecked())
		{
			if (selectedUnit && selectedUnit != oldSelectedUnitForLock && selectedUnit->owner != player)
				player->toggelLock (overUnitField);
		}

		mouseBox.invalidate();
	}
	else if (mouseState.leftButtonPressed && !mouseState.rightButtonPressed && mouseBox.startX == -1.f && mouseState.x > HUD_LEFT_WIDTH && mouseState.y > 20)
	{
		mouseBox.startX = (mouseState.x - HUD_LEFT_WIDTH + (offX * getZoom())) / getTileSize();
		mouseBox.startY = (mouseState.y - HUD_TOP_HIGHT + (offY * getZoom())) / getTileSize();
	}

	// check getZoom() via mousewheel
	if (mouseState.wheelUp)
	{
		setZoom (getZoom() + 0.05f, true, true);
	}
	else if (mouseState.wheelDown)
	{
		setZoom (getZoom() - 0.05f, true, true);
	}
}

void cGameGUI::doScroll (int dir)
{
	int step = cSettings::getInstance().getScrollSpeed();
	int newOffX = offX;
	int newOffY = offY;

	switch (dir)
	{
		case 1:
			newOffX -= step;
			newOffY += step;
			break;
		case 2:
			newOffY += step;
			break;
		case 3:
			newOffX += step;
			newOffY += step;
			break;
		case 4:
			newOffX -= step;
			break;
		case 6:
			newOffX += step;
			break;
		case 7:
			newOffX -= step;
			newOffY -= step;
			break;
		case 8:
			newOffY -= step;
			break;
		case 9:
			newOffX += step;
			newOffY -= step;
			break;
	}

	setOffsetPosition (newOffX, newOffY);
}

void cGameGUI::doCommand (const string& cmd)
{
	cServer* server = client->getServer();
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
	else if (cmd.substr (0, 12).compare ("/cache size ") == 0)
	{
		int size = atoi (cmd.substr (12, cmd.length()).c_str());
		//since atoi is too stupid to report an error, do an extra check, when the number is 0
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
	else if (cmd.substr (0, 6).compare ("/kick ") == 0)
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
	else if (cmd.substr (0, 9).compare ("/credits ") == 0)
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

		string playerStr = cmd.substr (9, cmd.find_first_of (" ", 9) - 9);
		string creditsStr = cmd.substr (cmd.find_first_of (" ", 9) + 1, cmd.length());

		cPlayer* Player = server->getPlayerFromString (playerStr);

		if (!Player)
		{
			addMessage ("Wrong parameter");
			return;
		}

		int credits = atoi (creditsStr.c_str());

		Player->Credits = credits;

		sendCredits (*server, credits, Player->getNr());
	}
	else if (cmd.substr (0, 12).compare ("/disconnect ") == 0)
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

		//server cannot be disconnected
		//can not disconnect local players
		if (!player || Player->getNr() == 0 || Player->isLocal())
		{
			addMessage ("Wrong parameter");
			return;
		}

		cNetMessage* message = new cNetMessage (TCP_CLOSE);
		message->pushInt16 (Player->getSocketNum());
		server->pushEvent (message);
	}
	else if (cmd.substr (0, 9).compare ("/deadline") == 0)
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

		int i = atoi (cmd.substr (9, cmd.length()).c_str());
		if (i == 0 && cmd[10] != '0')
		{
			addMessage ("Wrong parameter");
			return;
		}

		server->setDeadline (i);
		Log.write ("Deadline changed to " + iToStr (i), cLog::eLOG_TYPE_INFO);
	}
	else if (cmd.substr (0, 7).compare ("/resync") == 0)
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
				const std::vector<cPlayer*>& playerList = *server->PlayerList;
				for (unsigned int i = 0; i < playerList.size(); i++)
				{
					sendRequestResync (*client, playerList[i]->getNr());
				}
			}
			else
			{
				sendRequestResync (*client, client->getActivePlayer()->getNr());
			}
		}
	}
	else if (cmd.substr (0, 5).compare ("/mark") == 0)
	{
		std::string cmdArg (cmd);
		cmdArg.erase (0, 5);
		cNetMessage* message = new cNetMessage (GAME_EV_WANT_MARK_LOG);
		message->pushString (cmdArg);
		client->sendNetMessage (message);
	}
	else if (cmd.substr (0, 7).compare ("/color ") == 0)
	{
		int cl = 0;
		sscanf (cmd.c_str(), "color %d", &cl);
		cl %= 8;
		player->setColor (cl);
	}
	else if (cmd.compare ("/fog off") == 0)
	{
		if (!server)
		{
			addMessage ("Command can only be used by Host");
			return;
		}
		server->getPlayerFromNumber (player->getNr())->revealMap();
		player->revealMap();
	}
	else if (cmd.compare ("/survey") == 0)
	{
		if (!server)
		{
			addMessage ("Command can only be used by Host");
			return;
		}
		map->assignRessources (*server->Map);
		player->revealResource();
	}
	else if (cmd.substr (0, 6).compare ("/pause") == 0)
	{
		if (!server)
		{
			addMessage ("Command can only be used by Host");
			return;
		}
		server->enableFreezeMode (FREEZE_PAUSE);
	}
	else if (cmd.substr (0, 7).compare ("/resume") == 0)
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
	windDir = (float) (dir / 57.29577f);
}

void cGameGUI::selectUnit_vehicle (cVehicle& vehicle)
{
	// TOFIX: add that the unit renaming will be aborted here when active
	if (selectedUnit == &vehicle)
	{
		if (selectedUnit->owner == player)
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
	// TOFIX: add that the unit renaming will be aborted here when active
	if (selectedUnit == &building)
	{
		if (selectedUnit->owner == player)
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
	if (vehicle && !vehicle->moving && ! (plane && (unitMenuActive || vehicle->owner != player)))
	{
		selectUnit_vehicle (*vehicle);
		return true;
	}
	cBuilding* topBuilding = OverUnitField->getTopBuilding();
	const cVehicle* selectedVehicle = getSelectedVehicle();
	if (topBuilding && (base || ( (topBuilding->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE || !selectedVehicle) && (!OverUnitField->getTopBuilding()->data.canBeLandedOn || (!selectedVehicle || selectedVehicle->data.factorAir == 0)))))
	{
		selectUnit_building (*topBuilding);
		return true;
	}
	cBuilding* baseBuilding = OverUnitField->getBaseBuilding();
	if ( (base || !selectedVehicle) && baseBuilding && baseBuilding->owner != NULL)
	{
		selectUnit_building (*baseBuilding);
		return true;
	}
	return false;
}

void cGameGUI::selectBoxVehicles (sMouseBox& box)
{
	if (box.startX < 0.f || box.startY < 0.f || box.endX < 0.f || box.endY < 0.f) return;

	int startFieldX = (int) std::min (box.startX, box.endX);
	int startFieldY = (int) std::min (box.startY, box.endY);
	int endFieldX = (int) std::max (box.startX, box.endX);
	int endFieldY = (int) std::max (box.startY, box.endY);

	deselectGroup();

	bool newSelected = true;
	for (int x = startFieldX; x <= endFieldX; x++)
	{
		for (int y = startFieldY; y <= endFieldY; y++)
		{
			int offset = map->getOffset (x, y);

			cVehicle* vehicle = (*map) [offset].getVehicle();
			if (!vehicle || vehicle->owner != player) vehicle = (*map) [offset].getPlane();

			if (vehicle && vehicle->owner == player && !vehicle->IsBuilding && !vehicle->IsClearing && !vehicle->moving)
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
	if (newSelected && selectedVehiclesGroup.size() > 0)
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
		static int nextChange = 25, nextDirChange = 25, dir = 90, change = 3;
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
				change        = random (11) -  5;
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

void cGameGUI::handleKeyInput (SDL_KeyboardEvent& key, const string& ch)
{
	if (key.keysym.sym == SDLK_LSHIFT || key.keysym.sym == SDLK_RSHIFT)
	{
		if (key.type == SDL_KEYDOWN)
		{
			shiftPressed = true;
		}
		else if (key.type == SDL_KEYUP)
		{
			shiftPressed = false;
		}
	}

	// first check whether the end key was pressed
	if ( (activeItem != &chatBox || chatBox.isDisabled()) && activeItem != &selUnitNameEdit && key.keysym.sym == KeysList.KeyEndTurn && !client->isFreezed ())
	{
		if (key.state == SDL_PRESSED && !endButton.getIsClicked()) endButton.clicked (this);
		else if (key.state == SDL_RELEASED && endButton.getIsClicked() && !client->bWantToEnd) endButton.released (this);
		return;
	}

	// we will handle only pressed keys for all other hotkeys
	if (key.state != SDL_PRESSED) return;

	// check whether the player wants to abort waiting
	if (client->getFreezeMode (FREEZE_WAIT_FOR_RECONNECT) && key.keysym.sym == SDLK_F2)
	{
		sendAbortWaiting (*client);
	}

	cVehicle* selectedVehicle = getSelectedVehicle();
	cBuilding* selectedBuilding = getSelectedBuilding();

	if (key.keysym.sym == KeysList.KeyExit)
	{
		cDialogYesNo yesNoDialog (lngPack.i18n ("Text~Comp~End_Game"));
		if (yesNoDialog.show (client) == 0) end = true;
	}
	else if (activeItem && !activeItem->isDisabled() && activeItem->handleKeyInput (key.keysym, ch, this))
	{}
	else if (key.keysym.sym == KeysList.KeyJumpToAction)
	{
		if (msgCoordsX != -1)
		{
			int offsetX = msgCoordsX * 64 - ( (int) ( ( (float) (Video.getResolutionX() - HUD_TOTAL_WIDTH) / (2 * getTileSize())) * 64)) + 32;
			int offsetY = msgCoordsY * 64 - ( (int) ( ( (float) (Video.getResolutionY() - HUD_TOTAL_HIGHT) / (2 * getTileSize())) * 64)) + 32;
			setOffsetPosition (offsetX, offsetY);
			msgCoordsX = -1;
		}
	}
	else if (key.keysym.sym == KeysList.KeyChat)
	{
		if (! (key.keysym.mod & KMOD_ALT))
		{
			if (chatBox.isDisabled()) chatBox.setDisabled (false);
			activeItem = &chatBox;
			chatBox.setActivity (true);
		}
	}
	// scroll and zoom hotkeys
	else if (key.keysym.sym == KeysList.KeyScroll1) doScroll (1);
	else if (key.keysym.sym == KeysList.KeyScroll3) doScroll (3);
	else if (key.keysym.sym == KeysList.KeyScroll7) doScroll (7);
	else if (key.keysym.sym == KeysList.KeyScroll9) doScroll (9);
	else if (key.keysym.sym == KeysList.KeyScroll2a || key.keysym.sym == KeysList.KeyScroll2b) doScroll (2);
	else if (key.keysym.sym == KeysList.KeyScroll4a || key.keysym.sym == KeysList.KeyScroll4b) doScroll (4);
	else if (key.keysym.sym == KeysList.KeyScroll6a || key.keysym.sym == KeysList.KeyScroll6b) doScroll (6);
	else if (key.keysym.sym == KeysList.KeyScroll8a || key.keysym.sym == KeysList.KeyScroll8b) doScroll (8);
	else if (key.keysym.sym == KeysList.KeyZoomIna || key.keysym.sym == KeysList.KeyZoomInb) setZoom ( (float) (getZoom() + 0.05), true, false);
	else if (key.keysym.sym == KeysList.KeyZoomOuta || key.keysym.sym == KeysList.KeyZoomOutb) setZoom ( (float) (getZoom() - 0.05), true, false);
	// position handling hotkeys
	else if (key.keysym.sym == KeysList.KeyCenterUnit && selectedUnit) selectedUnit->center (*this);
	else if (key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_ALT) savePosition (0);
	else if (key.keysym.sym == SDLK_F6 && key.keysym.mod & KMOD_ALT) savePosition (1);
	else if (key.keysym.sym == SDLK_F7 && key.keysym.mod & KMOD_ALT) savePosition (2);
	else if (key.keysym.sym == SDLK_F8 && key.keysym.mod & KMOD_ALT) savePosition (3);
	else if (key.keysym.sym == SDLK_F5 && savedPositions[0].offsetX >= 0 && savedPositions[0].offsetY >= 0) jumpToSavedPos (0);
	else if (key.keysym.sym == SDLK_F6 && savedPositions[1].offsetX >= 0 && savedPositions[1].offsetY >= 0) jumpToSavedPos (1);
	else if (key.keysym.sym == SDLK_F7 && savedPositions[2].offsetX >= 0 && savedPositions[2].offsetY >= 0) jumpToSavedPos (2);
	else if (key.keysym.sym == SDLK_F8 && savedPositions[3].offsetX >= 0 && savedPositions[3].offsetY >= 0) jumpToSavedPos (3);
	// Hotkeys for the unit menues
	else if (key.keysym.sym == KeysList.KeyUnitMenuAttack && selectedUnit && selectedUnit->data.canAttack && selectedUnit->data.shotsCur && !client->isFreezed() && selectedUnit->owner == player)
	{
		mouseInputMode = mouseInputAttackMode;
		updateMouseCursor();
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuBuild && selectedVehicle && !selectedVehicle->data.canBuild.empty() && !selectedVehicle->IsBuilding && !client->isFreezed () && selectedVehicle->owner == player)
	{
		sendWantStopMove (*client, selectedVehicle->iID);
		cBuildingsBuildMenu buildMenu (*client, player, selectedVehicle);
		buildMenu.show (client);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuBuild && selectedBuilding && !selectedBuilding->data.canBuild.empty() && !client->isFreezed () && selectedBuilding->owner == player)
	{
		cVehiclesBuildMenu buildMenu (*this, player, selectedBuilding);
		buildMenu.show (client);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuTransfer && selectedVehicle && selectedVehicle->data.storeResType != sUnitData::STORE_RES_NONE && !selectedVehicle->IsBuilding && !selectedVehicle->IsClearing && !client->isFreezed () && selectedVehicle->owner == player)
	{
		mouseInputMode = transferMode;
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuTransfer && selectedBuilding && selectedBuilding->data.storeResType != sUnitData::STORE_RES_NONE && !client->isFreezed () && selectedBuilding->owner == player)
	{
		mouseInputMode = transferMode;
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuAutomove && selectedVehicle && selectedVehicle->data.canSurvey && !client->isFreezed () && selectedVehicle->owner == player)
	{
		for (unsigned int i = 1; i < selectedVehiclesGroup.size(); i++)
		{
			selectedVehiclesGroup[i]->executeAutoMoveJobCommand (*client);
		}
		selectedVehicle->executeAutoMoveJobCommand (*client);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuStart && selectedBuilding && selectedBuilding->data.canWork && !selectedBuilding->IsWorking && ( (selectedBuilding->BuildList && selectedBuilding->BuildList->size()) || selectedBuilding->data.canBuild.empty()) && !client->isFreezed () && selectedBuilding->owner == player)
	{
		sendWantStartWork (*client, *selectedBuilding);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuStop && selectedVehicle && (selectedVehicle->ClientMoveJob || (selectedVehicle->IsBuilding && selectedVehicle->BuildRounds) || (selectedVehicle->IsClearing && selectedVehicle->ClearingRounds)) && !client->isFreezed () && selectedVehicle->owner == player)
	{
		if (selectedVehicle->ClientMoveJob)
		{
			for (unsigned int i = 1; i < selectedVehiclesGroup.size(); i++)
			{
				if (selectedVehiclesGroup[i]->ClientMoveJob) sendWantStopMove (*client, selectedVehiclesGroup[i]->iID);
			}
			sendWantStopMove (*client, selectedVehicle->iID);
		}
		else if (selectedVehicle->IsBuilding)
		{
			for (unsigned int i = 1; i < selectedVehiclesGroup.size(); i++)
			{
				if (selectedVehiclesGroup[i]->IsBuilding && selectedVehiclesGroup[i]->BuildRounds) sendWantStopBuilding (*client, selectedVehiclesGroup[i]->iID);
			}
			sendWantStopBuilding (*client, selectedVehicle->iID);
		}
		else if (selectedVehicle->IsClearing)
		{
			for (unsigned int i = 1; i < selectedVehiclesGroup.size(); i++)
			{
				if (selectedVehiclesGroup[i]->IsClearing && selectedVehiclesGroup[i]->ClearingRounds) sendWantStopClear (*client, *selectedVehiclesGroup[i]);
			}
			sendWantStopClear (*client, *selectedVehicle);
		}
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuStop && selectedBuilding && selectedBuilding->IsWorking && !client->isFreezed () && selectedBuilding->owner == player)
	{
		sendWantStopWork (*client, *selectedBuilding);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuClear && selectedVehicle && selectedVehicle->data.canClearArea && map->fields[map->getOffset (selectedVehicle->PosX, selectedVehicle->PosY)].getRubble() && !selectedVehicle->IsClearing && !client->isFreezed () && selectedVehicle->owner == player)
	{
		for (unsigned int i = 1; i < selectedVehiclesGroup.size(); i++)
		{
			if (selectedVehiclesGroup[i]->data.canClearArea && map->fields[map->getOffset (selectedVehiclesGroup[i]->PosX, selectedVehiclesGroup[i]->PosY)].getRubble() && !selectedVehiclesGroup[i]->IsClearing) sendWantStartClear (*client, *selectedVehiclesGroup[i]);
		}
		sendWantStartClear (*client, *selectedVehicle);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuSentry && selectedVehicle && !client->isFreezed () && selectedVehicle->owner == player)
	{
		for (unsigned int i = 1; i < selectedVehiclesGroup.size(); i++)
		{
			if (selectedVehicle->sentryActive == selectedVehiclesGroup[i]->sentryActive)
			{
				sendChangeSentry (*client, selectedVehiclesGroup[i]->iID, true);
			}
		}
		sendChangeSentry (*client, selectedVehicle->iID, true);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuSentry && selectedBuilding && (selectedBuilding->sentryActive || selectedBuilding->data.canAttack) && !client->isFreezed () && selectedBuilding->owner == player)
	{
		sendChangeSentry (*client, selectedBuilding->iID, false);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuManualFire && selectedVehicle && (selectedVehicle->manualFireActive || selectedVehicle->data.canAttack) && !client->isFreezed () && selectedVehicle->owner == player)
	{
		for (unsigned int i = 1; i < selectedVehiclesGroup.size(); i++)
		{
			if ( (selectedVehiclesGroup[i]->manualFireActive || selectedVehiclesGroup[i]->data.canAttack)
				 && selectedVehicle->manualFireActive == selectedVehiclesGroup[i]->manualFireActive)
			{
				sendChangeManualFireStatus (*client, selectedVehiclesGroup[i]->iID, true);
			}
		}
		sendChangeManualFireStatus (*client, selectedVehicle->iID, true);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuManualFire && selectedBuilding && (selectedBuilding->manualFireActive || selectedBuilding->data.canAttack) && !client->isFreezed () && selectedBuilding->owner == player)
	{
		sendChangeManualFireStatus (*client, selectedBuilding->iID, false);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuActivate && selectedUnit && selectedUnit->data.storageUnitsMax > 0 && !client->isFreezed() && selectedUnit->owner == player)
	{
		cStorageMenu storageMenu (*client, selectedUnit->storedUnits, *selectedUnit);
		storageMenu.show (client);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuLoad && selectedUnit && selectedUnit->data.storageUnitsMax > 0 && !client->isFreezed() && selectedUnit->owner == player)
	{
		toggleMouseInputMode (loadMode);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuReload && selectedVehicle && selectedVehicle->data.canRearm && selectedVehicle->data.storageResCur >= 2 && !client->isFreezed () && selectedVehicle->owner == player)
	{
		mouseInputMode = muniActive;
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuRepair && selectedVehicle && selectedVehicle->data.canRepair && selectedVehicle->data.storageResCur >= 2 && !client->isFreezed () && selectedVehicle->owner == player)
	{
		mouseInputMode = repairActive;
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuLayMine && selectedVehicle && selectedVehicle->data.canPlaceMines && selectedVehicle->data.storageResCur > 0 && !client->isFreezed () && selectedVehicle->owner == player)
	{
		for (unsigned int i = 1; i < selectedVehiclesGroup.size(); i++)
		{
			if (selectedVehiclesGroup[i]->data.canPlaceMines || selectedVehiclesGroup[i]->data.storageResCur > 0) selectedVehiclesGroup[i]->executeLayMinesCommand (*client);
		}
		selectedVehicle->executeLayMinesCommand (*client);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuClearMine && selectedVehicle && selectedVehicle->data.canPlaceMines && selectedVehicle->data.storageResCur < selectedVehicle->data.storageResMax && !client->isFreezed () && selectedVehicle->owner == player)
	{
		for (unsigned int i = 1; i < selectedVehiclesGroup.size(); i++)
		{
			if (selectedVehiclesGroup[i]->data.canPlaceMines || selectedVehiclesGroup[i]->data.storageResCur < selectedVehiclesGroup[i]->data.storageResMax) selectedVehiclesGroup[i]->executeClearMinesCommand (*client);
		}
		selectedVehicle->executeClearMinesCommand (*client);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuDisable && selectedVehicle && selectedVehicle->data.canDisable && selectedVehicle->data.shotsCur && !client->isFreezed () && selectedVehicle->owner == player)
	{
		mouseInputMode = disableMode;
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuSteal && selectedVehicle && selectedVehicle->data.canCapture && selectedVehicle->data.shotsCur && !client->isFreezed () && selectedVehicle->owner == player)
	{
		mouseInputMode = stealMode;
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuInfo && selectedUnit)
	{
		cUnitHelpMenu helpMenu (*client, &selectedUnit->data, selectedUnit->owner);
		helpMenu.show (client);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuDistribute && selectedBuilding && selectedBuilding->data.canMineMaxRes > 0 && selectedBuilding->IsWorking && !client->isFreezed () && selectedBuilding->owner == player)
	{
		cMineManagerMenu mineManager (*client, selectedBuilding);
		mineManager.show (client);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuResearch && selectedBuilding && selectedBuilding->data.canResearch && selectedBuilding->IsWorking && !client->isFreezed () && selectedBuilding->owner == player)
	{
		cDialogResearch researchDialog (*client, selectedBuilding->owner);
		researchDialog.show (client);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuUpgrade && selectedBuilding && selectedBuilding->data.convertsGold && !client->isFreezed () && selectedBuilding->owner == player)
	{
		cUpgradeMenu upgradeMenu (*client, selectedBuilding->owner);
		upgradeMenu.show (client);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuDestroy && selectedBuilding && selectedBuilding->data.canSelfDestroy && !client->isFreezed () && selectedBuilding->owner == player)
	{
		cDestructMenu destructMenu;
		if (destructMenu.show (client) == 0) sendWantSelfDestroy (*client, *selectedBuilding);
	}
	// Hotkeys for the hud
	else if (key.keysym.sym == KeysList.KeyFog) setFog (!fogChecked());
	else if (key.keysym.sym == KeysList.KeyGrid) setGrid (!gridChecked());
	else if (key.keysym.sym == KeysList.KeyScan) setScan (!scanChecked());
	else if (key.keysym.sym == KeysList.KeyRange) setRange (!rangeChecked());
	else if (key.keysym.sym == KeysList.KeyAmmo) setAmmo (!ammoChecked());
	else if (key.keysym.sym == KeysList.KeyHitpoints) setHits (!hitsChecked());
	else if (key.keysym.sym == KeysList.KeyColors) setColor (!colorChecked());
	else if (key.keysym.sym == KeysList.KeyStatus) setStatus (!statusChecked());
	else if (key.keysym.sym == KeysList.KeySurvey) setSurvey (!surveyChecked());
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
	if (gui->selectedUnit) gui->selectedUnit->center (*gui);
}

void cGameGUI::reportsReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	cReportsMenu reportMenu (*gui->getClient(), gui->player);
	reportMenu.show (gui->getClient());
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
	cUnit* unit = gui->player->getNextUnit (gui->getSelectedUnit());
	if (unit)
	{
		gui->selectUnit (*unit);

		unit->center (*gui);
	}
}


void cGameGUI::prevReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	cUnit* unit = gui->player->getPrevUnit (gui->getSelectedUnit());
	if (unit)
	{
		gui->selectUnit (*unit);

		unit->center (*gui);
	}
}

void cGameGUI::doneReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);

	if (gui->shiftPressed)
	{
		sendMoveJobResume (*gui->client, 0);
		return;
	}

	cUnit* unit = gui->selectedUnit;

	if (unit && unit->owner == gui->client->getActivePlayer())
	{
		unit->center (*gui);
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

	for (unsigned int i = 0; i < gui->playersInfo.size(); i++)
	{
		gui->playersInfo[i]->setDisabled (!gui->showPlayers);
	}
}

void cGameGUI::changedMiniMap (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	gui->callMiniMapDraw();
}

void cGameGUI::resetMiniMapOffset()
{
	const int zoomFactor = twoXChecked() ? MINIMAP_ZOOM_FACTOR : 1;

	if (zoomFactor == 1) miniMapOffX = miniMapOffY = 0;
	else
	{
		int centerPosX = (int) (offX / 64.0 + (Video.getResolutionX() - 192.0) / (getTileSize() * 2));
		int centerPosY = (int) (offY / 64.0 + (Video.getResolutionY() -  32.0) / (getTileSize() * 2));
		miniMapOffX = centerPosX - (map->getSize() / (zoomFactor * 2));
		miniMapOffY = centerPosY - (map->getSize() / (zoomFactor * 2));

		miniMapOffX = std::max (miniMapOffX, 0);
		miniMapOffY = std::max (miniMapOffY, 0);
		miniMapOffX = std::min (map->getSize() - (map->getSize() / zoomFactor), miniMapOffX);
		miniMapOffY = std::min (map->getSize() - (map->getSize() / zoomFactor), miniMapOffY);
	}
}

void cGameGUI::miniMapClicked (void* parent)
{
	static int lastX = 0, lastY = 0;
	int x = mouse->x;
	int y = mouse->y;
	if (lastX == x && lastY == y) return;

	cGameGUI* gui = static_cast<cGameGUI*> (parent);

	const int displayedMapWidth = (int) ( (Video.getResolutionX() - HUD_TOTAL_WIDTH) / gui->getZoom());
	const int displayedMapHight = (int) ( (Video.getResolutionY() - HUD_TOTAL_HIGHT) / gui->getZoom());
	const int zoomFactor = gui->twoXChecked() ? MINIMAP_ZOOM_FACTOR : 1;

	gui->offX = gui->miniMapOffX * 64 + ( (x - MINIMAP_POS_X) * gui->map->getSize() * 64) / (MINIMAP_SIZE * zoomFactor);
	gui->offY = gui->miniMapOffY * 64 + ( (y - MINIMAP_POS_Y) * gui->map->getSize() * 64) / (MINIMAP_SIZE * zoomFactor);
	gui->offX -= displayedMapWidth / 2;
	gui->offY -= displayedMapHight / 2;

	lastX = x;
	lastY = y;

	gui->checkOffsetPosition();
}

void cGameGUI::miniMapRightClicked (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);

	cVehicle* selectedVehicle = gui->getSelectedVehicle();
	if (!selectedVehicle || selectedVehicle->owner != gui->client->getActivePlayer()) return;

	int x = mouse->x;
	int y = mouse->y;
	const int zoomFactor = gui->twoXChecked() ? MINIMAP_ZOOM_FACTOR : 1;

	int destX = gui->miniMapOffX + ( (x - MINIMAP_POS_X) * gui->map->getSize()) / (MINIMAP_SIZE * zoomFactor);
	int destY = gui->miniMapOffY + ( (y - MINIMAP_POS_Y) * gui->map->getSize()) / (MINIMAP_SIZE * zoomFactor);

	gui->client->addMoveJob (selectedVehicle, destX, destY, &gui->selectedVehiclesGroup);
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
	cDialogPreferences preferencesDialog (gui->getClient()->getActivePlayer());
	preferencesDialog.show (gui->getClient());
}

void cGameGUI::filesReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	cLoadSaveMenu loadSaveMenu (*gui->getClient(), gui->getClient()->getServer());
	if (loadSaveMenu.show (gui->getClient()) != 1) gui->end = true;
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
		else sendChatMessageToServer (*gui->client, gui->player->getName() + ": " + chatString);
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
	int zoomOffX = (int) (offX * getZoom());
	int zoomOffY = (int) (offY * getZoom());

	int startX = ( (offX - 1) / 64) - 1 < 0 ? 0 : ( (offX - 1) / 64) - 1;
	int startY = ( (offY - 1) / 64) - 1 < 0 ? 0 : ( (offY - 1) / 64) - 1;

	int endX = Round (offX / 64.0 + (float) (Video.getResolutionX() - HUD_TOTAL_WIDTH) / getTileSize());
	endX = std::min (endX, map->getSize() - 1);
	int endY = Round (offY / 64.0 + (float) (Video.getResolutionY() - HUD_TOTAL_HIGHT) / getTileSize());
	endY = std::min (endY, map->getSize() - 1);

	if (timer400ms) map->staticMap->generateNextAnimationFrame();

	SDL_Rect clipRect = { HUD_LEFT_WIDTH, HUD_TOP_HIGHT, Uint16 (Video.getResolutionX() - HUD_TOTAL_WIDTH), Uint16 (Video.getResolutionY() - HUD_TOTAL_HIGHT) };
	SDL_SetClipRect (buffer, &clipRect);

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
	if (surveyChecked() || (selectedVehicle && selectedVehicle->owner == player && selectedVehicle->data.canSurvey))
	{
		drawResources (startX, startY, endX, endY, zoomOffX, zoomOffY);
	}

	if (selectedVehicle && ( (selectedVehicle->ClientMoveJob && selectedVehicle->ClientMoveJob->bSuspended) || selectedVehicle->BuildPath))
	{
		selectedVehicle->DrawPath (*this);
	}

	debugOutput.draw();

	drawSelectionBox (zoomOffX, zoomOffY);

	SDL_SetClipRect (buffer, NULL);

	drawUnitCircles();

	if (selectedUnit && unitMenuActive) selectedUnit->drawMenu (*this);

	drawFx (false);

	displayMessages();
}

void cGameGUI::drawTerrain (int zoomOffX, int zoomOffY)
{
	int tileSize = getTileSize();
	SDL_Rect dest, tmp;
	dest.y = HUD_TOP_HIGHT - zoomOffY;
	// draw the terrain
	for (int y = 0; y < map->getSize(); y++)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX;
		if (dest.y >= HUD_TOP_HIGHT - tileSize)
		{
			int pos = y * map->getSize();
			for (int x = 0; x < map->getSize(); x++)
			{
				if (dest.x >= HUD_LEFT_WIDTH - tileSize)
				{
					tmp = dest;
					const sTerrain& terr = map->staticMap->getTerrain (pos);

					// draw the fog:
					if (fogChecked() && !player->ScanMap[pos])
					{
						if (!cSettings::getInstance().shouldDoPrescale() && (terr.shw->w != tileSize || terr.shw->h != tileSize)) scaleSurface (terr.shw_org, terr.shw, tileSize, tileSize);
						SDL_BlitSurface (terr.shw, NULL, buffer, &tmp);
					}
					else
					{
						if (!cSettings::getInstance().shouldDoPrescale() && (terr.sf->w != tileSize || terr.sf->h != tileSize)) scaleSurface (terr.sf_org, terr.sf, tileSize, tileSize);
						SDL_BlitSurface (terr.sf, NULL, buffer, &tmp);
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
	int tileSize = getTileSize();
	SDL_Rect dest;
	dest.x = HUD_LEFT_WIDTH;
	dest.y = HUD_TOP_HIGHT + tileSize - (zoomOffY % tileSize);
	dest.w = Video.getResolutionX() - HUD_TOTAL_WIDTH;
	dest.h = 1;
	for (int y = 0; y < (Video.getResolutionY() - HUD_TOTAL_HIGHT) / tileSize + 1; y++)
	{
		SDL_FillRect (buffer, &dest, GRID_COLOR);
		dest.y += tileSize;
	}
	dest.x = HUD_LEFT_WIDTH + tileSize - (zoomOffX % tileSize);
	dest.y = HUD_TOP_HIGHT;
	dest.w = 1;
	dest.h = Video.getResolutionY() - HUD_TOTAL_HIGHT;
	for (int x = 0; x < (Video.getResolutionX() - HUD_TOTAL_WIDTH) / tileSize + 1; x++)
	{
		SDL_FillRect (buffer, &dest, GRID_COLOR);
		dest.x += tileSize;
	}
}

void cGameGUI::addFx (cFx* fx)
{
	FxList.insert (FxList.begin(), fx);
	fx->playSound (*this);
}

void cGameGUI::drawFx (bool bottom) const
{
	SDL_Rect clipRect = { HUD_LEFT_WIDTH, HUD_TOP_HIGHT, Uint16 (Video.getResolutionX() - HUD_TOTAL_WIDTH), Uint16 (Video.getResolutionY() - HUD_TOTAL_HIGHT) };	SDL_SetClipRect (buffer, &clipRect);
	SDL_SetClipRect (buffer, &clipRect);

	for (unsigned int i = 0; i < client->FxList.size(); i++)
	{
		if (client->FxList[i]->bottom == bottom)
		{
			client->FxList[i]->draw (*this);
		}
	}

	for (unsigned int i = 0; i < FxList.size(); i++)
	{
		if (FxList[i]->bottom == bottom)
		{
			FxList[i]->draw (*this);
		}
	}

	SDL_SetClipRect (buffer, NULL);
}

void cGameGUI::runFx ()
{
	for (unsigned int i = 0; i < FxList.size(); i++)
	{
		FxList[i]->run ();

		if (FxList[i]->isFinished ())
		{
			delete FxList[i];
			FxList.erase (FxList.begin() + i);
			i--;
		}
	}
}

SDL_Rect cGameGUI::calcScreenPos (int x, int y) const
{
	SDL_Rect pos;
	pos.x = HUD_LEFT_WIDTH - ( (int) ( (offX - x) * getZoom()));
	pos.y = HUD_TOP_HIGHT  - ( (int) ( (offY - y) * getZoom()));

	return pos;
}

void cGameGUI::drawAttackCursor (int x, int y) const
{
	assert (mouse->cur == GraphicsData.gfx_Cattack);

	if (selectedUnit == NULL) return;

	const sUnitData& data = selectedUnit->data;

	cUnit* target = selectTarget (x, y, data.canAttack, client->getMap());

	if (!target || (target == selectedUnit))
	{
		SDL_Rect r = {1, 29, 35, 3};
		SDL_FillRect (GraphicsData.gfx_Cattack, &r, 0);
		return;
	}

	int t = target->data.hitpointsCur;
	int wc = (int) ( (float) t / target->data.hitpointsMax * 35);

	t = target->calcHealth (data.damage);

	int wp = 0;
	if (t)
	{
		wp = (int) ( (float) t / target->data.hitpointsMax * 35);
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
	int tileSize = getTileSize();
	SDL_Rect dest;
	//draw rubble and all base buildings (without bridges)
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;

	for (int y = startY; y <= endY; y++)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map->getOffset (startX, y);
		for (int x = startX; x <= endX; x++)
		{
			std::vector<cBuilding*>& buildings = map->fields[pos].getBuildings();
			for (std::vector<cBuilding*>::reverse_iterator it = buildings.rbegin(); it != buildings.rend(); ++it)
			{
				if ( (*it)->data.surfacePosition != sUnitData::SURFACE_POS_BENEATH_SEA &&
					 (*it)->data.surfacePosition != sUnitData::SURFACE_POS_BASE &&
					 (*it)->owner) break;

				if (player->canSeeAnyAreaUnder (**it))
				{
					// Draw big unit only once
					// TODO: bug when (x,y) is outside of the drawing screen.
					if ( (*it)->PosX == x && (*it)->PosY == y)
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
	sSubBase* sb;
	SDL_Rect tmp = { dest.x, dest.y, Uint16 (getTileSize()), 8 };
	if (building.data.isBig) tmp.w *= 2;
	sb = building.SubBase;
	// the VS compiler gives a warning on casting a pointer to long.
	// therefore we will first cast to long long and then cut this to Unit32 again.
	SDL_FillRect (buffer, &tmp, (Uint32) (long long) (sb));
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

	sSubBase* sb = client->getServer()->Map->fields[offset].getBuilding()->SubBase;
	if (sb == NULL) return;

	SDL_Rect tmp = { dest.x, dest.y, Uint16 (getTileSize()), 8 };
	if (building.data.isBig) tmp.w *= 2;

	// the VS compiler gives a warning on casting a pointer to long.
	// therefore we will first cast to long long and then cut this to Unit32 again.
	SDL_FillRect (buffer, &tmp, (Uint32) (long long) (sb));
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
	int tileSize = getTileSize();
	//draw top buildings (except connectors)
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; ++y, dest.y += tileSize)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map->getOffset (startX, y);
		for (int x = startX; x <= endX; ++x, ++pos, dest.x += tileSize)
		{
			cBuilding* building = map->fields[pos].getBuilding();
			if (building == NULL) continue;
			if (building->data.surfacePosition != sUnitData::SURFACE_POS_GROUND) continue;
			if (!player->canSeeAnyAreaUnder (*building)) continue;
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
	int tileSize = getTileSize();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; ++y, dest.y += tileSize)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map->getOffset (startX, y);
		for (int x = startX; x <= endX; ++x, ++pos, dest.x += tileSize)
		{
			cVehicle* vehicle = map->fields[pos].getVehicle();
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
	int tileSize = getTileSize();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; ++y, dest.y += tileSize)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map->getOffset (startX, y);
		for (int x = startX; x <= endX; ++x, ++pos, dest.x += tileSize)
		{
			std::vector<cBuilding*>& buildings = map->fields[pos].getBuildings();

			for (std::vector<cBuilding*>::iterator it = buildings.begin(); it != buildings.end(); ++it)
			{
				if ( (*it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)
				{
					(*it)->draw (&dest, *this);
				}
			}
			for (std::vector<cBuilding*>::iterator it = buildings.begin(); it != buildings.end(); ++it)
			{
				if ( (*it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)
				{
					(*it)->draw (&dest, *this);
				}
			}

			cVehicle* vehicle = map->fields[pos].getVehicle();
			if (vehicle && (vehicle->IsClearing || vehicle->IsBuilding) &&
				player->canSeeAnyAreaUnder (*vehicle))
			{
				//make sure a big vehicle is drawn only once
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
	int tileSize = getTileSize();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; ++y, dest.y += tileSize)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map->getOffset (startX, y);
		for (int x = startX; x <= endX; ++x, ++pos, dest.x += tileSize)
		{
			cVehicle* vehicle = map->fields[pos].getVehicle();
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
	int tileSize = getTileSize();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; ++y, dest.y += tileSize)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map->getOffset (startX, y);
		for (int x = startX; x <= endX; ++x, ++pos, dest.x += tileSize)
		{
			cBuilding* building = map->fields[pos].getTopBuilding();
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
	int tileSize = getTileSize();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; ++y, dest.y += tileSize)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map->getOffset (startX, y);
		for (int x = startX; x <= endX; ++x, ++pos, dest.x += tileSize)
		{
			std::vector<cVehicle*>& planes = map->fields[pos].getPlanes();
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
	int tileSize = getTileSize();
	SDL_Rect dest, tmp, src = { 0, 0, Uint16 (tileSize), Uint16 (tileSize) };
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; ++y, dest.y += tileSize)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = map->getOffset (startX, y);
		for (int x = startX; x <= endX; ++x, ++pos, dest.x += tileSize)
		{
			if (!player->hasResourceExplored (pos)) continue;
			if (map->isBlocked (pos)) continue;

			const sResources& resource = map->getResource (pos);
			if (resource.typ == RES_NONE)
			{
				src.x = 0;
				tmp = dest;
				if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_metal->w != ResourceData.res_metal_org->w / 64 * tileSize || ResourceData.res_metal->h != tileSize)) scaleSurface (ResourceData.res_metal_org, ResourceData.res_metal, ResourceData.res_metal_org->w / 64 * tileSize, tileSize);
				SDL_BlitSurface (ResourceData.res_metal, &src, buffer, &tmp);
			}
			else
			{
				src.x = resource.value * tileSize;
				tmp = dest;
				if (resource.typ == RES_METAL)
				{
					if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_metal->w != ResourceData.res_metal_org->w / 64 * tileSize || ResourceData.res_metal->h != tileSize)) scaleSurface (ResourceData.res_metal_org, ResourceData.res_metal, ResourceData.res_metal_org->w / 64 * tileSize, tileSize);
					SDL_BlitSurface (ResourceData.res_metal, &src, buffer, &tmp);
				}
				else if (resource.typ == RES_OIL)
				{
					if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_oil->w != ResourceData.res_oil_org->w / 64 * tileSize || ResourceData.res_oil->h != tileSize)) scaleSurface (ResourceData.res_oil_org, ResourceData.res_oil, ResourceData.res_oil_org->w / 64 * tileSize, tileSize);
					SDL_BlitSurface (ResourceData.res_oil, &src, buffer, &tmp);
				}
				else // Gold
				{
					if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_gold->w != ResourceData.res_gold_org->w / 64 * tileSize || ResourceData.res_gold->h != tileSize)) scaleSurface (ResourceData.res_gold_org, ResourceData.res_gold, ResourceData.res_gold_org->w / 64 * tileSize, tileSize);
					SDL_BlitSurface (ResourceData.res_gold, &src, buffer, &tmp);
				}
			}
		}
	}
}

void cGameGUI::drawSelectionBox (int zoomOffX, int zoomOffY)
{
	if (!mouseBox.isValid()) return;

	const int mouseTopX = static_cast<int> (min (mouseBox.startX, mouseBox.endX) * getTileSize());
	const int mouseTopY = static_cast<int> (min (mouseBox.startY, mouseBox.endY) * getTileSize());
	const int mouseBottomX = static_cast<int> (max (mouseBox.startX, mouseBox.endX) * getTileSize());
	const int mouseBottomY = static_cast<int> (max (mouseBox.startY, mouseBox.endY) * getTileSize());
	const Uint32 color = 0x00FFFF00;
	SDL_Rect d;

	d.h = 1;
	d.w = mouseBottomX - mouseTopX;
	d.x = mouseTopX - zoomOffX + HUD_LEFT_WIDTH;
	d.y = mouseBottomY - zoomOffY + 20;
	SDL_FillRect (buffer, &d, color);

	d.h = 1;
	d.w = mouseBottomX - mouseTopX;
	d.x = mouseTopX - zoomOffX + HUD_LEFT_WIDTH;
	d.y = mouseTopY - zoomOffY + 20;
	SDL_FillRect (buffer, &d, color);

	d.h = mouseBottomY - mouseTopY;
	d.w = 1;
	d.x = mouseTopX - zoomOffX + HUD_LEFT_WIDTH;
	d.y = mouseTopY - zoomOffY + 20;
	SDL_FillRect (buffer, &d, color);

	d.h = mouseBottomY - mouseTopY;
	d.w = 1;
	d.x = mouseBottomX - zoomOffX + HUD_LEFT_WIDTH;
	d.y = mouseTopY - zoomOffY + 20;
	SDL_FillRect (buffer, &d, color);
}

void cGameGUI::displayMessages()
{
	if (messages.size() == 0) return;

	int height = 0;
	for (int i = (int) messages.size() - 1; i >= 0; i--)
	{
		const sMessage* message = messages[i];
		height += 17 + font->getFontHeight() * (message->len / (Video.getResolutionX() - 300));
	}
	SDL_Rect scr = { 0, 0, Uint16 (Video.getResolutionX() - 200), Uint16 (height + 6) };
	SDL_Rect dest = { 180, 30, 0, 0 };

	if (cSettings::getInstance().isAlphaEffects()) SDL_BlitSurface (GraphicsData.gfx_shadow, &scr, buffer, &dest);
	dest.x = 180 + 2; dest.y = 34;
	dest.w = Video.getResolutionX() - 204;
	dest.h = height;

	for (unsigned int i = 0; i < messages.size(); i++)
	{
		const sMessage* message = messages[i];
		string msgString = message->msg;
		//HACK TO SHOW PLAYERCOLOR IN CHAT
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
			SDL_BlitSurface (color, &rColorSrc, buffer, &rDest);  //blit color
			dest.x += rColorSrc.w + CELLSPACE; //add border for color
			dest.w -= rColorSrc.w + CELLSPACE;
			dest.y = font->showTextAsBlock (dest, msgString);
			dest.x -= rColorSrc.w + CELLSPACE; //reset border from color
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
	map->staticMap->scaleSurfaces (getTileSize());
	// Vehicles:
	for (unsigned int i = 0; i < UnitsData.getNrVehicles(); ++i)
	{
		UnitsData.vehicle[i].scaleSurfaces (getZoom());
	}
	// Buildings:
	for (unsigned int i = 0; i < UnitsData.getNrBuildings(); ++i)
	{
		UnitsData.building[i].scaleSurfaces (getZoom());
	}

	if (UnitsData.dirt_small_org && UnitsData.dirt_small) scaleSurface (UnitsData.dirt_small_org, UnitsData.dirt_small, (int) (UnitsData.dirt_small_org->w * getZoom()), (int) (UnitsData.dirt_small_org->h * getZoom()));
	if (UnitsData.dirt_small_shw_org && UnitsData.dirt_small_shw) scaleSurface (UnitsData.dirt_small_shw_org, UnitsData.dirt_small_shw, (int) (UnitsData.dirt_small_shw_org->w * getZoom()), (int) (UnitsData.dirt_small_shw_org->h * getZoom()));
	if (UnitsData.dirt_big_org && UnitsData.dirt_big) scaleSurface (UnitsData.dirt_big_org, UnitsData.dirt_big, (int) (UnitsData.dirt_big_org->w * getZoom()), (int) (UnitsData.dirt_big_org->h * getZoom()));
	if (UnitsData.dirt_big_shw_org && UnitsData.dirt_big_shw) scaleSurface (UnitsData.dirt_big_shw_org, UnitsData.dirt_big_shw, (int) (UnitsData.dirt_big_shw_org->w * getZoom()), (int) (UnitsData.dirt_big_shw_org->h * getZoom()));

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

void cGameGUI::makePanel (bool open)
{
	SDL_Rect tmp;
	if (open)
	{
		PlayFX (SoundData.SNDPanelOpen);
		SDL_Rect top = { 0, Sint16 ( (Video.getResolutionY() / 2) - 479), 171, 479 };
		SDL_Rect bottom = { 0, Sint16 (Video.getResolutionY() / 2), 171, 481 };
		SDL_BlitSurface (GraphicsData.gfx_panel_top, NULL, buffer, &tmp);
		tmp = bottom;
		SDL_BlitSurface (GraphicsData.gfx_panel_bottom, NULL, buffer, &tmp);
		while (top.y > -479)
		{
			Video.draw();
			mouse->draw (false, screen);
			SDL_Delay (10);
			top.y -= 10;
			bottom.y += 10;
			draw (false, false);
			tmp = top;
			SDL_BlitSurface (GraphicsData.gfx_panel_top, NULL, buffer, &tmp);
			SDL_BlitSurface (GraphicsData.gfx_panel_bottom, NULL, buffer, &bottom);
		}
	}
	else
	{
		PlayFX (SoundData.SNDPanelClose);
		SDL_Rect top = { 0, -480, 171, 479 };
		SDL_Rect bottom = { 0, Sint16 (Video.getResolutionY()), 171, 481 };
		while (bottom.y > Video.getResolutionY() / 2)
		{
			Video.draw();
			mouse->draw (false, screen);
			SDL_Delay (10);
			top.y += 10;
			if (top.y > (Video.getResolutionY() / 2) - 479 - 9) top.y = (Video.getResolutionY() / 2) - 479;
			bottom.y -= 10;
			if (bottom.y < Video.getResolutionY() / 2 + 9) bottom.y = Video.getResolutionY() / 2;
			draw (false, false);
			tmp = top;
			SDL_BlitSurface (GraphicsData.gfx_panel_top, NULL, buffer, &tmp);
			tmp = bottom;
			SDL_BlitSurface (GraphicsData.gfx_panel_bottom, NULL, buffer, &tmp);
		}
		Video.draw();
		mouse->draw (false, screen);
		SDL_Delay (100);
	}
}

void cGameGUI::drawUnitCircles()
{
	SDL_Rect clipRect = { HUD_LEFT_WIDTH, HUD_TOP_HIGHT, Uint16 (Video.getResolutionX() - HUD_TOTAL_WIDTH), Uint16 (Video.getResolutionY() - HUD_TOTAL_HIGHT) };
	SDL_SetClipRect (buffer, &clipRect);

	cVehicle* selectedVehicle = getSelectedVehicle();
	cBuilding* selectedBuilding = getSelectedBuilding();

	if (selectedVehicle)
	{
		cVehicle& v = *selectedVehicle;
		bool movementOffset = !v.IsBuilding && !v.IsClearing;
		int const spx = v.getScreenPosX (*this, movementOffset);
		int const spy = v.getScreenPosY (*this, movementOffset);
		if (scanChecked())
		{
			if (v.data.isBig)
			{
				drawCircle (spx + getTileSize(), spy + getTileSize(), v.data.scan * getTileSize(), SCAN_COLOR, buffer);
			}
			else
			{
				drawCircle (spx + getTileSize() / 2, spy + getTileSize() / 2, v.data.scan * getTileSize(), SCAN_COLOR, buffer);
			}
		}
		if (rangeChecked())
		{
			if (v.data.canAttack & TERRAIN_AIR) drawCircle (spx + getTileSize() / 2, spy + getTileSize() / 2, v.data.range * getTileSize() + 2, RANGE_AIR_COLOR, buffer);
			else drawCircle (spx + getTileSize() / 2, spy + getTileSize() / 2, v.data.range * getTileSize() + 1, RANGE_GROUND_COLOR, buffer);
		}
		if (v.owner == player &&
			(
				(v.IsBuilding && v.BuildRounds    == 0) ||
				(v.IsClearing && v.ClearingRounds == 0)
			) && !v.BuildPath)
		{
			if (v.data.isBig)
			{
				if (map->possiblePlace (v, v.PosX - 1, v.PosY - 1)) drawExitPoint (spx - getTileSize(),     spy - getTileSize());
				if (map->possiblePlace (v, v.PosX    , v.PosY - 1)) drawExitPoint (spx,                spy - getTileSize());
				if (map->possiblePlace (v, v.PosX + 1, v.PosY - 1)) drawExitPoint (spx + getTileSize(),     spy - getTileSize());
				if (map->possiblePlace (v, v.PosX + 2, v.PosY - 1)) drawExitPoint (spx + getTileSize() * 2, spy - getTileSize());
				if (map->possiblePlace (v, v.PosX - 1, v.PosY)) drawExitPoint (spx - getTileSize(),     spy);
				if (map->possiblePlace (v, v.PosX + 2, v.PosY)) drawExitPoint (spx + getTileSize() * 2, spy);
				if (map->possiblePlace (v, v.PosX - 1, v.PosY + 1)) drawExitPoint (spx - getTileSize(),     spy + getTileSize());
				if (map->possiblePlace (v, v.PosX + 2, v.PosY + 1)) drawExitPoint (spx + getTileSize() * 2, spy + getTileSize());
				if (map->possiblePlace (v, v.PosX - 1, v.PosY + 2)) drawExitPoint (spx - getTileSize(),     spy + getTileSize() * 2);
				if (map->possiblePlace (v, v.PosX    , v.PosY + 2)) drawExitPoint (spx,                spy + getTileSize() * 2);
				if (map->possiblePlace (v, v.PosX + 1, v.PosY + 2)) drawExitPoint (spx + getTileSize(),     spy + getTileSize() * 2);
				if (map->possiblePlace (v, v.PosX + 2, v.PosY + 2)) drawExitPoint (spx + getTileSize() * 2, spy + getTileSize() * 2);
			}
			else
			{
				if (map->possiblePlace (v, v.PosX - 1, v.PosY - 1)) drawExitPoint (spx - getTileSize(), spy - getTileSize());
				if (map->possiblePlace (v, v.PosX    , v.PosY - 1)) drawExitPoint (spx,            spy - getTileSize());
				if (map->possiblePlace (v, v.PosX + 1, v.PosY - 1)) drawExitPoint (spx + getTileSize(), spy - getTileSize());
				if (map->possiblePlace (v, v.PosX - 1, v.PosY)) drawExitPoint (spx - getTileSize(), spy);
				if (map->possiblePlace (v, v.PosX + 1, v.PosY)) drawExitPoint (spx + getTileSize(), spy);
				if (map->possiblePlace (v, v.PosX - 1, v.PosY + 1)) drawExitPoint (spx - getTileSize(), spy + getTileSize());
				if (map->possiblePlace (v, v.PosX    , v.PosY + 1)) drawExitPoint (spx,            spy + getTileSize());
				if (map->possiblePlace (v, v.PosX + 1, v.PosY + 1)) drawExitPoint (spx + getTileSize(), spy + getTileSize());
			}
		}
		if (mouseInputMode == placeBand)
		{
			if (v.BuildingTyp.getUnitDataOriginalVersion()->isBig)
			{
				SDL_Rect dest;
				dest.x = HUD_LEFT_WIDTH - (int) (offX * getZoom()) + getTileSize() * v.BandX;
				dest.y =  HUD_TOP_HIGHT - (int) (offY * getZoom()) + getTileSize() * v.BandY;
				CHECK_SCALING (GraphicsData.gfx_band_big, GraphicsData.gfx_band_big_org, (float) getTileSize() / 64.0);
				SDL_BlitSurface (GraphicsData.gfx_band_big, NULL, buffer, &dest);
			}
			else
			{
				int x = mouse->getKachelX (*this);
				int y = mouse->getKachelY (*this);
				if (x == v.PosX || y == v.PosY)
				{
					SDL_Rect dest;
					dest.x = HUD_LEFT_WIDTH - (int) (offX * getZoom()) + getTileSize() * x;
					dest.y =  HUD_TOP_HIGHT - (int) (offY * getZoom()) + getTileSize() * y;
					CHECK_SCALING (GraphicsData.gfx_band_small, GraphicsData.gfx_band_small_org, (float) getTileSize() / 64.0);
					SDL_BlitSurface (GraphicsData.gfx_band_small, NULL, buffer, &dest);
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
		if (mouseInputMode == activateVehicle && v.owner == player)
		{
			v.DrawExitPoints (v.storedUnits[v.VehicleToActivate]->typ, *this);
		}
	}
	else if (selectedBuilding)
	{
		int spx = selectedBuilding->getScreenPosX (*this);
		int spy = selectedBuilding->getScreenPosY (*this);
		if (scanChecked())
		{
			if (selectedBuilding->data.isBig)
			{
				drawCircle (spx + getTileSize(),
							spy + getTileSize(),
							selectedBuilding->data.scan * getTileSize(), SCAN_COLOR, buffer);
			}
			else
			{
				drawCircle (spx + getTileSize() / 2,
							spy + getTileSize() / 2,
							selectedBuilding->data.scan * getTileSize(), SCAN_COLOR, buffer);
			}
		}
		if (rangeChecked() && (selectedBuilding->data.canAttack & TERRAIN_GROUND) && !selectedBuilding->data.explodesOnContact)
		{
			drawCircle (spx + getTileSize() / 2,
						spy + getTileSize() / 2,
						selectedBuilding->data.range * getTileSize() + 2, RANGE_GROUND_COLOR, buffer);
		}
		if (rangeChecked() && (selectedBuilding->data.canAttack & TERRAIN_AIR))
		{
			drawCircle (spx + getTileSize() / 2,
						spy + getTileSize() / 2,
						selectedBuilding->data.range * getTileSize() + 2, RANGE_AIR_COLOR, buffer);
		}

		if (selectedBuilding->BuildList                              &&
			selectedBuilding->BuildList->size()                      &&
			!selectedBuilding->IsWorking                             &&
			(*selectedBuilding->BuildList) [0]->metall_remaining <= 0 &&
			selectedBuilding->owner == player)
		{
			selectedBuilding->DrawExitPoints ( (*selectedBuilding->BuildList) [0]->type.getVehicle(), *this);
		}
		if (mouseInputMode == activateVehicle && selectedBuilding->owner == player)
		{
			selectedBuilding->DrawExitPoints (selectedBuilding->storedUnits[selectedBuilding->VehicleToActivate]->typ, *this);
		}
	}
	drawLockList (*player);

	SDL_SetClipRect (buffer, NULL);
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
		const SDL_Rect screenPos = {Sint16 (unit->getScreenPosX (*this)), Sint16 (unit->getScreenPosY (*this)), 0, 0};

		if (scanChecked())
		{
			if (unit->data.isBig)
				drawCircle (screenPos.x + tileSize, screenPos.y + tileSize, unit->data.scan * tileSize, SCAN_COLOR, buffer);
			else
				drawCircle (screenPos.x + tileSize / 2, screenPos.y + tileSize / 2, unit->data.scan * tileSize, SCAN_COLOR, buffer);
		}
		if (rangeChecked() && (unit->data.canAttack & TERRAIN_GROUND))
			drawCircle (screenPos.x + tileSize / 2, screenPos.y + tileSize / 2,
						unit->data.range * tileSize + 1, RANGE_GROUND_COLOR, buffer);
		if (rangeChecked() && (unit->data.canAttack & TERRAIN_AIR))
			drawCircle (screenPos.x + tileSize / 2, screenPos.y + tileSize / 2,
						unit->data.range * tileSize + 2, RANGE_AIR_COLOR, buffer);
		if (ammoChecked() && unit->data.canAttack)
			unit->drawMunBar (*this, screenPos);
		if (hitsChecked())
			unit->drawHealthBar (*this, screenPos);
	}
}

void cGameGUI::drawExitPoint (int x, int y)
{
	SDL_Rect dest, scr;
	int nr = getAnimationSpeed() % 5;
	scr.y = 0;
	scr.h = scr.w = getTileSize();
	scr.x = getTileSize() * nr;
	dest.x = x;
	dest.y = y;
	float factor = (float) (getTileSize() / 64.0);

	CHECK_SCALING (GraphicsData.gfx_exitpoints, GraphicsData.gfx_exitpoints_org, factor);
	SDL_BlitSurface (GraphicsData.gfx_exitpoints, &scr, buffer, &dest);
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

	savedPositions[slotNumber].offsetX = offX + (int) ( (Video.getResolutionX() - HUD_TOTAL_WIDTH) / getZoom() / 2);
	savedPositions[slotNumber].offsetY = offY + (int) ( (Video.getResolutionY() - HUD_TOTAL_HIGHT) / getZoom() / 2);
}

void cGameGUI::jumpToSavedPos (int slotNumber)
{
	if (slotNumber < 0 || slotNumber >= MAX_SAVE_POSITIONS) return;

	const int offsetX = savedPositions[slotNumber].offsetX - (int) ( (Video.getResolutionX() - HUD_TOTAL_WIDTH) / getZoom() / 2);
	const int offsetY = savedPositions[slotNumber].offsetY - (int) ( (Video.getResolutionY() - HUD_TOTAL_HIGHT) / getZoom() / 2);

	setOffsetPosition (offsetX, offsetY);
}
