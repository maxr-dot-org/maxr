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
#include "game/logic/action/action.h"
#include "utility/log.h"
#include "game/data/player/playerbasicdata.h"
#include <time.h>
#include "utility/random.h"
#include "game/data/savegame.h"
#include "netmessage2.h"
#include "utility/string/toString.h"
#include "connectionmanager.h"
#include "game/data/report/special/savedreportlostconnection.h"

//------------------------------------------------------------------------------
cServer2::cServer2(std::shared_ptr<cConnectionManager> connectionManager) :
	model(),
	gameTimer(),
	serverThread(nullptr),
	exit(false),
	connectionManager(connectionManager)
{
	model.turnEnded.connect([&]()
	{
		enableFreezeMode(eFreezeMode::WAIT_FOR_TURNEND);
	});
	model.newTurnStarted.connect([&]()
	{
		if (cSettings::getInstance().shouldAutosave())
		{
			saveGameState(10, lngPack.i18n("Text~Comp~Turn_5") + " " + toString(model.getTurnCounter()->getTurn()) + " - " + lngPack.i18n("Text~Settings~Autosave"));
		}
		disableFreezeMode(eFreezeMode::WAIT_FOR_TURNEND);
	});
}

//------------------------------------------------------------------------------
cServer2::~cServer2()
{
	stop();
	connectionManager->closeServer();
	connectionManager->disconnectAll();
	connectionManager->setLocalServer(nullptr);
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
	gameTimer.setPlayerNumbers(model.getPlayerList());
}

//------------------------------------------------------------------------------
const cModel& cServer2::getModel() const
{
	return model;
}

//------------------------------------------------------------------------------
void cServer2::saveGameState(int saveGameNumber, const std::string& saveName) const
{
	if (SDL_ThreadID() != SDL_GetThreadID(serverThread))
	{
		//allow save writing of the server model from the main thread
		exit = true;
		SDL_WaitThread(serverThread, nullptr);
		serverThread = nullptr;
	}

	Log.write(" Server: writing gamestate to save file " + toString(saveGameNumber) + ", Modelcrc: " + toString(model.getChecksum()), cLog::eLOG_TYPE_NET_DEBUG);

	int saveingID = savegame.save(model, saveGameNumber, saveName);
	cNetMessageRequestGUISaveInfo message(saveingID);
	sendMessageToClients(message);

	if (!serverThread)
	{
		exit = false;
		serverThread = SDL_CreateThread(serverThreadCallback, "server", const_cast<cServer2*>(this));
	}
}
//------------------------------------------------------------------------------
void cServer2::loadGameState(int saveGameNumber)
{
	Log.write(" Server: loading game state from save file " + iToStr(saveGameNumber), cLog::eLOG_TYPE_NET_DEBUG);
	savegame.loadModel(model, saveGameNumber);

	gameTimer.setPlayerNumbers(model.getPlayerList());
}
//------------------------------------------------------------------------------
void cServer2::sendGuiInfoToClients(int saveGameNumber, int playerNr /*= -1*/)
{
	try 
	{
		savegame.loadGuiInfo(this, saveGameNumber);
	}
	catch (std::runtime_error& e)
	{
		Log.write((std::string) " Server: Loading GuiInfo from savegame failed: " + e.what(), cLog::eLOG_TYPE_NET_ERROR);
	}
}

//------------------------------------------------------------------------------
void cServer2::resyncClientModel(int playerNr /*= -1*/) const
{
	assert(SDL_ThreadID() == SDL_GetThreadID(serverThread));

	Log.write(" Server: Resynchronize client model " + iToStr(playerNr), cLog::eLOG_TYPE_NET_DEBUG);
	cNetMessageResyncModel msg(model);
	sendMessageToClients(msg, playerNr);
}

//------------------------------------------------------------------------------
void cServer2::pushMessage(std::unique_ptr<cNetMessage2> message)
{
	eventQueue.push(std::move(message));
}


//------------------------------------------------------------------------------
void cServer2::sendMessageToClients(const cNetMessage2& message, int playerNr /* = -1 */) const
{
	if (message.getType() != eNetMessageType::GAMETIME_SYNC_SERVER && message.getType() != eNetMessageType::RESYNC_MODEL)
	{
		cTextArchiveIn archive;
		archive << message;
		Log.write("Server: --> " + archive.data() + " @" + iToStr(model.getGameTime()), cLog::eLOG_TYPE_NET_DEBUG);
	}

	if (playerNr == -1)
	{
		connectionManager->sendToPlayers(message);
	}
	else if (connectionManager->isPlayerConnected(playerNr))
	{
		connectionManager->sendToPlayer(message, playerNr);
	}
}

//------------------------------------------------------------------------------
void cServer2::start()
{
	if (serverThread) return;

	initRandomGenerator();
	initPlayerConnectionState();
	updateWaitForClientFlag();

	serverThread = SDL_CreateThread(serverThreadCallback, "server", this);
	gameTimer.maxEventQueueSize = MAX_SERVER_EVENT_COUNTER;
	gameTimer.start();
}

