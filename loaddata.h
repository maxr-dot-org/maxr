///////////////////////////////////////////////////////////////////////////////
//
// M.A.X. Reloaded - loaddata.h
//
///////////////////////////////////////////////////////////////////////////////
//
// 
// 
//
///////////////////////////////////////////////////////////////////////////////

#include "defines.h"
#include <map>
#include "main.h"
#include "sound.h"

#ifndef loaddataH
#define loaddataH

///////////////////////////////////////////////////////////////////////////////
// Defines
// ------------------------
// 
///////////////////////////////////////////////////////////////////////////////

/*
// Header
#define VEH_MIN_GAME_VERSION			""
#define VEH_VERSION						""
#define VEH_NAME						""
#define VEH_DESCRIPTION					""

// General
#define VEH_IS_CONTROLLABLE				"Unit;General_Info;Is_Controllable"
#define VEH_CAN_BE_CAPTURED				"Unit;General_Info;Can_Be_Captured"
#define VEH_CAN_BE_DISABLED				"Unit;General_Info;Can_Be_Disabled"
#define VEH_LENGHT						"Unit;General_Info;Size_Length"
#define VEH_WIDTH						"Unit;General_Info;Size_Width"

// Defence
#define VEH_IS_TARGET_LAND				"Unit;Defence;Is_Target_Land"
#define VEH_IS_TARGET_SEA				"Unit;Defence;Is_Target_Sea"
#define VEH_IS_TARGET_AIR				"Unit;Defence;Is_Target_Air"
#define VEH_IS_TARGET_UNDERWATER		"Unit;Defence;Is_Target_Underwater"
#define VEH_IS_TARGET_MINE				"Unit;Defence;Is_Target_Mine"
#define VEH_IS_TARGET_BUILDING			"Unit;Defence;Is_Target_Building"
#define VEH_IS_TARGET_SATELLITE			"Unit;Defence;Is_Target_Satellite"
#define VEH_IS_TARGET_WMD				"Unit;Defence;Is_Target_WMD"
#define VEH_ARMOR						"Unit;Defence;Armor"
#define VEH_HITPOINTS					"Unit;Defence;Hitpoints"				

// Production
#define VEH_BUILD_COSTS					"Unit;Production;Built_Costs"
#define VEH_BUILD_COSTS_MAX				"Unit;Production;Built_Costs_Max"
#define VEH_IS_PRDUCED_BY_ID			"Unit;Production;Is_Produced_by", "Unit_ID"

// Weapons
#define VEH_SHOT_TRAJECTORY				"Unit;Weapons;Weapon;Shot_Trajectory"
#define VEH_AMMO_TYPE					"Unit;Weapons;Weapon;Ammo_Type"
#define VEH_AMMO_QUANTITY				"Unit;Weapons;Weapon;Ammo_Quantity"

#define VEH_TARGET_LAND_DEMAGE			"Unit;Weapons;Weapon;Allowed_Targets;Target_Land;Damage"
#define VEH_TARGET_LAND_RANGE			"Unit;Weapons;Weapon;Allowed_Targets;Target_Land;Range"
#define VEH_TARGET_SEA_DEMAGE			"Unit;Weapons;Weapon;Allowed_Targets;Target_Sea;Damage"
#define VEH_TARGET_SEA_RANGE			"Unit;Weapons;Weapon;Allowed_Targets;Target_Sea;Range"
#define VEH_TARGET_AIR_DEMAGE			"Unit;Weapons;Weapon;Allowed_Targets;Target_Air;Damage"
#define VEH_TARGET_AIR_RANGE			"Unit;Weapons;Weapon;Allowed_Targets;Target_Air;Range"
#define VEH_TARGET_MINE_DEMAGE			"Unit;Weapons;Weapon;Allowed_Targets;Target_Mine;Damage"
#define VEH_TARGET_MINE_RANGE			"Unit;Weapons;Weapon;Allowed_Targets;Target_Mine;Range"
#define VEH_TARGET_SUBMARINE_DEMAGE		"Unit;Weapons;Weapon;Allowed_Targets;Target_Submarine;Damage"
#define VEH_TARGET_SUBMARINE_RANGE		"Unit;Weapons;Weapon;Allowed_Targets;Target_Submarine;Range"
#define VEH_TARGET_INFANTRY_DEMAGE		"Unit;Weapons;Weapon;Allowed_Targets;Target_Infantry;Damage"
#define VEH_TARGET_INFANTRY_RANGE		"Unit;Weapons;Weapon;Allowed_Targets;Target_Infantry;Range"
#define VEH_TARGET_WMD_DEMAGE			"Unit;Weapons;Weapon;Allowed_Targets;Target_WMD;Damage"
#define VEH_TARGET_WMD_RANGE			"Unit;Weapons;Weapon;Allowed_Targets;Target_WMD;Range"

#define VEH_SHOTS						"Unit;Weapons;Weapon;Shots"
#define VEH_DESTINATION_AREA			"Unit;Weapons;Weapon;Destination_Area"
#define VEH_DESTINATION_TYPE			"Unit;Weapons;Weapon;Destination_Type"
#define VEH_MOVEMENT_ALLOWED			"Unit;Weapons;Weapon;Movement_Allowed"

// Abilities
#define VEH_CAN_CLEAR_AREA				"Unit;Abilities;Can_Clear_Area"
#define VEH_GETS_EXPERIENCE				"Unit;Abilities;Gets_Experience"
#define VEH_CAN_DISABLE					"Unit;Abilities;Can_Disable"
#define VEH_CAN_CAPTURE					"Unit;Abilities;Can_Capture"
#define VEH_CAN_DIVE					"Unit;Abilities;Can_Dive"
#define VEH_LANDING_TYPE				"Unit;Abilities;Landing_Type"	
#define VEH_CAN_UPGRADE					"Unit;Abilities;Can_Upgrade"
#define VEH_CAN_REPAIR					"Unit;Abilities;Can_Repair"
#define VEH_CAN_RESEARCH				"Unit;Abilities;Is_Kamikaze"
#define VEH_IS_INFRASTRUCTURE			"Unit;Abilities;Is_Infrastructure"
#define VEH_IS_KAMIKAZE					"Unit;Abilities;Can_Place_Mines"		
#define VEH_CAN_PLACE_MINES				"Unit;Abilities;Can_Place_Mines"
#define VEH_MAKES_TRACKS				"Unit;Abilities;Makes_Tracks"
#define VEH_SELF_REPAIR_TYPE			"Unit;Abilities;Self_Repair_Type"
#define VEH_CONVERTS_GOLD				"Unit;Abilities;Converts_Gold"
#define VEH_NEEDS_ENERGY				"Unit;Abilities;Needs_Energy"
#define VEH_NEEDS_OIL					"Unit;Abilities;Needs_Oil"
#define VEH_NEEDS_METALL				"Unit;Abilities;Needs_Metall"
#define VEH_NEEDS_HUMANS				"Unit;Abilities;Needs_Humans"
#define VEH_MINES_RESOURCES				"Unit;Abilities;Mines_Resources"
#define VEH_CAN_LAUNCH_SRBM				"Unit;Abilities;Can_Launch_SRBM"
#define VEH_ENERGY_SHIELD_STRENGTH		"Unit;Abilities;Energy_Shield_Strength"
#define VEH_ENERGY_SHIELD_SIZE			"Unit;Abilities;Energy_Shield_Size"

// Scan_Abilities
#define VEH_RANGE_SIGHT					"Unit;Scan_Abilities;Range_Sight"
#define VEH_RANGE_AIR					"Unit;Scan_Abilities;Range_Air"
#define VEH_RANGE_GROUND				"Unit;Scan_Abilities;Range_Ground"
#define VEH_RANGE_SEA					"Unit;Scan_Abilities;Range_Sea"
#define VEH_RANGE_SUBMARINE				"Unit;Scan_Abilities;Range_Submarine"
#define VEH_RANGE_MINE					"Unit;Scan_Abilities;Range_Mine"
#define VEH_RANGE_INFANTRY				"Unit;Scan_Abilities;Range_Infantry"
#define VEH_RANGE_RESOURCES				"Unit;Scan_Abilities;Range_Resources"
#define VEH_RANGE_JAMMER				"Unit;Scan_Abilities;Range_Jammer"

// Movement
#define VEH_MOVEMENT_SUM				"Unit;Movement;Movement_Sum"
#define VEH_COSTS_AIR					"Unit;Movement;Costs_Air"
#define VEH_COSTS_SEA					"Unit;Movement;Costs_Sea"
#define VEH_COSTS_SUBMARINE				"Unit;Movement;Costs_Submarine"
#define VEH_COSTS_GROUND				"Unit;Movement;Costs_Ground"
#define VEH_FACTOR_COAST				"Unit;Movement;Factor_Coast"
#define VEH_FACTOR_WOOD					"Unit;Movement;Factor_Wood"
#define VEH_FACTOR_ROAD					"Unit;Movement;Factor_Road"
#define VEH_FACTOR_BRIDGE				"Unit;Movement;Factor_Bridge"
#define VEH_FACTOR_PLATFORM				"Unit;Movement;Factor_Platform"
#define VEH_FACTOR_MONORAIL				"Unit;Movement;Factor_Monorail"
#define VEH_FACTOR_WRECK				"Unit;Movement;Factor_Wreck"
#define VEH_FACTOR_MOUNTAINS			"Unit;Movement;Factor_Mountains"

// Storage
#define VEH_IS_GARAGE					"Unit;Storage;Is_Garage"
#define VEH_CAPACITY_METAL				"Unit;Storage;Capacity_Metal"
#define VEH_CAPACITY_OIL				"Unit;Storage;Capacity_Oil"
#define VEH_CAPACITY_GOLD				"Unit;Storage;Capacity_Gold"
#define VEH_CAPACITY_ENERGY				"Unit;Storage;Capacity_Energy"
#define VEH_CAPACITY_UNITS_AIR			"Unit;Storage;Capacity_Units_Air"
#define VEH_CAPACITY_UNITS_SEA			"Unit;Storage;Capacity_Units_Sea"
#define VEH_CAPACITY_UNITS_GROUND		"Unit;Storage;Capacity_Units_Ground"
#define VEH_CAPACITY_UNITS_INFANTRY		"Unit;Storage;Capacity_Units_Infantry"
#define VEH_CAN_USE_UNIT_AS_GARAGE_ID	"Unit;Storage;Can_Use_Unit_As_Garage;Unit_ID"*/


