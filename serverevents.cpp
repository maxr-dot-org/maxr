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
#include "menu.h"
#include "movejobs.h"


void sendAddUnit ( int iPosX, int iPosY, int iID, bool bVehicle, sID UnitID, int iPlayer, bool bInit, bool bAddToMap )
{
	cNetMessage* message;

	if ( bVehicle ) message = new cNetMessage ( GAME_EV_ADD_VEHICLE );
	else message = new cNetMessage ( GAME_EV_ADD_BUILDING );

	message->pushBool( bAddToMap );
	message->pushInt16( iID );
	message->pushInt16( iPosX );
	message->pushInt16( iPosY );
	message->pushInt16( UnitID.iSecondPart );
	message->pushInt16( UnitID.iFirstPart );
	message->pushInt16( iPlayer );
	message->pushBool( bInit );

	Server->sendNetMessage( message, iPlayer );

}

void sendAddRubble( cBuilding* building, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_ADD_RUBBLE );

	message->pushInt16( building->PosX );
	message->pushInt16( building->PosY );
	message->pushInt16( building->iID );
	message->pushInt16( building->RubbleValue );
	message->pushInt16( building->RubbleTyp );
	message->pushBool( building->data.is_big );

	Server->sendNetMessage( message, iPlayer );
}

void sendDeleteUnit ( cBuilding* building, int iClient )
{
	cNetMessage* message;
	if ( iClient == -1 )
	{
		for ( unsigned int i = 0; i < building->SeenByPlayerList.Size(); i++)
		{
			message = new cNetMessage ( GAME_EV_DEL_BUILDING );
			message->pushInt16( building->iID );
			Server->sendNetMessage(message, building->SeenByPlayerList[i]->Nr );
		}

		//send message to owner, since he is not in the SeenByPlayerList
		if ( building->owner )
		{
			message = new cNetMessage ( GAME_EV_DEL_BUILDING );
			message->pushInt16( building->iID );
			Server->sendNetMessage(message, building->owner->Nr);
		}
	}
	else
	{
		message = new cNetMessage ( GAME_EV_DEL_BUILDING );
		message->pushInt16( building->iID );
		Server->sendNetMessage( message, iClient );
	}
}

void sendDeleteUnit ( cVehicle* vehicle, int iClient )
{
	cNetMessage* message;
	if ( iClient == -1 )
	{
		for ( unsigned int i = 0; i < vehicle->SeenByPlayerList.Size(); i++)
		{
			message = new cNetMessage ( GAME_EV_DEL_VEHICLE );
			message->pushInt16( vehicle->iID );
			Server->sendNetMessage( message, vehicle->SeenByPlayerList[i]->Nr );
		}
		message = new cNetMessage ( GAME_EV_DEL_VEHICLE );
		message->pushInt16( vehicle->iID );
		Server->sendNetMessage( message, vehicle->owner->Nr );
	}
	else
	{
		message = new cNetMessage ( GAME_EV_DEL_VEHICLE );
		message->pushInt16( vehicle->iID );
		Server->sendNetMessage( message, iClient );
	}
}

void sendAddEnemyUnit ( cVehicle *Vehicle, int iPlayer )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_ADD_ENEM_VEHICLE );

	message->pushInt16( Vehicle->iID );
	message->pushInt16( Vehicle->dir );
	message->pushInt16( Vehicle->PosX );
	message->pushInt16( Vehicle->PosY );
	message->pushInt16( Vehicle->data.ID.iSecondPart );
	message->pushInt16( Vehicle->data.ID.iFirstPart );
	message->pushInt16( Vehicle->owner->Nr );

	Server->sendNetMessage( message, iPlayer );
}

void sendAddEnemyUnit ( cBuilding *Building, int iPlayer )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_ADD_ENEM_BUILDING );

	message->pushInt16( Building->iID );
	message->pushInt16( Building->PosX );
	message->pushInt16( Building->PosY );
	message->pushInt16( Building->data.ID.iSecondPart );
	message->pushInt16( Building->data.ID.iFirstPart );
	message->pushInt16( Building->owner->Nr );

	Server->sendNetMessage( message, iPlayer );
}

void sendMakeTurnEnd ( bool bEndTurn, bool bWaitForNextPlayer, int iNextPlayerNum, int iPlayer )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_MAKE_TURNEND );

	message->pushBool ( bEndTurn );
	message->pushBool ( bWaitForNextPlayer );
	message->pushInt16( iNextPlayerNum );

	Server->sendNetMessage( message, iPlayer );
}

