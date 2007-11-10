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
#ifndef vehiclesH
#define vehiclesH
#include "defines.h"
#include "SDL.h"
#include "main.h"
#include "player.h"
#include "sound.h"

// Define zum Updaten:
#define Update(from,to) if((from).hit_points==(from).max_hit_points){(from).hit_points=(to).max_hit_points;}(from).version=(to).version;(from).speed=(to).speed;(from).max_hit_points=(to).max_hit_points;(from).armor=(to).armor;(from).scan=(to).scan;(from).range=(to).range;(from).max_shots=(to).max_shots;(from).damage=(to).damage;(from).max_ammo=(to).max_ammo;(from).costs=(to).costs;

// Vehicle-Strukturen ////////////////////////////////////////////////////////

class cPlayer;
class cMJobs;

// Enum für die Symbole
#ifndef D_eSymbols
#define D_eSymbols
enum eSymbols {SSpeed,SHits,SAmmo,SMetal,SEnergy,SShots,SOil,SGold,STrans,SHuman,SAir,SShield};
enum eSymbolsBig {SBSpeed,SBHits,SBAmmo,SBAttack,SBShots,SBRange,SBArmor,SBScan,SBMetal,SBOil,SBGold,SBEnergy,SBHuman};
#endif

// Struktur für die Bilder und Sounds:
struct sVehicle{
  SDL_Surface *img[8],*img_org[8]; // 8 Surfaces des Vehicles
  SDL_Surface *shw[8],*shw_org[8]; // 8 Surfaces des Schattens
  SDL_Surface *build,*build_org;         // Surfaces beim Bauen
  SDL_Surface *build_shw,*build_shw_org; // Surfaces beim Bauen (Schatten)
  SDL_Surface *clear_small,*clear_small_org;         // Surfaces beim Clearen (die große wird in build geladen)
  SDL_Surface *clear_small_shw,*clear_small_shw_org; // Surfaces beim Clearen (Schatten) (die große wird in build geladen)
  SDL_Surface *overlay,*overlay_org;     // Overlays
  SDL_Surface *storage; // Bild des Vehicles im Lager  
  char *FLCFile;       // FLC-Video
  sUnitData data;   // Grunddaten des Vehicles
  char id[4];          // ID dieses Elements
  int nr;              // Nr dieses Elements  
  SDL_Surface *info;   // Infobild
  char *text;          // Infotext

  // Die Sounds:
  struct Mix_Chunk *Wait;
  struct Mix_Chunk *WaitWater;
  struct Mix_Chunk *Start;
  struct Mix_Chunk *StartWater;
  struct Mix_Chunk *Stop;
  struct Mix_Chunk *StopWater;
  struct Mix_Chunk *Drive;
  struct Mix_Chunk *DriveWater;
  struct Mix_Chunk *Attack;  
};

// Die Vehicle Klasse ////////////////////////////////////////////////////////
class cVehicle{
public:
  cVehicle(sVehicle *v,cPlayer *Owner);
  ~cVehicle(void);

