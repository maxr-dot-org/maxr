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
	message->pushInt16(building->PosX);
	message->pushInt16(building->PosY);
	Client->sendNetMessage(message);
}

void sendWantStopWork( cBuilding* building)
{
	cNetMessage* message = new cNetMessage( GAME_EV_WANT_STOP_WORK );
	message->pushInt16( building->PosX);
	message->pushInt16( building->PosY);
	Client->sendNetMessage(message);
}

void sendMoveJob( cMJobs *MJob )
{
	bool bEnd = false;
	sWaypoint *LastWaypoint = MJob->waypoints;
	while ( LastWaypoint ) LastWaypoint = LastWaypoint->next;
	sWaypoint *Waypoint;
	while ( !bEnd )
	{
		cNetMessage* message = new cNetMessage( GAME_EV_MOVE_JOB_CLIENT );

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
		message->pushInt16( MJob->ScrX+MJob->ScrY*MJob->map->size );
		message->pushInt16( MJob->vehicle->iID );

		// since there is an failure in the code yet, don't send movejobs that are to long
		if ( !bEnd ) return;

		Client->sendNetMessage( message );
	}
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