void sendTurnFinished ( int iPlayerNum, int iTimeDelay, cPlayer *Player )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_FINISHED_TURN );

	message->pushInt16 ( iTimeDelay );
	message->pushInt16( iPlayerNum );

	if ( Player != NULL ) Server->sendNetMessage( message, Player->Nr );
	else Server->sendNetMessage( message );
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
	//TODO: remove information such sentrystatus, build or clearrounds from normal data
	//		becouse this data will be received by enemys, too
	message->pushBool ( Vehicle->bSentryStatus );
	message->pushInt16 ( Vehicle->ClearingRounds );
	message->pushInt16 ( Vehicle->BuildRounds );
	message->pushBool ( Vehicle->IsBuilding );
	message->pushBool ( Vehicle->IsClearing );
	message->pushInt16 ( Vehicle->Disabled );
	message->pushBool ( Vehicle->bIsBeeingAttacked );
	message->pushString ( Vehicle->name );

	// Data for identifying the unit by the client
	message->pushBool( Vehicle->data.is_big );
	message->pushInt16( Vehicle->PosX );
	message->pushInt16( Vehicle->PosY );
	message->pushBool( true );	// true for vehicles
	message->pushInt16 ( Vehicle->iID );
	message->pushInt16( Vehicle->owner->Nr );

	Server->sendNetMessage( message, iPlayer );
}

void sendSpecificUnitData ( cVehicle *Vehicle )
{
	cNetMessage* message = new cNetMessage( GAME_EV_SPECIFIC_UNIT_DATA );
	message->pushInt16 ( Vehicle->BandY );
	message->pushInt16 ( Vehicle->BandX );
	message->pushBool ( Vehicle->BuildPath );
	message->pushInt16 ( Vehicle->BuildingTyp.iSecondPart );
	message->pushInt16 ( Vehicle->BuildingTyp.iFirstPart );
	message->pushInt16 ( Vehicle->dir );
	message->pushInt16 ( Vehicle->iID );
	Server->sendNetMessage( message, Vehicle->owner->Nr );
}

void sendUnitData ( cBuilding *Building, int iPlayer )
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
	message->pushBool ( Building->bSentryStatus );
	message->pushBool ( Building->IsWorking );
	message->pushInt16 ( Building->Disabled );
	message->pushString ( Building->name );

	// Data for identifying the unit by the client
	message->pushInt16( Building->PosX );
	message->pushInt16( Building->PosY );
	message->pushBool( false );	// false for buildings
	message->pushInt16 ( Building->iID );
	message->pushInt16( Building->owner->Nr );

	Server->sendNetMessage( message, iPlayer );
}

void sendChatMessageToClient( string message, int iType, int iPlayer, string inserttext )
{
	cNetMessage* newMessage;
	newMessage = new cNetMessage( GAME_EV_CHAT_SERVER );
	newMessage->pushString( inserttext );
	newMessage->pushString( message );
	newMessage->pushChar( iType );
	Server->sendNetMessage( newMessage, iPlayer );
}

void sendDoStartWork( cBuilding* building )
{
	int offset = building->PosX + building->PosY * Server->Map->size;

	//check all players
	for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++)
	{
		cPlayer* player = (*Server->PlayerList)[i];

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
		message->pushInt32( building->iID );

		Server->sendNetMessage( message, player->Nr );
	}
}

