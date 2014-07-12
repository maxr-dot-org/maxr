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

class cRadioGroup;
class cCheckBox;

class cWindowGameSettings : public cWindow
{
public:
	cWindowGameSettings ();
	~cWindowGameSettings ();

	void applySettings (const cGameSettings& gameSettings);
	cGameSettings getGameSettings () const;

	cSignal<void ()> done;
private:
	cSignalConnectionManager signalConnectionManager;

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

	cCheckBox* victoryTurns100CheckBox;
	cCheckBox* victoryTurns200CheckBox;
	cCheckBox* victoryTurns400CheckBox;

	cCheckBox* victoryPoints100CheckBox;
	cCheckBox* victoryPoints200CheckBox;
	cCheckBox* victoryPoints400CheckBox;

	cCheckBox* victoryNoLimitCheckBox;

	void okClicked ();
	void backClicked ();
};

#endif // ui_graphical_menu_windows_windowgamesettings_windowgamesettingsH
