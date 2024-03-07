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

#include "ui/graphical/game/control/mousemode/mousemodehelp.h"

#include "game/data/gui/unitselection.h"
#include "game/data/map/mapview.h"
#include "game/data/units/unit.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "input/mouse/mouse.h"
#include "ui/graphical/game/control/mouseaction/mouseactionhelp.h"

//------------------------------------------------------------------------------
cMouseModeHelp::cMouseModeHelp (const cMapView* map_, const cUnitSelection& unitSelection_, const cPlayer* player_) :
	cMouseMode (map_, unitSelection_, player_)
{}

//------------------------------------------------------------------------------
eMouseModeType cMouseModeHelp::getType() const
{
	return eMouseModeType::Help;
}

//------------------------------------------------------------------------------
void cMouseModeHelp::setCursor (cMouse& mouse, const cPosition&, const cUnitsData&) const /* override */
{
	mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Help));
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseAction> cMouseModeHelp::getMouseAction (const cPosition&, const cUnitsData&) const /* override */
{
	return std::make_unique<cMouseActionHelp>();
}
