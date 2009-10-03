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

#include <iostream>
#include <vector>
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
class cAJobs;
class cLanguage;
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
struct sTurnstartReport;
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
#define GRID_COLOR			0x305C04	// color of the grid
#define SCAN_COLOR			0xE3E300	// color of scan circles
#define RANGE_GROUND_COLOR	0xE20000	// color of range circles for ground attack
#define RANGE_AIR_COLOR		0xFCA800	// color of range circles for air attack
#define PFEIL_COLOR			0x00FF00	// color of a waypointarrow
#define PFEILS_COLOR		0x0000FF	// color of a special waypointarrow
#define MOVE_SPEED			7			// speed of vehcilemovements
#define MSG_TICKS			3000		// number of ticks for how long a message will be displayed
#define ANIMATION_SPEED		((int)(Client->iTimerTime/(2)))		// this means every 100ms becouse Client->iTimerTime will increase every 50ms.
#define LANDING_DISTANCE_WARNING	28
#define LANDING_DISTANCE_TOO_CLOSE	10

//minimap configuration
#define MINIMAP_COLOR		0xFC0000 //color of the screen borders on the minimap
#define MINIMAP_POS_X		15		 //the position of the map on the screen
#define MINIMAP_POS_Y		356		 //the position of the map on the screen
#define MINIMAP_SIZE		112		 //the size of the minimap in pixels
#define MINIMAP_ZOOM_FACTOR	2		 //the zoomfactor for minimap zoom switch

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

///////////////////////////////////////////////////////////////////////////////
// Structures
// ------------------------
//
///////////////////////////////////////////////////////////////////////////////

struct sUnitData;

// Struktur f¸r die IDs
struct sID
{
	sID () : iFirstPart(0), iSecondPart(0) {};

	int iFirstPart;
	int iSecondPart;
	string getText();
	void generate( string text );
	/** Get the most modern version of a unit, that a player has (including all his upgrades) researched. (Example: Newly built
		units will have these values. */
	sUnitData *getUnitDataCurrentVersion (cPlayer *Owner);
	/** Get the basic version of a unit.
		@param Owner If a owner is given, the basic version of this player is returned (with possible clan modifications).
					 If no owner is given, the basic version without any clan modifications is returned.
		@return the sUnitData of the owner without upgrades (but with the owner's clan modifications) */
	sUnitData *getUnitDataOriginalVersion (cPlayer *Owner = NULL);

	/** Returns the original version of a vehicle as stored in UnitsData. If Owner is given, his clan will be taken
		into consideration for modifications of the unit's values. */
	sVehicle *getVehicle (cPlayer* Owner = NULL);
	/** Returns the original version of a building as stored in UnitsData. If Owner is given, his clan will be taken
		into consideration for modifications of the unit's values. */
	sBuilding *getBuilding (cPlayer* Owner = NULL);

	bool operator==(const sID &ID) const;
};

// Nummbers of buildings //////////////////////////////////////////////////////
EX sID specialIDLandMine;
EX sID specialIDSeaMine;
EX sID specialIDMine;
EX sID specialIDSmallGen;
EX sID specialIDConnector;
EX sID specialIDSmallBeton;

// Struktur f¸r die Eigenschaften der Vehicles:
struct sUnitData
{
	sUnitData();
#define TERRAIN_NONE		0
#define TERRAIN_AIR			1
#define TERRAIN_SEA			2
#define TERRAIN_GROUND		4
#define TERRAIN_COAST		8
#define AREA_SUB			16
#define AREA_EXP_MINE		32

	// Main
	sID ID;
	string name;
	string description;
	int version;

	// Attack
	enum eMuzzleType
	{
		MUZZLE_TYPE_NONE,
		MUZZLE_TYPE_BIG,
		MUZZLE_TYPE_ROCKET,
		MUZZLE_TYPE_SMALL,
		MUZZLE_TYPE_MED,
		MUZZLE_TYPE_MED_LONG,
		MUZZLE_TYPE_ROCKET_CLUSTER,
		MUZZLE_TYPE_TORPEDO,
		MUZZLE_TYPE_SNIPER
	};
	eMuzzleType muzzleType;

	int ammoMax;
	int ammoCur;
	int shotsMax;
	int shotsCur;
	int range;
	int damage;

	char canAttack;

	bool canDriveAndFire;

	// Production
	int buildCosts;
	string canBuild;
	string buildAs;

	int maxBuildFactor;

