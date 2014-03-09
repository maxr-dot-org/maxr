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

#ifndef gui_menu_windows_windowlandingunitselection_windowlandingunitselectionH
#define gui_menu_windows_windowlandingunitselection_windowlandingunitselectionH

#include <map>
#include <array>

#include "../windowadvancedhangar/windowadvancedhangar.h"

class cCheckBox;
class cResourceBar;
class cLabel;
class cPushButton;
class cUnitUpgrade;
struct sID;
struct sLandingUnit;

class cWindowLandingUnitSelection : public cWindowAdvancedHangar
{
public:
	cWindowLandingUnitSelection (int playerColor, int playerClan, const std::vector<std::pair<sID, int>>& initialUnits, unsigned int initialGold);
	~cWindowLandingUnitSelection ();

	std::vector<sLandingUnit> getLandingUnits () const;

	std::vector<std::pair<sID, cUnitUpgrade>> getUnitUpgrades () const;

protected:
	virtual bool tryAddSelectedUnit (const cUnitListViewItemBuy& unitItem) const MAXR_OVERRIDE_FUNCTION;
	virtual bool tryRemoveSelectedUnit (const cUnitListViewItemCargo& unitItem) const MAXR_OVERRIDE_FUNCTION;

	virtual void setActiveUnit (const sID& unitId) MAXR_OVERRIDE_FUNCTION;
private:
	cSignalConnectionManager signalConnectionManager;

	std::unique_ptr<cPlayer> temporaryPlayer;

	static const int metalBarSteps = 5;
	static const int singleCreditResourceAmount = 5;

	cCheckBox* tankCheckBox;
	cCheckBox* planeCheckBox;
	cCheckBox* shipCheckBox;
	cCheckBox* buildingCheckBox;
	cCheckBox* tntCheckBox;

	cCheckBox* buyCheckBox;
	cCheckBox* upgradeCheckBox;

	cResourceBar* metalBar;
	cLabel* metalBarAmountLabel;
	cPushButton* metalBarUpButton;
	cPushButton* metalBarDownButton;

	cResourceBar* goldBar;
	cLabel* goldBarAmountLabel;

	cUnitListViewItemCargo* selectedCargoUnit;

	std::vector<const cUnitListViewItemCargo*> fixedSelectedUnits;

	static const size_t maxUpgradeButtons = 8;

	std::array<cPushButton*, maxUpgradeButtons> upgradeDecreaseButton;
	std::array<cPushButton*, maxUpgradeButtons> upgradeIncreaseButton;
	std::array<cLabel*, maxUpgradeButtons> upgradeCostLabel;

	std::map<sID, cUnitUpgrade> unitUpgrades;

	void generateSelectionList (bool select);

	void metalChanged ();
	void goldChanged ();

	void metalUpButtonClicked ();
	void metalDownButtonClicked ();

	void handleSelectedUnitSelectionChanged (cUnitListViewItemCargo* unitItem);

	std::pair<int,int> testBuyCargo (int cargo);

	void upgradeIncreaseClicked (size_t index);
	void upgradeDecreaseClicked (size_t index);

	void updateUpgradeButtons ();
};

#endif // gui_menu_windows_windowlandingunitselection_windowlandingunitselectionH
