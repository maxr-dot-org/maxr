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
#include "serverevents.h"
#include "network.h"
#include "events.h"
#include "server.h"
#include "client.h"


void sendAddUnit ( int iPosX, int iPosY, int iID, bool bVehicle, int iUnitNum, int iPlayer, bool bInit )
{
	cNetMessage* message;

	if ( bVehicle ) message = new cNetMessage ( GAME_EV_ADD_VEHICLE );
	else message = new cNetMessage ( GAME_EV_ADD_BUILDING );

	message->pushInt16( iID );
	message->pushInt16( iPosX );
	message->pushInt16( iPosY );
	message->pushInt16( iUnitNum );
	message->pushInt16( iPlayer );
	message->pushBool( bInit );

	Server->sendNetMessage( message, iPlayer );

}

void sendDeleteUnit ( int iID, bool bVehicle, int iClient )
{
	cNetMessage* message;
	if ( bVehicle ) message = new cNetMessage ( GAME_EV_DEL_VEHICLE );
	else message = new cNetMessage ( GAME_EV_DEL_BUILDING );

	message->pushInt16( iID );

	Server->sendNetMessage( message, iClient );

}

void sendAddEnemyUnit ( cVehicle *Vehicle, int iPlayer )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_ADD_ENEM_VEHICLE );

	message->pushInt16( Vehicle->iID );
	message->pushInt16( Vehicle->dir );
	message->pushInt16( Vehicle->PosX );
	message->pushInt16( Vehicle->PosY );
	message->pushInt16( Vehicle->typ->nr );
	message->pushInt16( Vehicle->owner->Nr );

	Server->sendNetMessage( message, iPlayer );
}

void sendAddEnemyUnit ( cBuilding *Building, int iPlayer )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_ADD_ENEM_BUILDING );

	message->pushInt16( Building->iID );
	message->pushInt16( Building->PosX );
	message->pushInt16( Building->PosY );
	message->pushInt16( Building->typ->nr );
	message->pushInt16( Building->owner->Nr );

	Server->sendNetMessage( message, iPlayer );
}

void sendMakeTurnEnd ( bool bEndTurn, bool bWaitForNextPlayer, int iNextPlayerNum, string sReport, int iVoiceNum, int iPlayer )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_MAKE_TURNEND );

	message->pushString ( sReport );
	message->pushInt16 ( iVoiceNum );
	message->pushBool ( bEndTurn );
	message->pushBool ( bWaitForNextPlayer );
	message->pushInt16( iNextPlayerNum );

	Server->sendNetMessage( message, iPlayer );
}

void sendTurnFinished ( int iPlayerNum, int iTimeDelay )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_FINISHED_TURN );

	message->pushInt16 ( iTimeDelay );
	message->pushInt16( iPlayerNum );

	Server->sendNetMessage( message );
}

void sendUnitData( cVehicle *Vehicle, int iPlayer )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_UNIT_DATA );

	// The unit data values
	message->pushInt16( Vehicle->data.max_speed );
	message->pushInt16( Vehicle->data.speed );

	message->pushInt16( Vehicle->data.version );
	message->pushInt16( Vehicle->data.max_hit_points );
	message->pushInt16( Vehicle->data.hit_points );
	message->pushInt16( Vehicle->data.armor );
	message->pushInt16( Vehicle->data.scan );
	message->pushInt16( Vehicle->data.range );
	message->pushInt16( Vehicle->data.max_shots );
	message->pushInt16( Vehicle->data.shots );
	message->pushInt16( Vehicle->data.damage );
	message->pushInt16( Vehicle->data.max_cargo );
	message->pushInt16( Vehicle->data.cargo );
	message->pushInt16( Vehicle->data.max_ammo );
	message->pushInt16( Vehicle->data.ammo );
	message->pushInt16( Vehicle->data.costs );

	// Current state of the unit
	message->pushBool ( Vehicle->Wachposten );
	message->pushInt16 ( Vehicle->BuildRounds );
	message->pushBool ( Vehicle->IsBuilding );
	message->pushBool ( Vehicle->IsClearing );
	message->pushInt16 ( Vehicle->Disabled );
	message->pushString ( Vehicle->name );

	// Data for identifying the unit by the client
	message->pushBool( Vehicle->data.can_drive == DRIVE_AIR );
	message->pushInt16( Vehicle->PosX );
	message->pushInt16( Vehicle->PosY );
	message->pushBool( true );	// true for vehicles
	message->pushInt16 ( Vehicle->iID );
	message->pushInt16( Vehicle->owner->Nr );

	Server->sendNetMessage( message, iPlayer );
}

