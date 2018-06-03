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
	GAME_EV_MARK_LOG,				// marks a position in the logfile
	GAME_EV_ADD_RUBBLE,				// adds a rubble field to the client
	GAME_EV_STOP_CLEARING,			// a bulldowzer has to stop clearing
	GAME_EV_NOFOG,					// the player can disable his fog
	GAME_EV_DEFEATED,				// a player has been defeated
	GAME_EV_DEL_PLAYER,				// a client has to delete a player
	GAME_EV_HUD_SETTINGS,			// hud settings for a client
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
* sends that the client wants to stop clearing the field under the unit
*@author alzi alias DoctorDeath
*/
void sendWantStopClear (const cClient& client, const cVehicle& vehicle);
/**
* sends that the client wants to abort waiting for the reconnect of a disconnected player
*@author alzi alias DoctorDeath
*/
void sendAbortWaiting (const cClient& client);

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


void sentWantKickPlayer (const cClient& client, const cPlayer& player);

#endif // game_logic_clienteventsH
