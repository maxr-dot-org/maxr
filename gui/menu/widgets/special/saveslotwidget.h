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

#ifndef gui_menu_widgets_special_saveslotwidgetH
#define gui_menu_widgets_special_saveslotwidgetH

#include "../clickablewidget.h"
#include "../../../../utility/signal/signal.h"

class cLabel;
class cLineEdit;
class cPosition;
struct sSaveFile;

class cSaveSlotWidget : public cClickableWidget
{
public:
	cSaveSlotWidget (const cPosition& position);

	const std::string& getName () const;

	void setSelected (bool selected);

	void setRenameable (bool renameable);

	void setSaveData (const sSaveFile& saveFile);
	void reset (int number);

	bool isEmpty () const;

	void forceKeyFocus ();

	cSignal<void ()> clicked;
	cSignal<void ()> doubleClicked;

	cSignal<void ()> nameChanged;
protected:

	virtual bool handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;
private:
	cLabel* numberLabel;
	cLabel* typeLabel;
	cLabel* timeLabel;
	cLineEdit* nameLineEdit;

	bool empty;
	bool renameable;
};

#endif // gui_menu_widgets_special_saveslotwidgetH
