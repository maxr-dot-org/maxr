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
#ifndef serverH
#define serverH
#include "defines.h"
#include "main.h"

/**
* Callback funktion for the serverthread
*@author alzi alias DoctorDeath
*/
int CallbackRunServerThread( void *arg );

/**
* Server class which takes all calculations for the game and has the data of all players
*@author alzi alias DoctorDeath
*/
class cServer
{
	/** a list with all events for the server */
	cList<SDL_Event*> *EventQueue;
	/** the thread the server runs in */
	SDL_Thread *ServerThread;
	/** mutex for the eventqueue */
	SDL_mutex *QueueMutex;
	/** true if the server should exit and end his thread */
	bool bExit;

	/**
	* gets the next event of the eventqueue and stores it to 'event'. If the queue is empty 'event' will be set to NULL.
	*@author alzi alias DoctorDeath
	*@return 1 for success, 0 if the eventqueue is empty
	*/
	int pollEvent( SDL_Event *event );
	/**
	* processes everything that is need for this event.
	*@author alzi alias DoctorDeath
	*param event The SDL_Event to be handled.
	*@return 0 for success
	*/
	int HandleEvent( SDL_Event *event );

public:
	/**
	* initialises the server class
	*@author alzi alias DoctorDeath
	*/
	void init();
	/**
	* pushes an event to the eventqueue of the server. This is threadsafe.
	*@author alzi alias DoctorDeath
	*@param event The SDL_Event to be pushed.
	*@return 0 for success
	*/
	int pushEvent( SDL_Event *event );

	/**
	* runs the server. Should only be called by the ServerThread!
	*@author alzi alias DoctorDeath
	*/
	void run();
} EX *Server;

#endif serverH
