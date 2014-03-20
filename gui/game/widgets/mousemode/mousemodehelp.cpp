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

#include "mousemodehelp.h"
#include "../mouseaction/mouseactionhelp.h"
#include "../../unitselection.h"
#include "../../../../map.h"
#include "../../../../unit.h"
#include "../../../../input/mouse/mouse.h"
#include "../../../../input/mouse/cursor/mousecursorsimple.h"

//------------------------------------------------------------------------------
eMouseModeType cMouseModeHelp::getType () const
{
	return eMouseModeType::Help;
}

//------------------------------------------------------------------------------
void cMouseModeHelp::setCursor (cMouse& mouse, const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection, const cPlayer* player) const
{
	mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Help));
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseAction> cMouseModeHelp::getMouseAction (const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection, const cPlayer* player) const
{
	return std::make_unique<cMouseActionHelp> ();
}