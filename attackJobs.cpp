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

#include <math.h>
#include "attackJobs.h"
#include "server.h"
#include "client.h"
#include "serverevents.h"
#include "netmessage.h"

int cServerAttackJob::iNextID = 0;

cServerAttackJob::cServerAttackJob( cVehicle* vehicle, int targetOff )
{
	executingClients = new cList<cPlayer*>;
	iID = iNextID;
	iNextID++;
	bMuzzlePlayed = false;
	this->iTargetOff = targetOff;
	this->building = NULL;
	this->vehicle = vehicle;

	iMuzzleType = vehicle->data.muzzle_typ;	
	iAgressorOff = vehicle->PosX + vehicle->PosY*Server->Map->size;

	//do local actions
	vehicle->data.shots--;
	vehicle->data.ammo--;
	if ( !vehicle->data.can_drive_and_fire ) vehicle->data.speed-= (int)(( ( float ) vehicle->data.max_speed ) /vehicle->data.max_shots);
	vehicle->Attacking = true;

	//lock targets
	//TODO: cluster
	lockTarget( targetOff );
	sendFireCommand();
}

cServerAttackJob::cServerAttackJob( cBuilding* building, int targetOff )
{
	executingClients = new cList<cPlayer*>;
	iID = iNextID;
	iNextID++;
	bMuzzlePlayed = false;
	iTargetOff = targetOff;
	this->building = building;
	vehicle = NULL;

	iMuzzleType = building->data.muzzle_typ;
	iAgressorOff = building->PosX + building->PosY*Server->Map->size;
			
	//do local actions
	building->data.shots--;
	building->data.ammo--;
	building->Attacking = true;

	//lock targets
	//TODO: cluster
	lockTarget( targetOff );
	sendFireCommand();
		
}

void cServerAttackJob::lockTarget(int offset)
{
	for ( int i = 0; i < Server->PlayerList->iCount; i++ )
	{
		cPlayer* player = Server->PlayerList->Items[i];
		
		//targed in sight?
		if ( player->ScanMap[offset] == 0 ) continue;
		
		int attackMode = building ? building->data.can_attack : vehicle->data.can_attack;
		if ( attackMode == ATTACK_AIR )
		{
			cVehicle* target = Server->Map->GO[offset].plane;
			if ( target )
			{
				if ( target->mjob ) target->mjob->EndForNow = true;
				target->bIsBeeingAttacked = true;

				cNetMessage* message = new cNetMessage( GAME_EV_ATTACKJOB_LOCK_TARGET );
				message->pushChar ( target->OffX);
				message->pushChar ( target->OffY);
				message->pushInt32( target->iID );
				message->pushInt32( offset );
				message->pushBool ( true ); //bIsAir
				Server->sendNetMessage( message );
				continue;
			}
		}
		if ( attackMode == ATTACK_AIRnLAND || attackMode == ATTACK_LAND || attackMode ==  ATTACK_SUB_LAND )
		{
			cVehicle* targetVehicle = Server->Map->GO[offset].vehicle;
			if ( targetVehicle && targetVehicle->data.is_stealth_sea && attackMode != ATTACK_SUB_LAND )
			{
				targetVehicle = NULL;
			}

			bool targetBuilding = false;
			if ( Server->Map->GO[offset].top )
			{
				Server->Map->GO[offset].top->bIsBeeingAttacked = true;
				targetBuilding = true;
			}
			if ( Server->Map->GO[offset].base )
			{
				Server->Map->GO[offset].base->bIsBeeingAttacked = true;
				targetBuilding = true;
			}
			if ( Server->Map->GO[offset].subbase )
			{
				Server->Map->GO[offset].subbase->bIsBeeingAttacked = true;
				targetBuilding = true;
			}

			if ( !targetVehicle && !targetBuilding ) continue;
			
			cNetMessage* message = new cNetMessage( GAME_EV_ATTACKJOB_LOCK_TARGET );
			if ( targetVehicle )
			{
				if ( targetVehicle->mjob ) targetVehicle->mjob->EndForNow = true;
				targetVehicle->bIsBeeingAttacked = true;

				message->pushChar ( targetVehicle->OffX );
				message->pushChar ( targetVehicle->OffY );
				message->pushInt32 ( targetVehicle->iID );
			}
			else
			{
				message->pushChar ( 0 );
				message->pushChar ( 0 );
				message->pushInt32 ( 0 );
			}
			message->pushInt32( offset );
			message->pushBool ( false ); //bIsAir

			Server->sendNetMessage( message, player->Nr );
		}

	}
}

