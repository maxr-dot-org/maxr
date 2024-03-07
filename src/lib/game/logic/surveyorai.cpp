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

#include "game/logic/surveyorai.h"

#include "game/data/map/map.h"
#include "game/data/player/player.h"
#include "game/data/units/vehicle.h"
#include "game/logic/client.h"
#include "utility/listhelpers.h"
#include "utility/mathtools.h"
#include "utility/ranges.h"

#include <algorithm>
#include <cmath>
#include <forward_list>

static const float FIELD_BLOCKED = -10000.f;
static const int ACTION_TIMEOUT = 50;

//main tuning knobs of the AI:
static const float A = 1.5f; //how important is it, to survey as much fields as possible with each move
static const float B = 1.3f; //how important is it, to stay near the operation point
static const float C = 9.0f; //how important is it, to hold a distance to other surveyors
static const float G = 2.0f; //how important is to go to directions where resources has been found already
static const float EXP = -1.f; //a negative integer; the influence of other surveyors is falling over the distance with x^EXP

// when there are no fields to survey next to the surveyor,
// where should the surveyor resume?
// if the surveyor seems to plan long senseless moves,
// rebalancing the following factors might help
static const float D = 1.f; // more likely near the operation point
static const float E = 3.f; // more likely near his position
static const float EXP2 = -1.f;
static const float F = 100.f; // more likely far away from other surveyors

static const int MAX_DISTANCE_OP = 19; //when the distance to the OP exceeds this value, the OP is changed
static const int DISTANCE_NEW_OP = 7; //the new OP will be between the surveyor and the old OP and has distance of DISTANCE_NEW_OP to the surveyor

//------------------------------------------------------------------------------
cSurveyorAi::cSurveyorAi (const cVehicle& vehicle) :
	vehicle (vehicle),
	operationPoint (vehicle.getPosition())
{
	connectionManager.connect (vehicle.destroyed, [this]() { finished = true; });
	connectionManager.connect (vehicle.ownerChanged, [this]() { finished = true; });
}

//------------------------------------------------------------------------------
// plans the auto move of the surveyor and
// adds new movejobs to the model, if necessary
void cSurveyorAi::run (cClient& client, const std::vector<std::unique_ptr<cSurveyorAi>>& jobs)
{
	if (finished) return;
	if (counter > 0)
	{
		counter--;
		return;
	}

	if (vehicle.isBeeingAttacked()) return;

	const cMap& map = *client.getModel().getMap();

	if (vehicle.getMoveJob() == nullptr)
	{
		changeOP();
		std::forward_list<cPosition> path;
		//push the starting point for planing
		path.push_front (vehicle.getPosition());

		int movePoints = vehicle.data.getSpeed();
		if (movePoints < vehicle.data.getSpeedMax())
		{
			movePoints += vehicle.data.getSpeedMax();
		}
		planMove (path, movePoints, jobs, map);

		// planMove gives the path with the last way point at the front. So reverse it.
		path.reverse();
		//remove the starting point of the path, as this position is not required in a movejob path
		path.pop_front();

		if (!path.empty())
		{
			client.startMove (vehicle, path, eStart::Immediate, eStopOn::DetectResource, cEndMoveAction::None());
			counter = ACTION_TIMEOUT;
		}
		else
		{
			planLongMove (jobs, client);
		}
	}
	else
	{
		const auto& moveJob = *vehicle.getMoveJob();
		if (moveJob.isWaiting() && !moveJob.getPath().empty())
		{
			int nextCosts = cPathCalculator::calcNextCost (vehicle.getPosition(), moveJob.getPath().front(), &vehicle, &map);
			if (nextCosts <= vehicle.data.getSpeed())
			{
				client.resumeMoveJob (vehicle);
				counter = ACTION_TIMEOUT;
			}
		}
	}
}

//------------------------------------------------------------------------------
void cSurveyorAi::planMove (std::forward_list<cPosition>& path, int remainingMovePoints, const std::vector<std::unique_ptr<cSurveyorAi>>& jobs, const cMap& map) const
{
	cPosition position = path.front();

	cPosition bestNextPosition;
	float bestNextFactor = FIELD_BLOCKED;
	int bestNextMoveCosts;

	for (const cPosition nextPosition : map.staticMap->collectAroundPositions (position, vehicle.getIsBig()))
	{
		// check out of move points
		int nextMoveCosts = cPathCalculator::calcNextCost (position, nextPosition, &vehicle, &map);
		if (nextMoveCosts > remainingMovePoints) continue;

		const float nextFactor = calcFactor (nextPosition, path, jobs, map);
		if (nextFactor > bestNextFactor)
		{
			bestNextFactor = nextFactor;
			bestNextPosition = nextPosition;
			bestNextMoveCosts = nextMoveCosts;
		}
	}

	if (bestNextFactor > FIELD_BLOCKED)
	{
		path.push_front (bestNextPosition);
		planMove (path, remainingMovePoints - bestNextMoveCosts, jobs, map);
	}
}

//------------------------------------------------------------------------------
float cSurveyorAi::calcScoreDistToOtherSurveyor (const std::vector<std::unique_ptr<cSurveyorAi>>& jobs, const cPosition& position, float e) const
{
	float res = 0;

	for (const auto& job : jobs)
	{
		if (job.get() == this) continue;
		const auto& otherVehicle = job->vehicle;
		if (otherVehicle.getOwner() != vehicle.getOwner()) continue;
		const auto dist = static_cast<float> ((position - otherVehicle.getPosition()).l2Norm());
		res += powf (dist, e);
	}
	return res;
}

