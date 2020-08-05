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

#include "game/data/report/savedreportunit.h"

#include "game/data/units/unit.h"
#include "utility/string/toString.h"

//------------------------------------------------------------------------------
cSavedReportUnit::cSavedReportUnit (const cUnit& unit) :
	unitId (unit.data.getId()),
	position (unit.getPosition())
{}

//------------------------------------------------------------------------------
bool cSavedReportUnit::hasUnitId() const
{
	return true;
}

//------------------------------------------------------------------------------
const sID& cSavedReportUnit::getUnitId() const
{
	return unitId;
}

//------------------------------------------------------------------------------
bool cSavedReportUnit::hasPosition() const
{
	return true;
}

//------------------------------------------------------------------------------
const cPosition& cSavedReportUnit::getPosition() const
{
	return position;
}

//------------------------------------------------------------------------------
std::string cSavedReportUnit::getMessage(const cUnitsData& unitsData) const
{
	return "[" + iToStr (position.x()) + ", " + iToStr (position.y()) + "] " + getText();
}

//------------------------------------------------------------------------------
bool cSavedReportUnit::isAlert() const
{
	return false;
}