void sendUnitData ( cBuilding *Building, cMap *Map, int iPlayer )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_UNIT_DATA );

	// The unit data values
	message->pushInt16( Building->data.version );
	message->pushInt16( Building->data.max_hit_points );
	message->pushInt16( Building->data.hit_points );
	message->pushInt16( Building->data.armor );
	message->pushInt16( Building->data.scan );
	message->pushInt16( Building->data.range );
	message->pushInt16( Building->data.max_shots );
	message->pushInt16( Building->data.shots );
	message->pushInt16( Building->data.damage );
	message->pushInt16( Building->data.max_cargo );
	message->pushInt16( Building->data.cargo );
	message->pushInt16( Building->data.max_ammo );
	message->pushInt16( Building->data.ammo );
	message->pushInt16( Building->data.costs );

	// Current state of the unit
	message->pushBool ( Building->Wachposten );
	message->pushBool ( Building->IsWorking );
	message->pushInt16 ( Building->Disabled );
	message->pushString ( Building->name );

	// Data for identifying the unit by the client
	if ( Map->GO[Building->PosX+Building->PosY*Map->size].subbase == Building ) message->pushBool( true );
	else message->pushBool( false );
	if ( Map->GO[Building->PosX+Building->PosY*Map->size].base == Building ) message->pushBool( true );
	else message->pushBool( false );
	message->pushInt16( Building->PosX );
	message->pushInt16( Building->PosY );
	message->pushBool( false );	// false for buildings
	message->pushInt16 ( Building->iID );
	message->pushInt16( Building->owner->Nr );

	Server->sendNetMessage( message, iPlayer );
}

void sendChatMessageToClient( string message, int iType, int iPlayer )
{
	cNetMessage* newMessage;
	newMessage = new cNetMessage( GAME_EV_CHAT_SERVER );
	newMessage->pushString( message );
	newMessage->pushChar( iType );
	Server->sendNetMessage( newMessage, iPlayer );
}

void sendDoStartWork( cBuilding* building )
{
	int offset = building->PosX + building->PosY * Server->Map->size;

	//check all players
	for ( unsigned int i = 0; i < Server->PlayerList->iCount; i++)
	{
		cPlayer* player = Server->PlayerList->Items[i];

		//do not send to players who can't see the building
		if ( !player->ScanMap[offset] && player!=building->owner ) continue;

		cNetMessage* message = new cNetMessage( GAME_EV_DO_START_WORK );

		//send the subbase data only to the owner
		if ( building->owner->Nr == player->Nr )
		{
			sSubBase* subbase = building->SubBase;
			message->pushInt16( subbase->HumanNeed );
			message->pushInt16( subbase->EnergyProd );
			message->pushInt16( subbase->OilNeed );
			message->pushInt16( subbase->EnergyNeed );
			message->pushInt16( subbase->MetalNeed );
			message->pushInt16( subbase->GoldNeed );
			message->pushInt16( subbase->MetalProd );
			message->pushInt16( subbase->OilProd );
			message->pushInt16( subbase->GoldProd );
		}
		message->pushInt32( offset );

		Server->sendNetMessage( message, player->Nr );
	}
}

void sendDoStopWork( cBuilding* building )
{
	int offset = building->PosX + building->PosY * Server->Map->size;

	//check all players
	for ( unsigned int i = 0; i < Server->PlayerList->iCount; i++)
	{
		cPlayer* player = Server->PlayerList->Items[i];

		//do not send to players who can't see the building
		if ( !player->ScanMap[offset] && player!=building->owner ) continue;

		cNetMessage* message = new cNetMessage( GAME_EV_DO_STOP_WORK );

		//send the subbase data only to the owner
		if ( building->owner->Nr == player->Nr )
		{
			sSubBase* subbase = building->SubBase;
			message->pushInt16( subbase->HumanNeed );
			message->pushInt16( subbase->EnergyProd );
			message->pushInt16( subbase->OilNeed );
			message->pushInt16( subbase->EnergyNeed );
			message->pushInt16( subbase->MetalNeed );
			message->pushInt16( subbase->GoldNeed );
			message->pushInt16( subbase->MetalProd );
			message->pushInt16( subbase->OilProd );
			message->pushInt16( subbase->GoldProd );
		}
		message->pushInt32( offset );

		Server->sendNetMessage( message, player->Nr );
	}
}

void sendNextMove( int iUnitID, int iDestOff, int iType, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_NEXT_MOVE );
	message->pushInt16( iType );
	message->pushInt16( iDestOff );
	message->pushInt16( iUnitID );
	Server->sendNetMessage( message, iPlayer );
}

void sendMoveJobServer( cMJobs *MJob, int iPlayer )
{
	bool bEnd = false;
	sWaypoint *LastWaypoint = MJob->waypoints;
	while ( LastWaypoint ) LastWaypoint = LastWaypoint->next;
	sWaypoint *Waypoint;
	while ( !bEnd )
	{
		cNetMessage* message = new cNetMessage( GAME_EV_MOVE_JOB_SERVER );

		int iCount = 0;
		do
		{
			Waypoint = MJob->waypoints;
			while ( Waypoint != LastWaypoint )
			{
				if ( Waypoint->next == LastWaypoint )
				{
					LastWaypoint = Waypoint;
					break;
				}
				Waypoint = Waypoint->next;
			}
			if ( MJob->waypoints == Waypoint ) bEnd = true;

			message->pushInt16( Waypoint->Costs );
			message->pushInt16( Waypoint->X+Waypoint->Y*MJob->map->size);
			iCount++;
		}
		while ( message->iLength <= PACKAGE_LENGHT-19 && !bEnd );

		message->pushInt16( iCount );
		message->pushBool ( MJob->plane );
		message->pushInt16( MJob->DestX+MJob->DestY*MJob->map->size );
		if ( MJob->waypoints->X != MJob->ScrX || MJob->waypoints->Y != MJob->ScrY ) message->pushInt16( MJob->waypoints->X+MJob->waypoints->Y*MJob->map->size );
		else message->pushInt16( MJob->ScrX+MJob->ScrY*MJob->map->size );
		message->pushInt16( MJob->vehicle->iID );

		Server->sendNetMessage( message, iPlayer );
	}
}

