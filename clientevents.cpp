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
		while ( message->iLength+7 <= PACKAGE_LENGHT-4 && !bEnd)
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

		message->pushInt16( iCount );
		message->pushBool ( MJob->plane );
		message->pushInt16( MJob->DestX+MJob->DestY*MJob->map->size );
		message->pushInt16( MJob->ScrX+MJob->ScrY*MJob->map->size );
		message->pushInt16( MJob->vehicle->iID );

		Client->sendNetMessage( message );
	}
}
