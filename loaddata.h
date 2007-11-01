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
	* Loades a unitsoundfile to the Mix_Chunk. If the file doesn't exists a dummy file will be loaded
	* @param dest Destination Mix_Chunk
	* @param directory Directory of the file, relativ to the main vehicles directory
	* @param filename Name of the file
	*/
void LoadUnitSoundfile(sSOUND *&dest, const char* directory, const char* filename);
/**
	* Loades the unitdata from the data.xml in the unitfolder
	* @param unitnum Indexnumber of unit for which the data should be loaded.
	* @param directory Unitdirectory , relativ to the main game directory
	* @param vehicle Should be true if unit is a vehicle
	* @return 1 on success
	*/
void LoadUnitData(int unitnum, const char* directory, bool vehicle, int ID);
/**
	* Sets all unitdata to default values
	* @param unitnum Indexnumber of unit for which the data should be loaded.
	* @param vehicle Should be true if unit is a vehicle
	* @param ID The ID which the unit should have
	* @return 1 on success
	*/
void SetDefaultUnitData(int unitnum, bool vehicle);
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
int LoadBuildings();
/**
	* Loads all Vehicles
	* @param path Directory of the Vehicles
	* @return 1 on success
	*/
int LoadVehicles();
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
