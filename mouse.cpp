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
#include "mouse.h"
#include "main.h"
#include "client.h"
#include "settings.h"
#include "hud.h"


cMouse::cMouse()
{
	visible = false;
	cur = NULL;
	LastX = -100;
	LastY = -100;
	x = 0;
	y = 0;
	isDoubleClick = false;
	prevScreenX = 0;
	prevScreenY = 0;

	DrawX = 0;
	DrawY = 0;
}

// Malt die Maus, und wenn draw_back gesetzt ist, auch den alten Hintergrund:
void cMouse::draw (bool draw_back, SDL_Surface* sf)
{
	if (!visible || cur == NULL) return;
	GetPos();

	// restore old background
	if (back && draw_back && LastX != -100)
	{
		SDL_Rect dest;
		dest.x = LastX;
		dest.y = LastY;
		SDL_BlitSurface (back, NULL, sf, &dest);

		SDL_UpdateRect (sf, dest.x, dest.y, dest.w, dest.h);
	}

	//change size of back surface if necessary, e.g. when the mouse cursor has changed
	if (!back || back->h != cur->h || back->w != cur->w)
	{
		back = SDL_CreateRGBSurface (Video.getSurfaceType(), cur->w, cur->h, 32, 0, 0, 0, 0);
	}

	// store new background
	GetBack (sf);
	SDL_Rect dest;
	dest.x = DrawX;
	dest.y = DrawY;
	LastX = DrawX;
	LastY = DrawY;

	//draw mouse
	SDL_BlitSurface (cur, NULL, sf, &dest);
	SDL_UpdateRect (sf, dest.x, dest.y, dest.w, dest.h);
}

bool cMouse::SetCursor (eCursor const typ)
{
	const SDL_Surface* const lastCur = cur;
	switch (typ)
	{
		default:
		case CHand:     cur = GraphicsData.gfx_Chand;     break;
		case CNo:       cur = GraphicsData.gfx_Cno;       break;
		case CSelect:   cur = GraphicsData.gfx_Cselect;   break;
		case CMove:     cur = GraphicsData.gfx_Cmove;     break;
		case CPfeil1:   cur = GraphicsData.gfx_Cpfeil1;   break;
		case CPfeil2:   cur = GraphicsData.gfx_Cpfeil2;   break;
		case CPfeil3:   cur = GraphicsData.gfx_Cpfeil3;   break;
		case CPfeil4:   cur = GraphicsData.gfx_Cpfeil4;   break;
		case CPfeil6:   cur = GraphicsData.gfx_Cpfeil6;   break;
		case CPfeil7:   cur = GraphicsData.gfx_Cpfeil7;   break;
		case CPfeil8:   cur = GraphicsData.gfx_Cpfeil8;   break;
		case CPfeil9:   cur = GraphicsData.gfx_Cpfeil9;   break;
		case CHelp:     cur = GraphicsData.gfx_Chelp;     break;
		case CAttack:   cur = GraphicsData.gfx_Cattack;   break;
		case CBand:     cur = GraphicsData.gfx_Cband;     break;
		case CTransf:   cur = GraphicsData.gfx_Ctransf;   break;
		case CLoad:     cur = GraphicsData.gfx_Cload;     break;
		case CMuni:     cur = GraphicsData.gfx_Cmuni;     break;
		case CRepair:   cur = GraphicsData.gfx_Crepair;   break;
		case CSteal:    cur = GraphicsData.gfx_Csteal;    break;
		case CDisable:  cur = GraphicsData.gfx_Cdisable;  break;
		case CActivate: cur = GraphicsData.gfx_Cactivate; break;
	}
	return lastCur != cur;
}

// Liest den Hintergrund in das Back-Surface ein:
void cMouse::GetBack (SDL_Surface* sf)
{
	SDL_Rect scr;
	GetPos();
	scr.x = DrawX;
	scr.y = DrawY;
	scr.w = back->w;
	scr.h = back->h;
	SDL_BlitSurface (sf, &scr, back, NULL);
}

void cMouse::restoreBack (SDL_Surface* sf)
{
	SDL_Rect dest;
	dest.x = LastX;
	dest.y = LastY;
	SDL_BlitSurface (back, NULL, sf, &dest);
}

//updates the internal mouse position
void cMouse::GetPos()
{
	SDL_GetMouseState (&x, &y);

	// Cursor Offset bestimmen:
	int offX;
	int offY;

	getCursorOffset (offX, offY);

	DrawX = x + offX;
	DrawY = y + offY;
}

// gets the cursor offset. transforms screenspace to clickspace
void cMouse::getCursorOffset (int& x, int& y) const
{
	if (cur == GraphicsData.gfx_Cselect || cur == GraphicsData.gfx_Chelp || cur == GraphicsData.gfx_Cmove || cur == GraphicsData.gfx_Cno || cur == GraphicsData.gfx_Ctransf || cur == GraphicsData.gfx_Cband || cur == GraphicsData.gfx_Cload || cur == GraphicsData.gfx_Cmuni || cur == GraphicsData.gfx_Crepair || cur == GraphicsData.gfx_Cactivate)
	{
		x = -12;
		y = -12;
	}
	else if (cur == GraphicsData.gfx_Cattack || cur == GraphicsData.gfx_Csteal || cur == GraphicsData.gfx_Cdisable)
	{
		x = -19;
		y = -16;
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

// Liefert die Koordinaten der Kachel unter der Maus:
int cMouse::getKachelX() const
{
	if (x < 180 || x > 180 + (Video.getResolutionX() - 192))
	{
		return -1;
	}

	int X = (int) ( (x - 180 + Client->gameGUI.getOffsetX() * Client->gameGUI.getZoom()) / Client->gameGUI.getTileSize());
	X = std::min (X, Client->getMap()->size - 1);

	return X;
}

int cMouse::getKachelY() const
{
	if (y < 18 || y > 18 + (Video.getResolutionY() - 32))
	{
		return -1;
	}

	int Y = (int) ( (y - 18 + Client->gameGUI.getOffsetY() * Client->gameGUI.getZoom()) / Client->gameGUI.getTileSize());
	Y = std::min (Y, Client->getMap()->size - 1);

	return Y;
}
