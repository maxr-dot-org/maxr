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
#include "clist.h"

#define sSOUND struct Mix_Chunk

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
	sSOUND* SNDHudSwitch;
	sSOUND* SNDHudButton;
	sSOUND* SNDMenuButton;
	sSOUND* SNDChat;
	sSOUND* SNDObjectMenu;
	sSOUND* SNDArm;
	sSOUND* SNDBuilding;
	sSOUND* SNDClearing;
	sSOUND* SNDQuitsch;
	sSOUND* SNDActivate;
	sSOUND* SNDLoad;
	sSOUND* SNDReload;
	sSOUND* SNDRepair;
	sSOUND* SNDLandMinePlace;
	sSOUND* SNDLandMineClear;
	sSOUND* SNDSeaMinePlace;
	sSOUND* SNDSeaMineClear;
	sSOUND* SNDPanelOpen;
	sSOUND* SNDPanelClose;
	sSOUND* SNDAbsorb;

	// Explosions
	sSOUND* EXPBigWet0;
	sSOUND* EXPBigWet1;
	sSOUND* EXPBig0;
	sSOUND* EXPBig1;
	sSOUND* EXPBig2;
	sSOUND* EXPBig3;
	sSOUND* EXPSmallWet0;
	sSOUND* EXPSmallWet1;
	sSOUND* EXPSmallWet2;
	sSOUND* EXPSmall0;
	sSOUND* EXPSmall1;
	sSOUND* EXPSmall2;

	// Dummy
	sSOUND* DummySound;
} EX SoundData;

// Voices ////////////////////////////////////////////////////////////////////
class cVoiceData
{
public:
	sSOUND* VOIAttackingEnemy1;
	sSOUND* VOIAttackingEnemy2;
	sSOUND* VOINoPath1;
	sSOUND* VOINoPath2;
	sSOUND* VOIBuildDone1;
	sSOUND* VOIBuildDone2;
	sSOUND* VOIBuildDone3;
	sSOUND* VOIBuildDone4;
	sSOUND* VOINoSpeed;
	sSOUND* VOIStatusRed;
	sSOUND* VOIStatusRed2;
	sSOUND* VOIStatusYellow;
	sSOUND* VOIStatusYellow2;
	sSOUND* VOIClearing;
	sSOUND* VOILowAmmo1;
	sSOUND* VOILowAmmo2;
	sSOUND* VOIOK1;
	sSOUND* VOIOK2;
	sSOUND* VOIOK3;
	sSOUND* VOIOK4;
	sSOUND* VOISentry;
	sSOUND* VOITransferDone;
	sSOUND* VOILoaded;
	sSOUND* VOILoaded2;
	sSOUND* VOIRepaired;
	sSOUND* VOIRepaired2;
	sSOUND* VOIRepairedAll1;
	sSOUND* VOIRepairedAll2;
	sSOUND* VOILayingMines;
	sSOUND* VOIClearingMines;
	sSOUND* VOIClearingMines2;
	sSOUND* VOIResearchComplete;
	sSOUND* VOIUnitStolen;
	sSOUND* VOIUnitDisabled;
	sSOUND* VOICommandoFailed1;
	sSOUND* VOICommandoFailed2;
	sSOUND* VOICommandoFailed3;
	sSOUND* VOIDisabled;
	sSOUND* VOISaved;
	sSOUND* VOIStartNone;
	sSOUND* VOIStartOne;
	sSOUND* VOIStartMore;
	sSOUND* VOIDetected1;
	sSOUND* VOIDetected2;
	sSOUND* VOIAttackingUs;
	sSOUND* VOIAttackingUs2;
	sSOUND* VOIAttackingUs3;
	sSOUND* VOIDestroyedUs;
	sSOUND* VOIAttacking1;
	sSOUND* VOIAttacking2;
	sSOUND* VOILanding;
	sSOUND* VOISubDetected;
	sSOUND* VOISurveying;
	sSOUND* VOISurveying2;
	sSOUND* VOITurnEnd20Sec1;
	sSOUND* VOITurnEnd20Sec2;
	sSOUND* VOIUnitStolenByEnemy;
} EX VoiceData;

// Prototypen ////////////////////////////////////////////////////////////////
int InitSound (int frequency, int chunksize);
void CloseSound();
void PlayVoice (sSOUND* snd);
void PlayFX (sSOUND* snd);
void PlayMusic (char const* file);
void SetMusicVol (int vol);
void StopMusic();
void StartMusic();
int PlayFXLoop (sSOUND* snd);
void StopFXLoop (int SndStream);
//void play(sSOUND *snd);
#endif
