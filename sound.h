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
	AutoSound SNDHudSwitch;
	AutoSound SNDHudButton;
	AutoSound SNDMenuButton;
	AutoSound SNDChat;
	AutoSound SNDObjectMenu;
	AutoSound SNDArm;
	AutoSound SNDBuilding;
	AutoSound SNDClearing;
	AutoSound SNDQuitsch;
	AutoSound SNDActivate;
	AutoSound SNDLoad;
	AutoSound SNDReload;
	AutoSound SNDRepair;
	AutoSound SNDLandMinePlace;
	AutoSound SNDLandMineClear;
	AutoSound SNDSeaMinePlace;
	AutoSound SNDSeaMineClear;
	AutoSound SNDPanelOpen;
	AutoSound SNDPanelClose;
	AutoSound SNDAbsorb;

	// Explosions
	AutoSound EXPBigWet[2];
	AutoSound EXPBig[4];
	AutoSound EXPSmallWet[3];
	AutoSound EXPSmall[3];

	// Dummy
	AutoSound DummySound;
} EX SoundData;

// Voices ////////////////////////////////////////////////////////////////////
class cVoiceData
{
public:
	AutoSound VOIAttackingEnemy[2];
	AutoSound VOINoPath[2];
	AutoSound VOIBuildDone[4];
	AutoSound VOINoSpeed;
	AutoSound VOIStatusRed[2];
	AutoSound VOIStatusYellow[2];
	AutoSound VOIClearing;
	AutoSound VOILowAmmo[2];
	AutoSound VOIOK[4];
	AutoSound VOISentry;
	AutoSound VOITransferDone;
	AutoSound VOILoaded[2];
	AutoSound VOIRepaired[2];
	AutoSound VOIRepairedAll[2];
	AutoSound VOILayingMines;
	AutoSound VOIClearingMines[2];
	AutoSound VOIResearchComplete;
	AutoSound VOIUnitStolen;
	AutoSound VOIUnitDisabled;
	AutoSound VOICommandoFailed[3];
	AutoSound VOIDisabled;
	AutoSound VOISaved;
	AutoSound VOIStartNone;
	AutoSound VOIStartOne;
	AutoSound VOIStartMore;
	AutoSound VOIDetected[2];
	AutoSound VOIAttackingUs[3];
	AutoSound VOIDestroyedUs;
	AutoSound VOIAttacking[2];
	AutoSound VOILanding;
	AutoSound VOISubDetected;
	AutoSound VOISurveying[2];
	AutoSound VOITurnEnd20Sec[2];
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
