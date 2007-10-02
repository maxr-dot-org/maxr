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
EX sSOUND *SNDHudSwitch;
EX sSOUND *SNDHudButton;
EX sSOUND *SNDMenuButton;
EX sSOUND *SNDChat;
EX sSOUND *SNDObjectMenu;
EX sSOUND *SNDArm;
EX sSOUND *SNDBuilding;
EX sSOUND *SNDClearing;
EX sSOUND *SNDQuitsch;
EX sSOUND *SNDActivate;
EX sSOUND *SNDLoad;
EX sSOUND *SNDReload;
EX sSOUND *SNDRepair;
EX sSOUND *SNDLandMinePlace;
EX sSOUND *SNDLandMineClear;
EX sSOUND *SNDSeaMinePlace;
EX sSOUND *SNDSeaMineClear;
EX sSOUND *SNDPanelOpen;
EX sSOUND *SNDPanelClose;
EX sSOUND *SNDAbsorb;

// Voices ////////////////////////////////////////////////////////////////////
EX sSOUND *VOINoPath1;
EX sSOUND *VOINoPath2;
EX sSOUND *VOIBuildDone1;
EX sSOUND *VOIBuildDone2;
EX sSOUND *VOINoSpeed;
EX sSOUND *VOIStatusRed;
EX sSOUND *VOIStatusYellow;
EX sSOUND *VOIClearing;
EX sSOUND *VOILowAmmo1;
EX sSOUND *VOILowAmmo2;
EX sSOUND *VOIOK1;
EX sSOUND *VOIOK2;
EX sSOUND *VOIOK3;
EX sSOUND *VOIWachposten;
EX sSOUND *VOITransferDone;
EX sSOUND *VOILoaded;
EX sSOUND *VOIRepaired;
EX sSOUND *VOILayingMines;
EX sSOUND *VOIClearingMines;
EX sSOUND *VOIResearchComplete;
EX sSOUND *VOIUnitStolen;
EX sSOUND *VOIUnitDisabled;
EX sSOUND *VOICommandoDetected;
EX sSOUND *VOIDisabled;
EX sSOUND *VOISaved;
EX sSOUND *VOIStartNone;
EX sSOUND *VOIStartOne;
EX sSOUND *VOIStartMore;
EX sSOUND *VOIDetected1;
EX sSOUND *VOIDetected2;
EX sSOUND *VOIAttackingUs;
EX sSOUND *VOIDestroyedUs;

// Explosionen ///////////////////////////////////////////////////////////////
EX sSOUND *EXPBigWet0;
EX sSOUND *EXPBigWet1;
EX sSOUND *EXPBig0;
EX sSOUND *EXPBig1;
EX sSOUND *EXPBig2;
EX sSOUND *EXPBig3;
EX sSOUND *EXPSmallWet0;
EX sSOUND *EXPSmallWet1;
EX sSOUND *EXPSmallWet2;
EX sSOUND *EXPSmall0;
EX sSOUND *EXPSmall1;
EX sSOUND *EXPSmall2;

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
