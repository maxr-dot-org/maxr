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

#ifndef game_logic_serverH
#define game_logic_serverH

#include "game/connectionmanager.h"
#include "game/data/model.h"
#include "game/data/savegame.h"
#include "game/logic/gametimer.h"
#include "game/protocol/netmessage.h"
#include "utility/thread/concurrentqueue.h"

#include <SDL_thread.h>
#include <memory>
#include <string>

class cConnectionManager;
class cPlayerBasicData;

struct sLobbyPreparationData;

class cServer : public INetMessageReceiver
{
	friend class cDebugOutputWidget;

public:
	explicit cServer (std::shared_ptr<cConnectionManager>);
	~cServer();

	void pushMessage (std::unique_ptr<cNetMessage>);

	/**
	* Send a serialized copy of the message to the specified player(s).
	*/
	void sendMessageToClients (const cNetMessage&, int playerNr = -1) const;

	void start();
	void stop();

	std::string getGameState() const;

	void setPreparationData (const sLobbyPreparationData&);
	void setPlayers (const std::vector<cPlayerBasicData>& splayers);

	const cModel& getModel() const;
	void saveGameState (int saveGameNumber, const std::string& saveName) const;
	void loadGameState (int saveGameNumber);
	void sendGuiInfoToClients (int saveGameNumber, int playerNr = -1);

	void resyncClientModel (int playerNr = -1) const;
	void enableFreezeMode (eFreezeMode mode);
	void disableFreezeMode (eFreezeMode mode);

	/**
	* Set player state to NOT_RESPONDING. Freeze mode WAIT_FOR_CLIENT will
	* be enabled if necessary.
	*/
	void setPlayerNotResponding (int playerId);

	/**
	* Set player state to CONNECTED. Freeze mode WAIT_FOR_CLIENT will
	* be disabled if possible.
	*/
	void clearPlayerNotResponding (int playerId);

private:
	void initRandomGenerator();
	/**
	* Update the player connection state and halt game if necessary.
	*/
	void playerConnected (int playerId);

	/**
	* Update the player connection state and resume game if possible.
	*/
	void playerDisconnected (int playerId);

	/**
	* Enables or disables the WaitForClient freeze mode after player connection state has changed.
	*/
	void updateWaitForClientFlag();

	/**
	* Starts or stops the gametimer after freezemodes have changed
	*/
	void updateGameTimerstate();

	/**
	* sets the initial player connection states after starting or loading a game
	*/
	void initPlayerConnectionState();

	// manage the server thread
	static int serverThreadCallback (void* arg);
	void run();

private:
	cModel model;

	std::map<int, ePlayerConnectionState> playerConnectionStates;
	cFreezeModes freezeModes;
	cGameTimerServer gameTimer;

	std::shared_ptr<cConnectionManager> connectionManager;
	cConcurrentQueue<std::unique_ptr<cNetMessage>> eventQueue;

	mutable int savingID = -1; //identifier number, to make sure the gui info from clients are written to the correct save file

	mutable SDL_Thread* serverThread = nullptr;
	mutable bool exit = false;
};

#endif
