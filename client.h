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
#ifndef clientH
#define clientH
#include "defines.h"
#include "main.h"

/**
* Client class which handles the in and output for a player
*@author alzi alias DoctorDeath
*/
class cClient
{
	/** true if the client should exit */
	bool bExit;

public:
	/**
	* initialises the client class
	*@author alzi alias DoctorDeath
	*/
	void init();
	/**
	* runs the client.
	*@author alzi alias DoctorDeath
	*/
	void run();

	/**
	* processes everything that is need for this event.
	*@author alzi alias DoctorDeath
	*@param event The SDL_Event to be handled.
	*@return 0 for success
	*/
	int HandleEvent( SDL_Event *event );
} EX *Client;

#endif clientH
