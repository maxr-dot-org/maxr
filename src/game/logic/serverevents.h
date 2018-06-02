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

#ifndef game_logic_servereventsH
#define game_logic_servereventsH

#include <string>
#include "defines.h"
#include "main.h" // for sID
#include "game/logic/upgradecalculator.h" // cResearch::ResearchArea
#include "network.h"
#include "endmoveaction.h"

class cBuilding;
class cHud;
class cMap;
class cNetMessage;
class cResearch;
class cUnit;
class cSavedReport;
class cGameGuiState;
class cSubBase;
class cServer;
class cPosition;
class cPlayer;
struct sID;

enum SERVER_EVENT_TYPES
{
	// Types between FIRST_SERVER_MESSAGE and FIRST_CLIENT_MESSAGE are for the serverserver
	GAME_EV_WANT_MARK_LOG = 209,			// marks a position in the log file
	GAME_EV_WANT_VEHICLE_UPGRADE,	// a client wants to upgrade a vehicle in a building to the newest version
	GAME_EV_WANT_START_CLEAR,		// a bulldowzer wants to start clearing the field under his position
	GAME_EV_WANT_STOP_CLEAR,		// a bulldowzer wants to stop the clearing
	GAME_EV_ABORT_WAITING,			// the player wants to abort waiting for the reconnect of a disconnected player
	GAME_EV_WANT_BUY_UPGRADES,		// a client wants to buy gold upgrades for units
	GAME_EV_WANT_BUILDING_UPGRADE,	// a client wants to upgrade one or more buildings to the newest version
	GAME_EV_WANT_RESEARCH_CHANGE,	// a client wants to change the research assignments of his research centers
	GAME_EV_AUTOMOVE_STATUS,		// a unit has been set to automoving
	GAME_EV_WANT_COM_ACTION,		// an infiltrator wants to steal or disable another unit
	GAME_EV_WANT_CHANGE_UNIT_NAME,	// the player wants to change the name of an unit
	GAME_EV_WANT_KICK_PLAYER,

	// DEDICATED_SERVER
	GAME_EV_WANT_DISCONNECT,		// the player wants to disconnect (but later reconnect to the dedicated server)
	GAME_EV_REQUEST_CASUALTIES_REPORT, // a client wants to have the current casualties data
};

/**
* Sends an event to a player that a unit has to be deleted
*@param unit unit that has to be deleted
*@param receiver The player who should receive this event.
*              Null for all player who can see the unit
*/
void sendDeleteUnit (cServer& server, const cUnit& unit, const cPlayer* receiver);
void sendDeleteUnitMessage (cServer& server, const cUnit& unit, const cPlayer& receiver);
/**
* adds a rubble object to the client
*/
void sendAddRubble (cServer& server, const cBuilding& building, const cPlayer& receiver);

void sendSpecificUnitData (cServer& server, const cVehicle& Vehicle);

/**
* sends the resourcedata of new scaned fields around the unit to a client
*@author alzi alias DoctorDeath
*/
void sendVehicleResources (cServer& server, const cVehicle& vehicle);
void sendResources (cServer& server, const cPlayer& player);

/**
* sends whether and how the unit has to clean the field
*@author alzi alias DoctorDeath
*/
void sendClearAnswer (cServer& server, int answertype, const cVehicle& vehicle, int turns, const cPosition& bigPosition, const cPlayer* player);
/**
* sends that a unit has to stop clearing
*@author alzi alias DoctorDeath
*/
void sendStopClear (cServer& server, const cVehicle& vehicle, const cPosition& bigPosition, const cPlayer& player);
/**
* sends that the player has to set his hole ScanMap to 1
*@author alzi alias DoctorDeath
*/
void sendNoFog (cServer& server, const cPlayer& receiver);
/**
* sends that a player has been defeated
*@author alzi alias DoctorDeath
*/
void sendDefeated (cServer& server, const cPlayer& player, const cPlayer* receiver = nullptr);

/**
* sends that a player has to be deleted
*@author alzi alias DoctorDeath
*/
void sendDeletePlayer (cServer& server, const cPlayer& player, const cPlayer* receiver);

void sendUnitUpgrades (cServer& server, const cDynamicUnitData& Data, const cPlayer& receiver);
void sendCredits (cServer& server, int newCredits, const cPlayer& receiver);
void sendUpgradeBuildings (cServer& server, const std::vector<cBuilding*>& upgradedBuildings, int totalCosts, const cPlayer& receiver);
void sendUpgradeVehicles (cServer& server, const std::vector<cVehicle*>& upgradedVehicles, int totalCosts, unsigned int storingBuildingID, const cPlayer& receiver);
void sendResearchSettings (cServer& server, const std::vector<cBuilding*>& researchCentersToChangeArea, const std::vector<cResearch::ResearchArea>& newAreasForResearchCenters, const cPlayer& receiver);
void sendResearchLevel (cServer& server, const cResearch& researchLevel, const cPlayer& receiver);
void sendFinishedResearchAreas (cServer& server, const std::vector<int>& areas, const cPlayer& receiver);
void sendRefreshResearchCount (cServer& server, const cPlayer& receiver);
void sendSetAutomoving (cServer& server, const cVehicle& vehicle);
/**
* sends the result of a infiltrating action to the client
*@author alzi alias DoctorDeath
*/
void sendCommandoAnswer (cServer& server, bool success, bool steal, const cVehicle& srcUnit, const cPlayer& receiver);


void sendScore (cServer& server, const cPlayer& subject, int turn, const cPlayer* receiver = nullptr);
void sendNumEcos (cServer& server, cPlayer& subject, const cPlayer* receiver = nullptr);
void sendUnitScore (cServer& server, const cBuilding&);

void sendRevealMap (cServer& server, const cPlayer& receiver);

#endif // game_logic_servereventsH
