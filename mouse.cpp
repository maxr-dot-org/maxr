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

#include <algorithm>

#include "mouse.h"

#include "main.h"
#include "client.h"
#include "hud.h"
#include "settings.h"
#include "video.h"

#include <SDL.h>

cMouse::cMouse() : sdlCursor (NULL)
{
	cur = NULL;
	x = 0;
	y = 0;
	isDoubleClick = false;
}

cMouse::~cMouse()
{
	if (sdlCursor) SDL_FreeCursor (sdlCursor);
}

bool cMouse::SetCursor (eCursor const typ)
{
	const SDL_Surface* const lastCur = cur;

	switch (typ)
	{
		default:
		case CHand:      cur = GraphicsData.gfx_Chand;       break;
		case CNo:        cur = GraphicsData.gfx_Cno;         break;
		case CSelect:    cur = GraphicsData.gfx_Cselect;     break;
		case CMove:      cur = GraphicsData.gfx_Cmove;       break;
		case CMoveDraft: cur = GraphicsData.gfx_Cmove_draft; break;
		case CPfeil1:    cur = GraphicsData.gfx_Cpfeil1;     break;
		case CPfeil2:    cur = GraphicsData.gfx_Cpfeil2;     break;
		case CPfeil3:    cur = GraphicsData.gfx_Cpfeil3;     break;
		case CPfeil4:    cur = GraphicsData.gfx_Cpfeil4;     break;
		case CPfeil6:    cur = GraphicsData.gfx_Cpfeil6;     break;
		case CPfeil7:    cur = GraphicsData.gfx_Cpfeil7;     break;
		case CPfeil8:    cur = GraphicsData.gfx_Cpfeil8;     break;
		case CPfeil9:    cur = GraphicsData.gfx_Cpfeil9;     break;
		case CHelp:      cur = GraphicsData.gfx_Chelp;       break;
		case CAttack:    cur = GraphicsData.gfx_Cattack;     break;
		case CAttackOOR: cur = GraphicsData.gfx_Cattackoor;  break;
		case CBand:      cur = GraphicsData.gfx_Cband;       break;
		case CTransf:    cur = GraphicsData.gfx_Ctransf;     break;
		case CLoad:      cur = GraphicsData.gfx_Cload;       break;
		case CMuni:      cur = GraphicsData.gfx_Cmuni;       break;
		case CRepair:    cur = GraphicsData.gfx_Crepair;     break;
		case CSteal:     cur = GraphicsData.gfx_Csteal;      break;
		case CDisable:   cur = GraphicsData.gfx_Cdisable;    break;
		case CActivate:  cur = GraphicsData.gfx_Cactivate;   break;
	}
	if (lastCur == cur)
	{
		return false;
	}
	if (sdlCursor) SDL_FreeCursor (sdlCursor);
	int hotx = 0;
	int hoty = 0;
	getCursorOffset (hotx, hoty);
	sdlCursor = SDL_CreateColorCursor (cur, hotx, hoty);
	SDL_SetCursor (sdlCursor);
	return true;
}

//updates the internal mouse position
void cMouse::updatePos()
{
	SDL_GetMouseState (&x, &y);
}

// gets the cursor offset. transforms screenspace to clickspace
void cMouse::getCursorOffset (int& x, int& y) const
{
	if (cur == GraphicsData.gfx_Cselect
		|| cur == GraphicsData.gfx_Chelp
		|| cur == GraphicsData.gfx_Cmove
		|| cur == GraphicsData.gfx_Cmove_draft
		|| cur == GraphicsData.gfx_Cno
		|| cur == GraphicsData.gfx_Ctransf
		|| cur == GraphicsData.gfx_Cband
		|| cur == GraphicsData.gfx_Cload
		|| cur == GraphicsData.gfx_Cmuni
		|| cur == GraphicsData.gfx_Crepair
		|| cur == GraphicsData.gfx_Cactivate)
	{
		x = 12;
		y = 12;
	}
	else if (cur == GraphicsData.gfx_Cattack
			 || cur == GraphicsData.gfx_Csteal
			 || cur == GraphicsData.gfx_Cdisable
			 || cur == GraphicsData.gfx_Cattackoor)
	{
		x = 19;
		y = 16;
	}
	else
	{
		x = 0;
		y = 0;
	}
}

bool cMouse::moved()
{
	static int lastX = 0;
	static int lastY = 0;
	const bool moved = (lastX != x || lastY != y);

	lastX = x;
	lastY = y;
	return moved;
}

void cMouse::Show()
{
	SDL_ShowCursor (true);
}

void cMouse::Hide()
{
	SDL_ShowCursor (false);
}

// Liefert die Koordinaten der Kachel unter der Maus:
int cMouse::getKachelX (const cGameGUI& gameGUI) const
{
	if (x < 180 || x > 180 + (Video.getResolutionX() - 192))
	{
		return -1;
	}
	const cMap& map = *gameGUI.getClient()->getMap();

	int X = (int) ((x - 180 + gameGUI.getOffsetX() * gameGUI.getZoom()) / gameGUI.getTileSize());
	X = std::min (X, map.getSize() - 1);

	return X;
}

int cMouse::getKachelY (const cGameGUI& gameGUI) const
{
	if (y < 18 || y > 18 + (Video.getResolutionY() - 32))
	{
		return -1;
	}
	const cMap& map = *gameGUI.getClient()->getMap();
	int Y = (int) ((y - 18 + gameGUI.getOffsetY() * gameGUI.getZoom()) / gameGUI.getTileSize());
	Y = std::min (Y, map.getSize() - 1);

	return Y;
}
