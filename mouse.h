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

class cGameGUI;
struct SDL_Surface;
struct SDL_Cursor;


// Die Mauszeigertypen ///////////////////////////////////////////////////////
enum eCursor {CHand, CNo, CSelect, CMove, CPfeil1, CPfeil2, CPfeil3, CPfeil4, CPfeil6, CPfeil7, CPfeil8, CPfeil9, CHelp, CAttack, CBand, CTransf, CLoad, CMuni, CRepair, CSteal, CDisable, CActivate, CMoveDraft, CAttackOOR};

// Die Maus-Klasse ///////////////////////////////////////////////////////////
class cMouse
{
public:
	cMouse();
	~cMouse();

	// Set a new cursor.
	bool SetCursor (eCursor);

	void updatePos();
	void getCursorOffset (int& x, int& y) const;
	bool moved();
	void Show();
	void Hide();
	/**
	* return the X Coordinate of the Cursor on the map
	*/
	int getKachelX (const cGameGUI& gameGUI) const;
	/**
	* return the Y Coordinate of the Cursor on the map
	*/
	int getKachelY (const cGameGUI& gameGUI) const;

public:
	SDL_Surface* cur; // Current Cursor.
private:
	SDL_Cursor* sdlCursor;
public:
	int x, y; /** the pixel position of the cursor on the map */
	bool isDoubleClick;
};

// Die Maus //////////////////////////////////////////////////////////////////
EX cMouse* mouse;

#endif
