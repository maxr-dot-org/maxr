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

#include "ui/graphical/menu/widgets/special/unitlistviewitembuy.h"

#include "game/data/player/player.h"
#include "ui/widgets/image.h"
#include "ui/widgets/label.h"

//------------------------------------------------------------------------------
cUnitListViewItemBuy::cUnitListViewItemBuy (unsigned int width, const sID& unitId, const cPlayer& owner, const cUnitsData& unitsData) :
	cUnitListViewItem (width, unitId, owner, unitsData),
	cost (owner.getLastUnitData (unitId)->getBuildCost())
{
	costLabel = emplaceChild<cLabel> (cBox<cPosition> (cPosition (width - 15, 0), cPosition (width, unitImage->getEndPosition().y())), std::to_string (cost), eUnicodeFontType::LatinSmallYellow, toEnumFlag (eAlignmentType::Right) | eAlignmentType::CenterVerical);
	costLabel->setConsumeClick (false);

	nameLabel->resize (nameLabel->getSize() - cPosition (15, 0));
}

//------------------------------------------------------------------------------
void cUnitListViewItemBuy::markAsInsufficient()
{
	nameLabel->setFont (eUnicodeFontType::LatinSmallRed);
}

//------------------------------------------------------------------------------
void cUnitListViewItemBuy::unmarkAsInsufficient()
{
	nameLabel->setFont (eUnicodeFontType::LatinSmallWhite);
}

//------------------------------------------------------------------------------
bool cUnitListViewItemBuy::isCostVisible() const
{
	return !costLabel->isHidden();
}

//------------------------------------------------------------------------------
void cUnitListViewItemBuy::hidePrice()
{
	costLabel->hide();
}
//------------------------------------------------------------------------------
void cUnitListViewItemBuy::showPrice()
{
	costLabel->show();
}
