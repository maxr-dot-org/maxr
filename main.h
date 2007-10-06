///////////////////////////////////////////////////////////////////////////////
//
// M.A.X. Reloaded - main.h
//
///////////////////////////////////////////////////////////////////////////////
//
// Main-declerations, -classes and structures for the game
// Contains all global varaibles needed for the game
//
///////////////////////////////////////////////////////////////////////////////

// Hides some warnings from the eye of VS users ///////////////////////////////
#ifdef _WIN32
#pragma warning(disable:4996)
#pragma warning(disable:4244)
#pragma warning(disable:4800)
#pragma warning(disable:4005)

#pragma warning(disable:4311)
#pragma warning(disable:4312)
#pragma warning(disable:4305)
#pragma warning(disable:4313)
#pragma warning(disable:4804)
#endif

#ifndef mainH
#define mainH

// Includes ///////////////////////////////////////////////////////////////////

#include <iostream>
#include <time.h>
#include <SDL.h>
#include "tinyxml.h"
#include "defines.h"

using namespace std;

// Predeclarations for the TList Class ////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
// Struktures
// ------------------------
// 
///////////////////////////////////////////////////////////////////////////////

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

// TList - All kind of Lists //////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
// Defines
// ------------------------
// 
///////////////////////////////////////////////////////////////////////////////
#define MAXVERSION "M.A.X. Reloaded v SVN..."
//#define MAXVERSION M.A.X. Reloaded v MAX_VERSION....

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
#define MAX_PATHFINDING    5000     // Maximale Endpunktezal fürs Pathfinding

// GFX On Demand /////////////////////////////////////////////////////////////
#define GFXOD_MAIN          "gfx_od\\main.pcx"
#define GFXOD_OPTIONS       "gfx_od\\options.pcx"
#define GFXOD_PLANET_SELECT "gfx_od\\planet_select.pcx"
#define GFXOD_HANGAR        "gfx_od\\hangar.pcx"
#define GFXOD_MULT          "gfx_od\\mult.pcx"
#define GFXOD_PLAYER_SELECT "customgame_menu.pcx"

// Colors /////////////////////////////////////////////////////////////////////
#define cl_red 0
#define cl_blue 1
#define cl_green 2
#define cl_grey 3
#define cl_orange 4
#define cl_yellow 5
#define cl_purple 6
#define cl_aqua 7


///////////////////////////////////////////////////////////////////////////////
// Variables-Classes
// ------------------------
// This classes are for saving global Variables needed by the game
///////////////////////////////////////////////////////////////////////////////

// Screenbuffers //////////////////////////////////////////////////////////////
EX SDL_Surface *screen ZERO;	// Der Bildschirm
EX SDL_Surface *buffer ZERO;	// Der Bildschirm-Buffer

// Nummbers of buildings //////////////////////////////////////////////////////
EX int BNrLandMine ZERO;
EX int BNrSeaMine ZERO;
EX int BNrMine ZERO;
EX int BNrSmallGen ZERO;
EX int BNrOilStore ZERO;

// Initvalues for Research ////////////////////////////////////////////////////
#ifdef __main__
int ResearchInits[8]={16,16,33,8,8,16,33,33};
#else
extern int ResearchInits[8];
#endif

///////////////////////////////////////////////////////////////////////////////
// Variables-Classes
// ------------------------
// This classes are for saving global Variables needed by the game
///////////////////////////////////////////////////////////////////////////////

// GameSettingsData - Class containing all gamesettings ///////////////////////
class cGameSettingsData
{
public:
	int ScrollSpeed;			// Scrollgeschwindigkeit
	bool Autosave;				// Gibt an, ob automatisch gespeichert werden soll
	bool Animation;				// Gibt am, ob es Animationen geben soll
	bool Schatten;				// Gibt an, ob es Schatten geben soll
	bool ShowDescription;		// Gibt an, ob die Beschreibung in Build-Menüs angezeigt werden soll
	bool DamageEffects;			// Gibt an, ob Schadenseffekte gemacht werden sollen
	bool DamageEffectsVehicles;	// Gibt an, ob Schadenseffekte bei Fahrzeugen gemacht werden sollen
	bool MakeTracks;			// Gibt an, ob Tracks gemacht werden sollen
	bool Alpha;					// Gibt an, ob es Alphaeffekte geben soll
	unsigned int Checksum;		// Die Checksumme über alle Eigenschaften
	bool FastMode;				// Gibt an, ob das Spiel im Fast-Mode laufen soll
	bool NoIntro;				// Gibt an, ob kein Intro abgespielt werden soll
	bool WindowMode;			// Gibt an, ob das Spiel im Fenster laufen soll

