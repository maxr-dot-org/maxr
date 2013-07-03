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
#include <math.h>
#include <vector>

#include "automjobs.h"

#include "client.h"
#include "clientevents.h"
#include "clist.h"
#include "movejobs.h"
#include "player.h"
#include "vehicles.h"

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

static std::vector<cAutoMJob*> autoMJobs;

// static function that calls DoAutoMove for all active auto move jobs
// this function is periodically called by the engine
/*static*/ void cAutoMJob::handleAutoMoveJobs()
{
	for (size_t i = 0; i != ::autoMJobs.size(); ++i)
	{
		::autoMJobs[i]->DoAutoMove (::autoMJobs, i);
		if (::autoMJobs[i]->isFinished())
		{
			delete ::autoMJobs[i];
			--i;
		}
	}
}

//functions of cAutoMJobs

cAutoMJob::cAutoMJob (cClient& client_, cVehicle* vehicle) :
	client (&client_),
	vehicle (vehicle)
{
	::autoMJobs.push_back (this);
	lastDestX = vehicle->PosX;
	lastDestY = vehicle->PosY;
	// this is just to prevent,
	// that possibly all surveyors try to calc their next move in the same frame
	n = (int) ::autoMJobs.size() % WAIT_FRAMES;
	finished = false;
	OPX = vehicle->PosX;
	OPY = vehicle->PosY;
	playerMJob = false;
}

//destruktor for cAutoMJob
cAutoMJob::~cAutoMJob()
{
	if (!playerMJob)
	{
		sendWantStopMove (*client, vehicle->iID);
	}
	sendSetAutoStatus (*client, vehicle->iID, false);

	Remove (::autoMJobs, this);

	vehicle->autoMJob = NULL;
}

// performs the auto move of a vehicle and
// adds new mjobs to the engine, if necessary
void cAutoMJob::DoAutoMove (const std::vector<cAutoMJob*>& jobs, int iNumber)
{
	if (vehicle->isBeeingAttacked) return;
	if (client->isFreezed()) return;
	if (vehicle->owner != client->getActivePlayer()) return;

	if (vehicle->ClientMoveJob == NULL || vehicle->ClientMoveJob->bFinished)
	{
		if (n > WAIT_FRAMES)
		{
			changeOP();
			PlanNextMove (jobs);
			n = 0;
		}
		else
		{
			n++;
		}
	}
	else
	{
		if (vehicle->ClientMoveJob && (vehicle->ClientMoveJob->DestX != lastDestX || vehicle->ClientMoveJob->DestY != lastDestY))
		{
			playerMJob = true;
		}
		if (vehicle->ClientMoveJob->bSuspended && vehicle->data.speedCur)
		{
			client->addMoveJob (vehicle, vehicle->ClientMoveJob->DestX, vehicle->ClientMoveJob->DestY);
			// prevent, that all surveyors try to calc
			// their next move in the same frame
			n = iNumber % WAIT_FRAMES;
		}
	}
}

// think about the next move:
// the AI looks at all fields next to the surveyor
// and calculates a factor for each field
// the surveyor will move to the field with the highest value
void cAutoMJob::PlanNextMove (const std::vector<cAutoMJob*>& jobs)
{
	// TODO: completely survey a partly explored area with resources,
	//       like in the org MAX
	// TODO: if no resources found in the immediate area of the surveyor,
	//       plan longer path immediately, like in the org MAX.
	//       This will also decrease the number of events
	//       generated by planning every step
	int bestX = -1, bestY = -1;
	float maxFactor = FIELD_BLOCKED;

	for (int x = vehicle->PosX - 1; x <= vehicle->PosX + 1; ++x)
	{
		for (int y = vehicle->PosY - 1; y <= vehicle->PosY + 1; ++y)
		{
			// skip the surveyor's current position
			if (x == vehicle->PosX && y == vehicle->PosY) continue;

			const float tempFactor = CalcFactor (x, y, jobs);
			if (tempFactor > maxFactor)
			{
				maxFactor = tempFactor;
				bestX = x;
				bestY = y;
			}
		}
	}

	if (maxFactor != FIELD_BLOCKED)
	{
		client->addMoveJob (vehicle, bestX, bestY);
		lastDestX = bestX;
		lastDestY = bestY;
	}
	else //no fields to survey next to the surveyor
	{
		PlanLongMove (jobs);
	}
}

float cAutoMJob::calcScoreDistToOtherSurveyor (const std::vector<cAutoMJob*>& jobs, int PosX, int PosY, float e) const
{
	float res = 0;

	for (size_t i = 0; i != jobs.size(); ++i)
	{
		if (this == jobs[i]) continue;
		const cVehicle* otherVehicle = jobs[i]->vehicle;
		if (otherVehicle->owner != vehicle->owner) continue;
		const int sqDist = Square (PosX - otherVehicle->PosX) + Square (PosY - otherVehicle->PosY);
		const float dist = sqrtf (float (sqDist));
		res += powf (dist, e);
	}
	return res;
}

