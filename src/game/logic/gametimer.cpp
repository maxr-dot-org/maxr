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
#include "utility/listhelpers.h"
#include "utility/files.h"
#include "utility/log.h"
#include "netmessage2.h"
#include "game/data/player/player.h"
#include "game/logic/server2.h"
#include "game/data/model.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"

Uint32 cGameTimer::gameTimerCallback (Uint32 interval, void* arg)
{
	reinterpret_cast<cGameTimer*> (arg)->timerCallback();
	return interval;
}

cGameTimer::cGameTimer() :
	mutex()
{
	eventCounter = 0;
	maxEventQueueSize = -1;
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
	// Obviously this is the only way, to increase the timer thread priority in SDL.
	// This is needed, because the timer loose events otherwise, when the CPU usage is too high.
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);

	pushEvent();

}

void cGameTimer::pushEvent()
{
	cLockGuard<cMutex> lock(mutex);

	//increase event counter and let the event handler increase the gametime
	if (eventCounter < maxEventQueueSize || maxEventQueueSize == -1)
	{
		eventCounter++;
	}
}

bool cGameTimer::popEvent()
{
	cLockGuard<cMutex> lock (mutex);

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

void cGameTimerServer::setNumberOfPlayers(unsigned int players)
{
	receivedTime.resize(players);
	clientDebugData.resize(players);
}

void cGameTimerServer::handleSyncMessage(const cNetMessageSyncClient& message, unsigned int gameTime)
{
	int playerNr = message.playerNr;

	if (message.gameTime > gameTime)
	{
		Log.write(" Server: the received game time from client is in the future", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	receivedTime[playerNr] = message.gameTime;

	//save debug data from clients
	clientDebugData[playerNr].crcOK = message.crcOK;
	const float filter = 0.1F;
	clientDebugData[playerNr].eventCounter  = (1-filter)*clientDebugData[playerNr].eventCounter  + filter*message.eventCounter;
	clientDebugData[playerNr].queueSize     = (1-filter)*clientDebugData[playerNr].queueSize     + filter*message.queueSize;
	clientDebugData[playerNr].ticksPerFrame = (1-filter)*clientDebugData[playerNr].ticksPerFrame + filter*message.ticksPerFrame;
	clientDebugData[playerNr].timeBuffer    = (1-filter)*clientDebugData[playerNr].timeBuffer    + filter*message.timeBuffer;
	clientDebugData[playerNr].ping          = (1-filter)*clientDebugData[playerNr].ping          + filter*10*(gameTime - message.gameTime);
}

void cGameTimerServer::checkPlayersResponding(const std::vector<std::shared_ptr<cPlayer>>& playerList, cServer2& server)
{
	for (auto player : playerList)
	{
		//TODO: playerconnection manager
	}
}

void cGameTimerServer::run(cModel& model, cServer2& server)
{
	checkPlayersResponding(model.getPlayerList(), server);

	for (unsigned int i = 0; i < maxEventQueueSize; i++)
	{
		if (!popEvent()) break;
		
		model.advanceGameTime();

		uint32_t checksum = model.calcChecksum();
		for (auto player : model.getPlayerList())
		{
			cNetMessageSyncServer message;
			message.checksum = checksum;
			message.ping = static_cast<int>(clientDebugData[player->getId()].ping);
			message.gameTime = model.getGameTime();
			server.sendMessageToClients(message, player->getId());
		}
	}
}


cGameTimerClient::cGameTimerClient() :
	cGameTimer(),
	remoteChecksum (0),
	localChecksum (0),
	receivedTime(0),
	timeSinceLastSyncMessage (0),
	debugRemoteChecksum (0),
	syncMessageReceived (false)
{
}

void cGameTimerClient::setReceivedTime(unsigned int time)
{
	cLockGuard<cMutex> lock(mutex);
	receivedTime = time;
}

unsigned int cGameTimerClient::getReceivedTime()
{
	cLockGuard<cMutex> lock(mutex);

	return receivedTime;
}

void cGameTimerClient::handleSyncMessage(const cNetMessageSyncServer& message, unsigned int gameTime)
{

	remoteChecksum = message.checksum;
	ping = message.ping;

	if (message.gameTime != gameTime + 1)
		Log.write ("Game Synchonisation Error: Received out of order sync message", cLog::eLOG_TYPE_NET_ERROR);

	syncMessageReceived = true;
}

void cGameTimerClient::checkServerResponding(cClient& client)
{
	if (syncMessageReceived)
	{
		client.disableFreezeMode(FREEZE_WAIT_FOR_SERVER);
		timeSinceLastSyncMessage = 0;
	}
	else
	{
		timeSinceLastSyncMessage++;
		if (timeSinceLastSyncMessage > MAX_WAITING_FOR_SERVER)
		{
			client.enableFreezeMode(FREEZE_WAIT_FOR_SERVER);
		}
	}
}

void cGameTimerClient::run(cClient& client, cModel& model)
{
	// maximum time before GUI update
	const unsigned int maxWorkingTime = 500; // milliseconds
	unsigned int startGameTime = SDL_GetTicks();

	//collect some debug data
	const unsigned int timeBuffer = getReceivedTime() - model.getGameTime();
	const unsigned int tickPerFrame = std::min(timeBuffer, eventCounter);	//assumes, that this function is called once per frame
																			//and we are not running in the maxWorkingTime limit

	while (popEvent())
	{
		if (!syncMessageReceived)
		{
			client.handleNetMessages();
		}
		checkServerResponding(client);

		if (syncMessageReceived)
		{
			
			model.advanceGameTime();

			//check crc
			localChecksum = model.calcChecksum();
			debugRemoteChecksum = remoteChecksum;
			if (localChecksum != remoteChecksum)
			{
				Log.write("OUT OF SYNC", cLog::eLOG_TYPE_NET_ERROR);
			}

			syncMessageReceived = false;

			//send syncMessage
			cNetMessageSyncClient message;
			message.gameTime = model.getGameTime();
			//add debug data
			message.crcOK = (localChecksum == remoteChecksum);
			message.eventCounter = eventCounter;
			message.queueSize = client.getNetMessageQueueSize();
			message.ticksPerFrame = tickPerFrame;
			message.timeBuffer = timeBuffer;
				
			client.sendNetMessage(message);
			
			if (SDL_GetTicks() - startGameTime >= maxWorkingTime)
				break;
		}
	}

	//check whether the client time lags too much behind the server time and add an extra increment of the client time
	if (model.getGameTime() + MAX_CLIENT_LAG < getReceivedTime())
	{
		//inject an extra timer event
		for (int i = 0; i < (getReceivedTime() - model.getGameTime()) / 2; i++)
			pushEvent();
	}
}
