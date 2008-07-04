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
#pragma warning(disable:4244)
#pragma warning(disable:4800)
#pragma warning(disable:4005)

#pragma warning(disable:4311)
#pragma warning(disable:4312)
#pragma warning(disable:4305)
#pragma warning(disable:4313)
#pragma warning(disable:4804)
#endif

#ifndef mainH
#define mainH

// Includes ///////////////////////////////////////////////////////////////////

#include <iostream>
#include <time.h>
#include <SDL.h>
#include "tinyxml.h"
#include "defines.h"
#include "language.h"
#include "clist.h"

using namespace std;

// Predeclarations
class cPlayer;
class cBuilding;
class cVehicle;
class cMJobs;
class cAJobs;
class cNetMessage;
struct sBuilding;
struct sUpgrades;
struct sTuple;
struct sBuildList;
struct sHUp;
struct sLanding;
struct sClientSettings;
struct sMessage;
struct sWachposten;
struct sLockElem;
struct sSubBase;
struct sBuildStruct;
struct sFX;
struct sPathCalc;
struct sReport;
struct sVehicle;

///////////////////////////////////////////////////////////////////////////////
// Defines
// ------------------------
//
///////////////////////////////////////////////////////////////////////////////

/** Slashscreen width  */
#define SPLASHWIDTH 500
/** Slashscreen height  */
#define SPLASHHEIGHT 420
#define GRID_COLOR         0x305C04 // Farbe der Gitternetzlinien
#define MINIMAP_COLOR      0xFC0000 // Farbe des Rahmens in der Minimap
#define SCAN_COLOR         0xE3E300 // Farbe des Scan-Kreises
#define RANGE_GROUND_COLOR 0xE20000 // Farbe des Reichweiten Kreises für Land
#define RANGE_AIR_COLOR    0xFCA800 // Farbe des Reichweiten Kreises für Luft
#define PFEIL_COLOR        0x00FF00 // Farbe eines Pfeiles
#define PFEILS_COLOR       0x0000FF // Farbe eines speziellen Pfeiles
#define MOVE_SPEED         7       // Geschwindigkeit der Fahrzeuge
#define MSG_FRAMES         150      // Anzahl an Frames, die eine Nachricht zu sehen ist
#define MAX_PATHFINDING    3890     // Maximale Endpunktezal fürs Pathfinding
#define USER_WARPMOUSE		SDL_USEREVENT+0	// An Event for moving the mouse; this is the first UserEvent

// Colors /////////////////////////////////////////////////////////////////////
#define cl_red 0
#define cl_blue 1
#define cl_green 2
#define cl_grey 3
#define cl_orange 4
#define cl_yellow 5
#define cl_purple 6
#define cl_aqua 7

///////////////////////////////////////////////////////////////////////////////
// Globals
// ------------------------
//
///////////////////////////////////////////////////////////////////////////////

// Languagepack ////////////////////////////////////////////////////////////////
EX cLanguage lngPack;

// Screenbuffers //////////////////////////////////////////////////////////////
EX SDL_Surface *screen ZERO;	// Der Bildschirm
EX SDL_Surface *buffer ZERO;	// Der Bildschirm-Buffer

// Nummbers of buildings //////////////////////////////////////////////////////
EX int BNrLandMine ZERO;
EX int BNrSeaMine ZERO;
EX int BNrMine ZERO;
EX int BNrSmallGen ZERO;
EX int BNrOilStore ZERO;

// Initvalues for Research ////////////////////////////////////////////////////
#ifdef __main__
int ResearchInits[8]={16,16,33,8,8,16,33,33};
#else
extern int ResearchInits[8];
#endif

///////////////////////////////////////////////////////////////////////////////
// Structures
// ------------------------
//
///////////////////////////////////////////////////////////////////////////////

// Struktur für die IDs
struct sID{
	int iFirstPart;
	int iSecondPart;
};

