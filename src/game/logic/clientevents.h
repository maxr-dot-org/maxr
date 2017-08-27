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

#ifndef game_logic_clienteventsH
#define game_logic_clienteventsH

#include <array>

#include "defines.h"
#include "network.h"
#include "game/logic/serverevents.h"
#include "game/logic/upgradecalculator.h"

class cBuildListItem;
class cUnit;
class cClient;
class cPosition;
class cSavedReport;
class cGameGuiState;
struct sLandingUnit;
class cPlayer;

enum CLIENT_EVENT_TYPES
{
	// Types between FIRST_CLIENT_MESSAGE and FIRST_MENU_MESSAGE are for the client
	GAME_EV_DEL_BUILDING = 102,			// deletes a building
	GAME_EV_DEL_VEHICLE,			// deletes a vehicle
	GAME_EV_UNIT_DATA,				// set new data values for a vehicle
	GAME_EV_SPECIFIC_UNIT_DATA,		// more specific unit values which are only for the owner
	GAME_EV_UNIT_UPGRADE_VALUES,	// message contains upgraded values for a unit
	GAME_EV_RESOURCES,				// a message with new scaned resources for a client
	GAME_EV_BUILD_ANSWER,			// the answer of the server to a build request of a client
	GAME_EV_STOP_BUILD,				// a vehicle has to stop building
	GAME_EV_SUBBASE_VALUES,			// the values of a subbase
	GAME_EV_BUILDLIST,				// the buildlist of a building
	GAME_EV_MINE_PRODUCE_VALUES,	// the produce values of a mine
	GAME_EV_MARK_LOG,				// marks a position in the logfile
	GAME_EV_SUPPLY,					// rearms or repairs a unit
	GAME_EV_ADD_RUBBLE,				// adds a rubble field to the client
	GAME_EV_DETECTION_STATE,		// informs a client whether a vehicle has been detected
	GAME_EV_CLEAR_ANSWER,			// the answer to a clearing request
	GAME_EV_STOP_CLEARING,			// a bulldowzer has to stop clearing
	GAME_EV_NOFOG,					// the player can disable his fog
	GAME_EV_DEFEATED,				// a player has been defeated
	GAME_EV_DEL_PLAYER,				// a client has to delete a player
	GAME_EV_HUD_SETTINGS,			// hud settings for a client
	GAME_EV_STORE_UNIT,				// a unit has to be stored
	GAME_EV_EXIT_UNIT,				// a unit has to be exit
	GAME_EV_CREDITS_CHANGED,		// the credits of a player changed (e.g. because he bought upgrades)
	GAME_EV_UPGRADED_BUILDINGS,		// the buildings in the msg have been upgraded to the current version
	GAME_EV_UPGRADED_VEHICLES,		// the vehicles in the msg have been upgraded to the current version
	GAME_EV_RESEARCH_SETTINGS,		// the research centers were newly assigned to research areas
	GAME_EV_RESEARCH_LEVEL,			// the research level reached by a player
	GAME_EV_FINISHED_RESEARCH_AREAS,
	GAME_EV_REFRESH_RESEARCH_COUNT,	// the client has to refresh the researchCount and the research sums for the areas after a resync
	GAME_EV_SET_AUTOMOVE,			// a unit has to enable automoving
	GAME_EV_COMMANDO_ANSWER,		// information about the result of a commando action
	GAME_EV_SCORE,                  // sends a player's score to a client
	GAME_EV_NUM_ECOS,               // sends a player's ecosphere count to a client
	GAME_EV_UNIT_SCORE,             // sends a unit's score to its owner
	GAME_EV_END_MOVE_ACTION_SERVER,	// the server has added an end move action to a movejob
	GAME_EV_CASUALTIES_REPORT,		// sends the casualties stats to a client
	GAME_EV_REVEAL_MAP,             // a client should reveal the whole map
};

enum CHAT_MESSAGE_TYPES
{
	USER_MESSAGE,
	SERVER_ERROR_MESSAGE,
	SERVER_INFO_MESSAGE,
};


void sendTakenUpgrades (const cClient& client, const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades);

