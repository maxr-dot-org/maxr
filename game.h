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
#ifndef gameH
#define gameH
#include "defines.h"
#include "SDL.h"
#include "hud.h"
#include "map.h"
#include "player.h"
#include "SDL_flic.h"
#include "engine.h"

// Objekttypen für das Savegame //////////////////////////////////////////////
#define SAVE_RES      0
#define SAVE_VEHICLE  1
#define SAVE_BUILDING 2

// Die Message-Struktur //////////////////////////////////////////////////////
struct sMessage{
  char *msg;
  int chars,len;
  unsigned int age;
};

// Der FX-Enum ///////////////////////////////////////////////////////////////
enum eFXTyps {fxMuzzleBig,fxMuzzleSmall,fxMuzzleMed,fxMuzzleMedLong,fxExploSmall,fxExploBig,fxExploAir,fxExploWater,fxHit,fxSmoke,fxRocket,fxDarkSmoke,fxTorpedo,fxTracks,fxBubbles,fxCorpse,fxAbsorb};
// Struktur für Zusatzinfos der Rocket:
struct sFXRocketInfos{
  int ScrX,ScrY;
  int DestX,DestY;
  int dir;
  float fpx,fpy,mx,my;
  cAJobs *aj;
};
// Struktur für Zusatzinfos des DarkSmoke:
struct sFXDarkSmoke{
  int alpha;
  float fx,fy;
  float dx,dy;
};
// Struktur für Zusatzinfos der Tracks:
struct sFXTracks{
  int alpha;
  int dir;
};

// Die FX-Struktur ///////////////////////////////////////////////////////////
struct sFX{
  eFXTyps typ;
  int PosX,PosY;
  int StartFrame;
  int param;
  sFXRocketInfos* rocketInfo;
  sFXDarkSmoke* smokeInfo;
  sFXTracks* trackInfo;
};

// Die Game-Klasse ///////////////////////////////////////////////////////////
class cGame
{
	void *WaitForReturnData( int iDelay );
public:
	cGame(cTCP *network, cMap *map);
	~cGame(void);

	// Variablen:
	cList<cPlayer*> *PlayerList;     // Liste der Player.
	string PlayerCheat;	 // Wenn ein Spieler ein Cheat eingegeben hat.
	cPlayer *ActivePlayer; // Der aktive Player.
	cMap *map;             // Die Karte.
	cHud *hud;             // Das Hud.
	cEngine *engine;       // Die Game-Engine.
	unsigned int FPSstart;        // Zeit für die fps Berechnung.
	unsigned int Comstart;        // Zeit für die Com-Infos.  
	bool DebugFPS;         // Frames pro Sekunde anzeigen.
	bool DebugCom;         // Com-Infos anzeigen.
	bool DebugBase;        // Basis infos anzeigen.
	bool DebugWache;       // Wachinfos anzeigen.
	bool DebugFX;          // FX-Infos anzeigen.
	bool DebugTrace;       // Zeigt Trace-Infos an.
	bool DebugLog;         // Loggt alle Nachrichten der Engine mit.
	bool ShowLog;          // Gibt an, ob die Logs angezeigt werden sollen.
	unsigned int frames;   // Anzahl der Frames.
	unsigned int cycles;   // Anzahl der Cycles.
	SDL_TimerID TimerID;   // ID des Timers.
	unsigned int Frame;    // Framezähler für Annimationen.
	cVehicle *SelectedVehicle;   // Das gerade ausgewählte Vehice.
	cBuilding *SelectedBuilding; // Das gerade ausgewählte Building.
	int ObjectStream;          // Der Sound-Stream des ausgewählten Objektes
	sGameObjects *OverObject;  // Das Objekt unter der Maus.
	unsigned int BlinkColor;   // Die aktuelle Blink-Farbe.
	FLI_Animation *FLC;    // Die FLC-Animation.
	string FLCname;    // Name der FLC-Animation.
	SDL_Surface *video;    // Videosurface für Gebäude.
	bool HelpActive;       // Gibt an, ob der Help-Cursor angezeigt werden soll.
	bool ChangeObjectName; // Gibt an, ob gerade der Name eines Objektes geändert wird.
	bool ChatInput;        // Gibt an, ob gerade eine Chat-Nachricht eingegeben werden soll.
	cList<sMessage*> *messages;       // Liste mit allen Nachrichten.
	int DebugIndex;        // Index im Buffer.
	int ComAvgSend;        // Durchschitt der gesendeten Bytes.
	int ComAvgRead;        // Durchschitt der empfangenen Bytes.
	int Runde;             // Nummer der aktuellen Runde.
	bool WantToEnd;        // Gibt an, ob die Runde beendet werden soll, sobald alle Bewegungen abgeschlossen sind.
	cList<sFX*> *FXList,*FXListBottom; // Liste mit FX-Effekten.
	cBuilding *DirtList;   // Liste mit dem Dreck.
	float WindDir;         // Richtung, aus der der Wind kommt (0-2pi).
	bool UpShowTank,UpShowPlane,UpShowShip,UpShowBuild,UpShowTNT; // Flags, was in der Raffinerie angezeigt werden soll.
	int MsgCoordsX,MsgCoordsY; // Koordinaten zu einer wichtigen Nachricht
	bool AlienTech;        // Gibt an, ob die AlienTechs aktiv sind.
	bool Defeated;         // Gib an, ob man besiegt wurde.
	bool HotSeat;          // Gibt an, ob dies ein HotSeat-Spiel ist.
	int HotSeatPlayer;     // Nummer des aktiven Hot-Seat-Spielers.
	bool PlayRounds;       // Gibt an, ob in Runden gespielt werden soll (nur MP).
	int ActiveRoundPlayerNr; // Nummer des aktiven Spielers in einem Runden-Spiel.
	string SaveLoadFile;   // Name of the savegame to load or to save
	int SaveLoadNumber;   // Index number of the savegame to load or to save
	bool End;
	void *ReturnData;

