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

#include "windowstorage.h"

#include "game/data/base/base.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/unit.h"
#include "game/data/units/vehicle.h"
#include "output/video/video.h"
#include "resources/pcx.h"
#include "resources/uidata.h"
#include "resources/vehicleuidata.h"
#include "ui/graphical/game/widgets/turntimeclockwidget.h"
#include "ui/graphical/game/widgets/unitdetailsstored.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/resourcebar.h"
#include "ui/translations.h"
#include "ui/uidefines.h"
#include "ui/widgets/application.h"
#include "ui/widgets/image.h"
#include "ui/widgets/label.h"
#include "utility/language.h"

#include <cassert>

//------------------------------------------------------------------------------
cWindowStorage::cWindowStorage (const cUnit& unit_, std::shared_ptr<const cTurnTimeClock> turnTimeClock) :
	cWindow (nullptr),
	unit (unit_),
	canRepairReloadUpgrade (unit_.isABuilding()),
	canStorePlanes (unit_.getStaticUnitData().storageUnitsImageType == eStorageUnitsImageType::Plane),
	canStoreShips (unit_.getStaticUnitData().storageUnitsImageType == eStorageUnitsImageType::Ship),
	columns (canStorePlanes ? 2 : 3),
	page (0)
{
	UniqueSurface background (LoadPCX (GFXOD_STORAGE));
	if (!canStorePlanes)
	{
		UniqueSurface surface (LoadPCX (GFXOD_STORAGE_GROUND));
		SDL_BlitSurface (surface.get(), nullptr, background.get(), nullptr);
	}
	setSurface (std::move (background));

	auto turnTimeClockWidget = emplaceChild<cTurnTimeClockWidget> (cBox<cPosition> (cPosition (523, 17), cPosition (523 + 62, 17 + 10)));
	turnTimeClockWidget->setTurnTimeClock (std::move (turnTimeClock));

	//
	// Units
	//
	const int stepX = canStorePlanes ? 227 : 156;
	const int stepImageX = canStorePlanes ? 227 : 156;
	const int startX = canStorePlanes ? 39 : 2;
	const int nameLabelX = canStorePlanes ? 190 : 118;

	for (size_t x = 0; x < columns; x++)
	{
		for (size_t y = 0; y < maxRows; y++)
		{
			const auto index = x + y * columns;

			activateButtons[index] = emplaceChild<cPushButton> (getPosition() + cPosition (startX + x * stepX, 188 + y * 236), ePushButtonType::Angular, lngPack.i18n ("Others~Active"), eUnicodeFontType::LatinNormal);
			signalConnectionManager.connect (activateButtons[index]->clicked, [this, index]() { activateClicked (index); });

			reloadButtons[index] = emplaceChild<cPushButton> (getPosition() + cPosition (startX + x * stepX, 188 + 23 + y * 236), ePushButtonType::Angular, canRepairReloadUpgrade ? lngPack.i18n ("Others~Reload") : "", eUnicodeFontType::LatinNormal);
			signalConnectionManager.connect (reloadButtons[index]->clicked, [this, index]() { reloadClicked (index); });

			repairButtons[index] = emplaceChild<cPushButton> (getPosition() + cPosition (78 + startX + x * stepX, 188 + y * 236), ePushButtonType::Angular, canRepairReloadUpgrade ? lngPack.i18n ("Others~Repair") : "", eUnicodeFontType::LatinNormal);
			signalConnectionManager.connect (repairButtons[index]->clicked, [this, index]() { repairClicked (index); });

			upgradeButtons[index] = emplaceChild<cPushButton> (getPosition() + cPosition (78 + startX + x * stepX, 188 + 23 + y * 236), ePushButtonType::Angular, canRepairReloadUpgrade ? lngPack.i18n ("Others~Upgrade") : "", eUnicodeFontType::LatinNormal);
			signalConnectionManager.connect (upgradeButtons[index]->clicked, [this, index]() { upgradeClicked (index); });

			unitImages[index] = emplaceChild<cImage> (getPosition() + cPosition (17 + x * stepImageX, 9 + y * 236));
			unitNames[index] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (17 + x * stepImageX + 5, 9 + y * 236 + 5), getPosition() + cPosition (17 + x * stepImageX + 5 + nameLabelX, 9 + y * 236 + 5 + 118)), "");
			unitNames[index]->setWordWrap (true);

			unitDetails[index] = emplaceChild<cUnitDetailsStored> (cBox<cPosition> (getPosition() + cPosition (17 + x * stepImageX + (canStorePlanes ? 35 : 0), 145 + y * 236), getPosition() + cPosition (17 + x * stepImageX + 130 + (canStorePlanes ? 35 : 0), 145 + y * 236 + 40)));
		}
	}

	//
	// Metal Bar
	//
	auto metalValue = 0;
	if (const auto* building = dynamic_cast<const cBuilding*> (&unit))
	{
		metalValue = building->subBase->getResourcesStored().metal;

		signalConnectionManager.connect (building->subBase->metalChanged, [=]() {
			metalBar->setValue (building->subBase->getResourcesStored().metal);
		});
	}

	metalBarAmountLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (536, 85), getPosition() + cPosition (536 + 40, 85 + 10)), std::to_string (metalValue), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);
	metalBar = emplaceChild<cResourceBar> (cBox<cPosition> (getPosition() + cPosition (546, 106), getPosition() + cPosition (546 + 20, 106 + 115)), 0, metalValue, eResourceBarType::Metal, eOrientationType::Vertical);
	signalConnectionManager.connect (metalBar->valueChanged, [this]() { metalBarAmountLabel->setText (std::to_string (metalBar->getValue())); });
	metalBar->disable();

	//
	// Buttons
	//
	upButton = emplaceChild<cPushButton> (getPosition() + cPosition (504, 426), ePushButtonType::ArrowUpBig);
	signalConnectionManager.connect (upButton->clicked, [this]() { upClicked(); });

	downButton = emplaceChild<cPushButton> (getPosition() + cPosition (532, 426), ePushButtonType::ArrowDownBig);
	signalConnectionManager.connect (downButton->clicked, [this]() { downClicked(); });

	activateAllButton = emplaceChild<cPushButton> (getPosition() + cPosition (518, 246), ePushButtonType::Angular, lngPack.i18n ("Others~Active"), eUnicodeFontType::LatinNormal);
	signalConnectionManager.connect (activateAllButton->clicked, [this]() { activateAllClicked(); });

	reloadAllButton = emplaceChild<cPushButton> (getPosition() + cPosition (518, 246 + 25), ePushButtonType::Angular, canRepairReloadUpgrade ? lngPack.i18n ("Others~Reload") : "", eUnicodeFontType::LatinNormal);
	signalConnectionManager.connect (reloadAllButton->clicked, [this]() { reloadAllClicked(); });

	repairAllButton = emplaceChild<cPushButton> (getPosition() + cPosition (518, 246 + 25 * 2), ePushButtonType::Angular, canRepairReloadUpgrade ? lngPack.i18n ("Others~Repair") : "", eUnicodeFontType::LatinNormal);
	signalConnectionManager.connect (repairAllButton->clicked, [this]() { repairAllClicked(); });

	upgradeAllButton = emplaceChild<cPushButton> (getPosition() + cPosition (518, 246 + 25 * 3), ePushButtonType::Angular, canRepairReloadUpgrade ? lngPack.i18n ("Others~Upgrade") : "", eUnicodeFontType::LatinNormal);
	signalConnectionManager.connect (upgradeAllButton->clicked, [this]() { upgradeAllClicked(); });

	doneButton = emplaceChild<cPushButton> (getPosition() + cPosition (518, 371), ePushButtonType::Angular, lngPack.i18n ("Others~Done"), eUnicodeFontType::LatinNormal);
	signalConnectionManager.connect (doneButton->clicked, [this]() { doneClicked(); });

	updateUnitsWidgets();
	updateGlobalButtons();
	updateUpDownButtons();

	signalConnectionManager.connect (unit.destroyed, [this]() { closeOnUnitDestruction(); });
}

