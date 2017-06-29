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


#include <SDL_rect.h>
#include <SDL_surface.h>

#include "movejob.h"
#include "pathcalculator.h"
#include "main.h"
#include "video.h"
#include "game/data/units/vehicle.h"
#include "gametimer.h"
#include "game/data/player/player.h"
#include "game/data/map/map.h"
#include "utility/string/toString.h"

cMoveJob::cMoveJob(const std::forward_list<sWaypoint>& path, cVehicle& vehicle, cMap& map) :
	path(path),
	vehicle(&vehicle),
	savedSpeed(0),
	state(ACTIVE),
	nextDir(0),
	timer100ms(1),
	currentSpeed(0)
{
	startMove(map);
}

//------------------------------------------------------------------------------
const std::forward_list<sWaypoint>& cMoveJob::getPath() const
{
	return path;
}

//------------------------------------------------------------------------------
unsigned int cMoveJob::getSavedSpeed() const
{
	return savedSpeed;
}

//------------------------------------------------------------------------------
void cMoveJob::removeVehicle(cVehicle* vehicle)
{
	vehicle = nullptr;
}

//------------------------------------------------------------------------------
void cMoveJob::run(cMap& map)
{
	if (!vehicle || vehicle->getMoveJob() != this)
	{
		state = FINISHED;
	}
	if (state == FINISHED || state == WAITING)
	{
		return;
	}

	if (vehicle->isBeeingAttacked())
	{
		// suspend movejobs of attacked vehicles
		return;
	}
	
	timer100ms++;
	if (timer100ms == 10) timer100ms = 0;

	if (nextDir != vehicle->dir)
	{
		if (timer100ms == 0)
		{
			vehicle->rotateTo (nextDir);
		}
	}
	else if (!reachedField())
	{
		moveVehicle(map);
	}
	else
	{
		startMove(map);
	}
}

//------------------------------------------------------------------------------
bool cMoveJob::isFinished() const
{
	return state == FINISHED;
}

//------------------------------------------------------------------------------
bool cMoveJob::isActive() const
{
	return state == ACTIVE || state == STOPPING;
}

//------------------------------------------------------------------------------
void cMoveJob::stop()
{
	if (isActive())
	{
		state = STOPPING;
	}
	else
	{
		state = FINISHED;
		vehicle->setMoving(false);
	}
}

//------------------------------------------------------------------------------
cVehicle* cMoveJob::getVehicle() const
{
	return vehicle;
}

//------------------------------------------------------------------------------
void cMoveJob::calcNextDir()
{
	if (path.empty()) return;

	const cPosition diff = path.begin()->position - vehicle->getPosition();

	//                       N, NE, E, SE, S, SW,  W, NW
	const int offsetX[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	const int offsetY[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };

	for (int i = 0; i != 8; ++i)
	{
		if (diff.x() == offsetX[i] && diff.y() == offsetY[i])
		{
			nextDir = i;
		}
	}
}

//------------------------------------------------------------------------------
void cMoveJob::changeVehicleOffset(int offset) const
{
	//                       N, NE, E, SE, S, SW,  W, NW
	const int offsetX[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	const int offsetY[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };

	auto newOffset = vehicle->getMovementOffset();
	newOffset.x() += offsetX[nextDir] * offset;
	newOffset.y() += offsetY[nextDir] * offset;

	vehicle->setMovementOffset(newOffset);
}


//------------------------------------------------------------------------------
void cMoveJob::startMove(cMap& map)
{
	if (path.empty() || state == STOPPING)
	{
		state = FINISHED;
		vehicle->setMoving(false);
		return;
	}

	//TODO: don't use precalculated costs
	if (vehicle->data.getSpeed() < path.front().costs)
	{
		savedSpeed += vehicle->data.getSpeed();
		vehicle->data.setSpeed(0);
		vehicle->setMoving(false);
		state = WAITING;
		return;
	}

	if (!map.possiblePlace(*vehicle, path.front().position))
	{
		//TODO: do nothing, when field is temporary blocked
		//TODO: sidestep stealth unit
		//TODO: recalc path & continue
		state = FINISHED;
		vehicle->setMoving(false);
		return;
	}

	// next step can be executed.
	// start the move

	vehicle->setMoving(true);
	calcNextDir();
	
	vehicle->data.setSpeed(vehicle->data.getSpeed() + savedSpeed);
	savedSpeed = 0;
	vehicle->DecSpeed(path.front().costs);

	map.moveVehicle(*vehicle, path.front().position);
	path.pop_front();
	vehicle->setMovementOffset(cPosition(0, 0));
	changeVehicleOffset(-64);

	vehicle->getOwner()->doScan();

	//TODO: handle detection of this unit
}

//------------------------------------------------------------------------------
bool cMoveJob::reachedField() const
{
	return (abs(vehicle->getMovementOffset().x()) < currentSpeed && abs(vehicle->getMovementOffset().y()) < currentSpeed);
}

//------------------------------------------------------------------------------
void cMoveJob::moveVehicle(cMap& map)
{
	// TODO: walkframe
	// TODO: tracks

	calcSpeed(map);

	changeVehicleOffset(currentSpeed);

	if (reachedField())
	{
		endMove();
		startMove(map);
	}
}

//------------------------------------------------------------------------------
void cMoveJob::calcSpeed(const cMap &map)
{
	if (vehicle->uiData->animationMovement)
	{
		currentSpeed = MOVE_SPEED / 2;
	}
	else if (!(vehicle->getStaticUnitData().factorAir > 0) && !(vehicle->getStaticUnitData().factorSea > 0 && vehicle->getStaticUnitData().factorGround == 0))
	{
		currentSpeed = MOVE_SPEED;
		cBuilding* building = map.getField(vehicle->getPosition()).getBaseBuilding();
		if (building && building->getStaticUnitData().modifiesSpeed)
			currentSpeed = (int)(currentSpeed / building->getStaticUnitData().modifiesSpeed);
	}
	else if (vehicle->getStaticUnitData().factorAir > 0)
	{
		currentSpeed = MOVE_SPEED * 2;
	}
	else
	{
		currentSpeed = MOVE_SPEED;
	}
}

//------------------------------------------------------------------------------
void cMoveJob::endMove()
{
	vehicle->setMovementOffset (cPosition (0, 0));

	//TODO: sentry reaction
	//TODO: expl. mines
	//TODO: handle detection
	//TODO: lay/ clear mines

	if (vehicle->getStaticUnitData().canSurvey)
	{
		vehicle->doSurvey();
	}
}
