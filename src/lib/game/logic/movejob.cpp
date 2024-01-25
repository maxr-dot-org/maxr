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

#include "movejob.h"

#include "game/data/map/map.h"
#include "game/data/map/mapview.h"
#include "game/data/model.h"
#include "game/data/player/player.h"
#include "game/data/units/vehicle.h"
#include "game/logic/gametimer.h"
#include "game/logic/pathcalculator.h"
#include "utility/ranges.h"

//                                 N, NE, E, SE, S, SW, W, NW
static const int directionDx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
static const int directionDy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

constexpr double MOVE_ACCELERATION = 0.08; // change of vehicle speed per tick

namespace
{

	//--------------------------------------------------------------------------
	/**
	* calculates the needed rotation before the next movement
	*/
	std::optional<int> calcNextDir (const cVehicle& vehicle, const cPosition& dest)
	{
		const cPosition diff = dest - vehicle.getPosition();

		for (int i = 0; i != 8; ++i)
		{
			if (diff.x() == directionDx[i] && diff.y() == directionDy[i])
			{
				return i;
			}
		}
		return std::nullopt;
	}

	//--------------------------------------------------------------------------
	/**
	* moves the vehicle by 'offset' pixel in direction of 'nextDir'
	*/
	void changeVehicleOffset (cVehicle& vehicle, int offset, int dir)
	{
		auto newOffset = vehicle.getMovementOffset();
		newOffset.x() += directionDx[dir] * offset;
		newOffset.y() += directionDy[dir] * offset;

		vehicle.setMovementOffset (newOffset);
	}

	//--------------------------------------------------------------------------
	/**
	* check, if the unit finished the current movement step
	*/
	bool reachedField (cVehicle& vehicle)
	{
		const auto& offset = vehicle.getMovementOffset();
		const auto dir = vehicle.dir;
		return (offset.x() * directionDx[dir] >= 0 && offset.y() * directionDy[dir] >= 0);
	}

} // namespace

//------------------------------------------------------------------------------
cMoveJob::cMoveJob() :
	state (eMoveJobState::Finished),
	endMoveAction (cEndMoveAction::None())
{}

//------------------------------------------------------------------------------
cMoveJob::cMoveJob (const std::forward_list<cPosition>& path, cVehicle& vehicle, cModel& model) :
	vehicleId (vehicle.getId()),
	path (path),
	state (eMoveJobState::Waiting),
	endMoveAction (cEndMoveAction::None())
{
}

//------------------------------------------------------------------------------
void cMoveJob::removeVehicle()
{
	vehicleId = std::nullopt;
}

//------------------------------------------------------------------------------
void cMoveJob::run (cModel& model)
{
	auto* vehicle = vehicleId ? model.getVehicleFromID (*vehicleId) : nullptr;
	if (!vehicle || vehicle->getMoveJob() != this)
	{
		state = eMoveJobState::Finished;
	}
	if (state == eMoveJobState::Finished || state == eMoveJobState::Waiting)
	{
		return;
	}
	assert (vehicle);
	if (vehicle->isBeeingAttacked())
	{
		// suspend movejobs of attacked vehicles
		return;
	}

	timer100ms++;
	if (timer100ms == 10) timer100ms = 0;
	timer50ms++;
	if (timer50ms == 5) timer50ms = 0;

	if (!nextDir)
	{
		startMove (model, *vehicle);
	}
	else if (nextDir != static_cast<unsigned int> (vehicle->dir))
	{
		if (timer100ms == 0)
		{
			vehicle->rotateTo (*nextDir);
		}
	}
	else if (!reachedField (*vehicle))
	{
		moveVehicle (model, *vehicle);
	}
	else
	{
		startMove (model, *vehicle);
	}
}

//------------------------------------------------------------------------------
bool cMoveJob::isFinished() const
{
	return state == eMoveJobState::Finished;
}

//------------------------------------------------------------------------------
bool cMoveJob::isWaiting() const
{
	return state == eMoveJobState::Waiting;
}

//------------------------------------------------------------------------------
bool cMoveJob::isActive() const
{
	return state == eMoveJobState::Active || state == eMoveJobState::Stopping;
}

//------------------------------------------------------------------------------
void cMoveJob::stop (cVehicle& vehicle)
{
	assert (vehicleId == vehicle.getId());
	if (isActive())
	{
		state = eMoveJobState::Stopping;
	}
	else
	{
		state = eMoveJobState::Finished;
		vehicle.setMoving (false);
		vehicle.WalkFrame = 0;
		vehicle.data.setSpeed (vehicle.data.getSpeed() + savedSpeed);
	}
}

