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

#ifndef game_logic_movejobs2H
#define game_logic_movejobs2H

#include <forward_list>

#include "utility/position.h"
#include "pathcalculator.h"

struct SDL_Rect;

class cMoveJob
{
public:
	cMoveJob(const std::forward_list<sWaypoint>& path, cVehicle& vehicle, cMap& map);
	/**
	* gets the list of position that make up the path. First element is the position,
	* the unit will drive to when starting the next movement step.
	*/
	const std::forward_list<sWaypoint>& getPath() const;
	/**
	* return the moved vehiclee
	*/
	cVehicle* getVehicle() const;
	/**
	* return the amount of saved movement points. These are saved at turn end and added to the
	* vehicles movement points in the next turn. This is done to prevent that the player looses
	* movement points due to rounding issues.
	*/
	unsigned int getSavedSpeed() const;
	/**
	* returns true, if the move job is finished (reached destination or was stopped)
	*/
	bool isFinished() const;
	/**
	* returns true, if the vehicle is currently moving (or rotating).
	*/
	bool isActive() const;

	/**
	* must be called, when the vehicle is destroyed/removed
	*/
	void removeVehicle(cVehicle* vehicle);
	/**
	* Execute the movement for the next game time tick
	*/
	void run(cMap& map);
	/**
	* Stop the movejob. If the job is not active, it is stopped immediately.
	* If the job is active, the state of the job is set to STOPPING and the job will be halted when the unit reaches the next field.
	*/
	void stop();

private:
	enum eMoveJobState {ACTIVE, WAITING, STOPPING, FINISHED};
	
	/**
	* calculates the needed rotation before the next movement 
	*/
	void calcNextDir();
	/**
	* moves the vehicle by 'offset' pixel in direction of 'nextDir'
	*/
	void changeVehicleOffset(int offset) const;
	/**
	* triggers all actions, that need to be done before starting a movement step
	*/
	void startMove(cMap& map);
	/**
	* check, if the unit finished the current movement step
	*/
	bool reachedField() const;
	/**
	* actually ecexute the movement
	*/
	void moveVehicle(cMap& map);
	/**
	* updates the current speed of the vehicle (for accelerating and breaking)
	*/
	void calcSpeed(const cMap &map);
	/**
	* triggers all actions, that need to be done after finishing a movement step
	*/
	void endMove();

	//------------------------------------------------------------------------------
	/** the vehicle to move */
	cVehicle* vehicle;
	/** list of positions. First element is the next field, that the unit will drive to after the current one. */
	std::forward_list<sWaypoint> path;
	eMoveJobState state;

	/** movement points, that are taken to the next turn, to prevent that the player looses movement points due to rounding issues */
	unsigned int savedSpeed;
	/** direction the vehicle must be rotated to, before moving */
	unsigned int nextDir; 
	/** 100 ms timer tick */
	unsigned int timer100ms;
	/** speed of the vehicle in pixel per game time tick */
	unsigned int currentSpeed;

};

#endif // game_logic_movejobsH
