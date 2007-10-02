//////////////////////////////////////////////////////////////////////////////
// M.A.X. - main.h
//////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4996)
#pragma warning(disable:4244)
#pragma warning(disable:4800)
#pragma warning(disable:4005)

#pragma warning(disable:4311)
#pragma warning(disable:4312)
#pragma warning(disable:4305)
#pragma warning(disable:4313)
#pragma warning(disable:4804)
#ifndef mainH
#define mainH

#include <iostream>
#include <time.h>
#include "defines.h"
#include <SDL.h>
#include "tinyxml.h"

using namespace std;

// Vordefinition für die TList-Struktur
class cPlayer;
class cBuilding;
class cVehicle;
class cMJobs;
class cAJobs;
class cNetMessage;
struct sUpgrades;
struct sTuple;
struct sBuildList;
struct sHUp;
struct sLanding;
struct sClientSettings;
struct sMessage;
struct sWachposten;
struct sLockElem;
struct sUpgradeStruct;
struct sSubBase;
struct sBuildStruct;
struct sFX;
struct sPathCalc;
struct sReport;

// Strukturen ////////////////////////////////////////////////////////////////
struct sTerrain{
  SDL_Surface *sf,*sf_org;   // Surfaces des terrains
  SDL_Surface *shw,*shw_org; // Surfaces des terrains im Schatten
  char *id;
  bool water;       // Gibt an, ob es Wasser ist
  bool coast;       // Gibt an, ob es ein Küstenstück ist
  bool overlay;     // Gibt an, ob es ein Overlay ist
  bool blocked;     // Gibt an, ob es blockiert ist
  int frames;       // Anzahl an Frames
};

// TList - Listen aller Art
class TList{
public:
	TList();
	string Items[1024];
	cPlayer *PlayerItems[1024];
	cBuilding *BuildItems[1024];
	cVehicle *VehicleItems[1024];
	cMJobs *MJobsItems[1024];
	cAJobs *AJobsItems[1024];
	cNetMessage *NetMessageItems[1024];

	sTuple *TupleItems[1024];
	sMessage *MessageItems[1024];
	sTerrain *TerItems[1024];
	sBuildList *BuildListItems[1024];
	sLanding *LandItems[1024];
	sHUp *HUpItems[1024];
	sWachposten *WaPoItems[1024];
	sLockElem *LockItems[1024];
	sUpgradeStruct *UpgraStrItems[1024];
	sSubBase *SubBaseItems[1024];
	sBuildStruct *BuildStructItems[1024];
	sFX *FXItems[1024];
	sPathCalc *PathCalcItems[5120];
	sReport *ReportItems[1024];
	sClientSettings *ClientSettingsItems[1024];
	int Count;

	void Add(string cStr) {Items[Count] = cStr; Count++;}
	void AddTuple(sTuple *Tuple) {TupleItems[Count] = Tuple; Count++;}
	void AddPlayer(cPlayer *Player) {PlayerItems[Count] = Player; Count++;}
	void AddBuild(cBuilding *Build) {BuildItems[Count] = Build; Count++;}
	void AddVehicle(cVehicle *Vehicle) {VehicleItems[Count] = Vehicle; Count++;}
	void AddMJobs(cMJobs *MJobs) {MJobsItems[Count] = MJobs; Count++;}
	void AddAJobs(cAJobs *AJobs) {AJobsItems[Count] = AJobs; Count++;}
	void AddNetMessage(cNetMessage *NetMessage) {NetMessageItems[Count] = NetMessage; Count++;}
	void AddMessage(sMessage *Message) {MessageItems[Count] = Message; Count++;}
	void AddTerrain(sTerrain *ter) {TerItems[Count] = ter; Count++;}
	void AddBuildList(sBuildList *BuildList) {BuildListItems[Count] = BuildList; Count++;}
	void AddHUp(sHUp *HUp) {HUpItems[Count] = HUp; Count++;}
	void AddLanding(sLanding *Land) {LandItems[Count] = Land; Count++;}
	void AddWaPo(sWachposten *WaPo) {WaPoItems[Count] = WaPo; Count++;}
	void AddLock(sLockElem *Lock) {LockItems[Count] = Lock; Count++;}
	void AddUpgraStr(sUpgradeStruct *UpgraStr) {UpgraStrItems[Count] = UpgraStr; Count++;}
	void AddSubBase(sSubBase *SubBase) {SubBaseItems[Count] = SubBase; Count++;}
	void AddBuildStruct(sBuildStruct *BuildStruct) {BuildStructItems[Count] = BuildStruct; Count++;}
	void AddFX(sFX *FX) {FXItems[Count] = FX; Count++;}
	void AddPathCalc(sPathCalc *PathCalc) {PathCalcItems[Count] = PathCalc; Count++;}
	void AddReport(sReport *Report) {ReportItems[Count] = Report; Count++;}
	void AddClientSettings(sClientSettings *ClientSettings) {ClientSettingsItems[Count] = ClientSettings; Count++;}