void sendDoStopWork( cBuilding* building )
{
	int offset = building->PosX + building->PosY * Server->Map->size;

	//check all players
	for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++)
	{
		cPlayer* player = (*Server->PlayerList)[i];

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
		message->pushInt32( building->iID );

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

void sendMoveJobServer( cServerMoveJob *MoveJob, int iPlayer )
{
	bool bEnd = false;
	sWaypoint *LastWaypoint = MoveJob->Waypoints;
	while ( LastWaypoint ) LastWaypoint = LastWaypoint->next;
	sWaypoint *Waypoint;
	cNetMessage* message = new cNetMessage( GAME_EV_MOVE_JOB_SERVER );

	int iCount = 0;
	do
	{
		Waypoint = MoveJob->Waypoints;
		while ( Waypoint != LastWaypoint )
		{
			if ( Waypoint->next == LastWaypoint )
			{
				LastWaypoint = Waypoint;
				break;
			}
			Waypoint = Waypoint->next;
		}
		if ( MoveJob->Waypoints == Waypoint ) bEnd = true;

		message->pushInt16( Waypoint->Costs );
		message->pushInt32( Waypoint->X+Waypoint->Y*MoveJob->Map->size );
		iCount++;
	}
	while ( message->iLength <= PACKAGE_LENGTH-19 && !bEnd );

	message->pushInt16( iCount );
	message->pushBool ( MoveJob->bPlane );
	message->pushInt32( MoveJob->DestX+MoveJob->DestY*MoveJob->Map->size );
	message->pushInt32( MoveJob->ScrX+MoveJob->ScrY*MoveJob->Map->size );
	message->pushInt16( MoveJob->Vehicle->iID );

	// don't send movejobs that are to long
	if ( !bEnd ) return;

	Server->sendNetMessage( message, iPlayer );
}

void sendVehicleResources( cVehicle *Vehicle, cMap *Map )
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

void sendResources( cPlayer *Player )
{
	int iCount = 0;
	cNetMessage* message = new cNetMessage( GAME_EV_RESOURCES );
	for ( int i = 0; i < Player->MapSize; i++ )
	{
		if (  Player->ResourceMap[i] == 1 )
		{
			message->pushInt16( Server->Map->Resources[i].value );
			message->pushInt16( Server->Map->Resources[i].typ );
			message->pushInt32( i );
			iCount++;
		}
		if ( message->iLength >= PACKAGE_LENGTH-10 )
		{
			message->pushInt16( iCount );
			Server->sendNetMessage( message, Player->Nr );
			message = new cNetMessage( GAME_EV_RESOURCES );
			iCount = 0;
		}
	}
	if ( iCount > 0 )
	{
		message->pushInt16( iCount );
		Server->sendNetMessage( message, Player->Nr );
	}
}

void sendBuildAnswer( bool bOK, cVehicle* vehicle )
{
	//message for the owner
	cNetMessage* message = new cNetMessage( GAME_EV_BUILD_ANSWER );
	message->pushInt16( vehicle->BandY );
	message->pushInt16( vehicle->BandX );
	message->pushBool(  vehicle->BuildPath );
	message->pushInt16( vehicle->BuildRounds );
	message->pushInt16( vehicle->BuildingTyp.iSecondPart );
	message->pushInt16( vehicle->BuildingTyp.iFirstPart );
	message->pushBool ( vehicle->BuildingTyp.getUnitData()->is_big );
	message->pushInt16( vehicle->PosY );
	message->pushInt16( vehicle->PosX );
	message->pushInt16( vehicle->iID );
	message->pushBool ( bOK );
	Server->sendNetMessage( message, vehicle->owner->Nr );

	//message for the enemys
	for ( unsigned int i = 0; i < vehicle->SeenByPlayerList.Size(); i++ )
	{
		cNetMessage* message = new cNetMessage( GAME_EV_BUILD_ANSWER );
		message->pushBool ( vehicle->BuildingTyp.getUnitData()->is_big );
		message->pushInt16( vehicle->PosY );
		message->pushInt16( vehicle->PosX );
		message->pushInt16( vehicle->iID );
		message->pushBool ( bOK );
		Server->sendNetMessage( message, vehicle->SeenByPlayerList[i]->Nr );
	}
}

void sendStopBuild ( int iVehicleID, int iNewPos, int iPlayer  )
{
	cNetMessage* message = new cNetMessage( GAME_EV_STOP_BUILD );
	message->pushInt32( iNewPos );
	message->pushInt16( iVehicleID );
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
		for ( unsigned int i = 0; i < SubBase->buildings.Size(); i++ )
		{
			if ( message->iLength+14 > PACKAGE_LENGTH )
			{
				message->pushInt16 ( iCount );
				message->pushInt16 ( SubBase->iID );
				Server->sendNetMessage( message, iPlayer );
				iCount = 0;
				message = new cNetMessage( GAME_EV_SUBBASE_BUILDINGS );
			}
			message->pushInt16(SubBase->buildings[i]->iID);
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

void sendBuildList ( cBuilding *Building )
{
	cNetMessage* message = new cNetMessage( GAME_EV_BUILDLIST );
	message->pushBool ( Building->RepeatBuild );
	message->pushInt16 ( Building->BuildSpeed );
	message->pushInt16 ( Building->MetalPerRound );
	for ( int i = (int)Building->BuildList->Size()-1; i >= 0; i-- )
	{
		message->pushInt16((*Building->BuildList)[i]->metall_remaining);
		message->pushInt16((*Building->BuildList)[i]->typ->data.ID.iSecondPart);
		message->pushInt16((*Building->BuildList)[i]->typ->data.ID.iFirstPart);
	}
	message->pushInt16 ( (int)Building->BuildList->Size() );
	message->pushInt16 ( Building->iID );
	Server->sendNetMessage( message, Building->owner->Nr );
}

void sendProduceValues ( cBuilding *Building )
{
	cNetMessage* message = new cNetMessage( GAME_EV_PRODUCE_VALUES );
	message->pushInt16 ( Building->MaxGoldProd );
	message->pushInt16 ( Building->GoldProd );
	message->pushInt16 ( Building->MaxOilProd );
	message->pushInt16 ( Building->OilProd );
	message->pushInt16 ( Building->MaxMetalProd );
	message->pushInt16 ( Building->MetalProd );
	message->pushInt16 ( Building->iID );
	Server->sendNetMessage( message, Building->owner->Nr );
}

void sendTurnReport ( cPlayer *Player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_TURN_REPORT );
	int iCount = 0;
	sTurnstartReport *Report;

	message->pushBool ( Player->ReportForschungFinished );
	Player->ReportForschungFinished = false;

	while ( Player->ReportBuildings.Size() )
	{
		Report = Player->ReportBuildings[0];
		message->pushInt16 ( Report->iAnz );
		message->pushInt16 ( Report->Type.iSecondPart );
		message->pushInt16 ( Report->Type.iFirstPart );
		Player->ReportBuildings.Delete ( 0 );
		delete Report;
		iCount++;
	}
	while ( Player->ReportVehicles.Size() )
	{
		Report = Player->ReportVehicles[0];
		message->pushInt16 ( Report->iAnz );
		message->pushInt16 ( Report->Type.iSecondPart );
		message->pushInt16 ( Report->Type.iFirstPart );
		Player->ReportVehicles.Delete ( 0 );
		delete Report;
		iCount++;
	}
	message->pushInt16 ( iCount );
	Server->sendNetMessage( message, Player->Nr );
}

void sendSupply ( int iDestID, bool bDestVehicle, int iValue, int iType, int iPlayerNum )
{
	cNetMessage* message = new cNetMessage( GAME_EV_SUPPLY );
	message->pushInt16 ( iValue );
	message->pushInt16 ( iDestID );
	message->pushBool ( bDestVehicle );
	message->pushChar ( iType );
	Server->sendNetMessage( message, iPlayerNum );
}

void sendDetectionState( cVehicle* vehicle )
{
	cNetMessage* message = new cNetMessage( GAME_EV_DETECTION_STATE );
	message->pushBool( vehicle->DetectedByPlayerList.Size() > 0 );
	message->pushInt32( vehicle->iID );
	Server->sendNetMessage( message, vehicle->owner->Nr );
}

void sendCheckVehiclePositions(cPlayer* p )
{
	//generate a message for all players
	for ( unsigned int n = 0; n < Server->PlayerList->Size(); n++ )
	{
		cPlayer* player = (*Server->PlayerList)[n];
		if ( p && p != player ) continue;

		cNetMessage* message = new cNetMessage( DEBUG_CHECK_VEHICLE_POSITIONS );
		for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
		{
			cVehicle* vehicle = (*Server->PlayerList)[i]->VehicleList;
			while ( vehicle )
			{
				//check wether the vehicle is visible on the client
				unsigned int x;
				for ( x = 0; x < vehicle->SeenByPlayerList.Size(); x++ )
				{
					if ( vehicle->SeenByPlayerList[x] == player ) break;
				}
				if ( x < vehicle->SeenByPlayerList.Size() || vehicle->owner == player )
				{
					if ( vehicle->ServerMoveJob )
					{
						message->pushInt16( -1 );
						message->pushInt16( -1 );
					}
					else
					{
						message->pushInt16( vehicle->PosX );
						message->pushInt16( vehicle->PosY );
					}
					message->pushInt32( vehicle->iID  );
					if ( message->iLength > PACKAGE_LENGTH - 6 )
					{
						message->pushBool(false);
						Server->sendNetMessage( message );
						message = new cNetMessage( DEBUG_CHECK_VEHICLE_POSITIONS );
					}
				}

				vehicle = vehicle->next;
			}
		}
		message->pushBool( true );	//true for last message part
		Server->sendNetMessage( message, player->Nr );
	}
}

void sendClearAnswer ( int answertype, cVehicle *Vehicle, int turns, int bigoffset, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_CLEAR_ANSWER );
	message->pushInt16( bigoffset );
	message->pushInt16( turns );
	message->pushInt16( Vehicle->iID );
	message->pushInt16( answertype );
	Server->sendNetMessage( message, iPlayer );
}

void sendStopClear ( cVehicle *Vehicle, int bigoffset, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_STOP_CLEARING );
	message->pushInt16( bigoffset );
	message->pushInt16( Vehicle->iID );
	Server->sendNetMessage( message, iPlayer );
}