// Struktur für die Waffen
struct sWeaponData{
	const char *szTurret_Gfx;
	int iSequence;
	int iShot_Trajectory;
#define SHOT_TRAJECTURY_STRAIGHT 0
#define SHOT_TRAJECTURY_BALISTIC 1
#define SHOT_TRAJECTURY_CONTROLED 2
	int iAmmo_Type;
#define AMMO_TYPE_STANDARD 0
#define AMMO_TYPE_ENERGY 1
	int iAmmo_Quantity;
	int iAmmo_Quantity_Max;

	int iTarget_Land_Damage;
	int iTarget_Land_Range;
	int iTarget_Sea_Damage;
	int iTarget_Sea_Range;
	int iTarget_Air_Damage;
	int iTarget_Air_Range;
	int iTarget_Mine_Damage;
	int iTarget_Mine_Range;
	int iTarget_Submarine_Damage;
	int iTarget_Submarine_Range;
	int iTarget_Infantry_Damage;
	int iTarget_Infantry_Range;
	int iTarget_WMD_Damage;
	int iTarget_WMD_Range;

	int iShots;
	int iShots_Max;
	int iDestination_Area;
	int iDestination_Type;
#define DESTINATION_TYPE_POINT 0
#define DESTINATION_TYPE_MIRV 1
#define DESTINATION_TYPE_SCATTER 2
	int iMovement_Allowed;
};

// Struktur für die Eigenschaften der Vehicles:
struct sUnitData{
	// Main info
	sID ID;
	const char *szName;
	const char *szDescribtion;
	int iVersion;

	// General info
	bool bIs_Controllable;
	bool bCan_Be_Captured;
	bool bCan_Be_Disabled;
	int iSize_Length;
	int iSize_Width;

	// Defence
	bool bIs_Target_Land;
	bool bIs_Target_Sea;
	bool bIs_Target_Air;
	bool bIs_Target_Underwater;
	bool bIs_Target_Mine;
	bool bIs_Target_Building;
	bool bIs_Target_Satellite;
	bool bIs_Target_WMD;
	int iArmor;
	int iHitpoints;
	int iHitpoints_Max;

	// Production
	int iBuilt_Costs;
	int iBuilt_Costs_Max;
	sID *iIs_Produced_by_ID;

	// Weapons
	int iWeaponsCount;
	sWeaponData *Weapons;

	// Abilities
	bool bIs_Base;
	bool bCan_Clear_Area;
	bool bGets_Experience;
	bool bCan_Disable;
	bool bCan_Capture;
	bool bCan_Dive;
	int iLanding_Type;
#define LANDING_TYPE_ONLY_GARAGE 0
#define LANDING_TYPE_GARAGE_AND_PLATFORM 1
#define LANDING_TYPE_EVERYWHERE 2
	bool bCan_Upgrade;
	bool bCan_Repair;
	bool bCan_Rearm;
	bool bCan_Research;
	bool bIs_Kamikaze;
	bool bIs_Infrastructure;
	bool bCan_Place_Mines;
	int iMakes_Tracks;
	int iSelf_Repair_Type;
#define SELF_REPAIR_TYPE_NONE 0
#define SELF_REPAIR_TYPE_AUTOMATIC 1
#define SELF_REPAIR_TYPE_NORMAL 2
	int iConverts_Gold;
	int iNeeds_Energy;
	int iNeeds_Oil;
	int iNeeds_Metal;
	int iNeeds_Humans;
	int iMines_Resources;
	bool bCan_Launch_SRBM;
	int iEnergy_Shield_Strength_Max;
	int iEnergy_Shield_Strength;
	int iEnergy_Shield_Size;
	bool bHas_Connector;
	bool bCan_Be_Passed;
	int iChanges_Terrain;
#define CHANGES_TERRAIN_TYPE_NONE 0
#define CHANGES_TERRAIN_TYPE_STREET 0
#define CHANGES_TERRAIN_TYPE_PLATTFORM 0
#define CHANGES_TERRAIN_TYPE_BRIDGE 0
#define CHANGES_TERRAIN_TYPE_MINE 0
#define CHANGES_TERRAIN_TYPE_CONNECTOR 0