	// Flags:
	bool fDrawHud;  // Gibt an, ob das Hud gemalt werden muss.
	bool fDrawMap;  // Gibt an, ob die Map gemalt werden muss.
	bool fDraw;     // Gibt an, ob irgendwas gemalt werden muss.
	bool fDrawMMap; // Gibt an, ob die Minimap neu gezeichnet werden soll.

	// Funktionen:
	void Init(cList<cPlayer*> *Player,int APNo);
	void Run(void);
	int CheckUser(void);
	void DrawMap(bool pure=false);
	void HandleTimer(void);
	void DrawMiniMap(bool pure=false,SDL_Surface *sf=NULL);
	void RotateBlinkColor(void);
	void DrawFLC(void);
	void HandleMessages(void);
	void AddMessage(const char *msg);
	void AddMessage(std::string msg);
	bool DoCommand(char *cmd);
	void ShowDebugComGraph(int Off);
	void AddDebugComGraph(int Send,int Read);
	void AddFX(eFXTyps typ,int x,int y,int param);
	void AddFX ( eFXTyps typ,int x,int y, sFXRocketInfos* param );
	void AddFX ( sFX* n );
	void DisplayFX(void);
	void DisplayFXBottom(void);  
	void DrawFX(int i);
	void DrawFXBottom(int i);  
	void AddDirt(int x,int y,int value,bool big);
	void DrawExitPoint(int x,int y);
	void DeleteDirt(cBuilding *ptr);
	void SetWind(int dir);
	void MakePanel(bool open);
	void AddCoords(const char *msg,int x,int y);
	void AddCoords(const string sMsg, int x, int y);
	void MakeLanding(int x,int y,cPlayer *p,cList<sLanding*> *list,bool fixed);
	bool Save(string name, int iNumber);
	void Load(string name,int AP,bool MP=false);
	bool CheckRecursivLoaded(cVehicle *v,cList<cVehicle*> *StoredVehicles);
	void MakeAutosave(void);
	void Trace(void);
	void TraceVehicle(cVehicle *v,int *y,int x);
	void TraceBuilding(cBuilding *b,int *y,int x);
	bool MakeHotSeatEnde(void);

	cMJobs *pushMoveJobRequest( int ScrOff,int DestOff,bool plane,bool suspended=false );
};

// Das Game-Objekt ///////////////////////////////////////////////////////////
EX cGame *game ZERO;

// Timer /////////////////////////////////////////////////////////////////////
Uint32 Timer(Uint32 interval, void *param);
EX unsigned int TimerTime; // Wird vom Timer incrementiert.
EX int timer0,timer1,timer2;

// Prototypen ////////////////////////////////////////////////////////////////
void MouseMoveCallback(bool force);
void DrawCircle(int x,int y,int r,int color,SDL_Surface *sf);
void SpecialCircle(int x,int y,int r,char *map);
void SpecialCircleBig(int x,int y,int r,char *map);

#endif
