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

#include "output/sound/sounddevice.h"

#include "output/sound/soundchannel.h"
#include "output/sound/soundchunk.h"
#include "resources/sound.h"
#include "settings.h"
#include "utility/log.h"
#include "utility/random.h"

const int cSoundDevice::soundEffectGroupTag = 0;
const int cSoundDevice::voiceGroupTag = 1;

const int cSoundDevice::soundEffectGroupSize = 5;
const int cSoundDevice::voiceGroupSize = 5;

//------------------------------------------------------------------------------
cSoundDevice::cSoundDevice() :
	soundEffectChannelGroup (soundEffectGroupTag),
	voiceChannelGroup (voiceGroupTag)
{}

//------------------------------------------------------------------------------
void cSoundDevice::SdlMixMusikDeleter::operator() (Mix_Music* music) const
{
	Mix_FreeMusic (music);
}

//------------------------------------------------------------------------------
cSoundDevice& cSoundDevice::getInstance()
{
	static cSoundDevice instance;
	return instance;
}

//------------------------------------------------------------------------------
void musicFinishedHookCallback()
{
	cSoundDevice::getInstance().startRandomMusic();
}

//------------------------------------------------------------------------------
void cSoundDevice::initialize (int frequency, int chunkSize)
{
	if (Mix_OpenAudio (frequency, AUDIO_S16, 2, chunkSize) != 0)
	{
		throw std::runtime_error (Mix_GetError());
	}

	Mix_AllocateChannels (soundEffectGroupSize + voiceGroupSize);

	soundEffectChannelGroup.addChannelRange (0, soundEffectGroupSize - 1);
	voiceChannelGroup.addChannelRange (soundEffectGroupSize, soundEffectGroupSize + voiceGroupSize - 1);

	Mix_HookMusicFinished (musicFinishedHookCallback);

	setSoundEffectVolume (cSettings::getInstance().getSoundVol());
	setVoiceVolume (cSettings::getInstance().getVoiceVol());
	setMusicVolume (cSettings::getInstance().getMusicVol());
}

//------------------------------------------------------------------------------
void cSoundDevice::close()
{
	Mix_CloseAudio();
}

//------------------------------------------------------------------------------
cSoundChannel* cSoundDevice::getFreeSoundEffectChannel()
{
	if (!cSettings::getInstance().isSoundEnabled() || cSettings::getInstance().isSoundMute()) return nullptr;

	return &soundEffectChannelGroup.getFreeChannel();
}

//------------------------------------------------------------------------------
cSoundChannel* cSoundDevice::getFreeVoiceChannel()
{
	if (!cSettings::getInstance().isSoundEnabled() || cSettings::getInstance().isVoiceMute()) return nullptr;

	return &voiceChannelGroup.getFreeChannel();
}

//------------------------------------------------------------------------------
bool cSoundDevice::playSoundEffect (const cSoundChunk& chunk)
{
	auto channel = getFreeSoundEffectChannel();

	if (channel == nullptr) return false;

	channel->play (chunk);

	return true;
}

//------------------------------------------------------------------------------
bool cSoundDevice::playVoice (const cSoundChunk& chunk)
{
	auto channel = getFreeVoiceChannel();

	if (channel == nullptr) return false;

	channel->play (chunk);

	return true;
}

//------------------------------------------------------------------------------
void cSoundDevice::startMusic (const std::filesystem::path& fileName)
{
	if (!cSettings::getInstance().isSoundEnabled() || cSettings::getInstance().isMusicMute()) return;

	musicStream = SaveSdlMixMusicPointer (Mix_LoadMUS (fileName.u8string().c_str()));
	if (!musicStream)
	{
		Log.warn ("Failed opening music stream:");
		Log.warn (Mix_GetError());
		return;
	}
	Mix_PlayMusic (musicStream.get(), 0);
}

//------------------------------------------------------------------------------
void cSoundDevice::startRandomMusic()
{
	if (MusicFiles.backgrounds.empty()) return;
	startMusic (MusicFiles.backgrounds[random (MusicFiles.backgrounds.size())]);
}

//------------------------------------------------------------------------------
void cSoundDevice::stopMusic()
{
	musicStream = nullptr;
}

//------------------------------------------------------------------------------
void cSoundDevice::setSoundEffectVolume (int volume)
{
	soundEffectChannelGroup.setVolume (volume);
}

//------------------------------------------------------------------------------
void cSoundDevice::setVoiceVolume (int volume)
{
	voiceChannelGroup.setVolume (volume);
}

//------------------------------------------------------------------------------
void cSoundDevice::setMusicVolume (int volume)
{
	Mix_VolumeMusic (volume);
}

//------------------------------------------------------------------------------
//int cSoundDevice::playInGroup (const cSoundChunk& sound, int groupTag)
//{
//	if (!cSettings::getInstance().isSoundEnabled()) return -1;
//
//	int channel = Mix_GroupAvailable (groupTag);
//	if (channel == -1)
//	{
//		channel = Mix_GroupOldest (groupTag);
//		Mix_HaltChannel (channel);
//	}
//	channel = Mix_PlayChannel (channel, sound.getSdlSound(), 0);
//	if (channel < 0)
//	{
//		Log.warn ("Could not play sound:");
//		Log.warn (Mix_GetError());
//	}
//	return channel;
//}
