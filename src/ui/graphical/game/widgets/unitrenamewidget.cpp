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

#include "ui/graphical/game/widgets/unitrenamewidget.h"

#include "ui/graphical/application.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/lineedit.h"

#include "game/data/units/unit.h"

//------------------------------------------------------------------------------
cUnitRenameWidget::cUnitRenameWidget (const cPosition& position, int width) :
	cWidget (position),
	activeUnit (nullptr),
	player (nullptr)
{
	selectedUnitStatusLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (0, 10), getPosition () + cPosition (width, 10)), "", FONT_LATIN_SMALL_WHITE, eAlignmentType::Left));
	selectedUnitNamePrefixLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition (), getPosition () + cPosition (width, 10)), "", FONT_LATIN_SMALL_GREEN, eAlignmentType::Left));
	selectedUnitNameEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition (), getPosition () + cPosition (width, 10)), eLineEditFrameType::None, FONT_LATIN_SMALL_GREEN));

	signalConnectionManager.connect (selectedUnitNameEdit->returnPressed, [&]()
	{
		if (activeUnit)
		{
			unitRenameTriggered ();
		}
	});
	signalConnectionManager.connect (selectedUnitNameEdit->editingFinished, [&](eValidatorState )
	{
		if (activeUnit)
		{
			selectedUnitNameEdit->setText (activeUnit->isNameOriginal () ? activeUnit->data.name : activeUnit->getName ());
		}
	});
	signalConnectionManager.connect (selectedUnitNameEdit->textSet, [&]()
	{
		auto application = getActiveApplication ();
		if (application)
		{
			application->releaseKeyFocus (*selectedUnitNameEdit);
		}
	});

	cBox<cPosition> area (getPosition(), getPosition());
	area.add (selectedUnitStatusLabel->getArea());
	area.add (selectedUnitNamePrefixLabel->getArea ());
	area.add (selectedUnitNameEdit->getArea ());
	resize (area.getSize());
}

void cUnitRenameWidget::setUnit (const cUnit* unit)
{
	activeUnit = unit;
	unitSignalConnectionManager.disconnectAll ();

	if (unit)
	{
		selectedUnitNamePrefixLabel->setText (unit->getNamePrefix ());
		selectedUnitNameEdit->setText (unit->isNameOriginal () ? unit->data.name : unit->getName ());
		selectedUnitStatusLabel->setText (unit->getStatusStr (player));

		unitSignalConnectionManager.connect (unit->renamed, [&]()
		{
			if (activeUnit)
			{
				selectedUnitNameEdit->setText (activeUnit->isNameOriginal () ? activeUnit->data.name : activeUnit->getName ());
			}
		});
		unitSignalConnectionManager.connect (unit->statusChanged, [&]()
		{
			if (activeUnit)
			{
				selectedUnitStatusLabel->setText (activeUnit->getStatusStr (player));
			}
		});
	}
	else
	{
		selectedUnitNamePrefixLabel->setText ("");
		selectedUnitNameEdit->setText ("");
		selectedUnitNameEdit->disable ();
		selectedUnitStatusLabel->setText ("");
		return;
	}

	selectedUnitNameEdit->enable ();

	const auto xPosition = selectedUnitNamePrefixLabel->getPosition ().x () + font->getTextWide (selectedUnitNamePrefixLabel->getText () + " ", FONT_LATIN_SMALL_GREEN);
	const cPosition moveOffset (xPosition - selectedUnitNameEdit->getPosition ().x (), 0);
	selectedUnitNameEdit->move (moveOffset);
	selectedUnitNameEdit->resize (selectedUnitNameEdit->getSize () - moveOffset);
}

//------------------------------------------------------------------------------
const cUnit* cUnitRenameWidget::getUnit () const
{
	return activeUnit;
}

//------------------------------------------------------------------------------
void cUnitRenameWidget::setPlayer (const cPlayer* player_)
{
	player = player_;
	if (activeUnit)
	{
		selectedUnitStatusLabel->setText (activeUnit->getStatusStr (player));
	}
}

//------------------------------------------------------------------------------
const std::string& cUnitRenameWidget::getUnitName () const
{
	return selectedUnitNameEdit->getText ();
}
