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

#ifndef ui_graphical_menu_dialogs_dialogcolorpickerH
#define ui_graphical_menu_dialogs_dialogcolorpickerH

#include "ui/widgets/window.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

class cImage;
class cLineEdit;
class cPushButton;
class cRgbColor;
class cRgbColorPicker;

class cDialogColorPicker : public cWindow
{
public:
	explicit cDialogColorPicker (const cRgbColor& color, eWindowBackgrounds backgroundType = eWindowBackgrounds::Alpha);
	~cDialogColorPicker();

	void retranslate() override;

	cRgbColor getSelectedColor() const;

	cSignal<void()> done;
	cSignal<void()> canceled;

private:
	cSignalConnectionManager signalConnectionManager;

	cPushButton* okButton = nullptr;
	cPushButton* cancelButton = nullptr;

	cRgbColorPicker* colorPicker = nullptr;
	cImage* selectedColorImage = nullptr;

	cLineEdit* redValueLineEdit = nullptr;
	cLineEdit* greenValueLineEdit = nullptr;
	cLineEdit* blueValueLineEdit = nullptr;

	UniqueSurface createSelectedColorSurface();
};

#endif // ui_graphical_menu_dialogs_dialogcolorpickerH
