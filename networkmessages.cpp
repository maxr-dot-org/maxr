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

#include "networkmessages.h"
#include "game.h"

void SendMoveVehicle(int iFromX, int iFromY, int iToX, int iToY, bool bPlane )
{
	string sMessage;
	sMessage = iToStr( iFromX ) + NET_MSG_SEPERATOR + iToStr( iFromY ) + NET_MSG_SEPERATOR + iToStr( iToX ) + NET_MSG_SEPERATOR + iToStr( iToY ) + NET_MSG_SEPERATOR + iToStr( bPlane );
	game->engine->network->TCPSend( MSG_MOVE_VEHICLE,sMessage.c_str() );
}

void SendSavedSpeed(int iOff, int iSavedSpeed, bool bPlane )
{
	string sMessage;
	sMessage = iToStr( iOff ) + NET_MSG_SEPERATOR + iToStr( iSavedSpeed ) + NET_MSG_SEPERATOR + iToStr( bPlane );
	game->engine->network->TCPSend( MSG_SAVED_SPEED,sMessage.c_str() );
}

void SendAddAtackJob(int iScrOff, int iDestOff, bool bScrAir, bool bDestAir, bool bScrBuilding)
{
	string sMessage;
	sMessage = iToStr( iScrOff ) + NET_MSG_SEPERATOR + iToStr( iDestOff ) + NET_MSG_SEPERATOR + iToStr( bScrAir ) + NET_MSG_SEPERATOR + iToStr( bDestAir ) + NET_MSG_SEPERATOR + iToStr( bScrBuilding );
	game->engine->network->TCPSend( MSG_ADD_ATTACKJOB, sMessage.c_str() );
}

void SendDestroyObject(int iOff, bool bAir)
{
	string sMessage;
	sMessage = iToStr( iOff ) + NET_MSG_SEPERATOR + iToStr( bAir );
	game->engine->network->TCPSend( MSG_DESTROY_OBJECT, sMessage.c_str() );
}

void SendStartBuild(int iScrOff, int iBuildTyp, int iRounds, int iCosts, int iBandX, int iBandY, int iTyp)
{
	string sMessage;
	sMessage = iToStr( iScrOff ) + NET_MSG_SEPERATOR + iToStr( iBuildTyp ) + NET_MSG_SEPERATOR + iToStr( iRounds ) + NET_MSG_SEPERATOR + iToStr( iCosts ) + NET_MSG_SEPERATOR + iToStr( iBandX ) + NET_MSG_SEPERATOR + iToStr( iBandY );
	game->engine->network->TCPSend ( iTyp, sMessage.c_str() );
}

void SendAddBuilding(int iPosX, int iPosY, int iBuildTyp, int iPlayerNr )
{
	string sMessage;
	sMessage = iToStr ( iPosX ) + NET_MSG_SEPERATOR + iToStr ( iPosY ) + NET_MSG_SEPERATOR + iToStr ( iBuildTyp ) + NET_MSG_SEPERATOR + iToStr ( iPlayerNr );
	game->engine->network->TCPSend ( MSG_ADD_BUILDING, sMessage.c_str() );
}

void SendResetConstructor(int iOff, int iExitOffX, int iExitOffY)
{
	string sMessage;
	sMessage = iToStr( iOff ) + NET_MSG_SEPERATOR + iToStr( iExitOffX ) + NET_MSG_SEPERATOR + iToStr( iExitOffY );
	game->engine->network->TCPSend ( MSG_RESET_CONSTRUCTOR, sMessage.c_str() );
}

