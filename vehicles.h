//////////////////////////////////////////////////////////////////////////////
// M.A.X. - vehicles.h
//////////////////////////////////////////////////////////////////////////////
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
// Struktur für die Eigenschaften der Vehicles:
struct sVehicleData{
	// Main info
	int iID;
	string sName;
	string sDescribtion;

	// General info
	bool bIs_Controllable;
	bool bCan_Be_Captured;
	bool bCan_Be_Disabled;
	bool bSize_Length;
	bool bSize_Width;

	// Defence
	bool bIs_Target_Land;
	bool bIs_Target_Sea;
	bool bIs_Target_Air;
	bool bIs_Target_Underwater;
	bool bIs_Target_Mine;
	bool bIs_Target_Building;
	bool bIs_Target_Satellite;
	bool bIs_Target_WMD;
	int iArmor;
	int iHitpoints;

	// Production
	int iBuilt_Costs;
	int iBuilt_Costs_Max;
	int iIs_Produced_by_ID;

	// Weapons
	int iShot_Trajectory;
	#define STRAIGHT  0
	int iAmmo_Type;
	int iAmmo_Quantity;

	int iTarget_Land_Damage;
	int iTarget_Land_Range;
	int iTarget_Sea_Damage;
	int iTarget_Sea_Range;
	int iTarget_Air_Damage;
	int iTarget_Air_Range;
	int iTarget_Mine_Damage;
	int iTarget_Mine_Range;
	int iTarget_Submarine_Damage;
	int iTarget_Submarine_Range;
	int iTarget_Infantry_Damage;
	int iTarget_Infantry_Range;
	int iTarget_WMD_Damage;
	int iTarget_WMD_Range;

	int iShots;
	int iDestination_Area;
	int iDestination_Type;
	#define POINT  0
	#define MIRV  0
	#define SCATTER  0
	int iMovement_Allowed;

	// Abilities
	bool bCan_Clear_Area;
	bool bGets_Experience;
	bool bCan_Disable;
	bool bCan_Capture;
	bool bCan_Dive;
	int iLanding_Type;
	#define ONLY_GARAGE  0
	#define GARAGE_AND_PLATFORM  1
	#define EVERYWHERE  2
	bool bCan_Upgrade;
	bool bCan_Repair;
	bool bCan_Research;
	bool bIs_Kamikaze;
	bool bIs_Infrastructure;
	bool bCan_Place_Mines;
	bool bMakes_Tracks;
	int iSelf_Repair_Type;
	#define NONE  0
	#define AUTOMATIC  1
	#define NORMAL  2
	bool bConverts_Gold;
	bool bNeeds_Energy;
	bool bNeeds_Oil;
	bool bNeeds_Metall;
	bool bNeeds_Humans;
	int iMines_Resources;
	bool bCan_Launch_SRBM;
	int iEnergy_Shield_Strength;
	int iEnergy_Shield_Size;

	// Scan_Abilities
	int iScan_Range_Sight;
	int iScan_Range_Air;
	int iScan_Range_Ground;
	int iScan_Range_Sea;
	int iScan_Range_Submarine;
	int iScan_Range_Mine;
	int iScan_Range_Infantry;
	int iScan_Range_Resources;
	int iScan_Range_Jammer;

	// Movement
	int iMovement_Sum;
	float fCosts_Air;
	float fCosts_Sea;
	float fCosts_Submarine;
	float fCosts_Ground;
	float fFactor_Coast;
	float fFactor_Wood;
	float fFactor_Road;
	float fFactor_Bridge;
	float fFactor_Platform;
	float fFactor_Monorail;
	float fFactor_Wreck;
	float fFactor_Mountains;

	// Storage
	bool bIs_Garage;
	int iCapacity_Metal;
	int iCapacity_Oil;
	int iCapacity_Gold;
	int iCapacity_Energy;
	int iCapacity_Units_Air;
	int iCapacity_Units_Sea;
	int iCapacity_Units_Ground;
	int iCapacity_Units_Infantry;
	int iCan_Use_Unit_As_Garage_ID;


	//////
	// Old-Stuff!
	//////
  int version; // Version des Vehicles
  char name[25];

  // Grunddaten:
  int max_speed;
  int speed;
  int max_hit_points;
  int hit_points;
  int armor;
  int scan;
  int range;
  int max_shots;
  int shots;
  int damage;
  int max_cargo;
  int cargo;
  int max_ammo;
  int ammo;
  int costs;

  // Die Bau-Eigenschaft:
  int can_build;
#define BUILD_NONE  0
#define BUILD_SMALL 1
#define BUILD_BIG   2

  // Die Fahr-Eigenschaft:
  int can_drive;
#define DRIVE_LAND     0
#define DRIVE_SEA      1
#define DRIVE_LANDnSEA 2
#define DRIVE_AIR      3

  // die Transport-Eigenschaft:
  int can_transport;
#define TRANS_NONE     0
#define TRANS_METAL    1
#define TRANS_OIL      2
#define TRANS_GOLD     3
#define TRANS_VEHICLES 4
#define TRANS_MEN      5

  // Die Attack-Eigenschaft:
  int can_attack;
#define ATTACK_NONE     0
#define ATTACK_LAND     1
#define ATTACK_SUB_LAND 2
#define ATTACK_AIR      3
#define ATTACK_AIRnLAND 4

  // Der Style des Mündungsfeuers:
  int muzzle_typ;
#define MUZZLE_BIG 0
#define MUZZLE_ROCKET 1
#define MUZZLE_SMALL 2
#define MUZZLE_MED 3
#define MUZZLE_MED_LONG 4
#define MUZZLE_ROCKET_CLUSTER 5
#define MUZZLE_TORPEDO 6
#define MUZZLE_SNIPER 7

  // weitere Eigenschaften:
  bool can_reload;
  bool can_repair;
  bool can_drive_and_fire;
  bool is_stealth_land;
  bool is_stealth_sea;
  bool is_human;
  bool can_survey;
  bool can_clear;
  bool has_overlay;
  bool build_by_big;
  bool can_lay_mines;
  bool can_detect_mines;
  bool can_detect_sea;
  bool can_detect_land;
  bool make_tracks;
  bool is_commando;
  bool is_alien;
};

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
  sVehicleData data;   // Grunddaten des Vehicles
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

class cPlayer;
class cMJobs;

// Enum für die Symbole
#ifndef D_eSymbols
#define D_eSymbols
enum eSymbols {SSpeed,SHits,SAmmo,SMetal,SEnergy,SShots,SOil,SGold,STrans,SHuman,SAir,SShield};
enum eSymbolsBig {SBSpeed,SBHits,SBAmmo,SBAttack,SBShots,SBRange,SBArmor,SBScan,SBMetal,SBOil,SBGold,SBEnergy,SBHuman};
#endif

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
  int BuildCosts;   // Die Baukosten pro Runde
  int BuildRounds;  // Die verbleibenden Baurunden
  int BuildRoundsStart; // Startwert der Baurunden
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
  sVehicleData data;    // Daten des Vehicles

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
  void ShowBuildList(TList *list,int selected,int offset,bool beschreibung,int *buildspeed);
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
