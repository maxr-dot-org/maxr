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
#include <algorithm>
#include <cmath>
#include <vector>

#include "game/logic/automjobs.h"

#include "game/logic/client.h"
#include "game/logic/clientevents.h"
#include "utility/listhelpers.h"
#include "game/logic/movejobs.h"
#include "game/data/player/player.h"
#include "game/data/units/vehicle.h"
#include "game/data/map/map.h"
#include "game/data/report/unit/savedreportsurveyoraisenseless.h"
#include "game/data/report/unit/savedreportsurveyoraiconfused.h"

using namespace std;

static const float FIELD_BLOCKED = -10000.f;
static const int WAIT_FRAMES = 4;

//main tuning knobs of the AI:
static const float A = 1.0f;   //how important is it, to survey as much fields as possible with each move
static const float B = 1.49f;  //how important is it, to stay near the operation point
static const float C = 9.0f;   //how important is it, to hold a distance to other surveyors
static const float EXP = -2.f; //a negative integer; the influence of other surveyors is falling over the distance with x^EXP

// when there are no fields to survey next to the surveyor,
// where should the surveyor resume?
// if the surveyor seems to plan long senseless moves,
// rebalancing the following factors might help
static const float D = 1.f; // more likely near the operation point
static const float E = 3.f; // more likely near his position
static const float EXP2 = -2.f;
static const float F = 100.f; // more likely far away from other surveyors
static const float G = 1.8f; // how important is to go to directions where resources has been found already

static const float MAX_DISTANCE_OP = 19.f; //when the distance to the OP exceeds this value, the OP is changed
static const float DISTANCE_NEW_OP = 7.f; //the new OP will be between the surveyor and the old OP and has distance of DISTANCE_NEW_OP to the surveyor

//functions of cAutoMJobs

cAutoMJob::cAutoMJob (cClient& client_, cVehicle& vehicle_) :
	client (client_),
	vehicle (vehicle_)
{
	lastDestination = vehicle.getPosition();
	finished = false;
	operationPoint = vehicle.getPosition();
	playerMJob = false;

	sendSetAutoStatus (client, vehicle.iID, true);
}

// performs the auto move of a vehicle and
// adds new mjobs to the engine, if necessary
void cAutoMJob::doAutoMove (const std::vector<cAutoMJob*>& jobs)
{
	if (vehicle.isBeeingAttacked()) return;
	if (client.isFreezed()) return;
	if (vehicle.getOwner() != &client.getActivePlayer()) return;

	if (vehicle.getClientMoveJob() == nullptr || vehicle.getClientMoveJob()->bFinished)
	{
		changeOP();
		planNextMove (jobs);
	}
	else
	{
		if (vehicle.getClientMoveJob() && (vehicle.getClientMoveJob()->destination != lastDestination))
		{
			playerMJob = true;
		}
		if (vehicle.getClientMoveJob()->bSuspended && vehicle.data.getSpeed())
		{
			client.addMoveJob (vehicle, vehicle.getClientMoveJob()->destination);
		}
	}
}

void cAutoMJob::stop()
{
	if (!playerMJob)
	{
		sendWantStopMove (client, vehicle.iID);
	}
	sendSetAutoStatus (client, vehicle.iID, false);
}

// think about the next move:
// the AI looks at all fields next to the surveyor
// and calculates a factor for each field
// the surveyor will move to the field with the highest value
void cAutoMJob::planNextMove (const std::vector<cAutoMJob*>& jobs)
{
	// TODO: completely survey a partly explored area with resources,
	//       like in the org MAX
	// TODO: if no resources found in the immediate area of the surveyor,
	//       plan longer path immediately, like in the org MAX.
	//       This will also decrease the number of events
	//       generated by planning every step
	cPosition bestPosition (-1, -1);
	float maxFactor = FIELD_BLOCKED;

	for (int x = vehicle.getPosition().x() - 1; x <= vehicle.getPosition().x() + 1; ++x)
	{
		for (int y = vehicle.getPosition().y() - 1; y <= vehicle.getPosition().y() + 1; ++y)
		{
			const cPosition position (x, y);
			// skip the surveyor's current position
			if (vehicle.isAbove (position)) continue;

			const float tempFactor = calcFactor (position, jobs);
			if (tempFactor > maxFactor)
			{
				maxFactor = tempFactor;
				bestPosition = position;
			}
		}
	}

	if (maxFactor == FIELD_BLOCKED)
	{
		// no fields to survey next to the surveyor
		planLongMove (jobs);
	}
	else
	{
		//TODO: send action
		//client.addMoveJob (vehicle, bestPosition);
		lastDestination = bestPosition;
	}
}

