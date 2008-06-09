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

#ifndef EVENTS_H
#define EVENTS_H

#include "defines.h"
#include "main.h"


#define NETWORK_EVENT	SDL_USEREVENT
#define GAME_EVENT		(SDL_USEREVENT + 1)

/**
 * Class for Event handling.
 *@author alzi alias DoctorDeath
 */
class cEventHandling
{
public:
	/**
	 * Pushes all events from the input devices to the event queue.
	 * @author alzi alias DoctorDeath
	 */
	void pumpEvents();

	/**
	 * Checks wether there are any events in the event queue
	 * and if there are it puts the first one to the pointer 'event'.
	 * @author alzi alias DoctorDeath
	 * @param event pointer were the found event should be stored in.
	 * @return see SDL_PollEvent
	 */
	int pollEvent(SDL_Event*);

	/**
	 * Waits until an event is placed in the queue and then copies it to the pointer 'event'
	 * @author alzi alias DoctorDeath
	 * @param event pointer were the found event should be stored in.
	 */
	void waitEvent(SDL_Event*);

	/**
	 * Places the event in the event queue.
	 * If the event queue is full it waits until there is room in the queue.
	 * @author alzi alias DoctorDeath
	 * @param event pointer to event which should be pushed to the queue
	 */
	void pushEvent(SDL_Event*);

	/**
	 * Handles spacial game events and the network events.
	 * Should be the only functions which polls the event queue.
	 * @author alzi alias DoctorDeath
	 */
	int HandleEvents();
} EX *EventHandler;

#endif