//------------------------------------------------------------------------------
// calculates an "importance-factor" for a given field
float cSurveyorAi::calcFactor (const cPosition& position, const std::forward_list<cPosition>& path, const std::vector<std::unique_ptr<cSurveyorAi>>& jobs, const cMap& map) const
{
	if (!map.possiblePlace (vehicle, position, true)) return FIELD_BLOCKED;

	const auto& owner = *vehicle.getOwner();

	// calculate some values, on which the "importance-factor" may depend

	// calculate the number of fields which would be surveyed by this move
	// also count how many of these fields are adjacent to already revealed resources
	float nrSurvFields = 0;
	float nrFielsdWithAdjacentRes = 0;
	for (const auto& aroundPosition : map.staticMap->collectAroundPositions (position, vehicle.getIsBig()))
	{
		if (positionHasBeenSurveyedByPath (aroundPosition, path)) continue;

		if (!owner.hasResourceExplored (aroundPosition)) //&& !map.isBlocked (aroundPosition))
		{
			nrSurvFields++;
			if (hasAdjacentResources (aroundPosition, map))
			{
				// spots with adjacent revealed resources have a high probability
				// for more resources
				nrFielsdWithAdjacentRes++;
			}
		}
	}
	// diagonal moves are more expensive. So scale down NrSurvFields
	if ((position - path.front()).l2NormSquared() > 1)
	{
		nrSurvFields /= 1.5;
		nrFielsdWithAdjacentRes /= 1.5;
	}

	// the distance to the OP
	const float newDistanceOP = static_cast<float> ((position - operationPoint).l2Norm());

	// the distance to other surveyors
	const float newDistancesSurv = calcScoreDistToOtherSurveyor (jobs, position, EXP);

	// and now calc the "importance-factor"

	if (nrSurvFields == 0) return FIELD_BLOCKED;

	float factor = A * nrSurvFields + G * nrFielsdWithAdjacentRes - B * newDistanceOP - C * newDistancesSurv;
	factor = std::max (factor, FIELD_BLOCKED);
	return factor;
}

//------------------------------------------------------------------------------
// searches the map for a location where the surveyor can resume
void cSurveyorAi::planLongMove (const std::vector<std::unique_ptr<cSurveyorAi>>& jobs, cClient& client)
{
	const cModel& model = client.getModel();
	const cMap& map = *model.getMap();
	const cPlayer& player = *vehicle.getOwner();

	cPosition bestPosition;
	float minValue = 0;

	for (int x = 0; x < map.getSize().x(); ++x)
	{
		for (int y = 0; y < map.getSize().y(); ++y)
		{
			const cPosition currentPosition (x, y);

			// if field is not passable/walkable or
			// if it's already has been explored, continue
			if (!map.possiblePlace (vehicle, currentPosition, false)) continue;
			if (player.hasResourceExplored (currentPosition)) continue;

			// calculate the distance to other surveyors
			const float distancesSurv = calcScoreDistToOtherSurveyor (jobs, currentPosition, EXP2);
			const float distanceOP = static_cast<float> ((currentPosition - operationPoint).l2Norm());
			const float distanceSurv = static_cast<float> ((currentPosition - vehicle.getPosition()).l2Norm());
			// TODO: take into account the length of the path to
			// the coordinates too
			// (I seen a case, when a surveyor took 7 additional senseless steps
			//  just to avoid or by-pass an impassable rocky terrain)
			const float factor = D * distanceOP + E * distanceSurv + F * distancesSurv;

			if ((factor < minValue) || (minValue == 0))
			{
				minValue = factor;
				bestPosition = currentPosition;
			}
			// TODO: check, if a path can be found to the coordinates
		}
	}
	if (minValue == 0)
	{
		client.surveyorAiConfused (vehicle);
		client.setAutoMove (vehicle, false);
		finished = true;
	}
	else
	{
		//use owners mapview to calc path
		const auto& playerList = model.getPlayerList();
		auto iter = ranges::find_if (playerList, [&] (const std::shared_ptr<cPlayer>& p) {
			return p->getId() == player.getId();
		});
		const cMapView mapView (model.getMap(), *iter);

		cPathCalculator pc (vehicle, mapView, bestPosition, false);
		const auto path = pc.calcPath();
		if (!path.empty())
		{
			client.startMove (vehicle, path, eStart::Immediate, eStopOn::DetectResource, cEndMoveAction::None());
			counter = ACTION_TIMEOUT;
		}
		else
		{
			client.surveyorAiConfused (vehicle);
			client.setAutoMove (vehicle, false);
			finished = true;
		}
	}
}

//------------------------------------------------------------------------------
// places the operation point nearer to the surveyor,
// if the distance between surv. and OP exceeds MAX_DISTANCE_OP
// places the OP to the actual position,
// if the surveyor was send there by the player
void cSurveyorAi::changeOP()
{
	const int sqDistanceOP = (vehicle.getPosition() - operationPoint).l2NormSquared();

	if (sqDistanceOP > Square (MAX_DISTANCE_OP))
	{
		operationPoint = vehicle.getPosition() + ((operationPoint - vehicle.getPosition()) * DISTANCE_NEW_OP) / MAX_DISTANCE_OP;
	}
}

//------------------------------------------------------------------------------
bool cSurveyorAi::positionHasBeenSurveyedByPath (const cPosition& position, const std::forward_list<cPosition>& path) const
{
	return ranges::any_of (path, [&] (const auto& pathPos) { return (pathPos - position).l2NormSquared() <= 2; });
}

//------------------------------------------------------------------------------
bool cSurveyorAi::hasAdjacentResources (const cPosition& centerPosition, const cMap& map) const
{
	const cPlayer& owner = *vehicle.getOwner();

	return ranges::any_of (map.staticMap->collectAroundPositions (centerPosition, vehicle.getIsBig()), [&] (const cPosition& position) {
		return owner.hasResourceExplored (position) && map.getResource (position).typ != eResourceType::None;
	});
}
