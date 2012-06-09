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

#ifndef loaddataH
#define loaddataH

#include <map>
#include <string>
#include "defines.h"
#include "main.h" // for sID

class TiXmlDocument;
struct sUnitData;

///////////////////////////////////////////////////////////////////////////////
// Defines
// ------------------------
//
///////////////////////////////////////////////////////////////////////////////
#define LOAD_GOING 0
#define LOAD_ERROR 1
#define LOAD_FINISHED 2

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
int LoadData (void*);

/**
 * Loades the unitdata from the data.xml in the unitfolder
 * @param directory Unitdirectory , relativ to the main game directory
 */
void LoadUnitData (sUnitData*, char const* directory, int iID);

void LoadUnitGraphicData (sUnitData*, char const* directory);

int getXMLNodeInt (TiXmlDocument& document, const char* path0 = NULL, const char* path1 = NULL, const char* path2 = NULL);
float getXMLNodeFloat (TiXmlDocument& document, const char* path0 = NULL, const char* path1 = NULL, const char* path2 = NULL);
std::string getXMLNodeString (TiXmlDocument& document, const char* attribut, const char* path0 = NULL, const char* path1 = NULL, const char* path2 = NULL);
bool getXMLNodeBool (TiXmlDocument& document, const char* path0 = NULL, const char* path1 = NULL, const char* path2 = NULL, const char* path3 = NULL);

/**
* Gets the name and (text) description for clan with internal id num from language file
* If no translation exists a warning is issued and the existing strings are not altered
* @param num engine internal ID of clan sorted by oder of clans in clan.xml
*/
void translateClanData (int num);

/**
* Gets the name and the description for the unit from the selected language file
* @param ID Id of the unit
*/
bool translateUnitData (sID ID, bool vehicle);

void reloadUnitValues();

#endif