	// Scan_Abilities
	int iScan_Range_Sight;
	int iScan_Range_Air;
	int iScan_Range_Ground;
	int iScan_Range_Sea;
	int iScan_Range_Submarine;
	int iScan_Range_Mine;
	int iScan_Range_Infantry;
	int iScan_Range_Resources;
	int iScan_Range_Jammer;

	// Movement
	int iMovement_Sum;
	int iMovement_Max;
	float fCosts_Air;
	float fCosts_Sea;
	float fCosts_Submarine;
	float fCosts_Ground;
	float fFactor_Coast;
	float fFactor_Wood;
	float fFactor_Road;
	float fFactor_Bridge;
	float fFactor_Platform;
	float fFactor_Monorail;
	float fFactor_Wreck;
	float fFactor_Mountains;

	// Storage
	bool bIs_Garage;
	int iCapacity_Metal_Max;
	int iCapacity_Oil_Max;
	int iCapacity_Gold_Max;
	int iCapacity_Energy_Max;
	int iCapacity_Units_Air_Max;
	int iCapacity_Units_Sea_Max;
	int iCapacity_Units_Ground_Max;
	int iCapacity_Units_Infantry_Max;
	int iCapacity_Metal;
	int iCapacity_Oil;
	int iCapacity_Gold;
	int iCapacity_Energy;
	int iCapacity_Units_Air;
	int iCapacity_Units_Sea;
	int iCapacity_Units_Ground;
	int iCapacity_Units_Infantry;
	sID *iCan_Use_Unit_As_Garage_ID;

	// Grafics
	bool bHas_Overlay;
	bool bBuild_Up_Grafic;
	bool bAnimation_Movement;
	bool bPower_On_Grafic;

	//////
	// Old-Stuff!
	//////
  int version; // Version des Vehicles
  char name[25];

  // Grunddaten:
  int max_speed;
  int speed;
  int max_hit_points;
  int hit_points;
  int armor;
  int scan;
  int range;
  int max_shots;
  int shots;
  int damage;
  int max_cargo;
  int cargo;
  int max_ammo;
  int ammo;
  int costs;

  int energy_prod;
  int oil_need;
  int energy_need;
  int metal_need;
  int human_prod;
  int human_need;
  int gold_need;
  int shield;
  int max_shield;

  // Die Bau-Eigenschaft:
  int can_build;
#define BUILD_NONE  0
#define BUILD_SMALL 1
#define BUILD_BIG   2

#define BUILD_SEA 3
#define BUILD_AIR 4
#define BUILD_MAN 5

  // Die Fahr-Eigenschaft:
  int can_drive;
#define DRIVE_LAND     0
#define DRIVE_SEA      1
#define DRIVE_LANDnSEA 2
#define DRIVE_AIR      3

  // die Transport-Eigenschaft:
  int can_load;
  int can_transport;
#define TRANS_NONE     0
#define TRANS_METAL    1
#define TRANS_OIL      2
#define TRANS_GOLD     3
#define TRANS_VEHICLES 4
#define TRANS_MEN      5
#define TRANS_AIR	   6

  // Die Attack-Eigenschaft:
  int can_attack;
#define ATTACK_NONE     0
#define ATTACK_LAND     1
#define ATTACK_SUB_LAND 2
#define ATTACK_AIR      3
#define ATTACK_AIRnLAND 4

  // Der Style des Mündungsfeuers:
  int muzzle_typ;
#define MUZZLE_BIG 0
#define MUZZLE_ROCKET 1
#define MUZZLE_SMALL 2
#define MUZZLE_MED 3
#define MUZZLE_MED_LONG 4
#define MUZZLE_ROCKET_CLUSTER 5
#define MUZZLE_TORPEDO 6
#define MUZZLE_SNIPER 7

