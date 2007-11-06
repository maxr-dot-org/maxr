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
#ifndef mapH
#define mapH
#include "defines.h"
#include "main.h"
#include "vehicles.h"
#include "buildings.h"

// GameObjects Struktur //////////////////////////////////////////////////////
struct sGameObjects{
  cVehicle *vehicle;		// Vehicles on the ground
  cVehicle *plane;			// Vehicles in the air
  cBuilding *base;			// Buildings on the ground whitch are under vehciles and top-buildings
  cBuilding *subbase;		// Normaley base-buildings but in this special case they are under an other base-building (for examble plattform under street)
  cBuilding *top;			// Normal buildings on the ground
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
