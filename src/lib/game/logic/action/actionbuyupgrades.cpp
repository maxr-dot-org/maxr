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

#include "actionbuyupgrades.h"

#include "game/data/model.h"

//------------------------------------------------------------------------------
cActionBuyUpgrades::cActionBuyUpgrades (const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades) :
	unitUpgrades (unitUpgrades)
{}

//------------------------------------------------------------------------------
cActionBuyUpgrades::cActionBuyUpgrades (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionBuyUpgrades::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	auto player = model.getPlayer (playerNr);
	if (player == nullptr) return;

	const cUnitsData& unitsdata = *model.getUnitsData();

	for (const auto& [unitType, upgradesForUnit] : unitUpgrades)
	{
		if (!unitsdata.isValidId (unitType)) return;

		// check costs for upgrading this unit
		const auto& originalUnitData = unitsdata.getDynamicUnitData (unitType, player->getClan());
		auto& currentUnitData = *player->getLastUnitData (unitType);
		int costs = upgradesForUnit.calcTotalCosts (originalUnitData, currentUnitData, player->getResearchState());
		if (costs <= 0) continue;
		if (costs > player->getCredits()) continue;

		// everything ok, upgrade the unit
		player->setCredits (player->getCredits() - costs);
		currentUnitData.makeVersionDirty();
		upgradesForUnit.updateUnitData (currentUnitData);
	}
}
