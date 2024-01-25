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

#include "game/logic/landingpositionmanager.h"

#include "game/data/player/player.h"
#include "utility/ranges.h"

#include <algorithm>
#include <cassert>

const double cLandingPositionManager::warningDistance = 28;
const double cLandingPositionManager::tooCloseDistance = 10;

//------------------------------------------------------------------------------
cLandingPositionManager::sLandingPositionData::sLandingPositionData (cPlayerBasicData player_) :
	landingPosition (0, 0),
	lastLandingPosition (0, 0),
	state (eLandingPositionState::Unknown),
	player (player_)
{}

//------------------------------------------------------------------------------
cLandingPositionManager::cLandingPositionManager (const std::vector<cPlayerBasicData>& players)
{
	for (size_t i = 0; i < players.size(); ++i)
	{
		landingPositions.push_back (sLandingPositionData (players[i]));
	}
}

//------------------------------------------------------------------------------
bool cLandingPositionManager::setLandingPosition (const cPlayerBasicData& player, const cPosition& landingPosition)
{
	auto& playerData = getLandingPositionData (player);

	playerData.lastLandingPosition = playerData.landingPosition;
	playerData.landingPosition = landingPosition;

	playerData.needNewPosition = false;

	landingPositionSet (player, landingPosition);

	//// wait for all players to choose at least once
	//for (size_t i = 0; i != landingPositions.size(); ++i)
	//{
	//	if (landingPositions[i].needNewPosition) return false;
	//}

	// update player state
	checkPlayerState (playerData, false);

	// update all other states
	for (const auto& landingPos : landingPositions)
	{
		if (landingPos.player.getNr() == player.getNr()) continue;

		auto& otherData = getLandingPositionData (landingPos.player);

		if (otherData.state != eLandingPositionState::Unknown)
		{
			checkPlayerState (otherData, true);
		}
	}

	// test if all states are valid now
	// mark players with invalid states that they need to select a new position.
	bool allValid = true;
	for (auto& landingPos : landingPositions)
	{
		const auto state = landingPos.state;
		if (state == eLandingPositionState::Unknown || state == eLandingPositionState::Warning || state == eLandingPositionState::TooClose)
		{
			landingPos.needNewPosition = true;
		}
		if (landingPos.needNewPosition)
		{
			allValid = false;
		}
	}

	if (allValid)
	{
		allPositionsValid();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void cLandingPositionManager::deleteLandingPosition (const cPlayerBasicData& player)
{
	auto& playerData = getLandingPositionData (player);

	playerData.lastLandingPosition = cPosition (0, 0);
	playerData.landingPosition = cPosition (0, 0);

	playerData.needNewPosition = true;

	playerData.state = eLandingPositionState::Unknown;
}

//------------------------------------------------------------------------------
eLandingPositionState cLandingPositionManager::getPlayerState (const cPlayerBasicData& player) const
{
	const auto& playerData = getLandingPositionData (player);

	return playerData.state;
}

//------------------------------------------------------------------------------
void cLandingPositionManager::checkPlayerState (sLandingPositionData& playerData, bool isOtherPlayer)
{
	bool positionTooClose = false;
	bool positionWarning = false;
	// check distances to all other players
	for (const auto& data : landingPositions)
	{
		if (data.state == eLandingPositionState::Unknown) continue;
		if (data.player.getNr() == playerData.player.getNr()) continue;

		const auto distance = (playerData.landingPosition - data.landingPosition).l2Norm();

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
	auto newState = eLandingPositionState::Unknown;

	if (positionTooClose)
	{
		newState = eLandingPositionState::TooClose;
	}
	else if (positionWarning)
	{
		if (isOtherPlayer && playerData.state == eLandingPositionState::TooClose)
		{
			// Do not assign a 'better' state to an other player
			newState = playerData.state;
		}
		else if (playerData.state == eLandingPositionState::Warning)
		{
			const auto distance = (playerData.landingPosition - playerData.lastLandingPosition).l2Norm();

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
		if (isOtherPlayer && (playerData.state == eLandingPositionState::TooClose || playerData.state == eLandingPositionState::Warning))
		{
			// Do not assign a 'better' state to an other player
			newState = playerData.state;
		}
		else if (playerData.state == eLandingPositionState::Confirmed)
		{
			// if the player has confirmed a warning already we let him stay in the confirmed state
			// so that he does not need to confirm the position again if an other player selects a
			// position that would put this player into a warning state again.
			newState = eLandingPositionState::Confirmed;
		}
		else
		{
			newState = eLandingPositionState::Clear;
		}
	}
	assert (newState != eLandingPositionState::Unknown);

	std::swap (playerData.state, newState);

	if (playerData.state != newState) landingPositionStateChanged (playerData.player, playerData.state);
}

//------------------------------------------------------------------------------
cLandingPositionManager::sLandingPositionData& cLandingPositionManager::getLandingPositionData (const cPlayerBasicData& player)
{
	const cLandingPositionManager& constMe = *this;
	return const_cast<sLandingPositionData&> (constMe.getLandingPositionData (player));
}

//------------------------------------------------------------------------------
const cLandingPositionManager::sLandingPositionData& cLandingPositionManager::getLandingPositionData (const cPlayerBasicData& player) const
{
	auto iter = ranges::find_if (landingPositions, [&] (const sLandingPositionData& data) { return data.player.getNr() == player.getNr(); });
	assert (iter != landingPositions.end());
	return *iter;
}
