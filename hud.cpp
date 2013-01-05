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

#include "autosurface.h"
#include "hud.h"
#include "main.h"
#include "mouse.h"
#include "sound.h"
#include "dialog.h"
#include "unifonts.h"
#include "client.h"
#include "clientevents.h"
#include "netmessage.h"
#include "keys.h"
#include "input.h"
#include "pcx.h"
#include "player.h"
#include "settings.h"
#include "events.h"
#include "video.h"
#include "buildings.h"
#include "vehicles.h"
#include "attackJobs.h"

using namespace std;

bool sMouseBox::isTooSmall() const
{
	if (startX == -1 || startY == -1 || endX == -1 || endY == -1) return true;
	return ! (endX > startX + 0.5 || endX < startX - 0.5 || endY > startY + 0.5 || endY < startY - 0.5);
}

sMouseBox::sMouseBox() :
	startX (-1),
	startY (-1),
	endX (-1),
	endY (-1)
{}

cDebugOutput::cDebugOutput()
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

void cDebugOutput::draw()
{
	#define DEBUGOUT_X_POS		(Video.getResolutionX()-200)

	int debugOff = 30;

	cPlayer* player = Client->gameGUI.player;
	const cGameGUI& gui = Client->gameGUI;

	if (debugPlayers)
	{
		font->showText (DEBUGOUT_X_POS, debugOff, "Players: " + iToStr ( (int) Client->PlayerList->Size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		SDL_Rect rDest = { DEBUGOUT_X_POS, debugOff, 20, 10 };
		SDL_Rect rSrc = { 0, 0, 20, 10 };
		SDL_Rect rDotDest = {DEBUGOUT_X_POS - 10, debugOff, 10, 10 };
		SDL_Rect rBlackOut = {DEBUGOUT_X_POS + 20, debugOff, 0, 10 };
		for (unsigned int i = 0; i < Client->PlayerList->Size(); i++)
		{
			//HACK SHOWFINISHEDPLAYERS
			SDL_Rect rDot = { 10 , 0, 10, 10 }; //for green dot

			if ( (*Client->PlayerList) [i]->bFinishedTurn/* && (*client->PlayerList)[i] != player*/)
			{
				SDL_BlitSurface (GraphicsData.gfx_player_ready, &rDot, buffer, &rDotDest);
			}
			/*else if( (*client->PlayerList)[i] == player && client->bWantToEnd )
			{
				SDL_BlitSurface( GraphicsData.gfx_player_ready, &rDot, buffer, &rDotDest );
			}*/
			else
			{
				rDot.x = 0; //for red dot
				SDL_BlitSurface (GraphicsData.gfx_player_ready, &rDot, buffer, &rDotDest);
			}

			SDL_BlitSurface ( (*Client->PlayerList) [i]->color, &rSrc, buffer, &rDest);
			if ( (*Client->PlayerList) [i] == player)
			{
				string sTmpLine = " " + (*Client->PlayerList) [i]->name + ", nr: " + iToStr ( (*Client->PlayerList) [i]->Nr) + " << you! ";
				rBlackOut.w = font->getTextWide (sTmpLine, FONT_LATIN_SMALL_WHITE);  //black out background for better recognizing
				SDL_FillRect (buffer, &rBlackOut, 0x000000);
				font->showText (rBlackOut.x, debugOff + 1, sTmpLine , FONT_LATIN_SMALL_WHITE);
			}
			else
			{
				string sTmpLine = " " + (*Client->PlayerList) [i]->name + ", nr: " + iToStr ( (*Client->PlayerList) [i]->Nr) + " ";
				rBlackOut.w = font->getTextWide (sTmpLine, FONT_LATIN_SMALL_WHITE);  //black out background for better recognizing
				SDL_FillRect (buffer, &rBlackOut, 0x000000);
				font->showText (rBlackOut.x, debugOff + 1, sTmpLine , FONT_LATIN_SMALL_WHITE);
			}
			debugOff += 10; //use 10 for pixel high of dots instead of text high
			rDest.y = rDotDest.y = rBlackOut.y = debugOff;

		}
	}

	if (debugAjobs)
	{
		font->showText (DEBUGOUT_X_POS, debugOff, "ClientAttackJobs: " + iToStr ( (int) Client->attackJobs.Size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		if (Server)
		{
			font->showText (DEBUGOUT_X_POS, debugOff, "ServerAttackJobs: " + iToStr ( (int) Server->AJobs.Size()), FONT_LATIN_SMALL_WHITE);
			debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		}
	}

	if (debugBaseClient)
	{
		font->showText (DEBUGOUT_X_POS, debugOff, "subbases: " + iToStr ( (int) player->base.SubBases.Size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
	}

	if (debugBaseServer)
	{
		cPlayer* serverPlayer = Server->getPlayerFromNumber (player->Nr);
		font->showText (DEBUGOUT_X_POS, debugOff, "subbases: " + iToStr ( (int) serverPlayer->base.SubBases.Size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
	}

	if (debugFX)
	{
		/*font->showText(DEBUGOUT_X_POS, debugOff, "fx-count: " + iToStr((int)FXList.Size() + (int)FXListBottom.Size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS, debugOff, "wind-dir: " + iToStr(( int ) ( fWindDir*57.29577 )), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);*/
	}
	if (debugTraceServer || debugTraceClient)
	{
		trace();
	}
	if (debugCache)
	{
		const cDrawingCache& dCache = Client->gameGUI.dCache;
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
		font->showText(DEBUGOUT_X_POS-20, debugOff, "Sync debug:", FONT_LATIN_SMALL_YELLOW);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		if (Server)
		{
			font->showText(DEBUGOUT_X_POS-10, debugOff, "-Server:", FONT_LATIN_SMALL_YELLOW);
			debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
			font->showText(DEBUGOUT_X_POS, debugOff, "Server Time: ", FONT_LATIN_SMALL_WHITE);
			font->showText(DEBUGOUT_X_POS + 110, debugOff, iToStr(Server->gameTimer.gameTime), FONT_LATIN_SMALL_WHITE);
			debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

			font->showText(DEBUGOUT_X_POS, debugOff, "Net MSG Queue: ", FONT_LATIN_SMALL_WHITE);
			font->showText(DEBUGOUT_X_POS + 110, debugOff, iToStr(Server->eventQueue.size()), FONT_LATIN_SMALL_WHITE);
			debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

			font->showText(DEBUGOUT_X_POS, debugOff, "EventCounter: ", FONT_LATIN_SMALL_WHITE);
			font->showText(DEBUGOUT_X_POS + 110, debugOff, iToStr(Server->gameTimer.eventCounter), FONT_LATIN_SMALL_WHITE);
			debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);


			font->showText(DEBUGOUT_X_POS, debugOff, "-Client Lag: ", FONT_LATIN_SMALL_WHITE);
			debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

			for (unsigned int i = 0; i < Server->PlayerList->Size(); i++)
			{
				eUnicodeFontType fontType = FONT_LATIN_SMALL_WHITE;
				if (Server->gameTimer.getReceivedTime(i)+PAUSE_GAME_TIMEOUT<Server->gameTimer.gameTime)
					fontType = FONT_LATIN_SMALL_RED;
				font->showText(DEBUGOUT_X_POS+10, debugOff, "Client " + iToStr(i) + ": ", fontType);
				font->showText(DEBUGOUT_X_POS + 110, debugOff, iToStr(Server->gameTimer.gameTime - Server->gameTimer.getReceivedTime(i)), fontType);
				debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
			}
		}

		font->showText(DEBUGOUT_X_POS-10, debugOff, "-Client:", FONT_LATIN_SMALL_YELLOW);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		eUnicodeFontType fontType = FONT_LATIN_SMALL_GREEN;
		if (Client->gameTimer.debugRemoteChecksum != Client->gameTimer.localChecksum)
			fontType = FONT_LATIN_SMALL_RED;
		font->showText(DEBUGOUT_X_POS, debugOff, "Server Checksum: ", FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS + 110, debugOff, "0x" + iToHex(Client->gameTimer.debugRemoteChecksum), fontType);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText(DEBUGOUT_X_POS, debugOff, "Client Checksum: ", FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS + 110, debugOff, "0x" + iToHex(Client->gameTimer.localChecksum), fontType);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText(DEBUGOUT_X_POS, debugOff, "Client Time: ", FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS + 110, debugOff, iToStr(Client->gameTimer.gameTime), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText(DEBUGOUT_X_POS, debugOff, "Net MGS Queue: ", FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS + 110, debugOff, iToStr(EventHandler->eventQueue.size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText(DEBUGOUT_X_POS, debugOff, "EventCounter: ", FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS + 110, debugOff, iToStr(Client->gameTimer.eventCounter), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText(DEBUGOUT_X_POS, debugOff, "Time Buffer: ", FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS + 110, debugOff, iToStr(Client->gameTimer.getReceivedTime()-Client->gameTimer.gameTime), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText(DEBUGOUT_X_POS, debugOff, "Ticks per Frame ", FONT_LATIN_SMALL_WHITE);
		static unsigned int lastGameTime = 0;
		font->showText(DEBUGOUT_X_POS + 110, debugOff, iToStr(Client->gameTimer.gameTime-lastGameTime), FONT_LATIN_SMALL_WHITE);
		lastGameTime = Client->gameTimer.gameTime;
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText(DEBUGOUT_X_POS, debugOff, "Time Adjustment: ", FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS + 110, debugOff, iToStr(Client->gameTimer.gameTimeAdjustment), FONT_LATIN_SMALL_WHITE);
		static int totalAdjust = 0;
		totalAdjust += Client->gameTimer.gameTimeAdjustment;
		Client->gameTimer.gameTimeAdjustment = 0;
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);

		font->showText(DEBUGOUT_X_POS, debugOff, "TotalAdj.: ", FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS + 110, debugOff, iToStr(totalAdjust), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight (FONT_LATIN_SMALL_WHITE);
		
	}
}


void cDebugOutput::trace()
{
	cMapField* field;

	int x = mouse->getKachelX();
	int y = mouse->getKachelY();
	if (x < 0 || y < 0) return;

	if (debugTraceServer) field = &Server->Map->fields[Server->Map->size * y + x];
	else field = &Client->Map->fields[Client->Map->size * y + x];

	y = 18 + 5 + 8;
	x = 180 + 5;

	if (field->getVehicles()) { traceVehicle (field->getVehicles(), &y, x); y += 20; }
	if (field->getPlanes()) { traceVehicle (field->getPlanes(), &y, x); y += 20; }
	cBuildingIterator bi = field->getBuildings();
	while (!bi.end) { traceBuilding (bi, &y, x); y += 20; bi++;}
}

void cDebugOutput::traceVehicle (cVehicle* vehicle, int* y, int x)
{
	string tmpString;

	tmpString = "name: \"" + vehicle->getDisplayName() + "\" id: \"" + iToStr (vehicle->iID) + "\" owner: \"" + vehicle->owner->name + "\" posX: +" + iToStr (vehicle->PosX) + " posY: " + iToStr (vehicle->PosY) + " offX: " + iToStr (vehicle->OffX) + " offY: " + iToStr (vehicle->OffY);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "dir: " + iToStr (vehicle->dir) + " moving: +" + iToStr (vehicle->moving) + " mjob: "  + pToStr (vehicle->ClientMoveJob) + " speed: " + iToStr (vehicle->data.speedCur) + " mj_active: " + iToStr (vehicle->MoveJobActive);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = " attacking: " + iToStr (vehicle->attacking) + " on sentry: +" + iToStr (vehicle->sentryActive) + " ditherx: " + iToStr (vehicle->ditherX) + " dithery: " + iToStr (vehicle->ditherY);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "is_building: " + iToStr (vehicle->IsBuilding) + " building_typ: " + vehicle->BuildingTyp.getText() + " build_costs: +" + iToStr (vehicle->BuildCosts) + " build_rounds: " + iToStr (vehicle->BuildRounds) + " build_round_start: " + iToStr (vehicle->BuildRoundsStart);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = " bandx: " + iToStr (vehicle->BandX) + " bandy: +" + iToStr (vehicle->BandY) + " build_big_saved_pos: " + iToStr (vehicle->BuildBigSavedPos) + " build_path: " + iToStr (vehicle->BuildPath);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = " is_clearing: " + iToStr (vehicle->IsClearing) + " clearing_rounds: +" + iToStr (vehicle->ClearingRounds) + " clear_big: " + iToStr (vehicle->data.isBig) + " loaded: " + iToStr (vehicle->Loaded);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "commando_rank: " + dToStr (Round (vehicle->CommandoRank, 2)) + " disabled: " + iToStr (vehicle->turnsDisabled);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "is_locked: " + iToStr (vehicle->IsLocked) + " clear_mines: +" + iToStr (vehicle->ClearMines) + " lay_mines: " + iToStr (vehicle->LayMines);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString =
		" vehicle_to_activate: +"  + iToStr (vehicle->VehicleToActivate) +
		" stored_vehicles_count: " + iToStr ( (int) vehicle->storedUnits.Size());
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	if (vehicle->storedUnits.Size())
	{
		cUnit* storedVehicle;
		for (unsigned int i = 0; i < vehicle->storedUnits.Size(); i++)
		{
			storedVehicle = vehicle->storedUnits[i];
			font->showText (x, *y, " store " + iToStr (i) + ": \"" + storedVehicle->getDisplayName() + "\"", FONT_LATIN_SMALL_WHITE);
			*y += 8;
		}
	}

	if (debugTraceServer)
	{
		tmpString = "seen by players: owner";
		for (unsigned int i = 0; i < vehicle->seenByPlayerList.Size(); i++)
		{
			tmpString += ", \"" + vehicle->seenByPlayerList[i]->name + "\"";
		}
		font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
		*y += 8;
	}

	tmpString = "flight height: " + iToStr (vehicle->FlightHigh);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;
}

void cDebugOutput::traceBuilding (cBuilding* building, int* y, int x)
{
	string tmpString;

	tmpString = "name: \"" + building->getDisplayName() + "\" id: \"" + iToStr (building->iID) + "\" owner: \"" + (building->owner ? building->owner->name : "<null>") + "\" posX: +" + iToStr (building->PosX) + " posY: " + iToStr (building->PosY);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "dir: " + iToStr (building->dir) + " on sentry: +" + iToStr (building->sentryActive) + " sub_base: " + pToStr (building->SubBase);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "attacking: " + iToStr (building->attacking) + " UnitsData.dirt_typ: " + iToStr (building->RubbleTyp) + " UnitsData.dirt_value: +" + iToStr (building->RubbleValue) + " big_dirt: " + iToStr (building->data.isBig) + " is_working: " + iToStr (building->IsWorking);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = " max_metal_p: " + iToStr (building->MaxMetalProd) + " max_oil_p: " + iToStr (building->MaxOilProd) + " max_gold_p: " + iToStr (building->MaxGoldProd);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString = "is_locked: " + iToStr (building->IsLocked) + " disabled: " + iToStr (building->turnsDisabled) + " vehicle_to_activate: " + iToStr (building->VehicleToActivate);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	tmpString =
		" stored_vehicles_count: " + iToStr ( (int) building->storedUnits.Size());
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	if (building->storedUnits.Size())
	{
		cUnit* storedVehicle;
		for (unsigned int i = 0; i < building->storedUnits.Size(); i++)
		{
			storedVehicle = building->storedUnits[i];
			font->showText (x, *y, " store " + iToStr (i) + ": \"" + storedVehicle->getDisplayName() + "\"", FONT_LATIN_SMALL_WHITE);
			*y += 8;
		}
	}

	tmpString =
		"build_speed: "        + iToStr (building->BuildSpeed)  +
		" repeat_build: "      + iToStr (building->RepeatBuild) +
		" build_list_count: +" + iToStr (building->BuildList ? (int) building->BuildList->Size() : 0);
	font->showText (x, *y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y += 8;

	if (building->BuildList && building->BuildList->Size())
	{
		sBuildList* BuildingList;
		for (unsigned int i = 0; i < building->BuildList->Size(); i++)
		{
			BuildingList = (*building->BuildList) [i];
			font->showText (x, *y, "  build " + iToStr (i) + ": " + BuildingList->type.getText() + " \"" + BuildingList->type.getVehicle()->data.name + "\"", FONT_LATIN_SMALL_WHITE);
			*y += 8;
		}
	}

	if (debugTraceServer)
	{
		tmpString = "seen by players: owner";
		for (unsigned int i = 0; i < building->seenByPlayerList.Size(); i++)
		{
			tmpString += ", \"" + building->seenByPlayerList[i]->name + "\"";
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


cGameGUI::cGameGUI (cPlayer* player_, cMap* map_, cList<cPlayer*>* const playerList) :
	cMenu (generateSurface()),
	iTimerTime(0),
	client (NULL),
	player (player_),
	map (map_),
	miniMapOffX (0),
	miniMapOffY (0),
	msgCoordsX (-1),
	msgCoordsY (-1),
	shiftPressed (false),
	overUnitField (NULL),
	zoomSlider (20, 274, calcMinZoom(), 1.0, this, 130, cMenuSlider::SLIDER_TYPE_HUD_ZOOM, cMenuSlider::SLIDER_DIR_RIGHTMIN),
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
	selUnitNameEdit (12, 30, 123, 10, this, FONT_LATIN_SMALL_GREEN, cMenuLineEdit::LE_TYPE_JUST_TEXT)
{
	unitMenuActive = false;
	frame = 0;
	zoom = 1.0;
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

	selectedVehicle = NULL;
	selectedBuilding = NULL;

	calcMinZoom();

	setWind (random (360));

	closed = false;

	zoomSlider.setMoveCallback (&zoomSliderMoved);
	menuItems.Add (&zoomSlider);
	menuItems.Add (zoomSlider.scroller);

	endButton.setReleasedFunction (&endReleased);
	menuItems.Add (&endButton);

	preferencesButton.setReleasedFunction (&preferencesReleased);
	menuItems.Add (&preferencesButton);
	filesButton.setReleasedFunction (&filesReleased);
	menuItems.Add (&filesButton);

	playButton.setReleasedFunction (&playReleased);
	menuItems.Add (&playButton);
	stopButton.setReleasedFunction (&stopReleased);
	menuItems.Add (&stopButton);

	FLCImage.setReleasedFunction(&swithAnimationReleased);
	menuItems.Add (&FLCImage);

	menuItems.Add (&unitDetails);

	// generate checkbuttons
	menuItems.Add (&surveyButton);
	menuItems.Add (&hitsButton);
	menuItems.Add (&scanButton);
	menuItems.Add (&statusButton);
	menuItems.Add (&ammoButton);
	menuItems.Add (&gridButton);
	menuItems.Add (&colorButton);
	menuItems.Add (&rangeButton);
	menuItems.Add (&fogButton);
	menuItems.Add (&lockButton);

	TNTButton.setClickedFunction (&changedMiniMap);
	menuItems.Add (&TNTButton);
	twoXButton.setClickedFunction (&twoXReleased);
	menuItems.Add (&twoXButton);
	playersButton.setClickedFunction (&playersReleased);
	menuItems.Add (&playersButton);

	helpButton.setReleasedFunction (&helpReleased);
	menuItems.Add (&helpButton);
	centerButton.setReleasedFunction (&centerReleased);
	menuItems.Add (&centerButton);

	reportsButton.setReleasedFunction (&reportsReleased);
	menuItems.Add (&reportsButton);
	chatButton.setReleasedFunction (&chatReleased);
	menuItems.Add (&chatButton);

	nextButton.setReleasedFunction (&nextReleased);
	menuItems.Add (&nextButton);
	prevButton.setReleasedFunction (&prevReleased);
	menuItems.Add (&prevButton);
	doneButton.setReleasedFunction (&doneReleased);
	menuItems.Add (&doneButton);

	miniMapImage.setClickedFunction (&miniMapClicked);
	miniMapImage.setRightClickedFunction (&miniMapRightClicked);
	menuItems.Add (&miniMapImage);

	coordsLabel.setCentered (true);
	menuItems.Add (&coordsLabel);

	unitNameLabel.setCentered (true);
	menuItems.Add (&unitNameLabel);

	turnLabel.setCentered (true);
	menuItems.Add (&turnLabel);

	timeLabel.setCentered (true);
	menuItems.Add (&timeLabel);

	chatBox.setDisabled (true);
	chatBox.setReturnPressedFunc (&chatBoxReturnPressed);
	menuItems.Add (&chatBox);

	infoTextLabel.setCentered (true);
	infoTextLabel.setDisabled (true);
	menuItems.Add (&infoTextLabel);

	infoTextAdditionalLabel.setCentered (true);
	infoTextAdditionalLabel.setDisabled (true);
	menuItems.Add (&infoTextAdditionalLabel);

	menuItems.Add (&selUnitStatusStr);
	menuItems.Add (&selUnitNamePrefixStr);

	selUnitNameEdit.setReturnPressedFunc (unitNameReturnPressed);
	menuItems.Add (&selUnitNameEdit);

	for (size_t i = 0; i < playerList->Size(); i++)
	{
		const int xPos = Video.getResolutionY() >= 768 ? 3 : 161;
		const int yPos = Video.getResolutionY() >= 768 ? (482 + GraphicsData.gfx_hud_extra_players->h * i) : (480 - 82 - GraphicsData.gfx_hud_extra_players->h * i);

		cMenuPlayerInfo* playerInfo = new cMenuPlayerInfo (xPos, yPos, (*playerList) [i]);
		playerInfo->setDisabled (true);
		playersInfo.Add (playerInfo);

		menuItems.Add (playerInfo);
	}

	updateTurn (1);
}

void cGameGUI::setClient (cClient* client)
{
	this->client = client;
}

float cGameGUI::calcMinZoom()
{
	minZoom = (float) ( (max (Video.getResolutionY() - HUD_TOTAL_HIGHT, Video.getResolutionX() - HUD_TOTAL_WIDTH) / (float) map->size) / 64.0);
	minZoom = max (minZoom, ( (int) (64.0 * minZoom) + (minZoom >= 1.0 ? 0 : 1)) / (float) 64.0);

	return minZoom;
}

void cGameGUI::recalcPosition (bool resetItemPositions)
{
	background = generateSurface();
	cMenu::recalcPosition (resetItemPositions);

	// reset minimal zoom
	calcMinZoom();
	setZoom (zoom, true, false);
	zoomSlider.setBorders (minZoom, 1.0);

	// move some items around
	coordsLabel.move (coordsLabel.getPosition().x, (Video.getResolutionY() - 21) + 3);
	unitNameLabel.move (unitNameLabel.getPosition().x, (Video.getResolutionY() - 21) + 3);
	chatBox.move (chatBox.getPosition().x, Video.getResolutionY() - 48);
	infoTextLabel.move (HUD_LEFT_WIDTH + (Video.getResolutionX() - HUD_TOTAL_WIDTH) / 2, infoTextLabel.getPosition().y);
	infoTextAdditionalLabel.move (HUD_LEFT_WIDTH + (Video.getResolutionX() - HUD_TOTAL_WIDTH) / 2, 235 + font->getFontHeight (FONT_LATIN_BIG));
}

cGameGUI::~cGameGUI()
{
	zoom = 1.0;
	scaleSurfaces();
	SDL_RemoveTimer (TimerID);

	if (FLC) FLI_Close (FLC);
	for (size_t i = 0; i != playersInfo.Size(); ++i)
	{
		delete playersInfo[i];
	}
	for (size_t i = 0; i != messages.Size(); ++i)
	{
		delete messages[i];
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
	int iHeight = 0;
	sMessage* message;
	if (messages.Size() == 0) return;
	//delete old messages
	for (int i = (int) messages.Size() - 1; i >= 0; i--)
	{
		message = messages[i];
		if (message->age + MSG_TICKS < SDL_GetTicks() || iHeight > 200)
		{
			delete message;
			messages.Delete (i);
			continue;
		}
		iHeight += 17 + font->getFontHeight() * (message->len  / (Video.getResolutionX() - 300));
	}
}

void cGameGUI::addMessage (const string& sMsg)
{
	sMessage* const Message = new sMessage (sMsg, SDL_GetTicks());
	messages.Add (Message);
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

int cGameGUI::show()
{
	drawnEveryFrame = true;

	cMenu* lastActiveMenu = ActiveMenu;
	ActiveMenu = this;

	// do startup actions
	makePanel (true);
	startup = true;
	if (client->isFreezed ()) setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_Until", client->getPlayerFromNumber (0)->name), "");

	int lastMouseX = 0, lastMouseY = 0;

	while (!end)
	{
		EventHandler->HandleEvents();
		Client->gameTimer.run ();

		mouse->GetPos();
		if (mouse->moved())
		{
			handleMouseMove();

			for (unsigned int i = 0; i < menuItems.Size(); i++)
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
			if (player->BuildingList) player->BuildingList->center();
			else if (player->VehicleList) player->VehicleList->center();
			startup = false;
		}

		handleMessages();

		handleTimer();
		if (timer50ms)
		{
			//run effects
			Client->runFX();
			Client->handleTurnTime();
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

		if (terminate)
		{
			EventHandler->HandleEvents();	//flush event queue before exiting menu

			if (lastActiveMenu) lastActiveMenu->returnToCallback();
			return 1;
		}
	}

	// code to work with DEDICATED_SERVER - network client sends the server, that it disconnects itself
	if (Server == 0)
	{
		cNetMessage* message = new cNetMessage (GAME_EV_WANT_DISCONNECT);
		client->sendNetMessage (message);
	}
	// end


	makePanel (false);

	EventHandler->HandleEvents();	//flush event queue before exiting menu

	if (lastActiveMenu) lastActiveMenu->returnToCallback();
	return 0;
}

void cGameGUI::returnToCallback()
{
	ActiveMenu = this;
}

void cGameGUI::updateInfoTexts ()
{
	int playerNumber = Client->getFreezeInfoPlayerNumber ();
	cPlayer* player = Client->getPlayerFromNumber (playerNumber);

	if (Client->getFreezeMode(FREEZE_WAIT_FOR_OTHERS))
	{
		setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_Until", player->name), "");
	}
	else if (Client->getFreezeMode(FREEZE_PAUSE))
	{
		setInfoTexts (lngPack.i18n ("Text~Multiplayer~Pause"), "");
	}
	else if (Client->getFreezeMode(FREEZE_WAIT_FOR_PLAYER))
	{
		setInfoTexts (lngPack.i18n ("Text~Multiplayer~No_Response", player->name),"");
	}
	else if (Client->getFreezeMode(FREEZE_WAIT_FOR_RECONNECT))
	{
		setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_Reconnect"), Server ? lngPack.i18n ("Text~Multiplayer~Abort_Waiting") : "");
	}
	else if (Client->getFreezeMode(FREEZE_WAIT_FOR_TURNEND))
	{
		setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_TurnEnd"), "");
	}
	else if (Client->getFreezeMode(FREEZE_WAIT_FOR_SERVER))
	{
		setInfoTexts (lngPack.i18n ("Text~Multiplayer~Wait_For_Server"),"");
	}
	else 
	{
		setInfoTexts("","");
	}
}

SDL_Surface* cGameGUI::generateSurface()
{
	SDL_Rect scr, dest;
	SDL_Surface* surface = SDL_CreateRGBSurface (SDL_HWSURFACE, Video.getResolutionX(), Video.getResolutionY(), Video.getColDepth(), 0, 0, 0, 0);

	SDL_FillRect (surface, NULL, 0xFF00FF);
	SDL_SetColorKey (surface, SDL_SRCCOLORKEY, 0xFF00FF);

	{
		AutoSurface tmpSurface (LoadPCX (cSettings::getInstance().getGfxPath() + PATH_DELIMITER + "hud_left.pcx"));
		if (tmpSurface)
		{
			SDL_BlitSurface (tmpSurface, NULL, surface, NULL);
		}
	}

	{
		AutoSurface tmpSurface (LoadPCX (cSettings::getInstance().getGfxPath() + PATH_DELIMITER + "hud_top.pcx"));
		if (tmpSurface)
		{
			scr.x = 0;
			scr.y = 0;
			scr.w = tmpSurface->w;
			scr.h = tmpSurface->h;
			dest.x = HUD_LEFT_WIDTH;
			dest.y = 0;
			SDL_BlitSurface (tmpSurface, &scr, surface, &dest);
			scr.x = 1275;
			scr.w = 18;
			scr.h = HUD_TOP_HIGHT;
			dest.x = surface->w - HUD_TOP_HIGHT;
			SDL_BlitSurface (tmpSurface, &scr, surface, &dest);
		}
	}

	{
		AutoSurface tmpSurface (LoadPCX (cSettings::getInstance().getGfxPath() + PATH_DELIMITER + "hud_right.pcx"));
		if (tmpSurface)
		{
			scr.x = 0;
			scr.y = 0;
			scr.w = tmpSurface->w;
			scr.h = tmpSurface->h;
			dest.x = surface->w - HUD_RIGHT_WIDTH;
			dest.y = HUD_TOP_HIGHT;
			SDL_BlitSurface (tmpSurface, &scr, surface, &dest);
		}
	}

	{
		AutoSurface tmpSurface (LoadPCX (cSettings::getInstance().getGfxPath() + PATH_DELIMITER + "hud_bottom.pcx"));
		if (tmpSurface)
		{
			scr.x = 0;
			scr.y = 0;
			scr.w = tmpSurface->w;
			scr.h = tmpSurface->h;
			dest.x = HUD_LEFT_WIDTH;
			dest.y = surface->h - 24;
			SDL_BlitSurface (tmpSurface, &scr, surface, &dest);
			scr.x = 1275;
			scr.w = 23;
			scr.h = 24;
			dest.x = surface->w - 23;
			SDL_BlitSurface (tmpSurface, &scr, surface, &dest);
			scr.x = 1299;
			scr.w = 16;
			scr.h = 22;
			dest.x = HUD_LEFT_WIDTH - 16;
			dest.y = surface->h - 22;
			SDL_BlitSurface (tmpSurface, &scr, surface, &dest);
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
	Uint32* minimap = ( (Uint32*) minimapSurface->pixels);

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

		if (miniMapOffX < 0) miniMapOffX = 0;
		if (miniMapOffY < 0) miniMapOffY = 0;
		if (miniMapOffX > map->size - (map->size / zoomFactor)) miniMapOffX = map->size - (map->size / zoomFactor);
		if (miniMapOffY > map->size - (map->size / zoomFactor)) miniMapOffY = map->size - (map->size / zoomFactor);
	}

	//draw the landscape
	for (int miniMapX = 0; miniMapX < MINIMAP_SIZE; miniMapX++)
	{
		//calculate the field on the map
		int terrainx = (miniMapX * map->size) / (MINIMAP_SIZE * zoomFactor) + miniMapOffX;
		if (terrainx >= map->size) terrainx = map->size - 1;

		//calculate the position within the terrain graphic (for better rendering of maps < 112)
		int offsetx  = ( (miniMapX * map->size) % (MINIMAP_SIZE * zoomFactor)) * 64 / (MINIMAP_SIZE * zoomFactor);

		for (int miniMapY = 0; miniMapY < MINIMAP_SIZE; miniMapY++)
		{
			int terrainy = (miniMapY * map->size) / (MINIMAP_SIZE * zoomFactor) + miniMapOffY;
			if (terrainy >= map->size) terrainy = map->size - 1;
			int offsety  = ( (miniMapY * map->size) % (MINIMAP_SIZE * zoomFactor)) * 64 / (MINIMAP_SIZE * zoomFactor);

			SDL_Color sdlcolor;
			Uint8* terrainPixels = (Uint8*) map->terrain[map->Kacheln[terrainx + terrainy * map->size]].sf_org->pixels;
			Uint8 index = terrainPixels[offsetx + offsety * 64];
			sdlcolor = map->terrain[map->Kacheln[terrainx + terrainy * map->size]].sf_org->format->palette->colors[index];
			Uint32 color = (sdlcolor.r << 16) + (sdlcolor.g << 8) + sdlcolor.b;

			minimap[miniMapX + miniMapY * MINIMAP_SIZE] = color;
		}
	}

	if (player)
	{
		//draw the fog
		for (int miniMapX = 0; miniMapX < MINIMAP_SIZE; miniMapX++)
		{
			int terrainx = (miniMapX * map->size) / (MINIMAP_SIZE * zoomFactor) + miniMapOffX;
			for (int miniMapY = 0; miniMapY < MINIMAP_SIZE; miniMapY++)
			{
				int terrainy = (miniMapY * map->size) / (MINIMAP_SIZE * zoomFactor) + miniMapOffY;

				if (!player->ScanMap[terrainx + terrainy * map->size])
				{
					Uint8* color = (Uint8*) &minimap[miniMapX + miniMapY * MINIMAP_SIZE];
					color[0] = (Uint8) (color[0] * 0.6);
					color[1] = (Uint8) (color[1] * 0.6);
					color[2] = (Uint8) (color[2] * 0.6);
				}
			}
		}

		//draw the units
		//here we go through each map field instead of through each minimap pixel,
		//to make sure, that every unit is diplayed and has the same size on the minimap.

		//the size of the rect, that is drawn for each unit
		int size = (int) ceil ( (float) MINIMAP_SIZE * zoomFactor / map->size);
		if (size < 2) size = 2;
		SDL_Rect rect;
		rect.h = size;
		rect.w = size;

		for (int mapx = 0; mapx < map->size; mapx++)
		{
			rect.x = ( (mapx - miniMapOffX) * MINIMAP_SIZE * zoomFactor) / map->size;
			if (rect.x < 0 || rect.x >= MINIMAP_SIZE) continue;
			for (int mapy = 0; mapy < map->size; mapy++)
			{
				rect.y = ( (mapy - miniMapOffY) * MINIMAP_SIZE * zoomFactor) / map->size;
				if (rect.y < 0 || rect.y >= MINIMAP_SIZE) continue;

				if (!player->ScanMap[mapx + mapy * map->size]) continue;

				cMapField& field = (*map) [mapx + mapy * map->size];

				//draw building
				cBuilding* building = field.getBuildings();
				if (building && building->owner)
				{
					if (!tntChecked() || building->data.canAttack)
					{
						unsigned int color = * ( (unsigned int*) building->owner->color->pixels);
						SDL_FillRect (minimapSurface, &rect, color);
					}
				}

				//draw vehicle
				cVehicle* vehicle = field.getVehicles();
				if (vehicle)
				{
					if (!tntChecked() || vehicle->data.canAttack)
					{
						unsigned int color = * ( (unsigned int*) vehicle->owner->color->pixels);
						SDL_FillRect (minimapSurface, &rect, color);
					}
				}

				//draw plane
				vehicle = field.getPlanes();
				if (vehicle)
				{
					if (!tntChecked() || vehicle->data.canAttack)
					{
						unsigned int color = * ( (unsigned int*) vehicle->owner->color->pixels);
						SDL_FillRect (minimapSurface, &rect, color);
					}
				}
			}
		}
	}


	//draw the screen borders
	int startx, starty, endx, endy;
	startx = (int) ( ( ( (offX / 64.0) - miniMapOffX) * MINIMAP_SIZE * zoomFactor) / map->size);
	starty = (int) ( ( ( (offY / 64.0) - miniMapOffY) * MINIMAP_SIZE * zoomFactor) / map->size);
	endx = (int) (startx + ( (Video.getResolutionX() - HUD_TOTAL_WIDTH) * MINIMAP_SIZE * zoomFactor) / (map->size * (getZoom() * 64.0)));
	endy = (int) (starty + ( (Video.getResolutionY() -  HUD_TOTAL_HIGHT) * MINIMAP_SIZE * zoomFactor) / (map->size * (getZoom() * 64.0)));

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

	int maxX = map->size * 64 - (int) ( (Video.getResolutionX() - HUD_TOTAL_WIDTH) / getZoom());
	int maxY = map->size * 64 - (int) ( (Video.getResolutionY() - HUD_TOTAL_HIGHT) / getZoom());
	offX = min (offX, maxX);
	offY = min (offY, maxY);

	callMiniMapDraw();
	updateMouseCursor();
}

void cGameGUI::setZoom (float newZoom, bool setScroller, bool centerToMouse)
{
	zoom = newZoom;
	if (zoom < minZoom) zoom = minZoom;
	if (zoom > 1.0) zoom = 1.0;

	if (setScroller) this->zoomSlider.setValue (zoom);

	static float lastZoom = 1.0;
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
		lastScreenPixel = (Video.getResolutionY() - HUD_TOTAL_HIGHT) / lastZoom ;
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
	return (getTileSize() / (float) 64.0);
}

int cGameGUI::getTileSize() const
{
	return Round (64.0 * zoom);
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
	unitDetails.setSelection (vehicle, building);

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
	int xPosition = 12 + font->getTextWide (selUnitNamePrefixStr.getText() + " ", FONT_LATIN_SMALL_GREEN);
	selUnitNameEdit.move (xPosition, 30);
	selUnitNameEdit.setSize (135 - xPosition, 10);
}

void cGameGUI::updateTurn (int turn)
{
	turnLabel.setText (iToStr (turn));
}

void cGameGUI::updateTurnTime (int time)
{
	if (time < 0) timeLabel.setText ("");
	else timeLabel.setText (iToStr (time));
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
		float a = 0.5f;	//low pass filter coefficient

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
	int x = mouse->getKachelX();
	int y = mouse->getKachelY();

	if (x < 0 || y < 0 || x >= map->size || y >= map->size) return;

	// draw the coordinates:
	/*array to get map coords in sceme XXX-YYY\0 = 8 characters
	a case where I accept an array since I don't know a better
	method to format x and y easily with leading 0 -- beko */
	char str[8];
	sprintf (str, "%.3d-%.3d", x, y);
	coordsLabel.setText (str);

	if (!player->ScanMap[x + y * map->size])
	{
		overUnitField = NULL;
		if (mouse->cur == GraphicsData.gfx_Cattack)
		{
			SDL_Rect r;
			r.x = 1; r.y = 29;
			r.h = 3; r.w = 35;
			SDL_FillRect (GraphicsData.gfx_Cattack, &r, 0);
		}
		return;
	}
	// check wether there is a unit under the mouse:
	overUnitField = map->fields + (x + y * map->size);
	if (mouse->cur == GraphicsData.gfx_Csteal && selectedVehicle)
	{
		selectedVehicle->drawCommandoCursor (x, y, true);
	}
	else if (mouse->cur == GraphicsData.gfx_Cdisable && selectedVehicle)
	{
		selectedVehicle->drawCommandoCursor (x, y, false);
	}
	if (overUnitField->getVehicles() != NULL)
	{
		//FIXME: displaying ownername to unit name may cause an overdraw on the infobox. This needs either a seperate infobox or a length check in the future. that goes for unitnames itself too. -- beko
		unitNameLabel.setText (overUnitField->getVehicles()->getDisplayName() + " (" + overUnitField->getVehicles()->owner->name + ")");
		if (mouse->cur == GraphicsData.gfx_Cattack)
		{
			if (selectedVehicle)
			{
				selectedVehicle->DrawAttackCursor (x, y);
			}
			else if (selectedBuilding)
			{
				selectedBuilding->DrawAttackCursor (x, y);
			}
		}
	}
	else if (overUnitField->getPlanes() != NULL)
	{
		unitNameLabel.setText (overUnitField->getPlanes()->getDisplayName() + " (" + overUnitField->getPlanes()->owner->name + ")");
		if (mouse->cur == GraphicsData.gfx_Cattack)
		{
			if (selectedVehicle)
			{
				selectedVehicle->DrawAttackCursor (x, y);
			}
			else if (selectedBuilding)
			{
				selectedBuilding->DrawAttackCursor (x, y);
			}
		}
	}
	else if (overUnitField->getTopBuilding() != NULL)
	{
		unitNameLabel.setText (overUnitField->getTopBuilding()->getDisplayName() + " (" + overUnitField->getTopBuilding()->owner->name + ")");
		if (mouse->cur == GraphicsData.gfx_Cattack)
		{
			if (selectedVehicle)
			{
				selectedVehicle->DrawAttackCursor (x, y);
			}
			else if (selectedBuilding)
			{
				selectedBuilding->DrawAttackCursor (x, y);
			}
		}
	}
	else if (overUnitField->getBaseBuilding() && overUnitField->getBaseBuilding()->owner)
	{
		unitNameLabel.setText (overUnitField->getBaseBuilding()->getDisplayName() + " (" + overUnitField->getBaseBuilding()->owner->name + ")");
		if (mouse->cur == GraphicsData.gfx_Cattack)
		{
			if (selectedVehicle)
			{
				selectedVehicle->DrawAttackCursor (x, y);
			}
			else if (selectedBuilding)
			{
				selectedBuilding->DrawAttackCursor (x, y);
			}
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
		selectedVehicle->FindNextband();
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

void cGameGUI::selectUnit (cVehicle* vehicle)
{
	if (vehicle->Loaded)
		return;

	deselectUnit();

	selectedVehicle = vehicle;
	selectedBuilding = NULL;

	unitMenuActive = false;
	mouseInputMode = normalInput;

	vehicle->Select();
	client->iObjectStream = vehicle->playStream();

	updateMouseCursor();
}

void cGameGUI::selectUnit (cBuilding* building)
{
	deselectUnit();

	selectedBuilding = building;
	selectedVehicle = NULL;

	unitMenuActive = false;
	mouseInputMode = normalInput;

	building->Select();
	client->iObjectStream = building->playStream();

	updateMouseCursor();
}

void cGameGUI::deselectUnit()
{
	if (selectedBuilding)
	{
		selectedBuilding->Deselct();
		selectedBuilding = NULL;
		StopFXLoop (client->iObjectStream);
	}
	else if (selectedVehicle)
	{
		selectedVehicle->Deselct();
		selectedVehicle = NULL;
		StopFXLoop (client->iObjectStream);
	}

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

	int x = mouse->x;
	int y = mouse->y;

	for (unsigned int i = 0; i < menuItems.Size(); i++)
	{
		if (menuItems[i]->overItem (x, y) && !menuItems[i]->isDisabled())
		{
			mouse->SetCursor (CHand);
			return;
		}
	}

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
	else if ( (selectedVehicle && mouseInputMode == transferMode && selectedVehicle->owner == client->getActivePlayer()) || (selectedBuilding && mouseInputMode == transferMode && selectedBuilding->owner == client->getActivePlayer()))
	{
		if (selectedVehicle)
		{
			if (overUnitField && selectedVehicle->CanTransferTo (overUnitField))
			{
				mouse->SetCursor (CTransf);
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else
		{
			if (overUnitField && selectedBuilding->CanTransferTo (overUnitField))
			{
				mouse->SetCursor (CTransf);
			}
			else
			{
				mouse->SetCursor (CNo);
			}
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

		if ( (selectedVehicle && unitMenuActive && selectedVehicle->areCoordsOverMenu (x, y)) ||
			 (selectedBuilding && unitMenuActive && selectedBuilding->areCoordsOverMenu (x, y)))
		{
			mouse->SetCursor (CHand);
		}
		else if (selectedVehicle && mouseInputMode == mouseInputAttackMode && selectedVehicle->owner == client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT)
		{
			if (! (selectedVehicle->data.muzzleType == sUnitData::MUZZLE_TYPE_TORPEDO && !client->getMap()->isWater (mouse->getKachelX(), mouse->getKachelY())))
			{
				if (mouse->SetCursor (CAttack))
				{
					selectedVehicle->DrawAttackCursor (mouse->getKachelX(), mouse->getKachelY());
				}
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else if (selectedVehicle && mouseInputMode == disableMode && selectedVehicle->owner == client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT)
		{
			if (selectedVehicle->canDoCommandoAction (mouse->getKachelX(), mouse->getKachelY(), client->getMap(), false))
			{
				if (mouse->SetCursor (CDisable))
				{
					selectedVehicle->drawCommandoCursor (mouse->getKachelX(), mouse->getKachelY(), false);
				}
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else if (selectedVehicle && mouseInputMode == stealMode && selectedVehicle->owner == client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT)
		{
			if (selectedVehicle->canDoCommandoAction (mouse->getKachelX(), mouse->getKachelY(), client->getMap(), true))
			{
				if (mouse->SetCursor (CSteal))
				{
					selectedVehicle->drawCommandoCursor (mouse->getKachelX(), mouse->getKachelY(), true);
				}
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT && selectedVehicle->canDoCommandoAction (mouse->getKachelX(), mouse->getKachelY(), client->getMap(), false) && (!overUnitField->getVehicles() || !overUnitField->getVehicles()->turnsDisabled))
		{
			if (mouse->SetCursor (CDisable))
			{
				selectedVehicle->drawCommandoCursor (mouse->getKachelX(), mouse->getKachelY(), false);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT && selectedVehicle->canDoCommandoAction (mouse->getKachelX(), mouse->getKachelY(), client->getMap(), true))
		{
			if (mouse->SetCursor (CSteal))
			{
				selectedVehicle->drawCommandoCursor (mouse->getKachelX(), mouse->getKachelY(), true);
			}
		}
		else if (selectedBuilding && mouseInputMode == mouseInputAttackMode && selectedBuilding->owner == client->getActivePlayer() && x >= HUD_LEFT_WIDTH && y >= HUD_TOP_HIGHT && x < Video.getResolutionX() - HUD_RIGHT_WIDTH && y < Video.getResolutionY() - HUD_BOTTOM_HIGHT)
		{
			if (selectedBuilding->isInRange (mouse->getKachelX(), mouse->getKachelY()))
			{
				if (mouse->SetCursor (CAttack))
				{
					selectedBuilding->DrawAttackCursor (mouse->getKachelX(), mouse->getKachelY());
				}
			}
			else
			{
				mouse->SetCursor (CNo);
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == client->getActivePlayer() && selectedVehicle->canAttackObjectAt (mouse->getKachelX(), mouse->getKachelY(), client->getMap(), false, false))
		{
			if (mouse->SetCursor (CAttack))
			{
				selectedVehicle->DrawAttackCursor (mouse->getKachelX(), mouse->getKachelY());
			}
		}
		else if (selectedBuilding && selectedBuilding->owner == client->getActivePlayer() && selectedBuilding->canAttackObjectAt (mouse->getKachelX(), mouse->getKachelY(), client->getMap()))
		{
			if (mouse->SetCursor (CAttack))
			{
				selectedBuilding->DrawAttackCursor (mouse->getKachelX(), mouse->getKachelY());
			}
		}
		else if (selectedVehicle && selectedVehicle->owner == client->getActivePlayer() && mouseInputMode == muniActive)
		{
			if (selectedVehicle->canSupply (mouse->getKachelX(), mouse->getKachelY(), SUPPLY_TYPE_REARM))
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
			if (selectedVehicle->canSupply (mouse->getKachelX(), mouse->getKachelY(), SUPPLY_TYPE_REPAIR))
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
					 overUnitField->getVehicles() ||
					 overUnitField->getPlanes() ||
					 (
						 overUnitField->getBuildings() &&
						 overUnitField->getBuildings()->owner
					 )
				 ) &&
				 (
					 !selectedVehicle                               ||
					 selectedVehicle->owner != client->getActivePlayer() ||
					 (
						 (
							 selectedVehicle->data.factorAir > 0 ||
							 overUnitField->getVehicles() ||
							 (
								 overUnitField->getTopBuilding() &&
								 overUnitField->getTopBuilding()->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE
							 ) ||
							 (
								 MouseStyle == OldSchool &&
								 overUnitField->getPlanes()
							 )
						 ) &&
						 (
							 selectedVehicle->data.factorAir == 0 ||
							 overUnitField->getPlanes() ||
							 (
								 MouseStyle == OldSchool &&
								 (
									 overUnitField->getVehicles() ||
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
							 !selectedBuilding->BuildList->Size()            ||
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
			if (selectedVehicle->canLoad (mouse->getKachelX(), mouse->getKachelY(), client->getMap(), false))
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
			if (selectedVehicle->canExitTo (mouse->getKachelX(), mouse->getKachelY(), client->getMap(), selectedVehicle->storedUnits[selectedVehicle->VehicleToActivate]->typ))
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
				else if (client->getMap()->possiblePlace (selectedVehicle, mouse->getKachelX(), mouse->getKachelY(), true))
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
					 client->getMap()->possiblePlace (selectedVehicle, mouse->getKachelX(), mouse->getKachelY()) && selectedVehicle->isNextTo (mouse->getKachelX(), mouse->getKachelY()))
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
			selectedBuilding->BuildList->Size()             &&
			!selectedBuilding->IsWorking                    &&
			(*selectedBuilding->BuildList) [0]->metall_remaining <= 0)
		{
			if (selectedBuilding->canExitTo (mouse->getKachelX(), mouse->getKachelY(), client->getMap(), (*selectedBuilding->BuildList) [0]->type.getVehicle()))
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
			if (selectedBuilding->canExitTo (mouse->getKachelX(), mouse->getKachelY(), client->getMap(), selectedBuilding->storedUnits[selectedBuilding->VehicleToActivate]->typ))
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
			if (selectedBuilding->canLoad (mouse->getKachelX(), mouse->getKachelY(), client->getMap(), false))
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
		mouseBox.endX = (float) ( ( (mouse->x - HUD_LEFT_WIDTH) + (offX * getZoom())) / getTileSize());
		mouseBox.endY = (float) ( ( (mouse->y - HUD_TOP_HIGHT) + (offY * getZoom())) / getTileSize());
	}
	if (savedMouseState.rightButtonPressed && !savedMouseState.leftButtonPressed && rightMouseBox.startX != -1 && mouse->x > HUD_LEFT_WIDTH)
	{
		rightMouseBox.endX = (float) ( ( (mouse->x - HUD_LEFT_WIDTH) + (offX * getZoom())) / getTileSize());
		rightMouseBox.endY = (float) ( ( (mouse->y - HUD_TOP_HIGHT) + (offY * getZoom())) / getTileSize());
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
	for (unsigned int i = 0; i < menuItems.Size(); i++)
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
		overVehicle  = overUnitField->getVehicles();
		overPlane    = overUnitField->getPlanes();
		overBuilding = overUnitField->getTopBuilding();
		overBaseBuilding = overUnitField->getBaseBuilding();
	}

	if (selectedVehicle && unitMenuActive && selectedVehicle->areCoordsOverMenu (mouseState.x, mouseState.y))
	{
		if (mouseState.leftButtonPressed) selectedVehicle->setMenuSelection();
		else if (mouseState.leftButtonReleased && !mouseState.rightButtonPressed) selectedVehicle->menuReleased (*this);
		return;
	}
	else if (selectedBuilding && unitMenuActive && selectedBuilding->areCoordsOverMenu (mouseState.x, mouseState.y))
	{
		if (mouseState.leftButtonPressed) selectedBuilding->setMenuSelection();
		else if (mouseState.leftButtonReleased && !mouseState.rightButtonPressed) selectedBuilding->menuReleased (*this);
		return;
	}

	// handle input on the map
	if (MouseStyle == OldSchool && mouseState.rightButtonReleased && !mouseState.leftButtonPressed && overUnitField)
	{
		if ( (overVehicle && overVehicle == selectedVehicle) || (overPlane && overPlane == selectedVehicle))
		{
			cUnitHelpMenu helpMenu (&selectedVehicle->data, selectedVehicle->owner);
			helpMenu.show();
		}
		else if ( (overBuilding && overBuilding == selectedBuilding) || (overBaseBuilding && overBaseBuilding == selectedBuilding))
		{
			cUnitHelpMenu helpMenu (&selectedBuilding->data, selectedBuilding->owner);
			helpMenu.show();
		}
		else if (overUnitField) selectUnit (overUnitField, true);
	}
	else
	{
		bool mouseOverSelectedUnit = false;
		if (selectedBuilding && mouse->getKachelX() == selectedBuilding->PosX && mouse->getKachelY() == selectedBuilding->PosY) mouseOverSelectedUnit = true;
		if (selectedBuilding && selectedBuilding->data.isBig && mouse->getKachelX() >= selectedBuilding->PosX && mouse->getKachelX() <= selectedBuilding->PosX + 1
			&& mouse->getKachelY() >= selectedBuilding->PosY && mouse->getKachelY() <= selectedBuilding->PosY + 1) mouseOverSelectedUnit = true;
		if (selectedVehicle && mouse->getKachelX() == selectedVehicle->PosX && mouse->getKachelY() == selectedVehicle->PosY) mouseOverSelectedUnit = true;
		if (selectedVehicle && selectedVehicle->data.isBig && mouse->getKachelX() >= selectedVehicle->PosX && mouse->getKachelX() <= selectedVehicle->PosX + 1
			&& mouse->getKachelY() >= selectedVehicle->PosY && mouse->getKachelY() <= selectedVehicle->PosY + 1) mouseOverSelectedUnit = true;


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
					cVehicleIterator planes = overUnitField->getPlanes();
					int next = 0;

					if (selectedVehicle)
					{
						if (planes.contains (*selectedVehicle))
						{
							while (!planes.end)
							{
								if (planes == selectedVehicle)
									break;
								planes++;
							}
							planes++;

							if (!planes.end) next = 'p';
							else if (overVehicle) next = 'v';
							else if (overBuilding) next = 't';
							else if (overBaseBuilding) next = 'b';
							else if (planes.size() > 1)
							{
								next = 'p';
								planes.rewind();
							}
						}
						else
						{
							if (overBuilding) next = 't';
							else if (overBaseBuilding) next = 'b';
							else if (overPlane) next = 'p';
						}
					}
					else if (selectedBuilding)
					{
						if (overBuilding == selectedBuilding)
						{
							if (overBaseBuilding) next = 'b';
							else if (overPlane) next = 'p';
							else if (overUnitField->getVehicles()) next = 'v';
						}
						else
						{
							if (overPlane) next = 'p';
							else if (overUnitField->getVehicles()) next = 'v';
							else if (overBuilding) next = 't';
						}
					}

					deselectUnit();

					switch (next)
					{
						case 't':
							selectUnit (overBuilding);
							break;
						case 'b':
							selectUnit (overBaseBuilding);
							break;
						case 'v':
							selectUnit (overVehicle);
							break;
						case 'p':
							selectUnit (planes);
							break;
						default:
							break;
					}
				}
				else
				{
					deselectUnit();
				}
			}
		}
	}

	if (mouseState.rightButtonReleased && !mouseState.leftButtonPressed)
	{
		rightMouseBox.startX = rightMouseBox.startY = -1;
		rightMouseBox.endX = rightMouseBox.endY = -1;
	}
	else if (mouseState.rightButtonPressed && !mouseState.leftButtonPressed && rightMouseBox.startX == -1 && mouseState.x > HUD_LEFT_WIDTH && mouseState.y > 20)
	{
		rightMouseBox.startX = (float) ( ( (mouseState.x - HUD_LEFT_WIDTH) + (offX * getZoom())) / getTileSize());
		rightMouseBox.startY = (float) ( ( (mouseState.y - HUD_TOP_HIGHT) + (offY * getZoom())) / getTileSize());
	}
	if (mouseState.leftButtonReleased && !mouseState.rightButtonPressed)
	{
		// Store the currently selected unit to determine if the lock state of the clicked unit maybe has to be changed.
		// If the selected unit changes during the click handling, then the newly selected unit has to be added / removed from the "locked units" list.
		cVehicle* oldSelectedVehicleForLock = selectedVehicle;
		cBuilding* oldSelectedBuildingForLock = selectedBuilding;

		if (!mouseBox.isTooSmall())
		{
			selectBoxVehicles (mouseBox);
		}
		else if (changeAllowed && selectedVehicle && mouse->cur == GraphicsData.gfx_Ctransf)
		{
			if (overVehicle)
			{
				cDialogTransfer transferDialog (NULL, selectedVehicle, NULL, overVehicle);
				transferDialog.show();
			}
			else if (overBuilding)
			{
				cDialogTransfer transferDialog (NULL, selectedVehicle, overBuilding, NULL);
				transferDialog.show();
			}
		}
		else if (changeAllowed && selectedBuilding && mouse->cur == GraphicsData.gfx_Ctransf)
		{
			if (overVehicle)
			{
				cDialogTransfer transferDialog (selectedBuilding, NULL, NULL, overVehicle);
				transferDialog.show();
			}
			else if (overBuilding)
			{
				cDialogTransfer transferDialog (selectedBuilding, NULL, overBuilding, NULL);
				transferDialog.show();
			}
		}
		else if (changeAllowed && selectedVehicle && mouseInputMode == placeBand && mouse->cur == GraphicsData.gfx_Cband)
		{
			mouseInputMode = normalInput;

			if (selectedVehicle->BuildingTyp.getUnitDataOriginalVersion()->isBig)
			{
				sendWantBuild (selectedVehicle->iID, selectedVehicle->BuildingTyp, selectedVehicle->BuildRounds, selectedVehicle->BandX + selectedVehicle->BandY * map->size, false, 0);
			}
			else
			{
				sendWantBuild (selectedVehicle->iID, selectedVehicle->BuildingTyp, selectedVehicle->BuildRounds, selectedVehicle->PosX + selectedVehicle->PosY * map->size, true, selectedVehicle->BandX + selectedVehicle->BandY * map->size);
			}
		}
		else if (changeAllowed && mouse->cur == GraphicsData.gfx_Cactivate && selectedBuilding && mouseInputMode == activateVehicle)
		{
			sendWantActivate (selectedBuilding->iID, false, selectedBuilding->storedUnits[selectedBuilding->VehicleToActivate]->iID, mouse->getKachelX(), mouse->getKachelY());
			updateMouseCursor();
		}
		else if (changeAllowed && mouse->cur == GraphicsData.gfx_Cactivate && selectedVehicle && mouseInputMode == activateVehicle)
		{
			sendWantActivate (selectedVehicle->iID, true, selectedVehicle->storedUnits[selectedVehicle->VehicleToActivate]->iID, mouse->getKachelX(), mouse->getKachelY());
			updateMouseCursor();
		}
		else if (changeAllowed && mouse->cur == GraphicsData.gfx_Cactivate && selectedBuilding && selectedBuilding->BuildList && selectedBuilding->BuildList->Size())
		{
			sendWantExitFinishedVehicle (selectedBuilding, mouse->getKachelX(), mouse->getKachelY());
		}
		else if (changeAllowed && mouse->cur == GraphicsData.gfx_Cload && selectedBuilding && mouseInputMode == loadMode)
		{
			if (overVehicle && selectedBuilding->canLoad (overVehicle, false))
			{
				if (selectedBuilding->isNextTo (overVehicle->PosX, overVehicle->PosY)) sendWantLoad (selectedBuilding->iID, false, overVehicle->iID);
				else
				{
					cPathCalculator pc (overVehicle->PosX, overVehicle->PosY, NULL, selectedBuilding, client->getMap(), overVehicle, true);
					sWaypoint* path = pc.calcPath();
					if (path)
					{
						sendMoveJob (path, overVehicle->iID);
						sendEndMoveAction (overVehicle->iID, selectedBuilding->iID, EMAT_GET_IN);
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
				if (selectedBuilding->isNextTo (overPlane->PosX, overPlane->PosY)) sendWantLoad (selectedBuilding->iID, false, overPlane->iID);
				else
				{
					cPathCalculator pc (overPlane->PosX, overPlane->PosY, NULL, selectedBuilding, client->getMap(), overPlane, true);
					sWaypoint* path = pc.calcPath();
					if (path)
					{
						sendMoveJob (path, overPlane->iID);
						sendEndMoveAction (overPlane->iID, selectedBuilding->iID, EMAT_GET_IN);
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
				if (overVehicle->PosX == selectedVehicle->PosX && overVehicle->PosY == selectedVehicle->PosY) sendWantLoad (selectedVehicle->iID, true, overVehicle->iID);
				else
				{
					cPathCalculator pc (selectedVehicle->PosX, selectedVehicle->PosY, overVehicle->PosX, overVehicle->PosY, client->getMap(), selectedVehicle);
					sWaypoint* path = pc.calcPath();
					if (path)
					{
						sendMoveJob (path, selectedVehicle->iID);
						sendEndMoveAction (selectedVehicle->iID, overVehicle->iID, EMAT_LOAD);
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
				if (selectedVehicle->isNextTo (overVehicle->PosX, overVehicle->PosY)) sendWantLoad (selectedVehicle->iID, true, overVehicle->iID);
				else
				{
					cPathCalculator pc (overVehicle->PosX, overVehicle->PosY, selectedVehicle, NULL, client->getMap(), overVehicle, true);
					sWaypoint* path = pc.calcPath();
					if (path)
					{
						sendMoveJob (path, overVehicle->iID);
						sendEndMoveAction (overVehicle->iID, selectedVehicle->iID, EMAT_GET_IN);
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
			if (overVehicle) sendWantSupply (overVehicle->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REARM);
			else if (overPlane && overPlane->FlightHigh == 0) sendWantSupply (overPlane->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REARM);
			else if (overBuilding) sendWantSupply (overBuilding->iID, false, selectedVehicle->iID, true, SUPPLY_TYPE_REARM);
		}
		else if (changeAllowed && mouse->cur == GraphicsData.gfx_Crepair && selectedVehicle && mouseInputMode == repairActive)
		{
			if (overVehicle) sendWantSupply (overVehicle->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REPAIR);
			else if (overPlane && overPlane->FlightHigh == 0) sendWantSupply (overPlane->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REPAIR);
			else if (overBuilding) sendWantSupply (overBuilding->iID, false, selectedVehicle->iID, true, SUPPLY_TYPE_REPAIR);
		}
		else if (!helpActive)
		{
			//Hud.CheckButtons();
			// check whether the mouse is over an unit menu:
			if ( (selectedVehicle && unitMenuActive && selectedVehicle->areCoordsOverMenu (mouseState.x, mouseState.y)) ||
				 (selectedBuilding && unitMenuActive && selectedBuilding->areCoordsOverMenu (mouseState.x, mouseState.y)))
			{
			}
			else
				// check, if the player wants to attack:
				if (changeAllowed && mouse->cur == GraphicsData.gfx_Cattack && selectedVehicle && !selectedVehicle->attacking && !selectedVehicle->MoveJobActive)
				{
					cVehicle* vehicle;
					cBuilding* building;
					selectTarget (vehicle, building, mouse->getKachelX(), mouse->getKachelY(), selectedVehicle->data.canAttack, client->getMap());

					if (selectedVehicle->isInRange (mouse->getKachelX(), mouse->getKachelY()))
					{
						//find target ID
						int targetId = 0;
						if (vehicle) targetId = vehicle->iID;

						Log.write (" Client: want to attack " + iToStr (mouse->getKachelX()) + ":" + iToStr (mouse->getKachelY()) + ", Vehicle ID: " + iToStr (targetId), cLog::eLOG_TYPE_NET_DEBUG);
						sendWantAttack (targetId, mouse->getKachelX() + mouse->getKachelY() *client->getMap()->size, selectedVehicle->iID, true);
					}
					else if (vehicle || building)
					{
						cPathCalculator pc (selectedVehicle->PosX, selectedVehicle->PosY, client->getMap(), selectedVehicle, mouse->getKachelX(), mouse->getKachelY());
						sWaypoint* path = pc.calcPath();
						if (path)
						{
							sendMoveJob (path, selectedVehicle->iID);
							sendEndMoveAction (selectedVehicle->iID, vehicle ? vehicle->iID : building->iID, EMAT_ATTACK);
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
					cVehicle* vehicle;
					cBuilding* building;
					selectTarget (vehicle, building, mouse->getKachelX(), mouse->getKachelY(), selectedBuilding->data.canAttack, client->getMap());
					if (vehicle) targetId = vehicle->iID;

					int offset = selectedBuilding->PosX + selectedBuilding->PosY * map->size;
					sendWantAttack (targetId, mouse->getKachelX() + mouse->getKachelY() *client->getMap()->size, offset, false);
				}
				else if (changeAllowed && mouse->cur == GraphicsData.gfx_Csteal && selectedVehicle)
				{
					if (overVehicle) sendWantComAction (selectedVehicle->iID, overVehicle->iID, true, true);
					else if (overPlane && overPlane->FlightHigh == 0) sendWantComAction (selectedVehicle->iID, overVehicle->iID, true, true);
				}
				else if (changeAllowed && mouse->cur == GraphicsData.gfx_Cdisable && selectedVehicle)
				{
					if (overVehicle) sendWantComAction (selectedVehicle->iID, overVehicle->iID, true, false);
					else if (overPlane && overPlane->FlightHigh == 0) sendWantComAction (selectedVehicle->iID, overPlane->iID, true, false);
					else if (overBuilding) sendWantComAction (selectedVehicle->iID, overBuilding->iID, false, false);
				}
				else if (MouseStyle == OldSchool && overUnitField && selectUnit (overUnitField, false))
				{}
				else if (changeAllowed && mouse->cur == GraphicsData.gfx_Cmove && selectedVehicle && !selectedVehicle->moving && !selectedVehicle->attacking)
				{
					if (selectedVehicle->IsBuilding)
					{
						sendWantEndBuilding (selectedVehicle, mouse->getKachelX(), mouse->getKachelY());
					}
					else
					{
						if (selectedVehiclesGroup.Size() > 1) client->startGroupMove();
						else client->addMoveJob (selectedVehicle, mouse->getKachelX(), mouse->getKachelY());
					}
				}
				else if (overUnitField)
				{
					// open unit menu
					if (changeAllowed && selectedVehicle && (overUnitField->getPlanes().contains (*selectedVehicle) || overVehicle == selectedVehicle))
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
				cUnitHelpMenu helpMenu (&overPlane->data, overPlane->owner);
				helpMenu.show();
			}
			else if (overVehicle)
			{
				cUnitHelpMenu helpMenu (&overVehicle->data, overVehicle->owner);
				helpMenu.show();
			}
			else if (overBuilding)
			{
				cUnitHelpMenu helpMenu (&overBuilding->data, overBuilding->owner);
				helpMenu.show();
			}
			else if (overBaseBuilding)
			{
				cUnitHelpMenu helpMenu (&overBaseBuilding->data, overBaseBuilding->owner);
				helpMenu.show();
			}
			helpActive = false;
		}

		// toggle the lock state of an enemy unit, if it is newly selected / deselected
		if (overUnitField && lockChecked())
		{
			if (selectedVehicle && selectedVehicle != oldSelectedVehicleForLock && selectedVehicle->owner != player)
				player->ToggelLock (overUnitField);
			else if (selectedBuilding && selectedBuilding != oldSelectedBuildingForLock && selectedBuilding->owner != player)
				player->ToggelLock (overUnitField);
		}

		mouseBox.startX = mouseBox.startY = -1;
		mouseBox.endX = mouseBox.endY = -1;
	}
	else if (mouseState.leftButtonPressed && !mouseState.rightButtonPressed && mouseBox.startX == -1 && mouseState.x > HUD_LEFT_WIDTH && mouseState.y > 20)
	{
		mouseBox.startX = (float) ( ( (mouseState.x - HUD_LEFT_WIDTH) + (offX * getZoom())) / getTileSize());
		mouseBox.startY = (float) ( ( (mouseState.y - HUD_TOP_HIGHT) + (offY * getZoom())) / getTileSize());
	}

	// check getZoom() via mousewheel
	if (mouseState.wheelUp)
	{
		setZoom (getZoom() + (float) 0.05, true, true);
	}
	else if (mouseState.wheelDown)
	{
		setZoom (getZoom() - (float) 0.05, true, true);
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
	if (cmd.compare ("/fps on") == 0) { debugOutput.showFPS = true;}
	else if (cmd.compare ("/fps off") == 0) { debugOutput.showFPS = false;}
	else if (cmd.compare ("/base client") == 0) { debugOutput.debugBaseClient = true; debugOutput.debugBaseServer = false;}
	else if (cmd.compare ("/base server") == 0) { if (Server) debugOutput.debugBaseServer = true; debugOutput.debugBaseClient = false;}
	else if (cmd.compare ("/base off") == 0) { debugOutput.debugBaseServer = false; debugOutput.debugBaseClient = false;}
	else if (cmd.compare ("/sentry server") == 0) { if (Server) debugOutput.debugSentry = true;}
	else if (cmd.compare ("/sentry off") == 0) { debugOutput.debugSentry = false;}
	else if (cmd.compare ("/fx on") == 0) { debugOutput.debugFX = true;}
	else if (cmd.compare ("/fx off") == 0) { debugOutput.debugFX = false;}
	else if (cmd.compare ("/trace server") == 0) { if (Server) debugOutput.debugTraceServer = true; debugOutput.debugTraceClient = false;}
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
			addMessage("Wrong parameter");
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
		if (!Server)
		{
			addMessage("Command can only be used by Host");
			return;
		}
		if (cmd.length() <= 6)
		{
			addMessage("Wrong parameter");
			return;
		}
		cPlayer* Player = Server->getPlayerFromString (cmd.substr (6, cmd.length()));

		// server can not be kicked
		if ( !Player || Player->Nr == 0) 
		{
			addMessage("Wrong parameter");
			return;
		}

		// close the socket
		if (network) network->close (Player->iSocketNum);
		for (unsigned int i = 0; i < Server->PlayerList->Size(); i++)
		{
			if ( (*Server->PlayerList) [i]->iSocketNum > Player->iSocketNum && (*Server->PlayerList) [i]->iSocketNum < MAX_CLIENTS) (*Server->PlayerList) [i]->iSocketNum--;
		}
		// delete the player
		Server->deletePlayer (Player);
	}
	else if (cmd.substr (0, 9).compare ("/credits ") == 0)
	{
		if (!Server)
		{
			addMessage("Command can only be used by Host");
			return;
		}
		if (cmd.length() <= 9)
		{
			addMessage("Wrong parameter");
			return;
		}

		string playerStr = cmd.substr (9, cmd.find_first_of (" ", 9) - 9);
		string creditsStr = cmd.substr (cmd.find_first_of (" ", 9) + 1, cmd.length());

		cPlayer* Player = Server->getPlayerFromString (playerStr);

		if (!Player)
		{
			addMessage("Wrong parameter");
			return;
		}

		int credits = atoi (creditsStr.c_str());

		Player->Credits = credits;

		sendCredits (credits, Player->Nr);
	}
	else if (cmd.substr (0, 12).compare ("/disconnect ") == 0)
	{
		if (!Server)
		{
			addMessage("Command can only be used by Host");
			return;
		}
		if (cmd.length() <= 12)
		{
			addMessage("Wrong parameter");
			return;
		}

		cPlayer* Player = Server->getPlayerFromString (cmd.substr (12, cmd.length()));

		//server cannot be disconnected
		//can not disconnect local players
		if (!player || Player->Nr == 0 || Player->iSocketNum == MAX_CLIENTS)
		{
			addMessage("Wrong parameter");
			return;
		}

		cNetMessage* message = new cNetMessage (TCP_CLOSE);
		message->pushInt16 (Player->iSocketNum);
		Server->pushEvent (message);
	}
	else if (cmd.substr (0, 9).compare ("/deadline") == 0)
	{
		if (!Server)
		{
			addMessage("Command can only be used by Host");
			return;
		}
		if (cmd.length() <= 9)
		{
			addMessage("Wrong parameter");
			return;
		}

		int i = atoi (cmd.substr (9, cmd.length()).c_str());
		if ( i == 0 && cmd[10] != '0')
		{
			addMessage("Wrong parameter");
			return;
		}

		Server->setDeadline (i);
		Log.write ("Deadline changed to "  + iToStr (i) , cLog::eLOG_TYPE_INFO);
	}
	else if (cmd.substr (0, 7).compare ("/resync") == 0)
	{
		if (cmd.length() > 7)
		{
			if (!Server)
			{
				addMessage("Command can only be used by Host");
				return;
			}
			cPlayer* player = Client->getPlayerFromString(cmd.substr (7, 8));
			if (!player)
			{
				addMessage("Wrong parameter");
				return;
			}
			sendRequestResync (player->Nr);
		}
		else
		{
			if (Server)
			{
				for (unsigned int i = 0; i < Server->PlayerList->Size(); i++)
				{
					sendRequestResync ( (*Server->PlayerList) [i]->Nr);
				}
			}
			else
			{
				sendRequestResync (client->getActivePlayer()->Nr);
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
		int cl = 0; sscanf (cmd.c_str(), "color %d", &cl); cl %= 8; player->color = OtherData.colors[cl];
	}
	else if (cmd.compare ("/fog off") == 0)
	{
		if (!Server)
		{
			addMessage("Command can only be used by Host");
			return;
		}
		memset (Server->getPlayerFromNumber (player->Nr)->ScanMap, 1, map->size * map->size);
		memset (player->ScanMap, 1, map->size * map->size);
	}
	else if (cmd.compare ("/survey") == 0)
	{
		if (!Server)
		{
			addMessage("Command can only be used by Host");
			return;
		}
		memcpy (map->Resources , Server->Map->Resources, map->size * map->size * sizeof (sResources));
		memset (player->ResourceMap, 1, map->size * map->size);
	}
	else if (cmd.substr (0, 6).compare ("/pause") == 0)
	{
		if (!Server)
		{
			addMessage("Command can only be used by Host");
			return;
		}
		Server->enableFreezeMode(FREEZE_PAUSE);
	}
	else if (cmd.substr (0, 7).compare ("/resume") == 0)
	{
		if (!Server)
		{
			addMessage("Command can only be used by Host");
			return;
		}
		Server->disableFreezeMode(FREEZE_PAUSE);
	}
	else
	{
		addMessage("Unknown command");
	}
}

void cGameGUI::setWind (int dir)
{
	windDir = (float) (dir / 57.29577);
}

bool cGameGUI::selectUnit (cMapField* OverUnitField, bool base)
{
	deselectGroup();
	if (OverUnitField->getPlanes() && !OverUnitField->getPlanes()->moving)
	{
		// TOFIX: add that the unit renaming will be aborted here when active
		if (selectedVehicle == OverUnitField->getPlanes())
		{
			if (selectedVehicle->owner == player)
			{
				unitMenuActive = !unitMenuActive;
				PlayFX (SoundData.SNDHudButton);
			}
		}
		else
		{
			selectUnit (OverUnitField->getPlanes());
		}
		return true;
	}
	else if (OverUnitField->getVehicles() && !OverUnitField->getVehicles()->moving && ! (OverUnitField->getPlanes() && (unitMenuActive || OverUnitField->getVehicles()->owner != player)))
	{
		// TOFIX: add that the unit renaming will be aborted here when active
		if (selectedVehicle == OverUnitField->getVehicles())
		{
			if (selectedVehicle->owner == player)
			{
				unitMenuActive = !unitMenuActive;
				PlayFX (SoundData.SNDHudButton);
			}
		}
		else
		{
			selectUnit (OverUnitField->getVehicles());
		}
		return true;
	}
	else if (OverUnitField->getTopBuilding() && (base || ( (OverUnitField->getTopBuilding()->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE || !selectedVehicle) && (!OverUnitField->getTopBuilding()->data.canBeLandedOn || (!selectedVehicle || selectedVehicle->data.factorAir == 0)))))
	{
		// TOFIX: add that the unit renaming will be aborted here when active
		if (selectedBuilding == OverUnitField->getTopBuilding())
		{
			if (selectedBuilding->owner == player)
			{
				unitMenuActive = !unitMenuActive;
				PlayFX (SoundData.SNDHudButton);
			}
		}
		else
		{
			selectUnit (OverUnitField->getTopBuilding());
		}
		return true;
	}
	else if ( (base || !selectedVehicle) && OverUnitField->getBaseBuilding() && OverUnitField->getBaseBuilding()->owner != NULL)
	{
		// TOFIX: add that the unit renaming will be aborted here when active
		if (selectedBuilding == OverUnitField->getBaseBuilding())
		{
			if (selectedBuilding->owner == player)
			{
				unitMenuActive = !unitMenuActive;
				PlayFX (SoundData.SNDHudButton);
			}
		}
		else
		{
			selectUnit (OverUnitField->getBaseBuilding());
		}
		return true;
	}
	return false;
}

void cGameGUI::selectBoxVehicles (sMouseBox& box)
{
	if (box.startX < 0 || box.startY < 0 || box.endX < 0 || box.endY < 0) return;
	int startFieldX, startFieldY;
	int endFieldX, endFieldY;
	startFieldX = (int) min (box.startX, box.endX);
	startFieldY = (int) min (box.startY, box.endY);
	endFieldX = (int) max (box.startX, box.endX);
	endFieldY = (int) max (box.startY, box.endY);

	deselectGroup();

	bool newSelected = true;
	for (int x = startFieldX; x <= endFieldX; x++)
	{
		for (int y = startFieldY; y <= endFieldY; y++)
		{
			int offset = x + y * map->size;

			cVehicle* vehicle;
			vehicle = (*map) [offset].getVehicles();
			if (!vehicle || vehicle->owner != player) vehicle = (*map) [offset].getPlanes();

			if (vehicle && vehicle->owner == player && !vehicle->IsBuilding && !vehicle->IsClearing && !vehicle->moving)
			{
				if (vehicle == selectedVehicle)
				{
					newSelected = false;
					selectedVehiclesGroup.Insert (0, vehicle);
				}
				else selectedVehiclesGroup.Add (vehicle);
				vehicle->groupSelected = true;
			}
		}
	}
	if (newSelected && selectedVehiclesGroup.Size() > 0)
	{
		selectUnit (selectedVehiclesGroup[0]);
	}
	if (selectedVehiclesGroup.Size() == 1)
	{
		selectedVehiclesGroup[0]->groupSelected = false;
		selectedVehiclesGroup.Delete (0);
	}
}

void cGameGUI::updateStatusText()
{
	if (selectedVehicle) selUnitStatusStr.setText (selectedVehicle->getStatusStr());
	else if (selectedBuilding) selUnitStatusStr.setText (selectedBuilding->getStatusStr());
	else selUnitStatusStr.setText ("");
}

void cGameGUI::deselectGroup()
{
	for (size_t i = 0; i != selectedVehiclesGroup.Size(); ++i)
	{
		selectedVehiclesGroup[i]->groupSelected = false;
	}
	selectedVehiclesGroup.Clear();
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
		sendAbortWaiting();
	}

	if (key.keysym.sym == KeysList.KeyExit)
	{
		cDialogYesNo yesNoDialog (lngPack.i18n ("Text~Comp~End_Game"));
		if (yesNoDialog.show() == 0) end = true;
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
	else if (key.keysym.sym == KeysList.KeyCenterUnit && selectedVehicle) selectedVehicle->center();
	else if (key.keysym.sym == KeysList.KeyCenterUnit && selectedBuilding) selectedBuilding->center();
	else if (key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_ALT) savePosition (0);
	else if (key.keysym.sym == SDLK_F6 && key.keysym.mod & KMOD_ALT) savePosition (1);
	else if (key.keysym.sym == SDLK_F7 && key.keysym.mod & KMOD_ALT) savePosition (2);
	else if (key.keysym.sym == SDLK_F8 && key.keysym.mod & KMOD_ALT) savePosition (3);
	else if (key.keysym.sym == SDLK_F5 && savedPositions[0].offsetX >= 0 && savedPositions[0].offsetY >= 0) jumpToSavedPos (0);
	else if (key.keysym.sym == SDLK_F6 && savedPositions[1].offsetX >= 0 && savedPositions[1].offsetY >= 0) jumpToSavedPos (1);
	else if (key.keysym.sym == SDLK_F7 && savedPositions[2].offsetX >= 0 && savedPositions[2].offsetY >= 0) jumpToSavedPos (2);
	else if (key.keysym.sym == SDLK_F8 && savedPositions[3].offsetX >= 0 && savedPositions[3].offsetY >= 0) jumpToSavedPos (3);
	// Hotkeys for the unit menues
	else if (key.keysym.sym == KeysList.KeyUnitMenuAttack && selectedVehicle && selectedVehicle->data.canAttack && selectedVehicle->data.shotsCur && !client->isFreezed () && selectedVehicle->owner == player)
	{
		mouseInputMode = mouseInputAttackMode;
		updateMouseCursor();
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuAttack && selectedBuilding && selectedBuilding->data.canAttack && selectedBuilding->data.shotsCur && !client->isFreezed () && selectedBuilding->owner == player)
	{
		mouseInputMode = mouseInputAttackMode;
		updateMouseCursor();
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuBuild && selectedVehicle && !selectedVehicle->data.canBuild.empty() && !selectedVehicle->IsBuilding && !client->isFreezed () && selectedVehicle->owner == player)
	{
		sendWantStopMove (selectedVehicle->iID);
		cBuildingsBuildMenu buildMenu (player, selectedVehicle);
		buildMenu.show();
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuBuild && selectedBuilding && !selectedBuilding->data.canBuild.empty() && !client->isFreezed () && selectedBuilding->owner == player)
	{
		cVehiclesBuildMenu buildMenu (player, selectedBuilding);
		buildMenu.show();
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
		for (unsigned int i = 1; i < selectedVehiclesGroup.Size(); i++)
		{
			selectedVehiclesGroup[i]->executeAutoMoveJobCommand();
		}
		selectedVehicle->executeAutoMoveJobCommand();
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuStart && selectedBuilding && selectedBuilding->data.canWork && !selectedBuilding->IsWorking && ( (selectedBuilding->BuildList && selectedBuilding->BuildList->Size()) || selectedBuilding->data.canBuild.empty()) && !client->isFreezed () && selectedBuilding->owner == player)
	{
		sendWantStartWork (selectedBuilding);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuStop && selectedVehicle && (selectedVehicle->ClientMoveJob || (selectedVehicle->IsBuilding && selectedVehicle->BuildRounds) || (selectedVehicle->IsClearing && selectedVehicle->ClearingRounds)) && !client->isFreezed () && selectedVehicle->owner == player)
	{
		if (selectedVehicle->ClientMoveJob)
		{
			for (unsigned int i = 1; i < selectedVehiclesGroup.Size(); i++)
			{
				if (selectedVehiclesGroup[i]->ClientMoveJob) sendWantStopMove (selectedVehiclesGroup[i]->iID);
			}
			sendWantStopMove (selectedVehicle->iID);
		}
		else if (selectedVehicle->IsBuilding)
		{
			for (unsigned int i = 1; i < selectedVehiclesGroup.Size(); i++)
			{
				if (selectedVehiclesGroup[i]->IsBuilding && selectedVehiclesGroup[i]->BuildRounds) sendWantStopBuilding (selectedVehiclesGroup[i]->iID);
			}
			sendWantStopBuilding (selectedVehicle->iID);
		}
		else if (selectedVehicle->IsClearing)
		{
			for (unsigned int i = 1; i < selectedVehiclesGroup.Size(); i++)
			{
				if (selectedVehiclesGroup[i]->IsClearing && selectedVehiclesGroup[i]->ClearingRounds) sendWantStopClear (selectedVehiclesGroup[i]);
			}
			sendWantStopClear (selectedVehicle);
		}
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuStop && selectedBuilding && selectedBuilding->IsWorking && !client->isFreezed () && selectedBuilding->owner == player)
	{
		sendWantStopWork (selectedBuilding);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuClear && selectedVehicle && selectedVehicle->data.canClearArea && map->fields[selectedVehicle->PosX + selectedVehicle->PosY * map->size].getRubble() && !selectedVehicle->IsClearing && !client->isFreezed () && selectedVehicle->owner == player)
	{
		for (unsigned int i = 1; i < selectedVehiclesGroup.Size(); i++)
		{
			if (selectedVehiclesGroup[i]->data.canClearArea && map->fields[selectedVehiclesGroup[i]->PosX + selectedVehiclesGroup[i]->PosY * map->size].getRubble() && !selectedVehiclesGroup[i]->IsClearing) sendWantStartClear (selectedVehiclesGroup[i]);
		}
		sendWantStartClear (selectedVehicle);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuSentry && selectedVehicle && !client->isFreezed () && selectedVehicle->owner == player)
	{
		for (unsigned int i = 1; i < selectedVehiclesGroup.Size(); i++)
		{
			if (selectedVehicle->sentryActive == selectedVehiclesGroup[i]->sentryActive)
			{
				sendChangeSentry (selectedVehiclesGroup[i]->iID, true);
			}
		}
		sendChangeSentry (selectedVehicle->iID, true);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuSentry && selectedBuilding && (selectedBuilding->sentryActive || selectedBuilding->data.canAttack) && !client->isFreezed () && selectedBuilding->owner == player)
	{
		sendChangeSentry (selectedBuilding->iID, false);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuManualFire && selectedVehicle && (selectedVehicle->manualFireActive || selectedVehicle->data.canAttack) && !client->isFreezed () && selectedVehicle->owner == player)
	{
		for (unsigned int i = 1; i < selectedVehiclesGroup.Size(); i++)
		{
			if ( (selectedVehiclesGroup[i]->manualFireActive || selectedVehiclesGroup[i]->data.canAttack)
				 && selectedVehicle->manualFireActive == selectedVehiclesGroup[i]->manualFireActive)
			{
				sendChangeManualFireStatus (selectedVehiclesGroup[i]->iID, true);
			}
		}
		sendChangeManualFireStatus (selectedVehicle->iID, true);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuManualFire && selectedBuilding && (selectedBuilding->manualFireActive || selectedBuilding->data.canAttack) && !client->isFreezed () && selectedBuilding->owner == player)
	{
		sendChangeManualFireStatus (selectedBuilding->iID, false);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuActivate && selectedVehicle && selectedVehicle->data.storageUnitsMax > 0 && !client->isFreezed () && selectedVehicle->owner == player)
	{
		cStorageMenu storageMenu (selectedVehicle->storedUnits, selectedVehicle, NULL);
		storageMenu.show();
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuActivate && selectedBuilding && selectedBuilding->data.storageUnitsMax > 0 && !client->isFreezed () && selectedBuilding->owner == player)
	{
		cStorageMenu storageMenu (selectedBuilding->storedUnits, NULL, selectedBuilding);
		storageMenu.show();
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuLoad && selectedVehicle && selectedVehicle->data.storageUnitsMax > 0 && !client->isFreezed () && selectedVehicle->owner == player)
	{
		toggleMouseInputMode (loadMode);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuLoad && selectedBuilding && selectedBuilding->data.storageUnitsMax > 0 && !client->isFreezed () && selectedBuilding->owner == player)
	{
		toggleMouseInputMode (loadMode);
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuReload && selectedVehicle && selectedVehicle->data.canRearm && selectedVehicle->data.storageResCur >= 2 && !client->isFreezed () && selectedVehicle->owner == player)
	{
		client->gameGUI.mouseInputMode = muniActive;
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuRepair && selectedVehicle && selectedVehicle->data.canRepair && selectedVehicle->data.storageResCur >= 2 && !client->isFreezed () && selectedVehicle->owner == player)
	{
		client->gameGUI.mouseInputMode = repairActive;
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuLayMine && selectedVehicle && selectedVehicle->data.canPlaceMines && selectedVehicle->data.storageResCur > 0 && !client->isFreezed () && selectedVehicle->owner == player)
	{
		for (unsigned int i = 1; i < selectedVehiclesGroup.Size(); i++)
		{
			if (selectedVehiclesGroup[i]->data.canPlaceMines || selectedVehiclesGroup[i]->data.storageResCur > 0) selectedVehiclesGroup[i]->executeLayMinesCommand();
		}
		selectedVehicle->executeLayMinesCommand();
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuClearMine && selectedVehicle && selectedVehicle->data.canPlaceMines && selectedVehicle->data.storageResCur < selectedVehicle->data.storageResMax && !client->isFreezed () && selectedVehicle->owner == player)
	{
		for (unsigned int i = 1; i < selectedVehiclesGroup.Size(); i++)
		{
			if (selectedVehiclesGroup[i]->data.canPlaceMines || selectedVehiclesGroup[i]->data.storageResCur < selectedVehiclesGroup[i]->data.storageResMax) selectedVehiclesGroup[i]->executeClearMinesCommand();
		}
		selectedVehicle->executeClearMinesCommand();
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuDisable && selectedVehicle && selectedVehicle->data.canDisable && selectedVehicle->data.shotsCur && !client->isFreezed () && selectedVehicle->owner == player)
	{
		mouseInputMode = disableMode;
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuSteal && selectedVehicle && selectedVehicle->data.canCapture && selectedVehicle->data.shotsCur && !client->isFreezed () && selectedVehicle->owner == player)
	{
		mouseInputMode = stealMode;
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuInfo && selectedVehicle)
	{
		cUnitHelpMenu helpMenu (&selectedVehicle->data, selectedVehicle->owner);
		helpMenu.show();
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuInfo && selectedBuilding)
	{
		cUnitHelpMenu helpMenu (&selectedBuilding->data, selectedBuilding->owner);
		helpMenu.show();
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuDistribute && selectedBuilding && selectedBuilding->data.canMineMaxRes > 0 && selectedBuilding->IsWorking && !client->isFreezed () && selectedBuilding->owner == player)
	{
		cMineManagerMenu mineManager (selectedBuilding);
		mineManager.show();
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuResearch && selectedBuilding && selectedBuilding->data.canResearch && selectedBuilding->IsWorking && !client->isFreezed () && selectedBuilding->owner == player)
	{
		cDialogResearch researchDialog (selectedBuilding->owner);
		researchDialog.show();
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuUpgrade && selectedBuilding && selectedBuilding->data.convertsGold && !client->isFreezed () && selectedBuilding->owner == player)
	{
		cUpgradeMenu upgradeMenu (selectedBuilding->owner);
		upgradeMenu.show();
	}
	else if (key.keysym.sym == KeysList.KeyUnitMenuDestroy && selectedBuilding && selectedBuilding->data.canSelfDestroy && !client->isFreezed () && selectedBuilding->owner == player)
	{
		cDestructMenu destructMenu;
		if (destructMenu.show() == 0) sendWantSelfDestroy (selectedBuilding);
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
	if (gui->selectedVehicle) gui->selectedVehicle->center();
	else if (gui->selectedBuilding) gui->selectedBuilding->center();
}

void cGameGUI::reportsReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	cReportsMenu reportMenu (gui->player);
	reportMenu.show();
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
	cUnit* unit = gui->player->getNextUnit();
	if (unit)
	{
		if (unit->isBuilding())
			gui->selectUnit (static_cast<cBuilding*> (unit));
		else
			gui->selectUnit (static_cast<cVehicle*> (unit));

		unit->center();
	}
}


void cGameGUI::prevReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	cUnit* unit = gui->player->getPrevUnit();
	if (unit)
	{
		if (unit->isBuilding())
			gui->selectUnit (static_cast<cBuilding*> (unit));
		else
			gui->selectUnit (static_cast<cVehicle*> (unit));

		unit->center();
	}
}

void cGameGUI::doneReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);

	if (gui->shiftPressed)
	{
		sendMoveJobResume (0);
		return;
	}

	cUnit* unit = gui->selectedVehicle;
	if (!unit) unit = gui->selectedBuilding;

	if (unit && unit->owner == gui->client->getActivePlayer())
	{
		unit->center();
		unit->isMarkedAsDone = true;
		sendMoveJobResume (unit->iID);
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

	for (unsigned int i = 0; i < gui->playersInfo.Size(); i++)
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
		miniMapOffX = centerPosX - (map->size / (zoomFactor * 2));
		miniMapOffY = centerPosY - (map->size / (zoomFactor * 2));

		if (miniMapOffX < 0) miniMapOffX = 0;
		if (miniMapOffY < 0) miniMapOffY = 0;
		if (miniMapOffX > map->size - (map->size / zoomFactor)) miniMapOffX = map->size - (map->size / zoomFactor);
		if (miniMapOffY > map->size - (map->size / zoomFactor)) miniMapOffY = map->size - (map->size / zoomFactor);
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

	gui->offX = gui->miniMapOffX * 64 + ( (x - MINIMAP_POS_X) * gui->map->size * 64) / (MINIMAP_SIZE * zoomFactor);
	gui->offY = gui->miniMapOffY * 64 + ( (y - MINIMAP_POS_Y) * gui->map->size * 64) / (MINIMAP_SIZE * zoomFactor);
	gui->offX -= displayedMapWidth / 2;
	gui->offY -= displayedMapHight / 2;

	lastX = x;
	lastY = y;

	gui->checkOffsetPosition();
}

void cGameGUI::miniMapRightClicked (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);

	if (!gui->selectedVehicle || gui->selectedVehicle->owner != gui->client->getActivePlayer()) return;

	int x = mouse->x;
	int y = mouse->y;
	const int zoomFactor = gui->twoXChecked() ? MINIMAP_ZOOM_FACTOR : 1;

	int destX = gui->miniMapOffX + ( (x - MINIMAP_POS_X) * gui->map->size) / (MINIMAP_SIZE * zoomFactor);
	int destY = gui->miniMapOffY + ( (y - MINIMAP_POS_Y) * gui->map->size) / (MINIMAP_SIZE * zoomFactor);

	gui->client->addMoveJob (gui->selectedVehicle, destX, destY, &gui->selectedVehiclesGroup);

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
	cDialogPreferences preferencesDialog;
	preferencesDialog.show();
}

void cGameGUI::filesReleased (void* parent)
{
	cGameGUI* gui = static_cast<cGameGUI*> (parent);
	cLoadSaveMenu loadSaveMenu (new cGameDataContainer);   // TODO: memory leak?
	if (loadSaveMenu.show() != 1) gui->end = true;
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
		else sendChatMessageToServer (gui->player->name + ": " + chatString);
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

	if (gui->selectedVehicle) sendWantChangeUnitName (nameString, gui->selectedVehicle->iID);
	else if (gui->selectedBuilding) sendWantChangeUnitName (nameString, gui->selectedBuilding->iID);

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
	if (endX >= map->size) endX = map->size - 1;
	int endY = Round (offY / 64.0 + (float) (Video.getResolutionY() - HUD_TOTAL_HIGHT) / getTileSize());
	if (endY >= map->size) endY = map->size - 1;

	if (timer400ms) map->generateNextAnimationFrame();

	SDL_Rect clipRect = { HUD_LEFT_WIDTH, HUD_TOP_HIGHT, Video.getResolutionX() - HUD_TOTAL_WIDTH, Video.getResolutionY() - HUD_TOTAL_HIGHT };
	SDL_SetClipRect (buffer, &clipRect);

	drawTerrain (zoomOffX, zoomOffY);
	if (gridChecked()) drawGrid (zoomOffX, zoomOffY);

	displayBottomFX();

	dCache.resetStatistics();

	drawBaseUnits (startX, startY, endX, endY, zoomOffX, zoomOffY);
	drawTopBuildings (startX, startY, endX, endY, zoomOffX, zoomOffY);
	drawShips (startX, startY, endX, endY, zoomOffX, zoomOffY);
	drawAboveSeaBaseUnits (startX, startY, endX, endY, zoomOffX, zoomOffY);
	drawVehicles (startX, startY, endX, endY, zoomOffX, zoomOffY);
	drawConnectors (startX, startY, endX, endY, zoomOffX, zoomOffY);
	drawPlanes (startX, startY, endX, endY, zoomOffX, zoomOffY);

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

	if (selectedVehicle && unitMenuActive) selectedVehicle->drawMenu (*this);
	else if (selectedBuilding && unitMenuActive) selectedBuilding->drawMenu (*this);

	displayFX();

	displayMessages();
}

void cGameGUI::drawTerrain (int zoomOffX, int zoomOffY)
{
	int tileSize = client->gameGUI.getTileSize();
	SDL_Rect dest, tmp;
	dest.y = HUD_TOP_HIGHT - zoomOffY;
	// draw the terrain
	struct sTerrain* terr;
	for (int y = 0; y < map->size; y++)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX;
		if (dest.y >= HUD_TOP_HIGHT - tileSize)
		{
			int pos = y * map->size;
			for (int x = 0 ; x < map->size; x++)
			{
				if (dest.x >= HUD_LEFT_WIDTH - tileSize)
				{
					tmp = dest;
					terr = map->terrain + map->Kacheln[pos];

					// draw the fog:
					if (fogChecked() && !player->ScanMap[pos])
					{
						if (!cSettings::getInstance().shouldDoPrescale() && (terr->shw->w != tileSize || terr->shw->h != tileSize)) scaleSurface (terr->shw_org, terr->shw, tileSize, tileSize);
						SDL_BlitSurface (terr->shw, NULL, buffer, &tmp);
					}
					else
					{
						if (!cSettings::getInstance().shouldDoPrescale() && (terr->sf->w != tileSize || terr->sf->h != tileSize)) scaleSurface (terr->sf_org, terr->sf, tileSize, tileSize);
						SDL_BlitSurface (terr->sf, NULL, buffer, &tmp);
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
	int tileSize = client->gameGUI.getTileSize();
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

void cGameGUI::displayFX()
{
	if (!client->FXList.Size()) return;

	SDL_Rect clipRect = { HUD_LEFT_WIDTH, HUD_TOP_HIGHT, Video.getResolutionX() - HUD_TOTAL_WIDTH, Video.getResolutionY() - HUD_TOTAL_HIGHT };
	SDL_SetClipRect (buffer, &clipRect);

	for (int i = (int) client->FXList.Size() - 1; i >= 0; i--)
	{
		drawFX (i);
	}
	SDL_SetClipRect (buffer, NULL);
}

void cGameGUI::drawFX (int num)
{
	SDL_Rect scr, dest;
	sFX* fx;

	fx = client->FXList[num];
	if (!player->ScanMap[fx->PosX / 64 + fx->PosY / 64 * map->size] && fx->typ != fxRocket) return;

	switch (fx->typ)
	{
		case fxMuzzleBig:
			if (!EffectsData.fx_muzzle_big) break;
			CHECK_SCALING (EffectsData.fx_muzzle_big[1], EffectsData.fx_muzzle_big[0], getZoom());
			if ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2 > 2)
			{
				delete fx;
				client->FXList.Delete (num);
				return;
			}
			scr.x = (int) (getZoom() * 64.0) * fx->param;
			scr.y = 0;
			scr.w = (int) (getZoom() * 64.0);
			scr.h = (int) (getZoom() * 64.0);
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - fx->PosX) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - fx->PosY) * getZoom()));
			SDL_BlitSurface (EffectsData.fx_muzzle_big[1], &scr, buffer, &dest);
			break;
		case fxMuzzleSmall:
			if (!EffectsData.fx_muzzle_small) break;
			CHECK_SCALING (EffectsData.fx_muzzle_small[1], EffectsData.fx_muzzle_small[0], getZoom());
			if ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2 > 2)
			{
				delete fx;
				client->FXList.Delete (num);
				return;
			}
			scr.x = (int) (getZoom() * 64.0) * fx->param;
			scr.y = 0;
			scr.w = (int) (getZoom() * 64.0);
			scr.h = (int) (getZoom() * 64.0);
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - fx->PosX) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - fx->PosY) * getZoom()));
			SDL_BlitSurface (EffectsData.fx_muzzle_small[1], &scr, buffer, &dest);
			break;
		case fxMuzzleMed:
			if (!EffectsData.fx_muzzle_med) break;
			CHECK_SCALING (EffectsData.fx_muzzle_med[1], EffectsData.fx_muzzle_med[0], getZoom());
			if ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2 > 2)
			{
				delete fx;
				client->FXList.Delete (num);
				return;
			}
			scr.x = (int) (getZoom() * 64.0) * fx->param;
			scr.y = 0;
			scr.w = (int) (getZoom() * 64.0);
			scr.h = (int) (getZoom() * 64.0);
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - fx->PosX) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - fx->PosY) * getZoom()));
			SDL_BlitSurface (EffectsData.fx_muzzle_med[1], &scr, buffer, &dest);
			break;
		case fxMuzzleMedLong:
			if (!EffectsData.fx_muzzle_med) break;
			CHECK_SCALING (EffectsData.fx_muzzle_med[1], EffectsData.fx_muzzle_med[0], getZoom());
			if ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2 > 5)
			{
				delete fx;
				client->FXList.Delete (num);
				return;
			}
			scr.x = (int) (getZoom() * 64.0) * fx->param;
			scr.y = 0;
			scr.w = (int) (getZoom() * 64.0);
			scr.h = (int) (getZoom() * 64.0);
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - fx->PosX) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - fx->PosY) * getZoom()));
			SDL_BlitSurface (EffectsData.fx_muzzle_med[1], &scr, buffer, &dest);
			break;
		case fxHit:
			if (!EffectsData.fx_hit) break;
			CHECK_SCALING (EffectsData.fx_hit[1], EffectsData.fx_hit[0], getZoom());
			if ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2 > 5)
			{
				delete fx;
				client->FXList.Delete (num);
				return;
			}
			scr.x = (int) (getZoom() * 64.0) * ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2);
			scr.y = 0;
			scr.w = (int) (getZoom() * 64.0);
			scr.h = (int) (getZoom() * 64.0);
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - fx->PosX) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - fx->PosY) * getZoom()));
			SDL_BlitSurface (EffectsData.fx_hit[1], &scr, buffer, &dest);
			break;
		case fxExploSmall:
			if (!EffectsData.fx_explo_small) break;
			CHECK_SCALING (EffectsData.fx_explo_small[1], EffectsData.fx_explo_small[0], getZoom());
			if ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2 > 14)
			{
				delete fx;
				client->FXList.Delete (num);
				return;
			}
			scr.x = (int) ( (int) (getZoom() * 64.0) * 114 * ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2) / 64.0);
			scr.y = 0;
			scr.w = (int) ( (int) (getZoom() * 64.0) * 114 / 64.0);
			scr.h = (int) ( (int) (getZoom() * 64.0) * 108 / 64.0);
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - (fx->PosX - 57)) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - (fx->PosY - 54)) * getZoom()));
			SDL_BlitSurface (EffectsData.fx_explo_small[1], &scr, buffer, &dest);
			break;
		case fxExploBig:
			if (!EffectsData.fx_explo_big) break;
			CHECK_SCALING (EffectsData.fx_explo_big[1], EffectsData.fx_explo_big[0], getZoom());
			if ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2 > 28)
			{
				delete fx;
				client->FXList.Delete (num);
				return;
			}
			scr.x = (int) ( (int) (getZoom() * 64.0) * 307 * ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2) / 64.0);
			scr.y = 0;
			scr.w = (int) ( (int) (getZoom() * 64.0) * 307 / 64.0);
			scr.h = (int) ( (int) (getZoom() * 64.0) * 194 / 64.0);
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - (fx->PosX - 134)) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - (fx->PosY - 85)) * getZoom()));
			SDL_BlitSurface (EffectsData.fx_explo_big[1], &scr, buffer, &dest);
			break;
		case fxExploWater:
			if (!EffectsData.fx_explo_water) break;
			CHECK_SCALING (EffectsData.fx_explo_water[1], EffectsData.fx_explo_water[0], getZoom());
			if ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2 > 14)
			{
				delete fx;
				client->FXList.Delete (num);
				return;
			}
			scr.x = (int) ( (int) (getZoom() * 64.0) * 114 * ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2) / 64.0);
			scr.y = 0;
			scr.w = (int) ( (int) (getZoom() * 64.0) * 114 / 64.0);
			scr.h = (int) ( (int) (getZoom() * 64.0) * 108 / 64.0);
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - (fx->PosX - 57)) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - (fx->PosY - 54)) * getZoom()));
			SDL_BlitSurface (EffectsData.fx_explo_water[1], &scr, buffer, &dest);
			break;
		case fxExploAir:
			if (!EffectsData.fx_explo_air) break;
			CHECK_SCALING (EffectsData.fx_explo_air[1], EffectsData.fx_explo_air[0], getZoom());
			if ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2 > 14)
			{
				delete fx;
				client->FXList.Delete (num);
				return;
			}
			scr.x = (int) ( (int) (getZoom() * 64.0) * 137 * ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2) / 64.0);
			scr.y = 0;
			scr.w = (int) ( (int) (getZoom() * 64.0) * 137 / 64.0);
			scr.h = (int) ( (int) (getZoom() * 64.0) * 121 / 64.0);
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - (fx->PosX - 61)) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - (fx->PosY - 68)) * getZoom()));
			SDL_BlitSurface (EffectsData.fx_explo_air[1], &scr, buffer, &dest);
			break;
		case fxSmoke:
			if (!EffectsData.fx_smoke) break;
			CHECK_SCALING (EffectsData.fx_smoke[1], EffectsData.fx_smoke[0], getZoom());
			if ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2 > 100 / 4)
			{
				delete fx;
				client->FXList.Delete (num);
				return;
			}
			SDL_SetAlpha (EffectsData.fx_smoke[1], SDL_SRCALPHA, 100 - ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2) * 4);
			scr.y = scr.x = 0;
			scr.w = EffectsData.fx_smoke[1]->h;
			scr.h = EffectsData.fx_smoke[1]->h;
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - (fx->PosX - EffectsData.fx_smoke[0]->h / 2 + 32)) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - (fx->PosY - EffectsData.fx_smoke[0]->h / 2 + 32)) * getZoom()));
			SDL_BlitSurface (EffectsData.fx_smoke[1], &scr, buffer, &dest);
			break;
		case fxRocket:
		{
			if (!EffectsData.fx_rocket) break;
			CHECK_SCALING (EffectsData.fx_rocket[1], EffectsData.fx_rocket[0], getZoom());
			sFXRocketInfos* ri;
			ri = fx->rocketInfo;

			scr.x = ri->dir * EffectsData.fx_rocket[1]->h;
			scr.y = 0;
			scr.h = scr.w = EffectsData.fx_rocket[1]->h;
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - (fx->PosX - EffectsData.fx_rocket[0]->h / 2 + 32)) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - (fx->PosY - EffectsData.fx_rocket[0]->h / 2 + 32)) * getZoom()));

			if (player->ScanMap[fx->PosX / 64 + fx->PosY / 64 * map->size])
				SDL_BlitSurface (EffectsData.fx_rocket[1], &scr, buffer, &dest);

			break;
		}
		case fxDarkSmoke:
		{
			if (!EffectsData.fx_dark_smoke) break;
			CHECK_SCALING (EffectsData.fx_dark_smoke[1], EffectsData.fx_dark_smoke[0], getZoom());
			sFXDarkSmoke* dsi;
			dsi = fx->smokeInfo;
			if ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2 > 50 || dsi->alpha <= 1)
			{
				delete fx;
				client->FXList.Delete (num);
				return;
			}
			scr.x = (int) (0.375 * (int) (getZoom() * 64.0)) * ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2);
			scr.y = 0;
			scr.w = EffectsData.fx_dark_smoke[1]->h;
			scr.h = EffectsData.fx_dark_smoke[1]->h;
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - ( (int) dsi->fx)) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - ( (int) dsi->fy)) * getZoom()));

			SDL_SetAlpha (EffectsData.fx_dark_smoke[1], SDL_SRCALPHA, dsi->alpha);
			SDL_BlitSurface (EffectsData.fx_dark_smoke[1], &scr, buffer, &dest);

			if (timer50ms)
			{
				dsi->fx += dsi->dx;
				dsi->fy += dsi->dy;
				dsi->alpha -= 3;
				if (dsi->alpha <= 0) dsi->alpha = 1;
			}
			break;
		}
		case fxAbsorb:
		{
			if (!EffectsData.fx_absorb) break;
			CHECK_SCALING (EffectsData.fx_absorb[1], EffectsData.fx_absorb[0], getZoom());
			if ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2 > 10)
			{
				delete fx;
				client->FXList.Delete (num);
				return;
			}
			scr.x = (int) (getZoom() * 64.0) * ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2);
			scr.y = 0;
			scr.w = (int) (getZoom() * 64.0);
			scr.h = (int) (getZoom() * 64.0);
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - fx->PosX) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - fx->PosY) * getZoom()));
			SDL_BlitSurface (EffectsData.fx_absorb[1], &scr, buffer, &dest);
			break;
		}
		case fxTorpedo:
		case fxTracks:
		case fxBubbles:
		case fxCorpse:
		{
			break;
		}
	}
}

