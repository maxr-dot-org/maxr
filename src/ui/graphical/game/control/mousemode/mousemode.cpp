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

#include "ui/graphical/game/control/mousemode/mousemode.h"

#include "game/data/gui/unitselection.h"
#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"

//------------------------------------------------------------------------------
cMouseMode::cMouseMode (const cMapView* map_, const cUnitSelection& unitSelection_, const cPlayer* player_) :
	map (map_),
	unitSelection (unitSelection_),
	player (player_)
{
	signalConnectionManager.connect (unitSelection.selectionChanged, [this]() { updateSelectedUnitConnections(); });
}

//------------------------------------------------------------------------------
void cMouseMode::handleMapTilePositionChanged (const cPosition& mapPosition)
{
	if (!map) return;

	mapFieldSignalConnectionManager.disconnectAll();
	mapFieldUnitsSignalConnectionManager.disconnectAll();

	if (map->isValidPosition (mapPosition))
	{
		const auto& field = map->getField (mapPosition);

		establishMapFieldConnections (field);
	}
}

//------------------------------------------------------------------------------
void cMouseMode::updateSelectedUnitConnections()
{
	selectedUnitSignalConnectionManager.disconnectAll();

	establishUnitSelectionConnections();
}

//------------------------------------------------------------------------------
cMouseMode::~cMouseMode()
{}

//------------------------------------------------------------------------------
void cMouseMode::setMap (const cMapView* map_)
{
	map = map_;

	needRefresh();
}

//------------------------------------------------------------------------------
void cMouseMode::setPlayer (const cPlayer* player_)
{
	player = player_;

	needRefresh();
}

//------------------------------------------------------------------------------
/* virtual */ void cMouseMode::establishUnitSelectionConnections()
{}

//------------------------------------------------------------------------------
/* virtual */ void cMouseMode::establishMapFieldConnections (const cMapFieldView&)
{}
