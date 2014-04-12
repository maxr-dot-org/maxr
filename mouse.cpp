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

bool cMouse::SetCursor (eCursor const typ, int value_, int max_value_)
{
	assert (value_ <= max_value_ || max_value_ == -1);
	assert (max_value_ == -1 || typ == CAttack);
	assert (value_ == -1 || typ == CAttack || typ == CDisable || typ == CSteal);

	const SDL_Surface* const lastCur = cur;
	const int last_value = value;
	const int lastmax_value = max_value;

	value = value_;
	max_value = max_value_;
	switch (typ)
	{
		default:
		case CHand:      cur = GraphicsData.gfx_Chand.get();       break;
		case CNo:        cur = GraphicsData.gfx_Cno.get();         break;
		case CSelect:    cur = GraphicsData.gfx_Cselect.get();     break;
		case CMove:      cur = GraphicsData.gfx_Cmove.get();       break;
		case CMoveDraft: cur = GraphicsData.gfx_Cmove_draft.get(); break;
		case CPfeil1:    cur = GraphicsData.gfx_Cpfeil1.get();     break;
		case CPfeil2:    cur = GraphicsData.gfx_Cpfeil2.get();     break;
		case CPfeil3:    cur = GraphicsData.gfx_Cpfeil3.get();     break;
		case CPfeil4:    cur = GraphicsData.gfx_Cpfeil4.get();     break;
		case CPfeil6:    cur = GraphicsData.gfx_Cpfeil6.get();     break;
		case CPfeil7:    cur = GraphicsData.gfx_Cpfeil7.get();     break;
		case CPfeil8:    cur = GraphicsData.gfx_Cpfeil8.get();     break;
		case CPfeil9:    cur = GraphicsData.gfx_Cpfeil9.get();     break;
		case CHelp:      cur = GraphicsData.gfx_Chelp.get();       break;
		case CAttack:
		{
			cur = GraphicsData.gfx_Cattack.get();
			SDL_Rect r = {1, 29, 35, 3};
			if (max_value == -1)
			{
				assert (value == -1);
				SDL_FillRect (GraphicsData.gfx_Cattack.get(), &r, 0);
			}
			else
			{
				assert (0 <= value && value <= 35);
				assert (0 <= max_value && max_value <= 35);
				SDL_Rect r = {1, 29, Uint16 (value), 3};

				if (r.w)
					SDL_FillRect (GraphicsData.gfx_Cattack.get(), &r, 0x00FF00);

				r.x += r.w;
				r.w = max_value - value;

				if (r.w)
					SDL_FillRect (GraphicsData.gfx_Cattack.get(), &r, 0xFF0000);

				r.x += r.w;
				r.w = 35 - max_value;

				if (r.w)
					SDL_FillRect (GraphicsData.gfx_Cattack.get(), &r, 0);
			}
			break;
		}
		case CAttackOOR: cur = GraphicsData.gfx_Cattackoor.get();  break;
		case CBand:      cur = GraphicsData.gfx_Cband.get();       break;
		case CTransf:    cur = GraphicsData.gfx_Ctransf.get();     break;
		case CLoad:      cur = GraphicsData.gfx_Cload.get();       break;
		case CMuni:      cur = GraphicsData.gfx_Cmuni.get();       break;
		case CRepair:    cur = GraphicsData.gfx_Crepair.get();     break;
		case CSteal: // follow
		case CDisable:
		{
			assert (0 <= value && value <= 100);
			cur = typ == CSteal ? GraphicsData.gfx_Csteal.get() : GraphicsData.gfx_Cdisable.get();
			SDL_Rect r = {1, 28, 35, 3};

			if (value == 0)
			{
				SDL_FillRect (cur, &r, 0);
			}
			else
			{
				SDL_FillRect (cur, &r, 0x00FF0000);
				r.w = 35 * value / 100;
				SDL_FillRect (cur, &r, 0x0000FF00);
			}
			break;
		}
		case CActivate:  cur = GraphicsData.gfx_Cactivate.get();   break;
	}
	if (lastCur == cur && value == last_value && max_value == lastmax_value)
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

// updates the internal mouse position
void cMouse::updatePos()
{
	SDL_GetMouseState (&x, &y);
}

// gets the cursor offset. transforms screenspace to clickspace
void cMouse::getCursorOffset (int& x, int& y) const
{
	if (cur == GraphicsData.gfx_Cselect.get()
		|| cur == GraphicsData.gfx_Chelp.get()
		|| cur == GraphicsData.gfx_Cmove.get()
		|| cur == GraphicsData.gfx_Cmove_draft.get()
		|| cur == GraphicsData.gfx_Cno.get()
		|| cur == GraphicsData.gfx_Ctransf.get()
		|| cur == GraphicsData.gfx_Cband.get()
		|| cur == GraphicsData.gfx_Cload.get()
		|| cur == GraphicsData.gfx_Cmuni.get()
		|| cur == GraphicsData.gfx_Crepair.get()
		|| cur == GraphicsData.gfx_Cactivate.get())
	{
		x = 12;
		y = 12;
	}
	else if (cur == GraphicsData.gfx_Cattack.get()
			 || cur == GraphicsData.gfx_Csteal.get()
			 || cur == GraphicsData.gfx_Cdisable.get()
			 || cur == GraphicsData.gfx_Cattackoor.get())
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
