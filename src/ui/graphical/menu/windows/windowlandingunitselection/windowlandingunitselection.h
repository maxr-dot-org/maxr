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

#ifndef ui_graphical_menu_windows_windowlandingunitselection_windowlandingunitselectionH
#define ui_graphical_menu_windows_windowlandingunitselection_windowlandingunitselectionH

#include "ui/graphical/menu/windows/windowadvancedhangar/windowadvancedhangar.h"

#include <array>
#include <map>

class cCheckBox;
class cLabel;
class cPushButton;
class cResourceBar;
class cUnitsData;
class cUnitUpgrade;
class cUnitListViewItemCargo;
struct sID;
struct sLandingUnit;

class cWindowLandingUnitSelection : public cWindowAdvancedHangar<cUnitListViewItemCargo>
{
public:
	cWindowLandingUnitSelection (cRgbColor playerColor, int playerClan, const std::vector<std::pair<sID, int>>& initialUnits, unsigned int initialGold, std::shared_ptr<const cUnitsData> unitsData);
	~cWindowLandingUnitSelection();

	void retranslate() override;

	std::vector<sLandingUnit> getLandingUnits() const;

	std::vector<std::pair<sID, cUnitUpgrade>> getUnitUpgrades() const;

protected:
	bool tryAddSelectedUnit (const cUnitListViewItemBuy& unitItem) const override;
	bool tryRemoveSelectedUnit (const cUnitListViewItemCargo& unitItem) const override;
	void setActiveUnit (const sID& unitId) override;

private:
	void generateSelectionList (bool select);

	void metalChanged();
	void goldChanged();

	void metalUpButtonClicked();
	void metalDownButtonClicked();

	void handleSelectedUnitSelectionChanged (cUnitListViewItemCargo* unitItem);

	std::pair<int, int> testBuyCargo (const cUnitListViewItemCargo&, int cargo) const;

	void upgradeIncreaseClicked (size_t index);
	void upgradeDecreaseClicked (size_t index);

	void updateUpgradeButtons();

private:
	cSignalConnectionManager signalConnectionManager;

	std::unique_ptr<cPlayer> temporaryPlayer;

	static const int metalBarSteps = 5;
	static const int singleCreditResourceAmount = 5;

	cCheckBox* tankCheckBox = nullptr;
	cCheckBox* planeCheckBox = nullptr;
	cCheckBox* shipCheckBox = nullptr;
	cCheckBox* buildingCheckBox = nullptr;
	cCheckBox* tntCheckBox = nullptr;

	cLabel* titleLabel = nullptr;
	cLabel* cargoLabel = nullptr;
	cLabel* creditLabel = nullptr;
	cCheckBox* buyCheckBox = nullptr;
	cCheckBox* upgradeCheckBox = nullptr;

	cResourceBar* metalBar = nullptr;
	cLabel* metalBarAmountLabel = nullptr;
	cPushButton* metalBarUpButton = nullptr;
	cPushButton* metalBarDownButton = nullptr;

	cResourceBar* goldBar = nullptr;
	cLabel* goldBarAmountLabel = nullptr;

	cUnitListViewItemCargo* selectedCargoUnit = nullptr;

	std::map<const cUnitListViewItemCargo*, int> fixedSelectedUnits;

	static const size_t maxUpgradeButtons = 8;

	std::array<cPushButton*, maxUpgradeButtons> upgradeDecreaseButton{};
	std::array<cPushButton*, maxUpgradeButtons> upgradeIncreaseButton{};
	std::array<cLabel*, maxUpgradeButtons> upgradeCostLabel{};

	std::map<sID, cUnitUpgrade> unitUpgrades;
};

#endif // ui_graphical_menu_windows_windowlandingunitselection_windowlandingunitselectionH
