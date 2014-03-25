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
#ifndef menusH
#define menusH

#include <string>
#include "main.h"

class cNetMessage;

struct sLandingUnit
{
	sID unitID;
	int cargo;
};

enum eLandingState
{
	LANDING_STATE_UNKNOWN,      //initial state
	LANDING_POSITION_OK,        //there are no other players near the position
	LANDING_POSITION_WARNING,   //there are players within the warning distance
	LANDING_POSITION_TOO_CLOSE, //the position is too close to another player
	LANDING_POSITION_CONFIRMED  //warnings about nearby players will be ignored,
	//because the player has confirmed his position
};

std::string ToString (eLandingState state);

struct sClientLandData
{
	int iLandX, iLandY;
	int iLastLandX, iLastLandY;
	eLandingState landingState;
	bool receivedOK;

	sClientLandData() :
		iLandX (-1), iLandY (-1), iLastLandX (-1), iLastLandY (-1),
		landingState (LANDING_STATE_UNKNOWN), receivedOK (false)
	{}

	/** checks whether the landing positions are okay
	 *@author alzi
	 */
	static eLandingState checkLandingState (std::vector<sClientLandData>& landData, unsigned int playerNr);
};

#endif //menusH
