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
#ifndef eventmessagesH
#define eventmessagesH
#include "defines.h"
#include "main.h"
#include "network.h"

enum GAME_EVENT_TYPES
{
	GAME_EV_LOST_CONNECTION = 0,	// connection on a socket has been lost
	GAME_EV_CHAT,					// simple text message

	GAME_EV_DEL_PLAYER = FIRST_CLIENT_MESSAGE,	// a Player should be deleted
	GAME_EV_ADD_BUILDING,			// adds a building
	GAME_EV_ADD_VEHICLE,			// adds a vehicle
	GAME_EV_DEL_BUILDING,			// deletes a building
};

/**
* Generates a new GAME_EVENT event.
*@author alzi alias DoctorDeath
*@param iTyp Typ of the new event. Will be set to event.user.code
*@param iLenght Lenght of the data for the event.
*@param data Data for the event. Will be set to event.user.data1. Should not be longer than PACKAGE_LENGHT-2
*/
SDL_Event* generateEvent ( int iTyp, int iLenght, void *data );

/**
* Generates a event with a chat message and pushes it to the event queue or sends it over TCP/IP if necessary
*@author alzi alias DoctorDeath
*@param sMsg the chat message.
*/
void sendChatMessage ( string sMsg );

/**
* Generates a event with a chat message and pushes it to the event queue or sends it over TCP/IP if necessary
*@author alzi alias DoctorDeath
*@param sMsg the chat message.
*/
void sendDelPlayer ( int iPlayerNum );

void sendAddUnit ( int iPosX, int iPosY, bool bVehicle, int iUnitNum, int iPlayer, bool bInit );
void sendDelBuilding ( int  );

#endif // eventmessagesH