//////////////////////////////////////////////////////////////////////////////
// M.A.X. - sound.h
//////////////////////////////////////////////////////////////////////////////
#ifndef soundH
#define soundH
#include <string.h>
#include "defines.h"
#include "main.h"

#ifndef __sSOUND__
#define __sSOUND__
#define sSOUND struct Mix_Chunk
extern "C"{
extern DECLSPEC struct Mix_Chunk * SDLCALL Mix_LoadWAV_RW(SDL_RWops *src, int freesrc);
#define Mix_LoadWAV(file)	Mix_LoadWAV_RW(SDL_RWFromFile(file, "rb"), 1)
extern DECLSPEC void SDLCALL Mix_FreeChunk(struct Mix_Chunk *chunk);
}
#endif

// Musik /////////////////////////////////////////////////////////////////////
EX int MusicAnz;
EX string MainMusicFile;
EX string CreditsMusicFile;
EX TList *MusicFiles;

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
void PlayMusic(char *file);
void SetMusicVol(int vol);
void StopMusic(void);
void StartMusic(void);
int PlayFXLoop(sSOUND *snd);
void StopFXLoop(int SndStream);

#endif
