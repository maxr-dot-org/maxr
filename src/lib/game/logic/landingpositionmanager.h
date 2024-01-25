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

#ifndef game_logic_landingpositionmanagerH
#define game_logic_landingpositionmanagerH

#include "game/data/player/playerbasicdata.h"
#include "game/logic/landingpositionstate.h"
#include "utility/position.h"
#include "utility/signal/signal.h"

#include <memory>
#include <vector>

class cPlayerBasicData;

class cLandingPositionManager
{
	struct sLandingPositionData
	{
		explicit sLandingPositionData (cPlayerBasicData player);

		cPosition landingPosition;
		cPosition lastLandingPosition;

		eLandingPositionState state = eLandingPositionState::Unknown;

		cPlayerBasicData player;

		bool needNewPosition = true;
	};

public:
	static const double warningDistance;
	static const double tooCloseDistance;

	/**
	 * Initializes the manager with a list of players.
	 *
	 * @param players The players whose landings should be managed by this object.
	 */
	explicit cLandingPositionManager (const std::vector<cPlayerBasicData>& players);

	/**
	 * Sets a new landing position for the given player.
	 *
	 * The landing position of the player will be checked against the positions of all other players
	 * that have been set before.
	 * The first check will be performed when all players have selected a position at least once.
	 *
	 * This method will invoke the signals @ref landingPositionStateChanged and @ref allPositionsValid.
	 *
	 * @param player The player to set a new landing position for.
	 * @param landingPosition The new landing position to set for the player.
	 * @return true if the passed player was the last one that had an uninitialized or
	 *         invalid landing position and now all positions are valid.
	 *         false if a player needs to select a new landing position, which is the case when
	 *         he has not selected any position yet, or his last position was in conflict with
	 *         the position of an other player.
	 */
	bool setLandingPosition (const cPlayerBasicData& player, const cPosition& landingPosition);

	/**
	 * Deletes the landing position of a player.
	 *
	 * @param player The player whose landing position to delete.
	 */
	void deleteLandingPosition (const cPlayerBasicData& player);

	/**
	 * Return the current landing state of the player.
	 *
	 * @param player The player to get the state for.
	 * @return The landing state.
	 */
	eLandingPositionState getPlayerState (const cPlayerBasicData& player) const;

	/**
	 * Will be triggered when ever a player has selected a new landing position.
	 */
	mutable cSignal<void (const cPlayerBasicData&, const cPosition&)> landingPositionSet;
	/**
	 * Will be triggered by @ref setLandingPosition when the landing position of a player has changed.
	 * The arguments are the players whose state has changed and his new state.
	 * If the new state is not @ref eLandingPositionState::Clear or @ref eLandingPositionState::Confirmed
	 * The player needs to select a new landing position.
	 */
	mutable cSignal<void (const cPlayerBasicData&, eLandingPositionState)> landingPositionStateChanged;
	/**
	 * Will be triggered by @ref setLandingPosition when the last player has selected his position and
	 * none of the positions are in conflict.
	 */
	mutable cSignal<void()> allPositionsValid;

private:
	std::vector<sLandingPositionData> landingPositions;

	sLandingPositionData& getLandingPositionData (const cPlayerBasicData& player);
	const sLandingPositionData& getLandingPositionData (const cPlayerBasicData& player) const;

	void checkPlayerState (sLandingPositionData& playerData, bool isOtherPlayer);
};

#endif // game_logic_landingpositionmanagerH
