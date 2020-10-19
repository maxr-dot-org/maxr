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
template <typename> class cRadioGroupValue;

enum class eGameSettingsResourceAmount;
enum class eGameSettingsResourceDensity;
enum class eGameSettingsBridgeheadType;
enum class eGameSettingsGameType;
enum class eGameSettingsVictoryCondition;

class cWindowGameSettings : public cWindow
{
public:
	explicit cWindowGameSettings (bool forHotSeatGame = false);
	~cWindowGameSettings();

	void applySettings (const cGameSettings& gameSettings);
	cGameSettings getGameSettings() const;

	cSignal<void()> done;

private:
	void okClicked();
	void backClicked();

	void disableTurnEndDeadlineOptions();
	void enableTurnEndDeadlineOptions();

private:
	cSignalConnectionManager signalConnectionManager;

	bool forHotSeatGame = false;

	cRadioGroupValue<eGameSettingsResourceAmount>* metalGroup = nullptr;
	cRadioGroupValue<eGameSettingsResourceAmount>* oilGroup = nullptr;
	cRadioGroupValue<eGameSettingsResourceAmount>* goldGroup = nullptr;

	cRadioGroupValue<eGameSettingsResourceDensity>* densityGroup = nullptr;

	cRadioGroupValue<eGameSettingsBridgeheadType>* bridgeheadGroup = nullptr;
	cRadioGroupValue<eGameSettingsGameType>* gameTypeGroup = nullptr;

	cRadioGroupValue<bool>* clansGroup = nullptr;

	cRadioGroupValue<int>* creditsGroup = nullptr;

	cRadioGroupValue<std::pair<eGameSettingsVictoryCondition, int>>* victoryGroup = nullptr;

	cRadioGroupValue<int>* turnLimitGroup = nullptr;
	cLineEdit* turnLimitCustomLineEdit = nullptr;

	cRadioGroupValue<int>* endTurnDeadlineGroup = nullptr;
	cLineEdit* turnEndTurnDeadlineLineEdit = nullptr;
	cLabel* turnEndDeadlineLabel = nullptr;
	cLabel* turnEndDeadlineSecondsLabel = nullptr;
};

#endif // ui_graphical_menu_windows_windowgamesettings_windowgamesettingsH
