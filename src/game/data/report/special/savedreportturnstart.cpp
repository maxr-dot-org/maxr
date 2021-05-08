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

#include "savedreportturnstart.h"

#include "utility/language.h"
#include "utility/string/toString.h"

//------------------------------------------------------------------------------
cSavedReportTurnStart::cSavedReportTurnStart (int turn, const std::vector<sTurnstartReport>& unitReports, const std::vector<cResearch::ResearchArea>& researchAreas) :
	turn (turn),
	unitReports (unitReports),
	researchAreas (researchAreas)
{}

//------------------------------------------------------------------------------
eSavedReportType cSavedReportTurnStart::getType() const
{
	return eSavedReportType::TurnStart;
}

//------------------------------------------------------------------------------
std::string cSavedReportTurnStart::getMessage(const cUnitsData& unitsData) const
{
	std::string message = lngPack.i18n ("Text~Comp~Turn_Start") + " " + iToStr (turn);

	if (!unitReports.empty())
	{
		int totalUnitsCount = 0;
		message += "\n";
		for (size_t i = 0; i < unitReports.size(); ++i)
		{
			const auto& entry = unitReports[i];

			if (i > 0) message += ", ";
			totalUnitsCount += entry.count;
			message += entry.count > 1 ? (iToStr(entry.count) + " " + unitsData.getStaticUnitData(entry.type).getName()) : (unitsData.getStaticUnitData(entry.type).getName());
		}
		// TODO: Plural rules are language dependant
		// - Russian has 3 forms, Chinese 1 form, ...
		//          | eng  | fre  | ...
		// singular | == 1 | <= 1 |
		// plural   | != 1 | 1 <  |
		// we should have `i18n (key, n)`
		if (totalUnitsCount == 1) message += " " + lngPack.i18n ("Text~Comp~Finished") + ".";
		else if (totalUnitsCount > 1) message += " " + lngPack.i18n ("Text~Comp~Finished2") + ".";
	}

	if (!researchAreas.empty())
	{
		message += "\n";
		message += lngPack.i18n ("Text~Others~Research") + " " + lngPack.i18n ("Text~Comp~Finished") + lngPack.i18n ("Text~Punctuation~Colon");

		const std::string themeNames[8] =
		{
			lngPack.i18n ("Text~Others~Attack"),
			lngPack.i18n ("Text~Others~Shots"),
			lngPack.i18n ("Text~Others~Range"),
			lngPack.i18n ("Text~Others~Armor"),
			lngPack.i18n ("Text~Others~Hitpoints"),
			lngPack.i18n ("Text~Others~Speed"),
			lngPack.i18n ("Text~Others~Scan"),
			lngPack.i18n ("Text~Others~Costs")
		};

		for (size_t i = 0; i < researchAreas.size(); ++i)
		{
			const auto researchArea = researchAreas[i];
			if (researchArea >= 0 && researchArea < 8)
			{
				if (i > 0) message += ", ";
				message += themeNames[researchArea];
			}
		}
	}

	return message;
}

//------------------------------------------------------------------------------
bool cSavedReportTurnStart::isAlert() const
{
	return false;
}
