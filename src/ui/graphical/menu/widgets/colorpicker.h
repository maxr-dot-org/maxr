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

#ifndef ui_graphical_menu_widgets_colorpickerH
#define ui_graphical_menu_widgets_colorpickerH

#include "ui/graphical/widget.h"
#include "utility/color.h"
#include "utility/autosurface.h"
#include "utility/signal/signal.h"

class cImage;

class cRgbColorPicker : public cWidget
{
public:
	explicit cRgbColorPicker (const cBox<cPosition>& area, const cRgbColor& color);

	void setSelectedColor (const cRgbColor& color);
	void setSelectedColor (const cHsvColor& color);
	cRgbColor getSelectedColor() const;

	cSignal<void ()> selectedColorChanged;

	bool handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset) override;
	bool handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button) override;
	bool handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button) override;
	void handleLooseMouseFocus (cApplication& application) override;
private:
	cHsvColor currentColor;

	bool startedPressInColor;
	bool startedPressInBar;

	cImage* colorsImage;
	cImage* colorBarImage;

	cImage* selectedColorMarker;
	cImage* selectedColorHueMarker;

	AutoSurface createColorsSurface();
	AutoSurface createColorBarSurface();

	AutoSurface createColorMarkerSurface();
	AutoSurface createColorHueMarkerSurface();

	void updateMarkers();

	void updateColorByMousePosition (const cPosition& mousePosition);
	void updateColorHueByMousePosition (const cPosition& mousePosition);
};


#endif // ui_graphical_menu_widgets_frameH