  int PosX,PosY;   // Position auf der Karte
  int OffX,OffY;   // Offset während der Bewegung
  sVehicle *typ;   // Typ des Vehicles
  int dir;         // aktuelle Drehrichtung
  bool selected;   // Gibt an, ob das Fahrzeug ausgewählt ist
  string name; // Name des Vehicles
  cPlayer *owner;  // Eigentümer des Vehicles
  cMJobs *mjob;    // Der Movejob des Vehicles
  bool moving;     // Gibt an, ob sich das Vehicle grade bewegt
  bool rotating;   // Gibt an, ob sich das Vehicle grade dreht
  bool MoveJobActive; // Gibt an, ob der MoveJob gerade ausgeführt wird
  bool MenuActive; // Gibt an, ob das Menü aktiv ist
  bool AttackMode; // Gibt an, ob der Attack-Modus aktiv ist
  bool Attacking;  // Gibt an, ob das Fahrzeug gerade angreift
  int ditherX,ditherY; // Dithering für Flugzeuge
  bool IsBuilding;  // Gibt an ob was gebaut wird
  int BuildingTyp;  // Gibt an, was gebaut wird
  int BuildCosts;   // Die verbleibenden Baukosten
  int BuildRounds;  // Die verbleibenden Baurunden
  int BuildRoundsStart; // Startwert der Baurunden (fürs Pfadbauen)
  int BuildCostsStart;  // Startwert der Baukosten (fürs Pfadbauen)
  bool PlaceBand;   // Gibt an, ob grad ein Band platziert wird
  int BandX,BandY;  // X,Y Position für das Band
  int BuildBigSavedPos; // Letzte Position vor dem Baubeginn
  bool BuildPath;   // Gibt an, ob ein Pfad gebaut werden soll
  bool BuildOverride; // Um nen kleinen Grafikfehler im MP zu beheben
  bool IsClearing;  // Gibt an, ob einn Feld geräumt wird
  int ClearingRounds; // Gibt an, wie lange ein Feld noch geräumt wird
  bool ClearBig;    // Gibt an, ob ein großes Feld geräumt werden soll
  bool ShowBigBeton; // Gibt an, ob eine große Betonfläche gemalt werden soll
  int BigBetonAlpha; // AlphaWert des großen Betons
  bool Wachposten;  // Gibt an, ob das Behicle auf Wachposten
  bool Transfer;    // Gibt an, ob gerade ein Transfer statfinden soll
  int StartUp;      // Zähler für die Startupannimation
  int FlightHigh;   // Die Flughöhe des Flugzeugs 
  bool LoadActive; // Gibt an, ob ein Vehicle geladen werden soll
  TList *StoredVehicles; // Liste mit geladenen Vehicles
  int VehicleToActivate; // Nummer des Vehicles, dass aktiviert werden soll
  bool ActivatingVehicle; // Gibt an, ob ein Vehicle aktiviert werden soll
  bool MuniActive;   // Gibt an, ob grad Munition aufgeladen werden soll
  bool RepairActive; // Gibt an, ob grad repariert werden soll
  bool LayMines;    // Gibt an, ob Minen gelegt werden sollen
  bool ClearMines;  // Gibt an, ob Minen geräumt werden sollen
  bool detected;    // Gibt an, ob das Vehicle vom aktuellen Spieler entdeckt wurde
  bool Loaded;      // Gibt an, ob das Vehicle geladen wurde
  int DamageFXPointX,DamageFXPointY; // Die Punkte, an denen Rauch bei beschädigung aufsteigen wird
  int WalkFrame;    // Frame der Geh-Annimation
  int CommandoRank; // Rang des Commandos
  bool StealActive,DisableActive; // Legt fest, ob gestohlen, oder sabotiert werden soll
  int Disabled;     // Gibt an, für wie lange diese Einheit disabled ist
  bool detection_override; // Override für die Detection
  bool IsLocked;    // Gibt an, ob dieses Vehicle in irgend einer Log-Liste ist

  cVehicle *next,*prev; // Verkettungselemente
  sUnitData data;    // Daten des Vehicles

  void Draw(SDL_Rect *dest);
  void Select(void);
  void Deselct(void);
  void ShowDetails(void);
  void GenerateName(void);
  void RefreshData(void);
  void DrawSymbol(eSymbols sym,int x,int y,int maxx,int value,int maxvalue,SDL_Surface *sf);
  void DrawNumber(int x,int y,int value,int maxvalue,SDL_Surface *sf);
  void ShowHelp(void);
  void DrawSymbolBig(eSymbolsBig sym,int x,int y,int maxx,int value,int orgvalue,SDL_Surface *sf);
  bool CanDrive(int MapOff);
  int GetScreenPosX(void);
  int GetScreenPosY(void);
  void DrawPath(void);
  void RotateTo(int Dir);
  char *GetStatusStr(void);
  int PlayStram(void);
  void StartMoveSound(void);
  void DrawMenu(void);
  int GetMenuPointAnz(void);
  SDL_Rect GetMenuSize(void);
  bool MouseOverMenu(int mx,int my);
  void DecSpeed(int value);
  void DrawMunBar(void);
  void DrawHelthBar(void);
  void Center(void);
  bool CanAttackObject(int off,bool override=false);
  bool IsInRange(int off);
  void DrawAttackCursor(struct sGameObjects *go,int can_attack);
  int CalcHelth(int damage);
  void ShowBuildMenu(void);
  void ShowBuildList(TList *list,int selected,int offset,bool beschreibung,int *buildspeed, int *iTurboBuildCosts, int *TurboBuildRounds );
  void DrawBuildButtons(int speed);
  void FindNextband(void);
  void DoSurvey(void);
  void MakeReport(void);
  bool CanTransferTo(sGameObjects *go);
  void ShowTransfer(sGameObjects *target);
  void DrawTransBar(int len);
  void MakeTransBar(int *trans,int MaxTarget,int Target);
  void ShowBigDetails(void);
  void Wachwechsel(void);
  bool InWachRange(void);
  void DrawExitPoints(sVehicle *typ);
  bool CanExitTo(int off,sVehicle *typ);
  bool CanLoad(int off);
  void StoreVehicle(int off);
  void ShowStorage(void);
  void DrawStored(int off);
  void ExitVehicleTo(int nr,int off,bool engine_call);
  bool CanMuni(int off);
  bool CanRepair(int off);
  void LayMine(void);
  void ClearMine(void);
  void DetectMines(void);
  bool IsInRangeCommando(int off,bool steal);
  void DrawCommandoCursor(struct sGameObjects *go,bool steal);
  int CalcCommandoChance(bool steal);
  void CommandoOperation(int off,bool steal);
  void DeleteStored(void);
};

#endif
