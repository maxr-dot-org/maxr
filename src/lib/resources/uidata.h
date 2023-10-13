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

#ifndef resources_uidataH
#define resources_uidataH

#include "SDLutility/autosurface.h"
#include "game/data/player/clans.h"
#include "game/data/units/unitdata.h"

#include <SDL.h>
#include <filesystem>
#include <vector>

class cBuilding;
class cClanData;
class cLanguage;
class cPlayer;
struct sBuildingUIData;
struct sVehicleUIData;

// GraphicsData - Class containing all normal graphic surfaces ////////////////
class cGraphicsData
{
public:
	AutoSurface gfx_Chand;
	AutoSurface gfx_Cno;
	AutoSurface gfx_Cselect;
	AutoSurface gfx_Cmove;
	AutoSurface gfx_Cmove_draft; // for shift + lmb to set a path but no move
	AutoSurface gfx_Chelp;
	AutoSurface gfx_Cattack;
	AutoSurface gfx_Cattackoor; // attack a unit out of range
	AutoSurface gfx_Cpfeil1;
	AutoSurface gfx_Cpfeil2;
	AutoSurface gfx_Cpfeil3;
	AutoSurface gfx_Cpfeil4;
	AutoSurface gfx_Cpfeil6;
	AutoSurface gfx_Cpfeil7;
	AutoSurface gfx_Cpfeil8;
	AutoSurface gfx_Cpfeil9;
	AutoSurface gfx_hud_stuff;
	AutoSurface gfx_shadow;
	AutoSurface gfx_tmp;
	AutoSurface gfx_context_menu;
	AutoSurface gfx_destruction;
	AutoSurface gfx_destruction_glas;
	AutoSurface gfx_Cband;
	AutoSurface gfx_band_small;
	AutoSurface gfx_band_big;
	AutoSurface gfx_band_small_org;
	AutoSurface gfx_band_big_org;
	AutoSurface gfx_big_beton_org;
	AutoSurface gfx_big_beton;
	AutoSurface gfx_Ctransf;
	AutoSurface gfx_Cload;
	AutoSurface gfx_Cactivate;
	AutoSurface gfx_storage;
	AutoSurface gfx_storage_ground;
	AutoSurface gfx_dialog;
	AutoSurface gfx_edock;
	AutoSurface gfx_ehangar;
	AutoSurface gfx_edepot;
	AutoSurface gfx_Cmuni;
	AutoSurface gfx_Crepair;
	AutoSurface gfx_panel_top;
	AutoSurface gfx_panel_bottom;
	AutoSurface gfx_Csteal;
	AutoSurface gfx_Cdisable;
	AutoSurface gfx_menu_stuff;
	AutoSurface gfx_horizontal_bar_blocked;
	AutoSurface gfx_horizontal_bar_gold;
	AutoSurface gfx_horizontal_bar_metal;
	AutoSurface gfx_horizontal_bar_oil;
	AutoSurface gfx_horizontal_bar_slim_gold;
	AutoSurface gfx_horizontal_bar_slim_metal;
	AutoSurface gfx_horizontal_bar_slim_oil;
	AutoSurface gfx_vertical_bar_slim_gold;
	AutoSurface gfx_vertical_bar_slim_metal;
	AutoSurface gfx_vertical_bar_slim_oil;
	AutoSurface gfx_hud_extra_players;
	AutoSurface gfx_player_pc;
	AutoSurface gfx_player_human;
	AutoSurface gfx_player_none;
	AutoSurface gfx_player_select;
	AutoSurface gfx_exitpoints_org;
	AutoSurface gfx_exitpoints;
	AutoSurface gfx_menu_buttons;
	AutoSurface gfx_player_ready;
	AutoSurface gfx_hud_chatbox;

	// Position in gfx_hud_stuff
	SDL_Rect getRect_SmallSymbol_Speed() const { return {0, 98, 7, 7}; }
	SDL_Rect getRect_SmallSymbol_Hits() const { return {14, 98, 6, 9}; }
	SDL_Rect getRect_SmallSymbol_Ammo() const { return {50, 98, 5, 7}; }
	SDL_Rect getRect_SmallSymbol_Shots() const { return {88, 98, 8, 4}; }
	SDL_Rect getRect_SmallSymbol_Metal() const { return {60, 98, 7, 10}; }
	SDL_Rect getRect_SmallSymbol_Oil() const { return {104, 98, 8, 9}; }
	SDL_Rect getRect_SmallSymbol_Gold() const { return {120, 98, 9, 8}; }
	SDL_Rect getRect_SmallSymbol_Energy() const { return {74, 98, 7, 7}; }
	SDL_Rect getRect_SmallSymbol_Human() const { return {170, 98, 8, 9}; }
	SDL_Rect getRect_SmallSymbol_TransportTank() const { return {138, 98, 16, 8}; }
	SDL_Rect getRect_SmallSymbol_TransportAir() const { return {186, 98, 21, 8}; }
};

// Effects - Class containing all effect surfaces /////////////////////////////
class cEffectsData
{
public:
	void load (const std::filesystem::path& directory);

public:
	AutoSurface fx_explo_big[2];
	AutoSurface fx_explo_small[2];
	AutoSurface fx_explo_water[2];
	AutoSurface fx_explo_air[2];
	AutoSurface fx_muzzle_big[2];
	AutoSurface fx_muzzle_small[2];
	AutoSurface fx_muzzle_med[2];
	AutoSurface fx_hit[2];
	AutoSurface fx_smoke[2];
	AutoSurface fx_rocket[2];
	AutoSurface fx_dark_smoke[2];
	AutoSurface fx_tracks[2];
	AutoSurface fx_corpse[2];
	AutoSurface fx_absorb[2];
};

// ResourceData - Class containing all resource surfaces //////////////////////
class cResourceData
{
public:
	void load (const std::filesystem::path& directory);

public:
	AutoSurface res_metal_org;
	AutoSurface res_metal;
	AutoSurface res_oil_org;
	AutoSurface res_oil;
	AutoSurface res_gold_org;
	AutoSurface res_gold;
};

// UnitsData - Class containing all building/vehicle surfaces & data ///////////////
class cUnitsUiData
{
public:
	cUnitsUiData();
	~cUnitsUiData();

	const sBuildingUIData* getBuildingUI (sID) const;
	const sBuildingUIData& getBuildingUI (const cBuilding&) const;
	const sVehicleUIData* getVehicleUI (sID) const;

	std::vector<sVehicleUIData> vehicleUIs;
	std::vector<sBuildingUIData> buildingUIs;

	std::unique_ptr<sBuildingUIData> rubbleBig;
	std::unique_ptr<sBuildingUIData> rubbleSmall;

	// direct pointer on some of the building graphics
	SDL_Surface* ptr_small_beton = nullptr;
	SDL_Surface* ptr_small_beton_org = nullptr;
	SDL_Surface* ptr_connector = nullptr;
	SDL_Surface* ptr_connector_org = nullptr;
	SDL_Surface* ptr_connector_shw = nullptr;
	SDL_Surface* ptr_connector_shw_org = nullptr;
};

// OtherData - Class containing the rest of surfaces //////////////////////////
class cOtherData
{
public:
	void loadWayPoints();

public:
	AutoSurface WayPointPfeile[8][60];
	AutoSurface WayPointPfeileSpecial[8][60];
};

extern cGraphicsData GraphicsData;
extern cEffectsData EffectsData;
extern cResourceData ResourceData;
extern cUnitsUiData UnitsUiData;
extern cOtherData OtherData;

#endif
