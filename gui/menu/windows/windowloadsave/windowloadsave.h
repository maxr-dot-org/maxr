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

#ifndef gui_menu_windows_windowloadsave_windowloadsaveH
#define gui_menu_windows_windowloadsave_windowloadsaveH

#include "../windowload/windowload.h"

class cPushButton;

class cWindowLoadSave : public cWindowLoad
{
public:
	cWindowLoadSave ();

	cSignal<void (int, const std::string&)> save;
	cSignal<void ()> exit;

protected:
	virtual void handleSlotClicked (size_t index) MAXR_OVERRIDE_FUNCTION;
	virtual void handleSlotDoubleClicked (size_t index) MAXR_OVERRIDE_FUNCTION;

private:
	cSignalConnectionManager signalConnectionManager;

	cPushButton* saveButton;

	void handleSaveClicked ();
};

#endif // gui_menu_windows_windowloadsave_windowloadsaveH