  // weitere Eigenschaften:
  bool can_reload;
  bool can_repair;
  bool can_drive_and_fire;
  bool is_stealth_land;
  bool is_stealth_sea;
  bool is_human;
  bool can_survey;
  bool can_clear;
  bool has_overlay;
  bool build_by_big;
  bool can_lay_mines;
  bool can_detect_mines;
  bool can_detect_sea;
  bool can_detect_land;
  bool make_tracks;
  bool is_commando;
  bool is_alien;

  bool is_base;
  bool is_big;
  bool is_road;
  bool is_connector;
  bool has_effect;
  bool can_work;
  bool is_mine;
  int has_frames;
  bool is_annimated;
  bool is_bridge;
  bool is_platform;
  bool build_on_water;
  bool is_pad;
  bool is_expl_mine;
  bool can_research;
  bool build_alien;
};

struct sTerrain{
  SDL_Surface *sf,*sf_org;   // Surfaces des terrains
  SDL_Surface *shw,*shw_org; // Surfaces des terrains im Schatten
  bool water;       // Gibt an, ob es Wasser ist
  bool coast;       // Gibt an, ob es ein Küstenstück ist
  bool overlay;     // Gibt an, ob es ein Overlay ist
  bool blocked;     // Gibt an, ob es blockiert ist
  int frames;       // Anzahl an Frames
};


///////////////////////////////////////////////////////////////////////////////
// Variables-Classes
// ------------------------
// This classes are for saving global Variables needed by the game
///////////////////////////////////////////////////////////////////////////////

// SettingsData - Class containing all gamesettings ///////////////////////
/**
* cSettings class. Stores gamesettings :-)
*
* @author Bernd "beko" Kosmahl
*/
class cSettings
{
public:
	//START
	/**screen width in pixels */
	int iScreenW;
	/**screen height in pixels */
	int iScreenH;
	/**colour depth - e.g. 32*/
	int iColourDepth;
	/**enable intro on start*/
	bool bIntro;
	/**start in windowmode*/
	bool bWindowMode;
	/**start in fastmode */
	bool bFastMode;
	/**start in fastmode */
	string sLanguage;

	//GAME
	/** enable debug*/
	bool bDebug;
	/**enable autosafe */
	bool bAutoSave;
	/**enable animations */
	bool bAnimations;
	/**enable shadows */
	bool bShadows;
	/**enable alpha effects */
	bool bAlphaEffects;
	/**enable describtions (e.g. in buildmenues) */
	bool bShowDescription;
	/**enable damage effects (smoke'n stuff)*/
	bool bDamageEffects;
	/**enable damage effects for vehicles (smoke'n stuff)*/
	bool bDamageEffectsVehicles;
	/**enable tracks (units leave tracks on the floor) */
	bool bMakeTracks;
	/**scrollspeed on map */
	int iScrollSpeed;

	//NET
	/**Last/default ip used for network game */
	string sIP; //string? why not int array? --beko
	/**Last/default port  used for network game */
	int iPort;
	/**Last/default player's name used for network game */
	string sPlayerName;

	//SOUND
	/**sound enabled*/
	bool bSoundEnabled;
	/**volume music */
	int MusicVol;
	/**volume sound effects */
	int SoundVol;
	/**volume voices */
	int VoiceVol;
	/**chunk size */
	int iChunkSize;
	/**frequenzy */
	int iFrequency;
	/**mute music */
	bool MusicMute;
	/**mute sound effects */
	bool SoundMute;
	/**mute voices */
	bool VoiceMute;	// Muteeigenschaften der Sounds

	//PATHS
	string sFontPath;			// Path to the fonts
	string sFxPath;				// Path to the effects
	string sGfxPath;				// Path to the graphics
	string sMapsPath;				// Path to the maps
	string sSavesPath;			// Path to the saves
	string sSoundsPath;			// Path to the sound-files
	string sVoicesPath;			// Path to the voice-files
	string sMusicPath;			// Path to the music-files
	string sVehiclesPath;		// Path to the vehicles
	string sBuildingsPath;		// Path to the buildings

