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

#ifndef gui_menu_widgets_sliderH
#define gui_menu_widgets_sliderH

#include "../../../maxrconfig.h"
#include "clickablewidget.h"
#include "sliderhandle.h"
#include "../../../autosurface.h"
#include "../../orientation.h"
#include "../../../utility/signal/signal.h"
#include "../../../utility/signal/signalconnectionmanager.h"

struct SDL_Surface;

class cSlider : public cClickableWidget
{
public:
	cSlider (const cBox<cPosition>& area, int minValue, int maxValue, eOrientationType orientation);
	cSlider (const cBox<cPosition>& area, int minValue, int maxValue, eOrientationType orientation, eSliderHandleType handleType);

	virtual void draw () MAXR_OVERRIDE_FUNCTION;

	virtual void handleMoved (const cPosition& offset) MAXR_OVERRIDE_FUNCTION;

	int getValue ();
	void setValue (int value);

	cSignal<void ()> valueChanged;
protected:
	virtual bool handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;

private:
	cSignalConnectionManager signalConnectionManager;

	AutoSurface surface;

	int minValue;
	int maxValue;

	eOrientationType orientation;

	cSliderHandle* handle;

	void createSurface ();
	void createHandle (eSliderHandleType handleType);

	void setHandleMinMaxPosition ();

	void computeHandleMinMaxPosition (int& minPosition, int& maxPosition);

	void movedHandle ();
};

#endif // gui_menu_widgets_sliderH
