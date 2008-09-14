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
	iID = iNextID;
	iNextID++;
	bMuzzlePlayed = false;
	this->iTargetOff = targetOff;
	building = NULL;
	this->vehicle = vehicle;
	damage = vehicle->data.damage;
	attackMode = vehicle->data.can_attack;

	iMuzzleType = vehicle->data.muzzle_typ;
	iAgressorOff = vehicle->PosX + vehicle->PosY*Server->Map->size;

	//lock targets
	//TODO: cluster
	lockTarget( targetOff );
	sendFireCommand();

	//do local actions
	vehicle->data.shots--;
	vehicle->data.ammo--;
	if ( !vehicle->data.can_drive_and_fire ) vehicle->data.speed-= (int)(( ( float ) vehicle->data.max_speed ) /vehicle->data.max_shots);
	vehicle->Attacking = true;

}

cServerAttackJob::cServerAttackJob( cBuilding* building, int targetOff )
{
	iID = iNextID;
	iNextID++;
	bMuzzlePlayed = false;
	iTargetOff = targetOff;
	this->building = building;
	vehicle = NULL;
	damage = building->data.damage;
	attackMode = building->data.can_attack;


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

	if ( building->data.is_expl_mine )
	{
		Server->deleteUnit( building, false );
		this->building = NULL;
	}
}

cServerAttackJob::~cServerAttackJob()
{
	if ( building ) building->Attacking = false;
	if ( vehicle  ) vehicle->Attacking = false;
}

