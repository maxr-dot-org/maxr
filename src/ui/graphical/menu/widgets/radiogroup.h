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

#ifndef ui_graphical_menu_widgets_radiogroupH
#define ui_graphical_menu_widgets_radiogroupH

#include "ui/widgets/widget.h"
#include "utility/signal/signalconnectionmanager.h"

class cCheckBox;

class cRadioGroup : public cWidget
{
public:
	explicit cRadioGroup (bool allowUncheckAll = false);
	~cRadioGroup() = default;

	cCheckBox* addButton (std::unique_ptr<cCheckBox> button);

	void handleMoved (const cPosition& offset) override;

private:
	void buttonToggled (cCheckBox* button);

private:
	cSignalConnectionManager signalConnectionManager;

	cCheckBox* currentlyCheckedButton = nullptr;

	bool allowUncheckAll = false;
	bool internalMoving = false;
};

#endif // ui_graphical_menu_widgets_radiogroupH