float cAutoMJob::calcScoreDistToOtherSurveyor (const std::vector<cAutoMJob*>& jobs, const cPosition& position, float e) const
{
	float res = 0;

	for (size_t i = 0; i != jobs.size(); ++i)
	{
		if (this == jobs[i]) continue;
		const auto& otherVehicle = jobs[i]->vehicle;
		if (otherVehicle.getOwner() != vehicle.getOwner()) continue;
		const auto dist = (position - otherVehicle.getPosition()).l2Norm();
		res += powf (dist, e);
	}
	return res;
}

// calculates an "importance-factor" for a given field
float cAutoMJob::calcFactor (const cPosition& position, const std::vector<cAutoMJob*>& jobs)
{
	const cMap& map = *client.getModel().getMap();

	if (!map.possiblePlace (vehicle, position, true)) return FIELD_BLOCKED;

	// calculate some values, on which the "importance-factor" may depend

	// calculate the number of fields which would be surveyed by this move
	float NrSurvFields = 0;
	const int minx = std::max (position.x() - 1, 0);
	const int maxx = std::min (position.x() + 1, map.getSize().x() - 1);
	const int miny = std::max (position.y() - 1, 0);
	const int maxy = std::min (position.y() + 1, map.getSize().y() - 1);
	for (int x = minx; x <= maxx; ++x)
	{
		for (int y = miny; y <= maxy; ++y)
		{
			const cPosition position (x, y);

			if (!vehicle.getOwner()->hasResourceExplored (position))  //&& !map.isBlocked(position))
			{
				NrSurvFields++;
			}
		}
	}

	// calculate the number of fields which has already revealed resources
	float NrResFound = 0;
	for (int x = minx; x <= maxx; ++x)
	{
		for (int y = miny; y <= maxy; ++y)
		{
			const cPosition position (x, y);

			// check if the surveyor already found some resources in this new direction or not
			if (vehicle.getOwner()->hasResourceExplored (position) && map.getResource (position).typ != 0)
			{
				NrResFound++;
			}
		}
	}

	// the distance to the OP
	const float newDistanceOP = (position - operationPoint).l2Norm();

	// the distance to other surveyors
	const float newDistancesSurv = calcScoreDistToOtherSurveyor (jobs, position, EXP);

	// and now calc the "importance-factor"

	if (NrSurvFields == 0) return FIELD_BLOCKED;

	float factor = A * NrSurvFields + G * NrResFound - B * newDistanceOP - C * newDistancesSurv;
	factor = std::max (factor, FIELD_BLOCKED);
	return factor;
}

// searches the map for a location where the surveyor can resume
void cAutoMJob::planLongMove (const std::vector<cAutoMJob*>& jobs)
{
	cPosition bestPosition (-1, -1);
	float minValue = 0;
	const cMap& map = *client.getModel().getMap();

	for (int x = 0; x < map.getSize().x(); ++x)
	{
		for (int y = 0; y < map.getSize().y(); ++y)
		{
			const cPosition currentPosition (x, y);

			// if field is not passable/walkable or
			// if it's already has been explored, continue
			if (!map.possiblePlace (vehicle, currentPosition)) continue;
			if (vehicle.getOwner()->hasResourceExplored (currentPosition)) continue;

			// calculate the distance to other surveyors
			const float distancesSurv = calcScoreDistToOtherSurveyor (jobs, currentPosition, EXP2);
			const float distanceOP = (currentPosition - operationPoint).l2Norm();
			const float distanceSurv = (currentPosition - vehicle.getPosition()).l2Norm();
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
		//client.getActivePlayer().addSavedReport (std::make_unique<cSavedReportSurveyorAiSenseless> (vehicle));
		finished = true;
	}
	else
	{
		if (client.addMoveJob (vehicle, bestPosition))
		{
			lastDestination = bestPosition;
		}
		else
		{
			//client.getActivePlayer().addSavedReport (std::make_unique<cSavedReportSurveyorAiConfused> (vehicle));
			finished = true;
		}
	}
}

// places the OP nearer to the surveyor,
// if the distance between surv. and OP exceeds MAX_DISTANCE_OP
// places the OP to the actual position,
// if the surveyor was send there by the player
void cAutoMJob::changeOP()
{
	if (playerMJob)
	{
		operationPoint = vehicle.getPosition();
		playerMJob = false;
	}
	else
	{
		const int sqDistanceOP = (vehicle.getPosition() - operationPoint).l2NormSquared();

		if (float (sqDistanceOP) > Square (MAX_DISTANCE_OP))
		{
			operationPoint = vehicle.getPosition() + (operationPoint - vehicle.getPosition()) * DISTANCE_NEW_OP / MAX_DISTANCE_OP;
		}
	}
}
