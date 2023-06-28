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

#include "ui/graphical/menu/windows/windowloadsave/windowloadsave.h"

#include "game/data/savegame.h"
#include "game/data/savegameinfo.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/saveslotwidget.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cWindowLoadSave::cWindowLoadSave (std::shared_ptr<const cTurnTimeClock> turnTimeClock) :
	cWindowLoad (std::move (turnTimeClock))
{
	exitButton = emplaceChild<cPushButton> (getPosition() + cPosition (246, 438), ePushButtonType::Huge, &SoundData.SNDMenuButton, lngPack.i18n ("Others~Exit"));
	signalConnectionManager.connect (exitButton->clicked, [this]() { exit(); });

	saveButton = emplaceChild<cPushButton> (getPosition() + cPosition (132, 438), ePushButtonType::Huge, lngPack.i18n ("Others~Save"));
	signalConnectionManager.connect (saveButton->clicked, [this]() { handleSaveClicked(); });
	saveButton->lock();
}

//------------------------------------------------------------------------------
void cWindowLoadSave::retranslate()
{
	cWindowLoad::retranslate();

	exitButton->setText (lngPack.i18n ("Others~Exit"));
	saveButton->setText (lngPack.i18n ("Others~Save"));
}

//------------------------------------------------------------------------------
void cWindowLoadSave::handleSlotClicked (size_t index)
{
	selectSlot (index, true);

	auto& slot = getSaveSlot (index);

	slot.forceKeyFocus();

	saveButton->unlock();
}

//------------------------------------------------------------------------------
void cWindowLoadSave::handleSlotDoubleClicked (size_t index)
{}

//------------------------------------------------------------------------------
void cWindowLoadSave::handleSaveClicked()
{
	auto saveNumber = getSelectedSaveNumber();
	if (saveNumber == -1) return;

	auto slot = getSaveSlotFromSaveNumber (saveNumber);
	if (slot)
	{
		save (saveNumber, slot->getName());
	}
	else
	{
		auto saveFile = getSaveFile (saveNumber);
		if (!saveFile) return;

		save (saveNumber, saveFile->gameName);
	}
}
