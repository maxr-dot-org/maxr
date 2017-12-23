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

#ifndef ui_graphical_game_control_mousemode_mousemodeselectbuildpathdestinationH
#define ui_graphical_game_control_mousemode_mousemodeselectbuildpathdestinationH

#include "maxrconfig.h"
#include "ui/graphical/game/control/mousemode/mousemode.h"

class cMouseModeSelectBuildPathDestination : public cMouseMode
{
public:
	cMouseModeSelectBuildPathDestination (const cMapView* map, const cUnitSelection& unitSelection, const cPlayer* player);

	virtual eMouseModeType getType() const MAXR_OVERRIDE_FUNCTION;

	virtual void setCursor(cMouse& mouse, const cPosition& mapPosition, const cUnitsData& unitsData) const MAXR_OVERRIDE_FUNCTION;

	virtual std::unique_ptr<cMouseAction> getMouseAction(const cPosition& mapPosition, const cUnitsData& unitsData) const MAXR_OVERRIDE_FUNCTION;
};

#endif // ui_graphical_game_control_mousemode_mousemodeselectbuildpathdestinationH