/**
* Generates a event with a chat message and pushes it to the event queue or sends it over TCP/IP if necessary
*@param sMsg the chat message.
*/
void sendChatMessageToServer(const cClient& client, const std::string& msg, const cPlayer& player);

/**
* sends that a vehicle wants to start building
*@author alzi alias DoctorDeath
*@param iVehicleID the ID of the vehicle which wants to start building
*@param iBuildingType type of the building to be build
*@param iBuildSpeed speed of building ( 0->1x, 1->2x or 2->4x )
*@param iBuildOff the offest were to build. Upper left coner on big buildings
*@param bBuildPath true if the vehicle is building in path
*@param iPathOff offset were the path will end
*/
void sendWantBuild (const cClient& client, int iVehicleID, sID buildingTypeID, int iBuildSpeed, const cPosition& buildPosition, bool bBuildPath, const cPosition& pathEndPosition);
/**
* sends that a vehicle wants to leave the building lot
*@author alzi alias DoctorDeath
*@param Vehicle the vehicle which has finished building
*@param EscapeX X coordinate to which he wants do move now
*@param EscapeY Y coordinate to which he wants do move now
*/
void sendWantEndBuilding (const cClient& client, const cVehicle& vehicle, const cPosition& escapePosition);
/**
* sends that the player wants a vehicle to stop building
*@author alzi alias DoctorDeath
*/
void sendWantStopBuilding (const cClient& client, int iVehicleID);
/**
* sends a request for building all vehicles in the buildlist of the building
*@author alzi alias DoctorDeath
*/
void sendWantBuildList (const cClient& client, const cBuilding& building, const std::vector<cBuildListItem>& buildList, bool bRepeat, int buildSpeed);
/**
* sends that the client wants to exit the finished vehicle
*@author alzi alias DoctorDeath
*/
void sendWantExitFinishedVehicle (const cClient& client, const cBuilding& building, const cPosition& position);
/**
* sends that the client wants to change the produce values in the minemanager of a building
*@author alzi alias DoctorDeath
*/
void sendChangeResources (const cClient& client, const cBuilding& building, int iMetalProd, int iOilProd, int iGoldProd);
/**
* sends that the client wants to rearm or repair a unit by an rearm-/repairable vehicle
*@author alzi alias DoctorDeath
*/
void sendWantSupply (const cClient& client, int iDestID, bool bDestVehicle, int iSrcID, bool bSrcVehicle, int iType);
/**
* sends that the client wants to start clearing the field under the unit
*@author alzi alias DoctorDeath
*/
void sendWantStartClear (const cClient& client, const cVehicle& vehicle);
/**
* sends that the client wants to stop clearing the field under the unit
*@author alzi alias DoctorDeath
*/
void sendWantStopClear (const cClient& client, const cVehicle& vehicle);
/**
* sends that the client wants to abort waiting for the reconnect of a disconnected player
*@author alzi alias DoctorDeath
*/
void sendAbortWaiting (const cClient& client);
void sendWantLoad (const cClient& client, int unitid, bool vehicle, int loadedunitid);
void sendWantActivate (const cClient& client, int unitid, bool vehicle, int activatunitid, const cPosition& position);

void sendRequestCasualtiesReport (const cClient& client);

/**
* sends that a unit has been set to automove status
*/
void sendSetAutoStatus (const cClient& client, int vehicleID, bool set);
/**
* sends that the infiltrator wants to infiltrate a unit
*@author alzi alias DoctorDeath
*/
void sendWantComAction (const cClient& client, int srcUnitID, int destUnitID, bool destIsVehicle, bool steal);
void sendUpgradeBuilding (const cClient& client, const cBuilding& building, bool upgradeAll);
void sendWantUpgrade (const cClient& client, int buildingID, int storageSlot, bool upgradeAll);
void sendWantResearchChange (const cClient& client, const std::array<int, cResearch::kNrResearchAreas>& newResearchSettings);

void sendWantChangeUnitName (const cClient& client, const std::string& newName, int unitID);

void sendEndMoveAction (const cClient& client, int vehicleID, int destID, eEndMoveActionType type);

void sentWantKickPlayer (const cClient& client, const cPlayer& player);

#endif // game_logic_clienteventsH
