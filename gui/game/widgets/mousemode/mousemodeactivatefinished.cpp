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

#include "mousemodeactivatefinished.h"
#include "../gamemapwidget.h"
#include "../../unitselection.h"
#include "../../../../map.h"
#include "../../../../buildings.h"
#include "../../../../input/mouse/mouse.h"
#include "../../../../input/mouse/cursor/mousecursorsimple.h"

//------------------------------------------------------------------------------
void cMouseModeActivateFinished::setCursor (cMouse& mouse, const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection) const
{
	if (canExecuteAction (map, mapPosition, unitSelection))
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Activate));
	}
	else
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::No));
	}
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseAction> cMouseModeActivateFinished::getMouseAction (const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection) const
{
	if (canExecuteAction (map, mapPosition, unitSelection))
	{
		return std::make_unique<cMouseActionTransfer> ();
	}
	else return nullptr;
}

//------------------------------------------------------------------------------
bool cMouseModeActivateFinished::canExecuteAction (const cMap& map, const cPosition& mapPosition, const cUnitSelection& unitSelection) const
{
	const auto& field = map.getField (mapPosition);

	const auto selectedUnit = unitSelection.getSelectedUnit ();

	return selectedUnit && selectedUnit->canTransferTo (mapPosition, field);
}