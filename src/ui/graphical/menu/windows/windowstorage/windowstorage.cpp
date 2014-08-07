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

#include "ui/graphical/menu/windows/windowstorage/windowstorage.h"
#include "pcx.h"
#include "main.h"
#include "video.h"
#include "game/data/units/unit.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/data/base/base.h"
#include "game/data/player/player.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/image.h"
#include "ui/graphical/menu/widgets/special/resourcebar.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/application.h"
#include "ui/graphical/game/widgets/unitdetailsstored.h"
#include "ui/graphical/game/widgets/turntimeclockwidget.h"

//------------------------------------------------------------------------------
cWindowStorage::cWindowStorage (const cUnit& unit_, std::shared_ptr<const cTurnTimeClock> turnTimeClock) :
	cWindow (nullptr),
	unit (unit_),
	canRepairReloadUpgrade (unit_.isABuilding ()),
	canStorePlanes (unit_.data.storeUnitsImageType == sUnitData::STORE_UNIT_IMG_PLANE),
	canStoreShips (unit_.data.storeUnitsImageType == sUnitData::STORE_UNIT_IMG_SHIP),
	columns (canStorePlanes ? 2 : 3),
	page (0)
{
	AutoSurface background (LoadPCX (GFXOD_STORAGE));
	if (!canStorePlanes)
	{
		AutoSurface surface (LoadPCX (GFXOD_STORAGE_GROUND));
		SDL_BlitSurface (surface.get (), NULL, background.get (), NULL);
	}
	setSurface (std::move(background));

	auto turnTimeClockWidget = addChild (std::make_unique<cTurnTimeClockWidget> (cBox<cPosition> (cPosition (523, 17), cPosition (523 + 62, 17 + 10))));
	turnTimeClockWidget->setTurnTimeClock (std::move (turnTimeClock));

	//
	// Units
	//
	const int stepX = canStorePlanes ? 227 : 155;
	const int stepImageX = canStorePlanes ? 227 : 155;
	const int startX = canStorePlanes ? 42 : 8;
	const int nameLabelX = canStorePlanes ? 190 : 118;

	for (size_t x = 0; x < columns; x++)
	{
		for (size_t y = 0; y < maxRows; y++)
		{
			const auto index = x + y * columns;
			activateButtons[index] = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (startX + x * stepX, 191 + y * 236), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Active"), FONT_LATIN_NORMAL));
			signalConnectionManager.connect (activateButtons[index]->clicked, std::bind (&cWindowStorage::activateClicked, this, index));

			reloadButtons[index] = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (startX + x * stepX, 191 + 25 + y * 236), ePushButtonType::Angular, canRepairReloadUpgrade ? lngPack.i18n ("Text~Others~Reload") : "", FONT_LATIN_NORMAL));
			signalConnectionManager.connect (reloadButtons[index]->clicked, std::bind (&cWindowStorage::reloadClicked, this, index));

			repairButtons[index] = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (75 + startX + x * stepX, 191 + y * 236), ePushButtonType::Angular, canRepairReloadUpgrade ? lngPack.i18n ("Text~Others~Repair") : "", FONT_LATIN_NORMAL));
			signalConnectionManager.connect (repairButtons[index]->clicked, std::bind (&cWindowStorage::repairClicked, this, index));

			upgradeButtons[index] = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (75 + startX + x * stepX, 191 + 25 + y * 236), ePushButtonType::Angular, canRepairReloadUpgrade ? lngPack.i18n ("Text~Others~Upgrade") : "", FONT_LATIN_NORMAL));
			signalConnectionManager.connect (upgradeButtons[index]->clicked, std::bind (&cWindowStorage::upgradeClicked, this, index));

			unitImages[index] = addChild (std::make_unique<cImage> (getPosition () + cPosition (17 + x * stepImageX, 9 + y * 236)));
			unitNames[index] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (17 + x * stepImageX + 5, 9 + y * 236 + 5), getPosition () + cPosition (17 + x * stepImageX + 5 + nameLabelX, 9 + y * 236 + 5 + 118)), ""));
			unitNames[index]->setWordWrap (true);

			unitDetails[index] = addChild (std::make_unique<cUnitDetailsStored> (cBox<cPosition> (getPosition () + cPosition (17 + x * stepImageX, 145 + y * 236), getPosition () + cPosition (17 + x * stepImageX + 130, 145 + y * 236 + 40))));
		}
	}

	//
	// Metal Bar
	//
	auto metalValue = 0;
	if (unit.isABuilding ())
	{
		const auto& building = static_cast<const cBuilding&>(unit);

		metalValue = building.SubBase->getMetal ();

		signalConnectionManager.connect (building.SubBase->metalChanged, [&]()
		{
			metalBar->setValue (building.SubBase->getMetal ());
		});
	}

	metalBarAmountLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (536, 85), getPosition () + cPosition (536 + 40, 85 + 10)), iToStr (metalValue), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
	metalBar = addChild (std::make_unique<cResourceBar> (cBox<cPosition> (getPosition () + cPosition (546, 106), getPosition () + cPosition (546 + 20, 106 + 115)), 0, metalValue, eResourceBarType::Metal, eOrientationType::Vertical));
	signalConnectionManager.connect (metalBar->valueChanged, [&](){ metalBarAmountLabel->setText (iToStr (metalBar->getValue ())); });
	metalBar->disable ();

	//
	// Buttons
	//
	upButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (504, 426), ePushButtonType::ArrowUpBig));
	signalConnectionManager.connect (upButton->clicked, std::bind (&cWindowStorage::upClicked, this));

	downButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (532, 426), ePushButtonType::ArrowDownBig));
	signalConnectionManager.connect (downButton->clicked, std::bind (&cWindowStorage::downClicked, this));

	activateAllButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (518, 246), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Active"), FONT_LATIN_NORMAL));
	signalConnectionManager.connect (activateAllButton->clicked, std::bind (&cWindowStorage::activateAllClicked, this));

	reloadAllButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (518, 246 + 25), ePushButtonType::Angular, canRepairReloadUpgrade ? lngPack.i18n ("Text~Others~Reload") : "", FONT_LATIN_NORMAL));
	signalConnectionManager.connect (reloadAllButton->clicked, std::bind (&cWindowStorage::reloadAllClicked, this));

	repairAllButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (518, 246 + 25 * 2), ePushButtonType::Angular, canRepairReloadUpgrade ? lngPack.i18n ("Text~Others~Repair") : "", FONT_LATIN_NORMAL));
	signalConnectionManager.connect (repairAllButton->clicked, std::bind (&cWindowStorage::repairAllClicked, this));

	upgradeAllButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (518, 246 + 25 * 3), ePushButtonType::Angular, canRepairReloadUpgrade ? lngPack.i18n ("Text~Others~Upgrade") : "", FONT_LATIN_NORMAL));
	signalConnectionManager.connect (upgradeAllButton->clicked, std::bind (&cWindowStorage::upgradeAllClicked, this));

	auto doneButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (518, 371), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Done"), FONT_LATIN_NORMAL));
	signalConnectionManager.connect (doneButton->clicked, std::bind (&cWindowStorage::doneClicked, this));

	updateUnitsWidgets ();
	updateGlobalButtons ();
	updateUpDownButtons ();

	signalConnectionManager.connect (unit.destroyed, std::bind (&cWindowStorage::closeOnUnitDestruction, this));
}

