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
#include "defines.h"
#include "main.h"
#include "network.h"
#include "clientevents.h"
#include "map.h"

class cResearch;
class cHud;
struct sSavedReportMessage;
struct sHudStateContainer;

enum SERVER_EVENT_TYPES
{
	// Types between 0 and FIRST_CLIENT_MESSAGE are for the server
	GAME_EV_LOST_CONNECTION = 0,	// connection on a socket has been lost
	GAME_EV_CHAT_CLIENT,			// a chat message from client to server
	GAME_EV_WANT_TO_END_TURN,		// a client wants to end the turn
	GAME_EV_WANT_START_WORK,		// a client wants to start a building
	GAME_EV_WANT_STOP_WORK,			// a client wants to stop a building
	GAME_EV_MOVE_JOB_CLIENT,		// a message with all waypoints
	GAME_EV_WANT_STOP_MOVE,			// a client wants to stop a moving vehicle
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
	GAME_EV_WANT_CHANGE_SENTRY,		// a client wants to change the sentry status of a unit
	GAME_EV_WANT_MARK_LOG,			// marks a position in the log file
	GAME_EV_WANT_SUPPLY,			// a client wants to rearm or repair a unit
	GAME_EV_WANT_VEHICLE_UPGRADE,	// a client wants to upgrade a vehicle in a building to the newest version
	GAME_EV_WANT_START_CLEAR,		// a bulldowzer wants to start clearing the field under his position
	GAME_EV_WANT_STOP_CLEAR,		// a bulldowzer wants to stop the clearing
	GAME_EV_ABORT_WAITING,			// the player wants to abort waiting for the reconnect of a disconnected player
	GAME_EV_IDENTIFICATION,			// a message with the name of the player who wants to reconnect
	GAME_EV_RECON_SUCESS,			// a client has reconnected sucsessfuly and is ready to receive his game data
	GAME_EV_WANT_LOAD,				// a client wants to load a unit into another
	GAME_EV_WANT_EXIT,				// a client wants to exit a stored unit
	GAME_EV_REQUEST_RESYNC,			// requests the server to resync a client
	GAME_EV_WANT_BUY_UPGRADES,		// a client wants to buy gold upgrades for units
	GAME_EV_WANT_BUILDING_UPGRADE,	// a client wants to upgrade one or more buildings to the newest version
	GAME_EV_WANT_RESEARCH_CHANGE,	// a client wants to change the research assignments of his research centers
	GAME_EV_AUTOMOVE_STATUS,		// a unit has been set to automoving
	GAME_EV_SAVE_HUD_INFO,			// the current hud settings
	GAME_EV_SAVE_REPORT_INFO,		// a saved report
	GAME_EV_FIN_SEND_SAVE_INFO,		// a unit has been set to automoving
	GAME_EV_WANT_COM_ACTION			// an infiltrator wants to steal or disable another unit
};

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
void sendAddUnit ( int iPosX, int iPosY, int iID, bool bVehicle, sID UnitID, int iPlayer, bool bInit, bool bAddToMap = true );
/**
* Sends an event to a player that a unit has to be deleted
*@param vehicle vehicle that has to be deleted
*@param building building that has to be deleted
*@param iClient The client who schould receive this event. -1 for all Clients who can see the unit
*/
void sendDeleteUnit ( cVehicle* vehicle, int iCLient);
void sendDeleteUnit ( cBuilding* building, int iCLient);
/**
* adds a rubble object to the client
*/
void sendAddRubble( cBuilding* building, int iPlayer );
/**
* Sends an event to a player that he has to detected an enemy unit and should add it
*@author alzi alias DoctorDeath
*@param Vehicle The vehicle that should be added by the player
*@param Building The building that should be added by the player
*@param iPlayer The player whos unit should be deleted
*/
void sendAddEnemyUnit( cVehicle *Vehicle, int iPlayer );
void sendAddEnemyUnit ( cBuilding *Building, int iPlayer );
/**
* A client has to make a turnend
*@author alzi alias DoctorDeath
*@param bEndTurn True if the turnnumber has to be increased
*@param bWaitForNextPlayer True if the receiver has to wait for an other player. (Only if he isn't the next player himselves)
*@param iNextPlayerNum The number of the player who has to make his turn next.
*@param sReport The turn start report for this player.
*@param iVoiceNum Number of voice with the player has to play at his turn beginning.
*/
void sendMakeTurnEnd ( bool bEndTurn, bool bWaitForNextPlayer, int iNextPlayerNum, int iPlayer );
/**
* Information for other clients that one player has finished his turn
*@author alzi alias DoctorDeath
*@param iPlayerNum Number of player who has finished his turn.
*@param iTimeDelay Deadline for the rest of the players until the turn will be finished. -1 for no deadline.
*/
void sendTurnFinished ( int iPlayerNum, int iTimeDelay, cPlayer *Player = NULL );
/**
* Sends the data values of this unit to the client
*@author alzi alias DoctorDeath
*@param Vehicle The vehicle from which the data should be taken
*@param Building The building from which the data should be taken
*@param iPlayer Player who should receive this message
*/
void sendUnitData( cVehicle *Vehicle, int iPlayer );
void sendUnitData ( cBuilding *Building, int iPlayer );

