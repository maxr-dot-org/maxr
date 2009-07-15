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
//
//
//
//
///////////////////////////////////////////////////////////////////////////////

#include "defines.h"
#include <map>
#include "main.h"
#include "sound.h"
#include "savegame.h"

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

// Typs for saving a setting
#define SAVETYPE_ANIMATIONS					0
#define SAVETYPE_SHADOWS					1
#define SAVETYPE_ALPHA						2
#define SAVETYPE_DAMAGEEFFECTS_BUILDINGS			3
#define SAVETYPE_DAMAGEEFFECTS_VEHICLES				4
#define SAVETYPE_TRACKS						5
#define SAVETYPE_AUTOSAVE					6
#define SAVETYPE_NAME						7
#define SAVETYPE_IP						8
#define SAVETYPE_PORT						9
#define SAVETYPE_MUSICMUTE					10
#define SAVETYPE_VOICEMUTE					11
#define SAVETYPE_SOUNDMUTE					12
#define SAVETYPE_MUSICVOL					13
#define SAVETYPE_VOICEVOL					14
#define SAVETYPE_SOUNDVOL					15
#define SAVETYPE_SCROLLSPEED					16
#define SAVETYPE_INTRO						17
#define SAVETYPE_WINDOW						18
#define SAVETYPE_RESOLUTION					19
#define SAVETYPE_COLOR						20
#define SAVETYPE_CACHESIZE					21

#define NECESSARY_FILE_FAILURE { Log.write ( "Missing a file needed for game. Check log and config! ", LOG_TYPE_ERROR ); LoadingData=LOAD_ERROR; return 0; }

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
	* Reads the Information out of the config file
	* @return 0 on success
	*/
int ReadMaxXml();

/**
	* Saves the acctual value of an option to the max.xml file
	* @param Typ of Data to write (see SAVETYPE-defines)
	* @return 1 on success
	*/
int SaveOption (int iTyp);

/**
 * Loades the unitdata from the data.xml in the unitfolder
 * @param directory Unitdirectory , relativ to the main game directory
 */
void LoadUnitData( sUnitData*, char const* directory, int iID );

void LoadUnitGraphicData( sUnitData*, char const* directory );

int getXMLNodeInt( TiXmlDocument &document, const char *path0 = NULL, const char *path1 = NULL,const char *path2 = NULL );
float getXMLNodeFloat( TiXmlDocument &document, const char *path0 = NULL, const char *path1 = NULL,const char *path2 = NULL );
string getXMLNodeString( TiXmlDocument &document, const char *attribut, const char *path0 = NULL, const char *path1 = NULL,const char *path2 = NULL );
bool getXMLNodeBool( TiXmlDocument &document, const char *path0 = NULL, const char *path1 = NULL,const char *path2 = NULL, const char *path3 = NULL );

/**
* Gets the name and the description for the unit from the selected language file
* @param ID Id of the unit
*/
void translateUnitData(sID ID, bool vehicle);

void reloadUnitValues ();

/**
* This is where paths are figured out - e.g. $HOME
*/
void setPaths ();

/**
* This is where the game data folder is searched - e.g. /usr/share/maxr
*/
std::string searchData ( std::string sDataDirFromConf="");

/**
* Generates a new config file.
* @return 0 on success<br>-1 on fail
**/
int GenerateMaxXml();
#endif
