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

#include "windowlandingunitselection.h"

#include "game/data/player/player.h"
#include "game/data/units/landingunit.h"
#include "resources/pcx.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/listview.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/radiogroup.h"
#include "ui/graphical/menu/widgets/special/resourcebar.h"
#include "ui/graphical/menu/widgets/special/unitlistviewitembuy.h"
#include "ui/graphical/menu/widgets/special/unitlistviewitemcargo.h"
#include "ui/uidefines.h"
#include "ui/widgets/label.h"
#include "utility/language.h"

#include <algorithm>

//------------------------------------------------------------------------------
cWindowLandingUnitSelection::cWindowLandingUnitSelection (cRgbColor playerColor, int playerClan, const std::vector<std::pair<sID, int>>& initialUnits, unsigned int initialGold, std::shared_ptr<const cUnitsData> unitsData) :
	cWindowAdvancedHangar<cUnitListViewItemCargo> (LoadPCX (GFXOD_HANGAR), unitsData, playerColor, playerClan),
	selectedCargoUnit (nullptr)
{
	titleLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (474, 12), getPosition() + cPosition (474 + 157, 12 + 10)), lngPack.i18n ("Title~Choose_Units"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	//
	// Unit Filters
	//
	tankCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (467, 411), eCheckBoxType::Tank);
	tankCheckBox->setChecked (true);
	signalConnectionManager.connect (tankCheckBox->toggled, [this]() { generateSelectionList (false); });

	planeCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (467 + 33, 411), eCheckBoxType::Plane);
	planeCheckBox->setChecked (true);
	signalConnectionManager.connect (planeCheckBox->toggled, [this]() { generateSelectionList (false); });

	shipCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (467 + 33 * 2, 411), eCheckBoxType::Ship);
	shipCheckBox->setChecked (true);
	signalConnectionManager.connect (shipCheckBox->toggled, [this]() { generateSelectionList (false); });

	buildingCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (467 + 33 * 3, 411), eCheckBoxType::Building);
	buildingCheckBox->setChecked (true);
	signalConnectionManager.connect (buildingCheckBox->toggled, [this]() { generateSelectionList (false); });

	tntCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (467 + 33 * 4, 411), eCheckBoxType::Tnt);
	tntCheckBox->setChecked (false);
	signalConnectionManager.connect (tntCheckBox->toggled, [this]() { generateSelectionList (false); });

	auto updateBuyGroup = emplaceChild<cRadioGroup>();

	buyCheckBox = updateBuyGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (542, 445), lngPack.i18n ("Others~Buy"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Right, eCheckBoxType::Round));
	buyCheckBox->setChecked (true);
	signalConnectionManager.connect (buyCheckBox->toggled, [this]() { generateSelectionList (false); });

	upgradeCheckBox = updateBuyGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (542, 445 + 17), lngPack.i18n ("Others~Upgrade"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Right, eCheckBoxType::Round));
	signalConnectionManager.connect (upgradeCheckBox->toggled, [this]() { generateSelectionList (false); });

	//
	// Resource Bar
	//
	cargoLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (411, 285), getPosition() + cPosition (411 + 42, 285 + 10)), lngPack.i18n ("Title~Cargo"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);
	metalBar = emplaceChild<cResourceBar> (cBox<cPosition> (getPosition() + cPosition (421, 301), getPosition() + cPosition (421 + 20, 301 + 115)), 0, 100, eResourceBarType::Metal, eOrientationType::Vertical);
	metalBar->setStepSize (metalBarSteps);
	signalConnectionManager.connect (metalBar->valueChanged, [this]() { metalChanged(); });
	metalBarAmountLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (411, 275), getPosition() + cPosition (411 + 40, 275 + 10)), "", eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	metalBarUpButton = emplaceChild<cPushButton> (getPosition() + cPosition (413, 424), ePushButtonType::ArrowUpSmall, &SoundData.SNDObjectMenu);
	signalConnectionManager.connect (metalBarUpButton->clicked, [this]() { metalUpButtonClicked(); });
	metalBarDownButton = emplaceChild<cPushButton> (getPosition() + cPosition (413 + 20, 424), ePushButtonType::ArrowDownSmall, &SoundData.SNDObjectMenu);
	signalConnectionManager.connect (metalBarDownButton->clicked, [this]() { metalDownButtonClicked(); });

	//
	// Gold Bar
	//
	creditLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (360, 285), getPosition() + cPosition (360 + 44, 285 + 10)), lngPack.i18n ("Title~Credits"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);
	goldBar = emplaceChild<cResourceBar> (cBox<cPosition> (getPosition() + cPosition (372, 301), getPosition() + cPosition (372 + 20, 301 + 115)), 0, initialGold, eResourceBarType::Gold, eOrientationType::Vertical);
	signalConnectionManager.connect (goldBar->valueChanged, [this]() { goldChanged(); });
	goldBar->disable();
	goldBarAmountLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (360, 275), getPosition() + cPosition (360 + 44, 275 + 10)), std::to_string (initialGold), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	//
	// Upgrade Buttons
	//
	for (size_t i = 0; i < maxUpgradeButtons; ++i)
	{
		upgradeDecreaseButton[i] = emplaceChild<cPushButton> (getPosition() + cPosition (283, 293 + 19 * i), ePushButtonType::ArrowLeftSmall, &SoundData.SNDObjectMenu);
		signalConnectionManager.connect (upgradeDecreaseButton[i]->clicked, [this, i]() { upgradeDecreaseClicked (i); });

		upgradeIncreaseButton[i] = emplaceChild<cPushButton> (getPosition() + cPosition (283 + 18, 293 + 19 * i), ePushButtonType::ArrowRightSmall, &SoundData.SNDObjectMenu);
		signalConnectionManager.connect (upgradeIncreaseButton[i]->clicked, [this, i]() { upgradeIncreaseClicked (i); });

		upgradeCostLabel[i] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (283 + 40, 293 + 2 + 19 * i), getPosition() + cPosition (283 + 40 + 40, 293 + 2 + 19 * i + 10)), "");
	}

	//
	// Initialization
	//
	for (const auto& [unitId, cargo] : initialUnits)
	{
		auto& addedItem = addSelectedUnit (unitId);
		addedItem.setCargo (cargo);
		fixedSelectedUnits.emplace (&addedItem, cargo);
	}

	generateSelectionList (true);
	updateUpgradeButtons();
	handleSelectedUnitSelectionChanged (nullptr);

	signalConnectionManager.connect (selectedUnitSelectionChanged, [this] (cUnitListViewItemCargo* unitItem) { handleSelectedUnitSelectionChanged (unitItem); });
}

