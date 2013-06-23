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
#include "tinyxml.h"
#include "tinyxml2.h"
#include <vector>


class cResearch;
class cMap;
class cPlayer;
class cVehicle;
class cBuilding;
class cServer;
class cTCP;
struct sResources;
struct sUnitData;
struct sID;
struct sHudStateContainer;
struct sSavedReportMessage;

#define SAVE_FORMAT_VERSION		((std::string)"0.3")

//--------------------------------------------------------------------------
struct sMoveJobLoad
{
	cVehicle* vehicle;
	int destX, destY;
};

//--------------------------------------------------------------------------
struct sSubBaseLoad
{
	int buildingID;
	int metalProd;
	int oilProd;
	int goldProd;
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
	cSavegame (int number);

	/* saves the current gamestate to a file */
	int save (const cServer& server, const std::string& saveName);
	/* loads a savegame */
	int load (cServer** pServer, cTCP* network);

	/* loads the header of a savefile and returns some values to the pointers */
	void loadHeader (std::string* name, std::string* type, std::string* time);
	std::string getMapName() const;
	std::string getPlayerNames() const;

	/**
	* ---
	*@author alzi alias DoctorDeath
	*/
	void writeAdditionalInfo (sHudStateContainer hudState, std::vector<sSavedReportMessage>& list, const cPlayer* player);

	//--------------------------------------------------------------------------
private:
	/* the number of the savefile */
	int number;
	/* filename of the last loaded savefile */
	std::string loadedXMLFileName;
	/* the number of the savefile as string with 3 chars */
	char numberstr[4];
	/* the xml save document */
	tinyxml2::XMLDocument SaveFile;
	/* the version of a loaded savegame */
	std::string version;

	/* list with loaded movejobs */
	std::vector<sMoveJobLoad*> MoveJobsLoad;
	/* list with loaded subbases */
	std::vector<sSubBaseLoad*> SubBasesLoad;

	/**
	* writes the saveheader
	*@author alzi alias DoctorDeath
	*/
	void writeHeader (const cServer& server, const std::string& saveName);
	/**
	* writes game infos such as turn or mode
	*@author alzi alias DoctorDeath
	*/
	void writeGameInfo (const cServer& server);
	/**
	* saves the map infos
	*@author alzi alias DoctorDeath
	*/
	void writeMap (const cMap* Map);
	/**
	* saves the information for the player to a new node
	*@author alzi alias DoctorDeath
	*/
	void writePlayer (const cPlayer* Player, int number);
	/**
	* saves the values of a upgraded unit
	*@author alzi alias DoctorDeath
	*/
	void writeUpgrade (tinyxml2::XMLElement* upgradesNode, int upgradenumber, const sUnitData* data, const sUnitData* originaldata);
	/**
	 * save the research level values of a player
	 *@author pagra
	 */
	void writeResearchLevel (tinyxml2::XMLElement* researchLevelNode, const cResearch& researchLevel);
	/**
	 * save the number of research centers that are working on each area of a player
	 *@author pagra
	 */
	void writeResearchCentersWorkingOnArea (tinyxml2::XMLElement* researchCentersWorkingOnAreaNode, const cPlayer* player);
	/**
	 * save the casualties of all players
	 *@author pagra
	 */
	void writeCasualties (const cServer& server);
	/**
	* saves the information of the vehicle
	*@author alzi alias DoctorDeath
	*/
	tinyxml2::XMLElement* writeUnit (const cServer& server, const cVehicle& Vehicle, int* unitnum);
	/**
	* saves the information of the building
	*@author alzi alias DoctorDeath
	*/
	void writeUnit (const cServer& server, const cBuilding& building, int* unitnum);
	/**
	* saves the information of the rubble
	*@author alzi alias DoctorDeath
	*/
	void writeRubble (const cServer& server, const cBuilding& building, int rubblenum);
	/**
	* saves the unit data values which are identic for buildings and vehicles
	*@author alzi alias DoctorDeath
	*/
	void writeUnitValues (tinyxml2::XMLElement* unitNode, const sUnitData* Data, const sUnitData* OwnerData);
	/**
	* saves the standard unit values from the unit xmls
	*@author alzi alias DoctorDeath
	*/
	void writeStandardUnitValues (const sUnitData* Data, int unitnum);