	void Delete(int i) { Items[i] = ""; for(int j = i; j < Count; j++) Items[j]=Items[j+1]; Count--; }
	void DeleteString(int i) { Items[i] = ""; for(int j = i; j < Count; j++) Items[j]=Items[j+1]; Count--; }
	void DeleteTuple(int i){ TupleItems[i] = 0; for(int j = i; j < Count; j++) TupleItems[j]=TupleItems[j+1]; Count--; }
	void DeleteSubBase(int i){ SubBaseItems[i] = 0; for(int j = i; j < Count; j++) SubBaseItems[j]=SubBaseItems[j+1]; Count--; }
	void DeleteMessage(int i) { MessageItems[i] = 0; for(int j = i; j < Count; j++) MessageItems[j]=MessageItems[j+1]; Count--;}
	void DeleteMJobs(int i) { MJobsItems[i] = 0; for(int j = i; j < Count; j++) MJobsItems[j]=MJobsItems[j+1]; Count--;}
	void DeletePathCalc(int i) { PathCalcItems[i] = 0; for(int j = i; j < Count; j++) PathCalcItems[j]=PathCalcItems[j+1]; Count--;}
	void DeleteBuilding(int i){ BuildItems[i] = 0; for(int j = i; j < Count; j++) BuildItems[j]=BuildItems[j+1]; Count--; }
	void DeleteVehicle(int i){ VehicleItems[i] = 0; for(int j = i; j < Count; j++) VehicleItems[j]=VehicleItems[j+1]; Count--; }
	void DeleteBuildList(int i){ BuildListItems[i] = 0; for(int j = i; j < Count; j++) BuildListItems[j]=BuildListItems[j+1]; Count--; }
	void DeleteUpgraStr(int i){ UpgraStrItems[i] = 0; for(int j = i; j < Count; j++) UpgraStrItems[j]=UpgraStrItems[j+1]; Count--; }
	void DeleteBuildStruct(int i){ BuildStructItems[i] = 0; for(int j = i; j < Count; j++) BuildStructItems[j]=BuildStructItems[j+1]; Count--; }
	void DeleteAJobs(int i){ AJobsItems[i] = 0; for(int j = i; j < Count; j++) AJobsItems[j]=AJobsItems[j+1]; Count--; }
	void DeleteNetMessage(int i) { NetMessageItems[i] = 0; for(int j = i; j < Count; j++) NetMessageItems[j]=NetMessageItems[j+1]; Count--;}
	void DeletePlayer(int i){ PlayerItems[i] = 0; for(int j = i; j < Count; j++) PlayerItems[j]=PlayerItems[j+1]; Count--; }
	void DeleteReport(int i){ ReportItems[i] = 0; for(int j = i; j < Count; j++) ReportItems[j]=ReportItems[j+1]; Count--; }
	void DeleteFX(int i){ FXItems[i] = 0; for(int j = i; j < Count; j++) FXItems[j]=FXItems[j+1]; Count--; }
	void DeleteTerrain(int i){ TerItems[i] = 0; for(int j = i; j < Count; j++) TerItems[j]=TerItems[j+1]; Count--; }
	void DeleteLanding(int i){ LandItems[i] = 0; for(int j = i; j < Count; j++) LandItems[j]=LandItems[j+1]; Count--; }
	void DeleteWaPo(int i){ WaPoItems[i] = 0; for(int j = i; j < Count; j++) WaPoItems[j]=WaPoItems[j+1]; Count--; }
	void DeleteLock(int i){ LockItems[i] = 0; for(int j = i; j < Count; j++) LockItems[j]=LockItems[j+1]; Count--; }
	void DeleteHUp(int i){ HUpItems[i] = 0; for(int j = i; j < Count; j++) HUpItems[j]=HUpItems[j+1]; Count--; }
	void DeleteClientSettings(int i){ ClientSettingsItems[i] = 0; for(int j = i; j < Count; j++) ClientSettingsItems[j]=ClientSettingsItems[j+1]; Count--; }
};

// Defines ///////////////////////////////////////////////////////////////////
#define DEFAULTVALUE_MATCHES (L"NONE")