void cServerAttackJob::sendFireCommand()
{
	//calculate fire direction
	int targetX = iTargetOff % Server->Map->size;
	int targetY = iTargetOff / Server->Map->size;
	int agressorX = iAgressorOff % Server->Map->size;
	int agressorY = iAgressorOff / Server->Map->size;

	float dx = targetX - agressorX;
	float dy =- ( targetY - agressorY );
	float r = sqrt ( dx*dx + dy*dy );

	int fireDir = vehicle?vehicle->dir:building->dir;
	if ( r<=0.001 )
	{
		//do not rotate agressor
	}
	else
	{
		dx /= r;
		dy /= r;
		r = asin ( dx ) *57.29577951;
		if ( dy >= 0 )
		{
			if ( r < 0 ) r += 360;
		}
		else
		{
			r = 180 - r;
		}
		if ( r>=337.5||r<=22.5 ) fireDir=0;
		else if ( r>=22.5&&r<=67.5 ) fireDir=1;
		else if ( r>=67.5&&r<=112.5 ) fireDir=2;
		else if ( r>=112.5&&r<=157.5 ) fireDir=3;
		else if ( r>=157.5&&r<=202.5 ) fireDir=4;
		else if ( r>=202.5&&r<=247.5 ) fireDir=5;
		else if ( r>=247.5&&r<=292.5 ) fireDir=6;
		else if ( r>=292.5&&r<=337.5 ) fireDir=7;
	}
	if ( vehicle )
	{
		vehicle->dir = fireDir;
	}
	else
	{
		building->dir = fireDir;
	}

	bool bMuzzleIsRocketType = ( iMuzzleType == MUZZLE_ROCKET) || ( iMuzzleType == MUZZLE_TORPEDO) || ( iMuzzleType == MUZZLE_ROCKET_CLUSTER );

	for ( int i = 0; i < Server->PlayerList->iCount; i++)
	{
		cPlayer* player = Server->PlayerList->Items[i];
		
		//send message to all player who can see the attacking unit
		if ( player->ScanMap[iAgressorOff] )
		{
			executingClients->Add(player);
			cNetMessage* message = new cNetMessage( GAME_EV_ATTACKJOB_FIRE );
			if ( bMuzzleIsRocketType ) message->pushInt32( iTargetOff );
			message->pushBool( bMuzzleIsRocketType );
			message->pushChar( fireDir );
			message->pushInt32( vehicle ? vehicle->iID : building->iID );
			message->pushInt16( iID );
			Server->sendNetMessage( message, player->Nr );
		}
		//if it is fireing a rocked, send also to players who can see the the target
		//TODO: avoid sending agressor coordinates to players who can't see the attacking unit
		else if ( bMuzzleIsRocketType && player->ScanMap[iTargetOff] )
		{
			executingClients->Add( player );
			cNetMessage* message = new cNetMessage( GAME_EV_ATTACKJOB_FIRE );
			message->pushInt32( iTargetOff );
			message->pushInt32( iAgressorOff );
			message->pushChar ( vehicle?vehicle->data.muzzle_typ:building->data.muzzle_typ );
			message->pushInt32( 0 ); //don't send ID, because agressor is not in sight
			message->pushInt16( iID );
			Server->sendNetMessage( message, player->Nr );
		}
	}
}

