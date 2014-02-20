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

#include "unitlistviewitemcargo.h"
#include "../image.h"
#include "../label.h"
#include "../../../../main.h"
#include "../../../../player.h"

//------------------------------------------------------------------------------
cUnitListViewItemCargo::cUnitListViewItemCargo (unsigned int width, const sID& unitId, cPlayer& owner) :
	cUnitListViewItem (width, unitId, owner),
	cargo (0)
{
	unitData = unitId.getUnitDataOriginalVersion (&owner);

	if (unitData->storeResType == sUnitData::STORE_RES_METAL || unitData->storeResType == sUnitData::STORE_RES_OIL)
	{
		cargoLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (nameLabel->getPosition ().x () + 15, nameLabel->getEndPosition ().y () - 13), nameLabel->getEndPosition () - cPosition (0, 3)), iToStr (owner.getUnitDataCurrentVersion (unitId)->buildCosts), FONT_LATIN_SMALL_YELLOW, toEnumFlag (eAlignmentType::Left) | eAlignmentType::CenterVerical));

		nameLabel->resize (nameLabel->getSize () - cPosition (0, 13));
		nameLabel->setAlignment (toEnumFlag (eAlignmentType::Left) | eAlignmentType::Bottom);

		updateCargoLabel ();
	}
	else
	{
		cargoLabel = nullptr;
	}
}

//------------------------------------------------------------------------------
int cUnitListViewItemCargo::getCargo () const
{
	return cargo;
}

//------------------------------------------------------------------------------
void cUnitListViewItemCargo::setCargo (int cargo_)
{
	cargo = cargo_;
	updateCargoLabel ();
}

//------------------------------------------------------------------------------
void cUnitListViewItemCargo::updateCargoLabel ()
{
	if (cargoLabel)
	{
		if (cargo == 0) cargoLabel->setText ("(empty)");
		else cargoLabel->setText("(" + iToStr (cargo) + "/" + iToStr (unitData->storageResMax) + ")");

		if (cargo <= unitData->storageResMax / 4) cargoLabel->setFont (FONT_LATIN_SMALL_RED);
		else if (cargo <= unitData->storageResMax / 2) cargoLabel->setFont (FONT_LATIN_SMALL_YELLOW);
		else cargoLabel->setFont (FONT_LATIN_SMALL_GREEN);
	}
}
