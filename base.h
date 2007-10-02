//////////////////////////////////////////////////////////////////////////////
// M.A.X. - base.h
//////////////////////////////////////////////////////////////////////////////
#ifndef baseH
#define baseH
#include "defines.h"
#include "SDL.h"
#include "player.h"
#include "buildings.h"

class cPlayer;
class cBuilding;
class cMap;

// Die SubBase Struktur //////////////////////////////////////////////////////
struct sSubBase{
  TList *buildings;

  int MaxMetal;
  int Metal;
  int MaxOil;
  int Oil;
  int MaxGold;
  int Gold;

  int MaxEnergyProd;
  int EnergyProd;
  int MaxEnergyNeed;
  int EnergyNeed;
  int MetalNeed;
  int OilNeed;
  int GoldNeed;
  int MaxMetalNeed;
  int MaxOilNeed;
  int MaxGoldNeed;

  int MetalProd;
  int OilProd;
  int GoldProd;

  int HumanProd;
  int HumanNeed;
  int MaxHumanNeed;
};


// Die Base Klasse ///////////////////////////////////////////////////////////
class cBase{
public:
  cBase(cPlayer *Owner);
  ~cBase(void);

  cPlayer *owner;
  TList *SubBases;
  cMap *map;

  void AddBuilding(cBuilding *b);
  void DeleteBuilding(cBuilding *b);
  void AddBuildingToSubBase(cBuilding *b,sSubBase *sb);
  void AddMetal(sSubBase *sb,int value);
  void AddOil(sSubBase *sb,int value);
  void AddGold(sSubBase *sb,int value);
  void Rundenende(void);
  bool OptimizeEnergy(sSubBase *sb);
  void RefreshSubbases(void);
};

#endif
