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

#include "ui/graphical/menu/widgets/special/buildspeedhandlerwidget.h"

#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/radiogroup.h"
#include "utility/language.h"

#include <cassert>

//------------------------------------------------------------------------------
cBuildSpeedHandlerWidget::cBuildSpeedHandlerWidget (const cPosition& position) :
	cWidget (position)
{
	cBox<cPosition> area (position, position);
	auto speedGroup = addChild (std::make_unique<cRadioGroup>());
	for (size_t i = 0; i < elementsCount; ++i)
	{
		int factor = i + 1;
		if (i == 2) factor = 4;
		turnLabels[i] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (80, 5 + 25 * i), getPosition() + cPosition (80 + 35, 5 + 25 * i + 10)), "", eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal));
		costLabels[i] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (120, 5 + 25 * i), getPosition() + cPosition (120 + 35, 5 + 25 * i + 10)), "", eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal));
		buttons[i] = speedGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (0, 25 * i), lngPack.i18n ("Text~Others~Build_7") + " x" + std::to_string (factor), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Left, eCheckBoxType::Angular));

		area.add (turnLabels[i]->getArea());
		area.add (costLabels[i]->getArea());
	}

	area.add (speedGroup->getArea());
	resize (area.getSize());
}

//------------------------------------------------------------------------------
void cBuildSpeedHandlerWidget::setValues (const std::array<int, elementsCount>& turns, const std::array<int, elementsCount>& costs)
{
	for (size_t i = 0; i < elementsCount; ++i)
	{
		if (turns[i] > 0)
		{
			turnLabels[i]->setText (std::to_string (turns[i]));
			costLabels[i]->setText (std::to_string (costs[i]));
			buttons[i]->unlock();
		}
		else
		{
			turnLabels[i]->setText ("");
			costLabels[i]->setText ("");
			buttons[i]->lock();
			if (buttons[i]->isChecked() && i > 0) buttons[i - 1]->setChecked (true);
		}
	}
}

//------------------------------------------------------------------------------
void cBuildSpeedHandlerWidget::setBuildSpeedIndex (size_t speedIndex)
{
	buttons[speedIndex]->setChecked (true);
}

//------------------------------------------------------------------------------
size_t cBuildSpeedHandlerWidget::getBuildSpeedIndex()
{
	for (size_t i = 0; i < elementsCount; ++i)
	{
		if (buttons[i]->isChecked()) return i;
	}
	assert (false);
	return 0;
}