//------------------------------------------------------------------------------
void cServer2::stop()
{
	exit = true;
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
	while (!exit)
	{
		std::unique_ptr<cNetMessage2> message;
		while (eventQueue.try_pop(message))
		{
			if (message->getType() != eNetMessageType::GAMETIME_SYNC_CLIENT)
			{
				cTextArchiveIn archive;
				archive << *message;
				Log.write("Server: <-- " + archive.data() + " @" + iToStr(model.getGameTime()), cLog::eLOG_TYPE_NET_DEBUG);
			}

			if (model.getPlayer(message->playerNr) == nullptr && 
				message->getType() != eNetMessageType::TCP_WANT_CONNECT) continue;

			switch (message->getType())
			{
			case eNetMessageType::ACTION:
			{
				const cAction& action = *static_cast<cAction*>(message.get());
				
				// filter disallowed actions
				if (action.getType() != cAction::eActiontype::ACTION_INIT_NEW_GAME)
				{
					if (freezeModes.isFreezed())
					{
						Log.write(" Server: Discarding action, because game is freezed.", cLog::eLOG_TYPE_NET_WARNING);
						break;
					}
					if (model.getGameSettings()->getGameType() == eGameSettingsGameType::Turns && message->playerNr != model.getActiveTurnPlayer()->getId())
					{
						Log.write(" Server: Discarding action, because it's another players turn.", cLog::eLOG_TYPE_NET_WARNING);
						break;
					}
				}

				action.execute(model);

				sendMessageToClients(*message);
				break;
			}
			case eNetMessageType::GAMETIME_SYNC_CLIENT:
			{
				const cNetMessageSyncClient& syncMessage = *static_cast<cNetMessageSyncClient*>(message.get());
				gameTimer.handleSyncMessage(syncMessage, model.getGameTime());
				break;
			}
			case eNetMessageType::REPORT:
			{
				sendMessageToClients(*message);
				break;
			}
			case eNetMessageType::GUI_SAVE_INFO:
			{
				const cNetMessageGUISaveInfo& saveInfo = *static_cast<cNetMessageGUISaveInfo*>(message.get());
				savegame.saveGuiInfo(saveInfo);
				break;
			}
			case eNetMessageType::REQUEST_RESYNC_MODEL:
			{
				const cNetMessageRequestResync& requestMessage = *static_cast<cNetMessageRequestResync*>(message.get());
				resyncClientModel(requestMessage.playerToSync);
				if (requestMessage.saveNumberForGuiInfo != -1)
				{
					sendGuiInfoToClients(requestMessage.saveNumberForGuiInfo, requestMessage.playerToSync);
				}
				break;
			}
			case eNetMessageType::TCP_WANT_CONNECT:
			{
				const cNetMessageTcpWantConnect& connectMessage = *static_cast<cNetMessageTcpWantConnect*>(message.get());
				const cPlayer* player = model.getPlayer(connectMessage.playerName);
				if (player == nullptr)
				{
					Log.write(" Server: Connecting player " + connectMessage.playerName + " is not part of the game", cLog::eLOG_TYPE_NET_WARNING);
					connectionManager->declineConnection(connectMessage.socket, "Text~Multiplayer~Reconnect_Not_Part_Of_Game");
					break;
				}
				if (connectionManager->isPlayerConnected(player->getId()))
				{
					Log.write(" Server: Connecting player " + connectMessage.playerName + " is already connected", cLog::eLOG_TYPE_NET_WARNING);
					connectionManager->declineConnection(connectMessage.socket, "Text~Multiplayer~Reconnect_Already_Connected");
					break;
				}

				connectionManager->acceptConnection(connectMessage.socket, player->getId());

				sendMessageToClients(cNetMessageGameAlreadyRunning(model), player->getId());
				break;
			}
			case eNetMessageType::TCP_CLOSE:
			{
				const cNetMessageTcpClose&  msg = *static_cast<cNetMessageTcpClose*>(message.get());
				sendMessageToClients(cNetMessageReport(std::make_unique<cSavedReportLostConnection>(*model.getPlayer(msg.playerNr))));
				playerDisconnected(msg.playerNr);
				break;
			}
			case eNetMessageType::WANT_REJOIN_GAME:
			{
				const cNetMessageWantRejoinGame&  msg = *static_cast<cNetMessageWantRejoinGame*>(message.get());

				const auto player = model.getPlayer(msg.playerNr);
				if (player == nullptr)
				{
					Log.write(" Server: Invalid player id: " + toString(msg.playerNr), cLog::eLOG_TYPE_NET_ERROR);
					break;
				}

				resyncClientModel(message->playerNr);
				playerConnected(msg.playerNr);

				if (savegame.getLastUsedSaveSlot() != -1)
				{
					savegame.loadGuiInfo(this, savegame.getLastUsedSaveSlot(), message->playerNr);
				}
				break;
			}
			default:
				Log.write(" Server: Can not handle net message!", cLog::eLOG_TYPE_NET_ERROR);
				break;
			}
			
		}

		//TODO: gameinit: start timer, when all clients are ready
		gameTimer.run(model, *this);

		SDL_Delay(10);
	}
}

