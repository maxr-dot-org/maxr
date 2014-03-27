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
#ifndef servereventsH
#define servereventsH

#include <string>
#include "defines.h"
#include "main.h" // for sID
#include "menus.h"
#include "movejobs.h"
#include "network.h"

class cBuilding;
class cHud;
class cMap;
class cNetMessage;
class cResearch;
class cUnit;
struct sSavedReportMessage;
struct sSubBase;

enum SERVER_EVENT_TYPES
{
	// Types between FIRST_SERVER_MESSAGE and FIRST_CLIENT_MESSAGE are for the server
	GAME_EV_CHAT_CLIENT = FIRST_SERVER_MESSAGE,		// a chat message from client to server
	GAME_EV_WANT_TO_END_TURN,		// a client wants to end the turn
	GAME_EV_WANT_START_WORK,		// a client wants to start a building
	GAME_EV_WANT_STOP_WORK,			// a client wants to stop a building
	GAME_EV_MOVE_JOB_CLIENT,		// a message with all waypoints
	GAME_EV_WANT_STOP_MOVE,			// a client wants to stop a moving vehicle
	GAME_EV_MOVEJOB_RESUME,			// a client wants a paused movejob to be resumed
	GAME_EV_WANT_ATTACK,			// a client wants to attack an other unit
	GAME_EV_MINELAYERSTATUS,		// a minelayer changes his laying status
	GAME_EV_ATTACKJOB_FINISHED,		// the client has finished animating the muzzle flash
	GAME_EV_WANT_BUILD,				// a vehicle wants to start building a building
	GAME_EV_END_BUILDING,			// a vehicle has finished building and will leave the building lot now
	GAME_EV_WANT_STOP_BUILDING,		// a vehicle wants to stop building
	GAME_EV_WANT_TRANSFER,			// information about a resource transfer
	GAME_EV_WANT_BUILDLIST,			// a building wants his buildlist to be verified by the server and start work
	GAME_EV_WANT_EXIT_FIN_VEH,		// a client wants to exit a finished vehicle out of a building
	GAME_EV_CHANGE_RESOURCES,		// a client wants to change his resource production
	GAME_EV_WANT_CHANGE_MANUAL_FIRE,// a client wants to change the manual fire status of a unit
	GAME_EV_WANT_CHANGE_SENTRY,		// a client wants to change the sentry status of a unit
	GAME_EV_WANT_MARK_LOG,			// marks a position in the log file
	GAME_EV_WANT_SUPPLY,			// a client wants to rearm or repair a unit
	GAME_EV_WANT_VEHICLE_UPGRADE,	// a client wants to upgrade a vehicle in a building to the newest version
	GAME_EV_WANT_START_CLEAR,		// a bulldowzer wants to start clearing the field under his position
	GAME_EV_WANT_STOP_CLEAR,		// a bulldowzer wants to stop the clearing
	GAME_EV_ABORT_WAITING,			// the player wants to abort waiting for the reconnect of a disconnected player
	GAME_EV_IDENTIFICATION,			// a message with the name of the player who wants to reconnect
	GAME_EV_RECON_SUCCESS,			// a client has reconnected successfully and is ready to receive his game data
	GAME_EV_WANT_LOAD,				// a client wants to load a unit into another
	GAME_EV_WANT_EXIT,				// a client wants to exit a stored unit
	GAME_EV_REQUEST_RESYNC,			// requests the server to resync a client
	GAME_EV_WANT_BUY_UPGRADES,		// a client wants to buy gold upgrades for units
	GAME_EV_WANT_BUILDING_UPGRADE,	// a client wants to upgrade one or more buildings to the newest version
	GAME_EV_WANT_RESEARCH_CHANGE,	// a client wants to change the research assignments of his research centers
	GAME_EV_AUTOMOVE_STATUS,		// a unit has been set to automoving
	GAME_EV_SAVE_HUD_INFO,			// the current hud settings
	GAME_EV_SAVE_REPORT_INFO,		// a saved report
	GAME_EV_FIN_SEND_SAVE_INFO,		//
	GAME_EV_WANT_COM_ACTION,		// an infiltrator wants to steal or disable another unit
	GAME_EV_WANT_SELFDESTROY,
	GAME_EV_WANT_CHANGE_UNIT_NAME,	// the player wants to change the name of an unit
	GAME_EV_END_MOVE_ACTION,		// specifies an action, which will be executed at the end of a movejob

