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

#include <string>

enum class ePlayerConnectionState
{
	INACTIVE,       // player is not connected, but game can continue (e. g. defeated player)
	CONNECTED,      // player is connected. Normal operation.
	NOT_RESPONDING, // player is connected, but no sync message received for some time. Game should be paused.
	DISCONNECTED    // player has lost connection. Game should be paused.
};
std::string enumToString (ePlayerConnectionState value);

enum class eFreezeMode
{
	WAIT_FOR_TURNEND,   // server is processing the turn end
	PAUSE,              // pause, because... pause
	WAIT_FOR_CLIENT,    // waiting for response from client
	WAIT_FOR_SERVER     // waiting for response from server
};
std::string enumToString (eFreezeMode value);

class cFreezeModes
{
public:
	cFreezeModes() = default;

	void disable (eFreezeMode);
	void enable (eFreezeMode);
	bool isEnabled (eFreezeMode) const;

	bool isFreezed() const;
	bool gameTimePaused() const;

	template <typename T>
	void serialize (T& archive)
	{
		archive & waitForTurnEnd;
		archive & pause;
		archive & waitForClient;
		archive & waitForServer;
	}

	// These modes are triggered on server (and synchronized to clients):
	bool waitForTurnEnd = false; // server is processing the turn end
	bool pause = false;          // pause, because... pause
	bool waitForClient = false;  // waiting for response from client

	// This mode is triggered on client (and not synchronized with server or other clients):
	bool waitForServer = false; // waiting for response from server
};

#endif
