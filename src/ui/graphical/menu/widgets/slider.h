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

#ifndef ui_graphical_menu_widgets_sliderH
#define ui_graphical_menu_widgets_sliderH

#include "maxrconfig.h"
#include "ui/graphical/menu/widgets/clickablewidget.h"
#include "ui/graphical/menu/widgets/sliderhandle.h"
#include "utility/autosurface.h"
#include "ui/graphical/orientation.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

struct SDL_Surface;

enum class eSliderType
{
	Default,
	Invisible,
	DrawnBackground
};

class cSlider : public cClickableWidget
{
public:
	cSlider (const cBox<cPosition>& area, int minValue, int maxValue, eOrientationType orientation, eSliderType sliderType = eSliderType::Default);
	cSlider (const cBox<cPosition>& area, int minValue, int maxValue, eOrientationType orientation, eSliderHandleType handleType, eSliderType sliderType = eSliderType::Default);

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;
	void handleMoved (const cPosition& offset) override;

	int getMinValue() const;
	void setMinValue (int minValue);

	int getMaxValue() const;
	void setMaxValue (int maxValue);

	int getValue() const;
	void setValue (int value);

	void increase (int offset);
	void decrease (int offset);

	cSignal<void ()> valueChanged;
protected:
	bool handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button) override;

private:
	cSignalConnectionManager signalConnectionManager;

	AutoSurface surface;

	eSliderType type;

	int currentValue;

	int minValue;
	int maxValue;

	bool settingValue;

	eOrientationType orientation;

	cSliderHandle* handle;

	void createSurface (eSliderType sliderType);
	void createHandle (eSliderHandleType handleType);

	void setHandleMinMaxPosition();

	void computeHandleMinMaxPosition (int& minPosition, int& maxPosition) const;

	void movedHandle();

	int getValueFromHandlePosition() const;
};

#endif // ui_graphical_menu_widgets_sliderH