void cGameGUI::displayBottomFX()
{
	if (!client->FXListBottom.Size()) return;

	SDL_Rect oldClipRect = buffer->clip_rect;
	SDL_Rect clipRect = { HUD_LEFT_WIDTH, HUD_TOP_HIGHT, Video.getResolutionX() - HUD_TOTAL_WIDTH, Video.getResolutionY() - HUD_TOTAL_HIGHT };
	SDL_SetClipRect (buffer, &clipRect);

	for (int i = (int) client->FXListBottom.Size() - 1; i >= 0; i--)
	{
		drawBottomFX (i);
	}
	SDL_SetClipRect (buffer, &oldClipRect);
}

void cGameGUI::drawBottomFX (int num)
{
	SDL_Rect scr, dest;

	sFX* fx = client->FXListBottom[num];
	if ( (!player->ScanMap[fx->PosX / 64 + fx->PosY / 64 * map->size]) && fx->typ != fxTorpedo && fx->typ != fxTracks) return;
	switch (fx->typ)
	{
		case fxTorpedo:
		{
			CHECK_SCALING (EffectsData.fx_rocket[1], EffectsData.fx_rocket[0], getZoom());
			sFXRocketInfos* ri;
			ri = fx->rocketInfo;

			scr.x = ri->dir * EffectsData.fx_rocket[1]->h;
			scr.y = 0;
			scr.h = scr.w = EffectsData.fx_rocket[1]->h;
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - (fx->PosX - EffectsData.fx_rocket[0]->h / 2 + 32)) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - (fx->PosY - EffectsData.fx_rocket[0]->h / 2 + 32)) * getZoom()));

			if (player->ScanMap[fx->PosX / 64 + fx->PosY / 64 * map->size])
			{
				SDL_BlitSurface (EffectsData.fx_rocket[1], &scr, buffer, &dest);
			}
			break;
		}
		case fxTracks:
		{
			CHECK_SCALING (EffectsData.fx_tracks[1], EffectsData.fx_tracks[0], getZoom());
			sFXTracks* tri;
			tri = fx->trackInfo;
			if (tri->alpha <= 1)
			{
				delete fx;
				client->FXListBottom.Delete (num);
				return;
			}

			SDL_SetAlpha (EffectsData.fx_tracks[1], SDL_SRCALPHA, tri->alpha);
			if (timer50ms)
			{
				tri->alpha--;
			}

			if (!player->ScanMap[fx->PosX / 64 + fx->PosY / 64 * map->size]) return;
			scr.y = 0;
			scr.w = scr.h = EffectsData.fx_tracks[1]->h;
			scr.x = tri->dir * scr.w;
			dest.x = HUD_LEFT_WIDTH - (int) ( (offX - fx->PosX) * getZoom());
			dest.y = HUD_TOP_HIGHT - (int) ( (offY - fx->PosY) * getZoom());
			SDL_BlitSurface (EffectsData.fx_tracks[1], &scr, buffer, &dest);
			break;
		}
		case fxBubbles:
			CHECK_SCALING (EffectsData.fx_smoke[1], EffectsData.fx_smoke[0], getZoom());
			if ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2 > 100 / 4)
			{
				delete fx;
				client->FXListBottom.Delete (num);
				return;
			}
			SDL_SetAlpha (EffectsData.fx_smoke[1], SDL_SRCALPHA, 100 - ( (Client->gameGUI.iTimerTime - fx->StartTime) / 2) * 4);
			scr.y = scr.x = 0;
			scr.w = EffectsData.fx_smoke[1]->h;
			scr.h = EffectsData.fx_smoke[1]->h;
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - (fx->PosX - EffectsData.fx_smoke[0]->h / 2 + 32)) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - (fx->PosY - EffectsData.fx_smoke[0]->h / 2 + 32)) * getZoom()));
			SDL_BlitSurface (EffectsData.fx_smoke[1], &scr, buffer, &dest);
			break;
		case fxCorpse:
			CHECK_SCALING (EffectsData.fx_corpse[1], EffectsData.fx_corpse[0], getZoom());
			SDL_SetAlpha (EffectsData.fx_corpse[1], SDL_SRCALPHA, fx->param--);
			scr.y = scr.x = 0;
			scr.w = EffectsData.fx_corpse[1]->h;
			scr.h = EffectsData.fx_corpse[1]->h;
			dest.x = HUD_LEFT_WIDTH - ( (int) ( (offX - fx->PosX) * getZoom()));
			dest.y = HUD_TOP_HIGHT - ( (int) ( (offY - fx->PosY) * getZoom()));
			SDL_BlitSurface (EffectsData.fx_corpse[1], &scr, buffer, &dest);

			if (fx->param <= 0)
			{
				delete fx;
				client->FXListBottom.Delete (num);
				return;
			}
			break;
		default:
			break;
	}
}

