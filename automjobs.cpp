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
	if (vehicle->mjob == NULL || vehicle->mjob->finished )
	{
		
		if (n > WAIT_FRAMES)
		{
			//think about the next move:
			//the AI look at all fields next to the surveyor
			//and calculates an factor for each field
			//the surveyor will move to the field with the highest value

			float temp_factor, max_factor = 0;
			int bestX, bestY;
			int x, y;

			for ( x = vehicle->PosX - 1; x <= vehicle->PosX + 1; x ++)
			{	
				for (y = vehicle->PosY - 1; y <= vehicle->PosY + 1; y++)
				{
					if ( x == vehicle->PosX && y == vehicle->PosY ) continue;

					temp_factor = CalcFactor( x, y);

					if ( temp_factor > max_factor)
					{
						max_factor = temp_factor;
						bestX = x;
						bestY = y;
					}
				}
			}
			
			//todo: don't know yet, what to do, if the path cannot be found
			engine->AddMoveJob(vehicle->PosX + vehicle->PosY * engine->map->size, bestX + bestY * engine->map->size, false, false);
			
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

//calculates an "importance-factor" for a given field
float cAutoMJob::CalcFactor(int x, int y)
{
	//test:
	return random (10,0);

}
