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
#include "math.h"
#include "mjobs.h"
#include "client.h"
#include "clientevents.h"
#include "automjobs.h"
#include "vehicles.h"


static cList<cAutoMJob*> autoMJobs;

//static functions of cAutoMJob

//static function that calls DoAutoMove for all active auto move jobs
//this function is periodically called by the engine
void cAutoMJob::handleAutoMoveJobs()
{
	for (size_t i = 0; i < autoMJobs.Size(); ++i)
	{
		autoMJobs[i]->DoAutoMove();
		if ( autoMJobs[i]->finished )
		{
			delete autoMJobs[i];
		}
	}
}

//functions of cAutoMJobs

//construktor for cAutoMJob
cAutoMJob::cAutoMJob(cVehicle *vehicle)
{
	 iNumber = autoMJobs.Size();
	 autoMJobs.Add(this);
	 this->vehicle = vehicle;
	 finished = false;
	 OPX = vehicle->PosX;
	 OPY = vehicle->PosY;
	 playerMJob = false;
	 lastMoveJob = NULL;
	 n = iNumber % WAIT_FRAMES; //this is just to prevent, that posibly all surveyors try to calc their next move in the same frame
}

//destruktor for cAutoMJob
cAutoMJob::~cAutoMJob()
{
	if (!playerMJob && lastMoveJob )
	{
		sendWantStopMove( vehicle->iID );
	}
	for (size_t i = iNumber; i < autoMJobs.Size() - 1; i++)
	{
		autoMJobs[i] = autoMJobs[i + 1];
		autoMJobs[i]->iNumber = i;
	}
	autoMJobs.PopBack();

	vehicle->autoMJob = NULL;
}

//performs the auto move of a vehicle and adds new mjobs to the engine, if nessesary
void cAutoMJob::DoAutoMove()
{
	if ( vehicle->bIsBeeingAttacked ) return;
	if ( Client->bWaitForOthers ) return;
	if ( vehicle->owner != Client->ActivePlayer ) return;

	if ( vehicle->mjob == NULL || vehicle->mjob->finished )
	{
			if (n > WAIT_FRAMES)
			{
				changeOP();
				PlanNextMove();
				n = 0;
			}
			else
			{
				n++;
			}
	}
	else
	{
		if ( vehicle->mjob != lastMoveJob && !vehicle->mjob->Suspended  )
		{
			playerMJob = true;
		}
		if ( vehicle->mjob->Suspended && vehicle->data.speed )
		{
			Client->addMoveJob( vehicle, vehicle->mjob->DestX + vehicle->mjob->DestY * Client->Map->size);
			lastMoveJob = vehicle->mjob;
			n = iNumber % WAIT_FRAMES; //prevent, that all surveyors try to calc their next move in the same frame
		}
	}

}

//think about the next move:
//the AI looks at all fields next to the surveyor
//and calculates an factor for each field
//the surveyor will move to the field with the highest value
void cAutoMJob::PlanNextMove()
{

	//TODO: survey an partly detected area with ressources completly, like in the org MAX
	int x, y;
	int bestX, bestY;
	float tempFactor;
	float maxFactor = FIELD_BLOCKED;

	for ( x = vehicle->PosX - 1; x <= vehicle->PosX + 1; x ++)
	{
		for (y = vehicle->PosY - 1; y <= vehicle->PosY + 1; y++)
		{
			if ( x == vehicle->PosX && y == vehicle->PosY ) continue;

			tempFactor = CalcFactor( x, y);
			if ( tempFactor > maxFactor )
			{
				maxFactor = tempFactor;
				bestX = x;
				bestY = y;
			}
		}
	}

	if ( maxFactor != FIELD_BLOCKED )
	{
		Client->addMoveJob( vehicle, bestX + bestY * Client->Map->size );
		lastMoveJob = vehicle->mjob;
	}
	else //no fields to survey next to the surveyor
	{
		PlanLongMove();
	}
}


//calculates an "importance-factor" for a given field
float cAutoMJob::CalcFactor(int PosX, int PosY)
{
	if ( !FieldIsFree(PosX, PosY) ) return FIELD_BLOCKED;

	//calculate some values, on which the "impotance-factor" may depend

	//the number of fields which would be surveyed by this move
	float NrSurvFields = 0;
	int x, y;

	for ( x = PosX - 1; x <= PosX + 1; x ++)
	{
		for (y = PosY - 1; y <= PosY + 1; y++)
		{
			if ( x == PosX && y == PosY ) continue;
			if ( x < 0 || y < 0 || x >= Client->Map->size || y >= Client->Map->size ) continue;

			int terrainNr = Client->Map->Kacheln[x + y * Client->Map->size];
			if ( vehicle->owner->ResourceMap[x + y * Client->Map->size] == 0 )//&& !Client->Map->terrain[terrainNr].blocked )
			{
				NrSurvFields++;
			}
		}
	}
	if (vehicle->PosX != PosX && vehicle->PosY != PosY) //diagonal move
	{
		NrSurvFields /= 2;
	}

	//the distance to the OP
	float newDistanceOP = sqrt( (float) (PosX - OPX) * (PosX - OPX) + (PosY - OPY) * (PosY - OPY) );

	//the distance to other surveyors
	float newDistancesSurv = 0;
	float temp;
	for (size_t i = 0; i < autoMJobs.Size(); ++i)
	{
		if ( i == iNumber ) continue;
		if (autoMJobs[i]->vehicle->owner != vehicle->owner) continue;

		temp = sqrt( pow( (float) PosX - autoMJobs[i]->vehicle->PosX , 2) + pow( (float) PosY - autoMJobs[i]->vehicle->PosY , 2) );
		newDistancesSurv += pow( temp, EXP);
	}

	//and now calc the "importance-factor"

	if (NrSurvFields == 0) return FIELD_BLOCKED;

	float factor = A * NrSurvFields - B * newDistanceOP - C * newDistancesSurv;

	if (factor < FIELD_BLOCKED)
	{
		factor = FIELD_BLOCKED;
	}

	return  factor;

}

