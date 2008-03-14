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

void sendChatMessage ( string sMsg )
{
	SDL_Event event;
	event.type = GAME_EVENT;
	event.user.code = GAME_EV_CHAT;
	event.user.data1 = malloc ( sMsg.length()+3 );
	memcpy ( event.user.data1, sMsg.c_str(), sMsg.length()+1 );
	event.user.data2 = NULL;
	if ( !network || network->isHost() ) EventHandler->pushEvent( &event );
	network->sendEvent( &event, (int)sMsg.length() );
}
