#include <SDL.h>
#include "gametimer.h"

#include "client.h"
#include "clientevents.h"
#include "clist.h"
#include "events.h"
#include "files.h"
#include "log.h"
#include "netmessage.h"
#include "player.h"
#include "server.h"
#include "vehicles.h"
#include "buildings.h"

bool cGameTimer::syncDebugSingleStep = false;

Uint32 cGameTimer::gameTimerCallback (Uint32 interval, void* arg)
{
	reinterpret_cast<cGameTimer*> (arg)->timerCallback();
	return interval;
}

cGameTimer::cGameTimer() :
	mutex()
{
	gameTime = 0;
	lastTimerCall = 0;
	eventCounter = 0;

	maxEventQueueSize = -1;

	timer10ms = false;
	timer50ms = false;
	timer100ms = false;

	timerID = 0;
}

cGameTimer::~cGameTimer()
{
	stop();
}

void cGameTimer::start()
{
	if (timerID == 0)
		timerID = SDL_AddTimer (GAME_TICK_TIME, gameTimerCallback, this);
}

void cGameTimer::stop()
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

bool cGameTimer::popEvent()
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
	if (gameTime != lastTimerCall)
	{
		lastTimerCall = gameTime;
		timer10ms  = true;
		if (gameTime %  5 == 0) timer50ms  = true;
		if (gameTime % 10 == 0) timer100ms = true;
	}
}


void cGameTimer::setReceivedTime (unsigned int time, unsigned int nr)
{
	cMutex::Lock lock (mutex);

	while (receivedTime.size() <= nr)
		receivedTime.push_back (0);

	receivedTime[nr] = time;
}

unsigned int cGameTimer::getReceivedTime (unsigned int nr)
{
	cMutex::Lock lock (mutex);

	if (receivedTime.size() <= nr)
		return 0;

	return receivedTime[nr];
}

cGameTimerClient::cGameTimerClient() :
	cGameTimer(),
	client (0),
	remoteChecksum (0),
	localChecksum (0),
	waitingForServer (0),
	debugRemoteChecksum (0),
	gameTimeAdjustment (0),
	nextMsgIsNextGameTime (false)
{
}

void cGameTimerClient::setClient (cClient* client_)
{
	client = client_;
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
		client->disableFreezeMode (FREEZE_WAIT_FOR_SERVER);
		waitingForServer = 0;
		return true;
	}

	gameTimeAdjustment--;

	waitingForServer++;
	if (waitingForServer > MAX_WAITING_FOR_SERVER)
	{
		client->enableFreezeMode (FREEZE_WAIT_FOR_SERVER);
	}
	return false;
}

void cGameTimerClient::run (cMenu* activeMenu)
{
	// maximum time before GUI update
	const unsigned int maxWorkingTime = 500; // 500 milliseconds
	unsigned int startGameTime = SDL_GetTicks();

	while (popEvent())
	{
		client->getEventHandling().handleNetMessages (client, activeMenu);

		if (nextTickAllowed() == false) continue;

		gameTime++;
		handleTimer();
		client->doGameActions (activeMenu);

		//check crc
		localChecksum = calcClientChecksum (*client);
		debugRemoteChecksum = remoteChecksum;
		if (localChecksum != remoteChecksum)
		{
			//gameGUI.debugOutput.debugSync = true;
			Log.write ("OUT OF SYNC", cLog::eLOG_TYPE_NET_ERROR);
		}

		if (syncDebugSingleStep)
			compareGameData (*client, *client->getServer());

		nextMsgIsNextGameTime = false;

		//send "still alive" message to server
		//if (gameTime % (PAUSE_GAME_TIMEOUT / 10) == 0)
		{
			cNetMessage* message = new cNetMessage (NET_GAME_TIME_CLIENT);
			message->pushInt32 (gameTime);
			client->sendNetMessage (message);
		}
		if (SDL_GetTicks() - startGameTime >= maxWorkingTime)
			break;
	}

	//check whether the client time lags too much behind the server time and add an extra increment of the client time
	if (gameTime + MAX_CLIENT_LAG < getReceivedTime())
	{
		//inject an extra timer event
		timerCallback();
		gameTimeAdjustment++;
	}
}

cGameTimerServer::cGameTimerServer() :
	waitingForPlayer (-1)
{
}

void cGameTimerServer::handleSyncMessage (cNetMessage& message)
{
	assert (message.iType == NET_GAME_TIME_CLIENT);
	setReceivedTime (message.popInt32(), message.iPlayerNr);

}

bool cGameTimerServer::nextTickAllowed (cServer& server)
{
	if (syncDebugSingleStep)
	{
		if (getReceivedTime (0) < gameTime)
			return false;

		return true;
	}

	int newWaitingForPlayer = -1;

	std::vector<cPlayer*>& playerList = server.PlayerList;
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		cPlayer& player = *playerList[i];
		if (!server.isPlayerDisconnected (player) && getReceivedTime (i) + PAUSE_GAME_TIMEOUT < gameTime)
			newWaitingForPlayer = player.getNr();
	}

	if (newWaitingForPlayer != -1 && newWaitingForPlayer != waitingForPlayer)
	{
		server.enableFreezeMode (FREEZE_WAIT_FOR_PLAYER, newWaitingForPlayer);	//TODO: betreffenden player nicht mit freezenachrichten zuballern. Das ist kontraproduktiv.
	}
	else if (newWaitingForPlayer == -1 && waitingForPlayer != -1)
	{
		server.disableFreezeMode (FREEZE_WAIT_FOR_PLAYER);
	}

	waitingForPlayer = newWaitingForPlayer;

	return waitingForPlayer == -1;
}