//checks if the destination field is free
bool cAutoMJob::FieldIsFree(int PosX, int PosY)
{
	cMap* map = Client->Map;

	if ( PosX < 0 || PosY < 0 || PosX >= map->size || PosY >= map->size ) return false; //check map borders

	int terrainNr = map->Kacheln[PosX + PosY * map->size];
	if ( map->terrain[terrainNr].blocked ) return false; //check terrain

	sGameObjects objects = map->GO[PosX + PosY * map->size];
	if ( objects.reserviert || objects.vehicle || ( objects.top && !objects.top->data.is_connector) ) return false; //check if there is another unit on the field

	if ( objects.base && objects.base->data.is_expl_mine && objects.base->owner != vehicle->owner) return false; //check for enemy mines

	return true;
}

//searches the map for a location where the surveyor can resume
void cAutoMJob::PlanLongMove()
{
	int x, y;
	int bestX, bestY;
	float distanceOP, distanceSurv;
	float tempValue;
	float minValue = 0;

	for ( x = 0; x < Client->Map->size; x++ )
	{
		for ( y = 0; y < Client->Map->size; y++ )
		{
			if ( !FieldIsFree( x, y) ) continue;
			if ( vehicle->owner->ResourceMap[x + y * Client->Map->size] == 1 ) continue;

			//the distance to other surveyors
			float distancesSurv = 0;
			float temp;
			for (size_t i = 0; i < autoMJobs.Size(); ++i)
			{
				if ( i == iNumber ) continue;
				if (autoMJobs[i]->vehicle->owner != vehicle->owner) continue;

				temp = sqrt( pow( (float) x - autoMJobs[i]->vehicle->PosX , 2) + pow( (float) y - autoMJobs[i]->vehicle->PosY , 2) );
				distancesSurv += pow( temp, EXP2);
			}

			distanceOP = sqrt( (float) (x - OPX) * (x - OPX) + (y - OPY) * (y - OPY) );
			distanceSurv = sqrt( (float) (x - vehicle->PosX) * (x - vehicle->PosX) + (y - vehicle->PosY) * (y - vehicle->PosY) );
			tempValue = D * distanceOP + E * distanceSurv + F * distancesSurv;

			if ( (tempValue < minValue) || (minValue == 0) )
			{
				minValue = tempValue;
				bestX = x;
				bestY = y;
			}
			//TODO: check, if a path can be found to the coordinates
		}
	}
	if ( minValue != 0 )
	{
		Client->addMoveJob( vehicle, bestX + bestY * Client->Map->size);
		lastMoveJob = vehicle->mjob;
		if ( !lastMoveJob || lastMoveJob->finished )
		{
			Client->addCoords( "Surveyor AI: I'm totally confused. Don't know what to do...", vehicle->PosX, vehicle->PosY );
			finished = true;
		}
	}
	else
	{
		Client->addCoords( "Surveyor AI: My life is so senseless. I've nothing to do...", vehicle->PosX, vehicle->PosY );
		finished = true;
	}
}

//places the OP nearer to the surveyor, if the distance between surv. and OP exceeds MAX_DISTANCE_OP
//places the OP to the actual position, if the surveyor was send there by the player
void cAutoMJob::changeOP()
{
	if ( playerMJob )
	{
		OPX = vehicle->PosX;
		OPY = vehicle->PosY;
		playerMJob = false;
	}
	else
	{
		float distanceOP = sqrt( (float) (vehicle->PosX - OPX) * (vehicle->PosX - OPX) + (vehicle->PosY - OPY) * (vehicle->PosY - OPY) );
		if ( distanceOP > MAX_DISTANCE_OP )
		{
			OPX = (int) (vehicle->PosX + ( OPX - vehicle->PosX ) * (float) DISTANCE_NEW_OP / MAX_DISTANCE_OP);
			OPY = (int) (vehicle->PosY + ( OPY - vehicle->PosY ) * (float) DISTANCE_NEW_OP / MAX_DISTANCE_OP);
		}
	}
}
