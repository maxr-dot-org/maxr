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
#include "defines.h"
#include "main.h"
#include "network.h"
#include "serverevents.h"

enum CLIENT_EVENT_TYPES
{
	// Types between FIRST_CLIENT_MESSAGE and FIRST_MENU_MESSAGE are for the client
	GAME_EV_ADD_BUILDING = FIRST_CLIENT_MESSAGE,	// adds a building
	GAME_EV_ADD_VEHICLE,			// adds a vehicle
	GAME_EV_DEL_BUILDING,			// deletes a building
	GAME_EV_DEL_VEHICLE,			// deletes a vehicle
	GAME_EV_ADD_ENEM_BUILDING,		// adds a enemy building with current data
	GAME_EV_ADD_ENEM_VEHICLE,		// adds a vehicle with current data
	GAME_EV_CHAT_SERVER,			// a chat message from server to client
	GAME_EV_MAKE_TURNEND,			// a player has to do actions for a turn ending
	GAME_EV_FINISHED_TURN,			// a player has finished his turn
	GAME_EV_UNIT_DATA,				// set new data values for a vehicle
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
	GAME_EV_NEW_SUBBASE,			// a new subbase
	GAME_EV_DELETE_SUBBASE,			// delete a subbase
	GAME_EV_SUBBASE_BUILDINGS,		// a message with all building-ids of a subbase
	GAME_EV_SUBBASE_VALUES,			// the values of a subbase
	GAME_EV_BUILDLIST,				//
	GAME_EV_PRODUCE_VALUES,			//
	GAME_EV_TURN_REPORT				//
};

enum CHAT_MESSAGE_TYPES
{
	USER_MESSAGE,
	SERVER_ERROR_MESSAGE,
	SERVER_INFO_MESSAGE,
};

/**
* Generates a event with a chat message and pushes it to the event queue or sends it over TCP/IP if necessary
*@param sMsg the chat message.
*/
void sendChatMessageToServer ( string sMsg );
/**
* Sends an event that the player wants to end this turn
*@author alzi alias DoctorDeath
*/
void sendWantToEndTurn();

/**
* sends a request to start a building to the Server
*@author Eiko
*/
void sendWantStartWork( cBuilding* building);

/**
* sends a request to stop a building to the Server
*@author Eiko
*/
void sendWantStopWork( cBuilding* building);

/**
* sends all waypoints of a movejob to the server.
*@author alzi alias DoctorDeath
*/
void sendMoveJob( cMJobs *MJob );

/**
*
*@author alzi alias DoctorDeath
*/
void sendWantStopMove ( int iVehicleID );

/**
* sends all nessesary information to identify agressor
* and target of an attack to the server
*@param targetID ID of the target if it is a vehicle, 0 otherwise.
*@param targetOffset the offset, where the player has aimed
*@param agressor ID of the agressor, if it is a vehicle. Offset os the agressor if it is a building
*@param isVehicle true if agressor is a vehicle, false otherwise
*@author Eiko
*/
void sendWantAttack ( int targetID, int targetOffset, int aggressor, bool isVehicle);

/**
* sends whether a minelayer is laying or clearing mines
*@author alzi alias DoctorDeath
*@param Vehicle the vehicle which status has to be send
*/
void sendMineLayerStatus( cVehicle *Vehicle );
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
void sendWantBuild( int iVehicleID, int iBuildingType, int iBuildSpeed, int iBuildOff, bool bBuildPath, int iPathOff );
/**
* sends that a vehicle wants to leave the building lot
*@author alzi alias DoctorDeath
*@param Vehicle the vehicle which has finished building
*@param EscapeX X coordinate to which he wants do move now
*@param EscapeY Y coordinate to which he wants do move now
*/
void sendEndBuilding( cVehicle *Vehicle, int EscapeX, int EscapeY );
void sendWantStopBuilding( int iVehicleID );
/**
* sends that the client wants to tranfer resources
*@author alzi alias DoctorDeath
*@param bSrcVehicle true if the source unit is a vehicle
*@param iSrcID ID of the source unit
*@param bDestVehicle true if the destination unit is a vehicleow
*@param iDestID ID of the destination unit
*@param iTransferValue value of the transfer
*@param iType Type of resources which will be transfered. ( See: NEED_METAL, NEED_OIL or NEED_GOLD )
*/
void sendWantTransfer ( bool bSrcVehicle, int iSrcID, bool bDestVehicle, int iDestID, int iTransferValue, int iType );

void sendWantBuildList ( cBuilding *Building, cList<sBuildStruct*> *BuildList, bool bRepeat );
void sendWantExitFinishedVehicle ( cBuilding *Building, int iX, int iY );
void sendChangeResources ( cBuilding *Building, int iMetalProd, int iOilProd, int iGoldProd );


#endif // clienteventsH