//------------------------------------------------------------------------------
void cWindowStorage::updateUnitButtons (const cVehicle& storedUnit, size_t positionIndex)
{
	const auto& upgraded = *storedUnit.getOwner ()->getUnitDataCurrentVersion (unit.data.ID);

	activateButtons[positionIndex]->unlock ();
	if (storedUnit.data.getAmmo () != storedUnit.data.ammoMax && metalBar->getValue () >= 1) reloadButtons[positionIndex]->unlock ();
	else reloadButtons[positionIndex]->lock ();
	if (storedUnit.data.getHitpoints () != storedUnit.data.hitpointsMax && metalBar->getValue () >= 1) repairButtons[positionIndex]->unlock ();
	else repairButtons[positionIndex]->lock ();
	if (storedUnit.data.getVersion () != upgraded.getVersion () && metalBar->getValue () >= 1) upgradeButtons[positionIndex]->unlock ();
	else upgradeButtons[positionIndex]->lock ();
}

//------------------------------------------------------------------------------
void cWindowStorage::updateUnitsWidgets ()
{
	unitsSignalConnectionManager.disconnectAll ();

	for (size_t x = 0; x < columns; x++)
	{
		for (size_t y = 0; y < maxRows; y++)
		{
			const auto positionIndex = x + y * columns;
			const auto unitIndex = page * columns * maxRows + positionIndex;

			if (unitIndex < unit.storedUnits.size ())
			{
				const auto& storedUnit = *unit.storedUnits[unitIndex];

				std::string name = storedUnit.getDisplayName ();

				const auto& upgraded = *storedUnit.getOwner ()->getUnitDataCurrentVersion (storedUnit.data.ID);
				if (storedUnit.data.getVersion () != upgraded.getVersion ())
				{
					name += "\n(" + lngPack.i18n ("Text~Comp~Dated") + ")";
				}
				unitNames[positionIndex]->setText (name);

				AutoSurface surface (SDL_CreateRGBSurface (0, storedUnit.uiData->storage->w, storedUnit.uiData->storage->h, Video.getColDepth (), 0, 0, 0, 0));
				SDL_BlitSurface (storedUnit.uiData->storage.get(), NULL, surface.get (), NULL);
				unitImages[positionIndex]->setImage (surface.get ());

				unitDetails[positionIndex]->setUnit (&storedUnit);

				updateUnitButtons (storedUnit, positionIndex);

				unitsSignalConnectionManager.connect (storedUnit.data.hitpointsChanged, std::bind (&cWindowStorage::updateUnitButtons, this, std::ref (storedUnit), positionIndex));
				unitsSignalConnectionManager.connect (storedUnit.data.ammoChanged, std::bind (&cWindowStorage::updateUnitButtons, this, std::ref (storedUnit), positionIndex));
				unitsSignalConnectionManager.connect (storedUnit.data.versionChanged, std::bind (&cWindowStorage::updateUnitButtons, this, std::ref (storedUnit), positionIndex));

				unitsSignalConnectionManager.connect (storedUnit.data.hitpointsChanged, std::bind (&cWindowStorage::updateGlobalButtons, this));
				unitsSignalConnectionManager.connect (storedUnit.data.ammoChanged, std::bind (&cWindowStorage::updateGlobalButtons, this));
				unitsSignalConnectionManager.connect (storedUnit.data.versionChanged, std::bind (&cWindowStorage::updateGlobalButtons, this));
			}
			else
			{
				unitNames[positionIndex]->setText ("");

				SDL_Surface* srcSurface;
				if (canStoreShips) srcSurface = GraphicsData.gfx_edock.get ();
				else if (canStorePlanes) srcSurface = GraphicsData.gfx_ehangar.get ();
				else srcSurface = GraphicsData.gfx_edepot.get ();

				AutoSurface surface (SDL_CreateRGBSurface (0, srcSurface->w, srcSurface->h, Video.getColDepth (), 0, 0, 0, 0));
				SDL_BlitSurface (srcSurface, NULL, surface.get (), NULL);
				unitImages[positionIndex]->setImage (surface.get ());

				unitDetails[positionIndex]->setUnit (nullptr);

				activateButtons[positionIndex]->lock ();
				reloadButtons[positionIndex]->lock ();
				repairButtons[positionIndex]->lock ();
				upgradeButtons[positionIndex]->lock ();
			}
		}
	}
}

