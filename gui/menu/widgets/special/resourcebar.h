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

#ifndef gui_menu_widgets_special_resourcebarH
#define gui_menu_widgets_special_resourcebarH

#include "../../../../maxrconfig.h"
#include "../clickablewidget.h"
#include "../../../orientation.h"
#include "../../../../autosurface.h"
#include "../../../../sound.h"
#include "../../../../utility/signal/signal.h"

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
	cResourceBar (const cBox<cPosition>& area, int minValue, int maxValue, eResourceBarType type, eOrientationType orientation, sSOUND* clickSound = SoundData.SNDObjectMenu.get ());

	void setType (eResourceBarType type);
	void setStepSize (int stepSize);

	int getMinValue () const;
	void setMinValue (int minValue);

	int getMaxValue () const;
	void setMaxValue (int maxValue);

	int getFixedMinValue () const;
	void setFixedMinValue (int minValue);

	int getFixedMaxValue () const;
	void setFixedMaxValue (int maxValue);

	bool isInverted () const;
	void setInverted (bool inverted);

	int getValue () const;
	void setValue (int value);

	void increase (int offset);
	void decrease (int offset);

	cSignal<void ()> valueChanged;

	virtual void draw () MAXR_OVERRIDE_FUNCTION;

protected:
	virtual bool handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;

private:
	AutoSurface surface;
	sSOUND* clickSound;

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

#endif // gui_menu_widgets_special_resourcebarH
