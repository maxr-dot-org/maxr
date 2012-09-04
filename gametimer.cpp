#include <SDL.h>
#include "gametimer.h"
#include "client.h"
#include "netmessage.h"
#include "files.h"
#include "vehicles.h"
#include "clientevents.h"
#include "events.h"

Uint32 gameTimerCallback (Uint32 interval, void* arg)
{
	reinterpret_cast<cGameTimer*> (arg)->timerCallback();
	return interval;
}

cGameTimer::cGameTimer(SDL_cond* conditionVariable)
{
	gameTime = 0;
	lastTimerCall = 0;
	lastSyncMessage = 0;

	timer10ms = false;
	timer50ms = false;
	timer100ms = false;
	timer400ms = false;

	localChecksum = 0;
	remoteChecksum = 0;
	debugRemoteChecksum = 0;
	currentTickFinished = false;

	TimerID = SDL_AddTimer (GAME_TICK_TIME, gameTimerCallback, this);
	condSignal = conditionVariable;
}

cGameTimer::~cGameTimer()
{
	SDL_RemoveTimer (TimerID);
}

void cGameTimer::timerCallback()
{
	if ( condSignal )
	{
		//set flag, and signal thread to increase the timer
		//FIXME: Racecondition: Timer anhalten, bevor condVariable gelöscht wird in ~cServer
		flag = true;
		SDL_CondSignal( condSignal );
	}
	else
	{
		//push event to the event queue and let the event handler increase the timer
		SDL_Event userevent;
	 
		userevent.type = SDL_USEREVENT;
		userevent.user.code = USER_EV_GAME_TIME_TICK;
		userevent.user.data1 = NULL;
		userevent.user.data2 = NULL;
	 
		SDL_PushEvent (&userevent);

	}
}

void cGameTimer::handleTimer()
{
	timer10ms  = false;
	timer50ms  = false;
	timer100ms = false;
	timer400ms = false;
	if (gameTime != lastTimerCall)
	{
		lastTimerCall = gameTime;
		timer10ms  = true;
		if (gameTime %  5 == 0) timer50ms  = true;
		if (gameTime % 10 == 0) timer100ms = true;
		if (gameTime % 40 == 0) timer400ms = true;
	}
}

void cGameTimer::HandleNetMessage_NET_GAME_TIME_SERVER (cNetMessage& message)
{
	assert (message.iType == NET_GAME_TIME_SERVER);

	remoteChecksum = message.popInt32();

	int newSyncTime = message.popInt32();
	if ( newSyncTime != lastSyncMessage + 1 )
		Log.write("Game Synchonisation Error: Received out of order sync message", cLog::eLOG_TYPE_NET_ERROR);
	lastSyncMessage = newSyncTime;

	currentTickFinished = true;
}

void cGameTimer::setReceivedTime(unsigned int time, unsigned int nr)
{
	cMutex::Lock lock(mutex);

	while (receivedTime.Size() <= nr)
		receivedTime.Add(0);

	receivedTime[nr] = time;
}

unsigned int cGameTimer::getReceivedTime(unsigned int nr)
{
	cMutex::Lock lock(mutex);

	if (receivedTime.Size() <= nr)
		return 0;

	return receivedTime[nr];
}

bool cGameTimer::nextTickAllowed()
{
	if (!currentTickFinished)
		return false;

	return gameTime < getReceivedTime();
}

Sint32 calcPlayerChecksum(const cPlayer& player)
{
	Uint32 crc = 0;
	cVehicle* vehicle = player.VehicleList;
	while (vehicle)
	{
		crc = calcCheckSum(vehicle->iID,  crc );
		crc = calcCheckSum(vehicle->PosX, crc );
		crc = calcCheckSum(vehicle->PosY, crc );
		crc = calcCheckSum(vehicle->OffX, crc );
		crc = calcCheckSum(vehicle->OffY, crc );
		crc = calcCheckSum(vehicle->dir,  crc );

		vehicle = (cVehicle*) vehicle->next;
	}
	return crc;
}
