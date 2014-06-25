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
#ifndef clienteventsH
#define clienteventsH

#include <array>

#include "defines.h"
#include "network.h"
#include "serverevents.h"
#include "upgradecalculator.h"

struct sBuildList;
class cUnit;
class cClient;
class cPosition;
class cSavedReport;

enum CLIENT_EVENT_TYPES
{
	// Types between FIRST_CLIENT_MESSAGE and FIRST_MENU_MESSAGE are for the client
	GAME_EV_ADD_BUILDING = FIRST_CLIENT_MESSAGE,	// adds a building
	GAME_EV_ADD_VEHICLE,			// adds a vehicle
	GAME_EV_DEL_BUILDING,			// deletes a building
	GAME_EV_DEL_VEHICLE,			// deletes a vehicle
	GAME_EV_ADD_ENEM_BUILDING,		// adds a enemy building with current data
	GAME_EV_ADD_ENEM_VEHICLE,		// adds a vehicle with current data
	GAME_EV_PLAYER_CLANS,			// data about the clans of the players
	GAME_EV_MAKE_TURNEND,			// a player has to do actions for a turn ending
	GAME_EV_FINISHED_TURN,			// a player has finished his turn
	GAME_EV_UNIT_DATA,				// set new data values for a vehicle
	GAME_EV_SPECIFIC_UNIT_DATA,		// more specific unit values which are only for the owner
	GAME_EV_UNIT_UPGRADE_VALUES,	// message contains upgraded values for a unit
	GAME_EV_DO_START_WORK,			// starts a building
	GAME_EV_DO_STOP_WORK,			// stops a building
	GAME_EV_NEXT_MOVE,				// infos about the next move
	GAME_EV_MOVE_JOB_SERVER,		// a message with all waypoints
	GAME_EV_ATTACKJOB_LOCK_TARGET,	// prepares a mapsquare for beeing attacked
	GAME_EV_ATTACKJOB_FIRE,			// plays the muzzle flash on a client
	GAME_EV_ATTACKJOB_IMPACT,		// makes impact and target unlocking of an attackjob
	GAME_EV_RESOURCES,				// a message with new scaned resources for a client
	GAME_EV_BUILD_ANSWER,			// the answer of the server to a build request of a client
	GAME_EV_STOP_BUILD,				// a vehicle has to stop building
	GAME_EV_SUBBASE_VALUES,			// the values of a subbase
	GAME_EV_BUILDLIST,				// the buildlist of a building
	GAME_EV_MINE_PRODUCE_VALUES,	// the produce values of a mine
	GAME_EV_TURN_REPORT,			// the turnstartreport of a player
	GAME_EV_MARK_LOG,				// marks a position in the logfile
	GAME_EV_SUPPLY,					// rearms or repairs a unit
	GAME_EV_ADD_RUBBLE,				// adds a rubble field to the client
	GAME_EV_DETECTION_STATE,		// informs a client whether a vehicle has been detected
	GAME_EV_CLEAR_ANSWER,			// the answer to a clearing request
	GAME_EV_STOP_CLEARING,			// a bulldowzer has to stop clearing
	GAME_EV_NOFOG,					// the player can disable his fog
	GAME_EV_DEFEATED,				// a player has been defeated
	GAME_EV_FREEZE,					// a client has to be freezed
	GAME_EV_UNFREEZE,				// a client has to be defreezed
	GAME_EV_WAIT_FOR,				// a client has to wait for an other player to finish his turn
	GAME_EV_DEL_PLAYER,				// a client has to delete a player
	GAME_EV_TURN,					// a message with the current turn
	GAME_EV_HUD_SETTINGS,			// hud settings for a client
	GAME_EV_STORE_UNIT,				// a unit has to be stored
	GAME_EV_EXIT_UNIT,				// a unit has to be exit
	GAME_EV_DELETE_EVERYTHING,		// a client has to delete all units to be ready for a resync
	GAME_EV_CREDITS_CHANGED,		// the credits of a player changed (e.g. because he bought upgrades)
	GAME_EV_UPGRADED_BUILDINGS,		// the buildings in the msg have been upgraded to the current version
	GAME_EV_UPGRADED_VEHICLES,		// the vehicles in the msg have been upgraded to the current version
	GAME_EV_RESEARCH_SETTINGS,		// the research centers were newly assigned to research areas
	GAME_EV_RESEARCH_LEVEL,			// the research level reached by a player
	GAME_EV_REFRESH_RESEARCH_COUNT,	// the client has to refresh the researchCount and the research sums for the areas after a resync
	GAME_EV_SET_AUTOMOVE,			// a unit has to enable automoving
	GAME_EV_COMMANDO_ANSWER,		// information about the result of a commando action
	GAME_EV_REQ_SAVE_INFO,			// request the hud state and the saved reports from a client
	GAME_EV_SAVED_REPORT,			// sends saved reports to a client
	GAME_EV_SCORE,                  // sends a player's score to a client
	GAME_EV_NUM_ECOS,               // sends a player's ecosphere count to a client
	GAME_EV_UNIT_SCORE,             // sends a unit's score to its owner
	GAME_EV_GAME_SETTINGS,          // the game settings
	GAME_EV_SELFDESTROY,
	GAME_EV_END_MOVE_ACTION_SERVER,	// the server has added an end move action to a movejob
	GAME_EV_CASUALTIES_REPORT,		// sends the casualties stats to a client
	GAME_EV_REVEAL_MAP,             // a client should reveal the whole map
	NET_GAME_TIME_SERVER,			// notification about current server time
	GAME_EV_SET_GAME_TIME,			// used to resync the gametime of a client
};

