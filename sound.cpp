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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <SDL_mixer.h>

#include "sound.h"
#include "log.h"
#include "settings.h"
#include "main.h"

static Mix_Music* music_stream = NULL;

static void MusicFinished();

// Initialisiert den Sound:
int InitSound (int frequency, int chunksize)
{
	int audio_rate, audio_channels;
	Uint16 audio_format;

	// init SDL Mixer
	if (Mix_OpenAudio (frequency, AUDIO_S16, 2, chunksize) < 0)
	{
		Log.write ("Could not init SDL_mixer:", cLog::eLOG_TYPE_ERROR);
		Log.write (Mix_GetError(), cLog::eLOG_TYPE_ERROR);
		return 0;
	}
	Mix_QuerySpec (&audio_rate, &audio_format, &audio_channels);

	// install callback for music
	Mix_HookMusicFinished (MusicFinished);

	SoundChannel = SOUND_CHANNEL_MIN;
	VoiceChannel = VOICE_CHANNEL_MIN;
	SoundLoopChannel = VOICE_CHANNEL_MAX + 1;
	return 1;
}

// closes sound
void CloseSound()
{
	if (!cSettings::getInstance().isSoundEnabled()) return;
	Mix_CloseAudio();
}

void FreesSound (sSOUND *sound)
{
	Mix_FreeChunk (sound);
}

// FIXME: internal play function, should not be accessed from outside
// and held more sanity checks and take care of sound channels
// to e.g. open new channels if needed
void play (sSOUND* snd)
{
	if (snd == NULL) return;
	if (Mix_PlayChannel (SoundChannel, snd, 0) == -1)
	{
		Log.write ("Could not play sound:", cLog::eLOG_TYPE_WARNING);
		Log.write (Mix_GetError(), cLog::eLOG_TYPE_WARNING);
		// TODO: maybe that just the channel wasn't free.
		// we could allocate another channel in that case -- beko
	}
}

// plays voice sound
void PlayVoice (sSOUND* snd)
{
	if (!cSettings::getInstance().isSoundEnabled() || cSettings::getInstance().isVoiceMute()) return;
	play (snd);
	Mix_Volume (VoiceChannel, cSettings::getInstance().getVoiceVol());
	VoiceChannel++;
	if (VoiceChannel > VOICE_CHANNEL_MAX) VoiceChannel = VOICE_CHANNEL_MIN;
}

template <int N> void PlayRandomVoice (AutoSound (&snds)[N])
{
	PlayVoice (snds[random (N)]);
}

// Instanciate used versions
template void PlayRandomVoice<2> (AutoSound (&snds)[2]);
template void PlayRandomVoice<3> (AutoSound (&snds)[3]);
template void PlayRandomVoice<4> (AutoSound (&snds)[4]);

// plays fx sound
void PlayFX (sSOUND* snd)
{
	if (!cSettings::getInstance().isSoundEnabled() || cSettings::getInstance().isSoundMute()) return;
	play (snd);
	Mix_Volume (SoundChannel, cSettings::getInstance().getSoundVol());
	SoundChannel++;
	if (SoundChannel > SOUND_CHANNEL_MAX) SoundChannel = SOUND_CHANNEL_MIN;
}

template <int N>
void PlayRandomFX (AutoSound (&snds)[N])
{
	PlayFX (snds[random (N)]);
}

// Instanciate used versions
template void PlayRandomFX<2> (AutoSound (&snds)[2]);
template void PlayRandomFX<3> (AutoSound (&snds)[3]);
template void PlayRandomFX<4> (AutoSound (&snds)[4]);

// plays passed ogg/wav/mod-musicfile in a loop
void PlayMusic (char const* const file)
{
	if (!cSettings::getInstance().isSoundEnabled() || cSettings::getInstance().isMusicMute() || file == NULL) return;
	music_stream = Mix_LoadMUS (file);
	if (!music_stream)
	{
		Log.write ("Failed opening music stream:", cLog::eLOG_TYPE_WARNING);
		Log.write (Mix_GetError(), cLog::eLOG_TYPE_WARNING);
		return;
	}
	Mix_PlayMusic (music_stream, 0);
	Mix_VolumeMusic (cSettings::getInstance().getMusicVol());
}

// sets volume for music
void SetMusicVol (int vol)
{
	if (!cSettings::getInstance().isSoundEnabled()) return;
	Mix_VolumeMusic (vol);
}

//stops music
void StopMusic()
{
	if (!cSettings::getInstance().isSoundEnabled() || !music_stream) return;
	Mix_FreeMusic (music_stream);
	music_stream = NULL;
}

// starts music
void StartMusic()
{
	if (!cSettings::getInstance().isSoundEnabled() || cSettings::getInstance().isMusicMute()) return;
	if (MusicFiles.size() == 0) return;
	PlayMusic (MusicFiles[random ( (int) MusicFiles.size())].c_str());
}

// callback when end of music title is reached
static void MusicFinished()
{
	if (!cSettings::getInstance().isSoundEnabled()) return;
	if (MusicFiles.size() == 0) return;
	PlayMusic (MusicFiles[random ( (int) MusicFiles.size())].c_str());
}

// starts a loop sound
int PlayFXLoop (sSOUND* snd)
{
	if (!cSettings::getInstance().isSoundEnabled() || cSettings::getInstance().isSoundMute() || snd == NULL) return 0;
	Mix_HaltChannel (SoundLoopChannel);
	if (Mix_PlayChannel (SoundLoopChannel, snd, -1) == -1)
	{
		Log.write ("Could not play loop sound", cLog::eLOG_TYPE_WARNING);
		Log.write (Mix_GetError(), cLog::eLOG_TYPE_WARNING);
		// TODO: maybe that just the channel wasn't free.
		// we could allocate another channel in that case -- beko
	}
	Mix_Volume (SoundLoopChannel, cSettings::getInstance().getSoundVol());
	return SoundLoopChannel;
}

// stops a loop sound
void StopFXLoop (int SndStream)
{
	if (!cSettings::getInstance().isSoundEnabled() || SndStream != SoundLoopChannel) return;
	Mix_HaltChannel (SoundLoopChannel);
}
