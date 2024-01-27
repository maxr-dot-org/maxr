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

#include "windowload.h"

#include "game/data/savegame.h"
#include "game/data/savegameinfo.h"
#include "resources/pcx.h"
#include "ui/graphical/game/widgets/turntimeclockwidget.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/saveslotwidget.h"
#include "ui/uidefines.h"
#include "ui/widgets/application.h"
#include "ui/widgets/label.h"
#include "utility/language.h"
#include "utility/log.h"
#include "utility/ranges.h"

//Versions prior to 1.0 are no longer compatible
#define MINIMUM_REQUIRED_SAVE_VERSION ((std::string) "1.0")
#define MINIMUM_REQUIRED_MAXR_VERSION ((std::string) "0.2.10")

//------------------------------------------------------------------------------
cWindowLoad::cWindowLoad (std::shared_ptr<const cTurnTimeClock> turnTimeClock, std::function<std::vector<cSaveGameInfo>()> saveGamesGetter) :
	cWindow (LoadPCX (GFXOD_SAVELOAD)),
	lastPage ((int) std::ceil ((double) maximalDisplayedSaves / (rows * columns)) - 1),
	saveGamesGetter (saveGamesGetter)
{
	titleLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 12), getPosition() + cPosition (getArea().getMaxCorner().x(), 12 + 10)), lngPack.i18n ("Title~Load"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	auto turnTimeClockWidget = emplaceChild<cTurnTimeClockWidget> (cBox<cPosition> (cPosition (525, 16), cPosition (525 + 60, 16 + 10)));
	turnTimeClockWidget->setTurnTimeClock (std::move (turnTimeClock));

	backButton = emplaceChild<cPushButton> (getPosition() + cPosition (353, 438), ePushButtonType::Huge, lngPack.i18n ("Others~Back"));
	signalConnectionManager.connect (backButton->clicked, [this]() { close(); });

	loadButton = emplaceChild<cPushButton> (getPosition() + cPosition (514, 438), ePushButtonType::Huge, lngPack.i18n ("Others~Load"));
	signalConnectionManager.connect (loadButton->clicked, [this]() { handleLoadClicked(); });
	loadButton->lock();

	auto upButton = emplaceChild<cPushButton> (getPosition() + cPosition (33, 438), ePushButtonType::ArrowUpBig);
	auto downButton = emplaceChild<cPushButton> (getPosition() + cPosition (63, 438), ePushButtonType::ArrowDownBig);
	signalConnectionManager.connect (upButton->clicked, [=]() { handleUpClicked(); if (page == 0) { upButton->lock(); } downButton->unlock(); });
	signalConnectionManager.connect (downButton->clicked, [=]() { handleDownClicked(); if (page == lastPage) {downButton->lock();} upButton->unlock(); });
	upButton->lock();

	for (size_t x = 0; x < columns; x++)
	{
		for (size_t y = 0; y < rows; y++)
		{
			const auto index = rows * x + y;
			saveSlots[index] = emplaceChild<cSaveSlotWidget> (getPosition() + cPosition (17 + 402 * x, 45 + 76 * y));
			signalConnectionManager.connect (saveSlots[index]->clicked, [this, index]() { handleSlotClicked (index); });
			signalConnectionManager.connect (saveSlots[index]->doubleClicked, [this, index]() { handleSlotDoubleClicked (index); });
		}
	}

	update();
}

//------------------------------------------------------------------------------
cWindowLoad::~cWindowLoad()
{
}

//------------------------------------------------------------------------------
void cWindowLoad::retranslate()
{
	cWindow::retranslate();

	titleLabel->setText (lngPack.i18n ("Title~Load"));
	backButton->setText (lngPack.i18n ("Others~Back"));
	loadButton->setText (lngPack.i18n ("Others~Load"));
}

//------------------------------------------------------------------------------
void cWindowLoad::update()
{
	saveGames.clear();

	if (saveGamesGetter)
	{
		saveGames = saveGamesGetter();
	}
	else
	{
		loadSaves();
	}
	updateSlots();
}

//------------------------------------------------------------------------------
void cWindowLoad::loadSaves()
{
	if (saveGamesGetter) return;
	fillSaveGames (page * columns * rows, (page + 1) * columns * rows, saveGames);
}

//------------------------------------------------------------------------------
void cWindowLoad::updateSlots()
{
	for (size_t x = 0; x < columns; x++)
	{
		for (size_t y = 0; y < rows; y++)
		{
			const auto slotNumber = x * rows + y;
			const auto saveNumber = page * (rows * columns) + slotNumber + 1;

			auto slot = saveSlots[slotNumber];

			auto saveFile = getSaveFile (saveNumber);

			if (saveFile)
			{
				slot->setSaveData (*saveFile);
			}
			else
			{
				slot->reset (saveNumber);
			}
			slot->setSelected (selectedSaveNumber == saveNumber);
		}
	}
}

