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
#include "clientevents.h"
#include "network.h"
#include "events.h"
#include "client.h"
#include "hud.h"

void sendChatMessageToServer ( string sMsg )
{

	cNetMessage* message = new cNetMessage( GAME_EV_CHAT_CLIENT );
	message->pushString( sMsg );
	Client->sendNetMessage( message );
}

void sendWantToEndTurn()
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_TO_END_TURN );
	Client->sendNetMessage( message );
}

void sendWantStartWork( cBuilding* building)
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_START_WORK );
	message->pushInt32(building->iID);
	Client->sendNetMessage(message);
}

void sendWantStopWork( cBuilding* building)
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_STOP_WORK );
	message->pushInt32( building->iID);
	Client->sendNetMessage(message);
}

void sendMoveJob( cClientMoveJob *MoveJob )
{
	bool bEnd = false;
	sWaypoint *LastWaypoint = MoveJob->Waypoints;
	while ( LastWaypoint ) LastWaypoint = LastWaypoint->next;
	sWaypoint *Waypoint;

	cNetMessage* message = new cNetMessage( GAME_EV_MOVE_JOB_CLIENT );

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
		message->pushInt32( Waypoint->X+Waypoint->Y*MoveJob->Map->size);
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

	Client->sendNetMessage( message );
}

void sendWantStopMove ( int iVehicleID )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_STOP_MOVE );
	message->pushInt16( iVehicleID );
	Client->sendNetMessage( message );
}

void sendWantAttack ( int targetID, int targetOffset, int agressor, bool isVehicle)
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_ATTACK );
	message->pushInt32( targetID );
	message->pushInt32( targetOffset );
	message->pushInt32( agressor );
	message->pushBool ( isVehicle );
	Client->sendNetMessage(message);
}

void sendMineLayerStatus( cVehicle *Vehicle )
{
	cNetMessage* message = new cNetMessage( GAME_EV_MINELAYERSTATUS );
	message->pushBool ( Vehicle->LayMines );
	message->pushBool ( Vehicle->ClearMines );
	message->pushInt16 ( Vehicle->iID );
	Client->sendNetMessage(message);
}

void sendWantBuild( int iVehicleID, sID BuildingTypeID, int iBuildSpeed, int iBuildOff, bool bBuildPath, int iPathOff )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_BUILD );
	message->pushInt32 ( iPathOff );
	message->pushBool ( bBuildPath );
	message->pushInt32 ( iBuildOff );
	message->pushInt16 ( iBuildSpeed );
	message->pushInt16 ( BuildingTypeID.iSecondPart );
	message->pushInt16 ( BuildingTypeID.iFirstPart );
	message->pushInt16 ( iVehicleID );
	Client->sendNetMessage( message );
}

void sendWantEndBuilding( cVehicle *Vehicle, int EscapeX, int EscapeY )
{
	cNetMessage* message = new cNetMessage( GAME_EV_END_BUILDING );
	message->pushInt16 ( EscapeY );
	message->pushInt16 ( EscapeX );
	message->pushInt16 ( Vehicle->iID );
	Client->sendNetMessage( message );
}

void sendWantStopBuilding( int iVehicleID )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_STOP_BUILDING );
	message->pushInt16 ( iVehicleID );
	Client->sendNetMessage( message );
}

void sendWantTransfer ( bool bSrcVehicle, int iSrcID, bool bDestVehicle, int iDestID, int iTransferValue, int iType )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_TRANSFER );
	message->pushInt16 ( iType );
	message->pushInt16 ( iTransferValue );
	message->pushInt16 ( iDestID );
	message->pushBool ( bDestVehicle );
	message->pushInt16 ( iSrcID );
	message->pushBool ( bSrcVehicle );
	Client->sendNetMessage( message );
}

void sendWantBuildList ( cBuilding *Building, cList<sBuildList*> &BuildList, bool bRepeat )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_BUILDLIST );
	message->pushBool ( bRepeat );
	for (int i = (int)BuildList.Size()-1; i >= 0; i--)
	{
		message->pushInt16( BuildList[i]->typ->data.ID.iSecondPart );
		message->pushInt16( BuildList[i]->typ->data.ID.iFirstPart );
	}
	message->pushInt16((int)BuildList.Size());
	message->pushInt16 ( Building->BuildSpeed );
	message->pushInt16 ( Building->iID );
	Client->sendNetMessage( message );
}

void sendWantExitFinishedVehicle ( cBuilding *Building, int iX, int iY )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_EXIT_FIN_VEH );
	message->pushInt16 ( iY );
	message->pushInt16 ( iX );
	message->pushInt16 ( Building->iID );
	Client->sendNetMessage( message );
}

void sendChangeResources ( cBuilding *Building, int iMetalProd, int iOilProd, int iGoldProd )
{
	cNetMessage* message = new cNetMessage( GAME_EV_CHANGE_RESOURCES );
	message->pushInt16 ( iGoldProd );
	message->pushInt16 ( iOilProd );
	message->pushInt16 ( iMetalProd );
	message->pushInt16 ( Building->iID );
	Client->sendNetMessage( message );
}

void sendChangeSentry ( int iUnitID, bool bVehicle )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_CHANGE_SENTRY );
	message->pushInt16 ( iUnitID );
	message->pushBool ( bVehicle );
	Client->sendNetMessage( message );
}

