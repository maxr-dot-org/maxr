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

#include <SDL.h>
#include "game/logic/gametimer.h"

#include "game/logic/client.h"
#include "game/logic/clientevents.h"
#include "utility/listhelpers.h"
#include "utility/files.h"
#include "utility/log.h"
#include "netmessage.h"
#include "game/data/player/player.h"
#include "game/logic/server.h"
#include "game/data/units/vehicle.h"

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

void cGameTimerClient::run ()
{
	// maximum time before GUI update
	const unsigned int maxWorkingTime = 500; // 500 milliseconds
	unsigned int startGameTime = SDL_GetTicks();

	while (popEvent())
	{
		client->handleNetMessages ();

		if (nextTickAllowed() == false) continue;

		gameTime++;
		gameTimeChanged ();
		handleTimer();
		client->doGameActions ();

		//check crc
		localChecksum = calcClientChecksum (*client);
		debugRemoteChecksum = remoteChecksum;
		if (localChecksum != remoteChecksum)
		{
			//gameGUI.debugOutput.debugSync = true;
			//Log.write ("OUT OF SYNC", cLog::eLOG_TYPE_NET_ERROR);
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

	const auto& playerList = server.playerList;
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
	gameTimeChanged ();
	handleTimer();
	server.doGameActions();
	const auto& playerList = server.playerList;
	for (size_t i = 0; i < playerList.size(); i++)
	{
		const auto& player = *playerList[i];
		AutoPtr<cNetMessage> message (new cNetMessage (NET_GAME_TIME_SERVER));

		message->pushInt32 (gameTime);
		const uint32_t checkSum = calcServerChecksum (server, &player);
		message->pushInt32 (checkSum);
		server.sendNetMessage (message, &player);
	}
}

uint32_t calcClientChecksum (const cClient& client)
{
	uint32_t crc = 0;
	const auto& players = client.getPlayerList();
	for (unsigned int i = 0; i < players.size(); i++)
	{
		const auto& vehicles = players[i]->getVehicles ();
		for (auto j = vehicles.begin (); j != vehicles.end (); ++j)
		{
			const auto& vehicle = *j;
			crc = calcCheckSum (vehicle->iID,  crc);
			crc = calcCheckSum (vehicle->getPosition().x(), crc);
			crc = calcCheckSum (vehicle->getPosition().y(), crc);
			crc = calcCheckSum (vehicle->getMovementOffset().x(), crc);
			crc = calcCheckSum (vehicle->getMovementOffset().y(), crc);
			crc = calcCheckSum (vehicle->dir,  crc);
		}
	}
	return crc;
}

uint32_t calcServerChecksum (const cServer& server, const cPlayer* player)
{
	uint32_t crc = 0;
	const auto& playerList = server.playerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		const auto& vehicles = playerList[i]->getVehicles ();
		for (auto j = vehicles.begin (); j != vehicles.end (); ++j)
		{
			const auto& vehicle = *j;
			if (Contains (vehicle->seenByPlayerList, player) || vehicle->owner == player)
			{
				crc = calcCheckSum (vehicle->iID,  crc);
				crc = calcCheckSum (vehicle->getPosition().x(), crc);
				crc = calcCheckSum (vehicle->getPosition().y(), crc);
				crc = calcCheckSum (vehicle->getMovementOffset().x(), crc);
				crc = calcCheckSum (vehicle->getMovementOffset().y(), crc);
				crc = calcCheckSum (vehicle->dir,  crc);
			}
		}
	}
	return crc;
}

void compareGameData (const cClient& client, const cServer& server)
{
#if !defined (NDEBUG)
	const auto& players = client.getPlayerList();
	for (unsigned int i = 0; i < players.size(); i++)
	{
		const auto& clientPlayer = players[i];

		const auto& vehicles = clientPlayer->getVehicles ();
		for (auto j = vehicles.begin (); j != vehicles.end (); ++j)
		{
			const auto& clientVehicle = *j;
			const cVehicle* serverVehicle = server.getVehicleFromID (clientVehicle->iID);

			assert (clientVehicle->getPosition() == serverVehicle->getPosition());
			assert (clientVehicle->getMovementOffset() == serverVehicle->getMovementOffset());
			assert (clientVehicle->dir == serverVehicle->dir);
		}
	}
#endif
}

