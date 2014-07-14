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

#include "ui/graphical/menu/windows/windowload/windowload.h"
#include "ui/graphical/menu/windows/windowload/savegamedata.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/saveslotwidget.h"
#include "ui/graphical/game/widgets/turntimeclockwidget.h"
#include "pcx.h"
#include "main.h"
#include "files.h"
#include "savegame.h"

//------------------------------------------------------------------------------
cWindowLoad::cWindowLoad (std::shared_ptr<const cTurnTimeClock> turnTimeClock) :
	cWindow (LoadPCX (GFXOD_SAVELOAD)),
	page (0),
	lastPage ((int)std::ceil ((double)maximalDisplayedSaves / (rows * columns)) - 1),
	selectedSaveNumber (-1)
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (0, 12), getPosition () + cPosition (getArea ().getMaxCorner ().x (), 12 + 10)), lngPack.i18n ("Text~Title~Load"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	auto turnTimeClockWidget = addChild (std::make_unique<cTurnTimeClockWidget> (cBox<cPosition> (cPosition (525, 16), cPosition (525 + 60, 16 + 10))));
	turnTimeClockWidget->setTurnTimeClock (std::move (turnTimeClock));

	auto backButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (353, 438), ePushButtonType::Huge, lngPack.i18n ("Text~Others~Back")));
	signalConnectionManager.connect (backButton->clicked, [&](){ close (); });

	loadButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (514, 438), ePushButtonType::Huge, lngPack.i18n ("Text~Others~Load")));
	signalConnectionManager.connect (loadButton->clicked, std::bind (&cWindowLoad::handleLoadClicked, this));
	loadButton->lock ();

	auto upButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (33, 438), ePushButtonType::ArrowUpBig));
	signalConnectionManager.connect (upButton->clicked, std::bind (&cWindowLoad::handleUpClicked, this));
	auto downButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (63, 438), ePushButtonType::ArrowDownBig));
	signalConnectionManager.connect (downButton->clicked, std::bind (&cWindowLoad::handleDownClicked, this));

	for (size_t x = 0; x < columns; x++)
	{
		for (size_t y = 0; y < rows; y++)
		{
			const auto index = rows * x + y;
			saveSlots[index] = addChild (std::make_unique<cSaveSlotWidget> (getPosition () + cPosition (17 + 402 * x, 45 + 76 * y)));
			signalConnectionManager.connect (saveSlots[index]->clicked, std::bind (&cWindowLoad::handleSlotClicked, this, index));
			signalConnectionManager.connect (saveSlots[index]->doubleClicked, std::bind (&cWindowLoad::handleSlotDoubleClicked, this, index));
		}
	}

	loadSaves ();
	updateSlots ();
}

//------------------------------------------------------------------------------
cWindowLoad::~cWindowLoad()
{
}

//------------------------------------------------------------------------------
void cWindowLoad::update ()
{
	saveGames.clear ();

	loadSaves ();
	updateSlots ();
}

//------------------------------------------------------------------------------
void cWindowLoad::loadSaves ()
{
	auto saveFileNames = getFilesOfDirectory (cSettings::getInstance ().getSavesPath ());

	for (size_t i = 0; i != saveFileNames.size (); ++i)
	{
		// only check for xml files and numbers for this offset
		const auto& file = saveFileNames[i];
		if (file.length () < 4 || file.compare (file.length () - 3, 3, "xml") != 0)
		{
			saveFileNames.erase (saveFileNames.begin () + i);
			i--;
			continue;
		}
		int number;
		if (file.length () < 8 || (number = atoi (file.substr (file.length () - 7, 3).c_str ())) < page || number > page + (int)(rows * columns)) continue;
		// don't add files twice
		bool found = false;
		for (unsigned int j = 0; j < saveGames.size (); j++)
		{
			if (saveGames[j].getNumber() == number)
			{
				found = true;
				break;
			}
		}
		if (found) continue;
		// read the information and add it to the saves list
		std::string gameName, type, time;
		cSavegame Savegame (number);
		Savegame.loadHeader (&gameName, &type, &time);
		saveGames.push_back (cSaveGameData (file, gameName, type, time, number));
	}
}

