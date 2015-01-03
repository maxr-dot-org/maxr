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

#include "ui/graphical/game/unitselectionbox.h"

//------------------------------------------------------------------------------
cUnitSelectionBox::cUnitSelectionBox()
{
	invalidate();
}

//------------------------------------------------------------------------------
bool cUnitSelectionBox::isTooSmall() const
{
	if (!isValid()) return true;

	auto diff = box.getSize();
	return std::abs (diff[0]) < 0.5 && std::abs (diff[1]) < 0.5;
}

//------------------------------------------------------------------------------
bool cUnitSelectionBox::isValid() const
{
	return isValidStart() && isValidEnd();
}

//------------------------------------------------------------------------------
bool cUnitSelectionBox::isValidStart() const
{
	return box.getMinCorner()[0] != -1 && box.getMinCorner()[1];
}

//------------------------------------------------------------------------------
bool cUnitSelectionBox::isValidEnd() const
{
	return box.getMaxCorner()[0] != -1 && box.getMaxCorner()[1] != -1;
}

//------------------------------------------------------------------------------
void cUnitSelectionBox::invalidate()
{
	box.getMinCorner()[0] = -1;
	box.getMinCorner()[1] = -1;
	box.getMaxCorner()[0] = -1;
	box.getMaxCorner()[1] = -1;
}

//------------------------------------------------------------------------------
cBox<cFixedVector<double, 2>>& cUnitSelectionBox::getBox()
{
	return box;
}

//------------------------------------------------------------------------------
const cBox<cFixedVector<double, 2>>& cUnitSelectionBox::getBox() const
{
	return box;
}

//------------------------------------------------------------------------------
cBox<cPosition> cUnitSelectionBox::getCorrectedMapBox() const
{
	cPosition minPosition (static_cast<int> (std::min (box.getMinCorner()[0], box.getMaxCorner()[0])), static_cast<int> (std::min (box.getMinCorner()[1], box.getMaxCorner()[1])));
	cPosition maxPosition (static_cast<int> (std::max (box.getMinCorner()[0], box.getMaxCorner()[0])), static_cast<int> (std::max (box.getMinCorner()[1], box.getMaxCorner()[1])));

	return cBox<cPosition> (minPosition, maxPosition);
}
