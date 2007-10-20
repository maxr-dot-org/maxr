//////////////////////////////////////////////////////////////////////////////
// M.A.X. - player.h
//////////////////////////////////////////////////////////////////////////////
#ifndef playerH
#define playerH
#include "defines.h"
#include "main.h"
#include "SDL.h"
#include "hud.h"
#include "buildings.h"
#include "vehicles.h"
#include "base.h"

struct sVehicle;
struct sUnitData;
struct sBuilding;
class cVehicle;
class cBuilding;
class cEngine;
class cBase;

// Die Wachposten Struktur ///////////////////////////////////////////////////
struct sWachposten{
  cVehicle *v;
  cBuilding *b;
};

// Eintrag in der Lock-Liste /////////////////////////////////////////////////
struct sLockElem{
  cVehicle *v;
  cBuilding *b;
};

// Die Research Struktur /////////////////////////////////////////////////////
struct sResearch{
  int working_on;
  int RoundsRemaining;
  int MaxRounds;
  double level;
};

// Die Player-Klasse /////////////////////////////////////////////////////////
class cPlayer{
friend class cEngine;
public:
  cPlayer(string Name,SDL_Surface *Color,int nr);
  ~cPlayer(void);

  string name;
  SDL_Surface *color;
  SDL_Surface *ShieldColor;
  int Nr;

  sUnitData *VehicleData; // Daten aller Vehicles für diesen Player.
  cVehicle *VehicleList;     // Liste aller Vehicles des Spielers.
  sUnitData *BuildingData; // Daten aller Buildings für diesen Player.
  cBuilding *BuildingList;     // Liste aller Buildings des Spielers.
  int MapSize;               // Kartengröße
  char *ScanMap;             // Map mit dem Scannerflags.
  char *ResourceMap;         // Map mit aufgedeckten Resourcen.
  cBase *base;               // Die Basis dieses Spielers.
  TList *WachpostenAir;      // Liste mit den Vehicles/Buildings auf Wachposten.
  char *WachMapAir;          // Map mit dem abgedeckten Bereich.
  TList *WachpostenGround;   // Liste mit den Vehicles/Buildings auf Wachposten.
  char *WachMapGround;       // Map mit dem abgedeckten Bereich.
  char *DetectLandMap;       // Map mit den Gebieten, die an Land gesehen werden können.
  char *DetectSeaMap;        // Map mit den Gebieten, die im Wasser gesehen werden können.
  sResearch ResearchTechs[8];// Map mit den erforschten Technologien.
  int ResearchCount;         // Anzahl an Forschungszentren (die arbeiten).
  int UnusedResearch;        // Nicht benutzte Forschungskapazitäten.
  int Credits;               // Anzahl der erworbenen Credits.
  char *ShieldMap;           // Map mit den gebieten, die durch ein Schild geschützt sind.
  cHud HotHud;               // Gespeichertes Hud für Hot-Seat-Spiele.
  TList *ReportVehicles,*ReportBuildings; // Reportlisten.
  bool ReportForschungFinished; // Merker, ob Forschung abgeschlossen ist.
  TList *LockList;           // Liste mit gelockten Objekten.

  void InitMaps(int MapSizeX);
  void DoScan(void);
  cVehicle *GetNextVehicle(void);
  cVehicle *GetPrevVehicle(void);
  void AddWachpostenV(cVehicle *v);
  void AddWachpostenB(cBuilding *b);
  void DeleteWachpostenV(cVehicle *v);
  void DeleteWachpostenB(cBuilding *b);
  void RefreshWacheAir(void);
  void RefreshWacheGround(void);
  void StartAResearch(void);
  void StopAReserach(void);
  void DoResearch(void);
  void DoTheResearch(int i);
  void CalcShields(void);
  bool ShieldImpact(int dest,int damage);
  bool IsDefeated(void);
  void AddLock(cBuilding *b);
  void AddLock(cVehicle *v);
  void DeleteLock(cBuilding *b);
  void DeleteLock(cVehicle *v);
  bool InLockList(cBuilding *b);
  bool InLockList(cVehicle *v);
  void ToggelLock(struct sGameObjects *OverObject);
  void DrawLockList(cHud *hud);

private:
  cVehicle *AddVehicle(int posx,int posy,sVehicle *v);
  cBuilding *AddBuilding(int posx,int posy,sBuilding *b);
};

#endif