void sendSpecificUnitData ( cVehicle *Vehicle );

/**
* sends a text message to one or all client
*@author Eiko
*@param message the message to be send
*@param iType spezifies if this is an error message, info message from the Server or a text message from an other player
*@param iPlayer -1 the playernumber or -1 for broatcast
*/
void sendChatMessageToClient( string message, int iType, int iPlayer = -1, string inserttext = "" );

/**
* sends all nessesary information to all clients to start the building
*@ author Eiko
*/
void sendDoStartWork( cBuilding* building );

/**
* sends all nessesary information to all clients to stop the building
*@ author Eiko
*/
void sendDoStopWork( cBuilding* building );

/**
* sends information about the move to the next field of a client
*@author alzi alias DoctorDeath
*/
void sendNextMove( cVehicle* vehicle, int iType, int iSavedSpeed = -1 );

/**
* sends all waypoints of a movejob to a client. If the movejob is already running,
* the sourceoffset will be changed to the actual position of the vehicle
*@author alzi alias DoctorDeath
*/
void sendMoveJobServer( cServerMoveJob *MoveJob, int iPlayer );
/**
* sends the resourcedata of new scaned fields around the unit to a client
*@author alzi alias DoctorDeath
*/
void sendVehicleResources(  cVehicle *Vehicle, cMap *Map );
void sendResources( cPlayer *Player );
/**
* sends an answer to a client wheter and how he has to build.
*@author alzi alias DoctorDeath
*/
void sendBuildAnswer( bool bOK, cVehicle* vehicle );
/**
* sends that a vehicle has to stop building
*@author alzi alias DoctorDeath
*/
void sendStopBuild ( int iVehicleID, int iNewPos, int iPlayer  );
/**
* send information about a new subbase.
*@author alzi alias DoctorDeath
*@param SubBase the new subbase
*/
void sendNewSubbase ( sSubBase *SubBase, int iPlayer );
/**
* sends the client a message that he has to delete this subbase.
*@author alzi alias DoctorDeath
*@param SubBase the subbase that should be deleted
*/
void sendDeleteSubbase ( sSubBase *SubBase, int iPlayer );
/**
* sends information of all buildings or just one new building that should be added to a subbase.
*@author alzi alias DoctorDeath
*@param Building if only one building should be added to the subbase, this parameter has to be this building.
*@param SubBase the subbase to which the buildings will be added. If Building == NULL then all buildings of the subbase will be send
*/
void sendAddSubbaseBuildings ( cBuilding *Building, sSubBase *SubBase, int iPlayer );
/**
* send the values if a subbase.
*@author alzi alias DoctorDeath
*@param SubBase the subbase which values should be send
*/
void sendSubbaseValues ( sSubBase *SubBase, int iPlayer );
/**
* sends a list with all units which are wanted to be produced by the building
*@author alzi alias DoctorDeath
*@param Building the building which buildlist should be send
*/
void sendBuildList ( cBuilding *Building );
/**
* send the new values of resource production of a building
*@author alzi alias DoctorDeath
*@param Building the building which producevalues should be send
*/
void sendProduceValues ( cBuilding *Building );
/**
* sends the turnstart report of a player
*@author alzi alias DoctorDeath
*@param Player player to who his report should be send
*/
void sendTurnReport ( cPlayer *Player );
/**
* sends that a unit has to be rearmed or repaired
*@author alzi alias DoctorDeath
*@param iDestID the ID of the destination unit
*@param bDestVehicle true if the destination unit is a vehicle
*@param iValue the new ammo or hitpoint value to be set
*@param iType SUPPLY_TYPE_REARM or SUPPLY_TYPE_REPAIR
*@param iPlayerNum number of the player, who will receive the message
*/
void sendSupply ( int iDestID, bool bDestVehicle, int iValue, int iType, int iPlayerNum );
/**
* informs the owner of the vehicle wether the vehicle has been detected by another player.
* this is used by the client for correct drawing of the unit
*/
void sendDetectionState( cVehicle* vehicle );

