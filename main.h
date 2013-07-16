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
#include "autosurface.h"
#include "defines.h"
#include "language.h"

// Predeclarations
class cPlayer;
class cLanguage;
struct sBuilding;
struct sBuildingUIData;
struct sVehicle;
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
	 * @param Owner If a owner is given,
	 *        the basic version of this player is returned
	 *        (with possible clan modifications).
	 *        If no owner is given,
	 *        the basic version without any clan modifications is returned.
	 * @return the sUnitData of the owner without upgrades
	 *         (but with the owner's clan modifications) */
	sUnitData* getUnitDataOriginalVersion (cPlayer* Owner = NULL) const;

	bool operator== (const sID& ID) const;
	bool operator!= (const sID& rhs) const { return !(*this == rhs);}
	bool less_vehicleFirst (const sID& ID) const;
	bool less_buildingFirst (const sID& ID) const;

private:
	/** Returns the original version of a vehicle as stored in UnitsData.
	 * If Owner is given, his clan will be taken
	 * into consideration for modifications of the unit's values. */
	sVehicle* getVehicle (cPlayer* Owner = NULL) const;
	/** Returns the original version of a building as stored in UnitsData.
	 * If Owner is given, his clan will be taken
	 * into consideration for modifications of the unit's values. */
	sBuilding* getBuilding (cPlayer* Owner = NULL) const;

public:
	int iFirstPart;
	int iSecondPart;
};

enum {
	TERRAIN_NONE = 0,
	TERRAIN_AIR = 1,
	TERRAIN_SEA = 2,
	TERRAIN_GROUND = 4,
	TERRAIN_COAST = 8,
	AREA_SUB = 16,
	AREA_EXP_MINE = 32
};

// struct for vehicle properties
struct sUnitData
{
	sUnitData();

	// Main
	sID ID;
	std::string name;
	std::string description;
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
	std::string canBuild;
	std::string buildAs;

	int maxBuildFactor;

	bool canBuildPath;
	bool canBuildRepeat;

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
	bool canScore;

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
	std::vector<std::string> storeUnitsTypes;
	std::string isStorageType;

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
	AutoSurface gfx_Chand;
	AutoSurface gfx_Cno;
	AutoSurface gfx_Cselect;
	AutoSurface gfx_Cmove;
	AutoSurface gfx_Chelp;
	AutoSurface gfx_Cattack;
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

	sVehicle& getVehicle (int nr, int clan = -1);  ///< -1: game without clans
	sBuilding& getBuilding (int nr, int clan = -1);  ///< -1: game without clans

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
	std::vector<sVehicle> svehicles;

	// Buildings
	// the standard version without clan modifications
	std::vector<sBuilding> sbuildings;

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
	std::vector<std::vector<sVehicle> > clanUnitDataVehicles;
	// contains the modified versions for the clans
	std::vector<std::vector<sBuilding> > clanUnitDataBuildings;
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
	AutoSurface colors[PLAYERCOLORS];
	AutoSurface colors_org[PLAYERCOLORS];
	AutoSurface WayPointPfeile[8][60];
	AutoSurface WayPointPfeileSpecial[8][60];
} EX OtherData;

///////////////////////////////////////////////////////////////////////////////
// Predeclerations
// ------------------------
//
///////////////////////////////////////////////////////////////////////////////

/**
 * scale a surface to the overgiven sice.
 * The scaled surface will be drawn to the destination surface.
 * If the destiniation surface is NULL a new surface will be created.
 * @author alzi alias DoctorDeath
 * @param scr the surface to be scaled.
 * @param dest the surface that will receive the new pixel data.
 *        If it is NUll a new surface will be created.
 * @param width width of the new surface.
 * @param height height of the new surface.
 * @return returns the destination surface.
 */
SDL_Surface* scaleSurface (SDL_Surface* scr, SDL_Surface* dest, int width, int height);

/** this function checks, whether the surface has to be rescaled,
 * and scales it if necessary */
inline void CHECK_SCALING (SDL_Surface* surface, SDL_Surface* surface_org, float factor)
{
	if (!cSettings::getInstance().shouldDoPrescale() &&
		(surface->w != (int) (surface_org->w * factor) ||
		 surface->h != (int) (surface_org->h * (factor))))
		scaleSurface (surface_org, surface, (int) (surface_org->w * factor), (int) (surface_org->h * factor));
}

SDL_Surface* CreatePfeil (int p1x, int p1y, int p2x, int p2y, int p3x, int p3y, unsigned int color, int size);

/** Draws a line on the surface */
void line (int x1, int y1, int x2, int y2, unsigned int color, SDL_Surface* sf);
/** Draws a circle on the surface */
void drawCircle (int iX, int iY, int iRadius, int iColor, SDL_Surface* surface);
/** Sets a pixel on the surface */
void setPixel (SDL_Surface* surface, int x, int y, int iColor);

// returns a random number in the range 0 <= r < x
int random (int x);

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
std::string pToStr (void* x);

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

/**
* Works like SDL_BlittSurface.
* But unlike SDL it respects the destination alpha channel of the surface.
* This function is only designed to blitt from a surface
* with per surface alpha value to a surface with alpha channel.
* A source color key is also supported.
*/
void blittPerSurfaceAlphaToAlphaChannel (SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect);
/**
* Works like SDL_BlittSurface.
* This function choses the right blitt function to use for blitting.
*/
void blittAlphaSurface (SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect);

void Quit();

#endif
