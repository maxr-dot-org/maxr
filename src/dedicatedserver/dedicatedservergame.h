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

#ifndef dedicatedserver_dedicatedservergameH
#define dedicatedserver_dedicatedservergameH

#include "game/logic/server.h"
#include "game/startup/lobbyserver.h"
#include "utility/signal/signalconnectionmanager.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <thread>

//------------------------------------------------------------------------
/** cDedicatedServerGame handles all server side tasks of one multiplayer game in a thread.
 */
class cDedicatedServerGame
{
public:
	explicit cDedicatedServerGame (int saveGameNumber);
	~cDedicatedServerGame();

	eOpenServerResult startServer (int port);
	int getPort() const { return port; }

	// all those methods can be called concurrently
	std::string getGameState() const;
	void runInThread();
	void saveGame (int saveGameNumber);

public: // external getter
	std::function<std::string()> getGamesString;
	std::function<std::string()> getAvailableMapsString;

private:
	void handleChatCommand (int fromPlayer, const std::vector<std::string>& tokens);
	void prepareGameData();
	void loadGame (int saveGameNumber);
	void run();

private:
	cSignalConnectionManager signalConnectionManager;

	int port = 0;

	std::atomic<bool> canceled{false};
	std::thread thread;
	mutable std::recursive_mutex mutex;

	// critical data
	cLobbyServer lobbyServer;
	cServer* server = nullptr;
	bool shouldSave = false;
	int saveGameNumber = -1;
};

#endif