void cGameGUI::drawBaseUnits (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	int tileSize = client->gameGUI.getTileSize();
	SDL_Rect dest;
	//draw rubble and all base buildings (without bridges)
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;

	for (int y = startY; y <= endY; y++)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = y * map->size + startX;
		for (int x = startX; x <= endX; x++)
		{
			cBuildingIterator bi = map->fields[pos].getBuildings();
			while (!bi.end) bi++;
			bi--;

			while (!bi.rend && (bi->data.surfacePosition == sUnitData::SURFACE_POS_BENEATH_SEA || bi->data.surfacePosition == sUnitData::SURFACE_POS_BASE || !bi->owner))
			{
				if (player->ScanMap[pos] ||
					(bi->data.isBig && ( (x < endX && player->ScanMap[pos + 1]) || (y < endY && player->ScanMap[pos + map->size]) || (x < endX && y < endY && player->ScanMap[pos + map->size + 1]))))
				{
					if (bi->PosX == x && bi->PosY == y)
					{
						bi->draw (&dest, *this);
					}
				}
				bi--;
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawTopBuildings (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	SDL_Rect dest;
	int tileSize = client->gameGUI.getTileSize();
	//draw top buildings (except connectors)
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; y++)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = y * map->size + startX;
		for (int x = startX; x <= endX; x++)
		{
			cBuilding* building = map->fields[pos].getBuildings();
			if (building && building->data.surfacePosition == sUnitData::SURFACE_POS_GROUND)
			{

				if (player->ScanMap[pos] ||
					(building->data.isBig && ( (x < endX && player->ScanMap[pos + 1]) || (y < endY && player->ScanMap[pos + map->size]) || (x < endX && y < endY && player->ScanMap[pos + map->size + 1]))))
				{
					if (building->PosX == x && building->PosY == y)	//make sure a big building is drawn only once
					{
						building->draw (&dest, *this);

						if (debugOutput.debugBaseClient && building->SubBase)
						{
							sSubBase* sb;
							SDL_Rect tmp = { dest.x, dest.y, getTileSize(), 8 };
							if (building->data.isBig) tmp.w *= 2;
							sb = building->SubBase;
							// the VS compiler gives a warning on casting a pointer to long.
							// therfore we will first cast to long long and then cut this to Unit32 again.
							SDL_FillRect (buffer, &tmp, (Uint32) (long long) (sb));
							font->showText (dest.x + 1, dest.y + 1, iToStr (sb->getID()), FONT_LATIN_SMALL_WHITE);
							string sTmp = "m " + iToStr (sb->Metal) + "/" + iToStr (sb->MaxMetal) + " +" + iToStr (sb->getMetalProd() - sb->MetalNeed);
							font->showText (dest.x + 1, dest.y + 1 + 8, sTmp, FONT_LATIN_SMALL_WHITE);

							sTmp = "o " + iToStr (sb->Oil) + "/" + iToStr (sb->MaxOil) + " +" + iToStr (sb->getOilProd() - sb->OilNeed);
							font->showText (dest.x + 1, dest.y + 1 + 16, sTmp, FONT_LATIN_SMALL_WHITE);

							sTmp = "g " + iToStr (sb->Gold) + "/" + iToStr (sb->MaxGold) + " +" + iToStr (sb->getGoldProd() - sb->GoldNeed);
							font->showText (dest.x + 1, dest.y + 1 + 24, sTmp, FONT_LATIN_SMALL_WHITE);
						}
						if (debugOutput.debugBaseServer && building->SubBase)
						{
							sSubBase* sb = Server->Map->fields[pos].getBuildings()->SubBase;;
							if (sb)
							{
								SDL_Rect tmp = { dest.x, dest.y, getTileSize(), 8 };
								if (building->data.isBig) tmp.w *= 2;

								// the VS compiler gives a warning on casting a pointer to long.
								// therfore we will first cast to long long and then cut this to Unit32 again.
								SDL_FillRect (buffer, &tmp, (Uint32) (long long) (sb));
								font->showText (dest.x + 1, dest.y + 1, iToStr (sb->getID()), FONT_LATIN_SMALL_WHITE);
								string sTmp = "m " + iToStr (sb->Metal) + "/" + iToStr (sb->MaxMetal) + " +" + iToStr (sb->getMetalProd() - sb->MetalNeed);
								font->showText (dest.x + 1, dest.y + 1 + 8, sTmp, FONT_LATIN_SMALL_WHITE);

								sTmp = "o " + iToStr (sb->Oil) + "/" + iToStr (sb->MaxOil) + " +" + iToStr (sb->getOilProd() - sb->OilNeed);
								font->showText (dest.x + 1, dest.y + 1 + 16, sTmp, FONT_LATIN_SMALL_WHITE);

								sTmp = "g " + iToStr (sb->Gold) + "/" + iToStr (sb->MaxGold) + " +" + iToStr (sb->getGoldProd() - sb->GoldNeed);
								font->showText (dest.x + 1, dest.y + 1 + 24, sTmp, FONT_LATIN_SMALL_WHITE);
							}
						}
					}
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawShips (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	SDL_Rect dest;
	int tileSize = client->gameGUI.getTileSize();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; y++)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = y * map->size + startX;
		for (int x = startX; x <= endX; x++)
		{
			if (player->ScanMap[pos])
			{
				cVehicle* vehicle = map->fields[pos].getVehicles();
				if (vehicle && vehicle->data.factorSea > 0 && vehicle->data.factorGround == 0)
				{
					vehicle->draw (dest, *this);
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawAboveSeaBaseUnits (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	SDL_Rect dest;
	int tileSize = client->gameGUI.getTileSize();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; y++)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = y * map->size + startX;
		for (int x = startX; x <= endX; x++)
		{
			if (player->ScanMap[pos])
			{
				cBuildingIterator building = map->fields[pos].getBuildings();
				do
				{
					if (building && building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)
					{
						building->draw (&dest, *this);
					}
					building++;
				}
				while (!building.end);

				building = map->fields[pos].getBuildings();
				do
				{
					if (building && building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)
					{
						building->draw (&dest, *this);
					}
					building++;
				}
				while (!building.end);

				cVehicle* vehicle = map->fields[pos].getVehicles();
				if (vehicle && (vehicle->IsClearing || vehicle->IsBuilding) && (player->ScanMap[pos] || (x < endX && player->ScanMap[pos + 1]) || (y < endY && player->ScanMap[pos + map->size]) || (x < endX && y < endY && player->ScanMap[pos + map->size + 1])))
				{
					if (vehicle->PosX == x && vehicle->PosY == y)	//make sure a big vehicle is drawn only once
					{
						vehicle->draw (dest, *this);
					}
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawVehicles (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	SDL_Rect dest;
	int tileSize = client->gameGUI.getTileSize();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; y++)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = y * map->size + startX;
		for (int x = startX; x <= endX; x++)
		{
			if (player->ScanMap[pos])
			{
				cVehicle* vehicle = map->fields[pos].getVehicles();
				if (vehicle && vehicle->data.factorGround != 0 && !vehicle->IsBuilding && !vehicle->IsClearing)
				{
					vehicle->draw (dest, *this);
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawConnectors (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	SDL_Rect dest;
	int tileSize = client->gameGUI.getTileSize();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; y++)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = y * map->size + startX;
		for (int x = startX; x <= endX; x++)
		{
			if (player->ScanMap[pos])
			{
				cBuilding* building = map->fields[pos].getTopBuilding();
				if (building && building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE)
				{
					building->draw (&dest, *this);
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawPlanes (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	SDL_Rect dest;
	int tileSize = client->gameGUI.getTileSize();
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; y++)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = y * map->size + startX;
		for (int x = startX; x <= endX; x++)
		{
			if (player->ScanMap[pos])
			{
				cVehicleIterator planes = map->fields[pos].getPlanes();
				planes.setToEnd();
				while (!planes.rend)
				{
					planes->draw (dest, *this);
					planes--;
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawResources (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY)
{
	int tileSize = client->gameGUI.getTileSize();
	SDL_Rect dest, tmp, src = { 0, 0, tileSize, tileSize };
	dest.y = HUD_TOP_HIGHT - zoomOffY + tileSize * startY;
	for (int y = startY; y <= endY; y++)
	{
		dest.x = HUD_LEFT_WIDTH - zoomOffX + tileSize * startX;
		int pos = y * map->size + startX;
		for (int x = startX; x <= endX; x++)
		{
			if (player->ResourceMap[pos] && !map->terrain[map->Kacheln[pos]].blocked)
			{
				if (map->Resources[pos].typ == RES_NONE)
				{
					src.x = 0;
					tmp = dest;
					if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_metal->w != ResourceData.res_metal_org->w / 64 * tileSize || ResourceData.res_metal->h != tileSize)) scaleSurface (ResourceData.res_metal_org, ResourceData.res_metal, ResourceData.res_metal_org->w / 64 * tileSize, tileSize);
					SDL_BlitSurface (ResourceData.res_metal, &src, buffer, &tmp);
				}
				else
				{
					src.x = map->Resources[pos].value * tileSize;
					tmp = dest;
					if (map->Resources[pos].typ == RES_METAL)
					{
						if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_metal->w != ResourceData.res_metal_org->w / 64 * tileSize || ResourceData.res_metal->h != tileSize)) scaleSurface (ResourceData.res_metal_org, ResourceData.res_metal, ResourceData.res_metal_org->w / 64 * tileSize, tileSize);
						SDL_BlitSurface (ResourceData.res_metal, &src, buffer, &tmp);
					}
					else if (map->Resources[pos].typ == RES_OIL)
					{
						if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_oil->w != ResourceData.res_oil_org->w / 64 * tileSize || ResourceData.res_oil->h != tileSize)) scaleSurface (ResourceData.res_oil_org, ResourceData.res_oil, ResourceData.res_oil_org->w / 64 * tileSize, tileSize);
						SDL_BlitSurface (ResourceData.res_oil, &src, buffer, &tmp);
					}
					else
					{
						if (!cSettings::getInstance().shouldDoPrescale() && (ResourceData.res_gold->w != ResourceData.res_gold_org->w / 64 * tileSize || ResourceData.res_gold->h != tileSize)) scaleSurface (ResourceData.res_gold_org, ResourceData.res_gold, ResourceData.res_gold_org->w / 64 * tileSize, tileSize);
						SDL_BlitSurface (ResourceData.res_gold, &src, buffer, &tmp);
					}
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawSelectionBox (int zoomOffX, int zoomOffY)
{
	if (mouseBox.startX == -1 || mouseBox.startY == -1 || mouseBox.endX == -1 || mouseBox.endY == -1) return;

	Uint32 color = 0xFFFF00;
	SDL_Rect d;

	int mouseStartX = (int) (min (mouseBox.startX, mouseBox.endX) * getTileSize());
	int mouseStartY = (int) (min (mouseBox.startY, mouseBox.endY) * getTileSize());
	int mouseEndX = (int) (max (mouseBox.startX, mouseBox.endX) * getTileSize());
	int mouseEndY = (int) (max (mouseBox.startY, mouseBox.endY) * getTileSize());

	d.h = 1;
	d.w = mouseEndX - mouseStartX;
	d.x = mouseStartX - zoomOffX + HUD_LEFT_WIDTH;
	d.y = mouseEndY - zoomOffY + 20;
	SDL_FillRect (buffer, &d, color);

	d.h = 1;
	d.w = mouseEndX - mouseStartX;
	d.x = mouseStartX - zoomOffX + HUD_LEFT_WIDTH;
	d.y = mouseStartY - zoomOffY + 20;
	SDL_FillRect (buffer, &d, color);

	d.h = mouseEndY - mouseStartY;
	d.w = 1;
	d.x = mouseStartX - zoomOffX + HUD_LEFT_WIDTH;
	d.y = mouseStartY - zoomOffY + 20;
	SDL_FillRect (buffer, &d, color);

	d.h = mouseEndY - mouseStartY;
	d.w = 1;
	d.x = mouseEndX - zoomOffX + HUD_LEFT_WIDTH;
	d.y = mouseStartY - zoomOffY + 20;
	SDL_FillRect (buffer, &d, color);
}

void cGameGUI::displayMessages()
{
	if (messages.Size() == 0) return;

	sMessage* message;
	int height = 0;
	for (int i = (int) messages.Size() - 1; i >= 0; i--)
	{
		message = messages[i];
		height += 17 + font->getFontHeight() * (message->len  / (Video.getResolutionX() - 300));
	}
	SDL_Rect scr = { 0, 0, Video.getResolutionX() - 200, height + 6 };
	SDL_Rect dest = { 180, 30, 0, 0 };

	if (cSettings::getInstance().isAlphaEffects()) SDL_BlitSurface (GraphicsData.gfx_shadow, &scr, buffer, &dest);
	dest.x = 180 + 2; dest.y = 34;
	dest.w = Video.getResolutionX() - 204;
	dest.h = height;

	for (unsigned int i = 0; i < messages.Size(); i++)
	{
		message = messages[i];
		string msgString = message->msg;
		//HACK TO SHOW PLAYERCOLOR IN CHAT
		int color = -1;
		for (unsigned int i = 0; i < msgString.length(); i++)
		{
			if (msgString[i] == ':')   //scan for chatmessages from _players_
			{
				string tmpString = msgString.substr (0, i);
				for (size_t i = 0; i < Client->getPlayerList()->Size(); i++)
				{
					cPlayer* const Player = (*Client->getPlayerList()) [i];
					if (Player)
					{
						if (tmpString.compare (Player->name) == 0)
						{
							color = GetColorNr (Player->color);
							break;
						}
					}
				}
				break;
			}
		}
		if (color != -1)
		{
#define CELLSPACE 3
			SDL_Rect rColorSrc = { 0, 0, 10, font->getFontHeight() };
			SDL_Rect rDest = dest;
			rDest.w = rColorSrc.w;
			rDest.h = rColorSrc.h;
			SDL_BlitSurface (OtherData.colors[color], &rColorSrc, buffer, &rDest);  //blit color
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
	sTerrain*& tlist = map->terrain;
	int numberOfTerrains = map->iNumberOfTerrains;
	for (int i = 0; i < numberOfTerrains; ++i)
	{
		sTerrain& t = tlist[i];
		scaleSurface (t.sf_org, t.sf, getTileSize(), getTileSize());
		scaleSurface (t.shw_org, t.shw, getTileSize(), getTileSize());
	}
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
#define SCALE_FX(a) if (a) scaleSurface(a[0],a[1], (a[0]->w * getTileSize())/64 , (a[0]->h * getTileSize())/64);
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
		SDL_Rect top = { 0, (Video.getResolutionY() / 2) - 479, 171, 479 };
		SDL_Rect bottom = { 0, (Video.getResolutionY() / 2) , 171, 481 };
		SDL_BlitSurface (GraphicsData.gfx_panel_top, NULL, buffer, &tmp);
		tmp = bottom;
		SDL_BlitSurface (GraphicsData.gfx_panel_bottom , NULL, buffer, &tmp);
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
		SDL_Rect bottom = { 0, Video.getResolutionY() , 171, 481 };
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
	SDL_Rect clipRect = { HUD_LEFT_WIDTH, HUD_TOP_HIGHT, Video.getResolutionX() - HUD_TOTAL_WIDTH, Video.getResolutionY() - HUD_TOTAL_HIGHT };
	SDL_SetClipRect (buffer, &clipRect);

	if (selectedVehicle)
	{
		cVehicle& v   = *selectedVehicle;
		bool movementOffset = !v.IsBuilding && !v.IsClearing;
		int const spx = v.getScreenPosX (movementOffset);
		int const spy = v.getScreenPosY (movementOffset);
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
				if (map->possiblePlace (&v, v.PosX - 1, v.PosY - 1)) drawExitPoint (spx - getTileSize(),     spy - getTileSize());
				if (map->possiblePlace (&v, v.PosX    , v.PosY - 1)) drawExitPoint (spx,                spy - getTileSize());
				if (map->possiblePlace (&v, v.PosX + 1, v.PosY - 1)) drawExitPoint (spx + getTileSize(),     spy - getTileSize());
				if (map->possiblePlace (&v, v.PosX + 2, v.PosY - 1)) drawExitPoint (spx + getTileSize() * 2, spy - getTileSize());
				if (map->possiblePlace (&v, v.PosX - 1, v.PosY)) drawExitPoint (spx - getTileSize(),     spy);
				if (map->possiblePlace (&v, v.PosX + 2, v.PosY)) drawExitPoint (spx + getTileSize() * 2, spy);
				if (map->possiblePlace (&v, v.PosX - 1, v.PosY + 1)) drawExitPoint (spx - getTileSize(),     spy + getTileSize());
				if (map->possiblePlace (&v, v.PosX + 2, v.PosY + 1)) drawExitPoint (spx + getTileSize() * 2, spy + getTileSize());
				if (map->possiblePlace (&v, v.PosX - 1, v.PosY + 2)) drawExitPoint (spx - getTileSize(),     spy + getTileSize() * 2);
				if (map->possiblePlace (&v, v.PosX    , v.PosY + 2)) drawExitPoint (spx,                spy + getTileSize() * 2);
				if (map->possiblePlace (&v, v.PosX + 1, v.PosY + 2)) drawExitPoint (spx + getTileSize(),     spy + getTileSize() * 2);
				if (map->possiblePlace (&v, v.PosX + 2, v.PosY + 2)) drawExitPoint (spx + getTileSize() * 2, spy + getTileSize() * 2);
			}
			else
			{
				if (map->possiblePlace (&v, v.PosX - 1, v.PosY - 1)) drawExitPoint (spx - getTileSize(), spy - getTileSize());
				if (map->possiblePlace (&v, v.PosX    , v.PosY - 1)) drawExitPoint (spx,            spy - getTileSize());
				if (map->possiblePlace (&v, v.PosX + 1, v.PosY - 1)) drawExitPoint (spx + getTileSize(), spy - getTileSize());
				if (map->possiblePlace (&v, v.PosX - 1, v.PosY)) drawExitPoint (spx - getTileSize(), spy);
				if (map->possiblePlace (&v, v.PosX + 1, v.PosY)) drawExitPoint (spx + getTileSize(), spy);
				if (map->possiblePlace (&v, v.PosX - 1, v.PosY + 1)) drawExitPoint (spx - getTileSize(), spy + getTileSize());
				if (map->possiblePlace (&v, v.PosX    , v.PosY + 1)) drawExitPoint (spx,            spy + getTileSize());
				if (map->possiblePlace (&v, v.PosX + 1, v.PosY + 1)) drawExitPoint (spx + getTileSize(), spy + getTileSize());
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
				int x = mouse->getKachelX();
				int y = mouse->getKachelY();
				if (x == v.PosX || y == v.PosY)
				{
					SDL_Rect dest;
					dest.x = HUD_LEFT_WIDTH - (int) (offX * getZoom()) + getTileSize() * x;
					dest.y =  HUD_TOP_HIGHT - (int) (offY * getZoom()) + getTileSize() * y;
					CHECK_SCALING (GraphicsData.gfx_band_small, GraphicsData.gfx_band_small_org, (float) getTileSize() / 64.0);
					SDL_BlitSurface (GraphicsData.gfx_band_small, NULL, buffer, &dest);
					v.BandX     = x;
					v.BandY     = y;
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
		int spx, spy;
		spx = selectedBuilding->getScreenPosX();
		spy = selectedBuilding->getScreenPosY();
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
			selectedBuilding->BuildList->Size()                      &&
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
	player->DrawLockList (*this);

	SDL_SetClipRect (buffer, NULL);
}

void cGameGUI::drawExitPoint (int x, int y)
{
	SDL_Rect dest, scr;
	int nr = ANIMATION_SPEED % 5;
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
			if (selectedVehicle && !selectedVehicle->data.shotsCur)
				mouseInputMode = normalInput;
			else if (selectedBuilding && !selectedBuilding->data.shotsCur)
				mouseInputMode = normalInput;
			break;
		case loadMode:
			if (selectedVehicle && selectedVehicle->data.storageUnitsCur == selectedVehicle->data.storageUnitsMax)
				mouseInputMode = normalInput;
			else if (selectedBuilding && selectedBuilding->data.storageUnitsCur == selectedBuilding->data.storageUnitsMax)
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

	int offsetX = savedPositions[slotNumber].offsetX - (int) ( (Video.getResolutionX() - HUD_TOTAL_WIDTH) / getZoom() / 2);
	int offsetY = savedPositions[slotNumber].offsetY - (int) ( (Video.getResolutionY() - HUD_TOTAL_HIGHT) / getZoom() / 2);

	setOffsetPosition (offsetX, offsetY);
}
