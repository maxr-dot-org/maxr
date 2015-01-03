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
#include "netmessage.h"
#include "game/data/player/player.h"

//------------------------------------------------------------------------------
cSavedReportUpgraded::cSavedReportUpgraded (const sID& unitId_, int unitsCount_, int costs_) :
	unitId (unitId_),
	unitsCount (unitsCount_),
	costs (costs_)
{}

//------------------------------------------------------------------------------
cSavedReportUpgraded::cSavedReportUpgraded (cNetMessage& message)
{
	costs = message.popInt32();
	unitsCount = message.popInt32();
	unitId = message.popID();
}

//------------------------------------------------------------------------------
cSavedReportUpgraded::cSavedReportUpgraded (const tinyxml2::XMLElement& element)
{
	unitId.generate (element.Attribute ("id"));
	unitsCount = element.IntAttribute ("unitsCount");
	costs = element.IntAttribute ("costs");
}

//------------------------------------------------------------------------------
void cSavedReportUpgraded::pushInto (cNetMessage& message) const
{
	message.pushID (unitId);
	message.pushInt32 (unitsCount);
	message.pushInt32 (costs);

	cSavedReport::pushInto (message);
}

//------------------------------------------------------------------------------
void cSavedReportUpgraded::pushInto (tinyxml2::XMLElement& element) const
{
	element.SetAttribute ("id", unitId.getText().c_str());
	element.SetAttribute ("unitsCount", iToStr (unitsCount).c_str());
	element.SetAttribute ("costs", iToStr (costs).c_str());

	cSavedReport::pushInto (element);
}

//------------------------------------------------------------------------------
eSavedReportType cSavedReportUpgraded::getType() const
{
	return eSavedReportType::Upgraded;
}

//------------------------------------------------------------------------------
std::string cSavedReportUpgraded::getMessage() const
{
	const auto& unitName = unitId.getUnitDataOriginalVersion()->name;
	return lngPack.i18n ("Text~Comp~Upgrades_Done") + " " + iToStr (unitsCount) + " " + lngPack.i18n ("Text~Comp~Upgrades_Done2", unitName) + " (" + lngPack.i18n ("Text~Others~Costs") + ": " + iToStr (costs) + ")";
}

//------------------------------------------------------------------------------
bool cSavedReportUpgraded::isAlert() const
{
	return false;
}
