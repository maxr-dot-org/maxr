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
#ifndef mouseH
#define mouseH
#include "defines.h"
#include "SDL.h"
#include "main.h"
#include "game.h"

// Die Mauszeigertypen ///////////////////////////////////////////////////////
enum eCursor{CHand,CNo,CSelect,CMove,CPfeil1,CPfeil2,CPfeil3,CPfeil4,CPfeil6,CPfeil7,CPfeil8,CPfeil9,CHelp,CAttack,CBand,CTransf,CLoad,CMuni,CRepair,CSteal,CDisable,CActivate};

// Die Maus-Klasse ///////////////////////////////////////////////////////////
class cMouse{
public:
	cMouse();
	~cMouse();

  bool visible; // Gibt an, ob die Maus angezeigt werden soll.
  SDL_Surface *cur; // Der aktuelle Cursor.
  SDL_Surface *back; // Zum Speichern des Maushintergrundes.
  int x,y; // Die Position der Maus.
  int LastX,LastY; // Die letzte Position der Maus. 
  int DrawX,DrawY; // Die Position, an die die Maus gezeichnet werden soll.
  bool MoveCallback; // Gibt an, ob die MoveCallback-Funktion aufgerufen werden soll.
  bool MoveCallbackEditor; // Gibt an, ob die MoveCallback-Funktion des Editors aufgerufen werden soll.

  void draw(bool draw_back,SDL_Surface *sf);
  bool SetCursor(eCursor typ);
  void GetBack(SDL_Surface *sf);
  void GetPos(void);
  void Show(void){LastX=-100;visible=true;}
  void Hide(void){visible=false;}
  int GetMouseButton(void);
  void GetKachel(int *X,int *Y);
  int GetKachelOff(void);  
};

// Die Maus //////////////////////////////////////////////////////////////////
EX cMouse *mouse;

#endif
