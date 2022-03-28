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

#ifndef ui_graphical_game_control_mousemode_mousemodeH
#define ui_graphical_game_control_mousemode_mousemodeH

#include <memory>

#include "ui/graphical/game/control/mousemode/mousemodetype.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

class cMouse;
class cMapView;
class cPosition;
class cGameMapWidget;
class cUnitSelection;
class cMouseAction;
class cPlayer;
class cMapFieldView;
class cUnitsData;

/**
 * Abstract base class for mouse modes.
 *
 * A mouse mode handles the mouse cursor and the action that gets performed on a click onto the map.
 */
class cMouseMode
{
public:
	/**
	 * Creates the instance of the new mouse mode.
	 *
	 * @param map The map on which the mouse mode should operate. May be null.
	 * @param unitSelection The current unit selection.
	 * @param player The player that is currently active in the GUI. May be null.
	 */
	cMouseMode (const cMapView* map, const cUnitSelection& unitSelection, const cPlayer* player);
	virtual ~cMouseMode();

	/**
	 * Set an other map to the mouse mode.
	 * @param map The new map. May be null.
	 */
	void setMap (const cMapView* map);
	/**
	 *
	 * @param player
	 */
	void setPlayer (const cPlayer* player);

	/**
	 * Should be called whenever the mouse moved from one map position to another one.
	 *
	 * @param mapPosition The new position on the map the mouse is now over.
	 *                    Can be an invalid position (e.g. (-1, -1)) if the mouse is currently not over
	 *                    any map field.
	 */
	void handleMapTilePositionChanged (const cPosition& mapPosition);

	/**
	 * Should return the type of the mouse modes implementation. This can be used
	 * for quick comparison of mouse modes (maybe even without the need to construct
	 * a mouse mode object).
	 */
	virtual eMouseModeType getType() const = 0;

	/**
	 * Should set the cursor on the passed mouse object according to the current
	 * position of the mouse on the map and the underlying mouse mode implementation.
	 *
	 * @param mouse The mouse to set the cursor to.
	 * @param mapPosition The position on the map where the mouse is currently located on.
	 */
	virtual void setCursor (cMouse& mouse, const cPosition& mapPosition, const cUnitsData& unitsData) const = 0;

	/**
	 * Gets the action that should be performed on a click at the given map position.
	 *
	 * @param mapPosition The position on the map for which the action should be returned.
	 * @return The action to be performed. Can be null if the mouse modes implementation
	 *         can not execute any action at the given position.
	 */
	virtual std::unique_ptr<cMouseAction> getMouseAction (const cPosition& mapPosition, const cUnitsData& unitsData) const = 0;

	/**
	 * Gets called when the mouse mode needs to be refreshed (E.g. because the active map
	 * or player has been changed).
	 */
	mutable cSignal<void()> needRefresh;
protected:
	/**
	 * Signal connection manager for connections to signals from selected units.
	 * All connected connections will be disconnected whenever the selection changes.
	 */
	cSignalConnectionManager selectedUnitSignalConnectionManager;
	/**
	 * Signal connection manager for connections to signals from the map field that
	 * is currently below the mouse.
	 * All connected connections will be disconnected whenever the mouse leaves the current field.
	 */
	cSignalConnectionManager mapFieldSignalConnectionManager;
	/**
	 * Signal connection manager for connections to signals from units on the map field
	 * that is currently below the mouse.
	 * All connected connections will be disconnected whenever the mouse leaves the current field.
	 */
	cSignalConnectionManager mapFieldUnitsSignalConnectionManager;

	const cMapView* map = nullptr;
	const cUnitSelection& unitSelection;
	const cPlayer* player = nullptr;

	/**
	 * Gets called whenever the unit selection has changed.
	 * Allows the mouse mode implementations to react on selection changes and
	 * establish new connections to signals triggered by the selected units.
	 */
	virtual void establishUnitSelectionConnections();
	/**
	 * Gets called whenever the map field below the mouse has changed.
	 * Allows the mouse mode implementations to react on mouse movements and
	 * establish new connections to signals triggered by the the units that are
	 * below the mouse.
	 *
	 * @param field The new field below the mouse.
	 */
	virtual void establishMapFieldConnections (const cMapFieldView& field);

private:
	cSignalConnectionManager signalConnectionManager;

	void updateSelectedUnitConnections();
};

#endif // ui_graphical_game_control_mousemode_mousemodeH
