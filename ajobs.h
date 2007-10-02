//////////////////////////////////////////////////////////////////////////////
// M.A.X. - ajobs.h
//////////////////////////////////////////////////////////////////////////////
#ifndef ajobsH
#define ajobsH
#include "defines.h"
#include "SDL.h"
#include "map.h"
#include "vehicles.h"
#include "buildings.h"

// Die AJobs-Klasse //////////////////////////////////////////////////////////
class cAJobs{
public:
  cAJobs(cMap *Map,int ScrOff,int DestOff,bool scr_air,bool dest_air,bool scr_building,bool wache=false);
  ~cAJobs(void);

  cMap *map;
  int ScrX,ScrY,DestX,DestY,dest,scr;
  cVehicle *vehicle;
  cBuilding *building;
  int FireDir;
  bool MuzzlePlayed;
  int wait;
  bool ScrAir,DestAir,ScrBuilding;
  bool Wache;
  bool MineDetonation;

  void PlayMuzzle(void);
  bool MakeImpact(void);
  void DetonateMine(void);
  void MakeClusters(void);
};

#endif
