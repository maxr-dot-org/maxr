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

#ifndef ui_graphical_menu_widgets_scrollbarH
#define ui_graphical_menu_widgets_scrollbarH

#include "maxrconfig.h"
#include "ui/graphical/widget.h"
#include "ui/graphical/orientation.h"
#include "utility/signal/signalconnectionmanager.h"

class cPushButton;
class cSlider;

enum class eScrollBarStyle
{
	Classic,
	Modern
};

class cScrollBar : public cWidget
{
public:
	cScrollBar (const cPosition& position, int width, eScrollBarStyle style, eOrientationType orientation = eOrientationType::Vertical);

	void setRange (int range);

	void setOffset (int offset);

	int getOffset ();

	cSignal<void ()> offsetChanged;

	cSignal<void ()> backClicked;
	cSignal<void ()> forwardClicked;

	virtual void handleResized (const cPosition& oldSize) MAXR_OVERRIDE_FUNCTION;
private:
	cSignalConnectionManager signalConnectionManager;

	cPushButton* forwardButton;
	cPushButton* backButton;

	cSlider* slider;

	eScrollBarStyle style;

	eOrientationType orientation;

	size_t range;
	size_t offset;
};

#endif // ui_graphical_menu_widgets_scrollbarH
