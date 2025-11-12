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

#ifndef game_data_freezemodeH
#define game_data_freezemodeH

#include "utility/serialization/serialization.h"

#include <utility>
#include <vector>

enum class ePlayerConnectionState
{
	Inactive, // player is not connected, but game can continue (e.g. defeated player)
	Connected, // player is connected. Normal operation.
	NotResponding, // player is connected, but no sync message received for some time. Game should be paused.
	Disconnected // player has lost connection. Game should be paused.
};

enum class eFreezeMode
{
	WaitForTurnend, // server is processing the turn end
	Pause, // pause, because... pause
	WaitForClient, // waiting for response from client
	WaitForServer // waiting for response from server
};

namespace serialization
{
	template <>
	struct sEnumStringMapping<ePlayerConnectionState>
	{
		static const std::vector<std::pair<ePlayerConnectionState, const char*>> m;
	};
	template <>
	struct sEnumStringMapping<eFreezeMode>
	{
		static const std::vector<std::pair<eFreezeMode, const char*>> m;
	};
} // namespace serialization
class cFreezeModes
{
public:
	cFreezeModes() = default;

	void disable (eFreezeMode);
	void enable (eFreezeMode);
	bool isEnabled (eFreezeMode) const;

	bool isFrozen() const;
	bool gameTimePaused() const;

	template <ArchiveInOrOut Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (waitForTurnEnd);
		archive & NVP (pause);
		archive & NVP (waitForClient);
		archive & NVP (waitForServer);
		// clang-format on
	}

	// These modes are triggered on server (and synchronized to clients):
	bool waitForTurnEnd = false; // server is processing the turn end
	bool pause = false; // pause, because... pause
	bool waitForClient = false; // waiting for response from client

	// This mode is triggered on client (and not synchronized with server or other clients):
	bool waitForServer = false; // waiting for response from server
};

#endif