#define LOAD_GOING 0
#define LOAD_ERROR 1
#define LOAD_FINISHED 2

#define NECESSARY_FILE_FAILURE { cLog::write ( "File for game needed! ", LOG_TYPE_ERROR ); LoadingData=LOAD_ERROR; return 0; }

///////////////////////////////////////////////////////////////////////////////
// Globals
// ------------------------
// 
///////////////////////////////////////////////////////////////////////////////

EX int LoadingData;

///////////////////////////////////////////////////////////////////////////////
// Predeclerations
// ------------------------
// 
///////////////////////////////////////////////////////////////////////////////

/**
	* Loads all relevant files and datas
	* @return 1 on success
	*/
int LoadData(void *);
/**
	* Writes a Logmessage on the SplashScreen
	* @param sztxt Text to write
	* @param ok If set writes "OK" at the end
	* @param pos Horizontal Positionindex on SplashScreen
	*/
void MakeLog(const char* sztxt,bool ok,int pos);
/**
	* Loades a graphic to the surface
	* @param dest Destination surface
	* @param directory Directory of the file
	* @param filename Name of the file
	* @return 1 on success
	*/
int LoadGraphicToSurface(SDL_Surface* &dest, const char* directory, const char* filename);
/**
	* Loades a effectgraphic to the surface
	* @param dest Destination surface
	* @param directory Directory of the file
	* @param filename Name of the file
	* @return 1 on success
	*/
