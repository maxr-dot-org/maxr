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

#ifndef game_logic_landingpositionstateH
#define game_logic_landingpositionstateH

enum class eLandingPositionState
{
	/**
	 * player has not selected any position yet
	 */
	Unknown,
	/**
	 * there are no other players near the position
	 */
	Clear,
	/**
	 * there are players within the warning distance
	 */
	Warning,
	/**
	 * the position is too close to another player
	 */
	TooClose,
	/**
	 * warnings about nearby players will be ignored,
	 * because the player has confirmed his position
	 */
	Confirmed
};

#endif // game_logic_landingpositionstateH
