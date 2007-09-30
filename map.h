//////////////////////////////////////////////////////////////////////////////
// M.A.X. - map.h
//////////////////////////////////////////////////////////////////////////////
#ifndef mapH
#define mapH
#include "defines.h"
#include "main.h"
#include "vehicles.h"
#include "buildings.h"

// GameObjects Struktur //////////////////////////////////////////////////////
struct sGameObjects{
  cVehicle *vehicle;
  cVehicle *plane;
  cBuilding *base;
  cBuilding *top;  
  bool reserviert;
  bool air_reserviert;  
};

// Resources Struktur ////////////////////////////////////////////////////////
struct sResources{
  unsigned char value;
  unsigned char typ;
};
// Die Resorces-Typen:
#define RES_NONE  0
#define RES_METAL 1
#define RES_OIL   2
#define RES_GOLD  3

// Die Map-Klasse ////////////////////////////////////////////////////////////
class cMap{
public:
  cMap(void);
  ~cMap(void);

  int size;     // Größe der Karte.
  int *Kacheln; // Map mit den Kacheln.
  int DefaultWater; // Nummer des Wassers für Küsten
  sGameObjects *GO; // Feld mit den Gameobjects für einen schnelleren Zugriff.
  sResources *Resources; // Feld mit den Resourcen.
  TList *TerrainInUse; // Liste mit Zeigern auf die terrains, die benutzt werden.
  string ErrorStr; // Der String mit der Fehlermeldung fürs Laden der Maps.
  string MapName;  // Name der aktuellen Map. 

  bool IsWater(int off,bool not_coast=false,bool is_ship=false);
  void NewMap(int size);
  void DeleteMap(void);
  bool SaveMap(string filename,SDL_Surface *preview);
  bool LoadMap(string filename);
  void UseAllTerrain(void);
  void PlaceRessources(int Metal,int Oil,int Gold,int Dichte); 
};

#endif