//------------------------------------------------------------------------------
void cWindowStorage::updateGlobalButtons ()
{
	if (unit.storedUnits.empty ()) activateAllButton->lock ();
	else activateAllButton->unlock ();

	reloadAllButton->lock ();
	repairAllButton->lock ();
	upgradeAllButton->lock ();
	if (canRepairReloadUpgrade && metalBar->getValue () >= 1)
	{
		for (size_t i = 0; i != unit.storedUnits.size (); ++i)
		{
			const auto& vehicle = *unit.storedUnits[i];
			const auto& upgraded = *vehicle.getOwner ()->getUnitDataCurrentVersion (vehicle.data.ID);

			if (vehicle.data.getAmmo () != vehicle.data.ammoMax) reloadAllButton->unlock ();
			if (vehicle.data.getHitpoints () != vehicle.data.hitpointsMax) repairAllButton->unlock ();
			if (vehicle.data.getVersion () != upgraded.getVersion ()) upgradeAllButton->unlock ();
		}
	}
}

//------------------------------------------------------------------------------
void cWindowStorage::updateUpDownButtons ()
{
	if (page > 0) upButton->unlock ();
	else upButton->lock ();

	const auto pageSize = columns * maxRows;
	const auto maxPage = unit.storedUnits.size () / pageSize;

	if (page < maxPage) downButton->unlock ();
	else downButton->lock ();
}

//------------------------------------------------------------------------------
void cWindowStorage::upClicked ()
{
	assert (page > 0);
	--page;
	updateUpDownButtons ();
	updateUnitsWidgets ();
}

//------------------------------------------------------------------------------
void cWindowStorage::downClicked ()
{
	++page;
	updateUpDownButtons ();
	updateUnitsWidgets ();
}

//------------------------------------------------------------------------------
void cWindowStorage::activateClicked (size_t index)
{
	activate(page * columns * maxRows + index);
}

//------------------------------------------------------------------------------
void cWindowStorage::reloadClicked (size_t index)
{
	reload(page * columns * maxRows + index);
}

//------------------------------------------------------------------------------
void cWindowStorage::repairClicked (size_t index)
{
	repair(page * columns * maxRows + index);
}

//------------------------------------------------------------------------------
void cWindowStorage::upgradeClicked (size_t index)
{
	upgrade(page * columns * maxRows + index);
}

//------------------------------------------------------------------------------
void cWindowStorage::activateAllClicked ()
{
	activateAll ();
}

//------------------------------------------------------------------------------
void cWindowStorage::reloadAllClicked ()
{
	reloadAll ();
}

//------------------------------------------------------------------------------
void cWindowStorage::repairAllClicked ()
{
	repairAll ();
}

//------------------------------------------------------------------------------
void cWindowStorage::upgradeAllClicked ()
{
	upgradeAll ();
}

//------------------------------------------------------------------------------
void cWindowStorage::doneClicked ()
{
	close ();
}

//------------------------------------------------------------------------------
void cWindowStorage::closeOnUnitDestruction ()
{
	close ();
	auto application = getActiveApplication ();
	if (application)
	{
		application->show (std::make_shared<cDialogOk> ("Unit destroyed!")); // TODO: translate
	}
}
