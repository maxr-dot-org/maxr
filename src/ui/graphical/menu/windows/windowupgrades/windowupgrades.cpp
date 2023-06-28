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

#include "windowupgrades.h"

#include "game/data/player/player.h"
#include "resources/pcx.h"
#include "ui/graphical/game/widgets/turntimeclockwidget.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/resourcebar.h"
#include "ui/graphical/menu/widgets/special/unitlistviewitembuy.h"
#include "ui/uidefines.h"
#include "ui/widgets/label.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cWindowUpgrades::cWindowUpgrades (const cPlayer& player, std::shared_ptr<const cTurnTimeClock> turnTimeClock, std::shared_ptr<cWindowUpgradesFilterState> filterState_, std::shared_ptr<const cUnitsData> unitsData) :
	cWindowHangar (LoadPCX (GFXOD_UPGRADE), unitsData, player),
	filterState (filterState_)
{
	titleLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (328, 12), getPosition() + cPosition (328 + 157, 12 + 10)), lngPack.i18n ("Title~Upgrades_Menu"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	auto turnTimeClockWidget = emplaceChild<cTurnTimeClockWidget> (cBox<cPosition> (cPosition (523, 16), cPosition (523 + 65, 16 + 10)));
	turnTimeClockWidget->setTurnTimeClock (std::move (turnTimeClock));

	//
	// Unit Filters
	//
	tankCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (467, 411), eCheckBoxType::Tank);
	tankCheckBox->setChecked (filterState->TankChecked);
	signalConnectionManager.connect (tankCheckBox->toggled, [this]() { generateSelectionList (false); });

	planeCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (467 + 33, 411), eCheckBoxType::Plane);
	planeCheckBox->setChecked (filterState->PlaneChecked);
	signalConnectionManager.connect (planeCheckBox->toggled, [this]() { generateSelectionList (false); });

	shipCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (467 + 33 * 2, 411), eCheckBoxType::Ship);
	shipCheckBox->setChecked (filterState->ShipChecked);
	signalConnectionManager.connect (shipCheckBox->toggled, [this]() { generateSelectionList (false); });

	buildingCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (467 + 33 * 3, 411), eCheckBoxType::Building);
	buildingCheckBox->setChecked (filterState->BuildingChecked);
	signalConnectionManager.connect (buildingCheckBox->toggled, [this]() { generateSelectionList (false); });

	tntCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (467 + 33 * 4, 411), eCheckBoxType::Tnt);
	tntCheckBox->setChecked (filterState->TNTChecked);
	signalConnectionManager.connect (tntCheckBox->toggled, [this]() { generateSelectionList (false); });

	//
	// Gold Bar
	//
	creditLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (362, 285), getPosition() + cPosition (362 + 40, 285 + 10)), lngPack.i18n ("Title~Credits"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);
	goldBar = emplaceChild<cResourceBar> (cBox<cPosition> (getPosition() + cPosition (372, 301), getPosition() + cPosition (372 + 20, 301 + 115)), 0, player.getCredits(), eResourceBarType::Gold, eOrientationType::Vertical);
	signalConnectionManager.connect (goldBar->valueChanged, [this]() { goldChanged(); });
	goldBar->disable();
	goldBarAmountLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (362, 275), getPosition() + cPosition (362 + 40, 275 + 10)), std::to_string (player.getCredits()), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

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

	generateSelectionList (true);
	updateUpgradeButtons();
}

//------------------------------------------------------------------------------
void cWindowUpgrades::retranslate()
{
	cWindowHangar::retranslate();
	titleLabel->setText (lngPack.i18n ("Title~Upgrades_Menu"));
	creditLabel->setText (lngPack.i18n ("Title~Credits"));
}

//------------------------------------------------------------------------------
std::vector<std::pair<sID, cUnitUpgrade>> cWindowUpgrades::getUnitUpgrades() const
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
void cWindowUpgrades::setActiveUnit (const sID& unitId)
{
	cWindowHangar::setActiveUnit (unitId);

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
void cWindowUpgrades::updateUpgradeButtons()
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
void cWindowUpgrades::generateSelectionList (bool select)
{
	//save state of the filter button
	filterState->TankChecked = tankCheckBox->isChecked();
	filterState->PlaneChecked = planeCheckBox->isChecked();
	filterState->ShipChecked = shipCheckBox->isChecked();
	filterState->BuildingChecked = buildingCheckBox->isChecked();
	filterState->TNTChecked = tntCheckBox->isChecked();

	const bool tank = tankCheckBox->isChecked();
	const bool plane = planeCheckBox->isChecked();
	const bool ship = shipCheckBox->isChecked();
	const bool build = buildingCheckBox->isChecked();
	const bool tnt = tntCheckBox->isChecked();

	clearSelectionUnits();

	for (const auto& data : unitsData->getStaticUnitsData())
	{
		if (data.isAlien) continue;
		if (data.ID.isABuilding() && !build) continue;
		if (!data.canAttack && tnt) continue;
		if (!data.ID.isABuilding())
		{
			if (data.factorAir > 0 && !plane) continue;
			if (data.factorSea > 0 && data.factorGround == 0 && !ship) continue;
			if (data.factorGround > 0 && !tank) continue;
		}

		auto& item = addSelectionUnit (data.ID);
		item.hidePrice();
		if (select)
		{
			setSelectedSelectionItem (item);
			select = false;
		}
	}
}

//------------------------------------------------------------------------------
void cWindowUpgrades::goldChanged()
{
	goldBarAmountLabel->setText (std::to_string (goldBar->getValue()));
}

//------------------------------------------------------------------------------
void cWindowUpgrades::upgradeIncreaseClicked (size_t index)
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
void cWindowUpgrades::upgradeDecreaseClicked (size_t index)
{
	auto activeUnitId = getActiveUnit();
	if (!activeUnitId) return;

	auto& unitUpgrade = unitUpgrades.at (*activeUnitId);
	const auto& researchLevel = getPlayer().getResearchState();

	const auto cost = unitUpgrade.upgrades[index].cancelPurchase (researchLevel);
	goldBar->increase (-cost);

	updateUpgradeButtons();
}