void SendPlayerSync( sSyncPlayer *SyncData )
{
	string sMessage;
	sMessage = iToStr( SyncData->PlayerID ) + NET_MSG_SEPERATOR + iToStr( SyncData->EndOfSync ) + NET_MSG_SEPERATOR + iToStr( SyncData->Credits ) + NET_MSG_SEPERATOR +
				iToStr( SyncData->ResearchCount ) + NET_MSG_SEPERATOR + iToStr( SyncData->UnusedResearch ) + NET_MSG_SEPERATOR + iToStr( SyncData->TNT ) + NET_MSG_SEPERATOR +
				iToStr( SyncData->Radar ) + NET_MSG_SEPERATOR + iToStr( SyncData->Nebel ) + NET_MSG_SEPERATOR + iToStr( SyncData->Gitter ) + NET_MSG_SEPERATOR +
				iToStr( SyncData->Scan ) + NET_MSG_SEPERATOR + iToStr( SyncData->Reichweite ) + NET_MSG_SEPERATOR + iToStr( SyncData->Munition ) + NET_MSG_SEPERATOR +
				iToStr( SyncData->Treffer ) + NET_MSG_SEPERATOR + iToStr( SyncData->Farben ) + NET_MSG_SEPERATOR + iToStr( SyncData->Status ) + NET_MSG_SEPERATOR +
				iToStr( SyncData->Studie ) + NET_MSG_SEPERATOR + iToStr( SyncData->Lock ) + NET_MSG_SEPERATOR + iToStr( SyncData->PlayFLC ) + NET_MSG_SEPERATOR +
				iToStr( SyncData->Zoom ) + NET_MSG_SEPERATOR + iToStr( SyncData->OffX ) + NET_MSG_SEPERATOR + iToStr( SyncData->OffY );
	for( int i = 0 ; i < 8 ; i++ )
	{
		sMessage += NET_MSG_SEPERATOR + iToStr( SyncData->ResearchTechs->working_on ) + NET_MSG_SEPERATOR + iToStr( SyncData->ResearchTechs->RoundsRemaining ) + NET_MSG_SEPERATOR +
				iToStr( SyncData->ResearchTechs->MaxRounds ) + NET_MSG_SEPERATOR + iToStr( SyncData->ResearchTechs->level );
	}
	game->engine->network->TCPSend ( MSG_SYNC_PLAYER, sMessage.c_str() );
}

void SendVehicleSync( sSyncVehicle *SyncData )
{
	string sMessage;
	sMessage = iToStr( SyncData->PlayerID ) + NET_MSG_SEPERATOR + iToStr( SyncData->EndOfSync ) + NET_MSG_SEPERATOR + iToStr( SyncData->isPlane ) + NET_MSG_SEPERATOR +
		iToStr( SyncData->off ) + NET_MSG_SEPERATOR + iToStr( SyncData->IsBuilding ) + NET_MSG_SEPERATOR + iToStr( SyncData->BuildingTyp ) + NET_MSG_SEPERATOR +
		iToStr( SyncData->BuildCosts ) + NET_MSG_SEPERATOR + iToStr( SyncData->BuildRounds ) + NET_MSG_SEPERATOR + iToStr( SyncData->BuildRoundsStart ) + NET_MSG_SEPERATOR +
		iToStr( SyncData->BandX ) + NET_MSG_SEPERATOR + iToStr( SyncData->BandY ) + NET_MSG_SEPERATOR + iToStr( SyncData->IsClearing ) + NET_MSG_SEPERATOR +
		iToStr( SyncData->ClearingRounds ) + NET_MSG_SEPERATOR + iToStr( SyncData->ClearBig ) + NET_MSG_SEPERATOR + iToStr( SyncData->ShowBigBeton ) + NET_MSG_SEPERATOR +
		iToStr( SyncData->FlightHigh ) + NET_MSG_SEPERATOR + iToStr( SyncData->LayMines ) + NET_MSG_SEPERATOR + iToStr( SyncData->ClearMines ) + NET_MSG_SEPERATOR +
		iToStr( SyncData->Loaded ) + NET_MSG_SEPERATOR + iToStr( SyncData->CommandoRank ) + NET_MSG_SEPERATOR + iToStr( SyncData->Disabled ) + NET_MSG_SEPERATOR +
		iToStr( SyncData->Ammo ) + NET_MSG_SEPERATOR + iToStr( SyncData->Cargo);

	game->engine->network->TCPSend ( MSG_SYNC_VEHICLE, sMessage.c_str() );
}

