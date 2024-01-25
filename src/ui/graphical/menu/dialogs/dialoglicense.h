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

#ifndef ui_graphical_menu_dialogs_dialoglicenseH
#define ui_graphical_menu_dialogs_dialoglicenseH

#include "ui/widgets/window.h"
#include "utility/signal/signalconnectionmanager.h"

#include <string>

class cPushButton;
class cLabel;

class cDialogLicense : public cWindow
{
public:
	cDialogLicense();
	~cDialogLicense();

private:
	cSignalConnectionManager signalConnectionManager;

	cLabel* headerLabel = nullptr;
	cLabel* textLabel = nullptr;

	cPushButton* upButton = nullptr;
	cPushButton* downButton = nullptr;

	int currentPage = 0;
	static constexpr int maxPage = 3;

	std::string authors;

	void pageDown();

	void pageUp();

	void readAuthors();

	void resetTexts();

	void updatePageButtons();
};

#endif // ui_graphical_menu_dialogs_dialoglicenseH
