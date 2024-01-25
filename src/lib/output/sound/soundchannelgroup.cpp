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

#include "output/sound/soundchannelgroup.h"

#include "output/sound/soundchannel.h"
#include "utility/log.h"

#include <SDL_mixer.h>
#include <cassert>

//------------------------------------------------------------------------------
cSoundChannelGroup::cSoundChannelGroup (int sdlGroupTag_) :
	sdlGroupTag (sdlGroupTag_)
{}

//------------------------------------------------------------------------------
cSoundChannelGroup::~cSoundChannelGroup()
{}

//------------------------------------------------------------------------------
void cSoundChannelGroup::addChannel (int channelIndex)
{
	Mix_GroupChannel (channelIndex, sdlGroupTag);

	soundChannels.insert (std::make_unique<cSoundChannel> (channelIndex));
}

//------------------------------------------------------------------------------
void cSoundChannelGroup::addChannelRange (int first, int last)
{
	Mix_GroupChannels (first, last, sdlGroupTag);

	for (int i = first; i <= last; ++i)
	{
		soundChannels.insert (std::make_unique<cSoundChannel> (i));
	}
}

//------------------------------------------------------------------------------
cSoundChannel& cSoundChannelGroup::getFreeChannel (bool haltIfNotAvailable)
{
	int channel = Mix_GroupAvailable (sdlGroupTag);
	if (channel == -1 && haltIfNotAvailable)
	{
		channel = Mix_GroupOldest (sdlGroupTag);
		if (channel != -1)
		{
			auto iter = soundChannels.find (channel);
			assert (iter != soundChannels.end());
			(*iter)->stop();
		}
	}

	if (channel == -1)
	{
		Log.warn ("Could not get any available channel of group: " + std::to_string (sdlGroupTag));
		static cSoundChannel dummyChannel (0);
		return dummyChannel;
	}

	auto iter = soundChannels.find (channel);
	assert (iter != soundChannels.end());
	return **iter;
}

//------------------------------------------------------------------------------
void cSoundChannelGroup::setVolume (int volume)
{
	for (const auto& soundChannel : soundChannels)
	{
		soundChannel->setVolume (volume);
	}
}

//------------------------------------------------------------------------------
bool cSoundChannelGroup::sChannelLess::operator() (const std::unique_ptr<cSoundChannel>& left, const std::unique_ptr<cSoundChannel>& right) const
{
	return left->getSdlChannelId() < right->getSdlChannelId();
}

//------------------------------------------------------------------------------
bool cSoundChannelGroup::sChannelLess::operator() (const std::unique_ptr<cSoundChannel>& left, int right) const
{
	return left->getSdlChannelId() < right;
}

//------------------------------------------------------------------------------
bool cSoundChannelGroup::sChannelLess::operator() (int left, const std::unique_ptr<cSoundChannel>& right) const
{
	return left < right->getSdlChannelId();
}
