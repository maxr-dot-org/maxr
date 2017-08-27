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
#include "game/data/model.h"

//                           N, NE, E, SE, S, SW, W, NW
const int directionDx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
const int directionDy[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };


cMoveJob::cMoveJob(const std::forward_list<cPosition>& path, cVehicle& vehicle, cModel& model) :
	path(path),
	vehicle(&vehicle),
	savedSpeed(0),
	state(ACTIVE),
	nextDir(0),
	timer100ms(1),
	timer50ms(1),
	currentSpeed(0),
	pixelToMove(0)
{
	startMove(model);
}

cMoveJob::cMoveJob() :
	vehicle(nullptr),
	savedSpeed(0),
	state(FINISHED),
	nextDir(0),
	timer100ms(1),
	timer50ms(1),
	currentSpeed(0),
	pixelToMove(0)
{}

//------------------------------------------------------------------------------
const std::forward_list<cPosition>& cMoveJob::getPath() const
{
	return path;
}

//------------------------------------------------------------------------------
unsigned int cMoveJob::getSavedSpeed() const
{
	return savedSpeed;
}

//------------------------------------------------------------------------------
void cMoveJob::removeVehicle()
{
	vehicle = nullptr;
}

//------------------------------------------------------------------------------
void cMoveJob::run(cModel& model)
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
	timer50ms++;
	if (timer50ms == 5) timer50ms = 0;

	if (nextDir != vehicle->dir)
	{
		if (timer100ms == 0)
		{
			vehicle->rotateTo (nextDir);
		}
	}
	else if (!reachedField())
	{
		moveVehicle(model);
	}
	else
	{
		startMove(model);
	}
}

//------------------------------------------------------------------------------
bool cMoveJob::isFinished() const
{
	return state == FINISHED;
}

//------------------------------------------------------------------------------
bool cMoveJob::isWaiting() const
{
	return state == WAITING;
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
		vehicle->data.setSpeed(vehicle->data.getSpeed() + savedSpeed);
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

	const cPosition diff = path.front() - vehicle->getPosition();

	for (int i = 0; i != 8; ++i)
	{
		if (diff.x() == directionDx[i] && diff.y() == directionDy[i])
		{
			nextDir = i;
		}
	}
}

//------------------------------------------------------------------------------
void cMoveJob::changeVehicleOffset(int offset) const
{
	auto newOffset = vehicle->getMovementOffset();
	newOffset.x() += directionDx[nextDir] * offset;
	newOffset.y() += directionDy[nextDir] * offset;

	vehicle->setMovementOffset(newOffset);
}


//------------------------------------------------------------------------------
void cMoveJob::startMove(cModel& model)
{
	cMap& map = *model.getMap();

	if (path.empty() || state == STOPPING)
	{
		state = FINISHED;
		vehicle->setMoving(false);
		vehicle->WalkFrame = 0;
		return;
	}

	if (vehicle->isBeeingAttacked())
	{
		// suspend movejobs of attacked vehicles
		return;
	}

	bool nextFieldFree = handleCollision(map);
	if (!nextFieldFree)
	{
		return;
	}

	int nextCosts = cPathCalculator::calcNextCost(vehicle->getPosition(), path.front(), vehicle, &map);
	if (vehicle->data.getSpeed() < nextCosts)
	{
		savedSpeed += vehicle->data.getSpeed();
		vehicle->data.setSpeed(0);
		vehicle->setMoving(false);
		vehicle->WalkFrame = 0;
		state = WAITING;
		currentSpeed = 0;
		return;
	}

	// next step can be executed.
	// start the move

	vehicle->setMoving(true);
	calcNextDir();
	
	vehicle->data.setSpeed(vehicle->data.getSpeed() + savedSpeed);
	savedSpeed = 0;
	vehicle->DecSpeed(nextCosts);

	map.moveVehicle(*vehicle, path.front());
	path.pop_front();
	vehicle->setMovementOffset(cPosition(0, 0));
	changeVehicleOffset(-64);

	vehicle->getOwner()->doScan();

	//TODO: handle detection of this unit
}

//------------------------------------------------------------------------------
bool cMoveJob::handleCollision(cMap &map)
{
	if (map.possiblePlace(*vehicle, path.front()))
	{
		return true;
	}

	//TODO: model.sideStepStealthUnit();
	if (map.possiblePlace(*vehicle, path.front()))
	{
		return true;
	}
		
	if (map.possiblePlace(*vehicle, path.front(), false, true))
	{
		// if the target field is blocked by a moving unit,
		// just wait and see if it gets free later
		return false;
	}

	// field is definitely blocked. Try to find another path to destination
	cPosition dest;
	for (const auto& pos : path) dest = pos;
	cPathCalculator pc(*vehicle, map, dest, false);
	auto newPath = pc.calcPath(); //TODO: don't execute path calculation on each model
	if (newPath.empty())
	{
		// no path to destination
		state = FINISHED;
		vehicle->setMoving(false);
		vehicle->WalkFrame = 0;
		vehicle->moveJobBlocked();
		return false;
	}

	// new path is ok. Use it to continue movement...
	path.swap(newPath);
	return true;
}

