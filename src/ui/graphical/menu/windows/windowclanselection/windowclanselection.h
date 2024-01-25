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

#include "ui/widgets/window.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <array>

class cClanData;
class cImage;
class cLabel;
class cPushButton;
class cUnitsData;

class cWindowClanSelection : public cWindow
{
public:
	cWindowClanSelection (std::shared_ptr<const cUnitsData> unitsData, std::shared_ptr<const cClanData> clanData);
	~cWindowClanSelection();

	void retranslate() override;

	cSignal<void()> done;
	cSignal<void()> canceled;

	unsigned int getSelectedClan() const;

private:
	cSignalConnectionManager signalConnectionManager;

	static constexpr size_t clanRows = 2;
	static constexpr size_t clanColumns = 4;
	static constexpr size_t clanCount = clanRows * clanColumns;

	std::array<cImage*, clanCount> clanImages;
	std::array<cLabel*, clanCount> clanTitles;

	cLabel* titleLabel = nullptr;
	cPushButton* okButton = nullptr;
	cPushButton* backButton = nullptr;

	cLabel* clanDescription1 = nullptr;
	cLabel* clanDescription2 = nullptr;
	cLabel* clanShortDescription = nullptr;

	std::shared_ptr<const cUnitsData> unitsData;
	std::shared_ptr<const cClanData> clanData;

	unsigned int selectedClan = 0;

	void clanClicked (const cImage* clanImage);

	void okClicked();
	void backClicked();

	void updateClanDescription();
};

#endif // ui_graphical_menu_windows_windowclanselection_windowclanselectionH
