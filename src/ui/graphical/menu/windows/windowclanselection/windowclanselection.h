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

#ifndef ui_graphical_menu_windows_windowclanselection_windowclanselectionH
#define ui_graphical_menu_windows_windowclanselection_windowclanselectionH

#include <array>

#include "ui/graphical/window.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/signal/signal.h"

class cImage;
class cLabel;

class cWindowClanSelection : public cWindow
{
public:
	cWindowClanSelection();
	~cWindowClanSelection();

	cSignal<void ()> done;
	cSignal<void ()> canceled;

	unsigned int getSelectedClan() const;
private:
	cSignalConnectionManager signalConnectionManager;

	static const size_t clanRows = 2;
	static const size_t clanColumns = 4;
	static const size_t clanCount = clanRows* clanColumns;

	std::array<cImage*, clanCount> clanImages;
	std::array<cLabel*, clanCount> clanTitles;

	cLabel* clanDescription1;
	cLabel* clanDescription2;
	cLabel* clanShortDescription;

	unsigned int selectedClan;

	void clanClicked (const cImage* clanImage);

	void okClicked();
	void backClicked();

	void updateClanDescription();
};

#endif // ui_graphical_menu_windows_windowclanselection_windowclanselectionH