	GAME_EV_REQ_RECON_IDENT,        // a server of a running game requests an identification of a player who wants to reconnect
	GAME_EV_RECONNECT_ANSWER,       // a server returns an answer for the reconnect

	// Preparation room
	MU_MSG_CLAN,					// a player sends his clan
	MU_MSG_LANDING_VEHICLES,		// the list of purchased vehicles
	MU_MSG_UPGRADES,				// data of upgraded units
	MU_MSG_LANDING_COORDS,			// the selected landing coords of a client
	MU_MSG_READY_TO_START,			// the client is ready and wants the server to start the game

	// DEDICATED_SERVER
	GAME_EV_WANT_DISCONNECT,		// the player wants to disconnect (but later reconnect to the dedicated server)
	GAME_EV_REQUEST_CASUALTIES_REPORT, // a client wants to have the current casualties data
	NET_GAME_TIME_CLIENT,			//reports the current gametime of the client to server
};


void sendGo (cServer& server);

/*
* Send the landing state.
*/
//void sendReselectLanding (cServer& server, eLandingState state, int iPlayer);

/*
* All player has landed, game can start.
*/
void sendAllLanded (cServer& server);

/**
* Sends an event to a player that a new unit has to be added
*@author alzi alias DoctorDeath
*@param iPosX The X position of the unit
*@param iPosY The Y position of the unit
*@param bVehicle True if the unit is an vehicle
*@param iUnitNum The typ number of the unit
*@param iPlayer The player who should receive this event and get the new unit
*@param bInit True if this is called by game initialisation
*/
void sendAddUnit (cServer& server, int iPosX, int iPosY, int iID, bool bVehicle, sID UnitID, int iPlayer, bool bInit, bool bAddToMap = true);
/**
* Sends an event to a player that a unit has to be deleted
*@param unit unit that has to be deleted
*@param iClient The client who should receive this event.
*               -1 for all Clients who can see the unit
*/
void sendDeleteUnit (cServer& server, const cUnit& unit, int iClient);
/// playerNr must be the number of a valid player
void sendDeleteUnitMessage (cServer& server, const cUnit& unit, int playerNr);
/**
* adds a rubble object to the client
*/
void sendAddRubble (cServer& server, const cBuilding& building, int iPlayer);
/**
* Sends an event to a player that he has detected
* an enemy unit and should add it
*@author alzi alias DoctorDeath
*@param unit The unit that should be added by the player
*@param iCLient The client who should receive this event
*/
void sendAddEnemyUnit (cServer& server, const cUnit& unit, int iCLient);
/**
* A client has to make a turnend
*@author alzi alias DoctorDeath
*@param bEndTurn True if the turnnumber has to be increased
*@param bWaitForNextPlayer True if the receiver has to wait for an other player.
*       (Only if he isn't the next player himselves)
*@param iNextPlayerNum The number of the player who has to make his turn next.
*@param sReport The turn start report for this player.
*@param iVoiceNum Number of voice with the player has to play
*       at his turn beginning.
*/
void sendMakeTurnEnd (cServer& server, bool bEndTurn, bool bWaitForNextPlayer, int iNextPlayerNum, int iPlayer);
/**
* Information for other clients that one player has finished his turn
*@author alzi alias DoctorDeath
*@param iPlayerNum Number of player who has finished his turn.
*@param iTimeDelay Deadline for the rest of the players
*       until the turn will be finished. -1 for no deadline.
*/
void sendTurnFinished (cServer& server, int iPlayerNum, int iTimeDelay, const cPlayer* Player = NULL);
/**
* Sends the data values of this unit to the client
*@author alzi alias DoctorDeath
*@param unit The unit from which the data should be taken
*@param iPlayer Player who should receive this message
*/
void sendUnitData (cServer& server, const cUnit& unit, int iPlayer);