void sendNoFog ( int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_NOFOG );
	Server->sendNetMessage( message, iPlayer );
}

void sendDefeated ( cPlayer *Player, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_DEFEATED );
	message->pushInt16( Player->Nr );
	Server->sendNetMessage( message, iPlayer );
}

void sendFreeze ( int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_FREEZE );
	Server->sendNetMessage( message, iPlayer );
}

void sendDefreeze ( int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_DEFREEZE );
	Server->sendNetMessage( message, iPlayer );
}

void sendDeletePlayer ( cPlayer *Player, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_DEL_PLAYER );
	message->pushInt16( Player->Nr );
	Server->sendNetMessage( message, iPlayer );
}

void sendRequestIdentification ( int iSocket )
{
	cNetMessage* message = new cNetMessage( GAME_EV_REQ_IDENT );
	message->pushInt16 ( iSocket );
	Log.write("Server: <-- " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );
	network->sendTo( iSocket, message->iLength, message->serialize() );
}

void sendOKReconnect ( cPlayer *Player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_OK_RECONNECT );
	for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
	{
		cPlayer const *SecondPlayer = (*Server->PlayerList)[i];
		if ( Player == SecondPlayer ) continue;
		message->pushInt16 ( SecondPlayer->Nr );
		message->pushInt16 ( GetColorNr( SecondPlayer->color ) );
		message->pushString ( SecondPlayer->name );
	}
	message->pushInt16 ( (int)Server->PlayerList->Size() );
	message->pushString ( Server->Map->MapName );
	message->pushInt16 ( GetColorNr( Player->color ) );
	message->pushInt16 ( Player->Nr );
	Server->sendNetMessage( message, Player->Nr );
}

