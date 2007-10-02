//////////////////////////////////////////////////////////////////////////////
// M.A.X. - mjobs.h
//////////////////////////////////////////////////////////////////////////////
#ifndef mjobsH
#define mjobsH
#include "map.h"
#include "vehicles.h"

// Struktur für die Wegberechnung ////////////////////////////////////////////
struct sPathCalc{
  sPathCalc *prev;
  int X,Y;
  int WayCosts;
  int CostsGes;
  bool road;
};

struct sWaypoint{
  sWaypoint *next;
  int X,Y;
  int Costs;
};

// Die MJobs-Klasse //////////////////////////////////////////////////////////
class cMJobs{
public:
  cMJobs(cMap *Map,int ScrOff,int DestOff,bool Plane);
  ~cMJobs(void);

  cMap *map;
  int ScrX,ScrY,DestX,DestY;
  cMJobs *next;
  bool finished;
  bool EndForNow;
  bool ClientMove;
  cVehicle *vehicle;
  int next_dir;
  int SavedSpeed;
  bool Suspended;
  bool plane,ship;
  bool BuildAtTarget;

  char *PathCalcMap;
  sPathCalc *PathCalcRoot;
  TList *PathCalcEnds;
  TList *PathCalcAll;
  sPathCalc *FoundEnd;

  sWaypoint *waypoints;

  void Release(void);
  bool CalcPath(void);
  int CalcDest(int x,int y);
  bool AddPoint(int x, int y, int m, sPathCalc *p);
  bool CreateNextPath(void);
  bool CheckPossiblePoint(int x,int y);
  bool CheckPointNotBlocked(int x,int y);
  int GetWayCost(int x,int y,bool *road);
  void DeleteWaypoints(void);
  void DrawPfeil(SDL_Rect dest,SDL_Rect *ldest,bool spezial);
  void CalcNextDir(void);
  void StartMove(void);
  void DoTheMove(void);
};

#endif
