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

#ifndef game_logic_endmoveaction_h
#define game_logic_endmoveaction_h

class cVehicle;
class cServer;

enum eEndMoveActionType
{
	EMAT_LOAD,
	EMAT_GET_IN,
	EMAT_ATTACK
};

class cEndMoveAction
{
public:
	cVehicle* vehicle_;
	eEndMoveActionType type_;
	int destID_;		//we store the ID and not a pointer to vehicle/building,
	//so we don't have to invalidate the pointer, when the dest unit gets destroyed

private:
	void executeLoadAction (cServer& server);
	void executeGetInAction (cServer& server);
	void executeAttackAction (cServer& server);

public:
	cEndMoveAction (cVehicle* vehicle, int destID, eEndMoveActionType type);

	void execute (cServer& server);
};


#endif // !game_logic_endmoveaction_h
