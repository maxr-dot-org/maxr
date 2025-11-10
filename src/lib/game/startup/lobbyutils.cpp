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

#include "lobbyutils.h"

#include "game/data/player/playerbasicdata.h"

#include <algorithm>

namespace
{
	//--------------------------------------------------------------------------
	bool sameColor (const cRgbColor& lhs, const cRgbColor& rhs)
	{
		constexpr double colorDeltaETolerance = 10.;

		return lhs.toLab().deltaE (rhs.toLab()) < colorDeltaETolerance;
	}
} // namespace

//------------------------------------------------------------------------------
eLobbyPlayerStatus checkTakenPlayerAttributes (const std::vector<cPlayerBasicData>& players, const cPlayerBasicData& player)
{
	if (!player.isReady()) return eLobbyPlayerStatus::Ok;

	if (std::ranges::find_if (players, [&] (const auto& p) { return player.getNr() != p.getNr() && player.getName() == p.getName(); }) != players.end())
	{
		return eLobbyPlayerStatus::DuplicatedName;
	}
	if (std::ranges::find_if (players, [&] (const auto& p) { return player.getNr() != p.getNr() && sameColor (player.getColor(), p.getColor()); }) != players.end())
	{
		return eLobbyPlayerStatus::DuplicatedColor;
	}
	return eLobbyPlayerStatus::Ok;
}
