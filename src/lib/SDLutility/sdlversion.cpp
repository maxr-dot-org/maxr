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

#include "sdlversion.h"

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_net.h>

#include "utility/log.h"

namespace
{
	//--------------------------------------------------------------------------
	std::string to_string(const SDL_version& version)
	{
		return std::to_string (version.major) + "." + std::to_string (version.minor) + "." + std::to_string (version.patch);
	}

	//--------------------------------------------------------------------------
	bool are_equal (const SDL_version& v1, const SDL_version& v2)
	{
		auto as_tuple = [](const auto& v) { return std::tuple (v.major, v.minor, v.patch); };
		return as_tuple(v1) == as_tuple(v2);
	}

	//--------------------------------------------------------------------------
	void logVersion (const std::string& name, const SDL_version& header, const SDL_version& linked)
	{
		if (are_equal (header, linked))
		{
			Log.info (name + " " + to_string (linked));
		}
		else
		{
			Log.warn ("linked version differ from header version:");
			Log.warn ("header " + name + " " + to_string (header));
			Log.warn ("linked " + name + " " + to_string (linked));
		}
	}

	//--------------------------------------------------------------------------
	SDL_version getLinkedSDLVersion()
	{
		SDL_version sdl_version;
		SDL_GetVersion (&sdl_version);
		return sdl_version;
	}

	//--------------------------------------------------------------------------
	SDL_version getHeaderSDLVersion()
	{
		SDL_version version;
		version.major = SDL_MAJOR_VERSION;
		version.minor = SDL_MINOR_VERSION;
		version.patch = SDL_PATCHLEVEL;
		return version;
	}

	//--------------------------------------------------------------------------
	SDL_version getLinkedSDLMixerVersion()
	{
		return *Mix_Linked_Version();
	}

	//--------------------------------------------------------------------------
	SDL_version getHeaderSDLMixerVersion()
	{
		SDL_version version;
		version.major = SDL_MIXER_MAJOR_VERSION;
		version.minor = SDL_MIXER_MINOR_VERSION;
		version.patch = SDL_MIXER_PATCHLEVEL;
		return version;
	}

	//--------------------------------------------------------------------------
	SDL_version getLinkedSDLNetVersion()
	{
		return *SDLNet_Linked_Version();
	}

	//--------------------------------------------------------------------------
	SDL_version getHeaderSDLNetVersion()
	{
		SDL_version version;
		version.major = SDL_NET_MAJOR_VERSION;
		version.minor = SDL_NET_MINOR_VERSION;
		version.patch = SDL_NET_PATCHLEVEL;
		return version;
	}

}

//------------------------------------------------------------------------------
void logSDLVersions()
{
	logVersion ("SDL", getHeaderSDLVersion(), getLinkedSDLVersion());
	logVersion ("SDL_Mixer", getHeaderSDLMixerVersion(), getLinkedSDLMixerVersion());
	logVersion ("SDL_Net", getHeaderSDLNetVersion(), getLinkedSDLNetVersion());
}