//------------------------------------------------------------------------------
void cWindowStorage::retranslate()
{
	cWindow::retranslate();

	for (auto* activateButton : activateButtons)
	{
		activateButton->setText (lngPack.i18n ("Others~Active"));
	}
	for (auto* reloadButton : reloadButtons)
	{
		reloadButton->setText (canRepairReloadUpgrade ? lngPack.i18n ("Others~Reload") : "");
	}
	for (auto* repairButton : repairButtons)
	{
		repairButton->setText (canRepairReloadUpgrade ? lngPack.i18n ("Others~Repair") : "");
	}
	for (auto* upgradeButton : upgradeButtons)
	{
		upgradeButton->setText (canRepairReloadUpgrade ? lngPack.i18n ("Others~Upgrade") : "");
	}
	activateAllButton->setText (lngPack.i18n ("Others~Active"));
	reloadAllButton->setText (canRepairReloadUpgrade ? lngPack.i18n ("Others~Reload") : "");
	repairAllButton->setText (canRepairReloadUpgrade ? lngPack.i18n ("Others~Repair") : "");
	upgradeAllButton->setText (canRepairReloadUpgrade ? lngPack.i18n ("Others~Upgrade") : "");
	doneButton->setText (lngPack.i18n ("Others~Done"));
}

//------------------------------------------------------------------------------
void cWindowStorage::updateUnitButtons (const cVehicle& storedUnit, size_t positionIndex)
{
	if (!storedUnit.getOwner()) return;
	const auto& upgraded = *storedUnit.getOwner()->getLastUnitData (storedUnit.data.getId());

	activateButtons[positionIndex]->unlock();
	if (storedUnit.data.getAmmo() != storedUnit.data.getAmmoMax() && metalBar->getValue() >= 1)
		reloadButtons[positionIndex]->unlock();
	else
		reloadButtons[positionIndex]->lock();
	if (storedUnit.data.getHitpoints() != storedUnit.data.getHitpointsMax() && metalBar->getValue() >= 1)
		repairButtons[positionIndex]->unlock();
	else
		repairButtons[positionIndex]->lock();
	if (storedUnit.data.getVersion() != upgraded.getVersion() && metalBar->getValue() >= 1)
		upgradeButtons[positionIndex]->unlock();
	else
		upgradeButtons[positionIndex]->lock();
}