#define MAX_VERSION        "Alpha 0.9"
#define GRID_COLOR         0x305C04 // Farbe der Gitternetzlinien
#define MINIMAP_COLOR      0xFC0000 // Farbe des Rahmens in der Minimap
#define SCAN_COLOR         0xE3E300 // Farbe des Scan-Kreises
#define RANGE_GROUND_COLOR 0xE20000 // Farbe des Reichweiten Kreises für Land
#define RANGE_AIR_COLOR    0xFCA800 // Farbe des Reichweiten Kreises für Luft
#define RANGE_SHIELD_COLOR 0x9CFFA5 // Farbe des Reichweiten Kreises für Schild
#define PFEIL_COLOR        0x00FF00 // Farbe eines Pfeiles
#define PFEILS_COLOR       0x0000FF // Farbe eines speziellen Pfeiles
#define MOVE_SPEED         16       // Geschwindigkeit der Fahrzeuge
#define MSG_FRAMES         150      // Anzahl an Frames, die eine Nachricht zu sehen ist
#define DB_COM_BUFFER      20       // Anzahl der DebugCom Buffer
#define MAX_PATHFINDING    5000     // Maximale Endpunktezal fürs Pathfinding
#define PING_COUNT         100      // Anzahl der Ping Messages

// Globales //////////////////////////////////////////////////////////////////
EX int ScrollSpeed;     // Scrollgeschwindigkeit
EX bool Autosave;       // Gibt an, ob automatisch gespeichert werden soll
EX bool Animation;      // Gibt am, ob es Animationen geben soll
EX bool Schatten;       // Gibt an, ob es Schatten geben soll
EX bool ShowBeschreibung; // Gibt an, ob die Beschreibung in Build-Menüs angezeigt werden soll
EX bool DamageEffects;  // Gibt an, ob Schadenseffekte gemacht werden sollen
EX bool DamageEffectsVehicles; // Gibt an, ob Schadenseffekte bei Fahrzeugen gemacht werden sollen
EX bool MakeTracks;     // Gibt an, ob Tracks gemacht werden sollen
EX unsigned int Checksum; // Die Checksumme über alle Eigenschaften
EX string SavePath; // Verzeichnis für die Savegames
EX bool FastMode;       // Gibt an, ob das Spiel im Fast-Mode laufen soll
EX bool NoIntro;        // Gibt an, ob kein Intro abgespielt werden soll
EX string MapPath;  // Dir mit den Map-Files

EX SDL_Surface *screen ZERO; // Der Bildschirm
EX SDL_Surface *buffer ZERO; // Der Bildschirm-Buffer
EX bool sound;          // true=Sound an
EX int MusicVol,SoundVol,VoiceVol;     // Lautsärken der Sounds
EX bool MusicMute,SoundMute,VoiceMute; // Muteeigenschaften der Sounds
EX string LastIP;   // Letzte eingegebene IP
EX int LastPort;        // Letzter eingegebener Port
EX string LastPlayerName; // Letzter benutzter Spielername
EX int ScreenW;         // Breite der ausgwählten Auflösung
EX int ScreenH;         // Höhe der ausgwählten Auflösung
EX char MichaelMoench[25]; // String mit dem Inhalt 'Michael Mönch'
EX bool Alpha;          // Gibt an, ob es Alphaeffekte geben soll
EX bool WindowMode;

