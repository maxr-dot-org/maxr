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

enum GAME_EVENT_TYPES
{
	GAME_EV_CHAT = 1	// simple text message
};

/**
* Generates a new GAME_EVENT event.
*@author alzi alias DoctorDeath
*@param iTyp Typ of the new event. Will be set to event.user.code
*@param iLenght Lenght of the data for the event.
*@param data Data for the event. Will be set to event.user.data1. Should not be longer than PACKAGE_LENGHT-2
*/
SDL_Event generateEvent ( int iTyp, int iLenght, void *data );

/**
* Generates a event with a chat message and pushes it to the event queue or sends it over TCP/IP if necessary
*@author alzi alias DoctorDeath
*@param sMsg the chat message.
*/
void sendChatMessage ( string sMsg );

#endif // eventmessagesH