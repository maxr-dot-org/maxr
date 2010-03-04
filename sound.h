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
#ifndef soundH
#define soundH
#include <string.h>
#include "defines.h"
#include "main.h"

#ifndef __sSOUND__
#define __sSOUND__
#define sSOUND struct Mix_Chunk
//FIXME: extern c? fix me, seriously!
extern "C"{
extern DECLSPEC struct Mix_Chunk * SDLCALL Mix_LoadWAV_RW(SDL_RWops *src, int freesrc);
#define Mix_LoadWAV(file)	Mix_LoadWAV_RW(SDL_RWFromFile(file, "rb"), 1)
extern DECLSPEC void SDLCALL Mix_FreeChunk(struct Mix_Chunk *chunk);
}
#endif

// Volumes ///////////////////////////////////////////////////////////////////
#define SOUND_CHANNEL_MIN	0
#define SOUND_CHANNEL_MAX	2
#define VOICE_CHANNEL_MIN	SOUND_CHANNEL_MAX+1
#define VOICE_CHANNEL_MAX	5
EX int SoundChannel;
EX int VoiceChannel;
EX int SoundLoopChannel;

// Musik /////////////////////////////////////////////////////////////////////
EX std::string MainMusicFile;
EX std::string CreditsMusicFile;
EX cList<std::string> MusicFiles;

// Sounds ////////////////////////////////////////////////////////////////////
class cSoundData
{
public:
	// General
	sSOUND *SNDHudSwitch;
	sSOUND *SNDHudButton;
	sSOUND *SNDMenuButton;
	sSOUND *SNDChat;
	sSOUND *SNDObjectMenu;
	sSOUND *SNDArm;
	sSOUND *SNDBuilding;
	sSOUND *SNDClearing;
	sSOUND *SNDQuitsch;
	sSOUND *SNDActivate;
	sSOUND *SNDLoad;
	sSOUND *SNDReload;
	sSOUND *SNDRepair;
	sSOUND *SNDLandMinePlace;
	sSOUND *SNDLandMineClear;
	sSOUND *SNDSeaMinePlace;
	sSOUND *SNDSeaMineClear;
	sSOUND *SNDPanelOpen;
	sSOUND *SNDPanelClose;
	sSOUND *SNDAbsorb;

	// Explosions
	sSOUND *EXPBigWet0;
	sSOUND *EXPBigWet1;
	sSOUND *EXPBig0;
	sSOUND *EXPBig1;
	sSOUND *EXPBig2;
	sSOUND *EXPBig3;
	sSOUND *EXPSmallWet0;
	sSOUND *EXPSmallWet1;
	sSOUND *EXPSmallWet2;
	sSOUND *EXPSmall0;
	sSOUND *EXPSmall1;
	sSOUND *EXPSmall2;

	// Dummy
	sSOUND *DummySound;
} EX SoundData;

// Voices ////////////////////////////////////////////////////////////////////
class cVoiceData
{
public:
	sSOUND *VOINoPath1;
	sSOUND *VOINoPath2;
	sSOUND *VOIBuildDone1;
	sSOUND *VOIBuildDone2;
	sSOUND *VOINoSpeed;
	sSOUND *VOIStatusRed;
	sSOUND *VOIStatusYellow;
	sSOUND *VOIClearing;
	sSOUND *VOILowAmmo1;
	sSOUND *VOILowAmmo2;
	sSOUND *VOIOK1;
	sSOUND *VOIOK2;
	sSOUND *VOIOK3;
	sSOUND *VOIWachposten;
	sSOUND *VOITransferDone;
	sSOUND *VOILoaded;
	sSOUND *VOIRepaired;
	sSOUND *VOILayingMines;
	sSOUND *VOIClearingMines;
	sSOUND *VOIResearchComplete;
	sSOUND *VOIUnitStolen;
	sSOUND *VOIUnitDisabled;
	sSOUND *VOICommandoDetected;
	sSOUND *VOIDisabled;
	sSOUND *VOISaved;
	sSOUND *VOIStartNone;
	sSOUND *VOIStartOne;
	sSOUND *VOIStartMore;
	sSOUND *VOIDetected1;
	sSOUND *VOIDetected2;
	sSOUND *VOIAttackingUs;
	sSOUND *VOIDestroyedUs;
} EX VoiceData;

// Prototypen ////////////////////////////////////////////////////////////////
int InitSound(int frequency,int chunksize);
void CloseSound(void);
void PlayVoice(sSOUND *snd);
void PlayFX(sSOUND *snd);
void PlayMusic(char const* file);
void SetMusicVol(int vol);
void StopMusic(void);
void StartMusic(void);
int PlayFXLoop(sSOUND *snd);
void StopFXLoop(int SndStream);
void play(sSOUND *snd);
#endif
