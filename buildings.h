//////////////////////////////////////////////////////////////////////////////
// M.A.X. - buildings.h
//////////////////////////////////////////////////////////////////////////////
#ifndef buildingsH
#define buildingsH
#include "defines.h"
#include "sound.h"
#include "player.h"
#include "main.h"
#include "SDL.h"
#include "base.h"

// Define zum Updaten:
#define UpdateBuilding(from,to) if((from).hit_points==(from).max_hit_points){(from).hit_points=(to).max_hit_points;}(from).version=(to).version;(from).max_hit_points=(to).max_hit_points;(from).armor=(to).armor;(from).scan=(to).scan;(from).range=(to).range;(from).max_shots=(to).max_shots;(from).damage=(to).damage;(from).max_ammo=(to).max_ammo;(from).costs=(to).costs;

// Struktur für Upgrades /////////////////////////////////////////////////////
struct sUpgrades{
  bool active;
  int NextPrice;
  int Purchased;
  int *value;
  int StartValue;
  string name;
};


// Building-Strukturen ///////////////////////////////////////////////////////
// Struktur für die Eigenschaften der Buildings:
struct sBuildingData{
  int version; // Version des Vehicles
  char name[25];

  // Grunddaten:
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
  int energy_prod;
  int oil_need;
  int energy_need;
  int metal_need;
  int human_prod;
  int human_need;
  int gold_need;
  int shield;
  int max_shield;

  // Die Bau-Eigenschaft:
  int can_build;
// see vehicles.h
#define BUILD_SEA 3
#define BUILD_AIR 4
#define BUILD_MAN 5

  // die Load-Eigenschaft:
  int can_load;
// see vehicles.h
#define TRANS_AIR 6

  // Die Attack-Eigenschaft:
  int can_attack;
// see vehicles.h

  // Der Style des Mündungsfeuers:
  int muzzle_typ;
// see vehicles.h

  // Weitere Eigenschaften:
  bool is_base;
  bool is_big;
  bool is_road;
  bool is_connector;
  bool has_effect;
  bool can_work;
  bool is_mine;
  int has_frames;
  bool is_annimated;
  bool is_bridge;
  bool is_platform;
  bool build_on_water;
  bool is_pad;
  bool is_expl_mine;
  bool can_research;
  bool build_alien;
  bool is_alien;
};

// Struktur für die Bilder und Sounds:
struct sBuilding{
  SDL_Surface *img,*img_org; // Surface des Buildings
  SDL_Surface *shw,*shw_org; // Surfaces des Schattens
  SDL_Surface *eff,*eff_org; // Surfaces des Effektes  
  SDL_Surface *video;  // Video
  sBuildingData data;  // Grunddaten des Buildings
  char id[4];          // ID dieses Elements
  int nr;              // Nr dieses Elements
  SDL_Surface *info;   // Infobild
  char *text;          // Infotext

  // Die Sounds:
  struct Mix_Chunk *Start;
  struct Mix_Chunk *Running;
  struct Mix_Chunk *Stop;
  struct Mix_Chunk *Attack;
};

class cPlayer;
class cBase;

// Enum für die Symbole
#ifndef D_eSymbols
#define D_eSymbols
enum eSymbols {SSpeed,SHits,SAmmo,SMetal,SEnergy,SShots,SOil,SGold,STrans,SHuman,SAir,SShield};
enum eSymbolsBig {SBSpeed,SBHits,SBAmmo,SBAttack,SBShots,SBRange,SBArmor,SBScan,SBMetal,SBOil,SBGold,SBEnergy,SBHuman};
#endif

// Struktur für die Bauliste:
struct sBuildList{
  struct sVehicle *typ;
  int metall_remaining;
};

// Die Buulding Klasse ///////////////////////////////////////////////////////
class cBuilding{
public:
  cBuilding(sBuilding *b,cPlayer *Owner,cBase *Base);
  ~cBuilding(void);