//------------------------------------------------------------------------------
bool cMoveJob::reachedField() const
{
	const auto& offset = vehicle->getMovementOffset();
	return (offset.x() * directionDx[nextDir] >= 0 && offset.y() * directionDy[nextDir] >= 0);
}

//------------------------------------------------------------------------------
void cMoveJob::moveVehicle(cModel& model)
{
	updateSpeed(*model.getMap());
	
	if (timer50ms == 0)
	{
		vehicle->WalkFrame++;
		if (vehicle->WalkFrame > 12) vehicle->WalkFrame = 0;
	}

	pixelToMove += currentSpeed;

	int x = abs(vehicle->getMovementOffset().x());
	int y = abs(vehicle->getMovementOffset().y());
	if ((x > 32 && x - pixelToMove <= 32) ||
		(y > 32 && y - pixelToMove <= 32) ||
		(x == 64 && pixelToMove >= 1) ||
		(y == 64 && pixelToMove >= 1))
	{
		// this is a bit crude, but I don't know another simple way of notifying the
		// gui, that is might wants to add a track effect.
		model.triggeredAddTracks(*vehicle);
	}

	changeVehicleOffset(static_cast<int>(pixelToMove));
	pixelToMove -= static_cast<int>(pixelToMove);

	if (reachedField())
	{
		endMove(model);
		startMove(model);
	}
}

//------------------------------------------------------------------------------
void cMoveJob::updateSpeed(const cMap &map)
{
	double maxSpeed = MOVE_SPEED;
	if (vehicle->uiData->animationMovement)
	{
		maxSpeed = MOVE_SPEED / 2;
	}
	else if (!(vehicle->getStaticUnitData().factorAir > 0) && !(vehicle->getStaticUnitData().factorSea > 0 && vehicle->getStaticUnitData().factorGround == 0))
	{
		maxSpeed = MOVE_SPEED;
		cBuilding* building = map.getField(vehicle->getPosition()).getBaseBuilding();
		if (building && building->getStaticUnitData().modifiesSpeed)
			maxSpeed = (int)(maxSpeed / building->getStaticUnitData().modifiesSpeed);
	}
	else if (vehicle->getStaticUnitData().factorAir > 0)
	{
		maxSpeed = MOVE_SPEED * 2;
	}

	if (path.empty() || state == STOPPING || cPathCalculator::calcNextCost(vehicle->getPosition(), path.front(), vehicle, &map) > vehicle->data.getSpeed())
	{
		double maxSpeedBreaking = sqrt(2 * MOVE_ACCELERATION * vehicle->getMovementOffset().l2Norm());
		maxSpeed = std::min(maxSpeed, maxSpeedBreaking);
		
		//don't break to zero before movejob is stopped
		maxSpeed = std::max(maxSpeed, 0.1);
	}
	
	if (currentSpeed < maxSpeed)
	{
		currentSpeed += MOVE_ACCELERATION;
	}
	if (currentSpeed > maxSpeed)
	{
		currentSpeed = maxSpeed;
	}
}

//------------------------------------------------------------------------------
void cMoveJob::endMove(cModel& model)
{
	vehicle->setMovementOffset (cPosition (0, 0));

	//TODO: expl. mines
	//TODO: handle detection
	//TODO: lay/ clear mines

	vehicle->inSentryRange(model);

	if (vehicle->getStaticUnitData().canSurvey)
	{
		vehicle->doSurvey();
	}

	if (path.empty())
	{
		endMoveAction.execute(model);
	}
}

//------------------------------------------------------------------------------
void cMoveJob::resume()
{
	if (state == WAITING)
	{
		state = ACTIVE;
	}
}

//------------------------------------------------------------------------------
void cMoveJob::setEndMoveAction(const cEndMoveAction& e)
{
	endMoveAction = e;
}

//------------------------------------------------------------------------------
const cEndMoveAction& cMoveJob::getEndMoveAction() const
{
	return endMoveAction;
}

//------------------------------------------------------------------------------
uint32_t cMoveJob::getChecksum(uint32_t crc) const
{
	crc = calcCheckSum(vehicle, crc);
	crc = calcCheckSum(path, crc);
	crc = calcCheckSum(state, crc);
	crc = calcCheckSum(savedSpeed, crc);
	crc = calcCheckSum(nextDir, crc);
	crc = calcCheckSum(timer100ms, crc);
	crc = calcCheckSum(timer50ms, crc);
	crc = calcCheckSum(currentSpeed, crc);
	crc = calcCheckSum(pixelToMove, crc);
	crc = calcCheckSum(endMoveAction, crc);

	return crc;
}

