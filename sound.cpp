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
#define _SOUND_CPP_
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <SDL_mixer.h>

#include "sound.h"
#include "log.h"
#include "settings.h"

// Globales //////////////////////////////////////////////////////////////////
Mix_Music *music_stream = NULL;

// Lokale Prototypen /////////////////////////////////////////////////////////
void MusicFinished ( void );

// Funktionen ////////////////////////////////////////////////////////////////
// Initialisiert den Sound:
int InitSound ( int frequency,int chunksize )
{
	int audio_rate,audio_channels;
	Uint16 audio_format;

	// SDL Mixer initialisieren:
	if ( Mix_OpenAudio ( frequency,AUDIO_S16,2,chunksize ) < 0 )
	{
		Log.write("Could not initialize SDL_mixer", cLog::eLOG_TYPE_ERROR);
		return 0;
	}
	Mix_QuerySpec ( &audio_rate, &audio_format, &audio_channels );

	// Callback für Musik installieren:
	Mix_HookMusicFinished ( MusicFinished );

	SoundChannel = SOUND_CHANNEL_MIN;
	VoiceChannel = VOICE_CHANNEL_MIN;
	SoundLoopChannel = VOICE_CHANNEL_MAX+1;
	return 1;
}

// Schließt den Sound:
void CloseSound ( void )
{
	if ( !SettingsData.bSoundEnabled ) return;
	Mix_CloseAudio();
}

// Spielt einen Voice-Sound:
void PlayVoice ( sSOUND *snd )
{
	if ( !SettingsData.bSoundEnabled||SettingsData.VoiceMute ) return;
	Mix_PlayChannel ( VoiceChannel,snd,0 );
	Mix_Volume ( VoiceChannel,SettingsData.VoiceVol );
	VoiceChannel++;
	if ( VoiceChannel>VOICE_CHANNEL_MAX ) VoiceChannel=VOICE_CHANNEL_MIN;
}

// Spielt einen FX-Sound:
void PlayFX ( sSOUND *snd )
{
	if ( !SettingsData.bSoundEnabled||SettingsData.SoundMute ) return;
	Mix_PlayChannel ( SoundChannel,snd,0 );
	Mix_Volume ( SoundChannel,SettingsData.SoundVol );
	SoundChannel++;
	if ( SoundChannel>SOUND_CHANNEL_MAX ) SoundChannel=SOUND_CHANNEL_MIN;
}

// Spielt die übergebene ogg/wav/mod-Datei:
void PlayMusic(char const* const file)
{
	if ( !SettingsData.bSoundEnabled||SettingsData.MusicMute ) return;
	music_stream = Mix_LoadMUS ( file );
	if ( !music_stream )
	{
		Log.write("failed opening music stream", cLog::eLOG_TYPE_WARNING);
		Log.write(Mix_GetError(), cLog::eLOG_TYPE_WARNING);
		return;
	}
	Mix_PlayMusic ( music_stream,0 );
	Mix_VolumeMusic ( SettingsData.MusicVol );
}

// Setzt das Volume der Musik:
void SetMusicVol ( int vol )
{
	if ( !SettingsData.bSoundEnabled ) return;
	Mix_VolumeMusic ( vol );
}

// Stoppt die Musik:
void StopMusic ( void )
{
	if ( !SettingsData.bSoundEnabled||!music_stream ) return;
	Mix_FreeMusic ( music_stream );
	music_stream=NULL;
}

// Startet die Musik:
void StartMusic ( void )
{
	if ( !SettingsData.bSoundEnabled ||SettingsData.MusicMute ) return;
	if ( MusicFiles.Size() == 0 ) return;
	PlayMusic(MusicFiles[random( (int)MusicFiles.Size())].c_str());
}

// Callback, wenn Musik am Ende:
void MusicFinished ( void )
{
	if ( !SettingsData.bSoundEnabled ) return;
	if ( MusicFiles.Size() == 0 ) return;
	PlayMusic(MusicFiles[random( (int)MusicFiles.Size())].c_str());
}

// Startet einen Loop-Sound:
int PlayFXLoop ( sSOUND *snd )
{
	if ( !SettingsData.bSoundEnabled|| SettingsData.SoundMute  ) return 0;
	Mix_HaltChannel ( SoundLoopChannel );
	Mix_PlayChannel ( SoundLoopChannel,snd,-1 );
	Mix_Volume ( SoundLoopChannel,SettingsData.SoundVol );
	return SoundLoopChannel;
}

// Stoppt einen Loop-Sound:
void StopFXLoop ( int SndStream )
{
	if ( !SettingsData.bSoundEnabled||SndStream!=SoundLoopChannel ) return;
	Mix_HaltChannel ( SoundLoopChannel );
}
