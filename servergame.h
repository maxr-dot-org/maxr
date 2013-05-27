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
#include "ringbuffer.h"

class cGameDataContainer;
class cMap;
class cNetMessage;
class cPlayer;
class cServer;
class cTCP;
struct sMenuPlayer;

int serverGameThreadFunction (void* data);

//------------------------------------------------------------------------
/** cServerGame handles all server side tasks of one multiplayer game in a thread.
 *  It is possible (in the future) to run several cServerGames in parallel.
 *  Each cServerGame has (in the future) its own queue of network events.
 */
class cServerGame
{
public:
	explicit cServerGame (cTCP& network_);
	virtual ~cServerGame();
	void prepareGameData();
	bool loadGame (int saveGameNumber);
	void saveGame (int saveGameNumber);

	void runInThread();

	void pushEvent (cNetMessage* message);

	// retrieve state
	std::string getGameState() const;

	int getSocketForPlayerNr (int playerNr) const;

	//------------------------------------------------------------------------
protected:
	cServer* server;
	cTCP* network;
	SDL_Thread* thread;
	bool canceled;
	bool shouldSave;
	int saveGameNumber;

	friend int serverGameThreadFunction (void* data);
	void run();
	cNetMessage* pollEvent();
	void handleNetMessage (cNetMessage* message);

	void handleNetMessage_TCP_ACCEPT (cNetMessage* message);
	void handleNetMessage_TCP_CLOSE (cNetMessage* message);
	void handleNetMessage_MU_MSG_IDENTIFIKATION (cNetMessage* message);
	void handleNetMessage_MU_MSG_CHAT (cNetMessage* message);
	void handleNetMessage_MU_MSG_CLAN (cNetMessage* message);
	void handleNetMessage_MU_MSG_LANDING_VEHICLES (cNetMessage* message);
	void handleNetMessage_MU_MSG_UPGRADES (cNetMessage* message);
	void handleNetMessage_MU_MSG_LANDING_COORDS (cNetMessage* message);

	void startGameServer();
	void terminateServer();

	cGameDataContainer* gameData;
	std::vector<sMenuPlayer*> menuPlayers;

	cMap* serverMap;
	std::vector<cPlayer*> serverPlayers;

private:
	void configRessources (std::vector<std::string>& tokens, sMenuPlayer* senderPlayer);

	//------------------------------------------------------------------------
	cRingbuffer<cNetMessage*> eventQueue;
};

#endif
