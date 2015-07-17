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
class cServer2;
class cNetMessageSyncServer;
class cNetMessageSyncClient;
class cModel;
class cPlayer;

#define GAME_TICK_TIME 10				/** Number of milliseconds of one game time tick */
#define MAX_CLIENT_LAG 15				/** Maximum ticks, the clients time is allowed to be behind 
										    the server time */
#define PAUSE_GAME_TIMEOUT 200			/** Number of ticks, after which the server pauses the game,
										    when no sync message from a client is received */
#define MAX_SERVER_EVENT_COUNTER 15		/** When the server is under heavy load, ticks are discarded, 
											when there are more than MAX_SERVER_EVENT_COUNTER timer events pending */
#define MAX_WAITING_FOR_SERVER 50		/** The client will display the "Waiting for Server message" when no sync messages are
											received for MAX_WAITING_FOR_SERVER ticks */

struct sGameTimerClientDebugData
{
	bool crcOK;
	float timeBuffer;
	float ticksPerFrame;
	float queueSize;
	float eventCounter;
	float ping;
};

class cGameTimer
{
	friend class cDebugOutputWidget;
protected:
	static Uint32 gameTimerCallback (Uint32 interval, void* arg);

	cGameTimer();

	/** SDL_Timer that controls the game time */
	SDL_TimerID timerID;
	cMutex mutex;
	unsigned int eventCounter;

	void timerCallback();
	void handleTimer(); 
	bool popEvent();
public:
	~cGameTimer();

	unsigned int maxEventQueueSize;

	unsigned int gameTime;

	bool timer50ms;
	bool timer100ms;
	
	void start();
	void stop();

	cSignal<void ()> gameTimeChanged;
};

class cGameTimerServer : public cGameTimer
{
	friend class cDebugOutputWidget;
private:
	void checkPlayersResponding(const std::vector<std::shared_ptr<cPlayer>>& playerList, cServer2& server);
	std::vector<sGameTimerClientDebugData> clientDebugData;
	std::vector<unsigned int> receivedTime;
public:
	void run (cModel& model, cServer2& server);
	void handleSyncMessage (const cNetMessageSyncClient& message);
	void setNumberOfPlayers(unsigned int players);

};

class cGameTimerClient : public cGameTimer
{
	friend class cDebugOutputWidget;
private:
	unsigned int receivedTime;			//gametime of the latest sync message in the netmessage queue
	unsigned int remoteChecksum;		//received checksum from server. After running the jobs for the next gematime, the clientmodel should have the same checksum!
	unsigned int timeSinceLastSyncMessage; //when no sync message is received for a certain time, user gets message "waiting for server" 

	bool syncMessageReceived; // The gametime can only be increased, after the sync message for the next gametime from the server has been received.
							  // After the sync message has been received, all following netmessages belong to the next gametime step. So handling of
							  // messages is stoped, until client reached the next gametime.


	unsigned int localChecksum;			// saved local checksum for debug view
	unsigned int debugRemoteChecksum;	// saved data for debug view only
	unsigned int ping;                  // saved data for debug view only

	void checkServerResponding(cClient& client);
public:
	cGameTimerClient();

	void setReceivedTime(unsigned int time);
	unsigned int getReceivedTime();

	void run(cClient& client);
	void handleSyncMessage (cNetMessageSyncServer& message);
};


#endif // game_logic_gametimerH