// calculates an "importance-factor" for a given field
float cAutoMJob::CalcFactor (int PosX, int PosY, const std::vector<cAutoMJob*>& jobs)
{
	const cMap& map = *client->getMap();

	if (!map.possiblePlace (*vehicle, PosX, PosY, true)) return FIELD_BLOCKED;

	// calculate some values, on which the "importance-factor" may depend

	// calculate the number of fields which would be surveyed by this move
	float NrSurvFields = 0;
	const int minx = std::max (PosX - 1, 0);
	const int maxx = std::min (PosX + 1, map.getSize() - 1);
	const int miny = std::max (PosY - 1, 0);
	const int maxy = std::min (PosY + 1, map.getSize() - 1);
	for (int x = minx; x <= maxx; ++x)
	{
		for (int y = miny; y <= maxy; ++y)
		{
			const int iPos = map.getOffset (x, y);

			if (!vehicle->owner->hasResourceExplored (iPos)) //&& !map.isBlocked(iPos))
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
			const int iPos = map.getOffset (x, y);

			// check if the surveyor already found some resources in this new direction or not
			if (vehicle->owner->hasResourceExplored (iPos) && map.getResource (iPos).typ != 0)
			{
				NrResFound++;
			}
		}
	}

	// the distance to the OP
	const float newDistanceOP = sqrtf (float (Square (PosX - OPX) + Square (PosY - OPY)));

	// the distance to other surveyors
	const float newDistancesSurv = calcScoreDistToOtherSurveyor (jobs, PosX, PosY, EXP);

	// and now calc the "importance-factor"

	if (NrSurvFields == 0) return FIELD_BLOCKED;

	float factor = A * NrSurvFields + G * NrResFound - B * newDistanceOP - C * newDistancesSurv;
	factor = std::max (factor, FIELD_BLOCKED);
	return factor;
}

// searches the map for a location where the surveyor can resume
void cAutoMJob::PlanLongMove (const std::vector<cAutoMJob*>& jobs)
{
	int bestX = -1;
	int bestY = -1;
	float minValue = 0;
	const cMap& map = *client->getMap();

	for (int x = 0; x < map.getSize(); ++x)
	{
		for (int y = 0; y < map.getSize(); ++y)
		{
			// if field is not passable/walkable or
			// if it's already has been explored, continue
			if (!map.possiblePlace (*vehicle, x, y)) continue;
			if (vehicle->owner->hasResourceExplored (map.getOffset (x, y))) continue;

			// calculate the distance to other surveyors
			const float distancesSurv = calcScoreDistToOtherSurveyor (jobs, x, y, EXP2);
			const float distanceOP = sqrtf (float (Square (x - OPX) + Square (y - OPY)));
			const float distanceSurv = sqrtf (float (Square (x - vehicle->PosX) + Square (y - vehicle->PosY)));
			// TODO: take into account the length of the path to
			// the coordinates too
			// (I seen a case, when a surveyor took 7 additional senseless steps
			//  just to avoid or by-pass an impassable rocky terrain)
			const float factor = D * distanceOP + E * distanceSurv + F * distancesSurv;

			if ( (factor < minValue) || (minValue == 0))
			{
				minValue = factor;
				bestX = x;
				bestY = y;
			}
			// TODO: check, if a path can be found to the coordinates
		}
	}
	if (minValue != 0)
	{
		if (client->addMoveJob (vehicle, bestX, bestY))
		{
			lastDestX = bestX;
			lastDestY = bestY;
		}
		else
		{
			string message = "Surveyor AI: I'm totally confused. Don't know what to do...";
			client->getActivePlayer()->addSavedReport (client->gameGUI.addCoords (message, vehicle->PosX, vehicle->PosY), sSavedReportMessage::REPORT_TYPE_UNIT, vehicle->data.ID, vehicle->PosX, vehicle->PosY);
			finished = true;
		}
	}
	else
	{
		string message = "Surveyor AI: My life is so senseless. I've nothing to do...";
		client->getActivePlayer()->addSavedReport (client->gameGUI.addCoords (message, vehicle->PosX, vehicle->PosY), sSavedReportMessage::REPORT_TYPE_UNIT, vehicle->data.ID, vehicle->PosX, vehicle->PosY);
		finished = true;
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
		OPX = vehicle->PosX;
		OPY = vehicle->PosY;
		playerMJob = false;
	}
	else
	{
		const int sqDistanceOP = Square (vehicle->PosX - OPX) + Square (vehicle->PosY - OPY);

		if (float (sqDistanceOP) > Square (MAX_DISTANCE_OP))
		{
			OPX = (int) (vehicle->PosX + (OPX - vehicle->PosX) * DISTANCE_NEW_OP / MAX_DISTANCE_OP);
			OPY = (int) (vehicle->PosY + (OPY - vehicle->PosY) * DISTANCE_NEW_OP / MAX_DISTANCE_OP);
		}
	}
}