//------------------------------------------------------------------------------
void cWindowLoad::updateSlots ()
{
	for (size_t x = 0; x < columns; x++)
	{
		for (size_t y = 0; y < rows; y++)
		{
			const auto slotNumber = x * rows + y;
			const auto saveNumber = page * (rows*columns) + slotNumber + 1;

			auto slot = saveSlots[slotNumber];

			auto saveFile = getSaveFile(saveNumber);

			if (saveFile)
			{
				slot->setSaveData (*saveFile);
			}
			else
			{
				slot->reset (saveNumber);
			}
			if (selectedSaveNumber == static_cast<int>(saveNumber)) slot->setSelected (true);
			else slot->setSelected (false);
		}
	}
}

//------------------------------------------------------------------------------
void cWindowLoad::handleSlotClicked (size_t index)
{
	if (saveSlots[index]->isEmpty ()) return;

	selectSlot (index, false);
}

//------------------------------------------------------------------------------
void cWindowLoad::handleSlotDoubleClicked (size_t index)
{
	if (saveSlots[index]->isEmpty ()) return;

	const auto saveNumber = page * (columns * rows) + index + 1;

	load (saveNumber);
}

//------------------------------------------------------------------------------
void cWindowLoad::selectSlot (size_t slotIndex, bool makeRenameable)
{
	auto oldSelected = getSaveFile (selectedSaveNumber);
	if (oldSelected)
	{
		auto slot = getSaveSlotFromSaveNumber (selectedSaveNumber);
		if (slot) slot->setSaveData (*oldSelected);
	}

	selectedSaveNumber = page * (columns * rows) + slotIndex + 1;

	auto newSelected = getSaveFile (selectedSaveNumber);
	selectedOriginalName = newSelected ? newSelected->getGameName() : "";

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
				isEmptySlot = saveSlots[index]->isEmpty ();
			}
			else
			{
				saveSlots[index]->setSelected (false);
				saveSlots[index]->setRenameable (false);
			}
		}
	}

	if (isEmptySlot) loadButton->lock ();
	else loadButton->unlock ();
}

//------------------------------------------------------------------------------
int cWindowLoad::getSelectedSaveNumber () const
{
	return selectedSaveNumber;
}

//------------------------------------------------------------------------------
cSaveGameData* cWindowLoad::getSaveFile (int saveNumber)
{
	auto iter = std::find_if (saveGames.begin (), saveGames.end (), [=](const cSaveGameData& save) { return save.getNumber() == saveNumber; });
	return iter == saveGames.end () ? nullptr : &(*iter);
}

//------------------------------------------------------------------------------
cSaveSlotWidget& cWindowLoad::getSaveSlot (size_t slotIndex)
{
	return *saveSlots[slotIndex];
}

//------------------------------------------------------------------------------
cSaveSlotWidget* cWindowLoad::getSaveSlotFromSaveNumber (size_t saveNumber)
{
	if (selectedSaveNumber - 1 >= page * (int)(rows * columns) && selectedSaveNumber - 1 < (page+1) * (int)(rows * columns))
	{
		return &getSaveSlot (selectedSaveNumber - 1 - page * (int)(rows * columns));
	}
	else return nullptr;
}

//------------------------------------------------------------------------------
void cWindowLoad::handleDownClicked ()
{
	if (page < lastPage)
	{
		++page;
		loadSaves ();
		updateSlots ();
	}
}

//------------------------------------------------------------------------------
void cWindowLoad::handleUpClicked ()
{
	if (page > 0)
	{
		--page;
		loadSaves ();
		updateSlots ();
	}
}

//------------------------------------------------------------------------------
void cWindowLoad::handleLoadClicked ()
{
	if (selectedSaveNumber == -1) return;

	load (selectedSaveNumber);
}