//------------------------------------------------------------------------------
cWindowLandingUnitSelection::~cWindowLandingUnitSelection()
{}

//------------------------------------------------------------------------------
void cWindowLandingUnitSelection::retranslate()
{
	cWindow::retranslate();

	titleLabel->setText (lngPack.i18n ("Title~Choose_Units"));
	buyCheckBox->setText (lngPack.i18n ("Others~Buy"));

	upgradeCheckBox->setText (lngPack.i18n ("Others~Upgrade"));
	cargoLabel->setText (lngPack.i18n ("Title~Cargo"));
	creditLabel->setText (lngPack.i18n ("Title~Credits"));
}

//------------------------------------------------------------------------------
std::vector<sLandingUnit> cWindowLandingUnitSelection::getLandingUnits() const
{
	std::vector<sLandingUnit> result;

	for (size_t i = 0; i < getSelectedUnitsCount(); ++i)
	{
		const auto& selectedUnitItem = getSelectedUnit (i);

		sLandingUnit landingUnit;
		landingUnit.unitID = selectedUnitItem.getUnitId();
		landingUnit.cargo = selectedUnitItem.getCargo();
		result.push_back (std::move (landingUnit));
	}

	return result;
}

//------------------------------------------------------------------------------
std::vector<std::pair<sID, cUnitUpgrade>> cWindowLandingUnitSelection::getUnitUpgrades() const
{
	std::vector<std::pair<sID, cUnitUpgrade>> result;

	for (const auto& p : unitUpgrades)
	{
		if (p.second.hasBeenPurchased())
		{
			result.push_back (p);
		}
	}

	return result;
}

//------------------------------------------------------------------------------
void cWindowLandingUnitSelection::setActiveUnit (const sID& unitId)
{
	cWindowAdvancedHangar<cUnitListViewItemCargo>::setActiveUnit (unitId);

	cUnitUpgrade* unitUpgrade;
	auto iter = unitUpgrades.find (unitId);
	if (iter == unitUpgrades.end())
	{
		unitUpgrade = &unitUpgrades[unitId];

		unitUpgrade->init (unitsData->getDynamicUnitData (unitId, getPlayer().getClan()), *getPlayer().getLastUnitData (unitId), unitsData->getStaticUnitData (unitId), getPlayer().getResearchState());
	}
	else
	{
		unitUpgrade = &iter->second;
	}

	setActiveUpgrades (*unitUpgrade);

	updateUpgradeButtons();
}

