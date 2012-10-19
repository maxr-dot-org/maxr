#include <SDL.h>
#include "gametimer.h"
#include "client.h"
#include "netmessage.h"
#include "files.h"
#include "vehicles.h"
#include "clientevents.h"
#include "events.h"

Uint32 cGameTimer::gameTimerCallback (Uint32 interval, void* arg)
{
	reinterpret_cast<cGameTimer*> (arg)->timerCallback();
	return interval;
}

cGameTimer::cGameTimer(SDL_cond* conditionVariable) :
	mutex ()
{
	gameTime = 0;
	lastTimerCall = 0;
	eventCounter = 0;

	maxEventQueueSize = -1;

	timer10ms = false;
	timer50ms = false;
	timer100ms = false;
	timer400ms = false;

	timerID = 0;
	condSignal = conditionVariable;
}

cGameTimer::~cGameTimer()
{
	stop();
}

void cGameTimer::start ()
{
	if (timerID == 0)
		timerID = SDL_AddTimer (GAME_TICK_TIME, gameTimerCallback, this);
}

void cGameTimer::stop ()
{
	if (timerID != 0)
		SDL_RemoveTimer (timerID);

	timerID = 0;
}

void cGameTimer::timerCallback()
{
	//increase event counter and let the event handler increase the gametime
	{
		cMutex::Lock lock(mutex);

		if (eventCounter < maxEventQueueSize || maxEventQueueSize == -1)
		{
			eventCounter++;
		}
	}

	if ( condSignal )
	{
		SDL_CondSignal (condSignal);
	}
	
}

bool cGameTimer::popEvent ()
{
	cMutex::Lock lock(mutex);

	if (eventCounter > 0)
	{
		eventCounter--;
		return true;
	}
	else
	{
		return false;
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

cGameTimerClient::cGameTimerClient () :
	cGameTimer (NULL),
	remoteChecksum(0),
	localChecksum(0),
	debugRemoteChecksum(0)
{
}

void cGameTimerClient::handleSyncMessage (cNetMessage& message)
{
	assert (message.iType == NET_GAME_TIME_SERVER);

	remoteChecksum = message.popInt32();

	int newSyncTime = message.popInt32();
	if ( newSyncTime != gameTime + 1 )
		Log.write("Game Synchonisation Error: Received out of order sync message", cLog::eLOG_TYPE_NET_ERROR);
}


bool cGameTimerClient::nextTickAllowed()
{
	if (gameTime < getReceivedTime())
	{
		Client->disableFreezeMode (FREEZE_WAIT_FOR_SERVER);
		return true;
	}

	Client->enableFreezeMode (FREEZE_WAIT_FOR_SERVER);
	return false;
}

void cGameTimerClient::run ()
{
	while (popEvent ())
	{
		if (nextTickAllowed ())
		{
			EventHandler->handleNetMessages();
			
			gameTime++;
			handleTimer ();	
			Client->doGameActions();

			//check crc
			localChecksum = calcPlayerChecksum (*Client->getActivePlayer ());
			debugRemoteChecksum = remoteChecksum;
			if ( localChecksum != remoteChecksum )
			{
				//gameGUI.debugOutput.debugSync = true;
				Log.write("OUT OF SYNC", cLog::eLOG_TYPE_NET_ERROR);
			}

			//send "still alive" message to server
			//if (gameTimer.gameTime % (PAUSE_GAME_TIMEOUT/4) == 0)
			{
				cNetMessage *message = new cNetMessage(NET_GAME_TIME_CLIENT);
				message->pushInt32 (gameTime);
				Client->sendNetMessage (message);
			}
		}
	}

	//check whether the client time lags too much behind the server time and add an extra increment of the client time
	if (gameTime + MAX_CLIENT_LAG < getReceivedTime())
	{
		//inject an extra timer event
		timerCallback();		
	}
}

cGameTimerServer::cGameTimerServer (SDL_cond* serverResumeCond) :
	cGameTimer (serverResumeCond),
	waitingForPlayer(-1)
{
}

void cGameTimerServer::handleSyncMessage(cNetMessage &message)
{
	assert (message.iType == NET_GAME_TIME_CLIENT);
	setReceivedTime (message.popInt32(), message.iPlayerNr);

}

bool cGameTimerServer::nextTickAllowed ()
{
	int newWaitingForPlayer = -1;

	for (unsigned int i = 0; i < Server->PlayerList->Size (); i++)
	{
		cPlayer* player = (*Server->PlayerList)[i];
		if (!Server->isPlayerDisconnected(player) && getReceivedTime(i) + PAUSE_GAME_TIMEOUT < gameTime)
			newWaitingForPlayer = player->Nr;
	}

	if (newWaitingForPlayer != -1 && newWaitingForPlayer != waitingForPlayer)
	{
		Server->enableFreezeMode(FREEZE_WAIT_FOR_PLAYER, newWaitingForPlayer);
	}
	else if (newWaitingForPlayer == -1 && waitingForPlayer != -1)
	{
		Server->disableFreezeMode(FREEZE_WAIT_FOR_PLAYER);
	}

	waitingForPlayer = newWaitingForPlayer;

	return waitingForPlayer == -1;
}

void cGameTimerServer::run ()
{
	//TODO:loop?
	if (popEvent ())
	{
		if (nextTickAllowed ())
		{
			gameTime++;
		}
	}

	handleTimer ();
	Server->doGameActions ();
		
	if (timer10ms)
	{
		for (size_t i = 0; i < Server->PlayerList->Size(); i++)
		{
			const cPlayer &player = *(*Server->PlayerList)[i];

			cNetMessage *message = new cNetMessage(NET_GAME_TIME_SERVER);
			message->pushInt32 (gameTime);	
			Uint32 checkSum = calcPlayerChecksum (player);
			message->pushInt32 (checkSum);
			Server->sendNetMessage (message, player.Nr);
		}
	}
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
