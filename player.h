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
#ifndef playerH
#define playerH
#include "defines.h"
#include "main.h"
#include <SDL.h>
#include "hud.h"
#include "buildings.h"
#include "vehicles.h"
#include "base.h"
#include "map.h"

struct sVehicle;
struct sUnitData;
struct sBuilding;
class cVehicle;
class cBuilding;
class cEngine;

struct sSentry
{
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

friend class cServer;
friend class cClient;
public:
  cPlayer(string Name,SDL_Surface *Color,int nr, int iSocketNum = -1 );
  ~cPlayer(void);
  cPlayer(const cPlayer &Player);

  string name;
  SDL_Surface *color;
  int Nr;

  sUnitData *VehicleData; // Daten aller Vehicles für diesen Player.
  cVehicle *VehicleList;     // Liste aller Vehicles des Spielers.
  sUnitData *BuildingData; // Daten aller Buildings für diesen Player.
  cBuilding *BuildingList;     // Liste aller Buildings des Spielers.
  int MapSize;               // Kartengröße
  char *ScanMap;             // Map mit dem Scannerflags.
  char *ResourceMap;         // Map mit aufgedeckten Resourcen. / Map with explored resources.
  cBase base;               // Die Basis dieses Spielers.
  cList<sSentry*> SentriesAir;		/** list with all units on sentry that can attack planes */
  char *SentriesMapAir;				/** the covered air area */
  cList<sSentry*> SentriesGround;	/** list with all units on sentry that can attack ground units */
  char *SentriesMapGround;			/** the covered ground area */
  char *DetectLandMap;       // Map mit den Gebieten, die an Land gesehen werden können.
  char *DetectSeaMap;        // Map mit den Gebieten, die im Wasser gesehen werden können.
  char *DetectMinesMap;				/** the area where the player can detect mines */
  sResearch ResearchTechs[8];// Map mit den erforschten Technologien.
  int ResearchCount;         // Anzahl an Forschungszentren (die arbeiten).
  int UnusedResearch;        // Nicht benutzte Forschungskapazitäten.
  int Credits;               // Anzahl der erworbenen Credits.
  cHud HotHud;               // Gespeichertes Hud für Hot-Seat-Spiele.
  cList<sTurnstartReport*> ReportVehicles; // Reportlisten.
  cList<sTurnstartReport*> ReportBuildings; // Reportlisten.
  bool ReportForschungFinished; // Merker, ob Forschung abgeschlossen ist.
  cList<sLockElem*> LockList;           // Liste mit gelockten Objekten.
  int iSocketNum;			// Number of socket over which this player is connected in network game
							// if MAX_CLIENTS its the lokal connected player; -1 for unknown
  bool bFinishedTurn;			//true when player send his turn end
  bool isDefeated;			// true if the player has been defeated

  void InitMaps(int MapSizeX, cMap *map = NULL ); // TODO: remove ' = NULL'
  void DoScan(void);
  cVehicle *GetNextVehicle(void);
  cVehicle *GetPrevVehicle(void);
  void addSentryVehicle( cVehicle *v );
  void addSentryBuilding( cBuilding *b );
  void deleteSentryVehicle( cVehicle *v );
  void deleteSentryBuilding( cBuilding *b) ;
  void refreshSentryAir();
  void refreshSentryGround();
  void StartAResearch(void);
  void StopAReserach(void);
  void DoResearch(void);
  void DoTheResearch(int i);
  void AddLock(cBuilding *b);
  void AddLock(cVehicle *v);
  void DeleteLock(cBuilding *b);
  void DeleteLock(cVehicle *v);
  bool InLockList(cBuilding *b);
  bool InLockList(cVehicle *v);
  void ToggelLock(cMapField *OverUnitField);
  void DrawLockList(cHud const&);
	/**
	* draws a circle on the map for the fog
	*@author alzi alias DoctorDeath
	*@param iX X coordinate to the center of the circle
	*@param iY Y coordinate to the center of the circle
	*@param iRadius radius of the circle
	*@param map map were to store the data of the circle
	*/
	void drawSpecialCircle( int iX, int iY, int iRadius, char *map, int mapsize );
	/**
	* draws a big circle on the map for the fog
	*@author alzi alias DoctorDeath
	*@param iX X coordinate to the center of the circle
	*@param iY Y coordinate to the center of the circle
	*@param iRadius radius of the circle
	*@param map map were to store the data of the circle
	*/
	void drawSpecialCircleBig( int iX, int iY, int iRadius, char *map, int mapsize );

private:
  cVehicle *AddVehicle( int posx, int posy, sVehicle *v );
  cBuilding *addBuilding( int posx, int posy, sBuilding *b );
};

#endif
