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

SDL_Event* generateEvent ( int iTyp, int iLenght, void *data )
{
	SDL_Event* event = new SDL_Event;
	event->type = GAME_EVENT;
	event->user.code = iTyp;
	event->user.data1 = malloc ( iLenght );
	memcpy ( event->user.data1, data, iLenght );
	event->user.data2 = NULL;
	return event;
}



void sendAddUnit ( int iPosX, int iPosY, bool bVehicle, int iUnitNum, int iPlayer, bool bInit )
{
	char data[7];
	((Sint16*)data)[0] = SDL_SwapLE16( iPosX );
	((Sint16*)data)[1] = SDL_SwapLE16( iPosY );
	((Sint16*)data)[2] = SDL_SwapLE16( iUnitNum );
	data[6] = bInit;

	SDL_Event* event;
	if ( bVehicle ) event = generateEvent ( GAME_EV_ADD_VEHICLE, 7, data );
	else event = generateEvent ( GAME_EV_ADD_BUILDING, 7, data );

	if ( Server ) Server->sendEvent ( event, 7, iPlayer );
}

void sendDeleteUnit ( int iPosX, int iPosY, int iPlayer, bool bVehicle, int iClient, bool bPlane, bool bBase, bool bSubBase )
{
	char data[9];
	((Sint16*)data)[0] = SDL_SwapLE16( iPosX );
	((Sint16*)data)[1] = SDL_SwapLE16( iPosY );
	((Sint16*)data)[2] = SDL_SwapLE16( iPlayer );
	data[6] = bPlane;
	data[7] = bBase;
	data[8] = bSubBase;

	SDL_Event* event;
	if ( bVehicle ) event = generateEvent ( GAME_EV_DEL_VEHICLE, 9, data );
	else event = generateEvent ( GAME_EV_DEL_BUILDING, 9, data );

	if ( Server ) Server->sendEvent ( event, 9, iClient );
}

void sendAddEnemyUnit ( cVehicle *Vehicle, int iPlayer )
{
	char data[37];
	((Sint16*)data)[0] = SDL_SwapLE16( Vehicle->PosX );
	((Sint16*)data)[1] = SDL_SwapLE16( Vehicle->PosY );
	((Sint16*)data)[2] = SDL_SwapLE16( Vehicle->owner->Nr );
	((Sint16*)data)[3] = SDL_SwapLE16( Vehicle->typ->nr );

	((Sint16*)data)[4] = SDL_SwapLE16( Vehicle->data.max_hit_points );
	((Sint16*)data)[5] = SDL_SwapLE16( Vehicle->data.max_ammo );
	((Sint16*)data)[6] = SDL_SwapLE16( Vehicle->data.max_speed );
	((Sint16*)data)[7] = SDL_SwapLE16( Vehicle->data.max_shots );
	((Sint16*)data)[8] = SDL_SwapLE16( Vehicle->data.damage );
	((Sint16*)data)[9] = SDL_SwapLE16( Vehicle->data.range );
	((Sint16*)data)[10] = SDL_SwapLE16( Vehicle->data.scan );
	((Sint16*)data)[11] = SDL_SwapLE16( Vehicle->data.armor );
	((Sint16*)data)[12] = SDL_SwapLE16( Vehicle->data.costs );

	((Sint16*)data)[13] = SDL_SwapLE16( Vehicle->data.hit_points );
	((Sint16*)data)[14] = SDL_SwapLE16( Vehicle->data.shots );
	((Sint16*)data)[15] = SDL_SwapLE16( Vehicle->data.speed );

	((Sint16*)data)[16] = SDL_SwapLE16( Vehicle->dir );
	data[34] = Vehicle->Wachposten;
	data[35] = Vehicle->IsBuilding;
	data[36] = Vehicle->IsClearing;

	SDL_Event* event;
	event = generateEvent ( GAME_EV_ADD_ENEM_VEHICLE, 37, data );

	if ( Server ) Server->sendEvent ( event, 37, iPlayer );
}

void sendAddEnemyUnit ( cBuilding *Building, int iPlayer )
{
	char data[29];
	((Sint16*)data)[0] = SDL_SwapLE16( Building->PosX );
	((Sint16*)data)[1] = SDL_SwapLE16( Building->PosY );
	((Sint16*)data)[2] = SDL_SwapLE16( Building->owner->Nr );
	((Sint16*)data)[3] = SDL_SwapLE16( Building->typ->nr );

	((Sint16*)data)[4] = SDL_SwapLE16( Building->data.max_hit_points );
	((Sint16*)data)[5] = SDL_SwapLE16( Building->data.max_ammo );
	((Sint16*)data)[6] = SDL_SwapLE16( Building->data.max_shots );
	((Sint16*)data)[7] = SDL_SwapLE16( Building->data.damage );
	((Sint16*)data)[8] = SDL_SwapLE16( Building->data.range );
	((Sint16*)data)[9] = SDL_SwapLE16( Building->data.scan );
	((Sint16*)data)[10] = SDL_SwapLE16( Building->data.armor );
	((Sint16*)data)[11] = SDL_SwapLE16( Building->data.costs );

	((Sint16*)data)[12] = SDL_SwapLE16( Building->data.hit_points );
	((Sint16*)data)[13] = SDL_SwapLE16( Building->data.shots );

	data[28] = Building->Wachposten;

	SDL_Event* event;
	event = generateEvent ( GAME_EV_ADD_ENEM_BUILDING, 29, data );

	if ( Server ) Server->sendEvent ( event, 29, iPlayer );
}
