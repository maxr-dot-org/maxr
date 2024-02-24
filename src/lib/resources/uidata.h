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

#include "SDLutility/uniquesurface.h"
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

struct sPartialSurface
{
	SDL_Surface* surface = nullptr;
	SDL_Rect rect{};
};

// GraphicsData - Class containing all normal graphic surfaces ////////////////
class cGraphicsData
{
public:
	sPartialSurface get_CheckBox_Round (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 151 + 18 : 151, 93, 18, 17}}; }
	sPartialSurface get_CheckBox_Angular (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 78 : 0, 196, 78, 23}}; }
	sPartialSurface get_CheckBox_Standard (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 187 + 18 : 187, 93, 18, 17}}; }
	sPartialSurface get_CheckBox_Tank (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 32 : 0, 219, 32, 31}}; }
	sPartialSurface get_CheckBox_Plane (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 32 * 2 + 32 : 32 * 2, 219, 32, 31}}; }
	sPartialSurface get_CheckBox_Ship (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 32 * 4 + 32 : 32 * 4, 219, 32, 31}}; }
	sPartialSurface get_CheckBox_Building (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 32 * 6 + 32 : 32 * 6, 219, 32, 31}}; }
	sPartialSurface get_CheckBox_Tnt (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 32 * 8 + 32 : 32 * 8, 219, 32, 31}}; }

	sPartialSurface get_CheckBox_HudIndex00 (bool pressed) const { return {gfx_hud_stuff.get(), {pressed ? 0 : 167, 44, 55, 18}}; }
	sPartialSurface get_CheckBox_HudIndex01 (bool pressed) const { return {gfx_hud_stuff.get(), {pressed ? 55 : 222, 44, 55, 18}}; }
	sPartialSurface get_CheckBox_HudIndex02 (bool pressed) const { return {gfx_hud_stuff.get(), {pressed ? 110 : 277, 44, 55, 18}}; }
	sPartialSurface get_CheckBox_HudIndex10 (bool pressed) const { return {gfx_hud_stuff.get(), {pressed ? 0 : 167, 62, 55, 16}}; }
	sPartialSurface get_CheckBox_HudIndex11 (bool pressed) const { return {gfx_hud_stuff.get(), {pressed ? 55 : 222, 62, 55, 16}}; }
	sPartialSurface get_CheckBox_HudIndex12 (bool pressed) const { return {gfx_hud_stuff.get(), {pressed ? 110 : 277, 62, 55, 16}}; }
	sPartialSurface get_CheckBox_HudIndex20 (bool pressed) const { return {gfx_hud_stuff.get(), {pressed ? 0 : 167, 78, 55, 18}}; }
	sPartialSurface get_CheckBox_HudIndex21 (bool pressed) const { return {gfx_hud_stuff.get(), {pressed ? 55 : 222, 78, 55, 18}}; }
	sPartialSurface get_CheckBox_HudIndex22 (bool pressed) const { return {gfx_hud_stuff.get(), {pressed ? 110 : 277, 78, 55, 18}}; }

	sPartialSurface get_CheckBox_UnitContextMenu (bool pressed) const { return {gfx_context_menu.get(), {0, pressed ? 21 : 0, 42, 21}}; }
	sPartialSurface get_CheckBox_ArrowDownSmall (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 187 + 18 : 187, 59, 18, 17}}; }

	sPartialSurface get_CheckBox_HudLock() const { return {gfx_hud_stuff.get(), {397, 298, 21, 22}}; }
	sPartialSurface get_CheckBox_HudUnlock() const { return {gfx_hud_stuff.get(), {397, 321, 21, 22}}; }
	sPartialSurface get_CheckBox_HudTnt (bool pressed) const { return {gfx_hud_stuff.get(), {pressed ? 362 : 334, 24, 27, 28}}; }
	sPartialSurface get_CheckBox_Hud2x (bool pressed) const { return {gfx_hud_stuff.get(), {pressed ? 362 : 334, 53, 27, 28}}; }
	sPartialSurface get_CheckBox_HudChat (bool pressed) const { return {gfx_hud_stuff.get(), {pressed ? 160 : 196, pressed ? 21 : 129, 49, 20}}; }
	sPartialSurface get_CheckBox_HudPlayer (bool pressed) const { return {gfx_hud_stuff.get(), {pressed ? 344 : 317, 479, 27, 28}}; }

	sPartialSurface get_PushButton_StandardBig (bool pressed) const { return {gfx_menu_stuff.get(), {0, pressed ? 29 : 0, 200, 29}}; }
	sPartialSurface get_PushButton_StandardSmall (bool pressed) const { return {gfx_menu_stuff.get(), {0, pressed ? 87 : 58, 150, 29}}; }
	sPartialSurface get_PushButton_Huge (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 109 : 0, 116, 109, 40}}; }
	sPartialSurface get_PushButton_ArrowUpBig (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 125 : 97, 157, 28, 29}}; }
	sPartialSurface get_PushButton_ArrowDownBig (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 181 : 153, 157, 28, 29}}; }
	sPartialSurface get_PushButton_ArrowLeftBig (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 293 : 265, 157, 28, 29}}; }
	sPartialSurface get_PushButton_ArrowRightBig (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 237 : 209, 157, 28, 29}}; }
	sPartialSurface get_PushButton_ArrowUpSmall (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 151 + 18 : 151, 59, 18, 17}}; }
	sPartialSurface get_PushButton_ArrowDownSmall (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 187 + 18 : 187, 59, 18, 17}}; }
	sPartialSurface get_PushButton_ArrowLeftSmall (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 151 + 18 : 151, 76, 18, 17}}; }
	sPartialSurface get_PushButton_ArrowRightSmall (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 187 + 18 : 187, 76, 18, 17}}; }
	sPartialSurface get_PushButton_ArrowUpBar (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 201 + 17 : 201, 1, 17, 17}}; }
	sPartialSurface get_PushButton_ArrowDownBar (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 201 + 17 : 201, 18, 17, 17}}; }
	sPartialSurface get_PushButton_Angular (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 78 : 0, 196, 78, 23}}; }

	sPartialSurface get_PushButton_HudHelpPressed() const { return {gfx_hud_stuff.get(), {366, 0, 26, 24}}; }
	sPartialSurface get_PushButton_HudHelp() const { return {gfx_hud_stuff.get(), {268, 151, 26, 24}}; }
	sPartialSurface get_PushButton_HudCenterPressed() const { return {gfx_hud_stuff.get(), {0, 21, 21, 22}}; }
	sPartialSurface get_PushButton_HudCenter() const { return {gfx_hud_stuff.get(), {139, 149, 21, 22}}; }
	sPartialSurface get_PushButton_HudNextPressed() const { return {gfx_hud_stuff.get(), {288, 0, 39, 23}}; }
	sPartialSurface get_PushButton_HudNext() const { return {gfx_hud_stuff.get(), {158, 172, 39, 23}}; }
	sPartialSurface get_PushButton_HudPrevPressed() const { return {gfx_hud_stuff.get(), {327, 0, 38, 23}}; }
	sPartialSurface get_PushButton_HudPrev() const { return {gfx_hud_stuff.get(), {198, 172, 38, 23}}; }
	sPartialSurface get_PushButton_HudDonePressed() const { return {gfx_hud_stuff.get(), {262, 0, 26, 24}}; }
	sPartialSurface get_PushButton_HudDone() const { return {gfx_hud_stuff.get(), {132, 172, 26, 24}}; }
	sPartialSurface get_PushButton_HudReportPressed() const { return {gfx_hud_stuff.get(), {210, 21, 49, 20}}; }
	sPartialSurface get_PushButton_HudReport() const { return {gfx_hud_stuff.get(), {245, 130, 49, 20}}; }
	sPartialSurface get_PushButton_HudChatPressed() const { return {gfx_hud_stuff.get(), {160, 21, 49, 20}}; }
	sPartialSurface get_PushButton_HudChat() const { return {gfx_hud_stuff.get(), {196, 129, 49, 20}}; }
	sPartialSurface get_PushButton_HudPreferencesPressed() const { return {gfx_hud_stuff.get(), {195, 0, 67, 20}}; }
	sPartialSurface get_PushButton_HudPreferences() const { return {gfx_hud_stuff.get(), {0, 169, 67, 20}}; }
	sPartialSurface get_PushButton_HudFilesPressed() const { return {gfx_hud_stuff.get(), {93, 21, 67, 20}}; }
	sPartialSurface get_PushButton_HudFiles() const { return {gfx_hud_stuff.get(), {71, 151, 67, 20}}; }
	sPartialSurface get_PushButton_HudEndPressed() const { return {gfx_hud_stuff.get(), {22, 21, 70, 17}}; }
	sPartialSurface get_PushButton_HudEnd() const { return {gfx_hud_stuff.get(), {0, 151, 70, 17}}; }
	sPartialSurface get_PushButton_HudPlayPressed() const { return {gfx_hud_stuff.get(), {157, 0, 19, 18}}; }
	sPartialSurface get_PushButton_HudPlay() const { return {gfx_hud_stuff.get(), {0, 132, 19, 18}}; }
	sPartialSurface get_PushButton_HudStopPressed() const { return {gfx_hud_stuff.get(), {176, 0, 19, 19}}; }
	sPartialSurface get_PushButton_HudStop() const { return {gfx_hud_stuff.get(), {19, 132, 19, 19}}; }

	sPartialSurface get_PushButton_UnitContextMenu (bool pressed) const { return {gfx_context_menu.get(), {0, pressed ? 21 : 0, 42, 21}}; }
	sPartialSurface get_PushButton_DestroyPressed() const { return {gfx_hud_stuff.get(), {6, 269, 59, 56}}; }
	sPartialSurface get_PushButton_Destroy() const { return {gfx_destruction.get(), {15, 13, 59, 56}}; }
	sPartialSurface get_PushButton_ArrowUpSmallModern (bool pressed) const { return {gfx_menu_stuff.get(), {224, pressed ? 75 : 83, 16, 8}}; }
	sPartialSurface get_PushButton_ArrowDownSmallModern (bool pressed) const { return {gfx_menu_stuff.get(), {224, pressed ? 59 : 67, 16, 8}}; }
	sPartialSurface get_PushButton_ArrowLeftSmallModern (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 272 : 264, 59, 8, 16}}; }
	sPartialSurface get_PushButton_ArrowRightSmallModern (bool pressed) const { return {gfx_menu_stuff.get(), {pressed ? 256 : 248, 59, 8, 16}}; }

	UniqueSurface gfx_Chand;
	UniqueSurface gfx_Cno;
	UniqueSurface gfx_Cselect;
	UniqueSurface gfx_Cmove;
	UniqueSurface gfx_Cmove_draft; // for shift + lmb to set a path but no move
	UniqueSurface gfx_Chelp;
	UniqueSurface gfx_Cattack;
	UniqueSurface gfx_Cattackoor; // attack a unit out of range
	UniqueSurface gfx_Cpfeil1;
	UniqueSurface gfx_Cpfeil2;
	UniqueSurface gfx_Cpfeil3;
	UniqueSurface gfx_Cpfeil4;
	UniqueSurface gfx_Cpfeil6;
	UniqueSurface gfx_Cpfeil7;
	UniqueSurface gfx_Cpfeil8;
	UniqueSurface gfx_Cpfeil9;
	UniqueSurface gfx_hud_stuff;
	UniqueSurface gfx_shadow;
	UniqueSurface gfx_tmp;
	UniqueSurface gfx_context_menu;
	UniqueSurface gfx_destruction;
	UniqueSurface gfx_destruction_glas;
	UniqueSurface gfx_Cband;
	UniqueSurface gfx_band_small;
	UniqueSurface gfx_band_big;
	UniqueSurface gfx_band_small_org;
	UniqueSurface gfx_band_big_org;
	UniqueSurface gfx_big_beton_org;
	UniqueSurface gfx_big_beton;
	UniqueSurface gfx_Ctransf;
	UniqueSurface gfx_Cload;
	UniqueSurface gfx_Cactivate;
	UniqueSurface gfx_storage;
	UniqueSurface gfx_storage_ground;
	UniqueSurface gfx_dialog;
	UniqueSurface gfx_edock;
	UniqueSurface gfx_ehangar;
	UniqueSurface gfx_edepot;
	UniqueSurface gfx_Cmuni;
	UniqueSurface gfx_Crepair;
	UniqueSurface gfx_panel_top;
	UniqueSurface gfx_panel_bottom;
	UniqueSurface gfx_Csteal;
	UniqueSurface gfx_Cdisable;
	UniqueSurface gfx_menu_stuff;
	UniqueSurface gfx_horizontal_bar_blocked;
	UniqueSurface gfx_horizontal_bar_gold;
	UniqueSurface gfx_horizontal_bar_metal;
	UniqueSurface gfx_horizontal_bar_oil;
	UniqueSurface gfx_horizontal_bar_slim_gold;
	UniqueSurface gfx_horizontal_bar_slim_metal;
	UniqueSurface gfx_horizontal_bar_slim_oil;
	UniqueSurface gfx_vertical_bar_slim_gold;
	UniqueSurface gfx_vertical_bar_slim_metal;
	UniqueSurface gfx_vertical_bar_slim_oil;
	UniqueSurface gfx_hud_extra_players;
	UniqueSurface gfx_player_pc;
	UniqueSurface gfx_player_human;
	UniqueSurface gfx_player_none;
	UniqueSurface gfx_player_select;
	UniqueSurface gfx_exitpoints_org;
	UniqueSurface gfx_exitpoints;
	UniqueSurface gfx_menu_buttons;
	UniqueSurface gfx_player_ready;
	UniqueSurface gfx_hud_chatbox;

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

	static SDL_Rect getRect_Slider_HudZoom() { return {132, 0, 25, 16}; }
};