//------------------------------------------------------------------------------
void cServer2::setUnitsData(std::shared_ptr<const cUnitsData> unitsData)
{
	model.setUnitsData(std::make_shared<cUnitsData>(*unitsData));
}

//------------------------------------------------------------------------------
void cServer2::initRandomGenerator()
{
	uint64_t t = random(UINT64_MAX);
	model.randomGenerator.seed(t);
	cNetMessageRandomSeed msg(t);
	sendMessageToClients(msg);
}

//------------------------------------------------------------------------------
void cServer2::enableFreezeMode(eFreezeMode mode)
{
	freezeModes.enable(mode);
	updateGameTimerstate();

	sendMessageToClients(cNetMessageFreezeModes(freezeModes, playerConnectionStates));
}

//------------------------------------------------------------------------------
void cServer2::disableFreezeMode(eFreezeMode mode)
{
	freezeModes.disable(mode);
	updateGameTimerstate();

	sendMessageToClients(cNetMessageFreezeModes(freezeModes, playerConnectionStates));
}

//------------------------------------------------------------------------------
void cServer2::setPlayerNotResponding(int playerId)
{
	if (playerConnectionStates[playerId] != ePlayerConnectionState::CONNECTED) return;

	playerConnectionStates[playerId] = ePlayerConnectionState::NOT_RESPONDING;
	Log.write(" Server: Player " + toString(playerId) + " not responding", cLog::eLOG_TYPE_NET_DEBUG);
	updateWaitForClientFlag();
}

//------------------------------------------------------------------------------
void cServer2::clearPlayerNotResponding(int playerId)
{
	if (playerConnectionStates[playerId] != ePlayerConnectionState::NOT_RESPONDING) return;

	playerConnectionStates[playerId] = ePlayerConnectionState::CONNECTED;
	Log.write(" Server: Player " + toString(playerId) + " responding again", cLog::eLOG_TYPE_NET_DEBUG);
	updateWaitForClientFlag();
}

//------------------------------------------------------------------------------
void cServer2::playerDisconnected(int playerId)
{
	const auto player = model.getPlayer(playerId);
	if (player->isDefeated)
	{
		playerConnectionStates[playerId] = ePlayerConnectionState::INACTIVE;
	}
	else
	{
		//TODO: set to INACTIVE when running in dedicated mode
		playerConnectionStates[playerId] = ePlayerConnectionState::DISCONNECTED;
	}
	Log.write(" Server: Player " + toString(playerId) + " disconnected", cLog::eLOG_TYPE_NET_DEBUG);
	updateWaitForClientFlag();
}

//------------------------------------------------------------------------------
void cServer2::playerConnected(int playerId)
{
	playerConnectionStates[playerId] = ePlayerConnectionState::CONNECTED;
	Log.write(" Server: Player " + toString(playerId) + " connected", cLog::eLOG_TYPE_NET_DEBUG);
	updateWaitForClientFlag(); 
}

//------------------------------------------------------------------------------
void cServer2::updateWaitForClientFlag()
{
	bool freeze = false;
	for (auto state : playerConnectionStates)
	{
		if (state.second == ePlayerConnectionState::DISCONNECTED || state.second == ePlayerConnectionState::NOT_RESPONDING)
		{
			freeze = true;
		}
	}

	if (freeze)
	{
		enableFreezeMode(eFreezeMode::WAIT_FOR_CLIENT);
	}
	else
	{
		disableFreezeMode(eFreezeMode::WAIT_FOR_CLIENT);
	}
}

//------------------------------------------------------------------------------
void cServer2::updateGameTimerstate()
{
	if (freezeModes.gameTimePaused())
	{
		gameTimer.stop();
	}
	else if (serverThread != nullptr)
	{
		gameTimer.start();
	}
}

//------------------------------------------------------------------------------
void cServer2::initPlayerConnectionState()
{
	for (const auto player : model.getPlayerList())
	{
		if (connectionManager->isPlayerConnected(player->getId()))
		{
			playerConnectionStates[player->getId()] = ePlayerConnectionState::CONNECTED;
		}
		else
		{
			// assume that game always start with all necessary players connected.
			// So set the disconnected players to INACTIVE
			playerConnectionStates[player->getId()] = ePlayerConnectionState::INACTIVE;
		}
	}
}

//------------------------------------------------------------------------------
int cServer2::serverThreadCallback(void* arg)
{
	cServer2* server = reinterpret_cast<cServer2*> (arg);
	server->run();
	return 0;
}