	bool sound;					// true=Sound an
	int MusicVol,SoundVol,VoiceVol;		// Lautsärken der Sounds
	bool MusicMute,SoundMute,VoiceMute;	// Muteeigenschaften der Sounds
	string LastIP;				// Letzte eingegebene IP
	int LastPort;				// Letzter eingegebener Port
	string LastPlayerName;		// Letzter benutzter Spielername
	int ScreenW;				// Breite der ausgwählten Auflösung
	int ScreenH;				// Höhe der ausgwählten Auflösung

	string BuildingPath;		// Path to the buildings
	string FontPath;			// Path to the fonts
	string FxPath;				// Path to the effects
	string GfxPath;				// Path to the graphics
	string GfxODPath;			// Path to the graphics on demand
	string MapPath;				// Path to the maps
	string MusicPath;			// Path to the music-files
	string SavePath;			// Path to the saves
	string SoundPath;			// Path to the sound-files
	string TerrainPath;			// Path to the terrains
	string VehiclesPath;		// Path to the vehicles
	string VoicesPath;			// Path to the voice-files
} EX GameSettingsData;

// GraphicsData - Class containing all normal graphic surfaces ////////////////
class cGraphicsData
{
public:
	SDL_Surface *gfx_hud;
	SDL_Surface *gfx_Chand;
	SDL_Surface *gfx_Cno;
	SDL_Surface *gfx_Cselect;
	SDL_Surface *gfx_Cmove;
	SDL_Surface *gfx_Chelp;
	SDL_Surface *gfx_Cattack;
	SDL_Surface *gfx_Cpfeil1;
	SDL_Surface *gfx_Cpfeil2;
	SDL_Surface *gfx_Cpfeil3;
	SDL_Surface *gfx_Cpfeil4;
	SDL_Surface *gfx_Cpfeil6;
	SDL_Surface *gfx_Cpfeil7;
	SDL_Surface *gfx_Cpfeil8;
	SDL_Surface *gfx_Cpfeil9;
	SDL_Surface *gfx_hud_stuff;
	SDL_Surface *gfx_praefer;
	SDL_Surface *gfx_shadow;
	SDL_Surface *gfx_tmp;
	SDL_Surface *gfx_help_screen;
	SDL_Surface *gfx_object_menu;
	SDL_Surface *gfx_destruction;
	SDL_Surface *gfx_destruction_glas;
	SDL_Surface *gfx_build_screen;
	SDL_Surface *gfx_Cband;
	SDL_Surface *gfx_band_small;
	SDL_Surface *gfx_band_big;
	SDL_Surface *gfx_band_small_org;
	SDL_Surface *gfx_band_big_org;
	SDL_Surface *gfx_big_beton_org;
	SDL_Surface *gfx_big_beton;
	SDL_Surface *gfx_Ctransf;
	SDL_Surface *gfx_transfer;
	SDL_Surface *gfx_mine_manager;
	SDL_Surface *gfx_fac_build_screen;
	SDL_Surface *gfx_Cload;
	SDL_Surface *gfx_Cactivate;
	SDL_Surface *gfx_storage;
	SDL_Surface *gfx_storage_ground;
	SDL_Surface *gfx_editor;
	SDL_Surface *gfx_dialog;
	SDL_Surface *gfx_edock;
	SDL_Surface *gfx_Cmuni;
	SDL_Surface *gfx_Crepair;
	SDL_Surface *gfx_research;
	SDL_Surface *gfx_upgrade;
	SDL_Surface *gfx_panel_top;
	SDL_Surface *gfx_panel_bottom;
	SDL_Surface *gfx_Csteal;
	SDL_Surface *gfx_Cdisable;
	SDL_Surface *gfx_menu_stuff;
	SDL_Surface *gfx_player_pc;
	SDL_Surface *gfx_player_human;
	SDL_Surface *gfx_player_none;
	SDL_Surface *gfx_player_select;
	SDL_Surface *gfx_load_save_menu;
	SDL_Surface *gfx_exitpoints_org;
	SDL_Surface *gfx_exitpoints;
	SDL_Surface *gfx_menu_buttons;

	string DialogPath;
	string Dialog2Path;
	string Dialog3Path;
} EX GraphicsData;

