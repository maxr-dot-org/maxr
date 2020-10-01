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

#ifndef game_servergameH
#define game_servergameH

#include "game/data/gamesettings.h"
#include "game/logic/server.h"
#include "game/startup/lobbyserver.h"
#include "protocol/lobbymessage.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/thread/concurrentqueue.h"

#include <memory>
#include <string>
#include <vector>

#include <SDL.h>

class cLandingPositionManager;
class cNetMessage;
class cPlayer;
class cPlayerBasicData;
class cServer;
class cStaticMap;

int serverGameThreadFunction (void* data);

//------------------------------------------------------------------------
/** cServerGame handles all server side tasks of one multiplayer game in a thread.
 *  It is possible (in the future) to run several cServerGames in parallel.
 *  Each cServerGame has (in the future) its own queue of network events.
 */
class cServerGame
{
public:
	explicit cServerGame (std::shared_ptr<cConnectionManager>);
	~cServerGame();

	void runInThread();

	void pushMessage (std::unique_ptr<cNetMessage>);
	std::unique_ptr<cNetMessage> popMessage();

	void prepareGameData();
	bool loadGame (int saveGameNumber);
	void saveGame (int saveGameNumber);

	// retrieve state
	std::string getGameState() const;

	std::function<std::string()> getGamesString;
	std::function<std::string()> getAvailableMapsString;

	void handleChatCommand (int fromPlayer, const std::vector<std::string>& tokens);
private:
	friend int serverGameThreadFunction (void* data);
	void run();

private:
	cSignalConnectionManager signalConnectionManager;
	cLobbyServer lobbyServer;

	std::unique_ptr<cServer> server;

	bool shouldSave = false;
	int saveGameNumber = -1;

	SDL_Thread* thread = nullptr;
	bool canceled = false;
};

#endif
