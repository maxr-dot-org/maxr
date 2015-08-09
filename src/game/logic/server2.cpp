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

#include <SDL_thread.h>

#include "server2.h"
#include "client.h"
#include "action.h"
#include "utility/log.h"
#include "game/data/player/playerbasicdata.h"
#include <time.h>
#include "utility/random.h"

//------------------------------------------------------------------------------
cServer2::cServer2() :
	model(),
	gameTimer(),
	localClient(nullptr),
	serverThread(nullptr),
	bExit(false)
{

}

//------------------------------------------------------------------------------
cServer2::~cServer2()
{
	stop();
	// disconnect clients
	/*if (network)
	{
		for (size_t i = 0; i != playerList.size(); ++i)
		{
			network->close(playerList[i]->getSocketNum());
		}
		network->setMessageReceiver(nullptr);
	}*/
}

//------------------------------------------------------------------------------
void cServer2::setLocalClient(cClient* client)
{
	localClient = client;
}

//------------------------------------------------------------------------------
void cServer2::setMap(std::shared_ptr<cStaticMap> staticMap)
{
	model.setMap(staticMap);
}

//------------------------------------------------------------------------------
void cServer2::setGameSettings(const cGameSettings& gameSettings)
{
	model.setGameSettings(gameSettings);
}
//------------------------------------------------------------------------------
void cServer2::setPlayers(const std::vector<cPlayerBasicData>& splayers)
{
	model.setPlayerList(splayers);
	gameTimer.setNumberOfPlayers(splayers.size());
}
//------------------------------------------------------------------------------
void cServer2::pushMessage(std::unique_ptr<cNetMessage2> message)
{
	eventQueue.push(std::move(message));
}


//------------------------------------------------------------------------------
//TODO: send to specific player
void cServer2::sendMessageToClients(std::unique_ptr<cNetMessage2> message, int playerNr) const
{
	if (message->getType() != cNetMessage2::GAMETIME_SYNC_SERVER)
	{
		cTextArchiveIn archive;
		archive << *message;
		Log.write("Server: --> Data: " + archive.data() + " @" + iToStr(gameTimer.gameTime), cLog::eLOG_TYPE_NET_DEBUG);

	}


	//TODO: network
	//if (network)
	//	network->send(message->iLength, message->data);

	localClient->pushMessage(std::move(message));

}

//------------------------------------------------------------------------------
void cServer2::start()
{
	if (serverThread) return;

	initRandomGenerator();

	serverThread = SDL_CreateThread(serverThreadCallback, "server", this);
	gameTimer.maxEventQueueSize = MAX_SERVER_EVENT_COUNTER;
	gameTimer.start();
}

//------------------------------------------------------------------------------
void cServer2::stop()
{
	bExit = true;
	gameTimer.stop();

	if (serverThread)
	{
		SDL_WaitThread(serverThread, nullptr);
		serverThread = nullptr;
	}
}

//------------------------------------------------------------------------------
void cServer2::run()
{
	while (!bExit)
	{
		std::unique_ptr<cNetMessage2> message;
		while (eventQueue.try_pop(message))
		{
			if (message->getType() != cNetMessage2::GAMETIME_SYNC_CLIENT)
			{
				cTextArchiveIn archive;
				archive << *message;
				Log.write("Server: <-- Data: " + archive.data() + " @" + iToStr(gameTimer.gameTime), cLog::eLOG_TYPE_NET_DEBUG);
			}

			switch (message->getType())
			{
			case cNetMessage2::ACTION:
				{
					const cAction* action = static_cast<cAction*>(message.get());
					action->execute(model);

					sendMessageToClients(std::move(message));
				}
				break;
			case cNetMessage2::GAMETIME_SYNC_CLIENT:
				{
					const cNetMessageSyncClient& syncMessage = *static_cast<cNetMessageSyncClient*>(message.get());
					gameTimer.handleSyncMessage(syncMessage);
				}
				break;
			default:
				Log.write(" Server: Can not handle net message!", cLog::eLOG_TYPE_NET_ERROR);
				sendMessageToClients(std::move(message));
				break;
			}
			
		}

		//TODO: gameinit: start timer, when all clients are ready
		gameTimer.run(model, *this);

		SDL_Delay(10);
	}
}

void cServer2::initRandomGenerator()
{
	uint64_t t = random(UINT64_MAX);
	model.randomGenerator.seed(t);
	auto msg = std::make_unique<cNetMessageRandomSeed>(t);
	sendMessageToClients(std::move(msg));
}

//------------------------------------------------------------------------------
int cServer2::serverThreadCallback(void* arg)
{
	cServer2* server = reinterpret_cast<cServer2*> (arg);
	server->run();
	return 0;
}
