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

enum SERVER_EVENT_TYPES
{
	// Types between 0 and FIRST_CLIENT_MESSAGE are for the server
	GAME_EV_LOST_CONNECTION = 0,	// connection on a socket has been lost
	GAME_EV_CHAT_CLIENT,			// a chat message from client to server
	GAME_EV_WANT_TO_END_TURN,		// a client wants to end the turn
	GAME_EV_WANT_START_WORK,		// a client wants to start a building
	GAME_EV_WANT_STOP_WORK,			// a client wants to stop a building
	GAME_EV_MOVE_JOB_CLIENT,		// a message with all waypoints
	GAME_EV_WANT_ATTACK,			// a client wants to attack an other unit
	GAME_EV_MINELAYERSTATUS,		// a minelayer changes his laying status
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
void sendAddUnit ( int iPosX, int iPosY, int iID, bool bVehicle, int iUnitNum, int iPlayer, bool bInit );
/**
* Sends an event to a player that a unit has to be deleted
*@author alzi alias DoctorDeath
*@param iPosX The X position of the unit
*@param iPosY The Y position of the unit
*@param iPlayer The player whos unit should be deleted
*@param bVehicle True if the unit is an vehicle
*@param iClient The client who schould receive this event
*@param bPlane True for planes
*@param bBase True if the building is on the ground
*@param bSubBase True if the building is under an building that is on the ground
*/
void sendDeleteUnit ( int iID, bool bVehicle, int iClient );
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
void sendMakeTurnEnd ( bool bEndTurn, bool bWaitForNextPlayer, int iNextPlayerNum, string sReport, int iVoiceNum, int iPlayer );
/**
* Information for other clients that one player has finished his turn
*@author alzi alias DoctorDeath
*@param iPlayerNum Number of player who has finished his turn.
*@param iTimeDelay Deadline for the rest of the players until the turn will be finished. -1 for no deadline.
*/
void sendTurnFinished ( int iPlayerNum, int iTimeDelay );
/**
* Sends the data values of this unit to the client
*@author alzi alias DoctorDeath
*@param Vehicle The vehicle from which the data should be taken
*@param Building The building from which the data should be taken
*@param iPlayer Player who should receive this message
*/
void sendUnitData( cVehicle *Vehicle, int iPlayer );
void sendUnitData ( cBuilding *Building, cMap *Map, int iPlayer );

/**
* sends a text message to one or all client
*@author Eiko
*@param message the message to be send
*@param iType spezifies if this is an error message, info message from the Server or a text message from an other player
*@param iPlayer -1 the playernumber or -1 for broatcast
*/
void sendChatMessageToClient( string message, int iType, int iPlayer = -1 );

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
void sendNextMove( int iUnitID, int iDestOff, int iType, int iPlayer );

/**
* sends all waypoints of a movejob to a client. If the movejob is already running,
* the sourceoffset will be changed to the actual position of the vehicle
*@author alzi alias DoctorDeath
*/
void sendMoveJobServer( cMJobs *MJob, int iPlayer );
/**
* sends the resourcedata of new scaned fields around the unit to a client
*@author alzi alias DoctorDeath
*/
void sendResources(  cVehicle *Vehicle, cMap *Map );

#endif // servereventsH
