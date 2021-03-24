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

#include "sdlnetcomponent.h"

#include <SDL_net.h>

#include "utility/log.h"

//------------------------------------------------------------------------------
SDLNetComponent::SDLNetComponent()
{
	if (SDLNet_Init() == -1)
	{
		Log.write ("Could not init SDLNet_Init\nNetwork games won't be available!", cLog::eLOG_TYPE_WARNING);
		Log.write (SDL_GetError(), cLog::eLOG_TYPE_WARNING);
	}
	else
	{
		Log.write ("Net started", cLog::eLOG_TYPE_INFO);
	}
}

//------------------------------------------------------------------------------
SDLNetComponent::~SDLNetComponent()
{
	SDLNet_Quit();
}