enum CHAT_MESSAGE_TYPES
{
	USER_MESSAGE,
	SERVER_ERROR_MESSAGE,
	SERVER_INFO_MESSAGE,
};

void sendClan (const cClient& client);
void sendLandingUnits (const cClient& client, const std::vector<sLandingUnit>& landingList);
void sendUnitUpgrades (const cClient& client);
void sendLandingCoords (const cClient& client, const cPosition& coords);
void sendReadyToStart (const cClient& client);

void sendReconnectionSuccess (const cClient& client);
void sendTakenUpgrades (const cClient& client, const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades);

/**
* Generates a event with a chat message and pushes it to the event queue or sends it over TCP/IP if necessary
*@param sMsg the chat message.
*/
void sendChatMessageToServer (const cClient& client, const cPlayer& player, const std::string& message);
/**
* Sends an event that the player wants to end this turn
*@author alzi alias DoctorDeath
*/
void sendWantToEndTurn (const cClient& client);

/**
* sends a request to start a building to the Server
*@author Eiko
*/
void sendWantStartWork (const cClient& client, const cUnit& building);

/**
* sends a request to stop a building to the Server
*@author Eiko
*/
void sendWantStopWork (const cClient& client, const cUnit& building);

/**
* sends all waypoints of a movejob to the server.
*@author alzi alias DoctorDeath
*/
void sendMoveJob (const cClient& client, sWaypoint* path, int vehicleID);

/**
*
*@author alzi alias DoctorDeath
*/
void sendWantStopMove (const cClient& client, int iVehicleID);

/**
*requests the server to resume the movejob of the vehicle. If 0 is passed, all movejobs of the player will be resumed.
*@author eiko
*/
void sendMoveJobResume (const cClient& client, int unitId);

/**
* sends all necessary information to identify aggressor
* and target of an attack to the server
*@param targetID ID of the target if it is a vehicle, 0 otherwise.
*@param targetPosition the position, where the player has aimed
*@param aggressor ID of the aggressor, if it is a vehicle. Offset os the aggressor if it is a building
*@author Eiko
*/
void sendWantVehicleAttack (const cClient& client, int targetID, const cPosition& targetPosition, int aggressorID);
void sendWantBuildingAttack (const cClient& client, int targetID, const cPosition& targetPosition, const cPosition& aggressorPosition);

/**
* sends whether a minelayer is laying or clearing mines
*@author alzi alias DoctorDeath
*@param Vehicle the vehicle which status has to be send
*/
void sendMineLayerStatus (const cClient& client, const cVehicle& vehicle);
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
* sends that the client wants to transfer resources
*@author alzi alias DoctorDeath
*@param bSrcVehicle true if the source unit is a vehicle
*@param iSrcID ID of the source unit
*@param bDestVehicle true if the destination unit is a vehicle
*@param iDestID ID of the destination unit
*@param iTransferValue value of the transfer
*@param iType Type of resources which will be transferred.
*       (See: NEED_METAL, NEED_OIL or NEED_GOLD)
*/
void sendWantTransfer (const cClient& client, bool bSrcVehicle, int iSrcID, bool bDestVehicle, int iDestID, int iTransferValue, int iType);
/**
* sends a request for building all vehicles in the buildlist of the building
*@author alzi alias DoctorDeath
*/
void sendWantBuildList (const cClient& client, const cBuilding& building, const std::vector<sBuildList>& buildList, bool bRepeat, int buildSpeed);
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
 * sends that the client wants to change the manual fire status of a unit
 *@author pagra
 */
void sendChangeManualFireStatus (const cClient& client, int iUnitID, bool bVehicle);
/**
* sends that the client wants to change the sentry status of a unit
*@author alzi alias DoctorDeath
*/
void sendChangeSentry (const cClient& client, int iUnitID, bool bVehicle);
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
/**
* sends a request to resync the player
*/
void sendRequestResync (const cClient& client, char PlayerNr);

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
void sendWantResearchChange (const cClient& client, const std::array<int, cResearch::kNrResearchAreas>& newResearchSettings, int ownerNr);
void sendSaveHudInfo (const cClient& client, int selectedUnitID, int ownerNr, int savingID);
void sendSaveReportInfo (const cClient& client, const cSavedReport& savedReport, int ownerNr, int savingID);
void sendFinishedSendSaveInfo (const cClient& client, int ownerNr, int savingID);

void sendWantSelfDestroy (const cClient& client, const cBuilding& building);
void sendWantChangeUnitName (const cClient& client, const std::string& newName, int unitID);

void sendEndMoveAction (const cClient& client, int vehicleID, int destID, eEndMoveActionType type);

#endif // clienteventsH