void cServerAttackJob::lockTarget(int offset)
{
	//make sure, that the unit data has been send to all clients
	Server->checkPlayerUnits();

	for (int i = 0; i < Server->PlayerList->Size(); i++)
	{
		cPlayer* player = (*Server->PlayerList)[i];

		//targed in sight?
		if ( player->ScanMap[offset] == 0 ) continue;

		if ( attackMode == ATTACK_AIR )
		{
			cVehicle* target = Server->Map->GO[offset].plane;
			if ( target )
			{
				target->bIsBeeingAttacked = true;

				cNetMessage* message = new cNetMessage( GAME_EV_ATTACKJOB_LOCK_TARGET );
				message->pushChar ( target->OffX);
				message->pushChar ( target->OffY);
				message->pushInt32( target->iID );
				message->pushInt32( offset );
				message->pushBool ( true ); //bIsAir
				Server->sendNetMessage( message, player->Nr );
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
	//make the agressor visible on all clients who can see the agressor offset
	for ( int i = 0; i < Server->PlayerList->Size(); i++ )
	{
		cPlayer* player = (*Server->PlayerList)[i];
		if ( !player->ScanMap[iAgressorOff] ) continue;
		
		if ( vehicle )
		{
			if ( vehicle->owner == player ) continue;

			vehicle->setDetectedByPlayer( player );
		}
		else
		{
			if ( building->owner == player ) continue;
			
			building->setDetectedByPlayer( player );
		}
	}
	Server->checkPlayerUnits();

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

	for (int i = 0; i < Server->PlayerList->Size(); i++)
	{
		cPlayer* player = (*Server->PlayerList)[i];

		//send message to all player who can see the attacking unit
		if ( player->ScanMap[iAgressorOff] )
		{
			executingClients.Add(player);
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
			executingClients.Add(player);
			cNetMessage* message = new cNetMessage( GAME_EV_ATTACKJOB_FIRE );
			message->pushChar( fireDir );
			message->pushInt32( iTargetOff );
			message->pushInt32( iAgressorOff );
			message->pushChar ( vehicle?vehicle->data.muzzle_typ:building->data.muzzle_typ );
			message->pushInt32( 0 ); //don't send ID, because agressor is not in sight
			message->pushInt16( iID );
			
			Server->sendNetMessage( message, player->Nr );
		}
	}
}

void cServerAttackJob::clientFinished( int playerNr )
{
	for (int i = 0; i < executingClients.Size(); i++)
	{
		if ( executingClients[i]->Nr == playerNr ) executingClients.Delete(i);
	}

	cLog::write( " Server: waiting for " + iToStr(executingClients.Size()) + " clients", cLog::eLOG_TYPE_NET_DEBUG ); 

	if (executingClients.Size() == 0) makeImpact();
}


void cServerAttackJob::makeImpact()
{

	cVehicle* targetVehicle = NULL;
	cBuilding* targetBuilding = NULL;

	//select target
	if ( attackMode == ATTACK_AIRnLAND )
	{
		targetVehicle = Server->Map->GO[iTargetOff].vehicle;
		targetBuilding = Server->Map->GO[iTargetOff].top;

		if ( !targetBuilding && !targetVehicle )
			targetVehicle = Server->Map->GO[iTargetOff].plane;

		if ( !targetBuilding && !targetVehicle )
			targetBuilding = Server->Map->GO[iTargetOff].base;

		if ( !targetBuilding && !targetVehicle )
			targetBuilding = Server->Map->GO[iTargetOff].subbase;

	}
	else if ( attackMode == ATTACK_LAND )
	{
		targetVehicle = Server->Map->GO[iTargetOff].vehicle;
		targetBuilding = Server->Map->GO[iTargetOff].top;

		if ( !targetBuilding && !targetVehicle )
			targetBuilding = Server->Map->GO[iTargetOff].base;

		if ( !targetBuilding && !targetVehicle )
			targetBuilding = Server->Map->GO[iTargetOff].subbase;

	}
	else if ( attackMode == ATTACK_AIR )
	{
		targetVehicle = Server->Map->GO[iTargetOff].plane;
	}
	else if ( attackMode == ATTACK_SUB_LAND )
	{
		targetVehicle = Server->Map->GO[iTargetOff].vehicle;
		targetBuilding = Server->Map->GO[iTargetOff].top;

		if ( !targetBuilding && !targetVehicle )
			targetBuilding = Server->Map->GO[iTargetOff].base;

		if ( !targetBuilding && !targetVehicle )
			targetBuilding = Server->Map->GO[iTargetOff].subbase;

	}

	//check for rubble
	if ( targetBuilding && !targetBuilding->owner )
		targetBuilding = NULL;

	//if target found
	if ( targetBuilding || targetVehicle )
	{
		if ( targetVehicle )
		{
			targetVehicle->data.hit_points = targetVehicle->CalcHelth( damage );

			cLog::write(" Server: vehicle '" + targetVehicle->name + "' (ID: " + iToStr(targetVehicle->iID) + ") hit. Remaining HP: " + iToStr(targetVehicle->data.hit_points), cLog::eLOG_TYPE_NET_DEBUG );

			if (targetVehicle->data.hit_points <= 0)
			{
				Server->destroyUnit( targetVehicle );
			}
			else
			{
				targetVehicle->InSentryRange();
			}
		}
		else
		{
			targetBuilding->data.hit_points = targetBuilding->CalcHelth( damage );

			cLog::write(" Server: Building '" + targetBuilding->name + "' (ID: " + iToStr(targetBuilding->iID) + ") hit. Remaining HP: " + iToStr(targetBuilding->data.hit_points), cLog::eLOG_TYPE_NET_DEBUG );

			if ( targetBuilding->data.hit_points <= 0 )
			{
				Server->destroyUnit( targetBuilding );
			}
		}
	}

	//Todo: cluster
	sendAttackJobImpact( iTargetOff, damage, attackMode );


	//TODO: unlocking here interferes with following attack job by sentry mode.
	//clean up
	if ( attackMode == ATTACK_AIR )
	{
		cVehicle* target = Server->Map->GO[iTargetOff].plane;
		if ( target )
		{
			target->bIsBeeingAttacked = false;
		}
	}
	if ( attackMode == ATTACK_AIRnLAND || attackMode == ATTACK_LAND || attackMode ==  ATTACK_SUB_LAND )
	{
		cVehicle* targetVehicle = Server->Map->GO[iTargetOff].vehicle;
		if ( targetVehicle && targetVehicle->data.is_stealth_sea && attackMode != ATTACK_SUB_LAND )
		{
			targetVehicle = NULL;
		}


		if ( Server->Map->GO[iTargetOff].top )
		{
			Server->Map->GO[iTargetOff].top->bIsBeeingAttacked = false;
		}
		if ( Server->Map->GO[iTargetOff].base )
		{
			Server->Map->GO[iTargetOff].base->bIsBeeingAttacked = false;
		}
		if ( Server->Map->GO[iTargetOff].subbase )
		{
			Server->Map->GO[iTargetOff].subbase->bIsBeeingAttacked = false;
		}

		if ( targetVehicle )
		{
			targetVehicle->bIsBeeingAttacked = false;
		}
	}
}

void cServerAttackJob::sendAttackJobImpact(int offset, int damage, int attackMode )
{
	for (int i = 0; i < Server->PlayerList->Size(); i++)
	{
		cPlayer* player = (*Server->PlayerList)[i];

		//targed in sight?
		if ( player->ScanMap[offset] == 0 ) continue;

		cNetMessage* message = new cNetMessage( GAME_EV_ATTACKJOB_IMPACT );
		message->pushInt32( offset );
		message->pushInt16( damage );
		message->pushInt16( attackMode );

		Server->sendNetMessage( message, player->Nr );

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
		if ( vehicle == NULL ) 
		{
			cLog::write(" Client: vehicle with ID " + iToStr(ID) + " not found", cLog::eLOG_TYPE_NET_ERROR );
			return;	//we are out of sync!!!
		}

		vehicle->bIsBeeingAttacked = true;
		
		//synchonize position
		int OffY = message->popChar();
		int OffX = message->popChar();
		if ( vehicle->PosX + vehicle->PosY * Client->Map->size != offset )
		{
			cLog::write(" Client: changed vehicle position to " + iToStr( offset ), cLog::eLOG_TYPE_NET_DEBUG );
			if ( bIsAir )
			{
				Client->Map->GO[vehicle->PosX + vehicle->PosY * Client->Map->size].plane = NULL;
				Client->Map->GO[offset].plane = vehicle;
				vehicle->PosX = offset % Client->Map->size;
				vehicle->PosY = offset / Client->Map->size;
				vehicle->OffX = OffX;
				vehicle->OffY = OffY;
			}
			else
			{
				Client->Map->GO[vehicle->PosX + vehicle->PosY * Client->Map->size].vehicle = NULL;
				Client->Map->GO[offset].vehicle = vehicle;
				vehicle->PosX = offset % Client->Map->size;
				vehicle->PosY = offset / Client->Map->size;
				vehicle->OffX = OffX;
				vehicle->OffY = OffY;
			}
		}
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
	for (int i = 0; i < Client->attackJobs.Size(); i++)
	{
		cClientAttackJob* job = Client->attackJobs[i];
		switch ( job->state )
		{
		case FINISHED:
			{
				job->sendFinishMessage();
				delete job;
				Client->attackJobs.Delete(i);
				break;
			}
		case UPDATE_AGRESSOR_DATA:
			{
				job->updateAgressorData();
				job->state = PLAYING_MUZZLE;
				job->playMuzzle();
				break;
			}
		case PLAYING_MUZZLE:
			{
				job->playMuzzle();
				break;
			}
		case ROTATING:
			{
				job->rotate();
				break;
			}

		}
	}
}

cClientAttackJob::cClientAttackJob( cNetMessage* message )
{
	state = ROTATING;
	wait = 0;
	this->iID = message->popInt16();
	iTargetOffset = -1;

	//check for duplicate jobs
	for (int i = 0; i < Client->attackJobs.Size(); i++)
	{
		if ( Client->attackJobs[i]->iID == this->iID )
		{
			state = FINISHED;
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
			state = FINISHED;
			cLog::write(" Client: agressor with id " + iToStr( unitID ) + " not found", cLog::eLOG_TYPE_NET_ERROR );
			return; //we are out of sync!!!
		}
		iFireDir = message->popChar();
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
			iAgressorOffset = building->PosX + building->PosY * Client->Map->size;
		}
	}
	else
	{
		iMuzzleType = message->popChar();
		if ( iMuzzleType != MUZZLE_ROCKET && iMuzzleType != MUZZLE_ROCKET_CLUSTER && iMuzzleType != MUZZLE_TORPEDO )
		{
			state = FINISHED;
			return;
		}
		iAgressorOffset = message->popInt32();
		iTargetOffset = message->popInt32();
		iFireDir = message->popChar();
		vehicle = NULL;
		building = NULL;

	}
}

void cClientAttackJob::rotate()
{
	if ( vehicle )
	{
		if ( vehicle->dir != iFireDir )
		{
			vehicle->rotating = true;
			vehicle->RotateTo( iFireDir );
		}
		else
		{
			vehicle->rotating = false;
			state = UPDATE_AGRESSOR_DATA;
		}
	}
	else if ( building )
	{
		if ( building->dir != iFireDir && !building->data.is_expl_mine )
		{
			building->RotateTo( iFireDir );
		}
		else
		{
			state = UPDATE_AGRESSOR_DATA;
		}
	}
	else
	{
		state = UPDATE_AGRESSOR_DATA;
	}
}

void cClientAttackJob::playMuzzle()
{

	int offx=0,offy=0;

	if ( building && building->data.is_expl_mine )
	{
		state = FINISHED;
		PlayFX ( building->typ->Attack );
		if ( Client->Map->IsWater( building->PosX + building->PosY * Client->Map->size ) )
		{
			Client->addFX( fxExploWater, building->PosX*64 + 32, building->PosY*64 + 32, 0);
		}
		else
		{
			Client->addFX( fxExploSmall, building->PosX*64 + 32, building->PosY*64 + 32, 0);
		}
		if ( building ) Client->deleteUnit( building );
		return;
	}

	switch ( iMuzzleType )
	{
		case MUZZLE_BIG:
			if ( wait++!=0 )
			{
				if ( wait>2 ) state = FINISHED;
				return;
			}
			switch ( iFireDir )
			{
				case 0:
					offy-=40;
					break;
				case 1:
					offx+=32;
					offy-=32;
					break;
				case 2:
					offx+=40;
					break;
				case 3:
					offx+=32;
					offy+=32;
					break;
				case 4:
					offy+=40;
					break;
				case 5:
					offx-=32;
					offy+=32;
					break;
				case 6:
					offx-=40;
					break;
				case 7:
					offx-=32;
					offy-=32;
					break;
			}
			if ( vehicle )
			{
				Client->addFX ( fxMuzzleBig,vehicle->PosX*64+offx,vehicle->PosY*64+offy,iFireDir );
			}
			else
			{
				Client->addFX ( fxMuzzleBig,building->PosX*64+offx,building->PosY*64+offy,iFireDir );
			}
			break;
		case MUZZLE_SMALL:
			if ( wait++!=0 )
			{
				if ( wait>2 ) state = FINISHED;
				return;
			}
			if ( vehicle )
			{
				Client->addFX ( fxMuzzleSmall,vehicle->PosX*64,vehicle->PosY*64,iFireDir );
			}
			else
			{
				Client->addFX ( fxMuzzleSmall,building->PosX*64,building->PosY*64,iFireDir );
			}
			break;
		case MUZZLE_ROCKET:
		case MUZZLE_ROCKET_CLUSTER:
		{
			if ( wait++!=0 ) return;
			
			int PosX = iAgressorOffset%Client->Map->size;
			int PosY = iAgressorOffset/Client->Map->size;
			Client->addFX ( fxRocket, PosX*64, PosY*64, this, iTargetOffset, iFireDir );
			
			break;
		}
		case MUZZLE_MED:
		case MUZZLE_MED_LONG:
			if ( wait++!=0 )
			{
				if ( wait>2 ) state = FINISHED;
				return;
			}
			switch ( iFireDir )
			{
				case 0:
					offy-=20;
					break;
				case 1:
					offx+=12;
					offy-=12;
					break;
				case 2:
					offx+=20;
					break;
				case 3:
					offx+=12;
					offy+=12;
					break;
				case 4:
					offy+=20;
					break;
				case 5:
					offx-=12;
					offy+=12;
					break;
				case 6:
					offx-=20;
					break;
				case 7:
					offx-=12;
					offy-=12;
					break;
			}
			if ( iMuzzleType == MUZZLE_MED )
			{
				if ( vehicle )
				{
					Client->addFX ( fxMuzzleMed,vehicle->PosX*64+offx,vehicle->PosY*64+offy,iFireDir );
				}
				else
				{
					Client->addFX ( fxMuzzleMed,building->PosX*64+offx,building->PosY*64+offy,iFireDir );
				}
			}
			else
			{
				if ( vehicle )
				{
					Client->addFX ( fxMuzzleMedLong,vehicle->PosX*64+offx,vehicle->PosY*64+offy,iFireDir );
				}
				else
				{
					Client->addFX ( fxMuzzleMedLong,building->PosX*64+offx,building->PosY*64+offy,iFireDir );
				}
			}
			break;
		case MUZZLE_TORPEDO:
		{
			if ( wait++!=0 ) return;

			int PosX = iAgressorOffset%Client->Map->size;
			int PosY = iAgressorOffset/Client->Map->size;
			Client->addFX ( fxTorpedo, PosX*64, PosY*64, this, iTargetOffset, iFireDir );

			break;
		}
		case MUZZLE_SNIPER:
			state = FINISHED;
			break;
	}
	if ( vehicle )
	{
		PlayFX ( vehicle->typ->Attack );
	}
	else if ( building )
	{
		PlayFX ( building->typ->Attack );
	}

}

void cClientAttackJob::sendFinishMessage()
{
	cNetMessage* message = new cNetMessage( GAME_EV_ATTACKJOB_FINISHED );
	message->pushInt16( iID );
	Client->sendNetMessage( message );
}

void cClientAttackJob::updateAgressorData()
{
	if ( vehicle )
	{
		vehicle->data.shots--;
		vehicle->data.ammo--;
		if ( !vehicle->data.shots ) vehicle->AttackMode = false;
		if ( !vehicle->data.can_drive_and_fire ) vehicle->data.speed -= (int)(( ( float ) vehicle->data.max_speed ) /vehicle->data.max_shots);
		if ( Client->SelectedVehicle && Client->SelectedVehicle == vehicle )
		{
			vehicle->ShowDetails();
		}
	}
	else if ( building )
	{
		building->data.shots--;
		building->data.ammo--;
		if ( !building->data.shots ) building->AttackMode = false;
		if ( Client->SelectedBuilding && Client->SelectedBuilding == building )
		{
			building->ShowDetails();
		}
	}
}

void cClientAttackJob::makeImpact(int offset, int damage, int attackMode )
{
	if ( offset < 0 || offset > Client->Map->size * Client->Map->size )
	{
		cLog::write(" Client: Invalid offset", cLog::eLOG_TYPE_NET_ERROR );
		return;
	}

	cVehicle* targetVehicle = NULL;
	cBuilding* targetBuilding = NULL;

	if ( attackMode == ATTACK_AIRnLAND )
	{
		targetVehicle = Client->Map->GO[offset].vehicle;
		targetBuilding = Client->Map->GO[offset].top;

		if ( !targetBuilding && !targetVehicle )
			targetVehicle = Client->Map->GO[offset].plane;

		if ( !targetBuilding && !targetVehicle )
			targetBuilding = Client->Map->GO[offset].base;

		if ( !targetBuilding && !targetVehicle )
			targetBuilding = Client->Map->GO[offset].subbase;

	}
	else if ( attackMode == ATTACK_LAND )
	{
		targetVehicle = Client->Map->GO[offset].vehicle;
		targetBuilding = Client->Map->GO[offset].top;

		if ( !targetBuilding && !targetVehicle )
			targetBuilding = Client->Map->GO[offset].base;

		if ( !targetBuilding && !targetVehicle )
			targetBuilding = Client->Map->GO[offset].subbase;

	}
	else if ( attackMode == ATTACK_AIR )
	{
		targetVehicle = Client->Map->GO[offset].plane;
	}
	else if ( attackMode == ATTACK_SUB_LAND )
	{
		targetVehicle = Client->Map->GO[offset].vehicle;
		targetBuilding = Client->Map->GO[offset].top;

		if ( !targetBuilding && !targetVehicle )
			targetBuilding = Client->Map->GO[offset].base;

		if ( !targetBuilding && !targetVehicle )
			targetBuilding = Client->Map->GO[offset].subbase;
	}
	else
	{
		cLog::write(" Client: Invalid attack mode", cLog::eLOG_TYPE_NET_ERROR );
		return;
	}

	//check for rubble
	if ( targetBuilding && !targetBuilding->owner )
		targetBuilding = NULL;

	bool playImpact = false;
	bool ownUnit = false;
	bool destroyed = false;
	string name;
	int offX = 0, offY = 0;

	//no target found
	if ( !targetBuilding && !targetVehicle )
	{
		playImpact = true;
	}
	else
	{

		if ( targetVehicle )
		{
			targetVehicle->data.hit_points = targetVehicle->CalcHelth( damage );

			cLog::write(" Client: vehicle '" + targetVehicle->name + "' (ID: " + iToStr(targetVehicle->iID) + ") hit. Remaining HP: " + iToStr(targetVehicle->data.hit_points), cLog::eLOG_TYPE_NET_DEBUG );

			name = targetVehicle->name;
			if ( targetVehicle->owner == Client->ActivePlayer ) ownUnit = true;

			if (targetVehicle->data.hit_points <= 0)
			{
				Client->destroyUnit( targetVehicle );
				destroyed = true;
			}
			else
			{
				playImpact = true;
				offX = targetVehicle->OffX;
				offY = targetVehicle->OffY;
				if ( Client->SelectedVehicle == targetVehicle ) targetVehicle->ShowDetails();
				Client->mouseMoveCallback( true );
			}
		}
		else
		{
			targetBuilding->data.hit_points = targetBuilding->CalcHelth( damage );

			cLog::write(" Client: building '" + targetBuilding->name + "' (ID: " + iToStr(targetBuilding->iID) + ") hit. Remaining HP: " + iToStr(targetBuilding->data.hit_points), cLog::eLOG_TYPE_NET_DEBUG );

			name = targetBuilding->name;
			if ( targetBuilding->owner == Client->ActivePlayer ) ownUnit = true;

			if ( targetBuilding->data.hit_points <= 0 )
			{
				Client->destroyUnit( targetBuilding );
				destroyed = true;
			}
			else
			{
				playImpact = true;
				if ( Client->SelectedBuilding == targetBuilding ) targetBuilding->ShowDetails();
				Client->mouseMoveCallback( true );
			}
		}
	}

	int x = offset % Client->Map->size;
	int y = offset / Client->Map->size;

	if ( playImpact && SettingsData.bAlphaEffects )
	{
		Client->addFX( fxHit, x*64 + offX, y*64 + offY, 0);
	}

	string message;
	if ( ownUnit )
	{
		if ( destroyed )
		{
			message = name + " " + lngPack.i18n("Text~Comp~Destroyed");
			Client->addCoords( message, x, y );
			PlayVoice( VoiceData.VOIDestroyedUs );
		}
		else
		{
			message = name + " " + lngPack.i18n("Text~Comp~Attacked");
			Client->addCoords( message, x, y );
			PlayVoice( VoiceData.VOIAttackingUs );
		}
	}

	//clean up
	if ( attackMode == ATTACK_AIR )
	{
		cVehicle* target = Client->Map->GO[offset].plane;
		if ( target )
		{
			if ( target->ClientMoveJob ) target->ClientMoveJob->bEndForNow = false;
			target->bIsBeeingAttacked = false;
		}
	}
	if ( attackMode == ATTACK_AIRnLAND || attackMode == ATTACK_LAND || attackMode ==  ATTACK_SUB_LAND )
	{
		cVehicle* targetVehicle = Client->Map->GO[offset].vehicle;
		if ( targetVehicle && targetVehicle->data.is_stealth_sea && attackMode != ATTACK_SUB_LAND )
		{
			targetVehicle = NULL;
		}

		if ( Client->Map->GO[offset].top )
		{
			Client->Map->GO[offset].top->bIsBeeingAttacked = false;
		}
		if ( Client->Map->GO[offset].base )
		{
			Client->Map->GO[offset].base->bIsBeeingAttacked = false;
		}
		if ( Client->Map->GO[offset].subbase )
		{
			Client->Map->GO[offset].subbase->bIsBeeingAttacked = false;
		}

		if ( targetVehicle )
		{
			if ( targetVehicle->ClientMoveJob ) targetVehicle->ClientMoveJob->bEndForNow = false;
			targetVehicle->bIsBeeingAttacked = false;
		}

	}
}