// GFX ///////////////////////////////////////////////////////////////////////
EX SDL_Surface *gfx_hud ZERO;
EX SDL_Surface *gfx_Chand ZERO;
EX SDL_Surface *gfx_Cno ZERO;
EX SDL_Surface *gfx_Cselect ZERO;
EX SDL_Surface *gfx_Cmove ZERO;
EX SDL_Surface *gfx_Chelp ZERO;
EX SDL_Surface *gfx_Cattack ZERO;
EX SDL_Surface *gfx_Cpfeil1 ZERO;
EX SDL_Surface *gfx_Cpfeil2 ZERO;
EX SDL_Surface *gfx_Cpfeil3 ZERO;
EX SDL_Surface *gfx_Cpfeil4 ZERO;
EX SDL_Surface *gfx_Cpfeil6 ZERO;
EX SDL_Surface *gfx_Cpfeil7 ZERO;
EX SDL_Surface *gfx_Cpfeil8 ZERO;
EX SDL_Surface *gfx_Cpfeil9 ZERO;
EX SDL_Surface *gfx_hud_stuff ZERO;
EX SDL_Surface *gfx_praefer ZERO;
EX SDL_Surface *gfx_shadow ZERO;
EX SDL_Surface *gfx_tmp ZERO;
EX SDL_Surface *gfx_help_screen ZERO;
EX SDL_Surface *gfx_object_menu ZERO;
EX SDL_Surface *gfx_destruction ZERO;
EX SDL_Surface *gfx_destruction_glas ZERO;
EX SDL_Surface *gfx_build_screen ZERO;
EX SDL_Surface *gfx_Cband ZERO;
EX SDL_Surface *gfx_band_small ZERO;
EX SDL_Surface *gfx_band_big ZERO;
EX SDL_Surface *gfx_band_small_org ZERO;
EX SDL_Surface *gfx_band_big_org ZERO;
EX SDL_Surface *gfx_big_beton_org ZERO;
EX SDL_Surface *gfx_big_beton ZERO;
EX SDL_Surface *gfx_Ctransf ZERO;
EX SDL_Surface *gfx_transfer ZERO;
EX SDL_Surface *gfx_mine_manager ZERO;
EX SDL_Surface *gfx_fac_build_screen ZERO;
EX SDL_Surface *gfx_Cload ZERO;
EX SDL_Surface *gfx_Cactivate ZERO;
EX SDL_Surface *gfx_storage ZERO;
EX SDL_Surface *gfx_storage_ground ZERO;
EX SDL_Surface *gfx_editor ZERO;
EX SDL_Surface *gfx_dialog ZERO;
EX SDL_Surface *gfx_edock ZERO;
EX SDL_Surface *gfx_Cmuni ZERO;
EX SDL_Surface *gfx_Crepair ZERO;
EX SDL_Surface *gfx_research ZERO;
EX SDL_Surface *gfx_upgrade ZERO;
EX SDL_Surface *gfx_panel_top ZERO;
EX SDL_Surface *gfx_panel_bottom ZERO;
EX SDL_Surface *gfx_Csteal ZERO;
EX SDL_Surface *gfx_Cdisable ZERO;
EX SDL_Surface *gfx_menu_stuff ZERO;

// New GFX
EX SDL_Surface *gfx_player_pc ZERO;
EX SDL_Surface *gfx_player_human ZERO;
EX SDL_Surface *gfx_player_none ZERO;
EX SDL_Surface *gfx_player_select ZERO;
EX SDL_Surface *gfx_load_save_menu ZERO;
EX SDL_Surface *gfx_exitpoints_org ZERO;
EX SDL_Surface *gfx_exitpoints ZERO;
/*EX SDL_Surface *gfx_build_finished_org ZERO;
EX SDL_Surface *gfx_build_finished ZERO;*/
EX SDL_Surface *gfx_menu_buttons ZERO;

// GFX On Demand /////////////////////////////////////////////////////////////
#define GFXOD_MAIN          "gfx_od\\main.pcx"
#define GFXOD_OPTIONS       "gfx_od\\options.pcx"
#define GFXOD_PLANET_SELECT "gfx_od\\planet_select.pcx"
#define GFXOD_HANGAR        "gfx_od\\hangar.pcx"
#define GFXOD_MULT          "gfx_od\\mult.pcx"
#define GFXOD_PLAYER_SELECT "customgame_menu.pcx"
EX string DialogPath;
EX string Dialog2Path;
EX string Dialog3Path;

// Zeiger auf Surfaces ///////////////////////////////////////////////////////
EX SDL_Surface *ptr_small_beton ZERO;
EX SDL_Surface *ptr_connector ZERO;
EX SDL_Surface *ptr_connector_shw ZERO;

// Nummern von Buildings /////////////////////////////////////////////////////
EX int BNrLandMine ZERO;
EX int BNrSeaMine ZERO;
EX int BNrMine ZERO;
EX int BNrSmallGen ZERO;
EX int BNrOilStore ZERO;

// Initwerte für die Forschung ///////////////////////////////////////////////
#ifdef __main__
int ResearchInits[8]={16,16,33,8,8,16,33,33};
#else
extern int ResearchInits[8];
#endif

// FX ////////////////////////////////////////////////////////////////////////
EX SDL_Surface **fx_explo_small0 ZERO;
EX SDL_Surface **fx_explo_small1 ZERO;
EX SDL_Surface **fx_explo_small2 ZERO;
EX SDL_Surface **fx_explo_big0 ZERO;
EX SDL_Surface **fx_explo_big1 ZERO;
EX SDL_Surface **fx_explo_big2 ZERO;
EX SDL_Surface **fx_explo_big3 ZERO;
EX SDL_Surface **fx_explo_big4 ZERO;
EX SDL_Surface **fx_muzzle_big ZERO;
EX SDL_Surface **fx_muzzle_small ZERO;
EX SDL_Surface **fx_muzzle_med ZERO;
EX SDL_Surface **fx_hit ZERO;
EX SDL_Surface **fx_smoke ZERO;
EX SDL_Surface **fx_rocket ZERO;
EX SDL_Surface **fx_dark_smoke ZERO;
EX SDL_Surface **fx_tracks ZERO;
EX SDL_Surface **fx_corpse ZERO;
EX SDL_Surface **fx_absorb ZERO;

