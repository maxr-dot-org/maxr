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

#include "game/data/report/unit/savedreportdestroyed.h"

#include "game/data/units/unit.h"
#include "ui/translations.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cSavedReportDestroyed::cSavedReportDestroyed (const cUnit& unit) :
	cSavedReportUnit (unit)
{}

//------------------------------------------------------------------------------
eSavedReportType cSavedReportDestroyed::getType() const
{
	return eSavedReportType::Destroyed;
}

//------------------------------------------------------------------------------
std::string cSavedReportDestroyed::getText (const cUnit& unit) const
{
	return getDisplayName (unit) + " " + lngPack.i18n ("Text~Comp~Destroyed");
}