void sendSpecificUnitData (cServer& server, const cVehicle& Vehicle);

/**
* sends a text message to one or all client
*@author Eiko
*@param message the message to be send
*@param iType spezifies if this is an error message,
*       info message from the Server or a text message from an other player
*@param iPlayer -1 the playernumber or -1 for broatcast
*/
void sendChatMessageToClient (cServer& server, const std::string& message, int iType, int iPlayer = -1, const std::string& inserttext = "");

/**
* sends all necessary information to all clients to start the building
*@ author Eiko
*/
void sendDoStartWork (cServer& server, const cBuilding& building);

/**
* sends all necessary information to all clients to stop the building
*@ author Eiko
*/
void sendDoStopWork (cServer& server, const cBuilding& building);

/**
* sends information about the move to the next field of a client
*@author alzi alias DoctorDeath
*/
void sendNextMove (cServer& server, const cVehicle& vehicle, int iType, int iSavedSpeed = -1);

/**
* sends all waypoints of a movejob to a client.
* If the movejob is already running,
* the sourceoffset will be changed to the actual position of the vehicle
*@author alzi alias DoctorDeath
*/
void sendMoveJobServer (cServer& server, const cServerMoveJob& MoveJob, int iPlayer);
/**
* sends the resourcedata of new scaned fields around the unit to a client
*@author alzi alias DoctorDeath
*/
void sendVehicleResources (cServer& server, const cVehicle& vehicle);
void sendResources (cServer& server, const cPlayer& player);
/**
* sends an answer to a client wheter and how he has to build.
*@author alzi alias DoctorDeath
*/
void sendBuildAnswer (cServer& server, bool bOK, const cVehicle& vehicle);
/**
* sends that a vehicle has to stop building
*@author alzi alias DoctorDeath
*/
void sendStopBuild (cServer& server, int iVehicleID, int iNewPos, int iPlayer);
/**
* send the values if a subbase.
*@author alzi alias DoctorDeath
*@param subbase the subbase which values should be send
*/
void sendSubbaseValues (cServer& server, const sSubBase& subbase, int iPlayer);
/**
* sends a list with all units which are wanted to be produced by the building
*@author alzi alias DoctorDeath
*@param building the building which buildlist should be send
*/
void sendBuildList (cServer& server, const cBuilding& building);
/**
* send the new values of resource production of a building
*@author alzi alias DoctorDeath
*@param building the building which producevalues should be send
*/
void sendProduceValues (cServer& server, const cBuilding& building);
/**
* sends the turnstart report of a player
*@author alzi alias DoctorDeath
*@param Player player to who his report should be send
*/
void sendTurnReport (cServer& server, cPlayer& player);
/**
* sends that a unit has to be rearmed or repaired
*@author alzi alias DoctorDeath
*@param iDestID the ID of the destination unit
*@param bDestVehicle true if the destination unit is a vehicle
*@param iValue the new ammo or hitpoint value to be set
*@param iType SUPPLY_TYPE_REARM or SUPPLY_TYPE_REPAIR
*@param iPlayerNum number of the player, who will receive the message
*/
void sendSupply (cServer& server, int iDestID, bool bDestVehicle, int iValue, int iType, int iPlayerNum);
/**
* informs the owner of the vehicle whether the vehicle has been detected
* by another player.
* this is used by the client for correct drawing of the unit
*/
void sendDetectionState (cServer& server, const cVehicle& vehicle);

