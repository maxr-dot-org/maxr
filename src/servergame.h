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

#ifndef ServerGame_H
#define ServerGame_H

#include <SDL.h>
#include <string>
#include <vector>
#include <memory>
#include "utility/thread/concurrentqueue.h"
#include "utility/signal/signalconnectionmanager.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"

class cNetMessage;
class cPlayer;
class cServer;
class cTCP;
class cPlayerBasicData;
class cStaticMap;
class cLandingPositionManager;

int serverGameThreadFunction (void* data);

//------------------------------------------------------------------------
/** cServerGame handles all server side tasks of one multiplayer game in a thread.
 *  It is possible (in the future) to run several cServerGames in parallel.
 *  Each cServerGame has (in the future) its own queue of network events.
 */
class cServerGame
{
public:
	explicit cServerGame (std::shared_ptr<cTCP> network);
	virtual ~cServerGame();
	void prepareGameData();
	bool loadGame (int saveGameNumber);
	void saveGame (int saveGameNumber);

	void runInThread();

	void pushEvent (std::unique_ptr<cNetMessage> message);

	// retrieve state
	std::string getGameState() const;

	int getSocketForPlayerNr (int playerNr) const;

	//------------------------------------------------------------------------
protected:
	cSignalConnectionManager signalConnectionManager;

	std::unique_ptr<cServer> server;
	cGameSettings settings;
	std::shared_ptr<cStaticMap> map;
	std::shared_ptr<cTCP> network;
	SDL_Thread* thread;
	bool canceled;
	bool shouldSave;
	int saveGameNumber;

	friend int serverGameThreadFunction (void* data);
	void run();
	void handleNetMessage (cNetMessage& message);

	void handleNetMessage_TCP_ACCEPT (cNetMessage& message);
	void handleNetMessage_TCP_CLOSE (cNetMessage& message);
	void handleNetMessage_MU_MSG_IDENTIFIKATION (cNetMessage& message);
	void handleNetMessage_MU_MSG_CHAT (cNetMessage& message);
	void handleNetMessage_MU_MSG_LANDING_POSITION (cNetMessage& message);

	void terminateServer();

	std::vector<std::shared_ptr<cPlayerBasicData>> menuPlayers;
	std::shared_ptr<cLandingPositionManager> landingPositionManager;

	int nextPlayerNumber;

private:
	void configRessources (std::vector<std::string>& tokens, cPlayerBasicData* senderPlayer);

	//------------------------------------------------------------------------
	cConcurrentQueue<std::unique_ptr<cNetMessage>> eventQueue;
};

#endif