//------------------------------------------------------------------------------
void cWindowLandingUnitSelection::updateUpgradeButtons()
{
	auto activeUnitId = getActiveUnit();
	if (!activeUnitId)
	{
		for (size_t i = 0; i < maxUpgradeButtons; ++i)
		{
			upgradeDecreaseButton[i]->lock();
			upgradeIncreaseButton[i]->lock();
			upgradeCostLabel[i]->hide();
		}
		return;
	}

	auto& unitUpgrade = unitUpgrades.at (*activeUnitId);

	for (size_t i = 0; i < maxUpgradeButtons; ++i)
	{
		const sUnitUpgrade& upgrade = unitUpgrade.upgrades[i];

		if (upgrade.getType() == sUnitUpgrade::eUpgradeType::None)
		{
			upgradeDecreaseButton[i]->lock();
			upgradeIncreaseButton[i]->lock();
			upgradeCostLabel[i]->hide();
			continue;
		}

		if (upgrade.getNextPrice())
		{
			upgradeCostLabel[i]->setText (std::to_string (*upgrade.getNextPrice()));
			upgradeCostLabel[i]->show();
		}
		else
		{
			upgradeCostLabel[i]->hide();
		}

		if (upgrade.getNextPrice() && goldBar->getValue() >= *upgrade.getNextPrice())
		{
			upgradeIncreaseButton[i]->unlock();
		}
		else
		{
			upgradeIncreaseButton[i]->lock();
		}

		if (upgrade.getPurchased() > 0)
		{
			upgradeDecreaseButton[i]->unlock();
		}
		else
		{
			upgradeDecreaseButton[i]->lock();
		}
	}
}

//------------------------------------------------------------------------------
bool cWindowLandingUnitSelection::tryAddSelectedUnit (const cUnitListViewItemBuy& unitItem) const
{
	const auto& unitId = unitItem.getUnitId();
	if (!unitsData->isValidId (unitId)) return false;

	const auto unitData = unitsData->getStaticUnitData (unitId);

	// is this unit type allowed to be bought for landing?
	if (unitId.isABuilding()) return false;
	if (unitData.factorGround == 0) return false;
	if (unitData.vehicleData.isHuman) return false;

	int buildCosts = unitsData->getDynamicUnitData (unitId, getPlayer().getClan()).getBuildCost();
	if (buildCosts > goldBar->getValue()) return false;

	goldBar->decrease (buildCosts);

	return true;
}

//------------------------------------------------------------------------------
bool cWindowLandingUnitSelection::tryRemoveSelectedUnit (const cUnitListViewItemCargo& unitItem) const
{
	if (fixedSelectedUnits.find (&unitItem) != fixedSelectedUnits.end()) return false;

	const auto& unitId = unitItem.getUnitId();
	int buildCosts = unitsData->getDynamicUnitData (unitId, getPlayer().getClan()).getBuildCost();

	const auto value = buildCosts + (unitItem.getCargo() / singleCreditResourceAmount);

	if (goldBar->getValue() + value > goldBar->getMaxValue()) return false;

	goldBar->increase (value);

	return true;
}

//------------------------------------------------------------------------------
void cWindowLandingUnitSelection::generateSelectionList (bool select)
{
	const bool buy = buyCheckBox->isChecked();
	const bool tank = tankCheckBox->isChecked();
	const bool plane = planeCheckBox->isChecked() && !buy;
	const bool ship = shipCheckBox->isChecked() && !buy;
	const bool build = buildingCheckBox->isChecked() && !buy;
	const bool tnt = tntCheckBox->isChecked();

	clearSelectionUnits();

	for (const auto& data : unitsData->getStaticUnitsData())
	{
		if (data.isAlien) continue;
		if (data.ID.isABuilding() && !build) continue;
		if (data.vehicleData.isHuman && buy) continue;
		if (!data.canAttack && tnt) continue;
		if (!data.ID.isABuilding())
		{
			if (data.factorAir > 0 && !plane) continue;
			if (data.factorSea > 0 && data.factorGround == 0 && !ship) continue;
			if (data.factorGround > 0 && !tank) continue;
		}

		auto& item = addSelectionUnit (data.ID);
		if (data.ID.isABuilding() || data.vehicleData.isHuman || data.factorAir > 0 || (data.factorSea > 0 && data.factorGround == 0))
		{
			item.hidePrice();
		}
		else if (goldBar->getValue() < item.getCost())
		{
			item.markAsInsufficient();
		}
		if (select)
		{
			setSelectedSelectionItem (item);
			select = false;
		}
	}
}