  int PosX,PosY;   // Position auf der Karte
  sBuilding *typ;  // Typ des Buildings
  bool selected;   // Gibt an, ob das Building ausgewählt ist
  string name; // Name des Buildings
  cPlayer *owner;  // Eigentümer des Buildings
  sBuildingData data;    // Daten des Buildings
  cBuilding *next,*prev; // Zeiger für die Verkettung
  bool MenuActive; // Gibt an, ob das Menü grad aktiv ist
  bool AttackMode; // Gibt an, ob der AttackMode grad aktiv ist
  int DirtTyp;     // Typ des Drecks
  int DirtValue;   // Wert des Drecks
  bool BigDirt;    // Gibt an, ob es sich um großen Dreck handelt
  int StartUp;     // Zähler für die Startupannimation
  cBase *base;     // Die Basis des Gebäudes
  bool BaseN,BaseE,BaseS,BaseW; // Gibt an, ob das Gebäude in einer Richting verbunden ist
  bool BaseBN,BaseBE,BaseBS,BaseBW; // Gibt an, ob das Gebäude in einer Richting verbunden ist (zusätzlich für große Gebäude)  
  struct sSubBase *SubBase;     // Die SubBase dieses Gebäudes
  int EffectAlpha; // Alphawert für den Effekt
  bool EffectInc;  // Gibt an, ob der Effect rauf, oder runter gezählt wird
  bool IsWorking;  // Gibt an, ob das Gebäude grade arbeitet
  bool Transfer;   // Gibt an, ob ein Transfer statfinden soll
  int MetalProd,OilProd,GoldProd; // Produktion des gebäudes
  int MaxMetalProd,MaxOilProd,MaxGoldProd; // Maximal mögliche Produktion
  int dir;         // Frame des Gebäudes
  bool Attacking;  // Gibt an, ob das Building gerade angreift
  TList *BuildList; // Die Bauliste der Fabrik
  int BuildSpeed;  // Die baugeschwindigkeit der Fabrik
  bool RepeatBuild; // Gibt an, ob der Bau wiederholt werden soll
  bool LoadActive; // Gibt an, ob ein Vehicle geladen werden soll
  TList *StoredVehicles; // Liste mit geladenen Vehicles
  int VehicleToActivate; // Nummer des Vehicles, dass aktiviert werden soll
  bool ActivatingVehicle; // Gibt an, ob ein Vehicle aktiviert werden soll
  bool detected;   // Merker, ob die Mine entdeckt wurde (vom aktiven Spieler)
  int DamageFXPointX,DamageFXPointY,DamageFXPointX2,DamageFXPointY2; // Die Punkte, an denen Rauch bei beschädigung aufsteigen wird
  int Disabled;    // Gibt an, für wie lange diese Einheit disabled ist
  bool IsLocked;   // Gibt an, ob dieses Building in irgend einer Log-Liste ist

  void Draw(SDL_Rect *dest);
  void Select(void);
  void Deselct(void);
  void ShowDetails(void);
  void GenerateName(void);
  int PlayStram(void);
  char *GetStatusStr(void);
  void DrawSymbol(eSymbols sym,int x,int y,int maxx,int value,int maxvalue,SDL_Surface *sf);
  void DrawNumber(int x,int y,int value,int maxvalue,SDL_Surface *sf);
  void RefreshData(void);
  void ShowHelp(void);
  void DrawSymbolBig(eSymbolsBig sym,int x,int y,int maxx,int value,int orgvalue,SDL_Surface *sf);
  void Center(void);
  void DrawMunBar(void);
  void DrawHelthBar(void);
  int GetScreenPosX(void);
  int GetScreenPosY(void);
  int CalcHelth(int damage);
  void DrawMenu(void);
  int GetMenuPointAnz(void);
  SDL_Rect GetMenuSize(void);
  bool MouseOverMenu(int mx,int my);
  void SelfDestructionMenu(void);
  void ShowBigDetails(void);
  void CheckNeighbours(void);
  void DrawConnectors(SDL_Rect dest);
  bool StartWork(bool engine_call=false);
  void StopWork(bool override,bool engine_call=false);
  bool CanTransferTo(struct sGameObjects *go);
  void ShowTransfer(sGameObjects *target);
  void DrawTransBar(int len);
  void MakeTransBar(int *trans,int MaxTarget,int Target);
  void CheckRessourceProd(void);
  void ShowMineManager(void);
  void MakeMineBars(int MaxM,int MaxO,int MaxG,int *FreeM,int *FreeO,int *FreeG);
  void DrawMineBar(int typ,int value,int max_value,int offy,bool number,int fixed);
  bool IsInRange(int off);
  bool CanAttackObject(int off,bool override=false);
  void DrawAttackCursor(struct sGameObjects *go,int can_attack);
  void RotateTo(int Dir);
  void ShowBuildMenu(void);
  void ShowBuildList(TList *list,int selected,int offset,bool beschreibung,int *buildspeed);
  void DrawBuildButtons(int speed);
  void ShowToBuildList(TList *list,int selected,int offset);
  void DrawExitPoints(sVehicle *typ);
  bool CanExitTo(int off,sVehicle *typ);
  bool CanLoad(int off);
  void StoreVehicle(int off);
  void ShowStorage(void);
  void DrawStored(int off);
  void ShowStorageMetalBar(void);
  void ExitVehicleTo(int nr,int off,bool engine_call);
  void MakeStorageButtonsAlle(bool *AlleAufladenEnabled,bool *AlleReparierenEnabled,bool *AlleUpgradenEnabled);
  void Detonate(void);
  void ShowResearch(void);
  void ShowResearchSchieber(void);
  void MakeResearchSchieber(int x,int y);
  void ShowUpgrade(void);
  void ShowUpgradeList(TList *list,int selected,int offset,bool beschreibung);
  void ShowGoldBar(int StartCredits);
  void MakeUpgradeSliderVehicle(sUpgrades *u,int nr);
  void MakeUpgradeSliderBuilding(sUpgrades *u,int nr);
  void CreateUpgradeList(TList *selection,TList *images,int *selected,int *offset);
  void MakeUpgradeSubButtons(void);
  int CalcPrice(int value,int org, int variety);
  int CalcSteigerung(int org, int variety);
  void SendUpdateStored(int index); 
};

#endif