/**
* sends whether and how the unit has to clean the field
*@author alzi alias DoctorDeath
*/
void sendClearAnswer (cServer& server, int answertype, const cVehicle& vehicle, int turns, int bigoffset, int iPlayer);
/**
* sends that a unit has to stop clearing
*@author alzi alias DoctorDeath
*/
void sendStopClear (cServer& server, const cVehicle& vehicle, int bigoffset, int iPlayer);
/**
* sends that the player has to set his hole ScanMap to 1
*@author alzi alias DoctorDeath
*/
void sendNoFog (cServer& server, int iPlayer);
/**
* sends that a player has been defeated
*@author alzi alias DoctorDeath
*/
void sendDefeated (cServer& server, const cPlayer& player, int iPlayerNum = -1);
/**
* sends that a client has to wait until he will be defrezzed
*@param waitForPlayer tells the client, for which other player he is waiting
*/
void sendFreeze (cServer& server, eFreezeMode mode, int waitForPlayer);
/**
* sends that the client can abort waiting
*/
void sendUnfreeze (cServer& server, eFreezeMode mode);
/**
* sends that a client has to wait for another player to end his turn
*@author alzi alias DoctorDeath
*/
void sendWaitFor (cServer& server, int waitForPlayerNr, int iPlayer = -1);
/**
* sends that a player has to be deleted
*@author alzi alias DoctorDeath
*/
void sendDeletePlayer (cServer& server, const cPlayer& player, int iPlayer = -1);
/**
* the server wants to get an identification of the new connected player
*@author alzi alias DoctorDeath
*/
void sendRequestIdentification (cTCP& network, int iSocket);
/**
* the server gives its ok to the reconnection
*@author alzi alias DoctorDeath
*/
void sendReconnectAnswer (cServer& server, int socketNumber);
void sendReconnectAnswer (cServer& server, int socketNumber, const cPlayer& player);

void sendTurn (cServer& server, int turn, unsigned int gameTime, const cPlayer& player);
void sendHudSettings (cServer& server, const cPlayer& player);
void sendStoreVehicle (cServer& server, int unitid, bool vehicle, int storedunitid, int player);
void sendActivateVehicle (cServer& server, int unitid, bool vehicle, int activatunitid, int x, int y, int player);
void sendDeleteEverything (cServer& server, int player);
void sendUnitUpgrades (cServer& server, const sUnitData& Data, int player);
void sendCredits (cServer& server, int newCredits, int player);
void sendUpgradeBuildings (cServer& server, const std::vector<cBuilding*>& upgradedBuildings, int totalCosts, int player);
void sendUpgradeVehicles (cServer& server, const std::vector<cVehicle*>& upgradedVehicles, int totalCosts, unsigned int storingBuildingID, int player);
void sendResearchSettings (cServer& server, const std::vector<cBuilding*>& researchCentersToChangeArea, const std::vector<int>& newAreasForResearchCenters, int player);
void sendResearchLevel (cServer& server, const cResearch& researchLevel, int player);
void sendRefreshResearchCount (cServer& server, int player);
void sendClansToClients (cServer& server, const std::vector<cPlayer*>& playerList);
void sendGameTime (cServer& server, const cPlayer& player, int gameTime);
void sendSetAutomoving (cServer& server, const cVehicle& vehicle);
/**
* sends the result of a infiltrating action to the client
*@author alzi alias DoctorDeath
*/
void sendCommandoAnswer (cServer& server, bool success, bool steal, const cVehicle& srcUnit, int player);
void sendRequestSaveInfo (cServer& server, const int saveingID);
void sendSavedReport (cServer& server, const sSavedReportMessage& savedReport, int player);

void sendCasualtiesReport (cServer& server, int player);

void sendScore (cServer& server, const cPlayer& subject, int turn, const cPlayer* Receiver = 0);
void sendNumEcos (cServer& server, cPlayer& subject, const cPlayer* Receiver = 0);
void sendUnitScore (cServer& server, const cBuilding&);
void sendGameSettings (cServer& server, const cPlayer& receiver);

void sendSelfDestroy (cServer& server, const cBuilding& building);

void sendEndMoveActionToClient (cServer& server, const cVehicle& vehicle, int destID, eEndMoveActionType type);

void sendRevealMap (cServer& server, int player);

#endif // servereventsH