void cClientAttackJob::clientLockTarget( cNetMessage* message )
{
	bool bIsAir = message->popBool();
	int offset = message->popInt32();
	int ID = message->popInt32();
	if ( ID != 0 )
	{
		cVehicle* vehicle = Client->getVehicleFromID( ID );
		if ( vehicle == NULL ) return;	//we are out of sync!!!

		vehicle->bIsBeeingAttacked = true;
		if ( vehicle->mjob ) vehicle->mjob->EndForNow = true;
		//TODO: synchronize position
		
	}
	if ( !bIsAir )
	{
		if ( Client->Map->GO[offset].top && Client->Map->GO[offset].top->owner )
		{
			Client->Map->GO[offset].top->bIsBeeingAttacked = true;
		}
		if ( Client->Map->GO[offset].base )
		{
			Client->Map->GO[offset].base->bIsBeeingAttacked = true;
		}
		if ( Client->Map->GO[offset].subbase )
		{
			Client->Map->GO[offset].subbase->bIsBeeingAttacked = true;
		}
	}
}

void cClientAttackJob::handleAttackJobs()
{
	for ( int i = 0; i < Client->attackJobs->iCount; i++ )
	{
		cClientAttackJob* job = Client->attackJobs->Items[i];
		if ( job->bMuzzlePlayed )
		{
			//TODO: attackjob auch im destruktor von cVehicle löschen
			job->sendFinishMessage();
			delete job;
			Client->attackJobs->Delete(i);
		}
		else if ( job->bPlayingMuzzle )
		{
			job->playMuzzle();
		}
		else
		{
			job->rotate();
		}
	}		
}

cClientAttackJob::cClientAttackJob( cNetMessage* message )
{
	bPlayingMuzzle = false;
	bMuzzlePlayed = false;
	this->iID = message->popInt16();
	iTargetOffset = -1;

	//check for duplicate jobs
	for ( int i = 0; i < Client->attackJobs->iCount; i++ )
	{
		if ( Client->attackJobs->Items[i]->iID == this->iID )
		{
			bMuzzlePlayed = true;
			return;
		}
	}

	int unitID = message->popInt32();
	if ( unitID != 0 )	//agressor in sight?
	{
		vehicle = Client->getVehicleFromID( unitID );
		building = Client->getBuildingFromID( unitID );
		if ( !vehicle && !building )
		{
			bMuzzlePlayed = true;
			return; //we are out of sync!!!
		}
		fireDir = message->popChar();
		bool bMuzzleIsRocketType = message->popBool();
		if ( bMuzzleIsRocketType )
		{
			iTargetOffset = message->popInt32();
		}
		iMuzzleType = vehicle ? vehicle->data.muzzle_typ : building->data.muzzle_typ;
		if ( vehicle )
		{
			iAgressorOffset = vehicle->PosX + vehicle->PosY * Client->Map->size;
		}
		else
		{
			iAgressorOffset = building->PosX + vehicle->PosY * Client->Map->size;
		}
	}
	else
	{
		iMuzzleType = message->popChar();
		if ( iMuzzleType != MUZZLE_ROCKET || iMuzzleType != MUZZLE_ROCKET_CLUSTER || iMuzzleType != MUZZLE_TORPEDO )
		{
			bMuzzlePlayed = true;
			return;
		}
		iAgressorOffset = message->popInt32();
		iTargetOffset = message->popInt32();
		vehicle = NULL;
		building = NULL;

	}
}

void cClientAttackJob::rotate()
{
	if ( vehicle )
	{
		if ( vehicle->dir != fireDir )
		{
			vehicle->RotateTo( fireDir );
		}
		else
		{
			bPlayingMuzzle = true;
		}
	}
	else if ( building )
	{
		if ( building->dir != fireDir )
		{
			building->RotateTo( fireDir );
		}
		else
		{
			bPlayingMuzzle = true;
		}
	}
	else
	{
		bPlayingMuzzle = true;
	}
}

void cClientAttackJob::playMuzzle()
{

}

void cClientAttackJob::sendFinishMessage()
{

}
