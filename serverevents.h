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

enum SERVER_EVENT_TYPES
{
	// Types between 0 and FIRST_CLIENT_MESSAGE are for the server
	GAME_EV_LOST_CONNECTION = 0,	// connection on a socket has been lost
	GAME_EV_CHAT_CLIENT,			// a chat message from client to server
};

/**
* Generates a event with a chat message and pushes it to the event queue or sends it over TCP/IP if necessary
*@author alzi alias DoctorDeath
*@param sMsg the chat message.
*/
void sendChatMessage ( string sMsg );
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
void sendAddUnit ( int iPosX, int iPosY, bool bVehicle, int iUnitNum, int iPlayer, bool bInit );
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
void sendDeleteUnit ( int iPosX, int iPosY, int iPlayer, bool bVehicle, int iClient, bool bPlane = false, bool bBase = false, bool bSubBase = false );
/**
* Sends an event to a player that he has to detected an enemy unit and should add it
*@author alzi alias DoctorDeath
*@param Vehicle The vehicle that should be added by the player
*@param Building The building that should be added by the player
*@param iPlayer The player whos unit should be deleted
*/
void sendAddEnemyUnit( cVehicle *Vehicle, int iPlayer );
void sendAddEnemyUnit ( cBuilding *Building, int iPlayer );

#endif // servereventsH