	unsigned int Checksum;		// Die Checksumme über alle Eigenschaften - NOT IN XML-file (yet?)!
} EX SettingsData;

// GraphicsData - Class containing all normal graphic surfaces ////////////////
class cGraphicsData
{
public:
	SDL_Surface *gfx_hud;
	SDL_Surface *gfx_Chand;
	SDL_Surface *gfx_Cno;
	SDL_Surface *gfx_Cselect;
	SDL_Surface *gfx_Cmove;
	SDL_Surface *gfx_Chelp;
	SDL_Surface *gfx_Cattack;
	SDL_Surface *gfx_Cpfeil1;
	SDL_Surface *gfx_Cpfeil2;
	SDL_Surface *gfx_Cpfeil3;
	SDL_Surface *gfx_Cpfeil4;
	SDL_Surface *gfx_Cpfeil6;
	SDL_Surface *gfx_Cpfeil7;
	SDL_Surface *gfx_Cpfeil8;
	SDL_Surface *gfx_Cpfeil9;
	SDL_Surface *gfx_hud_stuff;
	SDL_Surface *gfx_shadow;
	SDL_Surface *gfx_tmp;
	SDL_Surface *gfx_object_menu;
	SDL_Surface *gfx_destruction;
	SDL_Surface *gfx_destruction_glas;
	SDL_Surface *gfx_build_screen;
	SDL_Surface *gfx_Cband;
	SDL_Surface *gfx_band_small;
	SDL_Surface *gfx_band_big;
	SDL_Surface *gfx_band_small_org;
	SDL_Surface *gfx_band_big_org;
	SDL_Surface *gfx_big_beton_org;
	SDL_Surface *gfx_big_beton;
	SDL_Surface *gfx_Ctransf;
	SDL_Surface *gfx_transfer;
	SDL_Surface *gfx_mine_manager;
	SDL_Surface *gfx_fac_build_screen;
	SDL_Surface *gfx_Cload;
	SDL_Surface *gfx_Cactivate;
	SDL_Surface *gfx_storage;
	SDL_Surface *gfx_storage_ground;
	SDL_Surface *gfx_dialog;
	SDL_Surface *gfx_edock;
	SDL_Surface *gfx_Cmuni;
	SDL_Surface *gfx_Crepair;
	SDL_Surface *gfx_research;
	SDL_Surface *gfx_upgrade;
	SDL_Surface *gfx_panel_top;
	SDL_Surface *gfx_panel_bottom;
	SDL_Surface *gfx_Csteal;
	SDL_Surface *gfx_Cdisable;
	SDL_Surface *gfx_menu_stuff;
	SDL_Surface *gfx_player_pc;
	SDL_Surface *gfx_player_human;
	SDL_Surface *gfx_player_none;
	SDL_Surface *gfx_player_select;
	SDL_Surface *gfx_load_save_menu;
	SDL_Surface *gfx_exitpoints_org;
	SDL_Surface *gfx_exitpoints;
	SDL_Surface *gfx_menu_buttons;
	SDL_Surface *gfx_player_ready;

	string DialogPath;
	string Dialog2Path;
	string Dialog3Path;
} EX GraphicsData;

// Effects - Class containing all effect surfaces /////////////////////////////
class cEffectsData
{
public:
	SDL_Surface **fx_explo_big;
	SDL_Surface **fx_explo_small;
	SDL_Surface **fx_explo_water;
	SDL_Surface **fx_explo_air;
	SDL_Surface **fx_muzzle_big;
	SDL_Surface **fx_muzzle_small;
	SDL_Surface **fx_muzzle_med;
	SDL_Surface **fx_hit;
	SDL_Surface **fx_smoke;
	SDL_Surface **fx_rocket;
	SDL_Surface **fx_dark_smoke;
	SDL_Surface **fx_tracks;
	SDL_Surface **fx_corpse;
	SDL_Surface **fx_absorb;
} EX EffectsData;

