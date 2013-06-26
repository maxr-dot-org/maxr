#ifndef SIMSPEEDCTRL_H
#define SIMSPEEDCTRL_H


#include "cmutex.h"
#include <vector>
#include <SDL.h>

class cClient;
class cMenu;
class cNetMessage;
class cPlayer;
class cServer;

#define GAME_TICK_TIME 10
#define MAX_CLIENT_LAG 15
#define PAUSE_GAME_TIMEOUT 200
#define MAX_SERVER_EVENT_COUNTER 15
#define MAX_WAITING_FOR_SERVER 50

//TODO: comments
class cGameTimer
{
	friend class cDebugOutput;
protected:
	static Uint32 gameTimerCallback (Uint32 interval, void* arg);

	cGameTimer ();

	/** SDL_Timer that controls the game time */
	SDL_TimerID timerID;
	cMutex mutex;
	int eventCounter;
	unsigned int lastTimerCall;
	std::vector<unsigned int> receivedTime;

	void timerCallback ();
	void handleTimer ();
	bool popEvent ();

public:
	~cGameTimer ();

	int maxEventQueueSize;

	unsigned int gameTime;

	bool timer10ms;
	bool timer50ms;
	bool timer100ms;
	bool timer400ms;

	void setReceivedTime (unsigned int time, unsigned int nr = 0);
	unsigned int getReceivedTime (unsigned int nr = 0);

	void start ();
	void stop ();

	static bool syncDebugSingleStep;
};

class cGameTimerServer : public cGameTimer
{
	friend class cDebugOutput;
private:
	int waitingForPlayer;

	bool nextTickAllowed (cServer& server);
public:
	cGameTimerServer ();

	void run (cServer& server);
	void handleSyncMessage (cNetMessage& message);

};

class cGameTimerClient : public cGameTimer
{
	friend class cDebugOutput;
private:
	cClient* client;
	unsigned int remoteChecksum;
	unsigned int localChecksum;
	unsigned int waitingForServer;
	unsigned int debugRemoteChecksum;	//saved data for debug view only
	int gameTimeAdjustment;				//saved data for debug view only

	bool nextTickAllowed ();
public:
	bool nextMsgIsNextGameTime;
	cGameTimerClient ();
	void setClient (cClient* client);

	void run (cMenu* activeMenu);
	void handleSyncMessage (cNetMessage& message);
};

Uint32 calcClientChecksum (const cClient& client);
Uint32 calcServerChecksum (const cServer& server, const cPlayer* player);

void compareGameData (const cClient& client, const cServer& server);

#endif
