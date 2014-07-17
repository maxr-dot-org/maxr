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

#include <algorithm>

#include "game/logic/landingpositionmanager.h"
#include "game/data/player/player.h"

const double cLandingPositionManager::warningDistance = 28;
const double cLandingPositionManager::tooCloseDistance = 10;

//------------------------------------------------------------------------------
cLandingPositionManager::sLandingPositionData::sLandingPositionData (std::shared_ptr<sPlayer> player) :
	landingPosition (0, 0),
	lastLandingPosition (0, 0),
	state (eLandingPositionState::Waiting),
	player (std::move (player))
{}

//------------------------------------------------------------------------------
cLandingPositionManager::cLandingPositionManager (const std::vector<std::shared_ptr<sPlayer>>& players)
{
	for (size_t i = 0; i < players.size (); ++i)
	{
		landingPositions.push_back (sLandingPositionData(players[i]));
	}
}

//------------------------------------------------------------------------------
void cLandingPositionManager::setLandingPosition (const sPlayer& player, const cPosition& landingPosition)
{
	auto& playerData = getLandingPositionData (player);

	playerData.lastLandingPosition = playerData.landingPosition;
	playerData.landingPosition = landingPosition;

	if (playerData.state == eLandingPositionState::Waiting)
	{
		playerData.state = eLandingPositionState::Clear;
	}

	// wait for all players to choose at least ones
	for (size_t i = 0; i != landingPositions.size (); ++i)
	{
		if (landingPositions[i].state == eLandingPositionState::Waiting) return;
	}

	// update all states
	for (size_t i = 0; i != landingPositions.size (); ++i)
	{
		checkPlayerState (*landingPositions[i].player);
	}

	// test if all states are valid now.
	// remove invalid states
	bool allValid = true;
	for (size_t i = 0; i != landingPositions.size (); ++i)
	{
		const auto state = landingPositions[i].state;
		if (state == eLandingPositionState::Warning || state == eLandingPositionState::TooClose)
		{
			landingPositions[i].state = eLandingPositionState::Waiting;
			allValid = false;
		}
	}

	if (allValid)
	{
		allPositionsValid ();
	}
}

//------------------------------------------------------------------------------
void cLandingPositionManager::checkPlayerState (const sPlayer& player)
{
	auto& playerData = getLandingPositionData (player);

	bool positionTooClose = false;
	bool positionWarning = false;
	// check distances to all other players
	for (size_t i = 0; i != landingPositions.size (); ++i)
	{
		const auto& data = landingPositions[i];
		if (data.state == eLandingPositionState::Waiting) continue;
		if (data.player->getNr () == player.getNr()) continue;

		const auto distance = (playerData.landingPosition - data.landingPosition).l2Norm ();

		if (distance < tooCloseDistance)
		{
			positionTooClose = true;
		}
		if (distance < warningDistance)
		{
			positionWarning = true;
		}
	}

	// now set the new landing state,
	// depending on the last state, the last position, the current position,
	//  positionTooClose and positionWarning
	auto newState = eLandingPositionState::Waiting;

	if (positionTooClose)
	{
		newState = eLandingPositionState::TooClose;
	}
	else if (positionWarning)
	{
		if (playerData.state == eLandingPositionState::Warning)
		{
			const auto distance = (playerData.landingPosition - playerData.lastLandingPosition).l2Norm ();

			if (distance <= tooCloseDistance)
			{
				// the player has chosen the same position after a warning
				// so further warnings will be ignored
				newState = eLandingPositionState::Confirmed;
			}
			else
			{
				newState = eLandingPositionState::Warning;
			}
		}
		else if (playerData.state == eLandingPositionState::Confirmed)
		{
			// player is in state Confirmed,
			// so ignore the warning
			newState = eLandingPositionState::Confirmed;
		}
		else
		{
			newState = eLandingPositionState::Warning;
		}
	}
	else
	{
		if (playerData.state == eLandingPositionState::Confirmed)
		{
			newState = eLandingPositionState::Confirmed;
		}
		else
		{
			newState = eLandingPositionState::Clear;
		}
	}

	std::swap (playerData.state, newState);

	if (playerData.state != newState) landingPositionStateChanged (player, playerData.state);
}

//------------------------------------------------------------------------------
cLandingPositionManager::sLandingPositionData& cLandingPositionManager::getLandingPositionData (const sPlayer& player)
{
	auto iter = std::find_if (landingPositions.begin (), landingPositions.end (), [&](const sLandingPositionData& data){ return data.player->getNr () == player.getNr (); });
	assert (iter != landingPositions.end ());
	return *iter;
}
