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

#ifndef ui_graphical_menu_dialogs_dialogresearchH
#define ui_graphical_menu_dialogs_dialogresearchH

#include <array>

#include "ui/graphical/window.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"
#include "game/logic/upgradecalculator.h"

class cLabel;
class cSlider;
class cPushButton;
class cPlayer;

class cDialogResearch : public cWindow
{
public:
	cDialogResearch (const cPlayer& player);

	const std::array<int, cResearch::kNrResearchAreas>& getResearchSettings() const;

	cSignal<void()> done;
private:
	cSignalConnectionManager signalConnectionManager;

	static const size_t rows = 8;
	static_assert (cResearch::kNrResearchAreas == rows, "number of items displayed in the research dialog does not match the number of research areas!");

	const cPlayer& player;

	std::array<int, cResearch::kNrResearchAreas> researchSettings;
	int unusedResearchCenters;

	std::array<cLabel*, rows> researchCenterCountLabels;
	std::array<cLabel*, rows> percentageLabels;
	std::array<cLabel*, rows> turnsLabels;
	std::array<cSlider*, rows> sliders;
	std::array<cPushButton*, rows> decreaseButtons;
	std::array<cPushButton*, rows> increaseButtons;

	void updateWidgets();
	void handleSliderValueChanged (size_t index);
};

#endif // ui_graphical_menu_dialogs_dialogresearchH