void cGameTimerServer::run (cServer& server)
{
	if (popEvent() == false) return;
	if (nextTickAllowed (server) == false) return;

	gameTime++;
	handleTimer();
	server.doGameActions();
	const std::vector<cPlayer*>& playerList = server.PlayerList;
	for (size_t i = 0; i < playerList.size(); i++)
	{
		const cPlayer* player = playerList[i];
		AutoPtr<cNetMessage> message (new cNetMessage (NET_GAME_TIME_SERVER));

		message->pushInt32 (gameTime);
		const uint32_t checkSum = calcServerChecksum (server, player);
		message->pushInt32 (checkSum);
		server.sendNetMessage (message, player->getNr());
	}
}

uint32_t calcClientChecksum (const cClient& client)
{
	uint32_t crc = 0;
	const std::vector<cPlayer*>& players = client.getPlayerList();
	for (unsigned int i = 0; i < players.size(); i++)
	{
		for (const cVehicle* vehicle = players[i]->VehicleList;
			 vehicle;
			 vehicle = vehicle->next)
		{
			crc = calcCheckSum (vehicle->data.shotsCur, crc);
			crc = calcCheckSum (vehicle->data.ammoCur, crc);
			crc = calcCheckSum (vehicle->data.hitpointsCur, crc);
			crc = calcCheckSum (vehicle->data.speedCur, crc);
			crc = calcCheckSum (vehicle->FlightHigh, crc);
			crc = calcCheckSum (vehicle->iID,  crc);
			crc = calcCheckSum (vehicle->PosX, crc);
			crc = calcCheckSum (vehicle->PosY, crc);
			crc = calcCheckSum (vehicle->OffX, crc);
			crc = calcCheckSum (vehicle->OffY, crc);
			crc = calcCheckSum (vehicle->dir,  crc);
		}

		for (const cBuilding* building = players[i]->BuildingList;
			building;
			building = building->next)
		{
			crc = calcCheckSum(building->iID, crc);
			crc = calcCheckSum(building->data.shotsCur, crc);
			crc = calcCheckSum(building->data.ammoCur, crc);
			crc = calcCheckSum(building->data.hitpointsCur, crc);
			crc = calcCheckSum(building->dir, crc);
		}


	}
	return crc;
}

uint32_t calcServerChecksum (const cServer& server, const cPlayer* player)
{
	uint32_t crc = 0;
	const std::vector<cPlayer*>& playerList = server.PlayerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		for (const cVehicle* vehicle = playerList[i]->VehicleList;
			 vehicle;
			 vehicle = vehicle->next)
		{
			if (Contains (vehicle->seenByPlayerList, player) || vehicle->owner == player)
			{
				crc = calcCheckSum (vehicle->data.shotsCur, crc);
				crc = calcCheckSum (vehicle->data.ammoCur, crc);
				crc = calcCheckSum (vehicle->data.hitpointsCur, crc);
				crc = calcCheckSum(vehicle->data.speedCur, crc);
				crc = calcCheckSum(vehicle->FlightHigh, crc);
				crc = calcCheckSum (vehicle->iID,  crc);
				crc = calcCheckSum (vehicle->PosX, crc);
				crc = calcCheckSum (vehicle->PosY, crc);
				crc = calcCheckSum (vehicle->OffX, crc);
				crc = calcCheckSum (vehicle->OffY, crc);
				crc = calcCheckSum (vehicle->dir,  crc);
			}
		}

		for (const cBuilding* building = playerList[i]->BuildingList;
			building;
			building = building->next)
		{
			if (Contains(building->seenByPlayerList, player) || building->owner == player)
			{
				crc = calcCheckSum(building->iID, crc);
				crc = calcCheckSum(building->data.shotsCur, crc);
				crc = calcCheckSum(building->data.ammoCur, crc);
				crc = calcCheckSum(building->data.hitpointsCur, crc);
				crc = calcCheckSum(building->dir, crc);
			}
		}
	}
	return crc;
}

void compareGameData (const cClient& client, const cServer& server)
{
#if !defined (NDEBUG)
	const std::vector<cPlayer*>& players = client.getPlayerList();
	for (unsigned int i = 0; i < players.size(); i++)
	{
		const cPlayer* clientPlayer = players[i];

		for (const cVehicle* clientVehicle = clientPlayer->VehicleList;
			 clientVehicle;
			 clientVehicle = clientVehicle->next)
		{
			const cVehicle* serverVehicle = server.getVehicleFromID (clientVehicle->iID);

			assert (clientVehicle->PosX == serverVehicle->PosX);
			assert (clientVehicle->PosY == serverVehicle->PosY);
			assert (clientVehicle->OffX == serverVehicle->OffX);
			assert (clientVehicle->OffY == serverVehicle->OffY);
			assert (clientVehicle->dir == serverVehicle->dir);
			assert (clientVehicle->data.speedCur == serverVehicle->data.speedCur);
		}
	}
#endif
}

