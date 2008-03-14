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
#include "network.h"

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
  cEngine(cMap *Map);
  ~cEngine(void);

  cMap *map;
  cMJobs *mjobs;
  cList<cMJobs*> *ActiveMJobs;
  int EndeCount;
  cList<cAJobs*> *AJobs;
  int SyncWaiting;
  int RundenendeActionsReport;
  int SyncNo;
  int PingStart;
  SDL_RWops *LogFile;
  cList<string> *LogHistory;

  void AddVehicle(int posx,int posy,sVehicle *v,cPlayer *p,bool init=false,bool engine_call=false);
  void AddBuilding(int posx,int posy,sBuilding *b,cPlayer *p,bool init=false);
  void ChangeVehicleName(int posx,int posy,string name,bool override,bool plane);
  void ChangeBuildingName(int posx,int posy,string name,bool override,bool base);  
  cMJobs *AddMoveJob(int ScrOff,int DestOff,bool ClientMove,bool plane,bool suspended=false);
  void AddActiveMoveJob(cMJobs *job);
  void Reservieren(int x,int y,bool plane);
  void MoveVehicle(int FromX,int FromY,int ToX,int ToY,bool override,bool plane);
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
  void DestroyObject(int off,bool air);
  void CheckDefeat(void);
  void StartLog(void);
  void StopLog(void);
  void LogMessage(string msg);
  cList<string> *SplitMessage ( string sMsg );

  void HandleEvent( SDL_Event *event );
  void Run(void);
};

#endif
