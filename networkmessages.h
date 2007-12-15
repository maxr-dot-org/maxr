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
#ifndef NetworkMessagesH
#define NetworkMessagesH
#include "defines.h"
#include "engine.h"

#define NET_MSG_SEPERATOR	"#"

/**
* @author alzi
*
* @param iFromX
* @param iFromY
* @param iToX
* @param iToY
* @param bPlane
*/
void SendMoveVehicle(int iFromX, int iFromY, int iToX, int iToY, bool bPlane );
/**
* @author alzi
*
* @param iOff
* @param iSavedSpeed
* @param bPlane
*/
void SendSavedSpeed(int iOff, int iSavedSpeed, bool bPlane );
/**
* @author alzi
*
* @param iScrOff Source offset of unit which will attack
* @param iDestOff Offset of target unit
* @param bScrAir is the source unit a plane?
* @param bDestAir is the taget unit a plane?
* @param bScrBuilding is the source unit a building?
*/
void SendAddAtackJob(int iScrOff, int iDestOff, bool bScrAir, bool bDestAir, bool bScrBuilding);
/**
* @author alzi
*
* @param iOff Offset off unit to destroy
* @param bAir Is the unit an plane?
*/
void SendDestroyObject(int iOff, bool bAir);
/**
* @author alzi
*
* @param iScrOff Source offset of unit
* @param iBuildTyp What typ of building?
* @param iRounds How many turns will it take
* @param iCosts How much will it cost?
* @param iBandX BandX
* @param iBandY BandY
* @param iTyp Typ of the Message
*/
void SendStartBuild(int iScrOff, int iBuildTyp, int iRounds, int iCosts, int iBandX, int iBandY, int iTyp);
/**
* @author alzi
*
* @param iPosX Horizontal position
* @param iPosY Vertikal position
* @param iBuildTyp What typ of building?
* @param iPlayerNr Number of player who wants to build
*/
void SendAddBuilding(int iPosX, int iPosY, int iBuildTyp, int iPlayerNr);
/**
* @author alzi
*
* @param iOff
* @param iExitOffX
* @param iExitOffY
*/
void SendResetConstructor(int iOff, int iExitOffX, int iExitOffY);
/**
* @author alzi
*
* @param SyncData
*/
void SendPlayerSync( sSyncPlayer *SyncData );
/**
* @author alzi
*
* @param SyncData
*/
void SendVehicleSync( sSyncVehicle *SyncData );
/**
* @author alzi
*
* @param SyncData
*/
void SendBuildingSync( sSyncBuilding *SyncData );


/**
* @author alzi
*
* @param iScrOff Source offset of unit
* @param iDestOff Offset of target unit
* @param bScrAir is the source unit a plane?
* @param iTyp Typ of the Message
*/
void SendIntIntBool(int iScrOff, int iDestOff, bool bScrAir, int iTyp);
/**
* @author alzi
*
* @param iScrOff Source offset of unit
* @param bScrAir is the source unit a plane?
* @param iTyp Typ of the Message
*/
void SendIntBool(int iScrOff, bool bScrAir, int iTyp);

#endif // NetworkMessagesH
