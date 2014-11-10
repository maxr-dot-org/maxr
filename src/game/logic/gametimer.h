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

#ifndef game_logic_gametimerH
#define game_logic_gametimerH

#include <vector>
#include <SDL.h>

#include "utility/thread/mutex.h"
#include "utility/signal/signal.h"


class cClient;
class cNetMessage;
class cPlayer;
class cServer;

#define GAME_TICK_TIME 10				/** Number of milliseconds of one game time tick */
#define MAX_CLIENT_LAG 15				/** Maximum ticks, the clients time is allowed to be behind 
										    the server time */
#define PAUSE_GAME_TIMEOUT 200			/** Number of ticks, after which the server pauses the game,
										    when no sync message from a client is received */
#define MAX_SERVER_EVENT_COUNTER 15		/** When the server is under heavy load, ticks are discarded, 
											when there are more than MAX_SERVER_EVENT_COUNTER timer events pending */
#define MAX_WAITING_FOR_SERVER 50		/** The client will display the "Waiting for Server message" when no sync messages are
											received for MAX_WAITING_FOR_SERVER ticks */

class cGameTimer
{
	friend class cDebugOutputWidget;
protected:
	static Uint32 gameTimerCallback (Uint32 interval, void* arg);

	cGameTimer();

	/** SDL_Timer that controls the game time */
	SDL_TimerID timerID;
	cMutex mutex;
	int eventCounter;
	unsigned int lastTimerCall;
	std::vector<unsigned int> receivedTime;

	void timerCallback();
	void handleTimer();
	bool popEvent();
public:
	~cGameTimer();

	int maxEventQueueSize;

	unsigned int gameTime;

	bool timer10ms;
	bool timer50ms;
	bool timer100ms;

	void setReceivedTime (unsigned int time, unsigned int nr = 0);
	unsigned int getReceivedTime (unsigned int nr = 0);

	void start();
	void stop();

	static bool syncDebugSingleStep;

	cSignal<void ()> gameTimeChanged;
};

class cGameTimerServer : public cGameTimer
{
	friend class cDebugOutputWidget;
private:
	int waitingForPlayer;

	bool nextTickAllowed (cServer& server);
public:
	cGameTimerServer();

	void run (cServer& server);
	void handleSyncMessage (cNetMessage& message);

};

class cGameTimerClient : public cGameTimer
{
	friend class cDebugOutputWidget;
private:
	cClient* client;
	unsigned int remoteChecksum;
	unsigned int localChecksum;
	unsigned int waitingForServer;
	unsigned int debugRemoteChecksum;	//saved data for debug view only
	int gameTimeAdjustment;				//saved data for debug view only

	bool nextTickAllowed();
public:
	bool nextMsgIsNextGameTime;
	cGameTimerClient();
	void setClient (cClient* client);

	void run ();
	void handleSyncMessage (cNetMessage& message);
};

uint32_t calcClientChecksum (const cClient& client);
uint32_t calcServerChecksum (const cServer& server, const cPlayer* player);

void compareGameData (const cClient& client, const cServer& server);

#endif // game_logic_gametimerH
