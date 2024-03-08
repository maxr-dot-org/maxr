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

#include "ui/graphical/game/control/mouseaction/mouseactionselectbuildposition.h"

#include "game/data/gui/unitselection.h"
#include "game/data/units/unit.h"
#include "ui/graphical/game/widgets/gamemapwidget.h"

//------------------------------------------------------------------------------
cMouseActionSelectBuildPosition::cMouseActionSelectBuildPosition (sID buildId_, const cPosition& buildPosition_) :
	buildId (buildId_),
	buildPosition (buildPosition_)
{}

//------------------------------------------------------------------------------
bool cMouseActionSelectBuildPosition::executeLeftClick (cGameMapWidget& gameMapWidget, const cMapView&, const cPosition&, cUnitSelection& unitSelection, bool /* changeAllowed */) const /* override */
{
	const auto selectedVehicle = unitSelection.getSelectedVehicle();

	if (!selectedVehicle) return false;

	gameMapWidget.selectedBuildPosition (*selectedVehicle, buildPosition);

	return true;
}

//------------------------------------------------------------------------------
bool cMouseActionSelectBuildPosition::doesChangeState() const
{
	return true;
}

//------------------------------------------------------------------------------
bool cMouseActionSelectBuildPosition::isSingleAction() const
{
	return true;
}
