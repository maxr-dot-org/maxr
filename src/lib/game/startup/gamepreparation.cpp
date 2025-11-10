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

#include "gamepreparation.h"

#include "game/data/gamesettings.h"
#include "game/data/units/unitdata.h"

//------------------------------------------------------------------------------
std::vector<std::pair<sID, int>> computeInitialLandingUnits (int clan, const cGameSettings& gameSettings, const cUnitsData& unitsData)
{
	if (gameSettings.bridgeheadType == eGameSettingsBridgeheadType::Mobile) return {};

	const auto constructorID = unitsData.getConstructorID();
	const auto engineerID = unitsData.getEngineerID();
	const auto surveyorID = unitsData.getSurveyorID();

	std::vector<std::pair<sID, int>> initialLandingUnits{
		{constructorID, 40},
		{engineerID, 20},
		{surveyorID, 0}};

	if (clan == 7)
	{
		const int startCredits = gameSettings.startCredits;

		size_t numAddConstructors = 0;
		size_t numAddEngineers = 0;

		if (startCredits < 100)
		{
			numAddEngineers = 1;
		}
		else if (startCredits < 150)
		{
			numAddEngineers = 1;
			numAddConstructors = 1;
		}
		else if (startCredits < 200)
		{
			numAddEngineers = 2;
			numAddConstructors = 1;
		}
		else if (startCredits < 300)
		{
			numAddEngineers = 2;
			numAddConstructors = 2;
		}
		else
		{
			numAddEngineers = 3;
			numAddConstructors = 2;
		}

		for (size_t i = 0; i != numAddConstructors; ++i)
		{
			initialLandingUnits.emplace_back (constructorID, 0);
		}
		for (size_t i = 0; i != numAddEngineers; ++i)
		{
			initialLandingUnits.emplace_back (engineerID, 0);
		}
	}

	return initialLandingUnits;
}
