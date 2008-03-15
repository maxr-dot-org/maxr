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
#include "eventmessages.h"
#include "network.h"
#include "events.h"

SDL_Event generateEvent ( int iTyp, int iLenght, void *data )
{
	SDL_Event event;
	event.type = GAME_EVENT;
	event.user.code = iTyp;
	event.user.data1 = malloc ( iLenght );
	memcpy ( event.user.data1, data, iLenght );
	event.user.data2 = NULL;
	return event;
}

void sendChatMessage ( string sMsg )
{
	SDL_Event event = generateEvent ( GAME_EV_CHAT, (int)sMsg.length()+1, (char *)sMsg.c_str() );
	if ( !network || network->isHost() ) EventHandler->pushEvent( &event );
	else network->sendEvent( &event, (int)sMsg.length()+1 );
}

void sendDelPlayer ( int iPlayerNum )
{
	char data[2];
	((Sint16*)data)[0] = iPlayerNum;
	SDL_Event event = generateEvent ( GAME_EV_DEL_PLAYER, 2, data );
	if ( !network || network->isHost() ) EventHandler->pushEvent( &event );
	else network->sendEvent( &event, 2 );
}
