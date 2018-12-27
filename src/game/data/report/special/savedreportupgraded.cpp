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

#include "game/data/report/special/savedreportupgraded.h"
#include "game/data/player/player.h"

//------------------------------------------------------------------------------
cSavedReportUpgraded::cSavedReportUpgraded (const sID& unitId_, int unitsCount_, int costs_) :
	unitId (unitId_),
	unitsCount (unitsCount_),
	costs (costs_)
{}

//------------------------------------------------------------------------------
eSavedReportType cSavedReportUpgraded::getType() const
{
	return eSavedReportType::Upgraded;
}

//------------------------------------------------------------------------------
std::string cSavedReportUpgraded::getMessage(const cUnitsData& unitsData) const
{
	const auto& unitName = unitsData.getStaticUnitData(unitId).getName();
	return lngPack.i18n ("Text~Comp~Upgrades_Done") + " " + iToStr (unitsCount) + " " + lngPack.i18n ("Text~Comp~Upgrades_Done2", unitName) + " (" + lngPack.i18n ("Text~Others~Costs") + lngPack.i18n ("Text~Punctuation~Colon") + iToStr (costs) + ")";
}

//------------------------------------------------------------------------------
bool cSavedReportUpgraded::isAlert() const
{
	return false;
}