void SendBuildingSync( sSyncBuilding *SyncData )
{
	string sMessage;
	sMessage = iToStr( SyncData->PlayerID ) + NET_MSG_SEPERATOR + iToStr( SyncData->EndOfSync ) + NET_MSG_SEPERATOR + iToStr( SyncData->iTyp ) + NET_MSG_SEPERATOR +
		iToStr( SyncData->off ) + NET_MSG_SEPERATOR + iToStr( SyncData->isBase ) + NET_MSG_SEPERATOR + iToStr( SyncData->IsWorking ) + NET_MSG_SEPERATOR + iToStr( SyncData->MetalProd ) + NET_MSG_SEPERATOR +
		iToStr( SyncData->OilProd ) + NET_MSG_SEPERATOR + iToStr( SyncData->GoldProd ) + NET_MSG_SEPERATOR + iToStr( SyncData->MaxMetalProd ) + NET_MSG_SEPERATOR +
		iToStr( SyncData->MaxOilProd ) + NET_MSG_SEPERATOR + iToStr( SyncData->MaxGoldProd ) + NET_MSG_SEPERATOR + iToStr( SyncData->BuildSpeed ) + NET_MSG_SEPERATOR +
		iToStr( SyncData->RepeatBuild ) + NET_MSG_SEPERATOR + iToStr( SyncData->Disabled ) + NET_MSG_SEPERATOR + iToStr( SyncData->Ammo ) + NET_MSG_SEPERATOR + 
		iToStr( SyncData->Load);

	game->engine->network->TCPSend ( MSG_SYNC_BUILDING, sMessage.c_str() );
}

void SendAddVehicle( int iPlayerNr, int iVehicleNr, int iPosX, int iPosY)
{
	string sMessage;
	sMessage = iToStr(iPlayerNr) + NET_MSG_SEPERATOR + iToStr(iVehicleNr) + NET_MSG_SEPERATOR + iToStr(iPosX) + NET_MSG_SEPERATOR + iToStr(iPosY);
	game->engine->network->TCPSend( MSG_ADD_VEHICLE, sMessage.c_str() );
}

void SendStoreVehicle( bool bDestPlane, bool bBuilding, int iDestOff, int iScrOff, bool bScrPlane )
{
	string sMessage;
	sMessage = iToStr(bDestPlane) + NET_MSG_SEPERATOR + iToStr(bBuilding) + NET_MSG_SEPERATOR + iToStr(iDestOff) + NET_MSG_SEPERATOR + iToStr(iScrOff)+ NET_MSG_SEPERATOR + iToStr(bScrPlane);
	game->engine->network->TCPSend( MSG_STORE_VEHICLE, sMessage.c_str() );
}

void SendActivateVehicle( bool bBuilding, bool bScrPlane, int iUnitIndex, int iDestOff, int iScrOff, int iHitoints, int iAmmo )
{
	string sMessage;
	sMessage = iToStr(bBuilding) + NET_MSG_SEPERATOR + iToStr(bScrPlane) + NET_MSG_SEPERATOR + iToStr(iUnitIndex) + NET_MSG_SEPERATOR + iToStr(iDestOff) 
		+ NET_MSG_SEPERATOR + iToStr(iScrOff)+ NET_MSG_SEPERATOR + iToStr(iHitoints) + NET_MSG_SEPERATOR + iToStr(iAmmo);
	game->engine->network->TCPSend( MSG_ACTIVATE_VEHICLE, sMessage.c_str() );
}
void SendUpgrade( cPlayer *Owner, sUpgradeStruct *UpgradeStruct )
{
	string sMessage;
	if( UpgradeStruct->vehicle )
	{
		sMessage = iToStr( Owner->Nr ) + NET_MSG_SEPERATOR + "1" + NET_MSG_SEPERATOR + iToStr( UpgradeStruct->id ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].damage ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].range ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].max_shots ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].max_ammo ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].max_hit_points ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].armor ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].scan ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].costs ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].max_speed );
	}
	else
	{
		sMessage = iToStr( Owner->Nr ) + NET_MSG_SEPERATOR + "1" + NET_MSG_SEPERATOR + iToStr( UpgradeStruct->id ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].damage ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].range ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].max_shots ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].max_ammo ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].max_hit_points ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].armor ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].scan ) + NET_MSG_SEPERATOR +
			iToStr( Owner->VehicleData[UpgradeStruct->id].costs );
	}
	game->engine->network->TCPSend( MSG_UPGRADE, sMessage.c_str() );
}

