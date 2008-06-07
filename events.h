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

#ifndef eventsH
#define eventsH
#include "defines.h"
#include "main.h"

#define NETWORK_EVENT	SDL_USEREVENT
#define GAME_EVENT		SDL_USEREVENT+1

Uint32 eventTimerCallback(Uint32 interval, void *param);

/**
* Class for Event handling.
*@author alzi alias DoctorDeath
*/
class cEventHandling
{
public:
	SDL_mutex *EventLock;
	SDL_cond *EventWait;
	SDL_TimerID EventTimer;

	cEventHandling();
	~cEventHandling();

	/**
	* Initialises the mutexes and starts the timer.
	*@author alzi alias DoctorDeath
	*/
	int init();
	/**
	* Destroys the mutexes and removes the timer.
	*@author alzi alias DoctorDeath
	*/
	void quit();

	/**
	* Pushes all events from the input devices to the event queue.
	*@author alzi alias DoctorDeath
	*/
	void pumpEvents();
	/**
	* Checks wether there are any events in the event queue
	* and if there are it puts the first one to the pointer 'event'.
	*@author alzi alias DoctorDeath
	*@param event pointer were the found event should be stored in.
	*@return -1 if there are no events, else 1.
	*/
	int pollEvent( SDL_Event *event );
	/**
	* Waits until an event is placed in the queue and then copies it to the pointer 'event'
	*@author alzi alias DoctorDeath
	*@param event pointer were the found event should be stored in.
	*@return always 1. If no events will be found it will never return.
	*/
	int waitEvent( SDL_Event *event );
	/**
	* Places the event in the event queue.
	* If the event queue is full it waits until there is room in the queue.
	*@author alzi alias DoctorDeath
	*@param event pointer to event which should be pushed to the queue
	*@return always 1. If there never will never be room for the queue it will never return.
	*/
	int pushEvent( SDL_Event *event );

	/**
	* Handles spacial game events and the network events.
	* Should be the only funktions which polls the event queue.
	*@author alzi alias DoctorDeath
	*/
	int HandleEvents();
} EX *EventHandler;

#endif // eventsH
