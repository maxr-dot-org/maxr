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
cAutoMJob *cAutoMJob::autoMJobs[100];
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
	 autoMJobs[iCount] = this;
	 iNumber = iCount;
	 iCount++;
	 this->vehicle = vehicle;
	 n = 0;
}

//destruktor for cAutoMJob
cAutoMJob::~cAutoMJob()
{
	int i;
	for (i = iNumber; i < iCount - 1; i++)
	{
		autoMJobs[i] = autoMJobs[i + 1];
	}
	iCount--;
}

//performs the auto move of a vehicle and adds new mjobs to the engine, if nessesary
void cAutoMJob::DoAutoMove()
{
	if (vehicle->mjob == NULL )
	{
		//hier is the right place for the AI to think about the next move
		if (this->n > 10)
		{
			//very stupid testing code...
			int direktion = random(4,0);
			switch (direktion){
			case 0: engine->AddMoveJob( vehicle->PosX + vehicle->PosY*engine->map->size, vehicle->PosX + 1 + vehicle->PosY*engine->map->size, false, false);
				break;
			case 1: engine->AddMoveJob( vehicle->PosX + vehicle->PosY*engine->map->size, vehicle->PosX - 1 + vehicle->PosY*engine->map->size, false, false);
				break;
			case 2: engine->AddMoveJob( vehicle->PosX + vehicle->PosY*engine->map->size, vehicle->PosX + (vehicle->PosY + 1) * engine->map->size, false, false);
				break;
			case 3: engine->AddMoveJob( vehicle->PosX + vehicle->PosY*engine->map->size, vehicle->PosX + (vehicle->PosY - 1) * engine->map->size, false, false);
				break;
			}
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
