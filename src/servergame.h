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
#include "game/data/gamesettings.h"
#include "game/logic/server2.h"
#include "protocol/lobbymessage.h"
#include "ui/graphical/menu/widgets/special/chatboxlandingplayerlistviewitem.h"

class cNetMessage2;
class cPlayer;
class cServer2;
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
	explicit cServerGame (std::shared_ptr<cConnectionManager>);
	virtual ~cServerGame();
	void prepareGameData();
	bool loadGame (int saveGameNumber);
	void saveGame (int saveGameNumber);

	void runInThread();

	void pushMessage (std::unique_ptr<cNetMessage2> message);
	std::unique_ptr<cNetMessage2> popMessage();

	// retrieve state
	std::string getGameState() const;

	std::shared_ptr<cPlayerBasicData> getPlayerForNr (int playerNr) const;

	//------------------------------------------------------------------------
protected:
	cSignalConnectionManager signalConnectionManager;

	std::unique_ptr<cServer2> server;
	cGameSettings settings;
	std::shared_ptr<cStaticMap> map;
	std::shared_ptr<cConnectionManager> connectionManager;
	SDL_Thread* thread;
	bool canceled;
	bool shouldSave;
	int saveGameNumber;

	friend int serverGameThreadFunction (void* data);
	void run();

	void forwardMessage (const cNetMessage2&);

	void sendNetMessage (const cNetMessage2&, int receiverPlayerNr = -1);
	void sendGameData (int playerNr = -1);
	void sendChatMessage (const std::string&, int receiverPlayerNr = -1, int senderPlayerNr = -1);
	void sendTranslatedChatMessage (const std::string&, const std::string& insertText, int receiverPlayerNr = -1, int senderPlayerNr = -1);

	void handleNetMessage (cNetMessage2& message);

	void handleNetMessage (cNetMessageTcpWantConnect&);
	void handleNetMessage (cNetMessageTcpClose&);
	void handleNetMessage (cMultiplayerLobbyMessage&);

	void handleNetMessage (cMuMsgIdentification&);
	void handleNetMessage (cMuMsgChat&);

	void handleNetMessage (cMuMsgRequestMap&);
	void handleNetMessage (cMuMsgFinishedMapDownload&);

	void handleNetMessage (cMuMsgLandingPosition&);
	void handleNetMessage (cMuMsgInLandingPositionSelectionStatus&);
	void handleNetMessage (cMuMsgPlayerAbortedGamePreparations&);

	void terminateServer();

	std::vector<std::shared_ptr<cPlayerBasicData>> menuPlayers;
	std::shared_ptr<cLandingPositionManager> landingPositionManager;
	std::vector<std::unique_ptr<cPlayerLandingStatus>> playersLandingStatus;

	int nextPlayerNumber;

private:
	void configRessources (std::vector<std::string>& tokens, const cPlayerBasicData& senderPlayer);

	//------------------------------------------------------------------------
	cConcurrentQueue<std::unique_ptr<cNetMessage2>> eventQueue;
};

#endif
