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

#include "ui/graphical/menu/widgets/special/saveslotwidget.h"

#include "game/data/savegameinfo.h"
#include "input/mouse/mouse.h"
#include "output/sound/soundchannel.h"
#include "output/sound/sounddevice.h"
#include "resources/sound.h"
#include "ui/graphical/application.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/lineedit.h"

//------------------------------------------------------------------------------
cSaveSlotWidget::cSaveSlotWidget (const cPosition& position) :
	cClickableWidget (position),
	empty (true),
	renameable (false)
{
	numberLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (13, 28), getPosition() + cPosition (13 + 20, 28 + 15)), "", FONT_LATIN_BIG));
	numberLabel->setConsumeClick (false);
	timeLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (42, 19), getPosition() + cPosition (42 + 98, 19 + 10)), ""));
	timeLabel->setConsumeClick (false);
	typeLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (145, 19), getPosition() + cPosition (145 + 46, 19 + 10)), "", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
	typeLabel->setConsumeClick (false);
	nameLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition() + cPosition (42, 42), getPosition() + cPosition (42 + 148, 42 + 10))));
	nameLineEdit->setReadOnly (true);

	resize (cPosition (203, 71));
}

//------------------------------------------------------------------------------
const std::string& cSaveSlotWidget::getName() const
{
	return nameLineEdit->getText();
}

//------------------------------------------------------------------------------
void cSaveSlotWidget::setSelected (bool selected)
{
	if (selected) numberLabel->setFont (FONT_LATIN_BIG_GOLD);
	else numberLabel->setFont (FONT_LATIN_BIG);
}

//------------------------------------------------------------------------------
void cSaveSlotWidget::setRenameable (bool renameable_)
{
	renameable = renameable_;
	nameLineEdit->setReadOnly (!renameable);
}

//------------------------------------------------------------------------------
void cSaveSlotWidget::setSaveData (const cSaveGameInfo& saveFile)
{
	numberLabel->setText (std::to_string (saveFile.number));
	timeLabel->setText (saveFile.date);
	switch (saveFile.type)
	{
		case GAME_TYPE_HOTSEAT:
			typeLabel->setText ("HOT");
			break;
		case GAME_TYPE_SINGLE:
			typeLabel->setText ("IND");
			break;
		case GAME_TYPE_TCPIP:
			typeLabel->setText ("NET");
			break;
	}
	nameLineEdit->setText (saveFile.gameName);
	nameLineEdit->setReadOnly (!renameable);

	empty = false;
}

//------------------------------------------------------------------------------
void cSaveSlotWidget::reset (int number)
{
	numberLabel->setText (std::to_string (number));
	timeLabel->setText ("");
	typeLabel->setText ("");
	nameLineEdit->setText ("");
	nameLineEdit->setReadOnly (!renameable);

	empty = true;
}

//------------------------------------------------------------------------------
bool cSaveSlotWidget::isEmpty() const
{
	return empty;
}

//------------------------------------------------------------------------------
void cSaveSlotWidget::forceKeyFocus()
{
	auto application = getActiveApplication();

	if (!application) return;

	application->grapKeyFocus (*nameLineEdit);
}

//------------------------------------------------------------------------------
bool cSaveSlotWidget::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left)
	{
		clicked();

		cSoundDevice::getInstance().playSoundEffect (SoundData.SNDObjectMenu);

		if (mouse.getButtonClickCount (button) == 2)
		{
			doubleClicked();
		}
		return true;
	}
	return false;
}
