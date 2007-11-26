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
#ifndef engineH
#define engineH
#include "defines.h"
#include "SDL.h"
#include "main.h"
#include "map.h"
#include "player.h"
#include "mjobs.h"
#include "ajobs.h"
#include "fstcpip.h"

// IDs der NET-Messages //////////////////////////////////////////////////////
#define MSG_CHAT               1  // Chatnachricht
#define MSG_ADD_MOVEJOB        2  // Einen Movejob hinzufügen
#define MSG_MOVE_VEHICLE       3  // Ein Fahrzeug umsetzen
#define MSG_MOVE_TO            4  // Ein Fahrzeug um ein Feld bewegen
#define MSG_NO_PATH            5  // Benachrichtigung über einen versperrten Pfad
#define MSG_END_MOVE           6  // Beendet die Bewegung eines Fahrzeugs
#define MSG_CHANGE_VEH_NAME    7  // Ändert den Namen eines Vehicles
#define MSG_END_MOVE_FOR_NOW   8  // Beendet die Bewegung eines Fahrzeugs für diese Runde
#define MSG_CHANGE_PLAYER_NAME 9  // Ändert den Namen eines Spielers
#define MSG_ENDE_PRESSED       10 // Benachrichtigung über einen Druck auf Ende
#define MSG_MJOB_STOP          11 // Wird vom Client übermittelt, wenn er manuell den MJob stoppt
#define MSG_ADD_ATTACKJOB      12 // Einen Attackjob einfügen
#define MSG_DESTROY_OBJECT     13 // Zerstört ein Objekt
#define MSG_ERLEDIGEN          14 // Ein MJob soll erledigt werden
#define MSG_SAVED_SPEED        15 // Teilt einem Client ein Speed Save mit
#define MSG_CHANGE_BUI_NAME    16 // Ändert den Namen eines Buildings
#define MSG_START_BUILD        17 // Startet ein Building zu bauen
#define MSG_STOP_BUILD         18 // Stopt ein Building zu bauen
#define MSG_ADD_BUILDING       19 // Fügt ein Gebäude ein
#define MSG_START_BUILD_BIG    20 // Startet den bau eines großen Gebäudes
#define MSG_RESET_CONSTRUCTOR  21 // Setzt den Constructor nach beenden des Bauens neu
#define MSG_START_CLEAR        22 // Startet das Räumen eines Feldes
#define MSG_STORE_VEHICLE      23 // Läd ein Vehicle ein
#define MSG_ACTIVATE_VEHICLE   24 // Aktiviert ein geladenes Vehicle wieder
#define MSG_START_WORK         25 // Startet ein Gebäude
#define MSG_STOP_WORK          26 // Stoppt ein Gebäude
#define MSG_ADD_VEHICLE        27 // Erzeugt ein vehicle
#define MSG_REPAIR             28 // Repariert etwas
#define MSG_RELOAD             29 // Läd etwas nach
#define MSG_WACHE              30 // Ändert den Wachstatus eines Objektes
#define MSG_CLEAR_MINE         31 // Räumt eine Mine
#define MSG_UPGRADE            32 // Upgrade eines Spielers
#define MSG_RESEARCH           33 // Meldet eine abgeschlossene Forschung
#define MSG_UPDATE_BUILDING    34 // Meldet das Upgrade eines Gebäudes
#define MSG_COMMANDO_MISTAKE   35 // Berichtet über einen Fehler eines Commandos
#define MSG_COMMANDO_SUCCESS   36 // Meldet einen Erfolg eines Commandos
#define MSG_START_SYNC         37 // Fordert die Clients zum Synchronisieren auf
#define MSG_SYNC_PLAYER        38 // Sync Player
#define MSG_SYNC_VEHICLE       39 // Sync Vehicle
#define MSG_SYNC_BUILDING      40 // Sync Building
#define MSG_UPDATE_STORED      41 // Aktualisiert ein gespeichertes Vehicle
#define MSG_REPORT_R_E_A       42 // Berichtet über das Ende der RundenendeActions
#define MSG_PING               43 // Anforderung eines Pong
#define MSG_PONG               44 // Rückgabe des Pong
#define MSG_PLAYER_DEFEAT      45 // Medet die Niederlage eines Spielers
#define MSG_HOST_DEFEAT        46 // Meldet die Niederlage des Hostes
#define MSG_PLAY_ROUNDS_NEXT   47 // Nachricht, dass der nächste Spieler dran ist

