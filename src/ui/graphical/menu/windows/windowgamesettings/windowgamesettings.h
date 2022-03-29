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
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

class cGameSettings;

class cCheckBox;
class cEditableCheckBox;
class cLabel;
class cLineEdit;
class cLobbyClient;
template <typename>
class cRadioGroupValue;

enum class eGameSettingsResourceAmount;
enum class eGameSettingsResourceDensity;
enum class eGameSettingsBridgeheadType;
enum class eGameSettingsGameType;
enum class eGameSettingsVictoryCondition;

class cWindowGameSettings : public cWindow
{
public:
	explicit cWindowGameSettings (bool forHotSeatGame = false);

	void applySettings (const cGameSettings& gameSettings);
	void initFor (cLobbyClient&);

	cSignal<void (const cGameSettings&)> done;

private:
	void okClicked();
	void backClicked();

	void disableTurnEndDeadlineOptions();
	void enableTurnEndDeadlineOptions();
	cGameSettings getGameSettings() const;

private:
	cSignalConnectionManager signalConnectionManager;

	bool forHotSeatGame = false;

	cRadioGroupValue<eGameSettingsResourceAmount>* metalGroup = nullptr;
	cRadioGroupValue<eGameSettingsResourceAmount>* oilGroup = nullptr;
	cRadioGroupValue<eGameSettingsResourceAmount>* goldGroup = nullptr;

	cRadioGroupValue<eGameSettingsResourceDensity>* densityGroup = nullptr;

	cRadioGroupValue<eGameSettingsBridgeheadType>* bridgeheadGroup = nullptr;
	cRadioGroupValue<bool>* alienGroup = nullptr;
	cRadioGroupValue<eGameSettingsGameType>* gameTypeGroup = nullptr;

	cRadioGroupValue<bool>* clansGroup = nullptr;

	cRadioGroupValue<int>* creditsGroup = nullptr;

	cRadioGroupValue<std::pair<eGameSettingsVictoryCondition, int>>* victoryGroup = nullptr;
	cEditableCheckBox* customVictoryTurnsCheckBox = nullptr;
	cEditableCheckBox* customVictoryPointsCheckBox = nullptr;

	cRadioGroupValue<int>* turnLimitGroup = nullptr;
	cEditableCheckBox* customTurnLimitCheckBox = nullptr;

	cRadioGroupValue<int>* endTurnDeadlineGroup = nullptr;
	cEditableCheckBox* customEndTurnDeadlineCheckBox = nullptr;
	cLabel* turnEndDeadlineLabel = nullptr;
};

#endif // ui_graphical_menu_windows_windowgamesettings_windowgamesettingsH