int LoadEffectGraphicToSurface(SDL_Surface** &dest, const char* directory, const char* filename);
/**
	* Loades a effectgraphic with alpha to the surface
	* @param dest Destination surface
	* @param directory Directory of the file
	* @param filename Name of the file
	* @param aplha Strength og alpha (from 0=transparent to 255=opaque)
	* @return 1 on success
	*/
int LoadEffectAplhaToSurface(SDL_Surface** &dest, const char* directory, const char* filename, int alpha);
/**
	* Loades a soundfile to the Mix_Chunk
	* @param dest Destination Mix_Chunk
	* @param directory Directory of the file
	* @param filename Name of the file
	* @return 1 on success
	*/
int LoadSoundfile(sSOUND *&dest, const char* directory, const char* filename);
/**
	* Loades a vehiclesoundfile to the Mix_Chunk. If the file doesn't exists a dummy file will be loaded
	* @param dest Destination Mix_Chunk
	* @param directory Directory of the file, relativ to the main vehicles directory
	* @param filename Name of the file
	*/
void LoadVehicleSoundfile(sSOUND *&dest, const char* directory, const char* filename);
/**
	* Loades the vehicledata from the data.xml in the vehiclesfolder
	* @param vehiclenum Indexnumber of vehicle for which the data should be loaded.
	* @param directory Vehicledirectory , relativ to the main game directory
	* @return 1 on success
	*/
int LoadVehicleData(int vehiclenum, const char* directory);
/**
	* Checks whether a file exits
	* @param directory Directory to the file
	* @param filename Name of the file
	* @return 1 on success
	*/
int CheckFile(const char* directory, const char* filename);
/**
	* Reads the Information out of the max.xml
	*/
int ReadMaxXml();
/**
	* Loads the selected languagepack
	* @return 1 on success
	*/
int LoadLanguage();
/**
	* Generats a new max.xml file
	*/
int GenerateMaxXml();

/**
	* Loads the rest of the fonts
	* @param path Directory of the fonts
	* @return 1 on success
	*/
int LoadFonts(const char* path);
/**
	* Loads all Fonts
	* @param path Directory of the graphics
	* @return 1 on success
	*/
int LoadGraphics(const char* path);
/**
	* Loads the Terrain
	* @param path Directory of the Terrain
	* @return 1 on success
	*/
int LoadTerrain(const char* path);
/**
	* ??
	* @param src Source surface
	* @param dest Destination surface
	*/
void DupSurface(SDL_Surface *&src,SDL_Surface *&dest);
/**
	* Loads the Effects
	* @param path Directory of the Effects
	* @return 1 on success
	*/
int LoadEffects(const char* path);
/**
	* Loads all Buildings
	* @param path Directory of the Buildings
	* @return 1 on success
	*/
int LoadBuildings(const char* path);
/**
	* Loads all Vehicles
	* @param path Directory of the Vehicles
	* @return 1 on success
	*/
int LoadVehicles(const char* path);
/**
	* Loads all Musicfiles
	* @param path Directory of the Vehicles
	* @return 1 on success
	*/
int LoadMusic(const char* path);
/**
	* Loads all Sounds
	* @param path Directory of the Vehicles
	* @return 1 on success
	*/
int LoadSounds(const char* path);
/**
	* Loads all Voices
	* @param path Directory of the Vehicles
	* @return 1 on success
	*/
int LoadVoices(const char* path);

#endif
