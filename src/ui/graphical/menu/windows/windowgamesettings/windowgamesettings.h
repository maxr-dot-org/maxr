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

#ifndef ui_graphical_menu_windows_windowgamesettings_windowgamesettingsH
#define ui_graphical_menu_windows_windowgamesettings_windowgamesettingsH

#include "ui/graphical/window.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/signal/signal.h"

class cGameSettings;

class cCheckBox;
class cLineEdit;
class cLabel;

class cWindowGameSettings : public cWindow
{
public:
	explicit cWindowGameSettings (bool forHotSeatGame = false);
	~cWindowGameSettings();

	void applySettings (const cGameSettings& gameSettings);
	cGameSettings getGameSettings() const;

	cSignal<void ()> done;
private:
	cSignalConnectionManager signalConnectionManager;

	bool forHotSeatGame;

	cCheckBox* metalLimitedCheckBox;
	cCheckBox* metalNormalCheckBox;
	cCheckBox* metalHighCheckBox;
	cCheckBox* metalTooMuchCheckBox;

	cCheckBox* oilLimitedCheckBox;
	cCheckBox* oilNormalCheckBox;
	cCheckBox* oilHighCheckBox;
	cCheckBox* oilTooMuchCheckBox;

	cCheckBox* goldLimitedCheckBox;
	cCheckBox* goldNormalCheckBox;
	cCheckBox* goldHighCheckBox;
	cCheckBox* goldTooMuchCheckBox;

	cCheckBox* densitySparseCheckBox;
	cCheckBox* densityNormalCheckBox;
	cCheckBox* densityDenseCheckBox;
	cCheckBox* densityTooMuchCheckBox;

	cCheckBox* bridgeheadMobileCheckBox;
	cCheckBox* bridgeheadDefiniteCheckBox;

	cCheckBox* gameTypeTurnsCheckBox;
	cCheckBox* gameTypeSimultaneousCheckBox;

	cCheckBox* clansOnCheckBox;
	cCheckBox* clansOffCheckBox;

	cCheckBox* creditsNoneCheckBox;
	cCheckBox* creditsLowCheckBox;
	cCheckBox* creditsLimitedCheckBox;
	cCheckBox* creditsNormalCheckBox;
	cCheckBox* creditsHighCheckBox;
	cCheckBox* creditsMoreCheckBox;

	cCheckBox* victoryTurns0CheckBox;
	cCheckBox* victoryTurns1CheckBox;
	cCheckBox* victoryTurns2CheckBox;

	cCheckBox* victoryPoints0CheckBox;
	cCheckBox* victoryPoints1CheckBox;
	cCheckBox* victoryPoints2CheckBox;

	cCheckBox* victoryNoLimitCheckBox;

	cCheckBox* turnLimitNoLimitCheckBox;
	cCheckBox* turnLimit0CheckBox;
	cCheckBox* turnLimit1CheckBox;
	cCheckBox* turnLimit2CheckBox;
	cCheckBox* turnLimit3CheckBox;
	cCheckBox* turnLimit4CheckBox;
	cCheckBox* turnLimit5CheckBox;
	cCheckBox* turnLimitCustomCheckBox;
	cLineEdit* turnLimitCustomLineEdit;

	cCheckBox* turnEndTurnDeadlineNoLimitCheckBox;
	cCheckBox* turnEndTurnDeadline0CheckBox;
	cCheckBox* turnEndTurnDeadline1CheckBox;
	cCheckBox* turnEndTurnDeadline2CheckBox;
	cCheckBox* turnEndTurnDeadline3CheckBox;
	cCheckBox* turnEndTurnDeadline4CheckBox;
	cCheckBox* turnEndTurnDeadline5CheckBox;
	cCheckBox* turnEndTurnDeadlineCustomCheckBox;
	cLineEdit* turnEndTurnDeadlineLineEdit;
	cLabel* turnEndDeadlineLabel;
	cLabel* turnEndDeadlineSecondsLabel;

	void okClicked();
	void backClicked();

	void disableTurnEndDeadlineOptions();
	void enableTurnEndDeadlineOptions();
};

#endif // ui_graphical_menu_windows_windowgamesettings_windowgamesettingsH
