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
#ifndef savegameH
#define savegameH
#include "defines.h"
#include "main.h"
#include "tinyxml.h"
#include "map.h"
#include "player.h"

class cResearch;

#define SAVE_FORMAT_VERSION		"0.2"

//--------------------------------------------------------------------------
struct sMoveJobLoad
{
	cVehicle *vehicle;
	int destX, destY;
};

//--------------------------------------------------------------------------
struct sSubBaseLoad
{
	sSubBaseLoad() : SubBase(0) {};
	sSubBase SubBase;
	cPlayer *Player;
};

//--------------------------------------------------------------------------
/**
* Class for loading and saving savegames
*@author alzi alias DoctorDeath
*/
//--------------------------------------------------------------------------
class cSavegame
{
public:
	cSavegame ( int number );

	/* saves the current gamestate to a file */
	int save( string saveName );
	/* loads a savegame */
	int load();

	/* loads the header of a savefile and returns some values to the pointers */
	void loadHeader( string *name, string *type, string *time );
	string getMapName();
	string getPlayerNames();

//--------------------------------------------------------------------------
private:
	/* the number of the savefile */
	int number;
	/* the number of the savefile as string with 3 chars */
	char numberstr[4];
	/* the xml save document */
	TiXmlDocument *SaveFile;

	/* list with loaded movejobs */
	cList<sMoveJobLoad*> MoveJobsLoad;
	/* list with loaded subbases */
	cList<sSubBaseLoad*> SubBasesLoad;

	/**
	* writes the saveheader
	*@author alzi alias DoctorDeath
	*/
	void writeHeader( string saveName );
	/**
	* writes game infos such as turn or mode
	*@author alzi alias DoctorDeath
	*/
	void writeGameInfo();
	/**
	* saves the map infos
	*@author alzi alias DoctorDeath
	*/
	void writeMap( cMap *Map );
	/**
	* saves the information for the player to a new node
	*@author alzi alias DoctorDeath
	*/
	void writePlayer( cPlayer *Player, int number );
	/**
	* saves the values of a upgraded unit
	*@author alzi alias DoctorDeath
	*/
	void writeUpgrade ( TiXmlElement *upgradesNode, int upgradenumber, sUnitData *data, sUnitData *originaldata );
	/**
	 * save the research level values of a player
	 *@author pagra
	 */
	void cSavegame::writeResearchLevel( TiXmlElement *researchLevelNode, cResearch& researchLevel );
	/**
	 * save the number of research centers that are working on each area of a player
	 *@author pagra
	 */
	void writeResearchCentersWorkingOnArea (TiXmlElement *researchCentersWorkingOnAreaNode, cPlayer *player);
	/**
	* saves the information of the vehicle
	*@author alzi alias DoctorDeath
	*/
	TiXmlElement *writeUnit ( cVehicle *Vehicle, int *unitnum );
	/**
	* saves the information of the building
	*@author alzi alias DoctorDeath
	*/
	void writeUnit ( cBuilding *Building, int *unitnum );
	/**
	* saves the information of the rubble
	*@author alzi alias DoctorDeath
	*/
	void writeRubble ( cBuilding *Building, int rubblenum );
	/**
	* saves the unit data values which are identic for buildings and vehicles
	*@author alzi alias DoctorDeath
	*/
	void writeUnitValues ( TiXmlElement *unitNode, sUnitData *Data, sUnitData *OwnerData );
	/**
	* saves the standard unit values from the unit xmls
	*@author alzi alias DoctorDeath
	*/
	void writeStandardUnitValues ( sUnitData *Data, int unitnum );

	/**
	* loads the main game information
	*@author alzi alias DoctorDeath
	*/
	void loadGameInfo();
	/**
	* loads the map
	*@author alzi alias DoctorDeath
	*/
	cMap *loadMap();
	/**
	* loads all players from savefile
	*@author alzi alias DoctorDeath
	*/
	cList<cPlayer*> *loadPlayers( cMap *map );
	/**
	* loads a player
	*@author alzi alias DoctorDeath
	*/
	cPlayer *loadPlayer( TiXmlElement *playerNode, cMap *map );
	/**
	* loads the upgrade values of a unit in the players data
	*@author alzi alias DoctorDeath
	*/
	void loadUpgrade ( TiXmlElement *upgradeNode, sUnitData *data );
	/**
	 * loads the research level of a player into the players researchLevel
	 * @author pagra
	 */
	void loadResearchLevel ( TiXmlElement *researchLevelNode, cResearch& researchLevel );
	/**
	 * loads the number of research centers of a player that are working on each area
	 * @author pagra
	 */
	void loadResearchCentersWorkingOnArea( TiXmlElement *researchCentersWorkingOnAreaNode, cPlayer *player );
	/**
	* loads all units
	*@author alzi alias DoctorDeath
	*/
	void loadUnits();
	/**
	* loads a vehicle
	*@author alzi alias DoctorDeath
	*/
	void loadVehicle( TiXmlElement *unitNode, sID &ID );
	/**
	* loads a building
	*@author alzi alias DoctorDeath
	*/
	void loadBuilding( TiXmlElement *unitNode, sID &ID );
	/**
	* loads rubble
	*@author alzi alias DoctorDeath
	*/
	void loadRubble( TiXmlElement *rubbleNode );
	/**
	* loads unit data values that are the same for buildings and vehicles
	*@author alzi alias DoctorDeath
	*/
	void loadUnitValues ( TiXmlElement *unitNode, sUnitData *Data );
	/**
	* loads the standard unit values
	*@author alzi alias DoctorDeath
	*/
	void loadStandardUnitValues ( TiXmlElement *unitNode );
	/**
	* calculates and adds the movejobs after all units has been loaded
	*@author alzi alias DoctorDeath
	*/
	void generateMoveJobs ();

	/**
	* returns the player with the number
	*@author alzi alias DoctorDeath
	*/
	cPlayer *getPlayerFromNumber ( cList<cPlayer*> *PlayerList, int number );
	/**
	* converts the resource data to an string in HEX format
	*@author alzi alias DoctorDeath
	*/
	string convertDataToString ( sResources *resources, int size );
	/**
	* returns the HEX-string of a single byte
	*@author alzi alias DoctorDeath
	*/
	string getHexValue ( unsigned char byte );
	/**
	* converts the resource from HEX-string to byte-data
	*@author alzi alias DoctorDeath
	*/
	void convertStringToData ( string str, int size, sResources *resources );
	/**
	* returns the byte value of a single HEX-string
	*@author alzi alias DoctorDeath
	*/
	unsigned char getByteValue ( string str );
	/**
	* converts the resource-scanmap to an string format
	*@author alzi alias DoctorDeath
	*/
	string convertScanMapToString ( char *data, int size );
	/**
	* converts the resource-scanmap from string format back to the byte data
	*@author alzi alias DoctorDeath
	*/
	void convertStringToScanMap ( string str, char *data );

	/**
	* adds an node without undernodes
	*@author alzi alias DoctorDeath
	*/
	TiXmlElement *addMainElement( TiXmlElement *node, string nodename );
	/**
	* adds an attribute with given value to the node
	*@author alzi alias DoctorDeath
	*/
	void addAttribute ( TiXmlElement *element, string attributename, string value );
	/**
	* adds an node with maximal two attributes and there values
	*@author alzi alias DoctorDeath
	*/
	void addAttributeElement( TiXmlElement *node, string nodename, string attributename, string value, string attributename2 = "", string value2 = "" );
};

#endif // savegameH
