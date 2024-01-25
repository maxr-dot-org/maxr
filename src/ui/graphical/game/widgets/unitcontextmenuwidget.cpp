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

#include "ui/graphical/game/widgets/unitcontextmenuwidget.h"

#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/units/building.h"
#include "game/data/units/unit.h"
#include "game/data/units/vehicle.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/radiogroup.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
void cUnitContextMenuWidget::setUnit (const cUnit* unit_, eMouseModeType mouseInputMode, const cPlayer* player, const cMapView* map)
{
	unit = unit_;
	removeChildren();

	if (unit == nullptr) return;

	cBox<cPosition> area (getPosition(), getPosition());
	cPosition nextButtonPosition = getPosition();
	auto mouseActionGroup = emplaceChild<cRadioGroup> (true);

	// Attack
	if (unitHasAttackEntry (unit, player))
	{
		auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Others~Attack_7"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, &SoundData.SNDObjectMenu));
		button->setChecked (mouseInputMode == eMouseModeType::Attack);
		button->toggled.connect ([this]() { attackToggled(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Build:
	if (unitHasBuildEntry (unit, player))
	{
		auto button = emplaceChild<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, &SoundData.SNDObjectMenu, lngPack.i18n ("Others~Build_7"), eUnicodeFontType::LatinSmallWhite);
		button->clicked.connect ([this]() { buildClicked(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	const cBuilding* building = dynamic_cast<const cBuilding*> (unit);
	// Distribute:
	if (unitHasDistributeEntry (building, player))
	{
		auto button = emplaceChild<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, &SoundData.SNDObjectMenu, lngPack.i18n ("Others~Distribution_7"), eUnicodeFontType::LatinSmallWhite);
		button->clicked.connect ([this]() { distributeClicked(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Transfer:
	if (unitHasTransferEntry (unit, player))
	{
		auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Others~Transfer_7"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, &SoundData.SNDObjectMenu));
		button->setChecked (mouseInputMode == eMouseModeType::Transfer);
		button->toggled.connect ([this]() { transferToggled(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Start:
	if (unitHasStartEntry (building, player))
	{
		auto button = emplaceChild<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, &SoundData.SNDObjectMenu, lngPack.i18n ("Others~Start_7"), eUnicodeFontType::LatinSmallWhite);
		button->clicked.connect ([this]() { startClicked(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Auto survey movejob of surveyor
	const cVehicle* vehicle = dynamic_cast<const cVehicle*> (unit);
	if (unitHasAutoEntry (vehicle, player))
	{
		auto button = emplaceChild<cCheckBox> (nextButtonPosition, lngPack.i18n ("Others~Auto_7"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, &SoundData.SNDObjectMenu);
		button->setChecked (vehicle->isSurveyorAutoMoveActive());
		button->toggled.connect ([this]() { autoToggled(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Stop:
	if (unitHasStopEntry (unit, player))
	{
		auto button = emplaceChild<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, &SoundData.SNDObjectMenu, lngPack.i18n ("Others~Stop_7"), eUnicodeFontType::LatinSmallWhite);
		button->clicked.connect ([this]() { stopClicked(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Remove:
	if (unitHasRemoveEntry (vehicle, player, map))
	{
		auto button = emplaceChild<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, &SoundData.SNDObjectMenu, lngPack.i18n ("Others~Clear_7"), eUnicodeFontType::LatinSmallWhite);
		button->clicked.connect ([this]() { removeClicked(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Manual fire
	if (unitHasManualFireEntry (unit, player))
	{
		auto button = emplaceChild<cCheckBox> (nextButtonPosition, lngPack.i18n ("Others~ManualFireMode_7"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, &SoundData.SNDObjectMenu);
		button->setChecked (unit->isManualFireActive());
		button->toggled.connect ([this]() { manualFireToggled(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Sentry status:
	if (unitHasSentryEntry (unit, player))
	{
		auto button = emplaceChild<cCheckBox> (nextButtonPosition, lngPack.i18n ("Others~Sentry"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, &SoundData.SNDObjectMenu);
		button->setChecked (unit->isSentryActive());
		button->toggled.connect ([this]() { sentryToggled(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Activate
	if (unitHasActivateEntry (unit, player))
	{
		auto button = emplaceChild<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, &SoundData.SNDObjectMenu, lngPack.i18n ("Others~Active_7"), eUnicodeFontType::LatinSmallWhite);
		button->clicked.connect ([this]() { activateClicked(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Load
	if (unitHasLoadEntry (unit, player))
	{
		auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Others~Load_7"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, &SoundData.SNDObjectMenu));
		button->setChecked (mouseInputMode == eMouseModeType::Load);
		button->toggled.connect ([this]() { loadToggled(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Enter
	if (unitHasEnterEntry (vehicle, player))
	{
		auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Others~Enter_7"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, &SoundData.SNDObjectMenu));
		button->setChecked (mouseInputMode == eMouseModeType::Enter);
		button->toggled.connect ([this]() { enterToggled(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// research
	if (unitHasResearchEntry (building, player))
	{
		auto button = emplaceChild<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, &SoundData.SNDObjectMenu, lngPack.i18n ("Others~Research"), eUnicodeFontType::LatinSmallWhite);
		button->clicked.connect ([this]() { researchClicked(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// gold upgrades screen
	if (unitHasBuyEntry (building, player))
	{
		auto button = emplaceChild<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, &SoundData.SNDObjectMenu, lngPack.i18n ("Others~Upgrademenu_7"), eUnicodeFontType::LatinSmallWhite);
		button->clicked.connect ([this]() { buyUpgradesClicked(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Update this
	if (unitHasUpgradeThisEntry (building, player))
	{
		auto button = emplaceChild<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, &SoundData.SNDObjectMenu, lngPack.i18n ("Others~Upgradethis_7"), eUnicodeFontType::LatinSmallWhite);
		button->clicked.connect ([this]() { upgradeThisClicked(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Update all
	if (unitHasUpgradeAllEntry (building, player))
	{
		auto button = emplaceChild<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, &SoundData.SNDObjectMenu, lngPack.i18n ("Others~UpgradeAll_7"), eUnicodeFontType::LatinSmallWhite);
		button->clicked.connect ([this]() { upgradeAllClicked(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Self destruct
	if (unitHasSelfDestroyEntry (building, player))
	{
		auto button = emplaceChild<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, &SoundData.SNDObjectMenu, lngPack.i18n ("Others~Destroy_7"), eUnicodeFontType::LatinSmallWhite);
		button->clicked.connect ([this]() { selfDestroyClicked(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Ammo:
	if (unitHasSupplyEntry (vehicle, player))
	{
		auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Others~Reload_7"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, &SoundData.SNDObjectMenu));
		button->setChecked (mouseInputMode == eMouseModeType::SupplyAmmo);
		button->toggled.connect ([this]() { supplyAmmoToggled(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Repair:
	if (unitHasRepairEntry (vehicle, player))
	{
		auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Others~Repair_7"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, &SoundData.SNDObjectMenu));
		button->setChecked (mouseInputMode == eMouseModeType::Repair);
		button->toggled.connect ([this]() { repairToggled(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Lay mines:
	if (unitHasLayMinesEntry (vehicle, player))
	{
		auto button = emplaceChild<cCheckBox> (nextButtonPosition, lngPack.i18n ("Others~Seed"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, &SoundData.SNDObjectMenu);
		button->setChecked (vehicle->isUnitLayingMines());
		button->toggled.connect ([this]() { layMinesToggled(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Collect/clear mines:
	if (unitHasCollectMinesEntry (vehicle, player))
	{
		auto button = emplaceChild<cCheckBox> (nextButtonPosition, lngPack.i18n ("Others~Clear_7"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, &SoundData.SNDObjectMenu);
		button->setChecked (vehicle->isUnitClearingMines());
		button->toggled.connect ([this]() { collectMinesToggled(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Sabotage/disable:
	if (unitHasSabotageEntry (vehicle, player))
	{
		auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Others~Disable_7"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, &SoundData.SNDObjectMenu));
		button->setChecked (mouseInputMode == eMouseModeType::Disable);
		button->toggled.connect ([this]() { sabotageToggled(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Steal:
	if (unitHasStealEntry (vehicle, player))
	{
		auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Others~Steal_7"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, &SoundData.SNDObjectMenu));
		button->setChecked (mouseInputMode == eMouseModeType::Steal);
		button->toggled.connect ([this]() { stealToggled(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Info
	if (unitHasInfoEntry (unit, player))
	{
		auto button = emplaceChild<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, &SoundData.SNDObjectMenu, lngPack.i18n ("Others~Info_7"), eUnicodeFontType::LatinSmallWhite);
		button->clicked.connect ([this]() { infoClicked(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	// Done
	if (unitHasDoneEntry (unit, player))
	{
		auto button = emplaceChild<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, &SoundData.SNDObjectMenu, lngPack.i18n ("Others~Done_7"), eUnicodeFontType::LatinSmallWhite);
		button->clicked.connect ([this]() { doneClicked(); });
		nextButtonPosition.y() += button->getSize().y();
		area.add (button->getArea());
	}

	resize (area.getSize());
}

//------------------------------------------------------------------------------
const cUnit* cUnitContextMenuWidget::getUnit() const
{
	return unit;
}

//------------------------------------------------------------------------------
const cBuilding* cUnitContextMenuWidget::getBuilding() const
{
	return dynamic_cast<const cBuilding*> (unit);
}

//------------------------------------------------------------------------------
const cVehicle* cUnitContextMenuWidget::getVehicle() const
{
	return dynamic_cast<const cVehicle*> (unit);
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasAttackEntry (const cUnit* unit, const cPlayer* player)
{
	return unit && !unit->isDisabled() && unit->getOwner() == player && unit->getStaticUnitData().canAttack && unit->data.getShots();
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasBuildEntry (const cUnit* unit, const cPlayer* player)
{
	const auto* vehicle = dynamic_cast<const cVehicle*> (unit);
	return unit && !unit->isDisabled() && unit->getOwner() == player && !unit->getStaticUnitData().canBuild.empty() && (!vehicle || !vehicle->isUnitBuildingABuilding());
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasDistributeEntry (const cBuilding* building, const cPlayer* player)
{
	return building && !building->isDisabled() && building->getOwner() == player && building->getStaticData().canMineMaxRes > 0 && building->isUnitWorking();
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasTransferEntry (const cUnit* unit, const cPlayer* player)
{
	const auto* vehicle = dynamic_cast<const cVehicle*> (unit);
	return unit && !unit->isDisabled() && unit->getOwner() == player && unit->getStaticUnitData().storeResType != eResourceType::None && (!vehicle || !vehicle->isUnitBuildingABuilding()) && (!vehicle || !vehicle->isUnitClearing());
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasStartEntry (const cBuilding* building, const cPlayer* player)
{
	return building && !building->isDisabled() && building->getOwner() == player && building->buildingCanBeStarted();
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasAutoEntry (const cVehicle* vehicle, const cPlayer* player)
{
	return vehicle && !vehicle->isDisabled() && vehicle->getOwner() == player && vehicle->getStaticData().canSurvey;
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasStopEntry (const cUnit* unit, const cPlayer* player)
{
	return unit && !unit->isDisabled() && unit->getOwner() == player && unit->canBeStoppedViaUnitMenu();
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasRemoveEntry (const cVehicle* vehicle, const cPlayer* player, const cMapView* map)
{
	return vehicle && !vehicle->isDisabled() && vehicle->getOwner() == player && vehicle->getStaticData().canClearArea && map && map->getField (vehicle->getPosition()).getRubble() && !vehicle->isUnitClearing();
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasManualFireEntry (const cUnit* unit, const cPlayer* player)
{
	return unit && !unit->isDisabled() && unit->getOwner() == player && (unit->isManualFireActive() || unit->getStaticUnitData().canAttack);
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasSentryEntry (const cUnit* unit, const cPlayer* player)
{
	return unit && !unit->isDisabled() && unit->getOwner() == player && (unit->isSentryActive() || unit->getStaticUnitData().canAttack || (!unit->isABuilding() && !unit->canBeStoppedViaUnitMenu()));
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasActivateEntry (const cUnit* unit, const cPlayer* player)
{
	return unit && !unit->isDisabled() && unit->getOwner() == player && unit->getStaticUnitData().storageUnitsMax > 0;
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasLoadEntry (const cUnit* unit, const cPlayer* player)
{
	return unit && !unit->isDisabled() && unit->getOwner() == player && unit->getStaticUnitData().storageUnitsMax > 0;
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasEnterEntry (const cVehicle* vehicle, const cPlayer* player)
{
	return vehicle && !vehicle->isDisabled() && vehicle->getOwner() == player && !vehicle->isUnitClearing() && !vehicle->isUnitBuildingABuilding();
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasResearchEntry (const cBuilding* building, const cPlayer* player)
{
	return building && !building->isDisabled() && building->getOwner() == player && building->getStaticData().canResearch && building->isUnitWorking();
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasBuyEntry (const cBuilding* building, const cPlayer* player)
{
	return building && !building->isDisabled() && building->getOwner() == player && building->getStaticData().convertsGold;
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasUpgradeThisEntry (const cBuilding* building, const cPlayer* player)
{
	return building && !building->isDisabled() && building->getOwner() == player && building->buildingCanBeUpgraded();
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasUpgradeAllEntry (const cBuilding* building, const cPlayer* player)
{
	return building && !building->isDisabled() && building->getOwner() == player && building->buildingCanBeUpgraded();
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasSelfDestroyEntry (const cBuilding* building, const cPlayer* player)
{
	return building && !building->isDisabled() && building->getOwner() == player && building->getStaticData().canSelfDestroy;
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasSupplyEntry (const cVehicle* vehicle, const cPlayer* player)
{
	return vehicle && !vehicle->isDisabled() && vehicle->getOwner() == player && vehicle->getStaticUnitData().canRearm && vehicle->getStoredResources() > 0;
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasRepairEntry (const cVehicle* vehicle, const cPlayer* player)
{
	return vehicle && !vehicle->isDisabled() && vehicle->getOwner() == player && vehicle->getStaticUnitData().canRepair && vehicle->getStoredResources() > 0;
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasLayMinesEntry (const cVehicle* vehicle, const cPlayer* player)
{
	return vehicle && !vehicle->isDisabled() && vehicle->getOwner() == player && vehicle->getStaticData().canPlaceMines && vehicle->getStoredResources() > 0;
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasCollectMinesEntry (const cVehicle* vehicle, const cPlayer* player)
{
	return vehicle && !vehicle->isDisabled() && vehicle->getOwner() == player && vehicle->getStaticData().canPlaceMines && vehicle->getStoredResources() < vehicle->getStaticUnitData().storageResMax;
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasSabotageEntry (const cVehicle* vehicle, const cPlayer* player)
{
	return vehicle && !vehicle->isDisabled() && vehicle->getOwner() == player && vehicle->getStaticData().canDisable && vehicle->data.getShots();
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasStealEntry (const cVehicle* vehicle, const cPlayer* player)
{
	return vehicle && !vehicle->isDisabled() && vehicle->getOwner() == player && vehicle->getStaticData().canCapture && vehicle->data.getShots();
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasInfoEntry (const cUnit* unit, const cPlayer* player)
{
	return unit != nullptr;
}

//------------------------------------------------------------------------------
/*static*/ bool cUnitContextMenuWidget::unitHasDoneEntry (const cUnit* unit, const cPlayer* player)
{
	return unit != nullptr;
}
