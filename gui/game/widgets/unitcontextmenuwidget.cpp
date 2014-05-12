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

#include "unitcontextmenuwidget.h"
#include "../../menu/widgets/pushbutton.h"
#include "../../menu/widgets/radiogroup.h"
#include "../../menu/widgets/checkbox.h"
#include "../../../unit.h"
#include "../../../map.h"

//------------------------------------------------------------------------------
cUnitContextMenuWidget::cUnitContextMenuWidget () :
	unit (nullptr)
{}

//------------------------------------------------------------------------------
void cUnitContextMenuWidget::setUnit (const cUnit* unit_, eMouseModeType mouseInputMode, const cPlayer* player, const cMap* dynamicMap)
{
	unit = unit_;
	removeChildren ();

	if (unit == nullptr) return;

	cBox<cPosition> area (getPosition (), getPosition ());
	cPosition nextButtonPosition = getPosition ();
	auto mouseActionGroup = addChild (std::make_unique<cRadioGroup> (true));
	if (!unit->isDisabled () && unit->owner == player)
	{
		// Attack
		if (unit->data.canAttack && unit->data.getShots ())
		{
			auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Text~Others~Attack_7"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, SoundData.SNDObjectMenu.get ()));
			button->setChecked (mouseInputMode == eMouseModeType::Attack);
			button->toggled.connect ([&](){ attackToggled (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Build:
		if (unit->data.canBuild.empty () == false && unit->isUnitBuildingABuilding () == false)
		{
			auto button = addChild (std::make_unique<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, SoundData.SNDObjectMenu.get (), lngPack.i18n ("Text~Others~Build_7"), FONT_LATIN_SMALL_WHITE));
			button->clicked.connect ([&](){ buildClicked (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Distribute:
		if (unit->data.canMineMaxRes > 0 && unit->isUnitWorking ())
		{
			auto button = addChild (std::make_unique<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, SoundData.SNDObjectMenu.get (), lngPack.i18n ("Text~Others~Distribution_7"), FONT_LATIN_SMALL_WHITE));
			button->clicked.connect ([&](){ distributeClicked (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Transfer:
		if (unit->data.storeResType != sUnitData::STORE_RES_NONE && unit->isUnitBuildingABuilding () == false && unit->isUnitClearing () == false)
		{
			auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Text~Others~Transfer_7"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, SoundData.SNDObjectMenu.get ()));
			button->setChecked (mouseInputMode == eMouseModeType::Transfer);
			button->toggled.connect ([&](){ transferToggled (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Start:
		if (unit->data.canWork && unit->buildingCanBeStarted ())
		{
			auto button = addChild (std::make_unique<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, SoundData.SNDObjectMenu.get (), lngPack.i18n ("Text~Others~Start_7"), FONT_LATIN_SMALL_WHITE));
			button->clicked.connect ([&](){ startClicked (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Auto survey movejob of surveyor
		if (unit->data.canSurvey)
		{
			auto button = addChild (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Text~Others~Auto_7"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, SoundData.SNDObjectMenu.get ()));
			button->setChecked (unit->isAutoMoveJobActive ());
			button->toggled.connect ([&](){ autoToggled (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Stop:
		if (unit->canBeStoppedViaUnitMenu ())
		{
			auto button = addChild (std::make_unique<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, SoundData.SNDObjectMenu.get (), lngPack.i18n ("Text~Others~Stop_7"), FONT_LATIN_SMALL_WHITE));
			button->clicked.connect ([&](){ stopClicked (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Remove:
		if (unit->data.canClearArea && dynamicMap && dynamicMap->getField(unit->getPosition()).getRubble () && unit->isUnitClearing () == false)
		{
			auto button = addChild (std::make_unique<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, SoundData.SNDObjectMenu.get (), lngPack.i18n ("Text~Others~Clear_7"), FONT_LATIN_SMALL_WHITE));
			button->clicked.connect ([&](){ removeClicked (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Manual fire
		if ((unit->isManualFireActive() || unit->data.canAttack))
		{
			auto button = addChild (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Text~Others~ManualFireMode_7"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, SoundData.SNDObjectMenu.get ()));
			button->setChecked (unit->isManualFireActive());
			button->toggled.connect ([&](){ manualFireToggled (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Sentry status:
		if ((unit->isSentryActive() || unit->data.canAttack || (!unit->isABuilding () && !unit->canBeStoppedViaUnitMenu ())))
		{
			auto button = addChild (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Text~Others~Sentry"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, SoundData.SNDObjectMenu.get ()));
			button->setChecked (unit->isSentryActive());
			button->toggled.connect ([&](){ sentryToggled (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Activate
		if (unit->data.storageUnitsMax > 0)
		{
			auto button = addChild (std::make_unique<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, SoundData.SNDObjectMenu.get(), lngPack.i18n ("Text~Others~Active_7"), FONT_LATIN_SMALL_WHITE));
			button->clicked.connect ([&](){ activateClicked (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Load
		if (unit->data.storageUnitsMax > 0)
		{
			auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Text~Others~Load_7"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, SoundData.SNDObjectMenu.get ()));
			button->setChecked (mouseInputMode == eMouseModeType::Load);
			button->toggled.connect ([&](){ loadToggled (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// research
		if (unit->data.canResearch && unit->isUnitWorking ())
		{
			auto button = addChild (std::make_unique<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, SoundData.SNDObjectMenu.get (), lngPack.i18n ("Text~Others~Research"), FONT_LATIN_SMALL_WHITE));
			button->clicked.connect ([&](){ researchClicked (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// gold upgrades screen
		if (unit->data.convertsGold)
		{
			auto button = addChild (std::make_unique<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, SoundData.SNDObjectMenu.get (), lngPack.i18n ("Text~Others~Upgrademenu_7"), FONT_LATIN_SMALL_WHITE));
			button->clicked.connect ([&](){ buyUpgradesClicked (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Update this
		if (unit->buildingCanBeUpgraded ())
		{
			auto button = addChild (std::make_unique<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, SoundData.SNDObjectMenu.get (), lngPack.i18n ("Text~Others~Upgradethis_7"), FONT_LATIN_SMALL_WHITE));
			button->clicked.connect ([&](){ upgradeThisClicked (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Update all
		if (unit->buildingCanBeUpgraded ())
		{
			auto button = addChild (std::make_unique<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, SoundData.SNDObjectMenu.get (), lngPack.i18n ("Text~Others~UpgradeAll_7"), FONT_LATIN_SMALL_WHITE));
			button->clicked.connect ([&](){ upgradeAllClicked (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Self destruct
		if (unit->data.canSelfDestroy)
		{
			auto button = addChild (std::make_unique<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, SoundData.SNDObjectMenu.get (), lngPack.i18n ("Text~Others~Destroy_7"), FONT_LATIN_SMALL_WHITE));
			button->clicked.connect ([&](){ selfDestroyClicked (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Ammo:
		if (unit->data.canRearm && unit->data.storageResCur >= 1)
		{
			auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Text~Others~Reload_7"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, SoundData.SNDObjectMenu.get ()));
			button->setChecked (mouseInputMode == eMouseModeType::SupplyAmmo);
			button->toggled.connect ([&](){ supplyAmmoToggled (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Repair:
		if (unit->data.canRepair && unit->data.storageResCur >= 1)
		{
			auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Text~Others~Repair_7"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, SoundData.SNDObjectMenu.get ()));
			button->setChecked (mouseInputMode == eMouseModeType::Repair);
			button->toggled.connect ([&](){ repairToggled (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Lay mines:
		if (unit->data.canPlaceMines && unit->data.storageResCur > 0)
		{
			auto button = addChild (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Text~Others~Seed"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, SoundData.SNDObjectMenu.get ()));
			button->setChecked (unit->isUnitLayingMines());
			button->toggled.connect ([&](){ layMinesToggled (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Collect/clear mines:
		if (unit->data.canPlaceMines && unit->data.storageResCur < unit->data.storageResMax)
		{
			auto button = addChild (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Text~Others~Clear_7"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, SoundData.SNDObjectMenu.get ()));
			button->setChecked (unit->isUnitClearingMines());
			button->toggled.connect ([&](){ collectMinesToggled (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Sabotage/disable:
		if (unit->data.canDisable && unit->data.getShots ())
		{
			auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Text~Others~Disable_7"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, SoundData.SNDObjectMenu.get ()));
			button->setChecked (mouseInputMode == eMouseModeType::Disable);
			button->toggled.connect ([&](){ sabotageToggled (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}

		// Steal:
		if (unit->data.canCapture && unit->data.getShots ())
		{
			auto button = mouseActionGroup->addButton (std::make_unique<cCheckBox> (nextButtonPosition, lngPack.i18n ("Text~Others~Steal_7"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Right, eCheckBoxType::UnitContextMenu, false, SoundData.SNDObjectMenu.get ()));
			button->setChecked (mouseInputMode == eMouseModeType::Steal);
			button->toggled.connect ([&](){ stealToggled (); });
			nextButtonPosition.y () += button->getSize ().y ();
			area.add (button->getArea ());
		}
	}
	// Info
	{
		auto button = addChild (std::make_unique<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, SoundData.SNDObjectMenu.get (), lngPack.i18n ("Text~Others~Info_7"), FONT_LATIN_SMALL_WHITE));
		button->clicked.connect ([&](){ infoClicked (); });
		nextButtonPosition.y () += button->getSize ().y ();
		area.add (button->getArea ());
	}

	// Done
	{
		auto button = addChild (std::make_unique<cPushButton> (nextButtonPosition, ePushButtonType::UnitContextMenu, SoundData.SNDObjectMenu.get (), lngPack.i18n ("Text~Others~Done_7"), FONT_LATIN_SMALL_WHITE));
		button->clicked.connect ([&](){ doneClicked (); });
		nextButtonPosition.y () += button->getSize ().y ();
		area.add (button->getArea ());
	}

	resize (area.getSize());
}

//------------------------------------------------------------------------------
const cUnit* cUnitContextMenuWidget::getUnit ()
{
	return unit;
}