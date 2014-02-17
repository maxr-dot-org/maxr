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

#ifndef gui_menu_windows_windowgamesettings_windowgamesettingsH
#define gui_menu_windows_windowgamesettings_windowgamesettingsH

#include "../../../window.h"
#include "../../../../utility/signal/signalconnectionmanager.h"
#include "../../../../utility/signal/signal.h"

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

	cRadioGroup* metalRadioGroup;
	cCheckBox* metalLowCheckBox;
	cCheckBox* metalNormalCheckBox;
	cCheckBox* metalMuchCheckBox;
	cCheckBox* metalMostCheckBox;

	cRadioGroup* oilRadioGroup;
	cCheckBox* oilLowCheckBox;
	cCheckBox* oilNormalCheckBox;
	cCheckBox* oilMuchCheckBox;
	cCheckBox* oilMostCheckBox;

	cRadioGroup* goldRadioGroup;
	cCheckBox* goldLowCheckBox;
	cCheckBox* goldNormalCheckBox;
	cCheckBox* goldMuchCheckBox;
	cCheckBox* goldMostCheckBox;

	cRadioGroup* densityRadioGroup;
	cCheckBox* densityThinCheckBox;
	cCheckBox* densityNormalCheckBox;
	cCheckBox* densityThickCheckBox;
	cCheckBox* densityMostCheckBox;

	cRadioGroup* bridgeheadRadioGroup;
	cCheckBox* bridgeheadMobileCheckBox;
	cCheckBox* bridgeheadDefiniteCheckBox;

	cRadioGroup* gameTypeRadioGroup;
	cCheckBox* gameTypeTurnsCheckBox;
	cCheckBox* gameTypeSimultaneousCheckBox;

	cRadioGroup* clansRadioGroup;
	cCheckBox* clansOnCheckBox;
	cCheckBox* clansOffCheckBox;

	cRadioGroup* creditsRadioGroup;
	cCheckBox* creditsLowestCheckBox;
	cCheckBox* creditsLowerCheckBox;
	cCheckBox* creditsLowCheckBox;
	cCheckBox* creditsNormalCheckBox;
	cCheckBox* creditsMuchCheckBox;
	cCheckBox* creditsMoreCheckBox;

	cRadioGroup* victoryRadioGroup;
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

#endif // gui_menu_windows_windowgamesettings_windowgamesettingsH