//------------------------------------------------------------------------------
void cWindowLoad::handleSlotClicked (size_t index)
{
	if (saveSlots[index]->isEmpty()) return;

	selectSlot (index, false);
}

//------------------------------------------------------------------------------
void cWindowLoad::handleSlotDoubleClicked (size_t index)
{
	if (saveSlots[index]->isEmpty()) return;

	const auto saveNumber = page * (columns * rows) + index + 1;

	auto saveInfo = getSaveFile (saveNumber);
	if (cVersion (saveInfo->saveVersion) == cVersion ("0.0"))
	{
		getActiveApplication()->show (std::make_shared<cDialogOk> (lngPack.i18n ("Error_Messages~ERROR_Save_Loading")));
		return;
	}
	if (cVersion (saveInfo->saveVersion) < cVersion (MINIMUM_REQUIRED_SAVE_VERSION))
	{
		getActiveApplication()->show (std::make_shared<cDialogOk> (lngPack.i18n ("Error_Messages~ERROR_Save_Incompatible", MINIMUM_REQUIRED_MAXR_VERSION)));
		NetLog.warn ("Savegame Version " + saveInfo->gameVersion + " of file " + cSavegame::getFileName (saveNumber).u8string() + " is not compatible");
		return;
	}

	load (*saveInfo);
}

//------------------------------------------------------------------------------
void cWindowLoad::selectSlot (size_t slotIndex, bool makeRenameable)
{
	if (auto oldSelected = getSaveFile (selectedSaveNumber))
	{
		if (auto slot = getSaveSlotFromSaveNumber (selectedSaveNumber)) slot->setSaveData (*oldSelected);
	}

	selectedSaveNumber = page * (columns * rows) + slotIndex + 1;

	auto newSelected = getSaveFile (selectedSaveNumber);
	selectedOriginalName = newSelected ? newSelected->gameName : "";

	bool isEmptySlot = true;
	for (size_t x = 0; x < columns; x++)
	{
		for (size_t y = 0; y < rows; y++)
		{
			const auto index = x * rows + y;
			if (index == slotIndex)
			{
				saveSlots[index]->setSelected (true);
				saveSlots[index]->setRenameable (makeRenameable);
				isEmptySlot = saveSlots[index]->isEmpty();
			}
			else
			{
				saveSlots[index]->setSelected (false);
				saveSlots[index]->setRenameable (false);
			}
		}
	}

	if (isEmptySlot)
		loadButton->lock();
	else
		loadButton->unlock();
}

//------------------------------------------------------------------------------
std::optional<std::size_t> cWindowLoad::getSelectedSaveNumber() const
{
	return selectedSaveNumber;
}

//------------------------------------------------------------------------------
cSaveGameInfo* cWindowLoad::getSaveFile (std::optional<std::size_t> saveNumber)
{
	auto iter = ranges::find_if (saveGames, [=] (const cSaveGameInfo& save) { return save.number == saveNumber; });
	return iter == saveGames.end() ? nullptr : &(*iter);
}

//------------------------------------------------------------------------------
cSaveSlotWidget& cWindowLoad::getSaveSlot (size_t slotIndex)
{
	return *saveSlots[slotIndex];
}

//------------------------------------------------------------------------------
cSaveSlotWidget* cWindowLoad::getSaveSlotFromSaveNumber (std::optional<std::size_t> saveNumber)
{
	if (!saveNumber.has_value()) { return nullptr; }
	if (*saveNumber - 1 >= page * (rows * columns) && *saveNumber - 1 < (page + 1) * (rows * columns))
	{
		return &getSaveSlot (*saveNumber - 1 - page * (rows * columns));
	}
	else
		return nullptr;
}

//------------------------------------------------------------------------------
void cWindowLoad::handleDownClicked()
{
	if (page < lastPage)
	{
		++page;
		loadSaves();
		updateSlots();
	}
}

//------------------------------------------------------------------------------
void cWindowLoad::handleUpClicked()
{
	if (page > 0)
	{
		--page;
		loadSaves();
		updateSlots();
	}
}

//------------------------------------------------------------------------------
void cWindowLoad::handleLoadClicked()
{
	if (!selectedSaveNumber.has_value()) return;

	auto saveInfo = getSaveFile (selectedSaveNumber);
	if (cVersion (saveInfo->saveVersion) == cVersion ("0.0"))
	{
		getActiveApplication()->show (std::make_shared<cDialogOk> (lngPack.i18n ("Error_Messages~ERROR_Save_Loading")));
		return;
	}
	if (cVersion (saveInfo->saveVersion) < cVersion (MINIMUM_REQUIRED_SAVE_VERSION))
	{
		getActiveApplication()->show (std::make_shared<cDialogOk> (lngPack.i18n ("Error_Messages~ERROR_Save_Incompatible", MINIMUM_REQUIRED_MAXR_VERSION)));
		NetLog.warn ("Savegame Version " + saveInfo->gameVersion + " of file " + cSavegame::getFileName (*selectedSaveNumber).u8string() + " is not compatible");
		return;
	}

	load (*saveInfo);
}
