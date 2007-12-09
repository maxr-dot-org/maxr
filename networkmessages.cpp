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

void SendAddAtackJob(int iScrOff, int iDestOff, bool bScrAir, bool bDestAir, bool bScrBuilding)
{
	string sMessage;
	sMessage = iToStr( iScrOff ) + "#" + iToStr( iDestOff ) + "#" + iToStr( bScrAir ) + "#" + iToStr( bDestAir ) + "#" + iToStr( bScrBuilding );
	game->engine->network->TCPSend( MSG_ADD_ATTACKJOB, sMessage.c_str() );
}

void SendDestroyObject(int iOff, bool bAir)
{
	string sMessage;
	sMessage = iToStr( iOff ) + "#" + iToStr( bAir );
	game->engine->network->TCPSend( MSG_DESTROY_OBJECT, sMessage.c_str() );
}

void SendIntIntBool(int iScrOff, int iDestOff, bool bScrAir, int iTyp)
{
	string sMessage;
	sMessage = iToStr(iScrOff) + "#" + iToStr(iDestOff) + "#" + iToStr(bScrAir);
	game->engine->network->TCPSend( iTyp, sMessage.c_str() );
}

void SendInt(int iScrOff, int iTyp)
{
	game->engine->network->TCPSend( iTyp, iToStr(iScrOff).c_str() );
}

void SendIntBool(int iScrOff, bool bScrAir, int iTyp)
{
	string sMessage;
	sMessage = iToStr(iScrOff) + "#" + iToStr(bScrAir);
	game->engine->network->TCPSend( iTyp, sMessage.c_str() );
}