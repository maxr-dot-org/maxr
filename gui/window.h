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

#ifndef gui_windowH
#define gui_windowH

#include "widget.h"
#include "../maxrconfig.h"
#include "../autosurface.h"

enum class eWindowBackgrounds
{
	Black,
	Alpha,
	Transparent
};

class cWindow : public cWidget
{
public:
	explicit cWindow (SDL_Surface* surface, eWindowBackgrounds backgroundType = eWindowBackgrounds::Black);

	bool isClosing() const;

	void close ();

	virtual void draw () MAXR_OVERRIDE_FUNCTION;

	virtual void handleDeactivated (cApplication& application);
	virtual void handleActivated (cApplication& application);

protected:
	cApplication* getActiveApplication () const;

	AutoSurface surface;
private:
	eWindowBackgrounds backgroundType;

	cApplication* activeApplication;

	bool closing;

	bool hasBeenDrawnOnce;

	void setSurface (SDL_Surface* surface);
};

#endif // gui_windowH
