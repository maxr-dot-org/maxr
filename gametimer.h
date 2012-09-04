#ifndef SIMSPEEDCTRL_H
#define SIMSPEEDCTRL_H


#include "clist.h"
#include "player.h"

class cNetMessage;

#define GAME_TICK_TIME 10
#define MAX_CLIENT_LAG 30
#define PAUSE_GAME_TIMEOUT 1000

class cGameTimer
{
public:
	cGameTimer(SDL_cond* conditionVariable = NULL );
	~cGameTimer();

	/** SDL_Timer that controls the game time */
	SDL_TimerID TimerID;

	cMutex mutex;


	unsigned int gameTime;
	cList<unsigned int> receivedTime;	//private

	unsigned int lastTimerCall;
	
	void timerCallback();
	void handleTimer();

	bool timer10ms;
	bool timer50ms;
	bool timer100ms;
	bool timer400ms;

	unsigned int localChecksum;
	unsigned int remoteChecksum;

	void setReceivedTime(unsigned int time, unsigned int nr = 0);
	unsigned int getReceivedTime(unsigned int nr = 0);

	//client only
	bool currentTickFinished;
	void HandleNetMessage_NET_GAME_TIME_SERVER (cNetMessage& message);
	/** checks, whether the client has finished the current turn, and has an client time < servertime */
	bool nextTickAllowed();
	unsigned int lastSyncMessage;
	

	//server only
	SDL_cond* condSignal;
	bool flag;

	//saved data for debug only
	unsigned int debugRemoteChecksum;

};


int calcPlayerChecksum(const cPlayer& player);

#endif