// ResourceData - Class containing all resource surfaces //////////////////////
class cResourceData
{
public:
	SDL_Surface *res_metal_org;
	SDL_Surface *res_metal;
	SDL_Surface *res_oil_org;
	SDL_Surface *res_oil;
	SDL_Surface *res_gold_org;
	SDL_Surface *res_gold;
} EX ResourceData;

/*
// TerrainData - Class containing all terrain data ////////////////////////////
class cTerrainData
{
public:
	sTerrain *terrain;
	int terrain_anz;
} EX TerrainData;*/

// UnitsData - Class containing all building/vehicle surfaces & data ///////////////
class cUnitsData
{
public:
	// Vehicles
	cList<sVehicle> vehicle;

	// Buildings
	cList<sBuilding> building;

	SDL_Surface *dirt_small_org;
	SDL_Surface *dirt_small;
	SDL_Surface *dirt_small_shw_org;
	SDL_Surface *dirt_small_shw;
	SDL_Surface *dirt_big_org;
	SDL_Surface *dirt_big;
	SDL_Surface *dirt_big_shw_org;
	SDL_Surface *dirt_big_shw;

	SDL_Surface *ptr_small_beton;
	SDL_Surface *ptr_connector;
	SDL_Surface *ptr_connector_shw;
} EX UnitsData;

// OtherData - Class containing the rest of surfaces //////////////////////////
class cOtherData
{
public:
	SDL_Surface **colors;

	SDL_Surface *WayPointPfeile[8][60];
	SDL_Surface *WayPointPfeileSpecial[8][60];
} EX OtherData;


///////////////////////////////////////////////////////////////////////////////
// Predeclerations
// ------------------------
//
///////////////////////////////////////////////////////////////////////////////



void ScaleSurface(SDL_Surface *scr,SDL_Surface **dest,int size);
void ScaleSurface2(SDL_Surface *scr,SDL_Surface *dest,int size);
void ScaleSurfaceAdv(SDL_Surface *scr,SDL_Surface **dest,int sizex,int sizey);

//takes the surfaces src and scales it to match the given size.
//the result is directly written in the pixel memory of dst
void ScaleSurfaceAdv2(SDL_Surface *scr,SDL_Surface *dest,int sizex,int sizey);
void ScaleSurfaceAdv2Spec(SDL_Surface *scr,SDL_Surface *dest,int sizex,int sizey);
SDL_Surface *CreatePfeil(int p1x,int p1y,int p2x,int p2y,int p3x,int p3y,unsigned int color,int size);
void line(int x1,int y1,int x2,int y2,unsigned int color,SDL_Surface *sf);

// returns a random number in the range 0 <= r < x
int random(int x);

/**Converts integer to string
*/
std::string iToStr(int x);
/**Converts double to string
*/
std::string dToStr(double x);
int min (int a, int b );

/**
* Rounds given param num to specified position after decimal point<br>
* Example:<br>
* num := 3,234<br>
* n := 2<br>
* >>>>>>> Result = 3,23<br>
*
*@author MM
*@param num number to round up
*@param n the position after decimal point in dValueToRound, that will be rounded
*@return rounded num
*/
double Round(double num, unsigned int n);

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
int Round ( double num );

/**
*Terminates app
*@author beko
*/
void Quit();

/**
*Inits SDL
*@author beko
*@return -1 on error<br>0 on success<br>1 with warnings
*/
int initSDL();

/**
*Inits SDL_sound
*@author beko
*@return -1 on error<br>0 on success<br>1 with warnings
*/
int initSound();

/**
*Inits SDL_net
*@author beko
*@return -1 on error<br>0 on success<br>1 with warnings
*/
int initNet();

/**
*Shows splashscreen
*/
void showSplash();

/**
*Shows gamewindow
*/
void showGameWindow();

int runEventChecker( void *);

#endif
