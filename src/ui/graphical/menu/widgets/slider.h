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

#include "SDLutility/autosurface.h"
#include "ui/graphical/menu/widgets/sliderhandle.h"
#include "ui/widgets/clickablewidget.h"
#include "ui/widgets/orientation.h"
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
	cSlider (const cBox<cPosition>& area, int minValue, int maxValue, eOrientationType, eSliderType = eSliderType::Default);
	cSlider (const cBox<cPosition>& area, int minValue, int maxValue, eOrientationType, eSliderHandleType, eSliderType = eSliderType::Default);

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;
	void handleMoved (const cPosition& offset) override;

	int getMinValue() const { return minValue; }
	void setMinValue (int minValue);

	int getMaxValue() const { return maxValue; }
	void setMaxValue (int maxValue);

	int getValue() const { return currentValue; }
	void setValue (int value);

	void increase (int offset);
	void decrease (int offset);

	cSignal<void()> valueChanged;

protected:
	bool handleClicked (cApplication&, cMouse&, eMouseButtonType) override;

private:
	void createSurface (eSliderType);
	void createHandle (eSliderHandleType);

	void setHandleMinMaxPosition();

	void computeHandleMinMaxPosition (int& minPosition, int& maxPosition) const;

	void movedHandle();

	int getValueFromHandlePosition() const;

private:
	cSignalConnectionManager signalConnectionManager;

	UniqueSurface surface;

	eSliderType type;

	int currentValue;

	int minValue;
	int maxValue;

	bool settingValue = false;

	eOrientationType orientation;

	cSliderHandle* handle = nullptr;
};

#endif // ui_graphical_menu_widgets_sliderH
