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

#include "SDLutility/autosurface.h"
#include "ui/widgets/orientation.h"
#include "ui/widgets/widget.h"
#include "utility/signal/signal.h"

struct SDL_Surface;

enum class eSliderHandleType
{
	Horizontal,
	Vertical,
	HudZoom,
	ModernHorizontal,
	ModernVertical,
};

class cSliderHandle : public cWidget
{
public:
	cSliderHandle (const cPosition&, eSliderHandleType, eOrientationType);

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;

	void setMinMaxPosition (int minPosition, int maxPosition);

	cSignal<void()> moved;

private:
	UniqueSurface surface;

	eOrientationType orientation;

	int minPosition;
	int maxPosition;

	int grapOffset;

	void createSurface (eSliderHandleType);

	bool handleMouseMoved (cApplication&, cMouse&, const cPosition& offset) override;
	bool handleMousePressed (cApplication&, cMouse&, eMouseButtonType) override;
	bool handleMouseReleased (cApplication&, cMouse&, eMouseButtonType) override;
	void handleMoved (const cPosition& offset) override;
};

#endif // ui_graphical_menu_widgets_sliderhandleH