void sendCheckVehiclePositions(cPlayer* p = NULL);

/**
* sends whether and how the unit has to clean the field
*@author alzi alias DoctorDeath
*/
void sendClearAnswer ( int answertype, cVehicle *Vehicle, int turns, int bigoffset, int iPlayer );
/**
* sends that a unit has to stop clearing
*@author alzi alias DoctorDeath
*/
void sendStopClear ( cVehicle *Vehicle, int bigoffset, int iPlayer );
/**
* sends that the player has to set his hole ScanMap to 1
*@author alzi alias DoctorDeath
*/
void sendNoFog ( int iPlayer );
/**
* sends that a player has been defeated
*@author alzi alias DoctorDeath
*/
void sendDefeated ( cPlayer *Player, int iPlayerNum = -1 );
/**
* sends that a client has to wait untill he will be defrezzed
*@author alzi alias DoctorDeath
*/
void sendFreeze ( int iPlayer = -1 );
/**
* sends that a client has to wait for another player to end his turn
*@author alzi alias DoctorDeath
*/
void sendWaitFor ( int waitForPlayerNr, int iPlayer = -1 );
/**
* sends that the client can abort waiting
*@author alzi alias DoctorDeath
*/
void sendDefreeze ( int iPlayer = -1 );
/**
* sends that a player has to be deleted
*@author alzi alias DoctorDeath
*/
void sendDeletePlayer ( cPlayer *Player, int iPlayer = -1 );
/**
* the server wants to get an identification of the new connected player
*@author alzi alias DoctorDeath
*/
void sendRequestIdentification ( int iSocket );
/**
* the server gives his ok to the reconnection
*@author alzi alias DoctorDeath
*/
void sendOKReconnect ( cPlayer *Player );
void sendTurn ( int turn, cPlayer *Player );
void sendHudSettings ( sHudStateContainer hudStates, cPlayer *Player );
void sendStoreVehicle ( int unitid, bool vehicle, int storedunitid, int player );
void sendActivateVehicle ( int unitid, bool vehicle, int activatunitid, int x, int y, int player );
void sendDeleteEverything ( int player );
void sendUnitUpgrades ( sUnitData *Data, int player );
void sendCredits (int newCredits, int player);
void sendUpgradeBuildings (cList<cBuilding*>& upgradedBuildings, int totalCosts, int player);
void sendUpgradeVehicles (cList<cVehicle*>& upgradedVehicles, int totalCosts, unsigned int storingBuildingID, int player);
void sendResearchSettings(cList<cBuilding*>& researchCentersToChangeArea, cList<int>& newAreasForResearchCenters, int player);
void sendResearchLevel(cResearch* researchLevel, int player);
void sendRefreshResearchCount ( int player );
void sendClansToClients (cList<int>* clans);
void sendSetAutomoving ( cVehicle *Vehicle );
/**
* sends the result of a infiltrating action to the client
*@author alzi alias DoctorDeath
*/
void sendCommandoAnswer ( bool succsess, bool steal, cVehicle *srcUnit, int player );
void sendRequestSaveInfo ( int saveingID );
void sendSavedReport ( sSavedReportMessage &savedReport, int player );

void sendScore(cPlayer *Subject, int turn, cPlayer *Receiver = 0);
void sendNumEcos(cPlayer *Subject, cPlayer *Receiver = 0);
void sendUnitScore(cBuilding *);  
void sendVictoryConditions(int turnLimit, int scoreLimit, cPlayer *receiver=0);


#endif // servereventsH
