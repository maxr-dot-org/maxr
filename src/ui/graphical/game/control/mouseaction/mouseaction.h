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

#ifndef ui_graphical_game_control_mouseaction_mouseactionH
#define ui_graphical_game_control_mouseaction_mouseactionH

class cMouse;
class cMapView;
class cPosition;
class cGameMapWidget;
class cUnitSelection;

/**
 * Interface for mouse actions.
 *
 * A mouse action gets selected/created by the current mouse mode.
 */
class cMouseAction
{
public:
	virtual ~cMouseAction() = default;
	/**
	 * Executes the action on a left click.
	 *
	 * @param gameMapWidget The game map widget to execute the action on.
	 * @param mapPosition The map position on that the action should be executed.
	 *
	 * @return True if the action has been executed successfully.
	 */
	virtual bool executeLeftClick (cGameMapWidget& gameMapWidget, const cPosition& mapPosition) const = 0;

	/**
	 * Should return true if the action does change the game state and therefore should not be executed while e.g. the client is frozen.
	 */
	virtual bool doesChangeState() const = 0;

	/**
	 * Should return true if the action is a single action, which means the mouse mode should return to the default mouse mode
	 * after the action has been executed.
	 */
	virtual bool isSingleAction() const = 0;
};

#endif // ui_graphical_game_control_mouseaction_mouseactionH