// Nur für das MP-Menü:
#define MSG_SIGNING_IN        100 // Anmeldenachricht eines Spielers
#define MSG_YOUR_ID_IS        101 // Teilt dem Client nach dem Anmelden seine ID mit
#define MSG_MY_NAME_CHANGED   102 // Benachrichtigung wenn ein Client seinen Namen/Farbe ändert
#define MSG_PLAYER_LIST       103 // Übertragng der Spielerliste
#define MSG_OPTIONS           104 // Übertragung der Optionen des Spiels
#define MSG_WHO_ARE_YOU       105 // Fordert den Client auf sich zu identifizieren
#define MSG_CHECK_FOR_GO      106 // Läßt überprüfen, ob alle Clients bereit sind
#define MSG_READY_TO_GO       107 // Meldet der Client wenn er bereit ist zum Starten
#define MSG_NO_GO             108 // Meldet der Client, wenn er nicht bereit ist
#define MSG_LETS_GO           109 // Gibt dem Client das Signal zum Start
#define MSG_RESSOURCES        110 // Überträgt die Ressourcenmap
#define MSG_PLAYER_LANDING    111 // Landedaten eines Players
#define MSG_PLAYER_UPGRADES   112 // Übeträgt alle Upgrades eines Players
#define MSG_SAVEGAME_START    113 // Start der Übertragung des Savegames
#define MSG_SAVEGAME_PART     114 // Teil der Übertragung des Savegames

// Strukturen für die Synchronisation ////////////////////////////////////////
struct sSyncPlayer{
  int PlayerID;
  bool EndOfSync;

  int Credits;
  sResearch ResearchTechs[8];
  int ResearchCount;
  int UnusedResearch;

  // Hud-Einstellungen:
  bool TNT,Radar,Nebel,Gitter,Scan,Reichweite,Munition,Treffer,Farben,Status,Studie,Lock;
  bool PlayFLC;
  int Zoom,OffX,OffY;
};

struct sSyncVehicle{
  int PlayerID;
  int off;
  bool EndOfSync;

  bool isPlane;
  bool IsBuilding;
  int BuildingTyp;
  int BuildCosts;
  int BuildRounds;
  int BuildRoundsStart;
  int BandX,BandY;
  bool IsClearing;
  int ClearingRounds;
  bool ClearBig;
  bool ShowBigBeton;
  int FlightHigh;
  bool LayMines;
  bool ClearMines;
  bool Loaded;
  int CommandoRank;
  int Disabled;
  int Ammo,Cargo;

//  int StoredVehicles;
};

struct sSyncBuilding{
  int PlayerID;
  int off;
  bool EndOfSync;

  bool isBase;
  bool IsWorking;
  int MetalProd,OilProd,GoldProd;
  int MaxMetalProd,MaxOilProd,MaxGoldProd;
  int BuildSpeed;
  bool RepeatBuild;
  int Disabled;
  int Ammo,Load;
//  TList *StoredVehicles;

  int BuildList;
};

// Strukturen für den Ping ///////////////////////////////////////////////////
struct sPing{
  int PlayerID;
  int rx_count;
  // auskommentiert beim Mainumbau
  //int rx[PING_COUNT];
  // TODO:
};

// Strukturen für die Reports ////////////////////////////////////////////////
struct sReport{
  string name;
  int anz;
};

// Defines für das Loggen von Nachrichten ////////////////////////////////////
#define LOGR(a) if(game->DebugLog){string logmsg; logmsg="RX: "; logmsg+=(string)a; LogMessage(logmsg); }
//#define LOGT(a) LogMessage((AnsiString)"TX: "+((AnsiString)a));

// Die Engine-Klasse /////////////////////////////////////////////////////////
class cEngine{
public:
  cEngine(cMap *Map, cFSTcpIp *fstcpip);
  ~cEngine(void);

  cMap *map;
  cMJobs *mjobs;
  TList *ActiveMJobs;
  cFSTcpIp *fstcpip;
  SDL_mutex *mutex;
  int EndeCount;
  TList *AJobs;
  int SyncWaiting;
  int RundenendeActionsReport;
  int SyncNo;
  TList *PingList;
  SDL_RWops *LogFile;
  TList *LogHistory;

  void AddVehicle(int posx,int posy,sVehicle *v,cPlayer *p,bool init=false,bool engine_call=false);
  void AddBuilding(int posx,int posy,sBuilding *b,cPlayer *p,bool init=false);
  void ChangeVehicleName(int posx,int posy,string name,bool override,bool plane);
  void ChangeBuildingName(int posx,int posy,string name,bool override,bool base);  
  cMJobs *AddMoveJob(int ScrOff,int DestOff,bool ClientMove,bool plane,bool suspended=false);
  void AddActiveMoveJob(cMJobs *job);
  void Reservieren(int x,int y,bool plane);
  void MoveVehicle(int FromX,int FromY,int ToX,int ToY,bool override,bool plane);
  void ChangePlayerName(string name);
  void EndePressed(int PlayerNr);
  void CheckEnde(void);
  void Rundenende(void);
  bool DoEndActions(void);
  bool CheckVehiclesMoving(bool WantToEnd);
  void CollectTrash(void);
  void AddAttackJob(int ScrOff,int DestOff,bool override,bool ScrAir,bool DestAir,bool ScrBuilding,bool Wache=false);
  void AddReport(string name,bool vehicle);
  void MakeRundenstartReport(void);

  void SendChatMessage(const char *str);
  void HandleGameMessages();
  void DestroyObject(int off,bool air);
  void Ping(void);
  void CheckDefeat(void);
  void StartLog(void);
  void StopLog(void);
  void LogMessage(string msg);
  TList* SplitMessage ( string sMsg );

  void Run(void);
};

#endif
