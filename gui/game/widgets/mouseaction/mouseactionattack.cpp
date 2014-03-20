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

#include "mouseactionattack.h"
#include "../gamemapwidget.h"
#include "../../unitselection.h"
#include "../../../../map.h"
#include "../../../../vehicles.h"
#include "../../../../buildings.h"
#include "../../../../input/mouse/mouse.h"
#include "../../../../input/mouse/cursor/mousecursorsimple.h"

//------------------------------------------------------------------------------
bool cMouseActionAttack::executeLeftClick (cGameMapWidget& gameMapWidget, const cMap& map, const cPosition& mapPosition, cUnitSelection& unitSelection) const
{
	const auto selectedVehicle = unitSelection.getSelectedVehicle ();
	const auto selectedBuilding = unitSelection.getSelectedBuilding ();

	if (selectedVehicle && !selectedVehicle->isAttacking () && !selectedVehicle->MoveJobActive)
	{
		gameMapWidget.triggeredAttack (*selectedVehicle, mapPosition);
	}
	else if (selectedBuilding && !selectedBuilding->isAttacking ())
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
bool cMouseActionAttack::doesChangeState () const
{
	return true;
}

//------------------------------------------------------------------------------
bool cMouseActionAttack::isSingleAction () const
{
	return false;
}