#include <SDL.h>
#include "gametimer.h"
#include "client.h"
#include "netmessage.h"
#include "files.h"
#include "vehicles.h"
#include "clientevents.h"
#include "events.h"

bool cGameTimer::syncDebugSingleStep = false;

Uint32 cGameTimer::gameTimerCallback (Uint32 interval, void* arg)
{
	reinterpret_cast<cGameTimer*> (arg)->timerCallback();
	return interval;
}

cGameTimer::cGameTimer() :
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
	eventCounter = 0;
}

void cGameTimer::timerCallback()
{
	//increase event counter and let the event handler increase the gametime
	cMutex::Lock lock (mutex);

	if (eventCounter < maxEventQueueSize || maxEventQueueSize == -1)
	{
		eventCounter++;
	}
}

bool cGameTimer::popEvent ()
{
	cMutex::Lock lock (mutex);

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


void cGameTimer::setReceivedTime (unsigned int time, unsigned int nr)
{
	cMutex::Lock lock (mutex);

	while (receivedTime.Size() <= nr)
		receivedTime.Add (0);

	receivedTime[nr] = time;
}

unsigned int cGameTimer::getReceivedTime (unsigned int nr)
{
	cMutex::Lock lock (mutex);

	if (receivedTime.Size() <= nr)
		return 0;

	return receivedTime[nr];
}

cGameTimerClient::cGameTimerClient () :
	cGameTimer (),
	remoteChecksum (0),
	localChecksum (0),
	waitingForServer (0),
	debugRemoteChecksum (0),
	gameTimeAdjustment (0),
	nextMsgIsNextGameTime (false)
{
}

void cGameTimerClient::handleSyncMessage (cNetMessage& message)
{
	assert (message.iType == NET_GAME_TIME_SERVER);

	remoteChecksum = message.popInt32();

	const unsigned int newSyncTime = message.popInt32();
	if (newSyncTime != gameTime + 1)
		Log.write ("Game Synchonisation Error: Received out of order sync message", cLog::eLOG_TYPE_NET_ERROR);

	nextMsgIsNextGameTime = true;
}


bool cGameTimerClient::nextTickAllowed()
{
	if (nextMsgIsNextGameTime)
	{
		Client->disableFreezeMode (FREEZE_WAIT_FOR_SERVER);
		waitingForServer = 0;
		return true;
	}

	gameTimeAdjustment--;

	waitingForServer++;
	if (waitingForServer > MAX_WAITING_FOR_SERVER)
	{
		Client->enableFreezeMode (FREEZE_WAIT_FOR_SERVER);
	}
	return false;
}

void cGameTimerClient::run ()
{
	while (popEvent ())
	{
		EventHandler->handleNetMessages();

		if (nextTickAllowed ())
		{
			gameTime++;
			handleTimer ();
			Client->doGameActions();

			//check crc
			localChecksum = calcClientChecksum();
			debugRemoteChecksum = remoteChecksum;
			if (localChecksum != remoteChecksum)
			{
				//gameGUI.debugOutput.debugSync = true;
				//Log.write ("OUT OF SYNC", cLog::eLOG_TYPE_NET_ERROR);
			}

			if (syncDebugSingleStep)
				compareGameData();

			nextMsgIsNextGameTime = false;

			//send "still alive" message to server
			//if (gameTime % (PAUSE_GAME_TIMEOUT/10) == 0)
			{
				cNetMessage* message = new cNetMessage (NET_GAME_TIME_CLIENT);
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
		gameTimeAdjustment++;
	}
}

cGameTimerServer::cGameTimerServer () :
	waitingForPlayer (-1)
{
}

void cGameTimerServer::handleSyncMessage (cNetMessage& message)
{
	assert (message.iType == NET_GAME_TIME_CLIENT);
	setReceivedTime (message.popInt32(), message.iPlayerNr);

}

bool cGameTimerServer::nextTickAllowed ()
{
	if (syncDebugSingleStep)
	{
		if (getReceivedTime (0) < gameTime)
			return false;

		return true;
	}

	int newWaitingForPlayer = -1;

	for (unsigned int i = 0; i < Server->PlayerList->Size (); i++)
	{
		cPlayer* player = (*Server->PlayerList) [i];
		if (!Server->isPlayerDisconnected (player) && getReceivedTime (i) + PAUSE_GAME_TIMEOUT < gameTime)
			newWaitingForPlayer = player->Nr;
	}

	if (newWaitingForPlayer != -1 && newWaitingForPlayer != waitingForPlayer)
	{
		Server->enableFreezeMode (FREEZE_WAIT_FOR_PLAYER, newWaitingForPlayer);	//TODO: betreffenden player nicht mit freezenachrichten zuballern. Das ist kontraproduktiv.
	}
	else if (newWaitingForPlayer == -1 && waitingForPlayer != -1)
	{
		Server->disableFreezeMode (FREEZE_WAIT_FOR_PLAYER);
	}

	waitingForPlayer = newWaitingForPlayer;

	return waitingForPlayer == -1;
}

void cGameTimerServer::run ()
{
	if (popEvent ())
	{
		if (nextTickAllowed ())
		{
			gameTime++;
			handleTimer ();
			Server->doGameActions ();

			for (size_t i = 0; i < Server->PlayerList->Size(); i++)
			{
				cPlayer* player = (*Server->PlayerList) [i];

				cNetMessage* message = new cNetMessage (NET_GAME_TIME_SERVER);
				message->pushInt32 (gameTime);
				Uint32 checkSum = calcServerChecksum (player);
				message->pushInt32 (checkSum);
				Server->sendNetMessage (message, player->Nr);
			}

		}
	}
}

Uint32 calcClientChecksum()
{
	Uint32 crc = 0;
	for (unsigned int i = 0; i < Client->getPlayerList()->Size(); i++)
	{
		cVehicle* vehicle = (*Client->getPlayerList()) [i]->VehicleList;
		while (vehicle)
		{
			crc = calcCheckSum (vehicle->iID,  crc);
			crc = calcCheckSum (vehicle->PosX, crc);
			crc = calcCheckSum (vehicle->PosY, crc);
			crc = calcCheckSum (vehicle->OffX, crc);
			crc = calcCheckSum (vehicle->OffY, crc);
			crc = calcCheckSum (vehicle->dir,  crc);

			vehicle = (cVehicle*) vehicle->next;
		}
	}
	return crc;
}

Uint32 calcServerChecksum (cPlayer* player)
{
	Uint32 crc = 0;
	for (unsigned int i = 0; i < Server->PlayerList->Size(); i++)
	{
		cVehicle* vehicle = (*Server->PlayerList) [i]->VehicleList;
		while (vehicle)
		{
			if (vehicle->seenByPlayerList.Contains (player) || vehicle->owner == player)
			{
				crc = calcCheckSum (vehicle->iID,  crc);
				crc = calcCheckSum (vehicle->PosX, crc);
				crc = calcCheckSum (vehicle->PosY, crc);
				crc = calcCheckSum (vehicle->OffX, crc);
				crc = calcCheckSum (vehicle->OffY, crc);
				crc = calcCheckSum (vehicle->dir,  crc);
			}

			vehicle = (cVehicle*) vehicle->next;
		}
	}
	return crc;
}

void compareGameData()
{
	for (unsigned int i = 0; i < Client->getPlayerList()->Size(); i++)
	{
		cPlayer* clientPlayer = (*Client->getPlayerList()) [i];

		cVehicle* clientVehicle = clientPlayer->VehicleList;
		cVehicle* serverVehicle;
		while (clientVehicle)
		{
			serverVehicle = (cVehicle*) Server->getUnitFromID (clientVehicle->iID);

			assert (clientVehicle->PosX == serverVehicle->PosX);
			assert (clientVehicle->PosY == serverVehicle->PosY);
			assert (clientVehicle->OffX == serverVehicle->OffX);
			assert (clientVehicle->OffY == serverVehicle->OffY);
			assert (clientVehicle->dir == serverVehicle->dir);

			clientVehicle = (cVehicle*) clientVehicle->next;
		}
	}
}
