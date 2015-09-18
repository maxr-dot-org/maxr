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
#include "utility/serialization/serialization.h"

// Predeclarations
class cPlayer;
class cLanguage;
struct sBuildingUIData;
struct sVehicleUIData;

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

struct sUnitData;

struct sID
{
	sID() : iFirstPart (0), iSecondPart (0) {}

	std::string getText() const;
	void generate (const std::string& text);

	bool isAVehicle() const { return iFirstPart == 0; }
	bool isABuilding() const { return iFirstPart == 1; }

	/** Get the basic version of a unit.
	 * @param Owner If Owner is given, his clan will be taken
	 *        into consideration for modifications of the unit's values.
	 * @return the sUnitData of the owner without upgrades
	 *         (but with the owner's clan modifications) */
	const sUnitData* getUnitDataOriginalVersion (const cPlayer* Owner = nullptr) const;

	bool operator== (const sID& ID) const;
	bool operator!= (const sID& rhs) const { return ! (*this == rhs); }
	bool operator< (const sID& rhs) const { return less_vehicleFirst (rhs); }
	bool less_vehicleFirst (const sID& ID) const;
	bool less_buildingFirst (const sID& ID) const;

	template<typename T>
	void serialize(T& archive)
	{
		archive & NVP(iFirstPart);
		archive & NVP(iSecondPart);
	}

public:
	int iFirstPart;
	int iSecondPart;
};

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
class cUnitsData
{
public:
	cUnitsData();

	void initializeIDData();

	int getBuildingIndexBy (sID id) const;
	int getVehicleIndexBy (sID id) const;

	const sBuildingUIData* getBuildingUI (sID id) const;
	const sVehicleUIData* getVehicleUI (sID id) const;

	// clan = -1: without clans
	const sUnitData& getVehicle (int nr, int clan = -1);
	const sUnitData& getBuilding (int nr, int clan = -1);
	const sUnitData& getUnit (const sID& id, int clan = -1);

	// clan = -1: without clans
	const std::vector<sUnitData>& getUnitData_Vehicles (int clan);
	const std::vector<sUnitData>& getUnitData_Buildings (int clan);

	unsigned int getNrVehicles() const;
	unsigned int getNrBuildings() const;

	const sID& getConstructorID() const { return constructorID; }
	const sID& getEngineerID() const { return engineerID; }
	const sID& getSurveyorID() const { return surveyorID; }

	void scaleSurfaces (float zoomFactor);

private:
	void initializeClanUnitData();

public: // TODO: private
	// Vehicles
	// the standard version without clan modifications
	std::vector<sUnitData> svehicles;
	std::vector<sVehicleUIData> vehicleUIs;

	// Buildings
	// the standard version without clan modifications
	std::vector<sUnitData> sbuildings;
	std::vector<sBuildingUIData> buildingUIs;

	AutoSurface dirt_small_org;
	AutoSurface dirt_small;
	AutoSurface dirt_small_shw_org;
	AutoSurface dirt_small_shw;
	AutoSurface dirt_big_org;
	AutoSurface dirt_big;
	AutoSurface dirt_big_shw_org;
	AutoSurface dirt_big_shw;

	// direct pointer on some of the building graphics
	SDL_Surface* ptr_small_beton;
	SDL_Surface* ptr_small_beton_org;
	SDL_Surface* ptr_connector;
	SDL_Surface* ptr_connector_org;
	SDL_Surface* ptr_connector_shw;
	SDL_Surface* ptr_connector_shw_org;

private:
	// contains the modified versions for the clans
	std::vector<std::vector<sUnitData> > clanUnitDataVehicles;
	// contains the modified versions for the clans
	std::vector<std::vector<sUnitData> > clanUnitDataBuildings;
	bool initializedClanUnitData;

	sID constructorID;
	sID engineerID;
	sID surveyorID;
public: // TODO : private
	sID specialIDLandMine;
	sID specialIDSeaMine;
	sID specialIDMine;
	sID specialIDSmallGen;
	sID specialIDConnector;
	sID specialIDSmallBeton;
} EX UnitsData;

enum eFreezeMode
{
	FREEZE_WAIT_FOR_SERVER,    // waiting response from server
	FREEZE_WAIT_FOR_OTHERS,    // waiting for the others turn, in turn based mode
	FREEZE_PAUSE,              // pause, because... pause
	FREEZE_WAIT_FOR_RECONNECT, // game is paused, because the connection to a player is lost
	FREEZE_WAIT_FOR_TURNEND,   // server is processing the turn end
	FREEZE_WAIT_FOR_PLAYER     // waiting for response from a client
};

class sFreezeModes
{
public:
	sFreezeModes();

	void disable (eFreezeMode mode);
	void enable (eFreezeMode mode, int playerNumber);
	bool isEnable (eFreezeMode mode) const;
	int getPlayerNumber() const { return playerNumber; }
	bool isFreezed() const;
public:
	bool waitForOthers;    // waiting for the others turn, in turn based mode
	bool waitForServer;    // waiting response from server
	bool waitForReconnect; // paused, because the connection to a player is lost
	bool waitForTurnEnd;   // server is processing the turn end
	bool pause;            // pause, because... pause
	bool waitForPlayer;    // waiting for response from a client
private:
	int playerNumber;
};

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

void Quit();

#endif