//------------------------------------------------------------------------------
void cWindowStorage::updateUnitName (const cVehicle& storedUnit, size_t positionIndex)
{
	if (!storedUnit.getOwner()) return;
	auto name = getDisplayName (storedUnit);

	const auto& upgraded = *storedUnit.getOwner()->getLastUnitData (storedUnit.data.getId());
	if (storedUnit.data.getVersion() != upgraded.getVersion())
	{
		name += "\n(" + lngPack.i18n ("Comp~Dated") + ")";
	}
	unitNames[positionIndex]->setText (name);
}

//------------------------------------------------------------------------------
void cWindowStorage::updateUnitsWidgets()
{
	unitsSignalConnectionManager.disconnectAll();

	for (size_t x = 0; x < columns; x++)
	{
		for (size_t y = 0; y < maxRows; y++)
		{
			const auto positionIndex = x + y * columns;
			const auto unitIndex = page * columns * maxRows + positionIndex;

			if (unitIndex < unit.storedUnits.size())
			{
				const auto& storedUnit = *unit.storedUnits[unitIndex];
				auto* uiData = UnitsUiData.getVehicleUI (storedUnit.getStaticUnitData().ID);

				UniqueSurface surface (SDL_CreateRGBSurface (0, uiData->storage->w, uiData->storage->h, Video.getColDepth(), 0, 0, 0, 0));
				SDL_BlitSurface (uiData->storage.get(), nullptr, surface.get(), nullptr);
				unitImages[positionIndex]->setImage (surface.get());

				unitDetails[positionIndex]->setUnit (&storedUnit);

				updateUnitName (storedUnit, positionIndex);
				updateUnitButtons (storedUnit, positionIndex);

				unitsSignalConnectionManager.connect (storedUnit.data.versionChanged, [this, positionIndex, &storedUnit]() { updateUnitName (storedUnit, positionIndex); });

				const auto onStoreUnitChanged = [this, positionIndex, &storedUnit]() {
					updateUnitButtons (storedUnit, positionIndex);
					updateGlobalButtons();
				};
				unitsSignalConnectionManager.connect (storedUnit.data.hitpointsChanged, onStoreUnitChanged);
				unitsSignalConnectionManager.connect (storedUnit.data.ammoChanged, onStoreUnitChanged);
				unitsSignalConnectionManager.connect (storedUnit.data.hitpointsMaxChanged, onStoreUnitChanged);
				unitsSignalConnectionManager.connect (storedUnit.data.ammoMaxChanged, onStoreUnitChanged);
				unitsSignalConnectionManager.connect (storedUnit.data.versionChanged, onStoreUnitChanged);
			}
			else
			{
				unitNames[positionIndex]->setText ("");

				SDL_Surface* srcSurface = nullptr;
				if (canStoreShips)
					srcSurface = GraphicsData.gfx_edock.get();
				else if (canStorePlanes)
					srcSurface = GraphicsData.gfx_ehangar.get();
				else
					srcSurface = GraphicsData.gfx_edepot.get();

				UniqueSurface surface (SDL_CreateRGBSurface (0, srcSurface->w, srcSurface->h, Video.getColDepth(), 0, 0, 0, 0));
				SDL_BlitSurface (srcSurface, nullptr, surface.get(), nullptr);
				unitImages[positionIndex]->setImage (surface.get());

				unitDetails[positionIndex]->setUnit (nullptr);

				activateButtons[positionIndex]->lock();
				reloadButtons[positionIndex]->lock();
				repairButtons[positionIndex]->lock();
				upgradeButtons[positionIndex]->lock();
			}
		}
	}
}

