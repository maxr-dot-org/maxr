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
}