// Effects - Class containing all effect surfaces /////////////////////////////
class cEffectsData
{
public:
	void load (const std::filesystem::path& directory);

public:
	UniqueSurface fx_explo_big[2];
	UniqueSurface fx_explo_small[2];
	UniqueSurface fx_explo_water[2];
	UniqueSurface fx_explo_air[2];
	UniqueSurface fx_muzzle_big[2];
	UniqueSurface fx_muzzle_small[2];
	UniqueSurface fx_muzzle_med[2];
	UniqueSurface fx_hit[2];
	UniqueSurface fx_smoke[2];
	UniqueSurface fx_rocket[2];
	UniqueSurface fx_dark_smoke[2];
	UniqueSurface fx_tracks[2];
	UniqueSurface fx_corpse[2];
	UniqueSurface fx_absorb[2];
};

// ResourceData - Class containing all resource surfaces //////////////////////
class cResourceData
{
public:
	void load (const std::filesystem::path& directory);

public:
	UniqueSurface res_metal_org;
	UniqueSurface res_metal;
	UniqueSurface res_oil_org;
	UniqueSurface res_oil;
	UniqueSurface res_gold_org;
	UniqueSurface res_gold;
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
	UniqueSurface WayPointPfeile[8][60];
	UniqueSurface WayPointPfeileSpecial[8][60];
};

extern cGraphicsData GraphicsData;
extern cEffectsData EffectsData;
extern cResourceData ResourceData;
extern cUnitsUiData UnitsUiData;
extern cOtherData OtherData;

#endif
