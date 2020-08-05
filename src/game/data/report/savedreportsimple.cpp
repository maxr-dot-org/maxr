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

#include "game/data/report/savedreportsimple.h"

#include "utility/language.h"

//------------------------------------------------------------------------------
cSavedReportSimple::cSavedReportSimple (eSavedReportType type_) :
	type (type_)
{
	// TODO: check valid type?!
}

//------------------------------------------------------------------------------
eSavedReportType cSavedReportSimple::getType() const
{
	return type;
}

//------------------------------------------------------------------------------
std::string cSavedReportSimple::getMessage(const cUnitsData& unitsData) const
{
	switch (type)
	{
		case eSavedReportType::MetalInsufficient:
			return lngPack.i18n ("Text~Comp~Metal_Insufficient");
		case eSavedReportType::FuelInsufficient:
			return lngPack.i18n ("Text~Comp~Fuel_Insufficient");
		case eSavedReportType::GoldInsufficient:
			return lngPack.i18n ("Text~Comp~Gold_Insufficient");
		case eSavedReportType::EnergyInsufficient:
			return lngPack.i18n ("Text~Comp~Energy_Insufficient");
		case eSavedReportType::TeamInsufficient:
			return lngPack.i18n ("Text~Comp~Team_Insufficient");

		case eSavedReportType::MetalLow:
			return lngPack.i18n ("Text~Comp~Metal_Low");
		case eSavedReportType::FuelLow:
			return lngPack.i18n ("Text~Comp~Fuel_Low");
		case eSavedReportType::GoldLow:
			return lngPack.i18n ("Text~Comp~Gold_Low");
		case eSavedReportType::EnergyLow:
			return lngPack.i18n ("Text~Comp~Energy_Low");
		case eSavedReportType::TeamLow:
			return lngPack.i18n ("Text~Comp~Team_Low");

		case eSavedReportType::EnergyToLow:
			return lngPack.i18n ("Text~Comp~Energy_ToLow");
		case eSavedReportType::EnergyIsNeeded:
			return lngPack.i18n ("Text~Comp~Energy_IsNeeded");

		case eSavedReportType::BuildingDisabled:
			return lngPack.i18n ("Text~Comp~Building_Disabled");

		case eSavedReportType::Producing_InsufficientMaterial:
			return lngPack.i18n ("Text~Comp~Producing_InsufficientMaterial");
		case eSavedReportType::Producing_PositionBlocked:
			return lngPack.i18n("Text~Comp~Producing_PositionBlocked");


		case eSavedReportType::TurnWait:
			return lngPack.i18n ("Text~Comp~Turn_Wait");
		case eSavedReportType::TurnAutoMove:
			return lngPack.i18n ("Text~Comp~Turn_Automove");

		case eSavedReportType::SuddenDeath:
			return lngPack.i18n("Text~Comp~SuddenDeath");
		default: break;
	}
	return "";
}

//------------------------------------------------------------------------------
bool cSavedReportSimple::isAlert() const
{
	switch (type)
	{
		case eSavedReportType::MetalInsufficient:
		case eSavedReportType::FuelInsufficient:
		case eSavedReportType::GoldInsufficient:
		case eSavedReportType::EnergyInsufficient:
		case eSavedReportType::TeamInsufficient:
		case eSavedReportType::EnergyIsNeeded:
		case eSavedReportType::BuildingDisabled:
			return true;
		default:
			return false;
	}
	return false;
}
