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

#ifndef ui_graphical_menu_widgets_sliderhandleH
#define ui_graphical_menu_widgets_sliderhandleH

#include "maxrconfig.h"
#include "ui/graphical/widget.h"
#include "ui/graphical/orientation.h"
#include "utility/autosurface.h"
#include "utility/signal/signal.h"

struct SDL_Surface;

enum class eSliderHandleType
{
	Horizontal,
	Vertical,
	HudZoom
};

class cSliderHandle : public cWidget
{
public:
	cSliderHandle (const cPosition& position, eSliderHandleType sliderHandleType, eOrientationType orientation);

	virtual void draw () MAXR_OVERRIDE_FUNCTION;

	void setMinMaxPosition (int minPosition, int maxPosition);

	cSignal<void ()> moved;
private:
	AutoSurface surface;

	eOrientationType orientation;

	int minPosition;
	int maxPosition;

	int grapOffset;

	void createSurface (eSliderHandleType sliderHandleType);

	virtual bool handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset) MAXR_OVERRIDE_FUNCTION;

	virtual bool handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;
	virtual bool handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;

	virtual void handleMoved (const cPosition& offset) MAXR_OVERRIDE_FUNCTION;
};

#endif // ui_graphical_menu_widgets_sliderhandleH
