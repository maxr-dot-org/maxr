//////////////////////////////////////////////////////////////////////////////
// M.A.X. - sound.cpp
//////////////////////////////////////////////////////////////////////////////
#define _SOUND_CPP_
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "sound.h"
// #include "credits.h"
#include "SDL_mixer.h"
//#include "SDL_thread.h"

// Globales //////////////////////////////////////////////////////////////////
Mix_Music *music_stream = NULL;
#define SOUND_CHANNEL_MIN 0
#define SOUND_CHANNEL_MAX 2
int SoundChannel=SOUND_CHANNEL_MIN;
#define VOICE_CHANNEL_MIN 3
#define VOICE_CHANNEL_MAX 5
int VoiceChannel=VOICE_CHANNEL_MIN;
int SoundLoopChannel=6;

// Lokale Prototypen /////////////////////////////////////////////////////////
void MusicFinished(void);

// Funktionen ////////////////////////////////////////////////////////////////
// Initialisiert den Sound:
int InitSound(int frequency,int chunksize){
	int audio_rate,audio_channels;
	Uint16 audio_format;

  // SDL Mixer initialisieren:
	int haha = Mix_OpenAudio(frequency,AUDIO_S16,2,chunksize);
  if(haha<0)return 0;
	Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);

  // Callback für Musik installieren:
  Mix_HookMusicFinished(MusicFinished);

  return 1;
}

// Schließt den Sound:
void CloseSound(void){
  if(!sound)return;
	Mix_CloseAudio();
	return;
}

// Spielt einen Voice-Sound:
void PlayVoice(sSOUND *snd){
  if(!sound||VoiceMute)return;
  Mix_PlayChannel(VoiceChannel,snd,0);
  Mix_Volume(VoiceChannel,VoiceVol);
  VoiceChannel++;
  if(VoiceChannel>VOICE_CHANNEL_MAX)VoiceChannel=VOICE_CHANNEL_MIN;
}

// Spielt einen FX-Sound:
void PlayFX(sSOUND *snd){
  if(!sound||SoundMute)return;
  Mix_PlayChannel(SoundChannel,snd,0);
  Mix_Volume(SoundChannel,SoundVol);
  SoundChannel++;
  if(SoundChannel>SOUND_CHANNEL_MAX)SoundChannel=SOUND_CHANNEL_MIN;
}

// Spielt die übergebene ogg/wav/mod-Datei:
void PlayMusic(char *file){
  if(!sound||MusicMute)return;
  music_stream = Mix_LoadMUS(file);
  if(!music_stream) return;
  Mix_PlayMusic(music_stream,0);
  Mix_VolumeMusic(MusicVol);
}

// Setzt das Volume der Musik:
void SetMusicVol(int vol){
  if(!sound)return;
	Mix_VolumeMusic(vol);
}

// Stoppt die Musik:
void StopMusic(void){
  if(!sound||!music_stream)return;
	Mix_FreeMusic(music_stream);
  music_stream=NULL;
}

// Startet die Musik:
void StartMusic(void){
  if(!sound)return;
  PlayMusic((char *)MusicFiles->Items[random(MusicAnz,0)].c_str());
}

// Callback, wenn Musik am Ende:
void MusicFinished(void){
  if(!sound/*||in_credits*/)return;
  srand( (unsigned)time( NULL ) );
  PlayMusic((char *)MusicFiles->Items[random(MusicAnz,0)].c_str());
}

// Startet einen Loop-Sound:
int PlayFXLoop(sSOUND *snd){
  if(!sound)return 0;
  Mix_HaltChannel(SoundLoopChannel);
  Mix_PlayChannel(SoundLoopChannel,snd,-1);
  Mix_Volume(SoundLoopChannel,SoundVol);
  return SoundLoopChannel;
}

// Stoppt einen Loop-Sound:
void StopFXLoop(int SndStream){
  if(!sound||SndStream!=SoundLoopChannel)return;
  Mix_HaltChannel(SoundLoopChannel);
}
