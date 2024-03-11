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

#include "ui/graphical/game/control/mouseaction/mouseactionattack.h"

#include "game/data/gui/unitselection.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "ui/graphical/game/widgets/gamemapwidget.h"

//------------------------------------------------------------------------------
bool cMouseActionAttack::executeLeftClick (cGameMapWidget& gameMapWidget, const cPosition& mapPosition) const /* override */
{
	cUnitSelection& unitSelection = gameMapWidget.getUnitSelection();
	const auto selectedVehicle = unitSelection.getSelectedVehicle();
	const auto selectedBuilding = unitSelection.getSelectedBuilding();

	if (selectedVehicle && !selectedVehicle->isAttacking() && !selectedVehicle->isUnitMoving())
	{
		gameMapWidget.triggeredAttack (*selectedVehicle, mapPosition);
	}
	else if (selectedBuilding && !selectedBuilding->isAttacking())
	{
		gameMapWidget.triggeredAttack (*selectedBuilding, mapPosition);
	}
	else
	{
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
bool cMouseActionAttack::doesChangeState() const
{
	return true;
}

//------------------------------------------------------------------------------
bool cMouseActionAttack::isSingleAction() const
{
	return false;
}