void sendResources( cVehicle *Vehicle, cMap *Map )
{
	int iCount = 0;
	cNetMessage* message = new cNetMessage( GAME_EV_RESOURCES );

	// only send new scaned resources
	for ( int iX = Vehicle->PosX-1, iY = Vehicle->PosY-1; iY <= Vehicle->PosY+1; iX++ )
	{
		if ( iX > Vehicle->PosX+1 )
		{
			iX = Vehicle->PosX-1;
			iY++;
		}

		if ( iY > Vehicle->PosY+1 ) break;
		if ( iX < 0 || iX >= Map->size || iY < 0 || iY >= Map->size ) continue;
		if ( Vehicle->owner->ResourceMap[iX+iY*Map->size] != 0 ) continue;

		message->pushInt16( Map->Resources[iX+iY*Map->size].value );
		message->pushInt16( Map->Resources[iX+iY*Map->size].typ );
		message->pushInt32( iX+iY*Map->size );
		iCount++;
	}
	message->pushInt16( iCount );

	Server->sendNetMessage( message, Vehicle->owner->Nr );
}

void sendBuildAnswer( bool bOK, int iVehicleID, int iOff, int iBuildingType, int iBuildRounds, int iBuildCosts, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_BUILD_ANSWER );
	message->pushInt16( iBuildCosts );
	message->pushInt16( iBuildRounds );
	message->pushInt16( iBuildingType );
	message->pushInt32( iOff );
	message->pushInt16( iVehicleID );
	message->pushBool ( bOK );
	Server->sendNetMessage( message, iPlayer );
}

void sendNewSubbase ( sSubBase *SubBase, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_NEW_SUBBASE );
	message->pushInt16 ( SubBase->iID );
	Server->sendNetMessage( message, iPlayer );
}

void sendDeleteSubbase ( sSubBase *SubBase, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_DELETE_SUBBASE );
	message->pushInt16 ( SubBase->iID );
	Server->sendNetMessage( message, iPlayer );
}

void sendAddSubbaseBuildings ( cBuilding *Building, sSubBase *SubBase, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_SUBBASE_BUILDINGS );
	int iCount = 0;
	if ( Building == NULL )
	{
		for ( unsigned int i = 0; i < SubBase->buildings.iCount; i++ )
		{
			if ( message->iLength > PACKAGE_LENGHT-16 )
			{
				message->pushInt16 ( iCount );
				message->pushInt16 ( SubBase->iID );
				Server->sendNetMessage( message, iPlayer );
				iCount = 0;
				message = new cNetMessage( GAME_EV_SUBBASE_BUILDINGS );
			}
			message->pushInt16 ( SubBase->buildings.Items[i]->iID );
			iCount++;
		}
	}
	else
	{
		message->pushInt16 ( Building->iID );
		iCount++;
	}
	message->pushInt16 ( iCount );
	message->pushInt16 ( SubBase->iID );
	Server->sendNetMessage( message, iPlayer );
}

void sendSubbaseValues ( sSubBase *SubBase, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_SUBBASE_VALUES );

	message->pushInt16 ( SubBase->EnergyProd );
	message->pushInt16 ( SubBase->EnergyNeed );
	message->pushInt16 ( SubBase->MaxEnergyProd );
	message->pushInt16 ( SubBase->MaxEnergyNeed );
	message->pushInt16 ( SubBase->Metal );
	message->pushInt16 ( SubBase->MaxMetal );
	message->pushInt16 ( SubBase->MetalNeed );
	message->pushInt16 ( SubBase->MaxMetalNeed );
	message->pushInt16 ( SubBase->MetalProd );
	message->pushInt16 ( SubBase->Gold );
	message->pushInt16 ( SubBase->MaxGold );
	message->pushInt16 ( SubBase->GoldNeed );
	message->pushInt16 ( SubBase->MaxGoldNeed );
	message->pushInt16 ( SubBase->GoldProd );
	message->pushInt16 ( SubBase->Oil );
	message->pushInt16 ( SubBase->MaxOil );
	message->pushInt16 ( SubBase->OilNeed );
	message->pushInt16 ( SubBase->MaxOilNeed );
	message->pushInt16 ( SubBase->OilProd );
	message->pushInt16 ( SubBase->HumanNeed );
	message->pushInt16 ( SubBase->MaxHumanNeed );
	message->pushInt16 ( SubBase->HumanProd );
	message->pushInt16 ( SubBase->iID );
	Server->sendNetMessage( message, iPlayer );
}