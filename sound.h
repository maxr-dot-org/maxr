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
#include "autoobj.h"
#include "defines.h"
#include <vector>

#define sSOUND struct Mix_Chunk

extern void FreesSound (sSOUND *sound);

typedef AutoObj<sSOUND, FreesSound> AutoSound;

// Volumes ///////////////////////////////////////////////////////////////////
#define SOUND_CHANNEL_MIN 0
#define SOUND_CHANNEL_MAX 2
#define VOICE_CHANNEL_MIN (SOUND_CHANNEL_MAX + 1)
#define VOICE_CHANNEL_MAX 5
EX int SoundChannel;
EX int VoiceChannel;
EX int SoundLoopChannel;

// Musik /////////////////////////////////////////////////////////////////////
EX std::string MainMusicFile;
EX std::string CreditsMusicFile;
EX std::vector<std::string> MusicFiles;

// Sounds ////////////////////////////////////////////////////////////////////
class cSoundData
{
public:
	// General
	AutoSound SNDAbsorb;
	AutoSound SNDActivate;
	AutoSound SNDArm;
	AutoSound SNDBuilding;
	AutoSound SNDChat;
	AutoSound SNDClearing;
	AutoSound SNDHudButton;
	AutoSound SNDHudSwitch;
	AutoSound SNDLandMineClear;
	AutoSound SNDLandMinePlace;
	AutoSound SNDLoad;
	AutoSound SNDMenuButton;
	AutoSound SNDObjectMenu;
	AutoSound SNDPanelClose;
	AutoSound SNDPanelOpen;
	AutoSound SNDQuitsch;
	AutoSound SNDReload;
	AutoSound SNDRepair;
	AutoSound SNDSeaMineClear;
	AutoSound SNDSeaMinePlace;

	// Explosions
	AutoSound EXPBig[4];
	AutoSound EXPBigWet[2];
	AutoSound EXPSmall[3];
	AutoSound EXPSmallWet[3];

	// Dummy
	AutoSound DummySound;
} EX SoundData;

// Voices ////////////////////////////////////////////////////////////////////
class cVoiceData
{
public:
	AutoSound VOIAmmoLow[2];
	AutoSound VOIAmmoEmpty[2];
	AutoSound VOIAttacking[2];
	AutoSound VOIAttackingEnemy[2];
	AutoSound VOIAttackingUs[3];
	AutoSound VOIBuildDone[4];
	AutoSound VOIClearing;
	AutoSound VOIClearingMines[2];
	AutoSound VOICommandoFailed[3];
	AutoSound VOIDestroyedUs[2];
	AutoSound VOIDetected[2];
	AutoSound VOILanding[3];
	AutoSound VOILayingMines;
	AutoSound VOINoPath[2];
	AutoSound VOINoSpeed;
	AutoSound VOIOK[4];
	AutoSound VOIReammo;
	AutoSound VOIReammoAll;
	AutoSound VOIRepaired[2];
	AutoSound VOIRepairedAll[2];
	AutoSound VOIResearchComplete;
	AutoSound VOISaved;
	AutoSound VOISentry;
	AutoSound VOIStartMore;
	AutoSound VOIStartNone;
	AutoSound VOIStartOne;
	AutoSound VOIStatusRed[2];
	AutoSound VOIStatusYellow[2];
	AutoSound VOISubDetected;
	AutoSound VOISurveying[2];
	AutoSound VOITransferDone;
	AutoSound VOITurnEnd20Sec[3];
	AutoSound VOIUnitDisabled;
	AutoSound VOIUnitDisabledByEnemy[2];
	AutoSound VOIUnitStolen[2];
	AutoSound VOIUnitStolenByEnemy;
} EX VoiceData;

// Prototypen ////////////////////////////////////////////////////////////////
int InitSound (int frequency, int chunksize);
void CloseSound();
void PlayVoice (sSOUND* snd);
template <int N> void PlayRandomVoice (AutoSound (&snds)[N]);
void PlayFX (sSOUND* snd);
template <int N> void PlayRandomFX (AutoSound (&snds)[N]);
void PlayMusic (char const* file);
void SetMusicVol (int vol);
void StopMusic();
void StartMusic();
int PlayFXLoop (sSOUND* snd);
void StopFXLoop (int SndStream);
//void play(sSOUND *snd);
#endif