//------------------------------------------------------------------------------
void cMoveJob::startMove (cModel& model, cVehicle& vehicle)
{
	nextDir = std::nullopt;
	if (path.empty() || state == eMoveJobState::Stopping)
	{
		state = eMoveJobState::Finished;
		vehicle.setMoving (false);
		vehicle.WalkFrame = 0;
		return;
	}
	if (state == eMoveJobState::Waiting)
	{
		return;
	}
	if (vehicle.isBeeingAttacked())
	{
		// suspend movejobs of attacked vehicles
		return;
	}
	if (!handleCollision (model, vehicle))
	{
		vehicle.setMoving (false);
		return;
	}

	cMap& map = *model.getMap();
	const int nextCosts = cPathCalculator::calcNextCost (vehicle.getPosition(), path.front(), &vehicle, &map);
	if (vehicle.data.getSpeed() < nextCosts)
	{
		savedSpeed += vehicle.data.getSpeed();
		vehicle.data.setSpeed (0);
		vehicle.setMoving (false);
		vehicle.WalkFrame = 0;
		state = eMoveJobState::Waiting;
		currentSpeed = 0;
		return;
	}

	// next step can be executed.
	// start the move

	vehicle.setMoving (true);
	nextDir = calcNextDir (vehicle, path.front());

	vehicle.triggerLandingTakeOff (model);

	vehicle.data.setSpeed (vehicle.data.getSpeed() + savedSpeed);
	savedSpeed = 0;
	vehicle.DecSpeed (nextCosts);

	vehicle.tryResetOfDetectionStateBeforeMove (map, model.getPlayerList());

	if (vehicle.getOwner()) vehicle.getOwner()->updateScan (vehicle, path.front());
	map.moveVehicle (vehicle, path.front());

	path.pop_front();

	vehicle.setMovementOffset (cPosition (0, 0));
	changeVehicleOffset (vehicle, -64, *nextDir);

	NetLog.debug (" cMoveJob: Vehicle (ID: " + std::to_string (vehicle.getId()) + ") moved to " + toString (vehicle.getPosition()) + " @" + std::to_string (model.getGameTime()));
}

//------------------------------------------------------------------------------
bool cMoveJob::handleCollision (cModel& model, cVehicle& vehicle)
{
	const cMap& map = *model.getMap();

	//do not drive onto detected enemy mines
	const auto mine = map.getField (path.front()).getMine();
	if (mine && mine->getOwner() != vehicle.getOwner() && vehicle.getOwner() && vehicle.getOwner()->canSeeUnit (*mine, map))
	{
		return recalculatePath (model, vehicle);
	}

	if (map.possiblePlace (vehicle, path.front(), false))
	{
		return true;
	}

	if (map.possiblePlace (vehicle, path.front(), false, true)) // ignore moving units
	{
		// if the target field is blocked by a moving unit,
		// just wait and see if it gets free later
		return false;
	}

	// enemy stealth units get the chance to get out of the way...
	model.sideStepStealthUnit (path.front(), vehicle);
	if (map.possiblePlace (vehicle, path.front(), false))
	{
		return true;
	}

	// field is definitely blocked. Try to find another path to destination
	return recalculatePath (model, vehicle);
}

//------------------------------------------------------------------------------
bool cMoveJob::recalculatePath (cModel& model, cVehicle& vehicle)
{
	if (!vehicle.getOwner()) return false;
	//use owners mapview to calc path
	const auto& playerList = model.getPlayerList();
	auto iter = ranges::find_if (playerList, [&] (const std::shared_ptr<cPlayer>& player) { return player->getId() == vehicle.getOwner()->getId(); });
	const cMapView mapView (model.getMap(), *iter);

	cPosition dest;
	for (const auto& pos : path)
		dest = pos;

	cPathCalculator pc (vehicle, mapView, dest, false);
	auto newPath = pc.calcPath(); //TODO: don't execute path calculation on each model
	if (!newPath.empty())
	{
		const cMap& map = *model.getMap();
		model.sideStepStealthUnit (newPath.front(), vehicle);
		if (map.possiblePlace (vehicle, newPath.front(), false))
		{
			// new path is ok. Use it to continue movement...
			path.swap (newPath);
			return true;
		}
	}

	// no path to destination
	state = eMoveJobState::Finished;
	vehicle.setMoving (false);
	vehicle.WalkFrame = 0;
	vehicle.moveJobBlocked();
	return false;
}

//------------------------------------------------------------------------------
void cMoveJob::moveVehicle (cModel& model, cVehicle& vehicle)
{
	updateSpeed (vehicle, *model.getMap());

	if (timer50ms == 0)
	{
		vehicle.WalkFrame++;
		if (vehicle.WalkFrame > 12) vehicle.WalkFrame = 0;
	}

	pixelToMove += currentSpeed;

	int x = abs (vehicle.getMovementOffset().x());
	int y = abs (vehicle.getMovementOffset().y());
	if (vehicle.getStaticData().makeTracks && ((x > 32 && x - pixelToMove / 100 <= 32) || (y > 32 && y - pixelToMove / 100 <= 32) || (x == 64 && pixelToMove / 100 >= 1) || (y == 64 && pixelToMove / 100 >= 1)))
	{
		// this is a bit crude, but I don't know another simple way of notifying the
		// gui, that is might wants to add a track effect.
		model.triggeredAddTracks (vehicle);
	}

	changeVehicleOffset (vehicle, pixelToMove / 100, vehicle.dir);
	pixelToMove %= 100;

	if (reachedField (vehicle))
	{
		endMove (model, vehicle);
		startMove (model, vehicle);
	}
}

