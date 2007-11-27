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
#include "automjobs.h"
#include "engine.h"
#include "vehicles.h"



//static variables
cEngine *cAutoMJob::engine = NULL;
cAutoMJob **cAutoMJob::autoMJobs = NULL;
int cAutoMJob::iCount = 0;

//static functions of cAutoMJob

//static funktion, that initializes the cAutoMJob-Class
void cAutoMJob::init(cEngine* engine)
{
	cAutoMJob::engine = engine;
	iCount = 0;
}

//static function that calls DoAutoMove for all active auto move jobs
//this function is periodically called by the engine
void cAutoMJob::handleAutoMoveJobs()
{
	int i;
	for ( i = 0; i < iCount; i++)
	{
		autoMJobs[i]->DoAutoMove();
	}
}

//functions of cAutoMJobs

//construktor for cAutoMJob
cAutoMJob::cAutoMJob(cVehicle *vehicle)
{
	 autoMJobs = (cAutoMJob **) realloc(autoMJobs, (iCount + 1) * sizeof(this));
	 autoMJobs[iCount] = this;
	 iNumber = iCount;
	 iCount++;
	 this->vehicle = vehicle;
	 OPX = vehicle->PosX;
	 OPY = vehicle->PosY;
	 n = iNumber % WAIT_FRAMES; //this is just to prevent, that posibly all surveyors try to calc their next move in the same frame
}

//destruktor for cAutoMJob
cAutoMJob::~cAutoMJob()
{
	int i;
	for (i = iNumber; i < iCount - 1; i++)
	{
		autoMJobs[i] = autoMJobs[i + 1];
		autoMJobs[i]->iNumber = i;
	}
	iCount--;
	autoMJobs = (cAutoMJob **) realloc(autoMJobs, iCount * sizeof(this));
}

//performs the auto move of a vehicle and adds new mjobs to the engine, if nessesary
void cAutoMJob::DoAutoMove()
{
	//TODO: check if surveyor was moved by the player, and set new operation point
	if (vehicle->mjob == NULL || vehicle->mjob->finished )
	{
		
		if (n > WAIT_FRAMES)
		{
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
		if (vehicle->mjob->Suspended && vehicle->data.speed)
		{
			engine->AddActiveMoveJob(vehicle->mjob);
		}
	}

}

//think about the next move:
//the AI look at all fields next to the surveyor
//and calculates an factor for each field
//the surveyor will move to the field with the highest value
void cAutoMJob::PlanNextMove()
{

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
		engine->AddMoveJob(vehicle->PosX + vehicle->PosY * engine->map->size, bestX + bestY * engine->map->size, false, false);
		if (vehicle->mjob->finished) //debug
		{
			cLog::write("Error in Surveyor AI: no path to dest", LOG_TYPE_DEBUG);
		}
	}	
	else //no fields to survey next to the surveyor
	{
		//TODO: search for the nearest field to survey
	}
}


//calculates an "importance-factor" for a given field
float cAutoMJob::CalcFactor(int PosX, int PosY)
{
	//check if the destination field is free
	//we do this here manually, because this is much faster than using the result of AddMoveJob()

	if ( PosX < 0 || PosY < 0 || PosX >= engine->map->size || PosY >= engine->map->size ) return FIELD_BLOCKED; //check map borders
	
	int terrainNr=engine->map->Kacheln[PosX + PosY * engine->map->size];
	if ( TerrainData.terrain[terrainNr].blocked ) return FIELD_BLOCKED; //check terrain
	
	sGameObjects objects = engine->map->GO[PosX + PosY * engine->map->size];
	if ( objects.reserviert || objects.reserviert || objects.top ) return FIELD_BLOCKED; //check if there is another unit on the field


	//next, calculate some values, on which the "impotance-factor" may depend

	//the number of fields which would be surveyed by this move
	float NrSurvFields = 0; 
	int x, y;

	for ( x = PosX - 1; x <= PosX + 1; x ++)
	{	
		for (y = PosY - 1; y <= PosY + 1; y++)
		{
			if ( x == PosX && y == PosY ) continue;

			if ( vehicle->owner->ResourceMap[x + y * engine->map->size] == 0)
			{
				NrSurvFields++;
			}
		}
	}
	if (vehicle->PosX != PosX && vehicle->PosY != PosY)
	{
		NrSurvFields /= 2;
	}
	
	//the distances to the OP
	
	float oldDistanceOP = sqrt( (float) (vehicle->PosX - OPX) * (vehicle->PosX - OPX) + (vehicle->PosY - OPY) * (vehicle->PosY - OPY) );
	float newDistanceOP = sqrt( (float) (PosX - OPX) * (PosX - OPX) + (PosY - OPY) * (PosY - OPY) );
	float deltaDistanceOP = oldDistanceOP - newDistanceOP;

	//distances to other surveyors
	//TODO

	//and now the magic formula, which calculates the "importance-factor"
	//TODO: check for factor < FIELD_BLOCKED
	
	float factor = A * NrSurvFields + B * deltaDistanceOP;
	if (NrSurvFields == 0) return FIELD_BLOCKED;

	return  factor;

}
