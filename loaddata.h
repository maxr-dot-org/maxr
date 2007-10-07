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
#include "main.h"

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
int LoadGraphicToSurface(SDL_Surface *&dest, const char* directory, const char* filename);
/**
	* Checks whether a file exits
	* @param directory Directory of the file
	* @param filename Name of the file
	* @return 1 on success
	*/
int CheckFile(const char* directory, const char* filename);
/**
	* Reads the Information out of the max.xml
	*/
void ReadMaxXml();
/**
	* Generats a new max.xml file
	*/
void GenerateMaxXml();

/**
	* Loads all Graphics
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

#endif