//------------------------------------------------------------------------------
void cWindowStorage::updateGlobalButtons()
{
	if (unit.storedUnits.empty())
		activateAllButton->lock();
	else
		activateAllButton->unlock();

	reloadAllButton->lock();
	repairAllButton->lock();
	upgradeAllButton->lock();
	if (canRepairReloadUpgrade && metalBar->getValue() >= 1)
	{
		for (const auto* vehicle : unit.storedUnits)
		{
			if (vehicle->data.getAmmo() != vehicle->data.getAmmoMax()) reloadAllButton->unlock();
			if (vehicle->data.getHitpoints() != vehicle->data.getHitpointsMax()) repairAllButton->unlock();

			if (!vehicle->getOwner()) continue;
			const auto& upgraded = *vehicle->getOwner()->getLastUnitData (vehicle->data.getId());
			if (vehicle->data.getVersion() != upgraded.getVersion()) upgradeAllButton->unlock();
		}
	}
}

//------------------------------------------------------------------------------
void cWindowStorage::updateUpDownButtons()
{
	if (page > 0)
		upButton->unlock();
	else
		upButton->lock();

	const auto pageSize = columns * maxRows;
	const auto maxPage = unit.storedUnits.size() / pageSize;

	if (page < maxPage)
		downButton->unlock();
	else
		downButton->lock();
}

//------------------------------------------------------------------------------
void cWindowStorage::upClicked()
{
	assert (page > 0);
	--page;
	updateUpDownButtons();
	updateUnitsWidgets();
}

//------------------------------------------------------------------------------
void cWindowStorage::downClicked()
{
	++page;
	updateUpDownButtons();
	updateUnitsWidgets();
}

//------------------------------------------------------------------------------
void cWindowStorage::activateClicked (size_t index)
{
	activate (page * columns * maxRows + index);
}

//------------------------------------------------------------------------------
void cWindowStorage::reloadClicked (size_t index)
{
	reload (page * columns * maxRows + index);
}

//------------------------------------------------------------------------------
void cWindowStorage::repairClicked (size_t index)
{
	repair (page * columns * maxRows + index);
}

//------------------------------------------------------------------------------
void cWindowStorage::upgradeClicked (size_t index)
{
	upgrade (page * columns * maxRows + index);
}

//------------------------------------------------------------------------------
void cWindowStorage::activateAllClicked()
{
	activateAll();
}

//------------------------------------------------------------------------------
void cWindowStorage::reloadAllClicked()
{
	reloadAll();
}

//------------------------------------------------------------------------------
void cWindowStorage::repairAllClicked()
{
	repairAll();
}

//------------------------------------------------------------------------------
void cWindowStorage::upgradeAllClicked()
{
	upgradeAll();
}

//------------------------------------------------------------------------------
void cWindowStorage::doneClicked()
{
	close();
}

//------------------------------------------------------------------------------
void cWindowStorage::closeOnUnitDestruction()
{
	close();
	auto application = getActiveApplication();
	if (application)
	{
		application->show (std::make_shared<cDialogOk> (lngPack.i18n ("Others~Unit_destroyed")));
	}
}
