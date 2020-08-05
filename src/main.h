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
///////////////////////////////////////////////////////////////////////////////
//
// Main-declerations, -classes and structures for the game
// Contains all global varaibles needed for the game
//
///////////////////////////////////////////////////////////////////////////////

// Hides some warnings from the eye of VS users ///////////////////////////////
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#ifndef mainH
#define mainH

// Includes ///////////////////////////////////////////////////////////////////

#include <vector>
#include <SDL.h>
#include "utility/autosurface.h"
#include "defines.h"
#include "utility/language.h"
#include "game/data/units/unitdata.h"
#include "game/data/player/clans.h"


// Predeclarations
class cPlayer;
class cLanguage;
struct sBuildingUIData;
struct sVehicleUIData;
class cClanData;

///////////////////////////////////////////////////////////////////////////////
// Globals
// ------------------------
//
///////////////////////////////////////////////////////////////////////////////

// Languagepack ////////////////////////////////////////////////////////////////
EX cLanguage lngPack;

///////////////////////////////////////////////////////////////////////////////
// Structures
// ------------------------
//
///////////////////////////////////////////////////////////////////////////////


enum
{
	TERRAIN_NONE = 0,
	TERRAIN_AIR = 1,
	TERRAIN_SEA = 2,
	TERRAIN_GROUND = 4,
	TERRAIN_COAST = 8,
	AREA_SUB = 16,
	AREA_EXP_MINE = 32
};

///////////////////////////////////////////////////////////////////////////////
// Variables-Classes
// ------------------------
// This classes are for saving global Variables needed by the game
///////////////////////////////////////////////////////////////////////////////

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

	std::string DialogPath;
	std::string Dialog2Path;
	std::string Dialog3Path;
} EX GraphicsData;

// Effects - Class containing all effect surfaces /////////////////////////////
class cEffectsData
{
public:
	void load (const char* path);
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
} EX EffectsData;

// ResourceData - Class containing all resource surfaces //////////////////////
class cResourceData
{
public:
	void load (const char* path);
public:
	AutoSurface res_metal_org;
	AutoSurface res_metal;
	AutoSurface res_oil_org;
	AutoSurface res_oil;
	AutoSurface res_gold_org;
	AutoSurface res_gold;
} EX ResourceData;

// UnitsData - Class containing all building/vehicle surfaces & data ///////////////
EX cUnitsData UnitsDataGlobal;
EX cClanData  ClanDataGlobal;

class cUnitsUiData
{
public:
	cUnitsUiData();
	~cUnitsUiData();

	const sBuildingUIData* getBuildingUI(sID id) const;
	const sVehicleUIData* getVehicleUI(sID id) const;


	std::vector<sVehicleUIData> vehicleUIs;
	std::vector<sBuildingUIData> buildingUIs;

	sBuildingUIData* rubbleBig;
	sBuildingUIData* rubbleSmall;

	// direct pointer on some of the building graphics
	SDL_Surface* ptr_small_beton;
	SDL_Surface* ptr_small_beton_org;
	SDL_Surface* ptr_connector;
	SDL_Surface* ptr_connector_org;
	SDL_Surface* ptr_connector_shw;
	SDL_Surface* ptr_connector_shw_org;
} EX UnitsUiData;

enum class ePlayerConnectionState
{
	INACTIVE,       // player is not connected, but game can continue (e. g. defeated player)
	CONNECTED,      // player is connected. Normal operation.
	NOT_RESPONDING, // player is connected, but no sync message received for some time. Game should be paused.
	DISCONNECTED    // player has lost connection. Game should be paused.
};
std::string enumToString(ePlayerConnectionState value);

// OtherData - Class containing the rest of surfaces //////////////////////////
class cOtherData
{
public:
	void loadWayPoints();
public:
	AutoSurface WayPointPfeile[8][60];
	AutoSurface WayPointPfeileSpecial[8][60];
} EX OtherData;

///////////////////////////////////////////////////////////////////////////////
// Predeclerations
// ------------------------
//
///////////////////////////////////////////////////////////////////////////////

/**
 * Return if it is the main thread.
 * @note: should be called by main once by the main thread to initialize.
 */
bool is_main_thread();

/**Converts integer to string
*/
std::string iToStr (int x);
/**Converts integer to string in hex representation
*/
std::string iToHex (unsigned int x);
/**Converts float to string
*/
std::string fToStr (float x);
/**Converts pointer to string
*/
std::string pToStr (const void* x);
/**Converts bool to string
*/
std::string bToStr (bool x);

/**
* Rounds given param num to specified position after decimal point<br>
* Example:<br>
* num := 3,234<br>
* n := 2<br>
* >>>>>>> Result = 3,23<br>
*
*@author MM
*@param num number to round up
*@param n the position after decimal point in dValueToRound,
*         that will be rounded
*@return rounded num
*/
float Round (float num, unsigned int n);


template <typename T>
T Square (T v) { return v * v; }

/**
* Rounds given param num without numbers after decimal point<br>
* Example:<br>
* num := 3,234<br>
* >>>>>>> Result = 3<br>
*
*@author beko
*@param num number to round up
*@return rounded num
*/
int Round (float num);

std::string getHexValue(unsigned char byte);
unsigned char getByteValue(const std::string& str, int index);

#endif
