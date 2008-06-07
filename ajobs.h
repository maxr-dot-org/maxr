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