void SendResearch( int iPlayerNr, int iResearchNr )
{
	string sMessage;
	sMessage = iToStr ( iPlayerNr ) + NET_MSG_SEPERATOR + iToStr ( iResearchNr );
	game->engine->network->TCPSend( MSG_RESEARCH, sMessage.c_str() );
}

void SendCommandoSuccess( bool bSteal, int SrcVehicleOff, int TargetVehicleOff )
{
	string sMessage;
	sMessage = iToStr(bSteal) + NET_MSG_SEPERATOR + iToStr(SrcVehicleOff) + NET_MSG_SEPERATOR + iToStr(TargetVehicleOff);
	game->engine->network->TCPSend( MSG_COMMANDO_SUCCESS, sMessage.c_str() );
}

void SendReloadRepair( bool bBuilding, bool bPlane, int iUnitOffset, int iDataValue, int iTyp )
{
	string sMessage;
	sMessage = iToStr(bBuilding) + NET_MSG_SEPERATOR + iToStr(bPlane) + NET_MSG_SEPERATOR + iToStr(iUnitOffset) + NET_MSG_SEPERATOR + iToStr(iDataValue);
	game->engine->network->TCPSend( iTyp, sMessage.c_str() );
}

void SendSentryMode( bool bPlane, int iUnitOffset, int iDataValue)
{
	string sMessage;
	sMessage = iToStr(bPlane) + NET_MSG_SEPERATOR + iToStr(iUnitOffset) + NET_MSG_SEPERATOR + iToStr(iDataValue);
	game->engine->network->TCPSend( MSG_WACHE, sMessage.c_str() );
}

void SendChangeUnitName( int iPosX, int iPosY, string sName, bool bSpecialInformation, int iTyp )
{
	string sMessage;
	sMessage = iToStr(iPosX) + NET_MSG_SEPERATOR + iToStr(iPosY) + NET_MSG_SEPERATOR + sName + NET_MSG_SEPERATOR + iToStr(bSpecialInformation);
	game->engine->network->TCPSend( iTyp, sMessage.c_str() );
}

void SendChangePlayerName( int iNr, string sName )
{
	string sMessage;
	sMessage = iToStr(iNr) + NET_MSG_SEPERATOR + sName;
	game->engine->network->TCPSend( MSG_CHANGE_PLAYER_NAME, sMessage.c_str() );
}


void SendIntIntBool(int iScrOff, int iDestOff, bool bScrAir, int iTyp)
{
	string sMessage;
	sMessage = iToStr(iScrOff) + NET_MSG_SEPERATOR + iToStr(iDestOff) + NET_MSG_SEPERATOR + iToStr(bScrAir);
	game->engine->network->TCPSend( iTyp, sMessage.c_str() );
}

void SendIntBool(int iScrOff, bool bScrAir, int iTyp)
{
	string sMessage;
	sMessage = iToStr(iScrOff) + NET_MSG_SEPERATOR + iToStr(bScrAir);
	game->engine->network->TCPSend( iTyp, sMessage.c_str() );
}