	bool canBuildPath;
	bool canBuildRepeat;
	bool buildIntern;

	// Movement
	int speedMax;
	int speedCur;

	float factorGround;
	float factorSea;
	float factorAir;
	float factorCoast;

	// Abilities
	bool isBig;
	bool connectsToBase;
	int armor;
	int hitpointsMax;
	int hitpointsCur;
	int scan;
	float modifiesSpeed;
	bool canClearArea;
	bool canBeCaptured;
	bool canBeDisabled;
	bool canCapture;
	bool canDisable;
	bool canRepair;
	bool canRearm;
	bool canResearch;
	bool canPlaceMines;
	bool canSurvey;
	bool doesSelfRepair;
	int convertsGold;
	bool canSelfDestroy;

	int canMineMaxRes;

	int needsMetal;
	int needsOil;
	int needsEnergy;
	int needsHumans;
	int produceEnergy;
	int produceHumans;

	char isStealthOn;
	char canDetectStealthOn;

	enum eSurfacePosition
	{
		SURFACE_POS_BENEATH_SEA,
		SURFACE_POS_ABOVE_SEA,
		SURFACE_POS_BASE,
		SURFACE_POS_ABOVE_BASE,
		SURFACE_POS_GROUND,
		SURFACE_POS_ABOVE
	};
	eSurfacePosition surfacePosition;

	enum eOverbuildType
	{
		OVERBUILD_TYPE_NO,
		OVERBUILD_TYPE_YES,
		OVERBUILD_TYPE_YESNREMOVE
	};
	eOverbuildType canBeOverbuild;

	bool canBeLandedOn;
	bool canWork;
	bool explodesOnContact;
	bool isHuman;

	// Storage
	int storageResMax;
	int storageResCur;
	enum eStorageResType
	{
		STORE_RES_NONE,
		STORE_RES_METAL,
		STORE_RES_OIL,
		STORE_RES_GOLD
	};
	eStorageResType storeResType;

	int storageUnitsMax;
	int storageUnitsCur;
	enum eStorageUnitsImageType
	{
		STORE_UNIT_IMG_NONE,
		STORE_UNIT_IMG_TANK,
		STORE_UNIT_IMG_PLANE,
		STORE_UNIT_IMG_SHIP,
		STORE_UNIT_IMG_HUMAN
	};
	eStorageUnitsImageType storeUnitsImageType;
	vector<string> storeUnitsTypes;
	string isStorageType;

	// Graphic
	bool hasClanLogos;
	bool hasCorpse;
	bool hasDamageEffect;
	bool hasBetonUnderground;
	bool hasPlayerColor;
	bool hasOverlay;

	bool buildUpGraphic;
	bool animationMovement;
	bool powerOnGraphic;
	bool isAnimated;
	bool makeTracks;

	bool isConnectorGraphic;
	int hasFrames;
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
	SDL_Surface *gfx_context_menu;
	SDL_Surface *gfx_destruction;
	SDL_Surface *gfx_destruction_glas;
	SDL_Surface *gfx_Cband;
	SDL_Surface *gfx_band_small;
	SDL_Surface *gfx_band_big;
	SDL_Surface *gfx_band_small_org;
	SDL_Surface *gfx_band_big_org;
	SDL_Surface *gfx_big_beton_org;
	SDL_Surface *gfx_big_beton;
	SDL_Surface *gfx_Ctransf;
	SDL_Surface *gfx_Cload;
	SDL_Surface *gfx_Cactivate;
	SDL_Surface *gfx_storage;
	SDL_Surface *gfx_storage_ground;
	SDL_Surface *gfx_dialog;
	SDL_Surface *gfx_edock;
	SDL_Surface *gfx_ehangar;
	SDL_Surface *gfx_edepot;
	SDL_Surface *gfx_Cmuni;
	SDL_Surface *gfx_Crepair;
	SDL_Surface *gfx_panel_top;
	SDL_Surface *gfx_panel_bottom;
	SDL_Surface *gfx_Csteal;
	SDL_Surface *gfx_Cdisable;
	SDL_Surface *gfx_menu_stuff;
	SDL_Surface *gfx_hud_extra_players;
	SDL_Surface *gfx_player_pc;
	SDL_Surface *gfx_player_human;
	SDL_Surface *gfx_player_none;
	SDL_Surface *gfx_player_select;
	SDL_Surface *gfx_exitpoints_org;
	SDL_Surface *gfx_exitpoints;
	SDL_Surface *gfx_menu_buttons;
	SDL_Surface *gfx_player_ready;
	SDL_Surface *gfx_hud_chatbox;

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

// UnitsData - Class containing all building/vehicle surfaces & data ///////////////
class cUnitsData
{
public:
	cUnitsData ();