//------------------------------------------------------------------------------
void cMoveJob::updateSpeed (cVehicle& vehicle, const cMap& map)
{
	int maxSpeed = 100 * MOVE_SPEED;

	if (vehicle.getStaticData().animationMovement)
	{
		maxSpeed = 100 * MOVE_SPEED / 2;
	}
	else if (!(vehicle.getStaticUnitData().factorAir > 0) && !(vehicle.getStaticUnitData().factorSea > 0 && vehicle.getStaticUnitData().factorGround == 0))
	{
		maxSpeed = 100 * MOVE_SPEED;
		const cBuilding* building = map.getField (vehicle.getPosition()).getBaseBuilding();
		if (building && static_cast<int> (building->getStaticData().modifiesSpeed))
		{
			maxSpeed /= static_cast<int> (building->getStaticData().modifiesSpeed);
		}
	}
	else if (vehicle.getStaticUnitData().factorAir > 0)
	{
		maxSpeed = 100 * MOVE_SPEED * 2;
	}

	if (path.empty() || state == eMoveJobState::Stopping || cPathCalculator::calcNextCost (vehicle.getPosition(), path.front(), &vehicle, &map) > vehicle.data.getSpeed())
	{
		int maxSpeedBreaking = 100 * sqrt (2 * MOVE_ACCELERATION * vehicle.getMovementOffset().l2Norm());
		maxSpeed = std::min (maxSpeed, maxSpeedBreaking);

		//don't break to zero before movejob is stopped
		maxSpeed = std::max (maxSpeed, 10 * 100);
	}

	if (currentSpeed < maxSpeed)
	{
		currentSpeed += 100 * MOVE_ACCELERATION;
	}
	if (currentSpeed > maxSpeed)
	{
		currentSpeed = maxSpeed;
	}
}

//------------------------------------------------------------------------------
void cMoveJob::endMove (cModel& model, cVehicle& vehicle)
{
	const cMap& map = *model.getMap();
	nextDir.reset();
	vehicle.setMovementOffset (cPosition (0, 0));

	vehicle.detectOtherUnits (map);
	vehicle.detectThisUnit (map, model.getPlayerList());

	cBuilding* mine = map.getField (vehicle.getPosition()).getMine();
	if (mine && vehicle.getStaticUnitData().factorAir == 0 && mine->getOwner() != vehicle.getOwner() && mine->isManualFireActive() == false)
	{
		model.addAttackJob (*mine, vehicle.getPosition());

		vehicle.setMoving (false);
		vehicle.WalkFrame = 0;
		state = eMoveJobState::Waiting;
		currentSpeed = 0;
	}

	if (vehicle.isUnitLayingMines())
	{
		vehicle.layMine (model);
	}
	else if (vehicle.isUnitClearingMines())
	{
		vehicle.clearMine (model);
	}

	vehicle.inSentryRange (model);

	if (vehicle.getStaticData().canSurvey)
	{
		const bool resourceFound = vehicle.doSurvey (map);
		if (resourceFound && stopOn == eStopOn::DetectResource)
		{
			path.clear();
		}
	}

	if (path.empty())
	{
		state = eMoveJobState::Finished;
		vehicle.setMoving (false);
		vehicle.WalkFrame = 0;

		endMoveAction.execute (model, vehicle);
		vehicle.continuePathBuilding (model);
		vehicle.triggerLandingTakeOff (model);
	}
}

//------------------------------------------------------------------------------
void cMoveJob::resume()
{
	if (state == eMoveJobState::Waiting)
	{
		state = eMoveJobState::Active;
	}
}

//------------------------------------------------------------------------------
void cMoveJob::setEndMoveAction (const cEndMoveAction& e)
{
	endMoveAction = e;
}

//------------------------------------------------------------------------------
uint32_t cMoveJob::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (vehicleId, crc);
	crc = calcCheckSum (path, crc);
	crc = calcCheckSum (state, crc);
	crc = calcCheckSum (savedSpeed, crc);
	crc = calcCheckSum (nextDir, crc);
	crc = calcCheckSum (timer100ms, crc);
	crc = calcCheckSum (timer50ms, crc);
	crc = calcCheckSum (currentSpeed, crc);
	crc = calcCheckSum (pixelToMove, crc);
	crc = calcCheckSum (endMoveAction, crc);
	crc = calcCheckSum (stopOn, crc);

	return crc;
}