// Resourcen /////////////////////////////////////////////////////////////////
EX SDL_Surface *res_metal_org ZERO;
EX SDL_Surface *res_metal ZERO;
EX SDL_Surface *res_oil_org ZERO;
EX SDL_Surface *res_oil ZERO;
EX SDL_Surface *res_gold_org ZERO;
EX SDL_Surface *res_gold ZERO;

// Terrain ///////////////////////////////////////////////////////////////////
EX struct sTerrain *terrain ZERO;
EX int terrain_anz ZERO;

// Vehicles //////////////////////////////////////////////////////////////////
EX struct sVehicle *vehicle ZERO;
EX int vehicle_anz ZERO;

// Buildings /////////////////////////////////////////////////////////////////
EX struct sBuilding *building ZERO;
EX int building_anz ZERO;
EX SDL_Surface *dirt_small_org ZERO;
EX SDL_Surface *dirt_small ZERO;
EX SDL_Surface *dirt_small_shw_org ZERO;
EX SDL_Surface *dirt_small_shw ZERO;
EX SDL_Surface *dirt_big_org ZERO;
EX SDL_Surface *dirt_big ZERO;
EX SDL_Surface *dirt_big_shw_org ZERO;
EX SDL_Surface *dirt_big_shw ZERO;

// Fonts /////////////////////////////////////////////////////////////////////
EX SDL_Surface *font ZERO;
EX SDL_Surface *font_small_white ZERO;
EX SDL_Surface *font_small_red ZERO;
EX SDL_Surface *font_small_green ZERO;
EX SDL_Surface *font_small_yellow ZERO;
EX SDL_Surface *font_big ZERO;
EX SDL_Surface *font_big_gold ZERO;

// Farben ////////////////////////////////////////////////////////////////////
EX SDL_Surface **colors ZERO;
EX SDL_Surface **ShieldColors ZERO;
#define cl_red 0
#define cl_blue 1
#define cl_green 2
#define cl_grey 3
#define cl_orange 4
#define cl_yellow 5
#define cl_purple 6
#define cl_aqua 7

// Pfeile für Wegpunkte //////////////////////////////////////////////////////
EX SDL_Surface *WayPointPfeile[8][60];
EX SDL_Surface *WayPointPfeileSpecial[8][60];

// Vordekleration ////////////////////////////////////////////////////////////

bool GetXMLBool(TiXmlNode* rootnode,const char *nodename);
int LoadGFX();
void DeleteGFX(void);
int LoadFX();
void DeleteFX(void);
int LoadTerrain();
void DeleteTerrain(void);
int LoadFonts();
void DeleteFonts(void);
int LoadMusic();
void DeleteMusic(void);
int LoadSounds();
void DeleteSounds(void);
int LoadVoices();
void DeleteVoices(void);
int LoadBuildings();
void DeleteBuildings(void);
int LoadVehicles();
void DeleteVehicles(void);

int LoadData(void *);
void MakeLog(char* sztxt,bool ok,int pos);
bool LoadInfantery(sVehicle *v,string path);
int random(int x, int y);
bool teststr(char *s1,char *s2);
int InitSound();
void ScaleSurface(SDL_Surface *scr,SDL_Surface **dest,int size);
void ScaleSurface2(SDL_Surface *scr,SDL_Surface *dest,int size);
void ScaleSurfaceAdv(SDL_Surface *scr,SDL_Surface **dest,int sizex,int sizey);
void ScaleSurfaceAdv2(SDL_Surface *scr,SDL_Surface *dest,int sizex,int sizey);
void ScaleSurfaceAdv2Spec(SDL_Surface *scr,SDL_Surface *dest,int sizex,int sizey);
SDL_Surface *CreatePfeil(int p1x,int p1y,int p2x,int p2y,int p3x,int p3y,unsigned int color,int size);
void line(int x1,int y1,int x2,int y2,unsigned int color,SDL_Surface *sf);
void MakeShieldColor(SDL_Surface **dest,SDL_Surface *scr);
double Round(double num, unsigned int n);

#endif