	/**
	* loads the main game information
	*@author alzi alias DoctorDeath
	*/
	void loadGameInfo (cServer& server);
	/**
	* loads the map
	*@author alzi alias DoctorDeath
	*/
	cMap* loadMap();
	/**
	* loads all players from savefile
	*@author alzi alias DoctorDeath
	*/
	std::vector<cPlayer*>* loadPlayers (cMap* map);
	/**
	* loads a player
	*@author alzi alias DoctorDeath
	*/
	cPlayer* loadPlayer (tinyxml2::XMLElement* playerNode, cMap* map);
	/**
	* loads the upgrade values of a unit in the players data
	*@author alzi alias DoctorDeath
	*/
	void loadUpgrade (tinyxml2::XMLElement* upgradeNode, sUnitData* data);
	/**
	 * loads the research level of a player into the players researchLevel
	 * @author pagra
	 */
	void loadResearchLevel (tinyxml2::XMLElement* researchLevelNode, cResearch& researchLevel);
	/**
	 * loads the number of research centers of a player that are working on each area
	 * @author pagra
	 */
	void loadResearchCentersWorkingOnArea (tinyxml2::XMLElement* researchCentersWorkingOnAreaNode, cPlayer* player);
	/**
	 * loads the casualties of all players
	 *@author pagra
	 */
	void loadCasualties (cServer& server);
	/**
	* loads all units
	*@author alzi alias DoctorDeath
	*/
	void loadUnits (cServer& server);
	/**
	* loads a vehicle
	*@author alzi alias DoctorDeath
	*/
	void loadVehicle (cServer& server, tinyxml2::XMLElement* unitNode, sID& ID);
	/**
	* loads a building
	*@author alzi alias DoctorDeath
	*/
	void loadBuilding (cServer& server, tinyxml2::XMLElement* unitNode, sID& ID);
	/**
	* loads rubble
	*@author alzi alias DoctorDeath
	*/
	void loadRubble (cServer& server, tinyxml2::XMLElement* rubbleNode);
	/**
	* loads unit data values that are the same for buildings and vehicles
	*@author alzi alias DoctorDeath
	*/
	void loadUnitValues (tinyxml2::XMLElement* unitNode, sUnitData* Data);
	/**
	* loads the standard unit values
	*@author alzi alias DoctorDeath
	*/
	void loadStandardUnitValues (tinyxml2::XMLElement* unitNode);
	/**
	* recalculates the subbase values after loading all units
	*@author eiko
	*/
	void recalcSubbases(cServer& server);
	/**
	* calculates and adds the movejobs after all units has been loaded
	*@author alzi alias DoctorDeath
	*/
	void generateMoveJobs (cServer& server);

	/**
	* returns the player with the number
	*@author alzi alias DoctorDeath
	*/
	cPlayer* getPlayerFromNumber (const std::vector<cPlayer*>& PlayerList, int number);

	/**
	* converts the resource-scanmap to an string format
	*@author alzi alias DoctorDeath
	*/
	std::string convertScanMapToString (const cPlayer& player) const;
	/**
	* converts the resource-scanmap from string format back to the byte data
	*@author alzi alias DoctorDeath
	*/
	void convertStringToScanMap (const std::string& str, cPlayer& player);

	/**
	* adds an node without undernodes
	*@author alzi alias DoctorDeath
	*/
	tinyxml2::XMLElement* addMainElement (tinyxml2::XMLElement* node, const std::string& nodename);
	/**
	* adds an attribute with given value to the node
	*@author alzi alias DoctorDeath
	*/
	void addAttribute (tinyxml2::XMLElement* element, const std::string& attributename, const std::string& value);
	/**
	* adds an node with maximal two attributes and there values
	*@author alzi alias DoctorDeath
	*/
	void addAttributeElement (tinyxml2::XMLElement* node, const std::string& nodename, const std::string& attributename, const std::string& value, const std::string& attributename2 = "", const std::string& value2 = "");
};

/**
* Splits a string s by "word" according to one of separators seps.
*/
void Split(const std::string& s, const char* seps, std::vector<std::string>& words);

#endif // savegameH
