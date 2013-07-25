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

#include "autosurface.h"
#include "defines.h"
#include <SDL.h>

class cGameGUI;

// Die Mauszeigertypen ///////////////////////////////////////////////////////
enum eCursor {CHand, CNo, CSelect, CMove, CPfeil1, CPfeil2, CPfeil3, CPfeil4, CPfeil6, CPfeil7, CPfeil8, CPfeil9, CHelp, CAttack, CBand, CTransf, CLoad, CMuni, CRepair, CSteal, CDisable, CActivate, CMoveDraft, CAttackOOR};

// Die Maus-Klasse ///////////////////////////////////////////////////////////
class cMouse
{
public:
	cMouse();

	void draw (bool draw_back, SDL_Surface* sf);

	// Set a new cursor.
	bool SetCursor (eCursor);

	void GetBack (SDL_Surface* sf);
	/**
	* Draws the currently stored background to sf
	*/
	void restoreBack (SDL_Surface* sf);
	void GetPos();
	void getCursorOffset (int& x, int& y) const;
	bool moved();
	void Show() {LastX = -100; visible = true;}
	void Hide() {visible = false;}
	/**
	* return the X Coordinate of the Cursor on the map
	*/
	int getKachelX (const cGameGUI& gameGUI) const;
	/**
	* return the Y Coordinate of the Cursor on the map
	*/
	int getKachelY (const cGameGUI& gameGUI) const;
private:
	bool visible; // Gibt an, ob die Maus angezeigt werden soll.
public:
	SDL_Surface* cur; // Current Cursor.
private:
	AutoSurface back; // Zum Speichern des Maushintergrundes.
public:
	int x, y; /** the pixel position of the cursor on the map */
	bool isDoubleClick;
private:
	int prevScreenX, prevScreenY;
	int LastX, LastY; // Die letzte Position der Maus.
	int DrawX, DrawY; // Die Position, an die die Maus gezeichnet werden soll.
};

// Die Maus //////////////////////////////////////////////////////////////////
EX cMouse* mouse;

#endif
