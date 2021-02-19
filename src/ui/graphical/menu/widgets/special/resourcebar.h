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

#ifndef ui_graphical_menu_widgets_special_resourcebarH
#define ui_graphical_menu_widgets_special_resourcebarH

#include "ui/graphical/menu/widgets/clickablewidget.h"
#include "ui/graphical/orientation.h"
#include "utility/autosurface.h"
#include "resources/sound.h"
#include "utility/signal/signal.h"

enum class eResourceBarType
{
	Metal,
	Oil,
	Gold,
	Blocked,
	MetalSlim,
	OilSlim,
	GoldSlim,
};

class cResourceBar : public cClickableWidget
{
public:
	cResourceBar (const cBox<cPosition>& area, int minValue, int maxValue, eResourceBarType type, eOrientationType orientation, cSoundChunk* clickSound = &SoundData.SNDObjectMenu);

	void setType (eResourceBarType type);
	void setStepSize (int stepSize);

	int getMinValue() const;
	void setMinValue (int minValue);

	int getMaxValue() const;
	void setMaxValue (int maxValue);

	int getFixedMinValue() const;
	void setFixedMinValue (int minValue);

	int getFixedMaxValue() const;
	void setFixedMaxValue (int maxValue);

	bool isInverted() const;
	void setInverted (bool inverted);

	int getValue() const;
	void setValue (int value);

	void increase (int offset);
	void decrease (int offset);

	cSignal<void ()> valueChanged;

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;

protected:
	bool handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button) override;

private:
	AutoSurface surface;
	cSoundChunk* clickSound;

	eOrientationType orientation;

	const cPosition additionalArea;

	int minValue;
	int maxValue;

	int currentValue;

	bool fixedMinEnabled;
	int fixedMinValue;
	bool fixedMaxEnabled;
	int fixedMaxValue;

	bool inverted;

	int stepSize;

	void createSurface (eResourceBarType type);
};

#endif // ui_graphical_menu_widgets_special_resourcebarH
