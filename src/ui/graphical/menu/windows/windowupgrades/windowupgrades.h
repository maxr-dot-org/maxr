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

#ifndef ui_graphical_menu_windows_windowupgrades_windowupgradesH
#define ui_graphical_menu_windows_windowupgrades_windowupgradesH

#include "ui/graphical/menu/windows/windowhangar/windowhangar.h"

#include <array>
#include <map>

class cLabel;
class cPushButton;
class cResourceBar;
class cCheckBox;
class cPlayer;
class cTurnTimeClock;

class cWindowUpgradesFilterState
{
public:
	cWindowUpgradesFilterState() = default;

	bool TankChecked = true;
	bool PlaneChecked = true;
	bool ShipChecked = true;
	bool BuildingChecked = true;
	bool TNTChecked = false;
};

class cWindowUpgrades : public cWindowHangar
{
	// TODO: remove code duplication with @ref cWindowLandingUnitSelection
public:
	explicit cWindowUpgrades (const cPlayer& player, std::shared_ptr<const cTurnTimeClock> turnTimeClock, std::shared_ptr<cWindowUpgradesFilterState> filterState, std::shared_ptr<const cUnitsData> unitsData);

	void retranslate() override;

	std::vector<std::pair<sID, cUnitUpgrade>> getUnitUpgrades() const;

protected:
	void setActiveUnit (const sID& unitId) override;

private:
	void generateSelectionList (bool select);

	void goldChanged();

	void upgradeIncreaseClicked (size_t index);
	void upgradeDecreaseClicked (size_t index);

	void updateUpgradeButtons();

private:
	cSignalConnectionManager signalConnectionManager;

	cLabel* titleLabel = nullptr;
	cLabel* creditLabel = nullptr;

	cCheckBox* tankCheckBox = nullptr;
	cCheckBox* planeCheckBox = nullptr;
	cCheckBox* shipCheckBox = nullptr;
	cCheckBox* buildingCheckBox = nullptr;
	cCheckBox* tntCheckBox = nullptr;
	std::shared_ptr<cWindowUpgradesFilterState> filterState;

	cResourceBar* goldBar = nullptr;
	cLabel* goldBarAmountLabel = nullptr;

	static const size_t maxUpgradeButtons = 8;

	std::array<cPushButton*, maxUpgradeButtons> upgradeDecreaseButton;
	std::array<cPushButton*, maxUpgradeButtons> upgradeIncreaseButton;
	std::array<cLabel*, maxUpgradeButtons> upgradeCostLabel;

	std::map<sID, cUnitUpgrade> unitUpgrades;
};

#endif // ui_graphical_menu_windows_windowupgrades_windowupgradesH
