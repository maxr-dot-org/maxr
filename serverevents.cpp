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


void sendAddUnit ( int iPosX, int iPosY, bool bVehicle, int iUnitNum, int iPlayer, bool bInit )
{
	cNetMessage* message;
	
	if ( bVehicle ) message = new cNetMessage ( GAME_EV_ADD_VEHICLE );
	else message = new cNetMessage ( GAME_EV_ADD_BUILDING );

	message->pushInt16( iPosX );
	message->pushInt16( iPosY );
	message->pushInt16( iUnitNum );
	message->pushInt16( iPlayer );
	message->pushBool( bInit );

	Server->sendNetMessage( message, iPlayer );

}

void sendDeleteUnit ( int iPosX, int iPosY, int iPlayer, bool bVehicle, int iClient, bool bPlane, bool bBase, bool bSubBase )
{
	cNetMessage* message;
	if ( bVehicle ) message = new cNetMessage ( GAME_EV_DEL_VEHICLE );
	else message = new cNetMessage ( GAME_EV_DEL_BUILDING );

	message->pushInt16 ( iPosX );
	message->pushInt16 ( iPosY );
	message->pushInt16 ( iPlayer );
	message->pushBool ( bPlane );
	message->pushBool ( bBase );
	message->pushBool ( bSubBase );

	Server->sendNetMessage( message, iClient );

}

void sendAddEnemyUnit ( cVehicle *Vehicle, int iPlayer )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_ADD_ENEM_VEHICLE );

	message->pushInt16( Vehicle->data.max_hit_points );
	message->pushInt16( Vehicle->data.max_ammo );
	message->pushInt16( Vehicle->data.max_speed );
	message->pushInt16( Vehicle->data.max_shots );
	message->pushInt16( Vehicle->data.damage );
	message->pushInt16( Vehicle->data.range );
	message->pushInt16( Vehicle->data.scan );
	message->pushInt16( Vehicle->data.armor );
	message->pushInt16( Vehicle->data.costs );

	message->pushInt16( Vehicle->data.hit_points );
	message->pushInt16( Vehicle->data.shots );
	message->pushInt16( Vehicle->data.speed );

	message->pushInt16( Vehicle->dir );
	message->pushBool ( Vehicle->Wachposten );
	message->pushBool ( Vehicle->IsBuilding );
	message->pushBool ( Vehicle->IsClearing );
	
	message->pushInt16( Vehicle->PosX );
	message->pushInt16( Vehicle->PosY );
	message->pushInt16( Vehicle->typ->nr );
	message->pushInt16( Vehicle->owner->Nr );

	//Todo: Unitname?

	Server->sendNetMessage( message, iPlayer );
}

void sendAddEnemyUnit ( cBuilding *Building, int iPlayer )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_ADD_ENEM_BUILDING );

	//todo: bIsWorking. needed by client to show animation
	message->pushInt16( Building->data.max_hit_points );
	message->pushInt16( Building->data.max_ammo );
	message->pushInt16( Building->data.max_shots );
	message->pushInt16( Building->data.damage );
	message->pushInt16( Building->data.range );
	message->pushInt16( Building->data.scan );
	message->pushInt16( Building->data.armor );
	message->pushInt16( Building->data.costs );

	message->pushInt16( Building->data.hit_points );
	message->pushInt16( Building->data.shots );

	message->pushBool ( Building->Wachposten );

	message->pushInt16( Building->PosX );
	message->pushInt16( Building->PosY );
	message->pushInt16( Building->typ->nr );
	message->pushInt16( Building->owner->Nr );

	//Todo: unit name?

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
