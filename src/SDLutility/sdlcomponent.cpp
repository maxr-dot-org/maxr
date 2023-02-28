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

#include "sdlcomponent.h"

#include "utility/log.h"

#include <SDL.h>

//------------------------------------------------------------------------------
SDLComponent::SDLComponent (bool withVideo)
{
	auto flags = SDL_INIT_TIMER;
	if (SDL_Init (withVideo ? SDL_INIT_VIDEO | flags : flags) == -1)
	{
		Log.write ("Could not init SDL", cLog::eLogType::Error);
		Log.write (SDL_GetError(), cLog::eLogType::Error);
		throw std::runtime_error ("Could not init SDL");
	}
	Log.info ("Initialized SDL basics - looks good!");
	Log.mark();
}

//------------------------------------------------------------------------------
SDLComponent::~SDLComponent()
{
	SDL_Quit();
}
