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

#include "ui/graphical/game/control/mouseaction/mouseactionactivatefinished.h"

#include "game/data/gui/unitselection.h"
#include "ui/graphical/game/widgets/gamemapwidget.h"

//------------------------------------------------------------------------------
bool cMouseActionActivateFinished::executeLeftClick (cGameMapWidget& gameMapWidget, const cPosition& mapPosition) const /* override */
{
	cUnitSelection& unitSelection = gameMapWidget.getUnitSelection();
	const auto selectedBuilding = unitSelection.getSelectedBuilding();

	if (!selectedBuilding) return false;

	gameMapWidget.triggeredExitFinishedUnit (*selectedBuilding, mapPosition);

	return true;
}

//------------------------------------------------------------------------------
bool cMouseActionActivateFinished::doesChangeState() const
{
	return true;
}

//------------------------------------------------------------------------------
bool cMouseActionActivateFinished::isSingleAction() const
{
	return true;
}
