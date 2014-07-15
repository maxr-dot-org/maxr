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

#ifndef ui_graphical_game_widgets_mousemode_mousemodeH
#define ui_graphical_game_widgets_mousemode_mousemodeH

#include <memory>

#include "ui/graphical/game/widgets/mousemode/mousemodetype.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

class cMouse;
class cMap;
class cPosition;
class cGameMapWidget;
class cUnitSelection;
class cMouseAction;
class cPlayer;
class cMapField;

class cMouseMode
{
public:
	cMouseMode (const cMap* map_, const cUnitSelection& unitSelection_, const cPlayer* player_);
	virtual ~cMouseMode ();

	void setMap (const cMap* map_);
	void setPlayer (const cPlayer* player_);

	void handleMapTilePositionChanged (const cPosition& mapPosition);

	virtual eMouseModeType getType () const = 0;

	virtual void setCursor (cMouse& mouse, const cPosition& mapPosition) const = 0;

	virtual std::unique_ptr<cMouseAction> getMouseAction (const cPosition& mapPosition) const = 0;

	mutable cSignal<void ()> needRefresh;
protected:
	cSignalConnectionManager selectedUnitSignalConnectionManager;
	cSignalConnectionManager mapFieldSignalConnectionManager;
	cSignalConnectionManager mapFieldUnitsSignalConnectionManager;

	const cMap* map;
	const cUnitSelection& unitSelection;
	const cPlayer* player;

	virtual void establishUnitSelectionConnections ();
	virtual void establishMapFieldConnections (const cMapField& field);

private:
	cSignalConnectionManager signalConnectionManager;

	void updateSelectedUnitConnections ();
};

#endif // ui_graphical_game_widgets_mousemode_mousemodeH
