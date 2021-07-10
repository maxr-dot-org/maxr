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

#include "ui/graphical/menu/widgets/special/unitlistviewitemcargo.h"

#include "ui/graphical/menu/widgets/image.h"
#include "ui/graphical/menu/widgets/label.h"
#include "game/data/player/player.h"

//------------------------------------------------------------------------------
cUnitListViewItemCargo::cUnitListViewItemCargo (unsigned int width, const sID& unitId, const cPlayer& owner, const cUnitsData& unitsData) :
	cUnitListViewItem (width, unitId, owner, unitsData),
	cargo (0)
{
	unitData = &unitsData.getStaticUnitData (unitId);

	if (unitData->storeResType == eResourceType::Metal || unitData->storeResType == eResourceType::Oil)
	{
		cargoLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (nameLabel->getPosition().x() + 15, nameLabel->getEndPosition().y() - 13), nameLabel->getEndPosition() - cPosition (0, 3)), std::to_string (owner.getUnitDataCurrentVersion (unitId)->getBuildCost()), FONT_LATIN_SMALL_YELLOW, toEnumFlag (eAlignmentType::Left) | eAlignmentType::CenterVerical));
		cargoLabel->setConsumeClick (false);

		nameLabel->resize (nameLabel->getSize() - cPosition (0, 13));
		nameLabel->setAlignment (toEnumFlag (eAlignmentType::Left) | eAlignmentType::Bottom);

		updateCargoLabel();
	}
	else
	{
		cargoLabel = nullptr;
	}
}

//------------------------------------------------------------------------------
int cUnitListViewItemCargo::getCargo() const
{
	return cargo;
}

//------------------------------------------------------------------------------
void cUnitListViewItemCargo::setCargo (int cargo_)
{
	cargo = cargo_;
	updateCargoLabel();
}

//------------------------------------------------------------------------------
void cUnitListViewItemCargo::updateCargoLabel()
{
	if (cargoLabel)
	{
		if (cargo == 0) cargoLabel->setText ("(empty)");
		else cargoLabel->setText ("(" + std::to_string (cargo) + "/" + std::to_string (unitData->storageResMax) + ")");

		if (cargo <= unitData->storageResMax / 4) cargoLabel->setFont (FONT_LATIN_SMALL_RED);
		else if (cargo <= unitData->storageResMax / 2) cargoLabel->setFont (FONT_LATIN_SMALL_YELLOW);
		else cargoLabel->setFont (FONT_LATIN_SMALL_GREEN);
	}
}