void sendWantSupply ( int iDestID, bool bDestVehicle, int iSrcID, bool bSrcVehicle, int iType )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_SUPPLY );
	message->pushInt16 ( iDestID );
	message->pushBool ( bDestVehicle );
	message->pushInt16 ( iSrcID );
	message->pushBool ( bSrcVehicle );
	message->pushChar ( iType );
	Client->sendNetMessage( message );
}

void sendWantStartClear ( cVehicle *Vehicle )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_START_CLEAR );
	message->pushInt16 ( Vehicle->iID );
	Client->sendNetMessage( message );
}

void sendWantStopClear ( cVehicle *Vehicle )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_STOP_CLEAR );
	message->pushInt16 ( Vehicle->iID );
	Client->sendNetMessage( message );
}

void sendAbortWaiting ()
{
	cNetMessage* message = new cNetMessage( GAME_EV_ABORT_WAITING );
	Client->sendNetMessage( message );
}

void sendWantLoad ( int unitid, bool vehicle, int loadedunitid )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_LOAD );
	message->pushInt16 ( unitid );
	message->pushBool ( vehicle );
	message->pushInt16 ( loadedunitid );
	Client->sendNetMessage( message );
}

void sendWantActivate ( int unitid, bool vehicle, int activatunitid, int x, int y )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_EXIT );
	message->pushInt16 ( y );
	message->pushInt16 ( x );
	message->pushInt16 ( unitid );
	message->pushBool ( vehicle );
	message->pushInt16 ( activatunitid );
	Client->sendNetMessage( message );
}

void sendRequestResync( char PlayerNr )
{
	cNetMessage* message = new cNetMessage( GAME_EV_REQUEST_RESYNC );
	message->pushChar( PlayerNr );
	Client->sendNetMessage( message );
}

void sendSetAutoStatus( int unitID, bool set )
{
	cNetMessage* message = new cNetMessage( GAME_EV_AUTOMOVE_STATUS );
	message->pushBool ( set );
	message->pushInt16( unitID );
	Client->sendNetMessage( message );
}

void sendWantComAction( int srcUnitID, int destUnitID, bool destIsVehicle, bool steal )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_COM_ACTION );
	message->pushBool ( steal );
	message->pushInt16( destUnitID );
	message->pushBool ( destIsVehicle );
	message->pushInt16( srcUnitID );
	Client->sendNetMessage( message );
}

void sendWantUpgrade ( int buildingID, int storageSlot, bool upgradeAll )
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_VEHICLE_UPGRADE);
	message->pushInt32( buildingID );
	if ( !upgradeAll ) message->pushInt16( storageSlot );
	message->pushBool ( upgradeAll );
	Client->sendNetMessage ( message );
}

void sendWantResearchChange (int newResearchSettings[cResearch::kNrResearchAreas], int ownerNr)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_RESEARCH_CHANGE);
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
	{
		message->pushInt16 ( newResearchSettings[i] );
	}
	message->pushInt16 ( ownerNr );
	Client->sendNetMessage ( message );
}

void sendSaveHudInfo ( int selectedUnitID, int ownerNr, int savingID )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_SAVE_HUD_INFO );
	message->pushBool ( Client->gameGUI.tntChecked() );
	message->pushBool ( Client->gameGUI.hitsChecked() );
	message->pushBool ( Client->gameGUI.lockChecked() );
	message->pushBool ( Client->gameGUI.surveyChecked() );
	message->pushBool ( Client->gameGUI.statusChecked() );
	message->pushBool ( Client->gameGUI.scanChecked() );
	message->pushBool ( Client->gameGUI.rangeChecked() );
	message->pushBool ( Client->gameGUI.twoXChecked() );
	message->pushBool ( Client->gameGUI.fogChecked() );
	message->pushBool ( Client->gameGUI.ammoChecked() );
	message->pushBool ( Client->gameGUI.gridChecked() );
	message->pushBool ( Client->gameGUI.colorChecked() );
	message->pushFloat ( Client->gameGUI.getZoom() );
	message->pushInt16 ( Client->gameGUI.getOffsetY() );
	message->pushInt16 ( Client->gameGUI.getOffsetX() );
	message->pushInt16 ( selectedUnitID );
	message->pushInt16 ( ownerNr );
	message->pushInt16 ( savingID );
	Client->sendNetMessage ( message );
}

void sendSaveReportInfo ( sSavedReportMessage *savedReport, int ownerNr, int savingID )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_SAVE_REPORT_INFO );
	message->pushInt16 ( savedReport->colorNr );
	message->pushInt16 ( savedReport->unitID.iSecondPart );
	message->pushInt16 ( savedReport->unitID.iFirstPart );
	message->pushInt16 ( savedReport->yPos );
	message->pushInt16 ( savedReport->xPos );
	message->pushInt16 ( savedReport->type );
	message->pushString ( savedReport->message );
	message->pushInt16 ( ownerNr );
	message->pushInt16 ( savingID );
	Client->sendNetMessage ( message );
}

void sendFinishedSendSaveInfo ( int ownerNr, int savingID )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_FIN_SEND_SAVE_INFO );
	message->pushInt16 ( ownerNr );
	message->pushInt16 ( savingID );
	Client->sendNetMessage ( message );
}