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
	static SDL_Rect getRect_SmallSymbol_Speed() { return {0, 98, 7, 7}; }
	static SDL_Rect getRect_SmallSymbol_Hits() { return {14, 98, 6, 9}; }
	static SDL_Rect getRect_SmallSymbol_Ammo() { return {50, 98, 5, 7}; }
	static SDL_Rect getRect_SmallSymbol_Shots() { return {88, 98, 8, 4}; }
	static SDL_Rect getRect_SmallSymbol_Metal() { return {60, 98, 7, 10}; }
	static SDL_Rect getRect_SmallSymbol_Oil() { return {104, 98, 8, 9}; }
	static SDL_Rect getRect_SmallSymbol_Gold() { return {120, 98, 9, 8}; }
	static SDL_Rect getRect_SmallSymbol_Energy() { return {74, 98, 7, 7}; }
	static SDL_Rect getRect_SmallSymbol_Human() { return {170, 98, 8, 9}; }
	static SDL_Rect getRect_SmallSymbol_TransportTank() { return {138, 98, 16, 8}; }
	static SDL_Rect getRect_SmallSymbol_TransportAir() { return {186, 98, 21, 8}; }

	static SDL_Rect getRect_Symbol_Speed() { return {244, 97, 8, 10}; }
	static SDL_Rect getRect_Symbol_Shots() { return {254, 97, 5, 10}; }
	static SDL_Rect getRect_Symbol_Disabled() { return {150, 109, 25, 25}; }

	static SDL_Rect getRect_BigSymbol_Speed() { return {0, 109, 11, 12}; }
	static SDL_Rect getRect_BigSymbol_Hitpoints() { return {11, 109, 7, 11}; }
	static SDL_Rect getRect_BigSymbol_Ammo() { return {18, 109, 9, 14}; }
	static SDL_Rect getRect_BigSymbol_Attack() { return {27, 109, 10, 14}; }
	static SDL_Rect getRect_BigSymbol_Shots() { return {37, 109, 15, 7}; }
	static SDL_Rect getRect_BigSymbol_Range() { return {52, 109, 13, 13}; }
	static SDL_Rect getRect_BigSymbol_Armor() { return {65, 109, 11, 14}; }
	static SDL_Rect getRect_BigSymbol_Scan() { return {76, 109, 13, 13}; }
	static SDL_Rect getRect_BigSymbol_Metal() { return {89, 109, 12, 15}; }
	static SDL_Rect getRect_BigSymbol_Oil() { return {101, 109, 11, 12}; }
	static SDL_Rect getRect_BigSymbol_Gold() { return {112, 109, 13, 10}; }
	static SDL_Rect getRect_BigSymbol_Costs() { return {112, 109, 13, 10}; }
	static SDL_Rect getRect_BigSymbol_Energy() { return {125, 109, 13, 17}; }
	static SDL_Rect getRect_BigSymbol_Human() { return {138, 109, 12, 16}; }
	static SDL_Rect getRect_BigSymbol_MetalEmpty() { return {175, 109, 12, 15}; }

	static SDL_Rect getRect_CheckBox_HudLock() { return {397, 298, 21, 22}; }
	static SDL_Rect getRect_CheckBox_HudUnlock() { return {397, 321, 21, 22}; }
	static SDL_Rect getRect_CheckBox_HudTnt() { return {334, 24, 27, 28}; }
	static SDL_Rect getRect_CheckBox_HudTntPressed() { return {362, 24, 27, 28}; }
	static SDL_Rect getRect_CheckBox_Hud2x() { return {334, 53, 27, 28}; }
	static SDL_Rect getRect_CheckBox_Hud2xPressed() { return {362, 53, 27, 28}; }
	static SDL_Rect getRect_CheckBox_HudChat() { return {196, 129, 49, 20}; }
	static SDL_Rect getRect_CheckBox_HudChatPressed() { return {160, 21, 49, 20}; }
	static SDL_Rect getRect_CheckBox_HudPlayer() { return {317, 479, 27, 28}; }
	static SDL_Rect getRect_CheckBox_HudPlayerPressed() { return {344, 479, 27, 28}; }

	static SDL_Rect getRect_CheckBox_HudIndex00Pressed() { return {0, 44, 55, 18}; }
	static SDL_Rect getRect_CheckBox_HudIndex00() { return {167, 44, 55, 18}; }
	static SDL_Rect getRect_CheckBox_HudIndex01Pressed() { return {55, 44, 55, 18}; }
	static SDL_Rect getRect_CheckBox_HudIndex01() { return {222, 44, 55, 18}; }
	static SDL_Rect getRect_CheckBox_HudIndex02Pressed() { return {110, 44, 55, 18}; }
	static SDL_Rect getRect_CheckBox_HudIndex02() { return {277, 44, 55, 18}; }
	static SDL_Rect getRect_CheckBox_HudIndex10Pressed() { return {0, 62, 55, 16}; }
	static SDL_Rect getRect_CheckBox_HudIndex10() { return {167, 62, 55, 16}; }
	static SDL_Rect getRect_CheckBox_HudIndex11Pressed() { return {55, 62, 55, 16}; }
	static SDL_Rect getRect_CheckBox_HudIndex11() { return {222, 62, 55, 16}; }
	static SDL_Rect getRect_CheckBox_HudIndex12Pressed() { return {110, 62, 55, 16}; }
	static SDL_Rect getRect_CheckBox_HudIndex12() { return {277, 62, 55, 16}; }
	static SDL_Rect getRect_CheckBox_HudIndex20Pressed() { return {0, 78, 55, 18}; }
	static SDL_Rect getRect_CheckBox_HudIndex20() { return {167, 78, 55, 18}; }
	static SDL_Rect getRect_CheckBox_HudIndex21Pressed() { return {55, 78, 55, 18}; }
	static SDL_Rect getRect_CheckBox_HudIndex21() { return {222, 78, 55, 18}; }
	static SDL_Rect getRect_CheckBox_HudIndex22Pressed() { return {110, 78, 55, 18}; }
	static SDL_Rect getRect_CheckBox_HudIndex22() { return {277, 78, 55, 18}; }

	static SDL_Rect getRect_PushButton_HudHelpPressed() { return {366, 0, 26, 24}; }
	static SDL_Rect getRect_PushButton_HudHelp() { return {268, 151, 26, 24}; }
	static SDL_Rect getRect_PushButton_HudCenterPressed() { return {0, 21, 21, 22}; }
	static SDL_Rect getRect_PushButton_HudCenter() { return {139, 149, 21, 22}; }
	static SDL_Rect getRect_PushButton_HudNextPressed() { return {288, 0, 39, 23}; }
	static SDL_Rect getRect_PushButton_HudNext() { return {158, 172, 39, 23}; }
	static SDL_Rect getRect_PushButton_HudPrevPressed() { return {327, 0, 38, 23}; }
	static SDL_Rect getRect_PushButton_HudPrev() { return {198, 172, 38, 23}; }
	static SDL_Rect getRect_PushButton_HudDonePressed() { return {262, 0, 26, 24}; }
	static SDL_Rect getRect_PushButton_HudDone() { return {132, 172, 26, 24}; }
	static SDL_Rect getRect_PushButton_HudReportPressed() { return {210, 21, 49, 20}; }
	static SDL_Rect getRect_PushButton_HudReport() { return {245, 130, 49, 20}; }
	static SDL_Rect getRect_PushButton_HudChatPressed() { return {160, 21, 49, 20}; }
	static SDL_Rect getRect_PushButton_HudChat() { return {196, 129, 49, 20}; }
	static SDL_Rect getRect_PushButton_HudPreferencesPressed() { return {195, 0, 67, 20}; }
	static SDL_Rect getRect_PushButton_HudPreferences() { return {0, 169, 67, 20}; }
	static SDL_Rect getRect_PushButton_HudFilesPressed() { return {93, 21, 67, 20}; }
	static SDL_Rect getRect_PushButton_HudFiles() { return {71, 151, 67, 20}; }
	static SDL_Rect getRect_PushButton_HudEndPressed() { return {22, 21, 70, 17}; }
	static SDL_Rect getRect_PushButton_HudEnd() { return {0, 151, 70, 17}; }
	static SDL_Rect getRect_PushButton_HudPlayPressed() { return {157, 0, 19, 18}; }
	static SDL_Rect getRect_PushButton_HudPlay() { return {0, 132, 19, 18}; }
	static SDL_Rect getRect_PushButton_HudStopPressed() { return {176, 0, 19, 19}; }
	static SDL_Rect getRect_PushButton_HudStop() { return {19, 132, 19, 19}; }

	static SDL_Rect getRect_Slider_HudZoom() { return {132, 0, 26, 16}; }
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
