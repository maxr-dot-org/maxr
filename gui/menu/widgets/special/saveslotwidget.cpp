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

#include "saveslotwidget.h"
#include "../label.h"
#include "../lineedit.h"
#include "../../windows/windowload/savegamedata.h"
#include "../../../application.h"
#include "../../../../input/mouse/mouse.h"
#include "../../../../sound.h"
#include "../../../../main.h" // iToStr
#include "../../../../output/sound/sounddevice.h"
#include "../../../../output/sound/soundchannel.h"

//------------------------------------------------------------------------------
cSaveSlotWidget::cSaveSlotWidget (const cPosition& position) :
	cClickableWidget (position),
	empty (true),
	renameable (false)
{
	numberLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (13, 28), getPosition () + cPosition (13 + 20, 28 + 15)), "", FONT_LATIN_BIG));
	timeLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (42, 19), getPosition () + cPosition (42 + 98, 19 + 10)), ""));
	typeLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (145, 19), getPosition () + cPosition (145 + 46, 19 + 10)), "", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
	nameLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition () + cPosition (42, 42), getPosition () + cPosition (42 + 148, 42 + 10))));
	nameLineEdit->setReadOnly (true);

	resize (cPosition (203, 71));
}

//------------------------------------------------------------------------------
const std::string& cSaveSlotWidget::getName () const
{
	return nameLineEdit->getText ();
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
void cSaveSlotWidget::setSaveData (const cSaveGameData& saveFile)
{
	numberLabel->setText (iToStr (saveFile.getNumber()));
	timeLabel->setText (saveFile.getDate());
	typeLabel->setText (saveFile.getType());
	nameLineEdit->setText (saveFile.getGameName());
	nameLineEdit->setReadOnly (!renameable);

	empty = false;
}

//------------------------------------------------------------------------------
void cSaveSlotWidget::reset (int number)
{
	numberLabel->setText (iToStr (number));
	timeLabel->setText ("");
	typeLabel->setText ("");
	nameLineEdit->setText ("");
	nameLineEdit->setReadOnly (!renameable);

	empty = true;
}

//------------------------------------------------------------------------------
bool cSaveSlotWidget::isEmpty () const
{
	return empty;
}

//------------------------------------------------------------------------------
void cSaveSlotWidget::forceKeyFocus ()
{
	auto application = getActiveApplication ();

	if (!application) return;

	application->grapKeyFocus (*nameLineEdit);
}

//------------------------------------------------------------------------------
bool cSaveSlotWidget::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left)
	{
		clicked ();

        cSoundDevice::getInstance ().getFreeSoundEffectChannel ().play (SoundData.SNDObjectMenu);

		if (mouse.getButtonClickCount (button) == 2)
		{
			doubleClicked ();
		}
		return true;
	}
	return false;
}