//------------------------------------------------------------------------------
std::pair<int, int> cWindowLandingUnitSelection::testBuyCargo (const cUnitListViewItemCargo& unit, int amount) const
{
	auto it = fixedSelectedUnits.find (&unit);
	const int min_cargo = (it != fixedSelectedUnits.end() ? it->second : 0);

	amount = std::clamp (amount, min_cargo - unit.getCargo(), metalBar->getMaxValue() - unit.getCargo());
	auto price = amount / singleCreditResourceAmount; // may be negative (if we are selling)

	if (goldBar->getValue() - price < goldBar->getMinValue())
	{
		price = goldBar->getValue() - goldBar->getMinValue();
		amount = price * singleCreditResourceAmount;
	}
	if (goldBar->getValue() - price > goldBar->getMaxValue())
	{
		price = goldBar->getValue() - goldBar->getMaxValue();
		amount = price * singleCreditResourceAmount;
	}

	return std::make_pair (price, amount);
}

//------------------------------------------------------------------------------
void cWindowLandingUnitSelection::metalChanged()
{
	if (selectedCargoUnit)
	{
		const auto oldCargo = selectedCargoUnit->getCargo();
		const auto desiredCargo = metalBar->getValue();
		const auto desiredBuy = desiredCargo - oldCargo;
		const auto buyResult = testBuyCargo (*selectedCargoUnit, desiredBuy);

		if (buyResult.second != desiredBuy)
		{
			metalBar->setValue (oldCargo + buyResult.second); // will trigger metalChanged() again
			return;
		}
		else
		{
			goldBar->decrease (buyResult.first);
			selectedCargoUnit->setCargo (metalBar->getValue());
		}
	}
	metalBarAmountLabel->setText (std::to_string (metalBar->getValue()));
}

//------------------------------------------------------------------------------
void cWindowLandingUnitSelection::goldChanged()
{
	goldBarAmountLabel->setText (std::to_string (goldBar->getValue()));
	for (std::size_t i = 0; i != selectionUnitList->getItemsCount(); ++i)
	{
		auto& item = selectionUnitList->getItem (i);

		if (item.isCostVisible() && goldBar->getValue() < item.getCost())
		{
			item.markAsInsufficient();
		}
		else
		{
			item.unmarkAsInsufficient();
		}
	}
}

//------------------------------------------------------------------------------
void cWindowLandingUnitSelection::metalUpButtonClicked()
{
	metalBar->increase (metalBarSteps);
}

//------------------------------------------------------------------------------
void cWindowLandingUnitSelection::metalDownButtonClicked()
{
	metalBar->decrease (metalBarSteps);
}

//------------------------------------------------------------------------------
void cWindowLandingUnitSelection::upgradeIncreaseClicked (size_t index)
{
	auto activeUnitId = getActiveUnit();
	if (!activeUnitId) return;

	auto& unitUpgrade = unitUpgrades.at (*activeUnitId);
	const auto& researchLevel = getPlayer().getResearchState();

	const auto cost = unitUpgrade.upgrades[index].purchase (researchLevel);
	goldBar->decrease (cost);

	updateUpgradeButtons();
}

//------------------------------------------------------------------------------
void cWindowLandingUnitSelection::upgradeDecreaseClicked (size_t index)
{
	auto activeUnitId = getActiveUnit();
	if (!activeUnitId) return;

	auto& unitUpgrade = unitUpgrades.at (*activeUnitId);
	const auto& researchLevel = getPlayer().getResearchState();

	const int cost = unitUpgrade.upgrades[index].cancelPurchase (researchLevel);
	goldBar->increase (-cost);

	updateUpgradeButtons();
}

//------------------------------------------------------------------------------
void cWindowLandingUnitSelection::handleSelectedUnitSelectionChanged (cUnitListViewItemCargo* unitItem)
{
	if (unitItem == nullptr || !(unitsData->getStaticUnitData (unitItem->getUnitId()).storeResType == eResourceType::Metal || unitsData->getStaticUnitData (unitItem->getUnitId()).storeResType == eResourceType::Oil))
	{
		selectedCargoUnit = nullptr;
		metalBar->setValue (0);
		metalBar->disable();
		metalBarAmountLabel->hide();
		metalBarUpButton->lock();
		metalBarDownButton->lock();
	}
	else
	{
		selectedCargoUnit = nullptr;
		const auto& data = unitsData->getStaticUnitData (unitItem->getUnitId());
		if (data.storeResType == eResourceType::Oil)
		{
			metalBar->setType (eResourceBarType::Oil);
		}
		else
		{
			metalBar->setType (eResourceBarType::Metal);
		}
		metalBar->setMinValue (0);
		metalBar->setMaxValue (data.storageResMax);
		metalBar->setValue (unitItem->getCargo());
		metalBar->enable();
		metalBarAmountLabel->show();
		metalBarAmountLabel->setText (std::to_string (metalBar->getValue()));
		metalBarUpButton->unlock();
		metalBarDownButton->unlock();
		selectedCargoUnit = unitItem;
	}
}