	// Vehicles
	cList<sVehicle> vehicle; // the standard version without clan modifications

	// Buildings
	cList<sBuilding> building;  // the standard version without clan modifications

	sVehicle& getVehicle (int nr, int clan = -1); ///< -1: game without clans
	sBuilding& getBuilding (int nr, int clan = -1); ///< -1: game without clans

	unsigned int getNrVehicles () const { return (int) vehicle.Size (); }
	unsigned int getNrBuildings () const { return (int) building.Size (); }


	SDL_Surface *dirt_small_org;
	SDL_Surface *dirt_small;
	SDL_Surface *dirt_small_shw_org;
	SDL_Surface *dirt_small_shw;
	SDL_Surface *dirt_big_org;
	SDL_Surface *dirt_big;
	SDL_Surface *dirt_big_shw_org;
	SDL_Surface *dirt_big_shw;

	SDL_Surface *ptr_small_beton;
	SDL_Surface *ptr_small_beton_org;
	SDL_Surface *ptr_connector;
	SDL_Surface *ptr_connector_org;
	SDL_Surface *ptr_connector_shw;
	SDL_Surface *ptr_connector_shw_org;

//------------------------------------------------------------
private:
	void initializeClanUnitData ();

	std::vector<std::vector<sVehicle> > clanUnitDataVehicles; // contains the modified versions for the clans
	std::vector<std::vector<sBuilding> > clanUnitDataBuildings; // cotains the modified versions for the clans
	bool initializedClanUnitData;

} EX UnitsData;

// OtherData - Class containing the rest of surfaces //////////////////////////
class cOtherData
{
public:
	SDL_Surface **colors;
	Uint32 iSurface;
	SDL_Surface *WayPointPfeile[8][60];
	SDL_Surface *WayPointPfeileSpecial[8][60];
} EX OtherData;

///////////////////////////////////////////////////////////////////////////////
// Predeclerations
// ------------------------
//
///////////////////////////////////////////////////////////////////////////////

/** this macro checks, wether the surface has to be rescaled, and scales it if nessesary */
#define CHECK_SCALING( surface, surface_org, factor ) \
	if ( !SettingsData.bPreScale && ( (surface)->w != (int)((surface_org)->w * (factor)) || (surface)->h != (int)((surface_org)->h * (factor)) ) )  \
		scaleSurface ( (surface_org), (surface), (int)((surface_org)->w * (factor)), (int)((surface_org)->h * (factor)) );

/**
 * scale a surface to the overgiven sice. The scaled surface will be drawn to the destination surface.
 * If the destiniation surface is NULL a new surface will be created.
 * @author alzi alias DoctorDeath
 * @param scr the surface to be scaled.
 * @param dest the surface that will receive the new pixel data. If it is NUll a new surface will be created.
 * @param width width of the new surface.
 * @param height height of the new surface.
 * @return returns the destination surface.
 */
SDL_Surface *scaleSurface( SDL_Surface *scr, SDL_Surface *dest, int width, int height);

SDL_Surface *CreatePfeil(int p1x,int p1y,int p2x,int p2y,int p3x,int p3y,unsigned int color,int size);

/** Draws a line on the surface */
void line(int x1,int y1,int x2,int y2,unsigned int color,SDL_Surface *sf);
/** Draws a circle on the surface */
void drawCircle( int iX, int iY, int iRadius, int iColor, SDL_Surface *surface );
/** Sets a pixel on the surface */
void setPixel( SDL_Surface* surface, int x, int y, int iColor );

// returns a random number in the range 0 <= r < x
int random(int x);

/**Converts integer to string
*/
std::string iToStr(int x);
/**Converts double to string
*/
std::string dToStr(double x);
/**Converts pointer to string
*/
std::string pToStr(void *x);

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

/**
* Works like SDL_BlittSurface. But unlike SDL it respects the destination alpha channel of the surface.
* This function is only designed to blitt from a surface with per surface alpha value to a surface with alpha channel.
* A source color key is also supported.
*/
void blittPerSurfaceAlphaToAlphaChannel(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
/**
* Works like SDL_BlittSurface. This function choses the right blitt function to use for blitting.
*/
void blittAlphaSurface (SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
#endif