// Effects - Class containing all effect surfaces /////////////////////////////
class cEffectsData
{
public:
	SDL_Surface **fx_explo_small0;
	SDL_Surface **fx_explo_small1;
	SDL_Surface **fx_explo_small2;
	SDL_Surface **fx_explo_big0;
	SDL_Surface **fx_explo_big1;
	SDL_Surface **fx_explo_big2;
	SDL_Surface **fx_explo_big3;
	SDL_Surface **fx_explo_big4;
	SDL_Surface **fx_muzzle_big;
	SDL_Surface **fx_muzzle_small;
	SDL_Surface **fx_muzzle_med;
	SDL_Surface **fx_hit;
	SDL_Surface **fx_smoke;
	SDL_Surface **fx_rocket;
	SDL_Surface **fx_dark_smoke;
	SDL_Surface **fx_tracks;
	SDL_Surface **fx_corpse;
	SDL_Surface **fx_absorb;
} EX EffectsData;

// ResourceData - Class containing all resource surfaces //////////////////////
class cResourceData
{
public:
	SDL_Surface *res_metal_org;
	SDL_Surface *res_metal;
	SDL_Surface *res_oil_org;
	SDL_Surface *res_oil;
	SDL_Surface *res_gold_org;
	SDL_Surface *res_gold;
} EX ResourceData;

// TerrainData - Class containing all terrain data ////////////////////////////
class cTerrainData
{
public:
	sTerrain *terrain;
	int terrain_anz;
} EX TerrainData;

// VehicleData - Class containing all vehicle surfaces & data /////////////////
class cVehicleData
{
public:
	struct sVehicle *vehicle;
	int vehicle_anz;
} EX VehicleMainData;

// BuildingData - Class containing all building surfaces & data ///////////////
class cBuildingData
{
public:
	struct sBuilding *building;
	int building_anz;

	SDL_Surface *dirt_small_org;
	SDL_Surface *dirt_small;
	SDL_Surface *dirt_small_shw_org;
	SDL_Surface *dirt_small_shw;
	SDL_Surface *dirt_big_org;
	SDL_Surface *dirt_big;
	SDL_Surface *dirt_big_shw_org;
	SDL_Surface *dirt_big_shw;

	SDL_Surface *ptr_small_beton;
	SDL_Surface *ptr_connector;
	SDL_Surface *ptr_connector_shw;
} EX BuildingMainData;

// FontsData - Class containing all font surfaces /////////////////////////////
class cFontsData
{
public:
	SDL_Surface *font;
	SDL_Surface *font_small_white;
	SDL_Surface *font_small_red;
	SDL_Surface *font_small_green;
	SDL_Surface *font_small_yellow;
	SDL_Surface *font_big;
	SDL_Surface *font_big_gold;
} EX FontsData;

// OtherData - Class containing the rest of surfaces //////////////////////////
class cOtherData
{
public:
	SDL_Surface **colors;
	SDL_Surface **ShieldColors;

	SDL_Surface *WayPointPfeile[8][60];
	SDL_Surface *WayPointPfeileSpecial[8][60];
} EX OtherData;

///////////////////////////////////////////////////////////////////////////////
// Predeclerations
// ------------------------
// This classes are for saving global Variables needed by the game
///////////////////////////////////////////////////////////////////////////////

int InitSound();
int LoadData(void *);
void MakeLog(char* sztxt,bool ok,int pos);
void ScaleSurface(SDL_Surface *scr,SDL_Surface **dest,int size);
void ScaleSurface2(SDL_Surface *scr,SDL_Surface *dest,int size);
void ScaleSurfaceAdv(SDL_Surface *scr,SDL_Surface **dest,int sizex,int sizey);
void ScaleSurfaceAdv2(SDL_Surface *scr,SDL_Surface *dest,int sizex,int sizey);
void ScaleSurfaceAdv2Spec(SDL_Surface *scr,SDL_Surface *dest,int sizex,int sizey);
SDL_Surface *CreatePfeil(int p1x,int p1y,int p2x,int p2y,int p3x,int p3y,unsigned int color,int size);
void line(int x1,int y1,int x2,int y2,unsigned int color,SDL_Surface *sf);
void MakeShieldColor(SDL_Surface **dest,SDL_Surface *scr);
int random(int x, int y);

/**
* Rounds given param num to specified position after decimal point<br>
* Example:<br>
* num := 3,234<br>
* n := 2<br>
* >>>>>>> Result = 3,23<br>
*
*@author MM
*@param num number to round up
*@param n the position after decimal point in dValueToRound, that will be rounded
*@return rounded num
*/
double Round(double num, unsigned int n);

/**
* Rounds given param num without numbers after decimal point<br>
* Example:<br>
* num := 3,234<br>
* >>>>>>> Result = 3<br>
*
*@author beko
*@param num number to round up
*@return rounded num
*/
int Round ( double num );

#endif