void sendTurn ( int turn, cPlayer *Player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_TURN );
	message->pushInt16 ( turn );
	Server->sendNetMessage( message, Player->Nr );
}

void sendHudSettings ( cHud *Hud, cPlayer *Player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_HUD_SETTINGS );
	message->pushBool ( Hud->TNT );
	message->pushBool ( Hud->Treffer );
	message->pushBool ( Hud->Lock );
	message->pushBool ( Hud->Studie );
	message->pushBool ( Hud->Status );
	message->pushBool ( Hud->Scan );
	message->pushBool ( Hud->Reichweite );
	message->pushBool ( Hud->MinimapZoom );
	message->pushBool ( Hud->Nebel );
	message->pushBool ( Hud->Munition );
	message->pushBool ( Hud->Gitter );
	message->pushBool ( Hud->Farben );
	message->pushInt16 ( Hud->Zoom );
	message->pushInt16 ( Hud->OffY );
	message->pushInt16 ( Hud->OffX );
	message->pushInt16 ( Hud->tmpSelectedUnitID );
	Server->sendNetMessage( message, Player->Nr );
}

void sendStoreVehicle ( int unitid, bool vehicle, int storedunitid, int player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_STORE_UNIT );
	message->pushInt16 ( unitid );
	message->pushBool ( vehicle );
	message->pushInt16 ( storedunitid );
	Server->sendNetMessage( message, player );
}

void sendActivateVehicle ( int unitid, bool vehicle, int activatunitid, int x, int y, int player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_EXIT_UNIT );
	message->pushInt16 ( y );
	message->pushInt16 ( x );
	message->pushInt16 ( unitid );
	message->pushBool ( vehicle );
	message->pushInt16 ( activatunitid );
	Server->sendNetMessage( message, player );
}

void sendDeleteEverything ( int player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_DELETE_EVERYTHING );
	Server->sendNetMessage( message, player );
}

void sendUnitUpgrades ( sUnitData *Data, int player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_UNIT_UPGRADE_VALUES );
	message->pushInt16 ( Data->max_hit_points );
	message->pushInt16 ( Data->max_ammo );
	message->pushInt16 ( Data->max_shots );
	message->pushInt16 ( Data->max_speed );
	message->pushInt16 ( Data->armor );
	message->pushInt16 ( Data->costs );
	message->pushInt16 ( Data->damage );
	message->pushInt16 ( Data->range );
	message->pushInt16 ( Data->scan );
	message->pushInt16 ( Data->version );
	message->pushInt16 ( Data->ID.iSecondPart );
	message->pushInt16 ( Data->ID.iFirstPart );
	Server->sendNetMessage( message, player );
}
